/*
	VDiskVmdk.cpp

	VMDK class
	Copyright (c) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

#include "vmdisk.h"

#include "VDiskVmdk.h"
#include "VDiskExtRaw.h"
#include "VDiskExtVmdk.h"
#include "VDiskUtil.h"
#include "VDiskFile.h"

//
//	Constructor -- set default member values
//
VDiskVmdk::VDiskVmdk()
{
	m_nVMwareVer	= 4;
	m_nHardwareVer	= VMDK_HARDWARE_VMWARE4;
}

//
//	Destructor
//
VDiskVmdk::~VDiskVmdk()
{
}

//
//	Initialize instance from a VMware 4.x descriptor file
//
VDKSTAT VDiskVmdk::Initialize(PCHAR pPath)
{
	VDiskFile file;
	CHAR	buf[MAX_PATH + 40];
	CHAR	path[MAX_PATH];
	PVOID	cbparams[2];
	PCHAR	current;
	ULONG	signature;
	ULONG_PTR len;
	VDKSTAT	ret;

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
	cbparams[0] = pPath;

	//
	//	read first 4 bytes
	//
	signature = 0;

	ret = file.ReadByte((PUCHAR)&signature, sizeof(signature), &len);

	if (ret != VDK_OK) {
		return ret;
	}

	if (len != sizeof(signature)) {
		return VDK_EOF;
	}

	if (signature == VMDK_SIGNATURE) {
		//
		//	it's a monolithic sparse file
		//	-- descriptor offset is stored in header
		//
		VMDK_HEADER vmdk;

		ret = file.ReadByte((PUCHAR)&vmdk + sizeof(signature), 
			sizeof(vmdk) - sizeof(signature), &len);

		if (ret != VDK_OK) {
			return ret;
		}

		if (len != sizeof(vmdk) - sizeof(signature)) {
			return VDK_DATA;
		}

		if (vmdk.DescOffsetLow == 0		||
			vmdk.DescOffsetHigh != 0	||
			vmdk.DescSizeLow == 0		||
			vmdk.DescSizeHigh != 0) {

			VDiskCallBack(VDISK_CB_VMDK_NODESC, cbparams);

			return VDK_DATA;
		}

		ret = VdkSeekFile(file.Handle(),
			vmdk.DescOffsetLow << VDK_BYTE_SHIFT_TO_SECTOR);

		if (ret != VDK_OK) {
			return ret;
		}
	}
	else {
		ret = VdkSeekFile(file.Handle(), 0);

		if (ret != VDK_OK) {
			return ret;
		}
	}

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
		//	parse current line
		//
		if (!VdkCmpNoCaseN(current, "RW ", 3) ||
			!VdkCmpNoCaseN(current, "RDONLY", 6)) {
			VDiskExt	*ext;
			PCHAR		top, tail, p;
			ULONG		capacity;
			CHAR		delim;
			BOOL		sparse;
			ULONG		offset;
			HANDLE		hFile;

			//
			//	search capacity field
			//
			p = current + 3;

			while (*p == ' ') {
				p++;
			}

			capacity = atol(p);

			if (!capacity) {

				capacity = VDiskCallBack(
					VDISK_CB_DESC_CAPACITY, cbparams);

				if (!capacity) {
					return VDK_DATA;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}

			//
			//	search extent type field
			//
			while (isdigit(*p)) {
				p++;
			}

			while (*p == ' ') {
				p++;
			}

			if (!VdkCmpNoCaseN(p, "flat ", 5)) {
				sparse = FALSE;
			}
			else if (!VdkCmpNoCaseN(p, "sparse ", 7)) {
				sparse = TRUE;
			}
			else {

				ULONG type = VDiskCallBack(
					VDISK_CB_DESC_FILETYPE, cbparams);

				if (type == VDK_FILETYPE_FLAT) {
					sparse = FALSE;
				}
				else if (type == VDK_FILETYPE_VMDK) {
					sparse = TRUE;
				}
				else {
					return VDK_DATA;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}

			//
			//	search extent path field
			//
			top = p;

			while (*top && *top != ' ') {
				top++;
			}

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

			if (sparse) {
				offset = 0;
			}
			else {
				//
				//	search "backing offset" field
				//
				p = tail + 1;

				while (*p == ' ') {
					p++;
				}

				if (isdigit(*p)) {
					offset = atol(p);
				}
				else {
					offset = 0;
				}
			}

			//	open the extent file
			*tail = '\0';

			ret = VDiskSearchFile(&hFile, path, m_pPath);

			if (ret != VDK_OK) {
				return ret;
			}

			//
			//	create an extent object
			//
			if (sparse) {
				ext = new VDiskExtVmdk;
			}
			else {
				ext = new VDiskExtRaw;
			}

			if (ext == NULL) {
				ret = VdkLastError();
				VdkCloseFile(hFile);
				return ret;
			}

			ret = AddExtent(ext);

			if (ret != VDK_OK) {
				VdkCloseFile(hFile);
				delete ext;
				return ret;
			}

			ret = ext->SetPath(path);

			if (ret != VDK_OK) {
				VdkCloseFile(hFile);
				return ret;
			}

			ret = ext->Load(hFile);

			VdkCloseFile(hFile);

			if (ret != VDK_OK) {
				return ret;
			}

			ext->SetCapacity(capacity);

			if (!sparse) {
				((VDiskExtRaw *)ext)->SetBackOffset(offset);
			}
		}
		else if (!VdkCmpNoCaseN(current, "ddb.geometry.sectors", 20)) {
			PCHAR p = current + 20;

			while (*p == ' ' || *p == '=' || *p == '\"') {
				p++;
			}

			m_nSectors = atol(p);

			if (!m_nSectors) {

				m_nSectors = VDiskCallBack(
					VDISK_CB_DESC_GEOMETRY, cbparams);

				if (!m_nSectors) {
					return VDK_DATA;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}
		}
		else if (!VdkCmpNoCaseN(current, "ddb.geometry.heads", 18)) {
			PCHAR p = current + 18;

			while (*p == ' ' || *p == '=' || *p == '\"') {
				p++;
			}

			m_nTracks = atol(p);

			if (!m_nTracks) {

				m_nTracks = VDiskCallBack(
					VDISK_CB_DESC_GEOMETRY, cbparams);

				if (!m_nTracks) {
					return VDK_DATA;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}
		}
		else if (!VdkCmpNoCaseN(current, "ddb.geometry.cylinders", 22)) {
			PCHAR p = current + 22;

			while (*p == ' ' || *p == '=' || *p == '\"') {
				p++;
			}

			m_nCylinders = atol(p);

			if (!m_nCylinders) {

				m_nCylinders = VDiskCallBack(
					VDISK_CB_DESC_GEOMETRY, cbparams);

				if (!m_nCylinders) {
					return VDK_DATA;
				}

				SetFlag(VDISK_FLAG_DIRTY);
			}
		}
		else if (!VdkCmpNoCaseN(current, "CID", 3)) {
			PCHAR p = current + 3;

			while (*p == ' ' || *p == '=' || *p == '\"') {
				p++;
			}

			if (!sscanf(p, "%lx", &m_nTimeStamp)) {

				if (!VDiskCallBack(VDISK_CB_DESC_TIMESTAMP, cbparams)) {
					return VDK_CANCEL;
				}

				m_nTimeStamp = (ULONG)-1;
				SetFlag(VDISK_FLAG_DIRTY);
			}
		}
		else if (!VdkCmpNoCaseN(current, "parentCID", 9)) {
			PCHAR p = current + 9;

			while (*p == ' ' || *p == '=' || *p == '\"') {
				p++;

			}

			if (!sscanf(p, "%lx", &m_nParentTS)) {

				if (!VDiskCallBack(VDISK_CB_DESC_TIMESTAMP, cbparams)) {
					return VDK_CANCEL;
				}

				m_nParentTS = (ULONG)-1;
				SetFlag(VDISK_FLAG_DIRTY);
			}

			if (m_nParentTS != (ULONG)-1) {
				SetFlag(VDISK_FLAG_CHILD);
			}
		}
		else if (!VdkCmpNoCaseN(current, "ddb.virtualHWVersion", 20)) {
			PCHAR p = current + 20;

			while (*p == ' ' || *p == '=' || *p == '\"') {
				p++;
			}

			m_nHardwareVer = atol(p);
		}
		else if (!VdkCmpNoCaseN(current, "ddb.adapterType", 15)) {
			PCHAR p = current + 15;

			while (*p == ' ' || *p == '=' || *p == '\"') {
				p++;
			}

			if (!VdkCmpNoCaseN(p, "ide", 3)) {
				m_nController = VDISK_CONTROLLER_IDE;
			}
			else if (!VdkCmpNoCaseN(p, "buslogic", 8) ||
				!VdkCmpNoCaseN(p, "lsilogic", 8)) {
				m_nController = VDISK_CONTROLLER_SCSI;
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
		else if (!VdkCmpNoCaseN(current, "createType", 10)) {
			PCHAR p = current + 10;
			ULONG type;

			while (*p == ' ' || *p == '=' || *p == '\"') {
				p++;
			}

			if (!VdkCmpNoCaseN(p, "twoGbMaxExtentSparse", 20)) {
				type = VDISK_VMDK_SPLIT_SPARSE;
			}
			else if (!VdkCmpNoCaseN(p, "monolithicSparse", 16)) {
				type = VDISK_VMDK_MONO_SPARSE;
			}
			else if (!VdkCmpNoCaseN(p, "twoGbMaxExtentFlat", 18)) {
				type = VDISK_VMDK_SPLIT_FLAT;
			}
			else if (!VdkCmpNoCaseN(p, "monolithicFlat", 14)) {
				type = VDISK_VMDK_MONO_FLAT;
			}
			else {

				type = VDiskCallBack(
					VDISK_CB_DESC_DISKTYPE, cbparams);

				SetFlag(VDISK_FLAG_DIRTY);
			}

			switch (type) {
			case VDISK_VMDK_SPLIT_FLAT:
				ClrFlag(VDISK_FLAG_SINGLE);
				ClrFlag(VDISK_FLAG_CHILD);
				ClrFlag(VDISK_FLAG_SPARSE);
				break;

			case VDISK_VMDK_MONO_FLAT:
				SetFlag(VDISK_FLAG_SINGLE);
				ClrFlag(VDISK_FLAG_CHILD);
				ClrFlag(VDISK_FLAG_SPARSE);
				break;

			case VDISK_VMDK_SPLIT_SPARSE:
				ClrFlag(VDISK_FLAG_SINGLE);
				SetFlag(VDISK_FLAG_SPARSE);
				break;

			case VDISK_VMDK_MONO_SPARSE:
				SetFlag(VDISK_FLAG_SINGLE);
				SetFlag(VDISK_FLAG_SPARSE);
				break;

			default:
				return VDK_DATA;
			}
		}
		else if (!VdkCmpNoCaseN(current, "version", 7)) {
/*
			PCHAR p = current + 7;

			while (*p == ' ' || *p == '=' || *p == '\"') {
				p++;
			}

			m_nVmdkVersion = atol(p);
*/
		}
		else if (!VdkCmpNoCaseN(current, "parentFileNameHint", 18)) {
			PCHAR top = current + 18;
			PCHAR tail;

			while (*top == ' ' || *top == '=' || *top == '\"') {
				top++;
			}

			tail = top;

			while (*tail && *tail != '\"') {
				tail++;
			}

			*tail = '\0';

			ret = StoreParentPath(top);

			if (ret != VDK_OK) {
				return ret;
			}

			SetFlag(VDISK_FLAG_CHILD);
		}
		else if (!VdkCmpNoCaseN(current, "ddb.toolsVersion", 16)) {
			PCHAR p = current + 16;

			while (*p == ' ' || *p == '=' || *p == '\"') {
				p++;
			}

			m_nToolsFlag = atol(p);
		}
		else if (*current && *current != '#') {

			if (!VDiskCallBack(VDISK_CB_DESC_BADENTRY, cbparams)) {
				return VDK_CANCEL;
			}

			SetFlag(VDISK_FLAG_DIRTY);
		}
	}

	if (ret == VDK_EOF) {
		return Check();
	}
	else {
		return ret;
	}
}

