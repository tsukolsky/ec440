/*******************************************************************************\
| rasPiServer.c
| Author: Todd Sukolsky
| Initial Build: 3/18/2013
| Last Revised: 3/20/2013
|================================================================================
| Description: This is a server neeeded for ECE 440 homework3. The idea is to implement
|		a server on the RasPI that listens over an IP and Port, accepts incoming connectiongs
|		and from those connections spawns a new thread/process to handle the request.
|
|--------------------------------------------------------------------------------
| Revisions: 3/18-Initial Build
|	     3/19-Tweaked this to go into a function on positive listen and then fork and 
|		  exec. Initially had an issue with open pipes/blocking, fixed it though by
|		  closing every single pipe not being used in the master parent. See commits
|		  from earlier today. (2)Got the exec calls working with correct piping.
|		  (3) Tested hard, works well. Expanded buffer size but doesn't cause 
|		      issues. Just need to add the "Advanced feature"...thinking
|	     3/20-Cleaned code, added more comments. Made a makefile for this thing. If the port
|		  number is not given on execution, program will ask for it. Killed with ctrl-c.
|
|================================================================================
| *NOTES:(1) Basic socket info was provided by the instructor. Other information on 
|		  	 sockets can be found at ...
|		     http://www.microhowto.info/howto/listen_for_and_accept_tcp_connections_in_c.html
|	 (2) This document is being maintained on github in the location:
|		     https://github.com/tsukolsky/ec440.git/homework3
|	 (3) Come up with an idea for an advanced feature. Have riddles stored in a text files.
|	     If the command is "riddle" then deal with it. If "dmesg", deal with it.
|
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

//Define how large of a buffer we are going to allow
#define ONE_MB 1024
#define NUMBER_OF_MB 10		//<-------CHange this

//How many requests can be sent into a pending state/queue
#define PENDING_REQUESTS 20

/*===============================*/
/*	Forward Declarations	 */
/*===============================*/
void error(const char *msg);
bool dealWithConnection(int socketHandle);
bool giveThemARiddlePrecious(int socketHandle);
	
/*===============================*/
/*      Main Program		 */
/*===============================*/

int main(int argc, char *argv[]){
	//Declare Variables
        int sockfd, newsockfd, portno;				//Server socket descriptor, client<->server socket descriptor, portno
        socklen_t clilen;					//length of socket name for client
        char buffer[256];					//buffer length we can write to
        struct sockaddr_in serv_addr, cli_addr;
        int n;							//length used during read and write system calls

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
     	sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
	
        //Now listen
  	listen(sockfd,PENDING_REQUESTS);
	 
	//THis is what should continue and contine and continue
	for (;;){
		//Accept a new client
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);

		//new sockfd has what we are going to be printing too. Can spawn a new process or whatever
		if (newsockfd < 0){error("ERROR on accept");}
		n=read(newsockfd,buffer,255);				//get what they are asking for
		if (n<0){error("ERROR writing to socket...");}	
		
		//There wasn't an error, we receieved some string from the client.
		else {
			bool successful=false;
			if (!strncmp(buffer,"riddle",6)){successful = giveThemARiddlePrecious(newsockfd);}	//give the user a riddle to mess with and answer
			else if (!strncmp(buffer,"simple",6)){successful = dealWithConnection(newsockfd);}	//print out the "simple" version of the homework, last 30 lines of dmesg
			if (successful){printf("I sent the correct information.\r\n");}
			else{error("Unable to respond to request.");}
		}//end else
		close(newsockfd);
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
  // exit(1);
}

/*================================================================================================================*/

bool dealWithConnection(int socketHandle){
	//Declare Variables.
	const int bufferSize=ONE_MB*NUMBER_OF_MB;		//how long of a buffer we allow
	int pid1,pid2,dmesgPipe[2],tailPipe[2],n,n1;		//process ids, pipes, lengths or read/writes
	char buffer1[bufferSize];				//declare buffer
	char *tailCmd[] = {"tail","-n", "30", 0};		//give tailCommands as stack array
	char *dmesgCmd[] = {"dmesg",0};				//give dmesg commands as stack array

	//Make the pipes
	if (pipe(dmesgPipe) < 0){error("Unable to create dmesg pipe.");return false;}
	if (pipe(tailPipe) < 0){error("Unable to create tail pipe."); return false;}

	//Fork and execute "dmesg | tail -n 30"
	if ((pid1=fork())==0){
		if ((pid2=fork())==0){	//this is the dmesg program
			//Assign standard output to the output side of th pipe. Close the input side.
			dup2(dmesgPipe[1],1);
			close(dmesgPipe[0]);
			close(2);
				
			//Execute the program "dmesg"
			execvp(dmesgCmd[0],dmesgCmd);
		}else if (pid2>0){	//This is the tail process, parent of dmesg and child of the server
			//Assign standard input of this pipe as the input end of the dmesgPipe (coming from dmesg). 			
			dup2(dmesgPipe[0],0);	
			close(dmesgPipe[1]);		//Close output end
			close(2);
			
			//Assign standard output to go to output end of the tailPipe. 
			dup2(tailPipe[1],1);
			close(tailPipe[0]);		//close the input of this pipe

			//Exec the program, wait afterwards for the dmesg program to finish.
			execvp(tailCmd[0],tailCmd);
			waitpid(pid2,NULL,0);
		}else {error("Error forking/creating \"dmesg\" program"); return false;}

	}else if (pid1>0){	//parent of "tail" program, grandparent of "dmesg" program

		//Close all dmesgPipes to stop blocking, close output end of tailPIpe before trying to read. 
		close(dmesgPipe[0]);
		close(dmesgPipe[1]);
		close(tailPipe[1]);

		//Read from pipe, then forward to socket/client
		n=read(tailPipe[0],buffer1,bufferSize);					//read
		if (n<0){error("Error receiving data from pipes.");return false;}
		n1 = write(socketHandle,buffer1,sizeof(buffer1));			//Print to socket
		if (n1<0){error("Error writing data to socket.");return false;}		
		
		//Wait for child process to finish		
		waitpid(pid1,NULL,0);
		
		//Executed correctly, return true.
		return true;
	}else {error("Error forking/creating \"tail\" program."); return false;}
	return true;
}//end function

