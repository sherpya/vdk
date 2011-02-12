/*
	VDiskExtVmdk.h

	VMDK sparse extent class
	Copyright (c) 2003 Ken Kato
*/

#ifndef _VDISKEXTVMDK_H_
#define _VDISKEXTVMDK_H_

#include "VDiskExt.h"

class VDiskExtVmdk : public VDiskExt
{
public:
	VDiskExtVmdk();
	virtual ~VDiskExtVmdk();

	//
	//	Load and obtain parameters from extent file
	//
	virtual VDKSTAT	Load(HANDLE hFile);

	//
	//	Check parameter consistency
	//
	virtual VDKSTAT	Check();

	//
	//	Update extent header (cowd / vmdk sparse)
	//
	virtual VDKSTAT	Update();

	//
	//	Create actural extent file
	//
	virtual VDKSTAT	Create(ULONG flags);

	//
	//	Returns extent type
	//
	virtual ULONG	GetFileType()	{ return VDK_FILETYPE_VMDK; }

	//
	//	VMDK specific members
	//
public:
	//
	//	Get VMDK header
	//
	PVMDK_HEADER	GetHeader()	{ return &m_Header; }

protected:
	//
	//	Update header and actual file size
	//
	VDKSTAT UpdateFile(HANDLE hFile);

	//
	//	Write grain directory entries to the file
	//
	VDKSTAT WriteGrainDir(
		HANDLE	hFile,
		ULONG	offset,
		ULONG	gd_size,
		ULONG	gt_count,
		ULONG	gt_size);

	VMDK_HEADER		m_Header;
};

#endif	// _VDISKEXTVMDK_H_
