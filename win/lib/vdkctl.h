/*
	vdkctl.h

	Virtual Disk driver control routines header
	Copyright (C) 2003 Ken Kato
*/

#ifndef _VDKCTL_H_
#define _VDKCTL_H_

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

//
//	custom SERVICE STATE value
//
#define VDK_NOT_INSTALLED		0xffffffff

///////////////////////////////////////
//
//	Driver configuration routines
//

//	Get current driver config information

DWORD VdkGetDriverConfig(
	LPTSTR		driver_path,
	LPDWORD		start_type,
	PULONG		device_num);

//	Get current driver state

DWORD VdkGetDriverState(
	LPDWORD		current_state);

//	Install the driver

DWORD VdkInstall(
	LPCTSTR		driver_path,
	BOOL		auto_start);

//	Uninstall the driver

DWORD VdkRemove();

//	Start the driver

DWORD VdkStart(
	DWORD		*state);

//	Stop the driver

DWORD VdkStop(
	DWORD		*state);

//	Set initial number of devices

DWORD VdkSetDeviceNum(
	ULONG		device_num);

//	Check driver file version

DWORD VdkCheckFileVersion(
	LPCTSTR		driver_path,
	PULONG		version);

///////////////////////////////////////
//
//	Device control routines
//

//	Open a Virtual Disk device

HANDLE VdkOpenDevice(
	ULONG		disk_number,
	ULONG		part_number);

//	Check running driver version

DWORD VdkCheckVersion(
	HANDLE		hDevice,
	PULONG		version);

//	Get running driver information

DWORD VdkGetDriverInfo(
	HANDLE		hDevice,
	PULONG		disk_device,
	PULONG		attached_part,
	PULONG		orphaned_part,
	PULONG		reference_count);

//	Create a new disk device

DWORD VdkCreateDisk(
	HANDLE		hDevice);

//	Delete the last disk device

DWORD VdkDeleteDisk(
	HANDLE		hDevice);

//	Check if the device is active/writable

DWORD VdkCheckDeviceState(
	HANDLE		hDevice,
	ULONG		disk_number,
	ULONG		part_number);

//	Get device information

DWORD VdkGetDeviceList(
	PULONG		device_num,
	PVDK_DEVICE_INFO	*device_info);

#ifdef VDK_DEBUG

//	Set/get trace flags

ULONG VdkTraceFlags(
	PULONG		flags);

#endif
///////////////////////////////////////
//
//	Image file routines
//

//	Open image files

DWORD VdkOpen(
	HANDLE		hDevice,
	ULONG		disk_number,
	PVOID		pDisk,
	ULONG		AccessType);

//	Close image files

DWORD VdkClose(
	HANDLE		hDevice,
	ULONG		disk_number,
	ULONG		graceful);

//	Get current image file information
typedef struct _VDK_OPEN_FILE_INFO *PVDK_OPEN_FILE_INFO;

DWORD VdkGetFileInfo(
	HANDLE		hDevice,
	ULONG		disk_number,
	PVDK_OPEN_FILE_INFO		*file_info);

//	Dismount a virtual disk partition

DWORD VdkDismount(
	HANDLE		hDevice,
	ULONG		disk_number,
	ULONG		part_number,
	ULONG		unlock);

DWORD VdkDismountAll(BOOL zombie_only);

///////////////////////////////////////
//
//	Drive letter handling routines
//

//	Get VDK device name in the kernel namespace

DWORD VdkGetDeviceName(
	ULONG		disk_number,
	ULONG		part_number,
	LPTSTR		buf);

//	Get current drive letter

DWORD VdkGetDriveLetter(
	ULONG		disk_number,
	ULONG		part_number,
	TCHAR		*drive_letter);

//	Assign a drive letter

DWORD VdkSetDriveLetter(
	ULONG		disk_number,
	ULONG		part_number,
	TCHAR		drive_letter);

//	Remove a drive letter

DWORD VdkDelDriveLetter(
	TCHAR		drive_letter);

//	Choose first available drive letter

char ChooseDriveLetter();

//
//	Close current image
//

typedef DWORD (*VDK_CALLBACK)(PVOID param, DWORD err);

DWORD VdkCloseDrive(
	ULONG			disk_number,
	VDK_CALLBACK	retrycb,
	VDK_CALLBACK	contcb,
	PVOID			param);

#ifdef __cplusplus
}
#endif	//	__cplusplus

#endif	//	_VDKCTL_H_

//	End Of File
