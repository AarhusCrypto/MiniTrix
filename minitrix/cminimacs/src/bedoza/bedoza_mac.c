#include <bedoza/bedoza_mac.h>
#include <math/polynomial.h>
#include <stdlib.h>
#include <string.h>
void bedoza_mac_key_destroy(bedoza_mac_key * p_key) {
  int i = 0;
  byte * d = 0;
  
  if (!p_key) return;

  if (!*p_key) return;

  d = (byte*)*p_key;

  if ( (*p_key)->alpha) (free((*p_key)->alpha),(*p_key)->alpha = 0);

  if ( (*p_key)->beta) (free((*p_key)->beta),(*p_key)->beta = 0);

  for(i=0;i<sizeof(bedoza_mac_key);i++) d[i]=0;

  free(*p_key);
  
  *p_key = 0;

}

bedoza_mac_key bedoza_generate_compat_key( bedoza_mac_key other,
					   msgid mid,
					   parid toid,
					   parid fromid ) {

  uint i = 0;

  if (!other) goto failure;
  
  bedoza_mac_key res = (bedoza_mac_key)malloc(sizeof(*res));
  if (!res) goto failure;
  memset(res,0,sizeof(*res));
  
  res->toid = toid;
  res->fromid = fromid;
  res->mid = mid;

  res->alpha = (byte*)malloc(other->lalpha);
  if (!res->alpha) goto failure;
  memset(res->alpha,0,other->lalpha);
  res->lalpha = other->lalpha;

  memcpy(res->alpha,other->alpha,res->lalpha);

  res->beta = (byte*)malloc(other->lbeta);
  if (!res->beta) goto failure;
  memset(res->beta,0,other->lbeta);
  res->lbeta = other->lbeta;

  for(i = 0; i < res->lbeta;++i) {
    res->beta[i] = (byte)rand();
  }

  return res;
 failure:
  bedoza_mac_key_destroy( &res );
}

bedoza_mac_key generate_bedoza_mac_key( uint security_parameter, 
					msgid mid, 
					parid toid, 
					parid fromid ) {

  int i = 0;

  bedoza_mac_key res = (bedoza_mac_key)malloc(sizeof(*res));
  if (!res) return (bedoza_mac_key)0;
  memset( (byte*)res,0,sizeof(*res));

  res->mid = mid;
  res->fromid = fromid;
  res->toid = toid;

  res->alpha = (byte*)malloc( security_parameter );
  if (!res->alpha) goto failure;
  memset(res->alpha,0,security_parameter);
  res->lalpha = security_parameter;

  res->beta = (byte*)malloc( security_parameter );
  if (!res->beta) goto failure;
  memset(res->beta,0,security_parameter);
  res->lbeta = security_parameter;

  //  srand(time(0));
  rand();rand();rand();

  for(i=0;i<security_parameter;i++) {
    res->alpha[i] = 0; //(byte)(rand() % 256);
    res->beta[i] = 0; //(byte)(rand() % 256);
  }

  return res;
 failure:
  bedoza_mac_key_destroy(&res);
  return (bedoza_mac_key)0;
}


void bedoza_mac_destroy(bedoza_mac * mac) {

  int i = 0;
  byte * d = 0; 

  if (!mac) return;

  if (!*mac) return;

  if ( (*mac)->mac ) {
    memset( (*mac)->mac,0,(*mac)->lmac);
    (free((*mac)->mac),(*mac)->mac=0);
  }

  d = (byte*)*mac;

  for(i=0;i<sizeof(bedoza_mac);i++) d[i]=0;

  free(*mac);
  
  *mac = (bedoza_mac)0;
}

static do_compute_mac(byte * alpha, byte * beta, byte * msg, byte * res, uint len) {
  int i = 0;
  
  for(i=0;i<len;i++) {
    polynomial v = multiply(alpha[i],msg[i]);
    v = add(beta[i],v);
    res[i] = v;
  }
}

bedoza_mac compute_bedoza_mac(bedoza_mac_key key, byte * plaintext, uint lplaintext) {
  
  bedoza_mac res = (bedoza_mac)malloc(sizeof(*res));
  if (!res) return 0;
  memset( (byte*)res,0,sizeof(*res));
  int i = 0;
  
  if (!res) return (bedoza_mac)0;
  
  if (!key) goto failure;

  if (key->lalpha != lplaintext) goto failure;

  if (key->lbeta != lplaintext) goto failure;

  res->fromid = key->fromid;
  res->toid = key->toid;
  res->mid = key->mid;

  res->mac = (byte*)malloc(lplaintext);
  if (!res->mac) goto failure;
  memset(res->mac,0,lplaintext);
  res->lmac = lplaintext;

  do_compute_mac(key->alpha, key->beta, plaintext, res->mac, lplaintext);
 
  return res;
 failure:
  bedoza_mac_destroy(&res);
  return 0;
}



