#ifndef STRING_H
#define STRING_H

#include "common.h"
#include "osal.h"

/*!
 * << Interface >>
 *
 */

typedef struct _string_ {
  /*!
   * Number of characters this string contains. (not the number of
   * bytes)
   */
  uint (*length)(void);
  /*!
   * returns true if {s} is a suffix of this.
   */
  bool (*endswith)(struct _string_ * s);
  /*!
   * returns true if {s} is a prefix of this.
   */
  bool (*startswith)(struct _string_ * s);
  /*!
   * Concatenate s to this string.
   */
  void (*concat)(struct _string_ * s);
  /*!
   * Return a newly allocated string which contains the characters 
   * in the range from (incl) {start} to {end} (not incl).
   */
  struct _string_ * (*substr)(uint start, uint end);
  /*!
   * Convert the representation of this string to c_str.
   */
  const char * (*cstr)(void);

  /*!
   * Find position in this string where {s} appears.
   * -1 is returned if {s} does not appear.
   */
  int (*index_of)(struct _string_ * s);
  
  
  void * impl;
} * String;

#endif
