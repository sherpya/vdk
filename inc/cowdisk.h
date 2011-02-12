/*
	cowdisk.h

	Constants and structres for VMware 2.x/3.x Virtual Disk (COWdisk)
	Copyright (C) 2003 Ken Kato

	!!! CAUTION !!!
	Some of the information in this file may not be quite accurate.
	It is the result of my entirely personal research and absolutely no
	official information is provided by VMware,Inc. or any other sources.
*/

#ifndef _COWDISK_H_
#define _COWDISK_H_

//
//	Number of blocks per secondary map
//	-- doesn't seem to be stored in any header field
//
#define COWD_SECONDARY_MAP_SIZE		512

#define COWD1_DEFAULT_GRANULARITY	32
#define COWD3_DEFAULT_GRANULARITY	128
#define COWD_PRIMARY_MAP_OFFSET		4

//
//	VMware 2/3 virtual disk geometry values
//
#define COWD_SECTORS_IDE			63			// for IDE disks of any size
#define COWD1_TRACKS_IDE			15			// for VMware 2 IDE virtual disks
#define COWD3_TRACKS_IDE			16			// for VMware 3 IDE virtual disks

#define COWD_SECTORS_SCSI_1023M		32			// for SCSI disks <= 1023MB
#define COWD_TRACKS_SCSI_1023M		64			// for SCSI disks <= 1023MB

#define COWD_SECTORS_SCSI_2046M		32			// for SCSI disks <= 2046MB
#define COWD_TRACKS_SCSI_2046M		128			// for SCSI disks <= 2046MB

#define COWD_SECTORS_SCSI_LARGE		63			// for SCSI disks > 2046MB
#define COWD_TRACKS_SCSI_LARGE		255			// for SCSI disks > 2046MB

//
//	Maximum extent size for VMware2 virutal disks
//
#define COWD1_MAX_EXTENT_IDE		(4436 * COWD1_TRACKS_IDE * COWD_SECTORS_IDE)
#define COWD1_MAX_EXTENT_SCSI		(1023 * COWD_TRACKS_SCSI_2046M * COWD_SECTORS_SCSI_2046M)

//
//	Maximum extent size for VMware 3 virutal disks
//
#define COWD3_MAX_EXTENT_SPARSE		(1024 * 1024 / 512 * 2047)

//
//	Following are actually for plain disks and not for COWdisks
//	but included here for convenience
//
#define COWD1_SECTORS_IDE_PLAIN		COWD_SECTORS_IDE
#define COWD1_TRACKS_IDE_PLAIN		COWD3_TRACKS_IDE
#define COWD1_MAX_EXTENT_PLAIN		(1024 * 1024 / 512 * 2046)


///////////////////////////////////////
//
//	Virtual Disk Header values
//

//
//	Virtual disk signature
//
#define COWD_SIGNATURE				0x44574f43	// 'COWD'

//
//	VMware virtual file version
//
#define COWD_FILEVER_VMWARE2		1
#define COWD_FILEVER_VMWARE3		3

//
//	Virtual disk flags
//
#define COWD_FLAG_ROOT				0x00000001	// Indicates that the file does not have
												// a parent disk.

#define COWD_FLAG_CHECKED			0x00000002	// Indicates that the file was checked
												// and was clean at the previous close.

#define COWD_FLAG_MULTI				0x00000008	// Indicates that the vmdk file is
												// part of a multi-file virtual disk.

#define COWD_FLAG_HW_VER			0x00000010	// (v3) Indicates that Hardware version
												// has been decided for this virtual disk

#define COWD_VALID_FLAGS			0x0000001b

#define COWD_IS_ROOT(f)				((f) & COWD_FLAG_ROOT)
#define COWD_IS_CHECKED(f)			((f) & COWD_FLAG_CHECKED)
#define COWD_IS_MULTI(f)			((f) & COWD_FLAG_MULTI)
#define COWD_SET_HW_VER(f)			((f) & COWD_FLAG_HW_VER)

//
//	Controller type values
//
#define COWD_CONTROLLER_IDE			0x00656469	// 'ide'
#define COWD_CONTROLLER_SCSI		0x69736373	// 'scsi'

//
//	VMware virtual hardware version
//
#define COWD_HARDWARE_VMWARE2		1
#define COWD_HARDWARE_VMWARE3		2
#define COWD_HARDWARE_VMWARE4		3

//
//	VMware-Tools flag values
//
#define COWD_TOOL_INSTALLED			0x00000400	// VMware-Tools is installed
#define COWD_TOOL_UP_TO_DATE		0x00000001	// VMware-Tools is up to date

#define COWD_TOOL_IS_INSTALLED(f)	((f) & COWD_TOOL_INSTALLED)
#define COWD_TOOL_IS_UP_TO_DATE(f)	((f) & COWD_TOOL_UP_TO_DATE)

///////////////////////////////////////
//
//	Virtual Disk Header structrues
//

#ifdef _MSC_VER
#pragma pack(4)
#endif

