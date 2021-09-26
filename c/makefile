CXX = g++
FLAG = -g -O2 -W
#FLAG = -O2

$(shell if [ ! -d "../bin" ]; then mkdir ../bin; fi)

all: server client epollOneShot sigio atomic example

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

example:./threadpool/example.cpp ./threadpool/ThreadPool.h
	$(CXX) $(FLAG) -o example ./threadpool/example.cpp -I=./threadpool/ -lpthread
	cp example ../bin/.

clean: 
	rm -rf ../bin/* server client epollOneShot sigio atomic example
