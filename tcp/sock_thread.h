#ifndef _LEE_SOCK_THREAD_H
#define _LEE_SOCK_THREAD_H


//int EXPIRATION = 5;

void getInfoAndCreateThread(int connection, struct sockaddr_in *client, struct sys_conf *conf); //for every connected client

void *heartbeatThread(void *arg); //every connected client, transfer tcp data

void *timerThread(void *arg); //test if tcp is unconnected, and if so, free it

char *getInfo(char *buff, char *type);

#endif
