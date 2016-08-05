#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <error.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define PORT 26666
#define debug(format, args...) fprintf(stderr, format, ##args)

//static pthread_t thread;
struct sock_token {
    int connection;
    struct sockaddr_in *client;
};
struct sock_time {
    pthread_mutex_t *time_lock;
    time_t *last_time;
};

//static pthread_mutex_t mut;
void create_thread(int connection, struct sockaddr_in *client);
void *tcpThread(void *arg);
void *timeThread(void *arg);

int main(int argc, char *argv[]) {

    int sock_descriptor;
    extern int errno;
    sock_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    int on, ret;
    on = 1;
    ret = setsockopt(sock_descriptor, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_descriptor, (struct sockaddr*)&server, sizeof(server)) == -1) {
        perror("bind");
        exit(1);
    }
    if (listen(sock_descriptor, 20) == -1) {
        perror("listen");
        exit(1);
    }

    //char buff[1024];
    struct sockaddr_in *client;
    socklen_t len = sizeof(*client);


    while (1) {
        client = malloc(sizeof(struct sockaddr_in));
        int connection = accept(sock_descriptor, (struct sockaddr*)client, &len);
        if (connection < 0) {
            perror("accept");
            free(client);
            //exit(1);
            continue;
        }
        create_thread(connection, client);
        /*
        memset(buff, 0, sizeof(buff));
        int length = recv(connection, buff, sizeof(buff), 0);
        if (strcmp(buff, "exit\n") == 0) break;
        printf("from: %s", inet_ntoa(client->sin_addr));
        fputs(buff, stdout);
        send(connection, buff, length, 0);
        close(connection);
        */
    }
    close(sock_descriptor);
    return 0;
}

void create_thread(int connection, struct sockaddr_in *client)
{
    debug("create_thread\n");
    pthread_t myThread;
    //debug("1\n");
    memset(&myThread, 0, sizeof(myThread));
    //debug("2\n");
    struct sock_token *token = malloc(sizeof(struct sock_token));
    //debug("3\n");
    token->connection = connection;
    //debug("4\n");
    token->client = client;
    //debug("5\n");
    if (pthread_create(&myThread, NULL, tcpThread, (void *)token) != 0) {
        //debug("6\n");
        perror("create failed");
        free(client);
        free(token);
        client = NULL;
        token = NULL;
        close(connection);
        return;
    }
    //debug("7\n");
}

void *tcpThread(void *arg)
{
    time_t *last_time = malloc(sizeof(time_t));
    *last_time = time(0);

    debug("tcpThread\n");
    struct sock_token *token = (struct sock_token *)arg;
    char buff[1024];
    int length;
    pthread_mutex_t *time_lock = malloc(sizeof(pthread_mutex_t));

    pthread_t myThread;
    memset(&myThread, 0, sizeof(myThread));
    struct sock_time *sock_connection_time;
    sock_connection_time = malloc(sizeof(struct sock_time));
    sock_connection_time->last_time = last_time;
    sock_connection_time->time_lock = time_lock;
    if (pthread_create(&myThread, NULL, timeThread, (void *)sock_connection_time) != 0) {
        perror("create failed");
        close(token->connection);
        free(token->client);
        token->client = NULL;
        free(token);
        token = NULL;
        pthread_exit(NULL);
        debug("tcpThread exit\n");
        return NULL;
    }

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

void *timeThread(void *arg)
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
