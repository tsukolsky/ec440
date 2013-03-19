#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>

#define ONE_MB 1024
#define NUMBER_MB 5

void function(void);

int main(){
	function();
	return(0);
}

void function(){
	const int bufferSize=ONE_MB*NUMBER_MB;
	char *dmesgCmd[] = {"dmesg",0};
	char *tailCmd[] = {"tail","-n","30",0};
	char buffer[bufferSize];

	int pipe1[2],pipe2[2];
	pipe(pipe1);
	pipe(pipe2);
	
	int pid = fork();
	if (pid==0){
		int pid2= fork();
		if (pid2==0){	//this is the dmesg program
			dup2(pipe1[1],1);
			close(pipe1[0]);
			close(2);

			execvp(dmesgCmd[0],dmesgCmd);
		}else if (pid2>0){	
			dup2(pipe1[0],0);
			close(pipe1[1]);
			close(2);
			
			dup2(pipe2[1],1);
			close(pipe2[0]);	//close the input of this pipe

			execvp(tailCmd[0],tailCmd);
			waitpid(pid2,NULL,0);

		}else;	
	}else if (pid>0){
		close(pipe1[0]);
		close(pipe1[1]);
		close(pipe2[1]);
		int n=read(pipe2[0],buffer,bufferSize);
		printf("%s", buffer);
		waitpid(pid,NULL,0);
	}else;
}
