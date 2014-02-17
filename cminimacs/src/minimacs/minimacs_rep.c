#include "minimacs/minimacs_rep.h"
#include <ass/ass.h>
#include <stdlib.h>
#include <mutex.h>
#include <math/matrix.h>
#include <common.h>
#include <stdarg.h>
#include <encoding/der.h>
#include <stats.h>
// forward
void minimacs_rep_clean_up( MiniMacsRep * rep );


MiniMacsRep minimacs_rep_from_additive_share(byte * public_codeword, 
					     byte * private_codeword,
					     uint lcodeword,
					     uint nplayers,
					     MiniMacsRep * compat) {

  return 0;
}


MiniMacsRep * minimacs_create_rep_from_plaintext_f(MiniMacsEnc encoder, 
						  byte* text,
						  uint ltext,uint nplayers, 
						  uint codelength, 
						  MiniMacsRep * compat_with ) {  // allocate result

  MiniMacsRep * res_vec = (MiniMacsRep*)malloc(nplayers*sizeof(*res_vec));
  uint i = 0, kowner=0,mowner=0;
  byte ** shares = 0;
  byte * dx = 0;
  static unsigned int _unique_id_ = 0;
  ++_unique_id_;
  if (!text) goto failure;

  if (!res_vec) goto failure;

  memset(res_vec,0,sizeof(*res_vec)*nplayers);

  // create the shares (the last share is v, dx=C(v))
  shares = ass_create_shares( text, ltext, nplayers + 1 );
  if (!shares) goto failure;

  // encode the public code word for this message
  MEASURE_FN(dx = encoder->encode((polynomial*)shares[nplayers], ltext));
  if (!dx) goto failure;

  // generate all the mac keys up front
  for(kowner=0;kowner<nplayers;kowner++) {

    // allocate
    MiniMacsRep current = (MiniMacsRep)malloc(sizeof(*current));
    if (!current) goto failure;
    memset(current,0,sizeof(*current));
    res_vec[kowner] = current;
    res_vec[kowner]->lval = ltext;

    // allocate keys for kowner
    res_vec[kowner]->mac_keys_to_others = (bedoza_mac_key*)malloc(nplayers*sizeof(bedoza_mac_key));
    if (!res_vec[kowner]->mac_keys_to_others) goto failure;
    memset(res_vec[kowner]->mac_keys_to_others,0,nplayers*sizeof(bedoza_mac_key));
    res_vec[kowner]->lmac_keys_to_others = nplayers;

    // generate a key to every other mac owner
    for(mowner=0;mowner<nplayers;mowner++) {
      if (compat_with) {
	res_vec[kowner]->mac_keys_to_others[mowner] = 
	  bedoza_generate_compat_key(compat_with[kowner]->mac_keys_to_others[mowner],
				     _unique_id_,
				     mowner,
				     kowner );
      } else {
	res_vec[kowner]->mac_keys_to_others[mowner] = 
	  generate_bedoza_mac_key( codelength, _unique_id_, mowner, kowner );
      }
      if (!res_vec[kowner]->mac_keys_to_others[mowner]) goto failure;
    }

  }

  // for each players create its representation 
  for(i = 0; i < nplayers;i++) {
    int p = 0;  
    
    MiniMacsRep current = res_vec[i];

    // public reed solomon code word
    current->dx_codeword = (byte*)malloc(codelength);
    if (!current->dx_codeword) goto failure;
    memcpy(current->dx_codeword,dx,codelength);
    current->ldx_codeword = codelength;

    // reed solomon code word
    MEASURE_FN(current->codeword = (byte*)encoder->encode((polynomial*)shares[i], ltext));
    if (!current->codeword) goto failure;
    current->lcodeword= codelength;

    // allocate macs
    current->lmac = nplayers;
    current->mac = (bedoza_mac *)malloc(nplayers*sizeof(bedoza_mac));
    if (!current->mac) goto failure;
    memset(current->mac,0,nplayers*sizeof(bedoza_mac));
    // compute macs
    for(p=0;p<nplayers;p++) {
      
      uint mac_owner = i;
      uint key_owner = p;
      bedoza_mac_key key = res_vec[key_owner]->mac_keys_to_others[mac_owner];
      
      if (i == p) continue;
      
      current->mac[key_owner] = 
	compute_bedoza_mac(key,current->codeword, current->lcodeword);

      if (!current->mac[key_owner]) goto failure;
      
    }
  }

  ass_clean_shares( shares, nplayers + 1);

  if (dx) (free(dx),dx=0);

  return res_vec;
 failure:

  ass_clean_shares( shares, nplayers + 1);

  if (dx) (free(dx),dx=0);

  if (res_vec) {
    for(i=0;i<nplayers;i++) {
      minimacs_rep_clean_up(&(res_vec[i]));
    }
    (free(res_vec),res_vec=0);
  }

  // TODO Clean up
  return 0;
};


void minimacs_rep_clean_up( MiniMacsRep * rep ) {
  
  MiniMacsRep r = 0;
  int i = 0;
  
  if (!rep) return;

  if (!*rep) return;

  r = *rep;

  if (r->dx_codeword) (free(r->dx_codeword),r->dx_codeword=0,r->ldx_codeword=0);

  if (r->codeword) (free(r->codeword),r->codeword=0,r->lcodeword = 0);

  if (r->mac) {
    for(i=0;i<r->lmac;i++) {
      bedoza_mac_destroy ( r->mac + i );
    }
    (free(r->mac),r->mac=0,r->lmac=0);
  }

  if (r->mac_keys_to_others) {
    for(i=0;i<r->lmac_keys_to_others;i++) {
      bedoza_mac_key_destroy( r->mac_keys_to_others + i );
    }
    (free(r->mac_keys_to_others),r->mac_keys_to_others=0,r->lmac_keys_to_others=0);
  }

  (free(r),r=0,*rep=0);

  return;
}

