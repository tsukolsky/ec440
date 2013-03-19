#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>

void function(void);

int main(){
	function();
	return(0);
}

void function(){
	char *dmesgCmd[] = {"dmesg",0};
	char *tailCmd[] = {"tail","-n","30",0};
	char buffer[1024];

	int pipe1[2],pipe2[2];
	pipe(pipe1);
//	pipe(pipe2);
	
	int pid = fork();
	if (pid==0){
		int pid2= fork();
		if (pid2==0){
			dup2(pipe1[1],1);
			close(pipe1[0]);
			close(2);

			execvp(dmesgCmd[0],dmesgCmd);
		}else if (pid2>0){	
			dup2(pipe1[0],0);
			close(pipe1[1]);
			close(2);

			int n=read(pipe1[0],buffer,1024);
			execvp(tailCmd[0],tailCmd);
			waitpid(pid2,NULL,0);

		}else;	
	}else if (pid>0){
//Works after this

		close(pipe1[0]);
		close(pipe1[1]);
		printf("%s", buffer);
		waitpid(pid,NULL,0);
	}else;
}
