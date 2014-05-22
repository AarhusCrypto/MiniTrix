#include "minimacs/bitwisemulpar2_minimacs.h"
#include "reedsolomon/minimacs_bitfft_encoder.h"
#include "reedsolomon/minimacs_bit_encoder.h"
#include <coo.h>
#include <common.h>
#include <math/matrix.h>
#include <blockingqueue.h>
#include <config.h>
#include "stats.h"
#include <singlelinkedlist.h>


COO_DCL(MiniMacs, uint, get_id);
COO_DEF_RET_NOARGS(MiniMacs, uint, get_id) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  
  if (gmm->singles) {
    return minimacs_rep_whoami( gmm->singles[0] );
  } else {
    oe->p("I have no singles and thus no id :(");
  }

  return 0;
}

COO_DCL(BitWiseMulPar2MiniMacs, BitDecomposedTriple, next_triple)
COO_DEF_RET_NOARGS(BitWiseMulPar2MiniMacs, BitDecomposedTriple, next_triple) {
  if (this->idx_triple < this->ltriples) {
    return this->triples[this->idx_triple++];
  }
  return 0;
}

COO_DCL(BitWiseMulPar2MiniMacs, MiniMacsRep, next_single)
COO_DEF_RET_NOARGS(BitWiseMulPar2MiniMacs, MiniMacsRep, next_single) {
  if (this->idx_single < this->lsingles) {
    return this->singles[this->idx_single++];
  }
  return 0;
}

COO_DCL(BitWiseMulPar2MiniMacs, MiniMacsRep *, next_pair) 
COO_DEF_RET_NOARGS(BitWiseMulPar2MiniMacs, MiniMacsRep *, next_pair) {
  if (this->idx_pair < this->lpairs) {
    return this->pairs[this->idx_pair++];
  }
  return 0;
}

COO_DCL(MiniMacs,MR,add,uint dst, uint l, uint r)
COO_DEF_RET_ARGS(MiniMacs, MR, add, uint dst; uint l; uint r;,dst,l,r) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  MiniMacsRep left = this->heap_get(l);
  MiniMacsRep right = this->heap_get(r);
  MiniMacsRep res = 0;
  MR mr = 0;
  
  if (gmm->peer_map->size() < 1) MRRF(oe,"[add] No peers connected.");

  if (!left) MRRF(oe,"[add] Left operand (%d) was not set.",l);
  if (!right) MRRF(oe, "[add] Right operand (%d) was not set.", r);

  mr = gmm->__add__(&res,left,right);
  if (res == 0) MRRF(oe, "[add] Failed with null.");
  if (mr != 0) return mr;
  this->heap_set(dst,res);
  return mr;
}

COO_DCL(BitWiseMulPar2MiniMacs, MR, __add__, MiniMacsRep * res_out, MiniMacsRep left, MiniMacsRep right)
COO_DEF_RET_ARGS(BitWiseMulPar2MiniMacs, MR, __add__,MiniMacsRep * res_out; MiniMacsRep left; MiniMacsRep right;,res_out, left, right) {
  OE oe = this->oe;
  bool left_const = minimacs_rep_is_public_const(left);
  bool right_const = minimacs_rep_is_public_const(right);
  MiniMacsRep result = 0;

  if (!res_out) {
    MRRF(oe,"Programming error no pointer to store result");
  }
  if (!left) {
    MRRF(oe,"Left operand not set");
  }

  if (!right) {
    MRRF(oe,"Right operand not set");
  }

  // left is proper rep and right is a public constant
  if (left_const && !right_const) {
    MiniMacsEnc enc = this->encoder;
    result = minimacs_rep_add_const_fast_lval(enc, right, this->ltext,
                                              left->codeword, left->lval);
  }

  // left is a public constant and right is a proper rep
  if (!left_const && right_const) {
    MiniMacsEnc enc = this->encoder;
    result = minimacs_rep_add_const_fast_lval(enc, left, this->ltext,
                                              right->codeword, right->lval);
  }


  // The case where we have two public constants
  if (left_const && right_const) {
    uint i = 0;
    byte * ntxt = (byte*)oe->getmem(left->lval);
    MiniMacsEnc enc = this->encoder;
    for(i = 0;i < left->lval;++i) {
      ntxt[i] = left->codeword[i] ^ right->codeword[i];
    }
    result = minimacs_create_rep_public_plaintext_fast(enc, ntxt, left->lval, left->lcodeword);
  }

  // The non trivial case with to proper reps
  if (!left_const && !right_const) {
    result = minimacs_rep_xor(left, right);
  }

  if (!result) {
    MRRF(oe,"Result is null add failed. left_const=%u,right_const=%u\n",left_const, right_const);
  }

  *res_out = result;
  MR_RET_OK;
}


typedef struct _mulpar_entry_ {
  OE oe;
  uint dst;
  uint l;
  uint r;
} * MPE;

MPE MulParEntry_New(OE oe, uint dst, uint l, uint r) {
  MPE result = (MPE)oe->getmem(sizeof(*result));
  result->oe = oe;
  result->dst = dst;
  result->l = l;
  result->r = r;
  return result;
}

void MulParEntry_Destroy(MPE * e) {
  OE oe = 0;
  if (!e) return;
  if (!*e) return;
  
  oe = (*e)->oe;

  oe->putmem(*e);
  *e = 0;
}


