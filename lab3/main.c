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
#define LISTEN_PORT 8080
#define LOCAL_HOST "127.0.0.1"

static volatile sig_atomic_t gotSigHup = 0;

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
    addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    addr.sin_port = htons(LISTEN_PORT);

    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        close(lfd);
        return 1;
    }

    // Prepare sig mask for pselect with SIGHUP
    sigset_t blockedMask, emptyMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    if (sigprocmask(SIG_BLOCK, &blockedMask, NULL) == -1)
    {
        perror("sigprocmask");
        return 1;
    }
    sigemptyset(&emptyMask);

    // Register sigaction handler
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = sigHupHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGHUP, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    char buffer[BUF_SIZE];
    int activecfd;
    int clientCount = 0;

    if (listen(lfd, SOMAXCONN) == -1)
    {
        perror("listen");
        return 1;
    }

    for (;;)
    {
        bzero(buffer, sizeof(buffer));

        int nfds = -1;
        fd_set rfds;
        FD_SET(lfd, &rfds);

        if (lfd > nfds)
            nfds = lfd;

        if (clientCount > 0)
        {
            FD_SET(activecfd, &rfds);
            if (activecfd > nfds)
                nfds = activecfd;
        }

        printf("Server is waiting for connections or messages.\n");
        // Wait until ready to read or signals was caught.
        int ready = pselect(nfds + 1, &rfds, NULL, NULL, NULL, &emptyMask);
        if (ready == -1 && errno != EINTR)
        {
            perror("interrupted");
            return 1;
        }

        if (gotSigHup)
        {
            printf("Caught SIGHUP: keep working!\n");
            gotSigHup = 0;
            continue;
        }

        // Handle new connections
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

            activecfd = csfd;
            clientCount++;

            printf("New client has been accepted.\n");
            printf("Total number of clients: %d.\n\n", clientCount);
        }

        // Accept data from clients
        if (FD_ISSET(activecfd, &rfds))
        {
            ssize_t r = read(activecfd, &buffer, BUF_SIZE);
            if (r <= 0) // errors or EOF
            {
                close(activecfd);
                activecfd = -1;
                clientCount--;

                printf("Connection has been closed\n");
                continue;
            }

            printf("Message accepted:\n");
            size_t size = strlen(buffer);
            for (size_t i = 0; i < size; i++)
                printf("%c", buffer[i]);
            printf("\n");

            printf("Accepted size: %zu bytes\n", size);
        }
    }
}