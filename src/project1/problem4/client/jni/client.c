#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 256
#define PORTNO 2050

int main(int argc, char *argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }

    struct hostent *server;
    server = gethostbyname("127.0.0.1");
    if (server == NULL)
    {
        printf("ERROR no such host\n");
        exit(0);
    }

    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(PORTNO);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR connecting\n");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    while (1)
    {
        printf("Please enter the message:\n");
        bzero(buffer, BUFFER_SIZE);
        gets(buffer);
        // printf("len : %d\n", strlen(buffer));
        write(sockfd, buffer, strlen(buffer));
        if (strcmp(buffer, ":q") == 0)
        {
            printf("Client closing ...\n");
            return 0;
        }
        bzero(buffer, BUFFER_SIZE);
        read(sockfd, buffer, BUFFER_SIZE - 1);
        printf("From server: %s\n", buffer);
    }

    return 0;
}