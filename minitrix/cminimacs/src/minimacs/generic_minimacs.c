#include "minimacs/generic_minimacs.h"
#include <coov4.h>
#include <common.h>
#include <math/matrix.h>
#include <blockingqueue.h>
#include <config.h>
#include "stats.h"
#include <encoding/int.h>


COO_DEF(MiniMacs, uint, get_id)
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  
  if (gmm->singles) {
    return minimacs_rep_whoami( gmm->singles[0] );
  } else {
    oe->p("I have no singles and thus no id :(");
  }

  return 0;
}

COO_DEF(GenericMiniMacs, MiniMacsTripleRep, next_triple)
  if (this->idx_triple < this->ltriples) {
    return this->triples[this->idx_triple++];
  }
  return 0;
}

COO_DEF(GenericMiniMacs, MiniMacsRep, next_single)
  if (this->idx_single < this->lsingles) {
    return this->singles[this->idx_single++];
  }
  return 0;
}

COO_DEF(GenericMiniMacs, MiniMacsRep *, next_pair) 
  if (this->idx_pair < this->lpairs) {
    return this->pairs[this->idx_pair++];
  }
  return 0;
}

COO_DEF(MiniMacs,MR,add,uint dst, uint l, uint r)
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  MiniMacsRep left = this->heap_get(l);
  MiniMacsRep right = this->heap_get(r);
  MiniMacsRep res = 0;
  MR mr = 0;
  
  if (gmm->peer_map->size() < 1) MRRF(oe,"[add] No peers connected.");

  if (!left) MRRF(oe,"Left operand (%d) was not set.",l);
  if (!right) MRRF(oe, "Right operand (%d) was not set.", r);

  mr = gmm->__add__(&res,left,right);
  if (res == 0) MRRF(oe, "[add] Failed with null.");
  if (mr != 0) return mr;
  this->heap_set(dst,res);
  return mr;
}

COO_DEF(GenericMiniMacs, MR, __add__, MiniMacsRep * res_out,
	MiniMacsRep left, MiniMacsRep right)
  OE oe = this->oe;
  bool left_const = minimacs_rep_is_public_const(left);
  bool right_const = minimacs_rep_is_public_const(right);
  MiniMacsRep result = 0;

  if (!res_out) {
    MRRF(oe,"Programing error no pointer to store result");
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
    result = minimacs_rep_add_const_fast(oe, enc, right, left->codeword, left->lval);
  }

  // left is a public constant and right is a proper rep
  if (!left_const && right_const) {
    MiniMacsEnc enc = this->encoder;
    result = minimacs_rep_add_const_fast(oe,enc, left, right->codeword, right->lval);
  }


  // The case where we have two public constants
  if (left_const && right_const) {
    uint i = 0;
    byte * ntxt = (byte*)oe->getmem(left->lval);
    MiniMacsEnc enc = this->encoder;
    for(i = 0;i < left->lval;++i) {
      ntxt[i] = left->codeword[i] ^ right->codeword[i];
    }
    result = minimacs_create_rep_public_plaintext_fast(oe, enc, ntxt, left->lval, left->lcodeword);
  }

  // The non trivial case with to proper reps
  if (!left_const && !right_const) {
    result = minimacs_rep_xor(oe,left, right);
  }

  if (!result) {
    MRRF(oe,"Result is null add failed. left_const=%u,right_const=%u\n",left_const, right_const);
  }

  *res_out = result;
  MR_RET_OK;
}





