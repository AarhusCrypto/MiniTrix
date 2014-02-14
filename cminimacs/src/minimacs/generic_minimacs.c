#include "minimacs/generic_minimacs.h"
#include <coo.h>
#include <common.h>
#include <math/matrix.h>
#include <blockingqueue.h>
#include <config.h>
#include "stats.h"

COO_DCL(MiniMacs, uint, get_id);
COO_DEF_RET_NOARGS(MiniMacs, uint, get_id) {
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  
  if (gmm->singles) {
    return minimacs_rep_whoami( gmm->singles[0] );
  } else {
    oe->p("I have no singles and thus no id :(");
  }

  return 0;
}}

COO_DCL(GenericMiniMacs, MiniMacsTripleRep, next_triple)
COO_DEF_RET_NOARGS(GenericMiniMacs, MiniMacsTripleRep, next_triple) {
  if (this->idx_triple < this->ltriples) {
    return this->triples[this->idx_triple++];
  }
  return 0;
}}

COO_DCL(GenericMiniMacs, MiniMacsRep, next_single)
COO_DEF_RET_NOARGS(GenericMiniMacs, MiniMacsRep, next_single) {
  if (this->idx_single < this->lsingles) {
    return this->singles[this->idx_single++];
  }
  return 0;
}}

COO_DCL(GenericMiniMacs, MiniMacsRep *, next_pair) 
COO_DEF_RET_NOARGS(GenericMiniMacs, MiniMacsRep *, next_pair) {
  if (this->idx_pair < this->lpairs) {
    return this->pairs[this->idx_pair++];
  }
  return 0;
}}

COO_DCL(MiniMacs,MR,add,uint dst, uint l, uint r)
COO_DEF_RET_ARGS(MiniMacs, MR, add, uint dst; uint l; uint r;,dst,l,r) {
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  MiniMacsRep left = this->heap_get(l);
  MiniMacsRep right = this->heap_get(r);
  MiniMacsRep res = 0;
  MR mr = {0};
  
  oe->p("Generic MiniMacs add");

  if (gmm->peer_map->size() < 1) MRRF("[add] No peers connected.");

  if (!left) MRRF("Left operand (%d) was not set.",l);
  if (!right) MRRF("Right operand (%d) was not set.", r);

  mr = gmm->__add__(&res,left,right);
  if(mr.rc == 0) this->heap_set(dst,res);
  return mr;
}}

COO_DCL(GenericMiniMacs, MR, __add__, MiniMacsRep * res_out, MiniMacsRep left, MiniMacsRep right)
COO_DEF_RET_ARGS(GenericMiniMacs, MR, __add__,MiniMacsRep * res_out; MiniMacsRep left; MiniMacsRep right;,res_out, left, right) {
  OE oe = this->oe;
  bool left_const = minimacs_rep_is_public_const(left);
  bool right_const = minimacs_rep_is_public_const(right);
  MiniMacsRep result = 0;

  if (!res_out) {
    MRRF("Programing error no pointer to store result");
  }
  if (!left) {
    MRRF("Left operand not set");
  }

  if (!right) {
    MRRF("Right operand not set");
  }

  // left is proper rep and right is a public constant
  if (left_const && !right_const) {
    MiniMacsEnc enc = this->encoder;
    //    if (left->lval == matrix_getwidth(this->big_encoder)) enc = this->big_encoder;
    result = minimacs_rep_add_const_fast(enc, right, left->codeword, left->lval);
  }

  // left is a public constant and right is a proper rep
  if (!left_const && right_const) {
    MiniMacsEnc enc = this->encoder;
    result = minimacs_rep_add_const_fast(enc, left, right->codeword, right->lval);
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
  *res_out = result;
  MR_RET_OK;
}}



