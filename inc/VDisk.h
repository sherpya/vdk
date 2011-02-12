/*
	VDisk.h

	Virtual Disk base class
	Copyright (c) 2003 Ken Kato
*/

#ifndef _VDISK_H_
#define _VDISK_H_

//
//	Virtual Disk Extent base class
//
class VDiskExt;

//
//	Virtual Disk class
//
class VDisk
{
public:
	//
	//	Constructor
	//
	VDisk();

	//
	//	Destructor
	//
	virtual ~VDisk();

	//
	//	Initialize from a file path
	//
	virtual VDKSTAT	Initialize(PCHAR pPath) = 0;

	//
	//	Initialize as a root with parameters
	//
	virtual VDKSTAT InitRoot(
		ULONG		flags,
		PCHAR		pPath,
		ULONG		version,
		ULONG		controller,
		ULONG		capacity);

	//
	//	Initialize as a child from a parent
	//
	virtual VDKSTAT InitChild(
		ULONG		flags,
		PCHAR		pPath,
		ULONG		version,
		VDisk		*parent);

	//
	//	Create ancestral tree
	//
	VDKSTAT CreateTree();

	//
	//	Delete including parents
	//
	void DeleteTree();

	//
	//	Create virtual disk files
	//
	virtual VDKSTAT	Create(ULONG flags) = 0;

	//
	//	Check parameter integrity
	//
	virtual VDKSTAT	Check() = 0;

	//
	//	Member access functions
	//
	PCHAR		GetPath()			{ return m_pPath;		}
	PCHAR		GetBody()			{ return m_pBody;		}
	PCHAR		GetExt()			{ return m_pExtension;	}
	void		FullPath(PCHAR buf)
	{
		sprintf(buf,
			"%s" PATH_SEPARATOR_STR "%s.%s",
			m_pPath, m_pBody, m_pExtension);
	}

	ULONG		GetVMwareVer()		{ return m_nVMwareVer;	}
	ULONG		GetFlags()			{ return m_nFlags;		}
	PCHAR		GetParentPath()		{ return m_pParentPath;	}
	VDisk		*GetParent()		{ return m_pParent;		}

	ULONG		GetCapacity()		{ return m_nCapacity;	}
	ULONG		GetCylinders()		{ return m_nCylinders;	}
	ULONG		GetTracks()			{ return m_nTracks;		}
	ULONG		GetSectors()		{ return m_nSectors;	}

	ULONG		GetParentTS()		{ return m_nParentTS;	}
	ULONG		GetTimeStamp()		{ return m_nTimeStamp;	}
	ULONG		GetController()		{ return m_nController;	}
	ULONG		GetHardwareVer()	{ return m_nHardwareVer;}
	ULONG		GetToolsFlag()		{ return m_nToolsFlag;	}

	ULONG		GetExtentCnt()		{ return m_nExtents;	}
	VDiskExt	**GetExtents()		{ return m_ppExtents;	}

	void	SetFlag(ULONG flag)		{ m_nFlags |= flag;		}
	void	ClrFlag(ULONG flag)		{ m_nFlags &= ~flag;	}

#ifdef VDK_DEBUG
	void	Dump();
#endif

protected:
	virtual void	SetDefaultTS();
	virtual void	SetGeometry();
	virtual ULONG	DefaultExtSize();

	//
	//	Create suitable extent object
	//
	virtual VDiskExt *NewExtent() = 0;

	//
	//	Create extent path
	//
	virtual void	GetExtentPath(PCHAR pBuf, ULONG nSeq) = 0;

	VDKSTAT		StorePath(PCHAR pPath);
	VDKSTAT		StoreParentPath	(PCHAR pPath);

	VDKSTAT		CreateExtents(ULONG ext_size);
	VDKSTAT		AddExtent(VDiskExt *ext);

protected:
	//
	// Data members
	//
	PCHAR		m_pPath;			//	Path part of filename
	PCHAR		m_pBody;			//	Filename body
	PCHAR		m_pExtension;		//	Filename extension
	ULONG		m_nVMwareVer;		//	2, 3 or 4
	ULONG		m_nFlags;

	PCHAR		m_pParentPath;
	VDisk		*m_pParent;

	//
	//	Virtual disk parameters
	//
	ULONG		m_nCapacity;
	ULONG		m_nCylinders;
	ULONG		m_nTracks;
	ULONG		m_nSectors;

	ULONG		m_nParentTS;
	ULONG		m_nTimeStamp;
	ULONG		m_nController;
	ULONG		m_nHardwareVer;
	ULONG		m_nToolsFlag;

	//
	//	Virtual disk extents
	//
	ULONG		m_nExtents;
	ULONG		m_nArraySize;

	VDiskExt	**m_ppExtents;
};

#endif // _VDISK_H_
