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

#include "includes.h"
#include "SysCalls.h"

// eax is the syscall number
// ebx,ecx,edx,esi,edi,ebp are up to 6(max) parameters
// any more parameters and the caller just puts a struct pointer in one of these
// eax is the return value


/*
 *  uname(struct utsname *name)
 */
void SysCalls::sys_uname(CONTEXT& ctx)
{
	linux::new_utsname * pU = (linux::new_utsname *)ctx.Ebx;
	linux::new_utsname U;

	DWORD siz = sizeof(U.sysname);
	GetComputerName(U.sysname, &siz);
	StringCbCopy(U.nodename, sizeof(U.nodename), U.sysname);
	StringCbCopy(U.release, sizeof(U.release), g_pKernelTable->m_KernelVersion.c_str());
	StringCbCopy(U.version, sizeof(U.version), "keow");
	StringCbCopy(U.machine, sizeof(U.machine), g_pKernelTable->m_KernelCpuType.c_str());
	U.domainname[0] = 0;

	P->WriteMemory((ADDR)pU, sizeof(U), &U);
	ctx.Eax = 0;
}


/*****************************************************************************/

/*
 * int getpid()
 */
void SysCalls::sys_getpid(CONTEXT& ctx)
{
	ctx.Eax = P->m_Pid;
}

/*****************************************************************************/

/*
 * int getppid()
 */
void SysCalls::sys_getppid(CONTEXT& ctx)
{
	ctx.Eax = P->m_ParentPid;
}


/*****************************************************************************/

/*
 * mode_t umask(mode_t mask)
 */
void SysCalls::sys_umask(CONTEXT& ctx)
{
	ctx.Eax = P->m_umask;
	P->m_umask = ctx.Ebx;
}


/*****************************************************************************/

/*
 * time_t time(time_t *t);
 */
void SysCalls::sys_time(CONTEXT& ctx)
{
	SYSTEMTIME st;
	FILETIME ft;

	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);

	ctx.Eax = FILETIME_TO_TIME_T(ft);

	if(ctx.Ebx)
		P->WriteMemory((ADDR)ctx.Ebx, sizeof(DWORD), &ctx.Eax);
}


/*****************************************************************************/

/*
 * long getrlimit(uint resource, rlimit* rlim)
 */
void SysCalls::sys_ugetrlimit(CONTEXT& ctx)
{
	linux::rlimit *pRLim = (linux::rlimit *)ctx.Ecx;
	linux::rlimit RLim;

	if(!pRLim)
	{
		ctx.Eax = -linux::EFAULT;
		return;
	}

	ctx.Eax = 0;
	switch(ctx.Ebx)
	{
	case linux::RLIMIT_CPU:		/* CPU time in ms */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_FSIZE:		/* Maximum filesize */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_DATA:		/* max data size */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_STACK:		/* max stack size */
		RLim.rlim_cur = RLim.rlim_max = P->m_KeowUserStackTop - P->m_KeowUserStackBase;
		break;
	case linux::RLIMIT_CORE:		/* max core file size */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_RSS:		/* max resident set size */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_NPROC:		/* max number of processes */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_NOFILE:		/* max number of open files */
		RLim.rlim_cur = RLim.rlim_max = MAX_OPEN_FILES;
		break;
	case linux::RLIMIT_MEMLOCK:	/* max locked-in-memory address space */
		RLim.rlim_cur = RLim.rlim_max = 0x70000000;
		break;
	case linux::RLIMIT_AS:			/* address space limit */
		RLim.rlim_cur = RLim.rlim_max = 0x7000000;
		break;
	case linux::RLIMIT_LOCKS:		/* maximum file locks held */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	default:
		ctx.Eax = -linux::EINVAL;
		return;
	}

	P->WriteMemory((ADDR)pRLim, sizeof(RLim), &RLim);
}

/*
 * same as ugetrlimit except unsigned values
 */
