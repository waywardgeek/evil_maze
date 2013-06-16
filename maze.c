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
    maRoom room[numRooms];
    maRoom start, finish;
    maRoom from, to;
    int numDoors = (numRooms-1)*averageDoors;

    // First create a linear maze from start to finish
    start = maRoomCreate(maze);
    maRoomSetStart(start, true);
    room[0] = start;
    for(i = 1; i < numRooms; i++) {
        room[i] = maRoomCreate(maze);
        maDoorCreate(room[i-1], room[i]);
    }
    finish = room[numRooms - 1];
    maRoomSetFinish(finish, true);
    numDoors -= numRooms - 1;
    // Now add remaining doors randomly, but not from finish.
    while(numDoors != 0) {
        from = room[rand() % (numRooms - 1)];
        to = room[rand() % numRooms];
        maDoorCreate(from, to);
        numDoors--;
    }
    maMazeSetStartRoom(maze, start);
    maMazeSetFinishRoom(maze, finish);
    return maze;
}

// Find a non banned outgoing door.
static maDoor findNonBannedDoor(maRoom room)
{
    maDoor door;

    maForeachRoomOutDoor(room, door) {
        if(!maDoorBanned(door)) {
            return door;
        }
    } maEndRoomOutDoor;
    return maDoorNull;
}

// Backup to the previous room by following path pointers.
static maRoom backup(
    maRoom targetRoom)
{
    maRoom room = targetRoom;
    maDoor door;

    do {
        door = maRoomGetNextPathDoor(room);
        room = maDoorGetToRoom(door);
    } while(room != targetRoom);
    maDoorSetBanned(door, true);
    room = maDoorGetFromRoom(door);
    printf("Banned door %u in room %u\n", maDoor2Index(door),
        maRoom2Index(room));
    return room;
}

// Solve the maze problem.  We keep a running count of the number of doors we've been
// through, and when exploring we write that number on each door we go through.  If we
// come to a room we've been before, we back up by following highest numbers until we see
// the door just before our current count.  We mark that door banned.  The idea is that a
// banned door need not be part of a valid path from start to finish.  If we are in a room
// now with only banned doors, back up again and mark that door banned.  When we back up
// into a room that has a non-banned door, go back to explore mode, but only go through
// non-banned doors.  In general, marking doors as banned breaks loops.
void solveMaze(maMaze maze)
{
    maRoom currentRoom = maMazeGetStartRoom(maze);
    maRoom nextRoom;
    maRoom finish = maMazeGetFinishRoom(maze);
    maDoor door;

    maRoomSetInPath(currentRoom, true);
    while(currentRoom != finish) {
        // We are in a room at the tip of the path... find a door
        door = findNonBannedDoor(currentRoom);
        if(door == maDoorNull) {
            // All exits are banned.  Back up and ban the door we used to get here.
            maRoomSetInPath(currentRoom, false);
            currentRoom = backup(currentRoom);
        } else {
            // Explore through the non-banned door.
            maRoomSetNextPathDoor(currentRoom, door);
            nextRoom = maDoorGetToRoom(door);
            printf("Moved through door %u from room %u to %u\n",
                maDoor2Index(door), maRoom2Index(currentRoom), maRoom2Index(nextRoom));
            currentRoom = nextRoom;
            if(maRoomInPath(currentRoom)) {
                // We've looped back on our exploration path.  Mark the door we took banned
                currentRoom = backup(currentRoom);
            } else {
                maRoomSetInPath(currentRoom, true);
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
