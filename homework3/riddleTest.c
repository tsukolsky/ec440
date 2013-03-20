
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

bool giveThemARiddlePrecious(int socketHandle);

int main(){
	bool success = giveThemARiddlePrecious(20);
	return 0;
}

bool giveThemARiddlePrecious(int socketHandle){
	FILE *riddleFile;
	static int riddlesUsed=0;
	static bool flagAllRiddlesDone=false;
	
	srand(time(0));					//set seed for random number
	int whichRiddle=rand()%8;			//Get a value between 0 and 7
	
	//Check to see if we have used all the riddles. If we have, set the flag.
	if (riddlesUsed==255){flagAllRiddlesDone=true;}	
	else if (riddlesUsed == 0){int n1 = write(socketHandle,"Let's play a game of riddles, shall we? 'And if it loses, we eats it whole!'",79);}
	else;

	//If the flag is set, ask the client if they want to repeat riddles. Yes or yes are recognized, otherwise it will wait for a new connection.
	if (flagAllRiddlesDone){
		char inBuf[10];
		//int n1 = write(socketHandle,"Do you want to repeat riddles? I have run out of new ones...",61);
		//int n2 = read(socketHandle,inBuf,20);
		printf("Do you want to repeat riddles? ");
		fgets(inBuf,10,stdin);
		if (inBuf[0] == 'y' || inBuf[0] == 'Y'){flagAllRiddlesDone=false; riddlesUsed = 0;}
		else {return false;}
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
	
		//Open the file and read the first bit of text with the clue...
		sprintf(riddleFileName,"%d.txt",whichRiddle);

		riddleFile = fopen(riddleFileName,"r");
		fseek(riddleFile,0,SEEK_END);	//find out length of file
		lSize=ftell(riddleFile);
		rewind(riddleFile);
	
		size_t result = fread(riddleBuffer,1,lSize,riddleFile);	//read the file into the buffer
		fclose(riddleFile);					//close the riddle file
		
		//At this point we have the riddle in riddleBuffer. We now need to show the user what the clue is, then wait for a response
		int counter=0, answerCounter=0;
		char clueBuffer[500], answerBuffer[20],userAnswerBuffer[20],responseBuffer[40];
	
		//Parse the file looking for the clue. Seperated by a *
		while (riddleBuffer[counter] != '*'){clueBuffer[counter] = riddleBuffer[counter];counter++;}
		clueBuffer[counter]='\0';
		counter++;	//increment to get to answer
		while ((int)riddleBuffer[counter] > 96 && (int)riddleBuffer[counter] < 123){
			answerBuffer[answerCounter++]=riddleBuffer[counter];
			counter++;
		}
		answerBuffer[answerCounter]='\0';

		//Interact with the user
		//int n1 = write(socketHandle,clueBuffer,sizeof(clueBuffer));	//write what the clue is
		//int n2 = read(socketHandle,userAnswerBuffer,20);		//Get their answer
		printf("%s",clueBuffer);
		printf("\nAnswer:");
		fgets(userAnswerBuffer,10,stdin);	
		if (!strncmp(userAnswerBuffer,answerBuffer,answerCounter-1)){
			strcpy(responseBuffer,"Correct!");
		} else {
			strcpy(responseBuffer,"Wrong! The answer is ");
			strcat(responseBuffer,answerBuffer);
		}
		//int n3 = write(socketHandle,responseBuffer,sizeof(responseBuffer));
		printf("%s\n",responseBuffer);
	}//end if we are all out of riddles. If we got here, we should be done
	return true;
}