bool minimacs_check_representation_fast( MiniMacsEnc encoder, MiniMacsRep  rep ) {

  if (!rep) return 0;
  
  // check the code dx code word
  if (!encoder->validate(rep->dx_codeword, rep->lval) ) return 0;

  // check the code word
  if (!encoder->validate(rep->codeword, rep->lval)) return 0;

  return 1;

}
/*! Slooow 
bool minimacs_check_representation(MiniMacsRep  rep ) {

  MATRIX * encoder = 0;
  if (!rep) return 0;

  encoder = minimacs_generate_encoder( rep->lval, rep->lcodeword );
  if (!encoder) return 0;

  return minimacs_check_representation_fast( encoder, rep );
}
*/

/* Slooooow 
MiniMacsRep * minimacs_create_rep_from_plaintext(
						  byte* text,
						  uint ltext,uint nplayers, 
						  uint codelength, 
						  MiniMacsRep * compat_with ) {
  MiniMacsRep * res = 0;
  MATRIX * encoder = 0;

  encoder = minimacs_generate_encoder(ltext,codelength);
  if (!encoder) {
    return 0;
  }

  res = minimacs_create_rep_from_plaintext_f(encoder,text,ltext,nplayers,codelength,compat_with);
  destroy_matrix( encoder );encoder = 0;
 failure:
  return res;
  
}
*/

bool minimacs_check_othershare_fast(MiniMacsEnc enc,
				    MiniMacsRep rep,  
				    uint other_party_id,
				    byte * codeword, 
				    byte * mac, 
				    uint lcodeword) {

  bedoza_mac_key mkey = 0;

  bool mac_chk_ok = 0;
  bool code_chk_ok = 0;
  bool check_rep_ok = 0;

  struct __bedoza_mac__ m = {0};

  if (!rep) return 0;

  if (!codeword) return 0;

  if (!mac) return 0;

  check_rep_ok =  minimacs_check_representation_fast( enc,rep );
  if (!check_rep_ok) return 0;
  
  if (other_party_id > rep->lmac_keys_to_others) return 0;

  mkey = rep->mac_keys_to_others[other_party_id];

  if (!mkey) { 
    return 0;
  }

  m.mid = mkey->mid;
  m.toid = other_party_id;
  m.fromid = mkey->fromid;
  m.mac = mac;
  m.lmac = lcodeword;

  mac_chk_ok = bedoza_check_mac(mkey,&m,codeword,lcodeword);

  code_chk_ok = enc->validate(codeword, rep->lval);

  return mac_chk_ok && code_chk_ok;

}



MiniMacsRep minimacs_rep_xor(MiniMacsRep left, MiniMacsRep right) {

  int i = 0;
  MiniMacsRep res = 0;

  if (!left) return 0;

  if (!right) return 0;

  if (left->ldx_codeword != right->ldx_codeword) return 0;

  if (left->lcodeword != right->lcodeword ) return 0;

  if (left->lmac != right->lmac) return 0;

  if( left->lmac_keys_to_others != right->lmac_keys_to_others ) return 0;

  if ( left->lmac_keys_to_others != left->lmac) return 0;
  
  // This check fails for a normal and a star representation 
  //  if (left->lval != right->lval) return 0;

  res = (MiniMacsRep)malloc(sizeof(*res));
  if (!res) return 0;
  memset(res,0,sizeof(*res));

  res->lval = left->lval;

  // dx codeword
  res->dx_codeword = (byte*)malloc(left->ldx_codeword);
  if (!res->dx_codeword) goto failure;
  res->ldx_codeword = left->ldx_codeword;

  polynomial_add_vectors( res->dx_codeword, left->dx_codeword, right->dx_codeword, res->ldx_codeword);
  
  // codeword
  res->codeword = (byte*)malloc(left->lcodeword);
  if (!res->codeword) goto failure;
  res->lcodeword = left->lcodeword;
  
  polynomial_add_vectors( res->codeword, left->codeword, right->codeword, res->lcodeword);

  // add macs
  res->mac = (bedoza_mac*)malloc(left->lmac * sizeof(bedoza_mac));
  if (!res->mac) goto failure;
  memset(res->mac,0,left->lmac * sizeof(bedoza_mac));
  res->lmac = left->lmac;

  res->mac_keys_to_others = (bedoza_mac_key*)malloc(left->lmac_keys_to_others * sizeof(bedoza_mac_key));
  if (!res->mac_keys_to_others) goto failure;
  memset(res->mac_keys_to_others,0,left->lmac_keys_to_others * sizeof(bedoza_mac_key));
  res->lmac_keys_to_others = left->lmac_keys_to_others;

  for(i = 0; i < res->lmac; i++) {

    if (left->mac[i] == 0 && right->mac[i] == 0) continue; // no self macs

    bedoza_add_macs( left->mac[i], 
		     right->mac[i],
		     (res->mac+i));

    bedoza_add_mac_keys( left->mac_keys_to_others[i],
			 right->mac_keys_to_others[i],
			 (res->mac_keys_to_others+i));
    
    if (!res->mac[i]) goto failure;

    if (!res->mac_keys_to_others[i]) goto failure;
  }
  
  return res;
 failure:
  minimacs_rep_clean_up( &res );
  return 0;
}

MiniMacsRep minimacs_create_rep_public_plaintext_fast(
						      MiniMacsEnc encoder,
						      byte * text,
						      uint ltext,
						      uint codelength
						      ) {

  MiniMacsRep res = (MiniMacsRep)malloc(sizeof(*res));
  if (!res) goto failure;
  memset(res,0,sizeof(*res));

  res->lval = ltext;
  MEASURE_FN(res->codeword = encoder->encode((polynomial*)text,ltext));
  if (!res->codeword) goto failure;
  res->lcodeword = codelength;
  
  return res;
 failure:
  if (res) minimacs_rep_clean_up( & res );
  return 0;

}

