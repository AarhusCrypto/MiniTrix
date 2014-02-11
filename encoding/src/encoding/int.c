#include "encoding/int.h"

inline void i2b(int v, byte * o) {

  if (!o) return;

  o[0] = (byte) (( v & 0xFF000000 ) >> 24);

  o[1] = (byte) (( v & 0x00FF0000 ) >> 16 );

  o[2] = (byte) (( v & 0x0000FF00 ) >> 8 );

  o[3] = (byte) (( v & 0x000000FF ) ) ;

  return;
}

inline int b2i(byte * b) {
  return  ( b[0] << 24 ) +( b[1] << 16 ) + ( b[2] << 8 ) + ( b[3] );
}
