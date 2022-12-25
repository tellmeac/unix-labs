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

int main()
{
    int cfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(LISTEN_PORT);

    int r = connect(cfd, (struct sockaddr *)&address, sizeof(address));
    if (r == -1)
    {
        perror("connect");
        exit(1);
    }

    char buff[BUF_SIZE];
    for (;;)
    {
        printf("~ $ ");
        fgets(buff, BUF_SIZE, stdin);
        write(cfd, &buff, BUF_SIZE);
        read(cfd, &buff, BUF_SIZE);
        if (!strcmp(buff, "-1"))
            break;
    }

    close(cfd);
    puts("Connection closed");
    exit(0);
}