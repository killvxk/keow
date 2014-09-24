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

// IOHStaticData.cpp: implementation of the IOHStaticData class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "IOHStaticData.h"

//////////////////////////////////////////////////////////////////////

IOHStaticData::IOHStaticData(Path& path, DataType type, bool refreshable)
{
	m_pData = NULL;
	m_DataLength = 0;
	m_nFilePointer = 0;
	m_Path = path;
	m_DataType = type;
	m_bRefreshable = refreshable;
}

IOHStaticData::~IOHStaticData()
{
	delete m_pData;
}


void IOHStaticData::AddData(const char * text)
{
	int len = strlen(text);
	AddData((LPBYTE)text, len);
}

void IOHStaticData::AddData(void * pData, int length)
{
	if(length<=0)
		return;

	BYTE * p2 = new BYTE[m_DataLength+length];
	memcpy(p2, m_pData, m_DataLength);
	memcpy(p2+m_DataLength, pData, length);

	m_DataLength += length;
	delete m_pData;
	m_pData = p2;
}

void IOHStaticData::AddData(linux::dirent64 de)
{
	AddData(&de, sizeof(de));
}


IOHandler * IOHStaticData::Duplicate()
{
	IOHStaticData * p2 = new IOHStaticData(m_Path, m_DataType, m_bRefreshable);

	p2->m_pData = new BYTE[m_DataLength];
	memcpy(p2->m_pData, m_pData, m_DataLength);

	p2->m_DataLength = m_DataLength;
	p2->m_nFilePointer = m_nFilePointer;

	return p2;
}

bool IOHStaticData::Open(DWORD win32access, DWORD win32share, DWORD disposition, DWORD flags)
{
	//reset to start
	m_nFilePointer = 0;
	return true;
}

bool IOHStaticData::Close()
{
	//nothing to do
	return true;
}


bool IOHStaticData::Stat64(linux::stat64 * s)
{
	if(!s)
		return false;

	IOHandler::BasicStat64(s, 0); //fifo - aha pipe

	//make it all read only
	s->st_mode = 0444;  // r--r--r--

	if(m_DataType==File)
		s->st_mode |= linux::S_IFREG; //regular file
	else
		s->st_mode |= linux::S_IFDIR; //directory

	s->st_nlink = 1;//fi.nNumberOfLinks;
	s->st_uid = 0;
	s->st_gid = 0;


	s->st_dev = s->st_rdev = 0;

	s->st_ino = s->__st_ino = (DWORD)this; //dummy


	s->st_size = Length();

	s->st_blksize = 512; //block size for efficient IO
	
	s->st_blocks = (unsigned long)((s->st_size+511) / 512); //size in 512 byte blocks

	SYSTEMTIME st;
	FILETIME ft;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st,&ft);
	s->st_atime = FILETIME_TO_TIME_T(ft);
	s->st_mtime = FILETIME_TO_TIME_T(ft);
	s->st_ctime = FILETIME_TO_TIME_T(ft);

	return true;
}


DWORD IOHStaticData::ioctl(DWORD request, DWORD data)
{
	DWORD dwRet;

	switch(request)
	{
	case 0:
	default:
		ktrace("IMPLEMENT sys_ioctl 0x%lx for IOHStaticData\n", request);
		dwRet = -linux::ENOSYS;
		break;
	}

	return dwRet;
}

int IOHStaticData::GetDirEnts64(linux::dirent64 *de, int maxbytes)
{
	int filled = 0;
	while(filled+(int)sizeof(linux::dirent64) < maxbytes
	&&    m_nFilePointer < m_DataLength)
	{
		memcpy(de, &m_pData[m_nFilePointer], sizeof(linux::dirent64));

		de->d_reclen = sizeof(linux::dirent64);
		de->d_off = m_nFilePointer;
		de->d_ino = (DWORD)this + m_nFilePointer; //dummy

		filled += de->d_reclen;
		m_nFilePointer += de->d_reclen;
		++de;
	}
	return filled;
}

bool IOHStaticData::Read(void* address, DWORD size, DWORD *pRead)
{
	RefreshData();

	int DataLeft = m_DataLength - m_nFilePointer;
	if(DataLeft <= 0)
	{
		//EOF
		*pRead = 0;
		return true;
	}

	if(size > (DWORD)DataLeft)
		size = DataLeft;

	//read
	P->WriteMemory((ADDR)address, size, &m_pData[m_nFilePointer]);
	m_nFilePointer += size;
	*pRead = size;
	return true;
}


bool IOHStaticData::Write(void* address, DWORD size, DWORD *pWritten)
{
	//write not supported for static data
	*pWritten=0;
	return false;
}


bool IOHStaticData::CanRead()
{
	//ok if we are not at eof
	return m_nFilePointer < m_DataLength;
}

bool IOHStaticData::CanWrite()
{
	//never for static data
	return false;
}

bool IOHStaticData::HasException()
{
	//TODO: what could this be?
	return false;
}

__int64 IOHStaticData::Length()
{
	return m_DataLength;
}

__int64 IOHStaticData::Seek(__int64 offset, DWORD method)
{
	switch(method)
	{
	case FILE_BEGIN:
		m_nFilePointer = (DWORD)offset;
		break;
	case FILE_END:
		m_nFilePointer = m_DataLength-1 - (DWORD)offset;
		break;
	case FILE_CURRENT:
		m_nFilePointer += (DWORD)offset;
		break;
	}
	
	return m_nFilePointer;
}

void IOHStaticData::Truncate()
{
	//do nothing - can truncate static data
}

void IOHStaticData::RefreshData()
{
	if(!m_bRefreshable)
		return;

	//refresh the data if we can
	IOHandler * io2 = IOHandler::CreateForPath(m_Path);
	IOHStaticData * sd2 = dynamic_cast<IOHStaticData*>(io2);
	if(sd2!=NULL)
	{
		delete m_pData;
		m_pData = new BYTE[sd2->m_DataLength];
		memcpy(m_pData, sd2->m_pData, sd2->m_DataLength);
		m_DataLength = sd2->m_DataLength;
	}
	delete io2;
}
