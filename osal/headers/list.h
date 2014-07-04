/*!
 * << List >>
 *
 * Interface.
 *
 */

#ifndef LIST_H
#define LIST_H

#include <osal.h>

typedef struct _list_ {
  /*!
   * Get the i'th element of this list if the list is that long. NULL
   * otherwise.
   */
  void * (*get_element)(uint i);

  /*!
   * Add {e} to the end of this list.
   */
  void (*add_element)(void * e);

  /*!
   * Delete the i'th element of this list if the list is that long.
   */
  void (*rem_element)(uint i);

 /*!
   * The number of element in this list.
   */
  uint (*size)(void);

  void * impl;
} * List;

#endif