COO_DCL(MiniMacs,MR,mul,uint dst, uint l, uint r) 
COO_DEF_RET_ARGS(MiniMacs,MR, mul, uint dst; uint l; uint r;,dst,l,r) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  MiniMacsRep left = this->heap_get(l);
  MiniMacsRep right = this->heap_get(r);
  bool left_const = minimacs_rep_is_public_const(left);
  bool right_const = minimacs_rep_is_public_const(right);
  MR mr = 0;
  MiniMacsRep result = 0;

  BitDecomposedTriple * triples = 0;
  MiniMacsRep * deltas = 0;
  Data deltas_send = 0, deltas_in = 0, deltas_clear = 0;
  Data epsilons_send = 0, epsilons_in = 0, epsilons_clear = 0;
  MiniMacsRep * epsilons = 0;
  MpcPeer peer = 0;
  MiniMacsRep tmp = 0, tmp2=0;

  byte * ed_raw = 0;
  MiniMacsRep ae = 0, db = 0;
  MPE mpe = 0;

  uint mulcount = 0, mulcounter= 0, lcode = 0, id = 0, i=0, nplayers = 0;

  if (!left) MUL_FAIL(oe,"[mul] Left operand (%d) is not set.", l);

  if (!right) MUL_FAIL(oe,"[mul] Right operand (%d) is not set.", r);

  gmm->mulpar--;

  // Both public constants
  if (left_const && right_const) {
    uint j = 0;
    Data tmpd = Data_new(gmm->oe,left->lval);
    for(j = 0;j < left->lval;++j) {
      tmpd->data[j] = multiply(left->codeword[j],right->codeword[j]);
    }
   
    result = minimacs_create_rep_public_plaintext_fast(gmm->encoder,tmpd->data, tmpd->ldata, left->lcodeword);
    if (!result) MUL_FAIL(oe,"%u Result gave null, some condition is wrong.",__LINE__);
    
    
    this->heap_set(dst, result);
    Data_destroy(gmm->oe,&tmpd);
    
    
    //
    // NOTICE(rwl): Constant multiplication are dealt with immediately
    // however if this is the last multiplication in a mulpar sequence
    // we need to complete all the cached up multiplications. Thus, we
    // cannot return.
    //
    if (gmm->mulpar > 0)
      MR_RET_OK;
    
  } else {
    //
    // Ok we have a non-constant add it to the list of cached up work
    // in mulpar_entries. If this isn't the last in a mulpar section
    // we leave with success.
    //
    //

    mpe = MulParEntry_New(oe, dst, l ,r );
    if (!mpe) {
      MUL_FAIL(oe,"No more memory :(");
    }

    gmm->mulpar_entries->add_element( mpe );

    if (gmm->mulpar > 0) {
      MR_RET_OK;
    }
    
  }
  
  // Okay nice ! we have a cached up list of work ahead of us.

  // 1] Build everything to send

  // 2] Receive everything 

  // 3] Do the computation

  // 4] Send all the sigmas
  
  // 5] Receive all the sigmas

  // do {mulcount} multiplications in the following
  mulcount = gmm->mulpar_entries->size();
  lcode = this->get_lcode();


  {
    char _m[64] = {0};
    osal_sprintf(_m,"%u times mul",mulcount);
    oe->p(_m);
  }


  if (mulcount <= 0) { 
    oe->syslog(OSAL_LOGLEVEL_WARN,"Mulcount is zero");
    gmm->mulpar = 1;
    MR_RET_OK;
  }

  CHECK_POINT_S("Non-constant-AND");
  triples = oe->getmem(mulcount*sizeof(*triples));
  if (!triples) { MUL_FAIL(oe, "No more memory :(");}

  deltas = oe->getmem(mulcount*sizeof(*deltas));
  if (!deltas) { MUL_FAIL(oe, "No more memory :( "); }

  epsilons = oe->getmem(mulcount*sizeof(*epsilons));
  if (!epsilons) { MUL_FAIL(oe, "No more memory :( "); }


  for(mulcounter = 0;mulcounter < mulcount;++mulcounter) {
    mpe = (MPE) gmm->mulpar_entries->get_element(mulcounter);
    MiniMacsRep mul_left  = this->heap_get(mpe->l);
    MiniMacsRep mul_right = this->heap_get(mpe->r);

    if (mul_left == 0) {
      MUL_FAIL(oe, "Left operand (%u) is null.",mpe->l);
    }

    if (mul_right == 0) {
      MUL_FAIL(oe, "right operand (%u) is null.", mpe->r);
    }

    triples[mulcounter] = gmm->next_triple();
    if (!triples[mulcounter]) { 
      MUL_FAIL(oe,"No more triples (%d taken).", gmm->idx_triple);
    }
  
    mr = gmm->__add__(&deltas[mulcounter],triples[mulcounter]->a, mul_left);
    if (mr != 0) MUL_FAIL(oe,"Could not add triple->a and left");
    
    mr = gmm->__add__(&epsilons[mulcounter],triples[mulcounter]->b, mul_right);
    if (mr != 0) MUL_FAIL(oe,"Could not add triple->b and right");
    
  }

  // first {mulcount} codeword then {mulcount} macs for each other player
  deltas_send = Data_new(oe, 2*lcode*mulcount);
  if (!deltas_send) {
    MUL_FAIL(oe, "No more memory.");
  }

  epsilons_send = Data_new(oe, 2*lcode*mulcount);
  if (!epsilons_send) {
    MUL_FAIL(oe, "No more memory.");
  }


  for(mulcounter = 0; mulcounter < mulcount;++mulcounter) {
    uint off = (lcode*mulcounter);
    mcpy(deltas_send->data+off, deltas[mulcounter]->codeword,lcode);
    mcpy(epsilons_send->data+off, epsilons[mulcounter]->codeword,lcode);
  }
  
  
    // build one big codeword package

    // build one big mac package
    
    // broadcast delta and epsilon
    for(id = 0;id < gmm->peer_map->size()+1;++id) {
      if (id == gmm->myid) continue; 
      
      peer = gmm->peer_map->get( (void*)(unsigned long long)id );
      if (!peer) MUL_FAIL(oe,"Peer with id, %u, is undefined",id);
      
      for(mulcounter = 0; mulcounter < mulcount;++mulcounter) {
        uint off = mulcount*lcode+lcode*mulcounter;
        mcpy(deltas_send->data+off,deltas[mulcounter]->mac[id]->mac,lcode);
        mcpy(epsilons_send->data+off,epsilons[mulcounter]->mac[id]->mac,lcode);
      }

      // One giant burst to each peer
      peer->send(deltas_send);
      peer->send(epsilons_send);
    }

    // From each party we will receive a giant package. This package
    // is mulpar*2*{lcode} long and contains {lcode}-codewords followed by
    // {lcode}-macs.

    nplayers = gmm->peer_map->size()+1;
    deltas_in = Data_new(oe, lcode*2*mulcount);
    epsilons_in = Data_new(oe, lcode*2*mulcount);
    deltas_clear = Data_new(oe, lcode*mulcount);
    epsilons_clear = Data_new(oe, lcode*mulcount);

    // receive and check epsilon and delta from every one else.
    for(id = 0;id<gmm->peer_map->size()+1;++id) {
      if (id == gmm->myid) continue;
      
      peer = gmm->peer_map->get( (void*)(ull)id);
      if (!peer) MUL_FAIL(oe,"Peer with id, %u, is undefined (after sending to him)", id);

      peer->receive(deltas_in);
      if (deltas_in->ldata != lcode*2*mulcount) {
        MUL_FAIL(oe, "deltas in has wrong length %u expected %u",
                 deltas_in->ldata,lcode*2*mulcount);
      }

      peer->receive(epsilons_in);
      if (epsilons_in->ldata != lcode*2*mulcount) {
        MUL_FAIL(oe, "epsilons in has wrong length %u expected %u",
                 epsilons_in->ldata, lcode*2*mulcount);
      }
           

      for(mulcounter = 0;mulcounter < mulcount;++mulcounter) {

        if (!minimacs_check_othershare_fast(gmm->encoder,deltas[mulcounter],
                                           id,
                                           deltas_in->data+(mulcounter*lcode),
                                           deltas_in->data+(lcode*mulcount+
                                                           lcode*mulcounter),
                                           lcode)) {
          MRGF(oe,"Peer %u is cheating, mac didn't check out on delta. (lval=%u)",id,deltas[mulcounter]->lval);
        };

        if (!minimacs_check_othershare_fast(gmm->encoder, epsilons[mulcounter],
                                           id,
                                           epsilons_in->data+(mulcounter*lcode),
                                           epsilons_in->data+(lcode*mulcount +
                                                              lcode*mulcounter),
                                           lcode)) {
          MRGF(oe,"Peer %u is cheating, mac didn't check out on epsilon.",id);
        }
      

        // we'll have a mulpar*{lcodeword} clear delta and epsilon vector
        for(i = 0;i < lcode;++i) {
          deltas_clear->data[lcode*mulcounter+i] = 
            add(deltas_clear->data[lcode*mulcounter+i],
                deltas_in->data[lcode*mulcounter+i]);
        }
        
        for(i = 0;i < lcode;++i) {
          epsilons_clear->data[mulcounter*lcode+i] = 
            add(epsilons_clear->data[mulcounter*lcode+i],
                epsilons_in->data[mulcounter*lcode+i]);
        }
      }
    } // done receiving from all players

    Data_destroy(oe,&deltas_in);
    Data_destroy(oe,&epsilons_in);
    
    // again mulpar*{lcodeword} length vectors

    for(mulcounter = 0;mulcounter < mulcount;++mulcounter) {
      //compute clear text delta
      for(i = 0;i<deltas[mulcounter]->lcodeword;++i) {
        deltas_clear->data[mulcounter*lcode+i] = 
          add(deltas[mulcounter]->dx_codeword[i],
              add(deltas[mulcounter]->codeword[i], 
                  deltas_clear->data[mulcounter*lcode+i]));
      }
      minimacs_rep_clean_up( & deltas[mulcounter] );

      // compute clear text epsilon
      for(i = 0;i < epsilons[mulcounter]->lcodeword;++i) {
        epsilons_clear->data[mulcounter*lcode+i] = 
          add(epsilons_clear->data[lcode*mulcounter+i],
              add(epsilons[mulcounter]->dx_codeword[i],
                  epsilons[mulcounter]->codeword[i]));
      }
      minimacs_rep_clean_up( &epsilons[mulcounter] );
    }

    // ------------------------------------------------------------
    // We have epsilon(s) and delta(s) in clear
    // ------------------------------------------------------------

    // [  i] compute epsilon ^ delta = ed
    // [ ii] compute epsilon ^ [a_i] my share of a = ea
    // [iii] compute delta ^ [b] my share of b = db
    // [ iv] compute triple->c (x) ea (x) db (x) ed

    // [   i] compute ed


    // we'll have lval*{lcodeword} long vectors here
    for(mulcounter = 0; mulcounter < mulcount;++mulcounter) {
      mpe = (MPE) gmm->mulpar_entries->get_element(mulcounter);
      MiniMacsRep mul_left  = this->heap_get(mpe->l);
      MiniMacsRep mul_right = this->heap_get(mpe->r);
      MiniMacsEnc enc = (gmm->bitenc == 0 ? gmm->encoder : gmm->bitenc);


      ed_raw = oe->getmem(this->get_ltext());
      for(i = 0;i < this->get_ltext();++i) {
        ed_raw[i] = epsilons_clear->data[mulcounter*lcode+i] & 
          deltas_clear->data[mulcounter*lcode+i];
      }

      // we'll work on mulpar*{lcodeword} vectors instead 
      // having mulpar triples lined up
      for( i = 0; i < 8;++i ) {
        uint  j = 0;
        byte m_i = (1 << i);
        byte * e_i = oe->getmem(mul_left->lcodeword);
        byte * d_i = oe->getmem(mul_left->lcodeword);
        
        for(j = 0;j < this->get_lcode();++j) {
          e_i[j] = (epsilons_clear->data[mulcounter*lcode+j] & m_i) == 0 ? 0 : 1;
          d_i[j] = (deltas_clear->data[lcode*mulcounter+j] & m_i) == 0 ? 0 : 1;
        }
        
        
        tmp = minimacs_rep_mul_const_fast_lval(enc,
                                              this->get_ltext(),
                                              triples[mulcounter]->abits[i],
                                              e_i,
                                              this->get_ltext());
        
        if (ae == 0) ae = tmp;
        else {
          MiniMacsRep tmp2 = ae;
          ae=minimacs_rep_xor(ae,tmp);
          minimacs_rep_clean_up( &tmp );
          minimacs_rep_clean_up( &tmp2 );
        }
        

        tmp = minimacs_rep_mul_const_fast_lval(enc,
                                               this->get_ltext(),
                                               triples[mulcounter]->bbits[i],
                                               d_i, 
                                               this->get_ltext());
        
        if (db == 0) db = tmp; 
        else {
          MiniMacsRep tmp2 = db;
          db = minimacs_rep_xor(db,tmp);
          minimacs_rep_clean_up( &tmp );
          minimacs_rep_clean_up( &tmp2 );
        }
      
        oe->putmem(e_i);
        oe->putmem(d_i);
      }

      result = minimacs_rep_xor(db,ae);
      tmp = result;
      result = minimacs_rep_xor(triples[mulcounter]->c ,result);
      minimacs_rep_clean_up(&tmp);tmp = result;
      result = minimacs_rep_add_const_fast_lval(gmm->encoder,
                                                result, 
                                                this->get_ltext(),
                                                ed_raw,
                                                this->get_ltext());
      minimacs_rep_clean_up(&tmp);
      result->lval = this->get_ltext()*2;


    if (!result) {
      oe->p("Oh, result is null\n");
      return -1;
    }

    /*
    {
      char _m[64] = {0};
      osal_sprintf(_m,"mul(%u) is non null.",mpe->dst);
      oe->p(_m);
    }
    */
    this->heap_set(mpe->dst, result);
    

    }
      
    oe->putmem(deltas);
    oe->putmem(epsilons);
    
    // clear up and reset for next mul
    gmm->mulpar = 1;
    for(mulcounter = 0; mulcounter < mulcount; ++mulcounter) {
      MPE mpe = (MPE)gmm->mulpar_entries->get_element(mulcounter);
      MulParEntry_Destroy(&mpe);
    }
    
    SingleLinkedList_destroy(&gmm->mulpar_entries);
    gmm->mulpar_entries = (List)SingleLinkedList_new(oe);
    // leave ok
    CHECK_POINT_E("Non-constant-AND");
    MR_RET_OK;
  failure:
    oe->syslog(OSAL_LOGLEVEL_FATAL,"Multiplication failed");
    return mr;
}



