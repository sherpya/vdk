/*
	vdkpart.h

	VDK partition management header
	Copyright (C) 2003 Ken Kato
*/

#ifndef _VDKPART_H_
#define _VDKPART_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PART_NONE			0x00
#define PART_DOS_FAT12		0x01
#define PART_DOS_FAT16		0x04
#define PART_DOS_EXT		0x05
#define PART_DOS_HUGE		0x06
#define PART_NTFS_HPFS		0x07
#define PART_DOS_FAT32		0x0b
#define PART_DOS_FAT32X		0x0c
#define PART_DOS_FAT16X		0X0e
#define PART_DOS_EXTX		0x0f
#define PART_LINUX_MINIX	0x81
#define PART_LINUX_SWAP		0x82
#define PART_LINUX			0x83
#define PART_LINUX_EXT		0x85

#define IS_EXTENDED(x)		((x) == PART_DOS_EXT || (x) == PART_DOS_EXTX || (x) == PART_LINUX_EXT)
#define IS_DOS_PART(x) 		((x) == PART_DOS_FAT12 || (x) == PART_DOS_FAT16 || (x) == PART_DOS_HUGE)
#define HAS_DOS_LABEL(x)	(IS_DOS_PART(x) || (x) == PART_NTFS_HPFS || (x) == PART_DOS_FAT32 || (x) == PART_DOS_FAT32X || (x) == PART_DOS_FAT16X)

#define ACTIVE_FLAG			0x80
#define SIGNATURE_LOW		0x55
#define SIGNATURE_HIGH		0xaa
#define SIGNATURE_WORD		0xaa55

#define REISERFS_SUPER_MAGIC	"ReIsErFs"
#define REISER2FS_SUPER_MAGIC	"ReIsEr2Fs"
#define REISERFS_SUPER_OFFSET	128			// (64 * 1024) bytes

#define EXT2_LABEL_LENGTH		16
#define EXT2_SUPER_MAGIC		0xef53
#define EXT3_HAS_JOURNAL		0x0004

#define XFS_LABEL_LENGTH		12
#define XFS_SUPER_MAGIC			"XFSB"


#ifdef _MSC_VER
#pragma pack(1)
#endif

