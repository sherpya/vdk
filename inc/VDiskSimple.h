/*
	VDiskSimple.h

	simple file flat image class
	Copyright (c) 2003 Ken Kato
*/

#ifndef _VDISKSIMPLE_H_
#define _VDISKSIMPLE_H_

#include "VDisk.h"

//
//	Virtual Disk class
//
class VDiskSimple : public VDisk
{
public:
	//
	//	Constructor / Destructor
	//
	VDiskSimple()			{}
	virtual ~VDiskSimple()	{}

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

#endif // _VDISKSIMPLE_H_
