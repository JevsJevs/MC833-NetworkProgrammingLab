CC = gcc
CFLAGS = -Wall -g

all: cliente servidor

cliente:
	$(CC) $(CFLAGS) cliente.c -o cliente

servidor:
	$(CC) $(CFLAGS) servidor.c -o servidor

clean:
	rm cliente servidor