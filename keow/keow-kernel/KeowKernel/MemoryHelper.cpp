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

// MemoryHelper.cpp: implementation of the MemoryHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "MemoryHelper.h"

//////////////////////////////////////////////////////////////////////

MemoryHelper::MemoryHelper()
{
}

MemoryHelper::~MemoryHelper()
{
}


ADDR MemoryHelper::AllocateMemAndProtectProcess(HANDLE hProcess, ADDR addr, DWORD size, DWORD prot)
{
	ADDR p;
	ADDR tmp64k ,tmp4k;
	MEMORY_BASIC_INFORMATION mbi;
	ADDR end_addr;
//	DWORD oldprot;

	if(size<0)
	{
		return (ADDR)-1;
	}

	ktrace("allocating 0x%08lx, len %d in proc %lx\n", addr, size, hProcess);

	//if no addr, then we can assign one
	if(addr==0)
	{
		addr = (ADDR)LegacyWindows::VirtualAllocEx(hProcess, NULL, size, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	}
	end_addr = addr + size;

	//ensure reserved first
	//reservation is in 64k pages
	//may already be allocated
	tmp64k = (ADDR)((DWORD)addr & 0xFFFF0000);
	while(tmp64k < end_addr)
	{
		LegacyWindows::VirtualQueryEx(hProcess, tmp64k, &mbi, sizeof(mbi));

		DWORD len64k = end_addr - tmp64k;
		if(len64k > (mbi.RegionSize - (DWORD)(tmp64k - (LPBYTE)mbi.BaseAddress)))
			len64k = (mbi.RegionSize - (DWORD)(tmp64k - (LPBYTE)mbi.BaseAddress));
		len64k = (len64k+0xFFFFL) & ~0xFFFFL; //rounded up to 64k boundary

		if(mbi.State == MEM_FREE)
		{
			//allocate with full perms, individual pages can lock it down as required
			p = (ADDR)LegacyWindows::VirtualAllocEx(hProcess, tmp64k, len64k, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if(p!=tmp64k)
			{
				return (ADDR)-1;
			}
		}

		tmp64k += len64k;
	}

	//commit pages from reserved area
	//must allocate on a page boundary (4k for Win32)
	tmp4k = (ADDR)((DWORD)addr & 0xFFFFF000);
	while(tmp4k < end_addr)
	{
		LegacyWindows::VirtualQueryEx(hProcess, (void*)tmp4k, &mbi, sizeof(mbi));

		DWORD len4k = end_addr - tmp4k;
		if(len4k > (mbi.RegionSize - (DWORD)(tmp4k - (LPBYTE)mbi.BaseAddress)))
			len4k = (mbi.RegionSize - (DWORD)(tmp4k - (LPBYTE)mbi.BaseAddress));
		len4k = (len4k+0xFFFL) & ~0xFFFL; //rounded up to 4k boundary

		if(mbi.State == MEM_RESERVE)
		{
//seems this cannot be changed within the 64k successfully?
//			p = LegacyWindows::VirtualAlloc((void*)tmp4k, SIZE4k, MEM_COMMIT, prot);
			p = (ADDR)LegacyWindows::VirtualAllocEx(hProcess, (void*)tmp4k, len4k, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if(p!=tmp4k)
			{
				return (ADDR)-1;
			}
	//		LegacyWindows::VirtualProtect((void*)tmp64k, SIZE4k, prot, &oldprot); //in case re-using old area
		}

		tmp4k += len4k;
	}

	//ktrace("AllocateMemAndProtect [h%lX] 0x%08lx -> 0x%08lx\n", hProcess, addr, end_addr);

	return addr;
}


ADDR MemoryHelper::AllocateMemAndProtect(ADDR addr, DWORD size, DWORD prot)
{
	ADDR addrActual = AllocateMemAndProtectProcess(P->m_hProcess, addr, size, prot);
	if(addrActual != (ADDR)-1)
		P->m_MemoryAllocations.push_back(new Process::MemoryAlloc(addrActual, size, prot));
	return addrActual;
}

bool MemoryHelper::DeallocateMemory(ADDR addr, DWORD size)
{
	Process::MemoryAllocationsList::iterator it;
	for(it=P->m_MemoryAllocations.begin();
	    it!=P->m_MemoryAllocations.end();
		++it)
	{
		Process::MemoryAlloc * pAlloc = *it;
		if(pAlloc->addr == addr
		&& pAlloc->len == size)
		{
			ktrace("freeing 0x%08lx, len %d in proc %lx\n", addr, size, P->m_hProcess);

			if(!LegacyWindows::VirtualFreeEx(P->m_hProcess, addr, size, MEM_DECOMMIT))
				return false;

			P->m_MemoryAllocations.erase(it);
			return true;
		}
	}

	return false;
}


/*****************************************************************************/

bool MemoryHelper::ProcessReadWriteMemory(HANDLE hProcess, LPVOID pRemoteAddr, LPVOID pLocalAddr, DWORD len, DWORD * pDone, ReadWrite rw )
{
	/* It's not documented (anywhere I could find) but Read/Write ProcessMemory() calls
	   appear to fail if either the local or remote addresses cross a 4k boundary.
	   Possibly it is that you cannot cross an allocation boundary and that many things
	   use VirtualAlloc to commit in 4k blocks and not larger.

	   Therefore this routine has to transfer memory in chunks to ensure
	   that neither end (src,dest) crosses 4k.

	   Example: | is a 4k boundary, . is not accessed, * is copied

		local buffer   | . . . * | * * * * | * * * * | * * * . |
                            ___|   ----- |
                           /    ____/   /
                          /    /    ___/
	                     - -----   /
		remote buffer  | * * * * | * * * * | * * * * | . . . . |

		Basically, divide the local buffer into 4k chunks and then transfer those
		chunks (possibly in two goes) with the remote process.
	 */

	DWORD pLocal = (DWORD)pLocalAddr;
	DWORD pRemote = (DWORD)pRemoteAddr;

	DWORD dwCopied = 0;
	DWORD dwAmountLeft = len;
	while(dwAmountLeft != 0)
	{
		DWORD dwNext4k;
		DWORD dwXfer;
		BOOL bRet;

		//determine which peice of the local buffer to copy
		dwNext4k = (pLocal & 0xFFFFF000) + SIZE4k;
		DWORD dwLocalToCopy = dwNext4k - pLocal;

		if(dwLocalToCopy > dwAmountLeft)
			dwLocalToCopy = dwAmountLeft;


		//determine first block of the remote copy

		dwNext4k = (pRemote & 0xFFFFF000) + SIZE4k;
		DWORD dwRemoteToCopy = dwNext4k - pRemote;

		if(dwRemoteToCopy > dwLocalToCopy)
			dwRemoteToCopy = dwLocalToCopy;

		if(rw == Read)
			bRet = ReadProcessMemory(hProcess, (LPVOID)pRemote, (LPVOID)pLocal, dwRemoteToCopy, &dwXfer);
		else
			bRet = WriteProcessMemory(hProcess, (LPVOID)pRemote, (LPVOID)pLocal, dwRemoteToCopy, &dwXfer);
		if(bRet==0)
		{
			DWORD err = GetLastError();
			ktrace("error 0x%lx in xfer program segment\n", err);
			//keep trying //return false;
		}
		pLocal += dwRemoteToCopy;
		pRemote += dwRemoteToCopy;
		dwCopied += dwXfer;

		//determine second block of the remote copy
		//we already did part of a 4k block, so the rest will be less than 4k and
		//easily fit now.

		dwRemoteToCopy = dwLocalToCopy - dwRemoteToCopy; //all the rest

		if(dwRemoteToCopy!=0)
		{
			if(rw == Read)
				bRet = ReadProcessMemory(hProcess, (LPVOID)pRemote, (LPVOID)pLocal, dwRemoteToCopy, &dwXfer);
			else
				bRet = WriteProcessMemory(hProcess, (LPVOID)pRemote, (LPVOID)pLocal, dwRemoteToCopy, &dwXfer);
			if(bRet==0)
			{
				DWORD err = GetLastError();
				ktrace("error 0x%lx in xfer program segment\n", err);
				//keep trying //return false;
			}
			pLocal += dwRemoteToCopy;
			pRemote += dwRemoteToCopy;
			dwCopied += dwXfer;
		}

		dwAmountLeft -= dwLocalToCopy;
	}

	if(pDone)
		*pDone = dwCopied;

	return true;
}

bool MemoryHelper::WriteMemory(HANDLE hToProcess, ADDR toAddr, int len, ADDR fromAddr)
{
	DWORD dwWritten;
	if(!ProcessReadWriteMemory(hToProcess, toAddr, fromAddr, len, &dwWritten, Write))
	{
		DWORD err = GetLastError();
		ktrace("error 0x%lx in write program segment\n", err);
		return false;
	}
	return true;
}

bool MemoryHelper::ReadMemory(ADDR toAddr, HANDLE hFromProcess, ADDR fromAddr, int len)
{
	DWORD dwRead;
	if(!ProcessReadWriteMemory(hFromProcess, fromAddr, toAddr, len, &dwRead, Read))
	{
		DWORD err = GetLastError();
		ktrace("error 0x%lx in read program segment\n", err);
		return false;
	}
	return true;
}

/*
 * Copy memory between two managed processes
 */
bool MemoryHelper::TransferMemory(HANDLE hFromProcess, ADDR fromAddr, HANDLE hToProcess, ADDR toAddr, int len)
{
	BYTE buf[SIZE4k];

	for(int i=0; i<len; i+=sizeof(buf))
	{
		int copy = sizeof(buf);
		if(i+copy > len)
			copy = len - i;

		ReadMemory(buf, hFromProcess, fromAddr+i, copy);
		WriteMemory(hToProcess, toAddr+i, copy, buf);
	}
	return true;
}

/*
 * blank out a section of memory
 * this routine is used when the SysCallDll is not yet available
 * in the remote process (so we can't use SysCallDll::ZeroMemory)
 */
bool MemoryHelper::FillMem(HANDLE hToProcess, ADDR toAddr, int len, BYTE fill)
{
	//want a block of memory to be more efficient
	//todo: do full 4k alignment etc as per TransferMemory
	BYTE buf[1024];
	memset(buf, fill, sizeof(buf));

	while(len>0)
	{
		if(len>sizeof(buf))
		{
			WriteMemory(hToProcess, toAddr, sizeof(buf), (ADDR)buf);
			len -= sizeof(buf);
			toAddr += sizeof(buf);
		}
		else
		{
			WriteMemory(hToProcess, toAddr, len, (ADDR)buf);
			break; //was the last block
		}
	}
	return true;
}

/*****************************************************************************/

/*
 * Copy a string list (eg argv[]) between two processes
 * Return addr to use in the 'to' process (eg as argv)
 */
ADDR MemoryHelper::CopyStringListBetweenProcesses(HANDLE hFromProcess, ADDR pFromList, HANDLE hToProcess, Process* pProcToRecordMemoryIn, DWORD * pdwCount, DWORD * pdwMemSize)
{
	//calculate how large the data is
	DWORD count, datasize;

	ADDR pFrom;
	list<DWORD> string_sizes;
	list<ADDR> string_addresses;

	count=datasize=0;

	pFrom=pFromList;
	if(pFrom)
	{
		for(;;)
		{
			ADDR pStr;
			ReadMemory((ADDR)&pStr, hFromProcess, pFrom, sizeof(ADDR));
			if(pStr==NULL)
				break; //end of list

			string_addresses.push_back(pStr);

			//see how big the string is
			string s;
			s = ReadString(hFromProcess, pStr);
			DWORD strLen = s.length()+1; //include the null
			string_sizes.push_back(strLen);

			datasize += strLen;
			count++;

			pFrom += sizeof(ADDR);
		}
	}

	//how many strings?
	if(pdwCount)
		*pdwCount = count;

	//allocate space for the strings AND the list of pointers
	DWORD total = datasize + sizeof(ADDR)*(count+1); //+1 for the null array terminater
	ADDR pTo;

	pTo = AllocateMemAndProtectProcess(hToProcess, 0, total, PAGE_READWRITE);
	if(pProcToRecordMemoryIn)
		pProcToRecordMemoryIn->m_MemoryAllocations.push_back(new Process::MemoryAlloc(pTo, total, PAGE_READWRITE));
	if(pdwMemSize)
		*pdwMemSize = total;

	//copy the data, keeping track of pointers used
	ADDR pToArray = pTo;
	ADDR pToData = pTo + sizeof(ADDR)*(count+1); //write data here;
	list<DWORD>::iterator itSize;
	list<ADDR>::iterator itAddr;
	itSize=string_sizes.begin();
	itAddr=string_addresses.begin();
	while(itSize!=string_sizes.end())
	{
		//array entry (copy over value of pToData)
		WriteMemory(hToProcess, pToArray, sizeof(ADDR), (ADDR)&pToData);
		pToArray += sizeof(ADDR);

		//data
		TransferMemory(hFromProcess, *itAddr, hToProcess, pToData, *itSize);
		pToData += *itSize;

		//next (they are paired)
		++itSize;
		++itAddr;
	}
	//null terminate
	pToData = 0; //just a zero dword to write
	WriteMemory(hToProcess, pToArray, sizeof(ADDR), (ADDR)&pToData);

	return pTo; //array started here
}


string MemoryHelper::ReadString(HANDLE hFromProcess, ADDR fromAddr)
{
	string str;

	//read in 4k allocation blocks (to be faster than reading bytes)
	DWORD next4k;
	char buf[SIZE4k];
	DWORD len;
	DWORD addr;

	addr = (DWORD)fromAddr;
	for(;;)
	{
		next4k = (addr & 0xFFFFF000) + SIZE4k;
		len = next4k - addr;

		ReadMemory((ADDR)buf, hFromProcess, (ADDR)addr, len);

		for(DWORD i=0; i<len; ++i)
		{
			if(buf[i]==0)
				return str;

			str += buf[i];
		}

		//next block
		addr += len;
	}
}


/*****************************************************************************/

/*
static int GetLdtCount()
{
	PROCESS_LDT_INFORMATION information;
	information.Start = 0;
	information.Length = 8;
	NtQueryInformationProcess(GetCurrentProcess(), ProcessLdtInformation, &information, sizeof(information), NULL);
	return information.Length / sizeof(LDT_ENTRY);
}
*/

int MemoryHelper::AllocateLDTSelector(DWORD dwThreadId, bool bIsForLDT)
{
	//Look for a free LDT entry to use
	//Windows does not use LDT, so any entries are our to use, so just using internal tables to keep track

	/*
	 Windows uses only 9 GDT entries
	 And linux code thinks we are returning a GDT selector index,
	 (so it converts the index into a selector but does not set the LDT bit (bit 3)).

	 By returning LDT indexes > 9  we generate invalid GDT entries,
	 then we can trap when they are assigned to a segment register (currently GS in libpthread).
	 This trapping occurs in MemoryHelper::HandlePossibleLDTException()

	 To emulate per thread LDT behaviour (for sys_modify_ldt), we allocate an extra LDT entry.
	 This entry is the one returned to callers, but is always INVALID.
	 When we trap it's use, we then use the 'real' entry.
	*/

	for(int i=FIRST_USABLE_LDT_ENTRY; i<MAX_LDT_ENTRIES; ++i)
	{
		if(P->m_LdtEntries[i].dwAllocatingThreadId == 0)
		{
			//unallocated - can use this one

			//If the caller is requesting an LDT entry, then we make the returned index a fake
			//This means they caller will alway use this invalid entry, and subsequent called
			//to CalculateLDTEntryForThread() will allocate the correct per-thread index.
			if(bIsForLDT) {
				P->m_LdtEntries[i].bIsReallyLDT = bIsForLDT;
				P->m_LdtEntries[i].dwAllocatingThreadId = -1; //nothing will match this, users will get a new entry when they use it
				ktrace("Allocated LDT[%d] as LDT marker for thread %lx\n", i, dwThreadId);
			}
			else
			{
				P->m_LdtEntries[i].bIsReallyLDT = bIsForLDT;
				P->m_LdtEntries[i].dwAllocatingThreadId = dwThreadId;
				ktrace("Allocated LDT[%d] for thread %lx\n", i, dwThreadId);
			}

			return i;
		}
	}

	//none found
	return -1;
}

bool MemoryHelper::GetLDTSelector(DWORD dwThreadId, linux::user_desc &user_desc)
{
	//We wrote the LDT and keep a copy in the Procress.
	//Therefore we can just lookup the stored value, rather than perform all
	//the LDT_ENTRY <-> user_desc mapping that SetLDTSelector() performs.

	//calculate the real index
	int index = CalculateLDTEntryForThread(dwThreadId, user_desc.entry_number);
	if(index == 0) {
		return false; //bad index
	}
	Process::LdtData *pKernelUserDesc = &P->m_LdtEntries[index];

	memcpy(&user_desc, &pKernelUserDesc->user_desc, sizeof(linux::user_desc));
	return true;
}

/*
 * LDT read/write from: http://vxheavens.com/lib/vzo13.html
 */
bool MemoryHelper::SetLDTSelector(DWORD dwThreadId, linux::user_desc &user_desc, bool bIsReallyLDT)
{
	PROCESS_LDT_INFORMATION ldtInfo;
	LDT_ENTRY *ldt = &ldtInfo.LdtEntries[0];
	DWORD rc;

	//calculate the real index
	int index = CalculateLDTEntryForThread(dwThreadId, user_desc.entry_number);
	if(index == 0) {
		return false; //bad index
	}
	Process::LdtData *pKernelUserDesc = &P->m_LdtEntries[index];


	// See Chapter 5, 386INTEL.TXT

	//	                          DATA SEGMENT DESCRIPTOR
	// 31                23                15                7               0
	//+-----------------+-+-+-+-+---------+-+-----+---------+-----------------+
	//|                 | | | |A| LIMIT   | |     |  TYPE   |                 |
	//|   BASE 31..24   |G|B|0|V| 19..16  |P| DPL |         |   BASE 23..16   | 4
	//|                 | | | |L|         | |     |1|0|E|W|A|                 |
	//+=================+=+=+=+=+=========+=+=====+=+=+=+=+=+=================+
	//|                                   |                                   |
	//|        SEGMENT BASE 15..0         |        SEGMENT LIMIT 15..0        | 0
	//|                                   |                                   |
	//+-----------------+-----------------+-----------------+-----------------+

	//                       EXECUTABLE SEGMENT DESCRIPTOR
	// 31                23                15                7               0
	//+-----------------+-+-+-+-+---------+-+-----+---------+-----------------+
	//|                 | | | |A| LIMIT   | |     |  TYPE   |                 |
	//|   BASE 31..24   |G|D|0|V| 19..16  |P| DPL |         |   BASE 23..16   | 4
	//|                 | | | |L|         | |     |1|0|C|R|A|                 |
	//+=================+=+=+=+=+=========+=+=====+=+=+=+=+=+=================+
	//|                                   |                                   |
	//|        SEGMENT BASE 15..0         |        SEGMENT LIMIT 15..0        | 0
	//|                                   |                                   |
	//+-----------------+-----------------+-----------------+-----------------+

    //  A   - ACCESSED                              E   - EXPAND-DOWN
    //  AVL - AVAILABLE FOR PROGRAMMERS USE         G   - GRANULARITY
    //  B   - BIG                                   P   - SEGMENT PRESENT
    //  C   - CONFORMING                            R   - READABLE
    //  D   - DEFAULT                               W   - WRITABLE
    //  DPL - DESCRIPTOR PRIVILEGE LEVEL


	//NT Rules: base < 7FFF0000, limit < 7FFF0000, base+limit <= 7FFF0000
	//But we don't want to return this info to the caller, so create temp vars
	DWORD limit = user_desc.limit;
	DWORD max_limit;
	if(user_desc.limit_in_pages)
	{
		DWORD base4k = (user_desc.base_addr + 0xFFF) & 0xFFFFF000; //round-up to next 4k boundary
		max_limit = (0x7FFEFFFF - base4k) >> 12; //limit in pages
	}
	else 
	{
		max_limit = 0x7FFEFFFF - user_desc.base_addr;
	}
	if(limit > max_limit) {
		limit = max_limit;
	}


	//convert data
	//
	ZeroMemory(ldt, sizeof(LDT_ENTRY));

	ldt->BaseLow                = user_desc.base_addr & 0xFFFF;
	ldt->HighWord.Bytes.BaseMid = user_desc.base_addr >> 16;
	ldt->HighWord.Bytes.BaseHi  = user_desc.base_addr >> 24;

	ldt->LimitLow                  = limit & 0xFFFF;
	ldt->HighWord.Bits.LimitHi     = limit >> 16;
	ldt->HighWord.Bits.Granularity = user_desc.limit_in_pages; //0=bytes, 1=pages (4k)

	ldt->HighWord.Bits.Default_Big = user_desc.seg_32bit; //0=16bit, 1=32bit

	//Code or data segment?
	ldt->HighWord.Bits.Type = user_desc.read_exec_only ? 0x1b : 0x13;

	ldt->HighWord.Bits.Pres = !user_desc.seg_not_present;  //Presense bit
	ldt->HighWord.Bits.Dpl = 3;  //ALWAYS ring 3
	ldt->HighWord.Bits.Sys = 1;  // 0=sys, 1=user


	//Write LDT entry
	//
	ldtInfo.Start = index * sizeof(LDT_ENTRY); // selector --> offset
	ldtInfo.Length = sizeof(LDT_ENTRY);
	rc = LegacyWindows::NtSetInformationProcess(P->m_hProcess, ProcessLdtInformation, &ldtInfo, sizeof(ldtInfo));


	if(rc==0) { //NTSTATUS OK
		//ok
		ktrace("Set LDT[%d] for thread %lx to 0x%08lx\n", user_desc.entry_number, dwThreadId, user_desc.base_addr);
		P->DumpDescriptors();

		//record it
		pKernelUserDesc->user_desc = user_desc;
		pKernelUserDesc->bIsReallyLDT = bIsReallyLDT;

		return true;
	}
	else 
	{
		ktrace("Selector set error %lx\n", rc);
		return false;
	}

}

bool MemoryHelper::HandlePossibleLDTException(WORD instruction, ADDR exceptionAddress, CONTEXT& ctx)
{
	//This is _possibly_ an access violation occuring because the caller is attempting
	// to load an invalid GDT selector entry into FS,GS, etc.. 
	//See MemoryHelper::AllocateLDT() for why we cause this situation.

	//We detect these operations and correct the selector to be the
	// correct thread-local LDT selector.
	//and then let the process resume with the corrected value.
#define MAKE_SELECTOR(ldt_index)  ( ((ldt_index)<<3) | 0x7 ) /* LDT selector for DPL=3 */
#define EXTRACT_INDEX(selector)   ( selector >> 3 )

	// Testing for instructions that SET a Selector register
	// These causes determined by debugging the linux process with gdb and recording the specific cases.
	int index;
	switch(instruction) {

	case 0xe88e:  //GDB:  movl %eax,%gs
		ktrace("trap LDT: mov gs,eax   LDT[%d]\n", ctx.Eax>>3);
		index = CalculateLDTEntryForThread( T->dwThreadId, EXTRACT_INDEX(ctx.Eax) );
		ctx.Eax = MAKE_SELECTOR( index );
		ctx.SegGs = ctx.Eax;
		T->SegGs = ctx.Eax; //replace the to-be-injected value too
		ctx.Eip += 2;
		return true;
	}

	//not an exception that we handle
	return false;
}


// In Linux LDT (and GDT) entries are per task (= per thread)
// In Windows they are per-Process (except for the TEB entry in the GDT)
//
// We cause invalid LDT entries to be used in the linux process 
//  ( See AllocateLDTEntry() and HandlePossibleLDTException() )
//
// This function determines the correct value for any given LDT entry for a Thread
//
int MemoryHelper::CalculateLDTEntryForThread(DWORD dwThreadId, int requested_index)
{
	if(requested_index >= MAX_LDT_ENTRIES || requested_index <= 0) {
		return 0; //bad index
	}

	Process::LdtData *pRequestedUserDesc = &P->m_LdtEntries[requested_index];

	if(pRequestedUserDesc->dwAllocatingThreadId == dwThreadId)
	{
		//The entry does belong to this thread, go ahead and use it
		return requested_index;
	}

	if(pRequestedUserDesc->dwAllocatingThreadId == 0)
	{
		//badness, entry has never been allocated in the first place!
		return 0;
	}

	//The entry is for another thread.
	//Try to find the correct entry for this thread.
	//
	for(int i=FIRST_USABLE_LDT_ENTRY; i<MAX_LDT_ENTRIES; ++i) 
	{
		Process::LdtData *ud = &P->m_LdtEntries[i];
		if(ud->user_desc.entry_number == requested_index
		&& ud->dwAllocatingThreadId   == dwThreadId )
		{
			// This is the correct entry for the thread
			return i;
		}
	}

	// We did not find an entry for the thread
	// This will be because the entry was allocated in another thread and
	// would normally be expected to be usable for Thread Local Storage across threads.
	// We need to allocate an entry for this thread to use.
	int new_index = AllocateLDTSelector(dwThreadId, pRequestedUserDesc->bIsReallyLDT);
	if(new_index == -1) {
		//failed
		return 0;
	}
	ktrace("LDT lookup entry %d for thread %lx. New entry %d allocated\n", requested_index, dwThreadId, new_index);
	return new_index;
}

