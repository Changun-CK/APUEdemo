
CC = g++
FLAGS = -g -W -O2
#FLAG = -O2

all: client selectServer pollServer

client:client.cpp
	$(CC) $(FLAGS) -o client client.cpp

selectServer:selectServer.cpp
	$(CC) $(FLAGS) -o selectServer selectServer.cpp

pollServer:pollServer.cpp
	$(CC) $(FLAGS) -o pollServer pollServer.cpp

clean: 
	rm -f client selectServer pollServer
