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

// FilesystemProc.cpp: implementation of the FilesystemProc class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "FilesystemProc.h"
#include "IOHStaticData.h"


const int FilesystemProc::Jiffies = 100; //??

//////////////////////////////////////////////////////////////////////

FilesystemProc::FilesystemProc()
: FilesystemGenericStatic("proc")
{
	AddLister("/", GetPids);
	AddLister("/*/fd/", GetOpenFiles);

	AddFile("/meminfo", Get_MemInfo);
	AddFile("/cpuinfo", Get_CpuInfo);
	AddFile("/uptime", Get_Uptime);
	AddFile("/stat", Get_Stat);
	AddFile("/mounts", Get_Mounts);
	AddFile("/loadavg", Get_LoadAvg);

	AddFile("/*/cmdline", Get_Pid_Cmdline);
	AddFile("/*/cwd", Get_Pid_Cwd);
	AddFile("/*/environ", Get_Pid_Environ);
	AddFile("/*/exe", Get_Pid_Exe);
	AddFile("/*/fd/*", Get_Pid_Fd);
	AddFile("/*/maps", Get_Pid_Maps);
	AddFile("/*/mem", Get_Pid_Mem);
	AddFile("/*/root", Get_Pid_Root);
	AddFile("/*/stat", Get_Pid_Stat);
	AddFile("/*/statm", Get_Pid_Statm);
	AddFile("/*/status", Get_Pid_Status);
}

FilesystemProc::~FilesystemProc()
{
}


Process* FilesystemProc::PidStrToProcess(const char * pid)
{
	Process * pp;
	if(strcmp(pid,"self")==0)
		pp = P;
	else
	{
		int n = atoi(pid);
		pp = g_pKernelTable->FindProcess(n);
	}

	return pp;
}

DWORD FilesystemProc::ToJiffies(FILETIME * ftBase, FILETIME * ftNow)
{
	ULARGE_INTEGER base, now;

	base.LowPart = ftBase->dwLowDateTime;
	base.HighPart = ftBase->dwHighDateTime;

	now.LowPart = ftNow->dwLowDateTime;
	now.HighPart = ftNow->dwHighDateTime;

	now.QuadPart -= base.QuadPart;

	//seconds to jiffies? don't know what this conversion should be?
	// filetime is 100-nanosecond intervals 
	// jiffies is 10ms intervals? (100Hz?)
	return (DWORD)(now.QuadPart / 1000000L);
}

void FilesystemProc::CalcCpuTimingsInJiffies(DWORD *uptime, DWORD *user, DWORD *system, DWORD *nice, DWORD *idle)
{
	//init
	*user = *system = *nice = *idle = 0;

	SYSTEMTIME stNow;
	FILETIME ftNow;
	GetSystemTime(&stNow);
	SystemTimeToFileTime(&stNow, &ftNow);

	FILETIME ftBoot;
	SystemTimeToFileTime(&g_pKernelTable->m_BootTime, &ftBoot);
	*uptime = ToJiffies(&ftBoot, &ftNow);

	//for now just report the kernel itself
	//we don't have these values yet
	//TODO: calc kernel user/sys/idle cpu metrics
	FILETIME ftZero;
	ftZero.dwLowDateTime = ftZero.dwHighDateTime = 0;
	FILETIME ftCreateTime, ftExitTime, ftKernelTime, ftUserTime;
	GetProcessTimes(GetCurrentProcess(), &ftCreateTime, &ftExitTime, &ftKernelTime, &ftUserTime);

	*system = ToJiffies(&ftZero, &ftKernelTime);
	*user = ToJiffies(&ftZero, &ftUserTime);

	*idle = *uptime - (*user + *system);
}

//////////////////////////////////////////////////////////////////

void FilesystemProc::GetPids(DirEnt64List& lst)
{
	linux::dirent64 de;

	de.d_ino = 0; //dummy value
	de.d_type = 0; //not provided on linux x86 32bit?  (GetUnixFileType(p);
	StringCbCopy(de.d_name, sizeof(de.d_name), "self");
	lst.push_back(de);

	KernelTable::ProcessList::iterator it;
	for(it=g_pKernelTable->m_Processes.begin();
	    it!=g_pKernelTable->m_Processes.end();
		++it)
	{
		de.d_ino = 0; //dummy value
		de.d_type = 0; //not provided on linux x86 32bit?  (GetUnixFileType(p);

		StringCbPrintf(de.d_name, sizeof(de.d_name), "%d", (*it)->m_Pid);
	
		lst.push_back(de);
	}
}

