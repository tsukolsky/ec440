####### This is a makefile for the custom shell needed in EC440. See myShell*.c

CC=gcc
CARGS=-c -Wall
all: sukolskyShell_armv7
sukolskyShell_armv7: myShell_armv7.o
	$(CC) -Wall myShell_armv7.o -o sukolskyShell_armv7

myShell_armv7.o: myShell_armv7.c
	$(CC) $(CARGS) myShell_armv7.c

clean-full:
	rm -rf *o
	rm -rf *.c~
	rm -rf *o sukolskyShell*

clean:
	rm -rf *o
	rm -rf *.c~	
