/*******************************************************************************\
| rasPiServer.c
| Author: Todd Sukolsky
| Initial Build: 3/18/2013
| Last Revised: 3/20/2013
|================================================================================
| Description: This is a server neeeded for ECE 440 homework4. The idea is to implement
|		a server on the RasPI that listens over an IP and Port, accepts incoming connectiongs
|		and from those connections spawns a new thread/process to handle the request. This version
|		will accept a list of numbers, seperated by new lines, sum them, tell the client what the sum
|		was, then tell the client to aggregate sum of all connections as well as number of total clients
|		served so far.
|--------------------------------------------------------------------------------
| Revisions: 3/27- Initial build. Compiled everything successfully up to place for shared
|		   memory.
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
        unsigned int sockfd, newsockfd, portno,ids;				//Server socket descriptor, client<->server socket descriptor, portno
        socklen_t clilen;					//length of socket name for client
        char buffer[256];					//buffer length we can write to
        struct sockaddr_in serv_addr, cli_addr;
        int n;							//length used during read and write system calls
//	pthread_t threads;	//thread ID
	pthread_attr_t	attr;	//attributes of pthread
	
	//Initialize global variables
	pthread_mutex_init(&mutexSum,NULL);
	theData.overallSum=0;
	theData.clientsServed=0;

	//If there are too few arguments provided on command line, say it needs a port number and ask.
        if (argc < 2) {
                fprintf(stderr,"ERROR, no port provided\n");
		printf("Provide port number: ");
		scanf("%d",&portno);				//get the port number
        } else{portno = atoi(argv[1]);}

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
	
        //Now listen
  	listen(sockfd,PENDING_REQUESTS);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	//THis is what should continue and contine and continue
	for (;;){
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
			//pthread_join(threads,NULL);				//dON'T join. iF we do we won't be multithreaded, but will wait for current thread to finish before moving on.		
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
	int clientSocket;

	const int bufferSize=ONE_MB*NUMBER_OF_MB;
	char inputBuffer[bufferSize];
	char outputBuffer[bufferSize];
	unsigned int ioReturn;

	//Assign the clientSocket to the argument
	clientSocket=*(unsigned int *)arg;

	//Read the however many lines of data.
	ioReturn = read(clientSocket,inputBuffer,bufferSize);

	//At this point the inputBuffer has all the stuff we need in it. Sum up the numbers.
	int i, sum=0;
	char tempNumberString[10];
	int tempNumber=0, strLoc=0;
	
	for (i=0; i<strlen(inputBuffer); i++){
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
	int ioReturn2 = write(clientSocket,outputBuffer,strlen(outputBuffer));
	
	//Now look at how many clients we have served, and what the overall total is.
//	pthread_mutex_lock(&mutexSum);
	theData.overallSum += sum;	//add our sum to the overall sum
	theData.clientsServed += 1;	//add one to the number of users served
	unsigned int clients=theData.clientsServed;
	int totalSum=theData.overallSum;
//	pthread_mutex_unlock(&mutexSum);

	bzero(outputBuffer,bufferSize);
	snprintf(outputBuffer,256,"\n\tTotal Clients Served=%d, Overall Sum=%d\n",clients,totalSum);
	ioReturn2 = write(clientSocket,outputBuffer,strlen(outputBuffer));
	
	//Cleanup
	close(clientSocket);
	pthread_exit(NULL);
}
	
		
/*================================================================================================================*/



		


