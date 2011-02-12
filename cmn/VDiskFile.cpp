/*
	VDiskFile.cpp

	VDisk text file operation class
	Copyright (C) 2003 Ken Kato
*/

#include "vdkbase.h"
#include "vdkutil.h"
#include "VDiskFile.h"

#define BUFFER_SIZE	0x1000L	// 4KB

VDiskFile::VDiskFile()
{
	m_hFile		= INVALID_HANDLE_VALUE;
	m_pBuffer	= NULL;
	m_pCurrent	= NULL;
	m_nDataLen	= 0;
}

VDiskFile::~VDiskFile()
{
	if (m_hFile) {
		VdkCloseFile(m_hFile);
	}
	if (m_pBuffer) {
		VdkFreeMem(m_pBuffer);
	}
}

VDKSTAT VDiskFile::Open(PCHAR pPath)
{
	if (m_hFile != INVALID_HANDLE_VALUE) {
		return VDK_FUNCTION;
	}

	return VdkOpenFile(&m_hFile, pPath, strlen(pPath), TRUE);
}

VDKSTAT VDiskFile::Close()
{
	if (m_pBuffer) {
		VdkFreeMem(m_pBuffer);
		m_pBuffer	= NULL;
	}

	if (m_hFile != INVALID_HANDLE_VALUE) {
		VdkCloseFile(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	m_pCurrent	= NULL;
	m_nDataLen	= 0;

	return VDK_OK;
}

VDKSTAT VDiskFile::ReadByte(PUCHAR pBuffer, ULONG nLength, PULONG_PTR pResult)
{
	return VdkReadFileAt(m_hFile, -1, pBuffer, nLength, pResult);
}

//
//	Read one text line
//
VDKSTAT VDiskFile::ReadText(PCHAR pBuffer, ULONG nLength, PULONG_PTR pResult)
{
	VDKSTAT	ret = VDK_OK;
	ULONG	result = 0;

	//
	//	prepare read buffer
	//
	if (m_pBuffer == NULL) {
		if ((m_pBuffer = (PCHAR)VdkAllocMem(BUFFER_SIZE)) == NULL) {
			return VdkLastError();
		}
		m_pCurrent = m_pBuffer;
		m_nDataLen = 0;
	}

	while (result < nLength - 1) {

		if (m_nDataLen == 0) {

			//
			//	no data left in the buffer -- read file
			//
			m_pCurrent = m_pBuffer;

			ret = VdkReadFileAt(m_hFile, -1, m_pBuffer, BUFFER_SIZE, &m_nDataLen);

			if (ret != VDK_OK || m_nDataLen == 0) {
				break;
			}
		}

		if (*m_pCurrent == '\r' || *m_pCurrent == '\n') {

			//	encountered a newline

			while (m_nDataLen && (*m_pCurrent == '\r' || *m_pCurrent == '\n')) {
				m_pCurrent++;
				m_nDataLen--;
			}

			if (result == 0) {

				//	blank line
				continue;
			}
			else {

				//	one line completed
				break;
			}
		}
		else if (*m_pCurrent == '\0') {

			//	encountered a null character - treat as EOF
			break;
		}

		*(pBuffer++) = *(m_pCurrent++);
		m_nDataLen--;
		result++;
	}

	//	terminate the text string

	*pBuffer = '\0';

	//	store result length

	if (pResult) {
		*pResult = result;
	}

	//	read any data?

	if (result == 0) {
		ret = VDK_EOF;
	}

	return ret;
}
