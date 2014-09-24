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
 * exit process with a return code
 * EBX = exit code
 */
void SysCalls::sys_exit(CONTEXT& ctx)
{
	ktrace("Process terminating, code 0x%08lx\n", ctx.Ebx);
	SysCallDll::exit((UINT)ctx.Ebx);
}


/*****************************************************************************/


/*
 * int sigaction(int signum, const struct old_sigaction *act, struct old_sigaction *oldact);
 */
void SysCalls::sys_sigaction(CONTEXT& ctx)
{
	DWORD signum = ctx.Ebx;
	linux::old_sigaction *pAct = (linux::old_sigaction*)ctx.Ecx;
	linux::old_sigaction *pOldact = (linux::old_sigaction*)ctx.Edx;

	if(signum > MAX_SIGNALS)
	{
		ctx.Eax = -linux::EINVAL;
		return;
	}

	if(pOldact)
	{
		linux::old_sigaction sa;

		sa.sa_handler  = P->m_SignalAction[signum].sa_handler;
		sa.sa_flags    = P->m_SignalAction[signum].sa_flags;
		sa.sa_restorer = P->m_SignalAction[signum].sa_restorer;
		sa.sa_mask     = P->m_SignalAction[signum].sa_mask.sig[0];

		P->WriteMemory((ADDR)pOldact, sizeof(sa), &sa);
	}

	if(pAct)
	{
		linux::old_sigaction sa;
		P->ReadMemory(&sa, (ADDR)pAct, sizeof(sa));

		P->m_SignalAction[signum].sa_handler     = sa.sa_handler;
		P->m_SignalAction[signum].sa_flags       = sa.sa_flags;
		P->m_SignalAction[signum].sa_restorer    = sa.sa_restorer;
		P->m_SignalAction[signum].sa_mask.sig[0] = sa.sa_mask;

		ktrace("set old_sigaction handler for signal %d : 0x%08lx\n", signum, sa.sa_handler);
	}

	ctx.Eax = 0;
}

/*****************************************************************************/

/*
 * int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
 */
void SysCalls::sys_sigprocmask(CONTEXT& ctx)
{
	linux::sigset_t *pSet = (linux::sigset_t*)ctx.Ecx;
	linux::sigset_t *pOldset = (linux::sigset_t*)ctx.Edx;

	linux::sigset_t *pCurrSigset = &P->m_SignalMask;

	if(pOldset)
	{
		P->WriteMemory((ADDR)pOldset, sizeof(linux::sigset_t), pCurrSigset);
		ctx.Eax = 0;
		return;
	}

	if(pSet)
	{
		linux::sigset_t set;
		P->ReadMemory(&set, (ADDR)pSet, sizeof(set));

		for(int i=0; i<linux::_NSIG_WORDS; ++i)
		{
			switch(ctx.Ebx)
			{
			case linux::SIG_BLOCK:
				pCurrSigset->sig[i] |= set.sig[i];
				ctx.Eax = 0;
				break;
			case linux::SIG_UNBLOCK:
				pCurrSigset->sig[i] &= ~(set.sig[i]);
				ctx.Eax = 0;
				break;
			case linux::SIG_SETMASK:
				pCurrSigset->sig[i] = set.sig[i];
				ctx.Eax = 0;
				break;

			default:
				ctx.Eax = -linux::EINVAL;
				return;
			}
		}
	}
}



/*****************************************************************************/

/*
 * pid_t getpgrp()
 */
void SysCalls::sys_getpgrp(CONTEXT& ctx)
{
	ctx.Eax = P->m_ProcessGroupPID;
}

/*****************************************************************************/

/*
 * pid_t getpgid(int pid, int pgid)
 */
void SysCalls::sys_getpgid(CONTEXT& ctx)
{
	DWORD pid = ctx.Ebx;
	DWORD pgid = ctx.Ecx;

	if(pid==0)
		pid = P->m_Pid;

	//validate pid
	Process * p2 = g_pKernelTable->FindProcess(pid);
	if(p2==NULL)
	{
		ctx.Eax = -linux::ESRCH;
		return;
	}

	if(pgid==0)
		pgid = p2->m_ProcessGroupPID;
	
	p2->m_ProcessGroupPID = pgid;
	ctx.Eax = 0;
}

/*****************************************************************************/


/*
 * pid_t setpgrp()
 */
void SysCalls::sys_setpgid(CONTEXT& ctx)
{
	P->m_ProcessGroupPID = P->m_Pid;
	ctx.Eax = 0;
}

/*****************************************************************************/

/*
 * pid_t fork()
 */
