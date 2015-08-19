/*
Copyright (c) 2013, Rasmus Lauritsen, Aarhus University
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
   This product includes software developed by the Aarhus University.
4. Neither the name of the Aarhus University nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY Rasmus Lauritsen at Aarhus University ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Rasmus Lauritsen at Aarhus University BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Created: 2013-07-15

Author: Rasmus Winther Lauritsen, rwl@cs.au.dk

Changes: 
2013-07-15 18:04: Initial version created
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "math/polynomial.h"
#include <osal.h>

polynomial dividetable[256];
polynomial multiplicationtable[256][256];
char my_masktable[8] = {((unsigned char)0x1),
			((unsigned char)0x2),
			((unsigned char)0x4),
			((unsigned char)0x8),
			((unsigned char)0x10),
			((unsigned char)0x20),
			((unsigned char)0x40),
			((unsigned char)0x80)};

char isInitialized = 0;
const int numberOfElements = 256;

polynomial hard_multiply(polynomial p1, polynomial p2)
{
  polynomial p = 0;
  polynomial counter;
  polynomial hi_bit_set;
  for (counter = 0; counter < 8; counter++)
    {
      if ((p2 & 1) == 1)
        p ^= p1;
      hi_bit_set = (p1 & 0x80);
      p1 <<= 1;
      if (hi_bit_set == 0x80)
	p1 ^= 0x1b; /*x^4 + x^3 + x + 1 */
      p2 >>= 1;
    }
  return p;
}


void init_mult_table()
{
  int i, j;
  //  multiplicationtable = (polynomial **)oe->getmem(sizeof(polynomial *)*256);
  //  for (i = 0;i<256;i++) 
  //    multiplicationtable[i] = (polynomial *)oe->getmem(sizeof(polynomial)*256);

  for (i = 0;i < 256;i++)
    {
      for (j = 0;j < 256;j++)
	{
	  multiplicationtable[i][j] = hard_multiply(i,j);
	}
    }
}

void initialize_polynomial_dividetable()
{
#ifdef DEBUG
  printf("DEBUG: polynomial.c initializing divide table \n ");
#endif
  int n, i, j;
  //  dividetable =  (polynomial *)oe->getmem(sizeof(polynomial)*numberOfElements);
  n = numberOfElements;
  for(i = 0;i < n;i++) dividetable[i] = (polynomial)0;
  
  for (i = 0;i < n;i++)
    {
      for(j = 0; j < n;j++)
	{
	  polynomial p1 = (polynomial)i;
	  polynomial p2 = (polynomial)j;
	  polynomial product = multiply(p1,p2);
	  if (product == 1)
	    {
	      dividetable[p1] = p2;
	    }
	  if (product == 0 && p1 != 0 && p2 != 0)
	    {
	      //printf("FATAL ERROR: invalid field. Multiplication of %2x and %2x is zero. \n",p1,p2);
	      exit(-1);
	    }
	}
    }
}


void teardown_polynomial()
{ }

void init_polynomial()
{
  if (!isInitialized)
    {
#ifdef DEBUG
      printf("DEBUG: polynomial.c initializing mult table and dividetable\n");
#endif
      init_mult_table();
      initialize_polynomial_dividetable();
      isInitialized = 1;
    }
}

  /*
void print_polynomial(polynomial p)
{
  int i = 0;
  char first = (char)1;
  for (i = 0;i < 7;i++)
    if ((p & my_masktable[i]) != 0) 
      {
	if (first)
	  {
	    printf("x^(%i)",i);
	    first = 0;
	  }
	else
	  printf("+x^(%i)",i);
      }
}
  */

polynomial add(polynomial p1, polynomial p2)
{
  return p1 ^ p2;
}

polynomial sub(polynomial p1, polynomial p2)
{
  return p1 ^ p2;
}

polynomial multiply(polynomial p1, polynomial p2)
{
#ifdef DEBUG
  if (multiplicationtable != (void *)0)
    return multiplicationtable[p1][p2];
  else {
    // printf("\n\n\n\t\t WARNING \n \t\t Multiplication table not initialized \n\n");
  }
  return 0;
#else
    return multiplicationtable[p1][p2];
#endif
}



polynomial inverse(polynomial p)
{
  if (!isInitialized)
    {
      initialize_polynomial_dividetable();
    }
  return dividetable[p];
}


polynomial pol_pow(polynomial p, int exponent)
{
  polynomial res;
  if (exponent == 0) return (polynomial)1;
  if (exponent < 0 ) 
    {
      //printf("polynomial.c::pol_pow: Negative exponent not allowed. Used inverse followed by multiply.\n");
      return (polynomial)1;
    }
  res = p;
    
  while (exponent-- > 1)
    res = multiply(res,p);
  return res;
}


void polynomial_add_vectors(polynomial * r, 
				      polynomial * v1, 
				      polynomial * v2, unsigned int lv) {
  int i = 0;
  for(i=0;i<lv;i++) {
    r[i] = add( (polynomial)v1[i],(polynomial)v2[i]); // GF2^8 add
  }
}



#ifdef __cplusplus
}
#endif