COO_DCL(MiniMacs,MR,mul,uint dst, uint l, uint r) 
COO_DEF_RET_ARGS(MiniMacs,MR, mul, uint dst; uint l; uint r;,dst,l,r) {
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  MiniMacsRep left = this->heap_get(l);
  MiniMacsRep right = this->heap_get(r);
  bool left_const = minimacs_rep_is_public_const(left);
  bool right_const = minimacs_rep_is_public_const(right);
  MiniMacsRep result = 0;
  
  MR mr = {{0}};
  MiniMacsRep * star_pair = 0;
  MiniMacsTripleRep triple = 0;
  MiniMacsRep delta = 0;
  MiniMacsRep epsilon = 0;
  Data d = 0;
  MpcPeer peer = 0;
  Data * delta_in = 0;
  Data delta_clear = 0;
  Data * epsilon_in = 0;
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

  if (!left) MUL_FAIL("Left operand (%d) is not set.", l);

  if (!right) MUL_FAIL("Right operand (%d) is not set.", r);

  // Both public constants
  if (left_const && right_const) {
    uint j = 0;
    Data tmpd = Data_new(gmm->oe,left->lval);
    for(j = 0;j < left->lval;++j) {
      tmpd->data[j] = multiply(left->codeword[j],right->codeword[j]);
    }
    result = minimacs_create_rep_public_plaintext_fast(gmm->encoder,tmpd->data, tmpd->ldata, left->lcodeword);
    this->heap_set(dst, result);
    Data_destroy(gmm->oe,&tmpd);
    MR_RET_OK;
  }

  // left public constant, right proper rep
  if (!left_const && right_const) {
    MiniMacsEnc enc = gmm->encoder;
    result = minimacs_rep_mul_const_fast(enc, right, left->codeword, left->lval); 
    this->heap_set(dst, result);
    MR_RET_OK;
  }

  // left proper rep, right public constant
  if (left_const && !right_const) {
    MiniMacsEnc enc = gmm->encoder;
    result = minimacs_rep_mul_const_fast(enc,left,right->codeword, right->lval);
    this->heap_set(dst, result);
    MR_RET_OK;
  }

  oe->p("Generic MiniMacs mul");

  triple = gmm->next_triple();
  if (!triple) MUL_FAIL("No more triples (%d taken).", gmm->idx_triple);
  
  star_pair = gmm->next_pair();
  if (!star_pair) MUL_FAIL("No more pairs (%d taken).",gmm->idx_pair);
  
  mr = gmm->__add__(&delta,triple->a, left);
  if (mr.rc != 0) MUL_FAIL("Could not add triple->a and left");

  mr = gmm->__add__(&epsilon,triple->b, right);
  if (mr.rc != 0) MUL_FAIL("Could not add triple->b and right");

  if (epsilon->lval != delta->lval) MUL_FAIL("Inconsistent epsilon and delta");

  // broadcast delta and epsilon
  for(id = 0;id < gmm->peer_map->size()+1;++id) {
    if (id == gmm->myid) continue; 
    
    peer = gmm->peer_map->get( (void*)(unsigned long long)id );
    if (!peer) MUL_FAIL("Peer with id, %u, is undefined",id);
    
    peer->send(Data_shallow(delta->codeword, delta->lcodeword));
    peer->send(Data_shallow(delta->mac[id]->mac, delta->mac[id]->lmac));
    peer->send(Data_shallow(epsilon->codeword, epsilon->lcodeword));
    peer->send(Data_shallow(epsilon->mac[id]->mac, epsilon->mac[id]->lmac));
  }

  deltamac = Data_new(oe, delta->lcodeword);
  epsilonmac = Data_new(oe,epsilon->lcodeword);

  // receive and check epsilon and delta from every one else.
  delta_in = oe->getmem(sizeof(*delta_in)*(gmm->peer_map->size()+1));
  if (!delta_in) MRGF("Ran out of memory allocating shares for receiving delta.");
  for(id = 0;id<gmm->peer_map->size()+1;++id) {
    delta_in[id] = Data_new(gmm->oe,delta->lcodeword);
    if (!delta_in[id]) MRGF("Ran out of memory allocating shares for receiving delta (%u).",id);
  }
  epsilon_in = oe->getmem(sizeof(*epsilon_in)*(gmm->peer_map->size()+1));
  if (!epsilon_in) MRGF("Ran out of memory allocating shares for receiving epsilon.");
  for(id = 0;id<gmm->peer_map->size()+1;++id) {
    epsilon_in[id] = Data_new(gmm->oe, epsilon->lcodeword);
    if (!epsilon_in[id]) MRGF("Ran out of memory allocating shares for receiving epsilon (%u).", id);
  }
  for(id = 0;id<gmm->peer_map->size()+1;++id) {
    if (id == gmm->myid) continue;

    peer = gmm->peer_map->get( (void*)(ull)id);
    if (!peer) MUL_FAIL("Peer with id, %u, is undefined (after sending to him)", id);
    
    peer->receive(delta_in[id]);
    if (delta_in[id]->ldata != delta->lcodeword) 
      MRGF("Received delta with invalid length (%u) from %u",
	   delta_in[id]->ldata, id);

    peer->receive(deltamac);
    if (deltamac->ldata < delta->lcodeword) 
      MRGF("Received delta mac with invalid length (%u) from %u",
	   deltamac->ldata, id);

    peer->receive(epsilon_in[id]);
    if (epsilon_in[id]->ldata != epsilon->lcodeword) 
      MRGF("Received epsilon with invalid length (%u) from %u",
	   epsilon_in[id]->ldata, id);

    peer->receive(epsilonmac);
    if (epsilonmac->ldata != epsilon->lcodeword) 
      MRGF("Received epsilon with invalid length (%u) from %u",
	   epsilonmac->ldata, id);

    if (!
	minimacs_check_othershare_fast(gmm->encoder,delta,
				      id,
				      delta_in[id]->data,
				      deltamac->data,
				      delta_in[id]->ldata) ) {
      MRGF("Peer %u is cheating, mac didn't check out on delta.",id);
    }

    if (!minimacs_check_othershare_fast(gmm->encoder,epsilon,
					id, 
					epsilon_in[id]->data,
					epsilonmac->data,
					epsilon_in[id]->ldata)) {
      MRGF("Peer %u is cheating, mac didn't check out on epsilon.",id);
    }
  }

  Data_destroy(oe,&deltamac);
  Data_destroy(oe,&epsilonmac);

  // TODO(rwl): delta_in is one share that we can receive, if peer_map
  // has two or more peers this strategy fails. Fix it for more than
  // two peers be having a share list!

  //compute clear text delta
  delta_clear = Data_new(gmm->oe,delta->lval);
  for(id = 0;id < gmm->peer_map->size()+1;++id) {
    for(i = 0;i < delta->lval;++i) {
      delta_clear->data[i] = add(delta_clear->data[i],delta_in[id]->data[i]);
    }
    Data_destroy(gmm->oe, &delta_in[id] );
  }
  oe->putmem(delta_in);delta_in = 0;
  for(i = 0;i<delta->lval;++i) {
    delta_clear->data[i] = add(delta->dx_codeword[i],add(delta->codeword[i], delta_clear->data[i]));
  }
  minimacs_rep_clean_up( & delta );
  // compute clear text epsilon
  epsilon_clear = Data_new(gmm->oe,epsilon->lval);
  for(id = 0; id < gmm->peer_map->size()+1;++id) {
    for(i = 0;i < epsilon->lval;++i) {
      epsilon_clear->data[i] = add(epsilon_clear->data[i],epsilon_in[id]->data[i]);
    }
    Data_destroy(gmm->oe, &epsilon_in[id] );
  }
  oe->putmem(epsilon_in);epsilon_in = 0;
  for(i = 0;i < epsilon->lval;++i) {
    epsilon_clear->data[i] = add(epsilon_clear->data[i],add(epsilon->dx_codeword[i],epsilon->codeword[i]));
  }
  minimacs_rep_clean_up( &epsilon );

  
  delta_b = minimacs_rep_mul_const_fast(gmm->encoder, triple->b, delta_clear->data, delta_clear->ldata);
  if (!delta_b) MRGF("Failed to compute the product of delta clear and triple->b");

  epsilon_a = minimacs_rep_mul_const_fast(gmm->encoder, triple->a, 
					  epsilon_clear->data, epsilon_clear->ldata);
  if (!epsilon_a) MRGF("Failed to compute the product of epsilon clear and triple->a");

  epsilon_times_delta = Data_new(gmm->oe,delta_clear->ldata);
  if (!epsilon_times_delta) MRGF("Ran out of memory computing epsilon times delta");
  
  for(i = 0;i < delta_clear->ldata;++i) {
    epsilon_times_delta->data[i] = multiply(delta_clear->data[i],
					    epsilon_clear->data[i]);
  }

  tmp = minimacs_rep_xor(delta_b, epsilon_a);
  if (!tmp) MRGF("Failed to add delta_b and epsilon_a");

  minimacs_rep_clean_up(&delta_b);
  
  tmp2 = minimacs_rep_add_const_fast(gmm->encoder, tmp, epsilon_times_delta->data,
				     epsilon_times_delta->ldata);
  if (!tmp2) MRGF("Failed to add epsilon*delta with (b*delta + epsolon*a).");
  
  minimacs_rep_clean_up(&epsilon_a);
  minimacs_rep_clean_up(&tmp);

  res_star = minimacs_rep_xor(tmp2, triple->cstar);
  if (!res_star) MRGF("Failed to compute result shur transform.");
  minimacs_rep_clean_up(&tmp2);

  sigma_star = minimacs_rep_xor(res_star, star_pair[1]);
  if (!sigma_star) MRGF("Failed to compute sigma*.");

  // TODO(rwl): This is kind of strange:
  res_star->lval = star_pair[1]->lval;

  // Now sigma star is opened privately to player one who checks it is
  // a codeword in the shur transform. Then player one broadcasts
  // sigma using the first entry of the star_pair.
  //
  // All other players participate in opening to player one and
  // receives sigma star. 
  //
  // every peer forms the representation of left*right.
  if (gmm->myid == 0) {
    Data * sigma_star_in = oe->getmem(sizeof(*sigma_star_in)*(gmm->peer_map->size()+1));
    Data sigmamac = Data_new(oe, sigma_star->lcodeword);
    uint lsigme_star_in = gmm->peer_map->size()+1;
    for(id = 0; id < gmm->peer_map->size()+1;++id) {
      sigma_star_in[id] = Data_new(oe, sigma_star->lcodeword);
    }
    for(id = 0; id < gmm->peer_map->size()+1;++id) {
      if (id == gmm->myid) continue;

      peer = gmm->peer_map->get( (void*)(ull)id );
      if (!peer) MRGF("Peer %u is not defined, did it disconnect?");
      
      peer->receive(sigma_star_in[id]);
      peer->receive(sigmamac);

      if (!minimacs_check_othershare_fast(gmm->encoder,
					  sigma_star,
					  id,
					  sigma_star_in[id]->data,
					  sigmamac->data,
					  sigma_star_in[id]->ldata) ) {
	MRGF("Peer %u is cheating");
      }
    }

    sigma_star_plain = Data_new(oe,sigma_star->lval);
    if (!sigma_star_plain) MRGF("Ran out of memory while opening sigma star");
    
    for(id = 0;id < gmm->peer_map->size()+1;++id) {
      for(i = 0; i < sigma_star->lval;++i) {
	sigma_star_plain->data[i] = add(sigma_star_plain->data[i],sigma_star_in[id]->data[i]);
      }
    }
    for(i = 0;i<sigma_star->lval;++i) {
      sigma_star_plain->data[i] = 
	add(sigma_star_plain->data[i],add(sigma_star->dx_codeword[i],sigma_star->codeword[i]));
    }

    MEASURE_FN(sigma = gmm->encoder->encode(sigma_star_plain->data, triple->a->lval));
    if (!sigma) MRGF("Failed to create sigma representation.");

    for(id = 0;id<gmm->peer_map->size()+1;++id) {
      if (id == gmm->myid) continue;

      peer = (MpcPeer)gmm->peer_map->get( (void*)(ull)id);
      if (!peer) MRGF("Peer %u disconnected.");

      peer->send(Data_shallow(sigma,sigma_star->lcodeword));
    }

    result = minimacs_rep_add_const_fast(gmm->encoder,star_pair[0],sigma,left->lval);
    if (!result) MRGF("Computing result failed");
    
    this->heap_set(dst, result);
    MR_RET_OK;
  } else {
    Data sigma_plain = 0;
    peer = gmm->peer_map->get(0);
    if (!peer) MRGF("Party one disconnected.");
    
    peer->send(Data_shallow(sigma_star->codeword, sigma_star->lcodeword));
    peer->send(Data_shallow(sigma_star->mac[0]->mac, sigma_star->mac[0]->lmac));
    
    sigma_plain = Data_new(oe, sigma_star->lcodeword);
    peer->receive(sigma_plain);

    if (!gmm->encoder->validate(sigma_plain->data, left->lval)) 
      MRGF("Invalid sigma player one is cheating.");
    
    result = minimacs_rep_add_const_fast(gmm->encoder, star_pair[0], sigma_plain->data, left->lval);
    if (!result) MRGF("Failed to compute result.");
    
    this->heap_set(dst ,result);
    MR_RET_OK;
  }


  oe->p("Generic MiniMacs mul");
  MR_RET_OK;
 failure:
  return mr;
}}

