/*******************************************************************************\
| rasPiServer.c
| Author: Todd Sukolsky
| ID: U50387016
| Initial Build: 3/18/2013
| Last Revised: 3/18/2013
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
|================================================================================
| *NOTES:(1) Basic socket info was provided by the instructor. Other information on 
|		  	 sockets can be found at ...
|		     http://www.microhowto.info/howto/listen_for_and_accept_tcp_connections_in_c.html
|	 (2) This document is being maintained on github in the location:
|		     https://github.com/tsukolsky/ec440.git/homework3
\*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#define ONE_MB 1024
#define NUMBER_OF_MB 10

/*===============================*/
/*	Forward Declarations	 */
/*===============================*/
void error(const char *msg);
bool dealWithConnection(int socketHandle);
	
/*===============================*/
/*      Main Program		 */
/*===============================*/

int main(int argc, char *argv[]){
	//Declare Variables
        int sockfd, newsockfd, portno;		//Server socket descriptor, client<->server socket descriptor, portno
        socklen_t clilen;					//length of socket name for client
        char buffer[256];					//buffer length we can write to
        struct sockaddr_in serv_addr, cli_addr;
        int n;								//length used for certain things
        if (argc < 2) {
                fprintf(stderr,"ERROR, no port provided\n");
        	exit(1);
        }

	//Create server socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0){error("ERROR opening socket");}

        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = atoi(argv[1]);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
	 
        //Bind local address to server socket
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
     	sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
	
        //Now listen
  	listen(sockfd,5);
	 
	 //THis is what should continue and contine and continue
	for (;;){
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
		//new sockfd has what we are going to be printing too. Can spawn a new process or whatever
		if (newsockfd < 0){error("ERROR on accept");}
		
		//Print out something
		//n=write(newsockfd,"I am connected and forking to you.",40);
		if (n<0){error("ERROR writing to socket...");}	
		else {
			bool successful = dealWithConnection(newsockfd);
			if (successful){printf("I sent the correct information.\r\n");}
			else{printf("Resolving error...\r\n");}
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

bool dealWithConnection(int socketHandle){
	const int bufferSize=ONE_MB*NUMBER_OF_MB;
	int pid1,pid2,dmesgPipe[2],tailPipe[2],n,n2;
	char buffer1[bufferSize];
	char *tailCmd[] = {"tail","-n", "30", 0};
	char *dmesgCmd[] = {"dmesg",0};

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
		n=read(tailPipe[0],buffer1,bufferSize);				//read
		if (n<0){error("Error receiving data from pipes.");return false;}
		n2 = write(socketHandle,buffer1,sizeof(buffer1));		//Print to socket
		if (n2<0){error("Error writing data to socket.");return false;}		
		
		//Wait for child process to finish		
		waitpid(pid1,NULL,0);
		return true;
	}else {error("Error forking/creating \"tail\" program."); return false;
}	
}


