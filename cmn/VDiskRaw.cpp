/*
	VDiskRaw.cpp

	Multiple file flat image class
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

#include "VDiskRaw.h"
#include "VDiskExtRaw.h"
#include "VDiskUtil.h"
#include "VDiskFile.h"

//
//	Initialize instance from a descriptor file
//
VDKSTAT VDiskRaw::Initialize(PCHAR pPath)
{
	VDiskFile file;
	CHAR	buf[MAX_PATH + 40];
	PCHAR	current;
	VDKSTAT	ret;

	//
	//	parameter check
	//
	if (!pPath || !*pPath) {
		return VDK_PARAM;
	}

	//
	//	store path
	//
	if ((ret = StorePath(pPath)) != VDK_OK) {
		return ret;
	}

	//
	//	open file
	//
	if ((ret = file.Open(pPath)) != VDK_OK) {
		return ret;
	}

	//
	//	initialize members
	//
	m_nCapacity		= 0;

	while ((ret = file.ReadText(buf, sizeof(buf), NULL)) == VDK_OK) {

		//
		//	replace tabs with blanks
		//
		current = buf;

		while (*current) {
			if (*current == '\t') {
				*current = ' ';
			}
			current++;
		}

		//
		//	remove trailing blanks
		//
		while (current > buf && *(--current) == ' ') {
			*current = '\0';
		}

		//
		//	skip leading blanks
		//
		current = buf;

		while (*current == ' ') {
			current++;
		}

		//
		//	blank or comment line?
		//
		if (*current && *current != '#') {

			VDiskExtRaw *ext;
			HANDLE	hFile;

			//	search the target file

			ret = VDiskSearchFile(&hFile, current, m_pPath);

			if (ret != VDK_OK) {
				return ret;
			}

			//	create an extent object

			ext = new VDiskExtRaw;

			if (ext == NULL) {
				VdkCloseFile(hFile);
				return VdkLastError();
			}

			ret = AddExtent(ext);

			if (ret != VDK_OK) {
				VdkCloseFile(hFile);
				delete ext;
				return ret;
			}

			//	initialize the extent object

			ret = ext->SetPath(current);

			if (ret != VDK_OK) {
				VdkCloseFile(hFile);
				return ret;
			}

			ret = ext->Load(hFile);

			VdkCloseFile(hFile);

			if (ret != VDK_OK) {
				return ret;
			}
		}
	}

	file.Close();

	if (ret == VDK_EOF) {
		ret = Check();
	}

	return ret;
}

//
//	Initialize as a child disk
//
VDKSTAT VDiskRaw::InitChild(
	ULONG		flags,
	PCHAR		pPath,
	ULONG		version,
	VDisk		*parent)
{
	UNREFERENCED_PARAMETER(flags);
	UNREFERENCED_PARAMETER(pPath);
	UNREFERENCED_PARAMETER(version);
	UNREFERENCED_PARAMETER(parent);

	//
	//	Raw Disk cannot be a child
	//
	return VDK_FUNCTION;
}

//
//	Check paramters
//
VDKSTAT VDiskRaw::Check()
{
	ULONG idx;
	VDKSTAT ret;

	//
	//	At least one extent must be present
	//
	if (!m_nExtents) {
		CHAR path[MAX_PATH];
		PVOID param = path;

		FullPath(path);

		VDiskCallBack(VDISK_CB_EMPTY_IMAGE, &param);

		return VDK_DATA;
	}

	//
	//	Check each extent file
	//
	m_nCapacity = 0;

	for (idx = 0; idx < m_nExtents; idx++) {
		ret = m_ppExtents[idx]->Check();

		if (ret != VDK_OK) {
			return ret;
		}

		if (m_ppExtents[idx]->IsModified()) {
			SetFlag(VDISK_FLAG_DIRTY);
		}

		m_nCapacity += m_ppExtents[idx]->GetCapacity();
	}

	return VDK_OK;
}

//
//	Create a new image -- currently not supported
//
VDKSTAT VDiskRaw::Create(ULONG flags)
{
	UNREFERENCED_PARAMETER(flags);

	return VDK_FUNCTION;
}

//
//	Create a new extent object for this instance
//
VDiskExt *VDiskRaw::NewExtent()
{
	return new VDiskExtRaw;
}

//
//	Get path for each extent files
//
void VDiskRaw::GetExtentPath(
	PCHAR pPath, ULONG nSeq)
{
	sprintf(pPath, "%s" PATH_SEPARATOR_STR "%s%lu.%s",
		m_pPath, m_pBody, nSeq + 1, m_pExtension);
}
