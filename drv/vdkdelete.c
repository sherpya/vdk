/*
	vdkdelete.c

	Virtual Disk kernel-mode driver for Windows NT platform
	Delete disk/partition device objects
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkaccess.h"

#include "imports.h"
#include "vdkdrv.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdkDeleteDevice)
#endif	// ALLOC_PRAGMA

//
//	delete a device object
//
VOID
VdkDeleteDevice(
	IN PDEVICE_OBJECT	DeviceObject)
{
	PPART_EXTENSION	part_extension;

	VDKTRACE(VDKDELETE | VDKINFO,
		("[VDK] VdkDeleteDevice\n"));

	part_extension =
		(PPART_EXTENSION)DeviceObject->DeviceExtension;

	//
	// Release resourse common to disks and partitions
	//
	VDKTRACE(VDKDELETE | VDKINFO,
		("[VDK] Deleting %ws\n",
		part_extension->DeviceName.Buffer));

	if (part_extension->DeviceName.Buffer) {
		ExFreePool(part_extension->DeviceName.Buffer);
		RtlZeroMemory
			(&part_extension->DeviceName,
			sizeof(part_extension->DeviceName));
	}

	if (part_extension->SymbolicLink.Buffer) {
		VDKTRACE(VDKDELETE | VDKINFO,
			("[VDK] Deleting Symbolic link %ws\n",
			part_extension->SymbolicLink.Buffer));

		IoDeleteSymbolicLink(&part_extension->SymbolicLink);

		ExFreePool(part_extension->SymbolicLink.Buffer);
		RtlZeroMemory
			(&part_extension->SymbolicLink,
			sizeof(part_extension->SymbolicLink));
	}

	if (part_extension == (PPART_EXTENSION)part_extension->FirstPartition) {

		//	This is a disk device object
		PDISK_EXTENSION disk_extension = (PDISK_EXTENSION)part_extension;

		// Delete and release all resources that might be created
		//
		// - Disk device thread
		// - Image file information
		// - Symbolic link
		// - Directory object
		//

		//
		// Close the image files
		//
		if (disk_extension->DiskInfo.DiskType) {
			VdkCloseDisk(&disk_extension->DiskInfo);
		}

		//
		// Terminate disk device thread
		//
		VDKTRACE(VDKDELETE | VDKINFO,
			("[VDK] Terminating device thread\n"));

		disk_extension->TerminateThread = TRUE;

		KeSetEvent(
			&disk_extension->RequestEvent,
			(KPRIORITY) 0,
			FALSE);

		KeWaitForSingleObject(
			disk_extension->ThreadPointer,
			Executive,
			KernelMode,
			FALSE,
			NULL);

		ObDereferenceObject(disk_extension->ThreadPointer);

#ifdef VDK_SUPPORT_NETWORK
		//
		// Delete security context object
		//
		if (disk_extension->SecurityContext) {

			VDKTRACE(VDKDELETE | VDKINFO,
				("[VDK] Freeing security context\n"));

			SeDeleteClientSecurity(disk_extension->SecurityContext);

			ExFreePool(disk_extension->SecurityContext);
			RtlZeroMemory
				(&disk_extension->SecurityContext,
				sizeof(disk_extension->SecurityContext));

		}
#endif	// VDK_SUPPORT_NETWORK

		//
		// Delete default symbolic links
		//

		if (disk_extension->AnotherLink.Buffer) {
			VDKTRACE(VDKDELETE | VDKINFO,
				("[VDK] Deleting Symbolic link %ws\n",
				disk_extension->AnotherLink.Buffer));

			IoDeleteSymbolicLink(&disk_extension->AnotherLink);

			ExFreePool(disk_extension->AnotherLink.Buffer);
			RtlZeroMemory
				(&disk_extension->AnotherLink,
				sizeof(disk_extension->AnotherLink));
		}

		//
		// Delete the directory object
		//
		if (disk_extension->DirectoryHandle) {

			VDKTRACE(VDKDELETE | VDKINFO,
				("[VDK] Closing directory handle\n"));

			ZwClose(disk_extension->DirectoryHandle);
		}

		IoGetConfigurationInformation()->DiskCount--;
	}

	//
	// Delete the device object
	//
	IoDeleteDevice(DeviceObject);

	return;
}

