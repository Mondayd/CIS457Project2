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
   char command[20], filename[30], username[16], speed[10], portnum[20], hostname[30];
   char key[20];
   char* word  = (char*)(malloc((26)*sizeof(char)));
   char* linebuffer  = (char*)(malloc((255)*sizeof(char)));
   char* tmpbuffer  = (char*)(malloc((255)*sizeof(char)));
   FILE *fp, *tmp;
     
   
   printf("Conected!\n");
   n = write(sock,"central",9);
   bzero(buffer,256);
   
   n = read(sock,buffer,255);
   strcpy(username,buffer);
   bzero(buffer,256);
   n = write(sock, "Ok", 3);
   n = read(sock,buffer,255);
   strcpy(portnum,buffer);
   bzero(buffer, 256);
   n = write(sock, "Ok", 4);
   n = write(sock,"OK", 4);
   n = read(sock,buffer,255);
   strcpy(speed,buffer);
   bzero(buffer, 256);
   n = write(sock,"OK", 4);
   n = read(sock,buffer,255);
   strcpy(hostname,buffer);
   bzero(buffer, 256);
	
   fp = fopen("userlist.txt", "a");
   if (fp == NULL){
   	printf("FILE NOT FOUND!\n");
   }
   fprintf(fp,"User: %s Hostname: %s Port#: %s Speed: %s \n", username, hostname, portnum, speed);
   fclose(fp);

   n = write (sock, "Ok", 3);
   bzero(buffer, 256);
   
   printf("getting file descriptions...\n");

   fp = fopen("filelist.txt", "a");
   n = write(sock,"OK",3);
   n = read(sock, buffer, 255);
   while(strcmp(buffer,"done") != 0){
	fprintf(fp,"%s %s",username, buffer);
	bzero(buffer, 256);
	n = write(sock, "ok", 3);
        n = read(sock, buffer, 255);
   }

   fclose(fp);

   printf("Added %s to server....\n", username);
   
   bzero(buffer,256);

   while(strcmp(buffer,"quit") != 0){
	
	n = read(sock, buffer, 255);
	printf("%s\n",buffer);

	if(strcmp(buffer,"search") == 0){
		n = write(sock, "ok", 3);
		bzero(buffer,256);
		
		n = read(sock, buffer, 255);
		strcpy(key, "");
		strcpy(key,buffer);
		printf("Here are the files that contain %s \n", key);

		fp = fopen("filelist.txt", "r");
		fgets(linebuffer,255,fp);
		strcpy(buffer,linebuffer);
		while(feof(fp) == 0){
			word = strtok(linebuffer, " \n");
			if(strcmp(word,key) == 0){
				printf("%s",buffer);
			}
			while(word != NULL){
				if(strcmp(word,key) == 0){
					printf("%s",buffer);
					n = write(sock,buffer, strlen(buffer));
					n = read(sock, buffer, 255);
					bzero(buffer,256);
				}
				word = strtok(NULL, " \n");
			}
			fgets(linebuffer,255,fp);
			strcpy(buffer,linebuffer);

		}
		fclose(fp);
		n = write(sock, "end", 4);
		bzero(buffer,256);
	}
	if(strcmp(buffer,"retrieve") == 0){
		n = write(sock, "Central: Please connect to file host to retrieve a file\n", 57);
		bzero(buffer,256);
	}
	if(strcmp(buffer,"list") == 0){
		printf("Sending list to user: %s \n", username);
		n = write(sock, "ok", 3);
		fp = fopen("userlist.txt", "r");
		n = read(sock,buffer,255);
		fgets(buffer,255,fp);
		while(feof(fp) == 0){
			n = write(sock,(("%s"),buffer),strlen(buffer));
                    bzero(buffer,256);
                    n = read(sock,buffer,255);
                    if(strcmp(buffer,"ok") == 0){
                        bzero(buffer,256);
                        fgets(buffer,255,fp);
                    }else if(strcmp(buffer,"done") == 0){
			   printf("List sent!\n");
		      }else{
                        printf("Error Sending List!\n");
                        break;
		      }
		}
		strcpy(buffer, "");
		n = write(sock, "done", 5);
		bzero(buffer,256);
		rewind(fp);
		fclose(fp);
		fp == NULL;
	}

   }
 
   strcpy(linebuffer, "");
   fp = fopen("userlist.txt", "r");
   tmp = fopen ("temp.txt", "w");	
   
   fgets(linebuffer, 255, fp);
   strcpy(tmpbuffer, linebuffer);
   while(feof(fp) == 0){
	word = strtok(linebuffer, " \n");
	word = strtok(NULL, " \n");

	if(strcmp(word,username) != 0)
		fprintf(tmp, "%s", tmpbuffer);
	fgets(linebuffer, 255, fp);
   	strcpy(tmpbuffer, linebuffer);

   }
   fclose(fp);
   fclose(tmp);
   remove("userlist.txt");
   rename("temp.txt", "userlist.txt");

   strcpy(linebuffer, "");
   fp = fopen("filelist.txt", "r");
   tmp = fopen ("temp.txt", "w");	
   
   fgets(linebuffer, 255, fp);
   strcpy(tmpbuffer, linebuffer);
   while(feof(fp) == 0){
	word = strtok(linebuffer, " \n");

	if(strcmp(word,username) != 0)
		fprintf(tmp, "%s", tmpbuffer);

	fgets(linebuffer, 255, fp);
   	strcpy(tmpbuffer, linebuffer);
   }

   fclose(fp);
   fclose(tmp);
   remove("filelist.txt");
   rename("temp.txt", "filelist.txt");

   printf("%s has left the server.\n", username);	
}
