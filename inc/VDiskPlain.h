/*
	VDiskPlain.h

	VMware Plain disk class
	Copyright (c) 2003 Ken Kato
*/

#ifndef _VDISKPLAIN_H_
#define _VDISKPLAIN_H_

#include "VDisk.h"

//
//	Virtual Disk class
//
class VDiskPlain : public VDisk
{
public:
	//
	//	Constructor / Destructor
	//
	VDiskPlain();
	virtual ~VDiskPlain();

	virtual VDKSTAT	InitChild(
		ULONG		flags,
		PCHAR		pPath,
		ULONG		version,
		VDisk		*parent);

	virtual VDKSTAT	Create(ULONG flags);
	virtual VDKSTAT	Check();

protected:
	virtual VDKSTAT	Initialize(PCHAR pPath);

	virtual void	SetGeometry();
	virtual ULONG	DefaultExtSize();

	virtual VDiskExt *NewExtent();
	virtual void	GetExtentPath(PCHAR pBuf, ULONG nSeq);

	//
	//	Plain disc specific members
	//
	VDKSTAT	WriteDescriptor(BOOL create, BOOL force);
};

#endif // _VDISKPLAIN_H_
