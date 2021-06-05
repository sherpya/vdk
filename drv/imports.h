/*
	imports.h

	Imported elements from various sources
	Copyright (C) 2003 Ken Kato

	This file contains:

	a) 	Stuff imported from newer DDKs so that the driver for all versions
		of Windows can be compiled with the Windows NT 4.0 DDK.

	b) 	Prototypes of standard functions which are exported from ntoskrnl.exe
		but not declared in regular DDK header files.
*/

#ifndef	_IMPORTS_H_
#define _IMPORTS_H_

#include <ntdddisk.h>
#include <mountdev.h>
#include <ntddvol.h>
#include <wdm.h>

//
// Functions exported by ntoskrnl.exe
//
int swprintf(wchar_t *, const wchar_t *, ...);

#endif	// _IMPORTS_H_
