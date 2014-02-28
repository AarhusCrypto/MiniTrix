/*!
 * << Interface >>
 *
 * The pure C interface for MiniMacs.
 *
 Example usage
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
typedef uint MR;


#define MR_RET_OK {\
    MR mr = 0;     \
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


typedef struct _observer_ {
  MiniMacs instance;
  void(*notify)(void * data);
} * Observer;

Observer Observer_DefaultNew( OE oe,  void (*fn)(void *data) );

#endif