COO_DEF(MiniMacs,MR,mul,uint dst, uint l, uint r) 
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  MiniMacsRep left = this->heap_get(l);
  MiniMacsRep right = this->heap_get(r);
  bool left_const = minimacs_rep_is_public_const(left);
  bool right_const = minimacs_rep_is_public_const(right);
  MiniMacsRep result = 0;
  
  MR mr = 0;
  MiniMacsRep * star_pair = 0;
  MiniMacsTripleRep triple = 0;
  MiniMacsRep delta = 0;
  MiniMacsRep epsilon = 0;
  Data d = 0;
  MpcPeer peer = 0;
  Data delta_share = 0;
  Data delta_clear = 0;
  Data epsilon_share = 0;
  Data epsilon_clear = 0;
  uint id = 0, i=0;
  bool epsilon_ok = 0;
  bool delta_ok = 0;
  Data epsilonmac = 0;
  Data deltamac = 0;
  MiniMacsRep delta_b = 0;
  MiniMacsRep epsilon_a = 0;
  Data epsilon_times_delta = 0;
  MiniMacsRep tmp = 0, tmp2=0;
  MiniMacsRep res_star = 0;
  MiniMacsRep sigma_star = 0;
  Data sigma_star_plain=0;
  byte * sigma = 0;

  if (!left) MUL_FAIL(oe,"Left operand (%d) is not set.", l);

  if (!right) MUL_FAIL(oe,"Right operand (%d) is not set.", r);

  // Both public constants
  if (left_const && right_const) {
    uint j = 0;
    Data tmpd = Data_new(gmm->oe,left->lval);
    for(j = 0;j < left->lval;++j) {
      tmpd->data[j] = multiply(left->codeword[j],right->codeword[j]);
    }
    result = minimacs_create_rep_public_plaintext_fast(oe,gmm->encoder,tmpd->data, tmpd->ldata, left->lcodeword);
    if (!result) MUL_FAIL(oe,"%u Result gave null, some condition is wrong.",__LINE__);
    this->heap_set(dst, result);
    Data_destroy(gmm->oe,&tmpd);
    MR_RET_OK;
  }


  star_pair = gmm->next_pair();
  if (!star_pair) MUL_FAIL(oe,"No more pairs (%d taken).",gmm->idx_pair);

  // right constant 
  if (right_const) {
    MiniMacsEnc enc = gmm->encoder;
    res_star = minimacs_rep_mul_const_fast(oe, enc, left, right->codeword, right->lval); 
    if (!res_star) MUL_FAIL(oe,"%u Result gave null, some condition is wrong.",__LINE__);
    
    sigma_star = minimacs_rep_xor(oe,res_star, star_pair[1]);
    if (!sigma_star) MRGF(oe,"Failed to compute sigma*.");
    
  }

  // left constant
  if (left_const) {
    MiniMacsEnc enc = gmm->encoder;
    res_star = minimacs_rep_mul_const_fast(oe, enc,right,left->codeword, left->lval);
    if (!res_star) MUL_FAIL(oe,"%u Result gave null, some condition is wrong.",__LINE__);

    sigma_star = minimacs_rep_xor(oe, res_star, star_pair[1]);
    if (!sigma_star) MRGF(oe,"Failed to compute sigma*.");
  }

  // both real representations
  if (!right_const && !left_const) {
    oe->p("mul");
    CHECK_POINT_S("  BOTH  ");
    triple = gmm->next_triple();
    if (!triple) MUL_FAIL(oe,"No more triples (%d taken).", gmm->idx_triple);
    
    mr = gmm->__add__(&delta,triple->a, left);
    if (mr != 0) MUL_FAIL(oe,"Could not add triple->a and left");
    
    mr = gmm->__add__(&epsilon,triple->b, right);
    if (mr != 0) MUL_FAIL(oe,"Could not add triple->b and right");
    
    if (epsilon->lval != delta->lval) MUL_FAIL(oe,"Inconsistent epsilon and delta");
    
    // broadcast delta and epsilon
    for(id = 0;id < gmm->peer_map->size()+1;++id) {
      if (id == gmm->myid) continue; 
      
      peer = gmm->peer_map->get( (void*)(unsigned long long)id );
      if (!peer) MUL_FAIL(oe,"Peer with id, %u, is undefined",id);
      
      peer->send(Data_shallow(delta->codeword, delta->lcodeword));
      peer->send(Data_shallow(delta->mac[id]->mac, delta->mac[id]->lmac));
      peer->send(Data_shallow(epsilon->codeword, epsilon->lcodeword));
      peer->send(Data_shallow(epsilon->mac[id]->mac, epsilon->mac[id]->lmac));
    }
    
    deltamac = Data_new(oe, delta->lcodeword);
    delta_share = Data_new(oe, delta->lcodeword);
    delta_clear = Data_new(oe, delta->lcodeword);    
    epsilonmac = Data_new(oe,epsilon->lcodeword);
    epsilon_share = Data_new(oe, epsilon->lcodeword);
    epsilon_clear = Data_new(oe, epsilon->lcodeword);

    // receive and check epsilon and delta from every one else.
    for(id = 0;id<gmm->peer_map->size()+1;++id) {
      if (id == gmm->myid) continue;
      
      peer = gmm->peer_map->get( (void*)(ull)id);
      if (!peer) MUL_FAIL(oe,"Peer with id, %u, is undefined (after sending to him)", id);
      
      peer->receive(delta_share);
      if (delta_share->ldata != delta->lcodeword) 
        MRGF(oe,"Received delta with invalid length (%u) from %u",
             delta_share->ldata, id);
      
      peer->receive(deltamac);
      if (deltamac->ldata < delta->lcodeword) 
        MRGF(oe,"Received delta mac with invalid length (%u) from %u",
             deltamac->ldata, id);
      
      peer->receive(epsilon_share);
      if (epsilon_share->ldata != epsilon->lcodeword) 
        MRGF(oe,"Received epsilon with invalid length (%u) from %u",
             epsilon_share->ldata, id);
      
      peer->receive(epsilonmac);
      if (epsilonmac->ldata != epsilon->lcodeword) 
        MRGF(oe,"Received epsilon with invalid length (%u) from %u",
             epsilonmac->ldata, id);
      
      if (!
          minimacs_check_othershare_fast(gmm->encoder,delta,
                                         id,
                                         delta_share->data,
                                         deltamac->data,
                                         delta_share->ldata) ) {
        MRGF(oe,"Peer %u is cheating, mac didn't check out on delta.",id);
      }
      
      if (!minimacs_check_othershare_fast(gmm->encoder,epsilon,
                                          id, 
                                          epsilon_share->data,
                                          epsilonmac->data,
                                          epsilon_share->ldata)) {
        MRGF(oe,"Peer %u is cheating, mac didn't check out on epsilon.",id);
      }

      for(i = 0;i < delta->lcodeword;++i) {
        delta_clear->data[i] = add(delta_clear->data[i],
                                   delta_share->data[i]);
      }

      for(i = 0;i < epsilon->lcodeword;++i) {
        epsilon_clear->data[i] = add(epsilon_clear->data[i],
                                     epsilon_share->data[i]);
      }


    } // done receiving from all players


    Data_destroy(oe,&deltamac);
    Data_destroy(oe,&epsilonmac);
    
    //compute clear text delta
    for(i = 0;i<delta->lcodeword;++i) {
      delta_clear->data[i] = add(delta->dx_codeword[i],
                                 add(delta->codeword[i], delta_clear->data[i]));
    }
    minimacs_rep_clean_up( oe, & delta );

    // compute clear text epsilon
    for(i = 0;i < epsilon->lcodeword;++i) {
      epsilon_clear->data[i] = add(epsilon_clear->data[i],
                                   add(epsilon->dx_codeword[i],epsilon->codeword[i]));
    }
    minimacs_rep_clean_up(oe, &epsilon );

    /*
    // to be removed
    if (gmm->encoder->validate(delta_clear->data, star_pair[0]->lval) != True) {
      printf("ERROR: delta_clear is not a codeword.\n");
      goto failure;
    }

    if (gmm->encoder->validate(epsilon_clear->data, star_pair[0]->lval) != True) {
      printf("ERROR: delta_clear is not a codeword.\n");
      goto failure;
    }
    */
    
    delta_b = minimacs_rep_mul_const_fast(oe, gmm->encoder, 
                                          triple->b, 
                                          delta_clear->data, delta_clear->ldata);
    if (!delta_b) MRGF(oe,"Failed to compute the product of delta clear and triple->b");
    
    epsilon_a = minimacs_rep_mul_const_fast(oe,gmm->encoder, 
                                            triple->a, 
                                            epsilon_clear->data, epsilon_clear->ldata);
    if (!epsilon_a) MRGF(oe,"Failed to compute the product of epsilon clear and triple->a");
    
    
    epsilon_times_delta = Data_new(gmm->oe, delta_clear->ldata);
    if (!epsilon_times_delta) MRGF(oe,"Ran out of memory computing epsilon times delta");
    
    for(i = 0;i < delta_clear->ldata;++i) {
      epsilon_times_delta->data[i] = multiply(delta_clear->data[i],
                                              epsilon_clear->data[i]);
    }

    /*
    if (gmm->encoder->validate(epsilon_times_delta->data, 240) != True) {
      printf("ERROR: epsilon times delta does not give a valid codeword (%u).\n",
             delta_b->lval);
      goto failure;
    }
    */
    
    tmp = minimacs_rep_xor(oe,delta_b, epsilon_a);
    if (!tmp) MRGF(oe,"Failed to add delta_b and epsilon_a");
    
    minimacs_rep_clean_up(oe, &delta_b);
    
    tmp2 = minimacs_rep_add_const_fast(oe,gmm->encoder, tmp, epsilon_times_delta->data,
                                       epsilon_times_delta->ldata);
    if (!tmp2) MRGF(oe,"Failed to add epsilon*delta with (b*delta + epsolon*a).");
    
    minimacs_rep_clean_up(oe, &epsilon_a);
    minimacs_rep_clean_up(oe, &tmp);
    
    res_star = minimacs_rep_xor(oe,tmp2, triple->cstar);
    if (!res_star) MRGF(oe,"Failed to compute result shur transform.");
    minimacs_rep_clean_up(oe, &tmp2);
    
    sigma_star = minimacs_rep_xor(oe, res_star, star_pair[1]);
    if (!sigma_star) MRGF(oe,"Failed to compute sigma*.");

    CHECK_POINT_E("  BOTH  ");
  }
    res_star->lval = star_pair[1]->lval;

  // Now sigma star is opened privately to player one who checks it is
  // a codeword in the shur transform. Then player one broadcasts
  // sigma using the first entry of the star_pair.
  //
  // All other players participate in opening to player one and
  // receives sigma star. 
  //
  // every peer forms the representation of left*right.
  //
  if (gmm->myid == 0) {
    Data * sigma_star_in = oe->getmem(sizeof(*sigma_star_in)*(gmm->peer_map->size()+1));    
    Data sigmamac = Data_new(oe, sigma_star->lcodeword);
    uint lsigme_star_in = gmm->peer_map->size()+1;
    CHECK_POINT_S(" Peer0] all alone");
    for(id = 0; id < gmm->peer_map->size()+1;++id) {
      sigma_star_in[id] = Data_new(oe, sigma_star->lcodeword);
    }
    for(id = 0; id < gmm->peer_map->size()+1;++id) {
      if (id == gmm->myid) continue;

      peer = gmm->peer_map->get( (void*)(ull)id );
      if (!peer) MRGF(oe,"Peer %u is not defined, did it disconnect?");
      
      peer->receive(sigma_star_in[id]);
      peer->receive(sigmamac);

      if (!minimacs_check_othershare_fast(gmm->encoder,
					  sigma_star,
					  id,
					  sigma_star_in[id]->data,
					  sigmamac->data,
					  sigma_star_in[id]->ldata) ) {
        MRGF(oe,"Peer %u is cheating",id);
      }
    }

    sigma_star_plain = Data_new(oe,sigma_star->lcodeword);
    if (!sigma_star_plain) MRGF(oe,"Ran out of memory while opening sigma star");
    
    for(id = 0;id < gmm->peer_map->size()+1;++id) {
      for(i = 0; i < sigma_star->lcodeword;++i) {
        sigma_star_plain->data[i] = add(sigma_star_plain->data[i],sigma_star_in[id]->data[i]);
      }
    }
    for(i = 0;i<sigma_star->lcodeword;++i) {
      sigma_star_plain->data[i] = 
        add(sigma_star_plain->data[i],add(sigma_star->dx_codeword[i],sigma_star->codeword[i]));
    }

    MEASURE_FN(sigma = gmm->encoder->encode(sigma_star_plain->data, star_pair[0]->lval));

    if (!sigma) MRGF(oe,"Failed to create sigma representation.");

    for(id = 0;id<gmm->peer_map->size()+1;++id) {
      if (id == gmm->myid) continue;

      peer = (MpcPeer)gmm->peer_map->get( (void*)(ull)id);
      if (!peer) MRGF(oe,"Peer %u disconnected.");

      peer->send(Data_shallow(sigma,sigma_star->lcodeword));
    }  

    CHECK_POINT_E(" Peer0] all alone");
    result = minimacs_rep_add_const_fast(oe,gmm->encoder,star_pair[0],sigma,left->lval);
    if (!result) MRGF(oe,"Computing result failed");
    
    
    this->heap_set(dst, result);
    MR_RET_OK;
  } else {
    Data sigma_plain = 0;
    ull start = 0;
    CHECK_POINT_S(" Peer1] alone ");
    peer = gmm->peer_map->get(0);
    if (!peer) MRGF(oe,"Party one disconnected.");
    
    peer->send(Data_shallow(sigma_star->codeword, sigma_star->lcodeword));
    peer->send(Data_shallow(sigma_star->mac[0]->mac, sigma_star->mac[0]->lmac));

    sigma_plain = Data_new(oe, sigma_star->lcodeword);
    
    peer->receive(sigma_plain);

    CHECK_POINT_E(" Peer1] alone ");
    
    if (!gmm->encoder->validate(sigma_plain->data, left->lval)) 
      MRGF(oe,"Invalid sigma player one is cheating.");
    
    result = minimacs_rep_add_const_fast(oe,gmm->encoder, star_pair[0], sigma_plain->data, sigma_plain->ldata);
    if (!result) MRGF(oe,"Failed to compute result.");
    
    this->heap_set(dst ,result);
    
    MR_RET_OK;
  }


  MR_RET_OK;
 failure:
  return mr;
}



