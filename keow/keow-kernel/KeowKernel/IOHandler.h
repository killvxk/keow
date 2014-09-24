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

// IOHandler.h: interface for the IOHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOHANDLER_H__BDA9DB15_5F09_45C4_9607_507D02C38ACD__INCLUDED_)
#define AFX_IOHANDLER_H__BDA9DB15_5F09_45C4_9607_507D02C38ACD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class IOHandler  
{
public:
	int GetUnixFileType();
	IOHandler();
	virtual ~IOHandler();

	static IOHandler* CreateForPath(Path& path);

	virtual bool Open(DWORD win32access, DWORD win32share, DWORD disposition, DWORD flags) = 0;
	virtual bool Close() = 0;
	virtual DWORD ioctl(DWORD request, DWORD data) = 0;

	virtual bool Read(void* address, DWORD size, DWORD *pRead) = 0;
	virtual bool Write(void* address, DWORD size, DWORD *pWritten) = 0;

	virtual __int64 Length() = 0;
	virtual __int64 Seek(__int64 offset, DWORD method) = 0;
	virtual void Truncate() = 0;

	virtual IOHandler* Duplicate() = 0;

	bool IOHandler::Stat(linux::stat* s);
	void IOHandler::BasicStat64(linux::stat64 * s, int file_type);
	virtual bool IOHandler::Stat64(linux::stat64 * s) = 0;

	virtual int GetDirEnts64(linux::dirent64 *de, int maxbytes);

	virtual bool CanRead() = 0;
	virtual bool CanWrite() = 0;
	virtual bool HasException() = 0;

	bool GetInheritable()
	{
		return m_bInheritable;
	}
	void SetInheritable(bool inherit)
	{
		m_bInheritable = inherit;
	}

	DWORD GetFlags()
	{
		return m_Flags;
	}
	void SetFlags(DWORD flags)
	{
		m_Flags = flags;
	}

protected:
	bool m_bInheritable;
	DWORD m_Flags;
	Process * m_pInProcess;
};

#endif // !defined(AFX_IOHANDLER_H__BDA9DB15_5F09_45C4_9607_507D02C38ACD__INCLUDED_)
