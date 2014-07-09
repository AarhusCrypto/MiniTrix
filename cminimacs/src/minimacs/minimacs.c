/*!
 * Pure C implementation of the MiniMacs protocol.
 *
 *
 */
#include "minimacs/minimacs.h"


Observer Observer_DefaultNew( OE oe,  void (*fn)(void *data) ) {
  Observer res = (Observer)oe->getmem(sizeof(*res));
  if (!res) return 0;

  res->notify = fn;

  return res;
}




