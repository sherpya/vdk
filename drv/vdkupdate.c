/*
	vdkupdate.c

	Virtual Disk kernel-mode driver for Windows NT platform
	Device update routine
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkaccess.h"

#include "imports.h"
#include "vdkdrv.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdkUpdateDevice)
#endif	// ALLOC_PRAGMA

//
// This routine creates, deletes and changes device objects when
// the IOCTL_SET_DRIVE_LAYOUT is called.  It also updates the partition
// number information in the structure that will be returned to the caller.
// This routine can be used even in the GET_INFO case by insuring that
// the RewritePartition flag is off for all partitions in the drive layout
// structure.
//
VOID
VdkUpdateDevice(
	IN PDEVICE_OBJECT				DiskObject,
	IN PDRIVE_LAYOUT_INFORMATION	DriveLayout)
{
	PDISK_EXTENSION			disk_extension;
	PPART_EXTENSION			part_extension;
	PPARTITION_INFORMATION	partition_info;

	ULONG	part_count;
	ULONG	part_ordinal;
	ULONG	idx;

	VDKTRACE(VDKUPDATE | VDKINFO, ("[VDK] VdkUpdateDevice\n"));

	//
	// The virtual disk is non-partitioned (super-floppy) image
	// in this case, entire disk is treated as a single partition
	// and device 0 represents the partition.
	//
	if (DriveLayout->PartitionCount == 1 &&
		DriveLayout->PartitionEntry[0].StartingOffset.QuadPart == 0) {

		VDKTRACE(VDKUPDATE | VDKWARN,
			("[VDK] Non-Partitioned disk\n"));

		DriveLayout->PartitionEntry[0].PartitionNumber = 0;
		return;
	}

	//
	// First clear all partition numbers in the partition list.
	//
	part_count	= DriveLayout->PartitionCount;

	for (idx = 0; idx < part_count; idx++) {
		VDKTRACE(VDKUPDATE | VDKINFO,
			("[VDK] PartitionEntry[%lu].StartingOffset = %I64u\n", idx,
			DriveLayout->PartitionEntry[idx].StartingOffset.QuadPart));

		VDKTRACE(VDKUPDATE | VDKINFO,
			("[VDK] PartitionEntry[%lu].PartitionLength = %I64u\n", idx,
			DriveLayout->PartitionEntry[idx].PartitionLength.QuadPart));

		VDKTRACE(VDKUPDATE | VDKINFO,
			("[VDK] PartitionEntry[%lu].HiddenSectors = %lu\n", idx,
			DriveLayout->PartitionEntry[idx].HiddenSectors));

		VDKTRACE(VDKUPDATE | VDKINFO,
			("[VDK] PartitionEntry[%lu].PartitionNumber = %lu\n", idx,
			DriveLayout->PartitionEntry[idx].PartitionNumber));

		VDKTRACE(VDKUPDATE | VDKINFO,
			("[VDK] PartitionEntry[%lu].PartitionType = %lu\n", idx,
			DriveLayout->PartitionEntry[idx].PartitionType));

		VDKTRACE(VDKUPDATE | VDKINFO,
			("[VDK] PartitionEntry[%lu].BootIndicator = %lu\n", idx,
			DriveLayout->PartitionEntry[idx].BootIndicator));

		VDKTRACE(VDKUPDATE | VDKINFO,
			("[VDK] PartitionEntry[%lu].RecognizedPartition = %lu\n", idx,
			DriveLayout->PartitionEntry[idx].RecognizedPartition));

		VDKTRACE(VDKUPDATE | VDKINFO,
			("[VDK] PartitionEntry[%lu].RewritePartition = %lu\n", idx,
			DriveLayout->PartitionEntry[idx].RewritePartition));

		DriveLayout->PartitionEntry[idx].PartitionNumber = 0;
	}

	disk_extension = (PDISK_EXTENSION)DiskObject->DeviceExtension;
	part_extension = (PPART_EXTENSION)disk_extension;

	//
	// Walk through chain of partition devices to determine if
	// each existing partition device has a match in the layout list.
	//

	while (part_extension->NextPartition) {

		PPART_EXTENSION prev_part;

		prev_part = part_extension;

		part_extension =
			part_extension->NextPartition->DeviceExtension;

		//
		// Loop through partition information table to look for match.
		//
		part_ordinal = 0;

		for (idx = 0; idx < part_count; idx++) {

			//
			// Get partition info entry.
			//
			partition_info = &DriveLayout->PartitionEntry[idx];

			//
			// Skip empty or extended partitions
			//
			if (partition_info->PartitionType == PARTITION_ENTRY_UNUSED ||
				IsContainerPartition(partition_info->PartitionType)) {
				continue;
			}

			//
			// Advance partition ordinal.
			//
			part_ordinal++;

			//
			// Check if start offset and length is the same
			//
			if (partition_info->StartingOffset.QuadPart ==
				part_extension->PartitionInfo.StartingOffset.QuadPart &&
				partition_info->PartitionLength.QuadPart ==
				part_extension->PartitionInfo.PartitionLength.QuadPart) {

				//
				// This device matches the partition item in the list.
				//
				VDKTRACE(VDKUPDATE | VDKINFO,
					("[VDK] list item #%u matches object %u\n",
					part_extension->PartitionInfo.PartitionNumber,
					idx));

				partition_info->PartitionNumber
					= part_extension->PartitionInfo.PartitionNumber;

				//
				// update PartitionOrdinal to match the order this partition
				// is placed on the disk
				//
				part_extension->PartitionOrdinal = part_ordinal;

				//
				// A match is found.  If this partition is marked for update,
				// check for a partition type change.
				//
				if (partition_info->RewritePartition) {
					part_extension->PartitionInfo.PartitionType
						= partition_info->PartitionType;
				}

				break;
			}

		}	// next partition info entry

		if (idx == part_count) {

			//
			// no match was found, indicate the partition represented by
			// this partition device is gone.	This device is no longer used
			// so remove it from the chain
			//
			if (!prev_part->NextPartition->Vpb ||
				!prev_part->NextPartition->Vpb->ReferenceCount) {

				VDKTRACE(VDKUPDATE | VDKINFO,
					("[VDK] %ws becomes an orphan.\n",
					part_extension->DeviceName.Buffer));

				part_extension->PartitionOrdinal	= 0;
			}
			else {
				VDKTRACE(VDKUPDATE | VDKWARN,
					("[VDK] %ws becomes a zombie.\n",
					part_extension->DeviceName.Buffer));

				part_extension->PartitionOrdinal	= VDK_ZOMBIE;
			}

			prev_part->NextPartition = part_extension->NextPartition;

			RtlZeroMemory(
				&part_extension->PartitionInfo,
				sizeof(part_extension->PartitionInfo));

			part_extension->FirstPartition 		= NULL;
			part_extension->NextPartition 		= NULL;

			if (part_extension->SymbolicLink.Buffer) {
				VDKTRACE(VDKUPDATE | VDKINFO,
					("[VDK] Deleting symbolic link %ws\n",
					part_extension->SymbolicLink.Buffer));

				IoDeleteSymbolicLink(&part_extension->SymbolicLink);
				ExFreePool(part_extension->SymbolicLink.Buffer);
				RtlZeroMemory(
					&part_extension->SymbolicLink,
					sizeof(part_extension->SymbolicLink));
			}

			part_extension = prev_part;
		   	continue;
		}

		//
		// Adjust access type of the device object
		//
		if (disk_extension->DiskInfo.DiskType == VDK_DISKTYPE_READONLY) {
			prev_part->NextPartition->Characteristics |= FILE_READ_ONLY_DEVICE;
		}
		else {
			prev_part->NextPartition->Characteristics &= ~FILE_READ_ONLY_DEVICE;
		}

	}	// next device in the chain

	//
	// Make sure each list item has a partition device to represent it.
	// In some cases new device objects will be created.
	//
	part_ordinal = 0;

	for (idx = 0; idx < part_count; idx++) {

		//
		// Partition info entry to work with
		//
		partition_info = &DriveLayout->PartitionEntry[idx];

		//
		// Skip empty or extended partitons
		//
		if (partition_info->PartitionType == PARTITION_ENTRY_UNUSED ||
			IsContainerPartition(partition_info->PartitionType)) {

			VDKTRACE(VDKUPDATE | VDKINFO,
				("[VDK] Skip: item %lu is unused or container\n", idx));

			continue;
		}

		//
		// Keep track of position of the partition on the disk
		//
		part_ordinal++;

		//
		// Skip partitions which don't need to be updated
		//
		if (!partition_info->RewritePartition ||	// rewrite flag is off or
			partition_info->PartitionNumber) {		// match was found

			// no need to update device object for this partition

			VDKTRACE(VDKUPDATE | VDKINFO,
				("[VDK] Skip: item %lu - matched or not for rewrite\n", idx));

			continue;
		}

		//
		// If partition number is still zero then a new device object
		// must be associated.
		//
		if (partition_info->PartitionNumber == 0) {

			PDEVICE_OBJECT	part_object;
			WCHAR			name_buf[40];
			UNICODE_STRING	uni_name;
			ULONG			part_num;
			NTSTATUS		status;

			//
			//	First search orphaned partition device available
			//

			part_object = DiskObject->DriverObject->DeviceObject;

			while (part_object) {
				if ((!part_object->Vpb || !part_object->Vpb->ReferenceCount) &&
					((PPART_EXTENSION)part_object->DeviceExtension)->FirstPartition == NULL) {
					break;
				}

				part_object = part_object->NextDevice;
			}

			if (part_object) {
				if (disk_extension->DiskInfo.DiskType == VDK_DISKTYPE_READONLY) {
					part_object->Characteristics |= FILE_READ_ONLY_DEVICE;
				}
				else {
					part_object->Characteristics &= ~FILE_READ_ONLY_DEVICE;
				}

				//
				// Link into the partition list.
				//
				part_extension->NextPartition = part_object;

				part_extension =
					(PPART_EXTENSION)part_object->DeviceExtension;
			}
			else {

				//
				//	Create a partition device
				//
				part_num = 0;

				do {
					swprintf(name_buf,
						L"\\Device\\VirtualDiskVolume%u", ++part_num);

					RtlInitUnicodeString(&uni_name, name_buf);

					status = IoCreateDevice(
						DiskObject->DriverObject,
						sizeof(PART_EXTENSION),
						&uni_name,
						FILE_DEVICE_DISK,
						(DiskObject->Characteristics & FILE_READ_ONLY_DEVICE),
						FALSE,
						&part_object);

					if (status != STATUS_OBJECT_NAME_COLLISION) {
						break;
					}
				}
				while (part_num < 100);

				if (!NT_SUCCESS(status)) {
					VDKTRACE(VDKUPDATE,
						("[VDK] IoCreateDevice(%ws) %s\n",
						name_buf,
						VdkStatusStr(status)));

					continue;
				}

				VDKTRACE(VDKUPDATE|VDKINFO,
					("[VDK] Created a partition device %ws\n",
					name_buf));

				//
				// Set up device object fields.
				//
				part_object->Flags		|= DO_DIRECT_IO;
				part_object->StackSize = DiskObject->StackSize;
				part_object->Flags		&= ~DO_DEVICE_INITIALIZING;

				//
				// Link into the partition chain.
				//
				part_extension->NextPartition = part_object;

				part_extension =
					(PPART_EXTENSION)part_object->DeviceExtension;

				RtlZeroMemory(part_extension, sizeof(PART_EXTENSION));

				//
				// Store device name
				//
				part_extension->DeviceName.Buffer =
					ExAllocatePool(NonPagedPool, uni_name.Length + sizeof(WCHAR));

				if (part_extension->DeviceName.Buffer) {
					part_extension->DeviceName.Length = uni_name.Length;
					part_extension->DeviceName.MaximumLength = uni_name.Length;

					RtlZeroMemory(
						part_extension->DeviceName.Buffer,
						uni_name.Length + sizeof(WCHAR));

					RtlCopyMemory(
						part_extension->DeviceName.Buffer,
						uni_name.Buffer,
						uni_name.Length);
				}
				else {
					VDKTRACE(VDKUPDATE,
						("[VDK] Failed to allocate device name buffer.\n"));
				}
			}

			VDKTRACE(VDKUPDATE | VDKINFO,
				("[VDK] Partition List Item %lu uses %ws\n",
				idx, part_extension->DeviceName.Buffer));

			//
			// Set up device extension fields.
			//
			part_extension->FirstPartition	= disk_extension;
			part_extension->NextPartition	= NULL;

			//
			// Create symbolic link
			// Win2K/XP style
			//
			part_num = 0;

			do {
				swprintf(name_buf,
					L"\\Device\\Harddisk%lu\\Partition%u",
					disk_extension->PhysicalNumber, ++part_num);

				RtlInitUnicodeString(&uni_name, name_buf);

				status = IoCreateSymbolicLink(
					&uni_name, &part_extension->DeviceName);

				if (status != STATUS_OBJECT_NAME_COLLISION) {
					break;
				}
			}
			while (part_num <= 128);

			if (NT_SUCCESS(status)) {
				VDKTRACE(VDKUPDATE|VDKINFO,
					("[VDK] Created a symbolic link %ws\n",
					name_buf));

				part_extension->SymbolicLink.Buffer =
					ExAllocatePool(NonPagedPool, uni_name.Length + sizeof(WCHAR));

				if (part_extension->SymbolicLink.Buffer) {
					part_extension->SymbolicLink.Length = uni_name.Length;
					part_extension->SymbolicLink.MaximumLength = uni_name.Length;

					RtlZeroMemory(
						part_extension->SymbolicLink.Buffer,
						uni_name.Length + sizeof(WCHAR));

					RtlCopyMemory(
						part_extension->SymbolicLink.Buffer,
						uni_name.Buffer,
						uni_name.Length);
				}
				else {
					VDKTRACE(VDKUPDATE,
						("[VDK] Failed to allocate symlink name buffer.\n"));
				}
			}
			else {
				VDKTRACE(VDKUPDATE,
					("[VDK] IoCreateSymbolicLink(%ws) %s\n",
					name_buf,
					VdkStatusStr(status)));
			}

			partition_info->PartitionNumber = part_num;

			//
			// informs the Mount Manager of the new device
			//
#if 0
			//
			// I can't make this work for the time being...
			// I guess I have to learn more about MM & PnP stuff :-(
			//
			{
				PDEVICE_OBJECT	mntmgr_dev;

				// Obtain a pointer to the Mount Manager device object
				{
					UNICODE_STRING	mntmgr_name;
					PFILE_OBJECT	mntmgr_file;

					RtlInitUnicodeString(
						&mntmgr_name,
						MOUNTMGR_DEVICE_NAME);

					status = IoGetDeviceObjectPointer(
						&mntmgr_name,
						FILE_READ_ATTRIBUTES,
						&mntmgr_file,
						&mntmgr_dev);
				}

				if (NT_SUCCESS(status)) {
					IO_STATUS_BLOCK			io_status;
					KEVENT					event;
					PIRP					irp;
					PMOUNTMGR_TARGET_NAME	target_name;
					USHORT					target_name_buf[MAXIMUM_FILENAME_LENGTH];

					target_name = (PMOUNTMGR_TARGET_NAME)target_name_buf;

					target_name->DeviceNameLength =
						part_extension->DeviceName.Length;

					RtlCopyMemory(
						target_name->DeviceName,
						part_extension->DeviceName.Buffer,
						part_extension->DeviceName.Length);

					KeInitializeEvent(&event, NotificationEvent, FALSE);

					irp = IoBuildDeviceIoControlRequest(
						IOCTL_MOUNTMGR_VOLUME_ARRIVAL_NOTIFICATION,
						mntmgr_dev,
						target_name,
						sizeof(target_name->DeviceNameLength) + target_name->DeviceNameLength,
						NULL,
						0,
						FALSE,
						&event,
						&io_status);

					if (irp) {
						status = IoCallDriver(mntmgr_dev, irp);

						if (status == STATUS_PENDING) {
							KeWaitForSingleObject(
								&event, Executive, KernelMode, FALSE, NULL);

							status = io_status.Status;
						}
					}
				}
				else {
					//
					// probably running on Windows NT
					//
					VDKTRACE(VDKUPDATE | VDKWARN,
						("[VDK] Failed to obtain the Mount Manager device object.\n"));
				}
			}
#endif	// 0
		}

		//
		// Update partition extension to match the latest state
		//
		part_extension->PartitionInfo		= *partition_info;
		part_extension->PartitionOrdinal	= part_ordinal;
	}

	VDKTRACE(VDKUPDATE | VDKINFO, ("[VDK] VdkUpdateDevice - EXIT\n"));

	return;
}

