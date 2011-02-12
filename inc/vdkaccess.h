/*
	vdkaccess.h

	Virtual Disk access function header
	Copyright (C) 2003 Ken Kato
*/

#ifndef	_VDK_ACCESS_
#define _VDK_ACCESS_

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

//
// Solid file specific parameter used for accessing solid files
//
typedef struct _VDK_SOLID_PARAM
{
	ULONG			BackOffset;				// Sectors to reserve at the top of file
}											// -- used by VMware 4 flat virtual disks
VDK_SOLID_PARAM, *PVDK_SOLID_PARAM;

//
//	COWD specific parameter used for accessing COWD files
//
typedef struct _VDK_COWD_PARAM
{
	PVOID			Sector0;				// COWD header

	PULONG			PrimaryMap;				// Primary Map buffer
	ULONG			PrimaryMapSize;			// Number of ULONG entries
	ULONG			PrimaryGranularity;		// Number of sectors per entry

	PULONG			SecondaryMap;			// Secondary Map buffer
	ULONG			SecondaryMapSize;		// Number of ULONG entries
	ULONG			SecondaryGranularity;	// Capacity per entry
	ULONG			SecondaryMapIdx;		// Current Secondary Map Index
}
VDK_COWD_PARAM, *PVDK_COWD_PARAM;

//
//	VMDK specific parameters used for accessing VMDK sparse files
//
typedef struct _VDK_VMDK_PARAM
{
	PULONG			PrimaryDirectory;		// Primary Directory data
	PULONG			BackupDirectory;		// Backup Directory data

	PULONG			GrainTable;				// MRU Grain Table data
	ULONG			GrainTableIdx;			// MRU Grain Table Index

	ULONG			DirectorySize;			// number of tables in directory
	ULONG			GrainTableSize;			// number of items in grain table
	ULONG			SectorsPerGrain;		// Granularity
	ULONG			SectorsPerTable;		// Granularity * GrainTableSize
}
VDK_VMDK_PARAM, *PVDK_VMDK_PARAM;

//
//	File information actually used for accessing Virtual Disk files
//
typedef struct _VDK_FILE_INFO
{
	ULONG			FileType;				// Segment file type
	ULONG			StartSector;			// Offset of the first sector
	ULONG			Capacity;				// Logical file length in sectors
	ULONG			NameLength;				// Name length in bytes
	HANDLE			FileHandle;				// File handle
	ULONG			EndOfFile;				// Actual file length in sectors

	//
	// type specific paramters
	//
	union tag_prm {
//		PVDK_SOLID_PARAM	solid;
		ULONG	SolidBackOffset;
		PVDK_COWD_PARAM		cowd;
		PVDK_VMDK_PARAM		vmdk;
	} prm;
}
VDK_FILE_INFO, *PVDK_FILE_INFO;

//
//	Structure represents a virtual disk
//
typedef struct _VDK_DISK_INFO
{
	ULONG			DiskType;				// RW / RO
	ULONG			Capacity;				// Total capacity in sectors
	ULONG			Cylinders;				// Cylinders
	ULONG			Tracks;					// Tracks per cylinder
	ULONG			Sectors;				// Sectors per track
	PCHAR			NameBuffer;				// Filename buffer
	ULONG			BufferLen;				// Filename buffer length
	PVDK_FILE_INFO	Files;					// array of file info
	ULONG			FilesTotal;				// number of files
}
VDK_DISK_INFO, *PVDK_DISK_INFO;

#define DISK_INFO_DEFINED
//
//	functions in vdkopen.c
//
#ifndef OPEN_FILE_INFO_DEFINED
typedef struct _VDK_OPEN_FILE_INFO *PVDK_OPEN_FILE_INFO;
#endif

VDKSTAT	VdkOpenCheckParam(
	PVDK_OPEN_FILE_INFO	OpenInfo,	// buffer containing the open param
	ULONG				InputLen);	// length of the buffer

VDKSTAT	VdkOpenDisk(
	PVDK_OPEN_FILE_INFO	OpenFile,	// open file info structure
	PVDK_DISK_INFO 		DiskInfo);	// disk info structure

VOID	VdkCloseDisk(
	PVDK_DISK_INFO		DiskInfo);	// file info structure

//
//	functions in vdkread.c
//
VDKSTAT	VdkReadSector(
	PVDK_DISK_INFO		DiskInfo,	// file info structure
	ULONG				Offset,		// logical offset of the sector to read
	ULONG				Sectors,	// number of sectors to read
	PUCHAR				Buffer);	// buffer to store the data being read

//
//	functions in vdkwrite.c
//
VDKSTAT	VdkWriteSector(
	PVDK_DISK_INFO		DiskInfo,	// file info structure
	ULONG				Offset,		// logical offset of the sector to write
	ULONG				Sectors,	// number of sectors to write
	PUCHAR				Buffer);	// buffer containing the data to write

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif	// _VDK_ACCESS_
