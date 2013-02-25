/*******************************************************************************\
| myShell.c
| Author: Todd Sukolsky
| ID: U50387016
| Initial Build: 2/18/2013
| Last Revised: 2/24/2013
| Copyright of Todd Sukolsky
|================================================================================
| Description:  This is a C program that makes a custom shell with on a UNIX computer.
|	  	The goal of this shell is for it to be ported over to the Rasberry PI
|	  	and used on that to perform simple pipes and forks and such. It 
|		recognizes special character >, 1>, 2>, >>, 2>>, <, &>, |, and &.
|		It only works with two commands, however. Only so much time in a day/week.
|--------------------------------------------------------------------------------
| Revisions: 2/18: Initial build. Got everyhting working for week 1 checkout. Remembered
|		   how to use heap in C
|	     2/24: Finished everyhting for week 2 checkout, about 5 hours of work. 
|		   Had issues with piping at first, no longer. Also had errors with 
|		   incorrect arguments being retained in later argument calls; change 
|		   of variable scope fixed that. De-bloated the thing. Could add a revision
|		   to file piping. Instead of being read into parent then pushed to a file,
|		   could append STDOUT of pipe to the actual file...?
|================================================================================
| *NOTES: (1) Checkoff one is one week into the assignment and requires the demonstration
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
//#define CHECKOFF_ONE
//#define DEBUG1
//#define DEBUG2

#define BUFFER_SIZE 1024*2		//change second number for more space

//Global Variables
bool running=true;			//Main loop param; infinite loop till exit or memory/system error
int child1,child2;			//Get pid's of forked processes that can be killed



//Forward Declarations
void sighandler(int sig);		//Catches signals
void formatLine(char *line);		//Formats input line in neat way, takes out crap
void error(char *s);			//Prints error messages

/*--------------------------------------------------------------------BEGIN MAIN------------------------------------------------------------------------------------*/
void main(){
	//Variables used to identify what special character is input; bools represent whether something is happening or not; next represent current number of input arguments
	int redirects;
	int currentArg,currentPipedArg;
	bool redirection,piping;

	//Variables for input/file io
	char *line;			//declare array for line of input
	int nbytes=255;			//max number of charachters in that input
	char *args[128];		//Command line arguments, array of strings.
	char *secondArgs[128];		//Command line arguments for more pipes
	char *fileName;

	//Declare interrupt keys/commands to handlers.
	signal(SIGABRT, &sighandler);
	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);

	/*************************MAIN LOOP****************************/
	while (running){
		//Re-initialize variables
		redirection=false;
		piping=false;
		redirects=0;
		child1=-1;
		child2=-1;

		//Declare space in heap for the line and args, secondArgs for piped input, fileName for possible file io	
		line=(char *)malloc(nbytes+1);
		args[128]=(char *)malloc(128);
		secondArgs[128]=(char *)malloc(128);
		fileName=(char *)malloc(nbytes+1);
		
		//Re-initialize placement variables of arguments
		currentArg=0; currentPipedArg=0;

		//Print default shell forward. Get input of the line.
		printf("tms>");
		getline(&line,&nbytes,stdin);
		formatLine(line);
		
		//If line was an "exit" call, get out; if blank, print new command line; otherwise parse the string
		if (!strcmp(line,"exit")){running=false; exit(0);}
		else if (!strcmp(line,""));
		else { 
			/*********************Parse string**************************/
			char currentChar;		//char being read at the moment
			char *currentString;		//current string
			int i=-1, place=-1;		//'i' represents the place in "line" we are. 'place' represents where in "currentString" we are
		
			//While the input line isn't the null terminator, read the line.
		 	do {
				currentString=(char *)malloc(24);		//find 20 bytes of space
				do {
					currentChar=line[++i];			//get character
					currentString[++place]=currentChar;	//add character to new currentString
				} while (line[i+1]!=' ' && line[i+1]!='\0' && line[i+1]!='\n' && line[i+1]!=NULL); //Most important is blank space. This line defines what seperates arguments
				
				//Finished finding an argument, null terminate and increment the currentArg counter, rest place to -1.
				currentString[++i]='\0';	
				
				//see if that arg was something interesting, aka redirection. Stored as binary representation. 9'b where LSB is ">" and MSB is "&".				
				if (!strcmp(currentString,">\0")){redirection=true;redirects+=(1 << 0);}
				else if (!strcmp(currentString,"1>\0")){redirection=true;redirects+= (1 << 1);}
				else if (!strcmp(currentString,"2>\0")){redirection=true;redirects+= (1 << 2);}
				else if (!strcmp(currentString,">>\0")){redirection=true;redirects+= (1 << 3);}
				else if (!strcmp(currentString,"2>>\0")){redirection=true;redirects+= (1 << 4);}
				else if (!strcmp(currentString,"&>\0")){redirection=true;redirects+= (1 << 5);}
				else if (!strcmp(currentString,"<\0")){redirection=true;redirects+= (1 << 6);}
				else if (!strcmp(currentString,"|\0")){redirects+= (1 << 7);piping=true;}
				else if (!strcmp(currentString,"&\0")){redirects+= (1 << 8);}
				else if (!redirection) {	//No redirect means the last argument was not redirect, normal argument. 
					if (!piping){		//No pipe, argument goes into "args" string representing main program call
						args[currentArg++]=currentString;
					} else {		//There is a pipe command somewhere before this, add all remaining arguments to "secondArgs" array
						secondArgs[currentPipedArg++]=currentString;
					}
				}
				else if (redirection){fileName=currentString;redirection=false;}	//last character was a redirect, means there is a file string coming next. *NOte, we don't discern
													//whether it's an input file or ourput file here. That comes later, like fear <BANE>

				//Debug line
				#ifdef DEBUG1
					if (redirection){printf("Found redirect in arg[%d]=%s\n",currentArg,currentString);}				
				#endif

				//Reset place holder for next iteration through do while loop
				place=-1;
				
								
			} while(line[i+1]!='\0');			
		
			//add a null terminated arg at the end of args and secondArgs (if there is even secondArgs, still put it there anyway)
			args[currentArg++]=NULL;
			secondArgs[currentPipedArg++]=NULL;

			#ifdef CHECKOFF_ONE
				//For step 1, show that there is something in args[x]
				short int j,l;
				for (j=0; j < currentArg; j++){
					printf("arg[%d]=%s\n",j,args[j]);
				}
				for (l=0; l < currentPipedArg; l++){
					printf("secondArgs[%d]=%s\n",l,secondArgs[l]);
				}	
			#endif	
		


			/**********************Execution******************************/ 
			//Declare variables
			int readPipe[2],writePipe[2], internalPipeWrite[2], pid1 , pid2, n;		//read is reading from child process, write is writing too, internalPipeWrite is used for pipe command
			long lSize;
			char outbuf[BUFFER_SIZE],inbuf[BUFFER_SIZE];					//inbuf for reading from file, outbuf for reading from pipe
			bool toFile=false,fromFile=false,background=false,pipeCommand=false;		//Bools for what is going on. **Note:"piping" (~line 49,115) is used in this stage
			FILE *file;									//File that wea re going to be doing
			
			//Initialize pipes
			if (pipe(readPipe) <0){error("read pipe");}
			if (pipe(writePipe)<0){error("write pipe");}
			
			//Check redirects
			//First case is any FILE OUTPUT
			if (redirects  & (1 << 0) || redirects & (1 << 1) || redirects & (1 << 2) || redirects & (1 << 3) || redirects & (1 << 4) || redirects & (1 << 5)){		
				#ifdef DEBUG2
					printf("Opening %s.\n",fileName);
				#endif
				//Set printing to file flag to true				
				toFile=true;

				//Should be appending in these cases, otherwise just open for writing
				if (redirects & (1 << 3) || redirects & (1 << 4)){
					file = fopen(fileName,"a");
				}
				else { 
					file = fopen(fileName,"w");
				}
				free(fileName);		//free memory

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
			//Background op case, set flag. Fork deals with that
			} else if (redirects & (1 << 8)){
				background=true;	
			}
			
			//Go into child
			if ((pid1=fork())==0){	
				if (background){
					setpgid(0,0);	//puts child in new process group.
				} else if (piping){
					//Establish pipes
					if (pipe(internalPipeWrite)<0){error("innter output pipe");}

					//Get a new process
					pid2=fork();
					if (pid2==0){	
						//In child, kill normal FILEIO
						close(STDIN_FILENO);
						close(STDOUT_FILENO);
						close(STDERR_FILENO);
						
						//Need to get input from parent process which is going to be governed by pipedPipeRead					
						dup2(internalPipeWrite[0],STDIN_FILENO);	//stdin of file is coming in on the input of internalPipeWrite
						dup2(readPipe[1],STDERR_FILENO);		//readPipe STDOUT goes to main programs stdin
						dup2(readPipe[1],STDOUT_FILENO);
						close(readPipe[0]);			//close input side of read pipe
						close(internalPipeWrite[1]);		//close output side or internalPipeWrite
						
						//Execute program
						execvp(secondArgs[0],secondArgs);
						
						//Get ready for error; hofefully it isn't needed...						
						char errorBuffer2[50];
						int ret2;
						ret2=sprintf(errorBuffer2,"Could not exec program \"%s\"", secondArgs[0]);
						error(errorBuffer2);
					}//end child two 
					child2=pid2;		//declare child2 as the new PID, allows kill command in ctrl+C
				}//end if pipe command
				

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
				} else if (piping){
					dup2(internalPipeWrite[1],STDERR_FILENO);
					dup2(internalPipeWrite[1],STDOUT_FILENO);	//output of standard out
					close(internalPipeWrite[0]);	//close input side
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
			//Parent Process; Maybe Grandparent if piping	
			} else {
				#ifdef DEBUG1			
					printf("\n\t**NOTE:Spawned %s\n\n",args[0]);

				#endif
				child1=pid1;
			
				//If input coming from file, close input of writePipe, send inbuf over output of writePipe				
				if (fromFile){
					close(writePipe[0]);
					write(writePipe[1],inbuf,lSize);				
				}

				//Read from the child process(es)
				close(readPipe[1]);				//close output side of pipe
				n=read(readPipe[0],outbuf,BUFFER_SIZE);		//read
				outbuf[n]=0;					//null terminate

				//If printing to file, print to file
				if (toFile){
					#ifdef DEBUG2
						printf("Writing to file\n");
					#endif
					fprintf(file,"%s",outbuf);
					fclose(file);

				//Printing to terminal
				} else {	
					printf("%s",outbuf);
				}
				
				//Free variables
				free(line);
				int k=0;
				for (k=0; k <= currentArg; k++){
					free(args[k]);
				}

				int u=0;
				for (u=0; u <= currentPipedArg; u++){
					free(secondArgs[u]);
				}
				
				//Wait for process to finish, no zombie processes.
				waitpid(pid1,NULL,0);
			}		
		}//end if 
		
	} //end while running	
	exit(0);
}
/*--------------------------------------------------------------------END MAIN------------------------------------------------------------------------------------*/
	
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

