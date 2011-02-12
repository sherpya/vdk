/*
	VDiskExtCowd.h

	COWDisk extent class
	Copyright (c) 2003 Ken Kato
*/

#ifndef _VDISKEXTCOWD_H_
#define _VDISKEXTCOWD_H_

#include "VDiskExt.h"

class VDiskExtCowd : public VDiskExt
{
public:
	VDiskExtCowd();
	virtual ~VDiskExtCowd();

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
	virtual ULONG	GetFileType()	{ return VDK_FILETYPE_COWD; }

	//
	//	COWD specific members
	//
public:
	void CreateHeader(VDisk *pDisk, ULONG ordinal);

	PCOWD_SECTOR_0	GetSec0()	{ return &m_Sec0; }
	PCOWD_SECTOR_2	GetSec2()	{ return &m_Sec2; }
	PCOWD_SECTOR_3	GetSec3()	{ return &m_Sec3; }

protected:
	VDKSTAT UpdateFile(HANDLE hFile);

	COWD_SECTOR_0	m_Sec0;
	COWD_SECTOR_2	m_Sec2;
	COWD_SECTOR_3	m_Sec3;
};

#endif	// _VDISKEXTCOWD_H_
