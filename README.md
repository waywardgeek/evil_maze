WaywardGeek's One Way Door Maze Algorithm
=========================================

Introduction
------------

A difficult maze related problem is the one-way door maze, where every door
between rooms can only be traversed in one direction.  Imagine you have an
infinite crayon, and can write on walls floors and doors, but you have no paper
to make a map.  Your job is to exit the maze in as little time as possible.  The
"random mouse" algorithm will eventually get you out, just by picking doors at
random and going as fast as you can.  However, it's runtime is exponential!
Even with a map, evil mazes where most doors take you back towards the start and
away from the finish require O(n^2) time to solve.  For a 21 room maze where
every room has two doors, one leading toward the exit, and one leading back to
the start, the random walk on average takes over 1,000,000 door transitions.
So, is there an algorithm like the ones we use to solve regular mazes that will
get us out in O(n^2) time without a map?  Yes.  This algorithm solves the same
21 room maze in under 1,000 door transitions on average.

Algorithm
---------

1. We keep a door transition count that we increment every time we are about to
   go through a door.  We write this count on the door when we go through.
2. Every time we explore through a series of unexplored rooms, and arrive at a
   room we've seen before (which we can see from the door labels), we create a new
   "loop" consisting of the chain of newly explored rooms and the most recently
   visited rooms leading from the loop point back to itself.  Just follow the chain
   of largest numbered doors (without incrementing our counter or labeling doors)
   drawing an arrow from the door you entered through to the door you leave
   through, and label the arrow with the counter value.  Don't increment the
   counter while creating the loop, so the entire loop gets the same label.
3. While the current room has no unexplored doors, we first remember the current
   count value, and then do the following:
    - Merge any new loops seen in the room into old ones.  This does require
      that we traverse the newer loop, changing it's labels to the older loop
      label.  Also, we'll need to redraw the arrows so that the old path feeds
      into the new one and vise versa.  If we merged any loops, update the
      remembered count value to the current count.
    - If there is an unexplored door, explore through it.  Go back to step 2.
    - If the room has a door label larger than the remembered count, we must
      have traversed a whole loop without finding any unexplored doors.  There
      is no reason to keep track of this loop, so delete it, which requires
      another loop traversal.  Don't update the counter or label doors while
      deleting the loop.  After deleting it, draw an arrow from the door that
      used to lead into the loop to the door the loop used to go through when
      we'd finished the loop.  Since the same path can go through the same door
      multiple times, we need to mark the one used most recently when this
      occurs, so we know which one to delete later.

Efficiency
----------

If there are D doors and N rooms, then there are at most D loops of at most N
doors each, making the loop graph size O(N*D).  However, an unexplored door will
be found on each traversal of any of the loops until there are no more, at which
point the loop is deleted.  On average, a door will be found in O(N)
transitions.  The expected runtime is then O(N*D), or more informally, it's
quadratic in terms of the size of the maze.

Improvements
------------
Various improvements are possible, but I'd like to keep this algorithm simple.
For one thing, we don't have to build loops when we don't discover any rooms
without paths that have unexplored doors.  It is a significant complication,
however.  Also, we could combine creating a loop and splicing paths in one
transition of the loop instead of two.  Another improvement would be doing a
better job of detecting when we have to reset our startLabel, as we don't have
to do it every time we delete a loop or splice paths together.  However, all of
these improvements provide only a constant factor speedup, while complicating
the code.
