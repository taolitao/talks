#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <error.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "sock_thread.h"

#define debug(format, args...) fprintf(stderr, format, ##args)



int main(int argc, char *argv[]) {

    int sock_descriptor;
    extern int errno;
    sock_descriptor = socket(AF_INET, SOCK_STREAM, 0);

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
    if (listen(sock_descriptor, 20) == -1) {
        perror("listen");
        exit(1);
    }

    //char buff[1024];
    struct sockaddr_in *client;
    socklen_t len = sizeof(*client);


    while (1) {
        client = malloc(sizeof(struct sockaddr_in));
        int connection = accept(sock_descriptor, (struct sockaddr*)client, &len);
        if (connection < 0) {
            perror("accept error\n");
            free(client);
            //exit(1);
            continue;
        }
        createThread(connection, client);
    }
    close(sock_descriptor);
    return 0;
}