void SysCalls::sys_getrlimit(CONTEXT& ctx)
{
	linux::rlimit *pRLim = (linux::rlimit *)ctx.Ecx;
	linux::rlimit RLim;

	if(!pRLim)
	{
		ctx.Eax = -linux::EFAULT;
		return;
	}

	ctx.Eax = 0;
	switch(ctx.Ebx)
	{
	case linux::RLIMIT_CPU:		/* CPU time in ms */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_FSIZE:		/* Maximum filesize */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_DATA:		/* max data size */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_STACK:		/* max stack size */
		RLim.rlim_cur = RLim.rlim_max = P->m_KeowUserStackTop - P->m_KeowUserStackBase;
		break;
	case linux::RLIMIT_CORE:		/* max core file size */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_RSS:		/* max resident set size */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_NPROC:		/* max number of processes */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	case linux::RLIMIT_NOFILE:		/* max number of open files */
		RLim.rlim_cur = RLim.rlim_max = MAX_OPEN_FILES;
		break;
	case linux::RLIMIT_MEMLOCK:	/* max locked-in-memory address space */
		RLim.rlim_cur = RLim.rlim_max = 0x70000000;
		break;
	case linux::RLIMIT_AS:			/* address space limit */
		RLim.rlim_cur = RLim.rlim_max = 0x7000000;
		break;
	case linux::RLIMIT_LOCKS:		/* maximum file locks held */
		RLim.rlim_cur = RLim.rlim_max = linux::RLIM_INFINITY;
		break;
	default:
		ctx.Eax = -linux::EINVAL;
		return;
	}

	P->WriteMemory((ADDR)pRLim, sizeof(RLim), &RLim);
}



/*****************************************************************************/

/*
 * int reboot(int magic, int magic2, int flag, void *arg);
 */
void SysCalls::sys_reboot(CONTEXT& ctx)
{
	ctx.Eax = -linux::EINVAL;

	if(ctx.Ebx != linux::LINUX_REBOOT_MAGIC1)
		return;

	if(ctx.Ecx != linux::LINUX_REBOOT_MAGIC2
	&& ctx.Ecx != linux::LINUX_REBOOT_MAGIC2A
	&& ctx.Ecx != linux::LINUX_REBOOT_MAGIC2B)
		return;

	switch(ctx.Edx)
	{
	case linux::LINUX_REBOOT_CMD_RESTART:
	case linux::LINUX_REBOOT_CMD_HALT:
	case linux::LINUX_REBOOT_CMD_CAD_ON:
	case linux::LINUX_REBOOT_CMD_CAD_OFF:
	case linux::LINUX_REBOOT_CMD_POWER_OFF:
	case linux::LINUX_REBOOT_CMD_RESTART2:
		ktrace("Implement me: Dummy reboot() - no action taken\n");
		ctx.Eax = 0;
		break;
	}
}

/*****************************************************************************/


/*
 * int kill(pid, sig)
 */
void SysCalls::sys_kill(CONTEXT& ctx)
{
	PID pid = ctx.Ebx;
	unsigned int sig = ctx.Ecx;

	if(sig>=linux::_NSIG)
	{
		ctx.Eax = -linux::EINVAL;
		return;
	}

	//helper: sig 0 allows for error checking on pids. no signal actually sent

	// positive - send to 1 process
	if(pid>0)
	{
		ctx.Eax = -linux::ESRCH;
		Process * pDest = g_pKernelTable->FindProcess(pid);
		if(pDest!=NULL)
		{
			if(sig!=0)
				pDest->SendSignal(sig);
			ctx.Eax = 0;
		}
		return;
	}

	// -1 all processes except init
	if(pid == -1)
	{
		KernelTable::ProcessList::iterator it;
		for(it=g_pKernelTable->m_Processes.begin(); it!=g_pKernelTable->m_Processes.end(); ++it)
		{
			if((*it)->m_Pid != 1)
			{
				if(sig!=0)
					(*it)->SendSignal(sig);
			}
		}
		ctx.Eax = 0;
		return;
	}


	//else send to a process group

	PID pgrp = 0;

	if(pid==0)
		pgrp = P->m_Pid; //our group
	else
		pgrp = -pid; //dest group


	KernelTable::ProcessList::iterator it;
	for(it=g_pKernelTable->m_Processes.begin(); it!=g_pKernelTable->m_Processes.end(); ++it)
	{
		if((*it)->m_Pid != 1
		&& (*it)->m_ProcessGroupPID == pgrp)
		{
			if(sig!=0)
				(*it)->SendSignal(sig);
		}
	}
	ctx.Eax = 0;

#undef DO_SIGNAL

}

/*****************************************************************************/


/*
 * long ptrace(enum request, int pid, void* addr, void* data)
 */
