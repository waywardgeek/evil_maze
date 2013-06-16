// Solve the age old one way door problem... hopefully!
#include <stdlib.h>
#include <stdio.h>
#include "madatabase.h"

// Create a new room.
maRoom maRoomCreate(maMaze maze)
{
    maRoom room = maRoomAlloc();

    maMazeAppendRoom(maze, room);
    return room;
}

// Create a new Door.
maDoor maDoorCreate(maRoom from, maRoom to)
{
    maDoor door = maDoorAlloc();

    if((rand() % 2) == 1) {
        maRoomAppendOutDoor(from, door);
        maRoomAppendInDoor(to, door);
    } else {
        maRoomInsertOutDoor(from, door);
        maRoomInsertInDoor(to, door);
    }
    return door;
}

// Build a random maze.
maMaze buildMaze(int numRooms, int averageDoors, int seed)
{
    maMaze maze = maMazeAlloc();
    int i;
    maRoom *rooms = utNewA(maRoom, numRooms);
    maRoom start, finish;
    maRoom from, to;
    int numDoors = (numRooms-1)*averageDoors;

    // First create a linear maze from start to finish
    start = maRoomCreate(maze);
    maRoomSetStart(start, true);
    rooms[0] = start;
    for(i = 1; i < numRooms; i++) {
        rooms[i] = maRoomCreate(maze);
        maDoorCreate(rooms[i-1], rooms[i]);
    }
    finish = rooms[numRooms - 1];
    maRoomSetFinish(finish, true);
    numDoors -= numRooms - 1;
    // Now add remaining doors randomly, but not from finish.
    while(numDoors != 0) {
        from = rooms[rand() % (numRooms - 1)];
        to = rooms[rand() % numRooms];
        maDoorCreate(from, to);
        numDoors--;
    }
    maMazeSetStartRoom(maze, start);
    maMazeSetFinishRoom(maze, finish);
    utFree(rooms);
    return maze;
}

// Collapse all the rooms in the loop into this one, by moving their doors here and
// deleting them.
static void collapseLoop(
    maRoom destRoom)
{
    maRoom room = maDoorGetToRoom(maRoomGetFirstOutDoor(destRoom));
    maRoom nextRoom;
    maDoor door;

    printf("Collapsing loop into room %u\n", maRoom2Index(destRoom));
    while(room != destRoom) {
        nextRoom = maDoorGetToRoom(maRoomGetFirstOutDoor(room));
        utDo {
            door = maRoomGetFirstOutDoor(room);
        } utWhile(door != maDoorNull) {
            if(maDoorGetToRoom(door) == destRoom) {
                maDoorDestroy(door);
            } else {
                maRoomRemoveOutDoor(room, door);
                maRoomAppendOutDoor(destRoom, door);
            }
        } utRepeat;
        utDo {
            door = maRoomGetFirstInDoor(room);
        } utWhile(door != maDoorNull) {
            if(maDoorGetFromRoom(door) == destRoom) {
                maDoorDestroy(door);
            } else {
                maRoomRemoveInDoor(room, door);
                maRoomAppendInDoor(destRoom, door);
            }
        } utRepeat;
        printf("Destroying room %u\n", maRoom2Index(room));
        maRoomDestroy(room);
        room = nextRoom;
    }
}

// Solve the maze problem.
void solveMaze(maMaze maze)
{
    maRoom currentRoom = maMazeGetStartRoom(maze);
    maRoom nextRoom;
    maRoom finish = maMazeGetFinishRoom(maze);
    maDoor door;

    while(currentRoom != finish) {
        maRoomSetInPath(currentRoom, true);
        // We are in a room at the tip of the path... find an unexplored door
        door = maRoomGetFirstOutDoor(currentRoom);
        utAssert(door != maDoorNull);
        nextRoom = maDoorGetToRoom(door);
        if(nextRoom == currentRoom) {
            // Destroy self-looping door
            maDoorDestroy(door);
        } else {
            printf("Moved through door %u from room %u to %u\n",
                maDoor2Index(door), maRoom2Index(currentRoom), maRoom2Index(nextRoom));
            currentRoom = nextRoom;
            if(maRoomInPath(currentRoom)) {
                collapseLoop(currentRoom);
            }
        }
    }
    printf("Found finish!\n");
}

// Dump the room
static void printRoom(maRoom room)
{
    maDoor door;

    printf("R%u -> ", maRoom2Index(room));
    maForeachRoomOutDoor(room, door) {
        printf(" R%u", maRoom2Index(maDoorGetToRoom(door)));
    } maEndRoomOutDoor;
    printf("\n");
}

// Dump the maze.
static void printMaze(maMaze maze)
{
    maRoom room;

    maForeachMazeRoom(maze, room) {
        printRoom(room);
    } maEndMazeRoom;
}

int main(int argc, char **argv) {
    int numRooms, averageDoors, seed;
    maMaze maze;

    if(argc != 4) {
        printf("Usage: maze numRooms averageDoors seed\n");
        return 1;
    }
    numRooms = atoi(argv[1]);
    averageDoors = atoi(argv[2]);
    seed = atoi(argv[3]);
    utStart();
    maDatabaseStart();
    maze = buildMaze(numRooms, averageDoors, seed);
    printMaze(maze);
    solveMaze(maze);
    maDatabaseStop();
    utStop(false);
    return 0;
}
