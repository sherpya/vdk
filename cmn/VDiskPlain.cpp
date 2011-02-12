/*
	VDiskPlain.cpp

	Plain disk class
	Copyright (c) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

#include "cowdisk.h"

#include "VDiskPlain.h"
#include "VDiskExtRaw.h"
#include "VDiskUtil.h"
#include "VDiskFile.h"


//
//	Constructor -- set default members
//
VDiskPlain::VDiskPlain()
{
	m_nVMwareVer	= 2;
	m_nHardwareVer	= COWD_HARDWARE_VMWARE2;
}

//
//	Destructor
//
VDiskPlain::~VDiskPlain()
{
}

//
//	Initialize instance from a VMware 2.x plain disk descriptor file
//
VDKSTAT VDiskPlain::Initialize(PCHAR pPath)
{
	VDiskFile file;
	CHAR	buf[MAX_PATH + 40];
	CHAR	path[MAX_PATH];
	PCHAR	current;
	VDKSTAT	ret;
	PVOID	cbparams[2];

	//
	//	parameter check
	//
	if (!pPath || !*pPath) {
		return VDK_PARAM;
	}

	//
	//	store path
	//
	if ((ret = StorePath(pPath)) != VDK_OK) {
		return ret;
	}

	//
	//	open file
	//
	if ((ret = file.Open(pPath)) != VDK_OK) {
		return ret;
	}

	//
	//	initialize members
	//
	ClrFlag(VDISK_FLAG_CHILD);

	cbparams[0] = pPath;

	while ((ret = file.ReadText(buf, sizeof(buf), NULL)) == VDK_OK) {

		//
		//	replace tabs with blanks
		//
		current = buf;

		while (*current) {
			if (*current == '\t' || *current == '\n') {
				*current = ' ';
			}
			current++;
		}

		//
		//	remove trailing blanks
		//
		while (current > buf && *(--current) == ' ') {
			*current = '\0';
		}

		//
		//	skip leading blanks
		//
		current = buf;

		while (*current == ' ') {
			current++;
		}

		//
		//	blank line?
		//
		if (!*current) {
			continue;
		}

		cbparams[1] = current;

		//
		//	start parsing
		//
		if (!VdkCmpNoCaseN(current, "ACCESS ", 7)) {
			VDiskExtRaw *ext;
			PCHAR	top, tail;
			ULONG	capacity;
			ULONG	offset;
			CHAR	delim;
			HANDLE	hFile;

			//
			//	search path field
			//
			top = current + 7;

			while (*top == ' ') {
				top++;
			}

			if (*top == '\"') {
				delim = '\"';
				top++;
			}
			else {
				delim = ' ';
			}

			tail = top;

			while (*tail && *tail != delim) {
				tail++;
			}

			if (!*tail || tail - top >= MAX_PATH) {

				if (!VDiskCallBack(VDISK_CB_DESC_BADENTRY, cbparams)) {
					return VDK_CANCEL;
				}

				SetFlag(VDISK_FLAG_DIRTY);
				continue;
			}

			if ((isalpha(*top) && *(top + 1) == ':') ||
				*top == PATH_SEPARATOR_CHAR ||
				*top == ALT_SEPARATOR_CHAR) {

				SetFlag(VDISK_FLAG_ABSPATH);
			}
			else {
				ClrFlag(VDISK_FLAG_ABSPATH);
			}

			VdkCopyMem(path, top, tail - top);
			path[tail - top] = '\0';

			//
			//	search offset field
			//
			top = tail + 1;

			while (*top == ' ') {
				top++;
			}

			if (isdigit(*top)) {
				offset = atol(top);
			}
			else {

				offset = VDiskCallBack(
					VDISK_CB_DESC_OFFSET, cbparams);

				if (offset == (ULONG)-1) {
					return VDK_DATA;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}

			//
			//	search capacity field
			//
			while (isdigit(*top)) {
				top++;
			}

			while (*top == ' ') {
				top++;
			}

			capacity = atol(top);

			if (!capacity) {

				capacity = VDiskCallBack(
					VDISK_CB_DESC_CAPACITY, cbparams);

				if (!capacity) {
					return VDK_DATA;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}

			//
			//	create an extent object
			//
			ext = new VDiskExtRaw;

			if (ext == NULL) {
				return VdkLastError();
			}

			ret = AddExtent(ext);

			if (ret != VDK_OK) {
				delete ext;
				return ret;
			}

			ret = VDiskSearchFile(&hFile, path, m_pPath);

			if (ret != VDK_OK) {
				return ret;
			}

			ret = ext->SetPath(path);

			if (ret != VDK_OK) {
				return ret;
			}

			ret = ext->Load(hFile);

			VdkCloseFile(hFile);

			if (ret != VDK_OK) {
				return ret;
			}

			ext->SetCapacity(capacity);
			ext->SetStartOffset(offset);
		}
		else if (!VdkCmpNoCaseN(current, "CYLINDERS ", 10)) {
			m_nCylinders = atol(current + 10);

			if (!m_nCylinders) {

				m_nCylinders = VDiskCallBack(
					VDISK_CB_DESC_GEOMETRY, cbparams);

				if (!m_nCylinders) {
					return VDK_DATA;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}
		}
		else if (!VdkCmpNoCaseN(current, "HEADS ", 6)) {
			m_nTracks = atol(current + 6);

			if (!m_nTracks) {

				m_nTracks = VDiskCallBack(
					VDISK_CB_DESC_GEOMETRY, cbparams);

				if (!m_nTracks) {
					return VDK_DATA;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}
		}
		else if (!VdkCmpNoCaseN(current, "SECTORS ", 8)) {
			m_nSectors = atol(current + 8);

			if (!m_nSectors) {

				m_nSectors = VDiskCallBack(
					VDISK_CB_DESC_GEOMETRY, cbparams);

				if (!m_nSectors) {
					return VDK_DATA;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}
		}
		else if (!VdkCmpNoCaseN(current, "DRIVETYPE ", 10)) {
			PCHAR p = current + 10;

			while (*p == ' ') {
				p++;
			}

			if (!VdkCmpNoCase(p, "scsi")) {
				m_nController = VDISK_CONTROLLER_SCSI;
			}
			else if (!VdkCmpNoCase(p, "ide")) {
				m_nController = VDISK_CONTROLLER_IDE;
			}
			else {

				m_nController = VDiskCallBack(
					VDISK_CB_CONTROLLER, cbparams);

				if (m_nController != VDISK_CONTROLLER_SCSI &&
					m_nController != VDISK_CONTROLLER_IDE) {
					return VDK_CANCEL;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}
		}
		else if (!VdkCmpNoCaseN(current, "#vm|TOOLSVERSION ", 17)) {
			m_nVMwareVer = 3;
			m_nToolsFlag = atol(current + 17);
		}
		else if (!VdkCmpNoCaseN(current, "#vm|VERSION ", 12)) {
			m_nVMwareVer = 3;
			m_nHardwareVer = atol(current + 12);

			if (m_nHardwareVer != COWD_HARDWARE_VMWARE2 &&
				m_nHardwareVer != COWD_HARDWARE_VMWARE3 &&
				m_nHardwareVer != COWD_HARDWARE_VMWARE4) {

				m_nHardwareVer = VDiskCallBack(
					VDISK_CB_HARDWAREVER, cbparams);

				if (m_nHardwareVer != COWD_HARDWARE_VMWARE2 &&
					m_nHardwareVer != COWD_HARDWARE_VMWARE3 &&
					m_nHardwareVer != COWD_HARDWARE_VMWARE4) {

					return VDK_CANCEL;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}
		}
		else if (*current != '#') {

			if (!VDiskCallBack(VDISK_CB_DESC_BADENTRY, cbparams)) {
				return VDK_CANCEL;
			}

			SetFlag(VDISK_FLAG_DIRTY);
		}
	}

	file.Close();

	if (ret == VDK_EOF) {
		ret = Check();
	}

	return ret;
}

//
//	Initialize as a child disk
//
VDKSTAT VDiskPlain::InitChild(
	ULONG		flags,
	PCHAR		pPath,
	ULONG		version,
	VDisk		*parent)
{
	UNREFERENCED_PARAMETER(flags);
	UNREFERENCED_PARAMETER(pPath);
	UNREFERENCED_PARAMETER(version);
	UNREFERENCED_PARAMETER(parent);

	//
	//	Plain Disk cannot be a child
	//
	return VDK_FUNCTION;
}

//
//	Check paramters
//
VDKSTAT VDiskPlain::Check()
{
	CHAR	path[MAX_PATH];
	PVOID	cbparams[3];
	ULONG	idx;
	VDKSTAT ret;

	FullPath(path);

	//
	//	At least one extent must be present
	//
	if (!m_nExtents) {
		cbparams[0] = path;

		VDiskCallBack(VDISK_CB_EMPTY_IMAGE, cbparams);
		return VDK_DATA;
	}

	//
	//	Check extent offset
	//
	m_nCapacity		= 0;

	for (idx = 0; idx < m_nExtents; idx++) {
		VDiskExtRaw *ext = (VDiskExtRaw *)m_ppExtents[idx];

		if (ext->GetStartOffset() != m_nCapacity) {
			cbparams[0] = ext->GetFullPath();
			cbparams[1] = (PVOID)ext->GetStartOffset();
			cbparams[2] = (PVOID)m_nCapacity;

			//
			//	file range conflict or there is a hole
			//
			if (!VDiskCallBack(VDISK_CB_EXT_OFFSET, cbparams)) {
				return VDK_CANCEL;
			}

			ext->SetStartOffset(m_nCapacity);
			SetFlag(VDISK_FLAG_DIRTY);
		}

		if ((ret = ext->Check()) != VDK_OK) {
			return ret;
		}

		if (ext->IsModified()) {
			SetFlag(VDISK_FLAG_DIRTY);
		}

		m_nCapacity += ext->GetCapacity();
	}

	cbparams[0] = path;

	//
	//	Check geometry
	//
	/*
	if (m_nCylinders * m_nTracks * m_nSectors != m_nCapacity) {

		cbparams[1] = (PVOID)m_nCapacity;
		cbparams[2] = (PVOID)total_size;

		if (!VDiskCallBack(VDISK_CB_EXT_CAPACITY, cbparams)) {
			return VDK_DATA;
		}

		m_nCapacity	= total_size;
		SetGeometry();

		SetFlag(VDISK_FLAG_DIRTY);

		for (idx = 0; idx < m_nExtents; idx++) {
			if (total_size < m_ppExtents[idx]->GetCapacity()) {
				m_ppExtents[idx]->SetCapacity(total_size);
				total_size = 0;
			}
			else {
				total_size -= m_ppExtents[idx]->GetCapacity();
			}
		}
	}
	*/

	//
	//	Any bad parameter?
	//
	if (m_nFlags & VDISK_FLAG_DIRTY) {

		if (VDiskCallBack(VDISK_CB_CONFIRM_FIX, cbparams)) {
			ret = WriteDescriptor(FALSE, FALSE);

			if (ret != VDK_OK) {
				return ret;
			}

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
//	create plain disk descriptor file and extent files
//
VDKSTAT VDiskPlain::Create(ULONG flags)
{
	ULONG idx;
	VDKSTAT ret;

	if (!m_nExtents || !m_ppExtents || !m_ppExtents[0]) {
		return VDK_FUNCTION;
	}

	//
	//	create each extent file
	//
	for (idx = 0; idx < m_nExtents; idx++) {

		ret = m_ppExtents[idx]->Create(flags);

		if (ret != VDK_OK) {
			return ret;
		}
	}

	//
	//	create the description file
	//
	ret = WriteDescriptor(TRUE, (flags & VDISK_CREATE_FORCE));

	return ret;
}

//
//	Write plain disk descriptor file
//
VDKSTAT VDiskPlain::WriteDescriptor(BOOL create, BOOL force)
{
	CHAR buf[MAX_PATH + 30];
	ULONG idx;
	ULONG path_len;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	VDKSTAT ret = VDK_OK;

	//
	//	create/open the description file
	//
	FullPath(buf);

	if (create) {
		ret = VdkCreateFile(&hFile, buf, force);
	}
	else {
		ret = VdkOpenFile(&hFile, buf, strlen(buf), FALSE);
	}

	if (ret != VDK_OK) {
		return ret;
	}

	//
	//	create header entries
	//
	buf[0] = '\0';

	if (m_nVMwareVer >= 3) {

		//
		//	additional entry for VMware 3.x
		//
		sprintf(buf,
			"#vm|TOOLSVERSION      %lu\n"
			"#vm|VERSION           %lu\n",
			m_nToolsFlag, m_nHardwareVer);
	}

	sprintf(buf + strlen(buf),
		"DRIVETYPE     %s\n"
		"CYLINDERS %6lu\n"
		"HEADS     %6lu\n"
		"SECTORS   %6lu\n",
		m_nController == VDISK_CONTROLLER_IDE ? "ide" : "scsi",
		m_nCylinders, m_nTracks, m_nSectors);

	ret = VdkWriteFileAt(hFile, -1, buf, strlen(buf), NULL);

	if (ret != VDK_OK) {
		goto cleanup;
	}

	//
	//	create each extent entry
	//
	path_len = strlen(m_pPath);

	for (idx = 0; idx < m_nExtents; idx++) {
		VDiskExtRaw *ext;
		PCHAR path;

		ext = (VDiskExtRaw *)m_ppExtents[idx];

		path = ext->GetFullPath();

		if (!(m_nFlags & VDISK_FLAG_ABSPATH) &&
			!VdkCmpNoCaseN(m_pPath, path, path_len)) {

			path = ext->GetFileName();
		}

		sprintf(buf, "ACCESS \"%s\" %lu %lu\n",
			path, ext->GetStartOffset(), ext->GetCapacity());

		ret = VdkWriteFileAt(hFile, -1, buf, strlen(buf), NULL);

		if (ret != VDK_OK) {
			goto cleanup;
		}
	}

cleanup:
	if (hFile != INVALID_HANDLE_VALUE) {
		VdkCloseFile(hFile);
	}

	return ret;
}

//
//	Create a new extent object for this instance
//
VDiskExt *VDiskPlain::NewExtent()
{
	return new VDiskExtRaw;
}

//
//	Get path for each extent files
//
void VDiskPlain::GetExtentPath(
	PCHAR pPath, ULONG nSeq)
{
	sprintf(pPath, "%s" PATH_SEPARATOR_STR "%s%lu.dat",
		m_pPath, m_pBody, nSeq + 1);
}

//
//	Set default geometry values
//
void VDiskPlain::SetGeometry()
{
	if (m_nController == VDISK_CONTROLLER_IDE) {

		m_nTracks	= COWD1_TRACKS_IDE_PLAIN;
		m_nSectors	= COWD1_SECTORS_IDE_PLAIN;
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
//	Returns default extent size
//
ULONG VDiskPlain::DefaultExtSize()
{
	ULONG ext_size = COWD1_MAX_EXTENT_PLAIN;

	if (m_nCapacity > ext_size) {

		//
		//	In VMware 2.x plain disks, all extents are the same size
		//
		ULONG extents = (m_nCapacity + ext_size - 1) / ext_size;
		ext_size = (m_nCapacity + extents - 1) / extents;
	}

	return ext_size;
}


