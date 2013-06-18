// Solve the age old one way door problem... hopefully!
#include <stdlib.h>
#include <stdio.h>
#include "madatabase.h"

// Create a new room.
maRoom maRoomCreate(maMaze maze)
{
    maRoom room = maRoomAlloc();

    maRoomSetMinValue(room, UINT64_MAX);
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
    maLabel label = maLabelAlloc();

    maLabelSetValue(label, value);
    maDoorAppendLabel(door, label);
    if(value < maRoomGetMinValue(room)) {
        maRoomSetMinValue(room, value);
    }
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
        if(maDoorGetFirstLabel(door) == maLabelNull) {
            return door;
        }
    } maEndRoomOutDoor;
    return maDoorNull;
}

// Find the label with the smallest value.
static maLabel findSmallestLabel(
    maRoom room)
{
    maDoor door;
    maLabel label, smallestLabel = maLabelNull;
    uint64 smallestValue = UINT64_MAX;

    maForeachRoomOutDoor(room, door) {
        label = maDoorGetFirstLabel(door); // Smallest label on this door
        if(label != maLabelNull && maLabelGetValue(label) < smallestValue) {
            smallestValue = maLabelGetValue(label);
            smallestLabel = label;
        }
    } maEndRoomOutDoor;
    return smallestLabel;
}

// Follow skipTo values to skip labels.  If there's more than 1 in a row, update the
// skipTo values to point to the last one.
static maLabel skipLabels(
    maRoom room,
    uint32 minLabelValue,
    uint32 maxLabelValue)
{
    maDoor door;
    maLabel minLabel = maLabelNull;
    maLabel label;
    uint64 minValue = UINT64_MAX;
    uint64 value;

    maForeachRoomOutDoor(room, door) {
        maSafeForeachDoorLabel(door, label) {
            value = maLabelGetValue(label);
            if(value >= minLabelValue) {
                if(value < maxLabelValue) {
                    printf("Destroying label %llu\n", maLabelGetValue(label));
                    maLabelDestroy(label);
                } else if(value < minValue) {
                    minValue = value;
                    minLabel = label;
                }
            }
        } maEndSafeDoorLabel;
    } maEndRoomOutDoor;
    return minLabel;
}

// Solve the maze problem.
void solveMaze(maMaze maze)
{
    maRoom currentRoom = maMazeGetStartRoom(maze);
    maRoom nextRoom;
    maRoom finish = maMazeGetFinishRoom(maze);
    maDoor door;
    maLabel label;
    uint64 count = 0;
    uint64 followValue = 0;
    uint64 minValue = 0;
    bool following = false;

    while(currentRoom != finish) {
        count++;
        door = findUnexploredDoor(currentRoom);
        if(door != maDoorNull) {
            following = false;
            followValue = 0;
            minValue = 0;
            nextRoom = maDoorGetToRoom(door);
            printf("%llu Exploring door %u from room %u to %u\n", count,
                maDoor2Index(door), maRoom2Index(currentRoom), maRoom2Index(nextRoom));
        } else {
            if(!following) {
                following = true;
                minValue = maRoomGetMinValue(currentRoom);
                label = findSmallestLabel(currentRoom);
            } else {
                label = skipLabels(currentRoom, minValue, followValue);
            }
            followValue = maLabelGetValue(label) + 1;
            door = maLabelGetDoor(label);
            nextRoom = maDoorGetToRoom(door);
            printf("%llu Following door %u from room %u to %u with label %llu\n", count,
                maDoor2Index(door), maRoom2Index(currentRoom), maRoom2Index(nextRoom),
                maLabelGetValue(label));
        }
        maLabelCreate(door, count);
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