void FilesystemProc::GetOpenFiles(DirEnt64List& lst, const char * pid)
{
	Process * pp = PidStrToProcess(pid);
	if(pp==0)
		return;


	linux::dirent64 de;

	for(int i=0; i<MAX_OPEN_FILES; ++i)
	{
		if(pp->m_OpenFiles[i])
		{
			de.d_ino = 0; //dummy value
			de.d_type = 0; //not provided on linux x86 32bit?  (GetUnixFileType(p);

			StringCbPrintf(de.d_name, sizeof(de.d_name), "%d", i);
		
			lst.push_back(de);
		}
	}
}

//////////////////////////////////////////////////////////////////

IOHandler* FilesystemProc::Get_MemInfo(Path& path)
{
	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, true);

	MEMORYSTATUS ms;
	ms.dwLength = sizeof(ms);
	GlobalMemoryStatus(&ms);

	ioh->AddData( string::format("\t  total:  \t   used:  \t   free: \x0a") );

	ioh->AddData( string::format("Mem: \t%10ld \t%10ld \t%10ld \x0a", ms.dwTotalPhys, ms.dwTotalPhys-ms.dwAvailPhys, ms.dwAvailPhys) );

	ioh->AddData( string::format("Swap:\t%10ld \t%10ld \t%10ld \x0a", ms.dwTotalPageFile, ms.dwTotalPageFile-ms.dwAvailPageFile, ms.dwAvailPageFile) );

	ioh->AddData( string::format("MemTotal: \t%10ld kB\x0a", ms.dwTotalPhys / 1024) );

	ioh->AddData( string::format("MemFree:  \t%10ld kB\x0a", ms.dwAvailPhys / 1024) );

	ioh->AddData( string::format("MemShared:\t%10ld kB\x0a", 0) ); //shared how to determine?

	ioh->AddData( string::format("SwapTotal:\t%10ld kB\x0a", ms.dwTotalPageFile / 1024) );

	ioh->AddData( string::format("SwapFree: \t%10ld kB\x0a", ms.dwAvailPageFile / 1024) );

	return ioh;
}

IOHandler* FilesystemProc::Get_CpuInfo(Path& path)
{
	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, true);

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	for(DWORD cpu=0; cpu<si.dwNumberOfProcessors; ++cpu)
	{
		ioh->AddData( string::format("processor : %d\x0a", cpu) );

		ioh->AddData( string::format("vendor_id : %s\x0a", "Intel compatible (keow)") );

		ioh->AddData( string::format("bobomips  : %d\x0a", g_pKernelTable->m_BogoMips) );

		ioh->AddData( string::format("cpu count : %d\x0a", si.dwNumberOfProcessors) );

		ioh->AddData( string::format("fpu       : %s\x0a", LegacyWindows::IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED) ? "no" : "yes") );
	}

	return ioh;
}

IOHandler* FilesystemProc::Get_Uptime(Path& path)
{
	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, true);

	DWORD up,user,system,nice,idle;
	CalcCpuTimingsInJiffies(&up, &user, &system, &nice, &idle);

	float uptime = (float)up/Jiffies;
	float idletime = (float)idle/Jiffies;

	ioh->AddData( string::format("%.2lf %.2lf\x0a", uptime, idletime) );

	return ioh;
}

IOHandler* FilesystemProc::Get_Stat(Path& path)
{
	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, true);

	DWORD uptime,user,system,nice,idle;
	CalcCpuTimingsInJiffies(&uptime, &user, &system, &nice, &idle);

	FILETIME ftBoot;
	SystemTimeToFileTime(&g_pKernelTable->m_BootTime, &ftBoot);

	ioh->AddData( string::format("cpu %ld %ld %ld %ld\x0a", user, nice, system, idle) );

	ioh->AddData( string::format("page %ld %ld\x0a", 0, 0) );

	ioh->AddData( string::format("swap %ld %ld\x0a", 0, 0) );

	ioh->AddData( string::format("intr %ld\x0a", 0) );

	ioh->AddData( string::format("disk_io: \x0a") );

	ioh->AddData( string::format("ctxt %ld\x0a", 0) );

	ioh->AddData( string::format("btime %ld\x0a", FILETIME_TO_TIME_T(ftBoot)) );

	ioh->AddData( string::format("processes %ld\x0a", g_pKernelTable->m_ForksSinceBoot) );

	return ioh;
}

IOHandler* FilesystemProc::Get_Mounts(Path& path)
{
	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, true);

	KernelTable::MountPointList::iterator it;
	for(it=g_pKernelTable->m_MountPoints.begin();
	    it!=g_pKernelTable->m_MountPoints.end();
		++it)
	{
		MountPoint * mp = *it;

		ioh->AddData( string::format("%s %s %s %s %d %d\x0a", mp->GetDestination().c_str(), mp->GetUnixMountPoint().GetUnixPath().c_str(), mp->GetFilesystem()->Name(), mp->GetOptions().c_str(), 0,0) );
	}

	return ioh;
}

