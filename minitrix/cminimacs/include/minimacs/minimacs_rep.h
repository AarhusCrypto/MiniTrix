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


  Created: 2013-07-25

  Author: Rasmus Winther Lauritsen, rwl@cs.au.dk

  Changes: 
  2013-07-25 14:54: Initial version created
*/

#ifndef MINIMACS_REP_H
#define MINIMACS_REP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bedoza/bedoza_mac.h>
#include <reedsolomon/reedsolomon.h>
#include <osal.h>



  typedef struct _rep_value_ {

    /*
     * The length of the original message.
     */ 
    uint lval;


    /* Public code word for v such that
     * x = x_1 +,...,x_N+v 
     */
    byte * dx_codeword;
    uint ldx_codeword;

    /**
     * Reedsolomon code word
     *
     */
    byte * codeword;
    uint lcodeword;

    /**
     * mapping from 0 to N-1 of players
     * to my mac on this message.
     */
    bedoza_mac * mac;
    uint lmac;

    /* mapping from 0 to N-1 of players
     * to corresponding mac key for message
     * identified by msgid.
     */ 
    bedoza_mac_key * mac_keys_to_others;
    uint lmac_keys_to_others;

  } * MiniMacsRep;

  typedef struct _rep_triple_ {

    MiniMacsRep a;
    MiniMacsRep b;
    MiniMacsRep cstar;

  } * MiniMacsTripleRep;

