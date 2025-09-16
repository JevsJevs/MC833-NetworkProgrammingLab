CC = gcc
CFLAGS = -Wall -g

all: cliente servidor

cliente: cliente.c
	$(CC) $(CFLAGS) cliente.c -o cliente

servidor: servidor.c
	$(CC) $(CFLAGS) servidor.c -o servidor

clean:
	rm cliente servidor