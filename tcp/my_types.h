#ifndef _LEE_MY_TYPES_H
#define _LEE_MY_TYPES_H

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef _LEE_DEBUG
#define _LEE_DEBUG
#define debug(format, args...) fprintf(stderr, format, ##args)
#endif

struct sys_conf {
    char *host;
    char *user;
    char *passwd;
    char *db;
    FILE *log_fs;
    int port;
};

struct sock_token {
    int connection; //socket file descriptor
    struct sockaddr_in *client; //current connected client
    time_t *last_time; //last connect time
    pthread_mutex_t *time_lock; //lock
    pthread_t heartbeat;
};

struct sock_time {
    //pthread_mutex_t *time_lock; //lock
    //time_t *last_time; //last connect time
    //pthread_t *heartbeat;
    struct sock_token *token; //sock_token info
    FILE *log_fs;
};

#endif
