CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq -pthread

all: server client

server: server.cpp implementDB.hpp implementDB.cpp pugixml.hpp pugixml.cpp pugiconfig.hpp threadpool.hpp
	$(CC) $(CFLAGS) -o server server.cpp implementDB.cpp pugixml.cpp $(EXTRAFLAGS)

client: helper.cpp implementDB.hpp implementDB.cpp pugixml.hpp pugixml.cpp pugiconfig.hpp threadpool.hpp
	$(CC) $(CFLAGS) -o client client.cpp implementDB.cpp pugixml.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o test server client

clobber:
	rm -f *~ *.o
