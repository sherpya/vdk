/*
	vdkwrite.c

	Virtual Disk write function
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkaccess.h"

#include "cowdisk.h"
#include "vmdisk.h"


//
//	Copy existing data block from underlying files to current file
//

static VDKSTAT VdkCopyDataBlock(
	PVDK_DISK_INFO	DiskInfo,		// disk info
	PVDK_FILE_INFO	CurFile,		// current file item to write to
	ULONG			Offset,			// logical offset of the block to copy
	ULONG			Sectors);		// number of sectors to copy

//
// Write requested sectors into file
//

VDKSTAT VdkWriteSector(
	PVDK_DISK_INFO	DiskInfo,		// disk info
	ULONG			Offset,			// logical offset of the sector to write
	ULONG			Length,			// number of sectors to write
	PUCHAR			Buffer)			// buffer containing the data to write
{
	PVDK_FILE_INFO	cur_file;
	ULONG			file_offset;
	VDKSTAT			status;


	VDKTRACE(VDKWRITE | VDKINFO,
		("[VDK] VdkWriteFile - Request Offset sec. %u, length %u\n",
		Offset, Length));


	if (DiskInfo == NULL || DiskInfo->Files == NULL) {

		VDKTRACE(VDKWRITE,
			("[VDK] null DiskInfo\n"));

		return VDK_INTERNAL;
	}

	if (Offset + Length > DiskInfo->Capacity) {

		VDKTRACE(VDKWRITE,
			("[VDK] request out of range\n"));

		return VDK_INTERNAL;
	}

	if (Buffer == NULL) {

		VDKTRACE(VDKWRITE,
			("[VDK] null write buffer\n"));

		return VDK_INTERNAL;
	}

	cur_file = DiskInfo->Files;
	file_offset = 0;

	//
	// Start reading data from file
	//
	while (Length) {

		ULONG	write_length;
		ULONG	write_offset;

		//
		// Look for a file which contains the first sector to write
		//
		while (Offset >= cur_file->Capacity) {
			//
			// target sector continues to the next file
			//

			file_offset += cur_file->Capacity;
			Offset -= cur_file->Capacity;
			cur_file++;
		}

		VDKTRACE(VDKWRITE | VDKINFO,
			("[VDK] Checking file #%u.\n",
			cur_file - DiskInfo->Files));

		//
		// decide data length to be read at once
		//
		write_length = cur_file->Capacity;

		if (cur_file->FileType == VDK_FILETYPE_NONE) {

			VDKTRACE(VDKWRITE,
				("[VDK] file is missing.\n"));

			return VDK_INTERNAL;
		}
		else if (cur_file->FileType == VDK_FILETYPE_COWD) {

			if (write_length > cur_file->prm.cowd->SecondaryGranularity) {
				write_length = cur_file->prm.cowd->SecondaryGranularity;

				VDKTRACE(VDKWRITE | VDKINFO,
						("[VDK] Using Granularity %u\n", write_length));
			}
		}
		else if (cur_file->FileType == VDK_FILETYPE_VMDK) {

			if (write_length > cur_file->prm.vmdk->SectorsPerGrain) {
				write_length = cur_file->prm.vmdk->SectorsPerGrain;

				VDKTRACE(VDKWRITE | VDKINFO,
						("[VDK] Using Granularity %u\n", write_length));
			}
		}

		write_offset = Offset % write_length;

		write_length -= write_offset;

		if (write_length > Length) {
			write_length = Length;
		}

		//
		// Locate actual offset of the target sector in the file
		//
		if (cur_file->FileType == VDK_FILETYPE_FLAT) {

			write_offset += cur_file->prm.SolidBackOffset;

		}
		else if (cur_file->FileType == VDK_FILETYPE_COWD) {

			ULONG primary_idx, secondary_idx;

			//
			// check secondary map allocation
			//
			primary_idx = Offset / cur_file->prm.cowd->PrimaryGranularity;

			if (primary_idx >= cur_file->prm.cowd->PrimaryMapSize) {

				//
				// offset exceeds the primary map size
				// Probably COWD header is corrupt and inconsistent
				//
				VDKTRACE(VDKWRITE,
					("[VDK] Primary Map overflow\n"));

				return VDK_INTERNAL;
			}

			if (!cur_file->prm.cowd->PrimaryMap[primary_idx] ||
				cur_file->prm.cowd->PrimaryMap[primary_idx]
				>= cur_file->EndOfFile) {

				//
				// Need to allocate new Secondary Map
				//

				VDKTRACE(VDKWRITE | VDKINFO,
					("[VDK] Allocating SecondaryMap #%u at offset 0x%x\n",
					primary_idx, cur_file->EndOfFile));

				status = VdkSetFileSize(
					cur_file->FileHandle,
					((INT64)cur_file->EndOfFile << VDK_BYTE_SHIFT_TO_SECTOR) +
					(cur_file->prm.cowd->SecondaryMapSize * sizeof(ULONG)));

				if (!VDKSUCCESS(status)) {
					return status;
				}

				//
				// Update PrimaryMap
				//
				cur_file->prm.cowd->PrimaryMap[primary_idx] =
					cur_file->EndOfFile;

				VDKTRACE(VDKWRITE | VDKINFO,
					("[VDK] Writing PrimaryMap at offset %lu (%lu bytes.)\n",
					((PCOWD_SECTOR_0)cur_file->prm.cowd->Sector0)->PrimaryMapOffset,
					cur_file->prm.cowd->PrimaryMapSize * sizeof(ULONG)));

				status = VdkWriteFileAt(
					cur_file->FileHandle,
					(INT64)((PCOWD_SECTOR_0)cur_file->prm.cowd->Sector0)->PrimaryMapOffset <<
					VDK_BYTE_SHIFT_TO_SECTOR,
					cur_file->prm.cowd->PrimaryMap,
					cur_file->prm.cowd->PrimaryMapSize * sizeof(ULONG),
					NULL);

				if (!VDKSUCCESS(status)) {
					return status;
				}

				//
				// Update COWD header
				//
				cur_file->EndOfFile +=
					(cur_file->prm.cowd->SecondaryMapSize * sizeof(ULONG))
					>> VDK_BYTE_SHIFT_TO_SECTOR;

				((PCOWD_SECTOR_0)cur_file->prm.cowd->Sector0)->EndOfFile = cur_file->EndOfFile;

				VDKTRACE(VDKWRITE | VDKINFO,
						("[VDK] Updating COWD header\n"));

				status = VdkWriteFileAt(
					cur_file->FileHandle,
					0,
					cur_file->prm.cowd->Sector0,
					VDK_BYTES_PER_SECTOR,
					NULL);

				if (!VDKSUCCESS(status)) {
					return status;
				}

				//
				// Initialize new SecondaryMap
				//
				VdkZeroMem(
					cur_file->prm.cowd->SecondaryMap,
					cur_file->prm.cowd->SecondaryMapSize * sizeof(ULONG));

				cur_file->prm.cowd->SecondaryMapIdx = primary_idx;

			}
			else if (cur_file->prm.cowd->SecondaryMapIdx != primary_idx) {

				//
				// Need to read secondary map from the file
				//

				VDKTRACE(VDKWRITE | VDKINFO,
					("[VDK] Reading Secondary Map #%u\n", primary_idx));

				status = VdkReadFileAt(
					cur_file->FileHandle,
					(INT64)cur_file->prm.cowd->PrimaryMap[primary_idx] <<
						VDK_BYTE_SHIFT_TO_SECTOR,
					cur_file->prm.cowd->SecondaryMap,
					cur_file->prm.cowd->SecondaryMapSize * sizeof(ULONG),
					NULL);

				if (!VDKSUCCESS(status)) {
					return status;
				}

				cur_file->prm.cowd->SecondaryMapIdx = primary_idx;
			}

			//
			// check sector allocation
			//
			secondary_idx =
				(Offset % cur_file->prm.cowd->PrimaryGranularity) /
				cur_file->prm.cowd->SecondaryGranularity;

			if (secondary_idx >= cur_file->prm.cowd->SecondaryMapSize) {

				//
				// offset exceeds the secondary map size
				// Probably COWD header is corrupt and inconsistent
				// Or I have to find out how to get non-fixed secondary map size.
				//
				VDKTRACE(VDKWRITE,
					("[VDK] Secondary Map overflow\n"));

				return VDK_INTERNAL;
			}

			if (!cur_file->prm.cowd->SecondaryMap[secondary_idx] ||
				cur_file->prm.cowd->SecondaryMap[secondary_idx]
				>= cur_file->EndOfFile) {

				//
				// Need to allocate new sector block
				//
				VDKTRACE(VDKWRITE | VDKINFO,
					("[VDK] Allocating SectorBlock at offset 0x%x\n",
					cur_file->EndOfFile));

				status = VdkSetFileSize(
					cur_file->FileHandle,
					(INT64)(cur_file->EndOfFile + cur_file->prm.cowd->SecondaryGranularity)
					<< VDK_BYTE_SHIFT_TO_SECTOR);

				if (!VDKSUCCESS(status)) {
					return status;
				}

				//
				//	 Copy existing blocks from underlying file
				//
				status = VdkCopyDataBlock(
					DiskInfo,
					cur_file,
					file_offset + Offset,
					cur_file->prm.cowd->SecondaryGranularity);

				if (!VDKSUCCESS(status)) {
					return status;
				}

				cur_file->prm.cowd->SecondaryMap[secondary_idx] =
					cur_file->EndOfFile;

				cur_file->EndOfFile +=
					cur_file->prm.cowd->SecondaryGranularity;

				//
				// Update SecondaryMap
				//

				status = VdkWriteFileAt(
					cur_file->FileHandle,
					(INT64)cur_file->prm.cowd->PrimaryMap[primary_idx] <<
					VDK_BYTE_SHIFT_TO_SECTOR,
					cur_file->prm.cowd->SecondaryMap,
					cur_file->prm.cowd->SecondaryMapSize * sizeof(ULONG),
					NULL);

				if (!VDKSUCCESS(status)) {
					return status;
				}

				//
				//	Update COWD header
				//
				((PCOWD_SECTOR_0)cur_file->prm.cowd->Sector0)->EndOfFile = cur_file->EndOfFile;


				VDKTRACE(VDKWRITE | VDKINFO,
					("[VDK] Updating COWD header\n"));

				status = VdkWriteFileAt(
					cur_file->FileHandle,
					0,
					cur_file->prm.cowd->Sector0,
					VDK_BYTES_PER_SECTOR,
					NULL);

				if (!VDKSUCCESS(status)) {
					return status;
				}
			}

			write_offset += cur_file->prm.cowd->SecondaryMap[secondary_idx];
		}
		else if (cur_file->FileType == VDK_FILETYPE_VMDK) {

			ULONG table_idx, grain_idx;

			//
			// get grain table index
			//
			table_idx = Offset / cur_file->prm.vmdk->SectorsPerTable;

			if (table_idx >= cur_file->prm.vmdk->DirectorySize 	||
				!cur_file->prm.vmdk->PrimaryDirectory[table_idx] 	||
				cur_file->prm.vmdk->PrimaryDirectory[table_idx]
				>= cur_file->EndOfFile) {

				//
				// Grain table is not present;
				// something's wrong with VMDK header
				//
				VDKTRACE(VDKWRITE,
					("[VDK] Cannot locate Grain Table %lu\n",
					table_idx));

				return VDK_INTERNAL;
			}


			if (cur_file->prm.vmdk->GrainTableIdx != table_idx) {

				//
				// Need to read the target grain table from the file
				//

				VDKTRACE(VDKWRITE | VDKINFO,
					("[VDK] Reading Grain Table #%u\n", table_idx));

				status = VdkReadFileAt(
					cur_file->FileHandle,
					(INT64)cur_file->prm.vmdk->PrimaryDirectory[table_idx] <<
					VDK_BYTE_SHIFT_TO_SECTOR,
					cur_file->prm.vmdk->GrainTable,
					cur_file->prm.vmdk->GrainTableSize * sizeof(ULONG),
					NULL);

				if (!VDKSUCCESS(status)) {
					return status;
				}

				cur_file->prm.vmdk->GrainTableIdx = table_idx;
			}

			//
			// check grain allocation
			//
			grain_idx =
				(Offset % cur_file->prm.vmdk->SectorsPerTable) /
				cur_file->prm.vmdk->SectorsPerGrain;

			if (grain_idx >= cur_file->prm.vmdk->GrainTableSize) {

				//
				// offset exceeds the secondary map size
				// Probably VMDK header is corrupt and inconsistent
				//
				VDKTRACE(VDKWRITE,
					("[VDK] Grain table overflow\n"));

				return VDK_INTERNAL;
			}

			if (!cur_file->prm.vmdk->GrainTable[grain_idx] ||
				cur_file->prm.vmdk->GrainTable[grain_idx]
				>= cur_file->EndOfFile) {

				//
				// Need to allocate new grain
				//

				VDKTRACE(VDKWRITE | VDKINFO,
					("[VDK] Allocating Grain at offset 0x%x\n",
					cur_file->EndOfFile));

				status = VdkSetFileSize(
					cur_file->FileHandle,
					(INT64)(cur_file->EndOfFile + cur_file->prm.vmdk->SectorsPerGrain)
					<< VDK_BYTE_SHIFT_TO_SECTOR);

				if (!VDKSUCCESS(status)) {
					return status;
				}

				//
				//	 Copy data from underlying file
				//
				status = VdkCopyDataBlock(
					DiskInfo,
					cur_file,
					file_offset + Offset,
					cur_file->prm.vmdk->SectorsPerGrain);

				if (!VDKSUCCESS(status)) {
					return status;
				}

				cur_file->prm.vmdk->GrainTable[grain_idx] =
					cur_file->EndOfFile;

				cur_file->EndOfFile +=
					cur_file->prm.vmdk->SectorsPerGrain;

				//
				// Update Primary GrainTable
				//

				status = VdkWriteFileAt(
					cur_file->FileHandle,
					(INT64)cur_file->prm.vmdk->PrimaryDirectory[table_idx] <<
					VDK_BYTE_SHIFT_TO_SECTOR,
					cur_file->prm.vmdk->GrainTable,
					cur_file->prm.vmdk->GrainTableSize * sizeof(ULONG),
					NULL);

				if (!VDKSUCCESS(status)) {
					return status;
				}

				//
				// Update Backup GrainTable
				//

				status = VdkWriteFileAt(
					cur_file->FileHandle,
					(INT64)cur_file->prm.vmdk->BackupDirectory[table_idx] <<
					VDK_BYTE_SHIFT_TO_SECTOR,
					cur_file->prm.vmdk->GrainTable,
					cur_file->prm.vmdk->GrainTableSize * sizeof(ULONG),
					NULL);

				if (!VDKSUCCESS(status)) {
					return status;
				}
			}

			write_offset += cur_file->prm.vmdk->GrainTable[grain_idx];
		}

		//
		// Adjust EndOfFile parameter
		//
		if (write_offset + write_length > cur_file->EndOfFile) {
			cur_file->EndOfFile = write_offset + write_length;
		}

		//
		// Now write
		//
		VDKTRACE(VDKWRITE | VDKINFO,
			("[VDK] Writing %u sectors at file #%u, byte offset 0x%"INT64_PRINT_FORMAT"x\n",
			write_length,
			cur_file - DiskInfo->Files,
			(INT64)write_offset << VDK_BYTE_SHIFT_TO_SECTOR));

		status = VdkWriteFileAt(
			cur_file->FileHandle,
			(INT64)write_offset << VDK_BYTE_SHIFT_TO_SECTOR,
			Buffer,
			write_length << VDK_BYTE_SHIFT_TO_SECTOR,
			NULL);

		if (!VDKSUCCESS(status)) {
			return status;
		}

		//
		// Prepare for the following data
		//
		Buffer	+= (write_length << VDK_BYTE_SHIFT_TO_SECTOR);
		Offset	+= write_length;
		Length	-= write_length;
	}

	//
	// Writing successfull
	//

	return VDK_OK;
}

//
// Copy data block from underlying files to current file
//
VDKSTAT VdkCopyDataBlock(
	PVDK_DISK_INFO	DiskInfo,		// file info structure to read from
	PVDK_FILE_INFO	CurFile,		// current file item to write to
	ULONG			Offset,			// logical offset of the block to copy
	ULONG			Sectors)		// number of sectors to copy
{
	PVOID		copy_buf;
	VDKSTAT		status;

	copy_buf = VdkAllocMem(Sectors << VDK_BYTE_SHIFT_TO_SECTOR);

	if (!copy_buf) {

		VDKTRACE(VDKWRITE,
			("[VDK] Failed to allocate copy buffer\n"));

		return VDK_NOMEMORY;
	}

	//
	//	Read underlying data
	//

	status = VdkReadSector(
		DiskInfo,
		Offset / Sectors * Sectors,
		Sectors,
		copy_buf);

	if (!VDKSUCCESS(status)) {
		goto cleanup_exit;
	}

	//
	// Write back new block data
	//

	status = VdkWriteFileAt(
		CurFile->FileHandle,
		(INT64)CurFile->EndOfFile << VDK_BYTE_SHIFT_TO_SECTOR,
		copy_buf,
		Sectors << VDK_BYTE_SHIFT_TO_SECTOR,
		NULL);

cleanup_exit:
	VdkFreeMem(copy_buf);

	return status;
}
