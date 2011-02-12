/*
	vdkdrv.h

	Virtual Disk kernel-mode driver for Windows NT platform
	Copyright (C) 2003 Ken Kato
*/

#ifndef _VDKDRV_H_
#define _VDKDRV_H_

//
//	driver local constants
//
#define VDK_ZOMBIE		0xffffffffL

typedef struct _PART_EXTENSION	PART_EXTENSION, *PPART_EXTENSION;
typedef struct _DISK_EXTENSION	DISK_EXTENSION, *PDISK_EXTENSION;

//
// This is the partition extension, which is attached to all
// partition"n" device objects - except for partition 0, which
// gets a disk extension, which has this structure imbeded.
//
// NOTE THIS MUST BE EXACTLY THE SAME AS THE FIRST FIELDS OF THE
// DISK EXTENSION.
//

struct _PART_EXTENSION
{
	PARTITION_INFORMATION PartitionInfo;	// Standard partition information
	ULONG			PartitionOrdinal;		// Order partition appears on disk
	PDISK_EXTENSION	FirstPartition;			// Partition0 device extension
	PDEVICE_OBJECT	NextPartition;			// Next partition device object

	UNICODE_STRING	DeviceName;				// Real name of the device object
											// \Device\VirtualDiskVolume<n>

	UNICODE_STRING	SymbolicLink;			// Symbolic link to the device object
											// \Device\VirtualDK<x>\Partition<y>
};


//
// This is the disk extension, which is attached to all partition 0 device
// objects (which represent the disk).	NOTE THAT THE FIRST FIELDS ARE
// IDENTICAL TO THOSE OF THE PARTITION EXTENSION, so that the same code can
// access the disk via partition 0 or partition n.
//
struct _DISK_EXTENSION {
	//
	// These first entries must be exactly the same as PART_EXTENSION
	//
	PARTITION_INFORMATION PartitionInfo;	// Standard partition information
											// Stores information about whole disk

	ULONG			PartitionOrdinal;		// Order partition appears on disk
											// Always 0

	PDISK_EXTENSION	FirstPartition;			// Partition0 device extension
											// Always points to itself

	PDEVICE_OBJECT	NextPartition;			// Next partition device object

	UNICODE_STRING	DeviceName;				// Real name of the device object
											// \Device\Harddisk<n>\Partition0

	UNICODE_STRING	SymbolicLink;			// Symbolic link to the device object
											// \??\VirtualDK<n>

	//
	// disk specific fields
	//
	PDEVICE_OBJECT	DeviceObject;			// Pointer to the device object
	HANDLE			DirectoryHandle;		// Handle to the directory object

	ULONG			VirtualNumber;			// Value <n> of \??\VirtualDK<n>
	ULONG			PhysicalNumber;			// Value <n> of \Device\Harddisk<n>
	UNICODE_STRING	AnotherLink;			// \??\PhysicalDrive<n>

#ifdef VDK_SUPPORT_NETWORK
	//
	// Security context to access files on network drive
	//
	PSECURITY_CLIENT_CONTEXT	SecurityContext;
#endif	// VDK_SUPPORT_NETWORK

	//
	// IRP queue list
	//
	LIST_ENTRY		ListHead;				// List entry
	KSPIN_LOCK		ListLock;				// spin lock object

	//
	// device thread
	//
	KEVENT			RequestEvent;			// signaled when requests arrive
	PVOID			ThreadPointer;			// thread object pointer
	BOOLEAN 		TerminateThread;		// flag to terminate the thread

	//
	// image file information
	//
	VDK_DISK_INFO	DiskInfo;				// virtual disk information
};


//
// stanard driver routines
//

NTSTATUS
DriverEntry (
	IN	PDRIVER_OBJECT	DriverObject,
	IN	PUNICODE_STRING RegistryPath);

VOID
VdkUnloadDriver (
	IN PDRIVER_OBJECT	DriverObject);

NTSTATUS
VdkCreateClose (
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP 			Irp);

NTSTATUS
VdkReadWrite (
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP 			Irp);

NTSTATUS
VdkDeviceControl (
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP 			Irp);

NTSTATUS
VdkShutdown (
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				Irp);

//
// private routines
//
VOID
VdkThread (
	IN PVOID			Context);

NTSTATUS
VdkCreateDisk(
	IN PDRIVER_OBJECT	DriverObject,
	IN ULONG			DiskNumber);

VOID
VdkDeleteDevice(
	IN PDEVICE_OBJECT	DeviceObject);

VOID
VdkUpdateDevice(
	IN PDEVICE_OBJECT	DiskObject,
	IN PDRIVE_LAYOUT_INFORMATION	DriveLayout);

//
//	runtime operating system version
//
extern ULONG OsMajorVersion;
extern ULONG OsMinorVersion;
extern ULONG OsBuildNumber;

#define StoreCurrentOsVersion()	\
	PsGetVersion(&OsMajorVersion, &OsMinorVersion, &OsBuildNumber, NULL)

#if DBG
VOID	PrintIoCtrlStatus(
	PWSTR		dev_name,
	ULONG		ctrl_code,
	NTSTATUS	statuss);

PCSTR IoControlCodeToStr(
	ULONG 		ctrl_code);

#endif	// DBG

#endif	// _VDKDRV_H_
