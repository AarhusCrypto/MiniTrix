/*!
 * The flag-ship implementation of MiniMac !
 * 
 */
#ifndef BITWISEMULPAR2_MINIMACS_H
#define BITWISEMULPAR2_MINIMACS_H

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

  // Reed solomon encoders
  MiniMacsEnc encoder;
  MiniMacsEnc bitenc;

  // MPC memory
  MiniMacsRep * heap;
  uint lheap;

  uint ltext;
  uint lcode;

  // Raw material book keeping
  uint idx_triple;
  uint idx_single;
  uint idx_pair;
  BitDecomposedTriple (*next_triple)(void);
  MiniMacsRep (*next_single)(void);
  MiniMacsRep * (*next_pair)(void);

  // multiple parallel multiplications
  uint mulpar;
  List mulpar_entries;

  AesPreprocessing prep;

  // internal add 
  MR (*__add__)(MiniMacsRep * res_out, MiniMacsRep left, MiniMacsRep right);

  AesPreprocessing pre;
} * BitWiseMulPar2MiniMacs;


MiniMacs BitWiseMulPar2MiniMacs_NewAesPrep(OE oe, CArena, MiniMacsEnc encoder,
										   AesPreprocessing prep);


/*!
 * Constructor
 *
 */
MiniMacs BitWiseMulPar2MiniMacs_New(OE oe, CArena arena, MiniMacsEnc encoder,
                                    MiniMacsRep * singles, uint lsingles,
                                    MiniMacsRep ** pairs, uint lpairs,
                                    BitDecomposedTriple * triples, uint ltriples,
				    bool do_bit_enc, AesPreprocessing prep);


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
 * \param oe          - operating environment 
 * \param filename    - filename of file containing raw material with singles
 * \param bdt_filename- filename of file containing bit decomposed triples
 * \param do_bit_enc  - do bit decomposition trick or use norm. triples.
 * 
 */
MiniMacs BitWiseMulPar2MiniMacs_DefaultLoadNew(OE oe, 
                                               const char * filename, 
                                               const char * bdt_filename, 
                                               bool do_bit_enc);

/*!
 * \brief This is the absolutely fastests version of MiniMac we can
 * instantiate when {do_bit_enc} is on. Fast Fourier transform is used
 * for encoding.
 *
 * \param oe           - operating environment
 * \param filename     - raw material 
 * \param bdt_filename - decomposed triples
 * \param do_bit_enc   - enable bit encodings
 */
MiniMacs BitWiseMulPar2MiniMacs_DefaultLoadFFTNew(OE oe, const 
                                                  char * filename, 
                                                  const char * bdt_filename, 
                                                  bool do_bit_enc);

/*!
 * \brief Clean up {*instance} assuming we have an instance created
 * with the function above. If not behaviour is undefined but will
 * most likely crash ! Null pointers are guarded for though !
 */
void BitWiseMulPar2MiniMacs_destroy( MiniMacs * instance);

#endif
