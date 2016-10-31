#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define debug(format, args...) fprintf(stderr, format, ##args)
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    char ch;
    int port = 26666; //server port
    int times = 4;
    char *name = "Unkown";

    opterr = 0;
    while ((ch = getopt(argc, argv, "p:t:u:")) != -1) {
        switch (ch) {
            case 'p':
                port = atoi(optarg);
                break;
            case 't':
                times = atoi(optarg);
                break;
            case 'u':
                name = optarg;
                break;
        }
    }
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock_cli, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        exit(1);
    }

    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    sprintf(sendbuf, "I'm %s\n", name);

    int length;
    send(sock_cli, name, strlen(name), 0);
    length = recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);
    if (length < 0) {
        perror("error");
        exit(1);
    }
    recvbuf[length] = '\0';
    debug("------%s\n", recvbuf);
    int i = 0;
    while (i < times) {
        send(sock_cli, sendbuf, strlen(sendbuf), 0);
        length = recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);
        recvbuf[length] = '\0';

        debug("%d, msg: %s\n", i, recvbuf);
        memset(recvbuf, 0, sizeof(recvbuf));
        sleep(i++);
    }
    strcpy(sendbuf, "exit");
    send(sock_cli, sendbuf, strlen(sendbuf), 0);

    close(sock_cli);
    return 0;
}
