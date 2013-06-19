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

// Create a new label on a door.
maLabel maLabelCreate(
    maDoor door,
    uint64 value)
{
    maRoom room = maDoorGetFromRoom(door);
    maMaze maze = maRoomGetMaze(room);
    maLabel label = maLabelAlloc();

    maLabelSetValue(label, value);
    maDoorAppendLabel(door, label);
    maMazeAppendLabel(maze, label);
    return label;
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
        to = rooms[rand() % (numRooms - 1)];
        if(maRoom2Index(from) < maRoom2Index(to)) {
            maDoorCreate(to, from);
        } else {
            maDoorCreate(from, to);
        }
        numDoors--;
    }
    maMazeSetStartRoom(maze, start);
    maMazeSetFinishRoom(maze, finish);
    utFree(rooms);
    return maze;
}

// Find an unexplored door if one exists.
static maDoor findUnexploredDoor(
    maRoom room)
{
    maDoor door;

    maForeachRoomOutDoor(room, door) {
        if(!maDoorExplored(door)) {
            return door;
        }
    } maEndRoomOutDoor;
    return maDoorNull;
}

// Find the label with the smallest value.
static maLabel findLargestLabel(
    maRoom room)
{
    maDoor door;
    maLabel label, largestLabel = maLabelNull;
    uint64 largestValue = 0;

    maForeachRoomOutDoor(room, door) {
        label = maDoorGetLastLabel(door); // Biggest label on this door
        if(label != maLabelNull && maLabelGetValue(label) > largestValue) {
            largestValue = maLabelGetValue(label);
            largestLabel = label;
        }
    } maEndRoomOutDoor;
    return largestLabel;
}

// Delete the loop of labels so we wont follow them any more.
static void deleteLoop(
    maLabel startLabel)
{
    maLabel label, nextLabel;

    for(label = startLabel; label != maLabelNull;
            label = nextLabel) {
        nextLabel = maLabelGetNextMazeLabel(label);
        printf("Deleting label %llu\n", maLabelGetValue(label));
        maLabelDestroy(label);
    }
}

// Solve the maze problem.
void solveMaze(maMaze maze)
{
    maRoom currentRoom = maMazeGetStartRoom(maze);
    maRoom nextRoom;
    maRoom finish = maMazeGetFinishRoom(maze);
    maRoom startRoom;
    maDoor door;
    maLabel currentLabel, startLabel, stopLabel;
    uint64 count = 0;
    bool following = false;

    while(currentRoom != finish) {
        count++;
        door = findUnexploredDoor(currentRoom);
        if(door != maDoorNull) {
            following = false;
            currentLabel = maLabelNull;
            startLabel = maLabelNull;
            nextRoom = maDoorGetToRoom(door);
            maDoorSetExplored(door, true);
            printf("%llu Exploring door %u from room %u to %u\n", count,
                maDoor2Index(door), maRoom2Index(currentRoom), maRoom2Index(nextRoom));
        } else {
            if(!following) {
                following = true;
                currentLabel = findLargestLabel(currentRoom);
                startLabel = currentLabel;
                stopLabel = maMazeGetLastLabel(maze);
                startRoom = currentRoom;
            } else {
                currentLabel = maLabelGetNextMazeLabel(currentLabel);
                if(currentLabel == stopLabel) {
                    deleteLoop(startLabel);
                    currentLabel = findLargestLabel(currentRoom);
                    startLabel = currentLabel;
                    startRoom = currentRoom;
                }
            }
            door = maLabelGetDoor(currentLabel);
            nextRoom = maDoorGetToRoom(door);
            printf("%llu Following door %u from room %u to %u with label %llu\n", count,
                maDoor2Index(door), maRoom2Index(currentRoom), maRoom2Index(nextRoom),
                maLabelGetValue(currentLabel));
        }
        if(currentRoom != nextRoom) {
            // Avoid making loop back to same room
            maLabelCreate(door, count);
        }
        currentRoom = nextRoom;
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
