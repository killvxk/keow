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

// KernelTable.h: interface for the KernelTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KERNELTABLE_H__232A87EB_0936_4CC2_8C52_5412CEC21F68__INCLUDED_)
#define AFX_KERNELTABLE_H__232A87EB_0936_4CC2_8C52_5412CEC21F68__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Process.h"
#include "IOHFile.h"
#include "MountPoint.h"
#include "Path.h"
#include "DevConsole.h"


class KernelTable  
{
public:
	KernelTable();
	virtual ~KernelTable();

	Process * FindProcess(PID pid);

	SYSTEMTIME m_BootTime;
	DWORD m_BogoMips;
	DWORD m_ForksSinceBoot;

	string m_KernelVersion;
	string m_KernelCpuType;

	int m_DebugLevel;
	HANDLE m_hLogFile;

	string m_KeowExeDir;
	string m_FilesystemRoot;
	MountPoint * m_pRootMountPoint;

	int m_LastPID;

	typedef list<Process*> ProcessList;
	ProcessList m_Processes;

	typedef list<MountPoint*> MountPointList;
	MountPointList m_MountPoints;

	typedef list<Device*> DeviceList;
	DeviceList m_Devices;

	DevConsole * m_pMainConsole;
};



//refs to the global single copies
extern KernelTable * g_pKernelTable;

static const int KTRACE_BUFFER_SIZE = 32000;
extern _declspec(thread) char * g_pTraceBuffer;
extern _declspec(thread) Process * P;
extern _declspec(thread) ThreadInfo * T;


#endif // !defined(AFX_KERNELTABLE_H__232A87EB_0936_4CC2_8C52_5412CEC21F68__INCLUDED_)
