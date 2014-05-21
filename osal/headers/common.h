#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif



// ------------------------------------------------------------
// define basic types
// ------------------------------------------------------------
#ifndef __cplusplus
  typedef unsigned char bool;
  extern bool False;
  extern bool True;
#endif
  typedef unsigned char byte;     
  typedef unsigned int uint;      
  typedef unsigned long ulong;    
  typedef unsigned short ushort;  
  typedef unsigned long long ull;
  /*!
   * Zero memory locate {m} for {lm} bytes.
   */
  void zeromem(void * m, uint lm);
  /*!
   * Copy {l} bytes from source {s} to destination {d}
   */
  void mcpy(void * d, void * s, uint l);
  /*!
   * [Deprecated] C String are are limitted thus a more refined solution should be used. 
   */
  int osal_strlen(const char * s);
  /*!
   * [Deprecated] But necessary. We have not yet written a sprintf like function for String.h
   */
  int osal_sprintf(char * b, const char * fmt, ... );

  /*
   * Compare {a} and {b} as arrays of bytes. If a is "bigger" return
   * 1, if {a} and {b} are equal return 0 otherwise return -1.
   */
  int mcmp(void * a, void * b, uint l);

  /*
   * Convert an unsigned long long to bytes in Big Endian byte order
   * and store them in {out} which must be at least sizeof(l) in size.
   */
  void l2b(ull l, byte * out);

  /*
   * Convert a sequence of bytes in {data} to an unsigned long
   * long. {data} must hold at least sizeof(ull) bytes with the wanted
   * value stored as Big Endian byte order.
   */
  ull b2l(byte * data);

#ifdef __cplusplus
}
#endif



#endif
