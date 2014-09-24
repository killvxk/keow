CFLAGS= -O0 -g -static -nostartfiles -nodefaultlibs -fno-stack-protector

all: test tls-test lib-test

#many of these creating the .s assembly file to aid in debugging

test: test.c makefile
	cc -S $(CFLAGS) test.c
	cc $(CFLAGS) test.s -o test


tls-test: tls-test.c makefile
	gcc -S $(CFLAGS) tls-test.c
	gcc $(CFLAGS) tls-test.s -o tls-test


lib-test: lib-test.c makefile
	gcc -S -O0 -g lib-test.c
	gcc -O0 -g lib-test.s -o lib-test
