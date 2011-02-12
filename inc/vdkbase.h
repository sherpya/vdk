/*
	vdkbase.h

	VDK base stuff
	Copyright (C) 2003 Ken Kato
*/

#ifndef _VDKBASE_H_
#define _VDKBASE_H_


#if (DBG || defined(_DEBUG) || defined (DEBUG))
#define VDK_DEBUG
#else
#undef	VDK_DEBUG
#endif


//*******************************************************************
//	system headers
//*******************************************************************

#if defined(VDK_KERNEL_DRIVER)
//	NT kernel mode driver

#pragma warning(disable: 4115 4201 4214 4514)
//	4115:type defined in parameter list
//	4201:anonymous structure
//	4214:non-integer bit field
//	4514:unreferenced inline function
#include <ntifs.h>
#include <ntddk.h>
#include <ntverp.h>

#elif defined(_WIN32)
//	Win32 user application

#define WIN32_LEAN_AND_MEAN
#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>

#define strlen(x) ((ULONG) strlen(x))

#elif defined(__linux__)
//	Linux application

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#else
#error	Target not supported
#endif


//*******************************************************************
//	custom data types
//*******************************************************************

#ifdef _WIN32

typedef	LONGLONG					INT64, *PINT64;
typedef	ULONGLONG					UINT64, *PUINT64;

#else	// __linux__

typedef void						VOID, *PVOID;
typedef char						CHAR, *PCHAR;
typedef unsigned char				UCHAR, *PUCHAR;
typedef short						SHORT, *PSHORT;
typedef unsigned short				USHORT, *PUSHORT;
typedef long						LONG, *PLONG;
typedef unsigned long				ULONG, *PULONG;
typedef long long					INT64, *PINT64;
typedef unsigned long long			UINT64, *PUINT64;

typedef int							HANDLE;

#endif	// __linux__

#ifndef BOOL
#define BOOL						int
#endif
#ifndef TRUE
#define TRUE						1
#endif
#ifndef FALSE
#define FALSE						0
#endif


//*******************************************************************
//	status type and values
//*******************************************************************

#ifdef VDK_KERNEL_DRIVER

typedef NTSTATUS					VDKSTAT;
#define VDKSUCCESS(s)				NT_SUCCESS(s)

#define VDK_OK						STATUS_SUCCESS
#define VDK_NOMEMORY				STATUS_INSUFFICIENT_RESOURCES
#define	VDK_INTERNAL				STATUS_DRIVER_INTERNAL_ERROR
#define VDK_PARAM					STATUS_INVALID_PARAMETER
#define VDK_BUFFER					STATUS_BUFFER_TOO_SMALL
#define VDK_DATA					STATUS_DATA_ERROR

#elif defined(_WIN32)

typedef DWORD						VDKSTAT;
#define VDKSUCCESS(s)				((s) == VDK_OK)

#define VDK_OK						ERROR_SUCCESS
#define VDK_FUNCTION				ERROR_INVALID_FUNCTION
#define VDK_NOFILE					ERROR_FILE_NOT_FOUND
#define VDK_NOPATH					ERROR_PATH_NOT_FOUND
#define VDK_ACCESS					ERROR_ACCESS_DENIED
#define VDK_DATA					ERROR_INVALID_DATA
#define	VDK_INTERNAL				ERROR_INVALID_DATA
#define VDK_NOMEMORY				ERROR_OUTOFMEMORY
#define VDK_EOF						ERROR_HANDLE_EOF
#define VDK_PARAM					ERROR_INVALID_PARAMETER
#define VDK_BUFFER					ERROR_INSUFFICIENT_BUFFER
#define VDK_CANCEL					ERROR_CANCELLED
#define VDK_NG						0xffffffffUL

#else

typedef int							VDKSTAT;
#define VDKSUCCESS(s)				((s) == 0)

