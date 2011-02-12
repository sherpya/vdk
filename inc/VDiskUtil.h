/*
	VDiskUtil.h

	VDisk utility function header
	Copyright (C) 2003 Ken Kato
*/

#ifndef _VDISKUTIL_H_
#define _VDISKUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

//
//	Virtual disk flags
//
#define VDISK_FLAG_SINGLE	0x00000001
#define VDISK_FLAG_SPARSE	0x00000002
#define VDISK_FLAG_CHILD	0x00000004
#define VDISK_FLAG_ABSPATH	0x00000008
#define VDISK_FLAG_DIRTY	0x80000000

//
//	Extent creation flags
//
#define VDISK_CREATE_SPARSE	0x00000001
#define VDISK_CREATE_FORCE	0x00000002
#define VDISK_CREATE_SINGLE	0x00000004

//
//	Controller type
//
enum {
	VDISK_CONTROLLER_NONE = 0,
	VDISK_CONTROLLER_IDE,
	VDISK_CONTROLLER_SCSI
};

//
//	Virtual Disk callback reasons
//
enum {
	//	Cannot open specified file
	//	params[0]:	PCHAR	path,
	//	params[1]:	VDKSTAT	error
	//	return:		TRUE	retry with new path stored into params[0]
	//				FALSE	fail
	VDISK_CB_FILE_OPEN,

	//	Confirms the user if the file is raw sector image file
	//	params[0]	PCHAR	path
	//	return		TRUE	the file is a raw sector image file
	//				FALSE	abort
	VDISK_CB_FILE_TYPE,

	//	Confirmation for applying fixes to actual file
	//	params[0]	PCHAR	path
	//	return		TRUE	update the actual file
	//				FALSE	do not update the actual file
	VDISK_CB_CONFIRM_FIX,

	//	Descriptor file does not contain any extent entry (FATAL)
	//	params[0]	PCHAR	path
	//	return		NONE
	VDISK_CB_EMPTY_IMAGE,

	//	Extent file size is not proper multiple of sector size
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	size
	//	return:		TRUE	ignore / correct
	//				FALSE	abort
	VDISK_CB_SIZE_BOUNDARY,

	//	Invalid signature
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	bad signature
	//	return:		TRUE	ignore / correct
	//				FALSE	abort
	VDISK_CB_SIGNATURE,

	//	Unknown controller type in virtual disk file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct value
	//				0: abort
	VDISK_CB_CONTROLLER,

	//	bad hardware version in virtual disk file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct value
	//				0: abort
	VDISK_CB_HARDWAREVER,

	//	Unrecognizable entry in description file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		TRUE	ignore
	//				FALSE	abort
	VDISK_CB_DESC_BADENTRY,

	//	bad offset value in description file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct offset value
	//				-1: abort
	VDISK_CB_DESC_OFFSET,

	//	bad capacity value in description file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct capacity value
	//				0: abort
	VDISK_CB_DESC_CAPACITY,

	//	invalid geometry values in description file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct value
	//				0: abort
	VDISK_CB_DESC_GEOMETRY,

	//	Unknown extent type in virtual disk file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		VDK_FILETYPE value
	VDISK_CB_DESC_FILETYPE,

	//	bad timestamp value in virtual disk file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct value
	//				0: abort
	VDISK_CB_DESC_TIMESTAMP,

	//	Unknown disk type in virtual disk file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct value
	//				0: abort
	VDISK_CB_DESC_DISKTYPE,

	//	described offset does not match actual offset
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	described offset
	//	params[2]:	ULONG	calculated offset
	//	return		TRUE	use correct value
	//				FALSE	abort
	VDISK_CB_EXT_OFFSET,

	//	described capacity does not match actual file size
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	described size
	//	params[2]:	ULONG	calculated size
	//	return:		correct capacity
	//				0	abort
	VDISK_CB_EXT_FILESIZE,

	//	described disk capacity does not match the total of extents
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	described capacity
	//	params[2]:	ULONG	calculated capacity
	//	return:		TRUE	corrected or ignore
	//				FALSE	abort
	VDISK_CB_EXT_CAPACITY,

	//	Ordinal stored in an extent file does not match the actual order
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	ordinal in the header
	//	params[2]:	ULONG	correct ordinal
	//	return:		TRUE	corrected or ignore
	//				FALSE	abort
	VDISK_CB_COWD_ORDINAL,

	//	Parameter conflict between extents
	//	params[0]:	PCHAR	path1
	//	params[1]:			value1
	//	params[1]:	PCHAR	path2
	//	params[3]:			value2
	//	return:		TRUE	corrected or ignore
	//				FALSE	abort
	VDISK_CB_CONF_FILEVER,
	VDISK_CB_CONF_FLAGS,
	VDISK_CB_CONF_PARENTTS,
	VDISK_CB_CONF_TIMESTAMP,
	VDISK_CB_CONF_CONTROLLER,
	VDISK_CB_CONF_EXTENTS,
	VDISK_CB_CONF_CYLINDERS,
	VDISK_CB_CONF_TRACKS,
	VDISK_CB_CONF_SECTORS,
	VDISK_CB_CONF_CAPACITY,
	VDISK_CB_CONF_HARDWARE,
	VDISK_CB_CONF_TOOLSFLAG,
	VDISK_CB_CONF_SEQNUM,
	VDISK_CB_CONF_PARENTPATH,

