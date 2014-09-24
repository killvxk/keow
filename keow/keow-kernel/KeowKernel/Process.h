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

// Process.h: interface for the Process class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESS_H__065A3BC3_71C3_4302_8E39_297E193A46AF__INCLUDED_)
#define AFX_PROCESS_H__065A3BC3_71C3_4302_8E39_297E193A46AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//type of a Keow/Unix Process ID (needs to be signed)
typedef long PID;

//pointer
typedef BYTE* ADDR;


//some process limits
#define MAX_OPEN_FILES		1024
#define MAX_PENDING_SIGNALS	128
#define MAX_SIGNALS			linux::_NSIG
#define MAX_LDT_ENTRIES		8192

//We skip LDT entries that overlap GDT indexes. See MemoryHelper::AllocateLDTEntry()
#define FIRST_USABLE_LDT_ENTRY	    16


class Process  
{
public:
	int FindFreeFD();

	void GenerateCoreDump();
	void SendSignal(int sig);
	void HandleSignal(int sig);

	bool WriteMemory(ADDR addr, DWORD len, const void * pBuf);
	bool ReadMemory(LPVOID pBuf, ADDR addr, DWORD len);

	void FreeResourcesBeforeExec();
	DWORD StartNewImageRunning();
	static Process* StartInit(PID pid, Path& path, char ** InitialArguments, char ** InitialEnvironment);
	static Process* StartFork(Process * pParent);
	DWORD LoadImage(Path &img, bool LoadAsLibrary);
	static bool CanLoadImage(Path &img, bool LoadAsLibrary);

	void DumpMemory(ADDR addr, DWORD len);
	void DumpContext(CONTEXT &ctx);
	void DumpStackTrace(CONTEXT &ctx);
	void DumpDescriptors();

	void SetSingleStep(bool set, CONTEXT * pCtx);

	struct InjectionInfo {
		int StackParamToActAsPointer;	//which DWORD on the stack to update to point to actual data location
		int StackRelativeOffsetToBuffer;	//byte offset within stack data where buffer is
	};
	DWORD InjectFunctionCall(void *func, void *pStackData, int nStackDataSize, InjectionInfo * pInjectInfo = NULL, int CountInjectInfo = 0);

	virtual ~Process();

	//process handler thread
	static DWORD WINAPI KernelProcessHandlerEntry(LPVOID param);
	static const int KERNEL_PROCESS_THREAD_STACK_SIZE;
	static const char * KEOW_PROCESS_STUB;

public:
	bool IsSuspended();
	ThreadInfo* GetThreadInfo(DWORD dwThreadId);

	PID m_Pid;
	PID m_ParentPid;
	PID m_ProcessGroupPID;

	Device * m_pControllingTty;
	bool m_bIsInForeground;

	int m_gid, m_uid;
	int m_egid, m_euid;
	int m_saved_uid, m_saved_gid;
	int m_umask;

	DWORD m_dwProcessId;
	HANDLE m_hProcess;
	typedef list<ThreadInfo*> ThreadList;
	ThreadList m_ThreadList;

	bool m_bStillRunning;
	bool m_bCoreDumped;
	DWORD m_dwExitCode;

	struct Timings {
		FILETIME ftCreateTime, ftExitTime, ftKernelTime, ftUserTime;
	} m_BaseTimes; //so we can subtract from GetProcessTimes() when an exec() reuses this process

	HANDLE m_hWaitTerminatingEvent;

	//keep track of original win32 data needed for injection etc (when we run as a win32 process again)
	CONTEXT m_BaseWin32Ctx;

	ADDR m_KeowUserStackBase;
	ADDR m_KeowUserStackTop;

	DWORD m_KernelThreadId;
	HANDLE m_KernelThreadHandle;

	Path m_ProcessFileImage;
	Path m_UnixPwd;

	struct MemoryAlloc
	{
		ADDR addr;
		DWORD len;
		DWORD protection;

		MemoryAlloc(ADDR addrActual, DWORD size, DWORD prot)
			: addr(addrActual), len(size), protection(prot)
		{
		}
	};
	typedef list<MemoryAlloc*> MemoryAllocationsList;
	MemoryAllocationsList m_MemoryAllocations;

	struct MMapRecord 
	{
		int fd;
		HANDLE hRemoteMapHandle;
		DWORD offset;
		DWORD len;
		DWORD Protection;
		ADDR Address;

		MMapRecord(int fd, HANDLE hRemote, ADDR addr, DWORD offs, DWORD size, DWORD prot)
		{
			this->fd = fd;
			hRemoteMapHandle = hRemote;
			offset = offs;
			len = size;
			Protection = prot;
			Address = addr;
		}
	};
	typedef list<MMapRecord*> MmapList;
	MmapList m_MmapList;

	struct ElfLoadData {
		ADDR phdr_addr;
		DWORD phdr_phnum;
		DWORD phdr_phent;

		ADDR image_base;
		ADDR start_addr;
		ADDR program_base, program_max;
		ADDR interpreter_base;
		ADDR interpreter_start;
		ADDR bss_start, brk;
		ADDR last_lib_addr;

		char Interpreter[MAX_PATH];
	};
	ElfLoadData	m_ElfLoadData;

	struct PtraceData {
		PID OwnerPid;
		DWORD Request;
		CONTEXT ctx;
		bool ctx_valid;
		DWORD Saved_Eax;
		int new_signal;
	};
	PtraceData m_ptrace;

	//info about what resources the stub can provide
	SysCallDll::RemoteAddrInfo SysCallAddr;
	DWORD m_LinuxGateDSO;
	DWORD m_LinuxGateVSyscall;

	DWORD m_ArgCnt, m_EnvCnt;
	ADDR m_Environment;
	ADDR m_Arguments;

	//Data that the debugger thread uses to initialise this process
	HANDLE m_hProcessStartEvent;
	HANDLE m_hForkDoneEvent;
	bool m_bInWin32Setup;

	//signal handling
	int m_CurrentSignal;
	int m_KilledBySig;
	bool m_PendingSignals[linux::_NSIG];
	linux::sigset_t m_SignalMask;
	linux::sigaction m_SignalAction[linux::_NSIG];
	
	//we save this stuff when calling signal handlers, so that we can return from the signal
	struct SignalSaveState {
		CONTEXT ctx;
		linux::sigset_t SignalMask;
	};

	//open files
	IOHandler * m_OpenFiles[MAX_OPEN_FILES];

	//keep track of LDT entries allocated
	struct LdtData {
		DWORD dwAllocatingThreadId;
		bool bIsReallyLDT; //is this a real LDT entry, or fake GDT entry for get/set_thread_area
		linux::user_desc user_desc;
	};
	LdtData m_LdtEntries[MAX_LDT_ENTRIES];


private:
	Process(); //private - use methods to create
protected:
	void InitSignalHandling();
	void ForkCopyOtherProcess(Process &other, ThreadInfo& otherThread);
	void HandleException(DEBUG_EVENT &evt);
	void ConvertProcessToKeow();
	void DebuggerLoop();
	void ResumeKeowProcess(DEBUG_EVENT &evt, bool bNeedToRestoreGS);
	DWORD LoadElfImage(HANDLE hImg, struct linux::elf32_hdr * pElf, ElfLoadData * pElfLoadData, bool LoadAsLibrary);
};

#endif // !defined(AFX_PROCESS_H__065A3BC3_71C3_4302_8E39_297E193A46AF__INCLUDED_)
