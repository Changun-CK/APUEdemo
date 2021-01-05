
CC = g++
FLAG = -g -O2 -W
#FLAG = -O2

all: selectServer selectClient pollServer webServer epollOneShot sigio

selectServer:selectServer.c
	gcc $(FLAG) -o selectServer selectServer.c
	cp selectServer ../bin/.

selectClient:selectClient.c
	gcc $(FLAG) -o selectClient selectClient.c
	cp selectClient ../bin/.

pollServer:pollServer.c
	gcc $(FLAG) -o pollServer pollServer.c
	cp pollServer ../bin/.

webServer:webServer.c
	gcc $(FLAG) -o webServer webServer.c -lpthread
	cp webServer ../bin/.

epollOneShot:epollOneShot.c
	gcc $(FLAG) -o epollOneShot epollOneShot.c -lpthread
	cp epollOneShot ../bin/.

sigio:sigio.c
	gcc $(FLAG) -o sigio sigio.c
	cp sigio ../bin/.

clean: 
	rm -rf ../bin/* selectServer selectClient pollServer webServer epollOneShot sigio
