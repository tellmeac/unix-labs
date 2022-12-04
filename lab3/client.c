#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    char ch[80];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(10000);

    len = sizeof(address);
    result = connect(sockfd, (struct sockaddr *)&address, len);
    if (result < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("Enter your message: \n");
    while (1)
    {
        gets(ch);
        write(sockfd, &ch, 80);
        read(sockfd, &ch, 80);
        if (!strcmp(ch, "-1"))
            break;
    }

    close(sockfd);
    puts("Connection closed");
    exit(0);
}