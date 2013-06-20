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

// Create a new path through a door.
maPath maPathCreate(maDoor door, uint64 label, maPath prevPath, maPath nextPath)
{
    maPath path = maPathAlloc();

    maPathSetLabel(path, label);
    maPathSetPrevPath(path, prevPath);
    maPathSetNextPath(path, nextPath);
    if(prevPath != maPathNull) {
        maPathSetNextPath(prevPath, path);
    }
    if(nextPath != maPathNull) {
        maPathSetPrevPath(nextPath, path);
    }
    maDoorAppendPath(door, path);
    return path;
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

// Find the door with the largest label.
static maDoor findLargestLabelDoor(
    maRoom room)
{
    maDoor door, largestDoor = maDoorNull;
    uint64 label, largestLabel = 0;

    maForeachRoomOutDoor(room, door) {
        label = maDoorGetLabel(door);
        if(label > largestLabel) {
            largestLabel = label;
            largestDoor = door;
        }
    } maEndRoomOutDoor;
    return largestDoor;
}

// Delete the loop of labels so we wont follow them any more.
static void deletePathLoop(
    maPath stopPath)
{
    maRoom startRoom = maDoorGetFromRoom(maPathGetDoor(stopPath));
    maPath path = maPathGetPrevPath(stopPath);
    maPath prevPath;

    while(maDoorGetFromRoom(maPathGetDoor(path)) != startRoom) {
        prevPath = maPathGetPrevPath(path);
        printf("Deleting path %u\n", maPath2Index(path));
        maPathDestroy(path);
        path = prevPath;
    }
    prevPath = maPathGetPrevPath(path);
    printf("Deleting path %u\n", maPath2Index(path));
    maPathDestroy(path);
    maPathSetPrevPath(stopPath, prevPath);
    maPathSetNextPath(prevPath, stopPath);
}

// Follow the largest labeled doors in a loop back to this door, building path objects as
// we go.
static void buildLoop(
    maRoom startRoom,
    uint64 label)
{
    maRoom room = startRoom;
    maDoor door;
    maPath prevPath = maPathNull;
    maPath path, firstPath = maPathNull;

    printf("Building loop %llu: ", label);
    do {
        printf(" R%u", maRoom2Index(room));
        door = findLargestLabelDoor(room);
        path = maPathCreate(door, label, prevPath, maPathNull);
        if(firstPath == maPathNull) {
            firstPath = path;
        }
        prevPath = path;
        room = maDoorGetToRoom(door);
    } while(room != startRoom);
    printf("\n");
    // Now close the loop.
    maPathSetNextPath(path, firstPath);
    maPathSetPrevPath(firstPath, path);
}

// Just return the first path we find leaving the room.
static maPath findPathInRoom(maRoom room)
{
    maDoor door;
    maPath path;

    maForeachRoomOutDoor(room, door) {
        path = maDoorGetFirstPath(door);
        if(path != maPathNull) {
            return path;
        }
    } maEndRoomOutDoor;
    return maPathNull;
}

// Find the path in the room with the smallests label.
static maPath findOldestPathInRoom(
    maRoom room)
{
    maDoor door;
    maPath path, oldestPath = maPathNull;
    uint64 smallestLabel = UINT64_MAX;

    maForeachRoomOutDoor(room, door) {
        maForeachDoorPath(door, path) {
            if(maPathGetLabel(path) < smallestLabel) {
                smallestLabel = maPathGetLabel(path);
                oldestPath = path;
            }
        } maEndDoorPath;
    } maEndRoomOutDoor;
    return oldestPath;
}

// Just relable the entire path with the new label.
static void relabelPath(maPath path, uint64 label)
{
    while(maPathGetLabel(path) != label) {
        maPathSetLabel(path, label);
        path = maPathGetNextPath(path);
    }
}

// Splice sourcePath into destPath, and relable all the paths in sourcePath to match
// destPath.
static void splicePathIntoPath(maPath sourcePath, maPath destPath)
{
    maPath prevSourcePath = maPathGetPrevPath(sourcePath);
    maPath prevDestPath = maPathGetPrevPath(destPath);

    printf("Splicing path %llu into %llu in room %u\n", maPathGetLabel(sourcePath),
        maPathGetLabel(destPath), maRoom2Index(maDoorGetFromRoom(maPathGetDoor(destPath))));
    relabelPath(sourcePath, maPathGetLabel(destPath));
    maPathSetNextPath(prevDestPath, sourcePath);
    maPathSetPrevPath(sourcePath, prevDestPath);
    maPathSetNextPath(prevSourcePath, destPath);
    maPathSetPrevPath(destPath, prevSourcePath);
}

// Find all the paths in the room and splice them into one path.  Join the newer paths
// into the oldest one, since it's likely to be larger, and we want to relable the paths
// we splice into it.
static void splicePaths(maRoom room)
{
    maPath oldestPath = findOldestPathInRoom(room);
    maPath path;
    maDoor door;
    uint64 label = maPathGetLabel(oldestPath);

    maForeachRoomOutDoor(room, door) {
        maForeachDoorPath(door, path) {
            if(maPathGetLabel(path) != label) {
                splicePathIntoPath(path, oldestPath);
            }
        } maEndDoorPath;
    } maEndRoomOutDoor;
}

// Solve the maze problem.
void solveMaze(maMaze maze)
{
    maRoom currentRoom = maMazeGetStartRoom(maze);
    maRoom nextRoom;
    maRoom finish = maMazeGetFinishRoom(maze);
    maDoor door;
    maPath path;
    uint64 count = 0;
    uint64 startLabel;

    while(true) {
        // We're in a room with an unexplored door.  Explore through it.
        do {
            door = findUnexploredDoor(currentRoom);
            count++;
            maDoorSetLabel(door, count);
            nextRoom = maDoorGetToRoom(door);
            printf("%llu Exploring door %u from room %u to %u\n", count,
                maDoor2Index(door), maRoom2Index(currentRoom), maRoom2Index(nextRoom));
            maDoorSetExplored(door, true);
            currentRoom = nextRoom;
            door = findLargestLabelDoor(currentRoom);
            if(currentRoom == finish) {
                printf("Found finish!\n");
                return;
            }
        } while(door == maDoorNull);
        // We found an explored room.  Make a loop.
        buildLoop(currentRoom, count);
        // Now we're in a room with a path in it.  Follow a path until we find an
        // unexplored door.  Splice together different paths that we find, and delete
        // loops with no unexplored doors.
        startLabel = count;
        path = findPathInRoom(currentRoom);
        utDo {
            splicePaths(currentRoom);
            door = findUnexploredDoor(currentRoom);
        } utWhile(door == maDoorNull) {
            door = findLargestLabelDoor(currentRoom);
            if(maDoorGetLabel(door) > startLabel) {
                // Leaves this path object intact
                deletePathLoop(path);
            }
            door = maPathGetDoor(path);
            utAssert(maDoorGetFromRoom(door) == currentRoom);
            count++;
            maDoorSetLabel(door, count);
            nextRoom = maDoorGetToRoom(door);
            printf("%llu Following path %llu through door %u from room %u to %u\n", count,
                maPathGetLabel(path), maDoor2Index(door), maRoom2Index(currentRoom),
                maRoom2Index(nextRoom));
            currentRoom = nextRoom;
            path = maPathGetNextPath(path);
        } utRepeat;
    }
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
