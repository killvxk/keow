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

// KernelTable.cpp: implementation of the KernelTable class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "KernelTable.h"
#include "IOHNtConsole.h"
#include "SysCalls.h"

//////////////////////////////////////////////////////////////////////

KernelTable::KernelTable()
{
	ktrace("Init kernel tables\n");

	m_DebugLevel = 1;	//not zero - because we want to keep logging before arguments are finished processing
	m_hLogFile = NULL;
	m_pRootMountPoint = 0;
	m_LastPID = 0;


	//Original
//	m_KernelVersion = "2.4.20"; //specs we are using are this (Sep 2005)
//	m_KernelCpuType = "i386"; //use miniumum value for now?

	//Updated to run Ubuntu Lucid bootstrap image
	m_KernelVersion = "2.6.35";
	m_KernelCpuType = "i386";


	char * dir = m_KeowExeDir.GetBuffer(MAX_PATH);
	GetModuleFileName(NULL, dir, MAX_PATH);
	char * slash = strrchr(dir, '\\');
	if(slash)
		*slash=NULL;
	m_KeowExeDir.ReleaseBuffer();

	GetSystemTime(&m_BootTime);
	m_ForksSinceBoot=0;

	//simple - probably doesn't match the kernel
	m_BogoMips = 0;
	const DWORD msSample = 1000;
	DWORD dwEnd = GetTickCount() + msSample;
	while(GetTickCount() < dwEnd) {
		Sleep(0);
		m_BogoMips++;
	}
	m_BogoMips /= msSample;
	ktrace("keow bogomips: %ld\n", m_BogoMips);

	SysCalls::InitSysCallTable();

	//main console
	m_pMainConsole = new DevConsole(0);

	ktrace("Kernel table init done\n");
}

KernelTable::~KernelTable()
{

}



//Search for and return the process for a pid
//
Process * KernelTable::FindProcess(PID pid)
{
	//TODO: more efficient search

	KernelTable::ProcessList::iterator it;
	for(it=g_pKernelTable->m_Processes.begin(); it!=g_pKernelTable->m_Processes.end(); ++it)
	{
		if((*it)->m_Pid == pid)
			return *it;
	}

	return NULL;
}
