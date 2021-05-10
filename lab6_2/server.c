#include "header.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#define CLIENT_N    5

/*
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
*/

int* empty_socket(int *client_sock)
{
    for (int i=0; i<CLIENT_N; i++)
    {
        if (!client_sock[i])
            return &client_sock[i];
    }
    return NULL;
}

int new_client(int sock, int* new_ptr)
{
    struct sockaddr_in cli_addr;
    int clen = sizeof(cli_addr);
    
    int new_sock = accept(sock, (struct sockaddr*) &cli_addr, &clen);
    if (new_sock < 0)
        return EXIT_FAILURE;
    
    *new_ptr = new_sock;
    return EXIT_SUCCESS;
}

int recv_client(int* sock_ptr)
{
    char buf[BUF_SIZE];
    struct sockaddr_in cli_addr;
    int clen = sizeof(cli_addr);

    int bytes = recvfrom(*sock_ptr, buf, sizeof(buf), 0, (struct sockaddr*) &cli_addr, &clen);
    if (bytes <= 0)
    {
        printf("!!! %ud\n", cli_addr.sin_port);
        close(*sock_ptr);
        *sock_ptr = 0;
        return EXIT_FAILURE;
    }

    buf[bytes] = '\0';
    printf("Server received: %s\n", buf);
    return EXIT_SUCCESS;
}

int main(void)
{
    struct sockaddr_in serv_addr;
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (sock < 0) 
    {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERV_PORT);
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        close(sock);
        perror("bind failed\n");
        return EXIT_FAILURE;
    }

    if (listen(sock, 3))
    {
        close(sock);
        perror("listen failed\n");
        return EXIT_FAILURE;
    }

    int client_sock[CLIENT_N];
    for (int i=0; i<CLIENT_N; i++)
        client_sock[i] = 0;

    printf("Socket linked, server listening\n");
    fd_set sock_set;
    int max_sock;

    while (1)
    {
        struct timeval interval = {10, 0};
        max_sock = sock;
        FD_ZERO(&sock_set);

        FD_SET(sock, &sock_set);
        for (int i=0; i<CLIENT_N; i++)
        {
            if (client_sock[i])
            {
                FD_SET(client_sock[i], &sock_set);
                max_sock = MAX(max_sock, client_sock[i]);
            }
        }

        int code = select(max_sock+1, &sock_set, NULL, NULL, &interval);
        if (code == 0)
        {
            close(sock);
            printf("server closed\n");
            return 0;
        }
        else if (code < 0)
        {
            close(sock);
            perror("select failed\n");
            return EXIT_FAILURE;
        }

        if (FD_ISSET(sock, &sock_set))
        {
            if (new_client(sock, empty_socket(client_sock)) == EXIT_FAILURE)
            {
                close(sock);
                perror("aceept failed\n");
                return EXIT_FAILURE;
            }
        }

        for (int i=0; i<CLIENT_N; i++)
        {
            if (client_sock[i] && FD_ISSET(client_sock[i], &sock_set))
                recv_client(&client_sock[i]);
        }
    }
    
    return 0;
}