/*
 * Pool memory allocation
 *
 * Authors:
 *   Stéphane Gimenez <dev@gim.name>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

// not thread safe (a pool cannot be shared by threads safely)

/*
-- principle:

 - user operations on a pool of objects of type T are:
   - T *draw() : obtain a unused slot to store an object T
   - void drop(T *) : realease a slot

-- implementation:

 - a pool for objects T is:

   * blocks[64] : an array of allocated blocks of memory:
     |---0--> block with capacity 64
     |---1--> block with capacity 64
     |---2--> block with capacity 128
     |---3--> block with capacity 128
     |---4--> block with capacity 256
     |---5--> block with capacity 256
     |---6--> block with capacity 512
     |---7--> not yet allocated
     :
     |---k--> not yet allocated (future capacity ~ 2^(6+k/2))
     :
     '--63--> not yet allocated
   * cblock : the index of the next unallocated block (here 7).
   * next : a pointer to an unused slot inside an allocated bloc

 - the first bytes of an unallocated slot inside a bloc are used to store a
   pointer to some other unallocated slot. (this way, we keep a list of all
   unused slots starting at <next>)

 - insertions and deletions in this list are done at the root <next>.
   if <next> points to NULL (no slots are availlable) when a draw()
   operation is performed a new block is allocated, and the unused slots
   list is filled with the allocated slots.

 - memory is freed only at pool's deletion.

*/

#include <stdlib.h>

template <typename T>
class pool {

 public:

  pool()
  {
      cblock = 0;
      size = sizeof(T) > sizeof(void *) ? sizeof(T) : sizeof(void *);
      next = NULL;
      for (int k = 0; k < 64; k++) {
          block[k] = NULL;
      }
  }

  ~pool()
  {
      for (int k = 0; k < cblock; k++) {
        free(block[k]);
      }
  }

  T *draw()
  {
    if (!next) addblock();
    void *p = next;
    next = *(void **)p;
    return (T *) p;
  }

  void drop(T *p)
  {
    *(void **)p = next;
    next = (void *) p;
  }

 private:

  int size;
  int cblock;
  void *block[64]; //enough to store unlimited number of objects, if 64 is changed: see constructor too
  void *next;

  void addblock()
    {
      int i = cblock++;
      int blocksize = 1 << (6 + (i/2));
      //printf("pool allocating block: %d (size:%d)...", i, blocksize);//debug
      block[i] = (void *)malloc(blocksize * size);
      if (!block[i]) throw std::bad_alloc();
      char *p = (char *)block[i];
      for (int k = 0; k < blocksize - 1; k++)
	{
	  *(void**)p = (void *)(p + size);
	  p += size;
	}
      *(void **)p = next;
      next = block[i];
      //printf("done\n");//debug
    }

};

