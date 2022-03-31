CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq -pthread

all: test server client

test: main.cpp implementDB.hpp implementDB.cpp pugixml.hpp pugixml.cpp pugiconfig.hpp threadpool.hpp
	$(CC) $(CFLAGS) -o test main.cpp implementDB.cpp pugixml.cpp $(EXTRAFLAGS)

server: server.cpp implementDB.hpp implementDB.cpp pugixml.hpp pugixml.cpp pugiconfig.hpp threadpool.hpp
	$(CC) $(CFLAGS) -o server server.cpp implementDB.cpp pugixml.cpp $(EXTRAFLAGS)

client: helper.cpp implementDB.hpp implementDB.cpp pugixml.hpp pugixml.cpp pugiconfig.hpp threadpool.hpp
	$(CC) $(CFLAGS) -o client helper.cpp implementDB.cpp pugixml.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o test server client

clobber:
	rm -f *~ *.o