COO_DCL(MiniMacs,MR,secret_input, uint pid, hptr dst, Data plain_val)
COO_DEF_RET_ARGS(MiniMacs,MR,secret_input, uint pid; hptr dst; Data plain_val;, pid,dst,plain_val) {
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
  MR mr = {{0}};
  uint id=0,i=0;
  Data val = 0;

  if (gmm->peer_map->size() < 1) MRRF("[secret input] No peers connected.");

  r = gmm->next_single();
  if (!r) MRGF("Ran out of singles, unable to input");
  

  // ------------------------------------------------------------
  // PROTOCOL Branching
  // ------------------------------------------------------------
  if (gmm->myid == pid) {
    shares = oe->getmem(sizeof(*shares)*(gmm->peer_map->size()+1));
    if (!shares) MRGF("Ran out of memory");

    val = Data_new(oe, r->lval);
    if (!val) MRGF("Ran out of memory during input.");
    
    if (plain_val->ldata > val->ldata) 
      MRGF("Raw preprocessing material is setup for messages up length %u bytes."
           "The given message is too long (%u).",plain_val->ldata,val->ldata); 
  
    mcpy(val->data, plain_val->data, plain_val->ldata);


    shares = (Data *)oe->getmem(sizeof(*shares)*(gmm->peer_map->size()+1));
    if (!shares) MRGF("Ran out of memory.");

    for(id = 0;id < gmm->peer_map->size()+1;++id) {
      shares[id] = Data_new(oe, r->lcodeword);
      if (!shares[id]) MRGF("Ran out of memory for shares %u.",id);
    }
    sharemac = Data_new(oe, r->lcodeword);
    if (!sharemac) MRGF("Ran out of memory for sharemac.");

    clear_r = Data_new(oe,r->lval);
    if (!clear_r) MRGF("Ran out of memory allocating clear_r.");
    
    for(id = 0; id < gmm->peer_map->size()+1;++id) {
      if (gmm->myid == id) {
	continue;
      }

      peer = gmm->peer_map->get( (void*)(ull)id);
      if (!peer) MRGF("Peer %u disconnected aborting computation",id);

      peer->receive(shares[id]);
      peer->receive(sharemac);

      if (!minimacs_check_othershare_fast(gmm->encoder,
					  r, 
					  id, 
					  shares[id]->data,
					  sharemac->data,
					  shares[id]->ldata)) {
        MRGF("Peer %u is cheating, mac does not check out on randomness r.",id);
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
    if (!encoded_val->data) MRGF("Failed to encode input value.");
    encoded_r->data = gmm->encoder->encode(clear_r->data, clear_r->ldata);
    encoded_r->ldata = r->lcodeword;
    if (!encoded_r->data) MRGF("Failed to encode single.");
    Data_destroy(oe,&clear_r);
    epsilon = Data_new(oe,r->lcodeword);
    if (!epsilon) MRGF("Ran out of memory during open operation.");

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
    if (!result) MRGF("Unable to add constant to the random a.");


    mr = this->heap_set(dst, result);
    if (mr.rc != 0) return mr;
    MR_RET_OK;
    
  } else {
    // ------------------------------------------------------------
    // Hey I am not the inputter
    // ------------------------------------------------------------
    peer = gmm->peer_map->get( (void*)(ull)pid);
    if (!peer) MRGF("Peer %u is disconnected",pid);

    peer->send(Data_shallow(r->codeword,r->lcodeword));
    peer->send(Data_shallow(r->mac[pid]->mac, r->mac[pid]->lmac));
    epsilon = Data_new(oe,r->lcodeword);
    peer->receive(epsilon);
    if (!gmm->encoder->validate((polynomial*)epsilon->data, r->lval)) {
      MRGF("Data received during input for peer %u was not a codeword.",pid);
    }

    result = minimacs_rep_add_const_fast(gmm->encoder,r,epsilon->data, r->lval);
    if (!result) MRGF("Failed to perform input, add constant to form result failed.");
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
  oe->p("Generic MiniMacs secret input");
  return mr;
}}

COO_DCL(MiniMacs,MR,public_input, hptr dst, Data pub_val)
COO_DEF_RET_ARGS(MiniMacs,MR,public_input, hptr dst; Data pub_val;,dst,pub_val) {
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  MiniMacsRep vrep = minimacs_create_rep_public_plaintext_fast(gmm->encoder, pub_val->data, pub_val->ldata, gmm->singles[0]->lcodeword);
  this->heap_set(dst, vrep);
  oe->p("Generic MiniMacs public input");
  MR_RET_OK;
 failure:
  MR_RET_OK;
}}

COO_DCL(MiniMacs,MR,open, hptr dst)
COO_DEF_RET_ARGS(MiniMacs,MR,open,hptr dst;,dst) {
  uint i = 0;
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  Data * shares = 0;
  OE oe = gmm->oe;
  MiniMacsRep v = this->heap_get(dst);
  MR mr = {{0}};
  uint id = 0;
  MpcPeer peer = 0;
  Data clear = 0;
  Data sharemac = 0;
  MiniMacsRep result = 0;

  if (gmm->peer_map->size() > 1) MRRF("No peers connected.");

  if(!v) MRGF("Value at %u is not set",dst);

  if (minimacs_rep_is_public_const(v)) return;

  shares = (Data *)oe->getmem(sizeof(*shares)*(gmm->peer_map->size()+1));
  if (!shares) MRGF("Failed to allocate shares for opening.");

  for(id = 0;id < gmm->peer_map->size()+1;++id) {
    shares[id] = Data_new(oe, v->lcodeword);
    if (!shares[id]) MRGF("Ran out of memory.");
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
      MRGF("Opening invalid mac on share, peer %u is cheating.",id);
    }

    for(i = 0; i < v->lval;++i) {
      clear->data[i] = add(clear->data[i],shares[id]->data[i]);
    }
  }

  for(i = 0;i<v->lval;++i) {
    clear->data[i] = add(clear->data[i],add(v->dx_codeword[i], v->codeword[i]));
  }

  result = minimacs_create_rep_public_plaintext_fast(gmm->encoder, clear->data, clear->ldata, v->lcodeword);
  this->heap_set(dst, result);

  //  oe->p("Generic MiniMacs open");
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
}}

