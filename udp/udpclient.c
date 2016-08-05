#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PORT 28888
#define BUFFER_SIZE 1024

int main()
{
    int sock_cli = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("115.28.16.166");

    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];

    /*
     * udp connect, heart beat
     */
    strcpy(sendbuf, "online: light");
    int n;
    while (1) {
        printf("====\n");
        sendto(sock_cli, sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&server, sizeof(server));
        //recv(sock_cli, recvbuf, sizeof(recvbuf) , 0);
        socklen_t len = sizeof(server);
        n = recvfrom(sock_cli, recvbuf, sizeof(recvbuf) , 0, (struct sockaddr*)&server, &len);
        recvbuf[n] = '\0';
        fputs(recvbuf, stdout);

        //memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
        sleep(1);
    }

    close(sock_cli);
    return 0;
}
