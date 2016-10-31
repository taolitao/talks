#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "m_log.h"

#define PORT 26666
#define BUFFER_SIZE 1024

void input()
{
}

void show_msg()
{
}

int main(int argc, char *argv[])
{
    char ch;
    int port = PORT;
    char *address = "127.0.0.1";
    char *name = "Unkown";
    opterr = 0;
    while ((ch = getopt(argc, argv, "p:u:s:")) != -1) {
        switch (ch) {
            case 'u':
                name = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                address = optarg;
                break;
            default:
                break;
        }
    }
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(address);

    if (connect(sock_cli, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        exit(1);
    }

    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];

    sprintf(sendbuf, "%s", argv[1]);
    send(sock_cli, sendbuf, strlen(sendbuf), 0);
    int length = recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);
    int i = 0;
    i = 2;
    send(sock_cli, sendbuf, strlen(sendbuf), 0);
    recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);

    debug("%d, msg: %s\n", i,  recvbuf);
    memset(recvbuf, 0, sizeof(recvbuf));
    sleep(3);

    send(sock_cli, sendbuf, strlen(sendbuf), 0);
    recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);
    debug("%d, msg: %s\n", i,  recvbuf);
    printf("%d, msg: %s\n", i, recvbuf);

    close(sock_cli);
    return 0;
}
