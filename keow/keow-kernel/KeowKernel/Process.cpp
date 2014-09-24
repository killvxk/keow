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

// Process.cpp: implementation of the Process class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "Process.h"
#include "IohNtConsole.h"
#include "SysCalls.h"



//handler shouldn't need the default 1MB stack, try a smaller one
const int Process::KERNEL_PROCESS_THREAD_STACK_SIZE = 32*1024;

const char * Process::KEOW_PROCESS_STUB = "Keow.exe";


//////////////////////////////////////////////////////////////////////

Process::Process()
{
	m_gid = 0;
	m_uid = 0;
	m_egid = 0;
	m_euid = 0;

	m_saved_uid = 0;
	m_saved_gid = 0;

	m_umask = 022; //octal - default from "man 2 umask"

	m_Pid = 0;
	m_ParentPid = 0;
	m_Environment = NULL;
	m_Arguments = NULL;
	m_ArgCnt = 0;
	m_EnvCnt = 0;

	m_ProcessGroupPID = 0;

	m_pControllingTty = NULL;
	m_bIsInForeground = true;

	m_hProcess = NULL;
	m_dwProcessId = NULL;

	m_KernelThreadId = 0;
	m_KernelThreadHandle = NULL;

	memset(&m_ElfLoadData, 0, sizeof(m_ElfLoadData));
	memset(&m_ptrace, 0, sizeof(m_ptrace));
	m_LinuxGateDSO = m_LinuxGateVSyscall = NULL;

	memset(&m_OpenFiles, 0, sizeof(m_OpenFiles));

	InitSignalHandling();

	m_bStillRunning = true;
	m_dwExitCode = -linux::SIGABRT; //in case not set later
	m_bCoreDumped = false;

	m_hWaitTerminatingEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hProcessStartEvent = m_hForkDoneEvent = NULL;

	memset(&SysCallAddr, 0, sizeof(SysCallAddr));

	memset(&m_LdtEntries, 0, sizeof(m_LdtEntries));
}

Process::~Process()
{
	//remove from the kernel process map
	g_pKernelTable->m_Processes.erase(this);

	//close all handles

	CloseHandle(m_hProcess);

	for(Process::ThreadList::iterator it = m_ThreadList.begin();
		it != m_ThreadList.end();
		++it)
	{
		ThreadInfo * pInfo = *it;
		CloseHandle(pInfo->hThread);
	}

	CloseHandle(m_KernelThreadHandle);

	CloseHandle(m_hWaitTerminatingEvent);
}



//Create a new process using the given path to a program
//The pid is as assigned by the kernel
//The initial environment is as supplied
//
Process* Process::StartInit(PID pid, Path& path, char ** InitialArguments, char ** InitialEnvironment)
{
	ktrace("StartInit()\n");

	/*
	launch win32 process stub
	load elf code into the process
	set stack/cpu contexts
	let process run
	*/
	Process * NewP = new Process();
	NewP->m_Pid = pid;
	NewP->m_ParentPid = 0;
	NewP->m_ProcessFileImage = path;
	NewP->m_UnixPwd = Path("/");

	//add to the kernel
	g_pKernelTable->m_Processes.push_back(NewP);


	//env and args
	for(NewP->m_ArgCnt=0; InitialArguments[NewP->m_ArgCnt]!=NULL; NewP->m_ArgCnt++) {}
	for(NewP->m_EnvCnt=0; InitialEnvironment[NewP->m_EnvCnt]!=NULL; NewP->m_EnvCnt++) {}
	//StartNewImageRunning expects to be able to free these - so copy them - in the kernel still
	NewP->m_Arguments   = MemoryHelper::CopyStringListBetweenProcesses(GetCurrentProcess(), (ADDR)InitialArguments, GetCurrentProcess(), NULL, &NewP->m_ArgCnt, NULL);
	NewP->m_Environment = MemoryHelper::CopyStringListBetweenProcesses(GetCurrentProcess(), (ADDR)InitialEnvironment, GetCurrentProcess(), NULL, &NewP->m_EnvCnt, NULL);

	//events to co-ordinate starting
	NewP->m_hProcessStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	NewP->m_hForkDoneEvent = NULL; //not forking

	//need to start the process and debug it (to trap kernel calls etc)
	//the debugger thread
	NewP->m_KernelThreadHandle = CreateThread(NULL, KERNEL_PROCESS_THREAD_STACK_SIZE, KernelProcessHandlerEntry, NewP, 0, &NewP->m_KernelThreadId);
	if(NewP->m_KernelThreadHandle==NULL)
	{
		ktrace("failed to start debug thread for process\n");
		delete NewP;
		return NULL;
	}

	//Wait for the thread to start the process, or die
	DWORD dwRet = WaitForSingleObject(NewP->m_hProcessStartEvent, INFINITE);
	CloseHandle(NewP->m_hProcessStartEvent);
	NewP->m_hProcessStartEvent=NULL;

	//check it's ok
	if(NewP->m_hProcess==NULL)
	{
		ktrace("failed to start the process\n");
		delete NewP;
		return NULL;
	}

	//running, this is it
	return NewP;
}

//Create a new process by forking an existing one
//The pid is allocated as next available pid
//Parent becomes the parent process
//Otherwise the new process is a clone
//
Process* Process::StartFork(Process * pParent)
{
	ktrace("StartFork()\n");

	g_pKernelTable->m_ForksSinceBoot++;

	/*
	launch win32 process stub
	copy over memory
	set stack/cpu contexts
	let process run
	*/
	Process * NewP = new Process();

	//allocate a new pid
	int cnt=0;
	while(++g_pKernelTable->m_LastPID)
	{
		if(++cnt > 10000)
		{
			ktrace("not pid's available for fork\n");
			delete NewP;
			return NULL;
		}

		//loop around pid's?
		if(g_pKernelTable->m_LastPID > 0xFFFF)
			g_pKernelTable->m_LastPID = 2; //MUST skip '1' which is only ever for init

		//free?
		if(g_pKernelTable->FindProcess(g_pKernelTable->m_LastPID) == NULL)
			break;
	}
	NewP->m_Pid = g_pKernelTable->m_LastPID;

	//parent is who?
	NewP->m_ParentPid = P->m_Pid;


	//add to the kernel
	g_pKernelTable->m_Processes.push_back(NewP);


	//events to co-ordinate starting
	NewP->m_hProcessStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	NewP->m_hForkDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//need to start the process and debug it (to trap kernel calls etc)
	//the debugger thread
	NewP->m_KernelThreadHandle = CreateThread(NULL, KERNEL_PROCESS_THREAD_STACK_SIZE, KernelProcessHandlerEntry, NewP, 0, &NewP->m_KernelThreadId);
	if(NewP->m_KernelThreadHandle==NULL)
	{
		ktrace("failed to start debug thread for process\n");
		delete NewP;
		return NULL;
	}

	//Wait for the thread to start the process, or die
	DWORD dwRet = WaitForSingleObject(NewP->m_hProcessStartEvent, INFINITE);
	CloseHandle(NewP->m_hProcessStartEvent);
	NewP->m_hProcessStartEvent=NULL;

	//check it's ok
	if(NewP->m_hProcess==NULL)
	{
		ktrace("failed to start the process\n");
		delete NewP;
		return NULL;
	}


	//Wait for the new process to finish forking our state
	ktrace("Waiting for child to finish copying our state\n");
	dwRet = WaitForSingleObject(NewP->m_hForkDoneEvent, INFINITE);
	CloseHandle(NewP->m_hForkDoneEvent);
	NewP->m_hForkDoneEvent=NULL;


	//running, this is it
	return NewP;
}


//Handler Thread Entry
/*static*/ DWORD WINAPI Process::KernelProcessHandlerEntry(LPVOID param)
{
	//need thread stuff
	g_pTraceBuffer = new char [KTRACE_BUFFER_SIZE];
	P = (Process*)param;

	P->m_bInWin32Setup = true;

	//Need COM
	CoInitialize(NULL);//Ex(NULL, COINIT_MULTITHREADED);

	//Start the stub
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	GetStartupInfo(&si);
	if(CreateProcess(NULL, (char*)KEOW_PROCESS_STUB, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS|DEBUG_PROCESS, NULL, P->m_UnixPwd.GetWin32Path(), &si, &pi) == FALSE)
	{
		//failed
		P->m_hProcess = NULL;
		SetEvent(P->m_hProcessStartEvent);
		return 0;
	}
	P->m_hProcess    = pi.hProcess;
	P->m_dwProcessId = pi.dwProcessId;
	P->m_ThreadList.push_back( new ThreadInfo(pi.dwThreadId, pi.hThread) );

	//started now
	SetEvent(P->m_hProcessStartEvent);

	//We run the debug loop at higher priority to hopefully receive and handle
	//child system calls faster. The child is doing all the normal processing
	//so this should not end up elevating it's priority and system impact
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);


	//
	//the debugging loop
	//
	P->DebuggerLoop();
	ktrace("process handler debug loop exitted\n");


	//parent should get SIGCHLD when we exit
	if(P->m_Pid != 1)
	{
		Process * pParent = g_pKernelTable->FindProcess(P->m_ParentPid);
		ThreadInfo * pParentThread = pParent->m_ThreadList[0];
		if(pParent)
			pParent->SendSignal(linux::SIGCHLD);
	}

	//any children get inherited by init
	//also we are not ptracing any more
	KernelTable::ProcessList::iterator it;
	for(it=g_pKernelTable->m_Processes.begin();
	    it!=g_pKernelTable->m_Processes.end();
		++it)
	{
		Process * p2 = *it;
		if(p2->m_ParentPid == P->m_Pid)
			p2->m_ParentPid = 1; //init
		if(p2->m_ptrace.OwnerPid == P->m_Pid)
			p2->m_ptrace.OwnerPid = 0; //no-one
	}

	//cleanup
	//leave the process in the kernel table - it's parent will clean it up

	//clean up thread local stuff
	delete g_pTraceBuffer;
	P=NULL;

	CoUninitialize();

	return 0;
}


