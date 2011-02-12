/*
	VDiskExtRaw.h

	Raw extent class
	Copyright (c) 2003 Ken Kato
*/

#ifndef _VDISKEXTRAW_H_
#define _VDISKEXTRAW_H_

#include "VDiskExt.h"

class VDiskExtRaw : public VDiskExt
{
public:
	VDiskExtRaw();
	virtual ~VDiskExtRaw();

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
	virtual ULONG	GetFileType()	{ return VDK_FILETYPE_FLAT; }

	//
	//	class specific members
	//
public:
	void	SetStartOffset(ULONG val)	{ m_nStartOffset = val;		}
	ULONG	GetStartOffset()			{ return m_nStartOffset;	}

	void	SetBackOffset(ULONG val)	{ m_nBackOffset = val;		}
	ULONG	GetBackOffset()				{ return m_nBackOffset;		}

protected:
	//	extent offset for VMware 2.x plain disk
	ULONG	m_nStartOffset;

	//	backing offset for VMware 4.x flat virtual disk
	ULONG	m_nBackOffset;
};

#endif	// _VDISKEXTRAW_H_