IOHandler* FilesystemProc::Get_LoadAvg(Path& path)
{
	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, true);

	ioh->AddData( string::format(
			"%ld %ld %ld\x0a"
			,1
			,1
			,1
			) );

	return ioh;
}

//////////////////////////////////////////////////////////////////

IOHandler* FilesystemProc::Get_Pid_Cmdline(Path& path, const char * pid)
{
	Process * pp = PidStrToProcess(pid);
	if(pp==0)
		return NULL;

	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, false);

	ADDR StrArrayEntry;
	ADDR str;
	char zero = 0;

	StrArrayEntry = pp->m_Arguments;
	pp->ReadMemory(&str, StrArrayEntry, sizeof(ADDR));

	while(str!=0)
	{
		ioh->AddData( MemoryHelper::ReadString(pp->m_hProcess, str) );
		ioh->AddData(&zero, 1);

		StrArrayEntry+=sizeof(ADDR);
		pp->ReadMemory(&str, StrArrayEntry, sizeof(ADDR));
	}
	ioh->AddData(&zero, 1);

	return ioh;
}

IOHandler* FilesystemProc::Get_Pid_Cwd(Path& path, const char * pid)
{
	Process * pp = PidStrToProcess(pid);
	if(pp==0)
		return NULL;

	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, true);

	ioh->AddData( pp->m_UnixPwd.GetUnixPath().c_str() );

	return ioh;
}

IOHandler* FilesystemProc::Get_Pid_Environ(Path& path, const char * pid)
{
	Process * pp = PidStrToProcess(pid);
	if(pp==0)
		return NULL;

	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, false);

	ADDR StrArrayEntry;
	ADDR str;
	char zero = 0;

	StrArrayEntry = pp->m_Environment;
	pp->ReadMemory(&str, StrArrayEntry, sizeof(ADDR));

	while(str!=0)
	{
		ioh->AddData( MemoryHelper::ReadString(pp->m_hProcess, str) );
		ioh->AddData(&zero, 1);

		StrArrayEntry+=sizeof(ADDR);
		pp->ReadMemory(&str, StrArrayEntry, sizeof(ADDR));
	}
	ioh->AddData(&zero, 1);

	return ioh;
}

IOHandler* FilesystemProc::Get_Pid_Exe(Path& path, const char * pid)
{
	Process * pp = PidStrToProcess(pid);
	if(pp==0)
		return NULL;

	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, false);

	ioh->AddData(pp->m_ProcessFileImage.GetUnixPath().c_str());

	return ioh;
}

IOHandler* FilesystemProc::Get_Pid_Maps(Path& path, const char * pid)
{
	return NULL;
}

IOHandler* FilesystemProc::Get_Pid_Mem(Path& path, const char * pid)
{
	return NULL;
}

IOHandler* FilesystemProc::Get_Pid_Root(Path& path, const char * pid)
{
	return NULL;
}

IOHandler* FilesystemProc::Get_Pid_Stat(Path& path, const char * pid)
{
	Process * pp = PidStrToProcess(pid);
	if(pp==0)
		return NULL;

	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, true);

	FILETIME ftCreateTime, ftExitTime, ftKernelTime, ftUserTime;
	GetProcessTimes(pp->m_hProcess, &ftCreateTime, &ftExitTime, &ftKernelTime, &ftUserTime);

	DWORD starttime = ToJiffies(&pp->m_BaseTimes.ftCreateTime, &ftCreateTime);
	DWORD kerneltime = ToJiffies(&pp->m_BaseTimes.ftKernelTime, &ftKernelTime);
	DWORD usertime = ToJiffies(&pp->m_BaseTimes.ftUserTime, &ftUserTime);

	int priority=10, nice=0;

	//TODO: fill out missing stat fields
	ioh->AddData( string::format(
			"%d (%s) %c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld 0 %ld %lu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d\x0a"
			,pp->m_Pid
			,pp->m_ProcessFileImage.GetUnixPath().c_str()
			,pp->IsSuspended()?'S':'R'  //state, just sleeping or running at present
			,pp->m_ParentPid
			,pp->m_ProcessGroupPID
			,0    //session id
			,0	  //tty number
			,pp->m_ProcessGroupPID //tpgid 
			,0    //flags
			,0	//minflt %lu\x0a,
			,0	//cminflt %lu\x0a,
			,0	//majflt %lu\x0a,
			,0	//cmajflt %lu\x0a,
			,usertime
			,kerneltime
			,0	//cutime %ld\x0a,
			,0	//cstime %ld\x0a,
			,priority
			,nice
			//,0 %ld\x0a,  //removed field - in format list as: 0
			,0	//itrealvalue %ld\x0a,
			,starttime
			,1	//vsize
			,1	//rss %ld\x0a,
			,0	//rlim %lu\x0a,
			,pp->m_ElfLoadData.program_base //??
			,pp->m_ElfLoadData.brk //??
			,pp->m_KeowUserStackTop
			,0	//current stack ESP  kstkesp %lu\x0a,
			,0	//current EIP  kstkeip %lu\x0a,
			,0	// .signal %lu\x0a,
			,0	//blocked %lu\x0a,
			,0	//~(pKernelSharedData->ProcessTable[pid].sigmask) //sigignore
			,0	//pKernelSharedData->ProcessTable[pid].sigmask //sigcatch %lu\x0a,
			,0	//wchan %lu\x0a,
			,0	//nswap %lu\x0a,
			,0	//cnswap %lu\x0a,
			,linux::SIGCHLD	//exit_signal %d\x0a,
			,0	//processor %d\x0a,
			) );

	return ioh;
}

