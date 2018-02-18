#Makefile
all: beacon_flood

beacon_flood: beacon_flood.o
	g++ -o beacon_flood beacon_flood.o -O3 -std=c++11 -lpthread -ltins 

beacon_flood.o: beacon_flood.cpp
	g++ -c -o beacon_flood.o beacon_flood.cpp -O3 -std=c++11 -lpthread -ltins

clean:
	rm -f beacon_flood
	rm -f *.o
