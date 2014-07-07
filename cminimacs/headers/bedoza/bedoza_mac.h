/*!
 * \file bedoza_mac.h
 *
 * BeDOZa macs are information theoretic macs on the form mac(x) = a*x+b
 * where a and b are uniformly random and only known by the
 * verifier. The plaintext x and mac(x) are hold by a value holder.
 *
 * Suppose P1 is a value holder of x and P2 is a verifier of x then
 * BeDOZa macs are layout as below between the two parties P1 and P2:
 *
 * P1               p2
 * x,mac(x)         a,b
 *
 * P1 cannot lie about x because mac(x)=ax+b and P1 does not know a
 * and b. Hence P1 is forced be honest about x and mac(x) towards P2.
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


Created: 2013-07-22

Author: Rasmus Winther Lauritsen, rwl@cs.au.dk

Changes: 
2013-07-22 14:12: Initial version created
*/

#ifndef BEDOZA_TWO_REP_H
#define BEDOZA_TWO_REP_H

#include <common.h>
#include <encoding/der.h>

#ifdef __cplusplus
extern "C" {
#endif

  /* messages are identified by an unique id of this type.
   */
  typedef unsigned int msgid;

  /* parties are identified by an unique id of this type.
   */
  typedef unsigned int parid;

  /* a mac value is a byte array connected with three ids: toid,
   * fromid and msgid. Their meaning: the mac is hold by the player
   * with fromid for checking the mac of the share of the message with
   * msgid hold by the player identified toid.
   */
  typedef struct __bedoza_mac__ {
    msgid mid;
    parid toid;
    parid fromid;
    byte * mac;
    uint lmac;
  } * bedoza_mac;

  /* a mac key is two byte arrays connected with three ids: toid,
   * fromid and msgid. 
   */
  typedef struct __bedoza_mac_key__ {
    msgid mid;
    parid toid;
    parid fromid;
    byte * alpha;
    uint lalpha;
    byte * beta;
    uint lbeta;
  } * bedoza_mac_key;
  
  /* \brief This function will generate two random values alpha and
   * beta and allocate a bedoza_mca_key with those generated values
   * and the given ids.
   *
   * \param security_parameter - in bytes the length of alpha and beta
   * \param mid                - the id of the message
   * \param toid               - the id of the mac owner
   * \param fromid             - the id of the key owner
   *
   * \return an instance of bedoza_mac_key or (bedoza_mac_key)0 if an
   * error occurred.
   *
   */
  bedoza_mac_key generate_bedoza_mac_key( uint security_parameter, msgid mid, parid toid, parid fromid );
  
  /* \brief This function will compute the besoza mac with the given
   * key on the given plaintext.
   *
   * \param key         - mac key 
   * \param plaintext   - the clear text value of the message to compute a mac for
   * \param lplaintext  - plaintext length
   *
   * \return an instance of bedoza_mac or (bedoza_mac)0 if an error
   * occurred.
   */
  bedoza_mac compute_bedoza_mac(bedoza_mac_key key, byte * plaintext, uint lplaintext);

  /*
   * \brief This function will determine if the given mac is a mac for
   * the given plaintext forged with the given key.
   *
   * \param key           - mac key
   * \param mac           - the mac to check for
   * \param plaintext     - the plaintext to check validity of
   * \param lplaintext    - plaintext length
   *
   * \return true(non zero) if the mac is valid zero otherwise.
   *
   */
  bool bedoza_check_mac(bedoza_mac_key key, bedoza_mac mac, byte * plaintext, uint lplaintext);


  /*!
   * \brief Clean up a MAC key.
   *
   * \p_key      - a pointer-pointer to the key to destroy,
   *               the pointer is set to NULL after the mac key 
   *               is destroyed.
   *
   * Note: Accepts NULL, *p_key==NULL
   */
  void bedoza_mac_key_destroy(bedoza_mac_key * p_key);

  /*!
   * \brief Clean up a Mac
   *
   * \param mac     - a pointer-pointer to the mac to be destroyed.
   *                  The pointer is set to NULL after the mac is 
   *                  destroyed.
   *
   * Note: Accepts NULL, *mac == NULL
   *
   */
  void bedoza_mac_destroy(bedoza_mac * mac);

  /*!
   * \brief bedoza_add_macs adds two macs and returns a fresh mac on
   * the sum.
   *
   * \param left       - the mac of the left operand in the summation
   * \param right      - the mac of the right operand in the summation
   * \param result     - pointer-pointer that will contain the allocated result.
   *
   * Note: Allocation is performed with malloc !
   *
   * On failure *{res} is NULL.
   *
   */
  void bedoza_add_macs( bedoza_mac left, 
			bedoza_mac right,
			bedoza_mac * result);

  /*!
   * \brief adds two bedoza_mac_key's such that the key pointed to by
   * *{res} after will validate the mac on the sum.
   *
   * \param left        - mac key for the mac on the left operand in the summation
   * \param right       - mac key for the mac on the right operand in the summation
   * \param result      - pointer-pointer that will contain the allocated result.
   *
   * Note: Allocation is performed with malloc!
   *
   * On failure *{res} is NULL.
   *
   */
  void bedoza_add_mac_keys( bedoza_mac_key left, 
			    bedoza_mac_key right, 
			    bedoza_mac_key * res);

  /*!
   * \brief create the key validating the mac on the product of the
   * values for left and right. Notice, access to the clear text
   * values in {l_ptxt} and {r_ptxt} are needed and thus cannot be
   * computed by a single party in a real MPC setting. This is for
   * preprocessing assistance and testing purposes.
   *
   * \param left         - mac key on the left operand
   * \param l_ptxt       - the plain text for the left operand
   * \param right        - mac key on the right operand
   * \param r_ptxt       - the plain text for the right operand
   * \param result       - the mac key validating the mac on the product 
   *                       of left and right.
   *
   * result will be NULL on failure.
   *
   *
   */
  void bedoza_mul_mac_keys( bedoza_mac_key left, 
			    byte * l_ptxt,
			    bedoza_mac_key right, 
			    byte * r_ptxt,
			    bedoza_mac_key * result);

  /*!
   * \brief multiplies two macs component wise in Galoir 2^8.
   *
   * \param left
   * \param right
   * \param result
   *
   * *result is NULL on failure..
   *
   */
  void bedoza_mul_macs( bedoza_mac left, 
			bedoza_mac right, 
			bedoza_mac * result );
  /*!
   * \brief compatible bedoza mac keys has the same alpha value !
   * thus after generating the first key all subsequent keys should
   * have same alpha value if they should be compatible with the first
   * key.
   *
   * \param other        - another mac key that the result should be compatible with
   * \param mid          - id of the message that we are generating a mac key for
   * \param toid         - the receiver of the mac this key validates
   * \param fromid       - the receiver of this mac key
   *
   * \return a fresh mac key compatible with {other}.
   */
  bedoza_mac_key bedoza_generate_compat_key(bedoza_mac_key other,
					    msgid mid,
					    parid toid,
					    parid fromid );


  /*!
   * \brief allocate a fresh bedoza_mac and sets its content to that
   * of {m}.
   *
   * \param m  - The representation to copy
   *
   * \return a copy of {m} 
   *
   */
  bedoza_mac bedoza_mac_copy(bedoza_mac m);

  /*!
   * \brief copy the given key {k}
   * 
   * \param k   - the key to copy
   *
   * \return a fresh mac key with the same contents as {k}.
   *
   *
   */
  bedoza_mac_key bedoza_mac_key_copy(bedoza_mac_key k);

  /*!
   * \brief given a mac and a constant produce the of the product.
   *
   * \param mac   - the mac
   * \param c     - a constant less or equal the length of the mac
   * \param lc    - length of {c}
   *
   * \return fresh mac for the product
   *
   */
  bedoza_mac bedoza_mac_mul_const( bedoza_mac mac, byte * c, uint lc);

  /*!
   * \brief given a mac and a constant produce the representation the
   * AND.
   *
   * \param mac - the mac to AND {c} onto.
   * \param c   - the constant to AND with.
   * \param lc  - the length of {c}
   *
   * \return the mac of the bit wise and of {c} and r, where r is the
   * representation that {mac} is a MAC for.
   */
  bedoza_mac bedoza_mac_and_const( bedoza_mac mac, byte * c, uint lc);



  /*!
   * \brief given a mac key and a constant, make a fresh mac key for 
   * the mac on the product. 
   *
   * \param key    - the key
   * \param c      - the constant to multiply on the key
   * \param lc     - the length of {c}
   *
   * \return a fresh mac key for the product
   */
  bedoza_mac_key  bedoza_mac_key_mul_const( bedoza_mac_key key, byte * c, uint lc);

  bedoza_mac_key bedoza_mac_key_and_const( bedoza_mac_key key, byte * c, uint lc);

  /*!
   * \brief serialise the given {mac} data structure into data. If
   * data is a null-pointer only ldata is updated to the required
   * length.
   *
   * \param mac     - the mac to serialise
   * \param data    - output buffer for serialised {mac}
   * \param ldata   - pointer to the length of data if not null, will
   *                  otherwise be updated with the required length.
   *
   * \return DER_OK/0 on success.
   */
  DerRC bedoza_mac_save( bedoza_mac mac, byte * data, uint * ldata);


  /*!
   * \brief try to reconstruct a bedoza_mac from data.
   *
   * \param data    - data to load bedoza_mac from
   * \param ldata   - length of {data}
   * \param mac_out - result is stored in *mac_out
   *
   * \return DER_OK on success.
   * 
   */
  DerRC bedoza_mac_load( byte * data, uint ldata, bedoza_mac * mac_out);

  /*!
   * \brief serialise the given {key} data structure into data. If the
   * data is a null-pointer only ldata is updated to the required length.
   *
   * \param key    - the key to serialise
   * \param data   - output buffer for serialise {key}
   * \param ldata  - length of {data}
   *
   * \return DER_OK on success.
   */
  DerRC bedoza_mac_key_save( bedoza_mac_key key, byte * data, uint * ldata);

  /*!
   * \brief try to reconstruct a bedoza_mac_key from data.
   *
   * \param data    - the supposed serialised key
   * \param ldata   - length of {data}
   * \param key_out - on success this pointer is updated to a fresh bedoza_mac_key.
   *
   * \return DER_OK on success.
   */
  DerRC bedoza_mac_key_load( byte * data, uint ldata, bedoza_mac_key * key_out);

  /*!
   * \brief serialise the keys in {keys}
   *
   */
  DerRC bedoza_mac_keys_save( bedoza_mac_key * keys, uint lkeys, 
			      byte * data, uint * ldata);

  /*!
   * \brief load a sequence of keys
   *
   * \param data     - the data of a sequence of serialised keys
   * \param ldata    - the length of {data}
   * \param keys_out - points to an array of keys 
   * \param lkeys_out- length of {keys_out} 
   *
   * \return DER_OK on success
   */
  DerRC bedoza_mac_keys_load( byte * data, uint ldata, 
			      bedoza_mac_key ** keys_out, 
			      uint * lkeys_out );

  /*!
   * \brief save a list of keys
   *
   * \param keys      - keys to serialise
   * \param lkeys     - the number of keys
   * \param data      - buffer to store serialised keys in
   * \param ldata     - length of {data}. If {data} is null this contains 
   *                    the needed length
   *
   * \return DER_OK on success
   */
  DerRC bedoza_mac_keys_save( bedoza_mac_key * keys,
			      uint lkeys,
			      byte * data, uint * ldata );


  /*!
   *
   *
   */
  DerRC bedoza_macs_save( bedoza_mac * macs, uint lmacs,
			  byte * data, uint * ldata);

  DerRC bedoza_macs_load( byte * data, uint ldata,
			  bedoza_mac * macs_out, uint * lmacs_out );

  DerRC bedoza_mac_ld(DerCtx * c, bedoza_mac * mac_out);
#ifdef __cplusplus
}
#endif

#endif