//
//	create VMDK descriptor file and extent files
//
VDKSTAT VDiskVmdk::Create(ULONG flags)
{
	ULONG idx;
	VDKSTAT ret;

	if (!m_nExtents || !m_ppExtents || !m_ppExtents[0]) {
		return VDK_FUNCTION;
	}

	if (m_nFlags & VDISK_FLAG_SINGLE) {
		flags |= VDISK_CREATE_SINGLE;
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
	//	create the description file (or write description into monolithic file)
	//
	ret = WriteDescriptor(TRUE, (flags & VDISK_CREATE_FORCE));

	return ret;
}

//
//	write VMDK descriptor file
//
VDKSTAT VDiskVmdk::WriteDescriptor(BOOL create, BOOL force)
{
	CHAR	buf[MAX_PATH + 30];
	ULONG	idx;
	ULONG	path_len;
	HANDLE	hFile;
	VDKSTAT	ret = VDK_OK;

	//
	//	create/open the description file
	//
	sprintf(buf, "%s" PATH_SEPARATOR_STR "%s.%s",
		m_pPath, m_pBody, m_pExtension);

	if (create) {
		ret = VdkCreateFile(&hFile, buf, force);
	}
	else {
		ret = VdkOpenFile(&hFile, buf, strlen(buf), FALSE);
	}

	if (ret != VDK_OK) {
		return ret;
	}

	if ((m_nFlags & VDISK_FLAG_SINGLE) &&
		(m_nFlags & VDISK_FLAG_SPARSE)) {

		//
		//	adjust descriptor offset for monolithic sparse file
		//
		ret = VdkSeekFile(hFile,
			((VDiskExtVmdk *)m_ppExtents[0])->GetHeader()->DescOffsetLow);

		if (ret != VDK_OK) {
			goto cleanup;
		}
	}

	//
	//	create header entries
	//
	sprintf(buf,
		"# Disk DescriptorFile\n"
		"version=1\n"
		"CID=%08lx\n"
		"parentCID=%08lx\n"
		"createType=\"%s%s\"\n",

		m_nTimeStamp,
		m_nParentTS,
		(m_nFlags & VDISK_FLAG_SINGLE) ? "monolithic" : "twoGbMaxExtent",
		(m_nFlags & VDISK_FLAG_SPARSE) ? "Sparse" : "Flat");

	ret = VdkWriteFileAt(hFile, -1, buf, strlen(buf), NULL);

	if (ret != VDK_OK) {
		goto cleanup;
	}

	if (m_nFlags & VDISK_FLAG_CHILD) {
		sprintf(buf, "parentFileNameHint=\"%s\"\n", m_pParentPath);

		ret = VdkWriteFileAt(hFile, -1, buf, strlen(buf), NULL);

		if (ret != VDK_OK) {
			goto cleanup;
		}
	}

	//
	//	create each extent entry
	//
	strcpy(buf, "\n# Extent description\n");

	ret = VdkWriteFileAt(hFile, -1, buf, strlen(buf), NULL);

	if (ret != VDK_OK) {
		goto cleanup;
	}

	path_len = strlen(m_pPath);

	for (idx = 0; idx < m_nExtents; idx++) {
		VDiskExt *ext = *(m_ppExtents + idx);
		PCHAR path;

		path = ext->GetFullPath();

		if (!(m_nFlags & VDISK_FLAG_ABSPATH) &&
			!VdkCmpNoCaseN(m_pPath, path, path_len)) {

			path = ext->GetFileName();
		}

		if (ext->GetFileType() == VDK_FILETYPE_VMDK) {
			sprintf(buf, "RW %lu SPARSE \"%s\"\n",
				ext->GetCapacity(),
				path);
		}
		else {
			sprintf(buf, "RW %lu FLAT \"%s\" %lu\n",
				ext->GetCapacity(),
				path,
				((VDiskExtRaw *)ext)->GetBackOffset());
		}

		ret = VdkWriteFileAt(hFile, -1, buf, strlen(buf), NULL);

		if (ret != VDK_OK) {
			goto cleanup;
		}
	}

	//
	//	create trailing entries
	//
	strcpy(buf,
		"\n# The Disk Data Base \n"
		"#DDB\n\n");

	ret = VdkWriteFileAt(hFile, -1, buf, strlen(buf), NULL);

	if (ret != VDK_OK) {
		goto cleanup;
	}

	if (!(m_nFlags & VDISK_FLAG_CHILD)) {
		sprintf(buf,
			"ddb.virtualHWVersion = \"%lu\"\n"
			"ddb.geometry.cylinders = \"%lu\"\n"
			"ddb.geometry.heads = \"%lu\"\n"
			"ddb.geometry.sectors = \"%lu\"\n"
			"ddb.adapterType = \"%s\"\n",
			m_nHardwareVer,
			m_nCylinders,
			m_nTracks,
			m_nSectors,
			(m_nController == VDISK_CONTROLLER_IDE) ? "ide" : "buslogic");

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
//	Create a new extent object for this disk instance
//
VDiskExt *VDiskVmdk::NewExtent()
{
	if (m_nFlags & VDISK_FLAG_SPARSE) {
		return new VDiskExtVmdk;
	}
	else {
		return new VDiskExtRaw;
	}
}

//
//	Get path for each extent files
//
void VDiskVmdk::GetExtentPath(
	PCHAR pPath, ULONG nSeq)
{
	if (m_nFlags & VDISK_FLAG_SPARSE) {
		if (m_nFlags & VDISK_FLAG_SINGLE) {
			sprintf(pPath, "%s" PATH_SEPARATOR_STR "%s.%s",
				m_pPath, m_pBody, m_pExtension);
		}
		else {
			sprintf(pPath, "%s" PATH_SEPARATOR_STR "%s-s%03lu.%s",
				m_pPath, m_pBody, nSeq + 1, m_pExtension);
		}
	}
	else {
		if (m_nFlags & VDISK_FLAG_SINGLE) {
			sprintf(pPath, "%s" PATH_SEPARATOR_STR "%s-flat.%s",
				m_pPath, m_pBody, m_pExtension);
		}
		else {
			sprintf(pPath, "%s" PATH_SEPARATOR_STR "%s-f%03lu.%s",
				m_pPath, m_pBody, nSeq + 1, m_pExtension);
		}
	}
}

//
//	Set default geometry values
//
void VDiskVmdk::SetGeometry()
{
	if (m_nController == VDISK_CONTROLLER_IDE) {

		m_nTracks	= VMDK_TRACKS_IDE;
		m_nSectors	= VMDK_SECTORS_IDE;
	}
	else {
		if (m_nCapacity <=
			VMDK_SECTORS_SCSI_1023M * VMDK_TRACKS_SCSI_1023M * 1023) {

			m_nSectors	= VMDK_SECTORS_SCSI_1023M;
			m_nTracks	= VMDK_TRACKS_SCSI_1023M;
		}
		else if (m_nCapacity <=
			VMDK_SECTORS_SCSI_2046M * VMDK_TRACKS_SCSI_2046M * 1023) {

			m_nSectors	= VMDK_SECTORS_SCSI_2046M;
			m_nTracks	= VMDK_TRACKS_SCSI_2046M;
		}
		else {
			m_nSectors	= VMDK_SECTORS_SCSI_LARGE;
			m_nTracks	= VMDK_TRACKS_SCSI_LARGE;
		}
	}

	m_nCylinders = m_nCapacity / (m_nSectors * m_nTracks);
}

//
//	Set default Timestamp values;
//
void VDiskVmdk::SetDefaultTS()
{
	m_nParentTS	= (ULONG)-1;
	m_nTimeStamp = (ULONG)-2;
}

//
//	Get default extent size
//
ULONG VDiskVmdk::DefaultExtSize()
{
	ULONG ext_size;

	if (m_nFlags & VDISK_FLAG_SINGLE) {
		ext_size = m_nCapacity;
	}
	else {
		if (m_nFlags & VDISK_FLAG_SPARSE) {
			ext_size = VMDK_MAX_EXTENT_SPARSE;
		}
		else {
			ext_size = VMDK_MAX_EXTENT_SOLID;
		}

		if (ext_size > m_nCapacity) {
			ext_size = m_nCapacity;
		}
	}

	return ext_size;
}

//
//	Check VMDK sparse disk
//
VDKSTAT VDiskVmdk::Check()
{
	PVOID	cbparams[3];
	CHAR	path[MAX_PATH];
	ULONG	idx;
	VDKSTAT ret;

	FullPath(path);
	cbparams[0] = path;

	//
	//	At least one extent must be present
	//
	if (!m_nExtents) {
		VDiskCallBack(VDISK_CB_EMPTY_IMAGE, cbparams);
		return VDK_DATA;
	}

	//
	//	Sum up extent capacity
	//
	m_nCapacity = 0;

	for (idx = 0; idx < m_nExtents; idx++) {
		if ((ret = m_ppExtents[idx]->Check()) != VDK_OK) {
			return ret;
		}

		if (m_ppExtents[idx]->IsModified()) {
			SetFlag(VDISK_FLAG_DIRTY);
		}

		m_nCapacity += m_ppExtents[idx]->GetCapacity();
	}

	//
	//	Check total capacity
	//
	/*
	if (total_size != m_nCapacity) {

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

