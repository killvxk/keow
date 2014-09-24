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

// File.h: interface for the File class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILE_H__5FA578F4_1C37_466C_BED7_3CE1E76298CB__INCLUDED_)
#define AFX_FILE_H__5FA578F4_1C37_466C_BED7_3CE1E76298CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOHandler.h"


class IOHFile : public IOHandler
{
public:
	IOHFile(Path &path);
	virtual ~IOHFile();

	Path& GetPath()
	{
		return m_Path;
	}

	HANDLE GetRemoteHandle()
	{
		return m_RemoteHandle;
	}

	virtual bool Open(DWORD win32access, DWORD win32share, DWORD disposition, DWORD flags);
	virtual bool Close();
	virtual DWORD ioctl(DWORD request, DWORD data);

	virtual bool Read(void* address, DWORD size, DWORD *pRead);
	virtual bool Write(void* address, DWORD size, DWORD *pWritten);

	virtual __int64 Length();
	virtual __int64 Seek(__int64 offset, DWORD method);
	virtual void Truncate();

	virtual IOHandler* Duplicate();

	virtual bool Stat64(linux::stat64 * s);

	virtual int GetDirEnts64(linux::dirent64 *de, int maxbytes);

	virtual bool CanRead();
	virtual bool CanWrite();
	virtual bool HasException();

protected:
	Path m_Path;
	HANDLE m_RemoteHandle;
	bool m_bIsADirectory;
	HANDLE m_hFindData;
	int m_nFindCount;
};

#endif // !defined(AFX_FILE_H__5FA578F4_1C37_466C_BED7_3CE1E76298CB__INCLUDED_)
