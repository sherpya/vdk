/*
	vdkioctl.h

	VDK WinNT kernel-mode driver interface header
	Copyright (C) 2003 Ken Kato
*/

#ifndef _VDK_IOCTL_H_
#define _VDK_IOCTL_H_

//
// VDK device basename
//

#ifdef _NTDDK_
#define VDK_DEVICE_BASENAME			L"VirtualDK"
#define VDK_REG_DISKNUM_VALUE		L"NumberOfDisks"
#else
#define VDK_DEVICE_BASENAME			"VirtualDK"
#define VDK_REG_DISKNUM_VALUE		"NumberOfDisks"
#endif

//
// Disk Device Number
//
#define VDK_MAXIMUM_DISK_NUM		22
#define VDK_DEFAULT_DISK_NUM		4

//
// Device IO control values
//

// Get VDK driver version

#define IOCTL_VDK_GET_VERSION		CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x810,				\
										METHOD_BUFFERED,	\
										FILE_ANY_ACCESS)

// Mount existing image file or create a new image file of specified size

#define IOCTL_VDK_OPEN_FILE			CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x811,				\
										METHOD_BUFFERED,	\
										FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// Update device configuration according to current image
// Must be called after IOCTL_VDK_OPEN_FILE for the driver to
// recognize the partitions

#define IOCTL_VDK_UPDATE_DEVICE		CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x812,				\
										METHOD_NEITHER,		\
										FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// Unmount currently mounted image file

#define IOCTL_VDK_CLOSE_FILE		CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x813,				\
										METHOD_BUFFERED,	\
										FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// Get total size of image file information

#define IOCTL_VDK_QUERY_FILE_SIZE	CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x814,				\
										METHOD_BUFFERED,	\
										FILE_ANY_ACCESS)

// Get currently mounted image file information

#define IOCTL_VDK_QUERY_FILE		CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x815,				\
										METHOD_BUFFERED,	\
										FILE_ANY_ACCESS)

// Get number of partition devices attached to a disk device

#define IOCTL_VDK_NUMBER_OF_PARTS	CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x816,				\
										METHOD_BUFFERED,	\
										FILE_ANY_ACCESS)

// Create new virtual disk device

#define IOCTL_VDK_CREATE_DISK		CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x817,				\
										METHOD_NEITHER,		\
										FILE_ANY_ACCESS)

// Remove the last virtual disk device

#define IOCTL_VDK_DELETE_DISK		CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x818,				\
										METHOD_NEITHER,		\
										FILE_ANY_ACCESS)

// Notify that the device is dismounted

#define IOCTL_VDK_NOTIFY_DISMOUNT	CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x819,				\
										METHOD_NEITHER,		\
										FILE_ANY_ACCESS)


// Get VDK driver information

#define IOCTL_VDK_DRIVER_INFO		CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x81d,				\
										METHOD_BUFFERED,	\
										FILE_ANY_ACCESS)

// Get VDK device information

#define IOCTL_VDK_DEVICE_INFO		CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x81e,				\
										METHOD_BUFFERED,	\
										FILE_ANY_ACCESS)

// Set driver trace flags

#define IOCTL_VDK_DEBUG_TRACE		CTL_CODE(				\
										IOCTL_DISK_BASE,	\
										0x81f,				\
										METHOD_BUFFERED,	\
										FILE_ANY_ACCESS)

//
//	VDK_DRIVER_INFO structure
//
typedef struct _VDK_DRIVER_INFO
{
	ULONG	DiskDevices;		// Number of disk devices
	ULONG	AttachedParts;		// Number of attached partition devices
	ULONG	OrphanedParts;		// Number of orphaned partition devices
	ULONG	TotalReference;		// Total Number of reference
}
VDK_DRIVER_INFO, *PVDK_DRIVER_INFO;

//
//	VDK_DEVICE_INFO structure
//
#define MAX_DEVNAME_LEN		36

// 	DeviceType values
#define VDK_DEVICE_DISK		1
#define VDK_DEVICE_PART 	2

typedef struct _VDK_DEVICE_INFO
{
	ULONG	DeviceType;
	ULONG	Zombie;
	CHAR	DeviceName[MAX_DEVNAME_LEN];
	CHAR	SymbolicLink[MAX_DEVNAME_LEN];
	ULONG	ReferenceCount;
}
VDK_DEVICE_INFO, *PVDK_DEVICE_INFO;


#endif	// _VDK_IOCTL_H_
