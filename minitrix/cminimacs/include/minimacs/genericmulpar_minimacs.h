/*!
 * The first generic implementation of minimacs.
 * 
 */
#ifndef GENERICMULPAR_MINIMACS_H
#define GENERICMULPAR_MINIMACS_H

#include "minimacs.h"
#include <osal.h>
#include <carena.h>
#include <math/matrix.h>
#include <hashmap.h>

#define MUL_FAIL(OE, MSG, ... ) {                                       \
  char _mmm_[128] = {0};                                                \
  osal_sprintf(_mmm_,"MUL(dst=%u,left=%u,right=%u): " MSG,dst,l,r,##__VA_ARGS__); \
  (OE)->p(_mmm_);                                                       \
  mr = 1;                                                               \
  goto failure;}

typedef struct _genericmulpar_minimacs_ {

  // operating environment
  OE oe;
  MR last_error;
  // network squid 
  CArena arena;
  // Raw material
  MiniMacsRep * singles;
  MiniMacsRep ** pairs;
  MiniMacsTripleRep * triples;
  uint lsingles;
  uint lpairs;
  uint ltriples;

  // bookeeping mapping id to peer
  uint myid;
  Map peer_map;

  // Reed solomon encoder 
  MiniMacsEnc encoder;

  // MPC memory
  MiniMacsRep * heap;
  uint lheap;

  uint ltext;
  uint lcode;

  uint idx_triple;
  uint idx_single;
  uint idx_pair;
  MiniMacsTripleRep (*next_triple)(void);
  MiniMacsRep (*next_single)(void);
  MiniMacsRep * (*next_pair)(void);
  ConnectionListener conn_listener;
  int mulpar;
  List mulpar_entries;
  MR (*__add__)(MiniMacsRep * res_out, MiniMacsRep left, MiniMacsRep right);
} * GenericMulParMiniMacs;



/*!
 * GenericMulParMiniMacs constructor
 *
 */
MiniMacs GenericMulParMiniMacs_New(OE oe, CArena arena, MiniMacsEnc encoder,
                             MiniMacsRep * singles, uint lsingles,
                             MiniMacsRep ** pairs, uint lpairs,
                             MiniMacsTripleRep * triples, uint ltriples);


/*!
 * Factory method for create GenericMulParMiniMacs instance with default
 * encoder.
 *
 */
MiniMacs GenericMulParMiniMacs_DefaultNew( OE oe, CArena arena, 
			      MiniMacsRep * singles, uint lsingles,
			      MiniMacsRep ** pairs, uint lpairs,
			      MiniMacsTripleRep * triples, uint ltriples);



/*!
 * Create a default instance from the raw material in {filename}.
 *
 */
MiniMacs GenericMulParMiniMacs_DefaultLoadNew(OE oe, const char * filename);

void GenericMulParMiniMacs_destroy( MiniMacs * instance);

#endif
