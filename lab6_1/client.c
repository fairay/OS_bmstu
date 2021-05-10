#include "header.h"

#define handle_error(val, msg) \
    { if (val < 0) \
    { perror(msg); return (EXIT_FAILURE); }}

int main()
{
    char buf[BUF_SIZE];
    int sock;
    struct sockaddr srvr_name;

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    handle_error(sock, "socket failed");

    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCK_NAME);

    strcpy(buf, "Hello, Unix sockets!");
    sendto(sock, buf, strlen(buf), 0, &srvr_name,
            strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family));
    
    printf("Message sent \n");
    close(sock);
    return 0;
}