COO_DEF(MiniMacs,MR,secret_input, uint pid, hptr dst, Data plain_val) {
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
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

    result = minimacs_rep_add_const_fast(oe,gmm->encoder,r,epsilon->data, r->lval);
    minimacs_rep_clean_up(oe, &r);    
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

    result = minimacs_rep_add_const_fast(oe,gmm->encoder,r,epsilon->data, r->lval);
    if (!result) MRGF(oe,"Failed to perform input, add constant to form result failed.");
    Data_destroy(oe,&epsilon);
    minimacs_rep_clean_up(oe, &r);
    this->heap_set(dst, result);
    MR_RET_OK;
  }
 failure:
  minimacs_rep_clean_up( oe, &r);
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
  minimacs_rep_clean_up(oe, &result);

  return mr;
}}

COO_DEF(MiniMacs,MR,public_input, hptr dst, Data pub_val)
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  MiniMacsRep vrep = minimacs_create_rep_public_plaintext_fast(oe,gmm->encoder, pub_val->data, pub_val->ldata, gmm->singles[0]->lcodeword);
  this->heap_set(dst, vrep);
  MR_RET_OK;
 failure:
  MR_RET_OK;
}

COO_DEF(MiniMacs,MR,open, hptr dst)
  uint i = 0;
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  Data * shares = 0;
  OE oe = gmm->oe;
  MiniMacsRep v = this->heap_get(dst);
  MR mr = 0;
  uint id = 0;
  MpcPeer peer = 0;
  Data clear = 0;
  Data sharemac = 0;
  MiniMacsRep result = 0;

  if (gmm->peer_map->size() < 1) MRRF(oe,"No peers connected.");

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

  result = minimacs_create_rep_public_plaintext_fast(oe, gmm->encoder, clear->data, clear->ldata, v->lcodeword);
  this->heap_set(dst, result);

 failure:
  minimacs_rep_clean_up( oe, &v );
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

