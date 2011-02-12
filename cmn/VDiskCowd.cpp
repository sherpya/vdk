/*
	VDiskCowd.cpp

	COWDisk class
	Copyright (c) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

#include "cowdisk.h"

#include "VDiskCowd.h"
#include "VDiskExtCowd.h"
#include "VDiskUtil.h"


//
//	Constructor -- set default members
//
VDiskCowd::VDiskCowd()
{
	m_nVMwareVer	= 3;
	m_nHardwareVer	= COWD_HARDWARE_VMWARE3;
}

//
//	Destructor
//
VDiskCowd::~VDiskCowd()
{
}

//
//	Initialize instance from a VMware 2.x/3.x virtual file
//
VDKSTAT VDiskCowd::Initialize(PCHAR pPath)
{
	CHAR	path[MAX_PATH];
	ULONG	total_extents;
	ULONG	idx;
	VDKSTAT	ret;
	HANDLE	hFile;
	ULONG_PTR result;
	COWD_HEADER	cowd;

	//	check parameter

	if (!pPath || !*pPath) {
		return VDK_PARAM;
	}

	//	store path

	if ((ret = StorePath(pPath)) != VDK_OK) {
		return ret;
	}

	//	open file

	ret = VdkOpenFile(&hFile, pPath, strlen(pPath), TRUE);

	if (ret != VDK_OK) {
		return ret;
	}

	//	read COWD header

	ret = VdkReadFileAt(hFile, 0, &cowd, sizeof(cowd), &result);

	VdkCloseFile(hFile);

	if (ret != VDK_OK) {
		return ret;
	}

	if (result != sizeof(cowd)) {
		return VDK_EOF;
	}

	//	process header information

	if (cowd.sec0.Version == COWD_FILEVER_VMWARE2) {
		m_nVMwareVer = 2;
	}
	else {
		m_nVMwareVer = 3;
	}

	//
	//	decide how many extents should be present
	//
	if (COWD_IS_MULTI(cowd.sec0.Flags)) {
		total_extents = cowd.sec3.FilesPerDisk;

		if (cowd.sec3.FileOrdinal) {
			//
			// adjust base file name --
			// if the source path has a sequence number, remove it
			//
			idx = strlen(m_pBody);

			if (idx > 3 && *(m_pBody + idx - 3) == '-' &&
				(ULONG)atol(m_pBody + idx - 2) == cowd.sec3.FileOrdinal + 1) {

				*(m_pBody + idx - 3) = '\0';
			}
		}
	}
	else {
		total_extents = 1;
	}

	//
	// initialize each extent object
	//
	for (idx = 0; idx < total_extents; idx++) {
		VDiskExtCowd *ext;

		ext = new VDiskExtCowd;

		if (!ext) {
			return VdkLastError();
		}

		ret = AddExtent(ext);

		if (ret != VDK_OK) {
			delete ext;
			return ret;
		}

		GetExtentPath(path, idx);

		ret = VDiskSearchFile(&hFile, path, NULL);

		if (ret != VDK_OK) {
			return ret;
		}

		ret = ext->SetPath(path);

		if (ret == VDK_OK) {
			ret = ext->Load(hFile);
		}

		VdkCloseFile(hFile);

		if (ret != VDK_OK) {
			return ret;
		}
	}

	return Check();
}

//
//	check COWDisk consistency
//
VDKSTAT VDiskCowd::Check()
{
	ULONG total_sectors;
	PCOWD_SECTOR_0 sec0_0;
	PCOWD_SECTOR_2 sec2_0;
	PCOWD_SECTOR_3 sec3_0;
	PVOID cbparams[4];
	ULONG idx;
	VDKSTAT ret;

	for (idx = 0; idx < m_nExtents; idx++) {
		VDiskExtCowd *ext = (VDiskExtCowd *)m_ppExtents[idx];

		ret = ext->Check();

		if (ret != VDK_OK) {
			return ret;
		}

		if (ext->IsModified()) {
			SetFlag(VDISK_FLAG_DIRTY);
		}

		if (ext->GetSec3()->FileOrdinal != idx) {

			cbparams[0] = ext->GetFileName();
			cbparams[1] = (PVOID)ext->GetSec3()->FileOrdinal;
			cbparams[2] = (PVOID)idx;

			if (!VDiskCallBack(VDISK_CB_COWD_ORDINAL, cbparams)) {
				return VDK_CANCEL;
			}

			ext->GetSec3()->FileOrdinal = idx;
			ext->SetModify();
			SetFlag(VDISK_FLAG_DIRTY);
		}
	}

	//
	//	check extent parameters consistency
	//
	sec0_0 = ((VDiskExtCowd *)m_ppExtents[0])->GetSec0();
	sec2_0 = ((VDiskExtCowd *)m_ppExtents[0])->GetSec2();
	sec3_0 = ((VDiskExtCowd *)m_ppExtents[0])->GetSec3();

	cbparams[0] = m_ppExtents[0]->GetFullPath();

	for (idx = 1; idx < m_nExtents; idx++) {
		BOOL conflict = FALSE, fatal = FALSE;
		PCOWD_SECTOR_0 sec0 = ((VDiskExtCowd *)m_ppExtents[idx])->GetSec0();
		PCOWD_SECTOR_2 sec2 = ((VDiskExtCowd *)m_ppExtents[idx])->GetSec2();
		PCOWD_SECTOR_3 sec3 = ((VDiskExtCowd *)m_ppExtents[idx])->GetSec3();

		cbparams[2] = m_ppExtents[idx]->GetFullPath();

		//
		//	FATAL conflicts
		//
		if (sec0_0->Version != sec0->Version) {
			cbparams[1] = (PVOID)sec0_0->Version;
			cbparams[3] = (PVOID)sec0->Version;

			VDiskCallBack(VDISK_CB_CONF_FILEVER, cbparams);
			fatal = TRUE;
		}

		if (sec0_0->Flags != sec0->Flags) {
			cbparams[1] = (PVOID)sec0_0->Flags;
			cbparams[3] = (PVOID)sec0->Flags;

			VDiskCallBack(VDISK_CB_CONF_FLAGS, cbparams);
			fatal = TRUE;
		}

		if (sec3_0->FilesPerDisk != sec3->FilesPerDisk) {
			cbparams[1] = (PVOID)sec3_0->FilesPerDisk;
			cbparams[3] = (PVOID)sec3->FilesPerDisk;

			VDiskCallBack(VDISK_CB_CONF_EXTENTS, cbparams);
			fatal = TRUE;
		}

		if (sec3_0->DiskCapacity != sec3->DiskCapacity) {
			cbparams[1] = (PVOID)sec3_0->DiskCapacity;
			cbparams[3] = (PVOID)sec3->DiskCapacity;

			VDiskCallBack(VDISK_CB_CONF_CAPACITY, cbparams);
			fatal = TRUE;
		}

		if (!COWD_IS_ROOT(sec0_0->Flags)) {
			if (sec0_0->Capacity != sec0->Capacity) {
				cbparams[1] = (PVOID)sec0_0->Capacity;
				cbparams[3] = (PVOID)sec0->Capacity;

				VDiskCallBack(VDISK_CB_CONF_CAPACITY, cbparams);
				fatal = TRUE;
			}

			if (VdkCmpNoCase(sec0_0->u.ParentPath, sec0->u.ParentPath)) {
				cbparams[1] = sec0_0->u.ParentPath;
				cbparams[3] = sec0->u.ParentPath;

				VDiskCallBack(VDISK_CB_CONF_PARENTPATH, cbparams);
				fatal = TRUE;
			}
		}

		//
		//	Errors which can be ignored
		//

		if (sec2_0->ParentTS != sec2->ParentTS) {
			cbparams[1] = (PVOID)sec2_0->ParentTS;
			cbparams[3] = (PVOID)sec2->ParentTS;

			VDiskCallBack(VDISK_CB_CONF_PARENTTS, cbparams);
			conflict = TRUE;
		}

		if (sec2_0->TimeStamp != sec2->TimeStamp) {
			cbparams[1] = (PVOID)sec2_0->TimeStamp;
			cbparams[3] = (PVOID)sec2->TimeStamp;

			VDiskCallBack(VDISK_CB_CONF_TIMESTAMP, cbparams);
			conflict = TRUE;
		}

		if (sec3_0->Controller != sec3->Controller) {
			cbparams[1] = &sec3_0->Controller;
			cbparams[3] = &sec3->Controller;

			VDiskCallBack(VDISK_CB_CONF_CONTROLLER, cbparams);
			conflict = TRUE;
		}

		if (sec3_0->Cylinders	!= sec3->Cylinders) {
			cbparams[1] = (PVOID)sec3_0->Cylinders;
			cbparams[3] = (PVOID)sec3->Cylinders;

			VDiskCallBack(VDISK_CB_CONF_CYLINDERS, cbparams);
			conflict = TRUE;
		}

		if (sec3_0->Tracks != sec3->Tracks) {
			cbparams[1] = (PVOID)sec3_0->Tracks;
			cbparams[3] = (PVOID)sec3->Tracks;

			VDiskCallBack(VDISK_CB_CONF_TRACKS, cbparams);
			conflict = TRUE;
		}

		if (sec3_0->Sectors != sec3->Sectors) {
			cbparams[1] = (PVOID)sec3_0->Sectors;
			cbparams[3] = (PVOID)sec3->Sectors;

			VDiskCallBack(VDISK_CB_CONF_SECTORS, cbparams);
			conflict = TRUE;
		}

		if (sec3_0->SequenceNumber != sec3->SequenceNumber) {
			cbparams[1] = (PVOID)sec3_0->SequenceNumber;
			cbparams[3] = (PVOID)sec3->SequenceNumber;

			VDiskCallBack(VDISK_CB_CONF_SEQNUM, cbparams);
			conflict = TRUE;
		}

		if (sec3_0->HardwareVer != sec3->HardwareVer) {
			cbparams[1] = (PVOID)sec3_0->HardwareVer;
			cbparams[3] = (PVOID)sec3->HardwareVer;

			VDiskCallBack(VDISK_CB_CONF_HARDWARE, cbparams);
			conflict = TRUE;
		}

		if (sec3_0->ToolsFlag != sec3->ToolsFlag) {
			cbparams[1] = (PVOID)sec3_0->ToolsFlag;
			cbparams[3] = (PVOID)sec3->ToolsFlag;

			VDiskCallBack(VDISK_CB_CONF_TOOLSFLAG, cbparams);
			conflict = TRUE;
		}

		if (fatal) {
			return VDK_DATA;
		}

		if (conflict) {
			if (!VDiskCallBack(VDISK_CB_CONFLICT_IGNORE, NULL)) {
				return VDK_CANCEL;
			}
			SetFlag(VDISK_FLAG_DIRTY);
		}
	}

	//
	//	store necessary disk parameters
	//
	if (COWD_IS_ROOT(sec0_0->Flags)) {
		ClrFlag(VDISK_FLAG_CHILD);

		if (COWD_IS_MULTI(sec0_0->Flags)) {
			m_nCapacity		= sec3_0->DiskCapacity;
			m_nCylinders	= sec3_0->Cylinders;
			m_nTracks		= sec3_0->Tracks;
			m_nSectors		= sec3_0->Sectors;
		}
		else {
			m_nCapacity		= sec0_0->Capacity;
			m_nCylinders	= sec0_0->u.Geometry.Cylinders;
			m_nTracks		= sec0_0->u.Geometry.Tracks;
			m_nSectors		= sec0_0->u.Geometry.Sectors;
		}
	}
	else {
		SetFlag(VDISK_FLAG_CHILD);

		m_nCapacity = sec0_0->Capacity;

		StoreParentPath(sec0_0->u.ParentPath);

		if ((isalpha(*m_pParentPath) && *(m_pParentPath + 1) == ':') ||
			*m_pParentPath == PATH_SEPARATOR_CHAR ||
			*m_pParentPath == ALT_SEPARATOR_CHAR) {

			SetFlag(VDISK_FLAG_ABSPATH);
		}
		else {
			ClrFlag(VDISK_FLAG_ABSPATH);
		}
	}

	m_nParentTS		= sec2_0->ParentTS;
	m_nTimeStamp	= sec2_0->TimeStamp;
	m_nHardwareVer	= sec3_0->HardwareVer;
	m_nToolsFlag	= sec3_0->ToolsFlag;

	if (sec3_0->Controller == COWD_CONTROLLER_IDE) {
		m_nController	= VDISK_CONTROLLER_IDE;
	}
	else {
		m_nController	= VDISK_CONTROLLER_SCSI;
	}

	//
	// adjust file capacity of the last extent
	// -- cowd header does not explicitly specify it.
	//
	total_sectors = 0;

	for (idx = 0; idx < m_nExtents - 1; idx++) {
		total_sectors += m_ppExtents[idx]->GetCapacity();
	}

	if (total_sectors >= m_nCapacity) {
		CHAR path[MAX_PATH];

		cbparams[0] = path;
		cbparams[1] = (PVOID)m_nCapacity;
		cbparams[2] = (PVOID)total_sectors;

		FullPath(path);

		if (!VDiskCallBack(VDISK_CB_COWD_CAPACITY, cbparams)) {
			return VDK_DATA;
		}

		if (total_sectors >= m_nCapacity) {
			return VDK_DATA;
		}

		SetFlag(VDISK_FLAG_DIRTY);
	}

	m_ppExtents[idx]->SetCapacity(m_nCapacity - total_sectors);

	//
	//	Update ?
	//
	if (m_nFlags & VDISK_FLAG_DIRTY) {
		CHAR path[MAX_PATH];

		cbparams[0] = path;

		FullPath(path);

		if (VDiskCallBack(VDISK_CB_CONFIRM_FIX, cbparams)) {
			for (idx = 0; idx < m_nExtents; idx++) {
				ret = m_ppExtents[idx]->Update();

				if (ret != VDK_OK) {
					return ret;
				}
			}
		}
		ClrFlag(VDISK_FLAG_DIRTY);
	}


	return VDK_OK;
}

//
//	Create VMware 2/3 virtual disk
//
VDKSTAT VDiskCowd::Create(ULONG flags)
{
	ULONG idx;
	VDKSTAT ret = VDK_OK;

	if (!m_nExtents || !m_ppExtents || !m_ppExtents[0]) {
		return VDK_FUNCTION;
	}

	for (idx = 0; idx < m_nExtents; idx++) {
		VDiskExtCowd *ext = (VDiskExtCowd *)m_ppExtents[idx];

		ext->CreateHeader(this, idx);

		ret = ext->Create(flags);

		if (ret != VDK_OK) {
			break;
		}
	}

	return ret;
}

//
//	Creates a new extent object for this instance
//
VDiskExt *VDiskCowd::NewExtent()
{
	return new VDiskExtCowd;
}

//
//	Get path for each extent files
//
void VDiskCowd::GetExtentPath(
	PCHAR pPath, ULONG nSeq)
{
	if (nSeq) {
		sprintf(pPath, "%s" PATH_SEPARATOR_STR "%s-%02ld.%s",
			m_pPath, m_pBody, nSeq + 1, m_pExtension);
	}
	else {
		sprintf(pPath, "%s" PATH_SEPARATOR_STR "%s.%s",
			m_pPath, m_pBody, m_pExtension);
	}
}

//
//	Set default geometry values
//
void VDiskCowd::SetGeometry()
{
	if (m_nController == VDISK_CONTROLLER_IDE) {

		if (m_nVMwareVer == 2) {
			m_nTracks	= COWD1_TRACKS_IDE;
			m_nSectors	= COWD_SECTORS_IDE;
		}
		else {
			m_nTracks	= COWD3_TRACKS_IDE;
			m_nSectors	= COWD_SECTORS_IDE;
		}
	}
	else {
		if (m_nCapacity <=
			COWD_SECTORS_SCSI_1023M * COWD_TRACKS_SCSI_1023M * 1023) {

			m_nSectors	= COWD_SECTORS_SCSI_1023M;
			m_nTracks	= COWD_TRACKS_SCSI_1023M;
		}
		else if (m_nCapacity <=
			COWD_SECTORS_SCSI_2046M * COWD_TRACKS_SCSI_2046M * 1023) {

			m_nSectors	= COWD_SECTORS_SCSI_2046M;
			m_nTracks	= COWD_TRACKS_SCSI_2046M;
		}
		else {
			m_nSectors	= COWD_SECTORS_SCSI_LARGE;
			m_nTracks	= COWD_TRACKS_SCSI_LARGE;
		}
	}

	m_nCylinders = m_nCapacity / (m_nSectors * m_nTracks);
}

//
//	Get default extent size
//
ULONG VDiskCowd::DefaultExtSize()
{
	ULONG ext_size = 0;

	if (m_nVMwareVer == 2) {
		if (m_nController == VDISK_CONTROLLER_SCSI) {
			ext_size = COWD1_MAX_EXTENT_SCSI;
		}
		else if (m_nController == VDISK_CONTROLLER_IDE) {
			ext_size = COWD1_MAX_EXTENT_IDE;
		}
	}
	else {
		ext_size = COWD3_MAX_EXTENT_SPARSE;
	}

	if (m_nCapacity > ext_size) {

		if (m_nVMwareVer == 2) {
			//
			//	VMware 2.x doesn't allow multi-extent virutal disk
			//
			ext_size = 0;
		}
		else {
			ext_size = m_nCapacity;
		}
	}

	return ext_size;
}


