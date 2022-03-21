#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORTNO 2050
#define BUFFER_SIZE 256

pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
int client_cnt = 0;

void encrypt(char buf[], int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        if (buf[i] >= 'a' && buf[i] <= 'z')
        {
            int offset = (buf[i] - 'a' + 3) % 26;
            buf[i] = (char)('a' + offset);
        }
        if (buf[i] >= 'A' && buf[i] <= 'Z')
        {
            int offset = (buf[i] - 'A' + 3) % 26;
            buf[i] = (char)('A' + offset);
        }
    }
}

void *serve(void *sockfd)
{
    pthread_detach(pthread_self());
    int available = 0;
    int newsockfd = (int)(*((int *)sockfd));
    char buffer[BUFFER_SIZE];

    while (!available)
    {
        bzero(buffer, BUFFER_SIZE);
        int n = read(newsockfd, buffer, BUFFER_SIZE - 1);
        if (strcmp(buffer, ":q") == 0)
        {
            close(newsockfd);
            printf("Server thread closing ...\n");
            return;
        }
        pthread_mutex_lock(&mu);
        if (client_cnt < 2)
        {
            client_cnt++;
            available = 1;
        }
        pthread_mutex_unlock(&mu);

        if (available)
        {
            printf("Receiving message: %s\n", buffer);
            encrypt(buffer, n);
            write(newsockfd, buffer, n);
            break;
        }
        write(newsockfd, "Please wait ...", 15);
    }

    while (available)
    {
        bzero(buffer, BUFFER_SIZE);
        int n = read(newsockfd, buffer, BUFFER_SIZE - 1);
        if (strcmp(buffer, ":q") == 0)
        {
            break;
        }
        printf("Receiving message: %s\n", buffer);
        encrypt(buffer, n);
        write(newsockfd, buffer, n);
    }

    pthread_mutex_lock(&mu);
    client_cnt--;
    pthread_mutex_unlock(&mu);
    close(newsockfd);
    printf("Server thread closing ...\n");
}

int main(int argc, char *argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORTNO);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR on binding\n");
        exit(1);
    }
    listen(sockfd, 5);

    printf("Server initialing...\n");

    struct sockaddr_in cli_addr;
    int clilen = sizeof(cli_addr);
    int newsockfd;
    int ret;

    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            printf("ERROR on accepting\n");
            exit(1);
        }

        pthread_t thread;
        ret = pthread_create(&thread, NULL, serve, (void *)&newsockfd);
        if (ret)
        {
            printf("ERROR on creat thread\n");
            exit(1);
        }
    }

    return 0;
}
