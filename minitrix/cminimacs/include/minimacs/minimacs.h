/*!
 * << Interface >>
 *
 * The pure C interface for MiniMacs.
 *
 * Example usage
 *
 void computation(Data secret, Data public_constant) {
 
 MiniMacs minimacs = SomeMiniMacs_new(<material>);
 if (minimacs) {
 
 minimacs->secret_input(1,secret);
 minimacs->public_input(2,public_constant);
 // compute secret + public_constant and store the result in 0
 minimacs->add(0,1,2);
 // compute secret * (secret + public_constant) and store that in 3
 minimacs->mul(3,0,1);
 // open 3
 minimacs->open(3);
 }
 }

 *
 *
 *
 */
#ifndef MINIMACS_H
#define MINIMACS_H

#include <osal.h>
#include <common.h>
#include "minimacs_rep.h"

typedef unsigned int hptr;
struct _observer_;
typedef enum {

  // operation successful
  MR_OK=0,

  // argument is invalid
  MR_BAD_ARGS,

  // heap value not set
  MR_HEAP_NOT_AVAIL,

  // Not implemented
  MR_NOT_IMPL,

  // Peer has left
  MR_PEER_DISCON,

  // xor or and failed
  MR_REP_OP_FAILED,
  
  // Wrong peer
  MR_BAD_PEER,

  // ran out of preprocessing material
  MR_NO_PREP,

  // Out of memory
  MR_NOMEM,

  // Cheate or comm. error detected
  MR_WRONG_MAC,
} MR;

#define MR_RET_OK {\
    MR mr = MR_OK;     \
    return mr;	   \
}



#define MRRF(OE,MSG,...) {                      \
    MR mr = 1;                                  \
    char _mmm_[96] = {0};                       \
    osal_sprintf(_mmm_,(MSG),##__VA_ARGS__);    \
    (OE)->p(_mmm_);                               \
    return mr;}
  
#define MRGF(OE,MSG,...) {                      \
    mr = 1;                                     \
    char _mmm_[96] = {0};                       \
    osal_sprintf(_mmm_,(MSG),##__VA_ARGS__);    \
    (OE)->p(_mmm_);                             \
    goto failure;}                              
  

#define MR_RET_FAIL(OE,MSG){                    \
    MR mr = 1;                                  \
    (OE)->p((MSG));                             \
    return mr;}


typedef enum {
	//
	// AES With 7 state laid out in one codeword,
	// SR = shift rows and SRMC = shift rows mix colums
	//
	AES_7FLAT_SR,AES_7FLAT_SRMC,
	//
	//AES state laid out in eight codewords one for each bit in
	// each byte of the state.
	//
	AES_BIT_DECOMP_SR, AES_BIT_DECOMP_SRMC,
} LinTransID;