COO_DCL(MiniMacs,MR,secret_input, uint pid, hptr dst, Data plain_val)
COO_DEF_RET_ARGS(MiniMacs,MR,secret_input, uint pid; hptr dst; Data plain_val;, pid,dst,plain_val) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  MiniMacsRep r = 0;
  Data * shares = 0;
  Data sharemac = 0;
  Data clear_r = 0;
  Data encoded_val = 0;
  Data encoded_r = 0;
  Data epsilon = 0;
  MiniMacsRep result = 0;
  MpcPeer peer = 0;
  MR mr = 0;
  uint id=0,i=0;
  Data val = 0;

  if (gmm->peer_map->size() < 1) MRRF(oe,"[secret input] No peers connected.");

  r = gmm->next_single();
  if (!r) MRGF(oe,"Ran out of singles, unable to input");
  

  // ------------------------------------------------------------
  // PROTOCOL Branching
  // ------------------------------------------------------------
  if (gmm->myid == pid) {
    shares = oe->getmem(sizeof(*shares)*(gmm->peer_map->size()+1));
    if (!shares) MRGF(oe,"Ran out of memory");

    val = Data_new(oe, r->lval);
    if (!val) MRGF(oe,"Ran out of memory during input.");
    
    if (plain_val->ldata > val->ldata) 
      MRGF(oe,"Raw preprocessing material is setup for messages up length %u bytes."
           "The given message is too long (%u).",plain_val->ldata,val->ldata); 
  
    mcpy(val->data, plain_val->data, plain_val->ldata);


    shares = (Data *)oe->getmem(sizeof(*shares)*(gmm->peer_map->size()+1));
    if (!shares) MRGF(oe,"Ran out of memory.");

    for(id = 0;id < gmm->peer_map->size()+1;++id) {
      shares[id] = Data_new(oe, r->lcodeword);
      if (!shares[id]) MRGF(oe,"Ran out of memory for shares %u.",id);
    }
    sharemac = Data_new(oe, r->lcodeword);
    if (!sharemac) MRGF(oe,"Ran out of memory for sharemac.");

    clear_r = Data_new(oe,r->lval);
    if (!clear_r) MRGF(oe,"Ran out of memory allocating clear_r.");
    
    for(id = 0; id < gmm->peer_map->size()+1;++id) {
      if (gmm->myid == id) {
        continue;
      }

      peer = gmm->peer_map->get( (void*)(ull)id);
      if (!peer) MRGF(oe,"Peer %u disconnected aborting computation",id);

      peer->receive(shares[id]);
      peer->receive(sharemac);

      if (!minimacs_check_othershare_fast(gmm->encoder,
					  r, 
					  id, 
					  shares[id]->data,
					  sharemac->data,
					  shares[id]->ldata)) {
        MRGF(oe,"Peer %u is cheating, mac does not check out on randomness r.",id);
      }

      for(i = 0;i < clear_r->ldata;++i) {
        clear_r->data[i] = add(clear_r->data[i],shares[id]->data[i]);
      }
      Data_destroy(oe,&shares[id]);
    }
    for(i = 0;i < clear_r->ldata;++i) {
      clear_r->data[i] = add(clear_r->data[i], add(r->codeword[i], r->dx_codeword[i]));
    }
    
    for(id=0;id<gmm->peer_map->size()+1;++id) {
      Data_destroy(oe, &shares[id]);
    }
    oe->putmem(shares);shares=0;

    encoded_val = Data_new(oe, r->lcodeword);
    encoded_r = Data_new(oe, r->lcodeword);
    encoded_val->data = gmm->encoder->encode(val->data, val->ldata);
    encoded_val->ldata = r->lcodeword;
    if (!encoded_val->data) MRGF(oe,"Failed to encode input value.");
    encoded_r->data = gmm->encoder->encode(clear_r->data, clear_r->ldata);
    encoded_r->ldata = r->lcodeword;
    if (!encoded_r->data) MRGF(oe,"Failed to encode single.");
    Data_destroy(oe,&clear_r);
    epsilon = Data_new(oe,r->lcodeword);
    if (!epsilon) MRGF(oe,"Ran out of memory during open operation.");

    for(i=0;i<encoded_val->ldata;++i) {
      epsilon->data[i] = add(encoded_r->data[i], encoded_val->data[i]);
    }
    Data_destroy(oe,&encoded_r);
    Data_destroy(oe,&encoded_val);

    for(id = 0;id < gmm->peer_map->size()+1;++id) {
      if (gmm->myid == id) continue;

      peer = (MpcPeer)gmm->peer_map->get( (void*)(ull)id);
      peer->send(epsilon);
    }
    oe->p("About to complete secret input");

    result = minimacs_rep_add_const_fast(gmm->encoder,r,epsilon->data, r->lval);
    minimacs_rep_clean_up(&r);    
    if (!result) MRGF(oe,"Unable to add constant to the random a.");


    mr = this->heap_set(dst, result);
    if (mr != 0) return mr;
    MR_RET_OK;
    
  } else {
    // ------------------------------------------------------------
    // Hey I am not the inputter
    // ------------------------------------------------------------
    peer = gmm->peer_map->get( (void*)(ull)pid);
    if (!peer) MRGF(oe,"Peer %u is disconnected",pid);

    peer->send(Data_shallow(r->codeword,r->lcodeword));
    peer->send(Data_shallow(r->mac[pid]->mac, r->mac[pid]->lmac));
    epsilon = Data_new(oe,r->lcodeword);
    peer->receive(epsilon);
    if (!gmm->encoder->validate((polynomial*)epsilon->data, r->lval)) {
      MRGF(oe,"Data received during input for peer %u was not a codeword.",pid);
    }

    result = minimacs_rep_add_const_fast(gmm->encoder,r,epsilon->data, r->lval);
    if (!result) MRGF(oe,"Failed to perform input, add constant to form result failed.");
    Data_destroy(oe,&epsilon);
    minimacs_rep_clean_up(&r);
    this->heap_set(dst, result);
    MR_RET_OK;
  }
 failure:
  minimacs_rep_clean_up( &r);
  if (shares) {
    for(id = 0;id < gmm->peer_map->size()+1;++id) {
      Data_destroy(oe,&shares[id]);
    }
    oe->putmem(shares);shares = 0;
  }
  Data_destroy(oe,&sharemac);
  Data_destroy(oe,&clear_r);
  Data_destroy(oe,&encoded_val);
  Data_destroy(oe,&encoded_r);
  Data_destroy(oe,&epsilon);
  minimacs_rep_clean_up(&result);

  return mr;
}

