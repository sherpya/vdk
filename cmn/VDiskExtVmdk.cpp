/*
	VDiskExtVmdk.cpp

	VMDK sparse extent class
	Copyright (c) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

#include "vmdisk.h"

#include "VDisk.h"
#include "VDiskExtVmdk.h"
#include "VDiskUtil.h"

//
//	Constructor
//
VDiskExtVmdk::VDiskExtVmdk()
{
	VdkZeroMem(&m_Header, sizeof(m_Header));
}

//
//	Destructor
//
VDiskExtVmdk::~VDiskExtVmdk()
{
}

//
//	Load and obtain parameters from extent file
//
VDKSTAT VDiskExtVmdk::Load(HANDLE hFile)
{
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
	//	read VMDK header
	//
	ret = VdkReadFileAt(hFile, 0, &m_Header, sizeof(VMDK_HEADER), NULL);

	return ret;
}

//
//	Check parameter consistency
//
VDKSTAT VDiskExtVmdk::Check()
{
	PVOID cbparams[3];
	ULONG filesize;

	cbparams[0] = m_pFullPath;

	//
	//	Signature
	//
	if (m_Header.Signature != VMDK_SIGNATURE) {

		cbparams[1] = (PVOID)m_Header.Signature;

		if (!VDiskCallBack(VDISK_CB_SIGNATURE, cbparams)) {
			return VDK_CANCEL;
		}

		m_Header.Signature = VMDK_SIGNATURE;
		SetModify();
	}

	//
	//	File version
	//
	if (m_Header.FileVersion != VMDK_FILEVER_VMWARE4) {

		cbparams[1] = (PVOID)m_Header.FileVersion;

		m_Header.FileVersion = VDiskCallBack(
			VDISK_CB_VMDK_FILEVER, cbparams);

		if (m_Header.FileVersion != VMDK_FILEVER_VMWARE4) {
			return VDK_CANCEL;
		}

		SetModify();
	}

	//
	//	File Capacity
	//
	if (m_Header.CapacityLow == 0 ||
		m_Header.CapacityHigh != 0) {

		cbparams[1] = &(m_Header.CapacityLow);

		if (!VDiskCallBack(VDISK_CB_VMDK_FILECAP, cbparams)) {
			return VDK_DATA;
		}

		SetModify();
	}

	//
	//	Granularity
	//	Although VMware does not limit the maximum value,
	//	too large values are simply impractical.
	//
	if (m_Header.GranularityHigh != 0	||
		(m_Header.GranularityLow != 16	&&
		m_Header.GranularityLow != 32	&&
		m_Header.GranularityLow != 64	&&
		m_Header.GranularityLow != 128	&&
		m_Header.GranularityLow != 256	&&
		m_Header.GranularityLow != 512	&&
		m_Header.GranularityLow != 1024)) {

		cbparams[1] = &(m_Header.GranularityLow);

		if (!VDiskCallBack(VDISK_CB_VMDK_GRANULARITY, cbparams)) {
			return VDK_DATA;
		}

		SetModify();
	}

	//
	//	Descriptor Offset
	//
	filesize = (ULONG)(m_nFileSize >> VDK_BYTE_SHIFT_TO_SECTOR);

	if (m_Header.DescOffsetHigh != 0 ||
		(m_Header.DescSizeLow && !m_Header.DescOffsetLow) ||
		m_Header.DescOffsetLow >= filesize) {

		cbparams[1] = &(m_Header.DescOffsetLow);

		if (!VDiskCallBack(VDISK_CB_VMDK_DESCOFFSET, cbparams)) {
			return VDK_DATA;
		}

		SetModify();
	}

	//
	//	Descriptor Size
	//
	if (m_Header.DescSizeHigh != 0 ||
		(m_Header.DescOffsetLow && !m_Header.DescSizeLow) ||
		m_Header.DescSizeLow + m_Header.DescOffsetLow > filesize) {

		cbparams[1] = &(m_Header.DescSizeLow);

		if (!VDiskCallBack(VDISK_CB_VMDK_DESCSIZE, cbparams)) {
			return VDK_DATA;
		}

		SetModify();
	}

	//
	//	Number of Grain Table Entries per Grain Table
	//	Although VMware does not limit the minimum / maximum value,
	//	too large values are simply impractical.
	//
	if (m_Header.numGTEsPerGT != 128 &&
		m_Header.numGTEsPerGT != 256 &&
		m_Header.numGTEsPerGT != 512 &&
		m_Header.numGTEsPerGT != 1024) {

		cbparams[1] = (PVOID)m_Header.numGTEsPerGT;

		if (!VDiskCallBack(VDISK_CB_VMDK_GTESPERGT, cbparams)) {
			return VDK_DATA;
		}

		SetModify();
	}

	//
	//	Grain Directory 1 Offset
	//
	if (!m_Header.rgdOffsetLow || m_Header.rgdOffsetHigh ||
		m_Header.rgdOffsetLow >= filesize) {

		cbparams[1] = &(m_Header.rgdOffsetLow);

		if (!VDiskCallBack(VDISK_CB_VMDK_GDOFFSET,  cbparams)) {
			return VDK_DATA;
		}

		SetModify();
	}

	//
	//	Grain Directory 2 Offset
	//
	if (!m_Header.gdOffsetLow || m_Header.gdOffsetHigh ||
		m_Header.gdOffsetLow >= filesize) {

		cbparams[1] = &(m_Header.gdOffsetLow);

		if (!VDiskCallBack(VDISK_CB_VMDK_GDOFFSET, cbparams)) {
			return VDK_DATA;
		}

		SetModify();
	}

	//
	//	Grain Entry Offset
	//
	if (m_Header.GrainOffsetLow == 0 || m_Header.GrainOffsetHigh ||
		m_Header.GrainOffsetLow > filesize) {

		cbparams[1] = &(m_Header.GrainOffsetLow);

		if (!VDiskCallBack(VDISK_CB_VMDK_GRAINOFFSET, cbparams)) {
			return VDK_DATA;
		}

		SetModify();
	}

	//
	//	CheckBytes
	//
	if (memcmp(m_Header.CheckBytes,
		VMDK_HEADER_CHECKBYTES,
		sizeof(VMDK_HEADER_CHECKBYTES) - 1)) {

		if (!VDiskCallBack(VDISK_CB_VMDK_CHECKBYTES, cbparams)) {
			return VDK_CANCEL;
		}

		SetModify();
	}

	//
	//	compare capacity in the header and descriptor
	//
	if (m_nCapacity != m_Header.CapacityLow) {

		cbparams[1] = (PVOID)m_Header.CapacityLow;
		cbparams[2] = (PVOID)m_nCapacity;

		if (!VDiskCallBack(VDISK_CB_VMDK_SIZEMISMATCH, cbparams)) {

			return VDK_CANCEL;
		}

		m_nCapacity = m_Header.CapacityLow;
		SetModify();
	}

	return VDK_OK;
}

//
//	Update VMDK header
//
VDKSTAT VDiskExtVmdk::Update()
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

//
//	Create an actural VMDK file
//
VDKSTAT VDiskExtVmdk::Create(ULONG flags)
{
	HANDLE	hFile;
	ULONG	grains;
	ULONG	gd_size;
	ULONG	gt_size;
	ULONG	gt_count;
	VDKSTAT	ret;

	VdkZeroMem(&m_Header, sizeof(m_Header));

	// number of grains per file
	grains = (m_nCapacity + VMDK_DEFAULT_GRANULARITY - 1) /
		VMDK_DEFAULT_GRANULARITY;

	// size of a single grain table (sectors)
	gt_size = ((VMDK_DEFAULT_NUMGTESPERGT * sizeof(ULONG)) + 511)
		>> VDK_BYTE_SHIFT_TO_SECTOR;

	// number of necessary grain tables
	gt_count = (grains + VMDK_DEFAULT_NUMGTESPERGT - 1) /
		VMDK_DEFAULT_NUMGTESPERGT;

	// size of the grain directory (sectors)
	gd_size = (gt_count * sizeof(ULONG) + 511)
		>> VDK_BYTE_SHIFT_TO_SECTOR;

	// fixed parameters
	m_Header.Signature				= VMDK_SIGNATURE;
	m_Header.FileVersion			= VMDK_FILEVER_VMWARE4;
	m_Header.Flags					= (VMDK_FLAG_UNKNOWN1 | VMDK_FLAG_UNKNOWN2);

	// capacity and mapping parameters
	*(INT64 *)&m_Header.CapacityLow		= m_nCapacity;
	*(INT64 *)&m_Header.GranularityLow	= VMDK_DEFAULT_GRANULARITY;
	m_Header.numGTEsPerGT				= VMDK_DEFAULT_NUMGTESPERGT;

	// offset parameters
	if (flags & VDISK_CREATE_SINGLE) {
		*(INT64 *)&m_Header.DescOffsetLow	= VMDK_DEFAULT_DESCOFFSET;
		*(INT64 *)&m_Header.DescSizeLow		= VMDK_DEFAULT_DESCSIZE;

		*(INT64 *)&m_Header.rgdOffsetLow	=
			VMDK_DEFAULT_DESCOFFSET + VMDK_DEFAULT_DESCSIZE;
	}
	else {
		*(INT64 *)&m_Header.DescOffsetLow	= 0;
		*(INT64 *)&m_Header.DescSizeLow		= 0;
		*(INT64 *)&m_Header.rgdOffsetLow	= 1;
	}

	*(INT64 *)&m_Header.gdOffsetLow			=
		*(INT64 *)&m_Header.rgdOffsetLow +
		gd_size + (gt_size * gt_count);

	// grain entry offset == initial file size
	*(INT64 *)&m_Header.GrainOffsetLow		=
		((*(INT64 *)&m_Header.gdOffsetLow + gd_size + (gt_size * gt_count) +
		(VMDK_DEFAULT_GRANULARITY - 1)) / VMDK_DEFAULT_GRANULARITY) *
		VMDK_DEFAULT_GRANULARITY;

	// check bytes
	VdkCopyMem(
		m_Header.CheckBytes,
		VMDK_HEADER_CHECKBYTES,
		sizeof(m_Header.CheckBytes));

	// open/create file
	ret = VdkCreateFile(&hFile, m_pFullPath, (flags & VDISK_CREATE_FORCE));

	if (ret != VDK_OK) {
		return ret;
	}

	// update header and actual file size
	m_nFileSize = *(INT64 *)&m_Header.GrainOffsetLow << VDK_BYTE_SHIFT_TO_SECTOR;

	ret = UpdateFile(hFile);

	if (ret != VDK_OK) {
		VdkCloseFile(hFile);
		return ret;
	}

	// create grain directories
	ret = WriteGrainDir(hFile, m_Header.rgdOffsetLow, gd_size, gt_count, gt_size);

	if (ret != VDK_OK) {
		VdkCloseFile(hFile);
		return ret;
	}

	ret = WriteGrainDir(hFile, m_Header.gdOffsetLow, gd_size, gt_count, gt_size);

	if (ret != VDK_OK) {
		VdkCloseFile(hFile);
		return ret;
	}

	VdkCloseFile(hFile);

	return ret;
}

//
//	Write VMDK header
//
VDKSTAT VDiskExtVmdk::UpdateFile(HANDLE hFile)
{
	VDKSTAT	ret;

	ret = VdkWriteFileAt(hFile, 0, &m_Header, sizeof(VMDK_HEADER), NULL);

	if (ret != VDK_OK) {
		VdkCloseFile(hFile);

		return ret;
	}

	ret = VdkSetFileSize(hFile, m_nFileSize);

	return ret;
}

//
//	Create initial grain directory
//
VDKSTAT VDiskExtVmdk::WriteGrainDir(
	HANDLE	hFile,
	ULONG	offset,
	ULONG	gd_size,
	ULONG	gt_count,
	ULONG	gt_size)
{
	VDKSTAT ret;
	
	ret = VdkSeekFile(hFile, (INT64)offset << VDK_BYTE_SHIFT_TO_SECTOR);

	if (ret != VDK_OK) {
		return ret;
	}

	offset += gd_size;

	do {
		ret = VdkWriteFileAt(hFile, -1, &offset, sizeof(offset), NULL);

		if (ret != VDK_OK) {
			return ret;
		}

		offset += gt_size;
	}
	while (--gt_count);

	return VDK_OK;
}
