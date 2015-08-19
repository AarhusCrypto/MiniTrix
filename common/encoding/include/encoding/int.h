#include <common.h>
#ifndef INT_H
#define INT_H

#ifdef __cplusplus
extern "C" {
#endif

  void i2b(int v, byte * o);
  
  int b2i(byte * b);

  /**
   * atoui converts a cstr to an unsigned integer.
   *
   * \param s - cstr holding a number
   *
   * \param res- the result out
   *
   * \return the number of characters read from {s}. e.g. atoui("123",&res) will return 3 and report
   * 123 in variable uint res.
   */
  uint atoui(byte * s, uint * res);


#ifdef __cplusplus
}
#endif

#endif