COO_DEF(MiniMacs, MR, init_heap, uint size)
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  MR mr = 0;
  
  gmm->heap = oe->getmem(size*sizeof(MiniMacsRep));
  gmm->lheap = size;

  return mr;
 failure:
  return mr;
}

COO_DEF(MiniMacs, MR, heap_set, hptr addr, MiniMacsRep rep)
  MR mr = 0;
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  
  if (addr < gmm->lheap) {
    if (rep == 0) {
      char m[64] = {0};
      osal_sprintf(m,"Setting address %u to null",addr);
      oe->p(m);
    }
    gmm->heap[addr] = rep;
  } else MRGF(oe,"Address %u is out of range (0-%u). Invoke init_heap with a greater number ;).", gmm->lheap, (gmm->lheap==0?0:gmm->lheap-1));
  
  return mr;
 failure:
  return mr;
}

COO_DEF(MiniMacs, MiniMacsRep, heap_get, uint addr)
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  
  if (addr < gmm->lheap) {
    return gmm->heap[addr];
  } 
  
 failure:
  return 0;
}



COO_DEF(MiniMacs, MR, invite, uint count, uint port)
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
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
  GenericMiniMacs gmm;
  BlkQueue q;
  void (*wait_for)(uint n);
} *WaitingConnectionListener;
#define AS(SUB,PARENT) ((SUB)PARENT->impl) 

