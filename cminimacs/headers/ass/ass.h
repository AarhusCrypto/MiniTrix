/*!
 * \file ass.h
 *
 * Component for additive secret sharing.
 *
 */

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


Created: 2013-07-23

Author: Rasmus Winther Lauritsen, rwl@cs.au.dk

Changes: 
2013-07-23 13:14: Initial version created
*/

#ifndef ASS_H
#define ASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <common.h>

/** \brief create {nshares} additively secret shared shares for {msg}
 *         over GF[2^8]
 *
 * \param msg          - the message to additively secret share
 * \param lmsg         - the length of the message
 * \param nshares      - the number of shares to generate
 *
 * \return array of {nshares} each of length lmsg or (byte*)0 if an
 * error occurs.
 */
byte ** ass_create_shares( byte * msg, uint lmsg, uint nshares );


/** \brief reconstructs a secret from {nshares} given as an array. Each
 *         share has length {lshare}.
 *
 * \param shares   - the secret shares to combine and reconstruct the 
 *                   message of length {lshare} from.
 *
 * \param lshare   - the lenght of each share
 *
 * \param nshares  - the number of shares to be found in {shares}
 *
 * \return a message of length {lshare} that is the result of
 *         additively reconstructing the message byte-wise from 
 *         the given shares. e.g. result[i] = \sum_{j=0}^{nshares} shares[i][j] 
 *         with each entry considered as an element from GF[2^8].
 *         
 */
byte* ass_reconstruct(byte**shares,uint lshare, uint nshares );
  
  /** \brief convenient function to free an array of shares allocate
   * by ass_create_shares :)
   *
   * \param shares        - shares to clean up
   * \param lshares       - the number of shares in {shares}
   *
   */
  void ass_clean_shares(byte ** shares, uint lshares);

#ifdef __cplusplus
}
#endif

#endif
