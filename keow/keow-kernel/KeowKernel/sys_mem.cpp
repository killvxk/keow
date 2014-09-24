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
 * unsigned long brk(unsigned long end_data_segment)
 *
 * In Linux the brk (program break) is the top of the data segment of the process
 * It starts at process creation time as the end of the bss segment,
 * and continues to grow (and shrink) from there.
 */
void SysCalls::sys_brk(CONTEXT& ctx)
{
	ADDR p;
	ADDR old_brk = P->m_ElfLoadData.brk;
	ADDR new_brk = (ADDR)ctx.Ebx;

	if(new_brk == 0)
	{
		//return current location
		ctx.Eax = (DWORD)P->m_ElfLoadData.brk;
		ktrace("brk(0) = 0x%08lx\n", ctx.Eax);
		return;
	}


	//only allow growing for now
	//allocate all memory between old brk and new brk
	if(new_brk < old_brk)
	{
		ctx.Eax = -1; //failed
		return;
	}
	if(new_brk > old_brk)
	{
		ktrace("brk requested break @ 0x%08lx\n", new_brk);
		p = MemoryHelper::AllocateMemAndProtect(old_brk, new_brk-old_brk, PAGE_EXECUTE_READWRITE);//PAGE_READWRITE);
		if(p==(ADDR)-1)
		{
			ctx.Eax = -1;
			ktrace("out of memory in sys_brk?\n");
			return;
		}
		//dont do this or else things fail?
		//ZeroMemory(old_brk, new_brk-old_brk);
	}

	P->m_ElfLoadData.brk = new_brk;
	ktrace("brk(0x%08lx) = 0x%08lx\n", ctx.Ebx, new_brk);
	ctx.Eax = (DWORD)new_brk;
	return;
}


/*****************************************************************************/


/*
 * unsigned long sys_mmap(mmap_arg_struct* args)
 *
 * Map a section of a file into memory, or alternatively a peice of swap file (no fd). 
 */
