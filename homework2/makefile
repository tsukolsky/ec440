####### This is a makefile for the custom shell needed in EC440. See myShell.c


CC=gcc
CARGS=-c -Wall
all: sukolskyShell
sukolskyShell: myShell.o
	$(CC) $(CARGS) myShell.o -o sukolskyShell

main.o: myShell.c
	$(CC) $(CARGS) myShell.c

clean:
	rm -rf *o sukolskyShell
