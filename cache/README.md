cache simulation:

learnt that i am taking cache and memory storage for granted. a lot.

how it works:
using LIFO (last in first out), we have to simulate a cache in which
different blocks of memory are inputted at "random" times. each block
has an associated tag and time of when it was inputted. when cache
fills up, we have to find the most recently used cache and swap it out
with the new block

interesting stuff:
many mmu's use different types of data structure handlers. this one
was LIFO, but another widely used one is FIFO (first in first out).
efficiency depends on the specifications of the system. nice.