/*

Copyright (c) 2013, Rasmus Zakarias, Aarhus University
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software

   must display the following acknowledgement:

   This product includes software developed by Rasmus Winther Zakarias 
   at Aarhus University.

4. Neither the name of Aarhus University nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY Rasmus Zakarias at Aarhus University 
''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Rasmus Zakarias at Aarhus University BE 
LIABLE FOR ANY, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Created: 2014-10-26

Author: Rasmus Winther Zakarias, rwl@cs.au.dk

Changes: 
2014-10-26 14:26: Initial version created
*/
#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum __error_codes__ {
    EC_OK = 0,
    EC_MEM,
    EC_ARG,
    EC_IO
  } EC;

#define ERR(OE,MSG,...) (OE)->p("%s:%d " MSG,__FILE__,__LINE__,##__VA_ARGS__)

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
   * Zero memory locate at {m} for {lm} bytes ahead.
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
