/*
	vdkdrv.c

	Virtual Disk kernel-mode driver for Windows NT platform
	Device IO control routine
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkver.h"
#include "vdkioctl.h"
#include "vdkfile.h"
#include "vdkaccess.h"

#include "imports.h"
#include "vdkdrv.h"

#if (DBG && VER_PRODUCTBUILD < 2195)
#define FILE_AUTOGENERATED_DEVICE_NAME	0x00000080
#define FILE_DEVICE_SECURE_OPEN 		0x00000100
#define VPB_REMOVE_PENDING				0x00000008
#define VPB_RAW_MOUNT					0x00000010
#endif

#define IO_INPUTLEN(p)	(p)->Parameters.DeviceIoControl.InputBufferLength
#define IO_OUTPUTLEN(p)	(p)->Parameters.DeviceIoControl.OutputBufferLength

//
// Handle various IOCTL commands
//
NTSTATUS
VdkDeviceControl (
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP 			Irp)
{
	PPART_EXTENSION		part_extension;
	PDISK_EXTENSION		disk_extension;
	PIO_STACK_LOCATION	io_stack;
	NTSTATUS			status;

	//
	// Set up necessary object and extension pointers.
	//
	part_extension =
		(PPART_EXTENSION)DeviceObject->DeviceExtension;

	disk_extension =
		(PDISK_EXTENSION)part_extension->FirstPartition;

	io_stack = IoGetCurrentIrpStackLocation(Irp);

	VDKTRACE(VDKIOCTL | VDKINFO,
		("[VDK] %ws %s\n",
		part_extension->DeviceName.Buffer,
		IoControlCodeToStr(io_stack->Parameters.DeviceIoControl.IoControlCode)));

	Irp->IoStatus.Information = 0;

	switch (io_stack->Parameters.DeviceIoControl.IoControlCode) {
	//
	// Operation:	Returns VDK driver version
	// Target:		any device
	// Input:		none
	// Output:		ULONG driver version
	// Status:		STATUS_SUCCESS
	//
	case IOCTL_VDK_GET_VERSION:
		{
			if (IO_OUTPUTLEN(io_stack) >= sizeof(ULONG)) {
				*(PULONG)Irp->AssociatedIrp.SystemBuffer =
#if DBG
					VDK_DRIVER_VERSION_VAL | 0x00008000;
#else	// DBG
					VDK_DRIVER_VERSION_VAL;
#endif	// DBG
				Irp->IoStatus.Information = sizeof(ULONG);
			}
			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	Open image file(s).
	// Target:		disk device only
	// Input:		VDK_OPEN_FILE_INFO open file information
	// Output:		none
	// Status:		STATUS_INVALID_DEVICE_REQUEST	not a disk device
	//				STATUS_DEVICE_BUSY				image is already opened
	//				STATUS_INVALID_PARAMETER		invalid input parameter
	//				STATUS_INSUFFICIENT_RESOURSES	resource allocation error
	//				...or other status returned by system function call
	//
	case IOCTL_VDK_OPEN_FILE:
		{
#ifdef VDK_SUPPORT_NETWORK
			SECURITY_QUALITY_OF_SERVICE sqos;
#endif	// VDK_SUPPORT_NETWORK

			//
			//	Check device type
			//
			if ((PPART_EXTENSION)disk_extension != part_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is not a disk device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			// Check device status
			//
			if (disk_extension->DiskInfo.DiskType) {
				status = STATUS_DEVICE_BUSY;
				break;
			}

			if (disk_extension->PartitionOrdinal == VDK_ZOMBIE) {
				//
				//	The device can be a zombie
				//
				if (DeviceObject->Vpb && DeviceObject->Vpb->ReferenceCount > 1) {
					VDKTRACE(VDKIOCTL|VDKWARN,
						("[VDK] %ws is a zombie\n",
						disk_extension->DeviceName.Buffer));

					status = STATUS_DEVICE_BUSY;
					break;
				}

				//	This device is no longer a zombie
				VDKTRACE(VDKIOCTL|VDKWARN,
					("[VDK] %ws is no longer a zombie\n",
					disk_extension->DeviceName.Buffer));

				disk_extension->PartitionOrdinal = 0;
			}

			//
			// Check input parameter
			//
			status = VdkOpenCheckParam(
				(PVDK_OPEN_FILE_INFO)Irp->AssociatedIrp.SystemBuffer,
				IO_INPUTLEN(io_stack));

			if (!NT_SUCCESS(status)) {
				break;
			}

#ifdef VDK_SUPPORT_NETWORK
			//
			// create security context that matches the calling process' context
			//
			if (disk_extension->SecurityContext) {
				SeDeleteClientSecurity(disk_extension->SecurityContext);
			}
			else {
				disk_extension->SecurityContext = ExAllocatePool2(
					POOL_FLAG_NON_PAGED, sizeof(SECURITY_CLIENT_CONTEXT), 'ioct');

				if (!disk_extension->SecurityContext) {

					VDKTRACE(VDKIOCTL,
						("[VDK] IOCTL_VDK_OPEN_FILE: ExAllocatePool\n"));

					status = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}
			}

			RtlZeroMemory(&sqos, sizeof(sqos));

			sqos.Length = sizeof(sqos);
			sqos.ImpersonationLevel = SecurityImpersonation;
			sqos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
			sqos.EffectiveOnly = FALSE;

			SeCreateClientSecurity(
				PsGetCurrentThread(),
				&sqos,
				FALSE,
				disk_extension->SecurityContext);
#endif	// VDK_SUPPORT_NETWORK

			//
			// The device thread will do the rest of the job
			//
			status = STATUS_PENDING;
			break;
		}

	//
	// Operation:	configure devices to match the virtual disk's
	//				partition layout
	// Target: 		disk device only
	// Input:		none
	// Output:		ULONG size of partition layout information which would be
	//				returned by IOCTL_DISK_GET_DRIVE_LAYOUT
	// Status:		STATUS_INVALID_DEVICE_REQUEST	not a disk device
	//				...or other status returned by system function call
	//
	case IOCTL_VDK_UPDATE_DEVICE:
		{
			PDRIVE_LAYOUT_INFORMATION drive_layout;
			ULONG idx;

			//
			//	Check device type
			//
			if ((PPART_EXTENSION)disk_extension != part_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is not a disk device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			status = STATUS_SUCCESS;

			if (disk_extension->PartitionOrdinal == VDK_ZOMBIE) {
				if (!DeviceObject->Vpb || DeviceObject->Vpb->ReferenceCount == 1) {
					//	This device is no longer a zombie
					VDKTRACE(VDKIOCTL|VDKWARN,
						("[VDK] %ws is no longer a zombie\n",
						disk_extension->DeviceName.Buffer));

					disk_extension->PartitionOrdinal = 0;
				}
				break;
			}

			if (!disk_extension->DiskInfo.DiskType) {
				break;
			}

			//
			//	Read partition information from virtual disk
			//
			status = IoReadPartitionTable(
				disk_extension->DeviceObject,
				VDK_BYTES_PER_SECTOR,
				FALSE,
				&drive_layout);

			if (!NT_SUCCESS(status)) {

				VDKTRACE(VDKIOCTL,
					("[VDK] IOCTL_VDK_UPDATE_DEVICE: IoReadPartitionTable %s\n",
					VdkStatusStr(status)));

				break;
			}

			if (!drive_layout) {
				status = STATUS_DRIVER_INTERNAL_ERROR;

				VDKTRACE(VDKIOCTL,
					("[VDK] IOCTL_VDK_UPDATE_DEVICE: IoReadPartitionTable %s\n",
					VdkStatusStr(status)));

				break;
			}

			//
			// Create / Delete / Update device objects according to partition table
			//
			for (idx = 0; idx < drive_layout->PartitionCount; idx++) {
				drive_layout->PartitionEntry[idx].RewritePartition = TRUE;
			}

			VdkUpdateDevice(disk_extension->DeviceObject, drive_layout);

			ExFreePool(drive_layout);

			break;
		}

	//
	// Operation:	Close image file(s)
	// Target:		disk device only
	// Input:		!= 0	the volume has been dismounted
	//				0		the volume has not been dismounted
	// Output:		none
	// Status:		STATUS_INVALID_DEVICE_REQUEST	not a disk device
	//				STATUS_DEVICE_NOT_READY			image is not opened
	//				...or other status returned by system function call
	//
	case IOCTL_VDK_CLOSE_FILE:
		{
			//
			// check device type
			//
			if ((PPART_EXTENSION)disk_extension != part_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is not a disk device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			//	Check device status
			//
			if (!disk_extension->DiskInfo.DiskType) {
				status = STATUS_DEVICE_NOT_READY;
				break;
			}

			//
			// Change DiskType so that no one will use the file hereafter
			//
			disk_extension->DiskInfo.DiskType = VDK_DISKTYPE_NONE;

			//
			// The device thread will do the rest of the job
			//
			status = STATUS_PENDING;
			break;
		}

	//
	// Operation:	Returns total size of image file information
	// Target: 		disk device only
	// Input:		none
	// Output:		ULONG required buffer size for IOCTL_VDK_QUERY_FILE
	// Status:		STATUS_INVALID_DEVICE_REQUEST	not a disk device
	//				STATUS_BUFFER_TOO_SMALL			output buffer too small
	//
	case IOCTL_VDK_QUERY_FILE_SIZE:
		{
			//
			//	Check device type
			//
			if ((PPART_EXTENSION)disk_extension != part_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is not a disk device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			// Check output buffer length
			//
			if (IO_OUTPUTLEN(io_stack) < sizeof(ULONG)) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			// Store required buffer size
			//
			if (disk_extension->DiskInfo.DiskType) {

				*(PULONG)Irp->AssociatedIrp.SystemBuffer =
					FIELD_OFFSET(VDK_OPEN_FILE_INFO, Files) +
					(sizeof(VDK_OPEN_FILE_ITEM) * disk_extension->DiskInfo.FilesTotal) +
					disk_extension->DiskInfo.BufferLen;
			}
			else {
				*(PULONG)Irp->AssociatedIrp.SystemBuffer = 0;
			}

			Irp->IoStatus.Information = sizeof(ULONG);
			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	Returns current image file information
	// Target: 		disk device only
	// Input:		none
	// Output:		VDK_OPEN_FILE_INFO	image file information
	// Status:		STATUS_INVALID_DEVICE_REQUEST	not a disk device
	//				STATUS_BUFFER_TOO_SMALL			output buffer too small
	//
	case IOCTL_VDK_QUERY_FILE:
		{
			PVDK_OPEN_FILE_INFO open_file;
			ULONG	fixed_size;
			ULONG	idx;

			//
			//	Check device type
			//
			if ((PPART_EXTENSION)disk_extension != part_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is not a disk device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			// image is opened ?
			//
			if (!disk_extension->DiskInfo.DiskType) {
				// nothing to return
				status = STATUS_SUCCESS;
				break;
			}

			//
			// calculate necessaty buffer length
			//
			fixed_size =
				FIELD_OFFSET(VDK_OPEN_FILE_INFO, Files) +
				(sizeof(VDK_OPEN_FILE_ITEM) * disk_extension->DiskInfo.FilesTotal);

			//
			// Check output buffer length
			//
			if (IO_OUTPUTLEN(io_stack) <
				fixed_size + disk_extension->DiskInfo.BufferLen) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			// Store disk information
			//
			open_file = (PVDK_OPEN_FILE_INFO)Irp->AssociatedIrp.SystemBuffer;

			open_file->DiskType		= disk_extension->DiskInfo.DiskType;
			open_file->Capacity		= disk_extension->DiskInfo.Capacity;
			open_file->Cylinders	= disk_extension->DiskInfo.Cylinders;
			open_file->Tracks		= disk_extension->DiskInfo.Tracks;
			open_file->Sectors		= disk_extension->DiskInfo.Sectors;
			open_file->FilesTotal	= disk_extension->DiskInfo.FilesTotal;

			//
			// Store each file information
			//
			for (idx = 0; idx < disk_extension->DiskInfo.FilesTotal; idx++) {
				PVDK_FILE_INFO	file_item;
				PVDK_OPEN_FILE_ITEM	open_item;

				file_item = &(disk_extension->DiskInfo.Files[idx]);
				open_item = &(open_file->Files[idx]);

				open_item->FileType		= file_item->FileType;
				open_item->Capacity		= file_item->Capacity;
				open_item->NameLength	= file_item->NameLength;
			}

			//
			// copy filenames following the fixed info
			//
			if (disk_extension->DiskInfo.BufferLen) {
				RtlCopyMemory(
					(PCHAR)open_file + fixed_size,
					disk_extension->DiskInfo.NameBuffer,
					disk_extension->DiskInfo.BufferLen);
			}

			Irp->IoStatus.Information =
				fixed_size + disk_extension->DiskInfo.BufferLen;

			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	Returns number of partition devices attached to
	//				the current disk device
	// Target: 		disk device only
	// Input:		none
	// Output:		ULONG	number of partition devices
	// Status:		STATUS_INVALID_DEVICE_REQUEST	not a disk device
	//				STATUS_BUFFER_TOO_SMALL			output buffer too small
	//
	case IOCTL_VDK_NUMBER_OF_PARTS:
		{
			PDEVICE_OBJECT device_obj;
			ULONG num_parts;

			//
			//	Check device type
			//
			if ((PPART_EXTENSION)disk_extension != part_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is not a disk device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			// Check output buffer length
			//
			if (IO_OUTPUTLEN(io_stack) < sizeof(ULONG)) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			//	Count partition devices
			//
			num_parts = 0;
			device_obj = disk_extension->NextPartition;

			while (device_obj) {
				num_parts++;
				device_obj = ((PPART_EXTENSION)device_obj->DeviceExtension)->NextPartition;
			}

			*(PULONG)Irp->AssociatedIrp.SystemBuffer = num_parts;
			Irp->IoStatus.Information = sizeof(ULONG);

			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	Create new virtual disk device
	// Target:		any device
	// Input:		none
	// Output:		none
	//
	case IOCTL_VDK_CREATE_DISK:
		//
		//	Actual operation is performed in the device thread, for
		//	directory object must be created in the system thread context
		//
		status = STATUS_PENDING;
		break;

	//
	// Operation:	Delete the last created virtual disk device
	// Target:		any device
	// Input:		none
	// Output:		none
	//
	case IOCTL_VDK_DELETE_DISK:
		status = STATUS_PENDING;
		break;

	//
	// Operation:	Notifies the device that it has been dismounted
	// Target:		any device
	// Input:		none
	// Output:		none
	//
	case IOCTL_VDK_NOTIFY_DISMOUNT:
		if (part_extension->PartitionOrdinal == VDK_ZOMBIE) {

			VDKTRACE(VDKIOCTL | VDKWARN,
				("[VDK] %ws is no longer a zombie.\n",
				part_extension->DeviceName.Buffer));

			part_extension->PartitionOrdinal = 0;
		}
		status = STATUS_SUCCESS;
		break;

	//
	// Operation:	Returnes driver information
	// Target:		any device
	// Input:		none
	// Output:		VDK_DRIVER_INFO driver information
	// Status:		STATUS_BUFFER_TOO_SMALL		output buffer too small
	//
	case IOCTL_VDK_DRIVER_INFO:
		{
			PDEVICE_OBJECT device_obj;
			PVDK_DRIVER_INFO driver_info;

			//
			// Check output buffer length
			//
			if (IO_OUTPUTLEN(io_stack) < sizeof(VDK_DRIVER_INFO)) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			//	Initialize output buffer
			//
			driver_info =
				(PVDK_DRIVER_INFO)Irp->AssociatedIrp.SystemBuffer;

			RtlZeroMemory(driver_info, sizeof(VDK_DRIVER_INFO));

			//
			//	Count each type of devices
			//
			device_obj = DeviceObject->DriverObject->DeviceObject;

			while (device_obj) {
				PPART_EXTENSION part_ext =
					(PPART_EXTENSION)device_obj->DeviceExtension;

				if (device_obj->Vpb && device_obj->Vpb->ReferenceCount) {
					driver_info->TotalReference
						+= device_obj->Vpb->ReferenceCount;
				}

				if (part_ext) {
					if (part_ext == (PPART_EXTENSION)part_ext->FirstPartition) {
						driver_info->DiskDevices++;
					}
					else if (part_ext->FirstPartition) {
						driver_info->AttachedParts++;
					}
					else {
						driver_info->OrphanedParts++;
					}

#if DBG
					if ((TraceFlags & (VDKIOCTL | VDKINFO)) == (VDKIOCTL | VDKINFO)) {
						if (part_ext->DeviceName.Buffer) {
							KdPrint(("[VDK] DeviceName: %ws\n",
								part_ext->DeviceName.Buffer));
						}
						if (part_ext->SymbolicLink.Buffer) {
							KdPrint(("[VDK] SymbolicLink: %ws\n",
								part_ext->SymbolicLink.Buffer));
						}

						KdPrint(("[VDK] Characteristics = 0x%08x\n",
							device_obj->Characteristics));

						if (device_obj->Characteristics & FILE_AUTOGENERATED_DEVICE_NAME) {
							KdPrint(("    FILE_AUTOGENERATED_DEVICE_NAME\n"));
						}
						if (device_obj->Characteristics & FILE_DEVICE_IS_MOUNTED) {
							KdPrint(("    FILE_DEVICE_IS_MOUNTED\n"));
						}
						if (device_obj->Characteristics & FILE_DEVICE_SECURE_OPEN) {
							KdPrint(("    FILE_DEVICE_SECURE_OPEN\n"));
						}
						if (device_obj->Characteristics & FILE_FLOPPY_DISKETTE) {
							KdPrint(("    FILE_FLOPPY_DISKETTE\n"));
						}
						if (device_obj->Characteristics & FILE_READ_ONLY_DEVICE) {
							KdPrint(("    FILE_READ_ONLY_DEVICE\n"));
						}
						if (device_obj->Characteristics & FILE_REMOTE_DEVICE) {
							KdPrint(("    FILE_REMOTE_DEVICE\n"));
						}
						if (device_obj->Characteristics & FILE_REMOVABLE_MEDIA) {
							KdPrint(("    FILE_REMOVABLE_MEDIA\n"));
						}
						if (device_obj->Characteristics & FILE_VIRTUAL_VOLUME) {
							KdPrint(("    FILE_VIRTUAL_VOLUME\n"));
						}
						if (device_obj->Characteristics & FILE_WRITE_ONCE_MEDIA) {
							KdPrint(("    FILE_WRITE_ONCE_MEDIA\n"));
						}

						if (device_obj->Vpb) {
							KdPrint(("    Vpb->Type = %d\n",
								device_obj->Vpb->Type));

							KdPrint(("    Vpb->SerialNumber = %lu\n",
								device_obj->Vpb->SerialNumber));

							KdPrint(("    Vpb->ReferenceCount = %lu\n",
								device_obj->Vpb->ReferenceCount));

							if (device_obj->Vpb->Flags & VPB_MOUNTED) {
								KdPrint(("    VPB_MOUNTED\n"));
							}
							if (device_obj->Vpb->Flags & VPB_LOCKED) {
								KdPrint(("    VPB_LOCKED\n"));
							}
							if (device_obj->Vpb->Flags & VPB_PERSISTENT) {
								KdPrint(("    VPB_PERSISTENT\n"));
							}
							if (device_obj->Vpb->Flags & VPB_REMOVE_PENDING) {
								KdPrint(("    VPB_REMOVE_PENDING\n"));
							}
							if (device_obj->Vpb->Flags & VPB_RAW_MOUNT) {
								KdPrint(("    VPB_RAW_MOUNT\n"));
							}
						}
					}
#endif	// DBG
				}

				device_obj = device_obj->NextDevice;
			}

			Irp->IoStatus.Information = sizeof(VDK_DRIVER_INFO);
			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	Returns device information
	// Target:		any device
	// Input:		TRUE	query all devices
	//				FALSE	query this device only
	// Output:		VDK_DEVICE_INFO device information
	// Status:		STATUS_BUFFER_TOO_SMALL	output buffer too small
	//				STATUS_BUFFER_OVERFLOW
	//
	case IOCTL_VDK_DEVICE_INFO:
		{
			PDEVICE_OBJECT device_obj;
			PVDK_DEVICE_INFO device_info;
			ANSI_STRING ansi_str;
			ULONG device_num;

			//
			// Check output buffer length
			//
			if (IO_OUTPUTLEN(io_stack) < sizeof(VDK_DEVICE_INFO)) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			// Check request type
			//
			if (IO_INPUTLEN(io_stack) >= sizeof(ULONG) &&
				*(PULONG)Irp->AssociatedIrp.SystemBuffer) {

				//
				// Information request for all devices
				//
				device_obj = DeviceObject->DriverObject->DeviceObject;
				device_num = 0;

				while (device_obj) {
					device_num++;
					device_obj = device_obj->NextDevice;
				}

				//
				// Recheck output buffer length
				//
				if (IO_OUTPUTLEN(io_stack) < sizeof(VDK_DEVICE_INFO) * device_num) {

					device_num = IO_OUTPUTLEN(io_stack) / sizeof(VDK_DEVICE_INFO);
					status = STATUS_BUFFER_OVERFLOW;
				}
				else {
					status = STATUS_SUCCESS;
				}

				device_obj = DeviceObject->DriverObject->DeviceObject;
			}
			else {

				//
				// Information request for this device only
				//
				device_obj = DeviceObject;
				device_num = 1;
				status = STATUS_SUCCESS;
			}

			//
			// Initialize output buffer
			//
			Irp->IoStatus.Information =
				sizeof(VDK_DEVICE_INFO) * device_num;

			RtlZeroMemory(
				Irp->AssociatedIrp.SystemBuffer,
				sizeof(VDK_DEVICE_INFO) * device_num);

			device_info =
				(PVDK_DEVICE_INFO)Irp->AssociatedIrp.SystemBuffer;

			while (device_obj && device_num) {
				PPART_EXTENSION part_ext =
					(PPART_EXTENSION)device_obj->DeviceExtension;

				//
				// Store devie type
				//
				if (part_ext == (PPART_EXTENSION)part_ext->FirstPartition) {
					device_info->DeviceType = VDK_DEVICE_DISK;
				}
				else {
					device_info->DeviceType = VDK_DEVICE_PART;
				}

				if (part_ext->PartitionOrdinal == VDK_ZOMBIE) {
					device_info->Zombie = VDK_ZOMBIE;
				}

				//
				// Store device name
				//
				if (part_ext->DeviceName.Buffer) {
					ansi_str.Buffer = device_info->DeviceName;
					ansi_str.Length = 0;
					ansi_str.MaximumLength = sizeof(device_info->DeviceName);

					RtlUnicodeStringToAnsiString(
						&ansi_str, &part_ext->DeviceName, FALSE);
				}

				//
				// Store symbolic link name
				//
				if (part_ext->SymbolicLink.Buffer) {
					ansi_str.Buffer = device_info->SymbolicLink;
					ansi_str.Length = 0;
					ansi_str.MaximumLength = sizeof(device_info->SymbolicLink);

					RtlUnicodeStringToAnsiString(
						&ansi_str, &part_ext->SymbolicLink, FALSE);
				}

				//
				// store device reference count
				//
				if (device_obj->Vpb) {
					device_info->ReferenceCount =
						device_obj->Vpb->ReferenceCount;
				}

				//
				// next device;
				//
				device_obj = device_obj->NextDevice;
				device_num--;
				device_info++;
			}

			break;
		}

#if DBG
	//
	// Operation:	Set debug trace flag
	// Target:		any device
	// Input:		ULONG trace flags (optional)
	// Output:		none
	// Status:		STATUS_SUCCESS
	//
	case IOCTL_VDK_DEBUG_TRACE:
		{
			if (IO_INPUTLEN(io_stack) >= sizeof(ULONG)) {
				TraceFlags = *(PULONG)Irp->AssociatedIrp.SystemBuffer;
			}

			KdPrint(("[VDK] Current Trace Flags = 0x%08x\n", TraceFlags));

			if (IO_OUTPUTLEN(io_stack) >= sizeof(ULONG)) {
				*(PULONG)Irp->AssociatedIrp.SystemBuffer = TraceFlags;
				Irp->IoStatus.Information = sizeof(ULONG);
			}

			status = STATUS_SUCCESS;
			break;
		}
#endif	// DBG

	//
	// Operation:	Return the drive geometry for the specified drive.
	// Target:		active device
	// Input:		none
	// Output:		DISK_GEOMETRY
	//
	// we will return the geometry for the physical drive, regardless
	// of which partition was specified for the request.
	//
	case IOCTL_DISK_GET_DRIVE_GEOMETRY:
		{
			PDISK_GEOMETRY geometry;

			//
			//	check device type
			//
			if (!disk_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is an orphaned partition device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			// check device state
			//
			if (!disk_extension->DiskInfo.DiskType ||
				!part_extension->PartitionInfo.PartitionLength.QuadPart) {

				status = STATUS_DEVICE_NOT_READY;
				break;
			}

			//
			// check output buffer length
			//
			if (IO_OUTPUTLEN(io_stack) < sizeof(DISK_GEOMETRY)) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			// copy DISK_GEOMETRY information
			//
			geometry = (PDISK_GEOMETRY)Irp->AssociatedIrp.SystemBuffer;

			geometry->MediaType			= FixedMedia;
			geometry->BytesPerSector	= VDK_BYTES_PER_SECTOR;
			geometry->SectorsPerTrack	= disk_extension->DiskInfo.Sectors;
			geometry->TracksPerCylinder	= disk_extension->DiskInfo.Tracks;
			geometry->Cylinders.QuadPart = disk_extension->DiskInfo.Cylinders;

			Irp->IoStatus.Information = sizeof(DISK_GEOMETRY);
			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	Return the information about the partition specified by the
	// 				device object.
	// Target: 		active device
	// Input:		none
	// Output:		PARTITION_INFORMATION
	//
	case IOCTL_DISK_GET_PARTITION_INFO:
		{
			PPARTITION_INFORMATION partition_info;

			//
			//	check device type
			//
			if (!disk_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is an orphaned partition device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			// check device state
			//
			if (!disk_extension->DiskInfo.DiskType ||
				!part_extension->PartitionInfo.PartitionLength.QuadPart) {

				status = STATUS_DEVICE_NOT_READY;
				break;
			}

			//
			// check output buffer length
			//
			if (IO_OUTPUTLEN(io_stack) < sizeof(PARTITION_INFORMATION)) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			// set up the PARTITION_INFORMATION to return
			//
			partition_info =
				(PPARTITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

			RtlCopyMemory(partition_info,
				&part_extension->PartitionInfo,
				sizeof(PARTITION_INFORMATION));

			partition_info->RecognizedPartition = TRUE;
			partition_info->RewritePartition	= FALSE;

			Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION);
			status = STATUS_SUCCESS;
			break;
		}


	//
	// Operation:	Return the information about the partition specified by the
	// 				device object. (WINXP specific)
	// Target: 		active device
	// Input:		none
	// Output:		PARTITION_INFORMATION_EX
	//
	case IOCTL_DISK_GET_PARTITION_INFO_EX:	// WINXP
		{
			PPARTITION_INFORMATION_EX partition_info;

			//
			//	check device type
			//
			if (!disk_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is an orphaned partition device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			// check device state
			//
			if (!disk_extension->DiskInfo.DiskType ||
				!part_extension->PartitionInfo.PartitionLength.QuadPart) {

				status = STATUS_DEVICE_NOT_READY;
				break;
			}

			//
			// check output buffer length
			//
			if (IO_OUTPUTLEN(io_stack) < sizeof(PARTITION_INFORMATION_EX)) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			// set up the PARTITION_INFORMATION_EX to return
			//
			partition_info =
				(PPARTITION_INFORMATION_EX)Irp->AssociatedIrp.SystemBuffer;

			partition_info->PartitionStyle = PARTITION_STYLE_MBR;

			partition_info->StartingOffset.QuadPart =
				part_extension->PartitionInfo.StartingOffset.QuadPart;

			partition_info->PartitionLength.QuadPart =
				part_extension->PartitionInfo.PartitionLength.QuadPart;

			partition_info->PartitionNumber =
				part_extension->PartitionInfo.PartitionNumber;

			partition_info->RewritePartition	= FALSE;

			partition_info->Mbr.PartitionType =
				part_extension->PartitionInfo.PartitionType;

			partition_info->Mbr.BootIndicator =
				part_extension->PartitionInfo.BootIndicator;

			partition_info->Mbr.RecognizedPartition = TRUE;

			partition_info->Mbr.HiddenSectors =
				part_extension->PartitionInfo.HiddenSectors;

			Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION_EX);
			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	set the partition type.
	// Target: 		active partition device
	// Input:		SET_PARTITION_INFORMATION
	// Output:		none
	//
	case IOCTL_DISK_SET_PARTITION_INFO:
		{
			PSET_PARTITION_INFORMATION set_info;

			//
			// check device type
			//
			if (!disk_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is an orphaned partition device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			//	check device state
			//
			if (!disk_extension->DiskInfo.DiskType ||
				!part_extension->PartitionInfo.PartitionLength.QuadPart) {

				status = STATUS_DEVICE_NOT_READY;
				break;
			}

			//
			//	check device type
			//
			if (part_extension == (PPART_EXTENSION)disk_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is a disk device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			//	check input buffer length
			//
			if (IO_INPUTLEN(io_stack) < sizeof(SET_PARTITION_INFORMATION)) {

				status = STATUS_INVALID_PARAMETER;
				break;
			}

			//
			//	write partition information to virtual disk
			//
			set_info = (PSET_PARTITION_INFORMATION)
				Irp->AssociatedIrp.SystemBuffer;

			status = IoSetPartitionInformation(
				disk_extension->DeviceObject,
				VDK_BYTES_PER_SECTOR,
				part_extension->PartitionOrdinal,
				set_info->PartitionType);

			if (!NT_SUCCESS(status)) {
				VDKTRACE(VDKIOCTL,
					("[VDK] IOCTL_DISK_SET_PARTITION_INFO: IoSetPartitionInformation %s\n",
					VdkStatusStr(status)));

				break;
			}

			//
			// Store the partition type in the partition extension.
			//

			part_extension->PartitionInfo.PartitionType =
				set_info->PartitionType;

			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	Return the partition layout for the drive.
	// Target: 		active device
	// Input:		none
	// Output:		DRIVE_LAYOUT_INFORMATION
	// The layout is returned for the disk device, regardless
	// of which partition was specified for the request.
	//
	case IOCTL_DISK_GET_DRIVE_LAYOUT:
		{
			PDRIVE_LAYOUT_INFORMATION layout;
			ULONG data_size;

			//
			//	check device type
			//
			if (!disk_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is an orphaned partition device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			// check device state
			//
			if (!disk_extension->DiskInfo.DiskType ||
				!part_extension->PartitionInfo.PartitionLength.QuadPart) {

				status = STATUS_DEVICE_NOT_READY;
				break;
			}

			//
			// check output buffer length
			//
			if (IO_OUTPUTLEN(io_stack) < sizeof(DRIVE_LAYOUT_INFORMATION)) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			// Read drive layout data from virtual disk
			//
			status = IoReadPartitionTable(
				disk_extension->DeviceObject,
				VDK_BYTES_PER_SECTOR,
				FALSE,
				&layout);

			if (!NT_SUCCESS(status)) {

				VDKTRACE(VDKIOCTL,
					("[VDK] IOCTL_DISK_GET_DRIVE_LAYOUT: IoReadPartitionTable %s\n",
					VdkStatusStr(status)));

				break;
			}

			//
			// Make sure the layout information and device state match
			//
			VdkUpdateDevice(
				disk_extension->DeviceObject, layout);

			//
			// Determine its size and, if the data will fit
			// into the intermediary buffer, return it.
			//
			data_size =
				FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION, PartitionEntry) +
				(sizeof(PARTITION_INFORMATION) * layout->PartitionCount);

			if (IO_OUTPUTLEN(io_stack) < data_size) {
				data_size = IO_OUTPUTLEN(io_stack);
				status = STATUS_BUFFER_OVERFLOW;
			}
			else {
				status = STATUS_SUCCESS;
			}

			RtlMoveMemory(
				Irp->AssociatedIrp.SystemBuffer, layout, data_size);

			Irp->IoStatus.Information = data_size;

			ExFreePool(layout);
			break;
		}

	//
	// Operation:	Update the disk with new partition information.
	// Target:		disk device only
	// Input:		DRIVE_LAYOUT_INFORMATION
	// Output:		DRIVE_LAYOUT_INFORMATION
	//
	case IOCTL_DISK_SET_DRIVE_LAYOUT:
		{
			PDRIVE_LAYOUT_INFORMATION layout;

			//
			//	check device type
			//
			if ((PPART_EXTENSION)disk_extension != part_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is not a disk device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			//	check device state
			//
			if (!disk_extension->DiskInfo.DiskType ||
				!part_extension->PartitionInfo.PartitionLength.QuadPart) {

				status = STATUS_DEVICE_NOT_READY;
				break;
			}

			//
			// check input buffer length.
			//
			if (IO_INPUTLEN(io_stack) < sizeof(DRIVE_LAYOUT_INFORMATION)) {

				status = STATUS_INFO_LENGTH_MISMATCH;
				break;
			}

			layout = (PDRIVE_LAYOUT_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

			if (IO_INPUTLEN(io_stack) <
				FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION, PartitionEntry) +
				(sizeof(PARTITION_INFORMATION) * layout->PartitionCount)) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			// configure devices to match the layout information
			//
			VdkUpdateDevice(
				disk_extension->DeviceObject, layout);

			//
			// Write partition table to virtual disk
			//
			status = IoWritePartitionTable(
				disk_extension->DeviceObject,
				VDK_BYTES_PER_SECTOR,
				disk_extension->DiskInfo.Sectors,
				disk_extension->DiskInfo.Tracks,
				layout);

			if (!NT_SUCCESS(status)) {
				VDKTRACE(VDKIOCTL,
					("[VDK] IOCTL_DISK_SET_DRIVE_LAYOUT: IoWritePartitionTable %s\n",
					VdkStatusStr(status)));

				break;
			}

			Irp->IoStatus.Information = IO_OUTPUTLEN(io_stack);
			break;
		}

	//
	// Operation:	If the caller is kernel mode, set the verify bit.
	// Target:		any device
	// Input:		none
	// Output:		none
	//
	case IOCTL_DISK_INTERNAL_SET_VERIFY:
		{
			if (Irp->RequestorMode == KernelMode) {
				DeviceObject->Flags |= DO_VERIFY_VOLUME;
			}

			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	If the caller is kernel mode, clear the verify bit.
	// Target:		any device
	// Input:		none
	// Output:		none
	//
	case IOCTL_DISK_INTERNAL_CLEAR_VERIFY:
		{
			if (Irp->RequestorMode == KernelMode) {
				DeviceObject->Flags &= ~DO_VERIFY_VOLUME;
			}

			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	Returns partition length in bytes
	// Target: 		active device
	// Input:		none
	// Output:		GET_LENGTH_INFORMATION
	//
	case IOCTL_DISK_GET_LENGTH_INFO:	// WINXP
		{
			PGET_LENGTH_INFORMATION	len_info;

			//
			//	check device type
			//
			if (!disk_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is an orphaned partition device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			//	check device state
			//
			if (!disk_extension->DiskInfo.DiskType ||
				!part_extension->PartitionInfo.PartitionLength.QuadPart) {

				status = STATUS_DEVICE_NOT_READY;
				break;
			}

			//
			// Check output buffer size
			//
			if (IO_OUTPUTLEN(io_stack) < sizeof(GET_LENGTH_INFORMATION)) {

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//
			// store output data
			//
			len_info = (PGET_LENGTH_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

			len_info->Length.QuadPart
				= part_extension->PartitionInfo.PartitionLength.QuadPart;

			Irp->IoStatus.Information = sizeof(GET_LENGTH_INFORMATION);
			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	Checks if disk is writable
	// Target:		active device
	// Input:		none
	// Output:		none
	// Status:		STATUS_SUCCESS					disk is writable / write-blocked
	//				STATUS_WRITE_PROTECTED			disk is read-only
	//				STATUS_INVALID_DEVICE_REQUEST	device is an orphaned partition
	//				STATUS_DEVICE_BUSY				device is a zombie
	//
	case IOCTL_DISK_IS_WRITABLE:
		{
			if (part_extension->PartitionOrdinal == VDK_ZOMBIE) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is a zombie device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_DEVICE_BUSY;
				break;
			}

			//
			//	check device type
			//
			if (!disk_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is an orphaned partition device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			if (!disk_extension->DiskInfo.DiskType ||
				!part_extension->PartitionInfo.PartitionLength.QuadPart) {
				status = STATUS_DEVICE_NOT_READY;
			}
			else if (disk_extension->DiskInfo.DiskType == VDK_DISKTYPE_READONLY) {
				status = STATUS_MEDIA_WRITE_PROTECTED;
			}
			else {
				status = STATUS_SUCCESS;
			}

			break;
		}

	//
	// Operation:	Checks if removable media has been changed
	// Target:		any device
	// Input:		none
	// Output:		none
	//
	case IOCTL_DISK_CHECK_VERIFY:		// WIN2K requires this
	case IOCTL_STORAGE_CHECK_VERIFY:
	case IOCTL_STORAGE_CHECK_VERIFY2:
		{
			if (IO_OUTPUTLEN(io_stack) >= sizeof(ULONG)) {
				*(PULONG)Irp->AssociatedIrp.SystemBuffer = 0;
				Irp->IoStatus.Information = sizeof(ULONG);
			}

			status = STATUS_SUCCESS;
			break;
		}

	//
	// Operation:	Verify specified extent of a disk
	// Target: 		active device
	// Input:		VERIFY_INFORMATION
	// Output:		none
	// NT uses this command for formatting
	//
	case IOCTL_DISK_VERIFY:
		{
			PVERIFY_INFORMATION param;

			//
			//	check device type
			//
			if (!disk_extension) {
				VDKTRACE(VDKIOCTL,
					("[VDK] %ws is an orphaned partition device\n",
					part_extension->DeviceName.Buffer));

				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			//
			// Check media status
			//
			if (!disk_extension->DiskInfo.DiskType ||
				!part_extension->PartitionInfo.PartitionLength.QuadPart) {

				status = STATUS_DEVICE_NOT_READY;
				break;
			}

			//
			// Check input parameter size
			//
			if (IO_INPUTLEN(io_stack) < sizeof(VERIFY_INFORMATION)) {
				status = STATUS_INFO_LENGTH_MISMATCH;
				break;
			}

			//
			// Input parameter sanity check
			//
			param = (PVERIFY_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

			if (param->StartingOffset.QuadPart + param->Length >
				part_extension->PartitionInfo.PartitionLength.QuadPart)
			{
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			//
			// no need to actually perform the operation
			// -- virtual media is always clean
			//
			status = STATUS_SUCCESS;
			break;
		}

	//
	// Since removal lock is irrelevant for virtual disks,
	// there's really nothing to do here...
	//
	case IOCTL_DISK_MEDIA_REMOVAL:
	case IOCTL_STORAGE_MEDIA_REMOVAL:
		{
			status = STATUS_SUCCESS;
			break;
		}

#if 0
	//
	// Mount manager stuff... doesn't work for the time being :-(
	// I'd have to work harder, I guess.
	//
	case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID:	// WIN2K
	case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME:	// WIN2K
		{
			PMOUNTDEV_NAME	dev_name;

			Irp->IoStatus.Information =
				FIELD_OFFSET(MOUNTDEV_NAME, Name) +
				part_extension->DeviceName.Length;

			if (IO_OUTPUTLEN(io_stack) < Irp->IoStatus.Information) {
				status = STATUS_BUFFER_OVERFLOW;
				break;
			}

			dev_name = (PMOUNTDEV_NAME)Irp->AssociatedIrp.SystemBuffer;

			dev_name->NameLength = part_extension->DeviceName.Length;

			RtlCopyMemory(
				dev_name->Name,
				part_extension->DeviceName.Buffer,
				part_extension->DeviceName.Length);

			status = STATUS_SUCCESS;
		}
		break;
#endif	// 0

	default:
		//
		// Unknown IOCTL request
		//
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

#if DBG
	if ((TraceFlags & VDKIOCTL) && (((TraceFlags & VDKINFO) == VDKINFO) ||
		(!NT_SUCCESS(status) && (status != STATUS_DEVICE_NOT_READY ||
			(io_stack->Parameters.DeviceIoControl.IoControlCode != IOCTL_DISK_GET_PARTITION_INFO &&
			io_stack->Parameters.DeviceIoControl.IoControlCode != IOCTL_DISK_GET_DRIVE_GEOMETRY &&
			io_stack->Parameters.DeviceIoControl.IoControlCode != IOCTL_DISK_IS_WRITABLE))))) {

		PrintIoCtrlStatus(
			part_extension->DeviceName.Buffer,
			io_stack->Parameters.DeviceIoControl.IoControlCode,
			status);
	}
#endif	// DBG

	if (status == STATUS_PENDING) {
		//
		// Let the device thread perform the operation
		//
		IoMarkIrpPending(Irp);

		ExInterlockedInsertTailList(
			&disk_extension->ListHead,
			&Irp->Tail.Overlay.ListEntry,
			&disk_extension->ListLock);

		KeSetEvent(
			&disk_extension->RequestEvent,
			(KPRIORITY) 0,
			FALSE);
	}
	else {
		//
		// complete the operation
		//
		Irp->IoStatus.Status = status;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}

	return status;
}

