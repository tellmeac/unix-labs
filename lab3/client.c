#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024
#define LISTEN_PORT 8080
#define LOCAL_HOST "127.0.0.1"

int main()
{
    int cfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    address.sin_port = htons(LISTEN_PORT);

    if (connect(cfd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("connect");
        exit(1);
    }

    char buff[BUF_SIZE];
    for (;;)
    {
        // Get user imput
        printf("~ $ ");
        fgets(buff, BUF_SIZE, stdin);

        // Send to server
        write(cfd, &buff, BUF_SIZE);

        // Read the response back
        read(cfd, &buff, BUF_SIZE);
    }

    close(cfd);
    puts("Connection closed");
    exit(0);
}