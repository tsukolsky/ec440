/*******************************************************************************\
| client.c
| Original Author: Unknown -->provided by professor
| Last Revised by: Todd Sukolsky
| Initial Build: unknown
| Last Revised: 3/28/2013
|================================================================================
| Description: This file emulates a client and is used to test "rasPiServer.c". This
|	file was obtained from the professor as a tutorial from sockets and has been
|	appended and manipulated by Todd Sukolsky for his own purposes.
|--------------------------------------------------------------------------------
| Revisions: 3/28- Initial remake. Tweaked from homework3 version
|================================================================================
| *NOTES: (1) This file is maintained by Todd Sukolsky via github.
|		   https://github.com/tsukolsky/ec440.git/homework4/
\*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>

//Define length of buffer
#define SIZE_MB 1024
#define NUMBER_OF_MB 1

//Declare a mutex lock for writing out the variables
pthread_mutex_t readingFromServer;

void *connectToServer(void *arg);

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(){
	//Three different things
	char *message1= "5\n4\n3\n2\n1\n";				//Sum=15
	char *message2="10\n20\n30\n";					//Sum=60
	char *message3="-5\n-4\n-3\n-2\n-1\n";				//Sum=-15
	//Create 3 threads to ask different things.
    	pthread_attr_t attr;
    	pthread_t threads[3];
	pthread_attr_init(&attr);
	pthread_mutex_init(&readingFromServer,NULL);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	pthread_create(&threads[0],&attr,connectToServer,(void *)message1);
	pthread_join(threads[0],NULL);
	pthread_create(&threads[1],&attr,connectToServer,(void *)message2);
	pthread_join(threads[1],NULL);
	pthread_create(&threads[2],&attr,connectToServer,(void *)message3);
	pthread_join(threads[2],NULL);

	//Exit main
	printf("\nShould be finished now...\n");
	return 0;
}

void *connectToServer(void *arg)
{
	printf("In thread...\n");
	//Declare buffer size
        const int bufferSize=SIZE_MB*NUMBER_OF_MB;
    	int sockfd, portno, n;
   	struct sockaddr_in serv_addr;
   	struct hostent *server;

    	char receiveBuffer[bufferSize];
	char *sendBuffer;
	sendBuffer=(char *)arg;
	//ServerIP= 10.42.0.31:55000
	portno=55000;
	server=gethostbyname("10.42.0.31");
	
	//Create socket
    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0){error("ERROR opening socket");}
    	
	//Create the server socket
    	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    	serv_addr.sin_port = htons(portno);
   	
	//Get the socket up and running 	
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){error("ERROR connecting");}

	//Send the string of numbers to the server
  	n = write(sockfd,sendBuffer,strlen(sendBuffer));
	if (n < 0){error("ERROR writing to socket");}

	//Get response from the server.
	bzero(receiveBuffer,bufferSize);
	int n2 = read(sockfd,receiveBuffer,40);
	if (n2<0){error("Error receiving from socket");}
	printf("%s\n",receiveBuffer);
	
	//Now get second line of transmission
	bzero(receiveBuffer,strlen(receiveBuffer));
	n2=read(sockfd,receiveBuffer,256);
	if (n2<0){error("Error receiving from socket");}
	else{printf("%s\n",receiveBuffer);}

	close(sockfd);
	pthread_exit(NULL);
}
