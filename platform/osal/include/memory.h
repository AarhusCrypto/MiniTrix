/**  
 * RWL OS VERSION -1
 * Author Rasmus Winther Lauritsen
 *
 *
 * Memory.h is the interface for some physical memory. There can be
 * more memories with e.g. different characteristics.
 */
#ifndef _MEMORY_H_
#define _MEMORY_H_
#include "common.h"

struct _memory_
{
  /*
   * Allocate size byte of memory.
   */
  void*(*alloc)(uint size);
  
  /*
   * Free the memory pointed to by the given pointer.
   */
  void(*free)(void * memory);

  /*
   * Free all memory ever allocated by this memory. 
   */
  void(*free_all)(void);

  /*
   * Return the number of bytes free in this memory
   */ 
  uint (*free_size)(void);
};

typedef struct _memory_ * Memory;

#ifndef NULL
#define NULL ((void*)0)
#endif
#endif
