/*----------------------------------------------------------------------------------------
  Database ma
----------------------------------------------------------------------------------------*/

#include "madatabase.h"

struct maRootType_ maRootData;
uint8 maModuleID;
struct maMazeFields maMazes;
struct maRoomFields maRooms;
struct maDoorFields maDoors;
struct maPathFields maPaths;

/*----------------------------------------------------------------------------------------
  Constructor/Destructor hooks.
----------------------------------------------------------------------------------------*/
maMazeCallbackType maMazeConstructorCallback;
maMazeCallbackType maMazeDestructorCallback;
maRoomCallbackType maRoomConstructorCallback;
maRoomCallbackType maRoomDestructorCallback;
maDoorCallbackType maDoorConstructorCallback;
maDoorCallbackType maDoorDestructorCallback;
maPathCallbackType maPathConstructorCallback;
maPathCallbackType maPathDestructorCallback;

/*----------------------------------------------------------------------------------------
  Destroy Maze including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void maMazeDestroy(
    maMaze Maze)
{
    maRoom Room_;

    if(maMazeDestructorCallback != NULL) {
        maMazeDestructorCallback(Maze);
    }
    maSafeForeachMazeRoom(Maze, Room_) {
        maRoomDestroy(Room_);
    } maEndSafeMazeRoom;
    maMazeFree(Maze);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocMaze(void)
{
    maMaze Maze = maMazeAlloc();

    return maMaze2Index(Maze);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyMaze(
    uint64 objectIndex)
{
    maMazeDestroy(maIndex2Maze((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Maze.
----------------------------------------------------------------------------------------*/
static void allocMazes(void)
{
    maSetAllocatedMaze(2);
    maSetUsedMaze(1);
    maSetFirstFreeMaze(maMazeNull);
    maMazes.FirstRoom = utNewAInitFirst(maRoom, (maAllocatedMaze()));
    maMazes.LastRoom = utNewAInitFirst(maRoom, (maAllocatedMaze()));
    maMazes.StartRoom = utNewAInitFirst(maRoom, (maAllocatedMaze()));
    maMazes.FinishRoom = utNewAInitFirst(maRoom, (maAllocatedMaze()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Maze.
----------------------------------------------------------------------------------------*/
static void reallocMazes(
    uint32 newSize)
{
    utResizeArray(maMazes.FirstRoom, (newSize));
    utResizeArray(maMazes.LastRoom, (newSize));
    utResizeArray(maMazes.StartRoom, (newSize));
    utResizeArray(maMazes.FinishRoom, (newSize));
    maSetAllocatedMaze(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Mazes.
----------------------------------------------------------------------------------------*/
void maMazeAllocMore(void)
{
    reallocMazes((uint32)(maAllocatedMaze() + (maAllocatedMaze() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Maze.
----------------------------------------------------------------------------------------*/
void maMazeCopyProps(
    maMaze oldMaze,
    maMaze newMaze)
{
}

/*----------------------------------------------------------------------------------------
  Add the Room to the head of the list on the Maze.
----------------------------------------------------------------------------------------*/
void maMazeInsertRoom(
    maMaze Maze,
    maRoom _Room)
{
#if defined(DD_DEBUG)
    if(Maze == maMazeNull) {
        utExit("Non-existent Maze");
    }
    if(_Room == maRoomNull) {
        utExit("Non-existent Room");
    }
    if(maRoomGetMaze(_Room) != maMazeNull) {
        utExit("Attempting to add Room to Maze twice");
    }
#endif
    maRoomSetNextMazeRoom(_Room, maMazeGetFirstRoom(Maze));
    if(maMazeGetFirstRoom(Maze) != maRoomNull) {
        maRoomSetPrevMazeRoom(maMazeGetFirstRoom(Maze), _Room);
    }
    maMazeSetFirstRoom(Maze, _Room);
    maRoomSetPrevMazeRoom(_Room, maRoomNull);
    if(maMazeGetLastRoom(Maze) == maRoomNull) {
        maMazeSetLastRoom(Maze, _Room);
    }
    maRoomSetMaze(_Room, Maze);
}

/*----------------------------------------------------------------------------------------
  Add the Room to the end of the list on the Maze.
----------------------------------------------------------------------------------------*/
void maMazeAppendRoom(
    maMaze Maze,
    maRoom _Room)
{
#if defined(DD_DEBUG)
    if(Maze == maMazeNull) {
        utExit("Non-existent Maze");
    }
    if(_Room == maRoomNull) {
        utExit("Non-existent Room");
    }
    if(maRoomGetMaze(_Room) != maMazeNull) {
        utExit("Attempting to add Room to Maze twice");
    }
#endif
    maRoomSetPrevMazeRoom(_Room, maMazeGetLastRoom(Maze));
    if(maMazeGetLastRoom(Maze) != maRoomNull) {
        maRoomSetNextMazeRoom(maMazeGetLastRoom(Maze), _Room);
    }
    maMazeSetLastRoom(Maze, _Room);
    maRoomSetNextMazeRoom(_Room, maRoomNull);
    if(maMazeGetFirstRoom(Maze) == maRoomNull) {
        maMazeSetFirstRoom(Maze, _Room);
    }
    maRoomSetMaze(_Room, Maze);
}

/*----------------------------------------------------------------------------------------
  Insert the Room to the Maze after the previous Room.
----------------------------------------------------------------------------------------*/
void maMazeInsertAfterRoom(
    maMaze Maze,
    maRoom prevRoom,
    maRoom _Room)
{
    maRoom nextRoom = maRoomGetNextMazeRoom(prevRoom);

#if defined(DD_DEBUG)
    if(Maze == maMazeNull) {
        utExit("Non-existent Maze");
    }
    if(_Room == maRoomNull) {
        utExit("Non-existent Room");
    }
    if(maRoomGetMaze(_Room) != maMazeNull) {
        utExit("Attempting to add Room to Maze twice");
    }
#endif
    maRoomSetNextMazeRoom(_Room, nextRoom);
    maRoomSetNextMazeRoom(prevRoom, _Room);
    maRoomSetPrevMazeRoom(_Room, prevRoom);
    if(nextRoom != maRoomNull) {
        maRoomSetPrevMazeRoom(nextRoom, _Room);
    }
    if(maMazeGetLastRoom(Maze) == prevRoom) {
        maMazeSetLastRoom(Maze, _Room);
    }
    maRoomSetMaze(_Room, Maze);
}

/*----------------------------------------------------------------------------------------
 Remove the Room from the Maze.
----------------------------------------------------------------------------------------*/
void maMazeRemoveRoom(
    maMaze Maze,
    maRoom _Room)
{
    maRoom pRoom, nRoom;

#if defined(DD_DEBUG)
    if(_Room == maRoomNull) {
        utExit("Non-existent Room");
    }
    if(maRoomGetMaze(_Room) != maMazeNull && maRoomGetMaze(_Room) != Maze) {
        utExit("Delete Room from non-owning Maze");
    }
#endif
    nRoom = maRoomGetNextMazeRoom(_Room);
    pRoom = maRoomGetPrevMazeRoom(_Room);
    if(pRoom != maRoomNull) {
        maRoomSetNextMazeRoom(pRoom, nRoom);
    } else if(maMazeGetFirstRoom(Maze) == _Room) {
        maMazeSetFirstRoom(Maze, nRoom);
    }
    if(nRoom != maRoomNull) {
        maRoomSetPrevMazeRoom(nRoom, pRoom);
    } else if(maMazeGetLastRoom(Maze) == _Room) {
        maMazeSetLastRoom(Maze, pRoom);
    }
    maRoomSetNextMazeRoom(_Room, maRoomNull);
    maRoomSetPrevMazeRoom(_Room, maRoomNull);
    maRoomSetMaze(_Room, maMazeNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void maShowMaze(
    maMaze Maze)
{
    utDatabaseShowObject("ma", "Maze", maMaze2Index(Maze));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Room including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void maRoomDestroy(
    maRoom Room)
{
    maDoor OutDoor_;
    maDoor InDoor_;
    maMaze owningMaze = maRoomGetMaze(Room);

    if(maRoomDestructorCallback != NULL) {
        maRoomDestructorCallback(Room);
    }
    maSafeForeachRoomOutDoor(Room, OutDoor_) {
        maDoorDestroy(OutDoor_);
    } maEndSafeRoomOutDoor;
    maSafeForeachRoomInDoor(Room, InDoor_) {
        maDoorDestroy(InDoor_);
    } maEndSafeRoomInDoor;
    if(owningMaze != maMazeNull) {
        maMazeRemoveRoom(owningMaze, Room);
#if defined(DD_DEBUG)
    } else {
        utExit("Room without owning Maze");
#endif
    }
    maRoomFree(Room);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocRoom(void)
{
    maRoom Room = maRoomAlloc();

    return maRoom2Index(Room);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyRoom(
    uint64 objectIndex)
{
    maRoomDestroy(maIndex2Room((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Room.
----------------------------------------------------------------------------------------*/
static void allocRooms(void)
{
    maSetAllocatedRoom(2);
    maSetUsedRoom(1);
    maSetFirstFreeRoom(maRoomNull);
    maRooms.Start = utNewAInitFirst(uint8, (maAllocatedRoom()));
    maRooms.Finish = utNewAInitFirst(uint8, (maAllocatedRoom()));
    maRooms.Maze = utNewAInitFirst(maMaze, (maAllocatedRoom()));
    maRooms.NextMazeRoom = utNewAInitFirst(maRoom, (maAllocatedRoom()));
    maRooms.PrevMazeRoom = utNewAInitFirst(maRoom, (maAllocatedRoom()));
    maRooms.FirstOutDoor = utNewAInitFirst(maDoor, (maAllocatedRoom()));
    maRooms.LastOutDoor = utNewAInitFirst(maDoor, (maAllocatedRoom()));
    maRooms.FirstInDoor = utNewAInitFirst(maDoor, (maAllocatedRoom()));
    maRooms.LastInDoor = utNewAInitFirst(maDoor, (maAllocatedRoom()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Room.
----------------------------------------------------------------------------------------*/
static void reallocRooms(
    uint32 newSize)
{
    utResizeArray(maRooms.Start, (newSize));
    utResizeArray(maRooms.Finish, (newSize));
    utResizeArray(maRooms.Maze, (newSize));
    utResizeArray(maRooms.NextMazeRoom, (newSize));
    utResizeArray(maRooms.PrevMazeRoom, (newSize));
    utResizeArray(maRooms.FirstOutDoor, (newSize));
    utResizeArray(maRooms.LastOutDoor, (newSize));
    utResizeArray(maRooms.FirstInDoor, (newSize));
    utResizeArray(maRooms.LastInDoor, (newSize));
    maSetAllocatedRoom(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Rooms.
----------------------------------------------------------------------------------------*/
void maRoomAllocMore(void)
{
    reallocRooms((uint32)(maAllocatedRoom() + (maAllocatedRoom() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Room.
----------------------------------------------------------------------------------------*/
void maRoomCopyProps(
    maRoom oldRoom,
    maRoom newRoom)
{
    maRoomSetStart(newRoom, maRoomStart(oldRoom));
    maRoomSetFinish(newRoom, maRoomFinish(oldRoom));
}

/*----------------------------------------------------------------------------------------
  Add the OutDoor to the head of the list on the Room.
----------------------------------------------------------------------------------------*/
void maRoomInsertOutDoor(
    maRoom Room,
    maDoor _Door)
{
#if defined(DD_DEBUG)
    if(Room == maRoomNull) {
        utExit("Non-existent Room");
    }
    if(_Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(maDoorGetFromRoom(_Door) != maRoomNull) {
        utExit("Attempting to add Door to Room twice");
    }
#endif
    maDoorSetNextRoomOutDoor(_Door, maRoomGetFirstOutDoor(Room));
    if(maRoomGetFirstOutDoor(Room) != maDoorNull) {
        maDoorSetPrevRoomOutDoor(maRoomGetFirstOutDoor(Room), _Door);
    }
    maRoomSetFirstOutDoor(Room, _Door);
    maDoorSetPrevRoomOutDoor(_Door, maDoorNull);
    if(maRoomGetLastOutDoor(Room) == maDoorNull) {
        maRoomSetLastOutDoor(Room, _Door);
    }
    maDoorSetFromRoom(_Door, Room);
}

/*----------------------------------------------------------------------------------------
  Add the OutDoor to the end of the list on the Room.
----------------------------------------------------------------------------------------*/
void maRoomAppendOutDoor(
    maRoom Room,
    maDoor _Door)
{
#if defined(DD_DEBUG)
    if(Room == maRoomNull) {
        utExit("Non-existent Room");
    }
    if(_Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(maDoorGetFromRoom(_Door) != maRoomNull) {
        utExit("Attempting to add Door to Room twice");
    }
#endif
    maDoorSetPrevRoomOutDoor(_Door, maRoomGetLastOutDoor(Room));
    if(maRoomGetLastOutDoor(Room) != maDoorNull) {
        maDoorSetNextRoomOutDoor(maRoomGetLastOutDoor(Room), _Door);
    }
    maRoomSetLastOutDoor(Room, _Door);
    maDoorSetNextRoomOutDoor(_Door, maDoorNull);
    if(maRoomGetFirstOutDoor(Room) == maDoorNull) {
        maRoomSetFirstOutDoor(Room, _Door);
    }
    maDoorSetFromRoom(_Door, Room);
}

/*----------------------------------------------------------------------------------------
  Insert the OutDoor to the Room after the previous OutDoor.
----------------------------------------------------------------------------------------*/
void maRoomInsertAfterOutDoor(
    maRoom Room,
    maDoor prevDoor,
    maDoor _Door)
{
    maDoor nextDoor = maDoorGetNextRoomOutDoor(prevDoor);

#if defined(DD_DEBUG)
    if(Room == maRoomNull) {
        utExit("Non-existent Room");
    }
    if(_Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(maDoorGetFromRoom(_Door) != maRoomNull) {
        utExit("Attempting to add Door to Room twice");
    }
#endif
    maDoorSetNextRoomOutDoor(_Door, nextDoor);
    maDoorSetNextRoomOutDoor(prevDoor, _Door);
    maDoorSetPrevRoomOutDoor(_Door, prevDoor);
    if(nextDoor != maDoorNull) {
        maDoorSetPrevRoomOutDoor(nextDoor, _Door);
    }
    if(maRoomGetLastOutDoor(Room) == prevDoor) {
        maRoomSetLastOutDoor(Room, _Door);
    }
    maDoorSetFromRoom(_Door, Room);
}

/*----------------------------------------------------------------------------------------
 Remove the OutDoor from the Room.
----------------------------------------------------------------------------------------*/
void maRoomRemoveOutDoor(
    maRoom Room,
    maDoor _Door)
{
    maDoor pDoor, nDoor;

#if defined(DD_DEBUG)
    if(_Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(maDoorGetFromRoom(_Door) != maRoomNull && maDoorGetFromRoom(_Door) != Room) {
        utExit("Delete Door from non-owning Room");
    }
#endif
    nDoor = maDoorGetNextRoomOutDoor(_Door);
    pDoor = maDoorGetPrevRoomOutDoor(_Door);
    if(pDoor != maDoorNull) {
        maDoorSetNextRoomOutDoor(pDoor, nDoor);
    } else if(maRoomGetFirstOutDoor(Room) == _Door) {
        maRoomSetFirstOutDoor(Room, nDoor);
    }
    if(nDoor != maDoorNull) {
        maDoorSetPrevRoomOutDoor(nDoor, pDoor);
    } else if(maRoomGetLastOutDoor(Room) == _Door) {
        maRoomSetLastOutDoor(Room, pDoor);
    }
    maDoorSetNextRoomOutDoor(_Door, maDoorNull);
    maDoorSetPrevRoomOutDoor(_Door, maDoorNull);
    maDoorSetFromRoom(_Door, maRoomNull);
}

/*----------------------------------------------------------------------------------------
  Add the InDoor to the head of the list on the Room.
----------------------------------------------------------------------------------------*/
void maRoomInsertInDoor(
    maRoom Room,
    maDoor _Door)
{
#if defined(DD_DEBUG)
    if(Room == maRoomNull) {
        utExit("Non-existent Room");
    }
    if(_Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(maDoorGetToRoom(_Door) != maRoomNull) {
        utExit("Attempting to add Door to Room twice");
    }
#endif
    maDoorSetNextRoomInDoor(_Door, maRoomGetFirstInDoor(Room));
    if(maRoomGetFirstInDoor(Room) != maDoorNull) {
        maDoorSetPrevRoomInDoor(maRoomGetFirstInDoor(Room), _Door);
    }
    maRoomSetFirstInDoor(Room, _Door);
    maDoorSetPrevRoomInDoor(_Door, maDoorNull);
    if(maRoomGetLastInDoor(Room) == maDoorNull) {
        maRoomSetLastInDoor(Room, _Door);
    }
    maDoorSetToRoom(_Door, Room);
}

/*----------------------------------------------------------------------------------------
  Add the InDoor to the end of the list on the Room.
----------------------------------------------------------------------------------------*/
void maRoomAppendInDoor(
    maRoom Room,
    maDoor _Door)
{
#if defined(DD_DEBUG)
    if(Room == maRoomNull) {
        utExit("Non-existent Room");
    }
    if(_Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(maDoorGetToRoom(_Door) != maRoomNull) {
        utExit("Attempting to add Door to Room twice");
    }
#endif
    maDoorSetPrevRoomInDoor(_Door, maRoomGetLastInDoor(Room));
    if(maRoomGetLastInDoor(Room) != maDoorNull) {
        maDoorSetNextRoomInDoor(maRoomGetLastInDoor(Room), _Door);
    }
    maRoomSetLastInDoor(Room, _Door);
    maDoorSetNextRoomInDoor(_Door, maDoorNull);
    if(maRoomGetFirstInDoor(Room) == maDoorNull) {
        maRoomSetFirstInDoor(Room, _Door);
    }
    maDoorSetToRoom(_Door, Room);
}

/*----------------------------------------------------------------------------------------
  Insert the InDoor to the Room after the previous InDoor.
----------------------------------------------------------------------------------------*/
void maRoomInsertAfterInDoor(
    maRoom Room,
    maDoor prevDoor,
    maDoor _Door)
{
    maDoor nextDoor = maDoorGetNextRoomInDoor(prevDoor);

#if defined(DD_DEBUG)
    if(Room == maRoomNull) {
        utExit("Non-existent Room");
    }
    if(_Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(maDoorGetToRoom(_Door) != maRoomNull) {
        utExit("Attempting to add Door to Room twice");
    }
#endif
    maDoorSetNextRoomInDoor(_Door, nextDoor);
    maDoorSetNextRoomInDoor(prevDoor, _Door);
    maDoorSetPrevRoomInDoor(_Door, prevDoor);
    if(nextDoor != maDoorNull) {
        maDoorSetPrevRoomInDoor(nextDoor, _Door);
    }
    if(maRoomGetLastInDoor(Room) == prevDoor) {
        maRoomSetLastInDoor(Room, _Door);
    }
    maDoorSetToRoom(_Door, Room);
}

/*----------------------------------------------------------------------------------------
 Remove the InDoor from the Room.
----------------------------------------------------------------------------------------*/
void maRoomRemoveInDoor(
    maRoom Room,
    maDoor _Door)
{
    maDoor pDoor, nDoor;

#if defined(DD_DEBUG)
    if(_Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(maDoorGetToRoom(_Door) != maRoomNull && maDoorGetToRoom(_Door) != Room) {
        utExit("Delete Door from non-owning Room");
    }
#endif
    nDoor = maDoorGetNextRoomInDoor(_Door);
    pDoor = maDoorGetPrevRoomInDoor(_Door);
    if(pDoor != maDoorNull) {
        maDoorSetNextRoomInDoor(pDoor, nDoor);
    } else if(maRoomGetFirstInDoor(Room) == _Door) {
        maRoomSetFirstInDoor(Room, nDoor);
    }
    if(nDoor != maDoorNull) {
        maDoorSetPrevRoomInDoor(nDoor, pDoor);
    } else if(maRoomGetLastInDoor(Room) == _Door) {
        maRoomSetLastInDoor(Room, pDoor);
    }
    maDoorSetNextRoomInDoor(_Door, maDoorNull);
    maDoorSetPrevRoomInDoor(_Door, maDoorNull);
    maDoorSetToRoom(_Door, maRoomNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void maShowRoom(
    maRoom Room)
{
    utDatabaseShowObject("ma", "Room", maRoom2Index(Room));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Door including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void maDoorDestroy(
    maDoor Door)
{
    maPath Path_;
    maRoom owningFromRoom = maDoorGetFromRoom(Door);
    maRoom owningToRoom = maDoorGetToRoom(Door);

    if(maDoorDestructorCallback != NULL) {
        maDoorDestructorCallback(Door);
    }
    maSafeForeachDoorPath(Door, Path_) {
        maPathDestroy(Path_);
    } maEndSafeDoorPath;
    if(owningFromRoom != maRoomNull) {
        maRoomRemoveOutDoor(owningFromRoom, Door);
#if defined(DD_DEBUG)
    } else {
        utExit("Door without owning Room");
#endif
    }
    if(owningToRoom != maRoomNull) {
        maRoomRemoveInDoor(owningToRoom, Door);
#if defined(DD_DEBUG)
    } else {
        utExit("Door without owning Room");
#endif
    }
    maDoorFree(Door);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocDoor(void)
{
    maDoor Door = maDoorAlloc();

    return maDoor2Index(Door);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyDoor(
    uint64 objectIndex)
{
    maDoorDestroy(maIndex2Door((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Door.
----------------------------------------------------------------------------------------*/
static void allocDoors(void)
{
    maSetAllocatedDoor(2);
    maSetUsedDoor(1);
    maSetFirstFreeDoor(maDoorNull);
    maDoors.Explored = utNewAInitFirst(uint8, (maAllocatedDoor()));
    maDoors.Label = utNewAInitFirst(uint64, (maAllocatedDoor()));
    maDoors.FromRoom = utNewAInitFirst(maRoom, (maAllocatedDoor()));
    maDoors.NextRoomOutDoor = utNewAInitFirst(maDoor, (maAllocatedDoor()));
    maDoors.PrevRoomOutDoor = utNewAInitFirst(maDoor, (maAllocatedDoor()));
    maDoors.ToRoom = utNewAInitFirst(maRoom, (maAllocatedDoor()));
    maDoors.NextRoomInDoor = utNewAInitFirst(maDoor, (maAllocatedDoor()));
    maDoors.PrevRoomInDoor = utNewAInitFirst(maDoor, (maAllocatedDoor()));
    maDoors.FirstPath = utNewAInitFirst(maPath, (maAllocatedDoor()));
    maDoors.LastPath = utNewAInitFirst(maPath, (maAllocatedDoor()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Door.
----------------------------------------------------------------------------------------*/
static void reallocDoors(
    uint32 newSize)
{
    utResizeArray(maDoors.Explored, (newSize));
    utResizeArray(maDoors.Label, (newSize));
    utResizeArray(maDoors.FromRoom, (newSize));
    utResizeArray(maDoors.NextRoomOutDoor, (newSize));
    utResizeArray(maDoors.PrevRoomOutDoor, (newSize));
    utResizeArray(maDoors.ToRoom, (newSize));
    utResizeArray(maDoors.NextRoomInDoor, (newSize));
    utResizeArray(maDoors.PrevRoomInDoor, (newSize));
    utResizeArray(maDoors.FirstPath, (newSize));
    utResizeArray(maDoors.LastPath, (newSize));
    maSetAllocatedDoor(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Doors.
----------------------------------------------------------------------------------------*/
void maDoorAllocMore(void)
{
    reallocDoors((uint32)(maAllocatedDoor() + (maAllocatedDoor() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Door.
----------------------------------------------------------------------------------------*/
void maDoorCopyProps(
    maDoor oldDoor,
    maDoor newDoor)
{
    maDoorSetExplored(newDoor, maDoorExplored(oldDoor));
    maDoorSetLabel(newDoor, maDoorGetLabel(oldDoor));
}

/*----------------------------------------------------------------------------------------
  Add the Path to the head of the list on the Door.
----------------------------------------------------------------------------------------*/
void maDoorInsertPath(
    maDoor Door,
    maPath _Path)
{
#if defined(DD_DEBUG)
    if(Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(_Path == maPathNull) {
        utExit("Non-existent Path");
    }
    if(maPathGetDoor(_Path) != maDoorNull) {
        utExit("Attempting to add Path to Door twice");
    }
#endif
    maPathSetNextDoorPath(_Path, maDoorGetFirstPath(Door));
    if(maDoorGetFirstPath(Door) != maPathNull) {
        maPathSetPrevDoorPath(maDoorGetFirstPath(Door), _Path);
    }
    maDoorSetFirstPath(Door, _Path);
    maPathSetPrevDoorPath(_Path, maPathNull);
    if(maDoorGetLastPath(Door) == maPathNull) {
        maDoorSetLastPath(Door, _Path);
    }
    maPathSetDoor(_Path, Door);
}

/*----------------------------------------------------------------------------------------
  Add the Path to the end of the list on the Door.
----------------------------------------------------------------------------------------*/
void maDoorAppendPath(
    maDoor Door,
    maPath _Path)
{
#if defined(DD_DEBUG)
    if(Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(_Path == maPathNull) {
        utExit("Non-existent Path");
    }
    if(maPathGetDoor(_Path) != maDoorNull) {
        utExit("Attempting to add Path to Door twice");
    }
#endif
    maPathSetPrevDoorPath(_Path, maDoorGetLastPath(Door));
    if(maDoorGetLastPath(Door) != maPathNull) {
        maPathSetNextDoorPath(maDoorGetLastPath(Door), _Path);
    }
    maDoorSetLastPath(Door, _Path);
    maPathSetNextDoorPath(_Path, maPathNull);
    if(maDoorGetFirstPath(Door) == maPathNull) {
        maDoorSetFirstPath(Door, _Path);
    }
    maPathSetDoor(_Path, Door);
}

/*----------------------------------------------------------------------------------------
  Insert the Path to the Door after the previous Path.
----------------------------------------------------------------------------------------*/
void maDoorInsertAfterPath(
    maDoor Door,
    maPath prevPath,
    maPath _Path)
{
    maPath nextPath = maPathGetNextDoorPath(prevPath);

#if defined(DD_DEBUG)
    if(Door == maDoorNull) {
        utExit("Non-existent Door");
    }
    if(_Path == maPathNull) {
        utExit("Non-existent Path");
    }
    if(maPathGetDoor(_Path) != maDoorNull) {
        utExit("Attempting to add Path to Door twice");
    }
#endif
    maPathSetNextDoorPath(_Path, nextPath);
    maPathSetNextDoorPath(prevPath, _Path);
    maPathSetPrevDoorPath(_Path, prevPath);
    if(nextPath != maPathNull) {
        maPathSetPrevDoorPath(nextPath, _Path);
    }
    if(maDoorGetLastPath(Door) == prevPath) {
        maDoorSetLastPath(Door, _Path);
    }
    maPathSetDoor(_Path, Door);
}

/*----------------------------------------------------------------------------------------
 Remove the Path from the Door.
----------------------------------------------------------------------------------------*/
void maDoorRemovePath(
    maDoor Door,
    maPath _Path)
{
    maPath pPath, nPath;

#if defined(DD_DEBUG)
    if(_Path == maPathNull) {
        utExit("Non-existent Path");
    }
    if(maPathGetDoor(_Path) != maDoorNull && maPathGetDoor(_Path) != Door) {
        utExit("Delete Path from non-owning Door");
    }
#endif
    nPath = maPathGetNextDoorPath(_Path);
    pPath = maPathGetPrevDoorPath(_Path);
    if(pPath != maPathNull) {
        maPathSetNextDoorPath(pPath, nPath);
    } else if(maDoorGetFirstPath(Door) == _Path) {
        maDoorSetFirstPath(Door, nPath);
    }
    if(nPath != maPathNull) {
        maPathSetPrevDoorPath(nPath, pPath);
    } else if(maDoorGetLastPath(Door) == _Path) {
        maDoorSetLastPath(Door, pPath);
    }
    maPathSetNextDoorPath(_Path, maPathNull);
    maPathSetPrevDoorPath(_Path, maPathNull);
    maPathSetDoor(_Path, maDoorNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void maShowDoor(
    maDoor Door)
{
    utDatabaseShowObject("ma", "Door", maDoor2Index(Door));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Path including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void maPathDestroy(
    maPath Path)
{
    maDoor owningDoor = maPathGetDoor(Path);

    if(maPathDestructorCallback != NULL) {
        maPathDestructorCallback(Path);
    }
    if(owningDoor != maDoorNull) {
        maDoorRemovePath(owningDoor, Path);
#if defined(DD_DEBUG)
    } else {
        utExit("Path without owning Door");
#endif
    }
    maPathFree(Path);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocPath(void)
{
    maPath Path = maPathAlloc();

    return maPath2Index(Path);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyPath(
    uint64 objectIndex)
{
    maPathDestroy(maIndex2Path((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Path.
----------------------------------------------------------------------------------------*/
static void allocPaths(void)
{
    maSetAllocatedPath(2);
    maSetUsedPath(1);
    maSetFirstFreePath(maPathNull);
    maPaths.Label = utNewAInitFirst(uint64, (maAllocatedPath()));
    maPaths.NextPath = utNewAInitFirst(maPath, (maAllocatedPath()));
    maPaths.PrevPath = utNewAInitFirst(maPath, (maAllocatedPath()));
    maPaths.MostRecent = utNewAInitFirst(uint8, (maAllocatedPath()));
    maPaths.Door = utNewAInitFirst(maDoor, (maAllocatedPath()));
    maPaths.NextDoorPath = utNewAInitFirst(maPath, (maAllocatedPath()));
    maPaths.PrevDoorPath = utNewAInitFirst(maPath, (maAllocatedPath()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Path.
----------------------------------------------------------------------------------------*/
static void reallocPaths(
    uint32 newSize)
{
    utResizeArray(maPaths.Label, (newSize));
    utResizeArray(maPaths.NextPath, (newSize));
    utResizeArray(maPaths.PrevPath, (newSize));
    utResizeArray(maPaths.MostRecent, (newSize));
    utResizeArray(maPaths.Door, (newSize));
    utResizeArray(maPaths.NextDoorPath, (newSize));
    utResizeArray(maPaths.PrevDoorPath, (newSize));
    maSetAllocatedPath(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Paths.
----------------------------------------------------------------------------------------*/
void maPathAllocMore(void)
{
    reallocPaths((uint32)(maAllocatedPath() + (maAllocatedPath() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Path.
----------------------------------------------------------------------------------------*/
void maPathCopyProps(
    maPath oldPath,
    maPath newPath)
{
    maPathSetLabel(newPath, maPathGetLabel(oldPath));
    maPathSetMostRecent(newPath, maPathMostRecent(oldPath));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void maShowPath(
    maPath Path)
{
    utDatabaseShowObject("ma", "Path", maPath2Index(Path));
}
#endif

/*----------------------------------------------------------------------------------------
  Free memory used by the ma database.
----------------------------------------------------------------------------------------*/
void maDatabaseStop(void)
{
    utFree(maMazes.FirstRoom);
    utFree(maMazes.LastRoom);
    utFree(maMazes.StartRoom);
    utFree(maMazes.FinishRoom);
    utFree(maRooms.Start);
    utFree(maRooms.Finish);
    utFree(maRooms.Maze);
    utFree(maRooms.NextMazeRoom);
    utFree(maRooms.PrevMazeRoom);
    utFree(maRooms.FirstOutDoor);
    utFree(maRooms.LastOutDoor);
    utFree(maRooms.FirstInDoor);
    utFree(maRooms.LastInDoor);
    utFree(maDoors.Explored);
    utFree(maDoors.Label);
    utFree(maDoors.FromRoom);
    utFree(maDoors.NextRoomOutDoor);
    utFree(maDoors.PrevRoomOutDoor);
    utFree(maDoors.ToRoom);
    utFree(maDoors.NextRoomInDoor);
    utFree(maDoors.PrevRoomInDoor);
    utFree(maDoors.FirstPath);
    utFree(maDoors.LastPath);
    utFree(maPaths.Label);
    utFree(maPaths.NextPath);
    utFree(maPaths.PrevPath);
    utFree(maPaths.MostRecent);
    utFree(maPaths.Door);
    utFree(maPaths.NextDoorPath);
    utFree(maPaths.PrevDoorPath);
    utUnregisterModule(maModuleID);
}

/*----------------------------------------------------------------------------------------
  Allocate memory used by the ma database.
----------------------------------------------------------------------------------------*/
void maDatabaseStart(void)
{
    if(!utInitialized()) {
        utStart();
    }
    maRootData.hash = 0xff596422;
    maModuleID = utRegisterModule("ma", false, maHash(), 4, 30, 0, sizeof(struct maRootType_),
        &maRootData, maDatabaseStart, maDatabaseStop);
    utRegisterClass("Maze", 4, &maRootData.usedMaze, &maRootData.allocatedMaze,
        &maRootData.firstFreeMaze, 0, 4, allocMaze, destroyMaze);
    utRegisterField("FirstRoom", &maMazes.FirstRoom, sizeof(maRoom), UT_POINTER, "Room");
    utRegisterField("LastRoom", &maMazes.LastRoom, sizeof(maRoom), UT_POINTER, "Room");
    utRegisterField("StartRoom", &maMazes.StartRoom, sizeof(maRoom), UT_POINTER, "Room");
    utRegisterField("FinishRoom", &maMazes.FinishRoom, sizeof(maRoom), UT_POINTER, "Room");
    utRegisterClass("Room", 9, &maRootData.usedRoom, &maRootData.allocatedRoom,
        &maRootData.firstFreeRoom, 6, 4, allocRoom, destroyRoom);
    utRegisterField("Start", &maRooms.Start, sizeof(uint8), UT_BOOL, NULL);
    utRegisterField("Finish", &maRooms.Finish, sizeof(uint8), UT_BOOL, NULL);
    utRegisterField("Maze", &maRooms.Maze, sizeof(maMaze), UT_POINTER, "Maze");
    utRegisterField("NextMazeRoom", &maRooms.NextMazeRoom, sizeof(maRoom), UT_POINTER, "Room");
    utRegisterField("PrevMazeRoom", &maRooms.PrevMazeRoom, sizeof(maRoom), UT_POINTER, "Room");
    utRegisterField("FirstOutDoor", &maRooms.FirstOutDoor, sizeof(maDoor), UT_POINTER, "Door");
    utRegisterField("LastOutDoor", &maRooms.LastOutDoor, sizeof(maDoor), UT_POINTER, "Door");
    utRegisterField("FirstInDoor", &maRooms.FirstInDoor, sizeof(maDoor), UT_POINTER, "Door");
    utRegisterField("LastInDoor", &maRooms.LastInDoor, sizeof(maDoor), UT_POINTER, "Door");
    utRegisterClass("Door", 10, &maRootData.usedDoor, &maRootData.allocatedDoor,
        &maRootData.firstFreeDoor, 15, 4, allocDoor, destroyDoor);
    utRegisterField("Explored", &maDoors.Explored, sizeof(uint8), UT_BOOL, NULL);
    utRegisterField("Label", &maDoors.Label, sizeof(uint64), UT_UINT, NULL);
    utRegisterField("FromRoom", &maDoors.FromRoom, sizeof(maRoom), UT_POINTER, "Room");
    utRegisterField("NextRoomOutDoor", &maDoors.NextRoomOutDoor, sizeof(maDoor), UT_POINTER, "Door");
    utRegisterField("PrevRoomOutDoor", &maDoors.PrevRoomOutDoor, sizeof(maDoor), UT_POINTER, "Door");
    utRegisterField("ToRoom", &maDoors.ToRoom, sizeof(maRoom), UT_POINTER, "Room");
    utRegisterField("NextRoomInDoor", &maDoors.NextRoomInDoor, sizeof(maDoor), UT_POINTER, "Door");
    utRegisterField("PrevRoomInDoor", &maDoors.PrevRoomInDoor, sizeof(maDoor), UT_POINTER, "Door");
    utRegisterField("FirstPath", &maDoors.FirstPath, sizeof(maPath), UT_POINTER, "Path");
    utRegisterField("LastPath", &maDoors.LastPath, sizeof(maPath), UT_POINTER, "Path");
    utRegisterClass("Path", 7, &maRootData.usedPath, &maRootData.allocatedPath,
        &maRootData.firstFreePath, 24, 4, allocPath, destroyPath);
    utRegisterField("Label", &maPaths.Label, sizeof(uint64), UT_UINT, NULL);
    utRegisterField("NextPath", &maPaths.NextPath, sizeof(maPath), UT_POINTER, "Path");
    utRegisterField("PrevPath", &maPaths.PrevPath, sizeof(maPath), UT_POINTER, "Path");
    utRegisterField("MostRecent", &maPaths.MostRecent, sizeof(uint8), UT_BOOL, NULL);
    utRegisterField("Door", &maPaths.Door, sizeof(maDoor), UT_POINTER, "Door");
    utRegisterField("NextDoorPath", &maPaths.NextDoorPath, sizeof(maPath), UT_POINTER, "Path");
    utRegisterField("PrevDoorPath", &maPaths.PrevDoorPath, sizeof(maPath), UT_POINTER, "Path");
    allocMazes();
    allocRooms();
    allocDoors();
    allocPaths();
}

