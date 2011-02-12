/*
	VDiskExt.h

	Virtual Disk Extent base class
	Copyright (c) 2003 Ken Kato
*/

#ifndef _VDISKEXT_H_
#define _VDISKEXT_H_

//
//	Logical virtual disk class
//
class VDisk;

//
//	Virutal Disk extent class
//
class VDiskExt
{
public:
	VDiskExt();
	virtual ~VDiskExt();

	//
	//	Load and obtain parameters from extent file
	//
	virtual VDKSTAT	Load(HANDLE hFile) = 0;

	//
	//	Check parameters
	//
	virtual VDKSTAT	Check() = 0;

	//
	//	Update extent file
	//
	virtual VDKSTAT	Update() = 0;

	//
	//	Create extent file
	//
	virtual VDKSTAT	Create(ULONG flags) = 0;

	//
	//	Returns extent type
	//
	virtual ULONG	GetFileType() = 0;

	//
	//	get attribute members
	//
	VDKSTAT	SetPath(PCHAR sPath);
	PCHAR	GetFullPath()			{ return m_pFullPath;	}
	PCHAR	GetFileName()			{ return m_pFileName;	}

	void	SetCapacity(ULONG val)	{ m_nCapacity = val;	}
	ULONG	GetCapacity()			{ return m_nCapacity;	}
	INT64	GetFileSize()			{ return m_nFileSize;	}
	ULONG	GetFileAttr()			{ return m_nFileAttr;	}

	BOOL	IsModified()			{ return m_bModified;	}
	void	SetModify()				{ m_bModified = TRUE;	}
	void	ClrModify()				{ m_bModified = FALSE;	}

protected:
	PCHAR	m_pFullPath;	// Full path of the file
	PCHAR	m_pFileName;	// Filename position in m_pFullPath

	ULONG	m_nCapacity;	// Logical capacity of the file (sectors)
	INT64	m_nFileSize;	// Physical file size (bytes)
	ULONG	m_nFileAttr;	// File attributes

	BOOL	m_bModified;
};

#endif // _VDISKEXT_H_
