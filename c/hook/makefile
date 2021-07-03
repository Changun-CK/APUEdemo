GC=gcc
GXX=g++

all: libfilter.so server client

libfilter.so:filter.cpp filter.h
	$(GXX) -fPIC -shared -o libfilter.so filter.cpp -ldl

server:server.cpp
	$(GXX) -g -o server server.cpp -L./ -lfilter -ldl -lpthread

client:client.cpp
	$(GXX) -g -o client client.cpp -L./ -lfilter -ldl

clean:
	rm -f *.so server client
