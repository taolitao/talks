server: tcp_server.o sock_thread.o m_log.o
	-rm server.log
	touch server.log
	cc tcp_server.o sock_thread.o m_log.o -o server -lpthread

tcp_server.o: tcp_server.c sock_thread.h m_log.h
	cc -c tcp_server.c -lpthread

sock_thread.o: sock_thread.c sock_thread.h m_log.h
	cc -c sock_thread.c -lpthread

m_log.o: m_log.c m_log.h
	cc -c m_log.c

client: tcp_client.c
	cc tcp_client.c -o client


.PHONY: clean
clean:
	-rm server *.o *.log
