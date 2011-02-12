/*
	VDiskExtRaw.cpp

	Raw image file class
	Copyright (c) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

#ifdef USE_SPARSE_FILE
#include "SparseFile.h"
#endif

#include "VDisk.h"
#include "VDiskExtRaw.h"
#include "VDiskUtil.h"

//
//	Constructor
//
VDiskExtRaw::VDiskExtRaw()
{
	m_nStartOffset	= 0;
	m_nBackOffset	= 0;
}

//
//	Destructor
//
VDiskExtRaw::~VDiskExtRaw()
{
}

//
//	Load and obtain parameters from extent file
//
VDKSTAT VDiskExtRaw::Load(HANDLE hFile)
{
	VDKSTAT stat = VDK_OK;

	m_nFileAttr = VdkGetAttribute(m_pFullPath);

	if (m_nFileAttr == (ULONG)INVALID_FILE_ATTRIBUTES) {
		m_nFileAttr = 0;
		return VdkLastError();
	}

	stat = VdkGetFileSize(hFile, &m_nFileSize);

	if (stat != VDK_OK) {
		return stat;
	}

	m_nCapacity = (ULONG)(m_nFileSize >> VDK_BYTE_SHIFT_TO_SECTOR);

	return VDK_OK;
}

//
//	Check parameter consistency
//
VDKSTAT VDiskExtRaw::Check()
{
	PVOID cbparams[3];

	cbparams[0] = m_pFullPath;

	//
	//	check file size boundary
	//
	if (m_nFileSize & VDK_SECTOR_ALIGNMENT_MASK) {

		cbparams[1] = &m_nFileSize;

		if (!VDiskCallBack(VDISK_CB_SIZE_BOUNDARY, cbparams)) {
			return VDK_CANCEL;
		}

		SetModify();
	}

	//
	//	check file size against logical capacity
	//
	if ((ULONG)(m_nFileSize >> VDK_BYTE_SHIFT_TO_SECTOR) !=
		m_nCapacity + m_nBackOffset) {

		ULONG reply;

		cbparams[1] = (PVOID)(m_nCapacity + m_nBackOffset);
		cbparams[2] = (PVOID)(m_nFileSize >> VDK_BYTE_SHIFT_TO_SECTOR);

		reply = VDiskCallBack(VDISK_CB_EXT_FILESIZE, cbparams);

		if (reply <= m_nBackOffset) {
			return VDK_CANCEL;
		}

		m_nCapacity = reply - m_nBackOffset;
		SetModify();
	}

	return VDK_OK;
}

//
//	Update extent file
//
VDKSTAT VDiskExtRaw::Update()
{
	HANDLE	hFile;
	INT64	logical_size;
	VDKSTAT	ret;

	if (!IsModified()) {
		return VDK_OK;
	}

	//
	//	logical size is differenct from actual size?
	//
	logical_size =
		(INT64)(m_nCapacity + m_nBackOffset) <<
		VDK_BYTE_SHIFT_TO_SECTOR;

	if (logical_size == m_nFileSize) {
		ClrModify();
		return VDK_OK;
	}

	//
	//	adjust actual size
	//
	ret = VdkOpenFile(
		&hFile, m_pFullPath, strlen(m_pFullPath), FALSE);

	if (ret != VDK_OK) {
		return ret;
	}

	ret = VdkSetFileSize(hFile, logical_size);

	VdkCloseFile(hFile);

	if (ret == VDK_OK) {
		m_nFileSize = logical_size;
	}

	//	clear modified flag

	ClrModify();

	return ret;
}

//
//	Create actural extent file
//
VDKSTAT VDiskExtRaw::Create(ULONG flags)
{
	VDKSTAT ret;
	INT64	size;

	size = (INT64)(m_nCapacity + m_nBackOffset) <<
		VDK_BYTE_SHIFT_TO_SECTOR;

	if (!size || !m_pFullPath) {
		return VDK_FUNCTION;
	}

#ifdef USE_SPARSE_FILE
	if (flags & VDISK_CREATE_SPARSE) {
		ret = ::CreateSparse(
			m_pFullPath, size, flags & VDISK_CREATE_FORCE);
	}
	else
#endif
	{
		HANDLE hFile;

		//
		//	Create file
		//
		ret = VdkCreateFile(
			&hFile, m_pFullPath, (flags & VDISK_CREATE_FORCE));

		if (ret != VDK_OK) {
			return ret;
		}

		ret = VdkSetFileSize(hFile, size);

		VdkCloseFile(hFile);
	}

	if (ret == VDK_OK) {
		m_nFileSize = size;
	}

	return ret;
}

