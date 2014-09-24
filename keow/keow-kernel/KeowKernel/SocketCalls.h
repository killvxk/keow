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

// SocketCalls.h: interface for the SocketCalls class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETCALLS_H__BEFB0766_48E6_4C88_A8C1_74FD826CD1DD__INCLUDED_)
#define AFX_SOCKETCALLS_H__BEFB0766_48E6_4C88_A8C1_74FD826CD1DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class SocketCalls  
{
public:
	static void sys_socketcall(CONTEXT &ctx);

	static int sys_socket(int domain, int type, int protocol);
	static int sys_bind(int sockfd, linux::sockaddr *my_addr, int addrlen);
	static int sys_connect(int  sockfd,  const linux::sockaddr *serv_addr, int addrlen);
	static int sys_listen(int sockfd, int backlog);
	static int sys_accept(int sockfd, linux::sockaddr *addr, int *addrlen);
	static int sys_getsockname(int sockfd, linux::sockaddr *addr, int *addrlen);
	static int sys_getpeername(int sockfd, linux::sockaddr *addr, int *addrlen);
	static int sys_socketpair(int domain, int type, int protocol, int * sv);
	static int sys_send(int sockfd, const void *buf, linux::size_t len, int flags);
	static int sys_sendto(int sockfd, const void *buf, linux::size_t len, int flags, const linux::sockaddr *serv_addr, int addrlen);
	static int sys_recv(int sockfd, void *buf, linux::size_t len, int flags);
	static int sys_recvfrom(int sockfd, void *buf, linux::size_t len, int flags, linux::sockaddr *src_addr, int *addrlen);
	static int sys_shutdown(int sockfd, int how);
	static int sys_setsockopt(int sockfd, int level, int optname, const void *optval, int optlen);
	static int sys_getsockopt(int sockfd, int level, int optname, void *optval, int *optlen);
	static int sys_sendmsg(int sockfd, linux::msghdr *msg, int flags);
	static int sys_recvmsg(int sockfd, linux::msghdr *msg, int flags);

private:
	SocketCalls() {}
};

#endif // !defined(AFX_SOCKETCALLS_H__BEFB0766_48E6_4C88_A8C1_74FD826CD1DD__INCLUDED_)
