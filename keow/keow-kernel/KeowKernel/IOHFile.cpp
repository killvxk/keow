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

// File.cpp: implementation of the File class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "IOHFile.h"

//////////////////////////////////////////////////////////////////////

IOHFile::IOHFile(Path &path)
{
	m_RemoteHandle = INVALID_HANDLE_VALUE;
	m_Path = path;
	m_bIsADirectory = false;
	m_hFindData = INVALID_HANDLE_VALUE;

	//don't open yet, use Open()
}

IOHFile::~IOHFile()
{
	Close();
}

IOHandler * IOHFile::Duplicate()
{
	IOHFile * pF = new IOHFile(m_Path);

	DuplicateHandle(m_pInProcess->m_hProcess, m_RemoteHandle,
		P->m_hProcess, &pF->m_RemoteHandle,
		0,0, DUPLICATE_SAME_ACCESS);

	pF->m_bIsADirectory = m_bIsADirectory;

	if(m_hFindData!=INVALID_HANDLE_VALUE)
	{
		//Can't dup a find handle, so recreate the find

		linux::dirent64 tmp;
		pF->GetDirEnts64(&tmp, sizeof(tmp));
		pF->m_nFindCount = 0;
		while(pF->m_nFindCount < m_nFindCount)
			pF->GetDirEnts64(&tmp, sizeof(tmp));

	}

	return pF;
}

