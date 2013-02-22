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

//#define DEBUG0
#define CHECKOFF_ONE

//Global Variables
bool running=true;

//Forward Declarations
void sighandler(int sig);
void formatLine(char *line);

void main(){
	char *line;			//declare array for line of input
	int nbytes=255;			//max number of charachters in that input
	char *args[128];		//Command line arguments, array of strings.
	//Declare interrupt keys/commands to handlers.
	signal(SIGABRT, &sighandler);
	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);
	//Declare space in heap for the line and args
	line=(char *)malloc(nbytes+1);
	args[128]=(char*)malloc(128);
	while (running){
		int currentArg=0, j;
		//Print default shell forward. Get input of the line.
		printf("tms>");
		getline(&line,&nbytes,stdin);
		formatLine(line);
		#ifdef DEBUG0
			printf("%s\n",line);	//should just echo line back
		#endif
		if (!strcmp(line,"exit")){running=false; break;}
		else if (running){ 
			char currentChar;
			int i=-1, place=-1;
		 	do {
				#ifdef DEBUG0
					printf("Top of outer loop\n");
				#endif
				char *currentString=malloc(20);		//find 20 bytes of space
				do {
					#ifdef DEBUG0
						printf("While loop with i=%d\n",i);
						printf("\tCurrentchar = %c\n", line[i+1]);
					#endif
					currentChar=line[++i];
					currentString[++place]=currentChar;
				} while (line[i+1]!=' ' && line[i+1]!='\0' && line[i+1]!='\n' && line[i+1]!=NULL);
				#ifdef DEBUG0
					printf("Out of loop, about to place %s\n",currentString);
				#endif
				currentString[++i]='\0';	//null terminate the string.
				args[currentArg++]=currentString;
				place=-1;
				
			} while(line[i+1]!='\0');			
		}
		//add a null terminated arg at the end
		args[currentArg++]=NULL;
		#ifdef CHECKOFF_ONE
			//For step 1, show that there is something in args[x]
			for (j=0; j < currentArg; j++){
				printf("arg[%d]=%s\n",j,args[j]);
			}	
		#endif	
	}
	free(line);
	free(args[128]);
	
}


/*******************************************************************/

void sighandler(int sig)
{
   	printf(" Signal %d caught\n",sig);
	running=false;
//	printf("tms>");
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

