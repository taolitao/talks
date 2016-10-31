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
#include <semaphore.h>
#include "my_types.h"
#include "sock_thread.h"
#include "m_log.h"
extern sem_t GroupSize;

static struct msg_list *Msg = NULL;
static pthread_mutex_t MsgMutex = PTHREAD_MUTEX_INITIALIZER;

static void broadcast(const char *name, const char *msg)
{
    pthread_mutex_lock(&MsgMutex);
    struct msg_list *tmp;
    char buff[1024];
    sprintf(buff, "%s: %s", name, msg);
    tmp = Msg;
    while (tmp) {
        if (tmp->token) {
            send(tmp->token->connection, buff, strlen(buff), 0);
        }
        tmp = tmp->next;
    }
    pthread_mutex_unlock(&MsgMutex);
}

static void append(struct sock_token *token)
{
    pthread_mutex_lock(&MsgMutex);
    if (Msg == NULL) {
        Msg = Malloc(Msg, 1);
        Msg->token = token;
        Msg->next = NULL;
        pthread_mutex_unlock(&MsgMutex);
        return;
    }
    struct msg_list *tmp;
    tmp = Msg;
    while (tmp) {
        if (tmp->token == NULL) break;
        tmp = tmp->next;
    }
    if (tmp)
        tmp->token = token;
    else {
        tmp = Malloc(tmp, 1);
        tmp->token = token;
        tmp->next = Msg;
        Msg = tmp;
    }
    pthread_mutex_unlock(&MsgMutex);
}

static void moveout(struct sock_token *token)
{
    pthread_mutex_lock(&MsgMutex);
    struct msg_list *tmp;
    tmp = Msg;
    while (tmp) {
        if (tmp->token == token) {
            tmp->token = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&MsgMutex);
}

static void *gc_thread(void *arg)
{
    struct sock_token *token = (struct sock_token *)arg;
    moveout(token);
    sem_wait(token->done);
    sem_close(token->done);
    Free(token->client);
    Free(token->time_lock);
    Free(token->last_time);
    Free(token->done);
    Free(token);
    sem_post(&GroupSize);
}

void thread_dispatcher(int connection, struct sockaddr_in *client)
{
    debug("====thread_dispatcher\n");
    //get name
    char name[20];
    int length;
    length = recv(connection, name, sizeof(name), 0);
    if (length == -1) {
        debug("recv info failed\n");
        return;
    }
    sem_wait(&GroupSize);
    const char *answer = "passed";
    send(connection, answer, strlen(answer), 0);
    name[length] = '\0';
    debug("from %s:%d %s\n", inet_ntoa(client->sin_addr), ntohs(client->sin_port), name);

    //create threads
    debug("=====create threads\n");
    int ret;
    time_t *last_time = Malloc(last_time, 1);

    *last_time = time(0);
    pthread_mutex_t *time_lock = Malloc(time_lock, 1);

    ret = pthread_mutex_init(time_lock, NULL);
    if (ret) {
        debug("pthread_mutex_init failed, error number: %d\n", ret);
        perror(strerror(ret));
        return;
    }

    pthread_attr_t attr;
    pthread_t heartbeat;
    pthread_t timer;
    pthread_t garbage;
    memset(&heartbeat, 0, sizeof(heartbeat));
    memset(&timer, 0, sizeof(timer));
    memset(&garbage, 0, sizeof(garbage));

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //detach thread

    //create thread
    struct sock_token *token = Malloc(token, 1);
    token->connection = connection;
    token->client = client;
    token->last_time = last_time;
    token->time_lock = time_lock;
    sem_init(token->done, 0, 0);
    append(token);
    strcpy(token->user, name);
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
        if (pthread_create(&garbage, &attr, gc_thread, (void *)token) != 0) {
            perror("create garbage failed");
            break;
        }
        pthread_attr_destroy(&attr);
        debug("thread_dispatcher ended\n");
        pthread_exit(NULL);
        return;
    } while (0);

    //error occured
    pthread_attr_destroy(&attr);
    sem_close(token->done);

    Free(token->last_time);
    Free(token->time_lock);
    Free(token->client);
    close(token->connection);
    Free(client);
    Free(token);

    debug("thread_dispatcher ended\n");
    pthread_exit(NULL);
}

void *heartbeat_thread(void *arg)
{
    debug("heartbeat_thread\n");

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
        debug("from: %s:%d\n", inet_ntoa(token->client->sin_addr), ntohs(token->client->sin_port));
        buff[length] = '\0';
        if (strlen(buff) == 0) continue;
        broadcast(token->user, buff);
        if (strcmp(buff, "exit") == 0) break;
    }
    token->heartbeat = 0;

    debug("--6\n");
    debug("tcpThread exit\n");
    pthread_exit(NULL);
    return NULL;
}

void *timer_thread(void *arg)
{
    debug("time_thread\n");
    struct sock_token *token = (struct sock_token *)arg;
    time_t now;
    double diff;
    debug("++1\n");
    do {
        sleep(1);
        now = time(0);
        debug("++2.0\n");
        pthread_mutex_lock(token->time_lock);
        debug("++2.1\n");
        diff = difftime(now, *(token->last_time));
        debug("++2.2\n");
        pthread_mutex_unlock(token->time_lock);
        debug("++3\n");
    } while(diff < 5.0);

    //logout
    debug("logout user is:%s;\n", token->user);

    debug("++4\n");
    if (token->heartbeat) { // still alive
        debug("++5\n");
        if (!pthread_cancel(token->heartbeat)) debug("cancel the heartbeat thread\n");
        else debug("cancel heartbeat thread failed\n");
    }
    debug("++6\n");

    close(token->connection);
    pthread_mutex_destroy(token->time_lock);
    debug("++7\n");
    sem_post(token->done);

    debug("timer_thread ended\n");
    pthread_exit(NULL);
}