/*! Too slow
MiniMacsRep minimacs_create_rep_public_plaintext(byte * text,
						 uint ltext,
						 uint codelength
						 ) {

  MATRIX * encoder = minimacs_generate_encoder(ltext,codelength);
  if (!encoder) return 0;
  
  return minimacs_create_rep_public_plaintext_fast(encoder,text,ltext,codelength);
}
*/

MiniMacsRep * minimacs_rep_mul_fast(MiniMacsEnc enc,
				    MiniMacsRep * left, MiniMacsRep * right, uint nplayers) {
  uint i = 0, j = 0, p = 0;
  //  byte * alpha_inv = 0;
  byte * clear_txt = 0;
  uint lclear_txt = 0;

  // allocate result
  MiniMacsRep * res = 0;//(MiniMacsRep*)malloc(sizeof(*res)*nplayers);

  if (!left) goto failure;

  if (!right) goto failure;

  for(p = 0; p < nplayers; ++p) {

    if (!left[p]) goto failure;

    if (!right[p]) goto failure;

    if (left[p]->ldx_codeword != right[p]->ldx_codeword) goto failure;
    
    if (left[p]->lcodeword != right[p]->lcodeword) goto failure;
    
    if (left[p]->lmac != right[p]->lmac) goto failure;
    
    if (left[p]->lmac_keys_to_others != right[p]->lmac_keys_to_others) goto failure;
  
    if (!enc->validate(left[p]->dx_codeword, left[p]->lval ) ) {
      printf("Oh crap left !\n");
    }

    if (!enc->validate(right[p]->dx_codeword, right[p]->lval ) ) {
      printf("Oh crap right !\n");
    }
  }

  clear_txt = (byte*)malloc(2*left[0]->lval);
  if (!clear_txt) {
    goto failure;
  }
  memset(clear_txt,0,2*left[0]->lval);

  for(i=0;i<left[0]->lval;++i) {
    byte 
      left_plain = left[0]->dx_codeword[i],
      right_plain=right[0]->dx_codeword[i];
    
    for(j = 0; j < nplayers;++j) {
      left_plain ^= left[j]->codeword[i];
      right_plain ^= right[j]->codeword[i];
    }
    
    clear_txt[i] = multiply(left_plain,right_plain);
  }

  /*
   * TODO(rwl): This might not be the best thing to do, we are
   * encoding to the Schur transform and that is slow matrix wise
   * using the fft encoder...  But okay this functions requests all
   * shares to be available in one place ... only for preprocessing or
   * cheating ;)
   *
   */
  res = minimacs_create_rep_from_plaintext_f(enc, clear_txt,2*left[0]->lval,
						nplayers, left[0]->lcodeword,
						left);
  if (clear_txt) {free(clear_txt);clear_txt = 0;}
  return res;
 failure:
  if (clear_txt) {free(clear_txt);clear_txt = 0;}
  return 0 ;

}

/*! Slooooow 
MiniMacsRep * minimacs_rep_mul(MiniMacsRep * left, MiniMacsRep * right, uint nplayers) {

  MATRIX * sm = 0, * bm=0;
  MiniMacsRep * res = 0;

  if (!left) return 0;

  if (!left[0]) return 0 ;

  sm = minimacs_generate_encoder( left[0]->lval, left[0]->lcodeword );
  if (!sm) return 0;

  bm = minimacs_generate_encoder( left[0]->lval * 2, left[0]->lcodeword );
  if (!bm) return 0;

  res = minimacs_rep_mul_fast(sm,bm,left,right,nplayers);
  if (sm) {
    destroy_matrix( sm );
  }
  
  if (bm) {
    destroy_matrix( bm );
  }
  return res;
}

*/ 

  /*

    failure:
      if (alpha_inv) {
    free(alpha_inv);alpha_inv = 0;
  }
  if (res) {
    for(p=0;p<nplayers;++p) {
      minimacs_rep_clean_up( & res[p] );
    }
    free(res);
  }
// -----
    
    // dx codeword
    res[p]->dx_codeword = (byte*)malloc(left[p]->ldx_codeword);
    if (!res[p]->dx_codeword) goto failure;
    memset(res[p]->dx_codeword,0,left[p]->ldx_codeword);
    res[p]->ldx_codeword = left[p]->ldx_codeword;
    for(i = 0;i < res[p]->ldx_codeword;++i) {
      res[p]->dx_codeword[i] = multiply( (polynomial)left[p]->dx_codeword[i],
					 (polynomial)right[p]->dx_codeword[i]);
    }

    if (!minimacs_validate( res[p]->dx_codeword, res[p]->ldx_codeword, 2*(left[p]->lval) ) ) {
      printf("Oh crap !\n");
    }

    // codeword
    res[p]->codeword = (byte*)malloc(left[p]->lcodeword);
    if (!res[p]->codeword) goto failure;
    memset(res[p]->codeword,0,left[p]->lcodeword);
    res[p]->lcodeword = left[p]->lcodeword;
    for(i=0;i<res[p]->lcodeword;i++) {
      res[p]->codeword[i] = multiply( (polynomial)left[p]->codeword[i],
				      (polynomial)right[p]->codeword[i]);
    }

    // macs
    res[p]->mac = (bedoza_mac*)malloc(left[p]->lmac*sizeof(bedoza_mac));
    if (!res[p]->mac) goto failure;
    memset(res[p]->mac,0,left[p]->lmac*sizeof(bedoza_mac));
    res[p]->lmac = left[p]->lmac;


    // mac keys
    res[p]->mac_keys_to_others = (bedoza_mac_key*)
      malloc(left[p]->lmac_keys_to_others*sizeof(bedoza_mac_key));
    if (!res[p]->mac_keys_to_others) goto failure;
    memset(res[p]->mac_keys_to_others,0,left[p]->lmac_keys_to_others*sizeof(bedoza_mac_key));
    res[p]->lmac_keys_to_others = left[p]->lmac_keys_to_others;

    // compute new macs
    for(i=0;i<res[p]->lmac;++i) {
      if (!left[p]->mac[i]) continue;
      
      bedoza_mul_macs(left[p]->mac[i],right[p]->mac[i],&res[p]->mac[i]);
      if (!res[p]->mac[i]) goto failure;
     
      bedoza_mul_mac_keys(left[p]->mac_keys_to_others[i],
			  left[i]->codeword,
			  right[p]->mac_keys_to_others[i],
			  right[i]->codeword,
			  res[p]->mac_keys_to_others+i);

      
      
      if (!res[p]->mac_keys_to_others[i]) goto failure;
    }

    res[p]->lval = 2*(left[p]->lval );
  }

  for(p = 0; p <nplayers;++p) {
    for(i = 0;i<res[p]->lmac;++i) {
      uint k = 0;
      if (i == p) continue;
      
      for(k=0;k<res[p]->lcodeword;++k) {
	res[p]->mac[i]->mac[k] = multiply(res[p]->mac[i]->mac[k], 
					  inverse(res[i]->mac_keys_to_others[p]->alpha[k]));
      }
    }
  }

  */