void SysCalls::sys_ptrace(CONTEXT& ctx)
{
	int request = ctx.Ebx;
	int pid = ctx.Ecx;
	void* addr = (void*)ctx.Edx;
	void* data = (void*)ctx.Esi;

	Process * pTraced = g_pKernelTable->FindProcess(pid);

	//TODO: what does ptrace on a process with threads actually do?
	ThreadInfo * pTracedThread = pTraced->m_ThreadList[0];

	switch(request)
	{
	case linux::PTRACE_TRACEME:
		ktrace("ptrace PTRACE_TRACEME\n");
		P->m_ptrace.OwnerPid = P->m_ParentPid;
		P->m_ptrace.Request = linux::PTRACE_TRACEME;
		ctx.Eax = 0;
		break;

	case linux::PTRACE_SYSCALL:
		{
			ktrace("ptrace PTRACE_SYSCALL pid %d\n", pid);
			pTraced->m_ptrace.Request = linux::PTRACE_SYSCALL;
			if((int)data!=0 && (int)data!=linux::SIGSTOP)
				pTraced->m_ptrace.new_signal = (int)data;
			ktrace("ptrace_syscall resuming pid %d\n",pid);
			ResumeThread(pTraced->m_KernelThreadHandle); //kernel handler thread is paused when using ptrace
			ctx.Eax = 0;
		}
		break;

	case linux::PTRACE_CONT:
		{
			ktrace("ptrace PTRACE_CONT pid %d\n", pid);
			pTraced->m_ptrace.Request = 0;
			if((int)data!=0 && (int)data!=linux::SIGSTOP)
				pTraced->m_ptrace.new_signal = (int)data;
			ktrace("ptrace_cont resuming pid %d\n",pid);
			ResumeThread(pTraced->m_KernelThreadHandle); //kernel handler thread is paused when using ptrace
			ctx.Eax = 0;
		}
		break;

	case linux::PTRACE_PEEKUSR:
		{
			ktrace("ptrace PTRACE_PEEKUSR pid %d addr 0x%lx (reg %ld) data 0x%08lx\n", pid, addr, (DWORD)addr>>2, data);

			if((unsigned long)addr > sizeof(linux::user)-3)
			{
				ctx.Eax = -linux::EFAULT;
				break;
			}

			//read data at offset 'addr' in the kernel 'user' struct (struct user in asm/user.h)
			//we don't keep one of those so make one here
			CONTEXT *pTracedCtx;
			CONTEXT TempCtx;
			if(pTraced->m_ptrace.ctx_valid)
				pTracedCtx = &pTraced->m_ptrace.ctx; 
			else
			{
				ktrace("peekusr get Traced context\n");
				//it's possible that the traced process has caught a signal and then
				//signalled us, however it's kernel thread has yet to sleep
				//so we do an extra suspend here to get the correct context
				SuspendThread(pTracedThread->hThread);
				TempCtx.ContextFlags = CONTEXT_FULL;
				GetThreadContext(pTracedThread->hThread, &TempCtx);
				ResumeThread(pTracedThread->hThread);
				pTracedCtx = &TempCtx;
			}

			linux::user usr;
			memset(&usr,0,sizeof(usr));
			usr.regs.eax  = pTracedCtx->Eax;
			usr.regs.ebx  = pTracedCtx->Ebx;
			usr.regs.ecx  = pTracedCtx->Ecx;
			usr.regs.edx  = pTracedCtx->Edx;
			usr.regs.esi  = pTracedCtx->Esi;
			usr.regs.edi  = pTracedCtx->Edi;
			usr.regs.ebp  = pTracedCtx->Ebp;
			usr.regs.ds   = (unsigned short)pTracedCtx->SegDs;
			usr.regs.__ds = 0;
			usr.regs.es   = (unsigned short)pTracedCtx->SegEs;
			usr.regs.__es = 0;
			usr.regs.fs   = (unsigned short)pTracedCtx->SegFs;
			usr.regs.__fs = 0;
			usr.regs.gs   = (unsigned short)pTracedCtx->SegGs;
			usr.regs.__gs = 0;
			usr.regs.cs   = (unsigned short)pTracedCtx->SegCs;
			usr.regs.__cs = 0;
			usr.regs.ss   = (unsigned short)pTracedCtx->SegSs;
			usr.regs.__ss = 0;
			usr.regs.orig_eax = pTraced->m_ptrace.Saved_Eax;
			usr.regs.eip  = pTracedCtx->Eip;
			usr.regs.eflags = pTracedCtx->EFlags;
			usr.regs.esp  = pTracedCtx->Esp;
			//usr.signal = SIGTRAP; //man ptrace says parent thinks Traced is in this state

			char * pWanted = ((char*)&usr) + (DWORD)addr;
			DWORD retdata = *((DWORD*)pWanted);
			P->WriteMemory((ADDR)data, sizeof(retdata), &retdata);

			ktrace("ptrace [0x%x]=0x%x eax=0x%x orig_eax=0x%x\n", addr, retdata, usr.regs.eax, usr.regs.orig_eax);

			ctx.Eax = 0;
		}
		break;

	case linux::PTRACE_PEEKTEXT:
	case linux::PTRACE_PEEKDATA:
		{
			ktrace("ptrace PTRACE_PEEKDATA pid %d addr 0x%lx data 0x%08lx\n", pid, addr, data);
			DWORD tmp;
			if(!pTraced->ReadMemory(&tmp, (ADDR)addr, sizeof(tmp)))
			{
				ctx.Eax = -linux::EFAULT;
			}
			else
			{
				P->WriteMemory((ADDR)data, sizeof(tmp), &tmp);
				ctx.Eax = tmp;
				ktrace("ptrace [0x%x]=0x%x\n", addr, tmp);
			}
		}
		break;

	case linux::PTRACE_KILL:
		ktrace("ptrace PTRACE_KILL pid %d \n", pid);
		pTraced->SendSignal(linux::SIGKILL);
		ResumeThread(pTraced->m_KernelThreadHandle); //need to wake kernel handler thread so it can die
		ctx.Eax = 0;
		break;

	default:
		ktrace("IMPLEMENT ptrace request %lx\n", ctx.Ebx);
		ctx.Eax = -linux::ENOSYS;
		break;
	}
}


