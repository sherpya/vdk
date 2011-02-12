/*
	VDiskUtil.cpp

	Virtual Disk utility functions
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkfile.h"

#include "cowdisk.h"
#include "vmdisk.h"

#include "VDisk.h"
#include "VDiskPlain.h"
#include "VDiskCowd.h"
#include "VDiskVmdk.h"
#include "VDiskSimple.h"
#include "VDiskRaw.h"
#include "VDiskExt.h"
#include "VDiskUtil.h"


//
//	CallBack function pointer and default CallBack
//
static ULONG DefCallBack(ULONG reason, PVOID *params)
{
	UNREFERENCED_PARAMETER(reason);
	UNREFERENCED_PARAMETER(params);
	return 0;
}

VDISK_CALLBACK VDiskCallBack = DefCallBack;

void VDiskSetCallBack(VDISK_CALLBACK cb)
{
	VDiskCallBack = cb ? cb : DefCallBack;
}

//
//	Initialize from an existing virtual disk file
//
VDKSTAT VDiskLoadFile(
	PVOID *ppDisk,
	PCHAR pPath,
	PCHAR pBase)
{
	VDisk	*pDisk;
	UCHAR	buf[512];
	CHAR	path[MAX_PATH];
	HANDLE	hFile;
	INT64	size;
	ULONG_PTR len;
	PCHAR	p;
	VDKSTAT	ret;

	if (!ppDisk || !pPath || !*pPath) {
		return VDK_PARAM;
	}

	*ppDisk = pDisk = NULL;

	//
	// Open the file
	//
	strcpy(path, pPath);

	ret = VDiskSearchFile(&hFile, path, pBase);

	if (ret != VDK_OK) {
		return ret;
	}

	//
	//	read first 512 bytes of the file
	//
	VdkZeroMem(buf, sizeof(buf));

	ret = VdkReadFileAt(hFile, 0, buf, sizeof(buf), &len);

	if (ret != VDK_OK) {
		VdkCloseFile(hFile);
		return ret;
	}

	//
	//	get file size
	//
	ret = VdkGetFileSize(hFile, &size);

	if (ret != VDK_OK) {
		VdkCloseFile(hFile);
		return ret;
	}

	VdkCloseFile(hFile);

	//
	//	Identify the file type
	//
	if (*(PULONG)buf == COWD_SIGNATURE) {

		//	VMware 2 or 3 virtual disk

		if ((pDisk = new VDiskCowd) == NULL) {
			return VdkLastError();
		}

		goto initialize;
	}

	if (*(PULONG)buf == VMDK_SIGNATURE) {

		//	VMware 4 virtual disk extent file

		if ((pDisk = new VDiskVmdk) == NULL) {
			return VdkLastError();
		}

		goto initialize;
	}

	//
	//	does the file contain control character?
	//
	while (len--) {
		if (buf[len] <= 0x08 ||
			buf[len] == 0x0b ||
			buf[len] == 0x0c ||
			(buf[len] >= 0x0e && buf[len] <= 0x1f) ||
			buf[len] == 0x7f) {

			//	contains a control character
			//	-- cannot be a descriptor file

			goto simple_disk;
		}
	}

	//	does not contain a control character
	//	-- one of descriptor file types

	p = (PCHAR)buf;

	buf[sizeof(buf) - 1] = '\0';

	while (*p) {

		while (*p == ' ' || *p == '\t') {
			p++;
		}

		if (!VdkCmpNoCaseN(p, "#vm|TOOLSVERSION",	16) ||
			!VdkCmpNoCaseN(p, "#vm|VERSION",		11) ||
			!VdkCmpNoCaseN(p, "DRIVETYPE",			 9) ||
			!VdkCmpNoCaseN(p, "CYLINDERS",			 9) ||
			!VdkCmpNoCaseN(p, "HEADS",				 5) ||
			!VdkCmpNoCaseN(p, "SECTORS",			 7) ||
			!VdkCmpNoCaseN(p, "ACCESS",				 6)) {

			//
			//	The file has VMware 2 plain disk entry
			//
			if ((pDisk = new VDiskPlain) == NULL) {
				return VdkLastError();
			}

			goto initialize;
		}
		else if (
			!VdkCmpNoCaseN(p, "# Disk DescriptorFile",	21) ||
			!VdkCmpNoCaseN(p, "version",				 7) ||
			!VdkCmpNoCaseN(p, "CID",					 3) ||
			!VdkCmpNoCaseN(p, "parentCID",				 9) ||
			!VdkCmpNoCaseN(p, "createType",				10) ||
			!VdkCmpNoCaseN(p, "parentFileNameHint",		18) ||
			!VdkCmpNoCaseN(p, "# Extent description",	20) ||
			!VdkCmpNoCaseN(p, "RW",						 2) ||
			!VdkCmpNoCaseN(p, "# The Disk Data Base",	20) ||
			!VdkCmpNoCaseN(p, "#DDB",					 4) ||
			!VdkCmpNoCaseN(p, "ddb.geometry.sectors",	20) ||
			!VdkCmpNoCaseN(p, "ddb.geometry.heads",		18) ||
			!VdkCmpNoCaseN(p, "ddb.geometry.cylinders",	22) ||
			!VdkCmpNoCaseN(p, "ddb.adapterType",		15) ||
			!VdkCmpNoCaseN(p, "ddb.virtualHWVersion",	20) ||
			!VdkCmpNoCaseN(p, "ddb.toolsVersion",		16)) {

			//
			//	The file has VMware 4 descriptor file entry
			//
			if ((pDisk = new VDiskVmdk) == NULL) {
				return VdkLastError();
			}

			goto initialize;
		}

		//
		//	skip to next line
		//
		while (*(p++) != '\n')
			;
	}

	//
	//	not a VMware descriptor file
	//	-- raw descriptor
	//
	if ((pDisk = new VDiskRaw) == NULL) {
		return VdkLastError();
	}

	goto initialize;


simple_disk:

	//
	//	None of above file type
	//
	{
		PVOID param = path;

		if (!VDiskCallBack(VDISK_CB_FILE_TYPE, &param)) {
			return VDK_CANCEL;
		}
	}

	//
	//	generic flat sector image file
	//
	if ((pDisk = new VDiskSimple) == NULL) {
		return VdkLastError();
	}


initialize:
	ret = pDisk->Initialize(path);

#ifdef VDK_DEBUG
	pDisk->Dump();
#endif

	*ppDisk = pDisk;

	return ret;
}

//
//	Create a virtual disk tree
//
VDKSTAT VDiskCreateTree(PVOID pDisk)
{
	return ((VDisk *)pDisk)->CreateTree();
}

//
//	Create a REDO log for a virtual disk
//
VDKSTAT VDiskCreateRedo(PVOID *ppDisk, PSTR pUndoPath)
{
	VDKSTAT ret;
	VDisk *disk, *redo;
	CHAR path[MAX_PATH];

	disk = (VDisk *)*ppDisk;

	sprintf(path, "%s" PATH_SEPARATOR_STR "%s.%s.REDO",
		pUndoPath ? pUndoPath : disk->GetPath(),
		disk->GetBody(), disk->GetExt());

	if (disk->GetVMwareVer() >= 4) {
		redo = new VDiskVmdk;
	}
	else {
		redo = new VDiskCowd;
	}

	if (!redo) {
		return VdkLastError();
	}

	ret = redo->InitChild(
		pUndoPath
		? (VDISK_FLAG_SPARSE | VDISK_FLAG_CHILD | VDISK_FLAG_ABSPATH)
		: (VDISK_FLAG_SPARSE | VDISK_FLAG_CHILD),
		path, disk->GetVMwareVer(), disk);

	if (ret != VDK_OK) {
		delete redo;
		return ret;
	}

	ret = redo->Create(0);

	if (ret != VDK_OK) {
		delete redo;
		return ret;
	}

	*ppDisk = redo;

	return VDK_OK;
}

//
//	Delete VDisk Tree
//
VOID VDiskDelete(PVOID pDisk)
{
	if (pDisk) {
		((VDisk *)pDisk)->DeleteTree();
	}
}


//
//	Search file
//
static PSTR pSearchPath = NULL;

VOID VDiskSetSearchPath(PSTR pPath)
{
	pSearchPath = pPath;
}

VDKSTAT VDiskSearchFile(HANDLE *pFile, PCHAR pPath, PCHAR pBase)
{
	CHAR	path[MAX_PATH];
	VDKSTAT	ret;

	//
	//	remove leading ".\"
	//
	while (*pPath == '.' && (
		*(pPath + 1) == ALT_SEPARATOR_CHAR ||
		*(pPath + 1) == PATH_SEPARATOR_CHAR)) {
		memmove(pPath, pPath + 2, strlen(pPath + 2) + 1);
	}

	//
	//	first try the search path (if given)
	//
	if (pSearchPath) {
		PSTR p = pPath + strlen(pPath);
		
		while (p > pPath &&
			*(p - 1) != PATH_SEPARATOR_CHAR &&
			*(p - 1) != ALT_SEPARATOR_CHAR) {

			p--;
		}
		
		sprintf(path, "%s" PATH_SEPARATOR_STR "%s", pSearchPath, p);

		if (VdkOpenFile(pFile, path, strlen(path), TRUE) == VDK_OK) {
			strcpy(pPath, path);
			return VDK_OK;
		}
	}

	//
	//	try to open with given path
	//
	ret = VdkOpenFile(pFile, pPath, strlen(pPath), TRUE);

	if (ret == VDK_OK) {
		return ret;
	}

	if ((ret == VDK_NOFILE || ret == VDK_NOPATH) && pBase) {

		//
		//	Try given base path
		//
		PCHAR	body;

		if ((isalpha(*pPath) && *(pPath + 1) == ':') ||
			*pPath == PATH_SEPARATOR_CHAR ||
			*pPath == ALT_SEPARATOR_CHAR) {

			//
			//	absolute path -- extract filename only
			//
			body = pPath + strlen(pPath);

			while (body > pPath &&
				*(body - 1) != PATH_SEPARATOR_CHAR &&
				*(body - 1) != ALT_SEPARATOR_CHAR) {

				body--;
			}
		}
		else {
			//
			//	relative path -- use as it is
			//
			body = pPath;
		}

		sprintf(path, "%s" PATH_SEPARATOR_STR "%s", pBase, body);

		//	shouldn't modify 'ret' variable here

		if (VdkOpenFile(pFile, path, strlen(path), TRUE) == VDK_OK) {
			strcpy(pPath, path);
			return VDK_OK;
		}
	}

	//
	//	ask user
	//
	strcpy(path, pPath);

	do {
		PVOID cbparams[2];
		cbparams[0] = path;
		cbparams[1] = (PVOID)ret;

		if (!VDiskCallBack(VDISK_CB_FILE_OPEN, cbparams)) {
			return ret;
		}

		ret = VdkOpenFile(pFile, path, strlen(path), TRUE);
	}
	while (ret != VDK_OK);

	strcpy(pPath, path);

	return ret;
}

//
//	Map VDisk attributes to VDK_OPEN_FILE_INFO
//
VDKSTAT VDiskMapToOpenInfo(
	PVOID	pDisk,
	PVOID	*ppInfo,
	PULONG	pInfoLen)
{
	PVDK_OPEN_FILE_INFO open_info;
	PVDK_OPEN_FILE_ITEM open_item;
	VDisk *disk;
	VDiskExt **ext;

	ULONG total_files;
	ULONG name_len;
	PCHAR name_pos;
	ULONG idx;

	//
	//	Parameter check
	//
	if (!pDisk || !ppInfo || !pInfoLen) {
		return VDK_PARAM;
	}

	//
	//	Find out necessary memory size
	//
	total_files = 0;
	name_len = 0;

	disk = (VDisk *)pDisk;

	if (disk->GetCylinders() &&
		disk->GetTracks() &&
		disk->GetSectors()) {

		if (disk->GetCylinders() *
			disk->GetTracks() *
			disk->GetSectors() > disk->GetCapacity()) {

			return VDK_PARAM;
		}
	}

	do {
		if (!disk->GetExtentCnt()) {
			return VDK_PARAM;
		}

		total_files += disk->GetExtentCnt();

		ext = disk->GetExtents();

		for (idx = 0; idx < disk->GetExtentCnt(); idx++) {

			if (!ext[idx]->GetCapacity() ||
				ext[idx]->GetCapacity() > disk->GetCapacity()) {
				return VDK_DATA;
			}

			if (ext[idx]->GetFileAttr() & VDK_INVALID_ATTRIBUTES) {
				return VDK_DATA;
			}

			name_len += strlen(ext[idx]->GetFullPath()) + 1;
		}

		disk = disk->GetParent();
	}
	while (disk);

	//
	//	allocate file information area
	//
	*pInfoLen = FIELD_OFFSET(VDK_OPEN_FILE_INFO, Files) +
		sizeof(VDK_OPEN_FILE_ITEM) * total_files + name_len;

	open_info = (PVDK_OPEN_FILE_INFO)VdkAllocMem(*pInfoLen);

	if (!open_info) {
		return VDK_NOMEMORY;
	}

	VdkZeroMem(open_info, *pInfoLen);

	//
	//	Copy file structure into VDK_OPEN_FILE_INFO
	//
	disk = (VDisk *)pDisk;

	open_info->Capacity		= disk->GetCapacity();
	open_info->Cylinders	= disk->GetCylinders();
	open_info->Tracks		= disk->GetTracks();
	open_info->Sectors		= disk->GetSectors();
	open_info->FilesTotal	= total_files;

	//
	//	copy each file information
	//
	open_item = open_info->Files;
	name_pos = (PCHAR)&(open_item[total_files]);

	do {
		ext = disk->GetExtents();

		for (idx = 0; idx < disk->GetExtentCnt(); idx++) {

			open_item->FileType = ext[idx]->GetFileType();
			open_item->Capacity = ext[idx]->GetCapacity();

			strcpy(name_pos, ext[idx]->GetFullPath());
			open_item->NameLength = strlen(name_pos) + 1;

			name_pos += open_item->NameLength;
			open_item++;
		}

		disk = disk->GetParent();
	}
	while (disk);

	*ppInfo = open_info;

	return VDK_OK;
}


PCHAR	VDiskGetDiskName(PVOID pDisk)
{
	return ((VDisk *)pDisk)->GetBody();
}
