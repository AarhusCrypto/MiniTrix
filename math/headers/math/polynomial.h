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


Created: 2013-07-22

Author: Rasmus Winther Lauritsen, rwl@cs.au.dk

Changes: 
2013-07-22 15:28: Initial version created
*/

#ifdef __cplusplus
extern "C" {
#endif
/*!
 * Author Rasmus Lauritsen (c) 2007
 * \file polynomial.h
 *
 */
#ifndef __POLYNO__
#define __POLYNO__



/*!
 * \brief               This is how I represent a polynomial of deg. 8.
 *
 * You cannot change this representation with out changing the entire
 * polynomial implementation sorry. 
 *
 * My polynomial.c represent the Z_2[X]/<m(x)> where
 * m(x)=x^8+x^4+x^3+x+1, ie. the Rijndael polynomial or
 * the smallest irreducible polynomial of degree 8 in Z_2[X].
 * 
 */
typedef unsigned char polynomial;



/*!
 * \brief               Pretty print a polynomial ie. x^8 + ... + 1,
 *                      to stdout.
 *
 * \param               The polynomial to print.
 */
void print_polynomial(polynomial p);



/*!
 * \brief               Multiply two polynomials module m(x).
 *
 * \param p1            The first polynomial.
 * \param p2            The second polynomial.
 * \return              The product p1 * p2 module m(x).
 */
polynomial multiply(polynomial p1, polynomial p2);



/*!
 * \brief               Add two polynomials.
 *
 * \param p1            The first polynomial.
 * \param p2            The second polynomial.
 * \return              The sum p1 + p2.
 */
polynomial add(polynomial p1, polynomial p2);


/*!
 * \brief               Add two polynomial vectors
 *
 * \param r             Vector for the result 
 * \param v1            First operand
 * \param v2            Second operand
 * \param lv            The length of v1, v2 and r
 *
 */
void polynomial_add_vectors(polynomial * r, polynomial * v1, polynomial * v2, 
			    unsigned int lv);


/*!
 * \brief               Subtract two polynomials.
 * \param p1            The first polynomial.
 * \param p2            The second polynomial.
 * \return              The difference p1 - p2.
 */
polynomial sub(polynomial p1, polynomial p2);



/*
 * \brief               Invert p, ie. calculate 1/p.
 *
 * \param               The polynomial whos inverse to return.
 * \return              The multiplicative inverse of p (1/p).
 */
polynomial inverse(polynomial p);



/*!
 * \brief               Calculate the power of p ie. p^(exponent).
 *
 * \param p             A polynomial p.
 * \param exponent      The exponent to which p is raised.
 * \return              p^(exponent).
 */
polynomial pol_pow(polynomial p, int exponent);


/*!
 * \brief               Initialize polynomial divide and multiplication tables.
 *
 */
void init_polynomial();




/* 
 * \brief               Free the memory used by the division table and 
 *                      the multiplication table
 */
void teardown_polynomial();
#endif

#ifdef __cplusplus
}
#endif
