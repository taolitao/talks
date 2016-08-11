#ifndef _LEE_SOCK_THREAD_H
#define _LEE_SOCK_THREAD_H

#ifndef PORT
#define PORT 26666
#endif

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

void getInfoAndCreateThread(int connection, struct sockaddr_in *client, FILE *log_fs); //for every connected client


void *heartbeatThread(void *arg); //every connected client, transfer tcp data


void *timerThread(void *arg); //test if tcp is unconnected, and if so, free it

#endif
