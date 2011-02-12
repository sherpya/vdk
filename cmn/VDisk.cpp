/*
	VDisk.cpp

	Virtual Disk base class
	Copyright (c) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

#include "cowdisk.h"
#include "vmdisk.h"

#include "VDisk.h"
#include "VDiskExt.h"
#include "VDiskUtil.h"

#include <time.h>

//
//	Constructor
//
VDisk::VDisk()
{
	m_pPath			= NULL;
	m_pBody			= NULL;
	m_pExtension	= NULL;
	m_nVMwareVer	= 0;
	m_nFlags		= 0;

	m_pParentPath	= NULL;
	m_pParent		= NULL;

	m_nParentTS		= 0;
	m_nTimeStamp	= 0;
	m_nController	= 0;
	m_nHardwareVer	= 0;
	m_nToolsFlag	= 0;

	m_nCapacity		= 0;
	m_nCylinders	= 0;
	m_nTracks		= 0;
	m_nSectors		= 0;

	m_nExtents		= 0;
	m_nArraySize	= 0;
	m_ppExtents		= NULL;
}

//
//	Destructor
//
VDisk::~VDisk()
{
	if (m_pPath) {
		delete[] m_pPath;
	}

	if (m_pParentPath) {
		delete[] m_pParentPath;
	}

	if (m_ppExtents) {
		while (m_nExtents) {
			delete *(m_ppExtents + (--m_nExtents));
		}

		delete[] m_ppExtents;
	}
}

//
//	Store full path of given path and
//	split it into path, filename, extension
//
VDKSTAT VDisk::StorePath(PCHAR pPath)
{
	CHAR	path[MAX_PATH];
	ULONG	len;
	VDKSTAT	ret;

	if (!pPath || !*pPath) {
		return VDK_PARAM;
	}

	ret = VdkFullPath(path, sizeof(path), pPath);

	if (ret != VDK_OK) {
		return ret;
	}

	len = strlen(path);

	//
	//	allocate memory and copy path
	//
	if (m_pPath) {
		delete[] m_pPath;
	}

	if ((m_pPath = new CHAR [len + 1]) == NULL) {
		return VdkLastError();
	}

	strcpy(m_pPath, path);

	//
	// look for the beginning of extension
	//
	m_pExtension = m_pPath + len;

	while (--m_pExtension > m_pPath &&
		*m_pExtension != PATH_SEPARATOR_CHAR &&
		*m_pExtension != ALT_SEPARATOR_CHAR) {

		if (*m_pExtension == '.') {
			if (m_nVMwareVer >= 3 && !VdkCmpNoCaseN(m_pExtension, ".redo", 5)) {
				//
				// Cowd REDO file has double extensions
				//
				PCHAR p = m_pExtension;

				while (--p > m_pPath &&
					*p != PATH_SEPARATOR_CHAR &&
					*p != ALT_SEPARATOR_CHAR) {

					if (!VdkCmpNoCaseN(p, ".vmdk.", 6)) {
						m_pExtension = p;
						break;
					}
				}
			}

			break;
		}
	}

	if (*m_pExtension == '.') {
		*(m_pExtension++) = '\0';
	}
	else {
		m_pExtension = m_pPath + len;
	}

	//
	//	Look for the beginning of filename body
	//
	m_pBody = m_pExtension;

	while (--m_pBody > m_pPath) {
		if (*m_pBody == PATH_SEPARATOR_CHAR ||
			*m_pBody == ALT_SEPARATOR_CHAR) {
			*(m_pBody++) = '\0';
			break;
		}
	}

	return VDK_OK;
}

//
//	Store parent path
//
VDKSTAT VDisk::StoreParentPath(PCHAR pPath)
{
	ULONG len;

	if (m_pParentPath) {
		delete[] m_pParentPath;
		m_pParentPath = NULL;
	}

	if (!pPath || !*pPath) {
		return VDK_OK;
	}

	len = strlen(pPath);

	if (len) {
		if ((m_pParentPath = new CHAR [len + 1]) == NULL) {
			return VdkLastError();
		}

		strcpy(m_pParentPath, pPath);
	}

	return VDK_OK;
}

//
//	Add new extent object to list
//
VDKSTAT VDisk::AddExtent(VDiskExt *ext)
{
	if (!ext) {
		return VDK_PARAM;
	}

	if (m_nExtents == m_nArraySize) {
		VDiskExt **tmp = new VDiskExt *[m_nArraySize + 10];

		if (tmp == NULL) {
			return VdkLastError();
		}

		memcpy(tmp, m_ppExtents, sizeof(VDiskExt *) * m_nArraySize);
		memset(tmp + m_nArraySize, 0, sizeof(VDiskExt *) * 10);

		delete[] m_ppExtents;
		m_ppExtents = tmp;
		m_nArraySize += 10;
	}

	m_ppExtents[m_nExtents++] = ext;

	return VDK_OK;
}

//
//	Create all ancestors
//
VDKSTAT VDisk::CreateTree()
{
	VDKSTAT ret;
	VDisk *disk;

	disk = this;

	while (disk->m_nFlags & VDISK_FLAG_CHILD) {

		ret = VDiskLoadFile(
			(PVOID *)&(disk->m_pParent),
			disk->m_pParentPath,
			disk->m_pPath);

		if (ret != VDK_OK) {
			return ret;
		}

		disk = disk->m_pParent;
	}

	if (disk != this) {
		ULONG cylinders	= disk->m_nCylinders;
		ULONG tracks	= disk->m_nTracks;
		ULONG sectors	= disk->m_nSectors;

		disk = this;

		while (disk->m_nFlags & VDISK_FLAG_CHILD) {
			//
			//	Check parent-child consistency
			//
			PVOID cbparams[4];
			CHAR child_path[MAX_PATH], parent_path[MAX_PATH];

			cbparams[0] = child_path;
			cbparams[2] = parent_path;

			disk->FullPath(child_path);
			disk->m_pParent->FullPath(parent_path);

			if (disk->m_nCapacity != disk->m_pParent->m_nCapacity) {

				//	capacity mismatch
				//	-- this is fatal

				cbparams[1] = (PVOID)disk->m_nCapacity;
				cbparams[3] = (PVOID)disk->m_pParent->m_nCapacity;

				VDiskCallBack(VDISK_CB_PARENT_CAPACITY, cbparams);
				return VDK_DATA;
			}

			if (disk->m_nParentTS != disk->m_pParent->m_nTimeStamp) {

				//	timestamp mismatch

				cbparams[1] = (PVOID)disk->m_nParentTS;
				cbparams[3] = (PVOID)disk->m_pParent->m_nTimeStamp;

				if (!VDiskCallBack(VDISK_CB_PARENT_TIMESTAMP, cbparams)) {
					return VDK_CANCEL;
				}
			}

			if (disk->m_nController &&
				disk->m_nController != disk->m_pParent->m_nController) {

				//	controller type mismatch

				cbparams[1] = (PVOID)disk->m_nController;
				cbparams[3] = (PVOID)disk->m_pParent->m_nController;

				if (!VDiskCallBack(VDISK_CB_PARENT_CONTROLLER, cbparams)) {
					return VDK_CANCEL;
				}
			}

			disk->m_nCylinders	= cylinders;
			disk->m_nTracks		= tracks;
			disk->m_nSectors	= sectors;

			disk = disk->m_pParent;
		}
	}

	return VDK_OK;
}

//
//	Delete including all ancestors
//
void VDisk::DeleteTree()
{
	VDisk *disk = this;

	while (disk) {
		VDisk *temp = disk;

		disk = disk->m_pParent;

		delete temp;
	}
}

//
//	Initialize as a root disk with parameters
//
VDKSTAT VDisk::InitRoot(
	ULONG		flags,
	PCHAR		pPath,
	ULONG		version,
	ULONG		controller,
	ULONG		capacity)
{
	ULONG ext_size;
	VDKSTAT ret;

	if (!pPath || !*pPath ||
		!version || !controller || !capacity) {
		return VDK_PARAM;
	}

	if ((ret = StorePath(pPath)) != VDK_OK) {
		return ret;
	}

	m_nVMwareVer	= version;
	m_nFlags		= flags & ~VDISK_FLAG_CHILD;
	m_nHardwareVer	= version - 1;
	m_nController	= controller;

	SetDefaultTS();

	//
	//	decide geometry values
	//
	m_nCapacity = capacity;
	SetGeometry();

	//
	//	deside each extent size
	//
	ext_size = DefaultExtSize();

	if (ext_size == 0) {
		return VDK_PARAM;
	}

	//
	//	create each extent object
	//
	ret = CreateExtents(ext_size);

	if (ret != VDK_OK) {
		return ret;
	}

#ifdef VDK_DEBUG
	Dump();
#endif

	return ret;
}

//
//	Initialize as a child with parameters
//
VDKSTAT VDisk::InitChild(
	ULONG		flags,
	PCHAR		pPath,
	ULONG		version,
	VDisk		*parent)
{
	CHAR path[MAX_PATH];
	ULONG ext_size;
	VDKSTAT ret;

	if (!pPath || !*pPath || !version || !parent) {
		return VDK_PARAM;
	}

	if (!(flags & VDISK_FLAG_SPARSE) ||
		!(flags & VDISK_FLAG_CHILD)) {
		return VDK_PARAM;
	}

	if (version < parent->GetVMwareVer()) {
		return VDK_PARAM;
	}

	if ((ret = StorePath(pPath)) != VDK_OK) {
		return ret;
	}

	if ((flags & VDISK_FLAG_ABSPATH) ||
		VdkCmpNoCase(m_pPath, parent->m_pPath)) {

		sprintf(path, "%s" PATH_SEPARATOR_STR "%s.%s",
			parent->m_pPath, parent->m_pBody, parent->m_pExtension);
	}
	else {
		sprintf(path, "%s.%s", parent->m_pBody, parent->m_pExtension);
	}

	if ((ret = StoreParentPath(path)) != VDK_OK) {
		return ret;
	}

	m_pParent		= parent;
	m_nVMwareVer	= version;
	m_nFlags		= flags;

	m_nCapacity		= parent->GetCapacity();
	m_nCylinders	= parent->GetCylinders();
	m_nTracks		= parent->GetTracks();
	m_nSectors		= parent->GetSectors();

	m_nController	= parent->GetController();

	m_nParentTS		= parent->GetTimeStamp();
	m_nTimeStamp	= (ULONG) time(NULL);

	ext_size		= m_nCapacity;

	if (m_nVMwareVer >= 3) {
		m_nToolsFlag	= parent->GetToolsFlag();
		m_nHardwareVer	= parent->GetHardwareVer();

		if (m_nVMwareVer == 3) {
			if (ext_size > COWD3_MAX_EXTENT_SPARSE) {
				ext_size = COWD3_MAX_EXTENT_SPARSE;
			}
		}
		else {
			if (!(m_nFlags & VDISK_FLAG_SINGLE) &&
				ext_size > VMDK_MAX_EXTENT_SPARSE) {

				ext_size = VMDK_MAX_EXTENT_SPARSE;
			}
		}
	}

	//
	//	create each extent objects
	//
	ret = CreateExtents(ext_size);

	if (ret != VDK_OK) {
		return ret;
	}

#ifdef VDK_DEBUG
	Dump();
#endif

	return ret;
}

//
//	Create extent objects
//
VDKSTAT VDisk::CreateExtents(ULONG ext_size)
{
	CHAR	path[MAX_PATH];
	VDiskExt *ext;
	ULONG	total_size = 0;
	VDKSTAT	ret = VDK_OK;

	while (total_size < m_nCapacity) {

		GetExtentPath(path, m_nExtents);

		ext = NewExtent();

		if (!ext) {
			return VdkLastError();
		}

		ret = AddExtent(ext);

		if (ret != VDK_OK) {
			delete ext;
			return ret;
		}

		ret = ext->SetPath(path);

		if (ret != VDK_OK) {
			return ret;
		}

		ext->SetCapacity(ext_size);

		total_size += ext_size;

		if (ext_size > m_nCapacity - total_size) {
			ext_size = m_nCapacity - total_size;
		}
	}

	return ret;
}

//
//	Set default Timestamp values
//
void VDisk::SetDefaultTS()
{
	m_nParentTS	= 0;
	m_nTimeStamp = (ULONG) time(NULL);
}

//
//	Set geometry values
//
void VDisk::SetGeometry()
{
	m_nCylinders	= 0;
	m_nTracks		= 0;
	m_nSectors		= 0;
}

//
//	Returns default (maximum) capacity for a single extension
//
ULONG VDisk::DefaultExtSize()
{
	return 0;
}

#ifdef VDK_DEBUG
//
//	dump virtual disk information
//
void VDisk::Dump()
{
	VDKTRACE(VDKINFO,
		("%s" PATH_SEPARATOR_STR "%s.%s\n", m_pPath, m_pBody, m_pExtension));

	VDKTRACE(VDKINFO,
		("VMware %u.x\n", m_nVMwareVer));

	char buf[200];
	buf[0] = '\0';

	if (m_nFlags & VDISK_FLAG_SINGLE) {
		strcat(buf, "SINGLE ");
	}
	else {
		strcat(buf, "SPLIT ");
	}

	if (m_nFlags & VDISK_FLAG_SPARSE) {
		strcat(buf, "SPARSE ");
	}
	else {
		strcat(buf, "SOLID ");
	}

	if (m_nFlags & VDISK_FLAG_ABSPATH) {
		strcat(buf, "ABSPATH ");
	}
	else {
		strcat(buf, "RELPATH ");
	}

	if (m_nFlags & VDISK_FLAG_CHILD) {
		strcat(buf, "CHILD ");
	}
	else {
		strcat(buf, "ROOT ");
	}

	if (m_nFlags & VDISK_FLAG_DIRTY) {
		strcat(buf, "MODIFIED");
	}

	VDKTRACE(VDKINFO,
		("%s\n", buf));

	//
	//	Virtual disk parameters
	//
	VDKTRACE(VDKINFO,("ParentPath: %s\n", m_pParentPath));
	VDKTRACE(VDKINFO,("Capacity  : %lu\n", m_nCapacity));
	VDKTRACE(VDKINFO,("Cylinders : %lu\n", m_nCylinders));
	VDKTRACE(VDKINFO,("Tracks    : %lu\n", m_nTracks));
	VDKTRACE(VDKINFO,("Sectors   : %lu\n", m_nSectors));

	VDKTRACE(VDKINFO,("Parent TS : 0x%08x\n", m_nParentTS));
	VDKTRACE(VDKINFO,("TimeStamp : 0x%08x\n", m_nTimeStamp));

	if (m_nController == VDISK_CONTROLLER_SCSI) {
		VDKTRACE(VDKINFO,("Controller: SCSI\n"));
	}
	else if (m_nController == VDISK_CONTROLLER_IDE) {
		VDKTRACE(VDKINFO,("Controller: IDE\n"));
	}
	else {
		VDKTRACE(VDKINFO,("Controller: Unknown (%d)\n", m_nController));
	}

	VDKTRACE(VDKINFO,("Hardware  : %lu\n", m_nHardwareVer));
	VDKTRACE(VDKINFO,("ToolsFlag : 0x%08x\n", m_nToolsFlag));

	for (ULONG idx = 0; idx < m_nExtents; idx++) {
		VDiskExt *ext = m_ppExtents[idx];

		if (ext) {
			VDKTRACE(VDKINFO,("\nExtent #%lu\n", idx));

			VDKTRACE(VDKINFO,("Path      : %s\n", ext->GetFullPath()));
			VDKTRACE(VDKINFO,("Capacity  : %lu\n", ext->GetCapacity()));
			VDKTRACE(VDKINFO,("FileSize  : %" INT64_PRINT_FORMAT "u\n", ext->GetFileSize()));
		}
	}
	VDKTRACE(VDKINFO,("========================================\n\n"));
}
#endif	//	VDK_DEBUG