COO_DCL(MiniMacs,MR,public_input, hptr dst, Data pub_val)
COO_DEF_RET_ARGS(MiniMacs,MR,public_input, hptr dst; Data pub_val;,dst,pub_val) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  MiniMacsRep vrep = minimacs_create_rep_public_plaintext_fast(gmm->encoder, pub_val->data, pub_val->ldata, gmm->singles[0]->lcodeword);
  this->heap_set(dst, vrep);

  MR_RET_OK;
 failure:
  MR_RET_OK;
}

COO_DCL(MiniMacs,MR,open, hptr dst)
COO_DEF_RET_ARGS(MiniMacs,MR,open,hptr dst;,dst) {
  uint i = 0;
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  Data * shares = 0;
  OE oe = gmm->oe;
  MiniMacsRep v = this->heap_get(dst);
  MR mr = 0;
  uint id = 0;
  MpcPeer peer = 0;
  Data clear = 0;
  Data sharemac = 0;
  MiniMacsRep result = 0;

  if (gmm->peer_map->size() > 1) MRRF(oe,"No peers connected.");

  if(!v) MRGF(oe,"Value at %u is not set",dst);

  if (minimacs_rep_is_public_const(v)) return 0;

  shares = (Data *)oe->getmem(sizeof(*shares)*(gmm->peer_map->size()+1));
  if (!shares) MRGF(oe,"Failed to allocate shares for opening.");

  for(id = 0;id < gmm->peer_map->size()+1;++id) {
    shares[id] = Data_new(oe, v->lcodeword);
    if (!shares[id]) MRGF(oe,"Ran out of memory.");
  }
  sharemac = Data_new(oe,v->lcodeword);
  clear = Data_new(oe,v->lval);
  for(id = 0;id < gmm->peer_map->size()+1;++id) {
    if (id == gmm->myid) continue;

    peer = gmm->peer_map->get( (void*)(ull)id);
    
    peer->send(Data_shallow(v->codeword, v->lcodeword));
    peer->send(Data_shallow(v->mac[id]->mac, v->mac[id]->lmac));
    peer->receive(shares[id]);
    peer->receive(sharemac);

    if (!minimacs_check_othershare_fast(gmm->encoder, v, 
					id,
					shares[id]->data,
					sharemac->data,
					shares[id]->ldata)) {
      MRGF(oe,"Opening invalid mac on share, peer %u is cheating.",id);
    }

    for(i = 0; i < v->lval;++i) {
      clear->data[i] = add(clear->data[i],shares[id]->data[i]);
    }
  }

  for(i = 0;i<v->lval;++i) {
    clear->data[i] = add(clear->data[i],add(v->dx_codeword[i], v->codeword[i]));
  }

  result = minimacs_create_rep_public_plaintext_fast(gmm->encoder, clear->data, clear->ldata, v->lcodeword);


  if (!result) {
    MRGF(oe,"Failed with result being null. (%u)",dst);
  }

  this->heap_set(dst, result);

 failure:
  minimacs_rep_clean_up( &v );
  if (shares) {
    for(id = 0; id < gmm->peer_map->size()+1;++id) {
      Data_destroy(oe,&shares[id]);
    }
  }
  oe->putmem(shares);shares=0;
  Data_destroy(oe,&sharemac);
  Data_destroy(oe,&clear);
  return mr;
}

