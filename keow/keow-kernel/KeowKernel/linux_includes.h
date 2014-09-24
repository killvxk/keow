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

#ifndef LINUX_INCLUDES_H
#define LINUX_INCLUDES_H


#include "linux/linux_defines.h"
#include "linux/linux_types.h"



//bit manipulation helpers for fd_set etc
//based on linux kernel versions
inline void KEOW_FD_SET(int fd, linux::fd_set * pSet) {
	__asm {
		push ebx
		mov eax,fd
		mov ebx, pSet
		bts [ebx],eax
		pop ebx
	}
}
inline void KEOW_FD_CLR(int fd, linux::fd_set * pSet) {
	__asm {
		push ebx
		mov eax,fd
		mov ebx, pSet
		btr [ebx],eax
		pop ebx
	}
}
inline BYTE KEOW_FD_ISSET(int fd, linux::fd_set * pSet) {
	BYTE result;
	__asm {
		push ebx
		mov eax,fd
		mov ebx, pSet;
		bt [ebx],eax
		setb result
		pop ebx
	}
	return result;
}
inline void KEOW_FD_ZERO(linux::fd_set * pSet) {
	memset(pSet, 0, linux::__FDSET_LONGS);
}
/*
#define __FD_SET(fd,fdsetp) \
		__asm__ __volatile__("btsl %1,%0": \
			"=m" (*(__kernel_fd_set *) (fdsetp)):"r" ((int) (fd)))

#define __FD_CLR(fd,fdsetp) \
		__asm__ __volatile__("btrl %1,%0": \
			"=m" (*(__kernel_fd_set *) (fdsetp)):"r" ((int) (fd)))

#define __FD_ISSET(fd,fdsetp) (__extension__ ({ \
		unsigned char __result; \
		__asm__ __volatile__("btl %1,%2 ; setb %0" \
			:"=q" (__result) :"r" ((int) (fd)), \
			"m" (*(__kernel_fd_set *) (fdsetp))); \
		__result; }))

#define __FD_ZERO(fdsetp) \
do { \
	int __d0, __d1; \
	__asm__ __volatile__("cld ; rep ; stosl" \
			:"=m" (*(__kernel_fd_set *) (fdsetp)), \
			  "=&c" (__d0), "=&D" (__d1) \
			:"a" (0), "1" (__FDSET_LONGS), \
			"2" ((__kernel_fd_set *) (fdsetp)) : "memory"); \
} while (0)
*/


#endif // LINUX_INCLUDES_H