IOHandler* FilesystemProc::Get_Pid_Statm(Path& path, const char * pid)
{
	Process * pp = PidStrToProcess(pid);
	if(pp==0)
		return NULL;

	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, true);

	FILETIME ftCreateTime, ftExitTime, ftKernelTime, ftUserTime;
	GetProcessTimes(pp->m_hProcess, &ftCreateTime, &ftExitTime, &ftKernelTime, &ftUserTime);

	DWORD starttime = ToJiffies(&pp->m_BaseTimes.ftCreateTime, &ftCreateTime);

	int priority=10, nice=0;

	//TODO: fill out missing stat fields
	ioh->AddData( string::format(
			"%ld %ld %ld %ld %ld %ld %ld\x0a"
			,2	//size
			,1	//resident
			,0	//share
			,1	//trs
			,0	//drs
			,0	//lrs
			,0	//dt
			) );

	return ioh;
}

IOHandler* FilesystemProc::Get_Pid_Status(Path& path, const char * pid)
{
	Process * pp = PidStrToProcess(pid);
	if(pp==0)
		return NULL;

	IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::File, true);


	ioh->AddData( string::format("Name:   %s\x0a", pp->m_ProcessFileImage.GetUnixPathElement(pp->m_ProcessFileImage.GetElementCount()-1).c_str()) );
	ioh->AddData( string::format("State:  %s\x0a", pp->IsSuspended()?"S (sleeping)":"R (runnable)") );
	ioh->AddData( string::format("Tgid:   %ld\x0a", pp->m_pControllingTty) ); //TODO this is WRONG!
	ioh->AddData( string::format("Pid:    %ld\x0a", pp->m_Pid) );
	ioh->AddData( string::format("PPid:   %ld\x0a", pp->m_ParentPid) );
	ioh->AddData( string::format("Uid:    %ld %ld %ld %ld\x0a", pp->m_uid, pp->m_euid, pp->m_saved_uid, 0) ); //TODO which ones?
	ioh->AddData( string::format("Gid:    %ld %ld %ld %ld\x0a", pp->m_gid, pp->m_egid, pp->m_saved_gid, 0) ); //TODO which ones?
	ioh->AddData( string::format("VmSize: %8ld kB\x0a", 1) );
	ioh->AddData( string::format("VmLck:  %8ld kB\x0a", 0) );
	ioh->AddData( string::format("VmRSS:  %8ld kB\x0a", 1) );
	ioh->AddData( string::format("VmData: %8ld kB\x0a", 1) );
	ioh->AddData( string::format("VmStk:  %8ld kB\x0a", 1) );
	ioh->AddData( string::format("VmExe:  %8ld kB\x0a", 1) );
	ioh->AddData( string::format("VmLib:  %8ld kB\x0a", 1) );
	ioh->AddData( string::format("SigPnd: 0000000000000000\x0a") );
	ioh->AddData( string::format("SigBlk: 0000000000000000\x0a") );
	ioh->AddData( string::format("SigIgn: 0000000000000000\x0a") );

	return ioh;
}



IOHandler* FilesystemProc::Get_Pid_Fd(Path& path, const char * pid, const char * strFd)
{
	Process * pp = PidStrToProcess(pid);
	if(pp==0)
		return NULL;

	int fd = atoi(strFd);
	if(fd<0 || fd>=MAX_OPEN_FILES)
		return NULL;
	return pp->m_OpenFiles[fd]->Duplicate();
}

