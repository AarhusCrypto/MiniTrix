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
2013-07-22 15:32: Initial version created
*/

#ifdef __cplusplus
extern "C" {
#endif
/*!
 * \file matrix.h
 *
 *
 * Author Rasmus Lauritsen.
 *
 *
 * The matrix module (matrix.c) is implementing these functions. There
 * are function to find the inverse matrix, row, column and scalar
 * operations.
 *
 */
#include "polynomial.h"
#include <common.h>
#include <osal.h>
#ifndef IDAMATRIX
#define IDAMATRIX


/*!
 * \brief               This structure is needed for dianostics
 *
 * This structure keeps track of how many matricies are allocation
 * and how much memory the use. 
 */
struct matrix_allocation_table
{
  int n_matrix;  /* total number of allocated matricies */
  int mem_usage; /* total memusage */
};


/*!
 * \brief               This structure is a matrix. 
 */
typedef struct { 
  OE oe;
  int height; 
  int width; 
  polynomial **content; 
} MATRIX;




/*! 
 * \brief               Create a new height by width matrix with all 
 *                      entries set to zero 
 * 
 * \param height        The height of the new matrix
 * \param width         The width of the new matrix
 */
  MATRIX * new_matrix(OE oe, int height, int width);



/*! 
 * \brief               Free the memory used by m 
 * 
 * Fries the matrix pointed to by m.
 *
 * \param m             The matrix to destroy
 */
void destroy_matrix(MATRIX * m);



/*!
 * \brief               Initialize bookeeping for matricies
 * 
 * Sets the m_table entries to zero.
 */
void init_matrix();



/*!
 * \brief                Return the height of the matrix m 
 *
 * \param m              Matrix whos height to return
 * \return               Return the height as int
 */
int matrix_getheight(MATRIX * m);



/*!
 * \brief                Return the width of the matrix m 
 *
 * \param m              Matrix whos width to return
 * \return               Return the width as int
 */
int matrix_getwidth(MATRIX * m);



/*!
 * \brief                Returns the inverted of matrix m 
 *
 * \param m              The matrix to invert.
 * \return               A pointer to a new MATRIX structure representing
 *                       the inverse of m. If m is singular (ie. det(m)=0)
 *                       NULL is returned.
 */
MATRIX * matrix_invert(MATRIX * m);



/*!
 * \brief               Calculate the determinant of a matrix 
 *
 * \param m             The matrix whos determinant we calculate
 * \return              The the determinant of m as int.
 */
int determinant(MATRIX * m);



/*!
 * \brief               Pretty print the matrix m to stdout
 *
 * \param m             The matrix to be printed.
 */
void print_matrix(MATRIX * m);



/*!
 * \brief               Set the row'th row of m to actual_row   
 *                      (actual_row is assumed to be of length 
 *                       width of m) 
 *
 * \param m             The matrix whos row to set
 * \param row           An index between 0 and height(m)-1 indicating a row
 * \param actual_row    A pointer to an array of width(m) polynomials
 */
void matrix_setrow(MATRIX * m, int row, polynomial * actual_row);



/*!
 * \brief               Set the entry to val 
 *
 * \param m             The matrix whos entry to set
 * \param row           Row of the entry
 * \param col           Col of the entry
 * \param val           The new value for this entry
 */
void matrix_setentry(MATRIX * m , int row, int col, polynomial val);



/*!
 * \brief               Multiply two matrices returns new 
 *                      matrix with results.
 *
 * \param m1            Left matrix in multiplication
 * \param m2            Right matrix in multiplication
 * \return              The matrix product m1 (x) m2 or NULL if the dimensions
 *                      are wrong (ie. width(m1) != height(m2)).
 */
MATRIX * matrix_multiplication(MATRIX * m1, MATRIX * m2);



/*! 
 * \brief               Get an entry 
 *
 * \param m             The matrix to access
 * \param row           Row of the entry to access
 * \param col           Col of the entry to access
 * \return              the value stored in m[row][col].
 */
polynomial matrix_getentry(MATRIX * m, int row, int col);



/*!
 * \brief               Make a submatrix where the row'th row is removed
 *                      and the col'th column is removed.
 *
 * \param m             Initial n by m matrix.
 * \param row           The row to remove
 * \param col           The column to remove
 * \return              A pointer to a new MATRIX structure that represent
 *                      a n-1 by m-1 matrix with the entries from m except
 *                      for the col'th column and the row'th row.
 */
MATRIX * make_submstrix(MATRIX * m, int row, int col);



/*!
 * \brief               Print info on matricies 
 *
 * This function utilize the m_table to reveal how much memory is
 * used. It prints to stdout.
 */
void matrix_info();




/*!
 * \brief               Matrix to flat mem converts a matrix m into an linear array of bytes
 *                      it stats with entry 0,0 and continue column by column. That is entry
 *                      0,0 is followed by 1,0 ... height-1,0 and so on.
 * 
 * \param m             The matrix to flatten.
 * \return              A pointer to width(m)*height(m) array of unsigned char with
 *                      the entries from m.
 */
  unsigned char * matrix_to_flatmem(MATRIX *m);

  /*!
   * \brief
   *                    Matrix from flatmem inverts the {matrix_to_flatmem} function above.           
   *
   * \param oe          Operating Environment for allocating the new matrix
   * \param d           Data from which entries in the new matrix will be set
   * \param h           The height dimension of the matrix
   * \param w           The width dimension of the matrix
   *
   * \return            fresh allocated matrix with the content of the h*w-1 bytes in 
   *                    d as content. m_ij = d_{j*h+i}.
   */
  MATRIX * matrix_from_flatmem(OE oe, unsigned char * d, uint h, uint w);

/*!
 * \brief               Solves the equation Ax=b given the
 *                      LU-decomposition of n x n matrix A
 *
 * \param L             Lower triangular unit matrix.
 * \param U             Upper triangular matrix.
 * \param b             1 x n vector b.
 *
 * \return              An 1 x n vector x such that Ax = b
 */
MATRIX * LUSolve(MATRIX * L, MATRIX * U, MATRIX * b);


/*!
 * \brief               LUInverse uses LUDecomposition to invert the
 *                      matrix A. However, this requires A to be an
 *                      positive definite matrix.
 *
 * \param               An interable positive definite matrix A.
 * \return              An matrix B such that AB=I=BA.
 */
MATRIX * LUInverse(MATRIX * A);

/*!
 * \brief               Given the LUP decomposition of a matrix A and
 *                      a vector b, find the vector x such that A x =
 *                      b.
 * 
 * \param LUA           Matrix A=[LU]
 * \param P             A permutation column vector P.
 * \param b             Column vector b.
 *
 * \return x            Column vector x such that A x = b.
 */
MATRIX * LUPSolve(MATRIX *LUA, MATRIX *P, MATRIX * b);

/*!
 * \brief               Given a matrix A and a vector b find a vector
 *                      x such that Ax = b.
 *
 * \param A             n x m Matrix 
 * \param b             1 x n Column Vector
 */
MATRIX * Solve(MATRIX *A, MATRIX *b);


/*!
 * \brief               Given a matrix A find its inverse.
 *
 * \param A             An invertable square matrix.
 *
 * \return              Matrix A' such that AA' = A'A = I
 */
MATRIX * LUPInverse(MATRIX * A);


/*!
 * \brief               Extract a pointer to the vector in A
 *
 * \param A             A column vector ( n by 1 )
 * \param length        The length of the returned vector (optional)
 *
 * \return              pointer to the state of A of the first column 
 *                      vector
 *
 *
 */
  polynomial * matrix_topoly(MATRIX * A, int * length);

  /*!
   * \brief             Get a copy of a sub-segment in m.
   *
   * \param m           Matrix m to extract segment from
   * \param top         The upper row to start from
   * \param left        The left most row to start from
   * \param bottom      The lower row to end at
   * \param right       The right most row to end at
   *
   * \return            Fresh matrix containing the submatrix of m
   *                    starting in (top,left) and ending in (botton,right).
   *
   * Note! Indecies for entries in a matrix starts with (0,0) in the upper 
   *       left corner.
   */
  MATRIX * matrix_segment(MATRIX * m, int top, int left, int bottom, int right );

  /*!
   * \brief             Copy the matrix A
   *
   * \param             Matrix {A} to copy
   *
   * \return            Fresh matrix such that matrix_getentry(A,i,j) == 
   *                    matrix_getentry(RESULT,i,j);
   *
   */
  MATRIX * matrix_copy(MATRIX * A);

#endif

#ifdef __cplusplus
}
#endif
