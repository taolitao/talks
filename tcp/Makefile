server: tcp_server.o sock_thread.o
	cc tcp_server.o sock_thread.o -o server -lpthread

tcp_server.o: tcp_server.c sock_thread.h
	cc -c tcp_server.c -lpthread

sock_thread.o: sock_thread.c sock_thread.h
	cc -c sock_thread.c -lpthread

.PHONY: clean
clean:
	rm server *.o
