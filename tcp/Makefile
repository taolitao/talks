all: server client
.PHONY: all

server: tcp_server.o sock_thread.o m_log.o
	-rm server.log
	touch server.log
	$(CC) $^ -o $@ -lpthread -g

client: tcp_client.c
	$(CC) tcp_client.c -o client


.PHONY: cleanall
cleanall:
	-rm server *.o *.log client *.d

sources = tcp_server.c sock_thread.c m_log.c

include $(sources:.c=.d)

%.d: %.c
	set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : , g' < $@.$$$$ > $@; \
		rm -f $@.$$$$
