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
 * int gettimeofday(timeval* tv, timezone* tz)
 * same as time() but has tv_usec too;
 */
void SysCalls::sys_gettimeofday(CONTEXT& ctx)
{
	linux::timeval * pTv = (linux::timeval*)ctx.Ebx;
	//never used for linux -   linux::timezone * tz = (linux::timezone*)pCtx->Ecx;
	linux::timeval tv;

	SYSTEMTIME st;
	FILETIME ft;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st,&ft);

	tv.tv_sec = FILETIME_TO_TIME_T(ft);
	tv.tv_usec = st.wMilliseconds;

	P->WriteMemory((ADDR)pTv, sizeof(tv), &tv);
	ctx.Eax = 0;
}

/*
 *  int clock_gettime(clockid_t clk_id, struct timespec *tp)
 */
void SysCalls::sys_clock_gettime(CONTEXT& ctx)
{
	linux::__kernel_clockid_t clockid = ctx.Ebx;
	ADDR timespecAddr = (ADDR)ctx.Ecx;

	linux::timespec timespec;

	switch(clockid)
	{
	case linux::CLOCK_REALTIME:
	case linux::CLOCK_REALTIME_COARSE:
		{
			SYSTEMTIME st;
			FILETIME ft;

			GetSystemTime(&st);
			SystemTimeToFileTime(&st, &ft);

			timespec.tv_sec = FILETIME_TO_TIME_T(ft);
			timespec.tv_nsec = st.wMilliseconds * 1000000L;
		}
		break;

	case linux::CLOCK_MONOTONIC:
	case linux::CLOCK_MONOTONIC_COARSE:
		{
			DWORD ticks = GetTickCount();
			timespec.tv_sec = ticks/1000;
			timespec.tv_nsec = (ticks%1000) * 1000000L;
		}
		break;

	case linux::CLOCK_MONOTONIC_RAW:
		{
			LARGE_INTEGER counter;
			LARGE_INTEGER freq;
			QueryPerformanceCounter(&counter);
			QueryPerformanceFrequency(&freq);

			//nanoseconds per second?
			__int64 ns_per_freq = 1000000000L / freq.QuadPart;

			timespec.tv_sec = (linux::time_t)( counter.QuadPart / freq.QuadPart );
			timespec.tv_nsec = (long)( (counter.QuadPart % freq.QuadPart) * ns_per_freq );
		}
		break;

	case linux::CLOCK_PROCESS_CPUTIME_ID:
	case linux::CLOCK_THREAD_CPUTIME_ID:
		{
			FILETIME CreationTime;
			FILETIME ExitTime;
			FILETIME KernelTime;
			FILETIME UserTime;
			if(clockid==linux::CLOCK_PROCESS_CPUTIME_ID) {
				GetProcessTimes(P->m_hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime);
			}
			else {
				GetThreadTimes(T->hThread, &CreationTime, &ExitTime, &KernelTime, &UserTime);
			}

			unsigned __int64 ns100 = ((ULARGE_INTEGER*)&UserTime)->QuadPart;
			timespec.tv_sec =(linux::time_t)NS100_TO_SECOND(ns100);
			timespec.tv_nsec = (ns100 % SECOND_TO_100NS(1)) * 100L;
		}
		break;

	default:
		SysCalls::Unhandled(ctx);
		return;
	}

	P->WriteMemory(timespecAddr, sizeof(timespec), &timespec);
	ctx.Eax = 0;
}
