CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

all: cliente servidor

cliente: client_http_select.c client_http_poll.c
	$(CC) $(CFLAGS) -o client_http_select client_http_select.c
	$(CC) $(CFLAGS) -o client_http_poll client_http_poll.c

servidor: server_http_select.c server_http_poll.c
	$(CC) $(CFLAGS) -o server_http_select server_http_select.c
	$(CC) $(CFLAGS) -o server_http_poll server_http_poll.c

clean:
	rm client_http_select client_http_poll server_http_select server_http_poll