bool bedoza_check_mac(bedoza_mac_key key, bedoza_mac mac, byte * plaintext, uint lplaintext) {

  // initialise state
  bool res = 0;

  int i = 0;

  byte * computed_mac = malloc(lplaintext);

  // check guards
  if (!computed_mac) return 0;

  if (!key) goto failure; 
  
  if (!mac) goto failure;  

  if (!plaintext) goto failure;

  if (!key->alpha) goto failure;

  if (!key->beta) goto failure;

  if (!mac->mac) goto failure;

  if (mac->lmac != lplaintext) goto failure;

  if (key->lalpha != lplaintext) goto failure;

  if (key->lbeta != lplaintext) goto failure;

  if (key->mid != mac->mid ) goto failure;

  if (key->toid != mac->toid) goto failure;

  if (key->fromid != mac->fromid) goto failure;

  // compute the mac
  do_compute_mac(key->alpha, key->beta, plaintext, computed_mac, lplaintext);

  // check they are the same
  i = lplaintext;res=1;
  while(i--) {
    res &= (computed_mac[i] == mac->mac[i]);
    if (!res) goto failure;
  }

 failure:
  if (computed_mac) (free(computed_mac),computed_mac=0);
  return res;
}

static byte compare_arrays(byte * a1, uint la1, byte * a2, uint la2) {
  
  if (la1 != la2) return 0;

  if (!(a1 || a2) && la1 == 0) return 1; // null points are the same
  
  while(la1--) if (a1[la1] != a2[la1]) return 0;

  return 1;

}

static void copy_array(byte * dst, uint ldst, byte * src, uint lsrc) {
  
  uint len = ( ldst > lsrc ? lsrc : ldst );

  if (!dst) return;
  
  if (!src) return;
  
  while (len--) {
    dst[len] = src[len];
  }
  
} 


void bedoza_mul_macs( bedoza_mac left, 
		      bedoza_mac right, 
		      bedoza_mac * result ) {
  uint i = 0;

  bedoza_mac res = 0;

  if (!result) return ;

  if (!left) return;

  if (!right) return ;

  if (left->toid != right->toid) return ;
  
  if (left->fromid != right->fromid) return ;

  if (left->lmac != right->lmac ) return ;

  // allocate
  res = (bedoza_mac)malloc(sizeof(*res));
  if (!res) return ;
  memset(res,0,sizeof(*res));
  
  // set ids
  res->mid = 0;
  res->toid = left->toid;
  res->fromid = left->fromid;
  
  // handle macs that needs to be multiplied
  res->mac = (byte*)malloc(left->lmac);
  if (!res->mac) goto failure;
  res->lmac = left->lmac;
  memset(res->mac,0,res->lmac);

  for(i = 0;i<res->lmac;++i) {
    res->mac[i] = multiply(left->mac[i], right->mac[i]);
  }

  // report output
  *result = res;
  return;
 failure:
  bedoza_mac_destroy( &res );
  return;
}

void bedoza_mul_mac_keys( bedoza_mac_key left, 
			  byte * l_ptxt,
			  bedoza_mac_key right, 
			  byte * r_ptxt,
			  bedoza_mac_key * result) {

  bedoza_mac_key r = 0;
  uint i = 0;

  if (!left) return;
  if (!right) return;
  if (!result) return;
  if (left->fromid != right->fromid) return;
  if (left->toid != right->toid) return;
  if (left->lalpha != right->lalpha) return ;
  if (left->lbeta != right->lbeta) return;

  for(i=0;i<left->lalpha;++i) {
    if (left->alpha[i] != right->alpha[i]) {
      return;
    }
  }

  r = (bedoza_mac_key)malloc(sizeof(*r));
  if (!r) return;
  memset(r,0,sizeof(*r));

  r->toid = left->toid;
  r->fromid = left->fromid;
  r->mid = 0;
  
  // compute alpha 
  r->alpha = (byte*)malloc(left->lalpha);
  if (!r->alpha) goto failure;
  r->lalpha = left->lalpha;
  memset(r->alpha,0,r->lalpha);
  for(i = 0;i<r->lalpha;++i) {
    r->alpha[i] =left->alpha[i];
  }

  // compute beta
  r->beta = (byte*)malloc(left->lbeta);
  if (!r->beta) goto failure;
  r->lbeta = left->lbeta;
  memset(r->beta,0,r->lbeta);
  for(i=0;i<r->lbeta;++i) {
    polynomial a1Ab2 = multiply(multiply(left->alpha[i],l_ptxt[i]),right->beta[i]);
    polynomial b1a2B = multiply(multiply(left->beta[i],right->alpha[i]),r_ptxt[i]);
    polynomial b1b2 = multiply(left->beta[i],right->beta[i]);
    r->beta[i] = add(a1Ab2,add(b1a2B,b1b2));
    r->beta[i] = multiply(r->beta[i],
			  inverse(left->alpha[i]));
  }
  
  *result = r;
  return ;
 failure:
  bedoza_mac_key_destroy( &r );
  return ;
}

