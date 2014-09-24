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

/*
 * This is an include file based on linux kernel sources.
 * It helps describe the user-kernel interface.
 * We have this rather than the actual kernel headers because
 * there are too many #defines that would conflict the windows
 * versions (which are often different values. eg AF_INET6 in sockets).
 * This is a pair with linux/defines.h
 */

#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#pragma pack(push,1)	//all linux structures are byte aligned?


//all vars in own namespace to ease co-existence with windows ones
namespace linux {


typedef signed char		__s8;
typedef unsigned char	__u8;
typedef signed short	__s16;
typedef unsigned short	__u16;
typedef signed int		__s32;
typedef unsigned int	__u32;
typedef signed __int64	__s64;
typedef unsigned __int64 __u64;


typedef __int64	__kernel_loff_t;
typedef __kernel_loff_t loff_t;



typedef struct {
        unsigned long fds_bits [__FDSET_LONGS];
} __kernel_fd_set;



typedef unsigned short  __kernel_dev_t;
typedef unsigned long   __kernel_ino_t;
typedef unsigned short  __kernel_mode_t;
typedef unsigned short  __kernel_nlink_t;
typedef long            __kernel_off_t;
typedef int             __kernel_pid_t;
typedef unsigned short  __kernel_ipc_pid_t;
typedef unsigned short  __kernel_uid_t;
typedef unsigned short  __kernel_gid_t;
typedef unsigned int    __kernel_size_t;
typedef int             __kernel_ssize_t;
typedef int             __kernel_ptrdiff_t;
typedef long            __kernel_time_t;
typedef long            __kernel_suseconds_t;
typedef long            __kernel_clock_t;
typedef int             __kernel_timer_t;
typedef int             __kernel_clockid_t;
typedef int             __kernel_daddr_t;
typedef char *          __kernel_caddr_t;
typedef unsigned short  __kernel_uid16_t;
typedef unsigned short  __kernel_gid16_t;
typedef unsigned int    __kernel_uid32_t;
typedef unsigned int    __kernel_gid32_t;

typedef unsigned short  __kernel_old_uid_t;
typedef unsigned short  __kernel_old_gid_t;


typedef __kernel_fd_set         fd_set;
typedef __kernel_dev_t          dev_t;
typedef __kernel_ino_t          ino_t;
typedef __kernel_mode_t         mode_t;
typedef __kernel_nlink_t        nlink_t;
typedef __kernel_off_t          off_t;
typedef __kernel_pid_t          pid_t;
typedef __kernel_daddr_t        daddr_t;
//typedef __kernel_key_t          key_t;
typedef __kernel_suseconds_t    suseconds_t;
typedef __kernel_uid32_t        uid_t;
typedef __kernel_gid32_t        gid_t;
typedef __kernel_uid16_t        uid16_t;
typedef __kernel_gid16_t        gid16_t;
typedef __kernel_old_uid_t      old_uid_t;
typedef __kernel_old_gid_t      old_gid_t;
typedef __kernel_loff_t         loff_t;

typedef __kernel_size_t         size_t;
typedef __kernel_ssize_t        ssize_t;
typedef __kernel_ptrdiff_t      ptrdiff_t;
typedef __kernel_time_t         time_t;
typedef __kernel_clock_t        clock_t;
typedef __kernel_caddr_t        caddr_t;
typedef unsigned char           u_char;
typedef unsigned short          u_short;
typedef unsigned int            u_int;
typedef unsigned long           u_long;
typedef unsigned char           unchar;
typedef unsigned short          ushort;
typedef unsigned int            uint;
typedef unsigned long           ulong;
typedef         __u8            u_int8_t;
typedef         __s8            int8_t;
typedef         __u16           u_int16_t;
typedef         __s16           int16_t;
typedef         __u32           u_int32_t;
typedef         __s32           int32_t;
typedef         __u8            uint8_t;
typedef         __u16           uint16_t;
typedef         __u32           uint32_t;
typedef         __u64           uint64_t;
typedef         __u64           u_int64_t;
typedef         __s64           int64_t;



struct winsize {
        unsigned short ws_row;
        unsigned short ws_col;
        unsigned short ws_xpixel;
        unsigned short ws_ypixel;
};

#define NCC 8
struct termio {
        unsigned short c_iflag;         /* input mode flags */
        unsigned short c_oflag;         /* output mode flags */
        unsigned short c_cflag;         /* control mode flags */
        unsigned short c_lflag;         /* local mode flags */
        unsigned char c_line;           /* line discipline */
        unsigned char c_cc[NCC];        /* control characters */
};

typedef unsigned char   cc_t;
typedef unsigned int    speed_t;
typedef unsigned int    tcflag_t;

#define NCCS 19
struct termios {
        tcflag_t c_iflag;               /* input mode flags */
        tcflag_t c_oflag;               /* output mode flags */
        tcflag_t c_cflag;               /* control mode flags */
        tcflag_t c_lflag;               /* local mode flags */
        cc_t c_line;                    /* line discipline */
        cc_t c_cc[NCCS];                /* control characters */
};


/* 32-bit ELF base types. */
typedef __u32   Elf32_Addr;
typedef __u16   Elf32_Half;
typedef __u32   Elf32_Off;
typedef __s32   Elf32_Sword;
typedef __u32   Elf32_Word;

#define EI_NIDENT       16

typedef struct elf32_hdr{
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half    e_type;
  Elf32_Half    e_machine;
  Elf32_Word    e_version;
  Elf32_Addr    e_entry;  /* Entry point */
  Elf32_Off     e_phoff;
  Elf32_Off     e_shoff;
  Elf32_Word    e_flags;
  Elf32_Half    e_ehsize;
  Elf32_Half    e_phentsize;
  Elf32_Half    e_phnum;
  Elf32_Half    e_shentsize;
  Elf32_Half    e_shnum;
  Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

typedef struct elf32_phdr{
  Elf32_Word    p_type;
  Elf32_Off     p_offset;
  Elf32_Addr    p_vaddr;
  Elf32_Addr    p_paddr;
  Elf32_Word    p_filesz;
  Elf32_Word    p_memsz;
  Elf32_Word    p_flags;
  Elf32_Word    p_align;
} Elf32_Phdr;


struct stat {
        unsigned short st_dev;
        unsigned short __pad1;
        unsigned long st_ino;
        unsigned short st_mode;
        unsigned short st_nlink;
        unsigned short st_uid;
        unsigned short st_gid;
        unsigned short st_rdev;
        unsigned short __pad2;
        unsigned long  st_size;
        unsigned long  st_blksize;
        unsigned long  st_blocks;
        unsigned long  st_atime;
        unsigned long  __unused1;
        unsigned long  st_mtime;
        unsigned long  __unused2;
        unsigned long  st_ctime;
        unsigned long  __unused3;
        unsigned long  __unused4;
        unsigned long  __unused5;
};

/* This matches struct stat64 in glibc2.1, hence the absolutely
 * insane amounts of padding around dev_t's.
 */
struct stat64 {
        unsigned short  st_dev;
        unsigned char   __pad0[10];

#define STAT64_HAS_BROKEN_ST_INO        1
        unsigned long   __st_ino;

        unsigned int    st_mode;
        unsigned int    st_nlink;

        unsigned long   st_uid;
        unsigned long   st_gid;

        unsigned short  st_rdev;
        unsigned char   __pad3[10];

        //long long     st_size;
        __s64 st_size;

        unsigned long   st_blksize;

        unsigned long   st_blocks;      /* Number 512-byte blocks allocated. */
        unsigned long   __pad4;         /* future possible st_blocks high bits */

        unsigned long   st_atime;
        unsigned long   __pad5;

        unsigned long   st_mtime;
        unsigned long   __pad6;

        unsigned long   st_ctime;
        unsigned long   __pad7;         /* will be high 32 bits of ctime someday */

        //unsigned long long    st_ino;
        __u64 st_ino;
};


struct dirent {
        long            d_ino;
        __kernel_off_t  d_off;
        unsigned short  d_reclen;
        char            d_name[256]; /* We must not include limits.h! */
};

struct dirent64 {
        __u64           d_ino;
        __s64           d_off;
        unsigned short  d_reclen;
        unsigned char   d_type;
        char            d_name[256];
};


/* Type of a signal handler.  */
typedef void (*__sighandler_t)(int);

#define SIG_DFL ((linux::__sighandler_t)0)     /* default signal handling */
#define SIG_IGN ((linux::__sighandler_t)1)     /* ignore signal */
#define SIG_ERR ((linux::__sighandler_t)-1)    /* error return from signal */


typedef unsigned long old_sigset_t;             /* at least 32 bits */

typedef struct {
        unsigned long sig[_NSIG_WORDS];
} sigset_t;


struct old_sigaction {
        __sighandler_t sa_handler;
        old_sigset_t sa_mask;
        unsigned long sa_flags;
        void (*sa_restorer)(void);
};
struct sigaction {
        __sighandler_t sa_handler;
        unsigned long sa_flags;
        void (*sa_restorer)(void);
        sigset_t sa_mask;               /* mask last for extensibility */
};


typedef union sigval {
        int sival_int;
        void *sival_ptr;
} sigval_t;

#define SI_MAX_SIZE     128
#define SI_PAD_SIZE     ((SI_MAX_SIZE/sizeof(int)) - 3)
typedef struct siginfo {
        int si_signo;
        int si_errno;
        int si_code;

        union {
                int _pad[SI_PAD_SIZE];

                /* kill() */
                struct {
                        pid_t _pid;             /* sender's pid */
                        uid_t _uid;             /* sender's uid */
                } _kill;

                /* POSIX.1b timers */
                struct {
                        unsigned int _timer1;
                        unsigned int _timer2;
                } _timer;

                /* POSIX.1b signals */
                struct {
                        pid_t _pid;             /* sender's pid */
                        uid_t _uid;             /* sender's uid */
                        sigval_t _sigval;
                } _rt;

                /* SIGCHLD */
                struct {
                        pid_t _pid;             /* which child */
                        uid_t _uid;             /* sender's uid */
                        int _status;            /* exit code */
                        clock_t _utime;
                        clock_t _stime;
                } _sigchld;

                /* SIGILL, SIGFPE, SIGSEGV, SIGBUS */
                struct {
                        void *_addr; /* faulting insn/memory ref. */
                } _sigfault;

                /* SIGPOLL */
                struct {
                        int _band;      /* POLL_IN, POLL_OUT, POLL_MSG */
                        int _fd;
                } _sigpoll;
        } _sifields;
} siginfo_t;



struct flock {
        short l_type;
        short l_whence;
        off_t l_start;
        off_t l_len;
        pid_t l_pid;
};

struct flock64 {
        short  l_type;
        short  l_whence;
        loff_t l_start;
        loff_t l_len;
        pid_t  l_pid;
};


struct iovec
{
        void *iov_base;         /* BSD uses caddr_t (1003.1g requires void *) */
        __kernel_size_t iov_len; /* Must be size_t (1003.1g) */
};


struct timespec {
        time_t  tv_sec;         /* seconds */
        long    tv_nsec;        /* nanoseconds */
};
struct timeval {
        time_t          tv_sec;         /* seconds */
        suseconds_t     tv_usec;        /* microseconds */
};
struct timezone {
        int     tz_minuteswest; /* minutes west of Greenwich */
        int     tz_dsttime;     /* type of dst correction */
};
struct utimbuf {
        time_t actime;
        time_t modtime;
};

struct mmap_arg_struct { //from kernel source tree 2.6.11
        unsigned long addr;
        unsigned long len;
        unsigned long prot;
        unsigned long flags;
        unsigned long fd;
        unsigned long offset;
};


struct  rusage {
        struct timeval ru_utime;        /* user time used */
        struct timeval ru_stime;        /* system time used */
        long    ru_maxrss;              /* maximum resident set size */
        long    ru_ixrss;               /* integral shared memory size */
        long    ru_idrss;               /* integral unshared data size */
        long    ru_isrss;               /* integral unshared stack size */
        long    ru_minflt;              /* page reclaims */
        long    ru_majflt;              /* page faults */
        long    ru_nswap;               /* swaps */
        long    ru_inblock;             /* block input operations */
        long    ru_oublock;             /* block output operations */
        long    ru_msgsnd;              /* messages sent */
        long    ru_msgrcv;              /* messages received */
        long    ru_nsignals;            /* signals received */
        long    ru_nvcsw;               /* voluntary context switches */
        long    ru_nivcsw;              /* involuntary " */
};

struct rlimit {
        unsigned long   rlim_cur;
        unsigned long   rlim_max;
};


struct new_utsname {
        char sysname[65];
        char nodename[65];
        char release[65];
        char version[65];
        char machine[65];
        char domainname[65];
};


struct user_i387_struct {
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	st_space[20];	/* 8*10 bytes for each FP-reg = 80 bytes */
};

struct user_fxsr_struct {
	unsigned short	cwd;
	unsigned short	swd;
	unsigned short	twd;
	unsigned short	fop;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	mxcsr;
	long	reserved;
	long	st_space[32];	/* 8*16 bytes for each FP-reg = 128 bytes */
	long	xmm_space[32];	/* 8*16 bytes for each XMM-reg = 128 bytes */
	long	padding[56];
};

struct user_regs_struct {
	long ebx, ecx, edx, esi, edi, ebp, eax;
	unsigned short ds, __ds, es, __es;
	unsigned short fs, __fs, gs, __gs;
	long orig_eax, eip;
	unsigned short cs, __cs;
	long eflags, esp;
	unsigned short ss, __ss;
};

/* When the kernel dumps core, it starts by dumping the user struct -
   this will be used by gdb to figure out where the data and stack segments
   are within the file, and what virtual addresses to use. */
struct user{
/* We start with the registers, to mimic the way that "memory" is returned
   from the ptrace(3,...) function.  */
  struct user_regs_struct regs;		/* Where the registers are actually stored */
/* ptrace does not yet supply these.  Someday.... */
  int u_fpvalid;		/* True if math co-processor being used. */
                                /* for this mess. Not yet used. */
  struct user_i387_struct i387;	/* Math Co-processor registers. */
/* The rest of this junk is to help gdb figure out what goes where */
  unsigned long int u_tsize;	/* Text segment size (pages). */
  unsigned long int u_dsize;	/* Data segment size (pages). */
  unsigned long int u_ssize;	/* Stack segment size (pages). */
  unsigned long start_code;     /* Starting virtual address of text. */
  unsigned long start_stack;	/* Starting virtual address of stack area.
				   This is actually the bottom of the stack,
				   the top of the stack is always found in the
				   esp register.  */
  long int signal;     		/* Signal that caused the core dump. */
  int reserved;			/* No longer used */
  struct user_pt_regs * u_ar0;	/* Used by gdb to help find the values for */
				/* the registers. */
  struct user_i387_struct* u_fpstate;	/* Math Co-processor pointer. */
  unsigned long magic;		/* To uniquely identify a core file */
  char u_comm[32];		/* User command that was responsible */
  int u_debugreg[8];
};


struct sysinfo {
        long uptime;                    /* Seconds since boot */
        unsigned long loads[3];         /* 1, 5, and 15 minute load averages */
        unsigned long totalram;         /* Total usable main memory size */
        unsigned long freeram;          /* Available memory size */
        unsigned long sharedram;        /* Amount of shared memory */
        unsigned long bufferram;        /* Memory used by buffers */
        unsigned long totalswap;        /* Total swap space size */
        unsigned long freeswap;         /* swap space still available */
        unsigned short procs;           /* Number of current processes */
        unsigned short pad;             /* explicit padding for m68k */
        unsigned long totalhigh;        /* Total high memory size */
        unsigned long freehigh;         /* Available high memory size */
        unsigned int mem_unit;          /* Memory unit size in bytes */
        char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding: libc5 uses this.. */
};


struct vm_area_struct;
struct page;
struct kiocb;
struct sockaddr;
struct msghdr;
struct module;

typedef int socklen_t;


struct user_desc {
        unsigned int  entry_number;
        unsigned int  base_addr;
        unsigned int  limit;
        unsigned int  seg_32bit:1;
        unsigned int  contents:2;
        unsigned int  read_exec_only:1;
        unsigned int  limit_in_pages:1;
        unsigned int  seg_not_present:1;
        unsigned int  useable:1;
};



};//namespace

#endif _LINUX_TYPES_H
