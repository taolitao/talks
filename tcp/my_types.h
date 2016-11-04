#ifndef _LEE_MY_TYPES_H
#define _LEE_MY_TYPES_H

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct sock_token {
    char user[12]; //client user name
    int connection; //socket file descriptor
    struct sockaddr_in *client; //current connected client address info
    time_t *last_time; //last communication time
    pthread_mutex_t *time_lock; //lock
    pthread_t heartbeat;
    sem_t done;
};

#endif
