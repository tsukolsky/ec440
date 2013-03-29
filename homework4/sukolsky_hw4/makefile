############################################################################################
## makefile
##
## Author: Todd Sukolsky
## Initial Build: 3/29/13
## Last Revised: 3/29/13
## Copyright of Todd Sukolsky
##
############################################################################################
## Description: This is a makefile for the rasPi server and client.
##
############################################################################################
## Revisions: 3/29- Initial build
############################################################################################
## Command summary:
##      	"make"----------------->creates client and server programs
##		"make server"---------->creates only server program
##		"make client"---------->creates only client program
##		"make clean"----------->cleans object and save files
##		"make clean-server"---->cleans server executable and runs normal clean
##		"make clean-client"---->cleans client executable and runs normal clean			
############################################################################################

##Variables
CC=gcc
CARGS=-pthread -Wall -c

##Protocols
all: serverAndClient

serverAndClient: rasPiServer.o myClient.o
	$(CC) -Wall -pthread rasPiServer.o -o theServer
	$(CC) -Wall -pthread myClient.o -o theClient

server: rasPiServer.o
	$(CC) -Wall -pthread rasPiServer.o -o theServer

client: myClient.o
	$(CC) -Wall -pthread myClient.o -o theClient

rasPiServer.o:	rasPiServer.c
	$(CC) $(CARGS) rasPiServer.c

myClient.o: myClient.c
	$(CC) $(CARGS) myClient.c

clean-full: \
	clean-client	\
	clean-server	\
	clean

clean-client: \
	clean
	rm -rf *o theClient*

clean-server: \
	clean
	rm -rf *o theServer*

clean:
	rm -rf *o
	rm -rf *~
