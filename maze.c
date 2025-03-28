// Solve the age old one way door problem... hopefully!
#include <stdlib.h>
#include <stdio.h>
#include "madatabase.h"

// Create a new room.
maRoom maRoomCreate(maMaze maze, utSym name)
{
    maRoom room = maRoomAlloc();

    maRoomSetSym(room, name);
    maMazeAppendRoom(maze, room);
    return room;
}

// Create a new Door.
maDoor maDoorCreate(maRoom from, maRoom to, bool append)
{
    maDoor door = maDoorAlloc();

    if(append) {
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

// Build a random maze, unless uniform is true, in which case build a constant number of
// doors in each room leading back to the start.
maMaze buildMaze(int numRooms, int averageDoors, int seed, bool uniform)
{
    maMaze maze = maMazeAlloc();
    uint32 i, j;
    maRoom *rooms = utNewA(maRoom, numRooms);
    maRoom start, finish;
    maRoom from, to;
    int numDoors = (numRooms-1)*averageDoors;

    // First create a linear maze from start to finish
    start = maRoomCreate(maze, utSymCreate("Start"));
    rooms[0] = start;
    for(i = 1; i < numRooms; i++) {
        if(i != numRooms - 1) {
            rooms[i] = maRoomCreate(maze, utSymCreateFormatted("R%u", i));
        } else {
            rooms[i] = maRoomCreate(maze, utSymCreate("Finish"));
        }
        maDoorCreate(rooms[i-1], rooms[i], true);
    }
    finish = rooms[numRooms - 1];
    if(!uniform) {
        numDoors -= numRooms - 1;
        // Now add remaining doors randomly, but not from finish.
        while(numDoors != 0) {
            from = rooms[rand() % (numRooms - 1)];
            to = rooms[rand() % (numRooms - 1)];
            maDoorCreate(to, from, (rand() % 2) == 1);
            numDoors--;
        }
    } else {
        for(i = 0; i < numRooms - 1; i++) {
            for(j = 1; j < averageDoors; j++) {
                maDoorCreate(rooms[i], start, (rand() % 2) == 1);
            }
        }
    }
    maMazeSetStartRoom(maze, start);
    maMazeSetFinishRoom(maze, finish);
    utFree(rooms);
    return maze;
}

// Find an unexplored door if one exists.
static maDoor findUnexploredDoor(maRoom room)
{
    maDoor door;

    maForeachRoomOutDoor(room, door) {
        if(!maDoorExplored(door)) {
            return door;
        }
    } maEndRoomOutDoor;
    return maDoorNull;
}
// Find an unlabeled door if one exists.
static maDoor findUnlabeledDoor(maRoom room)
{
    maDoor door;

    maForeachRoomOutDoor(room, door) {
        if(maDoorGetLabel(door) == 0) {
            return door;
        }
    } maEndRoomOutDoor;
    return maDoorNull;
}


// Find the door with the largest label.
static maDoor findLargestLabelDoor(maRoom room)
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

// Find the door with the smallest label.
static maDoor findSmallestLabelDoor(maRoom room)
{
    maDoor door, smallestDoor = maDoorNull;
    uint64 label, smallestLabel = UINT64_MAX;

    maForeachRoomOutDoor(room, door) {
        label = maDoorGetLabel(door);
        if(label != 0 && label < smallestLabel) {
            smallestLabel = label;
            smallestDoor = door;
        }
    } maEndRoomOutDoor;
    return smallestDoor;
}

// Delete the loop of labels so we wont follow them any more.
static void deletePathLoop(maPath startPath, maPath stopPath)
{
    maPath prevPath = maPathGetPrevPath(startPath);
    maPath path, nextPath;

    for(path = startPath; path != stopPath; path = nextPath) {
        nextPath = maPathGetNextPath(path);
        printf("Deleting path %lu element %u in R%u\n", maPathGetLabel(path),
            maPath2Index(path), maRoom2Index(maDoorGetFromRoom(maPathGetDoor(path))));
        maPathDestroy(path);
    }
    maPathSetPrevPath(stopPath, prevPath);
    maPathSetNextPath(prevPath, stopPath);
}

// Mark the path through the door as the one most recently used.  This is needed when we
// we need to delete a path loop.
static void markPathAsMostRecent(maPath path)
{
    maDoor door = maPathGetDoor(path);
    maPath otherPath;

    maForeachDoorPath(door, otherPath) {
        maPathSetMostRecent(otherPath, false);
    } maEndDoorPath;
    maPathSetMostRecent(path, true);
}

// Find the most recently used path through the door.
static maPath findMostRecentPath(maDoor door)
{
    maPath path;

    maForeachDoorPath(door, path) {
        if(maPathMostRecent(path)) {
            return path;
        }
    } maEndDoorPath;
    return maPathNull;
}

// Find the path in the room with the smallests label.
static maPath findOldestPathInRoom(maRoom room)
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

// Follow the largest labeled doors in a loop back to this door, building path objects as
// we go.
static void buildLoop(maRoom startRoom, uint64 label)
{
    maRoom room = startRoom;
    maDoor door;
    maPath prevPath = maPathNull;
    maPath path, firstPath = maPathNull;
    maPath oldPath = findOldestPathInRoom(startRoom);

    if(oldPath != maPathNull) {
        label = maPathGetLabel(oldPath);
    }
    do {
        door = findLargestLabelDoor(room);
        path = maPathCreate(door, label, prevPath, maPathNull);
        printf("Building path %lu element %u through door %u in room %u\n", label,
            maPath2Index(path), maDoor2Index(door), maRoom2Index(room));
        if(firstPath == maPathNull) {
            firstPath = path;
        }
        prevPath = path;
        room = maDoorGetToRoom(door);
    } while(room != startRoom);
    // Now close the loop.
    if(oldPath == maPathNull) {
        maPathSetNextPath(path, firstPath);
        maPathSetPrevPath(firstPath, path);
    } else {
        prevPath = maPathGetPrevPath(oldPath);
        maPathSetNextPath(prevPath, firstPath);
        maPathSetPrevPath(firstPath, prevPath);
        maPathSetNextPath(path, oldPath);
        maPathSetPrevPath(oldPath, path);
    }
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

// Just relable the entire path with the new label.
static void relabelPath(maPath path, uint64 label)
{
    while(maPathGetLabel(path) != label) {
        printf("Relabling path %lu into %lu in room %u\n", maPathGetLabel(path),
            label, maRoom2Index(maDoorGetFromRoom(maPathGetDoor(path))));
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

    relabelPath(sourcePath, maPathGetLabel(destPath));
    maPathSetNextPath(prevDestPath, sourcePath);
    maPathSetPrevPath(sourcePath, prevDestPath);
    maPathSetNextPath(prevSourcePath, destPath);
    maPathSetPrevPath(destPath, prevSourcePath);
}

// Find all the paths in the room and splice them into one path.  Join the newer paths
// into the oldest one, since it's likely to be larger, and we want to relable the paths
// we splice into it.  Return true if we do splice any paths together.
static bool splicePaths(maRoom room)
{
    maPath oldestPath = findOldestPathInRoom(room);
    maPath path;
    maDoor door;
    uint64 label = maPathGetLabel(oldestPath);
    bool splicedAPath = false;

    maForeachRoomOutDoor(room, door) {
        maForeachDoorPath(door, path) {
            if(maPathGetLabel(path) != label) {
                splicePathIntoPath(path, oldestPath);
                splicedAPath = true;
            }
        } maEndDoorPath;
    } maEndRoomOutDoor;
    return splicedAPath;
}

// Just count the number of outgoing doors in the room.
static uint32 countRoomOutDoors(maRoom room)
{
    maDoor door;
    int xDoor = 0;

    maForeachRoomOutDoor(room, door) {
        xDoor++;
    } maEndRoomOutDoor;
    return xDoor;
}

// Pick a random one-way door leaving the room.
static maDoor pickRandomDoor(maRoom room)
{
    maDoor doors[countRoomOutDoors(room)];
    maDoor door;
    int xDoor = 0;

    maForeachRoomOutDoor(room, door) {
        doors[xDoor++] = door;
    } maEndRoomOutDoor;
    return doors[rand() % xDoor];
}

// This is the "Random mouse" solution, which I normally call the "run screaming"
// algorithm, since my daughter when she was young would have done exactly that to get out
// of the maze.
static void solveMazeRandomly(maMaze maze)
{
    maRoom room = maMazeGetStartRoom(maze);
    maRoom finish = maMazeGetFinishRoom(maze);
    maDoor door;
    uint64 count = 0;

    while(room != finish) {
        door = pickRandomDoor(room);
        room = maDoorGetToRoom(door);
        count++;
        printf("%lu went through door %u to room %u\n", count, maDoor2Index(door),
            maRoom2Index(room));
    }
}

typedef enum {
    EXPLORING,
    ASCEND,
    DESCEND,
} State;

// Dump the door.
static void dumpDoor(maDoor door, State state, uint64 counter) {
    maRoom currentRoom = maDoorGetFromRoom(door);
    maRoom nextRoom = maDoorGetToRoom(door);
    char *stateName;

    switch (state) {
        case EXPLORING:
            stateName = "Exploring";
            break;
        case ASCEND:
            stateName = "Ascending";
            break;
        case DESCEND:
            stateName = "Descending";
            break;
    }
    printf("%lu: %s door %u, label %lu, from room %u to %u\n", counter,
            stateName, maDoor2Index(door), maDoorGetLabel(door),
            maRoom2Index(currentRoom), maRoom2Index(nextRoom));
}

// This is the "Random mouse" solution, which I normally call the "run screaming"
// algorithm, since my daughter when she was young would have done exactly that to get out
// of the maze.
static void solveMazeWithNewAlgorithm(maMaze maze)
{
    maRoom cur = maMazeGetStartRoom(maze);
    maRoom end = maMazeGetFinishRoom(maze);
    // Room counter.
    uint64 lastLabel = 1;
    // Whether we're actively heading back up to relabel.
    bool updating = false;
    // which door needs to be relabeled.
    uint64 relabelTarget = 0;
    // what the door will be relabled to.
    uint64 relabel = 0;
    // explore/ascend/descend.
    State state = EXPLORING;
    maDoor door;
    uint64 counter = 0;

    while (cur != end) {
        counter++;
        switch (state) {
            case ASCEND: {
                uint64 highest = 0;
                bool foundEmpty = false;
                maForeachRoomOutDoor(cur, door) {
                    // Relabel the dead room with the lowest point it reaches.
                    if (updating && maDoorGetLabel(door) == relabelTarget) {
                        updating = false;
                        printf("Relabling door %u from %lu to %lu\n",
                                maDoor2Index(door), relabelTarget, relabel);
                        maDoorSetLabel(door, relabel);
                    }
                    highest = utMax(highest, (maDoorGetLabel(door)));
                    if (maDoorGetLabel(door) == 0) {
                        foundEmpty = true;
                    }
                } maEndRoomOutDoor;
                if (highest > maRoomGetLabel(cur)) {
                    // Found a higher node, head up.
                    door = findLargestLabelDoor(cur);
                    dumpDoor(door, state, counter);
                    cur = maDoorGetToRoom(door);
                } else if (foundEmpty) {
                    // Nothing higher up, so we're at the end of the active
                    // branch. Start exploring new nodes
                    door = findUnlabeledDoor(cur);
                    state = EXPLORING;
                    lastLabel++;
                    maDoorSetLabel(door, lastLabel);
                    dumpDoor(door, state, counter);
                    cur = maDoorGetToRoom(door);
                } else {
                    // All destination nodes are lower or equal to this node.
                    // At least one path should return to a lower node to
                    // eventually go to zero, so we should have a way back down
                    // a bit.  Will relabel the door going to this room with
                    // the lowest value reachable from descending from this
                    // room.
                    relabelTarget = maRoomGetLabel(cur);
                    updating = true;
                    state = DESCEND;
                }
                break;
            }
            case EXPLORING:
                if (maRoomGetLabel(cur) != 0) {
                    // Hit another explored node, need to relabel the last door with the
                    // descent value.
                    relabelTarget = lastLabel;
                    // optional decrease of counter to make room numbering nicer.
                    lastLabel -= 1;
                    updating = true;
                    state = DESCEND;
                } else {
                    // First time seeing this node, all doors should be unmarked.
                    maRoomSetLabel(cur, lastLabel);
                    door = findUnlabeledDoor(cur);
                    lastLabel++;
                    maDoorSetLabel(door, lastLabel);
                    dumpDoor(door, state, counter);
                    cur = maDoorGetToRoom(door);
              }
              break;
            case DESCEND: {
                maDoor lowest = findSmallestLabelDoor(cur);
                maDoor highest = findLargestLabelDoor(cur);
                if (maDoorGetLabel(highest) > maRoomGetLabel(cur)) {
                    // We're on the DFS path.
                    state = ASCEND;
                    relabel = maRoomGetLabel(cur);
                } else {
                    utAssert(maDoorGetLabel(lowest) < maRoomGetLabel(cur));
                    dumpDoor(lowest, state, counter);
                    cur = maDoorGetToRoom(lowest);
                }
                break;
            }
        }
    }
    utAssert(cur == end);
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
        path = findPathInRoom(currentRoom);
        startLabel = count;
        do {
            door = findUnexploredDoor(currentRoom);
            count++;
            maDoorSetLabel(door, count);
            nextRoom = maDoorGetToRoom(door);
            printf("%lu Exploring door %u from room %u to %u\n", count,
                maDoor2Index(door), maRoom2Index(currentRoom), maRoom2Index(nextRoom));
            maDoorSetExplored(door, true);
            currentRoom = nextRoom;
            if(currentRoom == finish) {
                printf("Found finish!\n");
                return;
            }
            door = findLargestLabelDoor(currentRoom);
        } while(door == maDoorNull);
        if(count != startLabel + 1 || path == maPathNull ||
                findPathInRoom(currentRoom) == maPathNull ||
                maPathGetLabel(path) != maPathGetLabel(findPathInRoom(currentRoom))) {
            // We found an explored room.  Make a loop.
            buildLoop(currentRoom, count);
        }
        // Now we're in a room with a path in it.  Follow a path until we find an
        // unexplored door.  Splice together different paths that we find, and delete
        // loops with no unexplored doors.
        startLabel = count;
        path = findPathInRoom(currentRoom);
        utDo {
            if(splicePaths(currentRoom)) {
                startLabel = count; // Splicing in a path can add unexplored doors
            }
            door = findUnexploredDoor(currentRoom);
        } utWhile(door == maDoorNull) {
            door = findLargestLabelDoor(currentRoom);
            if(maDoorGetLabel(door) > startLabel) {
                // Leaves the current path object intact
                deletePathLoop(findMostRecentPath(door), path);
                startLabel = count;
            }
            markPathAsMostRecent(path);
            door = maPathGetDoor(path);
            utAssert(maDoorGetFromRoom(door) == currentRoom);
            count++;
            maDoorSetLabel(door, count);
            nextRoom = maDoorGetToRoom(door);
            printf("%lu Following path %lu through door %u from room %u to %u\n", count,
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

    printf("%s -> ", maRoomGetName(room));
    maForeachRoomOutDoor(room, door) {
        printf(" %s", maRoomGetName(maDoorGetToRoom(door)));
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

// Read a line from the file.  Throw out blank lines and anything after a #.
static char *readLine(
    FILE *file)
{
    uint32 size = 256;
    uint32 pos = 0;
    char *buffer = utMakeString(size);
    char *tempBuffer;
    int c;

    do {
        utDo {
            c = getc(file);
        } utWhile(c != EOF && c != '\n') {
            if(c == '#') {
                do {
                    c = getc(file);
                } while(c != EOF && c != '\n');
            } else {
                if(c == '\t') {
                    c = ' ';
                }
                if(c >= ' ') {
                    if(pos + 1 == size) {
                        size <<= 1;
                        tempBuffer = utMakeString(size);
                        strcpy(tempBuffer, buffer);
                        buffer = tempBuffer;
                    }
                    buffer[pos++] = c;
                }
            }
        } utRepeat;
    } while(pos == 0 && c != EOF);
    if(pos == 0) {
        return NULL;
    }
    buffer[pos] = '\0';
    return buffer;
}

// Read the word and update the line pointer.
static char *readWord(char **linePtr)
{
    char *line = *linePtr;
    char *p = line;
    char c;

    if(line == NULL || *line == '\0') {
        return NULL;
    }
    do {
        c = *p++;
    } while(c <= ' ' && c != '\0');
    line = p - 1;
    do {
        c = *p++;
    } while(c > ' ');
    *(p - 1) = '\0';
    if(c == '\0') {
        *linePtr = NULL;
    } else {
        *linePtr = p;
    }
    return line;
}

// Bulid a room from the line.
static maRoom buildRoom(maMaze maze, char *line)
{
    char *name = readWord(&line);
    utSym sym = utSymCreate(name);
    maRoom room = maMazeFindRoom(maze, sym);
    maRoom destRoom;

    if(room == maRoomNull) {
        room = maRoomCreate(maze, sym);
    }
    if(strcmp(readWord(&line), "->")) {
        printf("Expected ->\n");
        exit(1);
    }
    utDo {
        name = readWord(&line);
    } utWhile(name != NULL) {
        sym = utSymCreate(name);
        destRoom = maMazeFindRoom(maze, sym);
        if(destRoom == maRoomNull) {
            destRoom = maRoomCreate(maze, sym);
        }
        maDoorCreate(room, destRoom, true);
    } utRepeat;
    return room;
}

// Read the maze from a file.
static maMaze readMaze(
    char *mazeFileName)
{
    maMaze maze = maMazeAlloc();
    maRoom start, finish;
    FILE *file = fopen(mazeFileName, "r");
    char *line;

    if(file == NULL) {
        printf("Unable to read %s\n", mazeFileName);
        exit(1);
    }
    utDo {
        line = readLine(file);
    } utWhile(line != NULL) {
        buildRoom(maze, line);
    } utRepeat;
    fclose(file);
    start = maMazeFindRoom(maze, utSymCreate("Start"));
    finish = maMazeFindRoom(maze, utSymCreate("Finish"));
    if(start == maRoomNull || finish == maRoomNull) {
        printf("Mazes must have both a Start and Finish room\n");
        exit(1);
    }
    maMazeSetStartRoom(maze, start);
    maMazeSetFinishRoom(maze, finish);
    return maze;
}

int main(int argc, char **argv) {
    int numRooms, averageDoors, seed = 0xdeadbeef;
    int xArg = 1;
    maMaze maze;
    bool uniformMaze = false;
    bool useNewAlgorithm = false;
    bool useRandomMouse = false;
    char *mazeFileName = NULL;

    while(xArg < argc && *(argv[xArg]) == '-') {
        if(!strcmp(argv[xArg], "-u")) {
            uniformMaze = true;
        } else if(!strcmp(argv[xArg], "-f")) {
            mazeFileName = argv[++xArg];
        } else if(!strcmp(argv[xArg], "-s")) {
            seed = atoi(argv[++xArg]);
        } else if(!strcmp(argv[xArg], "-r")) {
            useRandomMouse = true;
        } else if(!strcmp(argv[xArg], "-n")) {
            useNewAlgorithm = true;
        }
        xArg++;
    }
    if(mazeFileName == NULL ? argc - xArg != 2 : xArg != argc) {
        printf("Usage: maze [-u] [-s seed] numRooms averageDoors\n"
           "       maze [-u] [-s seed] -f mazeFile\n"
           "    With the -f flag, read the maze from the mazeFile.\n"
           "    With the -u flag, build an unfair maze where all wrong doors go to start.\n"
           "    With the -r flag, use the random mouse algorithm.\n"
           "    With the -n flag, use the 'new' algorithm.\n");
        return 1;
    }
    if(mazeFileName == NULL) {
        numRooms = atoi(argv[xArg++]);
        averageDoors = atoi(argv[xArg++]);
    }
    utStart();
    maDatabaseStart();
    srand(seed ^ 0xdeadbeef);
    if(mazeFileName == NULL) {
        maze = buildMaze(numRooms, averageDoors, seed, uniformMaze);
    } else {
        maze = readMaze(mazeFileName);
    }
    printMaze(maze);
    if(useRandomMouse) {
        solveMazeRandomly(maze);
    } else if(useNewAlgorithm) {
        solveMazeWithNewAlgorithm(maze);
    } else {
        solveMaze(maze);
    }
    maDatabaseStop();
    utStop(false);
    return 0;
}
