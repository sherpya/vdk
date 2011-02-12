/*
	imports.h

	Imported elements from various sources
	Copyright (C) 2003 Ken Kato

	This file contains:

	a) 	Stuff imported from newer DDKs so that the driver for all versions
		of Windows can be compiled with the Windows NT 4.0 DDK.

	b) 	Prototypes of standard functions which are exported from ntoskrnl.exe
		but not declared in regular DDK header files.
*/

#ifndef	_IMPORTS_H_
#define _IMPORTS_H_

#include <ntdddisk.h>

#if (VER_PRODUCTBUILD >= 2195)
#include <mountdev.h>
#endif

#if (VER_PRODUCTBUILD < 2195)
//
// Imports from Windows 2000 DDK
//

//
// from new <ntddk.h>
//
typedef enum _MM_PAGE_PRIORITY {
	LowPagePriority,
	NormalPagePriority = 16,
	HighPagePriority = 32
} MM_PAGE_PRIORITY;

#define FILE_ATTRIBUTE_ENCRYPTED			0x00004000

#define FILE_DEVICE_MASS_STORAGE			0x0000002d

//
//	from <ntddstor.h>
//
#define IOCTL_STORAGE_CHECK_VERIFY2			CTL_CODE(				\
												IOCTL_STORAGE_BASE, \
												0x0200,				\
												METHOD_BUFFERED,	\
												FILE_ANY_ACCESS)

#endif	// (VER_PRODUCTBUILD < 2195)

//
// Stuff from Windows XP DDK
//

#if (VER_PRODUCTBUILD < 2600)

#define IOCTL_DISK_GET_PARTITION_INFO_EX	CTL_CODE( 				\
												IOCTL_DISK_BASE, 	\
												0x0012, 			\
												METHOD_BUFFERED, 	\
												FILE_ANY_ACCESS)

#define IOCTL_DISK_GET_LENGTH_INFO			CTL_CODE( 				\
												IOCTL_DISK_BASE,	\
												0x0017,				\
												METHOD_BUFFERED, 	\
												FILE_READ_ACCESS)

typedef enum _PARTITION_STYLE {
	PARTITION_STYLE_MBR,
	PARTITION_STYLE_GPT
} PARTITION_STYLE;

typedef unsigned __int64 ULONG64, *PULONG64;

typedef struct _PARTITION_INFORMATION_MBR {
	UCHAR	PartitionType;
	BOOLEAN BootIndicator;
	BOOLEAN RecognizedPartition;
	ULONG	HiddenSectors;
} PARTITION_INFORMATION_MBR, *PPARTITION_INFORMATION_MBR;

typedef struct _PARTITION_INFORMATION_GPT {
	GUID	PartitionType;
	GUID	PartitionId;
	ULONG64 Attributes;
	WCHAR	Name[36];
} PARTITION_INFORMATION_GPT, *PPARTITION_INFORMATION_GPT;

typedef struct _PARTITION_INFORMATION_EX {
	PARTITION_STYLE PartitionStyle;
	LARGE_INTEGER	StartingOffset;
	LARGE_INTEGER	PartitionLength;
	ULONG			PartitionNumber;
	BOOLEAN 		RewritePartition;
	union {
		PARTITION_INFORMATION_MBR Mbr;
		PARTITION_INFORMATION_GPT Gpt;
	};
} PARTITION_INFORMATION_EX, *PPARTITION_INFORMATION_EX;

typedef struct _GET_LENGTH_INFORMATION {
	LARGE_INTEGER Length;
} GET_LENGTH_INFORMATION, *PGET_LENGTH_INFORMATION;

#endif // (VER_PRODUCTBUILD < 2600)

//
// Functions exported by ntoskrnl.exe
//
int swprintf(wchar_t *, const wchar_t *, ...);

#endif	// _IMPORTS_H_
