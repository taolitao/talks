#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
    strcpy(sendbuf, "login: light");
    int i = 4;
    while (i--) {
        send(sock_cli, sendbuf, strlen(sendbuf), 0);
        recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);

        debug("msg: %s\n", recvbuf);
        printf("msg: %s\n", recvbuf);
        //memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
        sleep(1);
    }
    strcpy(sendbuf, "exit");
    send(sock_cli, sendbuf, strlen(sendbuf), 0);
    recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);
    debug("msg: %s\n", recvbuf);
    printf("msg: %s\n", recvbuf);

    close(sock_cli);
    return 0;
}
