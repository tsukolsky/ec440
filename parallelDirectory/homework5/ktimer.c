#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
int main(int argc, char* argv[])
{
	int fd;
	char my_argument[130];
	char timer_argu[130];
	sprintf(my_argument, "%s %s", argv[2], argv[3]);
	fd = open("/dev/mytimer", O_RDWR | O_APPEND);
	if (fd < 0) {
		printf("Open /dev/mytimer failed!!!\n");
		exit(1);
	}
	else printf("Got it!\n");
	
	if (strcmp(argv[1],"-l")==0&&argc==2){
		//printf("list a new timer\n");
	  int char_count;
	  char_count=read(fd, timer_argu,129);
	  timer_argu[char_count]=0;
	  printf("%s\n",timer_argu);
	}
	else if (strcmp(argv[1],"-s")==0&&argc==4&&(argv[2][0]>='0'&&argv[2][0]<='9')&&((argv[3][0]>='a'&&argv[2][0]<='z')||(argv[3][0]>='A'&&argv[2][0]<='Z'))){	
		// Add timers	
		//printf("Argv[2] is %s and argv[3] is %s.\n", argv[2], argv[3]);		
		write(fd, my_argument, strlen(my_argument));
	}
	else{ 
        	printf("The command format is wrong\n");
		printf("ktimer -l\n");
                printf("lists to stdout the expiration time of the registered timers and messages that are set to be printed.\n");
		printf("ktimer -s [number] [message]\n");
                printf("specifies the time offset and the message to be printed on the console upon expiration.\n");
	}

	close(fd);
	return 0;
}
