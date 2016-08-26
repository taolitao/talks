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
#include "my_sql.h"
#include "sock_thread.h"
#include "m_log.h"
#include "my_protocal.h"

#ifndef _LEE_DEBUG
#define debug(format, args...) fprintf(stderr, format, ##args)
#endif


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
    debug("from %s:%d %s\n", inet_ntoa(client->sin_addr), ntohs(client->sin_port), buff);
    char type;
    char *msg = getInfo(buff, &type);
    if (type != USER_ENTER) {
        char *reply = "unknown msg type";
        send(connection, reply, strlen(reply), 0);
        close(connection);
        Free(client, log_fs);
        return;
    } else {
        char *reply = "****************login";
        send(connection, reply, strlen(reply), 0);
        userLogin(inet_ntoa(client->sin_addr), ntohs(client->sin_port), msg, conf);
    }

    //create threads
    debug("=====create threads\n");
    int ret;
    time_t *last_time = Malloc(last_time, log_fs, 1);

    *last_time = time(0);
    pthread_mutex_t *time_lock = Malloc(time_lock, log_fs, 1);

    ret = pthread_mutex_init(time_lock, NULL);
    if (ret) {
        debug("pthread_mutex_init failed, error number: %d\n", ret);
        perror(strerror(ret));
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
    struct sock_token *token = Malloc(token, log_fs, 1);
    token->connection = connection;
    token->client = client;
    token->last_time = last_time;
    token->time_lock = time_lock;
    strcpy(token->user, msg);
    if (pthread_create(&heartbeat, &attr, heartbeatThread, (void *)token) != 0) {
        perror("create failed");
        close(token->connection);

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
    //struct sock_conn *sock_connection;
    //sock_connection = Malloc(sock_connection, log_fs, 1);
    //sock_connection->token = token;
    //sock_connection->token->heartbeat = heartbeat;
    //sock_connection->conf = conf;
    token->heartbeat = heartbeat;
    token->conf = conf;
    if (pthread_create(&timer, &attr, timerThread, (void *)token) != 0) {
        int ret = pthread_cancel(timer);
        perror("create failed");
        close(token->connection);

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
        //send(token->connection, sendbuff, strlen(sendbuff), 0);
        debug("--1.2\n");
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
        debug("==info %d\n", i++);
        //if (strcmp(buff, "exit\n") == 0) break;
        //debug("--4\n");
        debug("from: %s:%d\n", inet_ntoa(token->client->sin_addr), ntohs(token->client->sin_port));
        buff[length] = '\0';
        if (strlen(buff) == 0) continue;
        //debug("msg: %s\n", buff);
        char type;
        char *msg = getInfo(buff, &type);
        if (type == USER_LEFT)
            break;
        else if (type == USER_REQUEST) {
            char info[100];
            getIpAndPort(info, msg, token->conf);
            send(token->connection, info, strlen(info), 0);
        }
        //send(token->connection, buff, length, 0);
        //if (strcmp(buff, "exit") == 0) break;
        //debug("--5\n");
    }

//end:
    //free(token->heartbeat);
    token->heartbeat = 0;
    //the timethread will free the memory which will never use

    debug("--6\n");
    debug("tcpThread exit\n");
    pthread_exit(NULL);
    return NULL;
}

void *timerThread(void *arg)
{
    debug("timeThread\n");
    //extern int EXPIRATION;
    struct sock_token *token = (struct sock_token *)arg;
    FILE *log_fs = token->conf->log_fs;
    time_t now;
    double diff;
    debug("++1\n");
    do {
        sleep(1);
        now = time(0);
        debug("++2\n");
        pthread_mutex_lock(token->time_lock);
        debug("++2.1\n");
        diff = difftime(now, *(token->last_time));
        debug("++2.2\n");
        pthread_mutex_unlock(token->time_lock);
        debug("++3\n");
    } while(diff < 5.0);

    //logout
    debug("********************logout\n");
    debug("user is:%s;\n", token->user);
    userLogout(token->user, token->conf);

    debug("++4\n");
    if (token->heartbeat) { // still alive
        debug("++5\n");
        if (!pthread_cancel(token->heartbeat)) debug("cancel the heartbeat thread\n");
        else debug("cancel heartbeat thread failed\n");
        //free(sock_connection->token->heartbeat);
        token->heartbeat = 0;
    }
    debug("++6\n");

    //free the memory
    close(token->connection);
    pthread_mutex_destroy(token->time_lock);
    debug("++7\n");

    debug("++8\n");
    Free(token->client, log_fs);
    Free(token->time_lock, log_fs);
    Free(token->last_time, log_fs);

    debug("++9\n");
    token->client = NULL;
    token->time_lock = NULL;
    token->last_time = NULL;

    Free(token, log_fs);
    token = NULL;

    debug("++10\n");
    debug("timeThread ended\n");
    pthread_exit(NULL);
}

char *getInfo(char *buff, char *type)
{
    if (strlen(buff) < 2) {
        debug("error getInfo\n");
        exit(1);
    }
    *type = buff[0];
    buff[1] = '\0';
    return buff+2;
}

