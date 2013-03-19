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
|
|================================================================================
| *NOTES:(1) Basic socket info was provided by the instructor. Other information on 
|		  	 sockets can be found at ...
|		     http://www.microhowto.info/howto/listen_for_and_accept_tcp_connections_in_c.html
\*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

/*===============================*/
/*		Forward Declarations	 */
/*===============================*/
void error(const char *msg);

	
/*===============================*/
/*		   Main Program			 */
/*===============================*/

int main(int argc, char *argv[])
{
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
     if (sockfd < 0) 
        error("ERROR opening socket");
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
		n=write(newsockfd,"I am connected and forking to you.",40);
		if (n<0){error("ERROR writing to socket...");}	
		else {printf("I sent what was requested.\n");}
		close(newsockfd);
	 }
     close(sockfd);
     return 0; 
}

/*===============================*/
/*		     Functions			 */
/*===============================*/

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