COO_DCL(MiniMacs, MR, init_heap, uint size)
COO_DEF_RET_ARGS(MiniMacs, MR, init_heap, uint size;,size) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  MR mr = 0;
  
  gmm->heap = oe->getmem(size*sizeof(MiniMacsRep));
  gmm->lheap = size;

  return mr;
 failure:
  return mr;
}

COO_DCL(MiniMacs, MR, heap_set, hptr addr, MiniMacsRep rep)
COO_DEF_RET_ARGS(MiniMacs, MR, heap_set ,hptr addr; MiniMacsRep rep;, addr, rep) {
  MR mr = 0;
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  
  if (addr < gmm->lheap) {
    if (rep == 0) {
      char m[64] = {0};
      osal_sprintf(m,"Setting address %u to null",addr);
      oe->syslog(OSAL_LOGLEVEL_FATAL, m);
    }
    gmm->heap[addr] = rep;
  } else MRGF(oe,"Address %u is out of range (0-%u). Invoke init_heap with a greater number ;).", gmm->lheap, (gmm->lheap==0?0:gmm->lheap-1));
  
  return mr;
 failure:
  return mr;
}

COO_DCL(MiniMacs, MiniMacsRep, heap_get, uint addr)
COO_DEF_RET_ARGS(MiniMacs,MiniMacsRep,heap_get,uint addr;,addr) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  
  if (addr < gmm->lheap) {
    return gmm->heap[addr];
  } 
  
 failure:
  return 0;
}


