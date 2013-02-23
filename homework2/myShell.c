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
#define NUMBER_OF_REDIRECTS 9

//Global Variables
bool running=true;
int child;



//Forward Declarations
void sighandler(int sig);
void formatLine(char *line);
void error(char *s);

void main(){
	//Initialize redirects array
	int redirects[20];
	int place=0;
	bool redirection=false;

	char *line;			//declare array for line of input
	int nbytes=255;			//max number of charachters in that input
	char *args[128];		//Command line arguments, array of strings.
	//Declare interrupt keys/commands to handlers.
	signal(SIGABRT, &sighandler);
	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);

	while (running){
		child=-1;
		//Declare space in heap for the line and args	
		line=(char *)malloc(nbytes+1);
		args[128]=(char *)malloc(128);
		
		//Declare variables used later in program; Current arg keeps track of how many args there are, j is for debugging
		int currentArg=0, j;

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
			int i=-1, place=-1;
		 	do {
				char *currentString=malloc(20);		//find 20 bytes of space
				do {
					currentChar=line[++i];
					currentString[++place]=currentChar;
				} while (line[i+1]!=' ' && line[i+1]!='\0' && line[i+1]!='\n' && line[i+1]!=NULL);
				
				//Finished finding an argument, null terminate and increment the currentArg counter, rest place to -1.
				currentString[++i]='\0';	
				args[currentArg++]=currentString;

				//see if that arg was something interesting, aka redirection
				if (!strcmp(currentString,">\0")){redirection=true;redirects[place++]=currentArg-1;}
				else if (!strcmp(currentString,"1>\0")){redirection=true;redirects[place++]=currentArg-1;}
				else if (!strcmp(currentString,"1>\0")){redirection=true;redirects[place++]=currentArg-1;}
				else if (!strcmp(currentString,"2>\0")){redirection=true;redirects[place++]=currentArg-1;}
				else if (!strcmp(currentString,">>\0")){redirection=true;redirects[place++]=currentArg-1;}
				else if (!strcmp(currentString,"2>>\0")){redirection=true;redirects[place++]=currentArg-1;}
				else if (!strcmp(currentString,"&>\0")){redirection=true;redirects[place++]=currentArg-1;}
				else if (!strcmp(currentString,"<\0")){redirection=true;redirects[place++]=currentArg-1;}
				else if (!strcmp(currentString,"|\0")){redirection=true;redirects[place++]=currentArg-1;}
				else;
				if (redirection){printf("Found redirect in arg[%d]=%s\n",currentArg-1,args[currentArg-1]);}				

				place=-1;
				
			} while(line[i+1]!='\0');			
		
			//add a null terminated arg at the end
			args[currentArg++]=NULL;

			#ifdef CHECKOFF_ONE
				//For step 1, show that there is something in args[x]
				for (j=0; j < currentArg; j++){
					printf("arg[%d]=%s\n",j,args[j]);
				}	
			#endif	
		


			/**********************Execution******************************/ 
			int main[2], pid, n;
			char buf[BUFFER_SIZE];	
			//pipe(child);
			pipe(main);
		
			//Go into child
			if ((pid=fork())==0){	
				//Close standard file procedures
				close(STDIN_FILENO);
				close(STDOUT_FILENO);
				close(STDERR_FILENO);
			
	
				dup2(main[1],STDOUT_FILENO);
				dup2(main[1],STDERR_FILENO);
				
				char *tempChar;
				tempChar=(char *)malloc(20);
				strcpy(tempChar,"/bin/");
				strcat(tempChar,args[0]);
				strcpy(args[0],tempChar);
				free(tempChar);
				
				execvp(args[0], args);

				error("Could not exec helloWorld program");	
			}
			child=pid;			
			printf("\n\t**NOTE:Spawned %s\n\n",args[0]);
			close(main[1]);

			n = read(main[0],buf,BUFFER_SIZE);
			buf[n]=0;
			printf("%s",buf);
	
			free(line);
			int k=0;
			for (k=0; k < currentArg; k++){
				free(args[k]);
			}
			currentArg=0;
			
			//Wait for process to finish, no zombie processes.
			waitpid(pid,NULL,0);
		}//end if running
			
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
   	if (child!=-1){kill(child,SIGTERM);}
	printf("Killed process %d\n",child);
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

