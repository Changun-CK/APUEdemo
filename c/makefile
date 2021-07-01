
CC = g++
FLAG = -g -O2 -W
#FLAG = -O2

all: server client epollOneShot sigio atomic

server:server.c
	gcc $(FLAG) -o server server.c -lpthread
	cp server ../bin/.

client:client.c
	gcc $(FLAG) -o client client.c
	cp client ../bin/.

epollOneShot:epollOneShot.c
	gcc $(FLAG) -o epollOneShot epollOneShot.c -lpthread
	cp epollOneShot ../bin/.

sigio:sigio.c
	gcc $(FLAG) -o sigio sigio.c
	cp sigio ../bin/.

atomic:atomic.c
	gcc $(FLAG) -o atomic atomic.c -lpthread
	cp atomic ../bin/.

clean: 
	rm -rf ../bin/* server client epollOneShot sigio atomic
