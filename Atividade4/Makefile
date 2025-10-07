CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

all: cliente servidor

cliente: client_http.c
	$(CC) $(CFLAGS) -o client_http client_http.c

servidor: server_http.c
	$(CC) $(CFLAGS) -o server_http server_http.c

clean:
	rm client_http server_http