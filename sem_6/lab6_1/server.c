#include "header.h"
#include <signal.h>

int sock;

void close_socket()
{
    close(sock);
    unlink(SOCK_NAME);
}

void sigint_action(int signum)
{
    printf("\nInterrupt signal received, closing server\n");
    close_socket();
    exit(0);
}

int main() 
{
    char buf[BUF_SIZE];
    struct sockaddr srvr_name;

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0) 
    {
        perror("socket failed\n");
        return EXIT_FAILURE;
    }

    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCK_NAME);

    if (bind(sock, &srvr_name, sizeof(srvr_name)) < 0)
    {
        close(sock);
        perror("bind failed\n");
        return EXIT_FAILURE;
    }

    printf("Socket linked, server listening\n");

    signal(SIGINT, sigint_action);

    while (1)
    {
        int bytes = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
        if (bytes < 0)
        {
            close_socket();
            printf("recvfrom failed, closing server\n");
            return EXIT_FAILURE;
        }

        buf[bytes] = '\0';
        printf("Server received: %s\n", buf);
    }

    return 0;
}