	//	prompts to ignore non-fatal conflicts
	//	return:		TRUE	yes
	//				FALSE	no
	VDISK_CB_CONFLICT_IGNORE,

	//	disk capacity stored in the header does not match the total of extents
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	stored capacity
	//	params[2]:	ULONG	total capacity
	//	return:		TRUE	corrected or ignore
	//				FALSE	abort
	VDISK_CB_COWD_CAPACITY,

	//	Invalid cowd version
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	bad version
	//	return:		correct version
	VDISK_CB_COWD_FILEVER,

	//	File geometry values inconsistent or capacity = 0
	//	params[0]:	PCHAR			path
	//	params[1]:	PCOWD_SECTOR_0	sec0
	//	return:		NONE
	VDIDK_CB_COWD_FILEGEOM,

	//	Disk geometry values inconsistent or capacity = 0
	//	params[0]:	PCHAR			path
	//	params[1]:	PCOWD_SECTOR_3	sec3
	//	return:		NONE
	VDISK_CB_COWD_DISKGEOM,

	//	parent path length
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	parent_path
	//	return:		NONE
	VDISK_CB_COWD_PARENT,

	//	Mapsize values in COWD header is inconsistent with capacity
	//	params[0]:	PCHAR	path
	//	params[2]:	ULONG	wrong value
	//	params[1]:	ULONG	correct value
	//	return:		corrected map size
	VDISK_CB_COWD_MAPSIZE,

	//	EndOfFile value does not match actual file size
	//	params[0]:	PCHAR	path
	//	params[2]:	ULONG	wrong value
	//	params[1]:	ULONG	correct value
	//	return:		TRUE	ignore
	//				FALSE	fail
	VDISK_CB_COWD_ENDOFFILE,

	//	cowd_sec2->TimeStamp != cowd_sec3->TimeStamp
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	timestamp1
	//	params[2]:	ULONG	timestamp2
	//	return:		TRUE	ignore
	//				FALSE	fail
	VDISK_CB_COWD_TIMESTAMP,

	//	VMDK descriptor is not found in the file
	//	params[0]:	PCHAR	path
	//	return:		NONE
	VDISK_CB_VMDK_NODESC,

	//	Invalid vmdk version
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	bad version
	//	return:		correct version
	VDISK_CB_VMDK_FILEVER,

	//	Invalid vmdk capacity
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_VMDK_FILECAP,

	//	Invalid vmdk granularity
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_VMDK_GRANULARITY,

	//	Invalid vmdk descriptor offset
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_VMDK_DESCOFFSET,

	//	Invalid vmdk descriptor size
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_VMDK_DESCSIZE,

	//	Invalid vmdk GTEsPerGT value
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_VMDK_GTESPERGT,

	//	Invalid vmdk grain dictionary offset
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_VMDK_GDOFFSET,

	//	Invalid vmdk grain offset
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_VMDK_GRAINOFFSET,

	//	Invalid vmdk check bytes
	//	params[0]:	PCHAR	path
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_VMDK_CHECKBYTES,

	//	VMDK capacity in the header and descriptor mismatch
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	size in the header
	//	params[2]:	ULONG	size in the descriptor
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_VMDK_SIZEMISMATCH,

	//	Capacity of the child and the parent do not match
	//	params[0]:	PCHAR	child path
	//	params[1]:	ULONG	child capacity
	//	params[2]:	PCHAR	parent path
	//	params[3]:	ULONG	parent capacity
	//	return:		NONE	this is fatal
	VDISK_CB_PARENT_CAPACITY,

	//	Controller type of the child and the parent do not match
	//	params[0]:	PCHAR	child path
	//	params[1]:	ULONG	child controller
	//	params[2]:	PCHAR	parent path
	//	params[3]:	ULONG	parent controller
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_PARENT_CONTROLLER,

	//	Child's ParentTS and Parent's Timestamp do not match
	//	params[0]:	PCHAR	child path
	//	params[1]:	ULONG	child ParentTS
	//	params[2]:	PCHAR	parent path
	//	params[3]:	ULONG	parent Timestamp
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	VDISK_CB_PARENT_TIMESTAMP,

};

//
//	Create and Initialize a new VDisk instance from existing files
//
VDKSTAT VDiskLoadFile(PVOID *ppDisk, PCHAR pPath, PCHAR pBasePath);

//
//	Create all ancestors of a virtual disk
//
VDKSTAT VDiskCreateTree(PVOID pDisk);

//
//	Create a REDO log for a virtual disk
//
VDKSTAT VDiskCreateRedo(PVOID *ppDisk, PSTR pUndoPath);

//
//	Delete VDisk Tree
//
VOID VDiskDelete(PVOID pDisk);

//
//	Search specified file
//
VOID VDiskSetSearchPath(PSTR pPath);
VDKSTAT VDiskSearchFile(HANDLE *pFile, PCHAR pPath, PCHAR pBase);

//
//	build VDK_OPEN_FILE_INFO structure
//
VDKSTAT VDiskMapToOpenInfo(
	PVOID pDisk,
	PVOID *ppInfo,
	PULONG pInfoLen);

PCHAR	VDiskGetDiskName(PVOID pDisk);

//
//	Virtual disk check callback function
//
typedef ULONG(*VDISK_CALLBACK)(ULONG reason, PVOID *params);

extern VDISK_CALLBACK VDiskCallBack;

void VDiskSetCallBack(VDISK_CALLBACK cb);

ULONG VDiskCallback(ULONG reason, PVOID *params);

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif	// _VDISKUTIL_H_
