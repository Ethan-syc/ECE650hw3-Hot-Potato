all: ringmaster player

ringmaster: ringmaster.cpp utility.cpp
	g++ -o3 -o ringmaster ringmaster.cpp utility.cpp
player: player.cpp utility.cpp player.h
	g++ -o3 -o player player.cpp utility.cpp

.PHONY:
	clean
clean:
	rm -rf *.o ringmaster player
