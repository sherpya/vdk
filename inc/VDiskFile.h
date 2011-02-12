/*
	VDiskFile.h

	VDisk file operation class header
	Copyright (C) 2003 Ken Kato
*/

#ifndef _VDISKFILE_H_
#define _VDISKFILE_H_

class VDiskFile
{
public:
	VDiskFile();
	~VDiskFile();

	VDKSTAT Open(PCHAR pPath);
	VDKSTAT Close();
	VDKSTAT ReadByte(PUCHAR pBuffer, ULONG nLength, PULONG_PTR pResult);
	VDKSTAT ReadText(PCHAR pBuffer, ULONG nLength, PULONG_PTR pResult);

	HANDLE	Handle()	{ return m_hFile; }
	void	Reset()		{ m_pCurrent = m_pBuffer; m_nDataLen = 0; }

protected:
	HANDLE	m_hFile;
	PCHAR	m_pBuffer;
	PCHAR	m_pCurrent;
	ULONG_PTR m_nDataLen;
};

#endif	// _VDISKFILE_H_
