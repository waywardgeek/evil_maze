maze: maze.c madatabase.c madatabase.h
	gcc -Wall -DDD_DEBUG -g maze.c madatabase.c -o maze -lddutil-dbg

madatabase.c: madatabase.h

madatabase.h: Maze.dd
	datadraw Maze.dd
