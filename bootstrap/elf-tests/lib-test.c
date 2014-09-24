/* A simple dynamic libs program using the gcc standard librarys */
#include <stdlib.h>
#include <unistd.h>

int main()
{
	write(1,"Hello with libraries\n",21);
	exit(3);
}