COO_DCL(MiniMacs, MR, invite, uint count, uint port)
COO_DEF_RET_ARGS(MiniMacs, MR, invite, uint count; uint port;, count, port) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  CArena arena = gmm->arena;
  uint id = 0;

  arena->listen(port);
  
  // TODO(rwl): Busy waiting for pairs is not a feasible solution in
  // the long run. Refactor this to use a connecting listener.
  while(arena->get_no_peers() < count) {
    usleep(500);
  }


  for(id = 0;id < arena->get_no_peers();++id) {
    MpcPeer peer = arena->get_peer(id);
    uint peer_id = 0;
    Data peer_id_d = Data_new(gmm->oe,4);
    Data my_id_raw = Data_new(gmm->oe,4);
    char m[64] = {0};

    i2b(gmm->myid, my_id_raw->data);
    peer->receive(peer_id_d);
    peer->send(my_id_raw);
    peer_id = b2i(peer_id_d->data);
    osal_sprintf(m,"registering peer with id %u.",peer_id);
    oe->p(m);
    if (gmm->peer_map->contains( ((void*)(ull)peer_id) )) {
      MRRF(oe,"Peer id %u already registered.",peer_id);
    }
    gmm->peer_map->put( ((void*)(ull)peer_id), peer);
  }
  MR_RET_OK;
}

typedef struct _waiting_connecting_listener_ {
  BitWiseMulPar2MiniMacs gmm;
  BlkQueue q;
  void (*wait_for)(uint n);
} *WaitingConnectionListener;
#define AS(SUB,PARENT) ((SUB)PARENT->impl) 

COO_DCL(ConnectionListener, void, client_connected, MpcPeer peer)
COO_DEF_NORET_ARGS(ConnectionListener, client_connected, MpcPeer peer;,peer) {
  WaitingConnectionListener wcl = AS(WaitingConnectionListener,this);
  BitWiseMulPar2MiniMacs gmm = wcl->gmm;
  OE oe = this->oe;
  Map map = 0;
  uint i = 0;
  Data raw_id = 0;
  Data my_id_raw = Data_new(oe,4);
  uint other_id = 0;
  oe->p("Registering new peer []");
  map = gmm->peer_map;
  raw_id = Data_new(oe, 4);
  if (!raw_id) return;
  i2b(gmm->myid, my_id_raw->data);

  oe->p("Sending my id");
  peer->send(my_id_raw);
  oe->p("Receiving my id");
  peer->receive(raw_id);
  
  other_id = b2i(raw_id->data);

  if (map->contains((void*)(ull)other_id) == True) {
    oe->p("Peer already registered :(.");
    return;
  }
  oe->p("Registering peer");
  map->put( (void*)(ull)other_id, peer );
  {
    char m[64] = {0};
    osal_sprintf(m,"Added peer %u to mapping.", other_id);
    oe->p(m);
  }
  wcl->q->put(0);
  oe->p("New peer registered.");
  
}

COO_DCL(ConnectionListener, void, client_disconnected, MpcPeer peer)
COO_DEF_NORET_ARGS(ConnectionListener, client_disconnected, MpcPeer peer;,peer) {
  return;
}

COO_DCL(WaitingConnectionListener, void, wait_for, uint count)
COO_DEF_NORET_ARGS(WaitingConnectionListener, wait_for, uint count;,count) {
  while(count--) {
    this->q->get();
  }
}



ConnectionListener WaitingConnectionListener_new(OE oe, BitWiseMulPar2MiniMacs gmm) {
  WaitingConnectionListener wcl = (WaitingConnectionListener)oe->getmem(sizeof(*wcl));
  if (!wcl) return 0;

  ConnectionListener cl = (ConnectionListener)oe->getmem(sizeof(*cl));
  if (!cl) goto failure;

  wcl->q = BlkQueue_new(oe,5);
  wcl->gmm = gmm;
  COO_ATTACH(WaitingConnectionListener, wcl, wait_for);
  cl->impl = wcl;
  cl->oe = oe;
  

  COO_ATTACH(ConnectionListener,cl, client_connected );
  COO_ATTACH(ConnectionListener,cl, client_disconnected);

  return cl;
 failure:
  return 0;
}