//
//	header part 1 -- starting offset 0x0000
//
typedef struct _COWD_SECTOR_0 {		//	offset
	ULONG Signature;				//	00000000
									//		signature
									//		"COWD"	VMware 2, 3

	ULONG Version;					//	00000004
									//		Virtual Disk version
									//		1	VMware 2
									//		3	VMware 3

	ULONG Flags;					//	00000008
									//		single file, root	0x00000013
				   					//		single file, child	0x00000012
		   							//		multi file, root  	0x0000001b
		   							//		multi file, child	0x0000001a

	ULONG Capacity;					//	0000000c
									//		single-root:  total sectors per disk
	   								//		single-child: total sectors per disk
	   								//		multi-root:	  approx. total sectors per file
	   								//					 -- cyl * head * sec
	   								//		multi-child:  total sectors per disk

	ULONG Granularity;				//	00000010
									//		number of sectors per secondary block

	ULONG PrimaryMapOffset;			//	00000014
									//		primary map position (sector offset)

	ULONG PrimaryMapSize;			//	00000018
									//		number of secondary maps in the primary map

	ULONG EndOfFile;				//	0000001c
									//		actuall current file size in sectors

	union _u {
		struct _geometry {			//	CHS values of this single file
			ULONG Cylinders;		//	00000020
			ULONG Tracks;			//	00000024
			ULONG Sectors;			//	00000028
		} Geometry;					//		Only in root files.
									//		Not accurate in multi-file virtual disks.

		char ParentPath[260];		//	00000020
									//		parent file path
	} u;

}
#ifdef __GNUC__
__attribute__ ((aligned(4),packed))
#endif
COWD_SECTOR_0, *PCOWD_SECTOR_0;

//
//	header part 2 -- starting offset 0x0420
//
typedef struct _COWD_SECTOR_2 {
	ULONG ParentTS;					//	00000420
									//		Parent's timestamp

	ULONG TimeStamp;				//	00000424
									//		Last modification timestamp

	char NodeFileName[60];			//	00000428
									//		Unknown purpose
									//		Probably for GSX Server?

	char NodeDescription[1];		//	00000464
									//		Unknown length and purpose
									//		Probably for GSX Server?
}
#ifdef __GNUC__
__attribute__ ((aligned(4),packed))
#endif
COWD_SECTOR_2, *PCOWD_SECTOR_2;

//
//	header part 3 -- starting offset 0x0664
//
typedef struct _COWD_SECTOR_3 {
	ULONG TimeStamp;				//	00000664
									//		Saved modification timestamp

	ULONG Controller;				//	00000668
									//		'ide' or 'scsi'

	ULONG unknown1;					//	0000066c
									//		Probably to terminate controller value

	ULONG FileOrdinal;				// 	00000670	(v3)
									//		0 based sequcence number
									//		for multi-part disk

	ULONG FilesPerDisk;				// 	00000674	(v3)
									//		total number of files
									//		for multi-part disk

	ULONG Cylinders;				// 	00000678	(v3)
									//		cylinders per disk
									//		present only in root files

	ULONG Tracks;					// 	0000067c	(v3)
									//		Tracks per cylinder
									//		present only in root files

	ULONG Sectors;					// 	00000680	(v3)
									//		Sectors per track
									//		present only in root files

	ULONG DiskCapacity;				// 	00000684	(v3)
									//		total sectors per disk
									//		present only in root files

	ULONG SequenceNumber;			//	00000688	(v3)
									//		updates since create / last shrink

	ULONG FileCapacity;				//	0000068c 	(v3)
									//		Maximum sectors this file can hold
									//		Equal to or larger than logical file
									//		capacity (C*H*S).

	ULONG HardwareVer;				//	00000690	(v3)
									//		virtual hardware version
									//		1	VMware 2
									//		2	VMware 3

	ULONG ToolsFlag;				//	00000694
									//		0x0401	VMware-Tools is up to date
									//		0x0400	VMware-Tools is outdated
									//		0x0001	VMware-Tools is outdated (?)
}
#ifdef __GNUC__
__attribute__ ((aligned(4),packed))
#endif
COWD_SECTOR_3, *PCOWD_SECTOR_3;

//
//	structure to access all header information as a unit
//	-- structure total size is 4 sectors = 2048 bytes
//
typedef struct _COWD_HEADER {
	//
	//	header part 1 at sector #0 and #1
	//
	COWD_SECTOR_0	sec0;
	UCHAR			sec0_trail[0x400 - sizeof(COWD_SECTOR_0)];

	//
	//	header part 2 at sector #2
	//
	UCHAR			sec2_lead[0x20];
	COWD_SECTOR_2	sec2;
	UCHAR			sec2_trail[0x200 - sizeof(COWD_SECTOR_2) - 0x20];

	//
	//	header part 3 at sector #3
	//
	UCHAR			sec3_lead[0x64];
	COWD_SECTOR_3	sec3;
	UCHAR			sec3_trail[0x200 - sizeof(COWD_SECTOR_3) - 0x64];
}
#ifdef __GNUC__
__attribute__ ((aligned(4),packed))
#endif
COWD_HEADER, *PCOWD_HEADER;

#ifdef _MSC_VER
#pragma pack()
#endif

#endif	// _COWDISK_H_
