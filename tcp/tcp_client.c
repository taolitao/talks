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
    char *name = "Anonymous";
    char *server_addr = "127.0.0.1";

    opterr = 0;
    while ((ch = getopt(argc, argv, "s:p:t:u:")) != -1) {
        switch (ch) {
            case 's':
                server_addr = optarg;
                break;
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
    server.sin_addr.s_addr = inet_addr(server_addr);

    if (connect(sock_cli, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        exit(1);
    }

    char sendbuff[BUFFER_SIZE];
    char recvbuff[BUFFER_SIZE];
    sprintf(sendbuff, "I'm %s\n", name);

    int length;
    send(sock_cli, name, strlen(name), 0);
    length = recv(sock_cli, recvbuff, sizeof(recvbuff) , 0);
    if (length < 0) {
        perror("error");
        exit(1);
    }
    recvbuff[length] = '\0';
    debug("------%s\n", recvbuff);
    int i = 0;
    while (i < times) {
        send(sock_cli, sendbuff, strlen(sendbuff), 0);
        length = recv(sock_cli, recvbuff, sizeof(recvbuff) , 0);
        recvbuff[length] = '\0';

        debug("%d, msg: %s\n", i, recvbuff);
        memset(recvbuff, 0, sizeof(recvbuff));
        sleep(i++);
    }
    strcpy(sendbuff, "exit");
    send(sock_cli, sendbuff, strlen(sendbuff), 0);

    close(sock_cli);
    return 0;
}