COO_DEF(ConnectionListener, void, client_connected, MpcPeer peer)
  WaitingConnectionListener wcl = AS(WaitingConnectionListener,this);
  GenericMiniMacs gmm = wcl->gmm;
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

COO_DEF(ConnectionListener, void, client_disconnected, MpcPeer peer)
  return;
}

COO_DEF(WaitingConnectionListener, void, wait_for, uint count)
  while(count--) {
    this->q->get();
  }
}



ConnectionListener WaitingConnectionListener_new(OE oe, GenericMiniMacs gmm) {
  WaitingConnectionListener wcl = (WaitingConnectionListener)oe->getmem(sizeof(*wcl));
  if (!wcl) return 0;

  ConnectionListener cl = (ConnectionListener)oe->getmem(sizeof(*cl));
  if (!cl) goto failure;

  wcl->q = BlkQueue_new(oe,5);
  wcl->gmm = gmm;
  wcl->wait_for = COO_attach( wcl, WaitingConnectionListener_wait_for);
  cl->impl = wcl;
  cl->oe = oe;
  

  cl->client_connected  = COO_attach(cl, ConnectionListener_client_connected );
  cl->client_disconnected = COO_attach(cl, ConnectionListener_client_disconnected);

  return cl;
 failure:
  return 0;
}



