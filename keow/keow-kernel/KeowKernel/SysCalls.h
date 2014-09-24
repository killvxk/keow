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

// SysCalls.h: interface for the SysCalls class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYSCALLS_H__A161E436_6A4E_4634_A139_C68E9ABAD424__INCLUDED_)
#define AFX_SYSCALLS_H__A161E436_6A4E_4634_A139_C68E9ABAD424__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class SysCalls  
{
public:
	static void InitSysCallTable();
	static void HandleInt80SysCall(CONTEXT& ctx);

protected:
	typedef void (*SYSCALL_HANDLER)(CONTEXT& ctx);

	static SYSCALL_HANDLER syscall_handlers[linux::NR_syscalls];
	static const char* syscall_names[linux::NR_syscalls];

private:
	SysCalls() {}

	static void Unhandled(CONTEXT& ctx);

//syscall prototypes
#define PROTO_SYSCALL(n) static void sys_##n(CONTEXT& ctx)

	PROTO_SYSCALL(exit);
	PROTO_SYSCALL(fork);
	PROTO_SYSCALL(read);
	PROTO_SYSCALL(write);
	PROTO_SYSCALL(open);
	PROTO_SYSCALL(close);
	PROTO_SYSCALL(waitpid);
	PROTO_SYSCALL(creat);
	PROTO_SYSCALL(link);
	PROTO_SYSCALL(unlink);
	PROTO_SYSCALL(execve);
	PROTO_SYSCALL(chdir);
	PROTO_SYSCALL(time);
	PROTO_SYSCALL(mknod);
	PROTO_SYSCALL(chmod);
	PROTO_SYSCALL(lchown);
	PROTO_SYSCALL(lseek);
	PROTO_SYSCALL(getpid);
	PROTO_SYSCALL(mount);
	PROTO_SYSCALL(umount);
	PROTO_SYSCALL(setuid);
	PROTO_SYSCALL(getuid);
	PROTO_SYSCALL(stime);
	PROTO_SYSCALL(ptrace);
	PROTO_SYSCALL(alarm);
	PROTO_SYSCALL(oldfstat);
	PROTO_SYSCALL(pause);
	PROTO_SYSCALL(utime);
	PROTO_SYSCALL(stty);
	PROTO_SYSCALL(gtty);
	PROTO_SYSCALL(access);
	PROTO_SYSCALL(nice);
	PROTO_SYSCALL(ftime);
	PROTO_SYSCALL(sync);
	PROTO_SYSCALL(kill);
	PROTO_SYSCALL(rename);
	PROTO_SYSCALL(mkdir);
	PROTO_SYSCALL(rmdir);
	PROTO_SYSCALL(dup);
	PROTO_SYSCALL(pipe);
	PROTO_SYSCALL(times);
	PROTO_SYSCALL(prof);
	PROTO_SYSCALL(brk);
	PROTO_SYSCALL(setgid);
	PROTO_SYSCALL(getgid);
	PROTO_SYSCALL(signal);
	PROTO_SYSCALL(geteuid);
	PROTO_SYSCALL(getegid);
	PROTO_SYSCALL(acct);
	PROTO_SYSCALL(umount2);
	PROTO_SYSCALL(lock);
	PROTO_SYSCALL(ioctl);
	PROTO_SYSCALL(fcntl);
	PROTO_SYSCALL(mpx);
	PROTO_SYSCALL(setpgid);
	PROTO_SYSCALL(ulimit);
	PROTO_SYSCALL(oldolduname);
	PROTO_SYSCALL(umask);
	PROTO_SYSCALL(chroot);
	PROTO_SYSCALL(ustat);
	PROTO_SYSCALL(dup2);
	PROTO_SYSCALL(getppid);
	PROTO_SYSCALL(getpgrp);
	PROTO_SYSCALL(setsid);
	PROTO_SYSCALL(sigaction);
	PROTO_SYSCALL(sgetmask);
	PROTO_SYSCALL(ssetmask);
	PROTO_SYSCALL(setreuid);
	PROTO_SYSCALL(setregid);
	PROTO_SYSCALL(sigsuspend);
	PROTO_SYSCALL(sigpending);
	PROTO_SYSCALL(sethostname);
	PROTO_SYSCALL(setrlimit);
	PROTO_SYSCALL(getrlimit);	/* Back compatible);Gig limited rlimit */
	PROTO_SYSCALL(getrusage);
	PROTO_SYSCALL(gettimeofday);
	PROTO_SYSCALL(settimeofday);
	PROTO_SYSCALL(getgroups);
	PROTO_SYSCALL(setgroups);
	PROTO_SYSCALL(select);
	PROTO_SYSCALL(symlink);
	PROTO_SYSCALL(oldlstat);
	PROTO_SYSCALL(readlink);
	PROTO_SYSCALL(uselib);
	PROTO_SYSCALL(swapon);
	PROTO_SYSCALL(reboot);
	PROTO_SYSCALL(readdir);
	PROTO_SYSCALL(mmap);
	PROTO_SYSCALL(munmap);
	PROTO_SYSCALL(truncate);
	PROTO_SYSCALL(ftruncate);
	PROTO_SYSCALL(fchmod);
	PROTO_SYSCALL(fchown);
	PROTO_SYSCALL(getpriority);
	PROTO_SYSCALL(setpriority);
	PROTO_SYSCALL(profil);
	PROTO_SYSCALL(statfs);
	PROTO_SYSCALL(fstatfs);
	PROTO_SYSCALL(ioperm);
	PROTO_SYSCALL(socketcall);
	PROTO_SYSCALL(syslog);
	PROTO_SYSCALL(setitimer);
	PROTO_SYSCALL(getitimer);
	PROTO_SYSCALL(stat);
	PROTO_SYSCALL(lstat);
	PROTO_SYSCALL(fstat);
	PROTO_SYSCALL(olduname);
	PROTO_SYSCALL(iopl);
	PROTO_SYSCALL(vhangup);
	PROTO_SYSCALL(idle);
	PROTO_SYSCALL(vm86old);
	PROTO_SYSCALL(wait4);
	PROTO_SYSCALL(swapoff);
	PROTO_SYSCALL(sysinfo);
	PROTO_SYSCALL(ipc);
	PROTO_SYSCALL(fsync);
	PROTO_SYSCALL(sigreturn);
	PROTO_SYSCALL(clone);
	PROTO_SYSCALL(setdomainname);
	PROTO_SYSCALL(uname);
	PROTO_SYSCALL(modify_ldt);
	PROTO_SYSCALL(adjtimex);
	PROTO_SYSCALL(mprotect);
	PROTO_SYSCALL(sigprocmask);
	PROTO_SYSCALL(create_module);
	PROTO_SYSCALL(init_module);
	PROTO_SYSCALL(delete_module);
	PROTO_SYSCALL(get_kernel_syms);
	PROTO_SYSCALL(quotactl);
	PROTO_SYSCALL(getpgid);
	PROTO_SYSCALL(fchdir);
	PROTO_SYSCALL(bdflush);
	PROTO_SYSCALL(sysfs);
	PROTO_SYSCALL(personality);
	PROTO_SYSCALL(afs_syscall); /* Syscall for Andrew File System */
	PROTO_SYSCALL(setfsuid);
	PROTO_SYSCALL(setfsgid);
	PROTO_SYSCALL(_llseek);
	PROTO_SYSCALL(getdents);
	PROTO_SYSCALL(_newselect);
	PROTO_SYSCALL(flock);
	PROTO_SYSCALL(msync);
	PROTO_SYSCALL(readv);
	PROTO_SYSCALL(writev);
	PROTO_SYSCALL(getsid);
	PROTO_SYSCALL(fdatasync);
	PROTO_SYSCALL(_sysctl);
	PROTO_SYSCALL(mlock);
	PROTO_SYSCALL(munlock);
	PROTO_SYSCALL(mlockall);
	PROTO_SYSCALL(munlockall);
	PROTO_SYSCALL(sched_setparam);
	PROTO_SYSCALL(sched_getparam);
	PROTO_SYSCALL(sched_setscheduler);
	PROTO_SYSCALL(sched_getscheduler);
	PROTO_SYSCALL(sched_yield);
	PROTO_SYSCALL(sched_get_priority_max);
	PROTO_SYSCALL(sched_get_priority_min);
	PROTO_SYSCALL(sched_rr_get_interval);
	PROTO_SYSCALL(nanosleep);
	PROTO_SYSCALL(mremap);
	PROTO_SYSCALL(setresuid);
	PROTO_SYSCALL(getresuid);
	PROTO_SYSCALL(vm86);
	PROTO_SYSCALL(query_module);
	PROTO_SYSCALL(poll);
	PROTO_SYSCALL(nfsservctl);
	PROTO_SYSCALL(setresgid);
	PROTO_SYSCALL(getresgid);
	PROTO_SYSCALL(prctl);
	PROTO_SYSCALL(rt_sigreturn);
	PROTO_SYSCALL(rt_sigaction);
	PROTO_SYSCALL(rt_sigprocmask);
	PROTO_SYSCALL(rt_sigpending);
	PROTO_SYSCALL(rt_sigtimedwait);
	PROTO_SYSCALL(rt_sigqueueinfo);
	PROTO_SYSCALL(rt_sigsuspend);
	PROTO_SYSCALL(pread);
	PROTO_SYSCALL(pwrite);
	PROTO_SYSCALL(chown);
	PROTO_SYSCALL(getcwd);
	PROTO_SYSCALL(capget);
	PROTO_SYSCALL(capset);
	PROTO_SYSCALL(sigaltstack);
	PROTO_SYSCALL(sendfile);
	PROTO_SYSCALL(getpmsg);	/* some people actually want streams */
	PROTO_SYSCALL(putpmsg);	/* some people actually want streams */
	PROTO_SYSCALL(vfork);
	PROTO_SYSCALL(ugetrlimit);	/* SuS compliant getrlimit */
	PROTO_SYSCALL(mmap2);
	PROTO_SYSCALL(truncate64);
	PROTO_SYSCALL(ftruncate64);
	PROTO_SYSCALL(stat64);
	PROTO_SYSCALL(lstat64);
	PROTO_SYSCALL(fstat64);
	PROTO_SYSCALL(lchown32);
	PROTO_SYSCALL(getuid32);
	PROTO_SYSCALL(getgid32);
	PROTO_SYSCALL(geteuid32);
	PROTO_SYSCALL(getegid32);
	PROTO_SYSCALL(setreuid32);
	PROTO_SYSCALL(setregid32);
	PROTO_SYSCALL(getgroups32);
	PROTO_SYSCALL(setgroups32);
	PROTO_SYSCALL(fchown32);
	PROTO_SYSCALL(setresuid32);
	PROTO_SYSCALL(getresuid32);
	PROTO_SYSCALL(setresgid32);
	PROTO_SYSCALL(getresgid32);
	PROTO_SYSCALL(chown32);
	PROTO_SYSCALL(setuid32);
	PROTO_SYSCALL(setgid32);
	PROTO_SYSCALL(setfsuid32);
	PROTO_SYSCALL(setfsgid32);
	PROTO_SYSCALL(pivot_root);
	PROTO_SYSCALL(mincore);
	PROTO_SYSCALL(madvise);
	PROTO_SYSCALL(madvise1);	/* delete when C lib stub is removed */
	PROTO_SYSCALL(getdents64);
	PROTO_SYSCALL(fcntl64);
	PROTO_SYSCALL(security);	/* syscall for security modules */
	PROTO_SYSCALL(gettid);
	PROTO_SYSCALL(readahead);
	PROTO_SYSCALL(setxattr);
	PROTO_SYSCALL(lsetxattr);
	PROTO_SYSCALL(fsetxattr);
	PROTO_SYSCALL(getxattr);
	PROTO_SYSCALL(lgetxattr);
	PROTO_SYSCALL(fgetxattr);
	PROTO_SYSCALL(listxattr);
	PROTO_SYSCALL(llistxattr);
	PROTO_SYSCALL(flistxattr);
	PROTO_SYSCALL(removexattr);
	PROTO_SYSCALL(lremovexattr);
	PROTO_SYSCALL(fremovexattr);
	PROTO_SYSCALL(tkill);
	PROTO_SYSCALL(sendfile64);
	PROTO_SYSCALL(futex);
	PROTO_SYSCALL(sched_setaffinity);
	PROTO_SYSCALL(sched_getaffinity);
	PROTO_SYSCALL(set_thread_area);
	PROTO_SYSCALL(get_thread_area);
	PROTO_SYSCALL(io_setup);
	PROTO_SYSCALL(io_destroy);
	PROTO_SYSCALL(io_getevents);
	PROTO_SYSCALL(io_submit);
	PROTO_SYSCALL(io_cancel);
	PROTO_SYSCALL(alloc_hugepages);
	PROTO_SYSCALL(free_hugepages);
	PROTO_SYSCALL(exit_group);

	//Added when moving from 2.4 to 2.6.35 kernel
	//....

	PROTO_SYSCALL(lookup_dcookie);
	PROTO_SYSCALL(epoll_create);
	PROTO_SYSCALL(epoll_ctl);
	PROTO_SYSCALL(epoll_wait);
	PROTO_SYSCALL(remap_file_pages);
	PROTO_SYSCALL(set_tid_address);
	PROTO_SYSCALL(timer_create);
	PROTO_SYSCALL(timer_settime);
	PROTO_SYSCALL(timer_gettime);
	PROTO_SYSCALL(timer_getoverrun);
	PROTO_SYSCALL(timer_delete);
	PROTO_SYSCALL(clock_settime);
	PROTO_SYSCALL(clock_gettime);
	PROTO_SYSCALL(clock_getres);
	PROTO_SYSCALL(clock_nanosleep);
	PROTO_SYSCALL(statfs64);
	PROTO_SYSCALL(fstatfs64);
	PROTO_SYSCALL(tgkill);
	PROTO_SYSCALL(utimes);
	PROTO_SYSCALL(fadvise64_64);
	PROTO_SYSCALL(vserver);
	PROTO_SYSCALL(mbind);
	PROTO_SYSCALL(get_mempolicy);
	PROTO_SYSCALL(set_mempolicy);
	PROTO_SYSCALL(mq_open);
	PROTO_SYSCALL(mq_unlink);
	PROTO_SYSCALL(mq_timedsend);
	PROTO_SYSCALL(mq_timedreceive);
	PROTO_SYSCALL(mq_notify);
	PROTO_SYSCALL(mq_getsetattr);
	PROTO_SYSCALL(kexec_load);
	PROTO_SYSCALL(waitid);
	/* PROTO_SYSCALL(sys_setaltroot);*/
	PROTO_SYSCALL(add_key);
	PROTO_SYSCALL(request_key);
	PROTO_SYSCALL(keyctl);
	PROTO_SYSCALL(ioprio_set);
	PROTO_SYSCALL(ioprio_get);
	PROTO_SYSCALL(inotify_init);
	PROTO_SYSCALL(inotify_add_watch);
	PROTO_SYSCALL(inotify_rm_watch);
	PROTO_SYSCALL(migrate_pages);
	PROTO_SYSCALL(openat);
	PROTO_SYSCALL(mkdirat);
	PROTO_SYSCALL(mknodat);
	PROTO_SYSCALL(fchownat);
	PROTO_SYSCALL(futimesat);
	PROTO_SYSCALL(fstatat64);
	PROTO_SYSCALL(unlinkat);
	PROTO_SYSCALL(renameat);
	PROTO_SYSCALL(linkat);
	PROTO_SYSCALL(symlinkat);
	PROTO_SYSCALL(readlinkat);
	PROTO_SYSCALL(fchmodat);
	PROTO_SYSCALL(faccessat);
	PROTO_SYSCALL(pselect6);
	PROTO_SYSCALL(ppoll);
	PROTO_SYSCALL(unshare);
	PROTO_SYSCALL(set_robust_list);
	PROTO_SYSCALL(get_robust_list);
	PROTO_SYSCALL(splice);
	PROTO_SYSCALL(sync_file_range);
	PROTO_SYSCALL(tee);
	PROTO_SYSCALL(vmsplice);
	PROTO_SYSCALL(move_pages);
	PROTO_SYSCALL(getcpu);
	PROTO_SYSCALL(epoll_pwait);
	PROTO_SYSCALL(utimensat);
	PROTO_SYSCALL(signalfd);
	PROTO_SYSCALL(timerfd_create);
	PROTO_SYSCALL(eventfd);
	PROTO_SYSCALL(fallocate);
	PROTO_SYSCALL(timerfd_settime);
	PROTO_SYSCALL(timerfd_gettime);
	PROTO_SYSCALL(signalfd4);
	PROTO_SYSCALL(eventfd2);
	PROTO_SYSCALL(epoll_create1);
	PROTO_SYSCALL(dup3);
	PROTO_SYSCALL(pipe2);
	PROTO_SYSCALL(inotify_init1);
	PROTO_SYSCALL(preadv);
	PROTO_SYSCALL(pwritev);
	PROTO_SYSCALL(rt_tgsigqueueinfo);
	PROTO_SYSCALL(perf_event_open);
	PROTO_SYSCALL(recvmmsg);

#undef PROTO_SYSCALL
};

#endif // !defined(AFX_SYSCALLS_H__A161E436_6A4E_4634_A139_C68E9ABAD424__INCLUDED_)
