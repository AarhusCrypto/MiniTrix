/*!
 * The first generic implementation of minimacs.
 * 
 */
#ifndef BITWISEAND_MINIMACS_H
#define BITWISEAND_MINIMACS_H

#include "minimacs.h"
#include <osal.h>
#include <carena.h>
#include <math/matrix.h>
#include <hashmap.h>
#include <reedsolomon/minimacs_enc_fft.h>

#define MUL_FAIL(OE, MSG, ... ) {                                       \
  char _mmm_[128] = {0};                                                \
  osal_sprintf(_mmm_,"MUL(dst=%u,left=%u,right=%u): " MSG,dst,l,r,##__VA_ARGS__); \
  (OE)->p(_mmm_);                                                       \
  mr = 1;                                                               \
  goto failure;}

typedef struct _bitwiseand_minimacs_ {

  // operating environment
  OE oe;
  MR last_error;
  // network squid 
  CArena arena;
  // Raw material
  MiniMacsRep * singles;
  MiniMacsRep ** pairs;
  BitDecomposedTriple * triples;
  uint lsingles;
  uint lpairs;
  uint ltriples;

  // bookeeping mapping id to peer
  uint myid;
  Map peer_map;

  // Reed solomon encoder 
  MiniMacsEnc encoder;
  MiniMacsEnc bitenc;

  // MPC memory
  MiniMacsRep * heap;
  uint lheap;

  MiniMacsRep * constants;

  uint ltext;
  uint lcode;

  uint idx_triple;
  uint idx_single;
  uint idx_pair;
  BitDecomposedTriple (*next_triple)(void);
  MiniMacsRep (*next_single)(void);
  MiniMacsRep * (*next_pair)(void);
  ConnectionListener conn_listener;

  uint mulpar;
  List mulpar_entries;

  MR (*__add__)(MiniMacsRep * res_out, MiniMacsRep left, MiniMacsRep right);
} * BitWiseMulPar2MiniMacs;



/*!
 * GenericMiniMacs constructor
 *
 */
MiniMacs BitWiseMulPar2MiniMacs_New(OE oe, CArena arena, MiniMacsEnc encoder,
                             MiniMacsRep * singles, uint lsingles,
                             MiniMacsRep ** pairs, uint lpairs,
                                    BitDecomposedTriple * triples, uint ltriples,bool do_bit_enc);


/*!
 * Factory method for create GenericMiniMacs instance with default
 * encoder.
 *
 */
MiniMacs BitWiseMulPar2MiniMacs_DefaultNew( OE oe, CArena arena, 
			      MiniMacsRep * singles, uint lsingles,
			      MiniMacsRep ** pairs, uint lpairs,
                                            BitDecomposedTriple * triples, uint ltriples, bool do_bit_enc);



/*!
 * Create a default instance from the raw material in {filename}.
 *
 */
MiniMacs BitWiseMulPar2MiniMacs_DefaultLoadNew(OE oe, const char * filename, 
                                               const char * bdt_filename, bool do_bit_enc);

MiniMacs BitWiseMulPar2MiniMacs_DefaultLoadFFTNew(OE oe, const char * filename, 
                                                  const char * bdt_filename, bool do_bit_enc);

void BitWiseMulPar2MiniMacs_destroy( MiniMacs * instance);

#endif