typedef struct _minimacs_ {

  /*!
   * Add an observer to this mini macs instance.
   */
  void (*observe_add)(struct _observer_ * obs);

  /*!
   * Notify the world that something has happened.
   *
   */
  void (*notify)( void * );

  /*!
   * When given raw material an id can be determined. Thus function
   * returns which player in the universe this instance plays.
   *
   */
  uint (*get_id)();

  /*!
   * When given raw material the number of players is determined. 
   * 
   */
  uint (*get_no_players)(void);

  /*!
   * After loaded with preprocessing material we can determine the
   * length of plaintext, return by this function. If no single are
   * available zero is returned.
   *
   */
  uint (*get_ltext)();

  /*!
   * After loaded with processing material we can determine the
   * codeword length. The codeword length is returned by this function
   * if any singles are avialble. Otherwise zero is returned.
   */
  uint (*get_lcode)();

  /*!
   * given {left} and {right} heap addresses, perform an ADD
   * instruction saving the result at address {dst}.
   *
   */
  MR (*add)(hptr dst, hptr left, hptr right);

  /*!  Given {left} and {right} compute the representation of the
   *multiplication of the operands and store the result in {dst}.
   */
  MR (*mul)(hptr dst, hptr left, hptr right);


  /*! 
   * Linear transformation can be pre-processed. When special purpose
   * pre-processing is present for a particular transformation MiniMac
   * can execute this extremely fast. Below {kind} identifies which
   * pre-processed transformation to apply to the operand in
   * {left}. The result is stored at {dst} in the heap.
   */
  MR (*lintrans)(LinTransID kind, hptr dst, hptr left);
  
  /*!
   * Indicate the next {count} multiplications can be done in
   * parallel.
   */
  MR (*mulpar)(uint count);

  /*!
   * Perform an input gate for the given value storing it in address
   * {dst}.
   */
  MR (*secret_input)(uint pid, hptr dst, Data plain_val);
  
  /*!
   * Perform an public input gate creating a public constant in the
   * circuit.
   */
  MR (*public_input)(hptr dst, Data pub_val);

  /*!
   * Perform an open gate for the given address. After this the plain
   * value of {dst} is stored in {dst}.
   */
  MR (*open)(hptr dst);

  /*!
   * Initiallize the heap
   */
  MR (*init_heap)(uint size);

  /*!
   * Get the minimacs representation stored at the address {addr}.
   */
  MiniMacsRep (*heap_get)(hptr addr);

  /*!
   * Set the given address 
   *
   */
  MR (*heap_set)(hptr addr, MiniMacsRep rep);

  /*!
   * Block until {count} parties connect on {port}. Then computation
   * can begin.
   */
  MR (*invite)(uint count, uint port);

  /*!
   * Connect to host at address {ip} on {port}.
   */
  MR (*connect)(char * ip, uint port);

  /*!
   * Number of peers
   */
  uint (*get_no_peers)(void);

  void * impl;

} * MiniMacs;

typedef struct _cheetah_decomposed_value_ {
	MiniMacsRep R;
	MiniMacsRep Ri[8];
} * CheetahDVal;

typedef struct _cheetah_sbox_srmc_result {
	MiniMacsRep R;
	MiniMacsRep table[112][256];
} * CheetahSBox;

// five full aes states and sbox(rot(w)) for 
// the last word in each makes 20 per key box
typedef struct _cheetah_key_box_result_ {
  MiniMacsRep R;
  MiniMacsRep T;
  MiniMacsRep KSxT;
  MiniMacsRep table[20][256];
} * CheetahKBox;

typedef struct _cheetah_lintrans_result_ {
  MiniMacsRep R;
  MATRIX * M;
  MiniMacsRep MxR;
} * CheetahLVal;

typedef struct _mult_triple_ {
	MiniMacsRep a;
	MiniMacsRep b;
	MiniMacsRep c;
} * CheetahTriple;

typedef struct _aes_preprocessing_ {

	/*
	 * Return a fresh instance of preprocessed material for
	 * S-Box with Shift-rows and Mix-columns.
	 */
	CheetahSBox(*get_sbox_srmc)(void);

	/*
	* Fresh instance of preprocessed s-box with only Shift-rows applied,
	* for the final round.
	*/
	CheetahSBox(*get_sbox_sr)(void);

	/*
	* Return a fresh random value and its
	* bit decomposition.
	*/
	CheetahDVal(*get_decom_val)(void);

	/*
	* Return a single
	*/
	MiniMacsRep(*get_single)(void);

	/*
	 * Linear transformation of random representation
	 *
	 * Pre-processing supports different kinds of linear transformations
	 * depending on what we are doing online, e.g. some might be dedicated.
	 */
	CheetahLVal(*get_lintrans)(LinTransID kind);

	/*
	* Retrieve the number of players this pre-processing material
	* is prepared for.
	*/
	uint(*get_nplayers)(void);

	/*
	* Get the identifier for which player this material were prepared.
	*/
	uint(*get_playerid)(void);

	/*
	* Get codeword length
	*/
	uint(*get_lcode)(void);

	/*
	* Get message length, the size in whole bytes that we can compute on
	*/
	uint(*get_lmsg)(void);
  
        CheetahKBox(*get_key_box)(void);

	/*
	 * In preprocessing material a string describing its purpose can be found.
	 * This function fetches and returns this string.
	 */
	char *(*get_purpose)(void);
  

	// private details hidden from the client
	void * impl;
} *AesPreprocessing;


typedef struct _observer_ {
  MiniMacs instance;
  void(*notify)(void * data);
} * Observer;

Observer Observer_DefaultNew( OE oe,  void (*fn)(void *data) );



#endif
