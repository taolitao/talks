#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <asm/errno.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include "sock_thread.h"
#include "m_log.h"
#include "my_types.h"

extern sem_t GroupSize;
void *gc_thread(void *arg)
{
    struct sock_token *token = (struct sock_token *)arg;
    sem_wait(&(token->done));
    sem_close(&token->done);
    Free(token->client);
    Free(token->time_lock);
    Free(token->last_time);
    Free(token);
}
void *thread_dispatcher(void *arg)
{
    debug("====thread_dispatcher\n");
    struct connection_info *conn_info = (struct connection_info*)arg;
    //get info
    char buff[1024];
    int length;
    length = recv(conn_info->connection, buff, sizeof(buff), 0);
    if (length == -1) {
        debug("recv info failed\n");
        return NULL;
    }
    buff[length] = '\0';
    debug("from %s: %s\n", inet_ntoa(conn_info->client->sin_addr), buff);
    debug("%s: waiting...\n", buff);
    sem_wait(&GroupSize);
    debug("%s: continue...\n", buff);

    //create threads
    debug("=====create threads\n");
    int ret;
    time_t *last_time;
    Malloc(last_time, 1);

    *last_time = time(0);
    pthread_mutex_t *time_lock;
    Malloc(time_lock, 1);

    ret = pthread_mutex_init(time_lock, NULL);
    if (ret) {
        debug("pthread_mutex_init failed, error number: %d\n", ret);
        perror(strerror(ret));
        return NULL;
    }

    pthread_attr_t attr;
    pthread_t heartbeat;
    pthread_t timer;
    pthread_t gc;
    memset(&heartbeat, 0, sizeof(heartbeat));
    memset(&timer, 0, sizeof(timer));

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //detached thread

    struct sock_token *token;
    Malloc(token, 1);
    token->connection = conn_info->connection;
    token->client = conn_info->client;
    token->last_time = last_time;
    token->time_lock = time_lock;
    strcpy(token->user, buff);

    sem_init(&token->done, 0, 0);
    strcpy(buff, "starting...\n");
    send(conn_info->connection, buff, strlen(buff), 0);
    do {
        if (pthread_create(&heartbeat, &attr, heartbeat_thread, (void *)token) != 0) {
            perror("create heartbeat failed");
            break;
        }

        token->heartbeat = heartbeat;
        if (pthread_create(&timer, &attr, timer_thread, (void *)token) != 0) {
            perror("create timer failed");
            break;
        }

        if (pthread_create(&gc, &attr, gc_thread, (void *)token) != 0) {
            perror("create garbage failed");
            break;
        }
        pthread_attr_destroy(&attr);
        debug("thread_dispatcher ended\n");
        Free(conn_info);
        pthread_exit(NULL);
        //return NULL;
    } while (0);

    //error occured
    pthread_attr_destroy(&attr);
    sem_close(&(token->done));

    close(token->connection);
    Free(token->last_time);
    Free(token->time_lock);
    Free(token->client);
    Free(conn_info);
    Free(token);

    debug("thread_dispatcher ended\n");
}

void *heartbeat_thread(void *arg)
{
    debug("tcpThread\n");

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    struct sock_token *token = (struct sock_token *)arg;
    char recvbuff[1024];
    char sendbuff[1024];
    strcpy(sendbuff, "got message\n");
    int length;

    int i = 0;
    while (1) {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        length = recv(token->connection, recvbuff, sizeof(recvbuff), 0);
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        if (length == -1) {
            debug("recv failed\n");
            break;
        }
        recvbuff[length] = '\0';
        pthread_mutex_lock(token->time_lock);
        *(token->last_time) = time(0);
        pthread_mutex_unlock(token->time_lock);
        debug("from: %s:%d, msg:%s\n", inet_ntoa(token->client->sin_addr),
              ntohs(token->client->sin_port), recvbuff);

        send(token->connection, sendbuff, strlen(sendbuff), 0);
        if (strcmp(recvbuff, "exit") == 0) break;
    }

    token->heartbeat = 0;

    debug("heartbeat_thread exit\n");
    pthread_exit(NULL);
    return NULL;
}

void *timer_thread(void *arg)
{
    debug("timer_thread\n");
    struct sock_token *token = (struct sock_token *)arg;
    time_t now;
    double diff;
    do {
        sleep(1);
        now = time(0);
        pthread_mutex_lock(token->time_lock);
        diff = difftime(now, *(token->last_time));
        pthread_mutex_unlock(token->time_lock);
    } while(diff < 5.0);

    if (token->heartbeat) { // still alive
        if (!pthread_cancel(token->heartbeat)) debug("cancel the heartbeat thread\n");
        else debug("cancel heartbeat thread failed\n");
        //free(sock_connection_time->token->heartbeat);
        token->heartbeat = 0;
    }

    close(token->connection);
    pthread_mutex_destroy(token->time_lock);
    debug("-----user: `%s` left\n", token->user);

    sem_post(&GroupSize);
    sem_post(&token->done);
}
