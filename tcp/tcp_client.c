#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "my_protocal.h"

#define debug(format, args...) fprintf(stderr, format, ##args)
#define PORT 26666
#define BUFFER_SIZE 1024

int main()
{
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock_cli, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        exit(1);
    }

    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];

    /*
     * tcp connect, login
     */
    sprintf(sendbuf, "%c:%s", USER_ENTER, "light");
    //strcpy(sendbuf, "i:light");
    send(sock_cli, sendbuf, strlen(sendbuf), 0);
    recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);
    int i = 0;
    while (i < 8) {
        send(sock_cli, sendbuf, strlen(sendbuf), 0);
        //recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);

        debug("%d, msg: %s\n", i,  recvbuf);
        //printf("%d, msg: %s\n", i, recvbuf);
        //memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
        sleep(2);
        ++i;
    }
    i = 2;
    sprintf(sendbuf, "%c:%s", USER_REQUEST, "light");
    send(sock_cli, sendbuf, strlen(sendbuf), 0);
    recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);

    debug("%d, msg: %s\n", i,  recvbuf);
    memset(recvbuf, 0, sizeof(recvbuf));
    sleep(3);

    sprintf(sendbuf, "%c:%s", USER_LEFT, "light");
    send(sock_cli, sendbuf, strlen(sendbuf), 0);
    recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);
    debug("%d, msg: %s\n", i,  recvbuf);
    printf("%d, msg: %s\n", i, recvbuf);

    close(sock_cli);
    return 0;
}
