/*
	vdkcmd.c

	Virtual Disk drive control program (console version)
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "vdkfile.h"
#include "vdkioctl.h"
#include "vdkaccess.h"
#include "vdkpart.h"
#include "vdkmsg.h"
#include "vdkver.h"
#include "vdkctl.h"

#include "cowdisk.h"
#include "vmdisk.h"

#include "VDiskUtil.h"

//
//	current driver state
//
static DWORD driver_state = VDK_NOT_INSTALLED;

//
//	command processing functions
//
typedef int (*cmdfnc)(char **args);

static int	Install(char **args);
static int	Remove(char **args);
static int	Start(char **args);
static int	Stop(char **args);
static int	Driver(char **args);
static int	DiskNum(char **args);
static int	Create(char **args);
static int	Delete(char **args);
static int	Device(char **args);
static int	Open(char **args);
static int	Close(char **args);
static int	Link(char **args);
static int	Unlink(char **args);
static int	Image(char **args);
static int	View(char **args);
static int	Help(char **args);
#ifdef _DEBUG
static int	Trace(char **args);
#endif	// _DEBUG

static const struct {
	char	*cmd;
	int		min_argc;
	int		max_argc;
	cmdfnc	func;
	DWORD	usage;
	DWORD	helpmsg;
}
Commands[] = {
	{ "INSTALL",	2, 4, Install,	MSG_USAGE_INSTALL	, MSG_HELP_INSTALL	},
	{ "REMOVE",		2, 2, Remove,	MSG_USAGE_REMOVE	, MSG_HELP_REMOVE	},
	{ "START",		2, 2, Start,	MSG_USAGE_START		, MSG_HELP_START	},
	{ "STOP",		2, 2, Stop,		MSG_USAGE_STOP		, MSG_HELP_STOP		},
	{ "DRIVER",		2, 3, Driver,	MSG_USAGE_DRIVER	, MSG_HELP_DRIVER	},
	{ "DISK",		3, 3, DiskNum,	MSG_USAGE_DISK		, MSG_HELP_DISK		},
	{ "CREATE",		2, 2, Create,	MSG_USAGE_CREATE	, MSG_HELP_CREATE	},
	{ "DELETE",		2, 2, Delete,	MSG_USAGE_DELETE	, MSG_HELP_DELETE	},
	{ "VIEW",		3, 4, View,		MSG_USAGE_VIEW		, MSG_HELP_VIEW		},
	{ "OPEN",		4, 8, Open,		MSG_USAGE_OPEN		, MSG_HELP_OPEN		},
	{ "CLOSE",		3, 4, Close,	MSG_USAGE_CLOSE		, MSG_HELP_CLOSE	},
	{ "LINK",		4, 5, Link,		MSG_USAGE_LINK		, MSG_HELP_LINK		},
	{ "ULINK",		3, 4, Unlink,	MSG_USAGE_ULINK		, MSG_HELP_ULINK	},
	{ "IMAGE",		2, 3, Image,	MSG_USAGE_IMAGE		, MSG_HELP_IMAGE	},
	{ "HELP",		2, 3, Help,		MSG_USAGE_HELP		, MSG_HELP_HELP		},
#ifdef _DEBUG
	{ "DEVICE",		2, 2, Device,	0, 0 },
	{ "TRACE",		2, 20, Trace,	0, 0 },
#endif	// _DEBUG
	{ NULL,			0, 0, NULL,		0, 0 }
};

//
//	local functions
//
static BOOL IsUserAdmin();
static DWORD GetDiskFromDrive(CHAR drive, PULONG disk);
static void PrintFileInfo(PVDK_OPEN_FILE_INFO file_info);
static DWORD Retry_Callback(PVOID param, DWORD err);
static DWORD Continue_Callback(PVOID param, DWORD err);
static void PartList_Callback(PPARTITION_ITEM pitem, PVOID param);
static void Assign_Callback(PPARTITION_ITEM pitem, PVOID param);

//
//	struct used for Assign_Callback parameter
//
typedef struct _ASSIGN_PARAM {
	ULONG	disk_number;
	ULONG	part_number;
	BOOL	read_only;
	PCHAR	drive_letters;
}
ASSIGN_PARAM, *PASSIGN_PARAM;

#ifdef VDK_DEBUG
#define VDK_DEBUG_TAG	" (debug)"
#else
#define VDK_DEBUG_TAG
#endif

//
//	main
//
int main(int argc, char **argv)
{
	char	*cmd;
	DWORD	ret;
	int		idx;

	//
	//	Reports memory leaks at process termination
	//
#ifdef VDK_DEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

	printf(
		VDK_PRODUCT_NAME " version "
		VDK_PRODUCT_VERSION_STR VDK_DEBUG_TAG "\n"
		"http://chitchat.at.infoseek.co.jp/vmware/\n\n"
	);

	if (!IsUserAdmin()) {
		PrintMessage(MSG_MUST_BE_ADMIN);
		return -1;
	}

	//
	//	At least one parameter (command) is required
	//
	if (argc < 2) {
		PrintMessage(MSG_HELP_USAGE);
		return -1;
	}

	//
	//	Get Current Driver state
	//
	if ((ret = VdkGetDriverState(&driver_state)) != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_STAT_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	//
	//	Try to dismount all zombie devices
	//
	VdkDismountAll(TRUE);

	//
	//	Process vdk command parameter
	//
	cmd = *(argv + 1);

	idx = 0;

	while (Commands[idx].cmd) {
		if (_stricmp(cmd, Commands[idx].cmd) == 0) {

			if (argc < Commands[idx].min_argc ||
				argc > Commands[idx].max_argc) {
				PrintMessage(Commands[idx].usage);
				return -1;
			}

			ret = (*Commands[idx].func)(argv + 2);

			return ret;
		}

		idx++;
	}

	//
	//	unknown command
	//
	PrintMessage(MSG_UNKNOWN_COMMAND, cmd);

	return -1;
}

//
//	Installs the Virtual Disk Driver Dynamically
//	Command Line Parameters:
//	vdk install [path] [/auto]
//	(optional) driver file path	- default to executive's dir
//	(optional) auto start switch - default to demand start
//
int	Install(char **args)
{
	char *install_path = NULL;
	BOOL auto_start = FALSE;

	DWORD ret;

	//	process parameters

	while (*args) {
		if (_stricmp(*args, "/auto") == 0) {
			if (auto_start) {
				PrintMessage(MSG_DUPLICATE_ARGS, *args);
				return -1;
			}
			else {
				auto_start = TRUE;
			}
		}
		else if (**args == '/') {
			PrintMessage(MSG_UNKNOWN_OPTION, *args, "INSTALL");
			return -1;
		}
		else {
			if (install_path) {
				PrintMessage(MSG_DUPLICATE_ARGS, "path");
				return -1;
			}
			else {
				install_path = *args;
			}
		}
		args++;
	}

	//	already installed?

	if (driver_state != VDK_NOT_INSTALLED) {
		PrintMessage(MSG_ALREADY_INSTALLED);
		return -1;
	}

	//	install the driver

	if ((ret = VdkInstall(install_path, auto_start)) != ERROR_SUCCESS) {
		PrintMessage(MSG_INSTALL_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	//	operation successfull

	PrintMessage(MSG_INSTALL_OK);

	return 0;
}

//
//	Remove Virtual Disk Driver from system
//	Command Line Parameters: None
//
int	Remove(char **args)
{
	DWORD	ret;
	int		i;

	UNREFERENCED_PARAMETER(args);

	//	ensure the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_NOT_INSTALLED);
		return -1;
	}

	//	ensure the driver is stopped

	if (driver_state != SERVICE_STOPPED) {
		if (Stop(NULL) == -1) {
			return -1;
		}
	}

	//	remove the driver

	if ((ret = VdkRemove()) != ERROR_SUCCESS) {
		PrintMessage(MSG_REMOVE_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	// Wait for the driver to be actually removed for 3 secs Max.

	for (i = 0; i < 10; i++) {
		ret = VdkGetDriverState(&driver_state);

		if (ret != ERROR_SUCCESS) {
			PrintMessage(MSG_GET_STAT_NG);
			printf("%s\n", VdkStatusStr(ret));
			return -1;
		}

		if (driver_state == VDK_NOT_INSTALLED) {
			break;
		}

		Sleep(300);
	}

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_REMOVE_OK);
	}
	else {
		PrintMessage(MSG_REMOVE_PENDING);
	}

	return 0;
}

//
//	Start the Virtual Disk Driver
//	Command Line Parameters: None
//
int	Start(char **args)
{
	DWORD	ret;

	UNREFERENCED_PARAMETER(args);

	//	ensure that the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		if ((ret = VdkInstall(NULL, FALSE)) == ERROR_SUCCESS) {
			PrintMessage(MSG_INSTALL_OK);
		}
		else {
			PrintMessage(MSG_INSTALL_NG);
			printf("%s\n", VdkStatusStr(ret));
			return -1;
		}
	}

	//	ensure that the driver is not started yet

	if (driver_state == SERVICE_RUNNING) {
		PrintMessage(MSG_ALREADY_RUNNING);
		return -1;
	}

	//	start the driver

	if ((ret = VdkStart(&driver_state)) != ERROR_SUCCESS) {
		PrintMessage(MSG_START_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	//	operation successfull

	PrintMessage(MSG_START_OK);

	return 0;
}

//
//	Stop the Virtual Disk Driver
//	Command Line Parameters: None
//
int	Stop(char **args)
{
	DWORD ret;

	UNREFERENCED_PARAMETER(args);

	//	ensure that the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_NOT_INSTALLED);
		return -1;
	}

	//	ensure that the driver is running

	if (driver_state == SERVICE_STOPPED) {
		PrintMessage(MSG_NOT_RUNNING);
		return -1;
	}

	//	ensure that all image are unmounted

	if (driver_state == SERVICE_RUNNING) {
		char *param[] = { "*", NULL };

		if (Close(param) == -1) {
			return -1;
		}
	}

	ret = VdkDismountAll(FALSE);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_STOP_NG);
		PrintMessage(MSG_DRIVE_IN_USE);
		return -1;
	}

	//	stop the driver

	if ((ret = VdkStop(&driver_state)) != ERROR_SUCCESS) {
		PrintMessage(MSG_STOP_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	if (driver_state != SERVICE_STOPPED) {
		PrintMessage(MSG_STOP_PENDING);
		PrintMessage(MSG_RESTART_WARN);
		return -1;
	}

	PrintMessage(MSG_STOP_OK);

	return 0;
}

//
//	Show current driver status
//	Command Line Parameters: None
//
int	Driver(char **args)
{
	TCHAR	path[MAX_PATH];
	DWORD	start_type;
	ULONG	initial_dev;
	ULONG	version;
	ULONG	disk_device;
	ULONG	attached_part;
	ULONG	orphaned_part;
	ULONG	reference_count;
	DWORD	ret;

	// if the driver is not installed, no more info to show

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_CURRENT_STATUS);
		PrintMessage(MSG_STATUS_NOT_INST);
		return 0;
	}

	//	get current driver config

	if ((ret = VdkGetDriverConfig(path, &start_type, &initial_dev)) != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_CONFIG_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	//	print driver file path

	PrintMessage(MSG_DRIVER_LOCATION, path);

	//	print running driver version
	version = 0;

	if (driver_state == SERVICE_RUNNING) {
		ret = VdkCheckVersion(NULL, &version);
	}
	else {
		ret = VdkCheckFileVersion(path, &version);
	}

	if (ret != ERROR_SUCCESS &&
		ret != ERROR_REVISION_MISMATCH) {
		PrintMessage(MSG_GET_CONFIG_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	if (version) {
		PrintMessage(MSG_DRIVER_VERSION,
			HIWORD(version),
			(version & 0x00007fff),
			(version & 0x00008000) ? " (debug)" : "");
	}

	//	print driver start type

	PrintMessage(MSG_START_TYPE);

	switch (start_type) {
	case SERVICE_AUTO_START:
		PrintMessage(MSG_START_AUTO);
		break;

	case SERVICE_BOOT_START:
		PrintMessage(MSG_START_BOOT);
		break;

	case SERVICE_DEMAND_START:
		PrintMessage(MSG_START_DEMAND);
		break;

	case SERVICE_DISABLED:
		PrintMessage(MSG_START_DISABLED);
		break;

	case SERVICE_SYSTEM_START :
		PrintMessage(MSG_START_SYSTEM);
		break;

	default:
		PrintMessage(MSG_UNKNOWN_ULONG, start_type);
		break;
	}

	//	print current status

	PrintMessage(MSG_CURRENT_STATUS);

	switch (driver_state) {
	case SERVICE_STOPPED:
		PrintMessage(MSG_STATUS_STOPPED);
		break;

	case SERVICE_START_PENDING:
		PrintMessage(MSG_STATUS_START_P);
		break;

	case SERVICE_STOP_PENDING:
		PrintMessage(MSG_STATUS_STOP_P);
		break;

	case SERVICE_RUNNING:
		PrintMessage(MSG_STATUS_RUNNING);
		break;

	case SERVICE_CONTINUE_PENDING:
		PrintMessage(MSG_STATUS_CONT_P);
		break;

	case SERVICE_PAUSE_PENDING:
		PrintMessage(MSG_STATUS_PAUSE_P);
		break;

	case SERVICE_PAUSED:
		PrintMessage(MSG_STATUS_PAUSED);
		break;

	default:
		PrintMessage(MSG_UNKNOWN_ULONG, driver_state);
		break;
	}

	if (version < VDK_DRIVER_VERSION_VAL) {
		return 0;
	}

	//	if driver is not running, no more info

	if (driver_state != SERVICE_RUNNING) {
		PrintMessage(MSG_DISK_DEVICE, initial_dev);
		return 0;
	}

	ret = VdkGetDriverInfo(NULL,
		&disk_device,
		&attached_part,
		&orphaned_part,
		&reference_count);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_CONFIG_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	PrintMessage(MSG_DISK_DEVICE, disk_device);

	//
	//	hidden option
	//
	if (*args && !_stricmp(*args, "full")) {
		PrintMessage(MSG_ATTACHED_PART, attached_part);
		PrintMessage(MSG_ORPHANED_PART, orphaned_part);
		PrintMessage(MSG_REFERENCE_COUNT, reference_count);
	}

	return 0;
}

//
//	Set initial number of disk devices
//	Command Line Parameters:
//		device number
//
int DiskNum(char **args)
{
	ULONG	dev_num;
	DWORD	ret;

	//	ensure that the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_NOT_INSTALLED);
		return -1;
	}

	if (!isdigit(**args)) {
		PrintMessage(MSG_INVALID_DISKNUM, *args);
		return -1;
	}

	dev_num = atol(*args);

	if (dev_num == 0 || dev_num > VDK_MAXIMUM_DISK_NUM) {
		PrintMessage(MSG_INVALID_DISKNUM, *args);
		return -1;
	}

	ret = VdkSetDeviceNum(dev_num);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_DISKNUM_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	PrintMessage(MSG_DISKNUM_OK, dev_num);

	return 0;
}

//
//	Create a new disk device
//	Command Line Parameters: none
//
int Create(char **args)
{
	ULONG disk_device;
	DWORD ret;

	UNREFERENCED_PARAMETER(args);

	//	ensure that the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_NOT_INSTALLED);
		return -1;
	}

	//	ensure that the driver is running

	if (driver_state != SERVICE_RUNNING) {
		PrintMessage(MSG_NOT_RUNNING);
		return -1;
	}

	//	get current number of disk devices

	ret = VdkGetDriverInfo(NULL, &disk_device, NULL, NULL, NULL);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_CONFIG_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	if (disk_device >= VDK_MAXIMUM_DISK_NUM) {
		PrintMessage(MSG_NO_MORE_DISK);
		return -1;
	}

	//	create a disk

	ret = VdkCreateDisk(NULL);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_CREATEDISK_NG, disk_device);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	PrintMessage(MSG_CREATEDISK_OK, disk_device);

	//	print new number of disk devices

	ret = VdkGetDriverInfo(NULL, &disk_device, NULL, NULL, NULL);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_CONFIG_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	PrintMessage(MSG_DISK_DEVICE, disk_device);

	return 0;
}

//
//	Delete a virtual disk device
//	Command Line Parameters: none
//
int Delete(char **args)
{
	ULONG disk_device;
	DWORD ret;

	UNREFERENCED_PARAMETER(args);

	//	ensure that the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_NOT_INSTALLED);
		return -1;
	}

	//	ensure that the driver is running

	if (driver_state != SERVICE_RUNNING) {
		PrintMessage(MSG_NOT_RUNNING);
		return -1;
	}

	//	get current number of disk devices

	ret = VdkGetDriverInfo(NULL, &disk_device, NULL, NULL, NULL);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_CONFIG_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	if (disk_device == 1) {
		PrintMessage(MSG_NO_LESS_DISK);
		return -1;
	}

	//	delete disk

	ret = VdkDeleteDisk(NULL);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_DELETEDISK_NG, disk_device - 1);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	PrintMessage(MSG_DELETEDISK_OK, disk_device - 1);

	//	print new number of disk devices

	ret = VdkGetDriverInfo(NULL, &disk_device, NULL, NULL, NULL);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_CONFIG_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	PrintMessage(MSG_DISK_DEVICE, disk_device);

	return 0;
}

//
//	Print disk image files information
//	Command Line Parameters:
//		path
//
int	View(char **args)
{
	PVOID				disk		= NULL;
	PVDK_OPEN_FILE_INFO	open_info	= NULL;
	VDK_DISK_INFO		disk_info	= { 0 };
	ULONG				info_size;
	PSTR				file_name	= NULL;
	PSTR				search_path	= NULL;
	DWORD				ret;

	while (*args) {
		if (_strnicmp(*args, "/search:", 8) == 0) {
			search_path = (*args + 8);
		}
		else {
			file_name = *args;
		}
		args++;
	}
	
	if (!file_name) {
		PrintMessage(MSG_USAGE_VIEW);
		return -1;
	}

	//	Create File structure

	VDiskSetCallBack(VDiskCallback);

	VDiskSetSearchPath(search_path);

	ret = VDiskLoadFile(&disk, file_name, NULL);

	if (ret != ERROR_SUCCESS || disk == NULL) {
		PrintMessage(MSG_OPENIMAGE_NG);
		printf("%s\n", VdkStatusStr(ret == ERROR_SUCCESS ? ERROR_INVALID_FUNCTION : ret));
		goto cleanup;
	}

	ret = VDiskCreateTree(disk);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_OPENIMAGE_NG);
		printf("%s\n", VdkStatusStr(ret));
		goto cleanup;
	}

	ret = VDiskMapToOpenInfo(disk, (PVOID *)&open_info, &info_size);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_OPENIMAGE_NG);
		printf("%s\n", VdkStatusStr(ret));
		goto cleanup;
	}

	//	print image file information

	PrintMessage(MSG_DISKIMAGE_NAME, VDiskGetDiskName(disk));
	PrintFileInfo(open_info);

	//	open virtual disk

	open_info->DiskType = VDK_DISKTYPE_READONLY;

	ret = VdkOpenDisk(open_info, &disk_info);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_OPENIMAGE_NG);
		printf("%s\n", VdkStatusStr(ret));
		goto cleanup;
	}

	//	print partition info

	PrintMessage(MSG_PARTITION_HEADER);

	ret = VdkListPartitions(&disk_info, NULL, 0, PartList_Callback, (PVOID)-1);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_PART_NG);
		printf("%s\n", VdkStatusStr(ret));
	}

cleanup:
	if (disk) {
		VDiskDelete(disk);
	}

	if (open_info) {
		VdkFreeMem(open_info);
	}

	if (disk_info.DiskType) {
		VdkCloseDisk(&disk_info);
	}

	return (ret == ERROR_SUCCESS ? 0 : -1);
}

//
//	Opens virtual disk image
//	Command Line Parameters:
//		disk number
//		image file path
//		(optional) access mode - default to read-only
//		(optional) partition to assign drive letter - default to all mountable
//		(optional) drive letters - default to the first available letter
//
int	Open(char **args)
{
	// for command line parameters
	ULONG	disk_number;
	LPTSTR	file_path;
	LPTSTR	search_path		= NULL;
	LPTSTR	undo_path		= NULL;
	BOOL	writable		= FALSE;
	BOOL	undoable		= FALSE;
	BOOL	writeblock		= FALSE;
	LPTSTR	drive_letters	= NULL;
	ULONG	part_number		= (ULONG)-1;

	//	for virtual disk operation
	PVOID	disk			= NULL;

	// for vdk driver operation
	ULONG	access;
	HANDLE	hDevice			= NULL;
	PVDK_OPEN_FILE_INFO		file_info = NULL;
	ASSIGN_PARAM			assign;

	DWORD	ret;

	//	process command line parameters
	if (**args == '*') {
		disk_number = (ULONG)-1;
	}
	else if (isdigit(**args)) {
		disk_number = atol(*args);

		if (disk_number >= VDK_MAXIMUM_DISK_NUM) {
			PrintMessage(MSG_INVALID_DISK, *args);
			return -1;
		}
	}
	else {
		PrintMessage(MSG_INVALID_DISK, *args);
		return -1;
	}

	args++;

	if (!*args || !**args) {
		PrintMessage(MSG_USAGE_OPEN);
		return -1;
	}

	file_path = *args;

	while (*(++args)) {
		if (_stricmp(*args, "/rw") == 0) {
			if (writable) {
				PrintMessage(MSG_DUPLICATE_ARGS, *args);
				return -1;
			}

			writable = TRUE;
		}
		else if (_stricmp(*args, "/undo") == 0 ||
			_strnicmp(*args, "/undo:", 6) == 0) {
			if (undoable) {
				PrintMessage(MSG_DUPLICATE_ARGS, *args);
				return -1;
			}

			undoable = TRUE;
			
			if (*(*args + 5) == ':') {
				undo_path = (*args + 6);
			}
		}
		else if (_strnicmp(*args, "/search:", 8) == 0) {
			if (search_path) {
				PrintMessage(MSG_DUPLICATE_ARGS, *args);
				return -1;
			}

			search_path = (*args + 8);
		}
		else if (_stricmp(*args, "/wb") == 0) {
			if (writeblock) {
				PrintMessage(MSG_DUPLICATE_ARGS, *args);
				return -1;
			}

			writeblock = TRUE;
		}
		else if (_strnicmp(*args, "/l:", 3) == 0) {
			if (drive_letters) {
				PrintMessage(MSG_DUPLICATE_ARGS, "/L");
				return -1;
			}

			drive_letters = *args + 3;
		}
		else if (_strnicmp(*args, "/p:", 3) == 0) {
			if (part_number != (ULONG)-1) {
				PrintMessage(MSG_DUPLICATE_ARGS, "/P");
				return -1;
			}

			if (!isdigit(*(*args + 3))) {
				PrintMessage(MSG_INVALID_PART, *args + 3);
				return -1;
			}

			part_number = atol(*args + 3);
		}
		else {
			PrintMessage(MSG_UNKNOWN_OPTION, *args, "OPEN");
			return -1;
		}
	}

	if ((writable && undoable) ||
		(writable && writeblock) ||
		(undoable && writeblock)) {
		PrintMessage(MSG_OPENMODE_OPTION);
		return -1;
	}

	//	ensure that the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		if ((ret = VdkInstall(NULL, FALSE)) == ERROR_SUCCESS) {
			PrintMessage(MSG_INSTALL_OK);
		}
		else {
			PrintMessage(MSG_INSTALL_NG);
			printf("%s\n", VdkStatusStr(ret));
			return -1;
		}
	}

	//	ensure that the driver is started

	if (driver_state != SERVICE_RUNNING) {
		if ((ret = VdkStart(&driver_state)) == ERROR_SUCCESS) {
			PrintMessage(MSG_START_OK);
		}
		else {
			PrintMessage(MSG_START_NG);
			printf("%s\n", VdkStatusStr(ret));
			return -1;
		}
	}

	//	Open device

	if (disk_number == (ULONG)-1) {
		disk_number = 0;

		for (;;) {
			ret = VdkCheckDeviceState(NULL, disk_number, 0);

			if (ret == ERROR_NOT_READY) {
				//	drive is empty -- use this disk
				break;
			}
			else if (ret == ERROR_SUCCESS || ret == ERROR_BUSY) {
				//	drive is busy --  try next disk
				CloseHandle(hDevice);
				disk_number++;
			}
			else if (ret == ERROR_PATH_NOT_FOUND) {
				//	no more drive
				//	-- create a new disk
				ret = VdkCreateDisk(NULL);

				if (ret != ERROR_SUCCESS) {
					PrintMessage(MSG_CREATEDISK_NG, disk_number);
					printf("%s\n", VdkStatusStr(ret));
					return -1;
				}

				PrintMessage(MSG_CREATEDISK_OK, disk_number);
			}
			else {
				//	unexpected error -- abort operation
				PrintMessage(MSG_OPENIMAGE_NG);
				printf("%s\n", VdkStatusStr(ret));
				return -1;
			}
		}
	}
	else {
		//	Ensure that another image is not opened

		ret = VdkCheckDeviceState(NULL, disk_number, 0);

		if (ret != ERROR_NOT_READY) {
			PrintMessage(MSG_OPENIMAGE_NG);
			printf("%s\n", VdkStatusStr(ret == ERROR_SUCCESS ? ERROR_BUSY : ret));
			return -1;
		}
	}

	hDevice = VdkOpenDevice(disk_number, 0);

	if (hDevice == INVALID_HANDLE_VALUE) {
		ret = GetLastError();
		PrintMessage(MSG_OPENIMAGE_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	//
	//	Create File structure
	//
	VDiskSetCallBack(VDiskCallback);

	VDiskSetSearchPath(search_path);
	
	ret = VDiskLoadFile(&disk, file_path, NULL);

	if (ret != ERROR_SUCCESS || disk == NULL) {
		PrintMessage(MSG_OPENIMAGE_NG);
		printf("%s\n", VdkStatusStr(ret == ERROR_SUCCESS ? ERROR_INVALID_FUNCTION : ret));
		goto cleanup;
	}

	ret = VDiskCreateTree(disk);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_OPENIMAGE_NG);
		printf("%s\n", VdkStatusStr(ret));
		goto cleanup;
	}

	//
	//	Create REDO log
	//
	if (undoable) {
		ret = VDiskCreateRedo(&disk, undo_path);

		if (ret != ERROR_SUCCESS) {
			PrintMessage(MSG_CREATEREDO_NG);
			printf("%s\n", VdkStatusStr(ret));
			VDiskDelete(disk);
			return -1;
		}
	}

	//	Open the image files
	if (undoable || writable) {
		access = VDK_DISKTYPE_WRITABLE;
	}
	else if (writeblock) {
		access = VDK_DISKTYPE_WRITEBLOCK;
	}
	else {
		access = VDK_DISKTYPE_READONLY;
	}

	PrintMessage(MSG_VIRTUAL_DISK, disk_number);

	ret = VdkOpen(hDevice, 0, disk, access);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_OPENIMAGE_NG);
		printf("%s\n", VdkStatusStr(ret));
		goto cleanup;
	}

	//
	//	print file information
	//
	if ((ret = VdkGetFileInfo(hDevice, 0, &file_info)) != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_IMAGE_NG, disk_number);
		printf("%s\n", VdkStatusStr(ret));
		goto cleanup;
	}

	if (file_info) {
		PrintFileInfo(file_info);
	}

	//
	//	assign drive letters and print partition info
	//
	PrintMessage(MSG_PARTITION_HEADER);

	assign.disk_number	= disk_number;
	assign.part_number	= part_number;
	assign.read_only	= (access == VDK_DISKTYPE_READONLY);
	assign.drive_letters = drive_letters;

	ret = VdkListPartitions(NULL, hDevice, file_info->Capacity, Assign_Callback, &assign);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_PART_NG);
		printf("%s\n", VdkStatusStr(ret));
		goto cleanup;
	}

cleanup:
	if (hDevice && hDevice != INVALID_HANDLE_VALUE) {
		CloseHandle(hDevice);
	}

	if (file_info) {
		VdkFreeMem(file_info);
	}

	if (disk) {
		VDiskDelete(disk);
	}

	return (ret == ERROR_SUCCESS ? 0 : -1);
}

//
//	Close current file(s)
//	Command Line Parameters:
//		disk number or drive letter
//
#define CLOSE_PROMPT	0
#define CLOSE_QUIET		1
#define CLOSE_FORCE		2

int	Close(char **args)
{
	ULONG	disk_number;
	ULONG	max_disk;
	CHAR	drive_letter;
	ULONG	interact = CLOSE_PROMPT;
	DWORD	ret;

	if (**args == '*') {
		disk_number = (ULONG)-1;
		drive_letter = '\0';
	}
	else if (isdigit(**args)) {
		disk_number = atol(*args);
		drive_letter = '\0';
	}
	else if (isalpha(**args)) {
		disk_number = (ULONG)-1;
		drive_letter = (CHAR)toupper(**args);
	}
	else {
		PrintMessage(MSG_INVALID_DISK, *args);
		return -1;
	}

	if (*(++args)) {
		if (!_stricmp(*args, "/q")) {
			interact = CLOSE_QUIET;
		}
		else if (!_stricmp(*args, "/f")) {
			interact = CLOSE_FORCE;
		}
		else {
			PrintMessage(MSG_UNKNOWN_OPTION, *args, "CLOSE");
			return -1;
		}
	}

	//	ensure that the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_NOT_INSTALLED);
		return -1;
	}

	//	ensure that the driver is running

	if (driver_state != SERVICE_RUNNING) {
		PrintMessage(MSG_NOT_RUNNING);
		return -1;
	}

	//	get the disk number from the drive letter

	if (drive_letter) {
		ret = GetDiskFromDrive(drive_letter, &disk_number);

		if (ret != ERROR_SUCCESS) {
			PrintMessage(MSG_RESOLVE_LINK_NG, drive_letter);
			printf("%s\n", VdkStatusStr(ret));
			return -1;
		}
	}

	//	Close virtual disk

	if (disk_number == (ULONG)-1) {
		disk_number = 0;

		ret = VdkGetDriverInfo(NULL, &max_disk, NULL, NULL, NULL);

		if (ret != ERROR_SUCCESS) {
			PrintMessage(MSG_GET_CONFIG_NG);
			printf("%s\n", VdkStatusStr(ret));
			return -1;
		}
	}
	else {
		max_disk = 0;
	}

	do {
		PrintMessage(MSG_CLOSING_DISK, disk_number);
		fflush(stdout);

		ret = VdkCloseDrive(
			disk_number, Retry_Callback, Continue_Callback, (PVOID)interact);

		if (ret == ERROR_SUCCESS) {
			PrintMessage(MSG_CLOSE_OK);
		}
		else if (ret == ERROR_NOT_READY) {
			PrintMessage(MSG_DEVICE_EMPTY);
		}
		else if (ret == ERROR_BUSY) {
			PrintMessage(MSG_DEVICE_BUSY);
		}
		else if (!interact || max_disk == 0) {
			return -1;
		}
	}
	while (++disk_number < max_disk);

	return 0;
}

//
//	Assign a drive letter to a partition
//	Command Line Parameters:
//		disk number - required
//		partition number - required
//		(optional) drive letter - default to first available letter
//
int	Link(char **args)
{
	ULONG	disk_number;
	ULONG	part_number;
	char	drive_letter;
	DWORD	ret;

	//	ensure that the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_NOT_INSTALLED);
		return -1;
	}

	//	ensure that the driver is running

	if (driver_state != SERVICE_RUNNING) {
		PrintMessage(MSG_NOT_RUNNING);
		return -1;
	}

	//	process command line parameters

	if (!isdigit(**args)) {
		PrintMessage(MSG_INVALID_DISK, *args);
		return -1;
	}

	disk_number = atol(*(args++));

	if (!isdigit(**args)) {
		PrintMessage(MSG_INVALID_PART, *args);
		return -1;
	}

	part_number = atol(*(args++));

	if (*args) {
		if (!isalpha(**args)) {
			PrintMessage(MSG_UNKNOWN_OPTION, *args, "LINK");
			return -1;
		}

		drive_letter = **args;
	}
	else {
		drive_letter = ChooseDriveLetter();

		if (!isalpha(drive_letter)) {
			PrintMessage(MSG_DRIVE_FULL);
			return -1;
		}
	}

	drive_letter = (char)toupper(drive_letter);

	ret = VdkSetDriveLetter(disk_number, part_number, drive_letter);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_LINK_NG, drive_letter, disk_number, part_number);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	PrintMessage(MSG_LINK_OK, drive_letter, disk_number, part_number);

	return 0;
}

//
//	Remove a drive letter from a partition
//	Command Line Parameters:
//		partition number or drive letter
//
int	Unlink(char **args)
{
	ULONG	disk_number		= (ULONG)-1;
	ULONG	part_number		= (ULONG)-1;
	TCHAR	drive_letter	= '\0';
	DWORD	ret;

	//	ensure that the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_NOT_INSTALLED);
		return -1;
	}

	//	ensure that the driver is running

	if (driver_state != SERVICE_RUNNING) {
		PrintMessage(MSG_NOT_RUNNING);
		return -1;
	}

	//	process command line parameters

	if (isdigit(**args)) {
		disk_number = atol(*(args++));

		if (!*args) {
			PrintMessage(MSG_USAGE_ULINK);
			return -1;
		}

		if (!isdigit(**args)) {
			PrintMessage(MSG_INVALID_PART, *args);
			return -1;
		}

		part_number = atol(*(args++));
	}
	else if (isalpha(**args)) {
		drive_letter = (char)toupper(**(args++));
	}
	else {
		PrintMessage(MSG_UNKNOWN_OPTION, *args, "ULINK");
		return -1;
	}

	if (*args) {
		PrintMessage(MSG_USAGE_ULINK);
		return -1;
	}

	if (!drive_letter) {
		// disk, part are specified
		ret = VdkGetDriveLetter(disk_number, part_number, &drive_letter);

		if (ret != ERROR_SUCCESS) {
			PrintMessage(MSG_GET_LINK_NG, disk_number, part_number);
			printf("%s\n", VdkStatusStr(ret));
			return -1;
		}

		if (!isalpha(drive_letter)) {
			PrintMessage(MSG_GET_LINK_NG, disk_number, part_number);
			printf("%s\n", VdkStatusStr(ERROR_FILE_NOT_FOUND));
			return -1;
		}
	}

	if (isalpha(drive_letter)) {
		ret = VdkDelDriveLetter(drive_letter);

		if (ret != ERROR_SUCCESS) {
			PrintMessage(MSG_UNLINK_NG, drive_letter);
			printf("%s\n", VdkStatusStr(ret));
			return -1;
		}
	}

	PrintMessage(MSG_UNLINK_OK, drive_letter);

	return 0;
}

//
//	Print image file information
//	Command Line Parameters:
//		disk number or drive letter
//
int Image(char **args)
{
	ULONG	disk_number;
	ULONG	max_disk;
	DWORD	ret;

	//	ensure that the driver is installed

	if (driver_state == VDK_NOT_INSTALLED) {
		PrintMessage(MSG_NOT_INSTALLED);
		return -1;
	}

	//	ensure that the driver is running

	if (driver_state != SERVICE_RUNNING) {
		PrintMessage(MSG_NOT_RUNNING);
		return -1;
	}

	ret = VdkGetDriverInfo(NULL, &max_disk, NULL, NULL, NULL);

	if (ret != ERROR_SUCCESS) {
		PrintMessage(MSG_GET_CONFIG_NG);
		printf("%s\n", VdkStatusStr(ret));
		return -1;
	}

	if (*args) {
		if (isdigit(**args)) {
			disk_number = atol(*args);
		}
		else if (isalpha(**args)) {
			ret = GetDiskFromDrive(**args, &disk_number);

			if (ret != ERROR_SUCCESS) {
				PrintMessage(MSG_RESOLVE_LINK_NG, **args);
				return -1;
			}
		}
		else {
			PrintMessage(MSG_INVALID_DISK, *args);
			return -1;
		}

		if (disk_number >= max_disk) {
			PrintMessage(MSG_INVALID_DISK, *args);
			return -1;
		}

		max_disk = disk_number;
	}
	else {
		disk_number = 0;
	}

	do {
		HANDLE hDisk;
		PVDK_OPEN_FILE_INFO	file_info;

		PrintMessage(MSG_VIRTUAL_DISK, disk_number);

		hDisk = VdkOpenDevice(disk_number, 0);

		if (hDisk == INVALID_HANDLE_VALUE) {
			ret = VdkLastError();
			PrintMessage(MSG_GET_IMAGE_NG, disk_number);
			printf("%s\n", VdkStatusStr(ret));
			continue;
		}

		ret = VdkCheckDeviceState(hDisk, 0, 0);

		if (ret != ERROR_SUCCESS) {
			if (ret == ERROR_BUSY) {
				PrintMessage(MSG_DEVICE_BUSY);
				printf("\n");
			}
			else if (ret == ERROR_NOT_READY) {
				PrintMessage(MSG_IMAGE_NONE);
				printf("\n");
			}
			else {
				printf("%s\n\n", VdkStatusStr(ret));
			}

			CloseHandle(hDisk);
			continue;
		}

		ret = VdkGetFileInfo(hDisk, 0, &file_info);

		if (ret != ERROR_SUCCESS) {
			CloseHandle(hDisk);
			PrintMessage(MSG_GET_IMAGE_NG, disk_number);
			printf("%s\n", VdkStatusStr(ret));
			continue;
		}

		//	print image file information

		PrintFileInfo(file_info);

		if (!file_info) {
			CloseHandle(hDisk);
			continue;
		}

		if (!file_info->DiskType) {
			VdkFreeMem(file_info);
			CloseHandle(hDisk);
			continue;
		}

		//
		//	Print partition information
		//
		PrintMessage(MSG_PARTITION_HEADER);

		ret = VdkListPartitions(NULL, hDisk, file_info->Capacity, PartList_Callback, (PVOID)disk_number);

		VdkFreeMem(file_info);
		CloseHandle(hDisk);

		if (ret != ERROR_SUCCESS) {
			PrintMessage(MSG_GET_PART_NG);
			printf("%s\n", VdkStatusStr(ret));
			continue;
		}

		printf("\n");
	}
	while (++disk_number < max_disk);

	return 0;
}

//
//	Print usage help
//	Command Line Parameters:
//		(optional) command
//
int	Help(char **args)
{
	DWORD msg = MSG_HELP_GENERAL;

	if (args && *args) {
		int idx = 0;

		while (Commands[idx].cmd) {
			if (_stricmp(*args, Commands[idx].cmd) == 0) {
				msg = Commands[idx].helpmsg;
				break;
			}
			idx++;
		}

		if (!Commands[idx].cmd) {
			PrintMessage(MSG_UNKNOWN_COMMAND, *args);
			msg = MSG_HELP_HELP;
		}
	}

	PrintMessage(msg);

	return 0;
}

//
//	Print device information
//
int	Device(char **args)
{
	ULONG device_num;
	PVDK_DEVICE_INFO device_info;

	UNREFERENCED_PARAMETER(args);

	if (VdkGetDeviceList(&device_num, &device_info) != ERROR_SUCCESS) {
		return -1;
	}

	while (device_num--) {
		printf("Device Type  : %s\n", (device_info[device_num].DeviceType == VDK_DEVICE_DISK) ? "DISK" : "PART");
		printf("Device Name  : %s\n", device_info[device_num].DeviceName);
		printf("Symbolic Link: %s\n", device_info[device_num].SymbolicLink);
		printf("Reference    : %lu\n", device_info[device_num].ReferenceCount);
		printf("\n");
	}

	VdkFreeMem(device_info);
	return 0;
}

#ifdef _DEBUG

struct _traceflags {
	char *name;
	ULONG value;
}
flagtable[] = {
	"WARN",		VDKWARN,
	"INFO",		VDKINFO,
	"OPEN",		VDKOPEN,
	"CLOSE",	VDKCLOSE,
	"READ",		VDKREAD,
	"WRITE",	VDKWRITE,
	"DISPATCH",	VDKDISPATCH,
	"IOCTL",	VDKIOCTL,
	"UPDATE",	VDKUPDATE,
	"CREATE",	VDKCREATE,
	"DELETE",	VDKDELETE,
	"FORMAT",	VDKFORMAT,
	"SERVER",	VDKSERVER,
	"CLIENT",	VDKCLIENT,
	NULL, 0
};

ULONG ParseFlag(PCHAR name)
{
	int idx;

	if (!_stricmp(name, "ALL")) {
		return 0xffffffff;
	}

	idx = 0;

	while (flagtable[idx].name) {
		if (!_stricmp(name, flagtable[idx].name)) {
			return flagtable[idx].value;
		}
		idx++;
	}

	return 0;
}

int	Trace(char **args)
{
	ULONG	flags, changed;
	int idx;

	flags = VdkTraceFlags(NULL);
	changed = flags;

	while (*args) {
		if (**args == '-') {
			changed &= ~(ParseFlag(*args + 1));
		}
		else if (**args == '+') {
			changed |= ParseFlag(*args + 1);
		}
		else {
			changed |= ParseFlag(*args);
		}
		args++;
	}

	if (changed != flags) {
		flags = VdkTraceFlags(&changed);
	}

	printf("Trace flag is : 0x%08x\n", flags);

	printf("Enabled : ");

	idx = 0;

	while (flagtable[idx].name) {
		if ((flagtable[idx].value & flags) == flagtable[idx].value) {
			printf("%s ", flagtable[idx].name);
		}
		idx++;
	}

	printf("\nDisabled: ");

	idx = 0;

	while (flagtable[idx].name) {
		if ((flagtable[idx].value & flags) != flagtable[idx].value) {
			printf("%s ", flagtable[idx].name);
		}
		idx++;
	}
	printf("\n");
	return 0;
}
#endif _DEBUG

//
//	Get disk number from a drive letter
//
DWORD GetDiskFromDrive(CHAR drive, PULONG disk)
{
	TCHAR	dos_device[] = " :";
	TCHAR	device_name[MAX_DEVNAME_LEN], dos_name[MAX_PATH];
	DWORD	ret;

	dos_device[0] = drive;

	if (!QueryDosDevice(dos_device, dos_name, sizeof(dos_name))) {
		return GetLastError();
	}

	ret = VdkGetDriverInfo(NULL, disk, NULL, NULL, NULL);

	if (ret != ERROR_SUCCESS) {
		return ret;
	}

	while ((*disk)--) {
		ret = VdkGetDeviceName((*disk), (ULONG)-1, device_name);

		if (ret != ERROR_SUCCESS) {
			return ret;
		}

		if (!_strnicmp(dos_name, device_name, strlen(device_name))) {
			return ERROR_SUCCESS;
		}
	}

	return ERROR_FILE_NOT_FOUND;
}

//
//	Print image file information
//
void PrintFileInfo(PVDK_OPEN_FILE_INFO file_info)
{
	ULONG total_sectors;
	PCHAR name_pos;
	ULONG idx;

	if (!file_info) {
		PrintMessage(MSG_IMAGE_NONE);
		printf("\n");
		return;
	}

	//	Virtual disk access type

	if (file_info->DiskType) {
		PrintMessage(MSG_ACCESS_TYPE);

		switch (file_info->DiskType) {
		case VDK_DISKTYPE_WRITABLE:
			PrintMessage(MSG_ACCESS_RW);
			break;

		case VDK_DISKTYPE_READONLY:
			PrintMessage(MSG_ACCESS_RO);
			break;

		case VDK_DISKTYPE_WRITEBLOCK:
			PrintMessage(MSG_ACCESS_WB);
			break;
		}
	}

	PrintMessage(MSG_DISK_CAPACITY,
		file_info->Capacity,
		file_info->Capacity / 2048);

	if (file_info->Cylinders &&
		file_info->Tracks &&
		file_info->Sectors) {

		PrintMessage(MSG_DISK_GEOMETRY,
			file_info->Cylinders,
			file_info->Tracks,
			file_info->Sectors);
	}

	PrintMessage(MSG_DISK_FILES,
		file_info->FilesTotal);

	PrintMessage(MSG_FILE_HEADER);

	total_sectors = 0;
	name_pos = (PCHAR)&(file_info->Files[file_info->FilesTotal]);

	for (idx = 0; idx < file_info->FilesTotal; idx++) {

		if (total_sectors == 0) {
			printf(" -------  -------  ----\n");
		}
		switch (file_info->Files[idx].FileType) {
		case VDK_FILETYPE_NONE:
			printf("  NONE ");
			break;

		case VDK_FILETYPE_FLAT:
			printf("  FLAT ");
			break;

		case VDK_FILETYPE_COWD:
			printf("  COWD ");
			break;

		case VDK_FILETYPE_VMDK:
			printf("  VMDK ");
			break;

		default:
			printf("  ???  ");
			break;
		}

		printf("  %8lu  %s\n",
			file_info->Files[idx].Capacity,
			name_pos);

		name_pos += file_info->Files[idx].NameLength;

		total_sectors += file_info->Files[idx].Capacity;

		if (total_sectors == file_info->Capacity) {
			total_sectors = 0;
		}
	}
	printf("\n");
}

//
//	Confirms unmount retry
//
DWORD Retry_Callback(PVOID param, DWORD err)
{
	printf("\n");
	PrintMessage(MSG_DISMOUNT_NG);
	printf("%s\n", VdkStatusStr(err));

	if (param) {
		// quiet / force mode
		return FALSE;
	}
	else {
		// normal mode
		PrintMessage(MSG_DRIVE_IN_USE);
		return (InputChar(MSG_PROMPT_RETRY, "yn") == 'y');
	}
}

//
//	Confirms unmount continue
//
DWORD Continue_Callback(PVOID param, DWORD err)
{
	UNREFERENCED_PARAMETER(err);

	if ((ULONG)param == CLOSE_QUIET) {
		// quiet mode
		return FALSE;
	}
	else if ((ULONG)param == CLOSE_FORCE) {
		// force mode
		PrintMessage(MSG_CLOSE_FORCED);
		return TRUE;
	}
	else {
		// prompt mode
		printf("\n");
		PrintMessage(MSG_FORCING_WARN);

		return (InputChar(MSG_PROMPT_CONTINUE, "yn") == 'y');
	}
}

//
//	VdkListPartitions callback
//	called for each partition item
//
void PartList_Callback(PPARTITION_ITEM pitem, PVOID param)
{
	CHAR drive[] = " :";

	if ((ULONG)param != (ULONG)-1) {
		VdkGetDriveLetter(
			(ULONG)param,
			pitem->idx,
			&drive[0]);
	}

	if (!isalpha(drive[0])) {
		strcpy(drive, "  ");
	}

	if (pitem->idx) {
		PCHAR name = GetPartitionTypeName(pitem->type);

		printf(" %s  %2d%c  %12lu   %8lu (%6lu MB)  %02xh:%s\n",
			drive,
			pitem->idx,
			pitem->num > 4 ? '*' : ' ',
			pitem->offset,
			pitem->length,
			pitem->length / 2048,
			pitem->type,
			name ? name : "");
	}
	else {
		printf(" %s  %2d%c  %12lu   %8lu (%6lu MB)  %s\n",
			drive,
			pitem->idx,
			pitem->num > 4 ? '*' : ' ',
			pitem->offset,
			pitem->length,
			pitem->length / 2048,
			pitem->fsname);
	}
}

//
//	VdkGetDriveLayout callback
//	called for each recognized partition item
//
void Assign_Callback(PPARTITION_ITEM pitem, PVOID param)
{
	PASSIGN_PARAM assign = (PASSIGN_PARAM)param;
	VDKSTAT ret;

	if ((assign->part_number == (ULONG)-1 &&
		IsPartitionMountable(pitem->type, assign->read_only)) ||
		assign->part_number == pitem->idx) {

		CHAR letter = ChooseDriveLetter();

		if (!isalpha(letter)) {
			PrintMessage(MSG_DRIVE_FULL);
			goto print_info;
		}

		if (assign->drive_letters &&
			isalpha(*(assign->drive_letters)) &&
			*(assign->drive_letters) > letter) {

			letter = *((assign->drive_letters)++);
		}

		ret = VdkSetDriveLetter(
			assign->disk_number,
			pitem->idx,
			letter);

		if (ret != ERROR_SUCCESS) {
			PrintMessage(MSG_LINK_NG, letter, assign->disk_number, pitem->idx);
			printf("%s\n", VdkStatusStr(ret));
		}
	}

print_info:
	//
	//	print partition information
	//
	PartList_Callback(pitem, (PVOID)(assign->disk_number));
}

//
//	Check if current user belongs to Administrators group
//
BOOL IsUserAdmin()
{
	DWORD	i, dwSize = 0;
	HANDLE	hToken;
	PTOKEN_GROUPS pGroupInfo;
	BYTE	sidBuffer[100];
	PSID	pSID = (PSID)&sidBuffer;
	SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
	BOOL	ret = FALSE;

	// Open a handle to the access token for the calling process.

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		return FALSE;
	}

	// Call GetTokenInformation to get the buffer size.

	if(!GetTokenInformation(hToken, TokenGroups, NULL, dwSize, &dwSize)) {

		if(GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
			return FALSE;
		}
	}

	// Allocate the buffer.

	pGroupInfo = (PTOKEN_GROUPS)GlobalAlloc(GPTR, dwSize);

	// Call GetTokenInformation again to get the group information.

	if (!GetTokenInformation(hToken, TokenGroups, pGroupInfo, dwSize, &dwSize)) {
		goto cleanup;
	}

	// Create a SID for the BUILTIN\Administrators group.

	if (!AllocateAndInitializeSid(
		&SIDAuth, 2, SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pSID)) {
		goto cleanup;
	}

	// Loop through the group SIDs looking for the administrator SID.

	for (i = 0; i < pGroupInfo->GroupCount; i++) {

		if (EqualSid(pSID, pGroupInfo->Groups[i].Sid)) {
			ret = TRUE;
			break;
		}
	}

cleanup:
	if (pSID) {
		FreeSid(pSID);
	}

	if (pGroupInfo) {
		GlobalFree(pGroupInfo);
	}

	return ret;
}
