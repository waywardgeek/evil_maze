all: maze fastmaze

maze: maze.c madatabase.c madatabase.h
	gcc -Wall -DDD_DEBUG -g maze.c madatabase.c -o maze -lddutil-dbg

fastmaze: fastmaze.c madatabase.c madatabase.h
	gcc -Wall -DDD_DEBUG -g fastmaze.c madatabase.c -o fastmaze -lddutil-dbg

madatabase.c: madatabase.h

madatabase.h: Maze.dd
	datadraw Maze.dd
