#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <asm/errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "my_types.h"
#include "sock_thread.h"
#include "m_log.h"

#define debug(format, args...) fprintf(stderr, format, ##args)

void getInfoAndCreateThread(int connection, struct sockaddr_in *client, struct sys_conf *conf)
{
    debug("====getInfoAndCreateThread\n");
    FILE *log_fs = conf->log_fs;
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
    send(connection, buff, strlen(buff), 0);

    //create threads
    debug("=====create threads\n");
    int ret;
    time_t *last_time = Malloc(last_time, log_fs);
    //m_log("last_time", sizeof(time_t), log_fs, 1);

    *last_time = time(0);
    pthread_mutex_t *time_lock = Malloc(time_lock, log_fs);
    //m_log("time_lock", sizeof(pthread_mutex_t), log_fs, 1);

    //pthread_mutexattr_t mattr;

    ret = pthread_mutex_init(time_lock, NULL);
    if (ret) {
        debug("pthread_mutex_init failed, error number: %d\n", ret);
        if (ret == EBUSY) debug("111\n");
        if (ret == EINVAL) debug("222\n");
        if (ret == EFAULT) debug("333\n");
        perror(strerror(ret));
        return;
    }

    pthread_attr_t attr;
    pthread_t heartbeat;
    pthread_t timer;
    //heartbeat = Malloc(sizeof(pthread_t));
    memset(&heartbeat, 0, sizeof(heartbeat));
    memset(&timer, 0, sizeof(timer));


    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //detach thread

    //create heartbeat thread
    struct sock_token *token = Malloc(token, log_fs);
    //m_log("token", sizeof(struct sock_token), log_fs, 1);
    token->connection = connection;
    token->client = client;
    token->last_time = last_time;
    token->time_lock = time_lock;
    if (pthread_create(&heartbeat, &attr, heartbeatThread, (void *)token) != 0) {
        perror("create failed");
        close(token->connection);

        //m_log("last_time", sizeof(*last_time), log_fs, 0);
        //m_log("time_lock", sizeof(*time_lock), log_fs, 0);
        //m_log("token->client", sizeof(*token->client), log_fs, 0);
        //m_log("client", sizeof(*client), log_fs, 0);
        //m_log("token", sizeof(*token), log_fs, 0);
        Free(last_time, log_fs);
        Free(time_lock, log_fs);
        Free(token->client, log_fs);
        Free(client, log_fs);
        Free(token, log_fs);
        client = NULL;

        last_time = NULL;
        time_lock = NULL;
        token = NULL;
        return;
    }

    //create timer thread
    struct sock_time *sock_connection_time;
    sock_connection_time = Malloc(sock_connection_time, log_fs);
    //m_log("sock_connection_time", sizeof(struct sock_time), log_fs, 1);
    //sock_connection_time->last_time = last_time;
    //sock_connection_time->time_lock = time_lock;
    sock_connection_time->token = token;
    sock_connection_time->token->heartbeat = heartbeat;
    sock_connection_time->log_fs = log_fs;
    if (pthread_create(&timer, &attr, timerThread, (void *)sock_connection_time) != 0) {
        int ret = pthread_cancel(timer);
        perror("create failed");
        close(token->connection);

        //m_log("token->client", sizeof(*token->client), log_fs, 0);
        //m_log("last_time", sizeof(*last_time), log_fs, 0);
        //m_log("time_lock", sizeof(*time_lock), log_fs, 0);
        //m_log("token", sizeof(*token), log_fs, 0);
        Free(token->client, log_fs);
        Free(last_time, log_fs);
        Free(time_lock, log_fs);
        Free(token, log_fs);

        token->client = NULL;
        last_time = NULL;
        time_lock = NULL;
        token = NULL;
        pthread_exit(NULL);
        debug("tcpThread exit\n");
        return;
    }

    pthread_attr_destroy(&attr);
    debug("getInfoAndCreateThread ended\n");
    return;
}

void *heartbeatThread(void *arg)
{
    debug("tcpThread\n");

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    struct sock_token *token = (struct sock_token *)arg;
    char buff[1024];
    char sendbuff[1024];
    strcpy(sendbuff, "fresh");
    int length;

    debug("--1\n");

    int i = 0;
    while (1) {
        debug("--1.1\n");
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        send(token->connection, sendbuff, strlen(sendbuff), 0);
        debug("--1.2\n");

        length = recv(token->connection, buff, sizeof(buff), 0);
        //pthread_testcancel();
        //pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
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
        debug("==info %d\n", i++);
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

    //free(token->heartbeat);
    token->heartbeat = 0;
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
    FILE *log_fs = sock_connection_time->log_fs;
    time_t now;
    double diff;
    debug("++1\n");
    do {
        sleep(1);
        now = time(0);
        debug("++2\n");
        pthread_mutex_lock(sock_connection_time->token->time_lock);
        diff = difftime(now, *(sock_connection_time->token->last_time));
        pthread_mutex_unlock(sock_connection_time->token->time_lock);
        debug("++3\n");
    } while(diff < 5.0);

    debug("++4\n");
    if (sock_connection_time->token->heartbeat) { // still alive
        debug("++5\n");
        if (!pthread_cancel(sock_connection_time->token->heartbeat)) debug("cancel the heartbeat thread\n");
        else debug("cancel heartbeat thread failed\n");
        //free(sock_connection_time->token->heartbeat);
        sock_connection_time->token->heartbeat = 0;
    }
    debug("++6\n");

    //free the memory
    close(sock_connection_time->token->connection);
    pthread_mutex_destroy(sock_connection_time->token->time_lock);
    debug("++7\n");

    //m_log("sock_connection_time->token->client", sizeof(*(sock_connection_time->token->client)), log_fs, 0);
    debug("++7.1\n");
    //m_log("sock_connection_time->token->time_lock", sizeof(*(sock_connection_time->token->time_lock)), log_fs, 0);
    debug("++7.2\n");
    //m_log("sock_connection_time->token->last_time", sizeof(*(sock_connection_time->token->last_time)), log_fs, 0);
    debug("++7.3\n");
    //m_log("sock_connection_time->token", sizeof(*(sock_connection_time->token)), log_fs, 0);
    debug("++8\n");
    Free(sock_connection_time->token->client, log_fs);
    Free(sock_connection_time->token->time_lock, log_fs);
    Free(sock_connection_time->token->last_time, log_fs);
    Free(sock_connection_time->token, log_fs);

    debug("++9\n");
    sock_connection_time->token->client = NULL;
    sock_connection_time->token->time_lock = NULL;
    sock_connection_time->token->last_time = NULL;
    sock_connection_time->token = NULL;

    //m_log("sock_connection_time", sizeof(*(sock_connection_time)), log_fs, 0);
    Free(sock_connection_time, log_fs);
    sock_connection_time = NULL;

    debug("++10\n");
    debug("timeThread ended\n");
    pthread_exit(NULL);
}