typedef struct _bit_decomposed_triples_ {
  MiniMacsRep a;
  MiniMacsRep b;
  MiniMacsRep c;
  MiniMacsRep abits[8];
  MiniMacsRep bbits[8];
} * BitDecomposedTriple;



  /*!
   * \brief Primary for testing ! Given plaintext {text} a
   * representation object for each peer is created.
   *
   * For Testing purposes and running the online phase only.
   *
   * \param text          - the plain text to create shares for
   * \param ltext         - the length of the plain text
   * \param nplayers      - the number of players
   * \param codelength    - the codelength
   * \param compat_with   - all mackeys will be consistent 
                            with this sharing (optional can by NULL)
   *
   * \return {nplayers} instances of MiniMacsRep one for each player
   * where the text has been additively secret shared and from each
   * share a speciel Reed Solomon is generated such that the {text} is
   * the first {ltext} values of the code. In the end a BeDOZa mac
   * keys and macs are created for each player towards all other
   * players.
   *
   */
  MiniMacsRep * minimacs_create_rep_from_plaintext(byte * text, 
						   uint ltext, 
						   uint nplayers,
						   uint codelength,
						   MiniMacsRep * compat_with);




  /*!
   * \brief Primary for testing ! Given plaintext {text} a
   * representation object for each peer is created. This function
   * does the exact same as the function above except it uses the
   * given van der monde matrix {encoder} for all generations. The
   * makes the preprocessing faster.
   *
   * For Testing purposes and running the online phase only.
   *
   * \param oe            - OSAL abstraction 
   * \param encoder       - van der monde matrix need for reedsolomon
   * \param mid           - an id associated with this plain text
   * \param text          - the plain text to create shares for
   * \param ltext         - the length of the plain text
   * \param nplayers      - the number of players
   * \param codelength    - the codelength
   * \param compat_with   - all mackeys will be consistent 
                            with this sharing (optional can by NULL)
   *
   * \return {nplayers} instances of MiniMacsRep one for each player
   * where the text has been additively secret shared and from each
   * share a speciel Reed Solomon is generated such that the {text} is
   * the first {ltext} values of the code. In the end a BeDOZa mac
   * keys and macs are created for each player towards all other
   * players.
   *
   */ 
  MiniMacsRep * minimacs_create_rep_from_plaintext_f(OE oe, MiniMacsEnc encoder, 
						     byte* text,
						     uint ltext,uint nplayers, 
						     uint codelength, 
						     MiniMacsRep * compat_with);
  /*!
   * \brief Create a share that represents a publicly known value like a
   * constant in an agreed circuit.
   * 
   * \param mid          - an identifier for the message
   * \param text         - the public message
   * \param ltext        - the length of the public message
   * \param codelength   - the length of the generated codeword
   * 
   * \return One representation that has no macs, no dx_codeword, no
   * keys. Only the codeword is present.
   *
   *
  MiniMacsRep minimacs_create_rep_public_plaintext( byte * text,
						   uint ltext,
						   uint codelength);

  */



  /*!
   * \brief Create {nplayers} shares BeDOZa style meaning there is no
   * extra publicly known code word dx_codeword. Thus, the number of
   * additive shares generated is {nplayers} rather than {nplayers}+1.
   *
   * \param mid          - an identifier for the message
   * \param text         - the message
   * \param ltext        - the length of the message
   * \param nplayers     - the number of players
   * \param codelength   - the length of the generated codeword
   *
   * \ return {nplayers} shares
   *
   */
  MiniMacsRep * minimacs_create_rep_bedoza_style(
					       byte * text,
					       uint ltext,
					       uint nplayers,
					       uint codelength);

  /*!
   * \brief Check internal consistency of the representation.
   *
   * \param rep       - a presentation to check validity of 
   *                    codewords for.
   *
   * \return true if dx_codeword and codeword are valid reed solomon
   * codewords.
   */
  bool minimacs_check_representation( MiniMacsRep rep );


  /*!
   * \brief Check that a codeword/share from another player check out
   * according to the given mac and my mac keys in {rep}.
   *
   * \param rep              - my representation
   * \param other_party_id   - the identify of the other player such that we can find
   *                           the right mac-key.
   * \param codeword         - the codeword/share to authenticate received from another
   *                            party.
   * \param mac              - the other party's bedoza mac on the codeword
   * \param lcodeword        - the length of {codeword}
   *
   * \return true if {codeword} is a valid reed solomon codeword and
   * if {mac} is a valid mac on {codeword} given the key in {rep} for
   * player {other_party_id}.
   *
   */
  bool minimacs_check_othershare(MiniMacsRep rep, 
				 uint other_party_id,
				 byte * codeword, 
				 byte * mac, 
				 uint lcodeword);

  /*!
   * \brief clean up a representation.
   * 
   * \param single_rep        - pointer-pointer to representation 
   *                            to clean
   *
   * Note: the representation pointed to by *single_rep is cleaned and
   * then single_rep is set to NULL.
   *
   */
  void minimacs_rep_clean_up(OE oe, MiniMacsRep * single_rep);

  /*!
   * \breif create the representation of xor'ing left and right
   *
   * \param oe             - osal instance for allocation
   * 
   * \param left           - a single share held by one player to 
   *                         be the left operand for out xor operation.
   * \param right          - a single share held by on player to be 
   *                         the right operand for out xor operation.
   *
   * \return if left and right are compatible shares (toid, fromid and
   * bedoza alpha keys match) then a fresh representation for a single
   * party is return being this party's representaion of xor'ing left
   * and right. NULL is return upon failure.
   *
   * 
   */
  MiniMacsRep minimacs_rep_xor(OE oe, MiniMacsRep left, MiniMacsRep right);

  
  /*!
   * \brief given all shares for {left} and {right} generate the
   * multiplied representation in the star representation.
   *
   * This function is intented for pre-processing, all shares must be
   * given for both operands.
   *
   * \param left       - {nplayers} shares for the left operand
   * \param right      - {nplayers} shares for the right operand
   *
   * \return a list of {nplayers} shares one for each player that
   * together for the product of {left} and {right}.
   *
   */
  MiniMacsRep * minimacs_rep_mul(MiniMacsRep * left, MiniMacsRep * right, uint nplayers);


  /*!
   * \brief obtain the representation of adding constant {c} to {rep}
   * 
   * {c} must be a constant with a length less than or equal to
   * {rep}->lval, e.g. the length of data available in the reed
   * solomon code for plaintext.
   *
   *
   * \param rep          - the representation to which we wish to add a
   *                       constant.
   * \param c            - the constant to be added
   * \param lc           - the length of the constant
   *
   * \return a fresh representation that is {rep} add with the
   * constant {c}.
   *
   */
  MiniMacsRep minimacs_rep_add_const(MiniMacsRep rep, byte * c, uint lc );


  /*!
   * \brief obtain the representation of multiplying constant {c} to {rep}
   *
   * \param rep           - the representation to which we wish 
   *                        to multiply a constant.
   *
   * \param c             - the constant to add
   * \param lc            - the length of {c}
   *
   * \return a fresh representation that is {rep} multiplied with the
   * constant {c}.
   */
  MiniMacsRep minimacs_rep_mul_const(MiniMacsRep rep, byte * c, uint lc );

  /*!
   * \brief predicate determining whether {rep} is a public constant.
   *
   * \param rep           - the minimacs representation to check
   *
   * \return true if {rep} has the form of a public constant.
   *
   * Note: This predicate is strongly dependent on the behaviour of 
   * minimacs_create_rep_public_plaintext.
   *
   */
  bool minimacs_rep_is_public_const( MiniMacsRep rep );

  /*!
   * \brief given an variable number of lists of shares for all
   * players, {reps_out} will hold the shares that should be given for
   * each player. 
   *
   * e.g. given msg1 and msg2 where player i should have shares
   * msg1[i] and msg2[i] rearrange will order the shares in {reps_out}
   * such that *reps_out[0] is the list of msg1[0],msg2[0] and so on.
   *
   * \param reps_out      - The output
   * \param ...           - a variable number of share lists (MiniMacsRep *)
   *
   */
  bool minimacs_rearrange(OE oe, MiniMacsRep *** reps_out,...);

  /*!
   * \brief given a byte array this function will try to create a
   * representation from it. On failure zero/null is returned. On
   * success a fresh {malloc}ed minimacs representation is returned.
   *
   *
   * \param data       - data buffer with serialise representation.
   * \param ldata      - the length of {data}
   *
   * \return fresh deserialise/reconstructed mininacs representation.
   *
   */
  MiniMacsRep minimacs_rep_load(OE oe, byte * data, uint ldata);

  /*!
   * \brief write {rep} to {data}. If {data} is null only this
   * function will return the number of bytes necessary to store the
   * serialisation of {rep}.
   *
   * \param rep      - representation to serialise
   * \param data     - (optional) buffer to serialise {rep} into
   *
   * \return the length of the serialisation, e.g. the number of bytes
   * written to {data} if non zero.
   *
   */
  uint minimacs_rep_store(MiniMacsRep rep, byte * data);

  

  /*!
   * \brief saves the shares to a set of files, one for each player.
   *
   * \param postfix - the last group of the filename typically used to
   *                  indicate encoding form: mxt=matrix, fft=fast fourier.
   *
   * \param ltext - the length of message this is not check against
   *                the acutal length in the triples. It is used for
   *                filename generation only.
   *
   * \param lcode - the length of codewords. This is not checked
   *                against the acutal length in the triples
   *                {btriples}. {lcode} is only used for the filename.
   *
   * \param nplayers - the number of players and hence the number of files 
   *                   that will be created.
   *
   * \param btriples - the triples layed out such that there are
   *                   {nplayers} rows of {ncount} triples.
   *
   * \param ncount - the number of triples in {btriples}
   *
   *
   */
  void 
  save_bdt(char * postfix, uint ltext, uint lcode, uint nplayers, 
           BitDecomposedTriple ** btriples, uint ncount);



  /*!
   * \brief load a single set of triples, e.g. for one player.
   *
   * \param filename - the filename to load from
   *
   * \param btriples - *{btriples} will point to an array of *{ncount}
   *                   bit decomposed triples upon success. On failure
   *                   *{btriples} will be ZERO.
   *
   * \param ncount - on success *{ncount} will contain the number of
   *                 triples loaded into {btriples}.
   *
   *
   * TODO(rwl): Better error reporting please.
   *
   */
  void
  load_bdt(const char * filename,
           BitDecomposedTriple ** btriples, uint * ncount);

  /*!
   * \brief load a set of bit decomposed triples for a one player from
   * {data}.
   *
   * \param data - the serialised triple
   * \param ldata- the length of {data}
   * \param btriples - *{btriples} will store *{ltriples} triples on success.
   *
   * Note: If any error occurres *{btriples} will be ZERO and
   * *{ltriples} will be ZERO.
   */
  DerRC read_bdt(byte * data, uint ldata, 
                 BitDecomposedTriple ** btriples, uint * lbtriples);
  
  /*!
   * \brief saves the shares to a set of files.
   *
   * player one will have file: minimacs_peer0_t<ltriples>_s<lsingles>_p<lpairs>.rep
   * 
   * for up to {nplayer} peers.
   *
   * \param nplayers    - the number of players
   * \param triples     - triples for all players
   * \param singles     - singles for all players
   * \param pairs       - pairs for all players
   *
   * TODO(rwl): We need error reporting.
   */
  void save_shares(char * postfix,  
                   uint nplayers,
                   MiniMacsTripleRep ** triples, uint ltriples,
                   MiniMacsRep ** singles, uint lsingles,
                   MiniMacsRep *** pairs, uint lpairs );

  /*!
   * \brief loads a single set, e.g. for one player from {filename}.
   *
   * \param filename    - the name of a file containing the shares for one player
   * \param triples     - allocated list of triples
   * \param singles     - allocated (by this function) list of singles
   * \param pairs       - allocated list of pairs
   *
   * TODO(rwl): We need error reporting, now zero length's in
   * ltriples, lsingles and lpairs indicate that an error occured or
   * zero items occur.
   *
   */
  void load_shares( OE oe, const char * filename, 
		    MiniMacsTripleRep ** triples, uint * ltriples,
		    MiniMacsRep ** singles, uint * lsingles,
		    MiniMacsRep *** pairs, uint * lpairs );


  /*!
   * \brief same as the counter part with out {encoder}
   * argument. However this version does not create the encoder van
   * der monde matrix.
   *
   * \param encoder - the encoder to use for reed solomon
   * \param text    - message to encode
   * \param ltext   - length of {text}
   * \param codelength - the length of the reed solomon code word.
   *
   * \return A fresh minimacs representation if successful oterwise (MiniMacsRep)0;
   */
  MiniMacsRep minimacs_create_rep_public_plaintext_fast(
                                                        OE oe,
							MiniMacsEnc encoder,
							byte * text,
							uint ltext,
							uint codelength
							);

  /*!
   * \brief as its counter part without the {encoder} argument.
   *
   * \param encoder - the encoder van der monde matrix to use for reed
   * solomon encoding and verification.
   * \param rep  - the representation to add a constant to
   * \param c    - the constant to add
   * \param lc   - the length of {c}
   *
   */
  MiniMacsRep minimacs_rep_add_const_fast(
                                          OE oe,
					  MiniMacsEnc encoder, 
					  MiniMacsRep rep, 
					  byte * c, uint lc 
					  );

  /*!
   * \brief as its counter part with out the {encoder argument.
   *
   * \param enc   - encoder can check codewords
   * \param rep   - the representation held by "this" party.
   * \param other_party_id - the belived id of the "remote" party
   * \param codeword - codeword held by the "remote" party to check vaildity of
   * \param mac      - the MAC 
   * \param lcodeword- length of {codeword} and {mac}
   *
   * \return true if all arguments are proper and {mac} is a valid
   * BeDOZa mac on {codeword}, false otherwise.
   */
  bool minimacs_check_othershare_fast(MiniMacsEnc encoder,
				      MiniMacsRep rep,  
				      uint other_party_id,
				      byte * codeword, 
				      byte * mac, 
				      uint lcodeword);
  
  /*!
   * \brief as its counter part without {encoder}.
   *
   *
   */
  MiniMacsRep minimacs_rep_mul_const_fast(
                                          OE oe, 
                                          MiniMacsEnc encoder, 
                                          MiniMacsRep rep, 
                                          byte * c, uint lc);

  
  /*!
   * \brief mul a constant to the representation in {rep}.
   *
   * \param encoder - minimacs encoder for create codewords
   * \param rep     - representation to which {c} is added
   * \param lval    - the length of the message we encode to
   * \param c       - the constant to add
   * \param lc      - length of {c}
   *
   * \return The sum of {rep} and {c} on success, (void*)0 otherwise.
   *
   *
   */
  MiniMacsRep minimacs_rep_mul_const_fast_lval(OE oe,MiniMacsEnc encoder, 
                                               uint lval,
                                               MiniMacsRep rep, 
                                               byte * c, uint lc);

  /*
   * \brief add constant in {c} fast to {rep}. Encoding will only
   * occur in the {lval} message length. E.g. {lval} is the degreee of
   * the Reed solomon code.
   *
   * \param encoder - minimacs encoder for create codewords
   * \param rep     - representation to which {c} is added
   * \param lval    - the length of the message we encode to
   * \param c       - the constant to add
   * \param lc      - length of {c}
   *
   * \return The sum of {rep} and {c} on success, (void*)0 otherwise.
   *
   */
  MiniMacsRep minimacs_rep_add_const_fast_lval(OE oe,MiniMacsEnc encoder, 
                                               MiniMacsRep rep, 
                                               uint lval,
                                               byte * c, uint lc);
  
  MiniMacsRep minimacs_rep_and_const(OE oe, MiniMacsRep left, byte * c, uint lc);


  /*!
   * \brief as its counter part without small and big encoder.
   *
   * \param encoder       - Encoder, must support the shur transform as well
   * \param left          - all shares for the left operand
   * \param right         - all shares for the right operand
   * \param nplayers      - length of {left} and {right}.
   *
   * \return list of length {nplayers} of representations (one for
   * each player) representing the product of {left} and {right}.
   */
  MiniMacsRep * minimacs_rep_mul_fast(
                                      OE oe,
				      MiniMacsEnc encoder,
				      MiniMacsRep * left, 
				      MiniMacsRep * right, uint nplayers);
  
  /*!
   * \brief given a representtion {rep} this function determine how
   * many players it was preprocessed for.
   *
   * \param rep    - the representation to extract number of players from.
   *
   * \return the number of players rep is meant for, or zero on error.
   *
   */
  uint minimacs_rep_no_players(MiniMacsRep rep);

  /*!
   * \brief given a representation {rep} figure of which player it is
   * for.
   *
   * \param rep   - the representation to examine.
   *
   * \return the id of the player {rep} is for.
   *
   */
  uint minimacs_rep_whoami(MiniMacsRep rep);
#ifdef __cplusplus
}
#endif

#endif
