CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

all: cliente servidor

cliente: client_http_select.c
	$(CC) $(CFLAGS) -o client_http client_http_select.c

servidor: server_http_select.c
	$(CC) $(CFLAGS) -o server_http server_http_select.c

clean:
	rm client_http server_http