#ifndef MINIMACS_PRE_H
#define MINIMACS_PRE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "minimacs/minimacs_rep.h"




  void minimacs_fake_bdt(OE oe, MiniMacsEnc encoder,
                         MiniMacsRep * proto,
                         BitDecomposedTriple *** triples,
                         uint ltriples);


typedef enum {
  SINGLES,
  PAIRS,
  TRIPLES
} SetupEventKind;

typedef void(*SetupListener)(SetupEventKind kind, uint count, uint total);
  /*!
   * \brief               Create Preprocessing data for MiniMacs Online-phase.
   *
   * \param ltext         Length of the plaintext in the representation
   * \param nplayers      Number of parties to preprocess for
   * \param codelength    Length of the codeword
   * \param triples       Will on success point to {ltriples}
   * \param singles       Will on success point to {lsingles} 
   * \param pairs         Will on success point to {lpairs}
   * \param listener      UI or anything else waiting for progress 
   *                      may provide a listener.
   * 
   *
   * \return compat, a pointer to the list of all the shares for the
   * first single to enable further consistent preprocessing.
   *
   * This function uses {malloc} for allocation. Called is responsible for
   * freeing with free.
   *
   */
MiniMacsRep * minimacs_fake_setup(
                                  OE oe,
                                  MiniMacsEnc encoder,
                                  uint ltext, uint nplayers, uint codelength, 
                                  MiniMacsTripleRep *** triples, uint ltriples,
                                  MiniMacsRep *** singles, uint lsingles,
                                  MiniMacsRep **** pairs, uint lpairs,
                                  SetupListener listener);


  /*!
   *\brief              Teardown variables typically allocated during preprocessing.
   *
   * \param             nplayers, how many players are in the computation
   * \param             triples, multiplication triples used
   * \param             singles, single values used 
   * \param             pairs, pairs used
   * 
   */
  void minimacs_fake_teardown(OE oe, uint nplayers,
			      MiniMacsTripleRep *** triples, uint ltriples,
			      MiniMacsRep *** singles, uint lsingles,
			      MiniMacsRep **** pairs, uint lpairs,
			      MiniMacsRep * compat); 
    

			      

  /*!
   * \brief given the preprocessed triples, pairs and singles store
   * the set needed for one particular player.
   *
   * \param nplayers     - the total number of players
   * \param triples      - the multi triples
   * \param ltripes      - length of {triples}
   * \param singles      - singles to store
   * \param lsingles     - length of {singles}
   * \param pairs        - the pairs <s>,<s>*
   * \param lpairs       - length of pairs
   * \param data_out     - buffer allocated by malloc with the serialised data
   * \param ldata_out    - length of {data_out}
   *
   */
  void minimacs_store_pre(uint nplayers, 
			  MiniMacsTripleRep * triples, uint ltriples,
			  MiniMacsRep * singles, uint lsingles,
			  MiniMacsRep ** pairs, uint lpairs,
			  byte ** data_out, uint * ldata_out);

  /*!
   * \brief loads the serialised data in {data} into triples, singles
   * and epairs assuming the right format. The number of successfully
   * loaded triples, singles and pairs are returned.
   *
   * This function fails with reporting zero loaded triples, pairs and
   * singles.
   *
   * \param data           - the serialised data
   * \param ldata          - length of {data}
   * \param triples_out    - the loaded triples e.g. (*triple_out)[0] is 
   *                         the first etc.
   * \param ltriples_out   - length of {triples_out}
   * \param singles_out    - the loaded singles
   * \param lsingles_out   - length of {singles_out}
   * \param pairs_out      - the loaded pairs
   * \param lpairs_out     - length of {pairs_out}
   *
   * 
   */
  void minimacs_load_pre(byte * data, uint ldata, 
			 MiniMacsTripleRep ** triples_out,uint * ltriples_out,
			 MiniMacsRep ** singles_out,uint * lsingles_out,
			 MiniMacsRep *** pairs_out,uint * lpairs_out); 
			 
			 
			  
#ifdef __cplusplus
}
#endif

#endif