MiniMacsRep minimacs_rep_add_const_fast(MiniMacsEnc encoder, MiniMacsRep rep, byte * c, uint lc ) {


  MiniMacsRep r = 0;
  byte * encoded_c = 0;
  uint lencoded_c = 0;
  uint i  = 0;
  uint ltext = 0;
  byte * tmp = 0;

  if (!rep) {
    goto failure;
  }

  if (!c) {
    goto failure;
  }

  if (lc > rep->lval) {
    goto failure;
  }

  if (rep->ldx_codeword != rep->lcodeword) {
    goto failure;
  }


  // extend c to length of rep->lval
  tmp = (byte*)malloc(rep->lval);
  if (!tmp) goto failure;
  memset(tmp,0,rep->lval);

  memcpy(tmp,c,lc);

  // compute C(u), the constant as a codeword 
  MEASURE_FN(encoded_c = encoder->encode(tmp,rep->lval));
  free(tmp);tmp=0;
  if (!encoded_c) { 
    goto failure;
  }
  lencoded_c = rep->lcodeword;

  // Allocate the result
  r = (MiniMacsRep)malloc(sizeof(*r));
  if (!r) {
    goto failure;
  }
  memset(r,0,sizeof(*r));
  
  // allocate dx_codeword
  r->dx_codeword = (byte*)malloc(rep->ldx_codeword);
  if (!r->dx_codeword) {
    goto failure;
  }
  memset(r->dx_codeword,0,rep->lcodeword);
  r->ldx_codeword = rep->ldx_codeword;
  r->lval = rep->lval;

  // compute r->dx_codeword = rep->dx_codeword - C(u)
  for(i = 0; i < lencoded_c;++i) {
    r->dx_codeword[i] = sub( rep->dx_codeword[i], 0x00 );
  }

  
  // allocate codeword
  r->codeword = (byte*)malloc(rep->lcodeword);
  if (!r->codeword) {
    goto failure;
  }
  memset(r->codeword,0,rep->lcodeword);
  r->lcodeword = rep->lcodeword;

  // allocate macs
  r->mac = (bedoza_mac*)malloc(rep->lmac*sizeof(bedoza_mac));
  if (!r->mac) {
    goto failure;
  }
  memset(r->mac,0,rep->lmac);
  r->lmac = rep->lmac;
  
  // allocate mey_keys
  r->mac_keys_to_others = (bedoza_mac_key*)
    malloc(rep->lmac_keys_to_others*sizeof(bedoza_mac_key));
  if (!r->mac_keys_to_others) {
    goto failure;
  }
  memset(r->mac_keys_to_others,
	 0,
	 rep->lmac_keys_to_others*sizeof(bedoza_mac_key)
	 );
  r->lmac_keys_to_others = rep->lmac_keys_to_others;

  // if first share we also modify the codeword
  // A player never holds a mac towards his own share because he need
  // not convince him self it is correct. 
  //
  // Thus the mac map has a NULL entry for the index of him self. The
  // first player has index 0 and so if mac[0] is null this
  // representation is the first share !
  //
  if (rep->mac[0] == 0) {
    for(i = 0; i < lencoded_c;++i) {
      r->codeword[i] = add(encoded_c[i],rep->codeword[i]);
    }
  } else {
    for(i = 0; i< rep->lcodeword;++i) {
      r->codeword[i] = rep->codeword[i];
    }
  }

  // the macs remain the same
  for(i = 0; i < rep->lmac;++i) {
    r->mac[i] = bedoza_mac_copy( rep->mac[i] );
  }
  
  if (rep->mac[0] == 0) { // if first share
    // mac keys are all the same for player 0
    for(i = 0; i < rep->lmac_keys_to_others;++i) {
      r->mac_keys_to_others[i] = 
	bedoza_mac_key_copy( rep->mac_keys_to_others[i] );
    }
  } else {
    // subtract alpha[i]*c(u)[i] from b[i] to form the new mac key
    // towards player 0
    for(i = 0; i < rep->lmac_keys_to_others;++i) {
      bedoza_mac_key k = 0;
      k = r->mac_keys_to_others[i] = bedoza_mac_key_copy( rep->mac_keys_to_others[i] );
      if (i == 0) {
	uint j = 0;
	for(j = 0;j<rep->lcodeword;++j) {
	  polynomial new_beta_j = add(k->beta[j], multiply(k->alpha[j],encoded_c[j]));
	  k->beta[j] = new_beta_j;
	}
      }
    }
  }
  
  
  return r;
 failure:
  minimacs_rep_clean_up( &r );
  return 0;

  
}

