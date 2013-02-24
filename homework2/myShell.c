/*******************************************************************************\
| myShell.c
| Author: Todd Sukolsky
| ID: U50387016
| Initial Build: 2/18/2013
| Last Revised: 2/18/2013
| Copyright of Todd Sukolsky
|================================================================================
| Description:  This is a C program that makes a custom shell with on a UNIX computer.
|	  	The goal of this shell is for it to be ported over to the Rasberry PI
|	  	and used on that to perform simple pipes and forks and such.
|--------------------------------------------------------------------------------
| Revisions:
|================================================================================
| *NOTES: Checkoff one is one week into the assignment and requires the demonstration
|	  of command line parsing.
\*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>

//#define DEBUG0
#define CHECKOFF_ONE
#define DEBUG1

#define BUFFER_SIZE 1024*10		//change second number for more space

//Global Variables
bool running=true;
int child1,child2;



//Forward Declarations
void sighandler(int sig);
void formatLine(char *line);
void error(char *s);

void main(){
	//Initialize redirects array
	int redirects;
	int place=0;
	bool redirection=false,piping=false;

	char *line;			//declare array for line of input
	int nbytes=255;			//max number of charachters in that input
	char *args[128];		//Command line arguments, array of strings.
	char *secondArgs[128];		//Command line arguments for more pipes
	char *fileName;

	//Declare interrupt keys/commands to handlers.
	signal(SIGABRT, &sighandler);
	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);

	while (running){
		redirects=0;
		child1=-1;
		child2=-1;

		//Declare space in heap for the line and args	
		line=(char *)malloc(nbytes+1);
		args[128]=(char *)malloc(128);
		fileName=(char *)malloc(nbytes+1);
		secondArgs[128]=(char *)malloc(128);
		
		//Declare variables used later in program; Current arg keeps track of how many args there are, j is for debugging
		int currentArg=0, currentSecondArg,j,redirectPlace=0;

		

		//Print default shell forward. Get input of the line.
		printf("tms>");
		getline(&line,&nbytes,stdin);
		formatLine(line);
		
		//If line was an "exit" call, get out; if blank, print new command line; otherwise parse the string
		if (!strcmp(line,"exit")){running=false; exit(0);}
		else if (!strcmp(line,""));
		else { 
			/*********************Parse string**************************/
			char currentChar;
			char *currentString;
			int i=-1, place=-1;
		 	do {
				currentString=(char *)malloc(24);		//find 20 bytes of space
				do {
					currentChar=line[++i];
					currentString[++place]=currentChar;
				} while (line[i+1]!=' ' && line[i+1]!='\0' && line[i+1]!='\n' && line[i+1]!=NULL);
				
				//Finished finding an argument, null terminate and increment the currentArg counter, rest place to -1.
				currentString[++i]='\0';	
				
				//see if that arg was something interesting, aka redirection. Stored as binary representation. 9'b where LSB is ">" and MSB is "&".				
				if (!strcmp(currentString,">\0")){redirection=true;redirects+=(1 << 0);redirectPlace=currentArg;}
				else if (!strcmp(currentString,"1>\0")){redirection=true;redirects+= (1 << 1);redirectPlace=currentArg;}
				else if (!strcmp(currentString,"2>\0")){redirection=true;redirects+= (1 << 2);redirectPlace=currentArg;}
				else if (!strcmp(currentString,">>\0")){redirection=true;redirects+= (1 << 3);redirectPlace=currentArg;}
				else if (!strcmp(currentString,"2>>\0")){redirection=true;redirects+= (1 << 4);redirectPlace=currentArg;}
				else if (!strcmp(currentString,"&>\0")){redirection=true;redirects+= (1 << 5);redirectPlace=currentArg;}
				else if (!strcmp(currentString,"<\0")){redirection=true;redirects+= (1 << 6);redirectPlace=currentArg;}
				else if (!strcmp(currentString,"|\0")){redirects+= (1 << 7);redirectPlace=currentArg;piping=true;}
				else if (!strcmp(currentString,"&\0")){redirection=true;redirects+= (1 << 8);redirectPlace=currentArg;}
				else if (!redirection) {
					if (!piping){
						args[currentArg++]=currentString;
					} else {
						secondArgs[currentSecondArg++]=currentString;
					}
				}
				else if (redirection){fileName=currentString;redirection=false;}

				//Debug line
				if (redirection){printf("Found redirect in arg[%d]=%s\n",currentArg,currentString);}				

				//Reset place holder for next iteration through do while loop
				place=-1;
				
								
			} while(line[i+1]!='\0');			
		
			//add a null terminated arg at the end
			args[currentArg++]=NULL;

			#ifdef CHECKOFF_ONE
				//For step 1, show that there is something in args[x]
				for (j=0; j < currentArg; j++){
					printf("arg[%d]=%s\n",j,args[j]);
				}
				int l;
				for (l=0; l < currentSecondArg; l++){
					printf("secondArgs[%d]=%s\n",l,secondArgs[l]);
				}	
			#endif	
		


			/**********************Execution******************************/ 
			//Declare variables
			int readPipe[2],writePipe[2], pipedPipeWrite[2], pid1 , pid2, n;		//read is reading from child process, write is writing too
			long lSize;
			char outbuf[BUFFER_SIZE],inbuf[BUFFER_SIZE];	
			bool toFile=false,fromFile=false,background=false,pipeCommand=false;
			FILE *file;
			
			//Initialize pipes
			if (pipe(readPipe) <0){error("read pipe");}
			if (pipe(writePipe)<0){error("write pipe");}
			
			//Check redirects
			//First case is any FILE OUTPUT
			if (redirects  & (1 << 0) || redirects & (1 << 1) || redirects & (1 << 2) || redirects & (1 << 3) || redirects & (1 << 4) || redirects & (1 << 5)){
				printf("See redirect in execution\n");				
				toFile=true;
				printf("Opening %s.\n",fileName);

				//Should be appending in these cases, otherwise just open in writing
				if (redirects & (1 << 3) || redirects & (1 << 4)){
					file = fopen(fileName,"a");
				}
				else { 
					file = fopen(fileName,"w");
				}
				free(fileName);
			//Using file as an input to a command, open for reading and set flag
			} else if (redirects & (1 << 6)){
				file = fopen(fileName,"r");
				fseek(file,0,SEEK_END);			//find out how long file is
				lSize=ftell(file);			//load size into a variable
				rewind(file);				//go abck to top of file
				size_t result = fread(inbuf,1,lSize,file);	//read into a buffer
				fclose(file);		
				
				fromFile=true;				//Set the flag, free memory
				printf("Reading %s.\n",fileName);
				free(fileName);
			} else if (redirects & (1 << 8)){
				background=true;
			} else if (redirects & (1 << 7)){
				pipeCommand=true;	
			}
			
			//Go into child
			if ((pid1=fork())==0){	
				if (background){
					setpgid(0,0);	//puts child in new process group.
				} else if (pipeCommand){
					//Establish pipes
					if (pipe(pipedPipeWrite)<0){error("innter output pipe");}

					//Get a new process
					pid2=fork();
					if (pid2==0){	
						//In child
						close(STDIN_FILENO);
						close(STDOUT_FILENO);
						close(STDERR_FILENO);
						
						//Need to get input from parent process which is going to be governed by pipedPipeRead					
						dup2(pipedPipeWrite[0],STDIN_FILENO);
						dup2(readPipe[1],STDERR_FILENO);
						dup2(readPipe[1],STDOUT_FILENO);
						close(readPipe[1]);			//close output side of read pipe
						close(pipedPipeWrite[0]);
						
						execvp(secondArgs[0],secondArgs);
						char errorBuffer2[50];
						int ret2;
						ret2=sprintf(errorBuffer2,"Could not exec program \"%s\"", secondArgs[0]);
						error(errorBuffer2);
					}//end child two
					
				}//end if pipe command

				child2=pid2;

				//Close standard file procedures
				close(STDIN_FILENO);
				close(STDOUT_FILENO);
				close(STDERR_FILENO);
	
				//Setup inputs.

/**************************Scenarios**********************************\
Parent receive from child: parent close main[1], child closes main[0].
Parent send to child: parent close main[0], child closes main[1].

\**************************END-Scenarios******************************/

				//Setup outputs
				if (redirects & (1 << 4) || redirects & (1 << 2)){
					dup2(readPipe[1],STDERR_FILENO);
					close(readPipe[0]);		//close input side of pipe
				} else if (redirects & (1 << 5)){
					dup2(readPipe[1],STDERR_FILENO);
					dup2(readPipe[1],STDOUT_FILENO);
					close(readPipe[0]);		//close input side of pipe
				} else if (fromFile){			//there is an input from a file coming.					
					dup2(writePipe[0],STDIN_FILENO);		//stdin is coming from writepipe
					dup2(readPipe[1],STDERR_FILENO);
					dup2(readPipe[1],STDOUT_FILENO);
					close(readPipe[1]);		//close output side of pipe, going to receive string.
					close(writePipe[0]);		//close input to line
				} else if (pipeCommand){
					dup2(pipedPipeWrite[1],STDOUT_FILENO);
					dup2(pipedPipeWrite[1],STDERR_FILENO);
					close(pipedPipeWrite[0]);	//close input side of pipe to forked fork
				} else { 
					dup2(readPipe[1],STDERR_FILENO);
					dup2(readPipe[1],STDOUT_FILENO);
					close(readPipe[0]);
				}
				
				//Execute the program; if error call an issue function
				execvp(args[0], args);
				char errorBuffer[50];
				int ret;
				ret=sprintf(errorBuffer,"Could not exec program \"%s\"",args[0]);
				error(errorBuffer);	
			} else {
				//In parent
				child1=pid1;			
				printf("\n\t**NOTE:Spawned %s\n\n",args[0]);
				
				if (fromFile){
					close(writePipe[0]);
					write(writePipe[1],inbuf,lSize);				
				}

				close(readPipe[1]);		//close output side of pipe
				n=read(readPipe[0],outbuf,BUFFER_SIZE);
				outbuf[n]=0;	
				if (toFile){
					printf("Writing to file\n");
					fprintf(file,"%s",outbuf);
					fclose(file);
				} else {	//if silent, chill and don't print anything
					printf("%s",outbuf);
				}
				
				free(line);
				int k=0;
				for (k=0; k < currentArg; k++){
					free(args[k]);
				}
				currentArg=0;
				
				//Wait for process to finish, no zombie processes.
				waitpid(pid1,NULL,0);
			}		
		}//end if 
		
	} //end while running	
	exit(0);
}
	
/*******************************************************************/

void error(char *s)
{
  perror(s);
  exit(1);
}

/*******************************************************************/

void sighandler(int sig)
{
   	if (child1!=-1 || child2!=-1){kill(child1,SIGTERM); kill(child2,SIGTERM);printf("Killed process %d and %d.\n",child1,child2);}
}

/******************************************************************/
//Note: New line is put in as the i-2 subscript. i-1 is the termination.
void formatLine(char * line){
	int i=-1;
	char currentChar;
	do {
		currentChar=line[++i];
	} while (currentChar!='\n');
	line[i]='\0';
}

/******************************************************************/

