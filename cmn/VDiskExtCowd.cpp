/*
	VDiskExtCowd.cpp

	COWDisk extent class
	Copyright (c) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

#include "cowdisk.h"

#include "VDisk.h"
#include "VDiskExtCowd.h"
#include "VDiskUtil.h"

//
//	Constructor
//
VDiskExtCowd::VDiskExtCowd()
{
	VdkZeroMem(&m_Sec0, sizeof(COWD_SECTOR_0));
	VdkZeroMem(&m_Sec2, sizeof(COWD_SECTOR_2));
	VdkZeroMem(&m_Sec3, sizeof(COWD_SECTOR_3));
}

//
//	Destructor
//
VDiskExtCowd::~VDiskExtCowd()
{
}

//
//	Load and obtain parameters from a cowdisk file
//
VDKSTAT VDiskExtCowd::Load(HANDLE hFile)
{
	COWD_HEADER	cowd;
	VDKSTAT		ret;

	//
	//	Get file attributes
	//
	m_nFileAttr = VdkGetAttribute(m_pFullPath);

	if (m_nFileAttr == (ULONG)INVALID_FILE_ATTRIBUTES) {
		m_nFileAttr = 0;
	}

	//
	//	Get actual file size
	//
	ret = VdkGetFileSize(hFile, &m_nFileSize);

	if (ret != VDK_OK) {
		return ret;
	}

	//
	//	read COWD header
	//
	ret = VdkReadFileAt(hFile, 0, &cowd, sizeof(COWD_HEADER), NULL);

	if (ret != VDK_OK) {
		return ret;
	}

	//
	//	Store header parameters
	//
	VdkCopyMem(&m_Sec0, &cowd.sec0, sizeof(COWD_SECTOR_0));
	VdkCopyMem(&m_Sec2, &cowd.sec2, sizeof(COWD_SECTOR_2));
	VdkCopyMem(&m_Sec3, &cowd.sec3, sizeof(COWD_SECTOR_3));

	return VDK_OK;
}

//
//	Check parameter consistency
//
VDKSTAT VDiskExtCowd::Check()
{
	ULONG ultmp;
	PVOID cbparams[3];

	cbparams[0] = m_pFullPath;

	//
	//	Signature
	//
	if (m_Sec0.Signature != COWD_SIGNATURE) {

		cbparams[1] = (PVOID)m_Sec0.Signature;

		if (!VDiskCallBack(VDISK_CB_SIGNATURE, cbparams)) {
			return VDK_CANCEL;
		}
	
		m_Sec0.Signature =  COWD_SIGNATURE;
		SetModify();
	}

	//
	//	File version
	//
	if (m_Sec0.Version != COWD_FILEVER_VMWARE2 &&
		m_Sec0.Version != COWD_FILEVER_VMWARE3) {

		cbparams[1] = (PVOID)m_Sec0.Version;

		m_Sec0.Version = VDiskCallBack(
			VDISK_CB_COWD_FILEVER, cbparams);

		if (m_Sec0.Version != COWD_FILEVER_VMWARE2 &&
			m_Sec0.Version != COWD_FILEVER_VMWARE3) {
			return VDK_CANCEL;
		}

		SetModify();
	}

	//
	//	geometries
	//
	if (COWD_IS_ROOT(m_Sec0.Flags)) {

		if (m_Sec0.u.Geometry.Cylinders *
			m_Sec0.u.Geometry.Tracks *
			m_Sec0.u.Geometry.Sectors !=
			m_Sec0.Capacity ||
			!m_Sec0.Capacity) {

			cbparams[1] = &m_Sec0;

			VDiskCallBack(
				VDIDK_CB_COWD_FILEGEOM, cbparams);

			if (m_Sec0.u.Geometry.Cylinders *
				m_Sec0.u.Geometry.Tracks *
				m_Sec0.u.Geometry.Sectors !=
				m_Sec0.Capacity ||
				!m_Sec0.Capacity) {
				return VDK_DATA;
			}

			SetModify();
		}

		if (m_Sec0.Version == COWD_FILEVER_VMWARE3) {
			if (m_Sec3.Cylinders *
				m_Sec3.Tracks *
				m_Sec3.Sectors	!=
				m_Sec3.DiskCapacity ||
				!m_Sec3.DiskCapacity) {

				cbparams[1] = &m_Sec3;

				VDiskCallBack(
					VDISK_CB_COWD_DISKGEOM, cbparams);

				if (m_Sec3.Cylinders *
					m_Sec3.Tracks *
					m_Sec3.Sectors	!=
					m_Sec3.DiskCapacity ||
					!m_Sec3.DiskCapacity) {

					return VDK_DATA;
				}

				SetModify();
			}
		}
	}
	else {

		//	Parent path

		if (!m_Sec0.u.ParentPath[0] ||
			strlen(m_Sec0.u.ParentPath) > MAX_PATH) {

			m_Sec0.u.ParentPath[MAX_PATH] = '\0';
			cbparams[1] = m_Sec0.u.ParentPath;

			if (!VDiskCallBack(VDISK_CB_COWD_PARENT, cbparams) ||
				!m_Sec0.u.ParentPath[0] ||
				strlen(m_Sec0.u.ParentPath) > MAX_PATH) {
				return VDK_DATA;
			}

			SetModify();
		}
	}

	//
	//	Map size
	//
	ultmp = m_Sec0.Granularity * COWD_SECONDARY_MAP_SIZE;
	ultmp = (m_Sec0.Capacity + ultmp - 1) / ultmp;

	if (ultmp != m_Sec0.PrimaryMapSize) {

		cbparams[1] = (PVOID)m_Sec0.PrimaryMapSize;
		cbparams[2] = (PVOID)ultmp;

		if (!VDiskCallBack(VDISK_CB_COWD_MAPSIZE, cbparams)) {
			return VDK_DATA;
		}


		m_Sec0.PrimaryMapSize = ultmp;
		SetModify();
	}

	//
	//	File size
	//
	if ((INT64)m_Sec0.EndOfFile << VDK_BYTE_SHIFT_TO_SECTOR != m_nFileSize) {

		cbparams[1] = (PVOID)m_Sec0.EndOfFile;
		cbparams[2] = (PVOID)(m_nFileSize >> VDK_BYTE_SHIFT_TO_SECTOR);

		if (!VDiskCallBack(VDISK_CB_COWD_ENDOFFILE, cbparams)) {
			return VDK_DATA;
		}


		m_Sec0.EndOfFile = (ULONG)(m_nFileSize >> VDK_BYTE_SHIFT_TO_SECTOR);
		SetModify();
	}

	//
	//	Timestamp
	//
	if (m_Sec2.TimeStamp !=
		m_Sec3.TimeStamp) {

		cbparams[1] = (PVOID)m_Sec2.TimeStamp;
		cbparams[2] = (PVOID)m_Sec3.TimeStamp;

		if (!VDiskCallBack(VDISK_CB_COWD_TIMESTAMP, cbparams)) {
			return VDK_CANCEL;
		}


		m_Sec2.TimeStamp = m_Sec3.TimeStamp;
		SetModify();
	}

	//
	//	Controller type
	//
	if (m_Sec3.Controller != COWD_CONTROLLER_IDE &&
		m_Sec3.Controller != COWD_CONTROLLER_SCSI) {

		cbparams[1] = &(m_Sec3.Controller);

		ultmp = VDiskCallBack(VDISK_CB_CONTROLLER, cbparams);

		if (ultmp == VDISK_CONTROLLER_IDE) {
			m_Sec3.Controller = COWD_CONTROLLER_IDE;
		}
		else if (ultmp == VDISK_CONTROLLER_SCSI) {
			m_Sec3.Controller = COWD_CONTROLLER_SCSI;
		}
		else {
			return VDK_CANCEL;
		}

		SetModify();
	}

	//	Hardware version

	if (COWD_SET_HW_VER(m_Sec0.Flags) &&
		m_Sec3.HardwareVer != COWD_HARDWARE_VMWARE2 &&
		m_Sec3.HardwareVer != COWD_HARDWARE_VMWARE3) {

		CHAR ver[10];

		cbparams[1] = ver;

		sprintf(ver, "%lu", m_Sec3.HardwareVer);

		m_Sec3.HardwareVer = VDiskCallBack(
			VDISK_CB_HARDWAREVER, cbparams);

		if (m_Sec3.HardwareVer != COWD_HARDWARE_VMWARE2 &&
			m_Sec3.HardwareVer != COWD_HARDWARE_VMWARE3) {
			return VDK_CANCEL;
		}

		SetModify();
	}

	//
	//	Store capacity
	//
	if (m_Sec0.Version == COWD_FILEVER_VMWARE2) {
		m_nCapacity	= m_Sec0.Capacity;
	}
	else {
		m_nCapacity	= m_Sec3.FileCapacity;
	}

	return VDK_OK;
}

//
//	Update COWDisk header
//
VDKSTAT VDiskExtCowd::Update()
{
	HANDLE			hFile;
	VDKSTAT			ret;

	if (!IsModified()) {
		return VDK_OK;
	}

	ret = VdkOpenFile(&hFile, m_pFullPath, strlen(m_pFullPath), FALSE);

	if (ret != VDK_OK) {
		return ret;
	}

	ret = UpdateFile(hFile);

	VdkCloseFile(hFile);

	ClrModify();

	return ret;
}

void VDiskExtCowd::CreateHeader(VDisk *pDisk, ULONG ordinal)
{
	ULONG			disk_flags;
	ULONG			vmware_ver;
	ULONG			ulong_tmp;

	VdkZeroMem(&m_Sec0, sizeof(m_Sec0));
	VdkZeroMem(&m_Sec2, sizeof(m_Sec2));
	VdkZeroMem(&m_Sec3, sizeof(m_Sec3));

	vmware_ver = pDisk->GetVMwareVer();
	disk_flags = pDisk->GetFlags();

	m_Sec0.Signature	= COWD_SIGNATURE;

	if (vmware_ver <= 2) {
		m_Sec0.Version			= COWD_FILEVER_VMWARE2;
		m_Sec0.Granularity		= COWD1_DEFAULT_GRANULARITY;
	}
	else {
		m_Sec0.Version			= COWD_FILEVER_VMWARE3;
		m_Sec0.Granularity		= COWD3_DEFAULT_GRANULARITY;

		m_Sec3.FileOrdinal		= ordinal;
		m_Sec3.FilesPerDisk		= pDisk->GetExtentCnt();
		m_Sec3.FileCapacity		= COWD3_MAX_EXTENT_SPARSE;
		m_Sec3.HardwareVer		= pDisk->GetHardwareVer();
		m_Sec3.ToolsFlag		= pDisk->GetToolsFlag();

		if (!(disk_flags & VDISK_FLAG_CHILD)) {
			m_Sec3.Cylinders	= pDisk->GetCylinders();
			m_Sec3.Tracks		= pDisk->GetTracks();
			m_Sec3.Sectors		= pDisk->GetSectors();
			m_Sec3.DiskCapacity	= pDisk->GetCapacity();
		}

		if (m_Sec3.HardwareVer) {
			m_Sec0.Flags	|= COWD_FLAG_HW_VER;
		}

		if (m_Sec3.FilesPerDisk > 1) {
			m_Sec0.Flags	|= COWD_FLAG_MULTI;
		}
	}

	m_Sec2.ParentTS		= pDisk->GetParentTS();
	m_Sec2.TimeStamp	= pDisk->GetTimeStamp();
	m_Sec3.TimeStamp	= m_Sec2.TimeStamp;

	if (pDisk->GetController() == VDISK_CONTROLLER_IDE) {
		m_Sec3.Controller	= COWD_CONTROLLER_IDE;
	}
	else {
		m_Sec3.Controller	= COWD_CONTROLLER_SCSI;
	}

	if ((disk_flags & VDISK_FLAG_CHILD)) {
		m_Sec0.Capacity = pDisk->GetCapacity();

		strcpy(m_Sec0.u.ParentPath, pDisk->GetParentPath());
	}
	else {
		m_Sec0.Flags |= COWD_FLAG_ROOT;

		if (pDisk->GetCapacity() == m_nCapacity) {
			//
			//	VMware 2.x virtual disk or single extent virtual disk
			//
			m_Sec0.Capacity				= m_nCapacity;
			m_Sec0.u.Geometry.Cylinders	= pDisk->GetCylinders();
			m_Sec0.u.Geometry.Tracks	= pDisk->GetTracks();
			m_Sec0.u.Geometry.Sectors	= pDisk->GetSectors();
		}
		else {
			//
			//	VMware 3.x multi extent virtual disk
			//
			if (m_Sec3.Controller == COWD_CONTROLLER_IDE) {
				m_Sec0.u.Geometry.Sectors	= COWD_SECTORS_IDE;
				m_Sec0.u.Geometry.Tracks	= COWD3_TRACKS_IDE;
			}
			else {
				if (m_nCapacity <=
					COWD_SECTORS_SCSI_1023M * COWD_TRACKS_SCSI_1023M * 1023) {

					m_Sec0.u.Geometry.Sectors	= COWD_SECTORS_SCSI_1023M;
					m_Sec0.u.Geometry.Tracks	= COWD_TRACKS_SCSI_1023M;
				}
				else {
					m_Sec0.u.Geometry.Sectors	= COWD_SECTORS_SCSI_2046M;
					m_Sec0.u.Geometry.Tracks	= COWD_TRACKS_SCSI_2046M;
				}
			}

			ulong_tmp =
				m_Sec0.u.Geometry.Sectors *
				m_Sec0.u.Geometry.Tracks;

			m_Sec0.u.Geometry.Cylinders =
				m_nCapacity / ulong_tmp;

			m_Sec0.Capacity =
				m_Sec0.u.Geometry.Cylinders * ulong_tmp;
		}
	}

	m_Sec0.PrimaryMapOffset = COWD_PRIMARY_MAP_OFFSET;

	ulong_tmp = m_Sec0.Granularity * COWD_SECONDARY_MAP_SIZE;

	m_Sec0.PrimaryMapSize =
		(m_Sec0.Capacity + ulong_tmp - 1) / ulong_tmp;

	m_Sec0.EndOfFile = COWD_PRIMARY_MAP_OFFSET +
		((m_Sec0.PrimaryMapSize + 127) / 128);
}

//
//	Create actural extent file
//
VDKSTAT VDiskExtCowd::Create(ULONG flags)
{
	HANDLE			hFile;
	VDKSTAT			ret;

	ret = VdkCreateFile(&hFile, m_pFullPath, (flags & VDISK_CREATE_FORCE));

	if (ret != VDK_OK) {
		return ret;
	}

	ret = UpdateFile(hFile);

	VdkCloseFile(hFile);

	return ret;
}

//
//	Write COWD header
//
VDKSTAT VDiskExtCowd::UpdateFile(HANDLE hFile)
{
	COWD_HEADER	cowd;
	VDKSTAT		ret;

	VdkZeroMem(&cowd, sizeof(cowd));

	VdkCopyMem(&cowd.sec0, &m_Sec0, sizeof(m_Sec0));
	VdkCopyMem(&cowd.sec2, &m_Sec2, sizeof(m_Sec2));
	VdkCopyMem(&cowd.sec3, &m_Sec3, sizeof(m_Sec3));

	ret = VdkWriteFileAt(hFile, 0, &cowd, sizeof(cowd), NULL);

	if (ret != VDK_OK) {
		VdkCloseFile(hFile);

		return ret;
	}

	ret = VdkSetFileSize(hFile, (INT64)m_Sec0.EndOfFile << VDK_BYTE_SHIFT_TO_SECTOR);

	if (ret == VDK_OK) {
		m_nFileSize = (INT64)m_Sec0.EndOfFile << VDK_BYTE_SHIFT_TO_SECTOR;
	}

	return ret;
}