/*! Sloooow 
MiniMacsRep minimacs_rep_add_const(MiniMacsRep rep, byte * c, uint lc ) {
  
  if (!rep) return 0;

  MATRIX * encoder = minimacs_generate_encoder(rep->lval, rep->lcodeword);
  if (!encoder) {
    return 0;
  }
  return minimacs_rep_add_const_fast(encoder,rep,c,lc);
}
*/


MiniMacsRep minimacs_rep_mul_const_fast(MiniMacsEnc encoder, MiniMacsRep rep, byte * c, uint lc) { 
  uint i = 0, j = 0;
  MiniMacsRep r = 0;
  byte * encoded_c = 0;
  byte * tmp = 0;

  if (!rep) {
    goto failure;
  }

  if (!c) {
    goto failure;
  }

  if (rep->lval < lc) {
    goto failure;
  }

  r = (MiniMacsRep)malloc(sizeof(*r));
  if (!r) { 
    goto failure;
  }
  memset(r,1,sizeof(*r));
  
  if (lc < rep->lval) {
    tmp =(byte*)malloc(rep->lval);
    if (!tmp) goto failure;
    memset(tmp,0,rep->lval);
    memcpy(tmp,c,lc);
  } else {
    tmp = c;
  }


  // encoded c
  MEASURE_FN(encoded_c = encoder->encode(tmp,rep->lval));
  if (!encoded_c) {
    goto failure;
  }
  
  r->lval = 2*rep->lval;

  // dx codeword 
  r->dx_codeword = (byte*)malloc(rep->ldx_codeword);
  if (!r->dx_codeword) {
    goto failure;
  }
  memset(r->dx_codeword,0,rep->ldx_codeword);
  r->ldx_codeword = rep->ldx_codeword;
  for(i = 0; i < r->ldx_codeword;++i) {
    r->dx_codeword[i] = multiply(encoded_c[i],rep->dx_codeword[i]);
  }

  // codeword
  r->codeword = (byte*)malloc(rep->lcodeword);
  if (!r->codeword) {
    goto failure;
  }
  memset(r->codeword,0,rep->lcodeword);
  r->lcodeword = rep->lcodeword;
  for(i = 0; i < rep->lcodeword;++i) {
    r->codeword[i] = multiply(rep->codeword[i],encoded_c[i]);
  }

  // macs
  r->mac = (bedoza_mac*)malloc(rep->lmac*sizeof(bedoza_mac));
  if (!r->mac) {
    goto failure;
  }
  memset(r->mac,0,rep->lmac*sizeof(bedoza_mac));
  r->lmac = rep->lmac;
  for(i = 0; i < r->lmac;++i) {
    if (rep->mac[i] == 0) continue;
    r->mac[i] = bedoza_mac_mul_const( rep->mac[i], encoded_c, rep->lcodeword);
    if (!r->mac[i]) {
      goto failure;
    }
  }
  
  // mac keys
  r->mac_keys_to_others = (bedoza_mac_key*)
    malloc(rep->lmac_keys_to_others*sizeof(bedoza_mac_key));
  if (!r->mac_keys_to_others) {
    goto failure;
  }
  memset(r->mac_keys_to_others,0,sizeof(bedoza_mac_key)*rep->lmac_keys_to_others);
  r->lmac_keys_to_others = rep->lmac_keys_to_others;
  for(i = 0; i < r->lmac_keys_to_others;++i) {
    r->mac_keys_to_others[i] = bedoza_mac_key_mul_const( rep->mac_keys_to_others[i], encoded_c, r->lcodeword);
    if (!r->mac_keys_to_others[i]) {
      goto failure;
    }
  }
  
  return r;
 failure:
  minimacs_rep_clean_up(&r);
  if (encoded_c) { 
    encoded_c = (byte*)0;
    free(encoded_c);
  }
  return 0;
  
}

/*! Sloooow 
MiniMacsRep minimacs_rep_mul_const(MiniMacsRep rep, byte * c, uint lc) {

  if (!rep) return 0;

  if (!c) return 0;

  if (lc > rep->lval) return 0;

  MATRIX * encoder = minimacs_generate_encoder(rep->lval, rep->lcodeword);
  if (!encoder) return 0;


  return minimacs_rep_mul_const_fast(encoder,rep,c,lc);
}
*/
bool minimacs_rep_is_public_const( MiniMacsRep rep)  {
    if (!rep) {
      return 1;
    }
    
    return (rep->dx_codeword == NULL);
}


bool minimacs_rearrange(MiniMacsRep *** reps_out, ...) {
  
  va_list lst = {0};
  MiniMacsRep * rep = 0;
  uint count = 0;
  uint nplayers = 0;
  uint nreps = 0;
  uint j = 0;
  
  // count number of arguments and determine number of players
  va_start(lst,reps_out);
  rep = va_arg(lst,MiniMacsRep*);
  while(rep) {
    ++count;
    nplayers = rep[0]->lmac;
    rep = va_arg(lst,MiniMacsRep*);
  }
  va_end(lst);
  nreps = count;
  
  // if no arguments were given there is nothing todo
  if (nreps <= 0) {
    return 1;
  }

  *reps_out = (MiniMacsRep**)malloc(nplayers*sizeof(MiniMacsRep**));
  if (!*reps_out) {
    return 0;
  }
  memset(*reps_out,0,nplayers*sizeof(MiniMacsRep**));

  for(j = 0;j < nplayers;++j) {
    (*reps_out)[j] = (MiniMacsRep*)malloc(nreps*sizeof(MiniMacsRep));
    if (!(*reps_out)[j]) {
      goto failure;
    }
    memset((*reps_out)[j],0,nreps*sizeof(MiniMacsRep));
  }
  
  // rearrange
  va_start(lst,reps_out);
  rep = va_arg(lst,MiniMacsRep*);
  count = 0;
  while(rep) {
    for(j = 0;j<nplayers;++j) {
      (*reps_out)[j][count]
	= rep[j];
    }
    ++count;
    rep = va_arg(lst,MiniMacsRep*);
  }
  va_end(lst);

  return 1;
 failure:
  if (*reps_out) {
    for(count =0;count<nplayers;++count) {
      for(j = 0;j < nreps;++j ) {
	minimacs_rep_clean_up( &(*reps_out)[count][j] );
      }
      free((*reps_out)[count]);(*reps_out)[count]=0;
    }
    free(*reps_out);*reps_out = 0;
  }
  return 0;
}


