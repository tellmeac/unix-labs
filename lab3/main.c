#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define BUF_SIZE 1024

volatile sig_atomic_t gotSigHup = 0;

static void sigHupHandler(int sig)
{
    gotSigHup = 1;
}

int main()
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket");
        return 1;
    }

    int yes = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        return 1;
    }

    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8080);

    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        close(lfd);
        return 1;
    }

    // Register sigaction handler
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    // Prepare sig mask for pselect with SIGHUP
    sigset_t blockedMask, origMask;
    sigemptyset(&blockedMask);

    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    char buffer[BUF_SIZE];
    int cfd, res;
    int clientCount = 0;

    if (listen(lfd, SOMAXCONN) == -1)
    {
        perror("listen");
        return 1;
    }

    for (;;)
    {

        bzero(buffer, sizeof(buffer));
        printf("-----------------\n");
        printf("Server is waiting\n");

        int nfds = -1;

        fd_set rfds;
        FD_SET(lfd, &rfds);

        if (lfd > nfds)
            nfds = lfd + 1;

        if (clientCount > 0)
        {
            FD_SET(cfd, &rfds);
            if (cfd > nfds)
                nfds = cfd + 1;
        }

        // Wait until ready to read or signals was caught.
        res = pselect(nfds, &rfds, NULL, NULL, NULL, &origMask);
        if (res == -1 && errno == EINTR)
        {
            puts("Caught kill signal.");
            return 1;
        }

        // New connections
        if (FD_ISSET(lfd, &rfds))
        {

            int csfd = accept(lfd, NULL, NULL);
            if (csfd == -1)
            {
                perror("accept client socket");
                continue;
            }

            // NOTE: By task deifinition, accept and instantly close any other new connections.
            if (clientCount >= 1)
            {
                close(csfd);
                printf("New client has been accepted ans closed.\n");
                continue;
            }

            cfd = csfd;
            clientCount++;

            printf("New client has been accepted.\n");
            printf("Total number of clients: %d.\n\n", clientCount);
        }

        // Accept data from clients
        if (FD_ISSET(cfd, &rfds))
        {
            read(cfd, &buffer, 80);
            size_t size = strlen(buffer);

            if (size <= 0)
            {
                close(cfd);
                cfd = -1;
                clientCount--;

                printf("Connection has been closed\n");
                continue;
            }

            printf("Message accepted:\n");
            for (size_t i = 0; i < size; i++)
                printf("%c", buffer[i]);
            printf("\n");

            // Echo message back
            send(cfd, buffer, BUF_SIZE, 0);
            printf("Accepted size: %zu bytes\n", size);
        }

        if (gotSigHup)
        {
            gotSigHup = 0;
            printf("SIGHUP caught! Keep working.\n");
        }
    }
}