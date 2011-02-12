/*
	VDiskCowd.h

	COWDisk class
	Copyright (c) 2003 Ken Kato
*/

#ifndef _VDISKCOWD_H_
#define _VDISKCOWD_H_

#include "VDisk.h"

//
//	Virtual Disk class
//
class VDiskCowd : public VDisk
{
public:
	//
	//	Constructor / Destructor
	//
	VDiskCowd();
	virtual ~VDiskCowd();

	virtual VDKSTAT	Create(ULONG flags);
	virtual VDKSTAT	Check();

protected:
	virtual VDKSTAT	Initialize(PCHAR pPath);

	virtual void	SetGeometry();
	virtual ULONG	DefaultExtSize();

	virtual VDiskExt *NewExtent();
	virtual void	GetExtentPath(PCHAR pBuf, ULONG nSeq);
};

#endif // _VDISKCOWD_H_