void bedoza_and_macs( bedoza_mac left, bedoza_mac right, bedoza_mac * result) {

  bedoza_mac res = 0;
  int i = 0;

  if (!result) return;

  if (!left) goto failure;
  
  if (!right) goto failure;

  if (left->toid 
      != 
      right->toid) goto failure;

  if (left->fromid != right->fromid) goto failure;

  if (left->lmac != right->lmac) goto failure;

  res = (bedoza_mac)malloc(sizeof(*res));
  if (!res) goto failure;
  memset(res,0,sizeof(*res));

  res->mid = 0;
  res->toid = left->toid;
  res->fromid = left->fromid;

  // Handle macs needs to be the sum
  res->mac = (byte*)malloc(left->lmac);
  if (!res->mac) goto failure;
  res->lmac = left->lmac;

  for(i = 0;i < left->lmac;++i) {
    res->mac[i] = left->mac[i] ^ right->mac[i];
  }

  *result = res;
  return;
 failure:
  bedoza_mac_destroy ( & res );
  return;
}

void bedoza_add_macs( bedoza_mac left, bedoza_mac right, bedoza_mac * result ) {

  bedoza_mac res = 0;

  if (!result) return;

  if (!left) goto failure;
  
  if (!right) goto failure;

  if (left->toid 
      != 
      right->toid) goto failure;

  if (left->fromid != right->fromid) goto failure;

  if (left->lmac != right->lmac) goto failure;

  res = (bedoza_mac)malloc(sizeof(*res));
  if (!res) goto failure;
  memset(res,0,sizeof(*res));

  res->mid = 0;
  res->toid = left->toid;
  res->fromid = left->fromid;

  // Handle macs needs to be the sum
  res->mac = (byte*)malloc(left->lmac);
  if (!res->mac) goto failure;
  res->lmac = left->lmac;

  polynomial_add_vectors( res->mac, left->mac, right->mac, res->lmac);

  // Output result
  *result = res;
  return;
 failure:
  bedoza_mac_destroy ( & res );
  return;
}



void bedoza_add_mac_keys( bedoza_mac_key leftkey, 
			  bedoza_mac_key rightkey, 
			  bedoza_mac_key * result) {

  uint i = 0;

  bedoza_mac_key reskey = 0;

  if (!leftkey) goto failure;

  if (!rightkey) goto failure;

  if (leftkey->toid != rightkey->toid) goto failure;

  if (rightkey->fromid != leftkey->fromid) goto failure;

  if (!result) return;

  if (leftkey->lalpha != rightkey->lalpha) goto failure;
  
  if (leftkey->lbeta != rightkey->lbeta) goto failure;

  for(i=0;i<leftkey->lalpha;++i) {
    if (leftkey->alpha[i] != rightkey->alpha[i]) {
      goto failure;
    }
  }

  reskey = (bedoza_mac_key)malloc(sizeof(*reskey));
  if (!reskey) goto failure;
  memset(reskey, 0, sizeof(*reskey));

  // Handle alpha needs to be the same
  reskey->alpha = (byte*)malloc(leftkey->lalpha);
  if (!reskey->alpha) goto failure;
  reskey->lalpha = leftkey->lalpha;
  copy_array(reskey->alpha, reskey->lalpha, leftkey->alpha, reskey->lalpha);

  // Handle beta needs to be the sum
  reskey->beta = (byte*)malloc(leftkey->lbeta);
  if (!reskey->beta) goto failure;
  reskey->lbeta = leftkey->lbeta;
  
  polynomial_add_vectors( reskey->beta, leftkey->beta, rightkey->beta, reskey->lbeta );

  reskey->mid    = 0; // needs to be set externally 
  reskey->toid   = leftkey->toid;
  reskey->fromid = leftkey->fromid;

  *result = reskey;
  return;
 failure:
  bedoza_mac_key_destroy ( & reskey );
  return;
}

