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

// IOHandler.cpp: implementation of the IOHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "IOHandler.h"

//////////////////////////////////////////////////////////////////////


IOHandler::IOHandler()
{
	m_bInheritable = true; //default
	m_Flags = 0;
	m_pInProcess = P; //process this is in (used for cloning)
}

IOHandler::~IOHandler()
{

}


IOHandler* IOHandler::CreateForPath(Path& path)
{
	Filesystem * fs = path.GetFinalFilesystem();

	return fs->CreateIOHandler(path);
}


/*
 * can be generic - relies on virtual Stat64
 */
bool IOHandler::Stat(linux::stat* s)
{
	linux::stat64 s64;

	bool ret = this->Stat64(&s64);

	s->st_atime = s64.st_atime;
	s->st_ctime = s64.st_ctime;
	s->st_mtime = s64.st_mtime;

	s->st_dev = s64.st_dev;
	s->st_rdev = s64.st_rdev;
	s->st_ino = (unsigned long)s64.st_ino;

	s->st_mode = s64.st_mode;
	s->st_nlink = s64.st_nlink;

	s->st_uid = (unsigned short)s64.st_uid;
	s->st_gid = (unsigned short)s64.st_gid;

	s->st_size = (unsigned long)s64.st_size > (unsigned long)-1 ? (unsigned long)-1 : (unsigned long)s64.st_size;
	s->st_blksize = s64.st_blksize;
	s->st_blocks = s64.st_blocks;


	s->__pad1 = s->__pad2 = 0;

	return ret;
}


/*
 * helper - minimal Stat64
 */
void IOHandler::BasicStat64(linux::stat64 * s, int file_type)
{
	//pading fields are very sensisitive to contents?!

	//memset(s, 0, sizeof(struct linux::stat64));
	//memset(s->__pad0, 0, sizeof(s->__pad0));
	//memset(s->__pad3, 0, sizeof(s->__pad3));
	s->__pad4 = s->__pad5 = s->__pad6 = 0;


	//basic details

	s->st_mode = 0755;  // rwxrwxrwx
	s->st_mode |= file_type;

	s->st_nlink = 1;
	s->st_uid = 0;
	s->st_gid = 0;

	s->st_dev = 0;	//dummy
	s->st_rdev = 0;

	s->st_ino = 0;    //dummy inode
	s->__st_ino = 0;

	s->st_size = 0;

	s->st_blksize = 512; //block size for efficient IO
	
	s->st_blocks = 0;

	s->st_atime = 0;//FILETIME_TO_TIME_T(fi.ftLastAccessTime);
	s->st_mtime = 0;//FILETIME_TO_TIME_T(fi.ftLastWriteTime);
	s->st_ctime = 0;//FILETIME_TO_TIME_T(fi.ftCreationTime);
}


int IOHandler::GetDirEnts64(linux::dirent64 *de, int maxbytes)
{
	//none - override to get more
	return 0;
}