#define VDK_OK						0
#define VDK_NOMEMORY				ENOMEM
#define	VDK_INTERNAL				EIO
#define VDK_PARAM					EINVAL
#define VDK_BUFFER					EINVAL
#define VDK_DATA					EINVAL
#define VDK_FUNCTION				EINVAL
#define VDK_NOFILE					ENOENT
#define VDK_NOPATH					ENOENT
#define VDK_ACCESS					EACCES
#define VDK_CANCEL					ECANCELED
#define VDK_EOF						EBADF
#define VDK_NG						-1
/*
	   E2BIG  		Arg list too long
	   EACCES 		Permission denied
	   EAGAIN 		Resource temporarily unavailable
	   EBADF  		Bad file descriptor
	   EBADMSG		Bad message
	   EBUSY  		Resource busy
	   ECANCELED	Operation canceled
	   ECHILD 		No child processes
	   EDEADLK		Resource deadlock avoided
	   EDOM   		Domain error
	   EEXIST 		File exists
	   EFAULT 		Bad address
	   EFBIG  		File too large
	   EINPROGRESS	Operation in progress
	   EINTR  		Interrupted function call
	   EINVAL 		Invalid argument
	   EIO	  		Input/output error
	   EISDIR 		Is a directory
	   EMFILE 		Too many open files
	   EMLINK 		Too many links
	   EMSGSIZE 	Inappropriate message buffer length
	   ENAMETOOLONG	Filename too long
	   ENFILE 		Too many open files in system
	   ENODEV 		No such device
	   ENOENT 		No such file or directory
	   ENOEXEC		Exec format error
	   ENOLCK 		No locks available
	   ENOMEM 		Not enough space
	   ENOSPC 		No space left on device
	   ENOSYS 		Function not implemented
	   ENOTDIR		Not a directory
	   ENOTEMPTY 	Directory not empty
	   ENOTSUP		Not supported
	   ENOTTY 		Inappropriate I/O control operation
	   ENXIO  		No such device or address
	   EPERM  		Operation not permitted
	   EPIPE  		Broken pipe
	   ERANGE 		Result too large
	   EROFS  		Read-only file system
	   ESPIPE 		Invalid seek
	   ESRCH  		No such process
	   ETIMEDOUT	Operation timed out
	   EXDEV  		Improper link
*/
#endif

//*******************************************************************
//	constant symbols
//*******************************************************************

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE		(-1)
#endif
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES		(-1)
#endif
#ifndef INVALID_SET_FILE_POINTER
#define	INVALID_SET_FILE_POINTER	(-1)
#endif

//
//	information trace level
//
#define VDKWARN						0x00000001
#define VDKINFO						0x00000003

//
//	information trace scope
//
#define VDKOPEN						0x00000010
#define VDKCLOSE					0x00000020
#define VDKREAD						0x00000040
#define VDKWRITE					0x00000080

#define VDKDISPATCH					0x00000100
#define VDKIOCTL					0x00000200
#define VDKUPDATE					0x00000400
#define VDKCREATE					0x00000800
#define VDKDELETE					0x00001000
#define VDKFORMAT					0x00002000

#define VDKSERVER					0x00010000
#define VDKCLIENT					0x00020000

//
// Disk type value for VDK_OPEN_DISK structure
//
#define VDK_DISKTYPE_NONE			0
#define VDK_DISKTYPE_READONLY		1
#define VDK_DISKTYPE_WRITABLE		2
#define VDK_DISKTYPE_WRITEBLOCK		3

//
// File type value for VDK_OPEN_FILE structrure
//
#define VDK_FILETYPE_NONE			0
#define VDK_FILETYPE_FLAT			1
#define VDK_FILETYPE_COWD			2
#define VDK_FILETYPE_VMDK			3

//
// default geometry values
//
#define VDK_BYTES_PER_SECTOR		512
#define VDK_SECTORS_PER_TRACK		32
#define VDK_TRACKS_PER_CYLINDER		64

//
// How many bits to shift to get sector offset from byte offset
//
#define VDK_BYTE_SHIFT_TO_SECTOR	9

//
// Used to check if a value is multiple of sector size
//
#define VDK_SECTOR_ALIGNMENT_MASK	(VDK_BYTES_PER_SECTOR - 1)


//
//	Path separator and printf format string
//
#if defined(_WIN32)

#define PATH_SEPARATOR_CHAR			'\\'
#define PATH_SEPARATOR_STR			"\\"
#define ALT_SEPARATOR_CHAR			'/'
#define ALT_SEPARATOR_STR			"/"

#define INT64_PRINT_FORMAT			"I64"

#define VDK_INVALID_ATTRIBUTES		\
	(FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_ENCRYPTED)

#else	// _WIN32

#define PATH_SEPARATOR_CHAR			'/'
#define PATH_SEPARATOR_STR			"/"
#define ALT_SEPARATOR_CHAR			'\\'
#define ALT_SEPARATOR_STR			"\\"

#define INT64_PRINT_FORMAT			"ll"

#define VDK_INVALID_ATTRIBUTES		0

#endif	// _WIN32

#endif	// _VDKBASE_H_
