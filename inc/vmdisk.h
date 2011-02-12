/*
	vmdisk.h

	Constants and structres for VMware 4.x Virtual Disk (VMDK)
	Copyright (C) 2003 Ken Kato

	!!! CAUTION !!!
	Some of the information in this file may not be quite accurate.
	It is the result of my entirely personal research and absolutely no
	official information is provided by VMware,Inc. or any other sources.
*/

#ifndef _VMDISK_H_
#define _VMDISK_H_

//
//	Default geometry values
//
#define VMDK_SECTORS_IDE			63			// for IDE disks of any size
#define VMDK_TRACKS_IDE				16			// for IDE disks of any size

#define VMDK_SECTORS_SCSI_1023M		32			// for SCSI disks <= 1023MB
#define VMDK_TRACKS_SCSI_1023M		64			// for SCSI disks <= 1023MB

#define VMDK_SECTORS_SCSI_2046M		32			// for SCSI disks <= 2046MB
#define VMDK_TRACKS_SCSI_2046M		128			// for SCSI disks <= 2046MB

#define VMDK_SECTORS_SCSI_LARGE		63			// for SCSI disks > 2046MB
#define VMDK_TRACKS_SCSI_LARGE		255			// for SCSI disks > 2046MB

//
//	Maximum extent size for VMware 4 split virtual disks
//
#define VMDK_MAX_EXTENT_SPARSE		4192256L
#define VMDK_MAX_EXTENT_SOLID		4193792L

//
//	Virtual Hardware version
//
#define VMDK_HARDWARE_VMWARE4		3

//
//	Header values
//

//
//	VMDK signature
//
#define VMDK_SIGNATURE				0x564d444b	// 'KDMV'

//
//	VMDK file version
//
#define VMDK_FILEVER_VMWARE4		1

//
//	VMDK flags
//
#define VMDK_FLAG_UNKNOWN1			0x01
#define VMDK_FLAG_UNKNOWN2			0x02

//
//	Default mapping parameters
//
#define VMDK_DEFAULT_GRANULARITY	128
#define VMDK_DEFAULT_NUMGTESPERGT	512
#define VMDK_DEFAULT_DESCOFFSET		1
#define VMDK_DEFAULT_DESCSIZE		20

//
//	VMDK header check bytes
//
#define VMDK_HEADER_CHECKBYTES		"\x0a\x20\x0d\x0a"

//
//	VMDK header structrure
//
#ifdef _MSC_VER
#pragma pack(4)
#endif

typedef struct _VMDK_HEADER {		// offset
	ULONG	Signature;				// 0x00 - 0x03
									//		signature

	ULONG	FileVersion;			// 0x04 - 0x07
									//		VMDK header version

	ULONG	Flags;					// 0x08 - 0x0b
									//		0x01 ?
									//		0x02 ?

	ULONG	CapacityLow;			// 0x0c - 0x13
	LONG	CapacityHigh;			//		File capacity in sectors

	ULONG	GranularityLow;			// 0x14 - 0x1b
	LONG	GranularityHigh;		//		number of sectors per GTE

	ULONG	DescOffsetLow;			// 0x1c - 0x23
	LONG	DescOffsetHigh;			//		offset of descriptor in sectors
									//		0 for multi-extent disk

	ULONG	DescSizeLow;			// 0x24 - 0x2b
	LONG	DescSizeHigh;			//		length of descriptor in sectors
									//		0 for multi-extent disk

	LONG	numGTEsPerGT;			// 0x2c - 0x2f
									//		Grain Table Entries per Grain
									//		Table

	ULONG	rgdOffsetLow;			// 0x30 - 0x37
	LONG	rgdOffsetHigh;			//		Primary Grain Directory Offset
									//		What does the 'r' stand for?

	ULONG	gdOffsetLow;			// 0x38 - 0x3f
	LONG	gdOffsetHigh;			//		Backup Grain Directory Offset

	ULONG	GrainOffsetLow;			// 0x40 - 0x47
	LONG	GrainOffsetHigh;		//		Offset of the first grain

	CHAR	Filler1[1];				// 0x48
									//		Unused

	CHAR	CheckBytes[4];			// 0x49 - 0x4c
									//		Used to check if the file was
									//		not transfered in text mode.
}
#ifdef __GNUC__
__attribute__ ((aligned(4),packed))
#endif
VMDK_HEADER, *PVMDK_HEADER;

#ifdef _MSC_VER
#pragma pack()
#endif

#endif	// _VMDISK_H_