void SysCalls::sys_fork(CONTEXT& ctx)
{
	/*
	launch win32 process stub
	get it to copy this process
	*/
	Process * NewP = Process::StartFork(P);

	if(NewP==NULL)
		ctx.Eax = -linux::EAGAIN;
	else
		ctx.Eax = NewP->m_Pid; //the new process (our fork processing has only the parent reaching this stage)
}

/*
 * int execve(filename, argv[], envp[])
 */
void SysCalls::sys_execve(CONTEXT& ctx)
{
	string filename = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx);
	//need a kernel copy of argv and envp  (we're about to remove the processes own memory)
	DWORD dwArgCnt, dwEnvCnt, dwMemSize;
	ADDR argv = MemoryHelper::CopyStringListBetweenProcesses(P->m_hProcess, (ADDR)ctx.Ecx, GetCurrentProcess(), NULL, &dwArgCnt, &dwMemSize);
	ADDR envp = MemoryHelper::CopyStringListBetweenProcesses(P->m_hProcess, (ADDR)ctx.Edx, GetCurrentProcess(), NULL, &dwEnvCnt, &dwMemSize);

	ktrace("execve(%s,...,...)\n", filename.c_str());

	//we can load this, right?
	Path img;
	img.SetUnixPath(filename);
	if(!P->CanLoadImage(img, false))
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
		return; //can't exec it
	}


	//close files and dealloc mem
	P->FreeResourcesBeforeExec();

	//new argv, and env (in the kernel - they are copied over later)
	P->m_Arguments = argv;
	P->m_Environment = envp;
	P->m_ArgCnt = dwArgCnt;
	P->m_EnvCnt = dwEnvCnt;

	//reset the keow stack (AFTER resource free - it may have injected code and new stack)
	//linux process start with a little bit of stack in use but overwritable?
	ctx.Esp = (DWORD)P->m_KeowUserStackTop - 0x200;
	SetThreadContext(P->m_ThreadList[0], &ctx);

	//load new image
	P->m_ProcessFileImage.SetUnixPath(filename);
	P->LoadImage(P->m_ProcessFileImage, false);
	P->StartNewImageRunning(); //set up context at least

	//participate in ptrace() 
	if(P->m_ptrace.OwnerPid)
	{
		ktrace("stopping for ptrace on successfull exec\n");

		//need dummy context for this - fake a successfull execve return
		P->m_ptrace.ctx = ctx;
		P->m_ptrace.ctx.Eax = 0;//success
		P->m_ptrace.Saved_Eax = linux::__NR_execve; //syscall
		P->m_ptrace.ctx_valid = true;
		P->SendSignal(linux::SIGTRAP);
	}

	//when we get here the execution point has changed - it's a brand new process
	// update ctx so that debugger loop doesn't restore a bad context
	GetThreadContext(T->hThread, &ctx);
}

/*****************************************************************************/

/*
 * pid_t wait4(pid_t pid, int *status, int options, rusage *ru)
 */
