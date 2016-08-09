#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "sock_thread.h"

#define debug(format, args...) fprintf(stderr, format, ##args)

void createThread(int connection, struct sockaddr_in *client)
{
    debug("createThread\n");
    pthread_attr_t attr;
    pthread_t heartbeat;
    pthread_t timer;
    memset(&heartbeat, 0, sizeof(heartbeat));
    memset(&timer, 0, sizeof(timer));

    struct sock_token *token = malloc(sizeof(struct sock_token));
    token->connection = connection;
    token->client = client;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    //create heartbeat thread
    if (pthread_create(&heartbeat, &attr, heartbeatThread, (void *)token) != 0) {
        perror("create failed");
        free(client);
        free(token);
        client = NULL;
        token = NULL;
        close(connection);
        return;
    }

    //create timer thread
    if (pthread_create(&timer, &attr, timerThread, (void *)sock_connection_time) != 0) {
        perror("create failed");
        close(token->connection);
        free(token->client);
        token->client = NULL;
        free(token);
        token = NULL;
        pthread_exit(NULL);
        debug("tcpThread exit\n");
        return;
    }

}

void *heartbeatThread(void *arg)
{
    time_t *last_time = malloc(sizeof(time_t));
    *last_time = time(0);

    debug("tcpThread\n");
    struct sock_token *token = (struct sock_token *)arg;
    char buff[1024];
    int length;
    pthread_mutex_t *time_lock = malloc(sizeof(pthread_mutex_t));

    struct sock_time *sock_connection_time;
    sock_connection_time = malloc(sizeof(struct sock_time));
    sock_connection_time->last_time = last_time;
    sock_connection_time->time_lock = time_lock;
    

    while ((length = recv(token->connection, buff, sizeof(buff), 0))
            != -1 && length > 0) {
        pthread_mutex_lock(time_lock);
        *last_time = time(0);
        pthread_mutex_unlock(time_lock);
        //if (strcmp(buff, "exit\n") == 0) break;
        if (length == 0) continue;
        debug("from: %s\n", inet_ntoa(token->client->sin_addr));
        buff[length] = '\0';
        debug("msg: %s\n", buff);
        send(token->connection, buff, length, 0);
        if (strcmp(buff, "exit") == 0) break;
    }

    close(token->connection);
    free(token->client);
    token->client = NULL;
    free(token);
    token = NULL;
    pthread_exit(NULL);
    debug("tcpThread exit\n");
    return NULL;
}

void *timerThread(void *arg)
{
    debug("timeThread\n");
    struct sock_time *sock_connection_time = (struct sock_time *)arg;
    time_t now;
    double diff;
    do {
        sleep(5);
        now = time(0);
        pthread_mutex_lock(sock_connection_time->time_lock);
        diff = difftime(now, *(sock_connection_time->last_time));
        pthread_mutex_unlock(sock_connection_time->time_lock);
    } while(diff <= 5.0);

    //TODO: free the memory
    debug("timeThread ended\n");
    pthread_exit(NULL);
}
