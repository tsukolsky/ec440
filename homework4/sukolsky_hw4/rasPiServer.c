/*******************************************************************************\
| rasPiServer.c
| Author: Todd Sukolsky
| Initial Build: 3/28/2013
| Last Revised: 3/29/2013
|================================================================================
| Description: This is a server neeeded for ECE 440 homework4. The idea is to implement
|		a server on the RasPI that listens over an IP and Port, accepts incoming connectiongs
|		and from those connections spawns a new thread/process to handle the request. This version
|		will accept a list of numbers, seperated by new lines, sum them, tell the client what the sum
|		was, then tell the client to aggregate sum of all connections as well as number of total clients
|		served so far.
|--------------------------------------------------------------------------------
| Revisions: 3/28- Initial build. Compiled everything successfully up to place for shared
|		   memory.
|	     3/29- Added mutex hold for incrementing the "sum" and "count" variables
|		   that tell what the aggregate total is as well as the number of clients.
|		   Renamed those two variables to go into a struct (OVERALLDATA) called overallSum and 
|		   clientsServed.  Added extra commenting and cleaned code.
|================================================================================
| *NOTES:(1) Basic socket info was provided by the instructor. Other information on 
|		  	 sockets can be found at ...
|		     http://www.microhowto.info/howto/listen_for_and_accept_tcp_connections_in_c.html
|	 (2) This document is being maintained on github in the location:
|		     https://github.com/tsukolsky/ec440.git/homework4
|	 (3) To compile, need to link pthread library. Command is "gcc -pthread -o rasPiServer rasPiServer.c"
\*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <pthread.h>

//Define how large of a buffer we are going to allow
#define ONE_MB 1024
#define NUMBER_OF_MB 1		//<-------CHange this

//How many requests can be sent into a pending state/queue
#define PENDING_REQUESTS 20

/*===============================*/
/*	Forward Declarations	 */
/*===============================*/
void error(const char *msg);
void *actionThread(void *arg);
	
/*===============================*/
/*	Global Structure	 */
/*===============================*/
typedef struct{
	long overallSum;
	unsigned long clientsServed;
} OVERALLDATA;

OVERALLDATA theData;
pthread_mutex_t mutexSum;		//define place that the mutex occurs
/*===============================*/
/*      Main Program		 */
/*===============================*/

int main(int argc, char *argv[]){
	//Declare Variables
        unsigned int sockfd, newsockfd, portno,ids;		//Server socket descriptor, client<->server socket descriptor, portno
        socklen_t clilen;					//length of socket name for client
        struct sockaddr_in serv_addr, cli_addr;
	pthread_attr_t	attr;					//attributes of any pthread we create
	
	//Initialize global variables used in the pthreads.
	pthread_mutex_init(&mutexSum,NULL);
	theData.overallSum=0;
	theData.clientsServed=0;

	//Declare the port number.
        portno=55000;

	//Create server socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);		//create the socket
        if (sockfd < 0){error("ERROR opening socket");}
        bzero((char *) &serv_addr, sizeof(serv_addr));		//zero out server address
        serv_addr.sin_family = AF_INET;				//assign to TCP
        serv_addr.sin_addr.s_addr = INADDR_ANY;			//assign server address
        serv_addr.sin_port = htons(portno);			//assign port number
	 
        //Bind local address to server socket
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
     	sizeof(serv_addr)) < 0){error("ERROR on binding");}
	
        //Now listen, initialize pthread attribute and set it to a joinable state.
  	listen(sockfd,PENDING_REQUESTS);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	//THis is what should continue and contine and continue
	for (;;){
		//Declare the pthread variable, say the server is ready
		pthread_t threads;
		printf("My server is ready.\n");

		//Accept a new client
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
		printf("A new client arrives...\n");

		//new sockfd has what we are going to be printing too. Can spawn a new process or whatever
		if (newsockfd < 0){error("ERROR on accept");}
		else{
			ids=newsockfd;
			//Make the new thread.
			pthread_create(&threads,&attr,actionThread,&ids);	//make a new thread that executes my function "actionThread" with the socket file descriptor.
			//Don't join, otherwise it will wait for it and it won't really be multithreaded...
	
		}//end else
	 }//end infinite for
     close(sockfd);
     return 0; 
}



/*===============================*/
/*         Functions		 */
/*===============================*/

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/*================================================================================================================*/

void *actionThread(void *arg){
	//Tell them we are in a new thread precious
	printf("\tIn a new thread...\n");

	//Make variables to use
	int clientSocket;				//Copy of the socket, comes in as the argument
	const int bufferSize=ONE_MB*NUMBER_OF_MB;	//bufferSize
	char inputBuffer[bufferSize];			//make two buffers
	char outputBuffer[bufferSize];
	unsigned int ioReturn,ioReturn2;				//var used to determine if successful send/receive from socket

	//Assign the clientSocket to the argument
	clientSocket=*(unsigned int *)arg;

	//Read the however many lines of data.
	ioReturn = read(clientSocket,inputBuffer,bufferSize);
	if (ioReturn < 0){error("Error reading from client socket.");}

	//At this point the inputBuffer has all the stuff we need in it. Sum up the numbers.
	int i, sum=0;
	char tempNumberString[10];
	int tempNumber=0, strLoc=0;
	
	//Parse the string
	for (i=0; i<strlen(inputBuffer); i++){
		//If its a '-' or a number, add it to a tempNumber string. If a new line, take the number string and make it into a number, add it to the sum. 	
		if ((int)inputBuffer[i]==45 || ((int)inputBuffer[i] <=57 && (int)inputBuffer[i] >=48)){
			tempNumberString[strLoc++]=inputBuffer[i];
		} else if (inputBuffer[i]=='\n'){
			int k=0;
			for (k=0; k<strLoc; k++){tempNumberString[strLoc]=(char *)NULL;}
			strLoc=0;
			tempNumber=atoi(tempNumberString);
			sum+=tempNumber;
		} else if (inputBuffer[i]=='\0'){i=strlen(inputBuffer);}	//done, get out of loop, we have the correct sum
		else {error("Bad input numbers...");break;}
	}//end for

	//We now have the sum, turn itno a string and print to the socket
	snprintf(outputBuffer,40,"\n\tYour sum=%d\n",sum);
	ioReturn2 = write(clientSocket,outputBuffer,strlen(outputBuffer));
	if (ioReturn2 < 0){error("Error writing to client socket.");}

	//Now look at how many clients we have served, and what the overall total is. Lock the mutex so no other thread steals it.
	pthread_mutex_lock(&mutexSum);
	theData.overallSum += sum;	//add our sum to the overall sum
	theData.clientsServed += 1;	//add one to the number of users served
	unsigned int clients=theData.clientsServed;
	int totalSum=theData.overallSum;
	pthread_mutex_unlock(&mutexSum);

	//Mutex is released, print to the client now.
	bzero(outputBuffer,bufferSize);
	snprintf(outputBuffer,256,"\n\tTotal Clients Served=%d, Overall Sum=%d\n",clients,totalSum);
	ioReturn2 = write(clientSocket,outputBuffer,strlen(outputBuffer));
	if (ioReturn2 < 0){error("Error writing to client socket.");}
	
	//Cleanup
	close(clientSocket);
	pthread_exit(NULL);
}
	
		
/*================================================================================================================*/



		