MiniMacsRep minimacs_rep_load( byte * data, uint ldata) {
  DerRC rc = DER_OK;

  byte * tmp = 0;
  uint ltmp = 0;
  uint otmp = 0;
  uint len = 0;
  bedoza_mac tmp_mac = 0;
  uint i = 0;

  byte * macs = 0;
  uint lmacs = 0;
  uint omacs = 0;
  
  MiniMacsRep res = (MiniMacsRep)malloc(sizeof(*res));
  if (!res) return 0;
  memset(res, 0, sizeof(*res));

  // lval
  rc = der_decode_seq( data,ldata,0,&otmp, &tmp,&ltmp);
  if (rc != DER_OK) return 0;

  otmp = 0;
  rc = der_decode_integer(&res->lval,tmp, &otmp, ltmp);
  if (rc != DER_OK) return 0;


  // dx codeword
  otmp = 0;
  rc = der_decode_seq( data,ldata, 1 , &otmp, &tmp, &ltmp );
  if (rc != DER_OK) return  0;

  otmp = 0;
  rc = der_decode_octetstring(0,&len,tmp,&otmp,ltmp);
  if (rc != DER_OK) return 0;
  res->dx_codeword = (byte*)malloc(len);
  if (!res->dx_codeword) return 0;
  memset(res->dx_codeword,0,len);otmp=0;
  rc = der_decode_octetstring(res->dx_codeword,&len,tmp,&otmp,ltmp);
  if (rc != DER_OK) return 0;
  res->ldx_codeword = len;

  // codeword
  otmp = 0;
  rc = der_decode_seq( data, ldata, 2, &otmp, &tmp, &ltmp );
  if (rc != DER_OK) return 0;
  otmp = 0;len=0;
  rc = der_decode_octetstring(0, &len,tmp,&otmp,ltmp);
  if (rc != DER_OK) return 0;
  res->codeword = (byte*)malloc(len);
  if (!res->codeword) return 0;
  memset(res->codeword,0,len);otmp = 0;
  rc = der_decode_octetstring( res->codeword, &len, tmp, &otmp, ltmp );
  if (rc != DER_OK) goto failure;
  res->lcodeword = len;
  
  // mac
  otmp = 0;
  rc = der_decode_seq( data, ldata, 3, &omacs, &macs, &lmacs );
  if (rc != DER_OK) goto failure;

  otmp = 0;
  rc = der_decode_seq( macs, lmacs, 0, &otmp, &tmp, &ltmp );
  if (rc != DER_OK) goto failure;

  otmp = 0;
  rc = der_decode_integer( &res->lmac, tmp, &otmp, ltmp );
  if (rc != DER_OK) goto failure;
  res->mac = (bedoza_mac*)malloc(sizeof(*(res->mac))*(res->lmac+1));
  if (!res->mac) goto failure;
  memset(res->mac, 0, sizeof(*(res->mac))*(res->lmac+1));
  

  for(i = 0;i<res->lmac-1;++i) {

    otmp = 0;
    rc = der_decode_seq( macs, lmacs, 1+i, &otmp, &tmp, &ltmp );
    if (rc != DER_OK) {
      goto failure;
    }

    rc = bedoza_mac_load( tmp, ltmp, &tmp_mac );
    if (rc != DER_OK) goto failure;

    res->mac[tmp_mac->fromid] = tmp_mac;
    
  }

  otmp = 0;
  rc = der_decode_seq(data, ldata, 4, &otmp, &tmp, &ltmp );
  if (rc != DER_OK) goto failure;

  rc = bedoza_mac_keys_load( tmp, ltmp, & res->mac_keys_to_others, 
			     &res->lmac_keys_to_others );
  if (rc != DER_OK) goto failure;
  
  
  return res;
 failure:
  minimacs_rep_clean_up( &res );
  return 0;
}
#define _DG(FN,...) {				\
  rc = FN(__VA_ARGS__);				\
  if (rc != DER_OK) goto failure;}

#define _DR(FN,...) {				\
  rc = FN(__VA_ARGS__);				\
  if (rc != DER_OK) return rc;}

DerRC write_macs( DerCtx * c, bedoza_mac * macs, uint lmacs ) {

  DerRC rc = DER_OK;
  uint i = 0;

  _DR(der_begin_seq,&c);
  _DR(der_insert_uint,c,lmacs);
  for(i = 0;i<lmacs;++i) {
    if (macs[i] == 0) continue;
    _DR(der_begin_seq, &c);
    _DR(der_insert_uint,c,macs[i]->mid);
    _DR(der_insert_uint,c,macs[i]->toid);
    _DR(der_insert_uint,c,macs[i]->fromid);
    _DR(der_insert_octetstring,c,macs[i]->mac,macs[i]->lmac);
    _DR(der_end_seq, &c);
  }
  _DR(der_end_seq,&c);

  return rc;
}

DerRC write_keys( DerCtx * c, bedoza_mac_key * keys, uint lkeys) {

  DerRC rc = DER_OK;
  uint i = 0;

  _DR(der_begin_seq,&c);
  _DR(der_insert_uint,c,lkeys);
  for(i = 0;i < lkeys;++i) {
    _DR(der_begin_seq, &c );
    _DR(der_insert_uint,c,keys[i]->mid);
    _DR(der_insert_uint,c,keys[i]->toid);
    _DR(der_insert_uint,c,keys[i]->fromid);
    _DR(der_insert_octetstring,c,keys[i]->alpha, keys[i]->lalpha);
    _DR(der_insert_octetstring,c,keys[i]->beta, keys[i]->lbeta );
    _DR(der_end_seq,&c);
  }
  _DR(der_end_seq,&c);

  return DER_OK;
}