void bedoza_add_macs_and_keys( bedoza_mac left, bedoza_mac_key leftkey, 
			       bedoza_mac right, bedoza_mac_key rightkey,
			       bedoza_mac * result, bedoza_mac_key * resultkey ) {
  bedoza_mac res = 0; 
  bedoza_mac_key reskey = 0;

  if (!left) goto failure;
  
  if (!right) goto failure;

  if (!leftkey) goto failure;

  if (!rightkey) goto failure;

  if (!result) goto failure;
  
  if (!resultkey) goto failure;

  if (left->toid != right->toid) goto failure;

  if (left->fromid != right->fromid) goto failure;

  if (left->lmac != right->lmac) goto failure;

  if (leftkey->toid != left->toid) goto failure;

  if (rightkey->toid != right->toid) goto failure;

  if (leftkey->fromid != left->fromid) goto failure;

  // BeDOZa requires aplhas to be the same for addition of keys
  if (!compare_arrays(leftkey->alpha, leftkey->lalpha, 
		      rightkey->alpha, rightkey->lalpha)) 
    goto failure;
  
  res = (bedoza_mac)malloc(sizeof(*res));
  if (!res)  goto failure;
  memset( res, 0, sizeof(*res));

  reskey = (bedoza_mac_key)malloc(sizeof(*reskey));
  if (!reskey) goto failure;
  memset(reskey, 0, sizeof(*reskey));

  

  // Handle alpha needs to be the same
  reskey->alpha = (byte*)malloc(leftkey->lalpha);
  if (!reskey->alpha) goto failure;
  reskey->lalpha = leftkey->lalpha;
  copy_array(reskey->alpha, reskey->lalpha, leftkey->alpha, reskey->lalpha);

  // Handle beta needs to be the sum
  reskey->beta = (byte*)malloc(leftkey->lbeta);
  if (!reskey->beta) goto failure;
  reskey->lbeta = leftkey->lbeta;
  
  polynomial_add_vectors( reskey->beta, leftkey->beta, rightkey->beta, reskey->lbeta );

  res->mid      = reskey->mid    = 0; // needs to be set externally 
  res->toid     = reskey->toid   = left->toid;
  res->fromid   = reskey->fromid = left->fromid;

  // Handle macs needs to be the sum
  res->mac = (byte*)malloc(left->lmac);
  if (!res->mac) goto failure;
  res->lmac = left->lmac;

  polynomial_add_vectors( res->mac, left->mac, right->mac, res->lmac);

  // Output result
  *result = res;

  *resultkey = reskey;

  return;

 failure:
  bedoza_mac_destroy ( & res );
  bedoza_mac_key_destroy( & reskey );

  return;

}


bedoza_mac bedoza_mac_mul_const( bedoza_mac mac, byte * c, uint lc) {
  uint i = 0;
  bedoza_mac res = 0;
  if (!mac) {
    goto failure;
  }

  if (lc != mac->lmac) {
    goto failure;
  }

  res = (bedoza_mac)malloc(sizeof(*res));
  if (!res) {
    goto failure;
  }
  memset(res,0,sizeof(*res));

  res->mid = mac->mid;
  res->toid = mac->toid;
  res->fromid = mac->fromid;
  res->mac = (byte*)malloc(mac->lmac);
  if (!res->mac) {
    goto failure;
  }
  memset(res->mac,0,mac->lmac);
  res->lmac = mac->lmac;
  for(i = 0;i < res->lmac;++i) {
    res->mac[i] = multiply(mac->mac[i],c[i]);
  }
  return res;
 failure:
  bedoza_mac_destroy( & res );
  return 0;
}



bedoza_mac bedoza_mac_and_const( bedoza_mac mac, byte * c, uint lc) {
  uint i = 0;
  bedoza_mac res = 0;
  if (!mac) {
    goto failure;
  }

  if (lc != mac->lmac) {
    goto failure;
  }

  res = (bedoza_mac)malloc(sizeof(*res));
  if (!res) {
    goto failure;
  }
  memset(res,0,sizeof(*res));

  res->mid = mac->mid;
  res->toid = mac->toid;
  res->fromid = mac->fromid;
  res->mac = (byte*)malloc(mac->lmac);
  if (!res->mac) {
    goto failure;
  }
  memset(res->mac,0,mac->lmac);
  res->lmac = mac->lmac;
  for(i = 0;i < res->lmac;++i) {
    res->mac[i] = mac->mac[i] & c[i];
  }
  return res;
 failure:
  bedoza_mac_destroy( & res );
  return 0;
}

