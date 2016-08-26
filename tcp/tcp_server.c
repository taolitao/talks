#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <error.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include "my_types.h"
#include "sock_thread.h"
#include "m_log.h"

#define debug(format, args...) fprintf(stderr, format, ##args)


int main(int argc, char *argv[])
{
    //extern int EXPIRATION;
    int ch;
    struct sys_conf conf;
    conf.host = "127.0.0.1";
    conf.user = "root";
    conf.passwd = "";
    conf.db = "test";
    conf.port = 3306; //sql port
    char *log_file = "server.log";
    int PORT = 26666; //app port

    opterr = 0;
    while ((ch = getopt(argc, argv, "h:u:w:p:d:l:t:e:")) != -1) {
        switch (ch) {
            case 'h':
                conf.host = optarg;
                break;
            case 'u':
                conf.user = optarg;
                break;
            case 'w':
                conf.passwd = optarg;
                break;
            case 'd':
                conf.db = optarg;
                break;
            case 'l':
                log_file = optarg;
                break;
            case 't':
                conf.port = atoi(optarg);
                break;
            case 'p':
                PORT = atoi(optarg);
                break;
            default:
                break;
        }
    }
    conf.log_fs = fopen(log_file, "a+");

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

    /*
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    */

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
        client = Malloc(client, conf.log_fs, 1);
        //m_log("client", sizeof(struct sockaddr_in), log_fs, 1);

        int connection = accept(sock_descriptor, (struct sockaddr*)client, &len);
        if (connection < 0) {
            perror("accept error\n");
            //m_log("client", sizeof(struct sockaddr_in), log_fs, 0);
            //free(client);
            Free(client, conf.log_fs);
            continue;
        }
        getInfoAndCreateThread(connection, client, &conf);
    }

    fclose(conf.log_fs);
    close(sock_descriptor);
    return 0;
}
