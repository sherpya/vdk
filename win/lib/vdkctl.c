/*
	vdkctl.c

	Virtual Disk driver control routines
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkfile.h"
#include "vdkioctl.h"
#include "vdkver.h"
#include "vdkctl.h"

#include "VDiskUtil.h"

#pragma warning(push,3)
#include <winioctl.h>
#pragma warning(pop)

//
//	Version info translation
//
#define VERSIONINFO_PATH	"\\StringFileInfo\\" VDK_VERSIONINFO_LANG "\\OriginalFileName"

//
//	DOS device name (\\.\VirtualDK<x>\Partition<y>)
//
#define VDK_DOS_TEMPLATE	VDK_DEVICE_BASENAME "%u" "\\Partition%u"
#define VDK_DEV_TEMPLATE	"\\\\.\\" VDK_DOS_TEMPLATE
#define VDK_REG_CONFIG_KEY	"System\\CurrentControlSet\\Services\\" VDK_DEVICE_BASENAME

//
//	Get Virtual Disk driver configuration
//
DWORD VdkGetDriverConfig(
	LPTSTR		driver_path,
	LPDWORD		start_type,
	PULONG		device_num)
{
	SC_HANDLE	hScManager;				// Service Control Manager
	SC_HANDLE	hService;				// Service (= Driver)
	LPQUERY_SERVICE_CONFIG config = NULL;
	DWORD		return_len;
	DWORD		ret = ERROR_SUCCESS;

	if (driver_path) {
		ZeroMemory(driver_path, MAX_PATH);
	}
	if (start_type) {
		*start_type = (DWORD)-1;
	}
	if (device_num) {
		*device_num = 0;
	}

	//	Connect to the Service Control Manager

	hScManager = OpenSCManager(NULL, NULL, 0);

	if (hScManager == NULL) {
		ret = GetLastError();

		VDKTRACE(0,
			("VdkGetDriverConfig: OpenSCManager() - %s\n",
			VdkStatusStr(ret)));

		return ret;
	}

	//	Open Existing Service Object

	hService = OpenService(
		hScManager,						// Service control manager
		VDK_DEVICE_BASENAME,			// service name
		SERVICE_QUERY_CONFIG);			// service access mode

	if (hService == NULL) {
		ret = GetLastError();

		VDKTRACE(0,
			("VdkGetDriverConfig: OpenService(SERVICE_QUERY_CONFIG) - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

	//	Get the return_len of config information

	if (!QueryServiceConfig(hService, NULL, 0, &return_len)) {
		ret = GetLastError();

		if (ret == ERROR_INSUFFICIENT_BUFFER) {
			ret = ERROR_SUCCESS;
		}
		else {
			VDKTRACE(0, ("VdkGetDriverConfig: QueryServiceConfig() - %s\n",
				VdkStatusStr(ret)));

			goto cleanup;
		}
	}

	//	allocate a required buffer

	config = (QUERY_SERVICE_CONFIG *)VdkAllocMem(return_len);

	if (config == NULL) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkGetDriverConfig: VdkAllocMem(%lu) - %s\n",
			return_len, VdkStatusStr(ret)));

		goto cleanup;
	}

	//	get the config information

	if (!QueryServiceConfig(hService, config, return_len, &return_len)) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkGetDriverConfig: QueryServiceConfig() - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

	//	copy information to output buffer

	if (driver_path) {
		if (strncmp(config->lpBinaryPathName, "\\??\\", 4) == 0) {
			strncpy(
				driver_path,
				config->lpBinaryPathName + 4,
				MAX_PATH);
		}
		else {
			strncpy(
				driver_path,
				config->lpBinaryPathName,
				MAX_PATH);
		}
	}

	if (start_type) {
		*start_type = config->dwStartType;
	}

	if (device_num) {
		//
		//	Get number of initial devices from registry
		//
		HKEY hKey;

		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			VDK_REG_CONFIG_KEY, 0, KEY_READ, &hKey);

		if (ret == ERROR_SUCCESS) {
			return_len = sizeof(DWORD);

			ret = RegQueryValueEx(
				hKey, VDK_REG_DISKNUM_VALUE, 0, NULL,
				(LPBYTE)device_num, &return_len);

			if (ret == ERROR_PATH_NOT_FOUND ||
				ret == ERROR_FILE_NOT_FOUND) {
				ret = ERROR_SUCCESS;
				*device_num = VDK_DEFAULT_DISK_NUM;
			}

			RegCloseKey(hKey);
		}

		if (ret != ERROR_SUCCESS) {
			VDKTRACE(0, ("VdkGetDriverConfig: RegOpenKeyEx - %s\n",
				VdkStatusStr(ret)));

			*device_num = 0;
		}
	}

cleanup:
	//	Free service config buffer

	if (config) {
		VdkFreeMem(config);
	}

	//	Close the service object handle

	if (hService) {
		CloseServiceHandle(hService);
	}

	//	Close handle to the service control manager.

	if (hScManager) {
		CloseServiceHandle(hScManager);
	}

	return ret;
}

//
//	Get Virtual Disk driver state
//
DWORD VdkGetDriverState(
	LPDWORD current_state)
{
	SC_HANDLE		hScManager = NULL;	// Service Control Manager
	SC_HANDLE		hService = NULL;	// Service (= Driver)
	SERVICE_STATUS	status;
	DWORD			ret = ERROR_SUCCESS;

	if (current_state) {
		*current_state = 0;
	}

	//	Connect to the Service Control Manager

	hScManager = OpenSCManager(NULL, NULL, 0);

	if (hScManager == NULL) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkGetDriverState: OpenSCManager() - %s\n",
			VdkStatusStr(ret)));

		return ret;
	}

	//	Open Existing Service Object

	hService = OpenService(
		hScManager,						// Service control manager
		VDK_DEVICE_BASENAME,			// service name
		SERVICE_QUERY_STATUS);			// service access mode

	if (hService == NULL) {
		ret = GetLastError();

		if (ret == ERROR_SERVICE_DOES_NOT_EXIST) {
			if (current_state) {
				*current_state = VDK_NOT_INSTALLED;
			}
			ret = ERROR_SUCCESS;
		}
		else {
			VDKTRACE(0, (
				"VdkGetDriverState: OpenService(SERVICE_QUERY_STATUS) - %s\n",
				VdkStatusStr(ret)));
		}

		goto cleanup;
	}

	//	Get current driver status

	memset(&status, 0, sizeof(status));

	if (!QueryServiceStatus(hService, &status)) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkGetDriverState: QueryServiceStatus() - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

	if (current_state) {
		*current_state = status.dwCurrentState;
	}

cleanup:
	//	Close the service object handle

	if (hService) {
		CloseServiceHandle(hService);
	}

	//	Close handle to the service control manager.

	if (hScManager) {
		CloseServiceHandle(hScManager);
	}

	return ret;
}

//
//	Install Virtual Disk Driver dynamically
//
DWORD VdkInstall(
	LPCTSTR		driver_path,
	BOOL		auto_start)
{
	SC_HANDLE	hScManager;				// Service Control Manager
	SC_HANDLE	hService;				// Service (= Driver)
	TCHAR		full_path[MAX_PATH];
	DWORD		ret = ERROR_SUCCESS;

	//	Prepare driver binary's full path

	if (driver_path == NULL || *driver_path == '\0') {

		// default driver file is vdk.sys in the same directory as executable

		DWORD len = GetModuleFileName(
			NULL, full_path, sizeof(full_path));

		if (len == 0) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkInstall: GetModuleFileName - %s\n",
				VdkStatusStr(ret)));

			return ret;
		}

		//	search the last '\' char

		while (len > 0 && full_path[len - 1] != '\\') {
			len --;
		}

		strcpy(&full_path[len], VDK_DRIVER_FILENAME);
	}
	else {
		//	ensure that tha path is absolute full path

		LPTSTR file_part;

		if (GetFullPathName(
			driver_path,
			sizeof(full_path),
			full_path,
			&file_part) == 0)
		{
			ret = GetLastError();

			VDKTRACE(0, ("VdkInstall: GetFullPathName(%s) - %s\n",
				driver_path, VdkStatusStr(ret)));

			return ret;
		}

		//	only directory is specified?

		if (GetFileAttributes(full_path) & FILE_ATTRIBUTE_DIRECTORY) {
			strcat(full_path, "\\");
			strcat(full_path, VDK_DRIVER_FILENAME);
		}
	}

	//	Check if the file is a valid Virtual Disk driver

	ret = VdkCheckFileVersion(full_path, NULL);

	if (ret != ERROR_SUCCESS) {
		VDKTRACE(0, ("VdkInstall: VdkCheckDriverFile(%s)\n", full_path));

		return ret;
	}

	//	If the path is under %SystemRoot% make it relative to %SystemRoot%
	{
		TCHAR windir[MAX_PATH];
		int len;

		len = GetEnvironmentVariable("SystemRoot", windir, sizeof(windir));

		if (len > sizeof(windir)) {
			VDKTRACE(0, (
				"VdkInstall: %%SystemRoot%% contains too long text\n"));

			return ERROR_BAD_ENVIRONMENT;
		}
		else if (len && _strnicmp(windir, full_path, len) == 0) {
			memmove(full_path, full_path + len + 1, strlen(full_path) - len);
		}
	}

	//	Connect to the Service Control Manager

	hScManager = OpenSCManager(
		NULL,							// local machine
		NULL,							// local database
		SC_MANAGER_CREATE_SERVICE);		// access required

	if (hScManager == NULL) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkInstall: OpenSCManager() - %s\n", VdkStatusStr(ret)));

		return ret;
	}

	//	Create a new service object

	hService = CreateService(
		hScManager,						// service control manager
		VDK_DEVICE_BASENAME,			// internal service name
		VDK_DEVICE_BASENAME,			// display name
		SERVICE_ALL_ACCESS,				// access mode
		SERVICE_KERNEL_DRIVER,			// service type
		auto_start
			? SERVICE_AUTO_START
			: SERVICE_DEMAND_START,		// service start type
		SERVICE_ERROR_NORMAL,			// start error sevirity
		full_path,						// service image file path
		NULL,							// service group
		NULL,							// service tag
		NULL,							// service dependency
		NULL,							// use LocalSystem account
		NULL							// password for the account
	);

	if (!hService) {
		// Failed to create a service object
		ret = GetLastError();

		VDKTRACE(0, ("VdkInstall: CreateService() - %s\n", VdkStatusStr(ret)));

		goto cleanup;
	}

cleanup:
	//	Close the service object handle

	if (hService) {
		CloseServiceHandle(hService);
	}

	//	Close handle to the service control manager.

	if (hScManager) {
		CloseServiceHandle(hScManager);
	}

	return ret;
}

//
//	Remove Virtual Disk Driver entry from system registry
//
DWORD VdkRemove()
{
	SC_HANDLE	hScManager;				// Service Control Manager
	SC_HANDLE	hService;				// Service (= Driver)
	DWORD		ret = ERROR_SUCCESS;

	//	Connect to the Service Control Manager

	hScManager = OpenSCManager(NULL, NULL, 0);

	if (hScManager == NULL) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkRemove: OpenSCManager() - %s\n", VdkStatusStr(ret)));

		return ret;
	}

	//	Open Existing Service Object

	hService = OpenService(
		hScManager,						// Service control manager
		VDK_DEVICE_BASENAME,			// service name
		DELETE);						// service access mode

	if (hService == NULL) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkRemove: OpenService(DELETE) - %s\n", VdkStatusStr(ret)));

		goto cleanup;
	}

	//	Remove driver entry from registry

	if (!DeleteService(hService)) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkRemove: DeleteService() - %s\n", VdkStatusStr(ret)));

		goto cleanup;
	}

cleanup:
	//	Close the service object handle

	if (hService) {
		CloseServiceHandle(hService);
	}

	//	Close handle to the service control manager.

	if (hScManager) {
		CloseServiceHandle(hScManager);
	}

	return ret;
}

//
//	Start Virtual Disk Driver
//
DWORD VdkStart(DWORD *state)
{
	SC_HANDLE hScManager;				// Service Control Manager
	SC_HANDLE hService;					// Service (= Driver)
	SERVICE_STATUS	stat;
	DWORD ret = ERROR_SUCCESS;
	int i;

	//	Connect to the Service Control Manager

	hScManager = OpenSCManager(NULL, NULL, 0);

	if (hScManager == NULL) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkStart: OpenSCManager() - %s\n", VdkStatusStr(ret)));

		return ret;
	}

	//	Open Existing Service Object

	hService = OpenService(
		hScManager,						// Service control manager
		VDK_DEVICE_BASENAME,			// service name
		SERVICE_START
		| SERVICE_QUERY_STATUS);		// service access mode

	if (hService == NULL) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkStart: OpenService(SERVICE_START) - %s\n", VdkStatusStr(ret)));

		goto cleanup;
	}

	//	Start the driver

	if (!StartService(hService, 0, NULL)) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkStart: StartService() - %s\n", VdkStatusStr(ret)));

		goto cleanup;
	}

	//	Ensure the driver is started
	for (i = 0;;) {
		if (!QueryServiceStatus(hService, &stat)) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkStart: QueryServiceStatus() - %s\n",
				VdkStatusStr(ret)));

			break;
		}

		if (stat.dwCurrentState == SERVICE_RUNNING || ++i == 5) {
			break;
		}

		Sleep(1000);
	}

	if (state) {
		*state = stat.dwCurrentState;
	}

cleanup:
	//	Close the service object handle

	if (hService) {
		CloseServiceHandle(hService);
	}

	//	Close handle to the service control manager.

	if (hScManager) {
		CloseServiceHandle(hScManager);
	}

	return ret;
}

//
//	Stop Virtual Disk Driver
//
DWORD VdkStop(DWORD *state)
{
	SC_HANDLE		hScManager;			// Service Control Manager
	SC_HANDLE		hService;			// Service (= Driver)
	SERVICE_STATUS	stat;
	DWORD			ret = ERROR_SUCCESS;
	int				i = 0;

	//	Connect to the Service Control Manager

	hScManager = OpenSCManager(NULL, NULL, 0);

	if (hScManager == NULL) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkStop: OpenSCManager() - %s\n", VdkStatusStr(ret)));

		return ret;
	}

	//	Open Existing Service Object

	hService = OpenService(
		hScManager,						// Service control manager
		VDK_DEVICE_BASENAME,			// service name
		SERVICE_STOP
		| SERVICE_QUERY_STATUS);		// service access mode

	if (hService == NULL) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkStop: OpenService(SERVICE_STOP) - %s\n", VdkStatusStr(ret)));

		goto cleanup;
	}

	//	Stop the driver

	if (!ControlService(hService, SERVICE_CONTROL_STOP, &stat)) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkStop: ControlService(SERVICE_CONTROL_STOP) - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

	//	Ensure the driver is stopped
	while (stat.dwCurrentState != SERVICE_STOPPED && ++i < 5) {
		Sleep(1000);

		if (!QueryServiceStatus(hService, &stat)) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkStop: QueryServiceStatus() - %s\n",
				VdkStatusStr(ret)));

			break;
		}
	}

	if (state) {
		*state = stat.dwCurrentState;
	}

cleanup:
	//	Close the service object handle

	if (hService) {
		CloseServiceHandle(hService);
	}

	//	Close handle to the service control manager.

	if (hScManager) {
		CloseServiceHandle(hScManager);
	}

	return ret;
}

//
//	Set number of initial device
//
DWORD VdkSetDeviceNum(
	ULONG	device_num)
{
	DWORD ret;
	HKEY hKey;

	if (device_num == 0 || device_num > VDK_MAXIMUM_DISK_NUM) {
		return ERROR_INVALID_PARAMETER;
	}

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		VDK_REG_CONFIG_KEY, 0, KEY_WRITE, &hKey);

	if (ret != ERROR_SUCCESS) {
		VDKTRACE(0,
			("VdkSetDeviceNum: RegOpenKeyEx - %s\n",
			VdkStatusStr(ret)));

		return ret;
	}

	ret = RegSetValueEx(
		hKey, VDK_REG_DISKNUM_VALUE, 0, REG_DWORD,
		(PBYTE)&device_num, sizeof(device_num));

	if (ret != ERROR_SUCCESS) {
		VDKTRACE(0,
			("VdkSetDeviceNum: RegSetValueEx - %s\n",
			VdkStatusStr(ret)));
	}

	RegCloseKey(hKey);

	return ret;
}

//
//	Check VDK driver file version
//
DWORD VdkCheckFileVersion(
	LPCTSTR	driver_path,
	PULONG	version)
{
	DWORD	return_len;
	DWORD	dummy;
	LPVOID	info;
	VS_FIXEDFILEINFO	*fixedinfo;
	DWORD	ret = ERROR_SUCCESS;
	LPTSTR	str;

	//	Check parameter

	if (!driver_path || !*driver_path) {
		return ERROR_INVALID_PARAMETER;
	}

	if (version) {
		*version = 0;
	}

	//	Check if the driver file is accessible?
	{
		HANDLE hFile = CreateFile(
			driver_path,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			return ret;
		}

		CloseHandle(hFile);
	}

	//	Ensure that the driver binary is located on a local drive
	//	because device driver cannot be started on network drives.

	if (*driver_path == '\\' && *(driver_path + 1) == '\\') {
		//	full path is a UNC path -- \\server\dir\...

		VDKTRACE(0, ( "VdkCheckDriverFile: Driver is located on a network drive\n"));

		return ERROR_NETWORK_ACCESS_DENIED;
	}
	else {
		//	ensure that the drive letter is not a network drive

		char root[] = " :\\";

		root[0] = *driver_path;

		if (GetDriveType(root) == DRIVE_REMOTE) {
			// the drive is a network drive

			VDKTRACE(0, ( "VdkCheckDriverFile: Driver is located on a network drive\n"));

			return ERROR_NETWORK_ACCESS_DENIED;
		}
	}

	//	check file version

	return_len = GetFileVersionInfoSize((LPTSTR)driver_path, &dummy);

	if (return_len == 0) {
		ret = ERROR_BAD_FORMAT;

		VDKTRACE(0, (
			"VdkCheckDriverFile: GetFileVersionInfoSize == 0\n"));

		return ret;
	}

	if ((info = VdkAllocMem(return_len)) == NULL) {
		ret = GetLastError();

		VDKTRACE(0, (
			"VdkCheckDriverFile: VdkAllocMem(%lu) - %s\n", return_len, VdkStatusStr(ret)));

		return ret;
	}

	if (!GetFileVersionInfo((LPTSTR)driver_path, 0, return_len, info)) {
		ret = GetLastError();

		VDKTRACE(0, (
			"VdkCheckDriverFile: GetFileVersionInfo - %s\n", VdkStatusStr(ret)));

		goto cleanup;
	}

	return_len = sizeof(fixedinfo);

	if (!VerQueryValue(info, "\\", (PVOID *)&fixedinfo, (PUINT)&return_len)) {

		VDKTRACE(0, (
			"VdkCheckDriverFile: Failed to get fixed version info\n"));

		ret = ERROR_BAD_FORMAT;

		goto cleanup;
	}

	if (version) {
		*version = fixedinfo->dwFileVersionMS;

		if (fixedinfo->dwFileFlags & VS_FF_DEBUG) {
			*version |= 0x00008000;
		}
	}

	if (fixedinfo->dwFileOS				!= VOS_NT_WINDOWS32 ||
		fixedinfo->dwFileType			!= VFT_DRV			||
		fixedinfo->dwFileSubtype		!= VFT2_DRV_SYSTEM) {

		VDKTRACE(0, (
			"VdkCheckDriverFile: Invalid file type flags\n"));

		ret = ERROR_BAD_FORMAT;

		goto cleanup;
	}

	if (HIWORD(fixedinfo->dwFileVersionMS)	  != HIWORD(VDK_DRIVER_VERSION_VAL) ||
		HIWORD(fixedinfo->dwProductVersionMS) != HIWORD(VDK_PRODUCT_VERSION_VAL)) {

		VDKTRACE(0, (
			"VdkCheckDriverFile: Invalid version values - file:%08x, prod: %08x\n",
			fixedinfo->dwFileVersionMS, fixedinfo->dwProductVersionMS));

		ret = ERROR_REVISION_MISMATCH;

		goto cleanup;
	}

	if (!VerQueryValue(info, VERSIONINFO_PATH, (PVOID *)&str, (PUINT)&return_len)) {

		VDKTRACE(0, (
			"VdkCheckDriverFile: Failed to get OriginalFileName\n"));

		ret = ERROR_BAD_FORMAT;

		goto cleanup;
	}

	if (strcmp(str, VDK_DRIVER_FILENAME)) {
		VDKTRACE(0, (
			"VdkCheckDriverFile: Invalid original file name\n"));

		ret = ERROR_BAD_FORMAT;

		goto cleanup;
	}

cleanup:
	VdkFreeMem(info);

	return ret;
}

//
//	open the Virtual Disk device without showing the "Insert Disk"
//	dialog when the drive is empty.
//
HANDLE VdkOpenDevice(
	ULONG disk_number,
	ULONG part_number)
{
	TCHAR	device_name[sizeof(VDK_DEV_TEMPLATE) + 10];
	HANDLE	hDevice;
	UINT	err_mode;

	sprintf(device_name, VDK_DEV_TEMPLATE, disk_number, part_number);

	// change error mode in order to avoid "Insert Disk" dialog

	err_mode = SetErrorMode(SEM_FAILCRITICALERRORS);

	//	open the Virtual Disk device

	hDevice = CreateFile(
		device_name,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_NO_BUFFERING,
		NULL);

	//	revert to the previous error mode

	SetErrorMode(err_mode);

	//	return the handle to the Virtual Disk device

	return hDevice;
}

//
//	check running VDK driver version
//
DWORD VdkCheckVersion(
	HANDLE	hDevice,
	PULONG	version)
{
	ULONG	ver;
	DWORD	len;
	BOOL	close_device = FALSE;
	DWORD	ret = ERROR_SUCCESS;

	if (version) {
		*version = 0;
	}

	//
	//	Open Virtual Disk device
	//
	if (!hDevice || hDevice == INVALID_HANDLE_VALUE) {
		if ((hDevice = VdkOpenDevice(0, 0)) == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkCheckVersion: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
				0, 0, VdkStatusStr(ret)));

			goto cleanup;
		}

		close_device = TRUE;
	}

	if (!DeviceIoControl(
		hDevice,
		IOCTL_VDK_GET_VERSION,
		NULL,
		0,
		&ver,
		sizeof(ver),
		&len,
		NULL))
	{
		ret = GetLastError();

		VDKTRACE(0, ("VdkCheckVersion: DeviceIoControl(IOCTL_VDK_GET_VERSION) - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

	if (HIWORD(ver) != HIWORD(VDK_DRIVER_VERSION_VAL)) {
		ret = ERROR_REVISION_MISMATCH;
	}

	if (version) {
		*version = ver;
	}

cleanup:
	if (hDevice != INVALID_HANDLE_VALUE && close_device) {
		CloseHandle(hDevice);
	}

	return ret;
}

//
//	Get running driver information
//
DWORD VdkGetDriverInfo(
	HANDLE		hDevice,
	PULONG		disk_device,
	PULONG		attached_part,
	PULONG		orphaned_part,
	PULONG		reference_count)
{
	DWORD	tmp;
	BOOL	close_device = FALSE;
	DWORD	ret = ERROR_SUCCESS;
	VDK_DRIVER_INFO	driver_info;

	if (disk_device) {
		*disk_device = 0;
	}

	if (attached_part) {
		*attached_part = 0;
	}

	if (orphaned_part) {
		*orphaned_part = 0;
	}

	if (reference_count) {
		*reference_count = 0;
	}

	//
	//	Open Virtual Disk device
	//
	if (!hDevice || hDevice == INVALID_HANDLE_VALUE) {
		if ((hDevice = VdkOpenDevice(0, 0)) == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkGetDriverInfo: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
				0, 0, VdkStatusStr(ret)));

			goto cleanup;
		}

		close_device = TRUE;
	}

	//
	//	Query driver information
	//
	if (!DeviceIoControl(hDevice, IOCTL_VDK_DRIVER_INFO,
		NULL, 0, &driver_info, sizeof(driver_info), &tmp, NULL)) {

		ret = GetLastError();

		VDKTRACE(0, ("VdkGetDriverInfo: IOCTL_VDK_DRIVER_INFO - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

	if (disk_device) {
		*disk_device = driver_info.DiskDevices;
	}
	if (attached_part) {
		*attached_part = driver_info.AttachedParts;
	}
	if (orphaned_part) {
		*orphaned_part = driver_info.OrphanedParts;
	}
	if (reference_count) {
		*reference_count = driver_info.TotalReference;
	}

cleanup:
	if (hDevice != INVALID_HANDLE_VALUE && close_device) {
		CloseHandle(hDevice);
	}

	return ret;
}

//
//	Create a new disk device
//
DWORD VdkCreateDisk(
	HANDLE	hDevice)
{
	DWORD	tmp;
	BOOL	close_device = FALSE;
	DWORD	ret = ERROR_SUCCESS;

	//
	//	Open Virtual Disk device
	//
	if (!hDevice || hDevice == INVALID_HANDLE_VALUE) {
		if ((hDevice = VdkOpenDevice(0, 0)) == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkCreateDisk: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
				0, 0, VdkStatusStr(ret)));

			goto cleanup;
		}

		close_device = TRUE;
	}

	//
	//	Create virtual disk device
	//
	if (!DeviceIoControl(hDevice, IOCTL_VDK_CREATE_DISK,
		NULL, 0, NULL, 0, &tmp, NULL)) {

		ret = GetLastError();

		VDKTRACE(0, ("VdkCreateDisk: DeviceIoControl(IOCTL_VDK_CREATE_DISK) - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

cleanup:
	if (hDevice != INVALID_HANDLE_VALUE && close_device) {
		CloseHandle(hDevice);
	}

	return ret;
}

//
//	Delete a disk device
//
DWORD VdkDeleteDisk(
	HANDLE	hDevice)
{
	DWORD	tmp;
	BOOL	close_device = FALSE;
	DWORD	ret = ERROR_SUCCESS;

	//
	//	Open Virtual Disk device
	//
	if (!hDevice || hDevice == INVALID_HANDLE_VALUE) {
		if ((hDevice = VdkOpenDevice(0, 0)) == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkDeleteDisk: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
				0, 0, VdkStatusStr(ret)));

			goto cleanup;
		}

		close_device = TRUE;
	}

	//
	//	Delete a virtual disk device
	//
	if (!DeviceIoControl(hDevice, IOCTL_VDK_DELETE_DISK,
		NULL, 0, NULL, 0, &tmp, NULL)) {

		ret = GetLastError();

		VDKTRACE(0, ("VdkDeleteDisk: DeviceIoControl(IOCTL_VDK_DELETE_DISK) - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

cleanup:
	if (hDevice != INVALID_HANDLE_VALUE && close_device) {
		CloseHandle(hDevice);
	}

	return ret;
}

//
//	Check current device state
//
DWORD VdkCheckDeviceState(
	HANDLE		hDevice,
	ULONG		disk_number,
	ULONG		part_number)
{
	DWORD	tmp;
	BOOL	close_device = FALSE;
	DWORD	ret = ERROR_SUCCESS;

	if (!hDevice || hDevice == INVALID_HANDLE_VALUE) {
		hDevice = VdkOpenDevice(disk_number, part_number);

		if (hDevice == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkCheckDeviceState: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
				disk_number, part_number, VdkStatusStr(ret)));

			return ret;
		}

		close_device = TRUE;
	}

	if (!DeviceIoControl(hDevice, IOCTL_DISK_IS_WRITABLE,
		NULL, 0, NULL, 0, &tmp, NULL)) {

		ret = GetLastError();

		if (ret == ERROR_BUSY) {
			//	drive is a zombie -- try to dismount
			if (VdkDismount(hDevice, 0, 0, TRUE) == ERROR_SUCCESS) {
				//	succeeded to dismount
				ret = ERROR_NOT_READY;
			}
		}
		else if (ret == ERROR_WRITE_PROTECT) {
			ret = ERROR_SUCCESS;
		}

#ifdef VDK_DEBUG
		if (ret != ERROR_NOT_READY) {
			VDKTRACE(0, ("VdkCheckDeviceState: IOCTL_DISK_IS_WRITABLE - %s\n",
				VdkStatusStr(ret)));
		}
#endif	// VDK_DEBUG
	}

	if (close_device && hDevice != INVALID_HANDLE_VALUE) {
		CloseHandle(hDevice);
	}

	return ret;
}

//
//	Get device information
//
DWORD VdkGetDeviceList(
	PULONG				device_num,
	PVDK_DEVICE_INFO	*device_info)
{
	HANDLE hDevice;
	ULONG disk_num, part_num, orphan_num;
	DWORD result;
	DWORD ret = ERROR_SUCCESS;

	if (device_num) {
		*device_num = 0;
	}

	if (device_info) {
		*device_info = NULL;
	}

	//
	//	Get information about all devices
	//
	hDevice = VdkOpenDevice(0, 0);

	if (hDevice == INVALID_HANDLE_VALUE) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkGetDeviceList: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
			0, 0, VdkStatusStr(ret)));

		return ret;
	}

	ret = VdkGetDriverInfo(hDevice, &disk_num, &part_num, &orphan_num, NULL);

	if (ret != ERROR_SUCCESS) {
		goto cleanup;
	}

	*device_num = disk_num + part_num + orphan_num;

	*device_info = (PVDK_DEVICE_INFO)VdkAllocMem(*device_num * sizeof(VDK_DEVICE_INFO));

	if (*device_info == NULL) {
		ret = GetLastError();
		goto cleanup;
	}

	if (!DeviceIoControl(hDevice, IOCTL_VDK_DEVICE_INFO, device_num, sizeof(device_num),
		(LPBYTE)*device_info, *device_num * sizeof(VDK_DEVICE_INFO), &result, NULL)) {

		ret = GetLastError();

		VDKTRACE(0, ("VdkGetDeviceList: IOCTL_VDK_DEVICE_INFO - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

cleanup:
	CloseHandle(hDevice);

	return ret;
}


#ifdef VDK_DEBUG
//
//	Get and set debug trace flags
//
ULONG VdkTraceFlags(
	PULONG		flags)
{
	HANDLE	hDevice;
	ULONG	result;
	ULONG	current = 0;

	hDevice = VdkOpenDevice(0, 0);

	if (hDevice != INVALID_HANDLE_VALUE) {
		DeviceIoControl(hDevice, IOCTL_VDK_DEBUG_TRACE,
			flags, flags ? sizeof(ULONG) : 0,
			&current, sizeof(current),
			&result, NULL);

		CloseHandle(hDevice);
	}

	return current;
}
#endif

//
//	Open Virtual Disk Image File
//
DWORD VdkOpen(
	HANDLE	hDevice,
	ULONG	disk_number,
	PVOID	pDisk,
	ULONG	AccessMode)
{
	PVDK_OPEN_FILE_INFO		open_info = NULL;
	ULONG	info_len;
	BOOL	close_device = FALSE;
	DWORD	ret = ERROR_SUCCESS;

	if (!pDisk) {
		return ERROR_INVALID_PARAMETER;
	}

	ret = VDiskMapToOpenInfo(pDisk, (PVOID *)&open_info, &info_len);

	if (ret != ERROR_SUCCESS) {
		return ret;
	}

	open_info->DiskType = AccessMode;

	//
	//	Open Virtual Disk device
	//
	if (!hDevice || hDevice == INVALID_HANDLE_VALUE) {
		if ((hDevice = VdkOpenDevice(disk_number, 0)) == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkOpen: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
				disk_number, 0, VdkStatusStr(ret)));

			goto cleanup;
		}

		close_device = TRUE;
	}

	//
	//	Check driver version
	//
	ret = VdkCheckVersion(hDevice, NULL);

	if (ret != ERROR_SUCCESS) {
		goto cleanup;
	}

	//
	//	Open image file
	//
	if (!DeviceIoControl(
		hDevice,
		IOCTL_VDK_OPEN_FILE,
		open_info,
		info_len,
		NULL,
		0,
		&info_len,
		NULL))
	{
		ret = GetLastError();

		VDKTRACE(0, ("VdkOpen: DeviceIoControl(IOCTL_VDK_OPEN_FILE) - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

	//
	// update device configuration accordingly
	//
	if (!DeviceIoControl(
		hDevice,
		IOCTL_VDK_UPDATE_DEVICE,
		NULL,
		0,
		NULL,
		0,
		&info_len,
		NULL))
	{
		ULONG graceful = TRUE;

		ret = GetLastError();

		VDKTRACE(0, ("VdkOpen: DeviceIoControl(IOCTL_VDK_UPDATE_DEVICE) - %s\n",
			VdkStatusStr(ret)));

		DeviceIoControl(hDevice, IOCTL_VDK_CLOSE_FILE,
			&graceful, sizeof(graceful), NULL, 0, &info_len, NULL);

		goto cleanup;
	}

cleanup:
	if (hDevice != INVALID_HANDLE_VALUE && close_device) {
		CloseHandle(hDevice);
	}

	if (open_info) {
		VdkFreeMem(open_info);
	}

	return ret;
}

//
//	Close Image File
//
DWORD VdkClose(
	HANDLE	hDevice,
	ULONG	disk_number,
	ULONG	graceful)
{
	DWORD	return_len;
	BOOL	close_device = FALSE;
	DWORD	ret = ERROR_SUCCESS;

	if (!hDevice || hDevice == INVALID_HANDLE_VALUE) {
		if ((hDevice = VdkOpenDevice(disk_number, 0)) == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkClose: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
				disk_number, 0, VdkStatusStr(ret)));

			goto cleanup;
		}

		close_device = TRUE;
	}

	if (!DeviceIoControl(hDevice, IOCTL_VDK_CLOSE_FILE,
		&graceful, sizeof(graceful), NULL, 0, &return_len, NULL)) {

		ret = GetLastError();

		VDKTRACE(0, ("VdkClose: IOCTL_VDK_CLOSE_FILE - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

cleanup:

	if (close_device && hDevice != INVALID_HANDLE_VALUE) {
		CloseHandle(hDevice);
	}

	return ret;
}

//
//	Get Virtual Disk image file info
//
DWORD VdkGetFileInfo(
	HANDLE	hDevice,
	ULONG	disk_number,
	PVDK_OPEN_FILE_INFO	*file_info)
{
	ULONG	info_size;
	DWORD	return_len;
	BOOL	close_device = FALSE;
	DWORD	ret = ERROR_SUCCESS;

	if (!file_info) {
		return ERROR_INVALID_PARAMETER;
	}

	*file_info = NULL;

	//
	//	Open device
	//
	if (!hDevice || hDevice == INVALID_HANDLE_VALUE) {
		if ((hDevice = VdkOpenDevice(disk_number, 0)) == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			VDKTRACE(0, (
				"VdkGetFileInfo: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
				disk_number, 0, VdkStatusStr(ret)));

			goto cleanup;;
		}

		close_device = TRUE;
	}

	//
	//	Query file information size
	//
	if (!DeviceIoControl(hDevice, IOCTL_VDK_QUERY_FILE_SIZE,
		NULL, 0, &info_size, sizeof(info_size), &return_len, NULL)) {

		ret = GetLastError();

		VDKTRACE(0, ("VdkGetFileInfo: DeviceIoControl(IOCTL_VDK_QUERY_FILE_SIZE) - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

	if (info_size == 0) {
		goto cleanup;
	}

	//
	//	Allocate information area
	//
	if ((*file_info = (PVDK_OPEN_FILE_INFO)VdkAllocMem(info_size)) == NULL) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkGetFileInfo: VdkAllocMem(%lu) - %s\n",
			info_size, VdkStatusStr(ret)));

		goto cleanup;
	}

	memset(*file_info, 0, info_size);

	//
	//	Query file information
	//
	if (!DeviceIoControl(hDevice, IOCTL_VDK_QUERY_FILE,
		NULL, 0, *file_info, info_size, &return_len, NULL)) {

		ret = GetLastError();

		VDKTRACE(0, ("VdkGetFileInfo: DeviceIoControl(IOCTL_VDK_QUERY_FILE) - %s\n",
			VdkStatusStr(ret)));

		VdkFreeMem(*file_info);
		*file_info = NULL;

		goto cleanup;
	}

cleanup:
	if (close_device && hDevice != INVALID_HANDLE_VALUE) {
		CloseHandle(hDevice);
	}

	return ret;
}

//
//	Dismount a single virtual partition
//
DWORD VdkDismount(
	HANDLE	hDevice,
	ULONG	disk_number,
	ULONG	part_number,
	ULONG	unlock)
{
	DWORD	return_len;
	BOOL	close_device = FALSE;
	DWORD	ret = ERROR_SUCCESS;

	if (!hDevice || hDevice == INVALID_HANDLE_VALUE) {
		hDevice = VdkOpenDevice(disk_number, part_number);

		if (hDevice == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkDismount: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
				disk_number, part_number, VdkStatusStr(ret)));

			return ret;
		}

		close_device = TRUE;
	}

	if (!DeviceIoControl(hDevice, FSCTL_LOCK_VOLUME,
		NULL, 0, NULL, 0, &return_len, NULL)) {

		ret = GetLastError();

		VDKTRACE(0, (
			"VdkDismount: DeviceIoControl(FSCTL_LOCK_VOLUME) - %s\n",
			VdkStatusStr(ret)));

		goto cleanup;
	}

	if (!DeviceIoControl(hDevice, FSCTL_DISMOUNT_VOLUME,
		NULL, 0, NULL, 0, &return_len, NULL)) {

		ret = GetLastError();

		VDKTRACE(0, ("VdkDismount: DeviceIoControl(FSCTL_DISMOUNT_VOLUME) - %s\n",
			VdkStatusStr(ret)));
	}

	if (ret == ERROR_SUCCESS) {
		if (!DeviceIoControl(hDevice, IOCTL_VDK_NOTIFY_DISMOUNT,
			NULL, 0, NULL, 0, &return_len, NULL)) {

			VDKTRACE(0, ("VdkDismount: DeviceIoControl(IOCTL_VDK_NOTIFY_DISMOUNT) - %s\n",
				VdkStatusStr(GetLastError())));
		}
	}

	if (unlock && !DeviceIoControl(hDevice, FSCTL_UNLOCK_VOLUME,
		NULL, 0, NULL, 0, &return_len, NULL)) {

		VDKTRACE(0, ("VdkDismount: DeviceIoControl(FSCTL_UNLOCK_VOLUME) - %s\n",
			VdkStatusStr(GetLastError())));
	}

cleanup:
	if (hDevice != INVALID_HANDLE_VALUE && close_device) {
		CloseHandle(hDevice);
	}

	return ret;
}

//
//	Dismount all virtual partition devices
//
DWORD VdkDismountAll(BOOL zombie_only)
{
	HANDLE hDevice;
	ULONG device_num = 0;
	PVDK_DEVICE_INFO device_info = NULL;
	DWORD tmp;
	DWORD ret;

	//
	//	Get information about all devices
	//
	ret = VdkGetDeviceList(&device_num, &device_info);

	if (ret != VDK_OK) {
		return ret;
	}

	//
	//	dismount existing devices
	//
	for (tmp = 0; tmp < device_num; tmp++) {
		if (!device_info[tmp].ReferenceCount ||
			(zombie_only && !device_info[tmp].Zombie)) {
			continue;
		}

		DefineDosDevice(DDD_RAW_TARGET_PATH, "VDKTMP", device_info[tmp].DeviceName);

		hDevice = CreateFile(
			"\\\\.\\VDKTMP",
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_NO_BUFFERING,
			NULL);

		if (hDevice == INVALID_HANDLE_VALUE) {
			ret = GetLastError();

			VDKTRACE(0, ("VdkDismountAll: CreateFile(\\\\.\\VDKTMP) - %s\n",
				VdkStatusStr(ret)));
		}

		DefineDosDevice(DDD_REMOVE_DEFINITION, "VDKTMP", NULL);

		if (ret == ERROR_SUCCESS) {
			VdkDismount(hDevice, 0, 0, TRUE);
			CloseHandle(hDevice);
		}
	}

	//
	//	check if all devices are released
	//
	ret = VdkGetDriverInfo(NULL, NULL, NULL, NULL, &tmp);

	if (ret == ERROR_SUCCESS && tmp > 1) {
		ret = ERROR_BUSY;
	}

	if (device_info) {
		VdkFreeMem(device_info);
	}

	return ret;
}

//
//	Get VDK device name in the kernel namespace
//
DWORD VdkGetDeviceName(
	ULONG disk_number,
	ULONG part_number,
	LPTSTR buf)
{
	TCHAR dos_name[sizeof(VDK_DOS_TEMPLATE) + 10];
	TCHAR dir_name[MAX_DEVNAME_LEN];

	*buf = '\0';

	sprintf(dos_name, VDK_DEVICE_BASENAME "%u", disk_number);

	if (!QueryDosDevice(dos_name, dir_name, sizeof(dir_name))) {
		return GetLastError();
	}

	if (part_number != (ULONG)-1) {
		sprintf(buf, "%s\\Partition%lu", dir_name, part_number);
	}
	else {
		strcpy(buf, dir_name);
	}

	return ERROR_SUCCESS;
}

//
//	Get Virtual Disk drive letter
//
DWORD VdkGetDriveLetter(
	ULONG	disk_number,
	ULONG	part_number,
	TCHAR	*drive_letter)
{
	DWORD	logical_drives;
	TCHAR	dos_device[] = " :";
	TCHAR	device_name[MAX_PATH], dos_name[MAX_PATH];
	DWORD	ret = ERROR_SUCCESS;

	if (!drive_letter) {
		return ERROR_INVALID_PARAMETER;
	}

	*drive_letter = '\0';

	ret = VdkGetDeviceName(disk_number, part_number, device_name);

	if (ret != ERROR_SUCCESS) {
		return ret;
	}

	logical_drives = GetLogicalDrives();

	if (logical_drives == 0) {
		ret = GetLastError();

		VDKTRACE(0, (
			"VdkGetDriveLetter: GetLogicalDrives - %s\n", VdkStatusStr(ret)));

		return ret;
	}

	dos_device[0] = 'A';

	while (logical_drives) {
		if (logical_drives & 0x01) {
			if (QueryDosDevice(dos_device, dos_name, sizeof(dos_name))) {
				if (_stricmp(device_name, dos_name) == 0) {
					*drive_letter = dos_device[0];
					return ERROR_SUCCESS;
				}
			}
			else {
				ret = GetLastError();

				VDKTRACE(0, (
					"VdkGetDriveLetter: QueryDosDevice(%s) - %s\n",
					dos_device, VdkStatusStr(ret)));

				return ret;
			}
		}
		logical_drives >>= 1;
		dos_device[0]++;
	}

	return ret;
}

//
//	Assign a DOS drive letter to Virtual Disk Drive
//
DWORD VdkSetDriveLetter(
	ULONG	disk_number,
	ULONG	part_number,
	TCHAR	drive_letter)
{
	DWORD ret = ERROR_SUCCESS;
	TCHAR dos_device[] = " :";
	TCHAR device_name[MAX_PATH], dos_name[MAX_PATH];

	if (!isalpha(drive_letter)) {
		return ERROR_INVALID_PARAMETER;
	}

	//	check if device is active

	ret = VdkCheckDeviceState(NULL, disk_number, part_number);

	if (ret != ERROR_SUCCESS) {
		return ret;
	}

	//	Check if the drive letter is already in use

	dos_device[0] = (TCHAR)toupper(drive_letter);

	ret = VdkGetDeviceName(disk_number, part_number, device_name);

	if (ret != ERROR_SUCCESS) {
		return ret;
	}

	if (QueryDosDevice(dos_device, dos_name, sizeof(dos_name))) {
		if (strcmp(dos_name, device_name) == 0) {
			return ERROR_SUCCESS;
		}
		else {
			VDKTRACE(0, (
				"VdkSetDriveLetter: Drive letter '%c' is linked to '%s'\n",
				dos_device[0], dos_name));

			return ERROR_ALREADY_ASSIGNED;
		}
	}
	else {
		if ((ret = GetLastError()) != ERROR_FILE_NOT_FOUND) {
			VDKTRACE(0, (
				"VdkSetDriveLetter: QueryDosDevice(%s) - %lu\n",
				dos_device, VdkStatusStr(ret)));

			return ret;
		}
	}

	//	Check if the Virtual Disk drive has a DriveLetter already assigned

	ret = VdkGetDriveLetter(disk_number, part_number, &drive_letter);

	if (ret != ERROR_SUCCESS) {
		return ret;
	}

	if (isalpha(drive_letter)) {
		if (drive_letter == dos_device[0]) {
			return ERROR_SUCCESS;
		}
		else {
			VDKTRACE(0, ("VdkSetDriveLetter: Drive Letter '%c' is already assigned to %s\n",
				drive_letter, device_name));

			return ERROR_ALREADY_ASSIGNED;
		}
	}

	//	Assign the new drive letter

	if (!DefineDosDevice(DDD_RAW_TARGET_PATH, dos_device, device_name)) {
		ret = GetLastError();

		VDKTRACE(0, ("VdkSetDriveLetter: DefineDosDevice(%s) - %s\n",
			dos_device, VdkStatusStr(ret)));
	}

	return ret;
}

//
//	Remove Dos Drive Letter
//
DWORD VdkDelDriveLetter(
	TCHAR	drive_letter)
{
	HANDLE	hDevice;
	ULONG	tmp;
	TCHAR	dos_device[] = "\\\\.\\ :";
	DWORD	ret = ERROR_SUCCESS;

	if (!isalpha(drive_letter)) {
		return ERROR_INVALID_DRIVE;
	}

	dos_device[4] = drive_letter;

	//
	//	Check if the target device is a VDK device
	//
	hDevice = CreateFile(
		dos_device,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		ret = GetLastError();

		if (ret != ERROR_FILE_NOT_FOUND &&
			ret != ERROR_PATH_NOT_FOUND) {

			return ret;
		}

		goto execute;
	}

	if (!DeviceIoControl(hDevice, IOCTL_VDK_GET_VERSION,
		NULL, 0, NULL, 0, &tmp, NULL)) {

		ret = ERROR_INVALID_PARAMETER;
	}

	CloseHandle(hDevice);

	if (ret != ERROR_SUCCESS) {
		return ret;
	}

execute:
	if (!DefineDosDevice(DDD_REMOVE_DEFINITION, &dos_device[4], NULL)) {
		ret = GetLastError();

		VDKTRACE(0, (
			"VdkUnlink: DefineDosDevice(%s) - %s\n",
			&dos_device[4], VdkStatusStr(ret)));
	}

	return ret;
}

//
// choose first available drive letter
//
char ChooseDriveLetter()
{
	DWORD	logical_drives = GetLogicalDrives();
	char	drive_letter = 'C';

	if (logical_drives == 0) {
		return '\0';
	}

	//
	//	Do not assign A and B to Virtual Disk even if they are not used
	//
	logical_drives >>= 2;

	while (logical_drives & 0x1) {
		logical_drives >>= 1;
		drive_letter++;
	}

	if (drive_letter > 'Z') {
		return '\0';
	}

	return drive_letter;
}

//
//	Ensure Virtual Disk image is closed
//
DWORD VdkCloseDrive(
	ULONG			disk_number,
	VDK_CALLBACK	retrycb,
	VDK_CALLBACK	contcb,
	PVOID			param)
{
	HANDLE	hDisk;
	TCHAR	vdk_dev[MAX_PATH];
	ULONG	dos_dev[26];
	ULONG	part_num;
	ULONG	max_parts;
	ULONG	graceful;
	ULONG	i;
	DWORD	ret = ERROR_SUCCESS;

	VdkGetDeviceName(disk_number, 0, vdk_dev);

	//
	//	Delete all drive letters
	//
	for (i = 0; i < 26; i++) {
		TCHAR	dos_link[] = " :";
		TCHAR	dos_device[MAX_PATH];

		dos_link[0] = (TCHAR)(i + 'a');
		dos_device[0] = '\0';

		if (QueryDosDevice(dos_link, dos_device, sizeof(dos_device)) &&
			!_strnicmp(vdk_dev, dos_device, strlen(vdk_dev) - 1)) {

			dos_dev[i] = atol(dos_device + strlen(vdk_dev) - 1);

			DefineDosDevice(
				(DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE),
				dos_link,
				dos_device);
		}
		else {
			dos_dev[i] = (ULONG)-1;
		}
	}

	hDisk = VdkOpenDevice(disk_number, 0);

	if (hDisk == INVALID_HANDLE_VALUE) {
		ret = VdkLastError();

		VDKTRACE(0, ("VdkCloseDrive: CreateFile(" VDK_DEV_TEMPLATE ") - %s\n",
			disk_number, 0, VdkStatusStr(ret)));

		return ret;
	}

	if (!DeviceIoControl(hDisk, IOCTL_VDK_NUMBER_OF_PARTS,
		NULL, 0, &max_parts, sizeof(max_parts), &part_num, NULL)) {

		ret = VdkLastError();

		VDKTRACE(0, ("VdkCloseDrive: IOCTL_VDK_NUMBER_OF_PARTS - %s\n",
			VdkStatusStr(ret)));

		CloseHandle(hDisk);
		return ret;
	}

	//
	//	Dismount all partitions on this disk
	//
	graceful = TRUE;

	for (part_num = 0; part_num <= max_parts; part_num++) {

		ret = VdkCheckDeviceState(
			part_num ? NULL : hDisk, disk_number, part_num);

		if (ret != ERROR_SUCCESS) {
			break;
		}

dismount_retry:

		SetCursor(LoadCursor(NULL, IDC_WAIT));

		for(i = 0;;) {
			ret = VdkDismount(
				part_num ? NULL : hDisk, disk_number, part_num, FALSE);

			if (ret != ERROR_ACCESS_DENIED || ++i > 10) {
				break;
			}

			Sleep(500);			//	retry after 0.5 sec.
		}

		SetCursor(LoadCursor(NULL, IDC_ARROW));

		if (ret == ERROR_ACCESS_DENIED &&
			retrycb && (*retrycb)(param, ret)) {

			goto dismount_retry;
		}

		if (ret != ERROR_SUCCESS) {

			// Failed to dismount
			if (contcb && (*contcb)(param, ret)) {

				//	proceed anyway
				if (!part_num) {
					graceful = FALSE;
				}

				ret = ERROR_SUCCESS;
			}
			else {

				//	abort operation
				break;
			}
		}
	}	// next partition

	//
	//	couldn't dismount partition 0 -> file is not opened
	//
	if (!part_num) {
		CloseHandle(hDisk);
		return ret;
	}

	//
	//	Close virtual disk file
	//
	if (ret == ERROR_SUCCESS) {
		ret = VdkClose(hDisk, disk_number, graceful);
	}

	if (ret != ERROR_SUCCESS) {
		//
		//	Close failed -- restore drive letters
		//
		for (i = 0; i < 26; i++) {

			if (dos_dev[i] != -1) {
				if (VdkSetDriveLetter(disk_number, dos_dev[i], (char)(i + 'a')) != ERROR_SUCCESS) {
				}
			}
		}
	}

	CloseHandle(hDisk);

	return ret;
}

// End Of File
