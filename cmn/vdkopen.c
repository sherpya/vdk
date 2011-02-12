/*
	vdkopen.c

	Virtual Disk open functions
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkfile.h"
#include "vdkaccess.h"

#include "cowdisk.h"
#include "vmdisk.h"

//
//	local functions
//
static VDKSTAT VdkSetCowdParam(
	PVDK_FILE_INFO	DiskInfo,
	ULONG			ReadOnly);

static VDKSTAT VdkSetVmdkParam(
	PVDK_FILE_INFO	DiskInfo,
	ULONG			ReadOnly);

//
//	declare pageable functions
//
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdkOpenCheckParam)
#pragma alloc_text(PAGE, VdkOpenDisk)
#pragma alloc_text(PAGE, VdkSetCowdParam)
#pragma alloc_text(PAGE, VdkSetVmdkParam)
#endif	// ALLOC_PRAGMA

//
//	Check open file info integrity
//
VDKSTAT VdkOpenCheckParam(
	PVDK_OPEN_FILE_INFO	OpenInfo,
	ULONG				InputLen)
{
	ULONG	fixed_size;
	ULONG	total_sectors;
	ULONG	total_name_len;
	ULONG	generation;
	ULONG	idx;

	//
	// check minimum length
	//
	if (InputLen < sizeof(VDK_OPEN_FILE_INFO)) {
		return VDK_BUFFER;
	}

	//
	// check DiskType
	//
	if (OpenInfo->DiskType != VDK_DISKTYPE_WRITABLE &&
		OpenInfo->DiskType != VDK_DISKTYPE_READONLY &&
		OpenInfo->DiskType != VDK_DISKTYPE_WRITEBLOCK) {

		VDKTRACE(VDKOPEN,
			("[VDK] IOCTL_VDK_OPEN_FILE: Invalid DiskType %u\n",
			OpenInfo->DiskType));

		return VDK_PARAM;
	}

	//
	// There must be at least one file to open
	//
	if (OpenInfo->FilesTotal == 0 || OpenInfo->Capacity == 0) {

		VDKTRACE(VDKOPEN,
			("[VDK] IOCTL_VDK_OPEN_FILE: disk size == 0\n"));

		return VDK_PARAM;
	}

	//
	// If geometry values are specified, they must be
	// consistent with the number of total sectors
	//
	if (OpenInfo->Cylinders *
		OpenInfo->Tracks *
		OpenInfo->Sectors > OpenInfo->Capacity) {

		VDKTRACE(VDKOPEN,
			("[VDK] IOCTL_VDK_OPEN_FILE: geometry / capacity conflict\n"));

		return VDK_PARAM;
	}

	//
	// Calculate the minimum necessary size for
	// the VDK_OEPN_DISK and all VDK_OPEN_FILE_ITEM structures.
	//
	fixed_size =
		FIELD_OFFSET(VDK_OPEN_FILE_INFO, Files) +
		(sizeof(VDK_OPEN_FILE_ITEM) * OpenInfo->FilesTotal);

	//
	// Recheck input parameter length
	// -- large enough to contain all fixed info?
	//
	if (InputLen < fixed_size) {

		return VDK_BUFFER;
	}

	//
	// Check each file info
	//
	total_sectors = 0;
	total_name_len = 0;
	generation = 0;
	idx = 0;

	do {
		PVDK_OPEN_FILE_ITEM open_file;

		//
		// file item to work with
		//
		open_file = &(OpenInfo->Files[idx]);

		//
		// check file type
		//
		if (open_file->FileType != VDK_FILETYPE_NONE &&
			open_file->FileType != VDK_FILETYPE_FLAT &&
			open_file->FileType != VDK_FILETYPE_COWD &&
			open_file->FileType != VDK_FILETYPE_VMDK) {

			VDKTRACE(VDKOPEN,
				("[VDK] IOCTL_VDK_OPEN_FILE: File #%u Invalid FileType %u\n",
				idx, open_file->FileType));

			return VDK_PARAM;
		}

		//
		// Top level files cannot be missing if disk is writable
		//
		if (!generation &&
			OpenInfo->DiskType == VDK_DISKTYPE_WRITABLE &&
			open_file->FileType == VDK_FILETYPE_NONE) {

			VDKTRACE(VDKOPEN,
				("[VDK] IOCTL_VDK_OPEN_FILE: Missing top level file #%u\n",
				idx));

			return VDK_PARAM;
		}

		//
		// check file length
		//
		if (open_file->Capacity == 0 ||
			open_file->Capacity > OpenInfo->Capacity) {

			VDKTRACE(VDKOPEN,
				("[VDK] IOCTL_VDK_OPEN_FILE: File #%u invalid size %u\n",
				idx, open_file->Capacity));

			return VDK_PARAM;
		}

		//
		// if the file is not missing, file name must be provided
		//
		if (open_file->FileType != VDK_FILETYPE_NONE &&
			open_file->NameLength == 0) {

			VDKTRACE(VDKOPEN,
				("[VDK] IOCTL_VDK_OPEN_FILE: File #%u filename required\n",
				idx));

			return VDK_PARAM;
		}

		//
		// BackOffset can only be used for solid files
		//
		if (open_file->FileType != VDK_FILETYPE_FLAT &&
			open_file->BackOffset != 0) {

			VDKTRACE(VDKOPEN,
				("[VDK] IOCTL_VDK_OPEN_FILE: File #%u BackOffset #%lu\n",
				idx, open_file->BackOffset));

			return VDK_PARAM;
		}

		//
		// Sum up the file length and name length
		//
		total_sectors	+= open_file->Capacity;
		total_name_len	+= open_file->NameLength;

		//
		// completed one generation?
		//
		if (total_sectors == OpenInfo->Capacity) {
			//
			// prepare for the next generation
			//
			generation++;
			total_sectors = 0;
		}
		else if (total_sectors > OpenInfo->Capacity) {

			VDKTRACE(VDKOPEN,
				("[VDK] IOCTL_VDK_OPEN_FILE: Size mismatch at File #%u\n",
				idx));

			return VDK_PARAM;
		}
	}
	while (++idx < OpenInfo->FilesTotal);

	//
	// The last generation is complete?
	//
	if (total_sectors) {
		VDKTRACE(VDKOPEN,
			("[VDK] IOCTL_VDK_OPEN_FILE: Size mismatch at File #%u\n",
			idx - 1));

		return VDK_PARAM;
	}

	//
	// now total name length is known, check buffer length again
	//
	if (InputLen < fixed_size + total_name_len) {

		return VDK_BUFFER;
	}

	return VDK_OK;
}

//
// Open image file(s)
//
VDKSTAT	VdkOpenDisk(
	PVDK_OPEN_FILE_INFO	OpenFile,
	PVDK_DISK_INFO 		DiskInfo)
{
	PCHAR	name_top;
	ULONG	name_total;
	ULONG	start_sector;
	ULONG	generation;
	ULONG	read_only;
	ULONG	idx;
	VDKSTAT	status = VDK_OK;

	//
	//	store disk information
	//
	VdkZeroMem(DiskInfo, sizeof(VDK_DISK_INFO));

	DiskInfo->DiskType		= OpenFile->DiskType;
	DiskInfo->Capacity		= OpenFile->Capacity;

	if (OpenFile->Cylinders && OpenFile->Tracks && OpenFile->Sectors) {

		DiskInfo->Cylinders	= OpenFile->Cylinders;
		DiskInfo->Tracks	= OpenFile->Tracks;
		DiskInfo->Sectors	= OpenFile->Sectors;
	}
	else {
		//
		// make up suitable values
		//
		DiskInfo->Sectors	= VDK_SECTORS_PER_TRACK;
		DiskInfo->Tracks	= VDK_TRACKS_PER_CYLINDER;
		DiskInfo->Cylinders	=
			OpenFile->Capacity / VDK_SECTORS_PER_TRACK / VDK_TRACKS_PER_CYLINDER;
	}

	DiskInfo->FilesTotal	= OpenFile->FilesTotal;

	//
	// allocate buffer to store file information
	//
	DiskInfo->Files = (PVDK_FILE_INFO)VdkAllocMem(
		sizeof(VDK_FILE_INFO) * OpenFile->FilesTotal);

	if (!DiskInfo->Files) {

		VDKTRACE(VDKOPEN,
			("[VDK] Can't allocate memory for disk info\n"));

		status = VDK_NOMEMORY;
		goto cleanup_exit;
	}

	VdkZeroMem(DiskInfo->Files,
		sizeof(VDK_FILE_INFO) * OpenFile->FilesTotal);


	//
	//	process each files
	//
	name_top = (PCHAR)OpenFile +
		FIELD_OFFSET(VDK_OPEN_FILE_INFO, Files) +
		(sizeof(VDK_OPEN_FILE_ITEM) * OpenFile->FilesTotal);

	name_total		= 0;
	start_sector	= 0;
	generation		= 0;
	read_only		= (OpenFile->DiskType != VDK_DISKTYPE_WRITABLE);

	for (idx = 0; idx < DiskInfo->FilesTotal; idx++) {

		PVDK_FILE_INFO	file_info = &DiskInfo->Files[idx];

		//
		// set up basic parameters
		//
		file_info->FileType		= OpenFile->Files[idx].FileType;
		file_info->Capacity		= OpenFile->Files[idx].Capacity;
		file_info->NameLength	= OpenFile->Files[idx].NameLength;
		file_info->StartSector	= start_sector;

		//
		// skip "Missing" files
		//
		if (file_info->FileType == VDK_FILETYPE_NONE) {
			goto next_file;
		}

		//
		//	open file
		//
		status = VdkOpenFile(
			&(file_info->FileHandle),
			name_top + name_total,
			file_info->NameLength,
			read_only);

		if (!VDKSUCCESS(status)) {
			goto cleanup_exit;
		}

		//
		//	check file attributes
		//
		status = VdkCheckAttribute(
			file_info->FileHandle);

		if (!VDKSUCCESS(status)) {
			goto cleanup_exit;
		}

		//
		//	get file size
		//
		{
			INT64 size;

			status = VdkGetFileSize(file_info->FileHandle, &size);

			if (!VDKSUCCESS(status)) {
				goto cleanup_exit;
			}

			file_info->EndOfFile = (ULONG)(size >> VDK_BYTE_SHIFT_TO_SECTOR);
		}

		//
		//	prepare type specific parameters
		//
		if (file_info->FileType == VDK_FILETYPE_FLAT) {

			file_info->prm.SolidBackOffset =
				OpenFile->Files[idx].BackOffset;

			VDKTRACE(VDKOPEN | VDKINFO,
				("[VDK] BackOffset    = %lu\n", file_info->prm.SolidBackOffset));

		}
		else if (file_info->FileType == VDK_FILETYPE_COWD) {

			status = VdkSetCowdParam(file_info, read_only);

			if (!VDKSUCCESS(status)) {
				goto cleanup_exit;
			}
		}
		else if (file_info->FileType == VDK_FILETYPE_VMDK) {

			status = VdkSetVmdkParam(file_info, read_only);

			if (!VDKSUCCESS(status)) {
				goto cleanup_exit;
			}
		}

next_file:
		VDKTRACE(VDKOPEN | VDKINFO,
			("[VDK] FileType    = %lu\n", file_info->FileType));

		VDKTRACE(VDKOPEN | VDKINFO,
			("[VDK] StartSector = %lu\n", file_info->StartSector));

		VDKTRACE(VDKOPEN | VDKINFO,
			("[VDK] Capacity    = %lu\n", file_info->Capacity));

		VDKTRACE(VDKOPEN | VDKINFO,
			("[VDK] EndOfFile   = %lu\n", file_info->EndOfFile));

		//
		// calculate next file's start sector
		//
		start_sector += file_info->Capacity;

		//
		// sum up name length
		//
		name_total += file_info->NameLength;

		//
		// completed one generation?
		//
		if (start_sector == OpenFile->Capacity) {
			start_sector = 0;
			generation++;
			read_only = TRUE;
		}
	}	// next file

	//
	// Store filenames
	//

	DiskInfo->NameBuffer = VdkAllocMem(name_total);

	if (!DiskInfo->NameBuffer) {

		VDKTRACE(VDKOPEN,
			("[VDK] Can't allocate memory for image path\n"));

		status = VDK_NOMEMORY;
		goto cleanup_exit;
	}

	DiskInfo->BufferLen = name_total;

	VdkCopyMem(DiskInfo->NameBuffer, name_top, name_total);

cleanup_exit:

	if (!VDKSUCCESS(status)) {
		VdkCloseDisk(DiskInfo);
	}

	return status;
}

//
// Prepare extra parameters for COWD
//
VDKSTAT VdkSetCowdParam(
	PVDK_FILE_INFO	FileInfo,
	ULONG			ReadOnly)
{
	PVDK_COWD_PARAM	cowd_prm = NULL;
	PCOWD_SECTOR_0	cowd_hdr = NULL;
	VDKSTAT			status = VDK_OK;

	//
	//	Allocate COWD parameter area
	//
	cowd_prm = (PVDK_COWD_PARAM)VdkAllocMem(
		sizeof(VDK_COWD_PARAM));

	if (!cowd_prm) {

		VDKTRACE(VDKOPEN,
			("[VDK] Failed to allocate cowd parameter\n"));

		return VDK_NOMEMORY;
	}

	VdkZeroMem(cowd_prm, sizeof(VDK_COWD_PARAM));

	FileInfo->prm.cowd = cowd_prm;

	//
	//	Allocate COWD header area
	//
	cowd_hdr = (PCOWD_SECTOR_0)VdkAllocMem(
		VDK_BYTES_PER_SECTOR);

	if (!cowd_hdr) {

		VDKTRACE(VDKOPEN,
			("[VDK] Failed to allocate cowd header buffer\n"));

		status = VDK_NOMEMORY;
		goto cleanup_exit;
	}

	//
	// Read COWD header
	//
	status = VdkReadFileAt(
		FileInfo->FileHandle,
		0,
		cowd_hdr,
		VDK_BYTES_PER_SECTOR,
		NULL);

	if (!VDKSUCCESS(status)) {
		goto cleanup_exit;
	}

	//
	// set up COWD parameters
	//
	cowd_prm->PrimaryMapSize =
		(((cowd_hdr->PrimaryMapSize * sizeof(ULONG)) +
		(VDK_BYTES_PER_SECTOR - 1)) & ~VDK_SECTOR_ALIGNMENT_MASK)
		/ sizeof(ULONG);

	cowd_prm->SecondaryMapSize = COWD_SECONDARY_MAP_SIZE;

	cowd_prm->PrimaryGranularity =
		cowd_hdr->Granularity * cowd_prm->SecondaryMapSize;

	cowd_prm->SecondaryGranularity =
		cowd_hdr->Granularity;

	VDKTRACE(VDKOPEN | VDKINFO,
		("[VDK] PrimaryMap size = %lu, granularity = %lu\n",
		cowd_prm->PrimaryMapSize, cowd_prm->PrimaryGranularity));

	VDKTRACE(VDKOPEN | VDKINFO,
		("[VDK] Secondary size = %lu, granularity = %lu\n",
		cowd_prm->SecondaryMapSize, cowd_prm->SecondaryGranularity));

	//
	// Check parameter logical consistency
	//
	if (cowd_prm->PrimaryMapSize * cowd_prm->PrimaryGranularity
		< FileInfo->Capacity) {

		//
		// Sector map cannot cover whole sectors in this file
		//
		VDKTRACE(VDKOPEN,
			("[VDK] map is too small\n"));

		status = VDK_PARAM;
		goto cleanup_exit;
	}

	if (cowd_hdr->PrimaryMapOffset < 4 ||
		cowd_hdr->PrimaryMapOffset >= FileInfo->EndOfFile) {

		//
		// PrimaryMapOffset is too large for this file.
		// Probably the file is corrupt.
		//
		VDKTRACE(VDKOPEN,
			("[VDK] Invalid PrimaryMapOffset %u\n",
			cowd_hdr->PrimaryMapOffset));

		status = VDK_PARAM;
		goto cleanup_exit;
	}

	//
	// Prepare PrimaryMap buffer
	//
	cowd_prm->PrimaryMap = (PULONG)VdkAllocMem(
		cowd_prm->PrimaryMapSize * sizeof(ULONG));

	if (!cowd_prm->PrimaryMap) {
		VDKTRACE(VDKOPEN,
			("[VDK] Can't allocate memory for primary map\n"));

		status = VDK_NOMEMORY;
		goto cleanup_exit;
	}

	//
	// Read primary map
	//
	status = VdkReadFileAt(
		FileInfo->FileHandle,
		(INT64)cowd_hdr->PrimaryMapOffset <<
		VDK_BYTE_SHIFT_TO_SECTOR,
		cowd_prm->PrimaryMap,
		cowd_prm->PrimaryMapSize * sizeof(ULONG),
		NULL);

	if (!VDKSUCCESS(status)) {
		goto cleanup_exit;
	}

	//
	// Prepare SecondaryMap buffer
	//
	cowd_prm->SecondaryMap = (PULONG)VdkAllocMem(
		cowd_prm->SecondaryMapSize * sizeof(ULONG));

	if (!cowd_prm->SecondaryMap) {

		VDKTRACE(VDKOPEN,
			("[VDK] Can't allocate memory for secondary map\n"));

		status = VDK_NOMEMORY;
		goto cleanup_exit;
	}

	//
	// Read secondary map
	//
	if (cowd_prm->PrimaryMap[0] <= cowd_hdr->PrimaryMapOffset ||
		cowd_prm->PrimaryMap[0] >= FileInfo->EndOfFile) {

		//
		// Secondary Map is not allocated or too large for this file.
		//
		VDKTRACE(VDKOPEN | VDKWARN,
			("[VDK] Invalid SecondaryMapOffset %u\n",
			cowd_prm->PrimaryMap[0]));

		VdkZeroMem(
			cowd_prm->SecondaryMap,
			cowd_prm->SecondaryMapSize * sizeof(ULONG));
	}
	else {
		status = VdkReadFileAt(
			FileInfo->FileHandle,
			(INT64)cowd_prm->PrimaryMap[0] <<
			VDK_BYTE_SHIFT_TO_SECTOR,
			cowd_prm->SecondaryMap,
			cowd_prm->SecondaryMapSize * sizeof(ULONG),
			NULL);

		if (!VDKSUCCESS(status)) {
			goto cleanup_exit;
		}
	}

cleanup_exit:
	if (cowd_hdr) {
		if (VDKSUCCESS(status) && !ReadOnly) {
			FileInfo->prm.cowd->Sector0 = cowd_hdr;
		}
		else {
			VdkFreeMem(cowd_hdr);
		}
	}

	return status;
}

//
// Prepare extra parameters for VMDK files
//
VDKSTAT VdkSetVmdkParam(
	PVDK_FILE_INFO	FileInfo,
	ULONG			ReadOnly)
{
	PVDK_VMDK_PARAM	vmdk_prm = NULL;
	PVMDK_HEADER	vmdk_hdr = NULL;
	PVOID			read_buf = NULL;
	ULONG			read_len;
	VDKSTAT			status = VDK_OK;

	//
	//	Allocate VMDK parameter area
	//
	vmdk_prm = (PVDK_VMDK_PARAM)VdkAllocMem(sizeof(VDK_VMDK_PARAM));

	if (!vmdk_prm) {

		VDKTRACE(VDKOPEN,
			("[VDK] Failed to allocate VMDK parameter\n"));

		return VDK_NOMEMORY;
	}

	VdkZeroMem(vmdk_prm, sizeof(VDK_VMDK_PARAM));

	FileInfo->prm.vmdk = vmdk_prm;

	//
	//	Allocate VMDK header area
	//
	vmdk_hdr = (PVMDK_HEADER)VdkAllocMem(VDK_BYTES_PER_SECTOR);

	if (!vmdk_hdr) {
		status = VDK_NOMEMORY;

		VDKTRACE(VDKOPEN,
			("[VDK] Failed to allocate VMDK header buffer\n"));

		goto cleanup_exit;
	}

	//
	// Read VMDK header
	//

	status = VdkReadFileAt(
		FileInfo->FileHandle,
		0,
		vmdk_hdr,
		VDK_BYTES_PER_SECTOR,
		NULL);

	if (!VDKSUCCESS(status)) {
		goto cleanup_exit;
	}

	//
	// Check parameter logical consistency
	//
	if (!vmdk_hdr->CapacityLow		||
		!vmdk_hdr->numGTEsPerGT		||
		!vmdk_hdr->GranularityLow	||
		!vmdk_hdr->rgdOffsetLow		||
		!vmdk_hdr->gdOffsetLow		||
		vmdk_hdr->CapacityHigh		||
		vmdk_hdr->GranularityHigh	||
		vmdk_hdr->rgdOffsetHigh		||
		vmdk_hdr->gdOffsetHigh		||
		vmdk_hdr->rgdOffsetLow >= FileInfo->EndOfFile ||
		vmdk_hdr->gdOffsetLow >= FileInfo->EndOfFile) {

		//
		// Directory Offset is too large for this file.
		// Probably the file is corrupt.
		//
		VDKTRACE(VDKOPEN,
			("[VDK] Invalid VMDK header param\n"));

		status = VDK_PARAM;
		goto cleanup_exit;
	}

	if (vmdk_hdr->CapacityLow < FileInfo->Capacity) {

		//
		// VMDK file cannot cover specified capacity
		//
		VDKTRACE(VDKOPEN,
			("[VDK] VMDK capacity mismatch\n"));

		status = VDK_PARAM;
		goto cleanup_exit;
	}

	//
	// set up VMDK parameters
	//
	vmdk_prm->GrainTableSize	= vmdk_hdr->numGTEsPerGT;

	vmdk_prm->SectorsPerGrain	= vmdk_hdr->GranularityLow;

	vmdk_prm->SectorsPerTable	=
		vmdk_prm->GrainTableSize * vmdk_prm->SectorsPerGrain;

	vmdk_prm->DirectorySize		=
		(vmdk_hdr->CapacityLow + vmdk_prm->SectorsPerTable - 1)
		/ vmdk_prm->SectorsPerTable;


	VDKTRACE(VDKOPEN | VDKINFO,
		("[VDK] Directory size = %lu\n",
		vmdk_prm->DirectorySize));

	VDKTRACE(VDKOPEN | VDKINFO,
		("[VDK] GrainTableSize = %lu\n",
		vmdk_prm->GrainTableSize));

	VDKTRACE(VDKOPEN | VDKINFO,
		("[VDK] SectorsPerTable = %lu\n",
		vmdk_prm->SectorsPerTable));

	VDKTRACE(VDKOPEN | VDKINFO,
		("[VDK] SectorsPerGrain = %lu\n",
		vmdk_prm->SectorsPerGrain));

	//
	//	Prepare Directory read buffer
	//
	read_len = (vmdk_prm->DirectorySize * sizeof(ULONG) + VDK_BYTES_PER_SECTOR - 1)
		& ~VDK_SECTOR_ALIGNMENT_MASK;

	read_buf = VdkAllocMem(read_len);

	if (!read_buf) {

		VDKTRACE(VDKOPEN,
			("[VDK] Can't allocate memory for read buffer\n"));

		status = VDK_NOMEMORY;
		goto cleanup_exit;
	}

	//
	// Prepare Primary Directory buffer
	//
	vmdk_prm->PrimaryDirectory = (PULONG)VdkAllocMem(
		vmdk_prm->DirectorySize * sizeof(ULONG));

	if (!vmdk_prm->PrimaryDirectory) {

		VDKTRACE(VDKOPEN,
			("[VDK] Can't allocate memory for PrimaryDirectory\n"));

		status = VDK_NOMEMORY;
		goto cleanup_exit;
	}

	//
	// Read Primary Directory
	//

	VDKTRACE(VDKOPEN | VDKINFO,
		("[VDK] Reading Primary Directory at offset 0x%08x\n",
		vmdk_hdr->rgdOffsetLow));

	status = VdkReadFileAt(
		FileInfo->FileHandle,
		(INT64)vmdk_hdr->rgdOffsetLow << VDK_BYTE_SHIFT_TO_SECTOR,
		read_buf,
		read_len,
		NULL);

	if (!VDKSUCCESS(status)) {
		goto cleanup_exit;
	}

	VdkCopyMem(vmdk_prm->PrimaryDirectory, read_buf,
		vmdk_prm->DirectorySize * sizeof(ULONG));


	if (!ReadOnly) {
		//
		//	Prepare Backup Directory buffer
		//
		vmdk_prm->BackupDirectory = (PULONG)VdkAllocMem(
			vmdk_prm->DirectorySize * sizeof(ULONG));

		if (!vmdk_prm->BackupDirectory) {

			VDKTRACE(VDKOPEN,
				("[VDK] Can't allocate memory for BackupDirectory\n"));

			status = VDK_NOMEMORY;
			goto cleanup_exit;
		}

		//
		// Read Backup Directory
		//

		VDKTRACE(VDKOPEN | VDKINFO,
			("[VDK] Reading Backup Directory at offset 0x%08x\n",
			vmdk_hdr->gdOffsetLow));

		status = VdkReadFileAt(
			FileInfo->FileHandle,
			(INT64)vmdk_hdr->gdOffsetLow << VDK_BYTE_SHIFT_TO_SECTOR,
			read_buf,
			read_len,
			NULL);

		if (!VDKSUCCESS(status)) {
			goto cleanup_exit;
		}

		VdkCopyMem(vmdk_prm->BackupDirectory, read_buf,
			vmdk_prm->DirectorySize * sizeof(ULONG));
	}

	//
	// Prepare Grain Table buffer
	//
	vmdk_prm->GrainTable = (PULONG)VdkAllocMem(
		vmdk_prm->GrainTableSize * sizeof(ULONG));

	if (!vmdk_prm->GrainTable) {

		VDKTRACE(VDKOPEN,
			("[VDK] Can't allocate memory for Grain Table\n"));

		status = VDK_NOMEMORY;
		goto cleanup_exit;
	}

	//
	// Read Grain Table
	//
	if (vmdk_prm->PrimaryDirectory[0] <= vmdk_hdr->rgdOffsetLow ||
		vmdk_prm->PrimaryDirectory[0] >= FileInfo->EndOfFile) {
		//
		// Secondary Map is not allocated or too large for this file.
		//
		VDKTRACE(VDKOPEN,
			("[VDK] Invalid Grain Table[0] Offset %u\n",
			vmdk_prm->PrimaryDirectory[0]));

		VdkZeroMem(
			vmdk_prm->GrainTable,
			vmdk_prm->GrainTableSize * sizeof(ULONG));
	}
	else {

		VDKTRACE(VDKOPEN | VDKINFO,
			("[VDK] Reading Grain Table[0] at offset 0x%08x\n",
			vmdk_prm->PrimaryDirectory[0]));

		status = VdkReadFileAt(
			FileInfo->FileHandle,
			(INT64)vmdk_prm->PrimaryDirectory[0] <<
			VDK_BYTE_SHIFT_TO_SECTOR,
			vmdk_prm->GrainTable,
			vmdk_prm->GrainTableSize * sizeof(ULONG),
			NULL);

		if (!VDKSUCCESS(status)) {
			goto cleanup_exit;
		}
	}

cleanup_exit:

	if (vmdk_hdr) {
		VdkFreeMem(vmdk_hdr);
	}

	if (read_buf) {
		VdkFreeMem(read_buf);
	}

	return status;
}