void Process::DebuggerLoop()
{
	DEBUG_EVENT evt;
	for(;;)
	{
		WaitForDebugEvent(&evt, INFINITE);

		//what thread?
		T = P->GetThreadInfo(evt.dwThreadId);

		//any signals to dispatch first?
		int sig=0;
		for(; sig<linux::_NSIG; ++sig)
		{
			if(m_PendingSignals[sig])
			{
				m_PendingSignals[sig] = false;
				break;
			}
		}
		if(sig<linux::_NSIG) //found a pending signal
		{
			m_CurrentSignal = sig;
			HandleSignal(sig);
			m_CurrentSignal = 0;

			//debuging in signal handler now - abort original reason for debug
			ResumeKeowProcess(evt, true);
			continue;
		}


		//handle the real reason we are in the debugger
		bool bNeedToRestoreGS = false;
		switch(evt.dwDebugEventCode)
		{
		case CREATE_PROCESS_DEBUG_EVENT:
			CloseHandle(evt.u.CreateProcessInfo.hFile); //don't need this
			GetProcessTimes(P->m_hProcess, &P->m_BaseTimes.ftCreateTime, &P->m_BaseTimes.ftExitTime, &P->m_BaseTimes.ftKernelTime, &P->m_BaseTimes.ftUserTime);
			bNeedToRestoreGS = false;
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			m_dwExitCode = evt.u.ExitProcess.dwExitCode;
			m_bStillRunning = false; //parent will clean up the process table entry
			ktrace("process exit debug event\n");
			bNeedToRestoreGS = false;
			return; //no longer debugging


		case CREATE_THREAD_DEBUG_EVENT:
		case EXIT_THREAD_DEBUG_EVENT:
			bNeedToRestoreGS = false;
			break;


		case EXCEPTION_DEBUG_EVENT:
			if(m_bInWin32Setup)
			{
				bNeedToRestoreGS = false;
				switch(evt.u.Exception.ExceptionRecord.ExceptionCode)
				{
				case EXCEPTION_SINGLE_STEP:
					ConvertProcessToKeow();
					SetSingleStep(false,NULL); //DEBUG - keep it up
					break;
				case EXCEPTION_BREAKPOINT:
					//seems to get generated when kernel32 loads.
					//ignore it while we are still in setup
					break;
				default:
					//ignore?
					break;
				}
			}
			else
			{
				HandleException(evt);
				bNeedToRestoreGS = true;
			}
			break;


		case LOAD_DLL_DEBUG_EVENT:
		case UNLOAD_DLL_DEBUG_EVENT:
			bNeedToRestoreGS = false;
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			{
				int bufLen = evt.u.DebugString.nDebugStringLength*2;
				char * buf = new char[bufLen];

				ReadMemory(buf, (ADDR)evt.u.DebugString.lpDebugStringData, evt.u.DebugString.nDebugStringLength);
				buf[evt.u.DebugString.nDebugStringLength] = 0;

				ktrace("relay: %*s", evt.u.DebugString.nDebugStringLength, buf);

				delete buf;
				bNeedToRestoreGS = false;
			}
			break;

		default:
			ktrace("ignoring unhandled debug event code %ld\n", evt.dwDebugEventCode);
			break;
		}


		//continue running
		ResumeKeowProcess(evt, bNeedToRestoreGS);

	}//for(;;)
}

/* Resume process after a Debug event */
void Process::ResumeKeowProcess(DEBUG_EVENT &evt, bool bNeedToRestoreGS)
{
	/*
	 Windows resets the GS register on all API calls (reset to zero)
	 But linux pthreads use GS extensively, so we need to preserve it

	 They way we do this is to arrange that when we return control to the code
	 we return to a peice of logic that restores GS first.
	*/
	if(bNeedToRestoreGS) {
		//Inject code (on the processes stack) that ensures the appropriate
		// segment register (eg GS) gets set with a value.
		//
		// We place values on the stack, and this code:
		//    0F A9        pop gs   ;set the register
		//    C2 06 00     ret 6    ;remove our code from the stack too (WORD+DWORD in StackFrame)
		//    00                    ;extra byte in last dword of StackFrame.ret
		//
		struct {
			DWORD  segGs;  //"pop gs" pops 4 bytes, even though gs is 16bit 
			DWORD retAddress;
			WORD  pop; 
			DWORD ret; 
		} StackFrame;

		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_CONTROL | CONTEXT_SEGMENTS;
		::GetThreadContext( T->hThread, &ctx);

		StackFrame.segGs = T->SegGs; //ctx.SegGs;
		StackFrame.retAddress = ctx.Eip;
		StackFrame.pop = 0xA90F;
		StackFrame.ret = 0x000006C2;

		//Inject onto the processes stack and adjust EIP to run it
		ctx.Esp -= sizeof(StackFrame);
		ctx.Eip = ctx.Esp + ((DWORD)&StackFrame.pop - (DWORD)&StackFrame); // ==> StackFrame.pop instruction

		::SetThreadContext(T->hThread, &ctx);
		WriteMemory((ADDR)ctx.Esp, sizeof(StackFrame), &StackFrame);

		ktrace("Inject GS : 0x%02x\n", StackFrame.segGs);
	}

	//Continue the process
	FlushInstructionCache(m_hProcess, 0, 0); //need this for our process modifications?
	ContinueDebugEvent(evt.dwProcessId, evt.dwThreadId, DBG_CONTINUE);
}


