#include "header.h"

#define SERV_ADDRESS    "127.0.0.1"
/*
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
*/

int main(void)
{
    struct hostent *server;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];
    char msg[BUF_SIZE];

    printf("Input message: ");
    if (!scanf("%s", msg))
    {
        perror("scaning message failed\n");
        return EXIT_FAILURE;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) 
    {
        perror("socket failed\n");
        return EXIT_FAILURE;
    }

    server = gethostbyname(SERV_ADDRESS);
    if (server == NULL)
    {
        close(sock);
        perror("host not found\n");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    // strncpy((char *)&serv_addr.sin_addr.s_addr,
    //         (char *)server->h_addr,
    //         server->h_length);
    serv_addr.sin_addr = *((struct in_addr*) server->h_addr_list[0]);
    serv_addr.sin_port = htons(SERV_PORT);

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect failed\n");
        return EXIT_FAILURE;
    }


    for (int i=0; i<10; i++)
    {
        sprintf(buf, "Message from %s", msg);
        printf("Sending message\n");
        if (send(sock, buf, strlen(buf), 0) < 0)
        {
            close(sock);
            perror("send failed\n");
            return EXIT_FAILURE;
        }
        
        sleep(2);
    }

    close(sock);
    return 0;
}
