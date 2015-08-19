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

#ifndef REEDSOLOMON_H
#define REEDSOLOMON_H
#include <math/polynomial.h>
#include <math/matrix.h>
#include <common.h>
#include <osal.h>
#ifdef __cplusplus
extern "C" {
#endif

  /* \brief
   * \struct
   *
   * Encoder that encodes a message in either the Reed Solomon code or
   * it Schur transform.
   */
  typedef struct _minimacs_encoder_ {
    polynomial * (*encode)(byte * msg, uint lmsg);
    bool (*validate)(byte * code, uint lmsg);
    void * impl;
  } * MiniMacsEnc;

  /*!
   *
   */
  MiniMacsEnc MiniMacsEnc_MatrixNew(OE oe, uint lcode, uint lmsg);
  void MiniMacsEnc_MatrixDestroy(MiniMacsEnc * mme);


/*!
 *
 * \brief This function creates an Reed Solomon code for the given
 * data.
 *
 * \param message - the clear text message to create a code for
 *
 *
 * \param lmessage- the length of the message
 *
 * \param lcode - the length of the returned code 
 *
 * \return a pointer to a memory (malloc) area with the generated code
 */
  MATRIX * encode(OE oe,polynomial * message, uint lmessage, uint lcode);


/*!
 * \brief Given a rscode word we check that the code word is actually
 * valid.
 *
 * \param rscode  - a reed solomon code word
 * \param lrccode - the length of the reed solomon code word
 *
 * \return zero if the code word is invalid/wrong/bad, non-zero otherwise
 *
 */
  bool validcode(OE oe,polynomial * rscode, uint lrccode, polynomial * msg, uint lmsg);

/*!
 * \brief Perform lagrange interpolation on the given set of points
 * computing the coefficients of the polynomial going through those
 * points.
 *
 * \param points - the points to determining the polynomial
 *                 coefficients from.
 *
 * \param lpoints - the number of points in coefficients
 *
 * \return the coefficients of the polynomial going through the given
 *         points of degree lpoint-1
 */
  polynomial * interpol(OE oe, polynomial * points, uint lpoints);


/*!
 * \brief Perform message encoding for the Mini Macs online phase.
 *
 * \param msg  - the message to encode as a code word
 *
 * \param lmsg - the length of the message above
 *
 * \param lcode- the length of the returned code word (lcode >= lmsg)
 *
 * \return a reed solomon code word with msg as the first lmsg bytes
 * of the code word.
 *
 * -----
 *
 * example:
 *
 * // we assume a {message} which is sequence of bytes in present in
 * // the context and that {lmessage} is the length of message is also
 * // present.
 *
 * // note polynomial indicates that bytes are elements in GF[2^8] and
 * // are simply just bytes in disguise. Any byte is a polynomial and any
 * // any polynomial is a byte, thus it is safe to char * p = (char *)poly and
 * // vice versa.
 *
 * { 
 *   polynomial * codeword = minimacs_encode( (polynomial*)message,
 *                                            lmessage, 2*lmessage);
 *
 *   // Do something with code word, but *YOU* own the pointer.
 *
 *   free(codeword);
 * }
 *   
 */
  polynomial * minimacs_encode(OE oe,polynomial * msg, uint lmsg, uint lcode);

/*!
 * \brief Returns true if the given code is valid.
 *
 * \param code - the code to validate 
 *
 * \param lcode- the length of the given code
 *
 * \param lmsg - the length of the code word that is 
 *               the message.
 *
 * \return true (non zero) if the code is valid, false (zero)
 * otherwise
 *
 * ----
 *
 * example: 
 *
 * void test_minimacs_validate(code) {
 *   // we assume a code and lcode is present in the context
 *   // created by minimac_encode or similar functionality.
 *
 *   if (minimacs_validate(code,lcode)) { 
 *     printf("code is valid\n");
 *   } else { 
 *     printf("code is invalid\n");
 *   }
 * }
 */
  bool minimacs_validate(OE oe, polynomial * code, uint lcode, uint lmsg);
 
  /*!
   *
   * \brief Returns true if the given code is valid.
   * 
   * \param enc   - the pre-computed encoder matrix
   *
   * \param code  - the code to validate
   *
   * \param lcode - lcode length of the code
   *
   * \param lmsg  - the length of the message
   */
  bool minimacs_validate_fast( MATRIX * enc, polynomial * code, uint lcode, uint lmsg);  

  /*!
   * \brief Generates an encoder matrix for fixed message and code
   *        lengths.
   *
   * Note! To be used with minimacs_validate_fast.
   *
   * \param lmsg - the length of the messages to encode
   *
   * \param lcode- the length of the reed solomon codes to produce.
   *
   * \return an lcode by lmsg matrix for encoding messages for length
   *         lmsg to code words for length lcode.
   *
   */
  MATRIX * minimacs_generate_encoder(OE oe, uint lmsg,uint lcode);

  /*!
   * \brief Returns an MiniMacs reed solomon code.
   *
   * \param enc - lcode by {lmsg} matrix 
   *
   * \param msg - the {lmsg} length message to encode
   *
   * \param lmsg- the length of the message {msg}
   *
   * \return a code word of length lcode.
   *
   */
  polynomial * minimacs_encode_fast(MATRIX * enc, polynomial * msg, uint lmsg);

  /*!
   * \brief Returns a Van Der Monde matrix with the given height and width.
   *
   * \param h - the height of the matrix
   * \param w - the width of the matrix
   *
   *\ return a fresh van der monde matrix with height {h} and width {w}.
   */
  MATRIX * build_matrix(OE oe,uint h, uint w);

#ifdef __cplusplus
}
#endif

#endif
