#ifndef _LEE_SOCK_THREAD_H
#define _LEE_SOCK_THREAD_H

void thread_dispatcher(int connection, struct sockaddr_in *client); //for every connected client

void *heartbeat_thread(void *arg); //every connected client, transfer tcp data

void *timer_thread(void *arg); //test if tcp is unconnected, and if so, free it

#endif
