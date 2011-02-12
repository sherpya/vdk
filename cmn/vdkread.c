/*
	vdkread.c

	Virtual Disk read function
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkaccess.h"

#include "cowdisk.h"
#include "vmdisk.h"

//
// Read requested sectors from virtual disk
//

VDKSTAT	VdkReadSector(
	PVDK_DISK_INFO	DiskInfo,
	ULONG			Offset,
	ULONG			Length,
	PUCHAR			Buffer)
{
	PVDK_FILE_INFO	top_level_file;
	PVDK_FILE_INFO	no_more_file;
	ULONG			top_level_offset;
	VDKSTAT			status;


	VDKTRACE(VDKREAD | VDKINFO,
		("[VDK] VdkRead - Request Offset sec. %u, length %u\n",
		Offset, Length));


	if (DiskInfo == NULL || DiskInfo->Files == NULL) {

		VDKTRACE(VDKREAD,
			("[VDK] null DiskInfo\n"));

		return VDK_INTERNAL;
	}

	if (Offset + Length > DiskInfo->Capacity) {

		VDKTRACE(VDKREAD,
			("[VDK] request out of range\n"));

		return VDK_INTERNAL;
	}

	if (Buffer == NULL) {

		VDKTRACE(VDKREAD,
			("[VDK] null read buffer\n"));

		return VDK_INTERNAL;
	}

	top_level_file	= DiskInfo->Files;

	no_more_file	= &(DiskInfo->Files[DiskInfo->FilesTotal]);

	top_level_offset = Offset;

	//
	// Start reading data from file
	//
	while (Length) {

		PVDK_FILE_INFO	file_info;
		ULONG			sector_map_idx;
		ULONG			read_offset;
		ULONG			read_length;
		ULONG			actual_length;

		//
		//	look for a top level file containing the target sector
		//
		while (top_level_offset >= top_level_file->Capacity) {
			//
			// data is stored past this file
			//

			top_level_offset -= top_level_file->Capacity;
			top_level_file++;
		}

		VDKTRACE(VDKREAD | VDKINFO,
			("[VDK] Checking file #%u.\n",
			top_level_file - DiskInfo->Files));

		//
		// current file info pointer to work with
		//
		file_info = top_level_file;

		//
		// offset in a allocation block
		// The whole file is one block for plain files.
		//
		read_offset = top_level_offset;

		//
		// maximum data which could be read at once
		//
		read_length = file_info->Capacity;

		//
		// initial sector map index number
		//
		sector_map_idx = 0;

		//
		// Walk through the file generation chain to find a file
		// which actually contains target sector range
		//
		for (;;) {

			if (file_info->FileType == VDK_FILETYPE_FLAT) {
				//
				// This file contains the target sector
				//
				break;
			}
			else if (file_info->FileType == VDK_FILETYPE_COWD) {
				//
				// check for the smallest block size
				//
				if (read_length > file_info->prm.cowd->SecondaryGranularity) {

					read_length = file_info->prm.cowd->SecondaryGranularity;

					VDKTRACE(VDKREAD | VDKINFO,
						("[VDK] Using Granularity %u\n", read_length));
				}

				//
				// check secondary map allocation
				//
				sector_map_idx =
					read_offset / file_info->prm.cowd->PrimaryGranularity;

				if (sector_map_idx >= file_info->prm.cowd->PrimaryMapSize) {

					//
					// offset exceeds the primary map size
					// Probably COWD header is corrupt and inconsistent
					//
					VDKTRACE(VDKREAD | VDKWARN,
						("[VDK] Primary Map overflow\n"));

					file_info = NULL;
					break;
				}

				if (file_info->prm.cowd->PrimaryMap[sector_map_idx]) {
					//
					// Secondary map is allocated in the file
					//
					if (file_info->prm.cowd->SecondaryMapIdx != sector_map_idx) {

						//
						// Need to read secondary map from the file
						//
						VDKTRACE(VDKREAD | VDKINFO,
							("[VDK] Loading Secondary Map #%u\n",
							sector_map_idx));

						if (file_info->prm.cowd->PrimaryMap[sector_map_idx] >=
							file_info->EndOfFile) {

							//
							// Secondary Map Offset is too large
							//
							VDKTRACE(VDKREAD | VDKWARN,
								("[VDK] Invalid Secondary Map Offset 0x%x\n",
								file_info->prm.cowd->PrimaryMap[sector_map_idx]));

							VdkZeroMem(
								file_info->prm.cowd->SecondaryMap,
								file_info->prm.cowd->SecondaryMapSize * sizeof(ULONG));
						}
						else {

							status = VdkReadFileAt(
								file_info->FileHandle,
								(INT64)file_info->prm.cowd->PrimaryMap[sector_map_idx] <<
								VDK_BYTE_SHIFT_TO_SECTOR,
								file_info->prm.cowd->SecondaryMap,
								file_info->prm.cowd->SecondaryMapSize * sizeof(ULONG),
								NULL);

							if (!VDKSUCCESS(status)) {
								return status;
							}
						}

						file_info->prm.cowd->SecondaryMapIdx = sector_map_idx;
					}

					//
					// check the target sector allocation
					//
					sector_map_idx =
						(read_offset % file_info->prm.cowd->PrimaryGranularity) /
						file_info->prm.cowd->SecondaryGranularity;

					if (sector_map_idx >= file_info->prm.cowd->SecondaryMapSize) {

						//
						// offset exceeds the secondary map size
						// Probably COWD header is corrupt and inconsistent
						// Or I have to find out how to get non-fixed secondary map size.
						//
						VDKTRACE(VDKREAD | VDKWARN,
							("[VDK] Secondary Map overflow\n"));

						file_info = NULL;
						break;
					}

					if (file_info->prm.cowd->SecondaryMap[sector_map_idx] &&
						file_info->prm.cowd->SecondaryMap[sector_map_idx] < file_info->EndOfFile) {

						//
						// Start sector is allocated in this file
						//
						break;
					}
					else if (file_info->prm.cowd->SecondaryMap[sector_map_idx] >= file_info->EndOfFile) {

						//
						// sector block offset exceeds current end of file
						//
						VDKTRACE(VDKREAD | VDKWARN,
							("[VDK] Sector block offset too large\n"));

						file_info = NULL;
						break;
					}

					//
					//	sector block is not allocated
					//
				}
			}
			else if (file_info->FileType == VDK_FILETYPE_VMDK) {

				//
				// check for the smallest block size
				//
				if (read_length > file_info->prm.vmdk->SectorsPerGrain) {

					read_length = file_info->prm.vmdk->SectorsPerGrain;

					VDKTRACE(VDKREAD | VDKINFO,
						("[VDK] Using Granularity %u\n",
						read_length));
				}

				//
				// Get grain table index
				//
				sector_map_idx =
					read_offset / file_info->prm.vmdk->SectorsPerTable;

				if (sector_map_idx >= file_info->prm.vmdk->DirectorySize 	||
					!file_info->prm.vmdk->PrimaryDirectory[sector_map_idx] 	||
					file_info->prm.vmdk->PrimaryDirectory[sector_map_idx]
					>= file_info->EndOfFile) {

					//
					// Grain table is not present;
					// something's wrong with VMDK header
					//
					VDKTRACE(VDKREAD | VDKWARN,
						("[VDK] Cannot locate Grain Table %lu\n",
						sector_map_idx));

					file_info = NULL;
					break;
				}

				if (file_info->prm.vmdk->GrainTableIdx != sector_map_idx) {

					//
					// Need to read the grain table from the file
					//

					VDKTRACE(VDKREAD | VDKINFO,
						("[VDK] Loading Grain Table #%u\n",
						sector_map_idx));

					status = VdkReadFileAt(
						file_info->FileHandle,
						(INT64)file_info->prm.vmdk->PrimaryDirectory[sector_map_idx]
						<< VDK_BYTE_SHIFT_TO_SECTOR,
						file_info->prm.vmdk->GrainTable,
						file_info->prm.vmdk->GrainTableSize * sizeof(ULONG),
						NULL);

					if (!VDKSUCCESS(status)) {
						return status;
					}

					file_info->prm.vmdk->GrainTableIdx = sector_map_idx;
				}

				//
				// Find grain offset
				//
				sector_map_idx =
					(read_offset % file_info->prm.vmdk->SectorsPerTable)
					/ file_info->prm.vmdk->SectorsPerGrain;

				if (sector_map_idx >= file_info->prm.vmdk->GrainTableSize) {

					//
					// offset exceeds the grain table size
					// Probably VMDK header is corrupt and inconsistent
					//
					VDKTRACE(VDKREAD | VDKWARN,
						("[VDK] Grain Table overflow\n"));

					file_info = NULL;
					break;
				}

				if (file_info->prm.vmdk->GrainTable[sector_map_idx] &&
					file_info->prm.vmdk->GrainTable[sector_map_idx] < file_info->EndOfFile) {

					//
					// Target sector is allocated in this file
					//
					break;
				}
				else if (file_info->prm.vmdk->GrainTable[sector_map_idx] >= file_info->EndOfFile) {

					//
					// Grain offset exceeds current end of file
					//
					VDKTRACE(VDKREAD | VDKWARN,
						("[VDK] Grain offset too large\n"));

					file_info = NULL;
					break;
				}

				//
				//	Grain is not allocated
				//
			}

			//
			// Look for the first file of the next generation
			//
			while (++file_info < no_more_file) {

				if (file_info->StartSector == 0) {
					//
					// Found it
					//
					break;
				}
			}

			if (file_info == no_more_file) {
				//
				// No more generation
				//
				file_info = NULL;

				VDKTRACE(VDKREAD | VDKINFO,
					("[VDK] No more parent.\n"));

				break;
			}

			//
			// look for a file containing the target sector
			//
			read_offset = Offset;

			while (read_offset >= file_info->Capacity) {
				//
				// data is stored past this file
				//

				read_offset -= file_info->Capacity;
				file_info++;
			}

			VDKTRACE(VDKREAD | VDKINFO,
				("[VDK] Checking file #%u.\n",
				file_info - DiskInfo->Files));

		}	// for (;;)

		//
		// How many sectors can be read at once?
		//
		read_offset %= read_length;
		read_length -= read_offset;

		if (read_length > Length) {
			read_length = Length;
		}

		//
		// If the Target sector is not allocated on any file.
		// Pretend it to be filled with zero.
		//
		if (file_info == NULL ||
			file_info->FileType == VDK_FILETYPE_NONE) {


			VDKTRACE(VDKREAD | VDKINFO,
				("[VDK] Sector not present. %u sectors filled with Zero.\n",
				read_length));

			VdkZeroMem(Buffer,
				read_length << VDK_BYTE_SHIFT_TO_SECTOR);

			goto next_block;
		}

		//
		// Get the actual sector offset in the file
		//

		if (file_info->FileType == VDK_FILETYPE_FLAT) {

			read_offset += file_info->prm.SolidBackOffset;

		}
		if (file_info->FileType == VDK_FILETYPE_COWD) {

			read_offset +=
				file_info->prm.cowd->SecondaryMap[sector_map_idx];

		}
		else if (file_info->FileType == VDK_FILETYPE_VMDK) {

			read_offset +=
				file_info->prm.vmdk->GrainTable[sector_map_idx];

		}

		//
		// Check if whole sector range is acturally presend in this file
		//
		actual_length = read_length;

		if (read_offset + read_length > file_info->EndOfFile) {

			if (read_offset > file_info->EndOfFile) {
				//
				// Offset is too large
				//
				actual_length = 0;

				VDKTRACE(VDKREAD | VDKWARN,
					("[VDK] Invalid Offset 0x%lu\n",
					read_offset));
			}
			else {
				//
				// Length is too large. Data is clipped.
				//
				actual_length = file_info->EndOfFile - read_offset;

				VDKTRACE(VDKREAD | VDKWARN,
					("[VDK] Only %u sectors can be read\n",
					actual_length));
			}

			VdkZeroMem(
				Buffer + (actual_length << VDK_BYTE_SHIFT_TO_SECTOR),
				(read_length - actual_length) << VDK_BYTE_SHIFT_TO_SECTOR);
		}

		if (!actual_length) {
			goto next_block;
		}

		//
		//	Now read
		//

		VDKTRACE(VDKREAD | VDKINFO,
			("[VDK] Reading %u sectors from file #%u, byte offset 0x%"INT64_PRINT_FORMAT"x\n",
			actual_length,
			file_info - DiskInfo->Files,
			(INT64)read_offset << VDK_BYTE_SHIFT_TO_SECTOR));


		status = VdkReadFileAt(
			file_info->FileHandle,
			(INT64)read_offset << VDK_BYTE_SHIFT_TO_SECTOR,
			Buffer,
			actual_length << VDK_BYTE_SHIFT_TO_SECTOR,
			NULL);

		if (!VDKSUCCESS(status)) {
			return status;
		}

next_block:
		//
		// Prepare for the following data
		//
		Buffer	+= (read_length << VDK_BYTE_SHIFT_TO_SECTOR);
		Offset	+= read_length;
		Length	-= read_length;
		top_level_offset += read_length;
	}

	//
	// Reading successfull
	//

	return VDK_OK;
}

