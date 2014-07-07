#include <common.h>
#ifndef INT_H
#define INT_H

#ifdef __cplusplus
extern "C" {
#endif

  void i2b(int v, byte * o);
  
  int b2i(byte * b);

#ifdef __cplusplus
}
#endif

#endif
