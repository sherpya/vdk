/*
	vdkntddk.c

	VDK Kernel mode driver utility routines
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"

//
//	Opens an image file
//
VDKSTAT
VdkOpenFile(
	IN HANDLE	*FileHandle,
	IN PCHAR	FileName,
	IN ULONG	NameLen,
	IN ULONG	ReadOnly)
{
	CHAR				buf[MAXIMUM_FILENAME_LENGTH];
	ULONG 				prefix;
	ANSI_STRING			ansi_name;
	UNICODE_STRING		unicode_name;
	OBJECT_ATTRIBUTES	attributes;
	IO_STATUS_BLOCK		io_status;
	VDKSTAT				status = VDK_OK;

	//
	// append appropriate prefix
	//
	prefix = 0;

	if (RtlCompareMemory(FileName, "\\??\\", 4) != 4) {

		RtlCopyMemory(buf, "\\??\\", 4);
		prefix = 4;

		if ((*FileName == '\\' || *FileName == '/') &&
			(*(FileName + 1) == '\\' || *(FileName + 1) == '/')) {
			//
			//	UNC path
			//
			RtlCopyMemory(buf + prefix, "UNC", 3);
			FileName++;
			NameLen--;
			prefix += 3;
		}
	}

	RtlCopyMemory(buf + prefix, FileName, NameLen);
	buf[NameLen + prefix] = '\0';

	VDKTRACE(VDKOPEN | VDKINFO,
		("[VDK] Opening file %s for %s\n",
			buf, ReadOnly ? "read-only" : "read-write"));

	//
	// generate unicode filename
	//
	RtlInitAnsiString(&ansi_name, buf);

	status = RtlAnsiStringToUnicodeString(
		&unicode_name, 	&ansi_name, TRUE);

	if (!VDKSUCCESS(status)) {

		VDKTRACE(VDKOPEN,
			("[VDK] Failed to convert filename to UNICODE\n"));

		return status;
	}

	//
	// open the file
	//
	InitializeObjectAttributes(
		&attributes,
		&unicode_name,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	status = ZwCreateFile(
		FileHandle,
		ReadOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
		&attributes,
		&io_status,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		ReadOnly ? FILE_SHARE_READ : 0,
		FILE_OPEN,
		FILE_NON_DIRECTORY_FILE |
		FILE_RANDOM_ACCESS |
		FILE_NO_INTERMEDIATE_BUFFERING |
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	RtlFreeUnicodeString(&unicode_name);

	if (!VDKSUCCESS(status)) {
		VDKTRACE(VDKOPEN,
			("[VDK] ZwCreateFile - %s\n",
			VdkStatusStr(status)));

		*FileHandle = NULL;
	}

	return status;
}

//
//	Check file attribute
//
VDKSTAT VdkCheckAttribute(
	HANDLE			FileHandle)
{
	IO_STATUS_BLOCK	io_status;
	VDKSTAT			status;

	FILE_BASIC_INFORMATION		file_basic;

	//
	// The NT cache manager can deadlock if a filesystem that is using
	// the cache manager is used in a virtual disk that stores its file
	// on a filesystem that is also using the cache manager, this is why
	// we open the file with FILE_NO_INTERMEDIATE_BUFFERING above, however
	// if the file is compressed or encrypted NT will not honor this
	// request and cache it anyway since it need to store the
	// decompressed/unencrypted data somewhere, therefor we put an extra
	// check here and don't alow disk images to be compressed/encrypted.
	//
	status = ZwQueryInformationFile(
		FileHandle,
		&io_status,
		&file_basic,
		sizeof(FILE_BASIC_INFORMATION),
		FileBasicInformation);

	if (!VDKSUCCESS(status)) {
		VDKTRACE(VDKOPEN,
			("[VDK] ZwQueryInformationFile - FILE_BASIC_INFORMATION %s\n",
			VdkStatusStr(status)));

		return status;
	}

	if (file_basic.FileAttributes & VDK_INVALID_ATTRIBUTES) {
		VDKTRACE(VDKOPEN,("[VDK] File is compressed and/or encrypted\n"));

		return STATUS_ACCESS_DENIED;
	}

	return STATUS_SUCCESS;
}

//
// Get actual file size
//
VDKSTAT	VdkGetFileSize(
	HANDLE	FileHandle,
	PINT64	FileSize)
{
	IO_STATUS_BLOCK	io_status;
	VDKSTAT			status;

	FILE_STANDARD_INFORMATION	file_standard;

	status = ZwQueryInformationFile(
		FileHandle,
		&io_status,
		&file_standard,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation);

	if (!VDKSUCCESS(status)) {
		VDKTRACE(VDKOPEN,
			("[VDK] ZwQueryInformationFile - FILE_STANDARD_INFORMATION %s\n",
			VdkStatusStr(status)));

		return status;
	}

	*FileSize = file_standard.EndOfFile.QuadPart;

	return status;
}

//
//	Change actual file size
//
VDKSTAT VdkSetFileSize(
	HANDLE	FileHandle,
	INT64	FileSize)
{
	FILE_END_OF_FILE_INFORMATION end_of_file;
	IO_STATUS_BLOCK	io_status;
	NTSTATUS		status;

	end_of_file.EndOfFile.QuadPart = FileSize;

	status = ZwSetInformationFile(
		FileHandle,
		&io_status,
		&end_of_file,
		sizeof(FILE_END_OF_FILE_INFORMATION),
		FileEndOfFileInformation);

	if (!NT_SUCCESS(status)) {
		VDKTRACE(VDKWRITE,
			("[VDK] ZwSetInformationFile %s\n",
			VdkStatusStr(status)));
	}

	return status;
}

//
//	Write to file at specified offset
//
VDKSTAT	VdkWriteFileAt(
	HANDLE	FileHandle,
	INT64	Offset,
	PVOID	Buffer,
	ULONG	Length,
	PULONG_PTR	Result)
{
	LARGE_INTEGER	loffset;
	IO_STATUS_BLOCK	io_status;
	NTSTATUS		status;

	loffset.QuadPart = Offset;

	status = ZwWriteFile(
		FileHandle,
		NULL,
		NULL,
		NULL,
		&io_status,
		Buffer,
		Length,
		&loffset,
		NULL);

	if (!NT_SUCCESS(status)) {
		VDKTRACE(VDKWRITE,
			("[VDK] ZwWriteFile %s\n", VdkStatusStr(status)));
	}

	if (Result) {
		*Result = io_status.Information;
	}

	return status;
}

//
//	Read from file at specified offset
//
VDKSTAT VdkReadFileAt(
	HANDLE	FileHandle,
	INT64	Offset,
	PVOID	Buffer,
	ULONG	Length,
	PULONG_PTR	Result)
{
	LARGE_INTEGER	loffset;
	IO_STATUS_BLOCK	io_status;
	NTSTATUS		status;

	loffset.QuadPart = Offset;

	status = ZwReadFile(
		FileHandle,
		NULL,
		NULL,
		NULL,
		&io_status,
		Buffer,
		Length,
		Offset == -1 ? NULL : &loffset,
		NULL);

	if (!NT_SUCCESS(status)) {
		VDKTRACE(VDKREAD,
			("[VDK] ZwReadFile %s\n", VdkStatusStr(status)));
	}

	if (Result) {
		*Result = io_status.Information;
	}

	return status;
}
