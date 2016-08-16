all: server client
.PHONY: all

server: tcp_server.o sock_thread.o m_log.o my_sql.o
	-rm server.log
	touch server.log
	$(CC) $^ -o $@ -lpthread -lpython2.7

client: tcp_client.c
	$(CC) tcp_client.c -o client


.PHONY: cleanall
clean:
	-rm server *.o *.log client

sources = tcp_server.c sock_thread.c m_log.c my_sql.c

override CFLAGS += -I/usr/include/python2.7

include $(sources:.c=.d)

%.d: %.c
	set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : , g' < $@.$$$$ > $@; \
		rm -f $@.$$$$
