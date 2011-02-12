/*
	vdkdrv.c

	Virtual Disk kernel-mode driver for Windows NT platform
	Standard driver routines other than ioctl dispatch routine
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkver.h"
#include "vdkioctl.h"
#include "vdkaccess.h"

#include "imports.h"
#include "vdkdrv.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, VdkShutdown)
#pragma alloc_text(PAGE, VdkCreateClose)
#pragma alloc_text(PAGE, VdkUnloadDriver)
#endif	// ALLOC_PRAGMA

//
//	runtime operating system version
//
extern ULONG OsMajorVersion = 0;
extern ULONG OsMinorVersion = 0;
extern ULONG OsBuildNumber 	= 0;

//
// Driver Entry routine
//
NTSTATUS
DriverEntry (
	IN PDRIVER_OBJECT	DriverObject,
	IN PUNICODE_STRING	RegistryPath)
{
	ULONG		idx;
	ULONG		disk_num = 0;
	PWCHAR 		reg_path = NULL;
	NTSTATUS	status;

	VDKTRACE(0, ("[VDK] " VDK_PRODUCT_NAME " version "
		VDK_DRIVER_VERSION_STR " by " VDK_COMPANY_NAME "\n"));

	// Store running OS version into global variables

	StoreCurrentOsVersion();

	VDKTRACE(0,
		("[VDK] Running on Windows NT %lu.%lu build %lu\n",
		OsMajorVersion, OsMinorVersion, OsBuildNumber));

	// Get driver config data from registry

	reg_path = ExAllocatePool(
		PagedPool, RegistryPath->Length + sizeof(WCHAR));

	if (reg_path) {
#if DBG
		RTL_QUERY_REGISTRY_TABLE	reg_param[3];
		ULONG default_trace = (ULONG)~(VDKINFO | VDKWARN);
#else
		RTL_QUERY_REGISTRY_TABLE	reg_param[2];
#endif
		idx = 0;

		RtlZeroMemory(reg_path, RegistryPath->Length + sizeof(WCHAR));
		RtlMoveMemory(reg_path, RegistryPath->Buffer, RegistryPath->Length);

		RtlZeroMemory(reg_param, sizeof(reg_param));

		reg_param[0].Flags			= RTL_QUERY_REGISTRY_DIRECT;
		reg_param[0].Name			= VDK_REG_DISKNUM_VALUE;
		reg_param[0].EntryContext 	= &disk_num;
		reg_param[0].DefaultType	= REG_DWORD;
		reg_param[0].DefaultData	= &idx;
		reg_param[0].DefaultLength	= sizeof(ULONG);

#if DBG
		reg_param[1].Flags			= RTL_QUERY_REGISTRY_DIRECT;
		reg_param[1].Name			= L"TraceFlags";
		reg_param[1].EntryContext	= &TraceFlags;
		reg_param[1].DefaultType	= REG_DWORD;
		reg_param[1].DefaultData	= &default_trace;
		reg_param[1].DefaultLength	= sizeof(ULONG);
#endif

		status = RtlQueryRegistryValues(
			RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
			reg_path, reg_param, NULL, NULL);

		ExFreePool(reg_path);

		if (!NT_SUCCESS(status)) {

			VDKTRACE(0,
				("[VDK] Failed to get config dat from registry - %s\n",
				VdkStatusStr(status)));

			disk_num = VDK_DEFAULT_DISK_NUM;
		}

		if (disk_num == 0 || disk_num > VDK_MAXIMUM_DISK_NUM) {
			VDKTRACE(0,
				("[VDK] Invalid disk device number - %lu\n", disk_num));

			disk_num = VDK_DEFAULT_DISK_NUM;
		}
	}
	else {
		//
		// Failed to allocate regstry path buffer
		//
		VDKTRACE(0,
			("[VDK] Failed to allocate registry path buffer.\n"));

		disk_num = VDK_DEFAULT_DISK_NUM;
	}

	VDKTRACE(0,
		("[VDK] Number of Disks = %lu, TraceFlags = 0x%08x\n",
		disk_num, TraceFlags));

	// Create disk device objects

	idx = 0;

	do {
		status = VdkCreateDisk(DriverObject, idx);

		if (!NT_SUCCESS(status)) {

			if (DriverObject->DeviceObject) {
				// at least one disk device was created
				status = STATUS_SUCCESS;
			}
			break;
		}
	}
	while (++idx < disk_num);

	if (NT_SUCCESS(status)) {

		// Setup dispatch table

		DriverObject->MajorFunction[IRP_MJ_CREATE]			= VdkCreateClose;
		DriverObject->MajorFunction[IRP_MJ_CLOSE]			= VdkCreateClose;
		DriverObject->MajorFunction[IRP_MJ_READ]			= VdkReadWrite;
		DriverObject->MajorFunction[IRP_MJ_WRITE]			= VdkReadWrite;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= VdkDeviceControl;
		DriverObject->MajorFunction[IRP_MJ_SHUTDOWN]		= VdkShutdown;
		DriverObject->DriverUnload = VdkUnloadDriver;

		VDKTRACE(0, ("[VDK] %lu virtual disks successfully initialized.\n", idx));
	}

	return status;
}

//
//	Device shutdown routine
//
NTSTATUS
VdkShutdown (
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				Irp)
{
	PDISK_EXTENSION	disk_extension;

	UNREFERENCED_PARAMETER(Irp);

	VDKTRACE(VDKDISPATCH | VDKINFO,
		("[VDK] VdkShutdown\n"));

	disk_extension =
		(PDISK_EXTENSION)DeviceObject->DeviceExtension;

	if (disk_extension &&
		disk_extension == disk_extension->FirstPartition &&
		disk_extension->DiskInfo.DiskType) {

		// close if virtual disk is opened

		disk_extension->DiskInfo.DiskType = VDK_DISKTYPE_NONE;
		VdkCloseDisk(&disk_extension->DiskInfo);
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

//
// IRP_MJ_CREATE and IRP_MJ_CLOSE handler
// Really nothing to do here...
//
NTSTATUS
VdkCreateClose (
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP 			Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	VDKTRACE(VDKDISPATCH | VDKINFO,
		("[VDK] %ws %s\n",
		((PPART_EXTENSION)DeviceObject->DeviceExtension)->DeviceName.Buffer,
		IoGetCurrentIrpStackLocation(Irp)->MajorFunction ==
			IRP_MJ_CLOSE ? "IRP_MJ_CLOSE" : "IRP_MJ_CREATE"));

	//
	// Information value for MJ_CLOSE is not defined so it's OK to
	// return FILE_OPEND as defined for MJ_OPEN
	//
	Irp->IoStatus.Information = FILE_OPENED;
	Irp->IoStatus.Status = STATUS_SUCCESS;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

//
// Driver unload routine
//
VOID
VdkUnloadDriver (
	IN PDRIVER_OBJECT DriverObject)
{
	PDEVICE_OBJECT	device_object;
	PDEVICE_OBJECT	next_device;

	VDKTRACE(VDKDISPATCH | VDKINFO,
		("[VDK] VdkUnloadDriver\n"));

	device_object = DriverObject->DeviceObject;

	// Delete all device objects

	while (device_object) {
		next_device = device_object->NextDevice;

		VdkDeleteDevice(device_object);

		device_object = next_device;
	}

	VDKTRACE(0, ("[VDK] driver unloaded\n"));
}

//
// IRP_MJ_READ and IRP_MJ_WRITE handler
// Insert the IRP into queue list.
// Actual read/write operation is performed by device thread
//
#define IO_READ_OFF(p)	(p)->Parameters.Read.ByteOffset.QuadPart
#define IO_READ_LEN(p)	(p)->Parameters.Read.Length

NTSTATUS
VdkReadWrite (
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP 			Irp)
{
	PPART_EXTENSION		part_extension;
	PDISK_EXTENSION		disk_extension;
	PIO_STACK_LOCATION	io_stack;

	//
	// Set up necessary object and extension pointers.
	//
	part_extension =
		(PPART_EXTENSION)DeviceObject->DeviceExtension;

	disk_extension =
		(PDISK_EXTENSION)part_extension->FirstPartition;

	io_stack = IoGetCurrentIrpStackLocation(Irp);

	//
	//	Check if the device is a zombie -- closed but not released
	//	READ / WRITE request must succeed or NTFS driver cannot release
	//	the device.
	//
	if (part_extension->PartitionOrdinal == VDK_ZOMBIE) {
		VDKTRACE(0,
			("[VDK] %ws %s: ZOMBIE DEVICE\n",
			part_extension->DeviceName.Buffer,
			io_stack->MajorFunction == IRP_MJ_WRITE ? "IRP_MJ_WRITE" : "IRP_MJ_READ"));

		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = IO_READ_LEN(io_stack);

		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return STATUS_SUCCESS;
	}

	//
	// Check if image is opened
	//
	if (!disk_extension || !disk_extension->DiskInfo.DiskType ||
		!part_extension->PartitionInfo.PartitionLength.QuadPart)	{

		VDKTRACE(VDKWARN,
			("[VDK] %ws %s: STATUS_DEVICE_NOT_READY\n",
			part_extension->DeviceName.Buffer,
			io_stack->MajorFunction == IRP_MJ_WRITE ? "IRP_MJ_WRITE" : "IRP_MJ_READ"));

		Irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
		Irp->IoStatus.Information = 0;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return STATUS_DEVICE_NOT_READY;
	}

	//
	// Check for invalid parameters.  It is an error for the starting offset
	// + length to go past the end of the partition, or for the length or
	// offset to not be a proper multiple of the sector size.
	//
	// Others are possible, but we don't check them since we trust the
	// file system and they aren't deadly.
	//
	if (IO_READ_OFF(io_stack) + IO_READ_LEN(io_stack) >
		part_extension->PartitionInfo.PartitionLength.QuadPart) {

		VDKTRACE(0,
			("[VDK] %ws Offset:%I64u + Length:%u goes past partition size:%I64u\n",
			part_extension->DeviceName.Buffer,
			IO_READ_OFF(io_stack),
			IO_READ_LEN(io_stack),
			part_extension->PartitionInfo.PartitionLength.QuadPart));

		Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		Irp->IoStatus.Information = 0;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return STATUS_INVALID_PARAMETER;
	}

	if ((IO_READ_LEN(io_stack) & VDK_SECTOR_ALIGNMENT_MASK) ||
		(IO_READ_OFF(io_stack) & VDK_SECTOR_ALIGNMENT_MASK)) {

		VDKTRACE(0,
			("[VDK] %ws Invalid Alignment Offset:%I64u Length:%u\n",
			part_extension->DeviceName.Buffer,
			IO_READ_OFF(io_stack), IO_READ_LEN(io_stack)));

		Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		Irp->IoStatus.Information = 0;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return STATUS_INVALID_PARAMETER;
	}

	//
	// Check if write operation is allowed
	//
	if (io_stack->MajorFunction == IRP_MJ_WRITE) {
		if (disk_extension->DiskInfo.DiskType == VDK_DISKTYPE_READONLY) {

			VDKTRACE(VDKWRITE | VDKWARN,
				("[VDK] %ws IRP_MJ_WRITE: STATUS_MEDIA_WRITE_PROTECTED\n",
				disk_extension->DeviceName.Buffer));

			Irp->IoStatus.Status = STATUS_MEDIA_WRITE_PROTECTED;
			Irp->IoStatus.Information = 0;

			IoCompleteRequest(Irp, IO_NO_INCREMENT);

			return STATUS_MEDIA_WRITE_PROTECTED;
		}
		else if (disk_extension->DiskInfo.DiskType == VDK_DISKTYPE_WRITEBLOCK) {

			VDKTRACE(VDKWRITE | VDKINFO,
				("[VDK] %ws IRP_MJ_WRITE: Blocked write operation\n",
				disk_extension->DeviceName.Buffer));

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = IO_READ_LEN(io_stack);

			IoCompleteRequest(Irp, IO_NO_INCREMENT);

			return STATUS_SUCCESS;
		}
	}

	//
	// If read/write data length is 0, we are done
	//
	if (IO_READ_LEN(io_stack) == 0) {
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return STATUS_SUCCESS;
	}

	//
	// The offset passed in is relative to the start of the partition.
	// We always work from partition 0 (the whole disk) so adjust the
	// offset.
	//
	IO_READ_OFF(io_stack) +=
		part_extension->PartitionInfo.StartingOffset.QuadPart;

	//
	// Mark the IRP as pending, insert the IRP into queue list
	// then signal the device thread to perform the operation
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

	return STATUS_PENDING;
}
