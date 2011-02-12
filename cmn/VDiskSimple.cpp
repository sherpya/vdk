/*
	VDiskSimple.cpp

	Single file flat image class
	Copyright (c) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

#include "VDiskSimple.h"
#include "VDiskExtRaw.h"
#include "VDiskUtil.h"

//
//	Initialize instance from a generic flat sector image file
//
VDKSTAT VDiskSimple::Initialize(PCHAR pPath)
{
	VDiskExtRaw	*ext;
	CHAR	path[MAX_PATH];
	HANDLE	hFile;
	VDKSTAT	ret;

	//	check parameter

	if (!pPath || !*pPath) {
		return VDK_PARAM;
	}

	//	store path

	if ((ret = StorePath(pPath)) != VDK_OK) {
		return ret;
	}

	//
	//	Set fixed parameters
	//
	m_nVMwareVer	= 0;
	m_nHardwareVer	= 0;
	m_nFlags		= VDISK_FLAG_SINGLE;

	//
	//	create an extent object
	//
	ext = new VDiskExtRaw;

	if (ext == NULL) {
		return VdkLastError();
	}

	ret = AddExtent(ext);

	if (ret != VDK_OK) {
		delete ext;
		return ret;
	}

	GetExtentPath(path, 0);

	ret = ext->SetPath(path);

	if (ret != VDK_OK) {
		return ret;
	}

	ret = VdkOpenFile(&hFile, path, strlen(path), TRUE);

	if (ret != VDK_OK) {
		return ret;
	}

	ret = ext->Load(hFile);

	VdkCloseFile(hFile);

	if (ret != VDK_OK) {
		return ret;
	}

	//
	//	check parameter integrity
	//
	return Check();
}

//
//	Initialize as a child of given parent
//
VDKSTAT VDiskSimple::InitChild(
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
	//	Simple disk cannot be a child
	//
	return VDK_FUNCTION;
}

//
//	Check parameter integrity
//
VDKSTAT VDiskSimple::Check()
{
	VDKSTAT ret;

	if (!m_nExtents || !m_ppExtents || !m_ppExtents[0]) {
		return VDK_FUNCTION;
	}

	ret = m_ppExtents[0]->Check();

	if (ret == VDK_OK) {
		if (m_ppExtents[0]->IsModified()) {
			SetFlag(VDISK_FLAG_DIRTY);
		}
		m_nCapacity = m_ppExtents[0]->GetCapacity();
	}

	return ret;
}

//
//	create generic flat sector image
//
VDKSTAT VDiskSimple::Create(ULONG flags)
{
	if (!m_nExtents || !m_ppExtents || !m_ppExtents[0]) {
		return VDK_FUNCTION;
	}

	return m_ppExtents[0]->Create(flags);
}

//
//	Create a new extent suitable for this instance
//
VDiskExt *VDiskSimple::NewExtent()
{
	return new VDiskExtRaw;
}

//
//	Get path for each extent files
//
void VDiskSimple::GetExtentPath(
	PCHAR pPath, ULONG nSeq)
{
	UNREFERENCED_PARAMETER(nSeq);
	sprintf(pPath, "%s" PATH_SEPARATOR_STR "%s.%s", m_pPath, m_pBody, m_pExtension);
}