bedoza_mac_key bedoza_mac_key_and_const( bedoza_mac_key key, byte * c, uint lc) {
  
  uint i = 0;
  bedoza_mac_key res = 0;

  if (!key) {
    goto failure;
  }

  if (!c) {
    goto failure;
  }
  
  if (lc != key->lalpha) {
    goto failure;
  }

  if (lc != key->lbeta) {
    goto failure;
  }

  res = (bedoza_mac_key)malloc(sizeof(*res));
  if (!res) {
    goto failure;
  }
  memset(res,0,sizeof(*res));

  res->mid = key->mid;
  res->toid = key->toid;
  res->fromid = key->fromid;
  
  // alpha
  res->alpha = (byte*)malloc(key->lalpha);
  if (!res->alpha) {
    goto failure;
  }
  memset(res->alpha,0,key->lalpha);
  res->lalpha = key->lalpha;

  // beta
  res->beta = (byte*)malloc(key->lbeta);
  if (!res->beta) {
    goto failure;
  }
  memset(res->beta,0,key->lbeta);
  res->lbeta = key->lbeta;
  
  // alpha is the same
  for(i = 0;i<res->lalpha;++i) {
    res->alpha[i] = key->alpha[i];
  }
  
  // beta is and'ed by our constant
  for(i = 0;i<res->lbeta;++i) {
    res->beta[i] = key->beta[i] & c[i];
  }

  return res;
 failure:
  bedoza_mac_key_destroy( &res );
  return 0;
}



bedoza_mac_key bedoza_mac_key_mul_const( bedoza_mac_key key, byte * c, uint lc) {
  uint i = 0;
  bedoza_mac_key res = 0;

  if (!key) {
    goto failure;
  }

  if (!c) {
    goto failure;
  }
  
  if (lc != key->lalpha) {
    goto failure;
  }

  if (lc != key->lbeta) {
    goto failure;
  }

  res = (bedoza_mac_key)malloc(sizeof(*res));
  if (!res) {
    goto failure;
  }
  memset(res,0,sizeof(*res));

  res->mid = key->mid;
  res->toid = key->toid;
  res->fromid = key->fromid;
  
  // alpha
  res->alpha = (byte*)malloc(key->lalpha);
  if (!res->alpha) {
    goto failure;
  }
  memset(res->alpha,0,key->lalpha);
  res->lalpha = key->lalpha;

  // beta
  res->beta = (byte*)malloc(key->lbeta);
  if (!res->beta) {
    goto failure;
  }
  memset(res->beta,0,key->lbeta);
  res->lbeta = key->lbeta;
  
  // alpha is the same
  for(i = 0;i<res->lalpha;++i) {
    res->alpha[i] = key->alpha[i];
  }
  
  // beta is multiplied by our constant
  for(i = 0;i<res->lbeta;++i) {
    res->beta[i] = multiply(key->beta[i],c[i]);
  }

  return res;
 failure:
  bedoza_mac_key_destroy( &res );
  return 0;
}

bedoza_mac bedoza_mac_copy(bedoza_mac m) {

  bedoza_mac r = 0;
  
  if (!m) return 0;

  if (!m->mac) return 0;
  
  r = (bedoza_mac)malloc(sizeof(*r));
  if (!r) {
    return 0;
  }
  memset(r,0,sizeof(*r));

  r->mid = m->mid;
  r->toid = m->toid;
  r->fromid = m->fromid;

  r->mac = (byte*)malloc(m->lmac);
  if (!r->mac) {
    goto failure;
  }
  memset(r->mac,0,m->lmac);
  r->lmac = m->lmac;

  memcpy(r->mac, m->mac, r->lmac );

  return r;
 failure:
  bedoza_mac_destroy( &r );
  return 0;

}


bedoza_mac_key bedoza_mac_key_copy(bedoza_mac_key k) {
  
  bedoza_mac_key r = 0;

  if (!k) {
    return 0;
  }

  if (!k->alpha) {
    return 0;
  }

  if (!k->beta) {
    return 0;
  }

  r = (bedoza_mac_key)malloc(sizeof(*r));
  if (!r) {
    return 0;
  }
  memset(r,0,sizeof(*r));

  r->mid = k->mid;
  r->toid = k->toid;
  r->fromid = k->fromid;

  r->alpha = (byte*)malloc(k->lalpha);
  if (!r->alpha) {
    goto failure;
  }
  memset(r->alpha,0,k->lalpha);
  r->lalpha = k->lalpha;

  memcpy(r->alpha, k->alpha, r->lalpha );

  r->beta = (byte*)malloc(k->lbeta);
  if (!r->beta) {
    goto failure;
  }
  memset(r->beta,0,k->lbeta);
  r->lbeta = k->lbeta;

  memcpy(r->beta,k->beta,r->lbeta);

  return r;
 failure:
  bedoza_mac_key_destroy( &r );
  return 0;  
}


