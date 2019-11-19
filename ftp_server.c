#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>    //need this for listing
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include "string.h"
#include <arpa/inet.h>
#include <unistd.h>

void dostuff(int); /* function prototype */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen, pid;
     
     struct sockaddr_in serv_addr, cli_addr;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     while (1) {
         newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
             error("ERROR on accept");
         pid = fork();
         if (pid < 0)
             error("ERROR on fork");
         if (pid == 0)  {
             close(sockfd);
             dostuff(newsockfd);
             exit(0);
         }
         else close(newsockfd);
     } /* end of while */
     return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
   int n;
   char buffer[256];
   char command[20], filename[30];
   FILE *fp;     
   
   printf("Conected!\n");
   n = write(sock,"server",9);
   bzero(buffer,256);
   
   while(strcmp(buffer,"quit") != 0){
	
	n = read(sock, buffer, 255);
	printf("%s\n",buffer);

	if(strcmp(buffer,"retrieve") == 0){
		n = write(sock, "true", 5);
		bzero(buffer,256);
		n = read(sock, buffer, 255);
		printf("%s\n",buffer);
                fp = fopen(buffer, "r");
		
                if (fp == NULL) {
                    n = write(sock, "error", 6);
                    break;
                }
                n = write(sock, "ready", 6);
                bzero(buffer, 256);
		n = read(sock,buffer,255);
		printf("%s", buffer);
		printf("sending file...\n");
		fgets(buffer,255,fp);
		
                while(feof(fp)==0){
                    n = write(sock,(("%s"),buffer),strlen(buffer));
                    bzero(buffer,256);
		    strcpy(buffer,"");
                    n = read(sock,buffer,255);
                    if(strcmp(buffer,"ok") == 0){
                        bzero(buffer,256);
                        fgets(buffer,255,fp);
                    }else{
                        printf("Error Sending File! %s \n",buffer);
                        break;
                    }
                }
                n = write(sock, "done", 5);
                bzero(buffer, 256);
		fclose(fp);
		fp = NULL;
        }
   }    	

}
