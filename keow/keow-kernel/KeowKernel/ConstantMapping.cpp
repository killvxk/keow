/*
 * Copyright 2005 Paul Walker
 *
 * GNU General Public License
 * 
 * This file is part of: Kernel Emulation on Windows (keow)
 *
 * Keow is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Keow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Keow; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// ConstantMapping.cpp: implementation of the ConstantMapping class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "ConstantMapping.h"

//////////////////////////////////////////////////////////////////////

DWORD ElfProtectionToWin32Protection(linux::Elf32_Word prot)
{
	DWORD win32prot;
	win32prot = 0;
	if(prot == (linux::PF_R) )
		win32prot = PAGE_READONLY;
	else
	if(prot == (linux::PF_W)
	|| prot == (linux::PF_W|linux::PF_R) )
		win32prot = PAGE_READWRITE;
	else
	if(prot == (linux::PF_X) )
		//NOT honoring this competely, should be PAGE_EXECUTE but this is better for debug
		win32prot = PAGE_EXECUTE_READ; 
	else
	if(prot == (linux::PF_X|linux::PF_R) )
		win32prot = PAGE_EXECUTE_READ;
	else
	if(prot == (linux::PF_W|linux::PF_X)
	|| prot == (linux::PF_W|linux::PF_X|linux::PF_R) )
		win32prot = PAGE_EXECUTE_READWRITE;
	else
	{
		win32prot = PAGE_EXECUTE_READWRITE;
		ktrace("unhandled protection 0x%d, loading page R+W+X\n");
	}
	return win32prot;
}


/*
 * translate win32 gtlasterror value to linux errno
 */
int Win32ErrToUnixError(DWORD err)
{
	switch(err)
	{
	case ERROR_SUCCESS:
		return 0;

	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
		return linux::ENOENT;

	case ERROR_TOO_MANY_OPEN_FILES:
		return linux::EMFILE;

	case ERROR_ACCESS_DENIED:
		return linux::EACCES;

	case ERROR_INVALID_HANDLE:
		return linux::EBADF;

	case ERROR_ARENA_TRASHED:
	case ERROR_INVALID_BLOCK:
		return linux::EFAULT;

	case ERROR_NOT_ENOUGH_MEMORY:
	case ERROR_OUTOFMEMORY:
		return linux::ENOMEM;

	case ERROR_INVALID_FUNCTION:
		return linux::ENOSYS;

	case ERROR_BROKEN_PIPE:
	case ERROR_NO_DATA:
	case ERROR_PIPE_NOT_CONNECTED:
		return linux::EIO;

	case ERROR_BAD_FORMAT:
		return linux::ENOEXEC;

	default:
		ktrace("Unhandled Win32 Error code %ld\n", err);
		return linux::EPERM; //generic
	}
}