COO_DCL(MiniMacs, MR, init_heap, uint size)
COO_DEF_RET_ARGS(MiniMacs, MR, init_heap, uint size;,size) {
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  MR mr = {{0}};
  
  gmm->heap = oe->getmem(size*sizeof(MiniMacsRep));
  gmm->lheap = size;

  return mr;
 failure:
  return mr;
}}

COO_DCL(MiniMacs, MR, heap_set, hptr addr, MiniMacsRep rep)
COO_DEF_RET_ARGS(MiniMacs, MR, heap_set ,hptr addr; MiniMacsRep rep;, addr, rep) {
  MR mr = {{0}};
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  
  if (addr < gmm->lheap) {
    if (rep == 0) {
      char m[64] = {0};
      osal_sprintf(m,"Setting address %u to null",addr);
      oe->p(m);
    }
    gmm->heap[addr] = rep;
  } else MRGF("Address %u is out of range (0-%u). Invoke init_heap with a greater number ;).", gmm->lheap, (gmm->lheap==0?0:gmm->lheap-1));
  
  return mr;
 failure:
  return mr;
}}

COO_DCL(MiniMacs, MiniMacsRep, heap_get, uint addr)
COO_DEF_RET_ARGS(MiniMacs,MiniMacsRep,heap_get,uint addr;,addr) {
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  
  if (addr < gmm->lheap) {
    return gmm->heap[addr];
  } 
  
 failure:
  return 0;
}}