DerRC bedoza_mac_save( bedoza_mac mac, byte * data, uint * ldata) {
  uint off = 0;
  DerRC rc = DER_OK;
  byte tmp_int[7] = {0};
  uint ltmp_int = 6;
  uint tmp_off = 0;
  byte * tmp_mac =0;
  uint mac_enc_len = 0;

  if (!ldata) return DER_ARG;

  if (!mac) {
    rc = der_encode_integer(0,data,&off,*ldata);
    return rc;
  }
  
  if (*ldata < 2) return DER_SHORT;

  // handle mac->mid
  rc = der_encode_integer( mac->mid, tmp_int, &tmp_off, ltmp_int);
  if (rc != DER_OK) return rc;

  tmp_off = *ldata;
  rc = der_encode_seq(data,&tmp_off, &off,tmp_int, ltmp_int,0);
  if (rc != DER_OK) return rc;
  
  // toid
  tmp_off = 0;
  rc = der_encode_integer( mac->toid, tmp_int, &tmp_off, ltmp_int);
  if (rc != DER_OK) return rc;
  rc = der_upd_seq(data,&off,*ldata,tmp_int,tmp_off);
  if (rc != DER_OK) return rc;

  // fromid
  tmp_off = 0;
  rc = der_encode_integer(mac->fromid, tmp_int, &tmp_off, ltmp_int);
  if (rc != DER_OK) return rc;
  rc = der_upd_seq(data,&off,*ldata,tmp_int,tmp_off);
  if (rc != DER_OK) return rc;

  // mac
  tmp_off = 0;
  rc = der_encode_octetstring( mac->mac, mac->lmac, 0, &mac_enc_len, mac->lmac);
  if (rc != DER_OK) return rc;

  tmp_mac = (byte*)malloc(mac_enc_len);
  if (!tmp_mac) return DER_MEM;
  memset(tmp_mac,0,mac_enc_len);

  rc = der_encode_octetstring( mac->mac, mac->lmac, tmp_mac, &tmp_off, mac_enc_len);
  if (rc != DER_OK) return rc;

  rc = der_upd_seq( data, &off, *ldata, tmp_mac,mac_enc_len );
  if (rc != DER_OK) return rc;
  
 failure:
  if (tmp_mac) {
    free(tmp_mac);
    tmp_mac = 0;
  }
  *ldata = off;
  return DER_OK;
}

DerRC bedoza_mac_ld(DerCtx * c, bedoza_mac * mac_out) {
  DerRC rc = DER_OK;
  bedoza_mac m = 0;

  if (!c) return DER_ARG;
  if (!mac_out) return DER_ARG;

  m = *mac_out = malloc(sizeof(**mac_out));
  if (!*mac_out) return DER_MEM;
  memset(*mac_out, 0, sizeof(**mac_out));

  rc = der_take_uint(c, 0, &m->mid);
  if (rc != DER_OK) goto failure;
  rc = der_take_uint(c, 1, &m->toid);
  if (rc != DER_OK) goto failure;
  rc = der_take_uint(c, 2, &m->fromid);
  if (rc != DER_OK) goto failure;
  rc = der_take_octetstring(c, 3, &m->mac, &m->lmac);
 failure:
  return rc;
}

DerRC bedoza_mac_load( byte * data, uint ldata, bedoza_mac * mac_out ) {
  uint off = 0;
  DerRC rc = DER_OK;
  uint mid=0,toid=0,fromid=0,lmac=0,ltmp;
  byte * pmid = 0, * ptoid = 0, * pfromid = 0, * pmac = 0;
  byte * mac = 0;
  uint seq_off = 0;

  if (!data)  {
    return DER_ARG;
  }

  if (!mac_out) {
    return DER_ARG;
  }

  // mid
  rc = der_decode_seq( data, ldata, 0, &seq_off,&pmid, &ltmp);
  if (rc != DER_OK) return rc;
  rc = der_decode_integer(&mid, pmid, &off, ltmp);off=0;
  if (rc != DER_OK) return rc;

  // toid
  rc=der_decode_seq( data, ldata, 1, &seq_off, &ptoid, &ltmp);
  if (rc != DER_OK) return rc;
  rc = der_decode_integer(&toid, ptoid, &off, ltmp); off = 0;
  if (rc != DER_OK) return rc;

  // fromid
  rc = der_decode_seq( data,ldata,2,&seq_off,&pfromid,&ltmp );
  if (rc != DER_OK) return rc;
  rc = der_decode_integer(&fromid,pfromid, &off, ltmp); off = 0;
  if (rc != DER_OK) return rc;

  // mac
  rc = der_decode_seq( data, ldata, 3, &seq_off, &pmac, &ltmp );
  if (rc != DER_OK) return rc;
  rc = der_decode_octetstring( 0,&lmac,pmac,&off,ltmp);
  if (rc != DER_OK) return rc;
  mac = (byte*)malloc(lmac);
  if (!mac) return DER_MEM;
  memset(mac,0,lmac);off=0;
  rc = der_decode_octetstring( mac,&lmac,pmac,&off,ltmp);
  if (rc != DER_OK) return rc;

  *mac_out = (bedoza_mac)malloc(sizeof(**mac_out));
  if(!*mac_out) return DER_MEM;
  memset(*mac_out,0,sizeof(**mac_out));
  
  (*mac_out)->mid = mid;
  (*mac_out)->toid = toid;
  (*mac_out)->fromid = fromid;
  (*mac_out)->lmac = lmac;
  (*mac_out)->mac = mac;

  return DER_OK;
}