DerRC write_rep( DerCtx * c, MiniMacsRep rep ) {
  
  DerRC rc = DER_OK;

  if (!c) return DER_ARG;

  if (!rep) return DER_ARG;

  _DR(der_begin_seq, &c );
  _DR(der_insert_uint, c, rep->lval );
  _DR(der_insert_octetstring,c,rep->dx_codeword, rep->ldx_codeword);
  _DR(der_insert_octetstring,c,rep->codeword, rep->lcodeword );
  _DR(write_macs,c,rep->mac,rep->lmac);
  _DR(write_keys,c,rep->mac_keys_to_others,rep->lmac_keys_to_others);
  _DR(der_end_seq,&c);

  return rc;
}


/*
 REP ::= SEQUENCE (
  lval         : INTEGER
  dx_codeword  : OCTET STRING
  codeword     : OCTET STRING
  lmac         : INTEGER
  mac0,...,macN: SEQUENCE
  lmackey,...  : SEQUENCE
 )

Deprecated needs rewrite, minimacs_rep_load is not compatible with
this approach.

 */
uint minimacs_rep_store(MiniMacsRep rep, byte * data) {

  DerCtx * c= 0;
  DerRC rc = DER_OK;
  uint ldata = 0;

  _DR(der_begin, &c );

  _DR(write_rep, c, rep );

  _DR(der_final, &c, 0, &ldata);
  _DR(der_final, &c, data, &ldata );
  
  return ldata;
}



void load_shares( const char * filename, 
		  MiniMacsTripleRep ** triples, uint * ltriples,
		  MiniMacsRep ** singles, uint * lsingles,
		  MiniMacsRep *** pairs, uint * lpairs )
{
  DerRC rc = DER_OK;
  byte * _player=0,* _nplayers=0,*_ltriples=0,*_lsingles=0,*_lpairs=0;
  uint _lplayer=0, _lnplayers=0, _lltriples=0, _llsingles=0, _llpairs=0;
  int player = 0;
  int nplayers = 0;
  FILE * file = 0;
  uint lbuf = 0;
  byte * buf = 0;
  uint off = 0;
  uint i = 0;
  uint seq_idx = 0;

  byte * ptriples;
  uint lptriples;

  byte * psingles = 0;
  uint lpsingles = 0;

  byte * ppairs = 0;
  uint lppairs = 0;

  printf("Loading ... \n");

  lbuf = read_file_size(filename);
  if (!lbuf) return;  

  buf = (byte*)malloc(lbuf);
  if (!buf) return;
  memset(buf,0,lbuf);

  lbuf = read_entire_file(filename,buf,lbuf);
  if (!lbuf) return;
  
  off = 0;
  rc = der_decode_seq(  buf, lbuf, 0, &off, &_player, &_lplayer );
  if (rc != DER_OK) return;
  
  off = 0;
  rc = der_decode_integer(&player,_player,&off,_lplayer);
  if (rc != DER_OK) return;
  
  off = 0;
  rc = der_decode_seq( buf, lbuf, 1, &off, &_nplayers, &_lnplayers );
  if (rc != DER_OK) return;

  off = 0;
  rc = der_decode_integer(&nplayers,_nplayers,&off,_lnplayers);
  if (rc != DER_OK) return;

  off = 0;
  rc = der_decode_seq( buf, lbuf, 2, &off, &_ltriples, &_lltriples );
  if (rc != DER_OK) return;

  off = 0;
  rc = der_decode_integer((int*)ltriples, _ltriples, &off, _lltriples );
  if (rc != DER_OK) return;

  rc = der_decode_seq( buf, lbuf, 3, 0, &_lsingles, &_llsingles);
  if (rc != DER_OK) return;

  off = 0;
  rc = der_decode_integer((int*)lsingles, _lsingles, &off, _llsingles );
  if (rc != DER_OK) return;

  rc = der_decode_seq(buf, lbuf, 4, 0, &_lpairs, &_llpairs );
  if (rc != DER_OK) return;

  off = 0;
  rc = der_decode_integer( (int*)lpairs, _lpairs, &off, _llpairs );
  if (rc != DER_OK) return;
  
  rc = der_decode_seq(buf,lbuf,5,0,&ptriples, &lptriples );
  if (rc != DER_OK) goto failure;

  *triples = (MiniMacsTripleRep*)malloc(*ltriples*sizeof(**triples));
  if (!*triples) {
    return;
  }
  memset(*triples,0,sizeof(**triples) * (*ltriples));
  
  for(i = 0;i < *ltriples;++i) {
    uint lprep = 0;
    byte * prep = 0;

    uint lrep = 0;
    byte * rep = 0;


    (*triples)[i] = (MiniMacsTripleRep)malloc(sizeof( *((*triples)[i]) )  );
    if (!(*triples)[i]) return;
    memset( (*triples)[i],0,sizeof(MiniMacsTripleRep));
   
    rc = der_decode_seq(ptriples,lptriples,i,0,&prep,&lprep);
    if (rc != DER_OK) goto failure;

    // a
    rc = der_decode_seq(prep,lprep,0,0,&rep, &lrep );
    if (rc != DER_OK) goto failure;

    (*triples)[i]->a = minimacs_rep_load( rep, lrep );
    if (!(*triples)[i]->a) goto failure;

    // b
    rc = der_decode_seq(prep,lprep,1,0,&rep, &lrep );
    if (rc != DER_OK) goto failure;

    (*triples)[i]->b = minimacs_rep_load( rep ,lrep);
    if (!(*triples)[i]->b) goto failure;

    // cstar
    rc = der_decode_seq(prep,lprep,2,0,&rep,&lrep);
    if (rc != DER_OK) goto failure;

    (*triples)[i]->cstar = minimacs_rep_load( rep, lrep );
    if (rc != DER_OK) goto failure;
  }

  rc = der_decode_seq( buf, lbuf, 6, 0, &psingles, &lpsingles );
  if (rc != DER_OK) goto failure;

  *singles = (MiniMacsRep*)malloc(*lsingles * sizeof(**singles));
  if (!*singles) return;
  memset(*singles, 0, sizeof(**singles)* (*lsingles));

  for(i = 0; i < *lsingles;++i) {
    uint lprep = 0;
    byte * prep = 0;

    rc = der_decode_seq(psingles, lpsingles, i,0,&prep, &lprep );
    if (rc != DER_OK) goto failure;

    (*singles)[i] = minimacs_rep_load(prep,lprep);
    if ( !(*singles)[i]) goto failure;
  }

  rc = der_decode_seq( buf, lbuf, 7, 0, &ppairs, &lppairs);
  if (rc != DER_OK) goto failure;

  *pairs = (MiniMacsRep**)malloc( sizeof(**pairs) * *lpairs);
  if (!*pairs) goto failure;
  memset(*pairs,0,*lpairs*sizeof(**pairs));

  for(i = 0;i < *lpairs;++i) {
    uint lprep=0,lprep1=0;
    byte * prep=0,* prep1=0;

    rc = der_decode_seq(ppairs, lppairs, i,0,&prep,&lprep );
    if (rc != DER_OK) goto failure;

    rc = der_decode_seq( prep, lprep, 0, 0, &prep1, &lprep1 );
    if (rc != DER_OK) goto failure;

    (*pairs)[i] = (MiniMacsRep*)malloc(sizeof(MiniMacsRep)*2);
    if ( !(*pairs)[i] ) goto failure;

    (*pairs)[i][0] = minimacs_rep_load( prep1, lprep1 );
    if ( !(*pairs)[i][0]  ) goto failure;

    rc = der_decode_seq( prep, lprep,1,0,&prep1, &lprep1 );
    if (rc != DER_OK) goto failure;

    (*pairs)[i][1] = minimacs_rep_load( prep1, lprep1 );
    if ( !(*pairs)[i][1]  ) goto failure;
  }
  return;
 failure:
  printf("Load failed\n");
  return;
}