/*================================================================================================================*/

bool giveThemARiddlePrecious(int socketHandle){
	FILE *riddleFile;
	static unsigned int riddlesUsed=0;
	static bool flagAllRiddlesDone=false;
	
	srand(time(0));					//set seed for random number
	int whichRiddle=rand()%8;			//Get a value between 0 and 7
	
	//Check to see if we have used all the riddles. If we have, set the flag.
	if (riddlesUsed==255){flagAllRiddlesDone=true;}	
	else;

	//If the flag is set, ask the client if they want to repeat riddles. Yes or yes are recognized, otherwise it will wait for a new connection.
	if (flagAllRiddlesDone){
		/*
		char inBuf[10];
		int n1 = write(socketHandle,"Do you want to repeat riddles? I have run out of new ones...",61);
		int n2 = read(socketHandle,inBuf,20);
		printf("Do you want to repeat riddles? ");
		fgets(inBuf,10,stdin);
		if (inBuf[0] == 'y' || inBuf[0] == 'Y'){flagAllRiddlesDone=false; riddlesUsed = 0;}
		else {return false;}
		*/
		flagAllRiddlesDone=false;
		riddlesUsed=0;
	}
	
	//If we need a new riddle and the one we previously got has already been used, pick a new one.
	while ((1 << whichRiddle) & riddlesUsed && !flagAllRiddlesDone){
		whichRiddle=rand()%8;			//gives random number between 0 and 7
	}

	//If we are going to give them a riddle, precious, then this is where we do it.
	if (!flagAllRiddlesDone){
		//Declare variables to be used
		char riddleFileName[10], riddleBuffer[511];
		long lSize;

		//Add this whichRiddle to the riddles used
		riddlesUsed += (1 << whichRiddle);
	
		//Open the file, see how long it is, read it till it's over, then close it
		sprintf(riddleFileName,"%d.txt",whichRiddle);			//Create file name string
		riddleFile = fopen(riddleFileName,"r");				//OPen file for reading
		fseek(riddleFile,0,SEEK_END);					//find out length of file
		lSize=ftell(riddleFile);					//get size of file
		rewind(riddleFile);						//go back to beginning of file
		size_t result = fread(riddleBuffer,1,lSize,riddleFile);		//read the file into the buffer
		if (result < 0){error("Unable to open riddle file.");return false;}
		fclose(riddleFile);						//close the riddle file
		
		//At this point we have the riddle in riddleBuffer. We now need to show the user what the clue is, then wait for a response
		//Declare variables		
		int counter=0, answerCounter=0;
		char clueBuffer[500], answerBuffer[20],userAnswerBuffer[20],responseBuffer[40];
	
		//Parse the file looking for the clue. Seperated by a *
		while (riddleBuffer[counter] != '*'){clueBuffer[counter] = riddleBuffer[counter];counter++;}
		
		//Null terminate the clueBuffer string so it prints correctly. Increment the counter to get to beginning of answer
		clueBuffer[counter]='\0';
		counter++;	

		//Put the answer into the answerBuffer.
		while ((int)riddleBuffer[counter] > 96 && (int)riddleBuffer[counter] < 123){	//must be lowercase character
			answerBuffer[answerCounter++]=riddleBuffer[counter];
			counter++;
		}

		//Null terminate
		answerBuffer[answerCounter]='\0';			

		//Interact with the user
		strcat(clueBuffer,"\nAnswer: ");
		int n0 = write(socketHandle,clueBuffer,strlen(clueBuffer)+10);	//write what the clue is
		if (n0<0){error("Error writing to socket."); return false;}
		int n1 = read(socketHandle,userAnswerBuffer,20);		//Get their answer
		if (n1 < 0){error("Error reading from socket."); return false;}
		//See if their answer was correct or not. Send the answer if it was or not	
		if (!strncmp(userAnswerBuffer,answerBuffer,answerCounter-1)){
			strcpy(responseBuffer,"Correct!");
		} else {
			strcpy(responseBuffer,"Wrong! The answer is ");
			strcat(responseBuffer,answerBuffer);
		}
		int n2 = write(socketHandle,responseBuffer,strlen(responseBuffer));
		if (n2<0){error("Error writing to socket.");return false;}
	}//end if we are all out of riddles. If we got here, we should be done
	return true;
}
		
		
/*================================================================================================================*/



		