DerRC bedoza_mac_key_save( bedoza_mac_key key, byte * data, uint * ldata) {
  uint off = 0;
  DerRC rc = DER_OK;
  byte tmp_int[6] = {0};
  uint ltmp_int = 6;
  uint tmp_off = 0;
  byte * tmp = 0;
  uint ltmp = 0;
  
  if (!key) return DER_ARG;

  if (!ldata) return DER_ARG;
  
  // handle mac->mid
  rc = der_encode_integer(key->mid, tmp_int, &tmp_off, ltmp_int);
  if (rc != DER_OK) return rc;
  tmp_off = *ldata;
  rc = der_encode_seq(data,&tmp_off,&off,tmp_int, ltmp_int,0);
  if (rc != DER_OK) return rc;
  
  // toid
  tmp_off = 0;
  rc = der_encode_integer( key->toid, tmp_int, &tmp_off, ltmp_int);
  if (rc != DER_OK) return rc;
  rc = der_upd_seq(data,&off,*ldata,tmp_int,tmp_off);
  if (rc != DER_OK) return rc;

  // fromid
  tmp_off = 0;
  rc = der_encode_integer(key->fromid, tmp_int, &tmp_off, ltmp_int);
  if (rc != DER_OK) return rc;
  rc = der_upd_seq(data,&off,*ldata,tmp_int,tmp_off);
  if (rc != DER_OK) return rc;

  // alpha
  tmp_off = 0;
  rc = der_encode_octetstring( key->alpha, key->lalpha, NULL, &tmp_off, key->lalpha);
  if (rc != DER_OK) return rc;

  ltmp = tmp_off;
  tmp = (byte*)malloc(ltmp);
  if (!tmp) { 
    goto failure;
  }
  memset(tmp,0,ltmp);
  
  tmp_off = 0;
  rc = der_encode_octetstring( key->alpha, key->lalpha, tmp, &tmp_off, ltmp);
  if (rc != DER_OK) return rc;
  rc = der_upd_seq( data, &off, *ldata, tmp,tmp_off);
  if (rc != DER_OK) return rc;

  // beta
  tmp_off = 0;
  memset(tmp,0,ltmp);
  tmp_off = 0;
  rc = der_encode_octetstring( key->beta, key->lbeta, tmp, &tmp_off, ltmp);
  if (rc != DER_OK) return rc;
  rc = der_upd_seq( data, &off, *ldata, tmp,tmp_off);
  if (rc != DER_OK) return rc;
  
  *ldata = off;
 failure:
  if (tmp) { free(tmp); tmp = 0; }
  return rc;
}


DerRC bedoza_mac_key_ld( DerCtx * c, bedoza_mac_key * key_out) {
  bedoza_mac_key res = 0;

  res = *key_out = malloc(sizeof(*res));
  if (!res) return DER_MEM;
  memset(res, 0, sizeof(*res));

  der_take_uint(c,0, &res->mid);
  der_take_uint(c,1, &res->toid);
  der_take_uint(c,2, &res->fromid);
  der_take_octetstring(c, 3, &res->alpha, &res->lalpha);
  der_take_octetstring(c, 4, &res->beta, &res->lbeta);
}

