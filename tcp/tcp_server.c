#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <error.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include "my_types.h"
#include "sock_thread.h"
#include "m_log.h"

FILE *LogFp;
sem_t GroupSize;

int main(int argc, char *argv[])
{
    int ch;
    char *log_file = "server.log";
    int port = 26666; //server port
    int size = 3;

    opterr = 0;
    while ((ch = getopt(argc, argv, "p:l:s:")) != -1) {
        switch (ch) {
            case 'l':
                log_file = optarg;
                break;
            case 's':
                size = atoi(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                break;
        }
    }
    LogFp = fopen(log_file, "a+");
    sem_init(&GroupSize, 0, size);

    int sock_descriptor;
    //extern int errno;
    sock_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    int on, ret;
    on = 1;
    ret = setsockopt(sock_descriptor, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_descriptor, (struct sockaddr*)&server, sizeof(server)) == -1) {
        perror("bind failed");
        exit(1);
    }
    if (listen(sock_descriptor, 20) == -1) {
        perror("listen failed");
        exit(1);
    }

    struct sockaddr_in *client;
    socklen_t len = sizeof(*client);

    while (1) {
        client = Malloc(client, 1);

        int connection = accept(sock_descriptor, (struct sockaddr*)client, &len);
        if (connection < 0) {
            perror("accept error\n");
            Free(client);
            continue;
        }
        thread_dispatcher(connection, client);
    }

    fclose(LogFp);
    close(sock_descriptor);
    return 0;
}
