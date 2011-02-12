/*
	vdkfile.h

	VDK open file structure
	Copyright (C) 2003 Ken Kato
*/

#ifndef _VDK_FILE_H_
#define _VDK_FILE_H_

#ifdef _MSC_VER
#pragma pack(4)
#endif

//
// Image file information structure used for opening files
// corresponds to each file composing a virtual disk
//
typedef struct _VDK_OPEN_FILE_ITEM
{
	ULONG				FileType;
	ULONG				Capacity;	// capacity in 512 byte sectors
	ULONG				BackOffset;
	ULONG				NameLength;
}
#ifdef __GNUC__
__attribute__ ((aligned(4),packed))
#endif
VDK_OPEN_FILE_ITEM, *PVDK_OPEN_FILE_ITEM;

//
// Virtual Disk information structure used for opening files
// Represents a virtual disk composed of one or more image files.
//
typedef struct _VDK_OPEN_FILE_INFO
{
	ULONG				DiskType;
	ULONG				Capacity;	// capacity in 512 byte sectors
	ULONG				Cylinders;
	ULONG				Tracks;
	ULONG				Sectors;
	ULONG				FilesTotal;
	VDK_OPEN_FILE_ITEM	Files[1];
}
#ifdef __GNUC__
__attribute__ ((aligned(4),packed))
#endif
VDK_OPEN_FILE_INFO, *PVDK_OPEN_FILE_INFO;

#define OPEN_FILE_INFO_DEFINED

#ifdef _MSC_VER
#pragma pack()
#endif

#endif	// _VDK_FILE_H_