COO_DEF(MiniMacs, MR, connect,  char * ip, uint port) 
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  CArena arena = gmm->arena;
  CAR car = {{0}};
  ConnectionListener cl = WaitingConnectionListener_new(oe,gmm);
  WaitingConnectionListener wcl = AS(WaitingConnectionListener,cl);
  MpcPeer * map = 0;
  arena->add_conn_listener(cl);
  car = arena->connect(ip,port);
  if (car.rc != 0) MR_RET_FAIL(oe,car.msg);
  arena->rem_conn_listener(cl);
  MR_RET_OK;
}



COO_DEF(MiniMacs, uint, get_no_peers)
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  return gmm->peer_map->size();
}


COO_DEF(MiniMacs, uint, get_ltext)
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  if (gmm->lsingles > 0) {
    return gmm->ltext;
  }
  return 0;
}


COO_DEF(MiniMacs, uint, get_lcode)
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  if (gmm->lsingles > 0) {
    return gmm->lcode;
  }
  return 0;
}



COO_DEF(MiniMacs, uint, get_no_players)
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  if (gmm->lsingles > 0) {
    return gmm->singles[0]->lmac;
  }
  return 0;
}



 uint uint_hfn(void * n) {
  uint un = (uint)(ull)n;
  return 101 * un + 1009;
}

 int uint_cfn(void *a, void *b) {
  uint an = (uint)(ull)a;
  uint bn = (uint)(ull)b;
  return an == bn ? 0 : (an > bn) ? 1 : -1;
}



