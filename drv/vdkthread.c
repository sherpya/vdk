/*
	vdkthread.c

	Virtual Disk kernel-mode driver for Windows NT platform
	Device thread routines
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkioctl.h"
#include "vdkfile.h"
#include "vdkaccess.h"

#include "imports.h"
#include "vdkdrv.h"

static NTSTATUS
VdkOpen(
	IN PDEVICE_OBJECT	DiskObject,
	IN PVDK_OPEN_FILE_INFO 	OpenFile);

static NTSTATUS
VdkClose(
	IN PDISK_EXTENSION	DiskExtension);

static NTSTATUS
VdkFormat(
	IN PVDK_DISK_INFO	DiskInfo,
	IN PFORMAT_PARAMETERS	FormatParam);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdkOpen)
#pragma alloc_text(PAGE, VdkClose)
#pragma alloc_text(PAGE, VdkFormat)
#endif	// ALLOC_PRAGMA

//
// Substitute for MmGetSystemAddressForMdlSafe
// for NT 4.0 DDK does not provide its equivqlent.
// originally written by Bruce Engle for filedisk
//
static PVOID MmGetSystemAddressForMdlPrettySafe (
	IN PMDL 			Mdl,
	IN MM_PAGE_PRIORITY	Priority)
{
	if (OsMajorVersion == 0) {
		StoreCurrentOsVersion();
	}

	if (OsMajorVersion >= 5) {
		return MmGetSystemAddressForMdlSafe(Mdl, Priority);
	}
	else {
		CSHORT	MdlMappingCanFail;
		PVOID	MappedSystemVa;

		MdlMappingCanFail = (CSHORT)(Mdl->MdlFlags & MDL_MAPPING_CAN_FAIL);

		Mdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;

		MappedSystemVa = MmGetSystemAddressForMdlSafe(Mdl, MdlMappingCanFail ? NormalPagePriority : HighPagePriority);

		if (!MdlMappingCanFail) {
			Mdl->MdlFlags &= ~MDL_MAPPING_CAN_FAIL;
		}

		return MappedSystemVa;
	}
}

//
// Device dedicated thread routine
// Handles read, write, open, close operation
//
VOID
VdkThread (
	IN PVOID Context)	// Disk device object
{
	PDEVICE_OBJECT		disk_object;
	PDISK_EXTENSION		disk_extension;
	PLIST_ENTRY 		request;
	PIRP				irp;
	PIO_STACK_LOCATION	io_stack;

	disk_object = (PDEVICE_OBJECT)Context;
	disk_extension = (PDISK_EXTENSION)disk_object->DeviceExtension;

	KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

	for (;;) {
		//
		// wait for the request event to be signalled
		//
		KeWaitForSingleObject(
			&disk_extension->RequestEvent,
			Executive,
			KernelMode,
			FALSE,
			NULL);

		//
		// terminate request ?
		//
		if (disk_extension->TerminateThread) {
			VDKTRACE(VDKINFO,
				("[VDK] Exitting device thread\n"));

			PsTerminateSystemThread(STATUS_SUCCESS);
		}

		//
		// perform requested task
		//
		while ((request = ExInterlockedRemoveHeadList(
			&disk_extension->ListHead, &disk_extension->ListLock)) != NULL) {

			irp = CONTAINING_RECORD(request, IRP, Tail.Overlay.ListEntry);

			io_stack = IoGetCurrentIrpStackLocation(irp);

			irp->IoStatus.Information = 0;

			switch (io_stack->MajorFunction) {
			case IRP_MJ_READ:
				if (!disk_extension->DiskInfo.DiskType) {
					irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;

					VDKTRACE(VDKWARN | VDKREAD,
						("[VDK] IRP_MJ_READ: STATUS_DEVICE_NOT_READY\n"));

					break;
				}

				irp->IoStatus.Status = VdkReadSector(
					&disk_extension->DiskInfo,
					(ULONG)(io_stack->Parameters.Read.ByteOffset.QuadPart >>
						VDK_BYTE_SHIFT_TO_SECTOR),
					io_stack->Parameters.Read.Length >>
						VDK_BYTE_SHIFT_TO_SECTOR,
					MmGetSystemAddressForMdlPrettySafe(
						irp->MdlAddress, NormalPagePriority));

				if (NT_SUCCESS(irp->IoStatus.Status)) {
					irp->IoStatus.Information =
						io_stack->Parameters.Read.Length;
				}
				break;

			case IRP_MJ_WRITE:
				if (!disk_extension->DiskInfo.DiskType) {
					irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;

					VDKTRACE(VDKWARN | VDKWRITE,
						("[VDK] IRP_MJ_READ: STATUS_DEVICE_NOT_READY\n"));

					break;
				}

				irp->IoStatus.Status = VdkWriteSector(
					&disk_extension->DiskInfo,
					(ULONG)(io_stack->Parameters.Write.ByteOffset.QuadPart >>
						VDK_BYTE_SHIFT_TO_SECTOR),
					io_stack->Parameters.Write.Length >>
						VDK_BYTE_SHIFT_TO_SECTOR,
					MmGetSystemAddressForMdlPrettySafe(
						irp->MdlAddress, NormalPagePriority));

				if (NT_SUCCESS(irp->IoStatus.Status)) {
					irp->IoStatus.Information =
						io_stack->Parameters.Read.Length;
				}
				break;

			case IRP_MJ_DEVICE_CONTROL:
				switch (io_stack->Parameters.DeviceIoControl.IoControlCode) {
				case IOCTL_VDK_OPEN_FILE:

#ifdef VDK_SUPPORT_NETWORK
					SeImpersonateClient(disk_extension->SecurityContext, NULL);
#endif	// VDK_SUPPORT_NETWORK

					irp->IoStatus.Status = VdkOpen(
						disk_object,
						(PVDK_OPEN_FILE_INFO)irp->AssociatedIrp.SystemBuffer);

#ifdef VDK_SUPPORT_NETWORK
					PsRevertToSelf();
#endif	// VDK_SUPPORT_NETWORK

					break;

				case IOCTL_VDK_CLOSE_FILE:
					//
					// Mark this device as a zombie if the volume is not dismounted
					//
					if ((io_stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG) ||
						!*(PULONG)irp->AssociatedIrp.SystemBuffer) &&
						disk_object->Vpb &&
						disk_object->Vpb->ReferenceCount > 1) {

						VDKTRACE(VDKIOCTL|VDKWARN,
							("[VDK] %ws becomes a zombie\n",
							disk_extension->DeviceName.Buffer));

						disk_extension->PartitionOrdinal = VDK_ZOMBIE;
					}

					irp->IoStatus.Status = VdkClose(disk_extension);
					break;

				case IOCTL_VDK_CREATE_DISK:
					{
						PDEVICE_OBJECT	device_obj;
						ULONG	disk_num = 0;

						device_obj = disk_object->DriverObject->DeviceObject;

						//
						//	count number of existing virtual disk devices
						//
						while (device_obj) {
							if (((PPART_EXTENSION)device_obj->DeviceExtension)->FirstPartition
								== (PDISK_EXTENSION)device_obj->DeviceExtension) {
								disk_num++;
							}
							device_obj = device_obj->NextDevice;
						}

						if (disk_num >= VDK_MAXIMUM_DISK_NUM) {
							VDKTRACE(VDKCREATE|VDKWARN,
								("[VDK] No more disk devies.\n"));

							irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
							break;
						}

						//
						//	create new virtual disk device
						//
						irp->IoStatus.Status = VdkCreateDisk(disk_object->DriverObject, disk_num);
						break;
					}

				case IOCTL_VDK_DELETE_DISK:
					{
						PDEVICE_OBJECT device_obj;
						PDISK_EXTENSION disk_ext = NULL;

						device_obj = disk_object->DriverObject->DeviceObject;

						//
						//	Look for the last created disk device (first in the chain)
						//
						while (device_obj) {
							disk_ext = (PDISK_EXTENSION)device_obj->DeviceExtension;

							if (disk_ext == disk_ext->FirstPartition) {
								break;
							}

							device_obj = device_obj->NextDevice;
						}

						if (!device_obj) {
							VDKTRACE(VDKDELETE,
								("[VDK] Disk device not found\n"));

							irp->IoStatus.Status = STATUS_DRIVER_INTERNAL_ERROR;
							break;
						}

						//
						//	Check if the device can be deleted
						//

						if (disk_ext->VirtualNumber == 0) {
							VDKTRACE(VDKDELETE,
								("[VDK] Cannot delete disk 0\n"));

							irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
							break;
						}

						if (disk_ext->DiskInfo.DiskType) {
							VDKTRACE(VDKDELETE,
								("[VDK] Device %ws in use.\n",
								disk_ext->DeviceName.Buffer));

							irp->IoStatus.Status = STATUS_DEVICE_BUSY;
							break;
						}

						if (device_obj->Vpb && device_obj->Vpb->ReferenceCount) {
							VDKTRACE(VDKDELETE,
								("[VDK] Device %ws reference count %lu\n",
								disk_ext->DeviceName.Buffer,
								device_obj->Vpb->ReferenceCount));

							irp->IoStatus.Status = STATUS_DEVICE_BUSY;
							break;
						}

						//
						//	Delete the device
						//
						VdkDeleteDevice(device_obj);

						irp->IoStatus.Status = STATUS_SUCCESS;
						break;
					}

				default:
					// This shouldn't happen...
					irp->IoStatus.Status = STATUS_DRIVER_INTERNAL_ERROR;
				}

				VDKTRACE((VDKINFO | VDKIOCTL),
					("[VDK] IRP_MJ_DEVICE_CONTROL\n"));

#if DBG
				if ((TraceFlags & VDKIOCTL)	&&
					(!NT_SUCCESS(irp->IoStatus.Status) || (TraceFlags & VDKINFO) == VDKINFO)) {

					PrintIoCtrlStatus(
						disk_extension->DeviceName.Buffer,
						io_stack->Parameters.DeviceIoControl.IoControlCode,
						irp->IoStatus.Status);
				}
#endif	// DBG
				break;

			default:
				// This shouldn't happen...
				irp->IoStatus.Status = STATUS_DRIVER_INTERNAL_ERROR;

				VDKTRACE(0,
					("[VDK] Unknown major function 0x%08lu\n",
					io_stack->MajorFunction));
			}

			if (irp->IoStatus.Status == STATUS_FILE_FORCED_CLOSED) {
				//
				// File has been forced to close e.g. due to network condition
				// Must update driver status
				//
				VDKTRACE(VDKIOCTL|VDKWARN,
					("[VDK] %ws becomes a zombie due to forced close.\n",
					disk_extension->DeviceName.Buffer));

				disk_extension->DiskInfo.DiskType = VDK_DISKTYPE_NONE;
				disk_extension->PartitionOrdinal = VDK_ZOMBIE;

				VdkClose(disk_extension);
			}

			//
			// complete the request
			//
			if (NT_SUCCESS(irp->IoStatus.Status)) {
				IoCompleteRequest(irp, IO_DISK_INCREMENT);
			}
			else {
				IoCompleteRequest(irp, IO_NO_INCREMENT);
			}

		} // while

	} // for (;;)

}

//
// Open image file(s)
//
NTSTATUS
VdkOpen(
	IN PDEVICE_OBJECT		DiskObject,
	IN PVDK_OPEN_FILE_INFO 	OpenFile)
{
	PDISK_EXTENSION	disk_extension;
	ULONG			idx;
	NTSTATUS		status = STATUS_SUCCESS;

	VDKTRACE(VDKOPEN | VDKINFO,("[VDK] VdkOpen - IN\n"));

	//
	// set necessary pointers
	//
	disk_extension = (PDISK_EXTENSION)DiskObject->DeviceExtension;

	//
	// open files in the OPEN_FILE list
	//
	status = VdkOpenDisk(OpenFile, &disk_extension->DiskInfo);

	if (!NT_SUCCESS(status)) {
		goto exit_func;
	}

	//
	// Retrieve alignment requirement for each file and use the
	// largest value as this device's alignment requirement.
	//
	for (idx = 0; idx < disk_extension->DiskInfo.FilesTotal; idx++) {

		if (disk_extension->DiskInfo.Files[idx].FileHandle) {

			IO_STATUS_BLOCK	io_status;
			FILE_ALIGNMENT_INFORMATION	file_alignment;

			status = ZwQueryInformationFile(
				disk_extension->DiskInfo.Files[idx].FileHandle,
				&io_status,
				&file_alignment,
				sizeof(FILE_ALIGNMENT_INFORMATION),
				FileAlignmentInformation);

			if (!NT_SUCCESS(status)) {
				VDKTRACE(VDKOPEN,
					("[VDK] ZwQueryInformationFile - FILE_ALIGNMENT_INFORMATION %s\n",
					VdkStatusStr(status)));

				goto exit_func;
			}

			if (DiskObject->AlignmentRequirement <
				file_alignment.AlignmentRequirement) {

				DiskObject->AlignmentRequirement =
					file_alignment.AlignmentRequirement;
			}
		}
	}

	//
	// store disk disk capacity also in this device's partition info
	//
	disk_extension->PartitionInfo.PartitionLength.QuadPart =
		(LONGLONG)disk_extension->DiskInfo.Capacity << VDK_BYTE_SHIFT_TO_SECTOR;

	//
	// Adjust device characteristics
	//
	if (disk_extension->DiskInfo.DiskType == VDK_DISKTYPE_READONLY) {
		DiskObject->Characteristics |= FILE_READ_ONLY_DEVICE;
	}
	else {
		DiskObject->Characteristics &= ~FILE_READ_ONLY_DEVICE;
	}

exit_func:

	VDKTRACE(VDKOPEN | VDKINFO, ("[VDK] VdkOpen - OUT\n"));

	return status;
}

//
// Close current image file(s)
//
NTSTATUS
VdkClose(
	IN PDISK_EXTENSION		DiskExtension)
{
	PDEVICE_OBJECT	next_partition;
	NTSTATUS 		status = STATUS_SUCCESS;

	VDKTRACE(VDKCLOSE | VDKINFO, ("[VDK] VdkClose - IN\n"));

	//
	// Detach all partition devices from the disk device
	//
	RtlZeroMemory(
		&DiskExtension->PartitionInfo,
		sizeof(DiskExtension->PartitionInfo));

	next_partition = DiskExtension->NextPartition;
	DiskExtension->NextPartition = NULL;

	while (next_partition) {
		PDEVICE_OBJECT current_part = next_partition;
		PPART_EXTENSION part_extension = current_part->DeviceExtension;
		next_partition = part_extension->NextPartition;

		if (!current_part->Vpb || !current_part->Vpb->ReferenceCount) {
			VDKTRACE(VDKCLOSE | VDKINFO,
				("[VDK] Deleting %ws from device chain.\n",
				part_extension->DeviceName.Buffer));

			VdkDeleteDevice(current_part);
		}
		else {
			//
			//	the device cannot be deleted yet
			//
			VDKTRACE(VDKCLOSE | VDKWARN,
				("[VDK] %ws becomes a zombie.\n",
				part_extension->DeviceName.Buffer));

			RtlZeroMemory(
				&part_extension->PartitionInfo,
				sizeof(part_extension->PartitionInfo));

			part_extension->PartitionOrdinal	= VDK_ZOMBIE;
			part_extension->FirstPartition 		= NULL;
			part_extension->NextPartition 		= NULL;

			if (part_extension->SymbolicLink.Buffer) {

				VDKTRACE(VDKCLOSE | VDKINFO,
					("[VDK] Deleting a symbolic link %ws\n",
					part_extension->SymbolicLink.Buffer));

				IoDeleteSymbolicLink(&part_extension->SymbolicLink);
				ExFreePool(part_extension->SymbolicLink.Buffer);

				RtlZeroMemory(
					&part_extension->SymbolicLink,
					sizeof(part_extension->SymbolicLink));
			}
		}
	}

	//
	// Close all files and release resources
	//

	VdkCloseDisk(&DiskExtension->DiskInfo);

	VDKTRACE(VDKCLOSE | VDKINFO, ("[VDK] VdkClose - OUT\n"));

	return status;
}

//
// Default fill character for format operation
//
#define MEDIA_FORMAT_FILL_DATA	0xf6

//
// Format tracks
// Actually, just fills specified range of tracks with fill characters
//
NTSTATUS
VdkFormat(
	IN PVDK_DISK_INFO		DiskInfo,
	IN PFORMAT_PARAMETERS	FormatParam)
{
	PUCHAR				format_buffer;
	ULONG				start_offset;
	ULONG				end_offset;
	NTSTATUS			status;

	VDKTRACE(VDKFORMAT | VDKINFO,
		("[VDK] VdkFormat\n"));

	//
	// prepare format buffer
	//
	{
		ULONG buf_length =
			DiskInfo->Sectors << VDK_BYTE_SHIFT_TO_SECTOR;

		format_buffer = ExAllocatePool2(POOL_FLAG_PAGED | POOL_FLAG_UNINITIALIZED, buf_length, 'thre');

		if (format_buffer == NULL) {

			VDKTRACE(VDKFORMAT,
				("[VDK] cannot allocate format buffer\n"));

			return STATUS_INSUFFICIENT_RESOURCES;
		}

		RtlFillMemory(format_buffer, buf_length, MEDIA_FORMAT_FILL_DATA);
	}

	//
	// Prepare format parameters
	//

	start_offset =
		(FormatParam->StartCylinderNumber * DiskInfo->Tracks +
		FormatParam->StartHeadNumber) *
		DiskInfo->Sectors;

	end_offset =
		(FormatParam->EndCylinderNumber * DiskInfo->Tracks +
		FormatParam->EndHeadNumber) *
		DiskInfo->Sectors;

	VDKTRACE(VDKFORMAT | VDKINFO,
		("[VDK] Formatting sectors %lu - %lu\n",
		start_offset, end_offset));

	do {
		status = VdkWriteSector(
			DiskInfo,
			start_offset,
			DiskInfo->Sectors,
			format_buffer);

		if (!NT_SUCCESS(status)) {
			break;
		}

		start_offset += DiskInfo->Sectors;
	}
	while (start_offset <= end_offset);

	ExFreePool(format_buffer);

	return status;
}

// End Of File
