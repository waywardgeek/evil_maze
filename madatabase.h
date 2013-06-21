/*----------------------------------------------------------------------------------------
  Module header file for: ma module
----------------------------------------------------------------------------------------*/
#ifndef MADATABASE_H

#define MADATABASE_H

#if defined __cplusplus
extern "C" {
#endif

#ifndef DD_UTIL_H
#include "ddutil.h"
#endif

extern uint8 maModuleID;
/* Class reference definitions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
typedef struct _struct_maMaze{char val;} *maMaze;
#define maMazeNull ((maMaze)0)
typedef struct _struct_maRoom{char val;} *maRoom;
#define maRoomNull ((maRoom)0)
typedef struct _struct_maDoor{char val;} *maDoor;
#define maDoorNull ((maDoor)0)
typedef struct _struct_maPath{char val;} *maPath;
#define maPathNull ((maPath)0)
#else
typedef uint32 maMaze;
#define maMazeNull 0
typedef uint32 maRoom;
#define maRoomNull 0
typedef uint32 maDoor;
#define maDoorNull 0
typedef uint32 maPath;
#define maPathNull 0
#endif

/* Constructor/Destructor hooks. */
typedef void (*maMazeCallbackType)(maMaze);
extern maMazeCallbackType maMazeConstructorCallback;
extern maMazeCallbackType maMazeDestructorCallback;
typedef void (*maRoomCallbackType)(maRoom);
extern maRoomCallbackType maRoomConstructorCallback;
extern maRoomCallbackType maRoomDestructorCallback;
typedef void (*maDoorCallbackType)(maDoor);
extern maDoorCallbackType maDoorConstructorCallback;
extern maDoorCallbackType maDoorDestructorCallback;
typedef void (*maPathCallbackType)(maPath);
extern maPathCallbackType maPathConstructorCallback;
extern maPathCallbackType maPathDestructorCallback;

/*----------------------------------------------------------------------------------------
  Root structure
----------------------------------------------------------------------------------------*/
struct maRootType_ {
    uint32 hash; /* This depends only on the structure of the database */
    maMaze firstFreeMaze;
    uint32 usedMaze, allocatedMaze;
    uint32 usedMazeRoomTable, allocatedMazeRoomTable, freeMazeRoomTable;
    maRoom firstFreeRoom;
    uint32 usedRoom, allocatedRoom;
    maDoor firstFreeDoor;
    uint32 usedDoor, allocatedDoor;
    maPath firstFreePath;
    uint32 usedPath, allocatedPath;
};
extern struct maRootType_ maRootData;

utInlineC uint32 maHash(void) {return maRootData.hash;}
utInlineC maMaze maFirstFreeMaze(void) {return maRootData.firstFreeMaze;}
utInlineC void maSetFirstFreeMaze(maMaze value) {maRootData.firstFreeMaze = (value);}
utInlineC uint32 maUsedMaze(void) {return maRootData.usedMaze;}
utInlineC uint32 maAllocatedMaze(void) {return maRootData.allocatedMaze;}
utInlineC void maSetUsedMaze(uint32 value) {maRootData.usedMaze = value;}
utInlineC void maSetAllocatedMaze(uint32 value) {maRootData.allocatedMaze = value;}
utInlineC uint32 maUsedMazeRoomTable(void) {return maRootData.usedMazeRoomTable;}
utInlineC uint32 maAllocatedMazeRoomTable(void) {return maRootData.allocatedMazeRoomTable;}
utInlineC uint32 maFreeMazeRoomTable(void) {return maRootData.freeMazeRoomTable;}
utInlineC void maSetUsedMazeRoomTable(uint32 value) {maRootData.usedMazeRoomTable = value;}
utInlineC void maSetAllocatedMazeRoomTable(uint32 value) {maRootData.allocatedMazeRoomTable = value;}
utInlineC void maSetFreeMazeRoomTable(int32 value) {maRootData.freeMazeRoomTable = value;}
utInlineC maRoom maFirstFreeRoom(void) {return maRootData.firstFreeRoom;}
utInlineC void maSetFirstFreeRoom(maRoom value) {maRootData.firstFreeRoom = (value);}
utInlineC uint32 maUsedRoom(void) {return maRootData.usedRoom;}
utInlineC uint32 maAllocatedRoom(void) {return maRootData.allocatedRoom;}
utInlineC void maSetUsedRoom(uint32 value) {maRootData.usedRoom = value;}
utInlineC void maSetAllocatedRoom(uint32 value) {maRootData.allocatedRoom = value;}
utInlineC maDoor maFirstFreeDoor(void) {return maRootData.firstFreeDoor;}
utInlineC void maSetFirstFreeDoor(maDoor value) {maRootData.firstFreeDoor = (value);}
utInlineC uint32 maUsedDoor(void) {return maRootData.usedDoor;}
utInlineC uint32 maAllocatedDoor(void) {return maRootData.allocatedDoor;}
utInlineC void maSetUsedDoor(uint32 value) {maRootData.usedDoor = value;}
utInlineC void maSetAllocatedDoor(uint32 value) {maRootData.allocatedDoor = value;}
utInlineC maPath maFirstFreePath(void) {return maRootData.firstFreePath;}
utInlineC void maSetFirstFreePath(maPath value) {maRootData.firstFreePath = (value);}
utInlineC uint32 maUsedPath(void) {return maRootData.usedPath;}
utInlineC uint32 maAllocatedPath(void) {return maRootData.allocatedPath;}
utInlineC void maSetUsedPath(uint32 value) {maRootData.usedPath = value;}
utInlineC void maSetAllocatedPath(uint32 value) {maRootData.allocatedPath = value;}

/* Validate macros */
#if defined(DD_DEBUG)
utInlineC maMaze maValidMaze(maMaze Maze) {
    utAssert(utLikely(Maze != maMazeNull && (uint32)(Maze - (maMaze)0) < maRootData.usedMaze));
    return Maze;}
utInlineC maRoom maValidRoom(maRoom Room) {
    utAssert(utLikely(Room != maRoomNull && (uint32)(Room - (maRoom)0) < maRootData.usedRoom));
    return Room;}
utInlineC maDoor maValidDoor(maDoor Door) {
    utAssert(utLikely(Door != maDoorNull && (uint32)(Door - (maDoor)0) < maRootData.usedDoor));
    return Door;}
utInlineC maPath maValidPath(maPath Path) {
    utAssert(utLikely(Path != maPathNull && (uint32)(Path - (maPath)0) < maRootData.usedPath));
    return Path;}
#else
utInlineC maMaze maValidMaze(maMaze Maze) {return Maze;}
utInlineC maRoom maValidRoom(maRoom Room) {return Room;}
utInlineC maDoor maValidDoor(maDoor Door) {return Door;}
utInlineC maPath maValidPath(maPath Path) {return Path;}
#endif

/* Object ref to integer conversions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
utInlineC uint32 maMaze2Index(maMaze Maze) {return Maze - (maMaze)0;}
utInlineC uint32 maMaze2ValidIndex(maMaze Maze) {return maValidMaze(Maze) - (maMaze)0;}
utInlineC maMaze maIndex2Maze(uint32 xMaze) {return (maMaze)(xMaze + (maMaze)(0));}
utInlineC uint32 maRoom2Index(maRoom Room) {return Room - (maRoom)0;}
utInlineC uint32 maRoom2ValidIndex(maRoom Room) {return maValidRoom(Room) - (maRoom)0;}
utInlineC maRoom maIndex2Room(uint32 xRoom) {return (maRoom)(xRoom + (maRoom)(0));}
utInlineC uint32 maDoor2Index(maDoor Door) {return Door - (maDoor)0;}
utInlineC uint32 maDoor2ValidIndex(maDoor Door) {return maValidDoor(Door) - (maDoor)0;}
utInlineC maDoor maIndex2Door(uint32 xDoor) {return (maDoor)(xDoor + (maDoor)(0));}
utInlineC uint32 maPath2Index(maPath Path) {return Path - (maPath)0;}
utInlineC uint32 maPath2ValidIndex(maPath Path) {return maValidPath(Path) - (maPath)0;}
utInlineC maPath maIndex2Path(uint32 xPath) {return (maPath)(xPath + (maPath)(0));}
#else
utInlineC uint32 maMaze2Index(maMaze Maze) {return Maze;}
utInlineC uint32 maMaze2ValidIndex(maMaze Maze) {return maValidMaze(Maze);}
utInlineC maMaze maIndex2Maze(uint32 xMaze) {return xMaze;}
utInlineC uint32 maRoom2Index(maRoom Room) {return Room;}
utInlineC uint32 maRoom2ValidIndex(maRoom Room) {return maValidRoom(Room);}
utInlineC maRoom maIndex2Room(uint32 xRoom) {return xRoom;}
utInlineC uint32 maDoor2Index(maDoor Door) {return Door;}
utInlineC uint32 maDoor2ValidIndex(maDoor Door) {return maValidDoor(Door);}
utInlineC maDoor maIndex2Door(uint32 xDoor) {return xDoor;}
utInlineC uint32 maPath2Index(maPath Path) {return Path;}
utInlineC uint32 maPath2ValidIndex(maPath Path) {return maValidPath(Path);}
utInlineC maPath maIndex2Path(uint32 xPath) {return xPath;}
#endif

/*----------------------------------------------------------------------------------------
  Fields for class Maze.
----------------------------------------------------------------------------------------*/
struct maMazeFields {
    maRoom *FirstRoom;
    maRoom *LastRoom;
    uint32 *RoomTableIndex_;
    uint32 *NumRoomTable;
    maRoom *RoomTable;
    uint32 *NumRoom;
    maRoom *StartRoom;
    maRoom *FinishRoom;
};
extern struct maMazeFields maMazes;

void maMazeAllocMore(void);
void maMazeCopyProps(maMaze maOldMaze, maMaze maNewMaze);
void maMazeAllocRoomTables(maMaze Maze, uint32 numRoomTables);
void maMazeResizeRoomTables(maMaze Maze, uint32 numRoomTables);
void maMazeFreeRoomTables(maMaze Maze);
void maCompactMazeRoomTables(void);
utInlineC maRoom maMazeGetFirstRoom(maMaze Maze) {return maMazes.FirstRoom[maMaze2ValidIndex(Maze)];}
utInlineC void maMazeSetFirstRoom(maMaze Maze, maRoom value) {maMazes.FirstRoom[maMaze2ValidIndex(Maze)] = value;}
utInlineC maRoom maMazeGetLastRoom(maMaze Maze) {return maMazes.LastRoom[maMaze2ValidIndex(Maze)];}
utInlineC void maMazeSetLastRoom(maMaze Maze, maRoom value) {maMazes.LastRoom[maMaze2ValidIndex(Maze)] = value;}
utInlineC uint32 maMazeGetRoomTableIndex_(maMaze Maze) {return maMazes.RoomTableIndex_[maMaze2ValidIndex(Maze)];}
utInlineC void maMazeSetRoomTableIndex_(maMaze Maze, uint32 value) {maMazes.RoomTableIndex_[maMaze2ValidIndex(Maze)] = value;}
utInlineC uint32 maMazeGetNumRoomTable(maMaze Maze) {return maMazes.NumRoomTable[maMaze2ValidIndex(Maze)];}
utInlineC void maMazeSetNumRoomTable(maMaze Maze, uint32 value) {maMazes.NumRoomTable[maMaze2ValidIndex(Maze)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 maMazeCheckRoomTableIndex(maMaze Maze, uint32 x) {utAssert(x < maMazeGetNumRoomTable(Maze)); return x;}
#else
utInlineC uint32 maMazeCheckRoomTableIndex(maMaze Maze, uint32 x) {return x;}
#endif
utInlineC maRoom maMazeGetiRoomTable(maMaze Maze, uint32 x) {return maMazes.RoomTable[
    maMazeGetRoomTableIndex_(Maze) + maMazeCheckRoomTableIndex(Maze, x)];}
utInlineC maRoom *maMazeGetRoomTable(maMaze Maze) {return maMazes.RoomTable + maMazeGetRoomTableIndex_(Maze);}
#define maMazeGetRoomTables maMazeGetRoomTable
utInlineC void maMazeSetRoomTable(maMaze Maze, maRoom *valuePtr, uint32 numRoomTable) {
    maMazeResizeRoomTables(Maze, numRoomTable);
    memcpy(maMazeGetRoomTables(Maze), valuePtr, numRoomTable*sizeof(maRoom));}
utInlineC void maMazeSetiRoomTable(maMaze Maze, uint32 x, maRoom value) {
    maMazes.RoomTable[maMazeGetRoomTableIndex_(Maze) + maMazeCheckRoomTableIndex(Maze, (x))] = value;}
utInlineC uint32 maMazeGetNumRoom(maMaze Maze) {return maMazes.NumRoom[maMaze2ValidIndex(Maze)];}
utInlineC void maMazeSetNumRoom(maMaze Maze, uint32 value) {maMazes.NumRoom[maMaze2ValidIndex(Maze)] = value;}
utInlineC maRoom maMazeGetStartRoom(maMaze Maze) {return maMazes.StartRoom[maMaze2ValidIndex(Maze)];}
utInlineC void maMazeSetStartRoom(maMaze Maze, maRoom value) {maMazes.StartRoom[maMaze2ValidIndex(Maze)] = value;}
utInlineC maRoom maMazeGetFinishRoom(maMaze Maze) {return maMazes.FinishRoom[maMaze2ValidIndex(Maze)];}
utInlineC void maMazeSetFinishRoom(maMaze Maze, maRoom value) {maMazes.FinishRoom[maMaze2ValidIndex(Maze)] = value;}
utInlineC void maMazeSetConstructorCallback(void(*func)(maMaze)) {maMazeConstructorCallback = func;}
utInlineC maMazeCallbackType maMazeGetConstructorCallback(void) {return maMazeConstructorCallback;}
utInlineC void maMazeSetDestructorCallback(void(*func)(maMaze)) {maMazeDestructorCallback = func;}
utInlineC maMazeCallbackType maMazeGetDestructorCallback(void) {return maMazeDestructorCallback;}
utInlineC maMaze maMazeNextFree(maMaze Maze) {return ((maMaze *)(void *)(maMazes.FirstRoom))[maMaze2ValidIndex(Maze)];}
utInlineC void maMazeSetNextFree(maMaze Maze, maMaze value) {
    ((maMaze *)(void *)(maMazes.FirstRoom))[maMaze2ValidIndex(Maze)] = value;}
utInlineC void maMazeFree(maMaze Maze) {
    maMazeFreeRoomTables(Maze);
    maMazeSetNextFree(Maze, maRootData.firstFreeMaze);
    maSetFirstFreeMaze(Maze);}
void maMazeDestroy(maMaze Maze);
utInlineC maMaze maMazeAllocRaw(void) {
    maMaze Maze;
    if(maRootData.firstFreeMaze != maMazeNull) {
        Maze = maRootData.firstFreeMaze;
        maSetFirstFreeMaze(maMazeNextFree(Maze));
    } else {
        if(maRootData.usedMaze == maRootData.allocatedMaze) {
            maMazeAllocMore();
        }
        Maze = maIndex2Maze(maRootData.usedMaze);
        maSetUsedMaze(maUsedMaze() + 1);
    }
    return Maze;}
utInlineC maMaze maMazeAlloc(void) {
    maMaze Maze = maMazeAllocRaw();
    maMazeSetFirstRoom(Maze, maRoomNull);
    maMazeSetLastRoom(Maze, maRoomNull);
    maMazeSetRoomTableIndex_(Maze, 0);
    maMazeSetNumRoomTable(Maze, 0);
    maMazeSetNumRoomTable(Maze, 0);
    maMazeSetNumRoom(Maze, 0);
    maMazeSetStartRoom(Maze, maRoomNull);
    maMazeSetFinishRoom(Maze, maRoomNull);
    if(maMazeConstructorCallback != NULL) {
        maMazeConstructorCallback(Maze);
    }
    return Maze;}

/*----------------------------------------------------------------------------------------
  Fields for class Room.
----------------------------------------------------------------------------------------*/
struct maRoomFields {
    utSym *Sym;
    maMaze *Maze;
    maRoom *NextMazeRoom;
    maRoom *PrevMazeRoom;
    maRoom *NextTableMazeRoom;
    maDoor *FirstOutDoor;
    maDoor *LastOutDoor;
    maDoor *FirstInDoor;
    maDoor *LastInDoor;
};
extern struct maRoomFields maRooms;

void maRoomAllocMore(void);
void maRoomCopyProps(maRoom maOldRoom, maRoom maNewRoom);
utInlineC utSym maRoomGetSym(maRoom Room) {return maRooms.Sym[maRoom2ValidIndex(Room)];}
utInlineC void maRoomSetSym(maRoom Room, utSym value) {maRooms.Sym[maRoom2ValidIndex(Room)] = value;}
utInlineC maMaze maRoomGetMaze(maRoom Room) {return maRooms.Maze[maRoom2ValidIndex(Room)];}
utInlineC void maRoomSetMaze(maRoom Room, maMaze value) {maRooms.Maze[maRoom2ValidIndex(Room)] = value;}
utInlineC maRoom maRoomGetNextMazeRoom(maRoom Room) {return maRooms.NextMazeRoom[maRoom2ValidIndex(Room)];}
utInlineC void maRoomSetNextMazeRoom(maRoom Room, maRoom value) {maRooms.NextMazeRoom[maRoom2ValidIndex(Room)] = value;}
utInlineC maRoom maRoomGetPrevMazeRoom(maRoom Room) {return maRooms.PrevMazeRoom[maRoom2ValidIndex(Room)];}
utInlineC void maRoomSetPrevMazeRoom(maRoom Room, maRoom value) {maRooms.PrevMazeRoom[maRoom2ValidIndex(Room)] = value;}
utInlineC maRoom maRoomGetNextTableMazeRoom(maRoom Room) {return maRooms.NextTableMazeRoom[maRoom2ValidIndex(Room)];}
utInlineC void maRoomSetNextTableMazeRoom(maRoom Room, maRoom value) {maRooms.NextTableMazeRoom[maRoom2ValidIndex(Room)] = value;}
utInlineC maDoor maRoomGetFirstOutDoor(maRoom Room) {return maRooms.FirstOutDoor[maRoom2ValidIndex(Room)];}
utInlineC void maRoomSetFirstOutDoor(maRoom Room, maDoor value) {maRooms.FirstOutDoor[maRoom2ValidIndex(Room)] = value;}
utInlineC maDoor maRoomGetLastOutDoor(maRoom Room) {return maRooms.LastOutDoor[maRoom2ValidIndex(Room)];}
utInlineC void maRoomSetLastOutDoor(maRoom Room, maDoor value) {maRooms.LastOutDoor[maRoom2ValidIndex(Room)] = value;}
utInlineC maDoor maRoomGetFirstInDoor(maRoom Room) {return maRooms.FirstInDoor[maRoom2ValidIndex(Room)];}
utInlineC void maRoomSetFirstInDoor(maRoom Room, maDoor value) {maRooms.FirstInDoor[maRoom2ValidIndex(Room)] = value;}
utInlineC maDoor maRoomGetLastInDoor(maRoom Room) {return maRooms.LastInDoor[maRoom2ValidIndex(Room)];}
utInlineC void maRoomSetLastInDoor(maRoom Room, maDoor value) {maRooms.LastInDoor[maRoom2ValidIndex(Room)] = value;}
utInlineC void maRoomSetConstructorCallback(void(*func)(maRoom)) {maRoomConstructorCallback = func;}
utInlineC maRoomCallbackType maRoomGetConstructorCallback(void) {return maRoomConstructorCallback;}
utInlineC void maRoomSetDestructorCallback(void(*func)(maRoom)) {maRoomDestructorCallback = func;}
utInlineC maRoomCallbackType maRoomGetDestructorCallback(void) {return maRoomDestructorCallback;}
utInlineC maRoom maRoomNextFree(maRoom Room) {return ((maRoom *)(void *)(maRooms.Sym))[maRoom2ValidIndex(Room)];}
utInlineC void maRoomSetNextFree(maRoom Room, maRoom value) {
    ((maRoom *)(void *)(maRooms.Sym))[maRoom2ValidIndex(Room)] = value;}
utInlineC void maRoomFree(maRoom Room) {
    maRoomSetNextFree(Room, maRootData.firstFreeRoom);
    maSetFirstFreeRoom(Room);}
void maRoomDestroy(maRoom Room);
utInlineC maRoom maRoomAllocRaw(void) {
    maRoom Room;
    if(maRootData.firstFreeRoom != maRoomNull) {
        Room = maRootData.firstFreeRoom;
        maSetFirstFreeRoom(maRoomNextFree(Room));
    } else {
        if(maRootData.usedRoom == maRootData.allocatedRoom) {
            maRoomAllocMore();
        }
        Room = maIndex2Room(maRootData.usedRoom);
        maSetUsedRoom(maUsedRoom() + 1);
    }
    return Room;}
utInlineC maRoom maRoomAlloc(void) {
    maRoom Room = maRoomAllocRaw();
    maRoomSetSym(Room, utSymNull);
    maRoomSetMaze(Room, maMazeNull);
    maRoomSetNextMazeRoom(Room, maRoomNull);
    maRoomSetPrevMazeRoom(Room, maRoomNull);
    maRoomSetNextTableMazeRoom(Room, maRoomNull);
    maRoomSetFirstOutDoor(Room, maDoorNull);
    maRoomSetLastOutDoor(Room, maDoorNull);
    maRoomSetFirstInDoor(Room, maDoorNull);
    maRoomSetLastInDoor(Room, maDoorNull);
    if(maRoomConstructorCallback != NULL) {
        maRoomConstructorCallback(Room);
    }
    return Room;}

/*----------------------------------------------------------------------------------------
  Fields for class Door.
----------------------------------------------------------------------------------------*/
struct maDoorFields {
    uint8 *Explored;
    uint64 *Label;
    maRoom *FromRoom;
    maDoor *NextRoomOutDoor;
    maDoor *PrevRoomOutDoor;
    maRoom *ToRoom;
    maDoor *NextRoomInDoor;
    maDoor *PrevRoomInDoor;
    maPath *FirstPath;
    maPath *LastPath;
};
extern struct maDoorFields maDoors;

void maDoorAllocMore(void);
void maDoorCopyProps(maDoor maOldDoor, maDoor maNewDoor);
utInlineC uint8 maDoorExplored(maDoor Door) {return maDoors.Explored[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetExplored(maDoor Door, uint8 value) {maDoors.Explored[maDoor2ValidIndex(Door)] = value;}
utInlineC uint64 maDoorGetLabel(maDoor Door) {return maDoors.Label[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetLabel(maDoor Door, uint64 value) {maDoors.Label[maDoor2ValidIndex(Door)] = value;}
utInlineC maRoom maDoorGetFromRoom(maDoor Door) {return maDoors.FromRoom[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetFromRoom(maDoor Door, maRoom value) {maDoors.FromRoom[maDoor2ValidIndex(Door)] = value;}
utInlineC maDoor maDoorGetNextRoomOutDoor(maDoor Door) {return maDoors.NextRoomOutDoor[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetNextRoomOutDoor(maDoor Door, maDoor value) {maDoors.NextRoomOutDoor[maDoor2ValidIndex(Door)] = value;}
utInlineC maDoor maDoorGetPrevRoomOutDoor(maDoor Door) {return maDoors.PrevRoomOutDoor[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetPrevRoomOutDoor(maDoor Door, maDoor value) {maDoors.PrevRoomOutDoor[maDoor2ValidIndex(Door)] = value;}
utInlineC maRoom maDoorGetToRoom(maDoor Door) {return maDoors.ToRoom[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetToRoom(maDoor Door, maRoom value) {maDoors.ToRoom[maDoor2ValidIndex(Door)] = value;}
utInlineC maDoor maDoorGetNextRoomInDoor(maDoor Door) {return maDoors.NextRoomInDoor[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetNextRoomInDoor(maDoor Door, maDoor value) {maDoors.NextRoomInDoor[maDoor2ValidIndex(Door)] = value;}
utInlineC maDoor maDoorGetPrevRoomInDoor(maDoor Door) {return maDoors.PrevRoomInDoor[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetPrevRoomInDoor(maDoor Door, maDoor value) {maDoors.PrevRoomInDoor[maDoor2ValidIndex(Door)] = value;}
utInlineC maPath maDoorGetFirstPath(maDoor Door) {return maDoors.FirstPath[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetFirstPath(maDoor Door, maPath value) {maDoors.FirstPath[maDoor2ValidIndex(Door)] = value;}
utInlineC maPath maDoorGetLastPath(maDoor Door) {return maDoors.LastPath[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetLastPath(maDoor Door, maPath value) {maDoors.LastPath[maDoor2ValidIndex(Door)] = value;}
utInlineC void maDoorSetConstructorCallback(void(*func)(maDoor)) {maDoorConstructorCallback = func;}
utInlineC maDoorCallbackType maDoorGetConstructorCallback(void) {return maDoorConstructorCallback;}
utInlineC void maDoorSetDestructorCallback(void(*func)(maDoor)) {maDoorDestructorCallback = func;}
utInlineC maDoorCallbackType maDoorGetDestructorCallback(void) {return maDoorDestructorCallback;}
utInlineC maDoor maDoorNextFree(maDoor Door) {return ((maDoor *)(void *)(maDoors.FromRoom))[maDoor2ValidIndex(Door)];}
utInlineC void maDoorSetNextFree(maDoor Door, maDoor value) {
    ((maDoor *)(void *)(maDoors.FromRoom))[maDoor2ValidIndex(Door)] = value;}
utInlineC void maDoorFree(maDoor Door) {
    maDoorSetNextFree(Door, maRootData.firstFreeDoor);
    maSetFirstFreeDoor(Door);}
void maDoorDestroy(maDoor Door);
utInlineC maDoor maDoorAllocRaw(void) {
    maDoor Door;
    if(maRootData.firstFreeDoor != maDoorNull) {
        Door = maRootData.firstFreeDoor;
        maSetFirstFreeDoor(maDoorNextFree(Door));
    } else {
        if(maRootData.usedDoor == maRootData.allocatedDoor) {
            maDoorAllocMore();
        }
        Door = maIndex2Door(maRootData.usedDoor);
        maSetUsedDoor(maUsedDoor() + 1);
    }
    return Door;}
utInlineC maDoor maDoorAlloc(void) {
    maDoor Door = maDoorAllocRaw();
    maDoorSetExplored(Door, 0);
    maDoorSetLabel(Door, 0);
    maDoorSetFromRoom(Door, maRoomNull);
    maDoorSetNextRoomOutDoor(Door, maDoorNull);
    maDoorSetPrevRoomOutDoor(Door, maDoorNull);
    maDoorSetToRoom(Door, maRoomNull);
    maDoorSetNextRoomInDoor(Door, maDoorNull);
    maDoorSetPrevRoomInDoor(Door, maDoorNull);
    maDoorSetFirstPath(Door, maPathNull);
    maDoorSetLastPath(Door, maPathNull);
    if(maDoorConstructorCallback != NULL) {
        maDoorConstructorCallback(Door);
    }
    return Door;}

/*----------------------------------------------------------------------------------------
  Fields for class Path.
----------------------------------------------------------------------------------------*/
struct maPathFields {
    uint64 *Label;
    maPath *NextPath;
    maPath *PrevPath;
    uint8 *MostRecent;
    maDoor *Door;
    maPath *NextDoorPath;
    maPath *PrevDoorPath;
};
extern struct maPathFields maPaths;

void maPathAllocMore(void);
void maPathCopyProps(maPath maOldPath, maPath maNewPath);
utInlineC uint64 maPathGetLabel(maPath Path) {return maPaths.Label[maPath2ValidIndex(Path)];}
utInlineC void maPathSetLabel(maPath Path, uint64 value) {maPaths.Label[maPath2ValidIndex(Path)] = value;}
utInlineC maPath maPathGetNextPath(maPath Path) {return maPaths.NextPath[maPath2ValidIndex(Path)];}
utInlineC void maPathSetNextPath(maPath Path, maPath value) {maPaths.NextPath[maPath2ValidIndex(Path)] = value;}
utInlineC maPath maPathGetPrevPath(maPath Path) {return maPaths.PrevPath[maPath2ValidIndex(Path)];}
utInlineC void maPathSetPrevPath(maPath Path, maPath value) {maPaths.PrevPath[maPath2ValidIndex(Path)] = value;}
utInlineC uint8 maPathMostRecent(maPath Path) {return maPaths.MostRecent[maPath2ValidIndex(Path)];}
utInlineC void maPathSetMostRecent(maPath Path, uint8 value) {maPaths.MostRecent[maPath2ValidIndex(Path)] = value;}
utInlineC maDoor maPathGetDoor(maPath Path) {return maPaths.Door[maPath2ValidIndex(Path)];}
utInlineC void maPathSetDoor(maPath Path, maDoor value) {maPaths.Door[maPath2ValidIndex(Path)] = value;}
utInlineC maPath maPathGetNextDoorPath(maPath Path) {return maPaths.NextDoorPath[maPath2ValidIndex(Path)];}
utInlineC void maPathSetNextDoorPath(maPath Path, maPath value) {maPaths.NextDoorPath[maPath2ValidIndex(Path)] = value;}
utInlineC maPath maPathGetPrevDoorPath(maPath Path) {return maPaths.PrevDoorPath[maPath2ValidIndex(Path)];}
utInlineC void maPathSetPrevDoorPath(maPath Path, maPath value) {maPaths.PrevDoorPath[maPath2ValidIndex(Path)] = value;}
utInlineC void maPathSetConstructorCallback(void(*func)(maPath)) {maPathConstructorCallback = func;}
utInlineC maPathCallbackType maPathGetConstructorCallback(void) {return maPathConstructorCallback;}
utInlineC void maPathSetDestructorCallback(void(*func)(maPath)) {maPathDestructorCallback = func;}
utInlineC maPathCallbackType maPathGetDestructorCallback(void) {return maPathDestructorCallback;}
utInlineC maPath maPathNextFree(maPath Path) {return ((maPath *)(void *)(maPaths.NextPath))[maPath2ValidIndex(Path)];}
utInlineC void maPathSetNextFree(maPath Path, maPath value) {
    ((maPath *)(void *)(maPaths.NextPath))[maPath2ValidIndex(Path)] = value;}
utInlineC void maPathFree(maPath Path) {
    maPathSetNextFree(Path, maRootData.firstFreePath);
    maSetFirstFreePath(Path);}
void maPathDestroy(maPath Path);
utInlineC maPath maPathAllocRaw(void) {
    maPath Path;
    if(maRootData.firstFreePath != maPathNull) {
        Path = maRootData.firstFreePath;
        maSetFirstFreePath(maPathNextFree(Path));
    } else {
        if(maRootData.usedPath == maRootData.allocatedPath) {
            maPathAllocMore();
        }
        Path = maIndex2Path(maRootData.usedPath);
        maSetUsedPath(maUsedPath() + 1);
    }
    return Path;}
utInlineC maPath maPathAlloc(void) {
    maPath Path = maPathAllocRaw();
    maPathSetLabel(Path, 0);
    maPathSetNextPath(Path, maPathNull);
    maPathSetPrevPath(Path, maPathNull);
    maPathSetMostRecent(Path, 0);
    maPathSetDoor(Path, maDoorNull);
    maPathSetNextDoorPath(Path, maPathNull);
    maPathSetPrevDoorPath(Path, maPathNull);
    if(maPathConstructorCallback != NULL) {
        maPathConstructorCallback(Path);
    }
    return Path;}

/*----------------------------------------------------------------------------------------
  Relationship macros between classes.
----------------------------------------------------------------------------------------*/
maRoom maMazeFindRoom(maMaze Maze, utSym Sym);
void maMazeRenameRoom(maMaze Maze, maRoom _Room, utSym sym);
utInlineC char *maRoomGetName(maRoom Room) {return utSymGetName(maRoomGetSym(Room));}
#define maForeachMazeRoom(pVar, cVar) \
    for(cVar = maMazeGetFirstRoom(pVar); cVar != maRoomNull; \
        cVar = maRoomGetNextMazeRoom(cVar))
#define maEndMazeRoom
#define maSafeForeachMazeRoom(pVar, cVar) { \
    maRoom _nextRoom; \
    for(cVar = maMazeGetFirstRoom(pVar); cVar != maRoomNull; cVar = _nextRoom) { \
        _nextRoom = maRoomGetNextMazeRoom(cVar);
#define maEndSafeMazeRoom }}
void maMazeInsertRoom(maMaze Maze, maRoom _Room);
void maMazeRemoveRoom(maMaze Maze, maRoom _Room);
void maMazeInsertAfterRoom(maMaze Maze, maRoom prevRoom, maRoom _Room);
void maMazeAppendRoom(maMaze Maze, maRoom _Room);
#define maForeachRoomOutDoor(pVar, cVar) \
    for(cVar = maRoomGetFirstOutDoor(pVar); cVar != maDoorNull; \
        cVar = maDoorGetNextRoomOutDoor(cVar))
#define maEndRoomOutDoor
#define maSafeForeachRoomOutDoor(pVar, cVar) { \
    maDoor _nextDoor; \
    for(cVar = maRoomGetFirstOutDoor(pVar); cVar != maDoorNull; cVar = _nextDoor) { \
        _nextDoor = maDoorGetNextRoomOutDoor(cVar);
#define maEndSafeRoomOutDoor }}
#define maForeachRoomInDoor(pVar, cVar) \
    for(cVar = maRoomGetFirstInDoor(pVar); cVar != maDoorNull; \
        cVar = maDoorGetNextRoomInDoor(cVar))
#define maEndRoomInDoor
#define maSafeForeachRoomInDoor(pVar, cVar) { \
    maDoor _nextDoor; \
    for(cVar = maRoomGetFirstInDoor(pVar); cVar != maDoorNull; cVar = _nextDoor) { \
        _nextDoor = maDoorGetNextRoomInDoor(cVar);
#define maEndSafeRoomInDoor }}
void maRoomInsertOutDoor(maRoom Room, maDoor _Door);
void maRoomRemoveOutDoor(maRoom Room, maDoor _Door);
void maRoomInsertAfterOutDoor(maRoom Room, maDoor prevDoor, maDoor _Door);
void maRoomAppendOutDoor(maRoom Room, maDoor _Door);
void maRoomInsertInDoor(maRoom Room, maDoor _Door);
void maRoomRemoveInDoor(maRoom Room, maDoor _Door);
void maRoomInsertAfterInDoor(maRoom Room, maDoor prevDoor, maDoor _Door);
void maRoomAppendInDoor(maRoom Room, maDoor _Door);
#define maForeachDoorPath(pVar, cVar) \
    for(cVar = maDoorGetFirstPath(pVar); cVar != maPathNull; \
        cVar = maPathGetNextDoorPath(cVar))
#define maEndDoorPath
#define maSafeForeachDoorPath(pVar, cVar) { \
    maPath _nextPath; \
    for(cVar = maDoorGetFirstPath(pVar); cVar != maPathNull; cVar = _nextPath) { \
        _nextPath = maPathGetNextDoorPath(cVar);
#define maEndSafeDoorPath }}
void maDoorInsertPath(maDoor Door, maPath _Path);
void maDoorRemovePath(maDoor Door, maPath _Path);
void maDoorInsertAfterPath(maDoor Door, maPath prevPath, maPath _Path);
void maDoorAppendPath(maDoor Door, maPath _Path);
void maDatabaseStart(void);
void maDatabaseStop(void);
#if defined __cplusplus
}
#endif

#endif
