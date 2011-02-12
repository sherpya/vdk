/*
	vdkutil.h

	header for platform dependent utility functions
	Copyright (C) 2003 Ken Kato
*/

#ifndef _VDK_UTIL_H_
#define _VDK_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

//*******************************************************************
//	common utility functions
//*******************************************************************

//
//	Get full path of a file
//
VDKSTAT VdkFullPath(
	PCHAR	FullPath,
	ULONG	BufSize,
	PCHAR	FileName);

//
//	open an existing file
//
VDKSTAT	VdkOpenFile(
	HANDLE	*FileHandle,
	PCHAR	FileName,
	ULONG	NameLen,
	ULONG	ReadOnly);

//
//	create a new file
//
VDKSTAT	VdkCreateFile(
	HANDLE	*FileHandle,
	PCHAR	FileName,
	BOOL	Force);

//
//	check file attributes
//
VDKSTAT	VdkCheckAttribute(
	HANDLE	FileHandle);

//
//	get file size in number of sectors
//
VDKSTAT	VdkGetFileSize(
	HANDLE	FileHandle,
	PINT64	FileSize);

//
//	set file size in number of sectors
//
VDKSTAT VdkSetFileSize(
	HANDLE	FileHandle,
	INT64	FileSize);

//
//	seek by byte offset
//
VDKSTAT VdkSeekFile(
	HANDLE	FileHandle,
	INT64	Offset);

//
//	write to file at specified byte offset
//
VDKSTAT	VdkWriteFileAt(
	HANDLE	FileHandle,
	INT64	Offset,
	PVOID	Buffer,
	ULONG	Length,
	PULONG_PTR	Result);

//
//	read from file at specified byte offset
//
VDKSTAT VdkReadFileAt(
	HANDLE	FileHandle,
	INT64	Offset,
	PVOID	Buffer,
	ULONG	Length,
	PULONG_PTR	Result);


//*******************************************************************
//	other utility macro / functions
//*******************************************************************

#ifdef VDK_KERNEL_DRIVER

#define VdkCloseFile(a)				ZwClose(a)
#define VdkAllocMem(a)				ExAllocatePool(NonPagedPool,a)
#define VdkFreeMem(a)				ExFreePool(a)
#define VdkZeroMem(a,b)				RtlZeroMemory(a,b)
#define VdkCopyMem(a,b,c)			RtlCopyMemory(a,b,c)

#elif defined(_WIN32)

#define VdkCloseFile(a)				CloseHandle(a)
#define VdkGetAttribute(a)			GetFileAttributes(a)

#define VdkAllocMem(a)				malloc(a)
#define VdkFreeMem(a)				free(a)
#define VdkZeroMem(a,b)				ZeroMemory(a,b)
#define VdkCopyMem(a,b,c)			CopyMemory(a,b,c)

#define VdkCmpNoCaseN(a,b,c)		_strnicmp(a,b,c)
#define VdkCmpNoCase(a,b)			_stricmp(a,b)

#define VdkLastError()				GetLastError()

const char *VdkStatusStr(VDKSTAT Status);
void PrintMessage(DWORD msg,...);
int InputChar(DWORD prompt, const char *accept);

#elif defined(__linux__)

#define VdkCloseFile(a)				close(a)
#define VdkGetAttribute(a)			(0)

#define VdkAllocMem(a)				malloc(a)
#define VdkFreeMem(a)				free(a)
#define VdkZeroMem(a,b)				memset(a,0,b)
#define VdkCopyMem(a,b,c)			memcpy(a,b,c)

#define VdkCmpNoCaseN(a,b,c)		strncasecmp(a,b,c)
#define VdkCmpNoCase(a,b)			strcasecmp(a,b)

#define VdkLastError()				errno
#define VdkStatusStr(a)				strerror(a)

#define PrintMessage				printf
int InputChar(const char *prompt, const char *accept);

#define UNREFERENCED_PARAMETER(x)
#define FIELD_OFFSET(type, field)	((long)&(((type *)0)->field))
#define MAX_PATH 					PATH_MAX

#endif


//*******************************************************************
//	debug utility
//*******************************************************************

#ifdef VDK_DEBUG

extern unsigned long	TraceFlags;
extern const char		*TraceFile;
extern unsigned long	TraceLine;

#define VDKTRACE(LEVEL,STRING)					\
	if ((TraceFlags & (LEVEL)) == (LEVEL)) {	\
		TraceFile = __FILE__;					\
		TraceLine = __LINE__;					\
		VdkTrace STRING;						\
	}

#ifdef VDK_KERNEL_DRIVER
PCSTR	VdkStatusStr(NTSTATUS status);
#define VdkTrace			DbgPrint
#else	// VDK_KERNEL_DRIVER
void	VdkTrace(const char *,...);
#endif	// VDK_KERNEL_DRIVER

#else	// VDK_DEBUG

#define VDKTRACE(LEVEL,STRING)

#endif	// VDK_DEBUG

#ifdef __cplusplus
}
#endif
#endif	// _VDK_UTIL_H_
