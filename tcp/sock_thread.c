#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "sock_thread.h"

#define debug(format, args...) fprintf(stderr, format, ##args)

void getInfoAndCreateThread(int connection, struct sockaddr_in *client)
{
    debug("====getInfoAndCreateThread\n");
    //get info
    char buff[1024];
    int length;
    length = recv(connection, buff, sizeof(buff), 0);
    if (length == -1) {
        debug("recv info failed\n");
        return;
    }
    buff[length] = '\0';
    debug("from %s: %s\n", inet_ntoa(client->sin_addr), buff);

    //create threads
    debug("=====create threads\n");
    int ret;
    time_t *last_time = malloc(sizeof(time_t));
    *last_time = time(0);
    pthread_mutex_t *time_lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_t mattr;

    ret = pthread_mutex_init(time_lock, &mattr);
    if (ret) {
        debug("pthread_mutex_init failed\n");
        return;
    }

    pthread_attr_t attr;
    pthread_t heartbeat;
    pthread_t timer;
    memset(&heartbeat, 0, sizeof(heartbeat));
    memset(&timer, 0, sizeof(timer));


    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //detach thread

    //create heartbeat thread
    struct sock_token *token = malloc(sizeof(struct sock_token));
    token->connection = connection;
    token->client = client;
    token->last_time = last_time;
    token->time_lock = time_lock;
    if (pthread_create(&heartbeat, &attr, heartbeatThread, (void *)token) != 0) {
        perror("create failed");
        close(token->connection);
        free(last_time);
        free(time_lock);
        free(token->client);
        free(client);
        free(token);
        client = NULL;
        last_time = NULL;
        time_lock = NULL;
        token = NULL;
        close(connection);
        return;
    }

    //create timer thread
    struct sock_time *sock_connection_time;
    sock_connection_time = malloc(sizeof(struct sock_time));
    //sock_connection_time->last_time = last_time;
    //sock_connection_time->time_lock = time_lock;
    sock_connection_time->token = token;
    if (pthread_create(&timer, &attr, timerThread, (void *)sock_connection_time) != 0) {
        int ret = pthread_cancel(timer);
        perror("create failed");
        close(token->connection);
        free(token->client);
        free(last_time);
        free(time_lock);
        token->client = NULL;
        last_time = NULL;
        time_lock = NULL;
        free(token);
        token = NULL;
        pthread_exit(NULL);
        debug("tcpThread exit\n");
        return;
    }

    pthread_attr_destroy(&attr);

    pthread_exit(NULL);
}

void *heartbeatThread(void *arg)
{
    debug("tcpThread\n");

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    struct sock_token *token = (struct sock_token *)arg;
    char buff[1024];
    int length;

    debug("--1\n");

    while (1) {
        debug("--1.1\n");
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        length = recv(token->connection, buff, sizeof(buff), 0);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        debug("--2\n");
        debug("length is %d\n", length);
        if (length == -1) {
            debug("recv failed\n");
            break;
        }
        debug("--3\n");
        pthread_mutex_lock(token->time_lock);
        *(token->last_time) = time(0);
        pthread_mutex_unlock(token->time_lock);
        //if (strcmp(buff, "exit\n") == 0) break;
        if (length == 0) debug("empty\n");
        debug("--4\n");
        debug("from: %s\n", inet_ntoa(token->client->sin_addr));
        buff[length] = '\0';
        debug("msg: %s\n", buff);
        send(token->connection, buff, length, 0);
        if (strcmp(buff, "exit") == 0) break;
        debug("--5\n");
    }

    //the timethread will free the memory which will never use
    //close(token->connection);
    //free(token->client);
    //token->client = NULL;
    //free(token);
    //token = NULL;

    debug("--6\n");
    debug("tcpThread exit\n");
    pthread_exit(NULL);
    return NULL;
}

void *timerThread(void *arg)
{
    debug("timeThread\n");
    struct sock_time *sock_connection_time = (struct sock_time *)arg;
    time_t now;
    double diff;
    debug("++1\n");
    do {
        sleep(3);
        now = time(0);
        debug("++2\n");
        pthread_mutex_lock(sock_connection_time->token->time_lock);
        diff = difftime(now, *(sock_connection_time->token->last_time));
        pthread_mutex_unlock(sock_connection_time->token->time_lock);
        debug("++3\n");
    } while(diff <= 7.0);

    debug("++4\n");
    int kill_rc = pthread_kill(*(sock_connection_time->heartbeat), 0);
    debug("++4.1\n");
    if (kill_rc == 0) { // still alive
        debug("++5\n");
        if (!pthread_cancel(*(sock_connection_time->heartbeat))) debug("cancel the heartbeat thread\n");
        else debug("cancel heartbeat thread failed\n");
    }
    debug("++6\n");

    //free the memory
    close(sock_connection_time->token->connection);
    free(sock_connection_time->token->client);
    pthread_mutex_destroy(sock_connection_time->token->time_lock);
    free(sock_connection_time->token->time_lock);
    free(sock_connection_time->token->last_time);
    free(sock_connection_time->token);
    free(sock_connection_time->heartbeat);
    sock_connection_time->token->client = NULL;
    sock_connection_time->token->time_lock = NULL;
    sock_connection_time->token->last_time = NULL;
    sock_connection_time->token = NULL;
    sock_connection_time->heartbeat = NULL;

    debug("++6\n");
    debug("timeThread ended\n");
    pthread_exit(NULL);
}