COO_DCL(MiniMacs, MR, invite, uint count, uint port)
COO_DEF_RET_ARGS(MiniMacs, MR, invite, uint count; uint port;, count, port) {
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
    peer->send(my_id_raw);
    peer->receive(peer_id_d);
    peer_id = b2i(peer_id_d->data);
    osal_sprintf(m,"registering peer with id %u.",peer_id);
    oe->p(m);
    if (gmm->peer_map->contains( ((void*)(ull)peer_id) )) {
      MRRF("Peer id %u already registered.",peer_id);
    }
    gmm->peer_map->put( ((void*)(ull)peer_id), peer);
  }
  usleep(500);
  MR_RET_OK;
}}

typedef struct _waiting_connecting_listener_ {
  GenericMiniMacs gmm;
  BlkQueue q;
  void (*wait_for)(uint n);
} *WaitingConnectionListener;
#define AS(SUB,PARENT) ((SUB)PARENT->impl) 

COO_DCL(ConnectionListener, void, client_connected, MpcPeer peer)
COO_DEF_NORET_ARGS(ConnectionListener, client_connected, MpcPeer peer;,peer) {
  WaitingConnectionListener wcl = AS(WaitingConnectionListener,this);
  GenericMiniMacs gmm = wcl->gmm;
  OE oe = this->oe;
  Map map = 0;
  uint i = 0;
  Data raw_id = 0;
  Data my_id_raw = Data_new(oe,4);
  uint other_id = 0;
  oe->p("Registering new peer");
  map = gmm->peer_map;
  raw_id = Data_new(oe, 4);
  if (!raw_id) return;
  i2b(gmm->myid, my_id_raw->data);

  peer->send(my_id_raw);
  peer->receive(raw_id);
  
  other_id = b2i(raw_id->data);

  if (map->contains((void*)(ull)other_id) == True) {
    oe->p("Peer already registered :(.");
    return;
  }

  map->put( (void*)(ull)other_id, peer );
  {
    char m[64] = {0};
    osal_sprintf(m,"Added peer %u to mapping.", other_id);
    oe->p(m);
  }
  usleep(500);
  wcl->q->put(0);
  oe->p("New peer registered.");
}}

