#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include "string.h"
#include <arpa/inet.h>
#include <unistd.h>

void verifyN(int n, int direction);

void error (char *message) {
    perror(message);
    exit(0);
}

int sockfd; //stores values returned by the socket system
int portno; //stores the port number on which the server accepts connection
int n; //return value for the read() and write() calls

struct sockaddr_in serv_addr; //contains server address of the server to be connected to
struct hostent *server; //holds host information in struct hostent inside netdb.h

char buffer[256]; //sets buffer to read into

int main() {
    char inputSize = 20;
    int option;
    char command[20];
    int quit = 0;
    char filename[30];
    FILE *fp;
    char keyword[20];
    while (quit != 1) {
        printf("Please enter a command\n");
        fscanf(stdin, "%s", command);

        //drops string to lowercase
        for (int i = 0; i < strlen(command); i++) {
            command[i] = tolower(command[i]);
        }

        //converts input text to option paramater
        option = strcmp(command, "connect") == 0 ? 0 : strcmp(command, "list") == 0 ? 1 :
                                                       strcmp(command, "retrieve") == 0 ? 2 :
                                                       strcmp(command, "search") == 0 ? 3 :
                                                       strcmp(command, "quit") == 0 ? 4 : -1;

        switch (option) {
            case 0: //connect
                printf("connect\n");
                char portNumber[32], hostName[32], username[16], filename[32], portnum[20];
                fprintf(stdout, "please enter a host name and port number seperated by a space\n");
                fscanf(stdin, "%s %s", hostName, portNumber);
                fprintf(stdout, "hostName: %s, port: %s\n", hostName, portNumber);
                //user enters port number, if port isnt available send error
                portno = atoi(portNumber);
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0)
                    error("ERROR opening socket");

                //gets hostname from hostent in host ent *h_addr contains host ip address
                server = gethostbyname(hostName);
                if (server == NULL) {
                    fprintf(stderr, "ERROR, no such host\n");
                    exit(0);
                }

                //sets all the fields in serv_addr to the values of the server
                bzero((char *) &serv_addr, sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                bcopy((char *) server->h_addr,
                      (char *) &serv_addr.sin_addr.s_addr,
                      server->h_length);
                serv_addr.sin_port = htons(portno);

                //attempts to connect to the given socket and returns error if it cannot connect
                if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
                    error("ERROR connecting");
                } else {
                    fprintf(stdin, "Successfully Connected!");
                }
                memset(buffer, 0, strlen(buffer));
		n = read(sockfd,buffer,255);
		if(strcmp(buffer,"server") == 0)
		  break;
		
		printf("Please enter username:\n");
		scanf("%s", &username);
	        printf("Please enter host port number:\n");
		scanf("%s", &portnum);
		n = write(sockfd,username,strlen(username));
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		bzero(buffer, 256);
		n = write(sockfd,portnum,strlen(portnum));
		bzero(buffer,256);
		n = read(sockfd, buffer, 255);
                n = write(sockfd,("100 Mb/s"), 9);
		n = read(sockfd, buffer, 255);
		n = write(sockfd,("localhost"), 10);
		n = read(sockfd, buffer, 255);

		printf("Please enter file description list name:");
		scanf("%s", &filename);

		fp = fopen(filename, "r");

		if (fp == NULL) {
                    printf("FILE NOT FOUND\n");
		    n = write(sockfd, "reject",7);
                } else {
                    //write to the server the filename to store the file.
                    //n = write(sockfd, (("%s"), filename), strlen(filename));
		      printf("Sending file list...\n");
                    n = read(sockfd, buffer, 255);
                    //betgin moving data by sending 1 line at a time.
                    fgets(buffer, 255, fp);
                    while (feof(fp) == 0) {
                        n = write(sockfd, (("%s"), buffer), strlen(buffer));
                        bzero(buffer, 256);
                        n = read(sockfd, buffer, 255);
                        if (strcmp(buffer, "ok") == 0) {
                            bzero(buffer, 256);
                            fgets(buffer, 255, fp);
                        } else {
                            printf("Error Sending File!\n");
                            break;
                        }
                    }
		}
                fclose(fp);
		n = write(sockfd, "done", 5);
		printf("Setup is complete!");
		
                break;

            case 1: {//list
                n = write(sockfd, "list", 18);
                // reads and displays a list the server is sending back.
                while (strcmp(buffer, "done") != 0) {
                    n = read(sockfd, buffer, 255);
                    printf("\n");
                    //Waits for the server to say it's done sending the list.
                    if (strcmp(buffer, "done") == 0)
                        break;
                    printf("%s", buffer);
                    bzero(buffer, 256);
                    n = write(sockfd, "ok", 3);
                }
		  n = write(sockfd, "done", 5);
                break;
            }
            case 2: { //retrieve (will be the opposite of store)
                //reads the name of the file to be retrieved
                strcpy(buffer, "retrieve");
                n = write(sockfd, ("%s", buffer), strlen(buffer));
                bzero(buffer, 256);
                verifyN(n, 1);
		n = read(sockfd, buffer, 255);
		if(strcmp(buffer,"true") != 0){
			printf("%s \n", buffer);
			break;
		}
                //Asks for a file to retrieve and creates the local version of it.
                printf("Please enter a filename:");
                scanf("%s", &filename);
                fp = fopen(filename, "w");
		if(fp == NULL){
			printf("file not found");
		}
		printf("%s",filename);
                //sends file name over to server to retrieve
                n = write(sockfd, (("%s"), filename), strlen(filename));
                bzero(buffer, 256);
                n = read(sockfd, buffer, 255);
                if (strcmp(buffer, "error") == 0) {
                    printf("error");
                    break;
                }
                bzero(buffer, 256);
		n = write(sockfd, "ok",3);
		
		n = read(sockfd,buffer,255);
		
                while (strcmp(buffer, "done") != 0) {//stops when the server says it's "done".
                    fprintf(fp, "%s", buffer);
                    bzero(buffer, 256);
                    n = write(sockfd, "ok", 3);
                    bzero(buffer, 256);
		    n = read(sockfd,buffer,255);
		    
                }
                fclose(fp);
		fp = NULL;
                break;
            }
            case 3:
                strcpy(buffer, "search");
                n = write(sockfd, ("%s", buffer), strlen(buffer));
                bzero(buffer, 256);
                verifyN(n, 1);
                //Asks for a file to store and opens it.
                printf("Keyword:");
                scanf("%s", &keyword);
		printf("Here are the list of files that contain %s \n", keyword);
                n = write(sockfd, keyword, strlen(keyword));
		  while (strcmp(buffer, "end") != 0) {
                    n = read(sockfd, buffer, 255);
                    printf("");
                    //Waits for the server to say it's done sending the list.
                    if (strcmp(buffer, "end") == 0)
                        break;
                    printf("%s", buffer);
                    bzero(buffer, 256);
                    n = write(sockfd, "", 3);
		    bzero(buffer, 256);
                }

                break;
            case 4: //quit
                n = write(sockfd, "quit", 5);
                verifyN(n, 1);
                quit = 1;
                break;
            default:
                error("incorrect command entered\n");
                break;
                }
        }
}


void verifyN(int n, int direction) {
    if (direction == 1) {
        if (n < 0)
            error("ERROR writing to socket");
        bzero(buffer, 256);
    }
    else {
        error("ERROR reading from socket");
        bzero(buffer, 256);
    }
}

