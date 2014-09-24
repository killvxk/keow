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

// MemoryHelper.h: interface for the MemoryHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORYHELPER_H__1A80F3C0_1A72_426A_B6E5_6D4BD43E2EC6__INCLUDED_)
#define AFX_MEMORYHELPER_H__1A80F3C0_1A72_426A_B6E5_6D4BD43E2EC6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class MemoryHelper  
{
public:
	static ADDR AllocateMemAndProtect(ADDR addr, DWORD size, DWORD prot);
	static bool DeallocateMemory(ADDR addr, DWORD size);
	static bool WriteMemory(HANDLE hToProcess, ADDR toAddr, int len, ADDR fromAddr);
	static bool ReadMemory(ADDR toAddr, HANDLE hFromProcess, ADDR fromAddr, int len);
	static bool TransferMemory(HANDLE hFromProcess, ADDR fromAddr, HANDLE hToProcess, ADDR toAddr, int len);
	static bool FillMem(HANDLE hToProcess, ADDR toAddr, int len, BYTE fill);
	static ADDR CopyStringListBetweenProcesses(HANDLE hFromProcess, ADDR pFromList, HANDLE hToProcess, Process* pProcToRecordMemoryIn, DWORD * pdwCount, DWORD * pdwMemSize);
	static string ReadString(HANDLE hFromProcess, ADDR fromAddr);
	static int AllocateLDTSelector(DWORD dwThreadId, bool bIsForLDT);
	static bool GetLDTSelector(DWORD dwThreadId, linux::user_desc &user_desc);
	static bool SetLDTSelector(DWORD dwThreadId, linux::user_desc &user_desc, bool bIsReallyLDT);
	static bool HandlePossibleLDTException(WORD instruction, ADDR exceptionAddress, CONTEXT& ctx);
	static int CalculateLDTEntryForThread(DWORD dwThreadId, int entry_number);

private: 
	MemoryHelper();
	virtual ~MemoryHelper();

	enum ReadWrite {
		Read,
		Write
	};
	static bool ProcessReadWriteMemory(HANDLE hProcess, LPVOID pRemoteAddr, LPVOID pLocalAddr, DWORD len, DWORD * pDone, ReadWrite rw );
	static ADDR AllocateMemAndProtectProcess(HANDLE hProcess, ADDR addr, DWORD size, DWORD prot);
};

#endif // !defined(AFX_MEMORYHELPER_H__1A80F3C0_1A72_426A_B6E5_6D4BD43E2EC6__INCLUDED_)