DerRC bedoza_mac_key_load( byte * data, uint ldata, bedoza_mac_key * key_out) {
	bedoza_mac_key res = 0;
	uint seq_off = 0;
  uint ltmp = 0;
  byte * tmp = 0;
  byte * pmid=0,*ptoid=0,*pfromid=0;
  byte * palpha = 0;
  byte * pbeta = 0;
  byte * alpha = 0;
  byte * beta = 0;
  uint lalpha = 0;
  uint lbeta = 0;
  DerRC rc = DER_OK;
  uint off = 0;
  uint mid=0, toid=0, fromid=0;

  if (!data) {
    return DER_ARG;
  }

  if (!key_out) {
    return DER_ARG;
  }

    // mid
  rc = der_decode_seq( data, ldata, 0, &seq_off,&pmid, &ltmp);
  if (rc != DER_OK) return rc;
  rc = der_decode_integer(&mid, pmid, &off, ltmp);off=0;
  if (rc != DER_OK) return rc;

  // toid
  rc=der_decode_seq( data, ldata, 1, &seq_off, &ptoid, &ltmp);
  if (rc != DER_OK) return rc;
  rc = der_decode_integer(&toid, ptoid, &off, ltmp); off = 0;
  if (rc != DER_OK) return rc;

  // fromid
  rc = der_decode_seq( data,ldata,2,&seq_off,&pfromid,&ltmp );
  if (rc != DER_OK) return rc;
  rc = der_decode_integer(&fromid,pfromid, &off, ltmp); off = 0;
  if (rc != DER_OK) return rc;


  // alpha
  rc = der_decode_seq( data, ldata, 3, &seq_off, &palpha, &ltmp );
  if (rc != DER_OK) return rc;
  alpha = (byte*)malloc(ltmp);
  if (!alpha) { 
    rc = DER_MEM;
    goto failure;
  }
  memset(alpha,0,ltmp);lalpha=ltmp;
  rc = der_decode_octetstring(alpha,&lalpha, palpha, &seq_off, ltmp);seq_off=0;
  if (rc != DER_OK) return rc;
  
  // beta
  rc = der_decode_seq( data, ldata, 4, &seq_off, &pbeta, &ltmp );
  if (rc != DER_OK) return rc;
  beta = (byte*)malloc(ltmp);
  if (!beta) { 
    rc = DER_MEM;
    goto failure;
  }
  memset(beta,0,ltmp);lbeta = ltmp;
  rc = der_decode_octetstring( beta, &lbeta, pbeta, &seq_off, ltmp);
  if (rc != DER_OK) return rc;

  res = (bedoza_mac_key)malloc(sizeof(*res));
  if (!res) goto failure;
  memset(res,0,sizeof(*res));

  res->mid = mid;
  res->toid = toid;
  res->fromid = fromid;
  res->lalpha = lalpha;
  res->alpha = alpha;
  res->lbeta = lbeta;
  res->beta = beta;

  *key_out = res;

  return DER_OK;

 failure:
  if (palpha) { free(palpha); palpha = 0; }
  if (pbeta) { free(pbeta); pbeta = 0; }
  if (res) {
    bedoza_mac_key_destroy( &res );
  }
  return rc;
}

DerRC bedoza_mac_keys_save( bedoza_mac_key * keys,
			    uint lkeys,
			    byte * data, uint * ldata) {

  DerRC rc = DER_OK;
  uint off = 0;
  uint i = 0;
  uint size = 0;
  byte tmp_int[6] = {0};
  uint ltmp_int = sizeof(tmp_int);
  uint tmp_off = 0;
  uint siz = 0;

  if (!ldata) {
    return DER_ARG;
  }

  if (!keys) {
    return DER_ARG;
  }

  rc = der_encode_integer( lkeys, tmp_int, &tmp_off, ltmp_int);
  if (rc != DER_OK) return rc; 
  
  siz = *ldata;
  rc = der_encode_seq( data, &siz, &off, tmp_int, ltmp_int, 0);
  if (rc != DER_OK) return rc;
  
  off=0;siz=*ldata;
  for(i = 0; i < lkeys; ++i) {
    uint len = 768;
    bedoza_mac_key key = keys[i];
    // TODO(rwl): This is a hack but we are guaranteed that 
    // bedoza macs over GF2^8 are less than 728 bytes when
    // serialised.
    byte tmp_buf[768] = {0};
    if (key) {
      rc = bedoza_mac_key_save( keys[i], tmp_buf, &len);
      if (rc != DER_OK) return rc;
      
      
      rc = der_upd_seq( data , &off, siz, tmp_buf, len);
      if (rc != DER_OK) return rc;
    }
  }

  *ldata = off;

  return DER_OK;
}

DerRC bedoza_mac_keys_load( byte * data, uint ldata, 
			    bedoza_mac_key ** keys_out, 
			    uint * lkeys_out ) {

  DerRC rc = DER_OK;
  byte * ptmp;
  uint ltmp = 0;
  uint tmp_off = 0;
  uint nkeys = 0;
  uint off = 0;
  uint i = 0;

  if (!data) {
    return DER_ARG;
  }

  if (!keys_out) {
    return DER_ARG;
  }

  if (!lkeys_out) {
    return DER_ARG;
  }

  rc = der_decode_seq( data, ldata, 0, &off, &ptmp, &ltmp);
  if (rc != DER_OK) return rc;

  rc = der_decode_integer( &nkeys, ptmp, &tmp_off, ltmp );
  if (rc != DER_OK) return rc;

  *keys_out = (bedoza_mac_key*)malloc(nkeys*sizeof(**keys_out));
  if (!*keys_out) {
    return DER_MEM;
  }
  memset(*keys_out,0,sizeof(**keys_out)*nkeys);
  
  for(i = 0; i < nkeys;++i) {
    rc = der_decode_seq( data, ldata, 1+i, &off, &ptmp, &ltmp );
    if (rc != DER_OK) return rc;
    
    rc = bedoza_mac_key_load( ptmp, ltmp, & (*keys_out)[i] );
    if (rc != DER_OK) return rc;
  }

  *lkeys_out  = nkeys;

  return DER_OK;
}