void SysCalls::sys_mmap(CONTEXT& ctx)
{
	linux::mmap_arg_struct args;
	DWORD err = -linux::EINVAL;
	HANDLE hMap, hFile;
	DWORD ProtOpen, ProtMap;
	void *p;
	DWORD dwFileSize;
	IOHFile * ioh = NULL;

	P->ReadMemory(&args, (ADDR)ctx.Ebx, sizeof(args));
	ktrace("mmap(fd %d, offset 0x%08lx, len 0x%08lx, addr 0x%08lx)\n", args.fd, args.offset, args.len, args.addr);

	if(args.flags & linux::MAP_ANONYMOUS)
		args.fd = -1; //swap

	//addr must be aligned
	if((args.addr & ~(SIZE4k-1)) != args.addr)
	{
		ctx.Eax = -linux::EINVAL;
		return;
	}

	if(args.fd != -1)
	{
		if(args.fd<0 || args.fd>MAX_OPEN_FILES)
		{
			ctx.Eax = -linux::EBADF;
			return;
		}
		ioh = dynamic_cast<IOHFile*>(P->m_OpenFiles[args.fd]);
		if(ioh==NULL)
		{
			ctx.Eax = -linux::EACCES; //not a simple file - can't handle that
			return;
		}

		__int64 len = ioh->Length();
		if(len>0x7fffffffL)
			dwFileSize = 0x7fffffffL;
		else
			dwFileSize = (DWORD)len;
	}


	//allocations are rounded up to 4k boundaries
	int len4k = (args.len+0xFFFL) & ~0xFFFL;


	//Need to unmap anything already there
	// void munmap(void* start, unsigned long len)
	if(args.addr!=0)
	{
		ktrace("unmap previous contents\n");
		CONTEXT ctxUnmap = ctx;
		ctxUnmap.Ebx = args.addr;
		ctxUnmap.Ecx = len4k;
		sys_munmap(ctxUnmap);
		ktrace("resuming allocation\n");
	}


	//If an address or offset is supplied, then it is probably 
	// 4k aligned (linux x86 page size). However win32 needs uses
	// a 64k boundary for file mappings. If they don't match then
	// MapViewOfFile cannot work.
	// However.. often mmap is a shortcut to loading an executable
	// into memory (readonly at a fixed address, so just read the 
	// file and ignore that we won't get any future updates.
	// If copy-on-write was requested then this means we get our own 
	// copy of the data anyhow so just read it and take the extra
	// swap space hit and IO hit.
	// Also linux allows(?) mmap larger than the underlying file
	// size to get extra memory. Win32 needs to grow the file to do
	// this and we don't want that, so read the file then too.
	// And! linux seems to alloc mmap over the top of an old map
	// area, which can't happen in win32. Therefore unless we
	// need to write (not copy-on-write) to the map, then always
	// allocate memory and read to get around all these problems.
	if((args.addr & 0xFFFF00000) != args.addr	    /*64k test*/
	|| (args.offset & 0xFFFF0000) != args.offset  /*64k test*/
	|| ((args.fd != -1) && (args.offset+args.len) > dwFileSize)  /*mmap larger than file*/
	|| ((args.prot & linux::PROT_WRITE)==0 || (args.flags & linux::MAP_SHARED)==0)  /* ReadOnly or private map (not write shared) */
	) {

		//can't handle shared write access in these scenarios
		if((args.prot & linux::PROT_WRITE) && (args.flags & linux::MAP_SHARED))
		{
			ctx.Eax = -linux::EINVAL; //invalid arguments
			return;
		}

		ktrace("fake mmap, len 0x%08lx\n", len4k);

		//allocate the requested mem size
		if(args.prot & linux::PROT_WRITE)
			ProtMap = PAGE_READWRITE;
		else
			ProtMap = PAGE_READONLY;
		p = MemoryHelper::AllocateMemAndProtect((ADDR)args.addr, len4k, PAGE_EXECUTE_READWRITE); //correct prot after we write mem
		if(p==(void*)-1)
		{
			ctx.Eax = -linux::ENOMEM;
			return;
		}

		//initialise it
		SysCallDll::ZeroMem(p, len4k);

		//read file into the memory (unless using swap)
		if(ioh != NULL)
		{
			DWORD dwReadLen;

			dwReadLen = args.len;
			if(args.offset+args.len > dwFileSize)
				dwReadLen = dwFileSize - args.offset;

			if(ioh->Seek(args.offset, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
			{
				ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError());
				return;
			}
			if(!ioh->Read(p, dwReadLen, &dwReadLen))
			{
				ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError());
				return;
			}
		}

//can't change protection within the 64k once set?
//		LegacyWindows::VirtualProtect(p, args.len, ProtMap, &ProtOpen);
		if(args.flags & linux::MAP_GROWSDOWN)
			p = (LPBYTE)p + args.len;
		ktrace("mmap'ed to 0x%08lx - 0x%08lx\n", p, (DWORD)p+len4k-1);
		ctx.Eax = (DWORD)p;
		return;
	}



	//the following is standard win32 file mappings
	

	// memory so we can just read it ourselves.
	// we don't get any future updates 
	// mapping 
	//protection needed
	if(args.prot & linux::PROT_WRITE)
	{
		if(args.flags & linux::MAP_PRIVATE)
		{
			ProtOpen = PAGE_WRITECOPY;
			ProtMap = FILE_MAP_COPY;
		}
		else
		{
			ProtOpen = PAGE_READWRITE;
			ProtMap = FILE_MAP_WRITE;
		}
	}
	else
	{
		ProtOpen = PAGE_READONLY;
		ProtMap = FILE_MAP_READ;
	}


	//if fd is invalid, use swapfile
	if(ioh==NULL)
		hFile=INVALID_HANDLE_VALUE;
	else
		hFile=ioh->GetRemoteHandle();

	//map the file (hMap & hFile are in the user process)
	hMap = (HANDLE)SysCallDll::CreateFileMapping(hFile, ProtOpen, 0, args.offset+args.len);
	if(hMap==NULL)
	{
		ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError());
		return;
	}
	p = (void*)SysCallDll::MapViewOfFileEx(hMap, ProtMap, 0,args.offset, args.len, (void*)args.addr);
	if(p==0)
	{
		ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError());
		return;
	}

	P->m_MmapList.push_back(new Process::MMapRecord(args.fd, hMap, (ADDR)p, args.offset, args.len, ProtMap));
	//leave open - CloseHandle(hMap); -its on the other process anyway

	//opened - return actual addr
	if(args.flags & linux::MAP_GROWSDOWN)
		p = (LPBYTE)p + args.len;
	ktrace("mmap'ed to 0x%08lx - 0x%08lx\n", p, (DWORD)p+args.len-1);
	ctx.Eax = (DWORD)p;
	return;
}

/*****************************************************************************/

/*
 * void munmap(void* start, unsigned long len)
 */
