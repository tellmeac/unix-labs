#include <sys/types.h>
#include <sys/socket.h> // содержит определения флагов уровня сокета
#include <stdio.h>
#include <netinet/in.h> // Protocols are specified
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

// An integer type which can be accessed as an atomic entity even in the presence of asynchronous interrupts made by signals.
volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int r)
{
    wasSigHup = 1;
}

int main()
{
    int listenfd = 0;

    // Создание сокета
    // The function socket() creates an endpoint for communication and returns a file descriptor for the socket.
    // Takes 3 arguments domain, which specifies the protocol family of the created socket; type; protocol, the value 0 may be used to select a default protocol from the selected domain and type.
    // The function returns -1 if an error occurred. Otherwise, it returns an integer representing the newly assigned descriptor.

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket"); // Prints a textual description of the error code currently stored in the system variable errno to stderr.
        return -1;
    }

    // Sets a socket option
    // Takes a descriptor that identifies a socket;
    // the level at which the option is defined;
    // the socket option for which the value is to be set, the optname parameter must be a socket option defined within the specified level, or behavior is undefined;
    // a pointer to the buffer in which the value for the requested option is specified;
    // The size, in bytes, of the buffer pointed to by the optval parameter
    // SO_REUSEADDR указывает, что правила проверки адресов, передаваемых с помощью вызова bind(2), должны позволять повторное использование локального адреса.

    int yes = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        return -1;
    }

    // Устанавливает первые n байтов области, начинающейся с s в нули (пустые байты).
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(10000);

    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(listenfd);
        return -1;
    }

    // If set to SOMAXCONN, the underlying service provider responsible for socket s will set the backlog to a maximum reasonable value.
    if (listen(listenfd, SOMAXCONN) < 0)
    {
        perror("listen");
        return -1;
    }

    // Register action handlers
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);

    // Blocking signals
    sigset_t blockedMask, origMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    char ch[80];
    int client_fd, res;
    int numberOfClients = 0;

    while (!wasSigHup)
    {

        memset(ch, 0, sizeof(ch));
        printf("-----------------\n");
        printf("Server is waiting\n");

        int maxFd = -1;

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(listenfd, &fds);

        // Подготовка списка fd
        if (listenfd > maxFd)
            maxFd = listenfd;

        if (numberOfClients > 0)
        {
            FD_SET(client_fd, &fds);
            if (client_fd > maxFd)
                maxFd = client_fd;
        }

        // n на единицу больше самого большого номера описателей из всех наборов.

        // Вызов функции pselect() временно
        // разблокирует необходимый сигнал и дождётся одного
        // из трёх событий

        res = pselect(maxFd + 1, &fds, NULL, NULL, NULL, &origMask);
        if (res == -1 && errno == EINTR)
        {
            puts("Caught kill signal");
            return -1;
        }

        // New connections
        if (FD_ISSET(listenfd, &fds))
        {

            int client_sockfd = accept(listenfd, NULL, NULL);
            if (client_sockfd == 0)
                perror("accept client_sockfd");

            // By task deifinition, close any other connection
            if (numberOfClients >= 1)
            {
                close(client_sockfd);
                continue;
            }

            if (client_fd < 0)
            {
                perror("accept");
                continue;
            }

            client_fd = client_sockfd;
            numberOfClients++;
            printf("New client has been accepted\n");
            printf("Total number of clients: %d\n\n", numberOfClients);
        }

        // Accept data from clients
        if (FD_ISSET(client_fd, &fds))
        {
            read(client_fd, &ch, 80);
            int k = strlen(ch);

            printf("Message accepted:\n");
            for (int i = 0; i < k; i++)
                printf("%c", ch[i]);
            printf("\n");

            if (k <= 0)
            {
                close(client_fd);
                printf("Connection closed\n");
                client_fd = -1;
                numberOfClients--;
                continue;
            }

            send(client_fd, ch, 80, 0);
            printf("Accepted size: %d bytes\n", k);
        }
    }
}