COO_DCL(MiniMacs, MR, connect,  char * ip, uint port) 
COO_DEF_RET_ARGS(MiniMacs, MR, connect,  char * ip; uint port;, ip, port) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  CArena arena = gmm->arena;
  CAR car = {{0}};
  ConnectionListener cl = WaitingConnectionListener_new(oe,gmm);
  WaitingConnectionListener wcl = AS(WaitingConnectionListener,cl);
  MpcPeer * map = 0;
  arena->add_conn_listener(cl);
  car = arena->connect(ip,port);
  if (car.rc != 0) MR_RET_FAIL(oe,car.msg);
  oe->p("Removing conn listener again");
  arena->rem_conn_listener(cl);
  MR_RET_OK;
}



COO_DCL(MiniMacs, uint, get_no_peers)
COO_DEF_RET_NOARGS(MiniMacs, uint, get_no_peers) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  return gmm->peer_map->size();
}


COO_DCL(MiniMacs, uint, get_ltext)
COO_DEF_RET_NOARGS(MiniMacs, uint, get_ltext) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  if (gmm->lsingles > 0) {
    return gmm->ltext;
  }
  return 0;
}


COO_DCL(MiniMacs, uint, get_lcode)
COO_DEF_RET_NOARGS(MiniMacs, uint, get_lcode) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  if (gmm->lsingles > 0) {
    return gmm->lcode;
  }
  return 0;
}



COO_DCL(MiniMacs, uint, get_no_players)
COO_DEF_RET_NOARGS(MiniMacs, uint, get_no_players) {
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  if (gmm->lsingles > 0) {
    return gmm->singles[0]->lmac;
  }
  return 0;
}



static uint uint_hfn(void * n) {
  uint un = (uint)(ull)n;
  return 101 * un + 1009;
}

static int uint_cfn(void *a, void *b) {
  uint an = (uint)(ull)a;
  uint bn = (uint)(ull)b;
  return an == bn ? 0 : (an > bn) ? 1 : -1;
}



static 
MiniMacsRep * build_constants(OE oe,
                              MiniMacsEnc enc, 
                              uint nplayers,
                              uint ltext,
                              uint lcode) {

  uint i=0,j=0;
  byte * val = 0;
  MiniMacsRep * result = 
    (MiniMacsRep*)oe->getmem(sizeof(*result)*8);
  val=oe->getmem(ltext);

  for(i = 0;i < 8;++i) {
    
    for(j = 0;j < ltext;++j) {
      val[j] = (0x1 << i);
    }
    result[i] = minimacs_create_rep_public_plaintext_fast(enc,
                                                          val,
                                                          ltext,
                                                          lcode);
    if (!result[i]) {
      oe->syslog(OSAL_LOGLEVEL_FATAL, "Could not initalize constants");
      return 0;
    }
  }
  
  return result;
}

typedef struct _mul_par_buffer_entry_ {
  Data delta;
  Data epsilon;
} mpbe;

COO_DCL(MiniMacs, MR, mulpar, uint count);
COO_DEF_RET_ARGS(MiniMacs, MR, mulpar, uint count;, count) {
  
  BitWiseMulPar2MiniMacs gmm = (BitWiseMulPar2MiniMacs)this->impl;
  OE oe = gmm->oe;
  
  gmm->mulpar = count;
}

MiniMacs BitWiseMulPar2MiniMacs_New(OE oe, CArena arena, MiniMacsEnc encoder,
                                    MiniMacsRep * singles, uint lsingles, 
                                    MiniMacsRep ** pairs, uint lpairs, 
                                    BitDecomposedTriple * triples, uint ltriples, bool do_bit_enc) {
  
  MiniMacs res = oe->getmem(sizeof(*res));
  if (!res) return 0;

  BitWiseMulPar2MiniMacs gres = (BitWiseMulPar2MiniMacs)oe->getmem(sizeof(*gres));
  if (!gres) goto failure;

  
  gres->oe = oe;
  gres->arena=arena;
  gres->singles = singles;
  gres->lsingles =lsingles;
  gres->triples = triples;
  gres->ltriples = ltriples;
  gres->pairs = pairs;
  gres->lpairs = lpairs;
  gres->lheap = 0;
  gres->heap = 0;
  gres->idx_triple = gres->idx_pair = gres->idx_single = 0;
  gres->peer_map = HashMap_new(oe, uint_hfn, uint_cfn, 3);
  gres->mulpar = 1;
  gres->mulpar_entries = (List)SingleLinkedList_new(oe);
  


  if (lsingles) {
    uint nplayers = singles[0]->lmac+1;
    gres->myid = minimacs_rep_whoami(singles[0]);
    gres->encoder = encoder;

    gres->lcode = singles[0]->lcodeword;
    gres->ltext = singles[0]->lval;
    if (do_bit_enc) {
      if (gres->ltext == 85) {
        gres->bitenc = MiniMacsEnc_BitFFTNew(oe,gres->lcode,gres->ltext);
      } else {
        gres->bitenc = MiniMacsEnc_BitNew(oe,gres->lcode,gres->ltext);
      }
    }

    gres->constants = build_constants(oe,encoder,nplayers,
                                      gres->ltext,gres->lcode);
  } else {
    oe->syslog(OSAL_LOGLEVEL_WARN, "No singles; this instance of BitWiseMulPar2MiniMacs" 
	       " will not have encoders nor a determined id.");
  }

  

  COO_ATTACH(BitWiseMulPar2MiniMacs, gres, next_triple);
  COO_ATTACH(BitWiseMulPar2MiniMacs, gres, next_single);
  COO_ATTACH(BitWiseMulPar2MiniMacs, gres, next_pair);
  COO_ATTACH(BitWiseMulPar2MiniMacs, gres, __add__);

  res->impl = gres;
  
  COO_ATTACH(MiniMacs, res, add);
  COO_ATTACH(MiniMacs, res, mul);
  COO_ATTACH(MiniMacs, res, secret_input);
  COO_ATTACH(MiniMacs, res, public_input);
  COO_ATTACH(MiniMacs, res, open);
  COO_ATTACH(MiniMacs, res, heap_get);
  COO_ATTACH(MiniMacs, res, heap_set);
  COO_ATTACH(MiniMacs, res, init_heap);
  COO_ATTACH(MiniMacs, res, invite);
  COO_ATTACH(MiniMacs, res, connect);
  COO_ATTACH(MiniMacs, res, get_no_peers);
  COO_ATTACH(MiniMacs, res, get_id);
  COO_ATTACH(MiniMacs, res, get_ltext);
  COO_ATTACH(MiniMacs, res, get_lcode);
  COO_ATTACH(MiniMacs, res, get_no_players);
  COO_ATTACH(MiniMacs, res, mulpar);

  oe->p("------------------------------------------------------------");
  oe->p("      MiniTrix: Assuming Bit Wise AND pre-processing");
  oe->p("   " PACKAGE_STRING " - " CODENAME " - " BUILD_TIME);
	oe->p("------------------------------------------------------------");

  return res;
 failure:
  BitWiseMulPar2MiniMacs_destroy( &res );
  return 0;
}

