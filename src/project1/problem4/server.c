#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void *serve(void *newsockfd);

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen, n;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    portno = 2050;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR on binding\n");
        exit(1);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    printf("Server initialing...\n");

    return 0;
}

void *serve(void *sockfd)
{
    int newsockfd = (int)(*((int *)sockfd));
    
}