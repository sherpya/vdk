/*
	vdkpart.c

	Read partition table from virtual disk
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkaccess.h"
#include "vdkpart.h"

//
//	Decide file system type from sector data
//
UCHAR VdkIdentifyFAT(
	PFAT16_PBR		Pbr,
	PPARTITION_ITEM	PartItem)
{
	if (Pbr->signature == SIGNATURE_WORD &&
		(Pbr->bpb.BytesPerSector == 512 ||
		Pbr->bpb.BytesPerSector == 1024 ||
		Pbr->bpb.BytesPerSector == 2048 ||
		Pbr->bpb.BytesPerSector == 4096) &&
		(Pbr->bpb.SectorsPerCluster == 1 ||
		Pbr->bpb.SectorsPerCluster == 2 ||
		Pbr->bpb.SectorsPerCluster == 4 ||
		Pbr->bpb.SectorsPerCluster == 8 ||
		Pbr->bpb.SectorsPerCluster == 16 ||
		Pbr->bpb.SectorsPerCluster == 32 ||
		Pbr->bpb.SectorsPerCluster == 64 ||
		Pbr->bpb.SectorsPerCluster == 128) &&
		Pbr->bpb.MediaDescriptor) {

		if (Pbr->bpb.NumberOfFATs == 0 &&
			Pbr->bpb.RootEntries == 0 &&
			Pbr->bpb.SmallSectors == 0 &&
			Pbr->bpb.SectorsPerFAT == 0 &&
			Pbr->bpb.LargeSectors == 0) {

			//
			//	could be NTFS
			//
			if (VdkCmpNoCaseN(Pbr->oemid, "OS2", 3) == 0) {
				//
				//	OS/2 filesystem
				//
				if (VdkCmpNoCaseN(Pbr->exbpb.FileSystemType, "HPFS", 4) == 0) {
					strncpy(PartItem->fsname, "OS2 HPFS", MAX_FSNAME_LEN);
				}
				else {
					strncpy(PartItem->fsname, "OS2 IFS", MAX_FSNAME_LEN);
				}
			}
			else if (VdkCmpNoCaseN(Pbr->oemid, "NTFS", 4) == 0) {
				//
				//	NTFS
				//
				strncpy(PartItem->fsname, "NTFS", MAX_FSNAME_LEN);
			}

			return PART_NTFS_HPFS;
		}
		else if (Pbr->bpb.NumberOfFATs == 2) {

			//
			//	FAT12/16/32
			//
			if (Pbr->bpb.RootEntries == 0 &&
				Pbr->bpb.SmallSectors == 0 &&
				Pbr->bpb.SectorsPerFAT == 0 &&
				Pbr->bpb.LargeSectors) {

				//
				//	FAT32
				//
				strcpy(PartItem->fsname, "FAT32");

				strncpy(
					PartItem->label,
					((PFAT32_PBR)Pbr)->exbpb.VolumeLabel,
					sizeof(((PFAT32_PBR)Pbr)->exbpb.VolumeLabel));

				return PART_DOS_FAT32;
			}
			else if (Pbr->bpb.RootEntries &&
				Pbr->bpb.SectorsPerFAT &&
				((Pbr->bpb.SmallSectors == 0 && Pbr->bpb.LargeSectors) ||
				(Pbr->bpb.SmallSectors && Pbr->bpb.LargeSectors == 0))) {

				//
				//	FAT12 or FAT16
				//
				if (!strncmp(Pbr->exbpb.FileSystemType, "FAT", 3)) {
					strncpy(
						PartItem->fsname,
						Pbr->exbpb.FileSystemType,
						sizeof(Pbr->exbpb.FileSystemType));
				}
				else {
					strcpy(PartItem->fsname, "FAT16");
				}

				strncpy(
					PartItem->label,
					Pbr->exbpb.VolumeLabel,
					sizeof(Pbr->exbpb.VolumeLabel));

				if (Pbr->bpb.LargeSectors) {
					return PART_DOS_HUGE;
				}
				else if (Pbr->bpb.SmallSectors >= MIN_FAT16_VOLUME) {
					return PART_DOS_FAT16;
				}
				else {
					return PART_DOS_FAT12;
				}
			}
		}
	}

	return PART_NONE;
}

UCHAR VdkIdentifyXFS(
	PXFS_SB			Xfsb,
	PPARTITION_ITEM	PartItem)
{
	if (!strncmp(Xfsb->s_magic, XFS_SUPER_MAGIC, sizeof(Xfsb->s_magic))) {

		strncpy(PartItem->fsname, "xfs", MAX_FSNAME_LEN);

		strncpy(
			PartItem->label,
			Xfsb->s_fname,
			sizeof(Xfsb->s_fname));

		return PART_LINUX;
	}

	return PART_NONE;
}

UCHAR VdkIdentifyEXT2(
	PEXT2_SB 		e2fsb,
	PPARTITION_ITEM	PartItem)
{
	if (e2fsb->s_magic == EXT2_SUPER_MAGIC) {

		if (e2fsb->s_feature & EXT3_HAS_JOURNAL) {
			strncpy(PartItem->fsname, "ext3fs", MAX_FSNAME_LEN);
		}
		else {
			strncpy(PartItem->fsname, "ext2fs", MAX_FSNAME_LEN);
		}

		strncpy(
			PartItem->label,
			e2fsb->s_volume_name,
			sizeof(e2fsb->s_volume_name));

		return PART_LINUX;
	}

	return PART_NONE;
}

UCHAR VdkIdentifyRFS(
	PREISER_SB		rfsb,
	PPARTITION_ITEM	PartItem)
{
	if (!strncmp(rfsb->s_magic, REISERFS_SUPER_MAGIC, strlen(REISERFS_SUPER_MAGIC)) ||
		!strncmp(rfsb->s_magic, REISER2FS_SUPER_MAGIC, strlen(REISER2FS_SUPER_MAGIC))) {

		strncpy(PartItem->fsname, "reiserfs", MAX_FSNAME_LEN);

		return PART_LINUX;
	}

	return PART_NONE;
}

UCHAR VdkIdentifyFS(
	PVDK_DISK_INFO	DiskInfo,
	HANDLE			hFile,
	PPARTITION_ITEM	PartItem)
{
	UCHAR	buf[1024];
	UCHAR	type;
	VDKSTAT	ret;

	//
	//	Read partition boot record
	//
	if (DiskInfo) {
		ret = VdkReadSector(
			DiskInfo,
			PartItem->offset,
			1,
			buf);
	}
	else {
		ret = VdkReadFileAt(
			hFile,
			(INT64)PartItem->offset << VDK_BYTE_SHIFT_TO_SECTOR,
			buf,
			VDK_BYTES_PER_SECTOR,
			NULL);
	}

	if (ret != VDK_OK) {
		return PART_NONE;
	}

	type = VdkIdentifyFAT((PFAT16_PBR)buf, PartItem);

	if (type != PART_NONE) {
		return type;
	}

	type = VdkIdentifyXFS((PXFS_SB)buf, PartItem);

	if (type != PART_NONE) {
		return type;
	}

	if (DiskInfo) {
		ret = VdkReadSector(
			DiskInfo,
			PartItem->offset + 2,
			1,
			buf);
	}
	else {
		ret = VdkReadFileAt(
			hFile,
			(INT64)(PartItem->offset + 2) << VDK_BYTE_SHIFT_TO_SECTOR,
			buf,
			VDK_BYTES_PER_SECTOR,
			NULL);
	}

	if (ret != VDK_OK) {
		return PART_NONE;
	}

	type = VdkIdentifyEXT2((PEXT2_SB)buf, PartItem);

	if (type != PART_NONE) {
		return type;
	}

	if (DiskInfo) {
		ret = VdkReadSector(
			DiskInfo,
			PartItem->offset + REISERFS_SUPER_OFFSET,
			1,
			buf);
	}
	else {
		ret = VdkReadFileAt(
			hFile,
			(INT64)(PartItem->offset + REISERFS_SUPER_OFFSET) << VDK_BYTE_SHIFT_TO_SECTOR,
			buf,
			VDK_BYTES_PER_SECTOR,
			NULL);
	}


	if (ret != VDK_OK) {
		return PART_NONE;
	}

	type = VdkIdentifyRFS((PREISER_SB)buf, PartItem);

	return type;
}

//
//	Read partition table and build partition list
//
VDKSTAT VdkListPartitions(
	PVDK_DISK_INFO	DiskInfo,
	HANDLE			hFile,
	ULONG			Capacity,
	PLIST_CALLBACK	CallBack,
	PVOID			Param)
{
	PARTITION_TABLE		ptable;
	PPARTITION_ENTRY	pentry;
	PARTITION_ITEM		pitem;
	ULONG ext_offset;
	int logical_num;
	int logical_offset;
	VDKSTAT ret;
	int idx;

	//
	//	Read master boot record
	//
	if (DiskInfo) {
		Capacity = DiskInfo->Capacity;

		ret = VdkReadSector(
			DiskInfo,
			0,
			1,
			(PUCHAR)&ptable);
	}
	else {
		ret = VdkReadFileAt(
			hFile,
			0,
			&ptable,
			VDK_BYTES_PER_SECTOR,
			NULL);
	}

	if (ret != VDK_OK) {
		return ret;
	}

	//
	//	Check if the MBR contains a valid partition table
	//
	idx = 0;

	if (ptable.p.signature == SIGNATURE_WORD) {
		do {
			pentry = &(ptable.p.partition[idx]);

			if (pentry->type) {
				ULONG start, end;

				start = pentry->start_head * (pentry->start_sec & 0x3f) *
					(((pentry->start_sec & 0xc0) << 8) + pentry->start_cyl);

				end = pentry->end_head * (pentry->end_sec & 0x3f) *
					(((pentry->end_sec & 0xc0) << 8) + pentry->end_cyl);

				if (end <= start ||
					pentry->lba_start >= Capacity ||
					pentry->lba_length > Capacity ||
					pentry->lba_start + pentry->lba_length > Capacity) {
					break;
				}
			}
		}
		while (++idx < 4);
	}

	VdkZeroMem(&pitem, sizeof(pitem));

	pitem.length = Capacity;
	strcpy(pitem.fsname, "<disk>");

	if (idx < 4) {
		pitem.type = VdkIdentifyFS(DiskInfo, hFile, &pitem);
		(*CallBack)(&pitem, Param);
		return VDK_OK;
	}

	//
	//	callback for disk itself
	//
	(*CallBack)(&pitem, Param);

	//
	//	Process primary partitions
	//
	ext_offset = 0;

	for (idx = 0; idx < 4; idx++) {
		pentry = &(ptable.p.partition[idx]);

		if (pentry->type) {
			if (IS_EXTENDED(pentry->type)) {
				ext_offset = pentry->lba_start;
			}
			else {
				pitem.idx++;
				pitem.num		= idx + 1;
				pitem.type		= pentry->type;
				pitem.offset	= pentry->lba_start;
				pitem.length	= pentry->lba_length;
				pitem.fsname[0]	= '\0';
				pitem.label[0]	= '\0';

				VdkIdentifyFS(DiskInfo, hFile, &pitem);
				(*CallBack)(&pitem, Param);
			}
		}
	}

	if (!ext_offset) {
		return VDK_OK;
	}

	//
	//	Process extended partitions
	//
	logical_offset = 0;
	logical_num = 5;

	for (;;) {

		//
		//	Read extended partition PBR
		//
		if (DiskInfo) {
			ret = VdkReadSector(
				DiskInfo,
				ext_offset + logical_offset,
				1,
				(PUCHAR)&ptable);
		}
		else {
			ret = VdkReadFileAt(
				hFile,
				(INT64)(ext_offset + logical_offset) << VDK_BYTE_SHIFT_TO_SECTOR,
				&ptable,
				VDK_BYTES_PER_SECTOR,
				NULL);
		}

		if (ret != VDK_OK) {
			return ret;
		}

		if (ptable.p.signature != SIGNATURE_WORD) {
			return VDK_DATA;
		}

		//
		//	Process logical partition
		//
		for (idx = 0; idx < 4; idx++) {
			pentry = &(ptable.p.partition[idx]);

			if (pentry->type && !IS_EXTENDED(pentry->type)) {
				pitem.idx++;
				pitem.num		= logical_num++;
				pitem.type		= pentry->type;
				pitem.offset	= pentry->lba_start + ext_offset + logical_offset;
				pitem.length	= pentry->lba_length;
				pitem.fsname[0]	= '\0';
				pitem.label[0]	= '\0';

				VdkIdentifyFS(DiskInfo, hFile, &pitem);
				(*CallBack)(&pitem, Param);

				break;
			}
		}

		//
		//	Search next extended partition
		//
		for (idx = 0; idx < 4; idx++) {
			if (IS_EXTENDED(ptable.p.partition[idx].type)) {
				logical_offset = ptable.p.partition[idx].lba_start;
				break;
			}
		}

		if (idx >= 4) {
			// No more extended partition
			break;
		}
	}

	return VDK_OK;
}

static const struct _PARTITION_NAME {
	ULONG type;
	PCHAR name;
}
p_names[] = {
	{ 0x00,	"(none)"			},
	{ 0x01,	"FAT12"				},
	{ 0x02,	"XENIX root"		},
	{ 0x03,	"XENIX /usr"		},
	{ 0x04,	"FAT16"				},
	{ 0x05,	"Extended"			},
	{ 0x06,	"FAT16 HUGE"		},
	{ 0x07,	"HPFS/NTFS"			},
	{ 0x08,	"AIX boot"			},
	{ 0x09,	"AIX data"			},
	{ 0x0a,	"OS/2 Boot Manager"	},
	{ 0x0b,	"FAT32"				},
	{ 0x0c,	"FAT32 LBA"			},
	{ 0x0e,	"FAT16 LBA"			},
	{ 0x0f,	"Extended LBA"		},

	{ 0x10,	"OPUS"				},
	{ 0x11,	"FAT12 hidden"		},
	{ 0x12,	"Compaq Diag"		},
	{ 0x13,	"B-right/V"			},
	{ 0x14,	"FAT16 hidden"		},
	{ 0x16,	"FAT16 HUGE hidden"	},
	{ 0x17,	"HPFS/NTFS hidden"	},
	{ 0x18,	"AST SmartSleep"	},
	{ 0x19,	"Willowtech"		},
	{ 0x1b,	"FAT32 hidden"		},
	{ 0x1c,	"FAT32 LBA hidden"	},
	{ 0x1e,	"FAT16 LBA hidden"	},

	{ 0x20,	"Willowsoft OFS1"	},
	{ 0x21,	"FSo2"				},
	{ 0x24,	"NEC DOS 3.x"		},

	{ 0x38,	"Theos"				},
	{ 0x39,	"Plan 9"			},
	{ 0x3c,	"PartitionMagic"	},

	{ 0x40,	"Venix 80286"		},
	{ 0x41,	"PPC PReP boot"		},
	{ 0x42,	"Dynamic Disk"		},
	{ 0x45,	"EUMEL/Elan"		},
	{ 0x46,	"EUMEL/Elan"		},
	{ 0x47,	"EUMEL/Elan"		},
	{ 0x48,	"EUMEL/Elan"		},
	{ 0x4d,	"QNX4.x"			},
	{ 0x4e,	"QNX4.x 2nd"		},
	{ 0x4f,	"QNX4.x 3rd"		},

	{ 0x50,	"OnTrack DM"		},
	{ 0x51,	"OnTrack DM"		},
	{ 0x52,	"CP/M"				},
	{ 0x53,	"OnTrack DM"		},
	{ 0x54,	"OnTrack DM"		},
	{ 0x55,	"EZ-Drive"			},
	{ 0x56,	"GoldenBow"			},
	{ 0x5c,	"Priam Edisk"		},

	{ 0x61,	"SpeedStor"			},
	{ 0x63,	"GNU HURD"			},
	{ 0x64,	"Netware 286"		},
	{ 0x65,	"Netware 386"		},

	{ 0x70,	"DiskSecure"		},
	{ 0x75,	"PC/IX"				},
	{ 0x7e,	"F.I.X."			},

	{ 0x80,	"Old Minix"			},
	{ 0x81,	"Minix/Old Linux"	},
	{ 0x82,	"Linux swap"		},
	{ 0x83,	"Linux"				},
	{ 0x84,	"NTFT FAT16"		},
	{ 0x85,	"Linux Extended"	},
	{ 0x86,	"NTFT FAT16 HUGE"	},
	{ 0x87,	"NTFT NTFS"			},
	{ 0x8b,	"NTFT FAT32"		},
	{ 0x8c,	"NTFT FAT32X"		},
	{ 0x8e,	"NTFT FAT16X"		},

	{ 0x93,	"Amoeba"			},
	{ 0x94,	"Amoeba BBT"		},
	{ 0x99,	"Mylex EISA SCSI"	},
	{ 0x9f,	"BSD/OS"			},

	{ 0xa0,	"Suspend"			},
	{ 0xa5,	"FreeBSD"			},
	{ 0xa6,	"OpenBSD"			},
	{ 0xa7,	"NeXTSTEP"			},
	{ 0xa9,	"NetBSD"			},

	{ 0xb7,	"BSDI fs"			},
	{ 0xb8,	"BSDI swap"			},
	{ 0xbb,	"Boot Wizard (hid)"	},
	{ 0xbe,	"Solaris boot"		},

	{ 0xc0,	"Novell DOS/sec"	},
	{ 0xc1,	"DRDOS FAT12"		},
	{ 0xc4,	"DRDOS FAT16"		},
	{ 0xc6,	"DRDOS FAT16 HUGE"	},
	{ 0xc7,	"Syrinx"			},
	{ 0xcb,	"DRDOS FAT32"		},
	{ 0xcc,	"DRDOS FAT32X"		},
	{ 0xce,	"DRDOS FAT16X"		},

	{ 0xd0,	"MDOS FAT12"		},
	{ 0xd1,	"MDOS FAT12"		},
	{ 0xd4,	"MDOS FAT16"		},
	{ 0xd5,	"MDOS Extended"		},
	{ 0xd6,	"MDOS FAT16 HUGE"	},
	{ 0xd8,	"CP/M"				},
	{ 0xda,	"Non-FS data"		},
	{ 0xdb,	"CP/M"				},
	{ 0xde,	"Dell Utility"		},
	{ 0xdf,	"BootIt"			},

	{ 0xe1,	"DOS access"		},
	{ 0xe2,	"DOS R/O"			},
	{ 0xe3,	"DOS R/O"			},
	{ 0xe4,	"SpeedStor"			},
	{ 0xeb,	"BeOS BFS"			},
	{ 0xee,	"EFI GPT"			},
	{ 0xef,	"EFI FAT"			},

	{ 0xf0,	"PA-RISC boot"		},
	{ 0xf1,	"SpeedStor"			},
	{ 0xf2,	"DOS secondary"		},
	{ 0xf4,	"SpeedStor"			},
	{ 0xf5,	"Prologue"			},
	{ 0xfd,	"Linux RAID"		},
	{ 0xfe,	"LANstep"			},
	{ 0xff,	"Xenix BBT"			},

	{ 0, 0 }
};


const PCHAR GetPartitionTypeName(ULONG type)
{
	const struct _PARTITION_NAME *p = p_names;

	while (p->name && p->type != type) {
		p++;
	}

	return p->name;
}

#ifdef _WIN32
//
//	Decide if a partition is mountable or not
//	NTFT partitions are not mountable because
//	VDK driver does not communicate with the
//	Windows Mount Manager
//
BOOL IsPartitionMountable(ULONG type, BOOL read_only)
{
	switch (type) {
	case 0x01:				// DOS FAT12
	case 0x04:				// DOS FAT16
	case 0x06:				// DOS FAT16 HUGE
	case 0x0e:				// DOS FAT16X
		return TRUE;

	case 0x07:				// HPFS, NTFS
		if (read_only) {
			OSVERSIONINFO os = { sizeof(os) };

			if (GetVersionEx(&os)) {
				if (os.dwPlatformId == VER_PLATFORM_WIN32_NT &&
					os.dwMajorVersion >= 5 &&
					os.dwMinorVersion >= 1) {

					// XP and later can mount NTFS read-only.
					return TRUE;
				}
			}

			return FALSE;
		}
		else {
			return TRUE;
		}

	case 0x0b:				// DOS FAT32
	case 0x0c:				// DOS FAT32X
		// Windows NT 4.0 usually cannot mount FAT32 partition
		return ((GetVersion() & 0xFF) >= 5);

	default:
		return FALSE;
	}
}
#endif	// _WIN32
