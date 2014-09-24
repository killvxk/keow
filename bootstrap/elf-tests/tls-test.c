/*
  A testbed for thread-local-storage (TLS) APIs.
  Does not use any libraries or libc/libgcc builtins (so we control all kernel api calls)
*/

#include <unistd.h>
#include <linux/unistd.h>
#include <asm/ldt.h>


unsigned char buff[4096];
struct user_desc ud;

//nodefaultlibs, so need to supply the write() system call
int _write(int fd, char*s, int n)
{
    int nr = __NR_write;
    asm (
        "int $0x80 \n\t"
        :
        :"a"(nr), "b"(fd), "c"(s), "d"(n)
        );
    return 0;
}

//nodefaultlibs, so need to supply a syscall() system call
int _syscall(int n, void*p)
{
    asm (
        "int $0x80 \n\t"
        :
        :"a"(n), "b"(p)
        );
    return 0;
}

//nodefaultlibs, so need to supply an exit() 
void exit_n(int n) {
    _syscall(__NR_exit, (void*)n);
}


// Write a string to stdout
void print(char *s)
{
    int i=0;
    while(s[i]) ++i;
    _write(1,s,i);
}

// Write a number to stdout
void print_dec(long n)
{
   char buf[20];
   int i;

   if(n<0) {
     _write(1,"-",1);
     n = -n;
   }

   i=19;
   buf[i]=0;
   do {
     --i;
     buf[i] = '0' + (n%10);
     n /= 10;
   } while(n!=0);
   _write(1,&buf[i],20-i);
}

// Write a number, in hex, to stdout
void print_hex(unsigned long n)
{
   char buf[20];
   int i;

   print("0x");

   i=19;
   buf[i]=0;
   do {
     --i;
     buf[i] = ((n&0xF)<=9 ? '0' : ('A'-10)) + (n&0xF);
     n>>=4;
   } while(n);
   _write(1,&buf[i],20-i);
}


/***************************************************************************/

/* Test using a selector to access memory */
void test_TLS(int granularity)
{
    int selector;
    int i;

	print("Testing ");
	print(granularity==0 ? "byte" : "page");
	print(" granularity\n");

    //init buffer with known values
    for(i=0; i<sizeof(buff); ++i) {
        buff[i] = i&0xFF;
    }
    print("buff ");
    print_hex((unsigned long)&buff);
    print("\n");


    //Setup a TLS slot
	ud.entry_number = -1;
	ud.base_addr = (unsigned long)&buff;
	ud.limit = sizeof(buff);
	ud.limit_in_pages = granularity;
	ud.seg_32bit = 1;
	ud.contents = 0;
	ud.read_exec_only = 0;
	ud.seg_not_present = 0;
	ud.useable = 1;
    //set_thread_area(&buff);
    i = _syscall(__NR_set_thread_area, &ud);
    if(i<0) {
	    print("error\n");
	    exit_n(2);
    }
    print("entry_num ");
    print_dec(ud.entry_number);
    print("\n");

    //assign to gs, and then use it (like libpthread does)
    selector = (ud.entry_number<<3) | 0x3;  //GDT index at DPL=3
    print("selector ");
    print_hex(selector);
    print("\n");
    asm (
        "movl %0, %%eax \n\t"
        "movw %%ax, %%gs \n\t"
        :
        :"r"(selector)
        );
    //print again - testing keow's ability to preserve gs across api calls
    print("gs=");
    asm("movw %%gs, %%ax" :"=r"(i));
    print_hex(i);
    print("\n");


    //Test that the selector works by accessing the data
    //with both C and also by using the gs: selector.

    //read [13]
    print("buff[13]=");
    print_hex(buff[13]);
    print(",  ");
    asm (
        "movl %%gs:13, %%eax \n\t"
        "movl %%eax, %0 \n\t"
        :"=r"(i)
        );
    print_hex(i);
    print("\n");

    //write [13]
    print("attempting write access\n");
    asm ("movb $0x91, %%gs:13" :);
    print("new buff[13]=");
    print_hex(buff[13]);
    print("\n");

    print("\n");
}
/* this is main() when compiled with -nostartfiles */
int _start()
{
	print("tls-test start\n");

    test_TLS(0);
    test_TLS(1);

	print("end\n");
	exit_n(0);
}
