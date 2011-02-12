/*
	VDiskCB.cpp

	VDiskInitialize() callback function (text mode)
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkmsg.h"

#include "cowdisk.h"
#include "vmdisk.h"

#include "VDiskUtil.h"


ULONG VDiskCallback(ULONG reason, PVOID *params)
{
	ULONG ret = 0;

#ifdef _WIN32
	//	Windows FormatMessage does not expand 64bit argument
	//	correctly so convert to a string argument
#define I64PRM(v)	_i64toa(*(PINT64)(v), i64buf, 10)
	CHAR i64buf[24];
#else
#define I64PRM(v)	(*(PINT64)(v))
#endif	// _WIN32

	switch (reason) {
	//	Cannot open specified file
	//	params[0]:	PCHAR	path,
	//	params[1]:	VDKSTAT	error
	//	return:		TRUE	retry with new path stored into params[0]
	//				FALSE	fail
	case VDISK_CB_FILE_OPEN:
		PrintMessage(MSG_CB_FILE_OPEN,
			(PCHAR)params[0], VdkStatusStr((ULONG)params[1]));

		if ((ULONG)params[1] == VDK_NOFILE ||
			(ULONG)params[1] == VDK_NOPATH) {

			//
			//	file not found -- prompt user for a correct path
			//
			PrintMessage(MSG_PROMPT_PATH);
			fflush(stdout);

			if (fgets((PCHAR)params[0], MAX_PATH, stdin) &&
				*(PCHAR)params[0] != '\n' &&
				*(PCHAR)params[0] != '\0') {

				PCHAR p = (PCHAR)params[0];

				while (*p && *p != '\n') {
					p++;
				}
				*p = '\0';
				ret = TRUE;
			}
		}
		break;

	//	Confirms the user if the file is raw sector image file
	//	params[0]	PCHAR	path
	//	return		TRUE	the file is a raw sector image file
	//				FALSE	abort
	case VDISK_CB_FILE_TYPE:
		PrintMessage(MSG_CB_FILE_TYPE, (PCHAR)params[0]);
//		ret = (InputChar(MSG_PROMPT_YESNO, "yn") == 'y');
		ret = TRUE;
		break;

	//	Confirmation for applying fixes to actual file
	//	params[0]	PCHAR	path
	//	return		TRUE	update the actual file
	//				FALSE	do not update the actual file
	case VDISK_CB_CONFIRM_FIX:
		break;

	//	Descriptor file does not contain any extent entry (FATAL)
	//	params[0]	PCHAR	path
	//	return		NONE
	case VDISK_CB_EMPTY_IMAGE:
		PrintMessage(MSG_CB_EMPTY_IMAGE, (PCHAR)params[0]);
		break;

	//	Extent file size is not proper multiple of sector size
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	size
	//	return:		TRUE	ignore / correct
	//				FALSE	abort
	case VDISK_CB_SIZE_BOUNDARY:
		PrintMessage(MSG_CB_SIZE_BOUNDARY,
			(PCHAR)params[0], I64PRM(params[1]),
			(ULONG)*(PINT64)params[1] & VDK_SECTOR_ALIGNMENT_MASK);

		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i');
		break;

	//	Invalid signature
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	bad signature
	//	return:		TRUE	ignore / correct
	//				FALSE	abort
	case VDISK_CB_SIGNATURE:
		PrintMessage(MSG_CB_SIGNATURE,
			(PCHAR)params[0], (ULONG)params[1]);

		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i');
		break;

	//	Unknown controller type in virtual disk file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct value
	//				0: abort
	case VDISK_CB_CONTROLLER:
		PrintMessage(MSG_CB_CONTROLLER,
			(PCHAR)params[0], (PCHAR)params[1]);

		switch (InputChar(MSG_PROMPT_CONTROLLER, "isc")) {
		case 'i':
			ret = VDISK_CONTROLLER_IDE;
			break;

		case 's':
			ret = VDISK_CONTROLLER_SCSI;
			break;

		default:
			ret = 0;
			break;
		}
		break;

	//	bad hardware version in virtual disk file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct value
	//				0: abort
	case VDISK_CB_HARDWAREVER:
		PrintMessage(MSG_CB_HARDWAREVER,
			(PCHAR)params[0], (PCHAR)params[1]);
		/*
		switch (InputChar("Input VMware version (2, 3, 4) or A) abort : ", "a234")) {
		case '2':
			ret = COWD_HARDWARE_VMWARE2;
			break;

		case '3':
			ret = COWD_HARDWARE_VMWARE3;
			break;

		case '4':
			ret = COWD_HARDWARE_VMWARE4;
			break;

		default:
			ret = 0;
			break;
		}
		*/
		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i')
			? COWD_HARDWARE_VMWARE3 : 0;
		break;

	//	Unrecognizable entry in description file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		TRUE	ignore
	//				FALSE	abort
	case VDISK_CB_DESC_BADENTRY:
		PrintMessage(MSG_CB_DESC_BADENTRY,
			(PCHAR)params[0], (PCHAR)params[1]);

		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i');
		break;

	//	bad offset value in description file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct offset value
	//				-1: abort
	case VDISK_CB_DESC_OFFSET:
		PrintMessage(MSG_CB_DESC_OFFSET,
			(PCHAR)params[0], (PCHAR)params[1]);
