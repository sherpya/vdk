/*
	VDiskRaw.h

	Multiple file flat image class
	Copyright (C) 2003 Ken Kato
*/

#ifndef _VDISKRAW_H_
#define _VDISKRAW_H_

#include "VDisk.h"

//
//	Virtual Disk class
//
class VDiskRaw : public VDisk
{
public:
	//
	//	Constructor / Destructor
	//
	VDiskRaw()			{}
	virtual ~VDiskRaw()	{}

	virtual VDKSTAT	InitChild(
		ULONG		flags,
		PCHAR		pPath,
		ULONG		version,
		VDisk		*parent);

	virtual VDKSTAT	Create(ULONG flags);
	virtual VDKSTAT	Check();

protected:
	virtual VDKSTAT	Initialize(PCHAR pPath);
	virtual VDiskExt *NewExtent();
	virtual void	GetExtentPath(PCHAR pBuf, ULONG nSeq);
};

#endif // _VDISKRAW_H_
