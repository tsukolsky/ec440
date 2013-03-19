//This is a test for the "dmesg | tail -n 30" command
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>

/* Changes*/
/* Define buffer size, removed spacing in tailCMD declaration, added parenthesis after pipe(tailPipe) error, took out } in else of pid<1, changed printf statement, doesn't really matter though.-->Command is "-n 30". Compiles, getting hung up. something with pipes.*/
#define BUFFER_SIZE 1024

void dealWithConnection(int socketHandle);

int main(){
	dealWithConnection(30);
	return 0;
}

void dealWithConnection(int socketHandle){
	int pid1,pid2,dmesgPipe[2],tailPipe[2],n,n2;
	char buffer1[BUFFER_SIZE];
	char *tailCmd[] = {"tail","-n","30",0};
	char *dmesgCmd[] = {"dmesg",0};
	
	if (pipe(dmesgPipe) <0){error("Error creating pipe 1.");}
	if (pipe(tailPipe) < 0){error("Error creating tail pipe.");}

	if ((pid1=fork())==0){//Child process (tail)
		//We are going to run dmesg and pipe that into tail -30.
		if ((pid2=fork())==0){	//child process (dmesg)
			printf("In the dmesg program.");

			dup2(dmesgPipe[1],1);		//set stdout to the output side of the pipe.
			dup2(dmesgPipe[1],2);		//same with standard error
			
			close(dmesgPipe[0]);		//Close input side of the pipe so it doesn't block

			execvp(dmesgCmd[0],dmesgCmd);	//executes dmesg command and is sent to tail(parent process.)
			
			//Error message
			char errorBuffer2[50];
			int ret2;
			ret2=sprintf(errorBuffer2,"Could not exec program \"%s\"", dmesgCmd[0]);
			error(errorBuffer2);

		} else if (pid2 <0){error("Error creating tail process.");}
		else {	//tail process.
			//Duplicate an input stream from the dmesgPipe
			dup2(dmesgPipe[0],0);		//Set stdin of this function to be the input side of dmesgPipe
			close(dmesgPipe[1]);		//close output end of the pipe
			
			dup2(tailPipe[1],1);		//clone stdout and stderr as the output side of the pipe
			dup2(tailPipe[1],2);
			close(tailPipe[0]);		//close input end of the pipe
			
			//Execute "tail -l 30" command, output goes to the parent process
			execvp(tailCmd[0],tailCmd);
			
			//Get ready for error; hofefully it isn't needed...						
			char errorBuffer2[50];
			int ret2;
			ret2=sprintf(errorBuffer2,"Could not exec program \"%s\"", tailCmd[0]);
			error(errorBuffer2);

			//Wait for child process
			//waitpid(pid2,NULL,0);		//no zombies
		}	//end parent process (dmesg)
		
	} else if (pid1 < 0){error("Error creating tail and dmesg process.");}
	else {
		//This is the real output. We are going to be printing this stuff to the client
		dup2(tailPipe[0],0);		//Set stdin as the input of the tail pipe.
		close(tailPipe[1]);		//close output

		//n = read(tailPipe[0],buffer1,BUFFER_SIZE);	//read into a buffer that can then be printed back to the socket
		//n2 = write(sockedHandle,buffer1,sizeof(buffer1));	//print to the socket.
		//printf("%s",buffer1);
		//waitpid(pid1,NULL,0);		//no zombie processes please.
	}		
}