/*
		ret = InputVal("Correct offset : ", (ULONG)-1);
*/
		ret = (ULONG)-1;
		break;

	//	bad capacity value in description file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct capacity value
	//				0: abort
	case VDISK_CB_DESC_CAPACITY:
		PrintMessage(MSG_CB_DESC_CAPACITY,
			(PCHAR)params[0], (PCHAR)params[1]);
/*
		ret = InputVal("Correct capacity : ", 0);
*/
		break;

	//	invalid geometry values in description file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct value
	//				0: abort
	case VDISK_CB_DESC_GEOMETRY:
		PrintMessage(MSG_CB_DESC_GEOMETRY,
			(PCHAR)params[0], (PCHAR)params[1]);
/*
		ret = InputVal("Correct value : ", 0);
*/
		break;

	//	Unknown extent type in virtual disk file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		VDK_FILETYPE value
	case VDISK_CB_DESC_FILETYPE:
		PrintMessage(MSG_CB_DESC_FILETYPE,
			(PCHAR)params[0], (PCHAR)params[1]);
/*
		ret = (InputChar("S) Sparse (compact) or F) Flat (preallocate) ? ", "sf") == 's');
*/
		break;

	//	bad timestamp value in virtual disk file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct value
	//				0: abort
	case VDISK_CB_DESC_TIMESTAMP:
		PrintMessage(MSG_CB_DESC_TIMESTAMP,
			(PCHAR)params[0], (PCHAR)params[1]);
/*
		ret = InputHexVal("Correct value : ", 0);
*/
		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i');
		break;

	//	Unknown disk type in virtual disk file
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	invalid entry
	//	return:		correct value
	//				0: abort
	case VDISK_CB_DESC_DISKTYPE:
		PrintMessage(MSG_CB_DESC_DISKTYPE,
			(PCHAR)params[0], (PCHAR)params[1]);
