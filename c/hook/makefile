GC=gcc
GXX=g++

all: libfilter.so server client libfilterNet.so clientNet

libfilter.so:filter.cpp filter.h
	$(GXX) -fPIC -shared -o libfilter.so filter.cpp -ldl

libfilterNet.so:filterNet.cpp filterNet.h
	$(GXX) -fPIC -shared -o libfilterNet.so filterNet.cpp -ldl

server:server.cpp
	$(GXX) -g -o server server.cpp -L./ -lfilter -ldl -lpthread

client:client.cpp
	$(GXX) -g -o client client.cpp -L./ -lfilter -ldl

clientNet:clientNet.cpp
	$(GXX) -g -o clientNet clientNet.cpp -L./ -lfilterNet -ldl

clean:
	rm -f *.so server client clientNet
