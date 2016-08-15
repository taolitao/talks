#ifndef _LEE_MY_TYPES_H
#define _LEE_MY_TYPES_H


struct sys_conf {
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