/*
		ret = InputChar(
					"1) flat/split\n"
					"2) flat/single\n"
					"3) sparse/split\n"
					"4) sparse/single\n"
					"0) cancel\n"
					"? ", "01234") - '0';
*/
		break;

	//	described offset does not match actual offset
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	described offset
	//	params[2]:	ULONG	calculated offset
	//	return		TRUE	use correct value
	//				FALSE	abort
	case VDISK_CB_EXT_OFFSET:
		PrintMessage(MSG_CB_EXT_OFFSET,
			(PCHAR)params[0], (ULONG)params[1], (ULONG)params[2]);

		ret = (InputChar(MSG_PROMPT_YESNO, "yn") == 'y');
		break;

	//	described capacity does not match actual file size
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	described size
	//	params[2]:	ULONG	actual size
	//	return:		correct capacity
	//				0	abort
	case VDISK_CB_EXT_FILESIZE:
		PrintMessage(MSG_CB_EXT_FILESIZE,
			(PCHAR)params[0], (ULONG)params[1], (ULONG)params[2]);

		ret = ((InputChar(MSG_PROMPT_YESNO, "yn") == 'y') ? (ULONG)params[2] : 0);
		break;

	//	described disk capacity does not match the total of extents
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	described capacity
	//	params[2]:	ULONG	calculated capacity
	//	return:		TRUE	corrected or ignore
	//				FALSE	abort
	case VDISK_CB_EXT_CAPACITY:
		PrintMessage(MSG_CB_EXT_CAPACITY,
			(PCHAR)params[0], (ULONG)params[1], (ULONG)params[2]);
		break;

	//	Ordinal stored in an extent file does not match the actual order
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	ordinal in the header
	//	params[2]:	ULONG	correct ordinal
	//	return:		TRUE	corrected or ignore
	//				FALSE	abort
	case VDISK_CB_COWD_ORDINAL:
		PrintMessage(MSG_CB_COWD_ORDINAL,
			(PCHAR)params[0], (ULONG)params[1], (ULONG)params[2]);

		ret = (InputChar(MSG_PROMPT_YESNO, "yn") == 'y');
		break;

	//	Parameter conflict between extents
	//	params[0]:	PCHAR	path1
	//	params[1]:			value1
	//	params[1]:	PCHAR	path2
	//	params[3]:			value2
	//	return:		TRUE	corrected or ignore
	//				FALSE	abort
	case VDISK_CB_CONF_FILEVER:
		PrintMessage(MSG_CB_CONF_FILEVER,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_FLAGS:
		PrintMessage(MSG_CB_CONF_FLAGS,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_PARENTTS:
		PrintMessage(MSG_CB_CONF_PARENTTS,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_TIMESTAMP:
		PrintMessage(MSG_CB_CONF_TIMESTAMP,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_CONTROLLER:
		PrintMessage(MSG_CB_CONF_CONTROLLER,
			(PCHAR)params[0], (PCHAR)params[1],
			(PCHAR)params[2], (PCHAR)params[3]);
		break;
	case VDISK_CB_CONF_EXTENTS:
		PrintMessage(MSG_CB_CONF_EXTENTS,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_CYLINDERS:
		PrintMessage(MSG_CB_CONF_CYLINDERS,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_TRACKS:
		PrintMessage(MSG_CB_CONF_TRACKS,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_SECTORS:
		PrintMessage(MSG_CB_CONF_SECTORS,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_CAPACITY:
		PrintMessage(MSG_CB_CONF_CAPACITY,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_HARDWARE:
		PrintMessage(MSG_CB_CONF_HARDWARE,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_TOOLSFLAG:
		PrintMessage(MSG_CB_CONF_TOOLSFLAG,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_SEQNUM:
		PrintMessage(MSG_CB_CONF_SEQNUM,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;
	case VDISK_CB_CONF_PARENTPATH:
		PrintMessage(MSG_CB_CONF_PARENTPATH,
			(PCHAR)params[0], (PCHAR)params[1],
			(PCHAR)params[2], (PCHAR)params[3]);
		break;

	//	prompts to ignore non-fatal conflicts
	//	return:		TRUE	yes
	//				FALSE	no
	case VDISK_CB_CONFLICT_IGNORE:
		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i');
		break;

	//	disk capacity stored in the header does not match the total of extents
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	stored capacity
	//	params[2]:	ULONG	total capacity
	//	return:		TRUE	corrected or ignore
	//				FALSE	abort
	case VDISK_CB_COWD_CAPACITY:
		PrintMessage(MSG_CB_COWD_CAPACITY,
			(PCHAR)params[0], (ULONG)params[1], (ULONG)params[2]);
		break;

	//	Invalid cowd version
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	bad version
	//	return:		correct version
	case VDISK_CB_COWD_FILEVER:
		PrintMessage(MSG_CB_COWD_FILEVER,
			(PCHAR)params[0], (ULONG)params[1]);

		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i')
			? COWD_FILEVER_VMWARE3 : 0;
		break;

	//	File geometry values inconsistent or capacity = 0
	//	params[0]:	PCHAR			path
	//	params[1]:	PCOWD_SECTOR_0	sec0
	//	return:		NONE
	case VDIDK_CB_COWD_FILEGEOM:
		PrintMessage(MSG_CB_COWD_GEOMETRY,
			(PCHAR)params[0],
			((PCOWD_SECTOR_0)params[1])->u.Geometry.Cylinders,
			((PCOWD_SECTOR_0)params[1])->u.Geometry.Tracks,
			((PCOWD_SECTOR_0)params[1])->u.Geometry.Sectors,
			((PCOWD_SECTOR_0)params[1])->Capacity);
		break;

	//	Disk geometry values inconsistent or capacity = 0
	//	params[0]:	PCHAR			path
	//	params[1]:	PCOWD_SECTOR_3	sec3
	//	return:		NONE
	case VDISK_CB_COWD_DISKGEOM:
		PrintMessage(MSG_CB_COWD_GEOMETRY,
			(PCHAR)params[0],
			((PCOWD_SECTOR_3)params[1])->Cylinders,
			((PCOWD_SECTOR_3)params[1])->Tracks,
			((PCOWD_SECTOR_3)params[1])->Sectors,
			((PCOWD_SECTOR_3)params[1])->DiskCapacity);
		break;

	//	parent path length
	//	params[0]:	PCHAR	path
	//	params[1]:	PCHAR	parent_path
	//	return:		TRUE	correct
	//				FALSE	abort
	case VDISK_CB_COWD_PARENT:
		PrintMessage(MSG_CB_COWD_PARENT,
			(PCHAR)params[0], (PCHAR)params[1]);
		break;

	//	Mapsize values in COWD header is inconsistent with capacity
	//	params[0]:	PCHAR	path
	//	params[2]:	ULONG	wrong value
	//	params[1]:	ULONG	correct value
	//	return:		corrected map size
	case VDISK_CB_COWD_MAPSIZE:
		PrintMessage(MSG_CB_COWD_MAPSIZE,
			(PCHAR)params[0], (ULONG)params[1], (ULONG)params[2]);

/*
		if (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i') {
			ret = (ULONG)params[1];
		}
*/
		break;

	//	EndOfFile value does not match actual file size
	//	params[0]:	PCHAR	path
	//	params[2]:	ULONG	wrong value
	//	params[1]:	ULONG	correct value
	//	return:		TRUE	ignore
	//				FALSE	fail
	case VDISK_CB_COWD_ENDOFFILE:
		PrintMessage(MSG_CB_COWD_ENDOFFILE,
			(PCHAR)params[0], (ULONG)params[1], (ULONG)params[2]);

		ret = TRUE;;
		break;

	//	cowd_sec2->TimeStamp != cowd_sec3->TimeStamp
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	timestamp1
	//	params[2]:	ULONG	timestamp2
	//	return:		TRUE	ignore
	//				FALSE	fail
	case VDISK_CB_COWD_TIMESTAMP:
		PrintMessage(MSG_CB_COWD_TIMESTAMP,
			(PCHAR)params[0], (ULONG)params[1], (ULONG)params[2]);

		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i');
		break;

	//	VMDK descriptor is not found in the file
	//	params[0]:	PCHAR	path
	//	return:		NONE
	case VDISK_CB_VMDK_NODESC:
		PrintMessage(MSG_CB_VMDK_NODESC, (PCHAR)params[0]);
		break;

	//	Invalid vmdk version
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	bad version
	//	return:		correct version
	case VDISK_CB_VMDK_FILEVER:
		PrintMessage(MSG_CB_VMDK_FILEVER,
			(PCHAR)params[0], (ULONG)params[1]);

		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i')
			? VMDK_FILEVER_VMWARE4 : 0;
		break;

	//	Invalid vmdk capacity
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_VMDK_FILECAP:
		PrintMessage(MSG_CB_VMDK_FILECAP,
			(PCHAR)params[0], I64PRM(params[1]));
		break;

	//	Invalid vmdk granularity
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_VMDK_GRANULARITY:
		PrintMessage(MSG_CB_VMDK_GRANULARITY,
			(PCHAR)params[0], I64PRM(params[1]));
		break;

	//	Invalid vmdk descriptor offset
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_VMDK_DESCOFFSET:
		PrintMessage(MSG_CB_VMDK_DESCOFFSET,
			(PCHAR)params[0], I64PRM(params[1]));
		break;

	//	Invalid vmdk descriptor size
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_VMDK_DESCSIZE:
		PrintMessage(MSG_CB_VMDK_DESCSIZE,
			(PCHAR)params[0], I64PRM(params[1]));
		break;

	//	Invalid vmdk GTEsPerGT value
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_VMDK_GTESPERGT:
		PrintMessage(MSG_CB_VMDK_GTESPERGT,
			(PCHAR)params[0], (ULONG)params[1]);
		break;

	//	Invalid vmdk grain dictionary offset
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_VMDK_GDOFFSET:
		PrintMessage(MSG_CB_VMDK_GDOFFSET,
			(PCHAR)params[0], I64PRM(params[1]));
		break;

	//	Invalid vmdk grain offset
	//	params[0]:	PCHAR	path
	//	params[1]:	PINT64	bad value
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_VMDK_GRAINOFFSET:
		PrintMessage(MSG_CB_VMDK_GRAINOFFSET,
			(PCHAR)params[0], I64PRM(params[1]));
		break;

	//	Invalid vmdk check bytes
	//	params[0]:	PCHAR	path
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_VMDK_CHECKBYTES:
		PrintMessage(MSG_CB_VMDK_CHECKBYTES, (PCHAR)params[0]);

		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i');
		break;

	//	VMDK capacity in the header and descriptor mismatch
	//	params[0]:	PCHAR	path
	//	params[1]:	ULONG	size in the header
	//	params[2]:	ULONG	size in the descriptor
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_VMDK_SIZEMISMATCH:
		PrintMessage(MSG_CB_VMDK_SIZEMISMATCH,
			(PCHAR)params[0], (ULONG)params[1], (ULONG)params[2]);

		ret = (InputChar(MSG_PROMPT_YESNO, "yn") == 'y');
		break;

	//	Capacity of the child and the parent do not match
	//	params[0]:	PCHAR	child path
	//	params[1]:	ULONG	child capacity
	//	params[2]:	PCHAR	parent path
	//	params[3]:	ULONG	parent capacity
	//	return:		NONE	this is fatal
	case VDISK_CB_PARENT_CAPACITY:
		PrintMessage(MSG_CB_PARENT_CAPACITY,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);
		break;

	//	Controller type of the child and the parent do not match
	//	params[0]:	PCHAR	child path
	//	params[1]:	ULONG	child controller
	//	params[2]:	PCHAR	parent path
	//	params[3]:	ULONG	parent controller
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_PARENT_CONTROLLER:
		PrintMessage(MSG_CB_PARENT_CONTROLLER,
			(PCHAR)params[0], (ULONG)params[1] == VDISK_CONTROLLER_SCSI ? "SCSI" : "IDE",
			(PCHAR)params[2], (ULONG)params[3] == VDISK_CONTROLLER_SCSI ? "SCSI" : "IDE");

		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i');
		break;

	//	Child's ParentTS and Parent's Timestamp do not match
	//	params[0]:	PCHAR	child path
	//	params[1]:	ULONG	child ParentTS
	//	params[2]:	PCHAR	parent path
	//	params[3]:	ULONG	parent Timestamp
	//	return:		TRUE	ignore / corrected
	//				FALSE	abort
	case VDISK_CB_PARENT_TIMESTAMP:
		PrintMessage(MSG_CB_PARENT_TIMESTAMP,
			(PCHAR)params[0], (ULONG)params[1],
			(PCHAR)params[2], (ULONG)params[3]);

		ret = (InputChar(MSG_PROMPT_ABORTIGNORE, "ai") == 'i');
		break;

	default:
		fprintf(stderr, "Unknown Callback reason %lu\n", reason);
		break;
	}

	return ret;
}
