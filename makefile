CC = gcc
CFLAGS = -g -Wall -Werror
HEADERS = -pthread

all: server client

server: Server.c
	$(CC) $(CFLAGS) -o server Server.c $(HEADERS)

client: Client.c
	$(CC) $(CFLAGS) -o client Client.c $(HEADERS)

clean:
	rm -f server client

fresh:
	clean all