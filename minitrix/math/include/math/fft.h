/*!
 * \file fft.h
 * Fast Fourier Transform
 * Author: Rasmus Lauritsen
 */
#ifndef FFT_H
#define FFT_H

#include <osal.h>
#include <math/matrix.h>

void fft_gf8(byte * ab, byte * a, byte *b, uint len);
uint nth_root_of_unity(byte * r, uint n);
byte smallest_nth_primitive_root_of_unity(uint n);
MATRIX * build_nth_matrix(OE oe, uint h, uint w, byte root);
void polynomial_mul(byte * f, byte * g, uint degree, byte * res);
void polynomial_gcd(byte * f, byte * g, byte * gcd, byte * a, byte * b);
/*!
 * Efficient Fast Fourier Transform - perform a fourier transform of a
 * sample {f} of length N={P}*{Q}. The result of length N will be
 * stored in {res}.
 *
 * \param P     - Factor of N, the length of {f}
 * \param Q     - Factor of N, the length of {f} st N=PQ
 * \param f     - Polynomial of length N, may have degree less than N-1
 *                 but should then be padded with zero
 *                 coefficients. {f} must be of length N
 * 
 * \param res   - The output vector
 * 
 * Note: Imple detail we choose the smallest nth roots of units.
 *
 */
void efft( uint P, uint Q, byte * f, byte * res);
void efftinv(uint P, uint Q, byte * f, byte *res );
MATRIX * minimacs_fft_shurgenerator(OE oe, uint lmsg, uint lcode, byte root);
void composite_fft2(uint P, uint Q, byte qroot, byte nroot, byte * f, byte * y);
#endif
