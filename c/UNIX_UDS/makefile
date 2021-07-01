
CC = g++
FLAG = -g
#FLAG = -O2

all: libUNIX_Domain_Socket.a server client

libUNIX_Domain_Socket.a:UNIX_Domain_Socket.c
	gcc -c -o libUNIX_Domain_Socket.a UNIX_Domain_Socket.c

server:server.c
	gcc $(FLAG) -o server server.c libUNIX_Domain_Socket.a

client:client.c
	gcc $(FLAG) -o client client.c libUNIX_Domain_Socket.a

clean: 
	rm -rf server client *.a *.socket
