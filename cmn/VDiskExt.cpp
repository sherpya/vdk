/*
	VDiskExt.cpp

	Virtual Disk Extent base class
	Copyright (c) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

#include "VDisk.h"
#include "VDiskExt.h"


//
//	constructor
//
VDiskExt::VDiskExt()
{
	m_pFullPath	= NULL;
	m_pFileName	= NULL;

	m_nCapacity	= 0;
	m_nFileSize = 0;
	m_nFileAttr	= 0;

	m_bModified = FALSE;
}

//
//	destructor
//
VDiskExt::~VDiskExt()
{
	if (m_pFullPath) {
		delete[] m_pFullPath;
	}
}

//
//	allocate and store initial path
//
VDKSTAT VDiskExt::SetPath(PCHAR sPath)
{
	VDKSTAT ret;
	CHAR path[MAX_PATH];

	//
	//	remove previous path
	//
	if (m_pFullPath) {
		if (!VdkCmpNoCase(m_pFullPath, sPath)) {
			return VDK_OK;
		}

		delete[] m_pFullPath;
	}

	m_pFullPath	= NULL;
	m_pFileName	= NULL;

	//
	//	Get full path of the given path
	//
	ret = VdkFullPath(path, sizeof(path), sPath);

	if (ret != VDK_OK) {
		return ret;
	}

	if ((m_pFullPath = new CHAR[strlen(path) + 1]) == NULL) {
		return VdkLastError();
	}

	strcpy(m_pFullPath, path);

	//
	//	search beginning of file name
	//
	m_pFileName = m_pFullPath + strlen(m_pFullPath);

	while (m_pFileName > m_pFullPath &&
		*(m_pFileName - 1) != PATH_SEPARATOR_CHAR &&
		*(m_pFileName - 1) != ALT_SEPARATOR_CHAR) {
		m_pFileName--;
	}

	return VDK_OK;
}

