/*
	VDiskVmdk.h

	VMDK class
	Copyright (c) 2003 Ken Kato
*/

#ifndef _VDISKVMDK_H_
#define _VDISKVMDK_H_

#include "VDisk.h"

enum _VDISK_VMDK_TYPE {
	VDISK_VMDK_NONE = 0,
	VDISK_VMDK_SPLIT_FLAT,
	VDISK_VMDK_MONO_FLAT,
	VDISK_VMDK_SPLIT_SPARSE,
	VDISK_VMDK_MONO_SPARSE
};

//
//	Virtual Disk class
//
class VDiskVmdk : public VDisk
{
public:
	//
	//	Constructor / Destructor
	//
	VDiskVmdk();
	virtual ~VDiskVmdk();

	virtual VDKSTAT	Create(ULONG flags);
	virtual VDKSTAT	Check();

protected:
	virtual VDKSTAT	Initialize(PCHAR pPath);

	virtual void	SetGeometry();
	virtual ULONG	DefaultExtSize();
	virtual void	SetDefaultTS();

	virtual VDiskExt *NewExtent();
	virtual void	GetExtentPath(PCHAR pBuf, ULONG nSeq);

	//
	//	VMDK specific members
	//
	VDKSTAT	WriteDescriptor(BOOL create, BOOL force);
};

#endif // _VDISKVMDK_H_
