/*******************************************************************************\
| client.c
| Original Author: Unknown -->provided by professor
| Last Revised by: Todd Sukolsky
| Initial Build: unknown
| Last Revised: 3/20/2013
|================================================================================
| Description: This file emulates a client and is used to test "server.c". This
|	file was obtained from the professor. This should be used
|	to test the server.c
|--------------------------------------------------------------------------------
| Revisions: 3/18-Pulled from professor. Tested with "server.c" and then somewhat
|	   	  tweaked for my own use. Changing how it works slightly with no
|		  string to be sent originally.
|	     3/19-Changed functionality to accept rasPi running "dmesg | tail -n 30". 
|		  Works like a charm. (2) Made the advanced feature be a riddle game.
|		  Changed read/write characteristics and must recognize what command
|		  was given to the server.
|	     3/20-Cleaned code, added more comments for explanation.
|		
|================================================================================
| *NOTES: (1) This file is maintained by Todd Sukolsky via github.
|		   https://github.com/tsukolsky/ec440.git/homework3/
\*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

//Define length of buffer
#define SIZE_MB 1024
#define NUMBER_OF_MB 10

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
	//Declare buffer size
        const int bufferSize=SIZE_MB*NUMBER_OF_MB;
    	int sockfd, portno, n;
   	struct sockaddr_in serv_addr;
   	struct hostent *server;
    	char buffer[bufferSize];

	//If port number and/or host isn't given, give usage and exit
   	if (argc < 3) {
       		fprintf(stderr,"usage %s hostname port\n", argv[0]);
       		exit(0);
    	}
	
	//Turn port number string into decimal
    	portno = atoi(argv[2]);

	//Create socket
    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0){error("ERROR opening socket");}
    	
	//Declare server as hostname
	server = gethostbyname(argv[1]);
	if (server == NULL) {error("ERROR, no such host\n");}

	//Create the server socket
    	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    	serv_addr.sin_port = htons(portno);

	//Loop continuously
	for (;;){
		//Get socket up and running
   	 	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){error("ERROR connecting");}

		//Ask the user what they want to request from the server, then send it to the server
   	 	printf("Please enter the message: ");
   	 	bzero(buffer,256);
   	 	fgets(buffer,255,stdin);
   	 	n = write(sockfd,buffer,strlen(buffer));
  	  	if (n < 0){error("ERROR writing to socket");}

		//No error, if command was "simple", wait for stuff to come back; if "riddle", run through riddle sequence. If exit, terminate. Else do nothing.
  	  	if (!strncmp(buffer,"simple",6)){
			n = read(sockfd,buffer,bufferSize); 
			printf("%s\n",buffer);
		} else if (!strncmp(buffer,"riddle",6)){
			//Read the riddle and print the clue	
			n = read(sockfd,buffer,bufferSize);	
			printf("%s",buffer);
		
			//Zero buffer and get user answer
			bzero(buffer,256);
			fgets(buffer,20,stdin);
		
			//Send the answer
			printf("Sending %s\n",buffer);
			n = write(sockfd,buffer,strlen(buffer));

			//Clear buffer and see if it's correct	
			bzero(buffer,256);
			n = read(sockfd,buffer,bufferSize);
			printf("%s\n\n",buffer);
		} else if (!strncmp(buffer,"exit",4)){
			printf("Exiting...\n");
			exit(0);
	        }else;
	    	close(sockfd);
	}//end forever for.
    	return 0;
}
