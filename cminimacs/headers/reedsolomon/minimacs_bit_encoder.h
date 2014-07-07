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
2014-04-15 15:32: Initial version created
*/


#ifndef MINIMACS_BIT_ENCODER_H
#define MINIMACS_BIT_ENCODER_H
#include <reedsolomon/reedsolomon.h>

/*!
 * \breif Create a minimacs encoder that encodes binary data using a
 * bit-selection technique to create codewords rather than
 * matrix-multiplication or FFT. This however only works if the data
 * to encode are bits.
 * 
 * \param oe   - Operating Environment for memory allication.
 * \param ltext- The length of messages that we can encode.
 * \param lcode- The length of generated code
 *
 * \return A MiniMacsEnc instance for Bit encoding.
 */
MiniMacsEnc MiniMacsEnc_BitNew(OE oe, uint ltext, uint lcode);

/*!
 * \brief Clean up resources occupied by {instance}.
 *
 * \param instance   - the instance to clean up resources for.
 *
 */
void MiniMacsEnc_BitDestroy(MiniMacsEnc * instance);


#endif
