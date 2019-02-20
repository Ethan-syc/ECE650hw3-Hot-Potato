all: ringmaster player

ringmaster: ringmaster.cpp utility.cpp ringmaster.h
	g++ -g -o ringmaster ringmaster.cpp utility.cpp
player: player.cpp utility.cpp player.h
	g++ -g -o player player.cpp utility.cpp

.PHONY:
	clean
clean:
	rm -rf *.o ringmaster player
