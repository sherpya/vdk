/*
	vdkwin32.c

	VDK Win32 user application utility routines
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

//
// convert given path to absolute path
//
VDKSTAT VdkFullPath(
	PCHAR	FullPath,
	ULONG	BufSize,
	PCHAR	FileName)
{
	ULONG len = GetFullPathName(FileName, BufSize, FullPath, &FileName);

	if (!len) {
		return GetLastError();
	}

	if (!FileName || !*FileName) {
		return VDK_PARAM;
	}

	return VDK_OK;
}

//
//	seek file
//
VDKSTAT VdkSeekFile(
	HANDLE	FileHandle,
	INT64	Offset)
{
	LARGE_INTEGER loffset;
	DWORD err;

	loffset.QuadPart = Offset;

	loffset.LowPart = SetFilePointer(
		FileHandle,
		loffset.LowPart,
		&loffset.HighPart,
		FILE_BEGIN);

	if (loffset.LowPart == INVALID_SET_FILE_POINTER &&
		(err = GetLastError()) != ERROR_SUCCESS) {

		VDKTRACE(0,("[VDK] SetFilePointer - %s\n", VdkStatusStr(err)));
		return err;
	}

	return ERROR_SUCCESS;
}

//
//	Open file
//
VDKSTAT	VdkOpenFile(
	HANDLE	*FileHandle,
	PCHAR	FileName,
	ULONG	NameLen,
	ULONG	ReadOnly)
{
	TCHAR buf[MAX_PATH];

	if (NameLen >= sizeof(buf)) {
		NameLen = sizeof(buf) - 1;
	}

	memcpy(buf, FileName, NameLen);
	buf[NameLen] = '\0';

	*FileHandle = CreateFile(
		buf,
		ReadOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
		ReadOnly ? FILE_SHARE_READ : 0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (*FileHandle == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();

		VDKTRACE(0,("[VDK] CreateFile\"%s\" - %s\n", buf, VdkStatusStr(err)));
		return err;
	}

	return ERROR_SUCCESS;
}

//
//	create a new file
//
VDKSTAT	VdkCreateFile(
	HANDLE	*FileHandle,
	PCHAR	FileName,
	BOOL	Force)
{
	*FileHandle = CreateFile(
		FileName,
		(GENERIC_READ | GENERIC_WRITE),
		0,
		NULL,
		Force ? CREATE_ALWAYS : CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (*FileHandle == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();

		VDKTRACE(0,("[VDK] CreateFile - %s\n", VdkStatusStr(err)));
		return err;
	}

	return ERROR_SUCCESS;
}

//
//	check file attributes
//

VDKSTAT VdkCheckAttribute(
	HANDLE			FileHandle)
{
	BY_HANDLE_FILE_INFORMATION info;

	if (!GetFileInformationByHandle(FileHandle, &info)) {
		DWORD err = GetLastError();

		VDKTRACE(0,
			("[VDK] GetFileInformationByHandle - %s\n",
			VdkStatusStr(err)));
		return err;
	}

	if (info.dwFileAttributes & VDK_INVALID_ATTRIBUTES) {
		VDKTRACE(0,("[VDK] File is compressed and/or encrypted\n"));

		return ERROR_ACCESS_DENIED;
	}

	return ERROR_SUCCESS;
}

//
//	get file size
//
VDKSTAT	VdkGetFileSize(
	HANDLE	FileHandle,
	PINT64	FileSize)
{
	ULARGE_INTEGER size;
	DWORD err;

	size.LowPart = GetFileSize(FileHandle, &size.HighPart);

	if (size.LowPart == INVALID_FILE_SIZE &&
		(err = GetLastError()) != ERROR_SUCCESS) {
		VDKTRACE(0,("[VDK] GetFileSize - %s\n", VdkStatusStr(err)));
		return err;
	}

	*FileSize = size.QuadPart;

	return ERROR_SUCCESS;
}

//
//	write to file at offset
//
VDKSTAT	VdkWriteFileAt(
	HANDLE			FileHandle,
	INT64			Offset,
	PVOID			Buffer,
	ULONG			Length,
	PULONG_PTR		Result)
{
	DWORD err = ERROR_SUCCESS;

	if (Offset != -1) {
		err = VdkSeekFile(FileHandle, Offset);

		if (err != ERROR_SUCCESS) {
			return err;
		}
	}

	if (WriteFile(FileHandle, Buffer, Length, &Length, NULL)) {
		if (Result) {
			*Result = Length;
		}
	}
	else {
		err = GetLastError();

		VDKTRACE(VDKWRITE,("[VDK] WriteFile - %s\n", VdkStatusStr(err)));
	}

	return err;
}

//
//	read from file at offset
//
VDKSTAT VdkReadFileAt(
	HANDLE			FileHandle,
	INT64			Offset,
	PVOID			Buffer,
	ULONG			Length,
	PULONG_PTR		Result)
{
	DWORD err = ERROR_SUCCESS;

	if (Offset != -1) {
		err = VdkSeekFile(FileHandle, Offset);

		if (err != ERROR_SUCCESS) {
			return err;
		}
	}

	if (ReadFile(FileHandle, Buffer, Length, &Length, NULL)) {
		if (Result) {
			*Result = Length;
		}
	}
	else {
		err = GetLastError();

		VDKTRACE(VDKREAD,("[VDK] ReadFile - %s\n", VdkStatusStr(err)));
	}

	return err;
}

//
// Adjust file size
//

VDKSTAT VdkSetFileSize(
	HANDLE	FileHandle,
	INT64	FileSize)
{
	DWORD err;

	err = VdkSeekFile(
		FileHandle, FileSize);

	if (err != ERROR_SUCCESS) {
		return err;
	}

	if (!SetEndOfFile(FileHandle)) {
		err = GetLastError();

		VDKTRACE(0,("[VDK] SetEndOfFile - %s\n", VdkStatusStr(err)));
		return err;
	}

	return ERROR_SUCCESS;
}

//
//	get status string
//
const char *VdkStatusStr(
	VDKSTAT			Status)
{
	static char buf[512];

	DWORD len = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, Status, 0, buf, sizeof(buf), NULL);

	while (len && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
		buf[--len] = '\0';
	}

	return buf;
}

//
//	print message to stdout
//
void PrintMessage(DWORD msg,...)
{
	va_list args;
	LPTSTR	buf = NULL;

	va_start(args, msg);

	FormatMessage(
		FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, msg, 0, (LPTSTR)&buf, 0, &args);

	va_end(args);

	if (buf) {
		printf("%s", buf);
		LocalFree(buf);
	}
}

//
//	Display prompt and accept answer
//
int InputChar(DWORD prompt, const char *accept)
{
	int c;
	LPTSTR	buf = NULL;

	FormatMessage(
		FORMAT_MESSAGE_FROM_HMODULE |
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, prompt, 0, (LPTSTR)&buf, 0, NULL);

	do {
		if (buf) {
			printf("%s", buf);
		}
		else {
			printf(accept);
		}
		fflush(stdout);

		fflush(stdin);
		c = tolower(getchar());
	}
	while (!strchr(accept, c));

	if (buf) {
		LocalFree(buf);
	}

	return c;
}

#ifdef VDK_DEBUG

//
//	Debug trace function
//
extern unsigned long	TraceFlags	= 0;
extern const char		*TraceFile	= 0;
extern unsigned long	TraceLine	= 0;

void VdkTrace(const char *format,...)
{
	char buf1[512], buf2[512];
	va_list args;

	va_start(args, format);

	_vsnprintf(buf1, sizeof(buf1), format, args);

	va_end(args);

	_snprintf(buf2, sizeof(buf2), "%s(%lu): %s",
		TraceFile, TraceLine, buf1);

	OutputDebugString(buf2);
}

#endif	// VDK_DEBUG