typedef struct _PARTITION_ENTRY {
	UCHAR boot;					/* 0x80 - active		*/
	UCHAR start_head;			/* starting head		*/
	UCHAR start_sec;		   	/* starting sector		*/
	UCHAR start_cyl;		 	/* starting cylinder	*/
	UCHAR type; 		 		/* partition type		*/
	UCHAR end_head; 			/* end head				*/
	UCHAR end_sec;		 		/* end sector			*/
	UCHAR end_cyl;				/* end cylinder			*/
	ULONG lba_start;			/* starting sector		*/
	ULONG lba_length;			/* partition length		*/
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
PARTITION_ENTRY, *PPARTITION_ENTRY;

typedef union _PARTITION_TABLE {
	UCHAR buf[VDK_BYTES_PER_SECTOR];
	struct {
		UCHAR			filler[0x1be];
		PARTITION_ENTRY	partition[4];
		USHORT			signature;
	} p;
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
PARTITION_TABLE, *PPARTITION_TABLE;

typedef struct _FAT16_BPB {		// BIOS parameter block for FAT16
	USHORT	BytesPerSector;
	UCHAR	SectorsPerCluster;
	USHORT	ReservedSectors;
	UCHAR	NumberOfFATs;
	USHORT	RootEntries;
	USHORT	SmallSectors;
	UCHAR	MediaDescriptor;
	USHORT	SectorsPerFAT;
	USHORT	SectorsPerTrack;
	USHORT	NumberofHeads;
	ULONG	HiddenSectors;
	ULONG	LargeSectors;
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
FAT16_BPB, *PFAT16_BPB;

typedef struct _FAT16_EXBPB {	// Extended BIOS parameter block for FAT16
	UCHAR	PhysicalDriveNumber;
	UCHAR	Reserved;
	UCHAR	ExtendedBootSignature;
	ULONG	VolumeSerialNumber;
	CHAR	VolumeLabel[11];
	CHAR	FileSystemType[8];
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
FAT16_EXBPB, *PFAT16_EXBPB;

typedef struct _FAT16_PBR {		// Partition Boot Record
	UCHAR 		jump[3];	// Jump Instruction (E9 or EB, xx, xx)
	CHAR 		oemid[8];	// OEM ID (OS type)
	FAT16_BPB	bpb;
	FAT16_EXBPB	exbpb;
	UCHAR 		code[448];	// Bootstrap Code
	USHORT 		signature;	// End of Sector Marker
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
FAT16_PBR, *PFAT16_PBR;

typedef struct _FAT32_BPB {		// BIOS Parameter Block
	USHORT	BytesPerSector;
	UCHAR	SectorsPerCluster;
	USHORT	ReservedSectors;
	UCHAR	NumberOfFATs;
	USHORT	RootEntries;		// (FAT12/FAT16 only).
	USHORT	SmallSectors;		// (FAT12/FAT16 only).
	UCHAR	MediaDescriptor;
	USHORT	SectorsPerFAT;		// (FAT12/FAT16 only).
	USHORT	SectorsPerTrack;
	USHORT	NumberOfHeads;
	ULONG	HiddenSectors;
	ULONG	LargeSectors;
	ULONG	SectorsPerFAT32;	// (FAT32 only)
	USHORT	ExtendedFlags;		// (FAT32 only)
	USHORT	FileSystemVersion;	// (FAT32 only)
	ULONG	RootClusterNumber;	// (FAT32 only)
	USHORT	FileSystemInformationSectorNumber;	// (FAT32 only)
	USHORT	BackupBootSector;	// (FAT32 only)
	UCHAR 	Reserved[12];		// (FAT32 only)
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
FAT32_BPB, *PFAT32_BPB;

typedef struct _FAT32_EXBPB {
	UCHAR	PhysicalDriveNumber;
	UCHAR	Reserved;
	UCHAR	ExtendedBootSignature;
	ULONG	VolumeSerialNumber;
	CHAR	VolumeLabel[11];
	CHAR	SystemID[8];
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
FAT32_EXBPB, *PFAT32_EXBPB;

typedef struct _FAT32_PBR {		// Partition Boot Record
	UCHAR 		jump[3];	// Jump Instruction
	CHAR		oemid[8];	// OEM ID (OS type)
	FAT32_BPB	bpb;
	FAT32_EXBPB	exbpb;
	UCHAR 		code[420];	// Bootstrap Code
	USHORT 		signature;	// End of Sector Marker
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
FAT32_PBR, *PFAT32_PBR;

typedef struct _NTFS_BPB {
	USHORT	BytesPerSector;
	UCHAR	SectorsPerCluster;
	USHORT	ReservedSectors;
	UCHAR	NumberOfFATs;		// always 0
	USHORT	RootEntries;		// always 0
	USHORT	SmallSectors;		// not used by NTFS
	UCHAR	MediaDescriptor;
	USHORT 	SectorsPerFAT;		// always 0
	USHORT	SectorsPerTrack;
	USHORT	NumberOfHeads;
	ULONG	HiddenSectors;
	ULONG	LargeSectors;		//	not used by NTFS
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
NTFS_BPB, *PNTFS_BPB;

typedef struct _NTFS_EXBPB {	// Extended BIOS parameter block for FAT16
	ULONG 	Reserved;		// not used by NTFS
	ULONG	TotalSectorsLow;
	ULONG	TotalSectorsHigh;
	ULONG	MFTLow;
	ULONG	MFTHigh;
	ULONG	MFTMirrLow;
	ULONG	MFTMirrHigh;
	ULONG	ClustersPerFileRecordSegment;
	ULONG	ClustersPerIndexBlock;
	ULONG	VolumeSerialNumberLow;
	ULONG	VolumeSerialNumberHigh;
	ULONG	Checksum;
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
NTFS_EXBPB, *PNTFS_EXBPB;

typedef struct _NTFS_PBR {		// Partition Boot Record
	UCHAR 		jump[3];	// Jump Instruction
	CHAR		oemid[8];	// OEM ID (OS type)
	NTFS_BPB	bpb;
	NTFS_EXBPB	exbpb;
	UCHAR 		code[426];	// Bootstrap Code
	USHORT 		signature;	// End of Sector Marker
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
NTFS_PBR, *PNTFS_PBR;

typedef struct _ext2_super_block {
	CHAR		s_dummy0[56];
	USHORT		s_magic;
	CHAR		s_dummy1[34];
	ULONG		s_feature;
	CHAR		s_dummy2[24];
	CHAR		s_volume_name[EXT2_LABEL_LENGTH];
	CHAR		s_last_mounted[64];
	CHAR		s_dummy3[824];
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
EXT2_SB, *PEXT2_SB;

typedef struct _reiserfs_super_block {
	CHAR		s_dummy0[ 52];
	CHAR		s_magic [ 12];
	CHAR		s_dummy1[140];
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
REISER_SB, *PREISER_SB;

typedef struct _xfs_super_block {
	CHAR		s_magic[4];
	UCHAR		s_dummy0[104];
	CHAR		s_fname[XFS_LABEL_LENGTH];
	UCHAR		s_dummy1[904];
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
XFS_SB, *PXFS_SB;

#ifdef _MSC_VER
#pragma pack()
#endif

#define MIN_FAT16_VOLUME	32680
#define MAX_FSNAME_LEN		8
#define MAX_LABEL_LEN		20

typedef struct _PARTITION_ITEM {
	ULONG	idx;			// 1 based partition index
	ULONG	num;			// Linux style partition number
	ULONG	type;			// Partition type
	ULONG	offset;			// Starting offset
	ULONG	length;			// Partition length
	CHAR	fsname[MAX_FSNAME_LEN + 1];
	CHAR	label[MAX_LABEL_LEN + 1];
}
PARTITION_ITEM, *PPARTITION_ITEM;

#ifndef DISK_INFO_DEFINED
typedef struct _VDK_DISK_INFO *PVDK_DISK_INFO;
#endif

UCHAR VdkIdentifyFAT(
	PFAT16_PBR		Pbr,
	PPARTITION_ITEM PartItem);

UCHAR VdkIdentifyXFS(
	PXFS_SB			Xfsb,
	PPARTITION_ITEM	PartItem);

UCHAR VdkIdentifyEXT2(
	PEXT2_SB 		e2fsb,
	PPARTITION_ITEM	PartItem);

UCHAR VdkIdentifyRFS(
	PREISER_SB		rfsb,
	PPARTITION_ITEM	PartItem);

UCHAR VdkIdentifyFS(
	PVDK_DISK_INFO	DiskInfo,
	HANDLE			hFile,
	PPARTITION_ITEM	PartItem);

typedef void(*PLIST_CALLBACK)(PPARTITION_ITEM, PVOID);

VDKSTAT VdkListPartitions(
	PVDK_DISK_INFO	DiskInfo,
	HANDLE			hFile,
	ULONG			Capacity,
	PLIST_CALLBACK	CallBack,
	PVOID			Param);

const PCHAR GetPartitionTypeName(ULONG type);

BOOL IsPartitionMountable(ULONG type, BOOL read_only);

#ifdef __cplusplus
}
#endif

#endif	// _VDKPART_H_