bool IOHFile::Open(DWORD win32access, DWORD win32share, DWORD disposition, DWORD flags)
{
	Close();


	DWORD attr = GetFileAttributes(m_Path.GetWin32Path());
	if((attr!=INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		m_bIsADirectory = true;
		flags = FILE_FLAG_BACKUP_SEMANTICS;
	}


	//Open in the kernel, then move the handle to the user

	HANDLE h = CreateFile(m_Path.GetWin32Path(), win32access, win32share, 0, disposition, flags, 0);
	if(h==INVALID_HANDLE_VALUE)
		return false;

	//move handle to the user
	DuplicateHandle(GetCurrentProcess(), h, P->m_hProcess, &m_RemoteHandle, 0, FALSE, DUPLICATE_SAME_ACCESS|DUPLICATE_CLOSE_SOURCE);

	return true;
}

bool IOHFile::Close()
{
	if(m_hFindData != INVALID_HANDLE_VALUE)
		FindClose(m_hFindData);
	m_hFindData = INVALID_HANDLE_VALUE;

	if(m_RemoteHandle != INVALID_HANDLE_VALUE)
		SysCallDll::CloseHandle(m_RemoteHandle);
	m_RemoteHandle = INVALID_HANDLE_VALUE;

	return true;
}


bool IOHFile::Stat64(linux::stat64 * s)
{
	if(!s)
		return false;


	WIN32_FILE_ATTRIBUTE_DATA fi;
	ULARGE_INTEGER i;


	IOHandler::BasicStat64(s, 0);

	string w32path = m_Path.GetWin32Path();

	//Win95 does not have GetFileAttributesEx, may need to emulate it
	if(!LegacyWindows::GetFileAttributesEx(w32path, GetFileExInfoStandard, &fi))
			return false;


	if(fi.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		s->st_mode = 0555;  // r-xr-xr-x
	else
		s->st_mode = 0755;  // rwxr-xr-x

	if(m_Path.IsSymbolicLink())
		s->st_mode |= linux::S_IFLNK;
	else
	if(fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		s->st_mode |= linux::S_IFDIR;
	else
		s->st_mode |= linux::S_IFREG; //regular file
		/*
	#define S_IFMT  00170000
	#define S_IFSOCK 0140000
	#define S_IFLNK	 0120000
	#define S_IFREG  0100000
	#define S_IFBLK  0060000
	#define S_IFDIR  0040000
	#define S_IFCHR  0020000
	#define S_IFIFO  0010000
	#define S_ISUID  0004000
	#define S_ISGID  0002000
	#define S_ISVTX  0001000
		*/


	s->st_nlink = 1;//fi.nNumberOfLinks;
	s->st_uid = 0;
	s->st_gid = 0;

	//use major '3' and minor as drive letter
	s->st_dev = s->st_rdev = (3<<8) | (BYTE)(w32path[0] - 'A');

	//use hash of filename as inode
	s->st_ino = s->__st_ino = w32path.hash();


	i.LowPart = fi.nFileSizeLow;
	i.HighPart = fi.nFileSizeHigh;
	s->st_size = i.QuadPart;

	s->st_blksize = 512; //block size for efficient IO
	
	s->st_blocks = (unsigned long)((s->st_size+511) / 512); //size in 512 byte blocks

	s->st_atime = FILETIME_TO_TIME_T(fi.ftLastAccessTime);
	s->st_mtime = FILETIME_TO_TIME_T(fi.ftLastWriteTime);
	s->st_ctime = FILETIME_TO_TIME_T(fi.ftCreationTime);


	return true;
}

__int64 IOHFile::Length()
{
	linux::stat64 s;
	Stat64(&s);
	return s.st_size;
}

__int64 IOHFile::Seek(__int64 pos, DWORD from)
{
	LARGE_INTEGER li;
	li.QuadPart = pos;
	li.LowPart = SysCallDll::SetFilePointer(m_RemoteHandle, li.LowPart, li.HighPart, from);
	return li.QuadPart;
}

void IOHFile::Truncate()
{
	//truncate at current location
	SysCallDll::SetEndOfFile(m_RemoteHandle);
}


DWORD IOHFile::ioctl(DWORD request, DWORD data)
{
	switch(request)
	{
		/*
	case TCGETS:
		{
			linux::termios * arg = (linux::termios*)pCtx->Edx;
			arg->c_iflag = 0;		/* input mode flags *
			arg->c_oflag = 0;		/* output mode flags *
			arg->c_cflag = 0;		/* control mode flags *
			arg->c_lflag = 0;		/* local mode flags *
			arg->c_line = 0;			/* line discipline *
			//arg->c_cc[NCCS];		/* control characters *
			return 0;
		}
		break;
		*/
	case 0:
	default:
		ktrace("IMPLEMENT sys_ioctl 0x%lx for IOHFile\n", request);
		return -linux::ENOSYS;
	}
}


int IOHFile::GetDirEnts64(linux::dirent64 *de, int maxbytes)
{
	DWORD err = 0;
	WIN32_FIND_DATA wfd;
	Path p(false);

	int filled = 0;
	while(filled+(int)sizeof(linux::dirent64) < maxbytes)
	{
		if(m_hFindData==INVALID_HANDLE_VALUE)
		{
			//need to start the search

			char DirPattern[MAX_PATH];
			StringCbPrintf(DirPattern, sizeof(DirPattern), "%s/*.*", m_Path.GetWin32Path());

			m_hFindData = FindFirstFile(DirPattern, &wfd);
			//can't inherit handle, so set count of 'find' so child processes can recreate our state
			m_nFindCount = 0;

			if(m_hFindData==INVALID_HANDLE_VALUE)
				return -1;
		}
		else
		{
			m_nFindCount++;
			if(!FindNextFile(m_hFindData, &wfd))
			{
				err = GetLastError();
				if(err==ERROR_NO_MORE_FILES)
				{
					//only close if returning no results - otherwise leave open
					//for the next call to detect end of data.
					if(filled==0)
					{
						FindClose(m_hFindData);
						m_hFindData = INVALID_HANDLE_VALUE;
					}
					err = 0;
				}
				break;
			}
		}

		p = m_Path;
		p.AppendUnixPath(wfd.cFileName);

		de->d_ino = p.GetUnixPath().hash(); //dummy value

		de->d_type = 0; //not provided on linux x86 32bit?  (GetUnixFileType(p);

		StringCbCopy(de->d_name, sizeof(de->d_name), wfd.cFileName);

		if(p.IsSymbolicLink())
		{
			//ensure name we return does not end in .lnk (we use windows shortcuts as symbolic links)
 			int e = strlen(de->d_name) - 4;
			if(e>0 && _stricmp(&de->d_name[e], ".lnk")==0)
				de->d_name[e] = 0;
		}


		//de->d_reclen = sizeof(linux::dirent64);
		de->d_reclen = sizeof(linux::dirent64)-sizeof(de->d_name)+strlen(de->d_name)+1;

		de->d_off = (m_nFindCount-1); //offset in dir, not memory

		filled += de->d_reclen;

		de = (linux::dirent64*)(((LPBYTE)de) + de->d_reclen); //move pointer along
	}

	if(err!=0)
	{
		filled = -1;
		SetLastError(err);
	}
	return filled;
}


bool IOHFile::Read(void* address, DWORD size, DWORD *pRead)
{
	//read
	*pRead = SysCallDll::ReadFile(m_RemoteHandle, address, size);
	if(*pRead==0)
	{
		if(SysCallDll::GetLastError()==0)
		{
			//EOF
			return true;
		}

		//failed read
		return false;
	}
	return true;
}


bool IOHFile::Write(void* address, DWORD size, DWORD *pWritten)
{
	*pWritten = SysCallDll::WriteFile(m_RemoteHandle, address, size);
	return *pWritten!=0;
}


bool IOHFile::CanRead()
{
	//ok if we are not at eof
	return SysCallDll::GetFilePointer(m_RemoteHandle) < Length();
}

bool IOHFile::CanWrite()
{
	//always except to be able to write to the File?
	return true;
}

bool IOHFile::HasException()
{
	//TODO: what could this be?
	return false;
}
