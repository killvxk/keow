/* a simple C testbed with NO c runtime to test api calls */
#include <unistd.h>
#include <linux/unistd.h>

int _start()
{
	//write(1,"Hello\n",6);
	//exit(3);
    asm ( "int $0x80 \n\t" : :"a"(__NR_write), "b"(1), "c"("Hello\n"), "d"(6) );
    asm ( "int $0x80 \n\t" : :"a"(__NR_exit), "b"(3) );
}