void SysCalls::sys_wait4(CONTEXT& ctx)
{
	int wait_pid = (int)ctx.Ebx;
	int* pStatus = (int*)ctx.Ecx;
	DWORD options = ctx.Edx;
	linux::rusage* pRU = (linux::rusage*)ctx.Esi;
	int result = -linux::ECHILD;
	bool one_stopped = false;

	//there can be multiple handles to wait on 
	int MaxProcs = g_pKernelTable->m_Processes.size();
	DWORD * pProcessWin32Pids = new DWORD[MaxProcs];
	HANDLE * pProcessHandles = new HANDLE[MaxProcs+1];
	HANDLE * pMainThreadHandles = new HANDLE[MaxProcs+1];
	DWORD * pPids = new DWORD[MaxProcs+1];
	int NumHandles = 0;


	//select children to wait on
	KernelTable::ProcessList::iterator it;
	for(it=g_pKernelTable->m_Processes.begin();
	    it!=g_pKernelTable->m_Processes.end();
		++it)
	{
		Process * pChild = *it;
		if(pChild->m_ParentPid != P->m_Pid)
			continue;

		bool wait_this = false;

		if(wait_pid < -1  &&  pChild->m_ProcessGroupPID == wait_pid)
			wait_this=true;
		else
		if(wait_pid == -1)
			wait_this=true;
		else
		if(wait_pid == 0  &&  pChild->m_ProcessGroupPID == P->m_ProcessGroupPID)
			wait_this=true;
		else
		if(wait_pid > 0  &&  pChild->m_Pid == wait_pid)
			wait_this=true;
		
		
		if(wait_this)
		{
			pProcessWin32Pids[NumHandles] = pChild->m_dwProcessId;
			pProcessHandles[NumHandles] = pChild->m_hProcess;
			pPids[NumHandles] = pChild->m_Pid;
			NumHandles++; 

			//a child already exited, no need to wait?
			if(pChild->m_bStillRunning==false)
			{
				result = pChild->m_Pid;
				break;
			}
		}
	}

	if(NumHandles==0)
	{
		//no such child
		result = -linux::ECHILD;
	}
	else
	{
		//also want to be able to be woken up (wait aborted)
		pProcessHandles[NumHandles] = P->m_hWaitTerminatingEvent;
		//not increment NumHandles+

		//wait until interrupted or child terminates
		while(result < 0)
		{
			DWORD dwRet = WaitForMultipleObjects(NumHandles+1, pProcessHandles, FALSE, options&linux::WNOHANG?0:50);
			if(dwRet>=WAIT_OBJECT_0 && dwRet<=WAIT_OBJECT_0+NumHandles)
			{
				if(dwRet==WAIT_OBJECT_0+NumHandles) //(NumHandles+1)-1
				{
					//received a signal that stops wait()
					result = -linux::EINTR;
					break;
				}

				int pid = pPids[dwRet-WAIT_OBJECT_0];
				//did the process die? or was there something else to test
				if(g_pKernelTable->FindProcess(pid)->m_bStillRunning==false)
				{
					result = pid;
					break;
				}

				//else try again
			}
			/*
			else
			if(dwRet>=WAIT_ABANDONED_0 && dwRet<WAIT_ABANDONED_0+NumHandles)
			{
				int pid = pPids[dwRet-WAIT_ABANDONED_0];
				//did the process die? or was there something else to test
				if(pKernelSharedData->ProcessTable[pid].in_use==false)
				{
					result = pid;
					break;
				}
				//else try again
			}*/
			else
			if(dwRet==WAIT_FAILED)
			{
				result = -linux::EINVAL;
				break;
			}

			if(options & linux::WNOHANG)
				break;

			//check for stopped children
			for(int i=0; i<NumHandles; ++i)
			{
				int pid = pPids[i];
				Process * pChild = g_pKernelTable->FindProcess(pid);

				//does caller want stopped children?
				if((options & linux::WUNTRACED)
				|| pChild->m_ptrace.OwnerPid == P->m_Pid)
				{
					if(pChild->m_bInWin32Setup)
					{
						ktrace("skipping wait for pid %d - in setup flag set\n", pid);
					}
					else
					{
						if(pChild->IsSuspended())
						{
							result = pid;
							one_stopped = true;
							break;
						}
					}
				}
			}

			//Wait debugging
			//Sleep(10);
			//DebugBreak();

		}
	}

	if(result>0)
	{
		//a process was found
		Process *pChild = g_pKernelTable->FindProcess(result);

		if(pStatus)
		{
			int TmpStatus = 0;

			if(one_stopped)
			{
				//stopped
				TmpStatus = 0x7f; 

				//signal that stopped it
				if(pChild->m_ptrace.OwnerPid==P->m_Pid && pChild->m_ptrace.Request!=0)
					TmpStatus |= linux::SIGTRAP << 8; //man ptrace says parent thinks child is in this state
				else
					TmpStatus |= (pChild->m_CurrentSignal&0xFF) << 8;
			}
			else
			{
				//exit status
				TmpStatus |= (pChild->m_dwExitCode & 0xFF) << 8;
			
				//terminating signal
				TmpStatus |= pChild->m_KilledBySig & 0x7f;
				
				//core flag
				TmpStatus |= pChild->m_bCoreDumped?0x80:0x00;
			}

			P->WriteMemory((ADDR)pStatus, sizeof(int), &TmpStatus);
		}

		if(pRU)
		{
			linux::rusage ru;
			memset(&ru, 0, sizeof(ru));

			ktrace("IMPLEMENT wait4 rusage\n");

			P->WriteMemory((ADDR)pRU, sizeof(ru), &ru);
		}

		//now forget about that child (unless it was a stopped one we returned)
		if(!one_stopped)
		{
			g_pKernelTable->m_Processes.erase(pChild);
		}
	}

	//free
	delete [] pProcessWin32Pids;
	delete [] pProcessHandles;
	delete [] pMainThreadHandles;
	delete [] pPids;
	ResetEvent(P->m_hWaitTerminatingEvent);

	ctx.Eax = result;


	ktrace("wait4(%d) returned %d [%s]\n", wait_pid, result, one_stopped?"stopped":"exited" );
}


/*
 * pid_t waitpid(pid_t pid, int *status, int options)
 */
void SysCalls::sys_waitpid(CONTEXT& ctx)
{
	//reuse wait4(pid_t pid, int *status, int options, rusage *ru)
	CONTEXT save = ctx;

	ctx.Esi = 0; //rusage*
	sys_wait4(ctx);

	ctx.Esi = save.Esi;
}
