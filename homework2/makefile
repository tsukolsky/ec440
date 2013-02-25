############################################################################################
## makefile
##
## Author: Todd Sukolsky
## Initial Build: 2/14/13
## Last Revised: 2/25/13
## Copyright of Todd Sukolsky
##
############################################################################################
## Description: This is a makefile for the "sukolskyShell*" program that is an interactive
## 	shell based on bash. 
############################################################################################
## Revisions: 2/14: Initial build. Bare boned
##     	      2/25: Made clean more modular, added new protocol for building _armv7 or _x86
##		    versions explicitly.
############################################################################################
## Command summary:
##      	"make" && "make armv7" --> makes arm version of shell
##		"make x86" --------------> makes x86 version of shell
## 		"clean-x86" -------------> cleans all .o and .c~ files, then *_x86 program
##		"clean-armv7"------------> "	  "   "   "   "   "   , "    *_armv7 "
##		"clean-full"-------------> "      "   "   "   "   "   , "     any/all shells
############################################################################################

##Variables
CC=gcc
CARGS=-c -Wall

##Protocols
all: armv7

armv7: myShell_armv7.o
	$(CC) -Wall myShell_armv7.o -o sukolskyShell_armv7

myShell_armv7.o: myShell_armv7.c
	$(CC) $(CARGS) myShell_armv7.c


x86: myShell_x86.o
	$(CC) -Wall myShell_x86.o -o sukolskyShell_x86

myShell_x86.o: myShell_x86.c
	$(CC) $(CARGS) myShell_x86.c

clean-x86: \
	clean
	rm -rf *o sukolskyShell_x86*

clean-armv7: \
	clean
	rm -rf *o sukolskyShell_armv7*

clean-full: \
	clean
	rm -rf *o sukolskyShell*

clean:
	rm -rf *o
	rm -rf *.c~	