MiniMacs BitWiseMulPar2MiniMacs_DefaultNew(OE oe, CArena arena, 
                                    MiniMacsRep * singles, uint lsingles,
                                    MiniMacsRep ** pairs, uint lpairs,
                                           BitDecomposedTriple * triples, uint ltriples, bool do_bit_enc) {
  uint ltext = 0, lcode = 0;
  
  if (lsingles > 0) {
    ltext = singles[0]->lval;
    lcode = singles[0]->lcodeword;
  } else {
    oe->syslog(OSAL_LOGLEVEL_WARN, "Cannot create BitWiseMulPar2MiniMacs default instance without any singles :(");
    return 0;
  }

  MiniMacsEnc matrix_encoder = MiniMacsEnc_MatrixNew(oe, lcode, ltext );
  return BitWiseMulPar2MiniMacs_New(oe, arena, matrix_encoder,
                                    singles, lsingles,
                                    pairs, lpairs, 
                                    triples, ltriples, do_bit_enc );
}


MiniMacs BitWiseMulPar2MiniMacs_DefaultFFTNew(OE oe, CArena arena, 
                                    MiniMacsRep * singles, uint lsingles,
                                    MiniMacsRep ** pairs, uint lpairs,
                                              BitDecomposedTriple * triples, uint ltriples, bool do_bit_enc) {
  uint ltext = 0, lcode = 0;
  
  if (lsingles > 0) {
    ltext = singles[0]->lval;
    lcode = singles[0]->lcodeword;
  } else {
    oe->syslog(OSAL_LOGLEVEL_WARN, "Cannot create BitWiseMulPar2MiniMacs default instance without any singles :(");
    return 0;
  }

  if (ltext != 85 || lcode != 255) {
    oe->syslog(OSAL_LOGLEVEL_FATAL, "Error: Fast Fourier Transform only works with ltext=85 and lcode=255");
    return 0;
  }

  MiniMacsEnc fft_encoder = MiniMacsEnc_FFTNew(oe);
  return BitWiseMulPar2MiniMacs_New(oe, arena, fft_encoder,
                                    singles, lsingles,
                                    pairs, lpairs, 
                                    triples, ltriples, do_bit_enc );
}


MiniMacs BitWiseMulPar2MiniMacs_DefaultLoadNew(OE oe, const char * filename, const char * bdt_filename, bool do_bit_enc) {
  
  MiniMacsRep * singles = 0;
  MiniMacsRep ** pairs  = 0;
  MiniMacsTripleRep * triples = 0;
  BitDecomposedTriple * btriples = 0;
  uint ltriples=0, lsingles=0,lpairs=0,lbtriples;
  CArena arena = CArena_new(oe);
  if (!arena) return 0;


  load_shares(filename,
              &triples, &ltriples, 
              &singles, &lsingles,
              &pairs, &lpairs );
  if (!singles || !triples || !pairs) {
    oe->syslog(OSAL_LOGLEVEL_WARN, "BitWiseDecomposed instances requires at least one of each, singles, triples and pairs in the preprocessing.\n");
    return 0;
  }

  load_bdt(bdt_filename, &btriples, &lbtriples);
  if (!btriples) {
    oe->syslog(OSAL_LOGLEVEL_WARN,"BitWiseDecomposed instance "\
               "requires special triples which could not be loaded.");
    return 0;
  }

  return BitWiseMulPar2MiniMacs_DefaultNew(oe, arena,singles, lsingles, pairs,lpairs,  btriples, lbtriples, do_bit_enc);
  
}


MiniMacs BitWiseMulPar2MiniMacs_DefaultLoadFFTNew(OE oe, const char * filename, const char * bdt_filename, bool do_bit_enc) {
  MiniMacsRep * singles = 0;
  MiniMacsRep ** pairs  = 0;
  MiniMacsTripleRep * triples = 0;
  BitDecomposedTriple * btriples = 0;
  uint ltriples=0, lsingles=0,lpairs=0,lbtriples;
  CArena arena = CArena_new(oe);
  if (!arena) return 0;


  load_shares(filename,
              &triples, &ltriples, 
              &singles, &lsingles,
              &pairs, &lpairs );
  if (!singles ) {
    oe->syslog(OSAL_LOGLEVEL_FATAL, "BitWiseDecomposed instances requires at least one of each, singles, triples and pairs in the preprocessing.\n");
    return 0;
  }

  load_bdt(bdt_filename, &btriples, &lbtriples);
  if (!btriples) {
    oe->syslog(OSAL_LOGLEVEL_WARN,"BitWiseDecomposed instance "\
               "requires special triples which could not be loaded.");
    return 0;
  }

  return BitWiseMulPar2MiniMacs_DefaultFFTNew(oe, arena,singles, lsingles, pairs,lpairs,  btriples, lbtriples, do_bit_enc );
  
}


void BitWiseMulPar2MiniMacs_destroy( MiniMacs * instance) {

  BitWiseMulPar2MiniMacs gmm = 0;
  if (!instance) return;
  if (!*instance) return;

  gmm =   (BitWiseMulPar2MiniMacs)(*instance)->impl;

  CArena_destroy( &gmm->arena );


  return;
}

