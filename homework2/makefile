####### This is a makefile for the custom shell needed in EC440. See myShell.c

CC=gcc
CARGS=-c -Wall
all: sukolskyShell
sukolskyShell: myShell.o
	$(CC) -Wall myShell.o -o sukolskyShell

main.o: myShell.c
	$(CC) $(CARGS) myShell.c

clean-full:
	rm -rf *o
	rm -rf *.c~
	rm -rf *o sukolskyShell

clean:
	rm -rf *o
	rm -rf *.c~	