COO_DCL(ConnectionListener, void, client_disconnected, MpcPeer peer)
COO_DEF_NORET_ARGS(ConnectionListener, client_disconnected, MpcPeer peer;,peer) {
  return;
}}

COO_DCL(WaitingConnectionListener, void, wait_for, uint count)
COO_DEF_NORET_ARGS(WaitingConnectionListener, wait_for, uint count;,count) {
  while(count--) {
    this->q->get();
  }
}}



ConnectionListener WaitingConnectionListener_new(OE oe, GenericMiniMacs gmm) {
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
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  OE oe = gmm->oe;
  CArena arena = gmm->arena;
  CAR car = {{0}};
  ConnectionListener cl = WaitingConnectionListener_new(oe,gmm);
  WaitingConnectionListener wcl = AS(WaitingConnectionListener,cl);
  MpcPeer * map = 0;
  arena->add_conn_listener(cl);
  car = arena->connect(ip,port);
  if (car.rc != 0) MR_RET_FAIL(car.msg);
  wcl->wait_for(1);

  MR_RET_OK;
}}



COO_DCL(MiniMacs, uint, get_no_peers)
COO_DEF_RET_NOARGS(MiniMacs, uint, get_no_peers) {
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  return gmm->peer_map->size();
}}


COO_DCL(MiniMacs, uint, get_ltext)
COO_DEF_RET_NOARGS(MiniMacs, uint, get_ltext) {
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  if (gmm->lsingles > 0) {
    return gmm->ltext;
  }
  return 0;
}}


COO_DCL(MiniMacs, uint, get_lcode)
COO_DEF_RET_NOARGS(MiniMacs, uint, get_lcode) {
  GenericMiniMacs gmm = (GenericMiniMacs)this->impl;
  if (gmm->lsingles > 0) {
    return gmm->lcode;
  }
  return 0;
}}


static uint uint_hfn(void * n) {
  uint un = (uint)(ull)n;
  return 101 * un + 1009;
}

static int uint_cfn(void *a, void *b) {
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

  

  COO_ATTACH(GenericMiniMacs, gres, next_triple);
  COO_ATTACH(GenericMiniMacs, gres, next_single);
  COO_ATTACH(GenericMiniMacs, gres, next_pair);
  COO_ATTACH(GenericMiniMacs, gres, __add__);

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

  load_shares(filename,
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

