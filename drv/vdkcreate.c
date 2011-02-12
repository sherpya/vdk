/*
	vdkcreate.c

	Virtual Disk kernel-mode driver for Windows NT platform
	Create disk device object
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkioctl.h"
#include "vdkaccess.h"

#include "imports.h"
#include "vdkdrv.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdkCreateDisk)
#endif	// ALLOC_PRAGMA

//
// Create a new disk device
//
NTSTATUS
VdkCreateDisk(
	IN PDRIVER_OBJECT	DriverObject,
	IN ULONG			DiskNumber)
{
	NTSTATUS			status;

	// for creating the directory object
	ULONG				drive_number;
	WCHAR				dir_buffer[20];
	UNICODE_STRING		dir_name;
	OBJECT_ATTRIBUTES	dir_attrib;
	HANDLE				dir_handle		= NULL;

	// for creating symbolic links
	WCHAR				link_buffer[25];
	UNICODE_STRING		link_name;
	BOOLEAN				link_created	= FALSE;

	// for creating device objects
	WCHAR				obj_buffer[40];
	UNICODE_STRING		obj_name;
	PDEVICE_OBJECT		disk_object		= NULL;
	PDISK_EXTENSION		disk_extension	= NULL;

	// for creating disk device thread
	HANDLE				thread_handle	= NULL;

	VDKTRACE(VDKCREATE|VDKINFO,
		("[VDK] VdkCreateDevice\n"));

	///////////////////////////////////////////////////////
	// Create the directory object
	// \Device\Harddisk<n>
	// -- first create it permanent and then make it temporary.
	//
	drive_number = IoGetConfigurationInformation()->DiskCount;

	do {
		swprintf(dir_buffer, L"\\Device\\Harddisk%lu", drive_number);

		RtlInitUnicodeString(&dir_name, dir_buffer);

		InitializeObjectAttributes(
			&dir_attrib, &dir_name, OBJ_PERMANENT, NULL, NULL);

		status = ZwCreateDirectoryObject(
			&dir_handle, DIRECTORY_ALL_ACCESS, &dir_attrib);

		if (status != STATUS_OBJECT_NAME_EXISTS) {
			break;
		}
	}
	while (++drive_number < 100);

	if (!NT_SUCCESS(status)) {
		VDKTRACE(VDKCREATE,
			("[VDK] ZwCreateDirectoryObject(%ws) %s\n",
			dir_buffer,
			VdkStatusStr(status)));

		goto cleanup;
	}

	VDKTRACE(VDKCREATE|VDKINFO,
		("[VDK] Created a directory object %ws\n", dir_buffer));

	status = ZwMakeTemporaryObject(dir_handle);

	if (!NT_SUCCESS(status)) {
		VDKTRACE(VDKCREATE,
			("[VDK] ZwMakeTemporaryObject() %s\n",
			VdkStatusStr(status)));

		goto cleanup;
	}

	///////////////////////////////////////////////////////
	// Create the default sumbolic link in the DOS namespace
	// \??\VirtualDK<n>
	//

	swprintf(link_buffer, L"\\??\\" VDK_DEVICE_BASENAME L"%lu", DiskNumber);

	RtlInitUnicodeString(&link_name, link_buffer);

	status = IoCreateSymbolicLink(&link_name, &dir_name);

	if (!NT_SUCCESS(status)) {
		VDKTRACE(VDKCREATE,
			("[VDK] IoCreateSymbolicLink(%ws) %s\n",
			link_buffer,
			VdkStatusStr(status)));

		goto cleanup;
	}

	link_created = TRUE;

	VDKTRACE(VDKCREATE|VDKINFO,
		("[VDK] Created a symbolic link %ws\n", link_buffer));

	///////////////////////////////////////////////////////
	// Create the disk device object
	// \Device\Harddisk<x>\Partition0
	//
	swprintf(obj_buffer,
		L"\\Device\\Harddisk%lu\\Partition0", drive_number);

	RtlInitUnicodeString(&obj_name, obj_buffer);

	status = IoCreateDevice(
		DriverObject,
		sizeof(DISK_EXTENSION),
		&obj_name,
		FILE_DEVICE_DISK,
		0,
		FALSE,
		&disk_object);

	if (!NT_SUCCESS(status)) {
		VDKTRACE(VDKCREATE, ("[VDK] IoCreateDevice(%ws) %s\n",
			obj_buffer,
			VdkStatusStr(status)));

		goto cleanup;
	}

	IoGetConfigurationInformation()->DiskCount++;

	VDKTRACE(VDKCREATE|VDKINFO,
		("[VDK] Created a disk device object %ws\n", obj_buffer));

	///////////////////////////////////////////////////////
	// Initialize the device object and extension.
	//
	disk_object->Flags |= DO_DIRECT_IO;

	if (DriverObject->DriverUnload) {

		// Called from outside the DriverEntry routine

		disk_object->Flags &= ~DO_DEVICE_INITIALIZING;
	}

	disk_extension = (PDISK_EXTENSION)(disk_object->DeviceExtension);

	RtlZeroMemory(disk_extension, sizeof(DISK_EXTENSION));

	disk_extension->FirstPartition	= disk_extension;
	disk_extension->DeviceObject 	= disk_object;
	disk_extension->DirectoryHandle = dir_handle;
	disk_extension->PhysicalNumber	= drive_number;
	disk_extension->VirtualNumber	= DiskNumber;

	//
	// Store device name
	//
	disk_extension->DeviceName.Buffer =
		ExAllocatePool(NonPagedPool, obj_name.Length + sizeof(WCHAR));

	if (!disk_extension->DeviceName.Buffer) {
		VDKTRACE(VDKCREATE,
			("[VDK] Failed to allocate device name buffer.\n"));

		status = STATUS_INSUFFICIENT_RESOURCES;
		goto cleanup;
	}

	disk_extension->DeviceName.Length = obj_name.Length;
	disk_extension->DeviceName.MaximumLength = obj_name.Length;

	RtlZeroMemory(
		disk_extension->DeviceName.Buffer,
		obj_name.Length + sizeof(WCHAR));

	RtlCopyMemory(
		disk_extension->DeviceName.Buffer,
		obj_name.Buffer,
		obj_name.Length);

	//
	// Store symbolic link
	//
	disk_extension->SymbolicLink.Buffer =
		ExAllocatePool(NonPagedPool, link_name.Length + sizeof(WCHAR));

	if (!disk_extension->SymbolicLink.Buffer) {
		VDKTRACE(VDKCREATE,
			("[VDK] Failed to allocate symbolic link buffer.\n"));

		status = STATUS_INSUFFICIENT_RESOURCES;
		goto cleanup;
	}

	disk_extension->SymbolicLink.Length = link_name.Length;
	disk_extension->SymbolicLink.MaximumLength = link_name.Length;

	RtlZeroMemory(
		disk_extension->SymbolicLink.Buffer,
		link_name.Length + sizeof(WCHAR));

	RtlCopyMemory(
		disk_extension->SymbolicLink.Buffer,
		link_name.Buffer,
		link_name.Length);


	///////////////////////////////////////////////////
	// Create the disk device thread which performs
	// most of actual I/O tasks in the system context
	//
	InitializeListHead(&disk_extension->ListHead);

	KeInitializeSpinLock(&disk_extension->ListLock);

	KeInitializeEvent(
		&disk_extension->RequestEvent,
		SynchronizationEvent,
		FALSE);

	disk_extension->TerminateThread = FALSE;

	status = PsCreateSystemThread(
		&thread_handle,
		(ACCESS_MASK) 0L,
		NULL,
		NULL,
		NULL,
		VdkThread,
		disk_object);

	if (!NT_SUCCESS(status)) {
		VDKTRACE(VDKCREATE,
			("[VDK] PsCreateSystemThread() %s\n",
			VdkStatusStr(status)));

		goto cleanup;
	}

	status = ObReferenceObjectByHandle(
		thread_handle,
		THREAD_ALL_ACCESS,
		NULL,
		KernelMode,
		&disk_extension->ThreadPointer,
		NULL);

	ZwClose(thread_handle);

	if (!NT_SUCCESS(status)) {
		VDKTRACE(VDKCREATE,
			("[VDK] ObReferenceObjectByHandle() %s\n",
			VdkStatusStr(status)));

		goto cleanup;
	}

	///////////////////////////////////////////////////////
	// Create alternate symbolic link
	// \??\PhysicalDrive<n>
	//
	swprintf(link_buffer,
		L"\\??\\PhysicalDrive%lu", disk_extension->PhysicalNumber);

	RtlInitUnicodeString(&link_name, link_buffer);

	disk_extension->AnotherLink.Buffer =
		ExAllocatePool(NonPagedPool, link_name.Length + sizeof(WCHAR));

	if (disk_extension->AnotherLink.Buffer) {
		disk_extension->AnotherLink.Length = link_name.Length;
		disk_extension->AnotherLink.MaximumLength = link_name.Length;

		RtlZeroMemory(
			disk_extension->AnotherLink.Buffer,
			link_name.Length + sizeof(WCHAR));

		RtlCopyMemory(
			disk_extension->AnotherLink.Buffer,
			link_name.Buffer,
			link_name.Length);

		IoCreateSymbolicLink(&link_name, &obj_name);

		VDKTRACE(VDKCREATE|VDKINFO,
			("[VDK] Created a symbolic link %ws\n", link_buffer));
	}
	else {
		// this is not fatal - it is just that some programs may not
		// recognize the virtual drive as a hard disk drive
		VDKTRACE(VDKCREATE,
			("[VDK] Failed to allocate alternate symlink name buffer.\n"));
	}


cleanup:
	if (!NT_SUCCESS(status)) {
		//
		// Delete everything created in this routine
		//
		// - Disk device thread
		// - Disk device object
		// - Symbolic links
		// - Directory object
		//

		if (disk_extension) {
			//
			// Terminate disk device thread
			//
			if (thread_handle) {
				VDKTRACE(VDKCREATE,
					("[VDK] Terminating device thread\n"));

				disk_extension->TerminateThread = TRUE;

				KeSetEvent(
					&disk_extension->RequestEvent,
					(KPRIORITY) 0,
					FALSE);
			}

			//
			// Release the reference pointer
			//
			if (disk_extension->ThreadPointer) {
				VDKTRACE(VDKCREATE,
					("[VDK] Releasing the thread reference pointer\n"));

				ObDereferenceObject(disk_extension->ThreadPointer);
			}

			if (disk_extension->DeviceName.Buffer) {
				ExFreePool(disk_extension->DeviceName.Buffer);
			}

			if (disk_extension->SymbolicLink.Buffer) {
				ExFreePool(disk_extension->SymbolicLink.Buffer);
			}

			if (disk_extension->AnotherLink.Buffer) {
				ExFreePool(disk_extension->AnotherLink.Buffer);
			}
		}

		//
		// Delete default symbolic link
		//
		if (link_created) {
			VDKTRACE(VDKCREATE,
				("[VDK] Deleting symbolic link %ws\n", link_buffer));

			IoDeleteSymbolicLink(&link_name);
		}

		//
		// Delete disk device object
		//
		if (disk_object) {
			VDKTRACE(VDKCREATE,
				("[VDK] Deleting device object %ws\n", obj_buffer));

			IoDeleteDevice(disk_object);
		}

		//
		// Delete directory object
		//
		if (dir_handle) {
			VDKTRACE(VDKCREATE,
				("[VDK] Closing directory handle %ws\n", dir_buffer));

			ZwClose(dir_handle);
		}
	}

	return status;
}