DerRC write_triple( DerCtx * c, MiniMacsTripleRep t ) {
  DerRC rc = DER_OK;
  uint ldata = 0;
  byte * data = 0;

  _DG(der_begin_seq,&c);
  _DG(write_rep, c, t->a);
  _DG(write_rep, c, t->b);
  _DG(write_rep, c, t->cstar);
  _DG(der_end_seq, &c);

 failure:
  return rc;
}


// TODO(rwl): Figure out how we do this. We need a filename for each
// player.
void save_shares(char * postfix, 
                 uint nplayers,
                  MiniMacsTripleRep ** triples, uint ltriples,
                  MiniMacsRep ** singles, uint lsingles,
                  MiniMacsRep *** pairs, uint lpairs ) {
  uint i = 0;
  uint player = 0;
  DerCtx * c = 0;
  FILE * file = 0;
  DerRC rc = DER_OK;

  for(player=0;player<nplayers;++player) {
    char filename[64] = {0};
    byte * buf = 0;
    uint lbuf = 0;
    uint ltext = singles[player][0]->lval;
    uint lcode = singles[player][0]->lcodeword;
    
    sprintf(filename,"minimacs_%u_%u_%u_%u_%s.rep",ltext,lcode,nplayers,player,postfix);
    _DG( der_begin, &c );
    _DG( der_begin_seq, &c );

    // player id
    _DG( der_insert_uint, c, player );
    _DG( der_insert_uint, c, nplayers );
    _DG( der_insert_uint, c, ltriples );
    _DG( der_insert_uint, c, lsingles );
    _DG( der_insert_uint, c, lpairs );

    // triples
    _DG( der_begin_seq, &c);
    for(i = 0; i < ltriples; ++i) {
      _DG( write_triple, c, triples[player][i] );
    }
    _DG( der_end_seq, &c );

    // singles
    _DG( der_begin_seq, &c );
    for(i = 0;i < lsingles;++i) {
      _DG( write_rep, c, singles[player][i] );
    }
    _DG( der_end_seq, &c );

    // pairs
    _DG( der_begin_seq, &c );
    for(i = 0; i < lpairs; ++i) {
      _DG( der_begin_seq, &c );
      _DG( write_rep,c, pairs[player][i][0]);
      _DG( write_rep,c, pairs[player][i][1]);
      _DG( der_end_seq, &c );
    }
    _DG( der_end_seq, &c );

    _DG( der_end_seq, &c );

    _DG( der_final, &c, buf, &lbuf );
    buf=(byte*)malloc(lbuf);
    if (!buf) {
      rc = DER_MEM;
      goto failure;
    }
    memset(buf, 0, lbuf );
    _DG(der_final, &c, buf, &lbuf );

    file = fopen(filename,"wb");
    if (!file) {
      rc = DER_ARG;
      goto failure;
    }

    fwrite(buf,1,lbuf,file);
    fclose(file);

    if (buf) {
      free(buf);buf=0;
    }
  }

    
  failure:
    der_ctx_free( &c );
    //    return rc;
}

uint minimacs_rep_no_players(MiniMacsRep rep) {
  if (!rep ) return 0;
  return rep->lmac;
}

uint minimacs_rep_whoami(MiniMacsRep rep) {
  uint i = 0;
  for(i = 0;i<rep->lmac;++i) {
    if (rep->mac[i] == 0) return i;
  }
  return 0;
}