MiniMacs GenericMiniMacs_New(OE oe, CArena arena, MiniMacsEnc encoder,
                             MiniMacsRep * singles, uint lsingles, 
                             MiniMacsRep ** pairs, uint lpairs, 
                             MiniMacsTripleRep * triples, uint ltriples) {
  
  MiniMacs res = oe->getmem(sizeof(*res));
  if (!res) return 0;

  GenericMiniMacs gres = (GenericMiniMacs)oe->getmem(sizeof(*gres));
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

  if (lsingles) {
    gres->myid = minimacs_rep_whoami(singles[0]);
    gres->encoder = encoder;
    gres->lcode = singles[0]->lcodeword;
    gres->ltext = singles[0]->lval;
  } else {
    oe->syslog(OSAL_LOGLEVEL_WARN, "No singles; this instance of GenericMiniMacs" 
	       " will not have encoders nor a determined id.");
  }

  

  gres->next_triple = COO_attach( gres, GenericMiniMacs_next_triple);
  gres->next_single = COO_attach( gres, GenericMiniMacs_next_single);
  gres->next_pair = COO_attach( gres, GenericMiniMacs_next_pair);
  gres->__add__ = COO_attach( gres, GenericMiniMacs___add__);

  res->impl = gres;
  
  res->add = COO_attach( res, MiniMacs_add);
  res->mul = COO_attach( res, MiniMacs_mul);
  res->secret_input = COO_attach( res, MiniMacs_secret_input);
  res->public_input = COO_attach( res, MiniMacs_public_input);
  res->open = COO_attach( res, MiniMacs_open);
  res->heap_get = COO_attach( res, MiniMacs_heap_get);
  res->heap_set = COO_attach( res, MiniMacs_heap_set);
  res->init_heap = COO_attach( res, MiniMacs_init_heap);
  res->invite = COO_attach( res, MiniMacs_invite);
  res->connect = COO_attach( res, MiniMacs_connect);
  res->get_no_peers = COO_attach( res, MiniMacs_get_no_peers);
  res->get_id = COO_attach( res, MiniMacs_get_id);
  res->get_ltext = COO_attach( res, MiniMacs_get_ltext);
  res->get_lcode = COO_attach( res, MiniMacs_get_lcode);
  res->get_no_players = COO_attach( res, MiniMacs_get_no_players);

  oe->p("------------------------------------------------------------");
  oe->p("   " PACKAGE_STRING " - " CODENAME);
	oe->p("------------------------------------------------------------");

  return res;
 failure:
  GenericMiniMacs_destroy( &res );
  return 0;
}

MiniMacs GenericMiniMacs_DefaultNew(OE oe, CArena arena, 
                                    MiniMacsRep * singles, uint lsingles,
                                    MiniMacsRep ** pairs, uint lpairs,
                                    MiniMacsTripleRep * triples, uint ltriples) {
  uint ltext = 0, lcode = 0;
  
  if (lsingles > 0) {
    ltext = singles[0]->lval;
    lcode = singles[0]->lcodeword;
  } else {
    oe->syslog(OSAL_LOGLEVEL_WARN, "Cannot create GenericMiniMacs default instance without any singles :(");
    return 0;
  }

  MiniMacsEnc matrix_encoder = MiniMacsEnc_MatrixNew(oe, lcode, ltext );
  return GenericMiniMacs_New(oe, arena, matrix_encoder,
                             singles, lsingles,
                             pairs, lpairs, 
                             triples, ltriples );
}




MiniMacs GenericMiniMacs_DefaultLoadNew(OE oe, const char * filename) {
  
  MiniMacsRep * singles = 0;
  MiniMacsRep ** pairs  = 0;
  MiniMacsTripleRep * triples = 0;
  uint ltriples=0, lsingles=0,lpairs=0;

  CArena arena = CArena_new(oe);
  if (!arena) return 0;

  load_shares(oe,filename,
              &triples, &ltriples, 
              &singles, &lsingles,
              &pairs, &lpairs );
  if (!singles || !triples || !pairs) {
    oe->syslog(OSAL_LOGLEVEL_WARN, "Default Generic instances requires at least one of each, singles, triples and pairs in the preprocessing.\n");
    return 0;
  }

  return GenericMiniMacs_DefaultNew(oe, arena,singles, lsingles, pairs,lpairs,  triples, ltriples );
  
}

void GenericMiniMacs_destroy( MiniMacs * instance) {

  GenericMiniMacs gmm = 0;
  if (!instance) return;
  if (!*instance) return;

  gmm =   (GenericMiniMacs)(*instance)->impl;

  CArena_destroy( &gmm->arena );

  return;
}