void SysCalls::sys_munmap(CONTEXT& ctx)
{
	ADDR addr = (ADDR)ctx.Ebx;
	DWORD len = ctx.Ecx;

	//try Unmap first (may not actually be a map - see sys_old_mmap)
	Process::MmapList::iterator it;
	for(it=P->m_MmapList.begin(); it!=P->m_MmapList.end(); ++it)
	{
		DebugBreak();//may need to split the mapping into two peices (before & after)?
		Process::MMapRecord * pRec = *it;
		if(pRec->Address == addr)
		{
			if(SysCallDll::UnmapViewOfFile(addr))
				ctx.Eax = 0;
			else
				ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError());
			P->m_MmapList.erase(it);
			return;
		}
		if(pRec->Address<=addr && (pRec->Address+pRec->len)>=addr)
		{
			//need to split the mapping into two peices (before & after)?
			DebugBreak();
			return;
		}
	}

	//was done via just plain memory (& file read)
	if(!MemoryHelper::DeallocateMemory(addr, len))
	{
		ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError());
		return;
	}
	ctx.Eax = 0;
}


/*****************************************************************************/

/*
 * int mprotect(const void *addr, size_t len, int prot)
 */
void SysCalls::sys_mprotect(CONTEXT& ctx)
{
	void* addr = (void*)ctx.Ebx;
	DWORD len = ctx.Ecx;
	DWORD prot = ctx.Edx;

	//for now we are not doing protection
	//pretend we succeeded
	ctx.Eax = 0;
}


/*****************************************************************************/


/*
 * int set_thread_area(struct user_desc *u_info);
 */
void SysCalls::sys_set_thread_area(CONTEXT& ctx)
{
	ADDR user_desc_Addr = (ADDR)ctx.Ebx;

	// These should be GDT entries, but we can't emulate them (we don't run in ring 0 ?)
	// So instead we reuse the LDT code

	//read user_desc from process
	linux::user_desc user_desc;
	P->ReadMemory(&user_desc, user_desc_Addr, sizeof(user_desc));

	//default return, unless we succeed
	ctx.Eax = -linux::EINVAL;

	//may need to allocate a selector
	if(user_desc.entry_number == -1) {
		user_desc.entry_number = MemoryHelper::AllocateLDTSelector(T->dwThreadId, false);
		if(user_desc.entry_number == -1) {
			//no room left
			ctx.Eax = -linux::ESRCH;
			return;
		}
	}

	if(MemoryHelper::SetLDTSelector(T->dwThreadId, user_desc, false))
	{
		//write result back to process
		P->WriteMemory(user_desc_Addr, sizeof(user_desc), &user_desc);
		ctx.Eax = 0;
	}
}

/*
 * int get_thread_area(struct user_desc *u_info);
 */
void SysCalls::sys_get_thread_area(CONTEXT& ctx)
{
	ADDR user_desc_Addr = (ADDR)ctx.Ebx;

	// These should be GDT entries, but we can't emulate them (we don't run in ring 0 ?)
	// So instead we reuse the LDT code: sys_modify_ldt

	//read user_desc from process
	linux::user_desc user_desc;
	P->ReadMemory(&user_desc, user_desc_Addr, sizeof(user_desc));

	//default return, unless we succeed
	ctx.Eax = -linux::EINVAL;

	if(MemoryHelper::GetLDTSelector(T->dwThreadId, user_desc))
	{
		//write result back to process
		P->WriteMemory(user_desc_Addr, sizeof(user_desc), &user_desc);
		ctx.Eax = 0;
	}
}

/*
 * int modify_ldt(int func, void *ptr, unsigned long bytecount);
 */
void SysCalls::sys_modify_ldt(CONTEXT& ctx)
{
	DWORD modify_function = ctx.Ebx;
	ADDR user_desc_Addr = (ADDR)ctx.Ecx;
	DWORD user_desc_Size = ctx.Edx;

	linux::user_desc user_desc;

	//read user_desc from process
	if(user_desc_Size != sizeof(user_desc)) {
		ctx.Eax = -linux::EINVAL;
		return;
	}
	P->ReadMemory(&user_desc, user_desc_Addr, sizeof(user_desc));

	//default return, unless we succeed
	ctx.Eax = -linux::EINVAL;

	switch(modify_function) 
	{
	case 0: //READ LDT
		if(MemoryHelper::GetLDTSelector(T->dwThreadId, user_desc))
		{
			//write result back to process
			P->WriteMemory(user_desc_Addr, sizeof(user_desc), &user_desc);
			ctx.Eax = 0;
		}
		break;
	case 1: //WRITE LDT
		if(MemoryHelper::SetLDTSelector(T->dwThreadId, user_desc, true))
		{
			//write result back to process
			P->WriteMemory(user_desc_Addr, sizeof(user_desc), &user_desc);
			ctx.Eax = 0;
		}
		break;

	default:
		ctx.Eax = -linux::ENOSYS; //no such function
		break;
	}

}
