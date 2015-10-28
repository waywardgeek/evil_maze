##WaywardGeek's One-Way Door Maze Algorithm

###Introduction

A difficult maze related problem is the one-way door maze, where every door
between rooms can only be traversed in one direction.  Imagine you have an
infinite crayon, and can write on walls floors and doors, but you have no paper
to make a map.  Your job is to exit the maze in as little time as possible.  The
"random mouse" algorithm will eventually get you out, just by picking doors at
random and going as fast as you can.  However, the runtime is exponential!  Even
with a map, evil mazes where most doors take you back towards the start and away
from the finish require O(n^2) time to solve.  For a 20 room maze where every
room has two doors, one leading toward the exit, and one leading back to the
start, the random walk on average takes over 1,000,000 door transitions.  So, is
there an algorithm like the ones we use to solve regular mazes that will get us
out in O(n^2) time without a map?  Yes.  This algorithm solves the same 20 room
maze in under 500 door transitions on average.

![Image of evil maze](img/maze.jpg?raw=true "Evil Maze")
Evil Maze Example

All of the code and documentation in this evil_maze project were created by me,
Bill Cox, in 2013.  I place all of it into the public domain and disclaim any
patent rights to the algorithms.

###Algorithm

This approach embeds a "tour" of the maze, which is a loop that goes through
every door, other than the exit, some more than once.  The tour can be
represented as a continuous line on the floor through the maze that forms a
path from the start room, through every door, and back to the start room, where
it joins itself into a continuous tour.

Since the tour can go through doors multiple times, it is possible to create
tours of arbitrary length.  To be efficient, we need to trim useless portions
of the tour.  When there is a loop, meaning a portion of the tour that leaves
from a room and later comes back to the same room, and if this loop went
through no door uniquely, meaning every door in the loop is also traversed by
some other part of the tour, then this is a useless loop.  It can be deleted
from the tour by erasing the path through the loop, and joining the two
dangling ends of the tour that result in the room where the loop was
discovered.

Suppose we are in a maze with N rooms and D one-way doors.  Once useless loops
have been trimmed, the tour has at most D loops, and each loop traverses at
most N doors.  This algorithm creates this tour in time proportional to the
tour length, and thus has runtime order O(N*D), which is runtime-order optimal.

A simple hack to enable us to do loop operations is to mark each door as we
traverse it with a counter value.  Unless doing a loop operation, we overwrite
any existing counter value on a door we traverse with the new one, and
increment the counter.  This way, when we discover a loop, we can re-traverse
the loop by traversing through the highest numbered doors in each room until we
return to the room where we discovered the loop.

We do the following "loop operations":

- Create labeled loop
- Delete labeled loop
- Merge new loops into old ones, changing the label along the new loop to the old loop label

We start by exploring through unexplored doors, labeling doors with the counter
value as we go, incrementing the counter each time.  When we enter a room we've
seen before, there are two cases:

case 1) This room does not contain any part of the tour, just door labels
case 2) The tour already goes through this room 1 or more times.

In case 1, we have to create a new loop on the floor.  Record this loop
by drawing a line on the floor through the doors and rooms in the loop, back to
the room where we detected the loop.  This can be done by simply traversing
highest numbered doors in each room until we come back to the room where we
detected the loop.

At that point, we are in a room with part of the tour present, so goto case 2.

In case 2, there are 2 sub cases:

subcase 1) There are multiple lines through the room with different labels
subcase 2) Each line through the room has the same label

In subcase 1, it's time to merge all the different tour pieces.  It is
generally more efficient to merge the new loops into the old tour, since they
will typically be smaller.  Therefore, for each line through the room, merge it
into the oldest one with the smallest label.  This is done by following the
line, relabeling as we go, until we come back to the room where we started, and
then splicing the new loop into the old tour:

Before splicing:

Oldest path: door 89 ---> 123 ---> door 90
New loop: door 410 ---> 1093 ---> door 411

After splicing:

door 89 ---> 123 ---> door 411
door 410 --> 123 ---> door 90

At this point, we are in a Case 2, but the room has only one tour, though it
occurs multiple times.  This is just subcase 2, so goto subcase 2.

In subcase 2, we are in a room we have seen before, with lines from the tour,
all with the same label.  If there is an unexplored door, increment the counter
and label it, and then traverse it.  Otherwise, pick a line from the tour, and follow it, incrementing the door counter and relabling doors as we go, until one of two sub-sub-cases happens:

sub-sub-case 1) we find an unexplored door.  This is just subcase 2, where we have an unexplored door.
sub-sub-case 2) we come back to the room where we started following the tour.

In sub-sub-case 2, we've detected a useless loop.  Simply erase it, and splice
the two dangling lines in the room where we started together.  Suppose the useless loop started through door 1234:

Before deletion:

door 1233 ---> 123 ---> door 1234
door 3288 ---> 123 ---> door 3289

The usless loop start through door 1234, and ends through door 3288.

After deletion:

door 1233 ---> 123 ---> door 3289

Now we are back in subcase 2, where we're in a room we've seen before.  As
before, follow the tour, looking for an unexplored door.  Each time we do this,
we either find an unexplored door, or delete a useless loop.  At some point,
since there are a finite number of useless loops, we will find an unexplored
door.  This guarentees the algorithm will complete by successfully finding the
exit door.

###Efficiency

If there are D doors and N rooms, then there are at most D loops of at most N
doors each, making the tour size O(N*D).  However, an unexplored door will
be found on each traversal of any of the loops until there are no more, at which
point the loop is deleted.  On average, a door will be found in O(N)
transitions when following the tour.  The expected runtime is then O(N*D), or
more informally, it's O(N^2) if N the "size" of the maze.

###Improvements

Various improvements are possible.  For one thing, we don't have to build loops
when we don't discover any rooms without paths that have unexplored doors.
Also, we could combine creating a loop and splicing paths in one transition of
the loop instead of two whenever we create a loop in a room that already has a
path.  Another improvement would be doing a better job of detecting when we have
to reset our startLabel, which is used to detect the start of a loop, since we
don't have to do it every time we delete a loop or splice paths together.  Some
of this is already done in maze.c, for about 2X speed improvement.
