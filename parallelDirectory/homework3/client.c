#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/*******************************************************************************\
|client.c
| Author: Todd Sukolsky
| ID: U50387016
| Initial Build: unknown
| Last Revised: 3/18/2013
|================================================================================
| Description: This file emulates a client and is used to test "server.c". This
|			file was obtained from the professor.
|--------------------------------------------------------------------------------
| Revisions:
|================================================================================
| *NOTES:
\*******************************************************************************/



#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define SIZE_MB 1024
#define NUMBER_OF_MB 10

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    const int bufferSize=SIZE_MB*NUMBER_OF_MB;
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[bufferSize];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    if (!strncmp(buffer,"simple",6)){n = read(sockfd,buffer,bufferSize); printf("%s\n",buffer);}
    else if (!strncmp(buffer,"riddle",6)){
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
	n = read(sockfd,buffer,strlen(buffer));
    } else;
    close(sockfd);
    return 0;
}
