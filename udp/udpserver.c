#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <error.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define PORT 28888

int main(int argc, char *argv[]) {
    int sock_descriptor;
    extern int errno;
    sock_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

    int on, ret;
    on = 1;
    ret = setsockopt(sock_descriptor, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_descriptor, (struct sockaddr*)&server, sizeof(server)) == -1) {
        perror("bind");
        exit(1);
    }

    char buff[1024];
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int i, n;

    while (1) {
        printf("======\n");
        n = recvfrom(sock_descriptor, buff, 1024, 0, (struct sockaddr*)&client, &len);
        buff[n] = '\0';
        //printf("from: %s -->", inet_ntoa(client.sin_addr));
        printf("msg: %s", buff);
        sendto(sock_descriptor, buff, n, 0, (struct sockaddr*)&client, len);
        memset(buff, 0, sizeof(buff));
    }
    close(sock_descriptor);
    return 0;
}