/*****************************************************************************/


/*
 * int nanosleep(timespec*req, timespec*rem)
 */
void SysCalls::sys_nanosleep(CONTEXT& ctx)
{
	linux::timespec * pReq = (linux::timespec*)ctx.Ebx;
	linux::timespec * pRem = (linux::timespec*)ctx.Ecx;
	linux::timespec req;
	linux::timespec rem;

	P->ReadMemory(&req, (ADDR)pReq, sizeof(req));

	DWORD start = GetTickCount();
	DWORD msec = (req.tv_sec*1000) + (req.tv_nsec/1000000); //milliseconds to wait

	//signal interruptable wait
	if(WaitForSingleObject(P->m_hWaitTerminatingEvent, msec) == WAIT_OBJECT_0)
	{
		ctx.Eax = -linux::EINTR;
	}
	else
	{
		ctx.Eax = 0;
	}

	if(pRem)
	{
		DWORD end = GetTickCount();
		if(end < start)
		{
			//tick count wrapped!
			start -= (end+1);
			end -= (end+1);;
		}
		msec -= (end-start);
		rem.tv_sec = msec/1000;
		rem.tv_nsec = msec%1000 * 1000000;

		P->WriteMemory((ADDR)pRem, sizeof(rem), &rem);
	}
}



/*
 * int sysinfo(struct sysinfo *info)
 */
void SysCalls::sys_sysinfo(CONTEXT& ctx)
{
	linux::sysinfo si;
	
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);

	memset(&si, 0, sizeof(si));
	si.uptime = GetTickCount()/1000;
	si.loads[0] = 0; //1, 5, and 15 minute load averages
	si.loads[1] = 0;
	si.loads[2] = 0;
	si.totalram = ms.dwAvailPhys;
	si.freeram  = ms.dwTotalPhys;
	si.sharedram = 0;/* Amount of shared memory */
	si.bufferram = 0; /* Memory used by buffers */
	si.totalswap = ms.dwTotalPageFile;
	si.freeswap  = ms.dwAvailPageFile;
	si.procs = g_pKernelTable->m_Processes.size();
	si.totalhigh = 0; /* Total high memory size */
	si.freehigh = 0;  /* Available high memory size */
	si.mem_unit = 4096; //same as page size???

	P->WriteMemory((ADDR)ctx.Ebx, sizeof(si), &si);
	ctx.Eax = 0;
}
