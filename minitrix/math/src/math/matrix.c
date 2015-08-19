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
2013-07-15 17:58: Initial version created
*/

/* matrix.c
 *
 *
 * Author Rasmus Lauritsen.
 *
 * All rights reserved.
 *
 */
#ifdef __cplusplus
extern "C" {
#endif 

#include "math/matrix.h"
#include "math/polynomial.h"
#include <osal.h>
#ifdef _GUI_
#define NOTIFY_GUI(String,Procentage) notify_listeners(String,Procentage);
#else
#define NOTIFY_GUI(String,Procentage) ;
#endif

#ifndef NULL
#define NULL (void*)0
#endif

#define ERR(OE,MSG,...) {				\
  char _____m[512] = {0};				\
  osal_sprintf(_____m,MSG,##__VA_ARGS__);		\
  (OE)->p(_____m);					\
}						\

/*!
 * \brief               Concrete instance of matrix_allocation_table.
 */
static struct matrix_allocation_table m_table;


MATRIX * make_submatrix(MATRIX * m,int rm_row,int rm_col);

void init_matrix()
{
  m_table.n_matrix = 0;
  m_table.mem_usage = 0;
}


  MATRIX * new_matrix(OE oe, int height, int width) {
    MATRIX * m = (MATRIX *) oe->getmem(sizeof(MATRIX));
    int j;
    if (m != NULL && height > 0 && width > 0){ /* m was allocated */
      m->content = (polynomial **)oe->getmem(sizeof(polynomial *)*height);
#ifdef DEBUG
      printf("DEBUG: matrix.c::new_matrix size of pol * = %i height is %i\n",sizeof(polynomial *), height);
#endif
      
      if (m->content != NULL) { /* rows was allocated  */
	for(j = 0; j < height;j++) { /* allocate width cols for each row */
	  int k;
	  m->content[j] = (polynomial *)oe->getmem(sizeof(polynomial)*width);
	  if (m->content[j] == NULL) { /* col for row allocation failed ! ABORTING */
	    int z = 0;
	    for (z = 0; z < j;z++)
	      oe->putmem(m->content[z]);
#ifdef DEBUG
	    printf("matrix.c:new_matrix: Out of memory,"		\
		   " could not allocate col in row %i. ABORTING \n",j);
	    
#endif
	    oe->putmem(m->content);
	    return NULL;
	  }
	  for (k = 0;k < width;k++) m->content[j][k] = (polynomial)0;
	}
      } else { /* failed to allocate rows */
#ifdef DEBUG
	printf("matrix.c:new_matrix: Out of memory, could not allocate rows for matrix. ABORTING \n");
#endif
	return NULL;
      }
    } else  {	/* failed to allocate matrix */
#ifdef DEBUG
      printf("matrix.c:new_matrix: could not allocation matrix, ABOUTING h %i w %i p %p\n",height,width,(void *)m);
#endif
      if (m) oe->putmem(m);
      return NULL;
    }
    /* success full matrix allocation */
    m_table.n_matrix++;  
    m_table.mem_usage += sizeof(MATRIX) + sizeof(polynomial *)*height + height*width;
    m->height = height;
    m->width = width;
    m->oe = oe;
  return m;
}



void destroy_matrix(MATRIX * m) {
  if (m != (MATRIX *)0) {
    OE oe = m->oe;
    int width = m->width;
    int height = m->height;
    int row = 0;

    for (row = 0; row < height;++row) {
      oe->putmem(m->content[row]);
    }
    oe->putmem(m->content);
    oe->putmem(m);
    m_table.n_matrix--;
    m_table.mem_usage -= sizeof(MATRIX) + sizeof(polynomial *)*height + height*width;
  }
}

int matrix_getheight(MATRIX * m)
{
  if (m != (MATRIX *)0){
    return m->height;
  }
  return -1;
}

int matrix_getwidth(MATRIX * m)
{
  if (m != (MATRIX *)0) {
    return m->width;
  }
  return -1;
}

polynomial matrix_getentry(MATRIX * m, int row, int col)
{
  if (row < matrix_getheight(m) &&
      col < matrix_getwidth(m)) {
    return m->content[row][col];
  } else {
#ifdef DEBUG
	printf("matrix.c::matrix_getentry: (%i,%i)(row,col) is outside bounds which are (%i,%i) (height,width)\n",row,col,matrix_getheight(m),matrix_getwidth(m));
#endif
	ERR(m->oe,"matrix.c::matrix_getentry: (%i,%i)(row,col) is "	\
	    "outside bounds which are (%i,%i) (height,width)\n",
	    row,col,matrix_getheight(m),matrix_getwidth(m));
	return (polynomial)-1;
      }
}
/*
int main()
{
  int height = 5;
  int width = 10;
  MATRIX * m = new_matrix(height,width);
  printf("height %i width %i\n",matrix_getheight(m),matrix_getwidth(m));
  printf(" (5,5)=%i\n",matrix_getentry(m,4,5));
  return 0;
}*/

 /*
void matrix_info(OE oe) {
  oe->("Matrix Information :\n");
  oe->p(" Currently allocation matricies: %i\n",m_table.n_matrix);
  oe->p(" Currently allocated memory: %i bytes\n",m_table.mem_usage);
}
 */


MATRIX * cofactor_matrix(MATRIX * m)
{
  int width, height, row, col, i = 0;
  MATRIX * res;
  width = matrix_getwidth(m);
  height = matrix_getheight(m);
  res = new_matrix(m->oe,height,width);
  if (width == 1)
    {
      matrix_setentry(res,0,0,matrix_getentry(m,0,0));
      return res;
    }
  for (row = 0; row < height;row++)
    for (col = 0; col < width;col++)
      {
	MATRIX * submatrix = make_submatrix(m,col,row);
	polynomial det = determinant(submatrix);
	if (i % 2 == 0)
	  matrix_setentry(res,row,col,det);
	else
	  matrix_setentry(res,row,col,sub(0,det));
	destroy_matrix(submatrix);
      }
  return res;
}


void matrix_multiply_scalar(polynomial scalar, MATRIX * m)
{

  int width, height, row, col;
  width = matrix_getwidth(m);
  height = matrix_getheight(m);
  for (row =0;row < height;row++)
    for (col = 0;col < width;col++)
      {
	polynomial v = matrix_getentry(m,row,col);
	v = multiply(v,scalar);
	matrix_setentry(m,row,col,v);
      }
}

MATRIX * matrix_multiplication(MATRIX * m1, MATRIX * m2)
{
  MATRIX * res;
  /*
    | a_11 a_12 ... a_1m |     | b_11 b_12 ... b_1k |
    | .        .       . |     | .         .      . |
    | .        .       . |  x  | .         .      . | =
    | .        .       . |     | .         .      . |
    | a_n1 a_n2 ... a_nm |     | b_m1 b_p2 ... n_mk |

    a m by k matrix
  */
  int row, col, k;
  int m1height, m1width, m2width;
  OE oe = m1->oe;
  m1height = matrix_getheight(m1);
  m1width = matrix_getwidth(m1);
  m2width = matrix_getwidth(m2);
  if (m1width != matrix_getheight(m2)) {
    char m[512] = {0};
    osal_sprintf(m,"Wrong dim: w1(%d),h2(%d)\n",m1width, matrix_getheight(m2)); 
    oe->p(m);
    return NULL;
  }

  res = new_matrix(oe,m1height,m2width);
  
  for (row = 0; row < m1height;row++) {
      NOTIFY_GUI("Handling row ... ",0);
	  for (k = 0; k < m2width; k++) {
		  for (col = 0; col < m1width; col++) {
			  polynomial product = multiply(m1->content[row][col], m2->content[col][k]);
			  polynomial val = add(res->content[row][k], product);
			  res->content[row][k] = val;
		  }

		  if (k % 8000 == 0){
			  char * str = (char *)oe->getmem(osal_strlen("Processing slice ... .") + 11+512);
			  unsigned int pp = k * 10 / m2width;
			  osal_sprintf(str, "Processing slice ... %i of %i.", row + 1, m1height);
			  NOTIFY_GUI(str, pp * 10);
			  oe->putmem(str);
		  }
	  }
      NOTIFY_GUI("processing slice ... .",100);
    }
  return res;
}



MATRIX * matrix_invert(MATRIX * m) {
#ifdef DEBUG
  printf("DEBUG: matrix.c::matrix_invert entering \n");
#endif
  OE oe = m->oe;
  int width = matrix_getwidth(m);
  int height = matrix_getheight(m);
  if (width == height && width > 0)
    {
      
      polynomial det = determinant(m);

      if (det != 0) {
	polynomial inverse_det;
	MATRIX * cofac_matrix = cofactor_matrix(m);
	det = determinant(m);
	inverse_det = inverse(det);
	matrix_multiply_scalar(inverse_det,cofac_matrix);
#ifdef DEBUG
	  printf("DEBUG:matrix.c:matrix_invert matrix invert returning a value \n");
#endif
	  return cofac_matrix;
	} else {
#ifdef DEBUG	    
	printf("DEBUG matrix.c::matrix_invert you are trying to invert an singular matrix ...\n");
	print_matrix(m);
#endif
	    return NULL;
      }
    }
#ifdef DEBUG
  printf(" should not end here \n");
#endif
  return NULL;
}

void matrix_setentry(MATRIX * m , int row, int col, polynomial val)
{
  if (row < matrix_getheight(m) && col < matrix_getwidth(m)) {
      m->content[row][col] = val;
    }
  else {
#ifdef DEBUF
      printf("DEBUG: matrix.c::setentry \n");
#endif
      ERR(m->oe, "WARNING: matrix access violation ... (%i,%i) (%i,%i)\n",
	  row,col,matrix_getheight(m),matrix_getwidth(m));
    }
  return;
}

MATRIX * make_submatrix(MATRIX * m,int rm_row,int rm_col)
{
  int row, col, org_width = matrix_getwidth(m);
  MATRIX * res = new_matrix(m->oe,org_width-1,org_width-1);
  for(row = 0;row < org_width;row++) {
    for (col = 0; col < org_width;col++) {
      if (row < rm_row && col < rm_col)
	matrix_setentry(res,row,col,matrix_getentry(m,row,col));
      if (row < rm_row && col > rm_col)
	matrix_setentry(res,row,col-1,matrix_getentry(m,row,col));
      if (row > rm_row && col < rm_col)
	matrix_setentry(res,row-1,col,matrix_getentry(m,row,col));
      if (row > rm_row && col > rm_col)
	matrix_setentry(res,row-1,col-1,matrix_getentry(m,row,col));
    }
  }
  return res;
}



/* Calculate the determinant of the submatrix of m starting in 0,0 and
 * is submatrix_size by submatrix_size big.
 */
polynomial recdeterminant(MATRIX * m) {
  int width = matrix_getwidth(m);
  polynomial res = (polynomial)0;
  if (width == 1) {
      res = matrix_getentry(m,0,0);
    }

  if (width == 2) {
	  /*
	    |a b|
	    |c d| = a*d - c*b
	  */
    polynomial a = matrix_getentry(m,0,0);
    polynomial b = matrix_getentry(m,0,1);
    polynomial c = matrix_getentry(m,1,0);
    polynomial d = matrix_getentry(m,1,1);
    
    polynomial ad = multiply(a,d);
    polynomial cb = multiply(c,b);
    
#ifdef DEBUG
	  printf("a %i, b %i, c %i, d %i \n",a,b,c,d);
	  printf("ad %i, cb %i, ad-cb %i \n",ad,cb,sub(ad,cb));
#endif

	  res = sub(ad,cb);
  }
  
  if (width > 2) {
      int i;
      res = (polynomial)0;
      for(i = 0;i < width;i++)
	{
#ifdef DEBUG
	  printf("DEBUG: matrix.c::recdeterminant fister \n");
#endif
	  MATRIX * submatrix = make_submatrix(m,0,i);
	  polynomial det_submatrix = recdeterminant(submatrix);
	  polynomial entry = matrix_getentry(m,0,i);
	  if (i % 2 == 0)
	    res = add(res,multiply(entry,
				   det_submatrix));
	  else
	    res = sub(res,multiply(entry,
				   det_submatrix));

	  destroy_matrix(submatrix); 
	}
    }
  return res;
}
  
int determinant(MATRIX * m)
{
  int width = matrix_getwidth(m);
  int height = matrix_getheight(m);
  if (width == height && width > 0) {
      return recdeterminant(m);
    }
#ifdef DEBUG
  printf("DEBUG: matrix.c::determinant non square matrix \n");
#endif
  return -1;
}

  int _print_matrix_dim = 16;
void print_matrix(MATRIX * m) {
  OE oe = m->oe;
  const int max = _print_matrix_dim;
  int width = 0;
  int height = 0;
  int w = matrix_getwidth(m);
  int h = matrix_getheight(m);
  int row,col;
  byte * buf = oe->getmem(1024*w*h);

  height = max > h ? h : max;
  width = max > w ? w : max;

  for(row = 0;row < 8*width*height;++row) {
      buf[row] = '-';
  }

  buf[0] = '\n';
  for(row = 0;row < height;++row) {
    for(col = 0;col < width+1;++col) {
      if (col == width)
	osal_sprintf(buf+(4*row*(width+1)+4*col)+1,"   \n");
      else
	osal_sprintf(buf+(4*row*(width+1)+4*col)+1," %2x ",matrix_getentry(m,row,col));
    }
  }

  oe->print(buf);
  oe->putmem(buf);
}


void matrix_setrow(MATRIX * m, int row, polynomial * actual_row)
{
  if (row < matrix_getheight(m)) {
#ifdef DEBUG
    printf("inserting in row %i of :\n",row);
    printf(" row pointer is %p \n",(void *)actual_row);
    printf(" matrix info :\n");
    printf("  width   : %i\n",m->width);
    printf("  height  : %i\n",m->height);
    printf("  content : %p\n",(void *)m->content);
    print_matrix(m);
#endif
    mcpy(m->content[row], actual_row,matrix_getwidth(m));
#ifdef DEBUG
    printf("TEST \n");
#endif
  }
}


  unsigned char * matrix_to_flatmem(MATRIX *m) {
    OE oe = m->oe;
    int row, col;
    int height = matrix_getheight(m);
    int width = matrix_getwidth(m);
    unsigned char * result = (unsigned char *)oe->getmem(height*width);
    for (col = 0;col < width;col++)
      for (row = 0;row < height;row++) {
	uint index = col*height+row;
	polynomial val = matrix_getentry(m,row,col);
	result[index] = val;
      }
    return result;
  }

  MATRIX * matrix_from_flatmem(OE oe, unsigned char * d, uint w, uint h) {
    uint row=0,col=0;
    MATRIX * result = new_matrix(oe,h,w);
    for(col=0;col<w;++col) {
      for(row=0;row<h;++row) {
	uint index = col*h+row;
	matrix_setentry(result,row,col,d[index]);
      }
    }

    return result;
  }


MATRIX * LUSolve(MATRIX * L, MATRIX * U, MATRIX * b)
{
  int n=matrix_getheight(L);
  int i,j;
  OE oe = L->oe;
  MATRIX *y = new_matrix(oe,n,1);
  MATRIX *x = new_matrix(oe,n,1);
  
  for(i = 0;i < n;i++){
    polynomial sum = 0;
    for(j = 0;j < i;j++) {
      polynomial l_ij = matrix_getentry(L,i,j);
      polynomial y_j = matrix_getentry(y,j,0);
      sum = add(sum,multiply(l_ij,y_j));
    }
    matrix_setentry(y,i,0,sub(matrix_getentry(b,i,0),sum));
  }
  
  for(i=n-1;i>=0;i--) {
    polynomial y_i = matrix_getentry(y,i,0);
    polynomial sum = 0;
    polynomial u_ii;
    polynomial u_iiinv;
    polynomial tmp;
    for(j = i+1;j<n;j++) {
      polynomial u_ij = matrix_getentry(U,i,j);
      polynomial x_j = matrix_getentry(x,j,0);
      sum = add(sum,multiply(u_ij,x_j));
    }
    u_ii = matrix_getentry(U,i,i);
    u_iiinv = inverse(u_ii);
    tmp = sub(y_i,sum);
    matrix_setentry(x,i,0,multiply(tmp,u_iiinv));
  }
  
  destroy_matrix(y);
  
  return x;
}




  MATRIX **LUDecomposition(MATRIX * A) {
    OE oe = A->oe;
  int n = 0;
  int k, i, j;
  MATRIX *L =0,*U =0;
  MATRIX** result = (MATRIX **)oe->getmem(sizeof(MATRIX *)*2);
  
  if (!A) goto exit;
  if (!result) goto exit;
  
  n = matrix_getwidth(A);

  L=new_matrix(oe,n,n);
  if (!L) goto exit;

  for(k = 0;k < n;k++)
    matrix_setentry(L,k,k,1);

  U=new_matrix(oe,n,n);
  if (!U) goto exit;

  result[0] = L;
  result[1] = U;
  for(k=0;k<n;k++) {
      matrix_setentry(U,k,k,matrix_getentry(A,k,k));
      for(i = k + 1;i<n;i++) {
	  polynomial ukk = matrix_getentry(U,k,k);
	  polynomial ukkinv = 0;
	  polynomial a_ik = matrix_getentry(A,i,k);
	  polynomial a_ki = matrix_getentry(A,k,i);
	  if (ukk == (polynomial)0) {
	      ERR(oe,"LUDecomposition may have failed\nk=%i i=%i\n",k,i);
	      destroy_matrix(L);
	      destroy_matrix(U);
	      return NULL;
	    }
	  ukkinv = inverse(matrix_getentry(U,k,k));
	  matrix_setentry(L,i,k,multiply(a_ik,ukkinv));
	  matrix_setentry(U,k,i,a_ki);
	}

      for(i = k + 1;i < n;i++)
	{
	  for(j = k + 1; j < n;j++)
	    {
	      polynomial a_ij = matrix_getentry(A,i,j);
	      polynomial l_ik = matrix_getentry(L,i,k);
	      polynomial u_kj = matrix_getentry(U,k,j);
	      matrix_setentry(A,i,j,sub(a_ij,multiply(l_ik,u_kj)));
	    }
	}
    }
  return result;
 exit:
  destroy_matrix(U);
  destroy_matrix(L);
  oe->putmem(result);
  return 0;
}



MATRIX * LUPSolve(MATRIX *LUA, MATRIX *P, MATRIX * b)
{
  OE oe = LUA->oe;
  int n=matrix_getheight(LUA);
  int i,j;
  MATRIX *y = new_matrix(oe,n,1);
  MATRIX *x = new_matrix(oe,n,1);

  /* Forward substitution */
  for(i = 0;i < n;i++){
      polynomial sum = 0;
      for(j = 0;j < i;j++) {
	  polynomial l_ij = matrix_getentry(LUA,i,j);
	  polynomial y_j = matrix_getentry(y,j,0);
	  sum = add(sum,multiply(l_ij,y_j));
      }
      matrix_setentry(y,i,0,sub(matrix_getentry(b, matrix_getentry(P,0,i),0),sum));
  }

  /* Backward substitution */
  for(i=n-1;i>=0;i--)
    {
      polynomial y_i = matrix_getentry(y,i,0);
      polynomial sum = 0;
      polynomial u_ii;
      polynomial u_iiinv;
      polynomial tmp;
      for(j = i+1;j<n;j++)
	{
	  polynomial u_ij = matrix_getentry(LUA,i,j);
	  polynomial x_j = matrix_getentry(x,j,0);
	  sum = add(sum,multiply(u_ij,x_j));
	}
      u_ii = matrix_getentry(LUA,i,i);
      u_iiinv = inverse(u_ii);
      tmp = sub(y_i,sum);
      matrix_setentry(x,i,0,multiply(tmp,u_iiinv));
    }
  destroy_matrix(y);
  return x;
}

  MATRIX **LUPDecomposition(MATRIX * A) {
  OE oe = A->oe;
  int i,j,k,n = matrix_getheight(A);
  int * perm = (int *)oe->getmem(n*sizeof(int));
  MATRIX *P;
  MATRIX **result;
  P = new_matrix(oe,1,n);
  
  /* identity permutation */
  for(i = 0;i<n;i++)
    perm[i] = i;

  for(k=0;k<n;k++)
    {
      polynomial p = 0; /* pivot */
      int k_prime = 0;
      int tmp;
      for(i=k;i<n;i++)
	{
	  polynomial a_ik = matrix_getentry(A,i,k);
	  polynomial abs_a_ik = a_ik;
	  if (abs_a_ik > p) /* abs(a_ik) > p ? */
	    {
	      p = abs_a_ik;
	      k_prime = i;
	    }
	}
      if (p == 0)
	{
	  oe->p("LUPDecomposition - Matrix is singular ... aborting \n");
	  return NULL;
	}

      tmp = perm[k];
      perm[k] = perm[k_prime];
      perm[k_prime]=tmp;
      
      /* swap rows */
      for(i =0;i<n;i++)
	{
	  int a_ki, a_kprime_i;
	  a_ki = matrix_getentry(A,k,i);
	  a_kprime_i = matrix_getentry(A,k_prime,i);
	  matrix_setentry(A,k,i,a_kprime_i);
	  matrix_setentry(A,k_prime,i,a_ki);
	}

      for(i=k+1;i<n;i++)
	{
	  polynomial a_ik, a_kk,res;
	  a_ik = matrix_getentry(A,i,k);
	  a_kk = matrix_getentry(A,k,k);
	  res = multiply(a_ik,inverse(a_kk));
	  matrix_setentry(A,i,k,res);
	  for(j=k+1;j<n;j++)
	    {
	      polynomial a_ij,a_kj;
	      a_ij = matrix_getentry(A,i,j);
	      a_ik = matrix_getentry(A,i,k);
	      a_kj = matrix_getentry(A,k,j);
	      matrix_setentry(A,i,j,sub(a_ij,multiply(a_ik,a_kj)));
	    }
	}
    }

  for(i=0;i<n;i++)
    matrix_setentry(P,0,i,perm[i]);
  oe->putmem(perm);
    
  result = (MATRIX **)oe->getmem(2*sizeof(MATRIX*));
  result[0]=A;
  result[1]=P;
  return result;
}

  MATRIX * LUInverse(MATRIX * A) {
    OE oe = A->oe;
    int n=0,i=0,j=0;
    MATRIX *L = 0, *U = 0, *InverseA=0;
    MATRIX **LUPair = 0;;
    MATRIX **cols =0;
    A = matrix_copy(A);
    if (!A) return 0;
    
    n = matrix_getheight(A);

    cols = (MATRIX **)oe->getmem(sizeof(MATRIX *)*n);
    if (!cols) goto exit;


    LUPair = LUDecomposition(A);
    if (!LUPair) goto exit;
  
    L = LUPair[0];
    U = LUPair[1];
    
    for(i=0;i<n;i++) {
      MATRIX * e_i = new_matrix(oe,n,1);
      if (!e_i) goto exit;
      
      matrix_setentry(e_i,i,0,1);
      
      cols[i] = LUSolve(L,U,e_i);
      if (!cols[i]) goto exit;
      
      destroy_matrix(e_i);
    }
    
    InverseA = new_matrix(oe,n,n);
    if (!InverseA) goto exit;
    
    for(i = 0;i < n;i++) {
      for(j = 0;j < n;j++) {
	  matrix_setentry(InverseA,j,i,matrix_getentry(cols[i],j,0));
	}
    }
  
  exit:
    destroy_matrix(A);
    if (LUPair) {
      destroy_matrix(LUPair[0]);
      destroy_matrix(LUPair[1]);
      oe->putmem(LUPair);LUPair = 0;
    }
    if (cols) { 
      for(i = 0; i<n;i++) destroy_matrix(cols[i]);
      oe->putmem(cols);
    }
    return InverseA;
}


  MATRIX * LUPInverse(MATRIX * Ain) {
    OE oe = 0; 
    int n = 0, i = 0, j = 0;
    MATRIX *LUA=0, *P=0, *InverseA = 0 ; 
    MATRIX **LUA_P=0;
    MATRIX **cols = 0;
    MATRIX * A = 0;
    
    if (!Ain) goto exit;
    oe = Ain->oe;
    A = matrix_copy(Ain);
    n = matrix_getheight(A);

    cols = (MATRIX **)oe->getmem(sizeof(MATRIX *)*n);
    if (!cols) goto exit;

    InverseA = new_matrix(oe,n,n);
    if (!InverseA) goto exit;


    LUA_P = LUPDecomposition(A);
    if (!LUA_P) {
      ERR(oe, "LUP Decomposition failed matrix not "\
	  "invertiable by this operation. :(");
      goto exit;
    }
    
    LUA = LUA_P[0];
    P = LUA_P[1];
    
    for(i=0;i<n;i++) {
      MATRIX * e_i = new_matrix(oe,n,1);
      NOTIFY_GUI("Inverting matrix ... ",(i*100)/n);
      matrix_setentry(e_i,i,0,1);
      cols[i] = LUPSolve(LUA,P,e_i);
      destroy_matrix(e_i);
    }
      
    for(i = 0;i < n;i++) {
      for(j = 0;j < n;j++){
	matrix_setentry(InverseA,j,i,matrix_getentry(cols[i],j,0));
      }
    }
  
    NOTIFY_GUI("Inverting matrix ...",100);
    

  cleanup:
    if (cols) {
      for(i = 0;i<n;i++) destroy_matrix(cols[i]);oe->putmem(cols);cols=0;
    }
    
    if (LUA_P) {
      destroy_matrix(LUA_P[0]);
      destroy_matrix(LUA_P[1]);
      oe->putmem(LUA_P);
      LUA_P = 0;
    }

    return InverseA;    
  exit:
    // failure 
    oe->syslog(OSAL_LOGLEVEL_FATAL,"LUP Inverse failed");
    (destroy_matrix(InverseA),InverseA=0);
    goto cleanup;
}


MATRIX * matrix_copy(MATRIX * A) {
  OE oe = A->oe;
  int height = matrix_getheight(A);
  int width = matrix_getwidth(A);
  MATRIX * result = new_matrix(oe, height,width);
  int i,j;
  for(i=0;i<height;i++)
    for(j=0;j<width;j++)
      matrix_setentry(result,i,j,matrix_getentry(A,i,j));
  return result;
}

polynomial * matrix_topoly(MATRIX * A, int * l) {
  
  if (!A) return 0;

  if (l) *l = matrix_getwidth(A) * matrix_getheight(A);
  
  return (polynomial*)matrix_to_flatmem(A);
}

  MATRIX * matrix_segment(MATRIX * m, int top, int left, int bottom, int right ) {
    OE oe = m->oe;
    int h=0,w=0,nw=0,nh=0,row=0,col=0;
    MATRIX * result = 0;
    
    if (!m) {
      return 0;
    }
    
    if (top < 0 || left < 0 || bottom < 0 || right < 0) {
      return 0;
    }
    
    h = matrix_getheight(m);
    w = matrix_getwidth(m);

    if (top > h || bottom > h || left > w || right > w || left >= right || top >= bottom ) {
      #ifdef DEBUG
      oe->p("Invalid parameters: %d %d %d %d (%d,%d)\n",top,left,bottom,right,h,w);
      #endif
      return 0;
    }

    nw = right-left;
    nh = bottom - top;

    result = new_matrix(oe,nh,nw);
    if (!result) {
      return 0; 
    }
    
    for(col=0;col<nw;++col) {
      for(row=0;row<nh;++row) {
	polynomial val = matrix_getentry(m,top+row, left+col);
	matrix_setentry(result,row,col,val);
      }
    }

    return result;
  }


/*
int main()
{
  MATRIX * m = new_matrix(3,3);
  matrix_info();
  matrix_setentry(m,0,0,(polynomial)1);
  matrix_setentry(m,1,0,(polynomial)0);
  matrix_setentry(m,2,0,(polynomial)0);

  matrix_setentry(m,0,1,(polynomial)0);
  matrix_setentry(m,1,1,(polynomial)1);
  matrix_setentry(m,2,1,(polynomial)0);

  matrix_setentry(m,0,2,(polynomial)0);
  matrix_setentry(m,1,2,(polynomial)0);
  matrix_setentry(m,2,2,(polynomial)1);

  print_matrix(m);

  printf(" determinant %i \n",determinant(m));
  matrix_info();
  destroy_matrix(m);
  return 0;
}
*/

#ifdef __cplusplus
}
#endif