//Process starts as a win32 stub, need to set up a unix-like process in it
//
void Process::ConvertProcessToKeow()
{
	//Get the info about the win32 side, before changing to keow/unix
	//The stub provides this via the loading of the KeowUserSysCalls dll
	//The info is passed as a pointer in Eax and causing an initial breakpoint 

	m_bInWin32Setup = true; //not done just yet

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_FULL;
	GetThreadContext( T->hThread, &ctx);

	//This is the details we use as win32 state for code injection etc
	//keep this seperate from the keow stuff that happens later (eg FS reg changes)
	m_BaseWin32Ctx = ctx;

	GetProcessTimes(P->m_hProcess, &P->m_BaseTimes.ftCreateTime, &P->m_BaseTimes.ftExitTime, &P->m_BaseTimes.ftKernelTime, &P->m_BaseTimes.ftUserTime);


	//read syscall data from dll
	ReadMemory(&SysCallAddr, (ADDR)ctx.Ebx, sizeof(SysCallAddr));

#ifdef _DEBUG
	//ensure correct coding - all functions are populated?
	for(int a=0; a<sizeof(SysCallAddr); a+=sizeof(DWORD))
	{
		DWORD * pa = (DWORD*)( ((LPBYTE)&SysCallAddr) + a );
		if(*pa == 0)
		{
			ktrace("Bad SysCallDll coding in dll\n");
			DebugBreak();
		}
	}
#endif

	//disable single-step that the stub started (to alert us)
	SetSingleStep(false, &ctx);
	SetThreadContext( T->hThread, &ctx);


	//are we forking or starting init?

	if(m_hForkDoneEvent!=NULL)
	{
		//forking

		Process * pParent = g_pKernelTable->FindProcess(m_ParentPid);
		ThreadInfo * pParentThread = pParent->m_ThreadList[0]; //TODO: get correct thread that was forked!
		ForkCopyOtherProcess(*pParent, *pParentThread);

		m_bInWin32Setup = false; //done now
		SetEvent(m_hForkDoneEvent);
		return;
	}
	//starting init


	//want a new stack for the process
	// - at the TOP of the address space and top being like 0xbfffffff ???
	//[I got this value from gdb of /bin/ls on debian i386 linux]
	//also seems that some libpthread code relies on (esp|0x1ffff) being the top of the stack.
	//TODO: alloc dynamically within the contraints
	DWORD size=1024*1024L;  //1MB will do?
	ADDR stack_bottom = (ADDR)0x70000000 - size; //ok location?
	m_KeowUserStackBase = MemoryHelper::AllocateMemAndProtect(stack_bottom, size, PAGE_EXECUTE_READWRITE);//we sometimes inject executable code into the stack!
	m_KeowUserStackTop = m_KeowUserStackBase + size;
	//linux process start with a little bit of stack in use but overwritable?
	ctx.Esp = (DWORD)m_KeowUserStackTop - 0x200;
	//set new stack
	SetThreadContext(T->hThread, &ctx);

	//not fork or exec, must be 'init', the very first process
	ktrace("init/exec: load image: %s\n", m_ProcessFileImage.GetWin32Path().c_str());
	LoadImage(m_ProcessFileImage, false);

	//what new kernel call mechanism
	string dso(g_pKernelTable->m_KeowExeDir);
	dso += "\\keow-gate.dso";
	HANDLE hImg = CreateFile(dso, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	struct linux::elf32_hdr ehdr; //ELF header
	Process::ElfLoadData LoadData;
	LoadData = m_ElfLoadData;
	DWORD dummy;
	ReadFile(hImg, &ehdr, sizeof(ehdr), &dummy, NULL);
	if( LoadElfImage(hImg, &ehdr, &LoadData, true) == 0)
	{
		m_LinuxGateDSO = (DWORD)LoadData.image_base;
		m_LinuxGateVSyscall = (DWORD)LoadData.start_addr;
	}
	CloseHandle(hImg);

	//std in,out,err
	m_OpenFiles[0] = new IOHNtConsole(g_pKernelTable->m_pMainConsole);
	m_OpenFiles[1] = new IOHNtConsole(g_pKernelTable->m_pMainConsole);
	m_OpenFiles[2] = new IOHNtConsole(g_pKernelTable->m_pMainConsole);
	m_OpenFiles[0]->SetInheritable(true);
	m_OpenFiles[1]->SetInheritable(true);
	m_OpenFiles[2]->SetInheritable(true);

	m_bInWin32Setup = false; //done now

	//make the stub execute the image
	StartNewImageRunning();
}


//This is either a fault to signal to the process,
//or it's a kernel call to intercept
//
void Process::HandleException(DEBUG_EVENT &evt)
{
	/* From Winnt.H for reference
		#define CONTEXT_CONTROL         (CONTEXT_i386 | 0x00000001L) // SS:SP, CS:IP, FLAGS, BP
		#define CONTEXT_INTEGER         (CONTEXT_i386 | 0x00000002L) // AX, BX, CX, DX, SI, DI
		#define CONTEXT_SEGMENTS        (CONTEXT_i386 | 0x00000004L) // DS, ES, FS, GS
		#define CONTEXT_FLOATING_POINT  (CONTEXT_i386 | 0x00000008L) // 387 state
		#define CONTEXT_DEBUG_REGISTERS (CONTEXT_i386 | 0x00000010L) // DB 0-3,6,7
		#define CONTEXT_EXTENDED_REGISTERS  (CONTEXT_i386 | 0x00000020L) // cpu specific extensions

		#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER |\
							  CONTEXT_SEGMENTS)
	*/

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS | CONTEXT_EXTENDED_REGISTERS;
	GetThreadContext( T->hThread, &ctx);
	T->SegGs = ctx.SegGs;

	switch(evt.u.Exception.ExceptionRecord.ExceptionCode)
	{
	case EXCEPTION_SINGLE_STEP:
		DumpContext(ctx);
		SetSingleStep(true, &ctx); //keep it up (Debug only?)
		break;

	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_STACK_CHECK:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		ktrace("math exception\n");
		SendSignal(linux::SIGFPE);
		break;

	case EXCEPTION_INT_OVERFLOW:
		{
			//is this an INT 80 linux SYSCALL?
			//--Actually is it the replacement INT 4 instruction that we placed there?
			//  Or is it a real overflow exception?

			WORD instruction;
			ReadMemory(&instruction, (ADDR)(ctx.Eip-2), sizeof(instruction));
			if(instruction == 0x04CD) //Int 4 - overflow exception - see InterceptInt80Calls()
			{
				//this exception is raised with EIP already pointing after the instruction
				//no need to increment EIP

				//handle int 80h
				SysCalls::HandleInt80SysCall(ctx);
			}
			else
			{
				ReadMemory(&instruction, (ADDR)(ctx.Eip-1), sizeof(instruction));
				if(instruction == 0x04CD) //Int 4 - overflow exception - see InterceptInt80Calls()
				{
					//this exception is raised with EIP already pointing after the instruction
					ctx.Eip += 1;

					//handle int 80h
					SysCalls::HandleInt80SysCall(ctx);
				}
				else
				{
					//genuine exception
					ktrace("math exception\n");
					ktrace("(instruction = 0x%04lX)\n", instruction);
					SendSignal(linux::SIGFPE);
				}
			}
		}
		break;

	case EXCEPTION_ACCESS_VIOLATION:
		{
			//is this an INT 80 linux SYSCALL?
			//or a replacement instruction that we placed there?

			WORD instruction;
			ReadMemory(&instruction, (ADDR)evt.u.Exception.ExceptionRecord.ExceptionAddress, sizeof(instruction));

			if(instruction == 0x80CD  //INT 80h
			|| instruction == 0x04CD) //Int 4 - overflow exception - see InterceptInt80Calls()
			{
				//skip over the INT instruction
				//so that the process resumes on the next instruction
				ctx.Eip += 2;

				//handle int 80h
				SysCalls::HandleInt80SysCall(ctx);
			}
			else
			{
				if(MemoryHelper::HandlePossibleLDTException(instruction, (ADDR)evt.u.Exception.ExceptionRecord.ExceptionAddress, ctx)) {
					break;
				}
				else {
					//Seems to be a real exception

					ktrace("Access violation @ 0x%08lx\n", evt.u.Exception.ExceptionRecord.ExceptionAddress);
					ktrace("instruction = 0x%04lX\n", instruction);
					ktrace("thread = 0x%04lX\n", evt.dwThreadId);

					DumpContext(ctx);
					DumpStackTrace(ctx);

					SendSignal(linux::SIGSEGV); //access violation
				}
			}
		}
		break;

	case EXCEPTION_PRIV_INSTRUCTION:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		ktrace("instruction exception\n");
		SendSignal(linux::SIGILL);
		break;

	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	case EXCEPTION_IN_PAGE_ERROR:
	case EXCEPTION_STACK_OVERFLOW:
	case EXCEPTION_GUARD_PAGE:
		ktrace("page access exception\n");
		SendSignal(linux::SIGSEGV);
		break;

	case EXCEPTION_BREAKPOINT:
		DebugBreak();
		//TODO: if ptrace send to parent, else terminate signal
		break;

	default:
		ktrace("Unhandled exception type 0x%08lx @ 0x%08lx\n", evt.u.Exception.ExceptionRecord.ExceptionCode, evt.u.Exception.ExceptionRecord.ExceptionAddress);
		SendSignal(linux::SIGSEGV);
		break;
	}


	//SetSingleStep(true); //DEBUG - keep it up

	//set any context changes
	SetThreadContext( T->hThread, &ctx);
}


// Clone an existing process into this one,
// except for a few things (eg ptrace, pid, ppid)
void Process::ForkCopyOtherProcess(Process &other, ThreadInfo &otherThread)
{
	int i;
	ktrace("fork : Copying state from parent process\n");

	m_ProcessGroupPID = other.m_ProcessGroupPID;

	m_gid = other.m_gid;
	m_uid = other.m_uid;
	m_egid = other.m_egid;
	m_euid = other.m_euid;

	m_saved_uid = other.m_saved_uid;
	m_saved_gid = other.m_saved_gid;

	m_umask = other.m_umask;

	//use same thread (it'll be valid after we copy the other processes memory allocations)
	m_KeowUserStackBase = other.m_KeowUserStackBase;
	m_KeowUserStackTop = other.m_KeowUserStackTop;

	m_ProcessFileImage = other.m_ProcessFileImage;
	m_UnixPwd = other.m_UnixPwd;

	m_ElfLoadData = other.m_ElfLoadData;

	//DO NOT copy the ptrace state on a fork
	//m_ptrace;

	//these are valid after memory copy
	m_Environment = other.m_Environment;
	m_Arguments = other.m_Arguments;
	m_ArgCnt = other.m_ArgCnt;
	m_EnvCnt = other.m_EnvCnt;

	//signal handling
	memcpy(&m_PendingSignals, &other.m_PendingSignals, sizeof(m_PendingSignals));
	memcpy(&m_SignalMask, &other.m_SignalMask, sizeof(m_SignalMask));
	memcpy(&m_SignalAction, &other.m_SignalAction, sizeof(m_SignalAction));

	//copy memory allocations (except mmap)
	MemoryAllocationsList::iterator mem_it;
	for(mem_it = other.m_MemoryAllocations.begin();
	    mem_it != other.m_MemoryAllocations.end();
		++mem_it)
	{
		MemoryAlloc * pMemAlloc = *mem_it;

		ADDR p = MemoryHelper::AllocateMemAndProtect(pMemAlloc->addr, pMemAlloc->len, pMemAlloc->protection);
		MemoryHelper::TransferMemory(other.m_hProcess, pMemAlloc->addr, m_hProcess, p, pMemAlloc->len);
	}

	//clone files
	for(i=0; i<MAX_OPEN_FILES; ++i)
	{
		if(other.m_OpenFiles[i]==NULL)
			continue;

		m_OpenFiles[i] = other.m_OpenFiles[i]->Duplicate();
	}

	//copy mmap'd memory
	MmapList::iterator mmap_it;
	for(mmap_it = other.m_MmapList.begin();
	    mmap_it != other.m_MmapList.end();
		++mmap_it)
	{
		MMapRecord * pMmap = *mmap_it;

		DebugBreak(); //TODO: handle cloning mmaps 
	}

		
	//Clone the context
	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS | CONTEXT_EXTENDED_REGISTERS;
	GetThreadContext(otherThread.hThread, &ctx);

	//we are the child - need zero return from the fork
	ctx.Eip += 2; //skip the Int 80
	ctx.Eax = 0; //we are the child

	SetThreadContext(T->hThread, &ctx);

	ktrace("fork : copy done\n");
}


//Load the process executable images
//and then set the process to run
//
DWORD Process::LoadImage(Path &img, bool LoadAsLibrary)
{
	DWORD rc;

	HANDLE hImg = CreateFile(img.GetWin32Path().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(hImg==INVALID_HANDLE_VALUE)
	{
		DWORD dwErr = GetLastError();
		ktrace("image not found - err %d\n",dwErr);
		return dwErr;
	}

	//Read the first chunk of the file
	int buflen=1024; //plenty enough to understand the file - enough for elf header and also the 128 chars allowed for a #! shell script
	DWORD dwRead;
	BYTE * buf = new BYTE[buflen];
	memset(buf,0,buflen);
	ReadFile(hImg, buf, buflen, &dwRead, 0);

	//determine exe type
	struct linux::elf32_hdr * pElf; //ELF header
	pElf = (struct linux::elf32_hdr *)buf;
	if(pElf->e_ident[linux::EI_MAG0] == linux::ELFMAG0
	&& pElf->e_ident[linux::EI_MAG1] == linux::ELFMAG1
	&& pElf->e_ident[linux::EI_MAG2] == linux::ELFMAG2
	&& pElf->e_ident[linux::EI_MAG3] == linux::ELFMAG3)
	{
		rc = LoadElfImage(hImg, pElf, &m_ElfLoadData, LoadAsLibrary);
	}
	else
	if(buf[0]=='#'
	&& buf[1]=='!')
	{
		//yes - it is a shell script with a header
		// eg: #! /bin/sh -x
		//Use that program as the process and apppend our original args

		char * nl = strchr((char*)buf, 0x0a);
		if(nl==NULL)
			nl = (char*)&buf[buflen-1]; //use end of buffer
		*nl = NULL; //line ends here

		//skip #! and any whitespace
		char * interp = (char*)buf + 2;
		while(*interp == ' ')
			interp++;

		//process the command and arguments
		list<string> arglist;
		ParseCommandLine(interp, arglist);

		rc = LoadImage(Path(arglist[0]), false); //scripts are never libraries

		//At this point the arguments are still in the kernel
		//Prepend the interpreter arguments to the list

		//TODO: this is a hack - think about it and make it better
		//need list of pointers
		const char **pTmpAddr = new const char*[m_ArgCnt+arglist.size()+1];
		//copy new pointers
		list<string>::iterator it;
		int i;
		for(i=0,it=arglist.begin(); it!=arglist.end(); ++i,++it)
		{
			pTmpAddr[i] = (*it).c_str();
		}
		//original pointers plus the null
		memcpy(&pTmpAddr[i], m_Arguments, sizeof(ADDR)*(m_ArgCnt+1));
		//new copy
		ADDR pOldArgs = m_Arguments;
		m_Arguments = MemoryHelper::CopyStringListBetweenProcesses(GetCurrentProcess(), (ADDR)pTmpAddr, GetCurrentProcess(), NULL, &m_ArgCnt, NULL);
		//free old kernel copy
		//leak! VirtualFree(pOldArgs, dwArgsSize, MEM_DECOMMIT);
	}
	else
	{
		//unhandled file format
		ktrace("bad/unhandled image format\n");
		rc = ERROR_BAD_FORMAT;
	}


	//done with handles
	delete buf;
	CloseHandle(hImg);

	return rc;
}


bool Process::CanLoadImage(Path &img, bool LoadAsLibrary)
{
	//can load it?
	HANDLE hImg = CreateFile(img.GetWin32Path().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(hImg==INVALID_HANDLE_VALUE)
		return false;


	//Read the first chunk of the file
	int buflen=1024; //plenty enough to understand the file - enough for elf header and also the 128 chars allowed for a #! shell script
	DWORD dwRead;
	BYTE * buf = new BYTE[buflen];
	memset(buf,0,buflen);
	ReadFile(hImg, buf, buflen, &dwRead, 0);

	CloseHandle(hImg);

	//determine exe type
	struct linux::elf32_hdr * pElf; //ELF header
	pElf = (struct linux::elf32_hdr *)buf;
	if(pElf->e_ident[linux::EI_MAG0] == linux::ELFMAG0
	&& pElf->e_ident[linux::EI_MAG1] == linux::ELFMAG1
	&& pElf->e_ident[linux::EI_MAG2] == linux::ELFMAG2
	&& pElf->e_ident[linux::EI_MAG3] == linux::ELFMAG3)
	{
		return true; //loadable we think - should really test intepreter etc but we don't yet
	}
	else
	if(buf[0]=='#'
	&& buf[1]=='!')
	{
		//yes - it is a shell script with a header
		// eg: #! /bin/sh -x
		//Use that program as the process and append our original args

		char * nl = strchr((char*)buf, 0x0a);
		if(nl==NULL)
			nl = (char*)&buf[buflen-1]; //use end of buffer
		*nl = NULL; //line ends here

		//skip #! and any whitespace
		char * interp = (char*)buf + 2;
		while(*interp == ' ')
			interp++;

		//process the command and arguments
		list<string> arglist;
		ParseCommandLine(interp, arglist);

		return CanLoadImage(Path(arglist[0]), false); //can load interpreter?
	}
	else
	{
		//unhandled file format
		SetLastError(ERROR_BAD_FORMAT);
		return false;
	}

	//shouldn't get here
	return false;
}


DWORD Process::LoadElfImage(HANDLE hImg, struct linux::elf32_hdr * pElfHdr, ElfLoadData * pElfLoadData, bool LoadAsLibrary)
{
	//Validate the ELF file
	//

	if(pElfHdr->e_type != linux::ET_EXEC
	&& pElfHdr->e_type != linux::ET_DYN)
	{
		ktrace("not an ELF executable type\n");
		return ERROR_BAD_FORMAT;
	} 
	if(pElfHdr->e_machine != linux::EM_386)
	{
		ktrace("not for Intel 386 architecture (needed for syscall interception)\n");
		return ERROR_BAD_FORMAT;
	}
	if(pElfHdr->e_version != linux::EV_CURRENT
	|| pElfHdr->e_ident[linux::EI_VERSION] != linux::EV_CURRENT )
	{
		ktrace("not version %d\n", linux::EV_CURRENT);
		return ERROR_BAD_FORMAT;
	}


	//Load the image into the keow stub process
	//
	int i;
	linux::Elf32_Phdr * phdr; //Program header
	ADDR pMem, pWantMem, pAlignedMem;
	DWORD AlignedSize;
	DWORD protection;//, oldprot;
	DWORD loadok;
	ADDR pMemTemp = NULL;
	ADDR pBaseAddr = 0;

	if(pElfHdr->e_phoff == 0)
		return -1; //no header

	//what about these??
	if(LoadAsLibrary || pElfHdr->e_type==linux::ET_DYN)
	{
		if(pElfLoadData->last_lib_addr == 0)
			pBaseAddr = (ADDR)0x40000000L; //linux 2.4 x86 uses this for start of libs?
		else
			pBaseAddr = pElfLoadData->last_lib_addr;
		//align to next 64k boundary
		pBaseAddr = (ADDR)( ((DWORD)pBaseAddr + (SIZE64k-1)) & (~(SIZE64k-1)) ); 
		pElfLoadData->interpreter_base = pBaseAddr;
	}
	ktrace("using base address 0x%08lx\n", pBaseAddr);

	pElfLoadData->Interpreter[0] = 0;
	pElfLoadData->start_addr = pElfHdr->e_entry + pBaseAddr;
	pElfLoadData->image_base = pBaseAddr;	

	loadok=1;

	phdr = (linux::Elf32_Phdr*)new char[pElfHdr->e_phentsize];
	if(phdr==0)
		loadok=0;

	//load program header segments
	//initial load - needs writable memory pages
	DWORD dwRead;
	for(i=0; loadok && i<pElfHdr->e_phnum; ++i)
	{
		SetFilePointer(hImg, pElfHdr->e_phoff + (i*pElfHdr->e_phentsize), 0, FILE_BEGIN);
		ReadFile(hImg, phdr, pElfHdr->e_phentsize, &dwRead, 0);

		if(phdr->p_type == linux::PT_LOAD)
		{
			//load segment into memory
			protection = ElfProtectionToWin32Protection(phdr->p_flags);
			pWantMem = phdr->p_vaddr + pBaseAddr;
			pAlignedMem = pWantMem;
			AlignedSize = phdr->p_memsz;
			if(phdr->p_align > 1)
			{
				//need to handle alignment
				//start addr rounds down
				//length rounds up

				pAlignedMem = (ADDR)( (DWORD)pAlignedMem & ~(phdr->p_align-1) );
				AlignedSize = (phdr->p_memsz + (pWantMem-pAlignedMem) + (phdr->p_align-1)) & ~(phdr->p_align-1);
			}

			ktrace("load segment file offset %d, file len %d, mem len %d (%d), to 0x%08lx (0x%08lx)\n", phdr->p_offset, phdr->p_filesz, phdr->p_memsz, AlignedSize, phdr->p_vaddr, pAlignedMem);

			pMem = MemoryHelper::AllocateMemAndProtect(pAlignedMem, AlignedSize, PAGE_EXECUTE_READWRITE);
			if(pMem!=pAlignedMem)
			{
				DWORD err = GetLastError();
				ktrace("error 0x%lx in load program segment @ 0x%lx (got 0x%lx)\n", err, pAlignedMem, pMem);
				loadok = 0;
				break;
			}

			//need to zero first? (alignment creates gaps needing filled with zeros)
			MemoryHelper::FillMem(P->m_hProcess, pAlignedMem, AlignedSize, 0);

			//need to load file into our memory, then copy to process
			if(phdr->p_filesz != 0)
			{
				pMemTemp = (ADDR)realloc(pMemTemp, phdr->p_filesz);
				SetFilePointer(hImg, phdr->p_offset, 0, FILE_BEGIN);
				ReadFile(hImg, pMemTemp, phdr->p_filesz, &dwRead, 0);

				if(dwRead != phdr->p_filesz)
				{
					DWORD err = GetLastError();
					ktrace("error 0x%lx in load program segment, read %d (got %d)\n", err, dwRead, phdr->p_filesz);
					loadok = 0;
					break;
				}

				//load into ask addr, not the aligned one
				if(!WriteMemory(pWantMem, phdr->p_filesz, pMemTemp))
				{
					DWORD err = GetLastError();
					ktrace("error 0x%lx in write program segment\n", err);
					loadok = 0;
					break;
				}
			}


			if(!LoadAsLibrary)
			{
				//keep track largest filled size and allocated size
				//between start_bss and last_bss is the bs section.
				if(pMem+phdr->p_filesz  > pElfLoadData->bss_start)
					pElfLoadData->bss_start = pMem + phdr->p_filesz;
				if(pAlignedMem+AlignedSize > pElfLoadData->brk)
					pElfLoadData->brk = pAlignedMem+AlignedSize;

				//keep track in min/max addresses
				if(pMem<pElfLoadData->program_base || pElfLoadData->program_base==0)
					pElfLoadData->program_base = pMem;
				if(pMem+phdr->p_memsz > pElfLoadData->program_max)
					pElfLoadData->program_max = pMem+phdr->p_memsz;
			}
			else
			{
				if(pMem+phdr->p_memsz  > pElfLoadData->last_lib_addr)
					pElfLoadData->last_lib_addr = pMem + phdr->p_memsz;
			}

		}
		else
		if(phdr->p_type == linux::PT_PHDR)
		{
			//will be loaded later in a PT_LOAD
			pElfLoadData->phdr_addr = phdr->p_vaddr + pBaseAddr;

			pElfLoadData->phdr_phnum = pElfHdr->e_phnum;
			pElfLoadData->phdr_phent = pElfHdr->e_phentsize;
		}
		else
		if(phdr->p_type == linux::PT_INTERP)
		{
			SetFilePointer(hImg, phdr->p_offset, 0, FILE_BEGIN);
			ReadFile(hImg, pElfLoadData->Interpreter, phdr->p_filesz, &dwRead, 0);
		}
	}

	//align brk to a page boundary
	//NO!! //pElf->brk = (void*)( ((DWORD)pElf->brk + (SIZE4k-1)) & (~(SIZE4k-1)) ); 

	//load interpreter?
	if(pElfLoadData->Interpreter[0]!=0)
	{
		if(strcmp(pElfLoadData->Interpreter, "/usr/lib/libc.so.1")==0
		|| strcmp(pElfLoadData->Interpreter, "/usr/lib/ld.so.1")==0 )
		{
			//IBCS2 interpreter? (tst from linux binfmt_elf.c)
			//I think these expect to use intel Call Gates 
			//we cannot (yet?) do these (only int80) so fail
			ktrace("Cannot handle IBCS executables");
			loadok = 0;
		}

		//need a copy of the load data
		//the interpreter uses this to know where it can load to
		//and then we use the interpreter load results to update
		//the real process load data.
		ElfLoadData LoadData2;
		LoadData2 = *pElfLoadData;

		Path InterpPath(pElfLoadData->Interpreter);
		HANDLE hInterpImg = CreateFile(InterpPath.GetWin32Path(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if(hInterpImg==INVALID_HANDLE_VALUE)
		{
			ktrace("failed to load interpreter: %s\n", InterpPath.GetUnixPath().c_str());
			loadok = 0;
		}
		else
		{
			struct linux::elf32_hdr hdr2;
			ReadFile(hInterpImg, &hdr2, sizeof(hdr2), &dwRead, 0);

			DWORD rc = LoadElfImage(hInterpImg, &hdr2, &LoadData2, true);
			if(rc!=0)
				loadok=0;

			m_ElfLoadData.interpreter_start = LoadData2.start_addr;
			m_ElfLoadData.interpreter_base = LoadData2.interpreter_base;
			m_ElfLoadData.last_lib_addr = LoadData2.last_lib_addr;

			CloseHandle(hInterpImg);
		}

	}

	//protect all the pages
	/* does not work?
	for(i=0; loadok && i<pElfHdr->e_phnum; ++i)
	{
		if(phdr->p_type == PT_LOAD
		|| phdr->p_type == PT_PHDR)
		{
			fseek(fElf, pElfHdr->e_phoff + (i*pElfHdr->e_phentsize), SEEK_SET);
			fread(phdr, pElfHdr->e_phentsize, 1, fElf);

			if(phdr->p_vaddr == 0
			|| phdr->p_memsz == 0 )
				continue;

			pMem = CalculateVirtualAddress(phdr, pBaseAddr);
			protection = ElfProtectionToWin32Protection(phdr->p_flags);
			if(!LegacyWindows::VirtualProtectEx(pElf->pinfo.hProcess, pMem, phdr->p_memsz, protection, &oldprot))
			{
				DWORD err = GetLastError();
				ktrace("error 0x%lx in protect program segment @ 0x%lx (got 0x%lx)\n", err,pWantMem, pMem);
				loadok = 0;
				break;
			}
		}
	}
	*/

	delete phdr;
	free(pMemTemp); //used 'realloc', not 'new'
	if(loadok)
		return 0;

	ktrace("program load failed\n");
	return -1;
}


// Transfers control to the image to run it
// This is only for 'new' images (exec() etc)
//
DWORD Process::StartNewImageRunning()
{
	//Place some ELF interpreter start info onto the stack
	//and then jump to the start of the program
	/*
	When the userspace receives control, the stack layout has a fixed format.
	The rough order is this:

		   <arguments> <environ> <auxv> <string data>

	The detailed layout, assuming IA32 architecture, is this (Linux kernel
	series 2.2/2.4):

	  position            content                     size (bytes) + comment
	  ------------------------------------------------------------------------
	  stack pointer ->  [ argc = number of args ]     4
						[ argv[0] (pointer) ]         4   (program name)
						[ argv[1] (pointer) ]         4
						[ argv[..] (pointer) ]        4 * x
						[ argv[n - 1] (pointer) ]     4
						[ argv[n] (pointer) ]         4   (= NULL)

						[ envp[0] (pointer) ]         4
						[ envp[1] (pointer) ]         4
						[ envp[..] (pointer) ]        4
						[ envp[term] (pointer) ]      4   (= NULL)

						[ auxv[0] (Elf32_auxv_t) ]    8
						[ auxv[1] (Elf32_auxv_t) ]    8
						[ auxv[..] (Elf32_auxv_t) ]   8
						[ auxv[term] (Elf32_auxv_t) ] 8   (= AT_NULL vector)

						[ padding ]                   0 - 16

						[ argument ASCIIZ strings ]   >= 0
						[ environment ASCIIZ str. ]   >= 0

	  (0xbffffffc)      [ end marker ]                4   (= NULL)

	  (0xc0000000)      < top of stack >              0   (virtual)
	  ------------------------------------------------------------------------

	When the runtime linker (rtld) has done its duty of mapping and resolving
	all the required libraries and symbols, it does some initialization work
	and hands over the control to the real program entry point afterwards. As
	this happens, the conditions are:

			- All required libraries mapped from 0x40000000 on
			- All CPU registers set to zero, except the stack pointer ($sp) and
			  the program counter ($eip/$ip or $pc). The ABI may specify
			  further initial values, the i386 ABI requires that %edx is set to
			  the address of the DT_FINI function.

	 ...

	On Linux, the runtime linker requires the following Elf32_auxv_t
	structures:

			AT_PHDR, a pointer to the program headers of the executeable
			AT_PHENT, set to 'e_phentsize' element of the ELF header (constant)
			AT_PHNUM, number of program headers, 'e_phnum' from ELF header
			AT_PAGESZ, set to constant 'PAGE_SIZE' (4096 on x86)
			AT_ENTRY, real entry point of the executeable (from ELF header)

	*/

	GetProcessTimes(P->m_hProcess, &P->m_BaseTimes.ftCreateTime, &P->m_BaseTimes.ftExitTime, &P->m_BaseTimes.ftKernelTime, &P->m_BaseTimes.ftUserTime);

	if(m_ElfLoadData.interpreter_start==0)
	{
		m_ElfLoadData.interpreter_base = m_ElfLoadData.program_base;
		m_ElfLoadData.interpreter_start = m_ElfLoadData.start_addr;
		ktrace("elf, no interpreter: entry @ 0x%08lx\n", m_ElfLoadData.interpreter_start);
	}

	//when a new process first starts, it's args etc are in the kernel
	//transfer them to the process
	DWORD dwEnvSize, dwArgsSize;
	ADDR pEnv = MemoryHelper::CopyStringListBetweenProcesses(GetCurrentProcess(), m_Environment, m_hProcess, this, &m_EnvCnt, &dwEnvSize);
	ADDR pArgs = MemoryHelper::CopyStringListBetweenProcesses(GetCurrentProcess(), m_Arguments, m_hProcess, this, &m_ArgCnt, &dwArgsSize);
	//strings are now in the other process
	//free kernel copy
	VirtualFree(m_Arguments, dwArgsSize, MEM_DECOMMIT);
	VirtualFree(m_Environment, dwEnvSize, MEM_DECOMMIT);
	//point to the process
	m_Environment = pEnv;
	m_Arguments = pArgs;
	ktrace("%d args @ 0x%08lx\n", m_ArgCnt, m_Arguments);
	ktrace("%d env @ 0x%08lx\n", m_EnvCnt, m_Environment);

	//stack data needed
	const int AUX_RESERVE = 2*sizeof(DWORD)*20; //heaps for what is below
	int stack_needed = sizeof(ADDR)*(m_EnvCnt+1+m_ArgCnt+1) + AUX_RESERVE + sizeof(ADDR)/*end marker*/;

	ThreadInfo * pInitialThread = m_ThreadList[0];

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_FULL;
	GetThreadContext(pInitialThread->hThread, &ctx);

	ADDR StackTop = (ADDR)ctx.Esp;

	//stack grows DOWN
	ctx.Esp -= stack_needed;
	ktrace("writing initial stack frame @ 0x%08lx, len %d\n", ctx.Esp, stack_needed);

	//copy data to the stack
	//write so that first written are popped first
	ADDR addr = (ADDR)ctx.Esp;

	//argc
	WriteMemory(addr, sizeof(DWORD), &m_ArgCnt);
	addr += sizeof(DWORD);
	//argv[]: clone the array of pointers that are already in the target process
	//include the null end entry
	MemoryHelper::TransferMemory(m_hProcess, m_Arguments, m_hProcess, addr, (m_ArgCnt+1)*sizeof(ADDR));
	addr += (m_ArgCnt+1)*sizeof(ADDR);

	//envp[]: clone the array of pointers that are already in the target process
	//include the null end entry
	MemoryHelper::TransferMemory(m_hProcess, m_Environment, m_hProcess, addr, (m_EnvCnt+1)*sizeof(ADDR));
	addr += (m_EnvCnt+1)*sizeof(ADDR);

	//aux
	struct Elf32_auxv_t {
		DWORD type;	//pop this first
		DWORD val;	//2nd
	} Aux;

#define PUSH_AUX_VAL(t,v) \
		Aux.type = linux::t; \
		Aux.val = v; \
		WriteMemory(addr, sizeof(Aux), &Aux); \
		addr += sizeof(Aux);

	PUSH_AUX_VAL(AT_GID,	m_gid)
	PUSH_AUX_VAL(AT_UID,	m_uid)
	PUSH_AUX_VAL(AT_ENTRY,	(DWORD)m_ElfLoadData.start_addr) //real start - ignoring 'interpreter'
	PUSH_AUX_VAL(AT_BASE,	(DWORD)m_ElfLoadData.interpreter_base)
	PUSH_AUX_VAL(AT_PHNUM,	m_ElfLoadData.phdr_phnum)
	PUSH_AUX_VAL(AT_PHENT,	m_ElfLoadData.phdr_phent)
	PUSH_AUX_VAL(AT_PHDR,	(DWORD)m_ElfLoadData.phdr_addr)
	PUSH_AUX_VAL(AT_PAGESZ,	0x1000) //4K
//	PUSH_AUX_VAL(AT_SYSINFO_EHDR, m_LinuxGateDSO)
//	PUSH_AUX_VAL(AT_SYSINFO, m_LinuxGateVSyscall)
	PUSH_AUX_VAL(AT_NULL,	0)

#undef PUSH_AUX_VAL

	//end marker (NULL)
	DWORD zero=0;
	WriteMemory(addr, sizeof(DWORD), &zero);
	addr += sizeof(DWORD);


	//init required registers
	//(is this all we need?)
	ctx.Eax = 0;
	ctx.Ebx = 0;
	ctx.Ecx = 0;
	ctx.Edx = 0;
	ctx.Esi = 0;
	ctx.Edi = 0;
	ctx.Ebp = 0;

	//start here
	ctx.Eip = (DWORD)m_ElfLoadData.interpreter_start;

	//set
	SetThreadContext(pInitialThread->hThread, &ctx);

	ktrace("Transfer to ELF code done\n");
	return 0;
}

void Process::SetSingleStep(bool set, CONTEXT * pCtx)
{
	const int SINGLE_STEP_BIT = 0x100L; //8th bit is trap flag
	CONTEXT TmpCtx;
	bool bDoSet = false;

	if(pCtx==NULL)
	{
		TmpCtx.ContextFlags = CONTEXT_CONTROL;
		GetThreadContext(T->hThread, &TmpCtx);

		pCtx = &TmpCtx;
	}

	if(set)
	{
		//enable single-step 
		pCtx->EFlags |= SINGLE_STEP_BIT; 
	}
	else
	{
		//disable single-step 
		pCtx->EFlags &= ~SINGLE_STEP_BIT;
	}

	if(bDoSet)
		SetThreadContext(T->hThread, &TmpCtx);

}


bool Process::ReadMemory(LPVOID pBuf, ADDR addr, DWORD len)
{
	return MemoryHelper::ReadMemory((ADDR)pBuf, m_hProcess, addr, len);
}

bool Process::WriteMemory(ADDR addr, DWORD len, const void * pBuf)
{
	return MemoryHelper::WriteMemory(m_hProcess, addr, len, (ADDR)pBuf);
}


void Process::SendSignal(int sig)
{
	//deal with it immediatly (it's on this kernel thread)
	//or schedule it for the correct thread

	if(this==P)
	{
		//we are the thread handling this process
		m_CurrentSignal = sig;
		HandleSignal(sig);
		m_CurrentSignal = 0;
		return;
	}

	//send the signal to this process
	m_PendingSignals[sig] = true;
	SetEvent(m_hWaitTerminatingEvent);

	//NOTE: this may be executing on a different thread than the one handling this process

	//because it may not be in the debugger
	SuspendThread(T->hThread);

	//make single-step
	//this forces the debugger to intervene and then it can see the pending signals
	SetSingleStep(true, NULL);

	ResumeThread(T->hThread);
}


void Process::HandleSignal(int sig)
{
	ktrace("signal handler starting for %d\n", sig);

	//participate in ptrace()
	if(m_ptrace.OwnerPid)
	{
		ktrace("stopping for ptrace in HandleSignal\n");
		m_ptrace.new_signal = sig;
		g_pKernelTable->FindProcess(m_ptrace.OwnerPid)->SendSignal(linux::SIGCHLD);
		SuspendThread(GetCurrentThread()); //the debugger stops here while parent is ptracing

		ktrace("resumed from ptrace stop in HandleSignal\n");
		//did tracer alter the signal to deliver?
		if(sig != m_ptrace.new_signal)
		{
			sig = m_ptrace.new_signal;
			ktrace("ptrace replacement signal %d\n", sig);
			if(sig==0)
				return;
			//sig is different now
		}
	}

	//some signals cannot be caught
	switch(sig)
	{
	case linux::SIGKILL:
		ktrace("killed - sigkill\n");
		m_KilledBySig=sig;
		SysCallDll::exit(-sig);
		return;
	case linux::SIGSTOP:
		ktrace("stopping on sigstop\n");
		//suspend all threads
		for(Process::ThreadList::iterator it = m_ThreadList.begin();
			it != m_ThreadList.end();
			++it)
		{
			SuspendThread( (*it)->hThread );
		}
		return;
	}

	//signal handler debugging
	//DebugBreak();

	//signal is blocked by this function?
	//if(pProcessData->signal_blocked[sig] > 0)
	//{
	//	ktrace("signal not delivered - currently blocked\n");
	//	pProcessData->current_signal = 0;
	//	return;
	//}

	//is this process ignoring this signal?
	if( (m_SignalMask.sig[(sig-1)/linux::_NSIG_BPW]) & (1 << (sig-1)%linux::_NSIG_BPW) )
	{
		ktrace("signal not delivered - currently masked\n");
		return;
	}


	//is there a signal handler registered for this signal?
	//	pProcessData->signal_action[signum].sa_handler     = act->sa_handler;
	//	pProcessData->signal_action[signum].sa_flags       = act->sa_flags;
	//	pProcessData->signal_action[signum].sa_restorer    = act->sa_restorer;
	//	pProcessData->signal_action[signum].sa_mask.sig[0] = act->sa_mask;
	if(m_SignalAction[sig].sa_handler == SIG_DFL)
	{
		//default action for this signal
		ktrace("executing default action for signal\n");
		switch(sig)
		{
		//ignore
		case linux::SIGCHLD:
		case linux::SIGURG:
			break;

		case linux::SIGCONT: 
			//Resume all threads
			for(Process::ThreadList::iterator it = m_ThreadList.begin();
				it != m_ThreadList.end();
				++it)
			{
				ResumeThread( (*it)->hThread );
			}
			break;

		//stop
		case linux::SIGSTOP:
		case linux::SIGTSTP:
		case linux::SIGTTIN:
		case linux::SIGTTOU:
			//suspend all threads
			for(Process::ThreadList::iterator it = m_ThreadList.begin();
				it != m_ThreadList.end();
				++it)
			{
				SuspendThread( (*it)->hThread );
			}
			break;

		//terminate
		case linux::SIGHUP:
		case linux::SIGINT:
		case linux::SIGPIPE:
		case linux::SIGALRM:
		case linux::SIGTERM:
		case linux::SIGUSR1:
		case linux::SIGUSR2:
		case linux::SIGPOLL:
		case linux::SIGPROF:
			ktrace("Exiting using SIG_DFL for sig %d\n",sig);
			m_KilledBySig=sig;
			SysCallDll::exit(-sig);
			break;

		//ptrace
		case linux::SIGTRAP:
			if(m_ptrace.OwnerPid)
				break; //ignore when being ptraced
			//else drop thru to core dump

		//core dump
		case linux::SIGQUIT:
		case linux::SIGILL:
		case linux::SIGABRT:
		case linux::SIGFPE:
		case linux::SIGSEGV:
		case linux::SIGBUS:
		case linux::SIGSYS:
			ktrace("Exiting using SIG_DFL for sig %d\n",sig);
			GenerateCoreDump();
			m_KilledBySig=sig;
			SysCallDll::exit(-sig);
			break;

		default:
			ktrace("IMPLEMENT default action for signal %d\n", sig);
			ktrace("Exiting using SIG_DFL for sig %d\n",sig);
			m_KilledBySig=sig;
			SysCallDll::exit(-sig);
			break;
		}
	}
	else
	if(m_SignalAction[sig].sa_handler == SIG_IGN)
	{
		//ignore the signal
		ktrace("ignoring signal - sig_ign\n");
	}
	else
	{
		ktrace("dispatching to signal handler @ 0x%08lx\n", m_SignalAction[sig].sa_handler);

		//supposed to supress the signal whilst in the handler?
		//linux just sets to SIG_DFL? (see man 2 signal)
		linux::__sighandler_t handler = m_SignalAction[sig].sa_handler;

		//restore handler?
		if((m_SignalAction[sig].sa_flags & linux::SA_ONESHOT)
		|| (m_SignalAction[sig].sa_flags & linux::SA_RESETHAND) )
			m_SignalAction[sig].sa_handler = SIG_DFL;


		//need to update user process
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS | CONTEXT_EXTENDED_REGISTERS;
		SuspendThread(T->hThread);
		GetThreadContext(T->hThread, &ctx);


		//Dispatch to custom handler
		//
		//The handler may accept a single argument (sig) OR a several (sig,siginfo,data)
		//however they both have sig as the first argument so we can
		//build up a siginfo type one on the stack in both cases
		//the single argument handler will just ignore the extras

		//Signal handlers have their own state (registers etc) and when
		//they exit, the original state is restored.
		//To do this we also use the stack to save a CONTEXT record and
		//use a call to the kernels sigreturn routine to restore the record.
		//The stack is set up such that a return from the handler runs the
		//restoration code.

		//basic info
		linux::siginfo si;
		si.si_signo = sig;
		si.si_errno = 0;
		si.si_code = linux::SI_USER;

		BYTE asm_code[] = {
			//the code we want
			0xbb, 0,0,0,0,		// mov ebx, context_addr
			0xb8, 0,0,0,0,		// mov eax, _NR_sigreturn
			0xcd, 0x80,			// int 80

			//this is code that gdb looks for 
			//(so says the kernel source in arch/i386/kernel/signal.c)
			0x58,				// popl eax
			0xb8, 0,0,0,0,		// mov eax,__NR_sigreturn
			0xcd, 0x80,			// int 80
		};
		*((DWORD*)&asm_code[1]) = 0x0; //context_addr
		*((DWORD*)&asm_code[6]) = linux::__NR_sigreturn;
		*((DWORD*)&asm_code[14]) = linux::__NR_sigreturn;

		//add new items to write onto the stack
		struct {
			//what the handler sees
			DWORD ReturnAddress;
			int    Arg_Signal;
			void * Arg_pSigInfo;
			void * Arg_pData;

			//supporting data for stack arguments
			linux::siginfo SigInfo;

			//restoration stuff
			Process::SignalSaveState SavedState;

			BYTE Restorer[sizeof(asm_code)]; //room for a small assembly routine
		} stack;

		memset(&stack, 0, sizeof(stack));
		stack.Arg_Signal = sig;
		stack.SavedState.ctx = ctx;
		memcpy(&stack.SavedState.SignalMask, &m_SignalMask, sizeof(m_SignalMask));

		DWORD NewEsp = ctx.Esp - sizeof(stack);

		//put in the restorer code
		*((DWORD*)&asm_code[1]) = NewEsp + offset_of(stack, stack.SavedState.ctx);
		memcpy(stack.Restorer, asm_code, sizeof(asm_code));

		//populate extra items?
		if(m_SignalAction[sig].sa_flags & linux::SA_SIGINFO)
		{
			ktrace("IMPLEMENT correct signal sa_action siginfo stuff\n");

			/*
			linux::ucontext ct;
			ct.uc_flags = ?;
			ct.uc_link = 0;
			stack_t		  uc_stack;
			struct sigcontext uc_mcontext;
			sigset_t	  uc_sigmask;	/* mask last for extensibility */
			ktrace("IMPLEMENT correct signal sa_action ucontext stuff\n");
			//copy the context to the process and pass as param

			stack.SigInfo.si_signo = sig;
			stack.SigInfo.si_errno = 0;
			stack.SigInfo.si_code = linux::SI_USER;

			stack.Arg_pSigInfo = (void*)(NewEsp + offset_of(stack, stack.SigInfo));
			stack.Arg_pData = 0;
		}

		stack.ReturnAddress = NewEsp + offset_of(stack, stack.Restorer);
		//use restorer
		//in linux this is a call to sys_sigreturn ?
		//to return to user-land?
		//documented as 'dont use'
		//if(m_SignalAction[sig].sa_flags & SA_RESTORER)
		//{
		//	ReturnAddress = sa_restorer?
		//}
		ktrace("signal restorer @ 0x%08lx\n", stack.ReturnAddress);

		//move to new masks
		m_SignalMask = m_SignalAction[sig].sa_mask;
		if((m_SignalAction[sig].sa_flags & linux::SA_NODEFER)==0
		|| (m_SignalAction[sig].sa_flags & linux::SA_NOMASK)==0 )
		{
			//mask current signal too
			m_SignalMask.sig[(sig-1)/linux::_NSIG_BPW] |= (1 << (sig-1)%linux::_NSIG_BPW);
		}

		//kernel arch/i386/kernel/signal.c seems to set up registers too
		ctx.Eax = stack.Arg_Signal;
		ctx.Edx = (DWORD)stack.Arg_pSigInfo;
		ctx.Ecx = (DWORD)stack.Arg_pData;

		//update stack and then run the handler
		ctx.Esp = NewEsp;
		P->WriteMemory((ADDR)NewEsp, sizeof(stack), &stack);

		ctx.Eip = (DWORD)handler;

		SetThreadContext(T->hThread, &ctx);
		ResumeThread(T->hThread);

	}

}


void Process::GenerateCoreDump()
{
	m_bCoreDumped = true;
	ktrace("Core Dump\n");
}

DWORD Process::InjectFunctionCall(void *func, void *pStackData, int nStackDataSize, InjectionInfo * pInjectInfo /*=NULL*/, int CountInjectInfo /*=0*/)
{
	//Expect that the func is declared as _stdcall calling convention.
	//(actually it ought to be from the SysCallDll::RemoteAddrInfo) 
	//This means we place args on the stack so that param1 pops first
	//Stack&register cleanup is unnessesary because our caller will restore the correct state

	//we need current state whilst in setup
	// this is because we are using context in ConvertProcessToKeow()
	// and StartNewImageRunning() and both those cause VirtualAlloc calls
	// that on win95 cause an injection to occur.

	CONTEXT OldCtx;
	if(m_bInWin32Setup)
	{
		OldCtx.ContextFlags = CONTEXT_FULL;
		GetThreadContext(T->hThread, &OldCtx);
	}

	//Injection uses the original Win32 context (& stack etc) from when
	//the stub process started.

	ktrace("Injecting call to SysCallDll func @ 0x%08lx\n", func);

	CONTEXT InjectCtx = m_BaseWin32Ctx;
	//trying to get win95 memory written to correctly
	//InjectCtx.Esp = OldCtx.Esp; //use stack WE allocated

	InjectCtx.Eip = (DWORD)func;		//call this in the stub
	InjectCtx.Esp -= sizeof(ADDR);	//the return address
	InjectCtx.Esp -= nStackDataSize;	//the params

//	SetSingleStep(true, &InjectCtx); //FOR DEBUG

	SetThreadContext(T->hThread, &InjectCtx);


	ADDR addr = (ADDR)InjectCtx.Esp;
	ktrace("inject data @ 0x%08lx\n", addr);

	//return address - we don't actually need it (exit trapped below) 
	DWORD dummy = 0;
	WriteMemory(addr, sizeof(DWORD), &dummy);
	addr += sizeof(DWORD);

	//parameters & extra data
	ADDR ParamAddr = addr;
	if(pInjectInfo)
	{
		//some pointers in the stack frame need updating to where a buffer will be in the remote process
		for(int i=0; i<CountInjectInfo; ++i)
		{
			DWORD a = (DWORD)ParamAddr + pInjectInfo[i].StackRelativeOffsetToBuffer;
			int offset = pInjectInfo[i].StackParamToActAsPointer;
			((DWORD*)pStackData)[offset] = a;
		}
	}
	WriteMemory(addr, nStackDataSize, pStackData);
	addr += nStackDataSize;
	//trying to get win95 memory written to correctly
	//DumpMemory((ADDR)InjectCtx.Esp, nStackDataSize+4);
	//DumpContext(InjectCtx);



	//resume the process to let it run the function
	//expect the stub to do a Break (int 3) at the end
	//we'll capture the break and then return

	DEBUG_EVENT evt;
	for(;;) {

		FlushInstructionCache(m_hProcess, 0, 0); //need this for our process modifications?
		ContinueDebugEvent(m_dwProcessId, T->dwThreadId, DBG_CONTINUE);

		WaitForDebugEvent(&evt, INFINITE);

		if(evt.dwDebugEventCode==EXCEPTION_DEBUG_EVENT)
		{
			if(evt.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT)
				break; //exit loop - injected function finished

			if(evt.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP) //for debug
			{
				CONTEXT ctx;
				ctx.ContextFlags = CONTEXT_FULL;
				GetThreadContext(T->hThread, &ctx);
				DumpContext(ctx);
				//single step is for debugging - keep it up if it was started
				P->SetSingleStep(true, &ctx);
				SetThreadContext(T->hThread, &ctx);
			}
			else
			{
				//not expecting an exception in the SysCallDll
				ktrace("unexpected exception from SysCallDll: 0x%8lx\n", evt.u.Exception.ExceptionRecord.ExceptionCode);
#ifdef _DEBUG
				CONTEXT ctx;
				ctx.ContextFlags = CONTEXT_FULL;
				GetThreadContext(T->hThread, &ctx);
				DumpContext(ctx);
				SendSignal(linux::SIGSEGV); //terminate
#endif
			}
		}

		if(evt.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT)
		{
			ktrace("process exit whilst in injected code\n");
			m_dwExitCode = evt.u.ExitProcess.dwExitCode;
			m_bStillRunning = false;
			ExitThread(0); //no longer debugging
		}

		if(evt.dwDebugEventCode==OUTPUT_DEBUG_STRING_EVENT) //for debug info
		{
			int bufLen = evt.u.DebugString.nDebugStringLength*2;
			char * buf = new char[bufLen];

			ReadMemory(buf, (ADDR)evt.u.DebugString.lpDebugStringData, evt.u.DebugString.nDebugStringLength);
			buf[evt.u.DebugString.nDebugStringLength] = 0;

			ktrace("syscalldll relay: %*s", evt.u.DebugString.nDebugStringLength, buf);

			delete buf;
		}

		//ignore the rest
		//TODO is this ok?
	}

	//done calling it, stub is still in the function, 
	// but we can restore the stack and registers

	// first retreive any altered parameters
	addr = ParamAddr;
	ReadMemory(pStackData, addr, nStackDataSize);


	//only for debug - show exit point
	GetThreadContext(T->hThread, &InjectCtx);
	DWORD dwRet = InjectCtx.Eax;
	ktrace("Injection exit @ 0x%08lx\n", InjectCtx.Eip);

	if(m_bInWin32Setup)
	{
		SetThreadContext(T->hThread, &OldCtx);
	}

	return dwRet;
}


// Output current process context
// and disassemble instructions
void Process::DumpContext(CONTEXT &ctx)
{
	BYTE buf[8];
	ReadMemory(buf, (ADDR)ctx.Eip, sizeof(buf));

	ktrace("0x%08lX %02X %02X %02X %02X  %02X %02X %02X %02X  %-12s   eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx eip=%08lx esp=%08lx ebp=%08lx flags=%08lx cs=%x ds=%x es=%x fs=%x gs=%x ss=%x\n",
		    ctx.Eip,buf[0],buf[1],buf[2],buf[3],
										 buf[4],buf[5],buf[6],buf[7],
															  "instr",
																	  ctx.Eax,  ctx.Ebx,  ctx.Ecx,  ctx.Edx,  ctx.Esi,  ctx.Edi,  ctx.Eip,  ctx.Esp,  ctx.Ebp,  ctx.EFlags, 
																																											ctx.SegCs,ctx.SegDs,ctx.SegEs,ctx.SegFs,ctx.SegGs,ctx.SegSs );
	//todo: disassemble
}

// Display a memory dump
void Process::DumpMemory(ADDR addr, DWORD len)
{
	const int BYTES_PER_LINE = 8;
	char hexbuf[5 * BYTES_PER_LINE + 1]; // "0x00 "... + null
	char charbuf[BYTES_PER_LINE + 1];    // "xxxxxxxx" + null
	int x;
	BYTE b;

	memset(charbuf, 0, sizeof(charbuf));

	ktrace("memory dump @ 0x%08lx, len %d\n", addr,len);
	x=0;
	for(DWORD i=0; i<len; ++i)
	{
		ReadMemory(&b, addr+x, 1); //byte-by-byte is slow but this is just a debug thing anyway
		StringCbPrintf(&hexbuf[x*5], sizeof(hexbuf)-(x*5), "0x%02x ", b);
		charbuf[x] = (isalnum(b)||ispunct(b)) ? b : '.';

		++x;
		if(x>=BYTES_PER_LINE)
		{
			ktrace("  0x%08lx: %s %s\n", addr, hexbuf, charbuf);
			addr+=BYTES_PER_LINE;
			x=0;
			memset(charbuf, 0, sizeof(charbuf));
		}
	}
	if(x>0)
		ktrace("  0x%08lx: %s %s\n", addr, hexbuf, charbuf);
}

//trace stack frames
void Process::DumpStackTrace(CONTEXT &ctx)
{
	//trace the stack back
	//assumes frame pointers exist
	ADDR esp = (ADDR)ctx.Esp;
	ADDR ebp = (ADDR)ctx.Ebp;
	ADDR retAddr = 0;
	int cnt=0;
	while(esp>=(ADDR)ctx.Esp && esp<=m_KeowUserStackTop && ++cnt<1000) //while data seems sensible
	{
		//edp is old stack pos
		esp=ebp;
		if(ebp==0)
			break;

		//stack is now ret addr
		esp+=4;
		ReadMemory(&retAddr, esp, sizeof(ADDR));

		//first pushed item was old ebp
		esp-=sizeof(ADDR);
		ReadMemory(&ebp, esp, sizeof(ADDR));


		ktrace(": from 0x%08lx\n", retAddr);
	}
}


// Output current process GDT & LDT descriptors
void Process::DumpDescriptors()
{
	int index;
	DWORD dwSelector;
	PROCESS_LDT_INFORMATION ldtInfo;
	DWORD rc;
	LDT_ENTRY *ldt = &ldtInfo.LdtEntries[0];

	const bool bFullDump = true;

	if(bFullDump) {
		ktrace("GDT Table [initial thread]:\n");
		for(index=0; index<256; ++index) {
			dwSelector = index*sizeof(LDT_ENTRY);
			if(::GetThreadSelectorEntry(m_ThreadList[0]->hThread, dwSelector, ldt)==0) {
				break;
			}
			ktrace("GDT %d.%04x: 0x%02x%02x%04x:x%02x%04x DPL:%x Type:%02x 4k:%x Sys:%x Pres:%x Big:%x\n",
					index, dwSelector,
					ldt->HighWord.Bytes.BaseHi, ldt->HighWord.Bytes.BaseMid, ldt->BaseLow,
					ldt->HighWord.Bits.LimitHi, ldt->LimitLow,
					ldt->HighWord.Bits.Dpl, ldt->HighWord.Bits.Type, ldt->HighWord.Bits.Granularity,
					ldt->HighWord.Bits.Sys, ldt->HighWord.Bits.Pres, ldt->HighWord.Bits.Default_Big
					);
		}
	}

	HMODULE hlib = GetModuleHandle("NTDLL");
	FARPROC fp = GetProcAddress(hlib, "NtQueryInformationProcess");
	NTSTATUS (CALLBACK *NtQueryInformationProcess) (HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
    *(FARPROC *)&NtQueryInformationProcess = fp;


	ktrace("LDT Table:\n");
	ldtInfo.Start = 0;
	ldtInfo.Length = 1*sizeof(LDT_ENTRY);
	rc = NtQueryInformationProcess(m_hProcess, ProcessLdtInformation, &ldtInfo, sizeof(ldtInfo), NULL);
	int numEntries = ldtInfo.Length / sizeof(LDT_ENTRY);
	for(index = bFullDump?0:16; index<numEntries; ++index) {
		dwSelector = index*sizeof(LDT_ENTRY);

		ldtInfo.Start = dwSelector ;//& 0xFFFFFFF8;  // selector --> offset
		ldtInfo.Length = 1*sizeof(LDT_ENTRY);
		rc = NtQueryInformationProcess(m_hProcess, ProcessLdtInformation, &ldtInfo, sizeof(ldtInfo), NULL);
		
		if(rc!=0) {
			ktrace("LDT read failed %lx\n", rc);
			break;
		}
		
		ktrace(" LDT %3d.%04x: 0x%02x%02x%04x:x%02x%04x DPL:%x Type:%02x 4k:%x Sys:%x Pres:%x Big:%x\n",
				index, dwSelector,
				ldt->HighWord.Bytes.BaseHi, ldt->HighWord.Bytes.BaseMid, ldt->BaseLow,
				ldt->HighWord.Bits.LimitHi, ldt->LimitLow,
				ldt->HighWord.Bits.Dpl, ldt->HighWord.Bits.Type, ldt->HighWord.Bits.Granularity,
				ldt->HighWord.Bits.Sys, ldt->HighWord.Bits.Pres, ldt->HighWord.Bits.Default_Big
				);
	}
}


// Find a free entry in the open files list
int Process::FindFreeFD()
{
	int fd;
	for(fd=0; fd<MAX_OPEN_FILES; ++fd)
		if(m_OpenFiles[fd] == NULL)
			return fd;

	return -1; //none
}

void Process::FreeResourcesBeforeExec()
{
	//unmap memory?
	MmapList::iterator mmap_it;
	for(mmap_it = m_MmapList.begin();
	    mmap_it != m_MmapList.end();
		++mmap_it)
	{
		MMapRecord * pMmap = *mmap_it;

		DebugBreak(); //TODO: handle cloning mmaps 
	}

	//close all files that we need to
	for(int i=0; i<MAX_OPEN_FILES; ++i)
	{
		if(m_OpenFiles[i]==NULL)
			continue;
		if(!m_OpenFiles[i]->GetInheritable())
		{
			m_OpenFiles[i]->Close();
			delete m_OpenFiles[i];
			m_OpenFiles[i] = NULL;
		}
	}

	//free other memory
	MemoryAllocationsList::iterator mem_it;
	for(mem_it = m_MemoryAllocations.begin();
	    mem_it != m_MemoryAllocations.end();
		++mem_it)
	{
		MemoryAlloc * pMemAlloc = *mem_it;

		//NOT the stack
		if(pMemAlloc->addr == m_KeowUserStackBase)
		{
			continue;
		}

		if(!LegacyWindows::VirtualFreeEx(m_hProcess, pMemAlloc->addr, pMemAlloc->len, MEM_DECOMMIT))
			ktrace("failed to free 0x%08lx, len %d\n", pMemAlloc->addr, pMemAlloc->len);

		m_MemoryAllocations.erase(mem_it);
		//iterator is invalid now? reset?
		mem_it = m_MemoryAllocations.begin();
	}


	//signals are reset on exec
	InitSignalHandling();

	//Also reset the elf memory stuff
	memset(&m_ElfLoadData, 0, sizeof(m_ElfLoadData));
}

void Process::InitSignalHandling()
{
	int i;

	m_CurrentSignal = 0;
	m_KilledBySig = 0;

	memset(&m_PendingSignals, 0, sizeof(m_PendingSignals));

	memset(&m_SignalMask, 0, sizeof(m_SignalMask));

	//default signal handling
	for(i=0; i<linux::_NSIG; ++i) {
		m_SignalAction[i].sa_handler = SIG_DFL;
		m_SignalAction[i].sa_flags = NULL;
		m_SignalAction[i].sa_restorer = NULL;
		ZeroMemory(&m_SignalAction[i].sa_mask, sizeof(m_SignalAction[i].sa_mask));
	}
}

bool Process::IsSuspended()
{
	//the process doesn't get suspended, just the kernel thread
	//testing the process would be meaningless anyway as it appears
	//suspended if the kernel currently is processing a syscall

	bool suspended = false;
	DWORD cnt;

	cnt = SuspendThread(m_KernelThreadHandle);
	if(cnt==-1)
	{
		//some error
		return false;
	}
	else
	{
		suspended = (cnt>0);
		ResumeThread(m_KernelThreadHandle);
		return suspended;
	}
}


ThreadInfo* Process::GetThreadInfo(DWORD dwThreadId)
{
	Process::ThreadList::iterator it;
	for(it=m_ThreadList.begin(); it!=m_ThreadList.end(); ++it)
	{
		ThreadInfo * pInfo = *it;
		if(pInfo->dwThreadId == dwThreadId)
		{
			return pInfo;
		}
	}

	//no such thread recorded
	return NULL;
}
