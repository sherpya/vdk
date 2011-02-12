/*
	vdkclose.c

	Virtual Disk close functions
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
static VOID	VdkFreeCowdParam(
	PVDK_COWD_PARAM	CowdParam);

static VOID	VdkFreeVmdkParam(
	PVDK_VMDK_PARAM	VmdkParam);

//
//	declare pageable functions
//
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdkCloseDisk)
#pragma alloc_text(PAGE, VdkFreeCowdParam)
#pragma alloc_text(PAGE, VdkFreeVmdkParam)
#endif	// ALLOC_PRAGMA

//
// Close all files and release resources
//
VOID VdkCloseDisk(
	PVDK_DISK_INFO	DiskInfo)
{
	ULONG idx;

	if (DiskInfo) {
		if (DiskInfo->Files) {
			for (idx = 0; idx < DiskInfo->FilesTotal; idx++) {

				VDKTRACE(VDKCLOSE | VDKINFO, ("[VDK] Cleaning up File #%u\n", idx));

				if (DiskInfo->Files[idx].FileHandle) {
					VdkCloseFile(DiskInfo->Files[idx].FileHandle);
				}

				if (DiskInfo->Files[idx].FileType == VDK_FILETYPE_COWD) {
					VdkFreeCowdParam(DiskInfo->Files[idx].prm.cowd);
				}
				else if (DiskInfo->Files[idx].FileType == VDK_FILETYPE_VMDK) {
					VdkFreeVmdkParam(DiskInfo->Files[idx].prm.vmdk);
				}
			}

			VDKTRACE(VDKCLOSE | VDKINFO, ("[VDK] Freeing DiskInfo->Files\n"));

			VdkFreeMem(DiskInfo->Files);
		}

		if (DiskInfo->NameBuffer) {

			VDKTRACE(VDKCLOSE | VDKINFO, ("[VDK] Freeing FileNames buffer\n"));

			VdkFreeMem(DiskInfo->NameBuffer);
		}

		VdkZeroMem(DiskInfo, sizeof(VDK_DISK_INFO));
	}
}

//
//	Release COWD parameter
//
VOID VdkFreeCowdParam(
	PVDK_COWD_PARAM CowdParam)
{
	if (CowdParam) {

		VDKTRACE(VDKCLOSE | VDKINFO, ("[VDK] Freeing COWD param.\n"));

		if (CowdParam->PrimaryMap) {
			VdkFreeMem(CowdParam->PrimaryMap);
		}

		if (CowdParam->SecondaryMap) {
			VdkFreeMem(CowdParam->SecondaryMap);
		}

		if (CowdParam->Sector0) {
			VdkFreeMem(CowdParam->Sector0);
		}

		VdkFreeMem(CowdParam);
	}
}

//
//	Release VMDK parameter
//
VOID VdkFreeVmdkParam(
	PVDK_VMDK_PARAM VmdkParam)
{
	if (VmdkParam) {

		VDKTRACE(VDKCLOSE | VDKINFO, ("[VDK] Freeing VMDK param.\n"));

		if (VmdkParam->PrimaryDirectory) {
			VdkFreeMem(VmdkParam->PrimaryDirectory);
		}

		if (VmdkParam->BackupDirectory) {
			VdkFreeMem(VmdkParam->BackupDirectory);
		}

		if (VmdkParam->GrainTable) {
			VdkFreeMem(VmdkParam->GrainTable);
		}

		VdkFreeMem(VmdkParam);
	}
}

