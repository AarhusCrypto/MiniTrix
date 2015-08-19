
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "reedsolomon/reedsolomon.h"
#include "math/fft.h"
#include "math/matrix.h"
#include <coov4.h>
#include <common.h>
#include "stats.h"

#ifdef _DEBUG
#undef _DEBUG
#endif
int _DEBUG = 0;
#define GOTOEXIT(msg) \
   { if (_DEBUG) { printf("%s:%d \"%s\"\n",__FILE__,__LINE__,msg); }; goto exit; }


  static void load_vector(MATRIX * vec, polynomial * m, uint lm) {
    int i = 0;
    for(i = 0;i<lm;i++) matrix_setentry(vec,i,0,m[i]);
  }
  
  MATRIX * build_matrix(OE oe, uint h, uint w) {
    
    MATRIX * m = new_matrix(oe, h,w);
    int i = 0, j = 0;
    for(i=0;i<h;i++)
      {
        polynomial val = 1;
        polynomial d = i+1;
        for(j=0;j<w;j++) {
          matrix_setentry(m,i,j,val);
          val = multiply(val,d);
        }
      }
    
    return m;
  }
  
static void set_vandermonde_row(MATRIX * m, uint row, polynomial d) {
  int j = 0,w=0;
  polynomial val = 1;
  if (!m) return;

   w = matrix_getwidth(m);

   for(j = 0; j < w;j++) {
     matrix_setentry(m,row,j,val);
     val = multiply(val,d);
   }
}

  MATRIX * encode(OE oe, polynomial * message, uint lmessage, uint lcode) {

  if (!message) return 0;
  
  if (lcode < lmessage) return 0;

  {
    int i,j;
    MATRIX * vander = 0;
    MATRIX * msg = new_matrix(oe,lmessage,1), * code = 0;

    if (!msg) GOTOEXIT("failed to initialise msg");
    
     for(i = 0;i<lmessage;i++)
      matrix_setentry(msg,i,0,message[i]);

     vander = build_matrix(oe,lcode,lmessage);
     if (!vander) GOTOEXIT("failed build vander");

     code = matrix_multiplication(vander,msg);
     if (!code) GOTOEXIT("failed to multiply vander and msg-vector");

  exit:
     destroy_matrix(vander);
     destroy_matrix(msg);
     return code;
  }
  
  return 0;
}

static bool compare(polynomial * a, polynomial *b, uint la) {
  uint i = la;
  if (!a && b) return 0;
  if (a && !b) return 0;
  if (!(a && b)) return 1;
  if (la == 0) return 1;
  while(i--) { if (a[i] != b[i]) return 0; }
  return 1;
}

  bool validcode(OE oe, polynomial * rscode, uint lrscode, polynomial * msg, uint lmsg) {
    bool result = 0;
    MATRIX * encoder= build_matrix(oe,lrscode,lmsg);
    MATRIX * vander = build_matrix(oe,lmsg,lmsg);
    MATRIX * ivander = 0;
    MATRIX * code = new_matrix(oe,lmsg,1);
    MATRIX * msgvec = new_matrix(oe,lmsg,1);
    MATRIX * expectedmsgvec = 0;
    MATRIX * expectedcodevec = 0;
    polynomial * expectedmsg = 0;
    polynomial * expectedcode = 0;
    int i = 0;

    // check inputs
    if (!rscode) GOTOEXIT("validcode rscode null");
    if (!msg) GOTOEXIT("msg is null");
    
    // check initialization
    if (!encoder) GOTOEXIT("init of encoder gave null");
    if (!vander) GOTOEXIT("init of vander gave null");
    if (!code) GOTOEXIT("init of code gave null");
    if (!msgvec) GOTOEXIT("init of msgvec gave null");
    
  // build code word matrix
    for(i=0;i<lmsg;i++) {
      matrix_setentry(code,i,0,rscode[i]);
      matrix_setentry(msgvec,i,0,msg[i]);
    }

  // invert the Vandermonde matrix
    ivander = LUInverse(vander);
    if (!ivander) GOTOEXIT("failed to invert vander");
    
    // compute expected message
    expectedmsgvec = matrix_multiplication(ivander,code);
    if (!expectedmsgvec) GOTOEXIT("failed to multiply ivander and code");

    expectedmsg = matrix_topoly(expectedmsgvec,0);
    if (!expectedmsg) GOTOEXIT("failed to convert expectedmsgvec to a flat array");

    // compute expected code word
    expectedcodevec = matrix_multiplication(encoder,msgvec);
    if (!expectedcodevec) GOTOEXIT("failed to multiply encoder and msgvec");

    expectedcode = matrix_topoly(expectedcodevec,0);
    if (!expectedcode) GOTOEXIT("failed to convetr expectedcode to a flat array");
    
    // report result
    result = compare(expectedmsg,msg,lmsg) && compare(expectedcode,rscode,lrscode);

 exit:
    // clean up
    destroy_matrix(encoder);
    destroy_matrix(vander);
    destroy_matrix(ivander);
    destroy_matrix(code);
    destroy_matrix(msgvec);
    destroy_matrix(expectedmsgvec);
    destroy_matrix(expectedcodevec);
    if (expectedmsg) oe->putmem(expectedmsg);
    if (expectedcode) oe->putmem(expectedcode);
    
    // leave
    return result;
}


  polynomial * interpol(OE oe, polynomial * points, uint lpoints) {
    
    if (!points) return 0;
    {
      MATRIX * vander = build_matrix(oe,lpoints,lpoints);
      MATRIX * ivander = 0, * msg = new_matrix(oe,lpoints,1);
      MATRIX * result = 0;
      polynomial * result_vector = 0;


      // check input
      if (!msg) GOTOEXIT("input msg is null");

      // check initialisation
      if (!vander) GOTOEXIT("init of vander failed");
    
      ivander = matrix_invert(vander);
      if (!ivander) GOTOEXIT("failed to invert vander");
      
      result = matrix_multiplication(ivander,msg);
      if (!result) GOTOEXIT("multiplication if ivander and msg failed");

      result_vector = matrix_topoly(result,0);
      if (!result_vector) GOTOEXIT("failed to convert result to a flat array");
    exit:
      destroy_matrix(vander);
      destroy_matrix(ivander);
      destroy_matrix(msg);
      destroy_matrix(result);
    
      return result_vector;
    }
}


  polynomial * minimacs_encode(OE oe, polynomial * msg, uint lmsg, uint lcode) {
    MATRIX * ABar = 0;
    polynomial * res = 0;
    
    // check inputs
    if (lmsg > lcode) GOTOEXIT("a message cannot be longer than its code");
    if (!msg) GOTOEXIT("input msg null");

  // compute ABar, the encoder matrix for lmsg and lcode
  
    ABar = minimacs_generate_encoder(oe,lmsg,lcode);
    if (!ABar) GOTOEXIT("generating encoder matrix failed");
    
    // encode code word
    res = minimacs_encode_fast(ABar,msg,lmsg);
    
  exit:
    destroy_matrix(ABar);
    return res;
}

  /*
   * Implementation: 
   *
   * Checks the code by computing what it should have been based on
   * the first lmsg bytes and compares. This is the slow approach.
   *
   */
  bool minimacs_validate(OE oe, polynomial * code, uint lcode, uint lmsg) {
    bool res = 0;
    
    polynomial * codeword = minimacs_encode(oe,code,lmsg,lcode);
    if (!codeword) GOTOEXIT("Codeword is null encoding failed");

    res = compare(code,codeword,lcode);
    
  exit:
    oe->putmem(codeword);
    return res;
  }

  /*
   * Implementation:
   *
   * Checks the code by multiplying the given {enc} encoder vander
   * monde matrix with the first {lmsg} bytes from the given code.
   *
   * Should be faster as matrix inversion and generation is omitted.
   *
   */
  bool minimacs_validate_fast(MATRIX * enc, polynomial * code, uint lcode, uint lmsg) {
    bool res = 0;
    MATRIX * msgvec = new_matrix(enc->oe,lmsg,1);
    MATRIX * codevec = 0;
    polynomial * flat = 0;

    if (!msgvec) GOTOEXIT("Could not allocate message vector");
    if (!enc) GOTOEXIT("Invalid input enc is null");
    if (!code) GOTOEXIT("Invalid input code is null");

    if (matrix_getheight(enc) != lcode) GOTOEXIT("Enc has wrong dimensions for the length of the code");
    
    if (matrix_getwidth(enc) != lmsg) GOTOEXIT("Enc has wrong dimensions for the length of the message");

    load_vector(msgvec,code,lmsg);
    
    codevec = matrix_multiplication(enc,msgvec);
    if (!codevec) GOTOEXIT("Failed to multiply enc and msgvec");

    flat = matrix_topoly(codevec,0);
    if (!flat) GOTOEXIT("Failed to convert code vector to flat array");

    res = compare(flat,code,lcode);
    
  exit:
    destroy_matrix(msgvec);
    destroy_matrix(codevec);
    if (flat) (enc->oe->putmem(flat),flat=0);
    return res;
  }


  MATRIX * minimacs_generate_encoder(OE oe, uint lmsg,uint lcode) {
    MATRIX * vander_lmsg = build_matrix(oe,lmsg,lmsg);
    MATRIX * ivander_lmsg = 0;
    MATRIX * vander_lcode = build_matrix(oe,lcode,lmsg);
    MATRIX * Abar = 0;

    //    printf("GENERATING MATRIX, this might not be the best thing to do?\n");

    // check inputs
    if (lmsg > lcode) GOTOEXIT("Message cannot be longer than its code");

    // check initialisation
    if (!vander_lmsg) GOTOEXIT("failed to initialise vander_lmsg");
    if (!vander_lcode) GOTOEXIT("failed to initialise vander_lcode");

    // invert matrix
    // ivander_lmsg = matrix_invert(vander_lmsg);
    ivander_lmsg = LUInverse(vander_lmsg);
    if (!ivander_lmsg) GOTOEXIT("failed to invert vander_lmsg");

    // compute Abar
    Abar = matrix_multiplication(vander_lcode,ivander_lmsg);
    if (!Abar) GOTOEXIT("failed to multiply vander_lcode and vander_lmsg");

  exit:
    destroy_matrix(vander_lmsg);
    destroy_matrix(vander_lcode);
    destroy_matrix(ivander_lmsg);
    return Abar;
  }
  
  polynomial * minimacs_encode_256_fft(OE oe, polynomial * msg, uint lmsg) {
    uint P = 0;
    uint Q = 0;
    byte * res = (byte*)oe->getmem(256);
    byte m[256] = {0};
    uint i = 0;

    for(i = 0;i < lmsg;++i) {
      res[i] = msg[i];
    }
    
    efftinv2(P,Q,res,m);
      
    efft(P,Q,m,res);
    
    return res;
  }
  
  polynomial * minimacs_encode_fast(MATRIX * enc, polynomial * msg, uint lmsg) {
    MATRIX * codevec = 0;
    MATRIX * msgvec = new_matrix(enc->oe,lmsg,1);
    polynomial * res = 0;
    
    // load message
    load_vector(msgvec,msg,lmsg);
    
    // compute Abar times x producing the code word
    codevec = matrix_multiplication(enc,msgvec);
    if (!codevec) GOTOEXIT("failed to multiply Abar and msgvec");
    
    // extract result
    res = matrix_topoly(codevec,0);
  exit:
    destroy_matrix(msgvec);
    destroy_matrix(codevec);
    return res;
  }

  polynomial * minimacs_encode_fft(MATRIX * enc, polynomial * msg, uint lmsg) {
    MATRIX * codevec = 0;
    MATRIX * msgvec = new_matrix(enc->oe, lmsg,1);
    polynomial * res = 0;
    
    // load message
    load_vector(msgvec,msg,lmsg);
    
    // compute Abar times x producing the code word
    //    codevec = matrix_multiplication_fft(enc,msgvec);
    if (!codevec) GOTOEXIT("failed to multiply Abar and msgvec");
    
    // extract result
    res = matrix_topoly(codevec,0);
  exit:
    destroy_matrix(msgvec);
    destroy_matrix(codevec);
    return res;
  }

  typedef struct _matrix_minimacs_enc_ {
    OE oe;
    MATRIX * encoder;
    MATRIX * big_encoder; 
  } * MatrixMiniMacsEnc;

  COO_DEF(MiniMacsEnc, byte *, encode, byte * msg, uint lmsg)
    byte * r = 0;
    MatrixMiniMacsEnc mmme = (MatrixMiniMacsEnc)this->impl;
    if (lmsg == matrix_getwidth(mmme->encoder)) {
      
      CHECK_POINT_S("[RSCODE] Encoding Matrix");
      r = minimacs_encode_fast(mmme->encoder,msg,lmsg);
      CHECK_POINT_E("[RSCODE] Encoding Matrix");
      
      return r;

    } else {
      CHECK_POINT_S("[RSCODE] Encoding Matrix Schur Transform");
      r = minimacs_encode_fast(mmme->big_encoder,msg,lmsg);
      CHECK_POINT_E("[RSCODE] Encoding Matrix Schur Transform");
      return r;
    }
  }

  COO_DEF(MiniMacsEnc, bool, validate, byte * code, uint lmsg)
    MatrixMiniMacsEnc mmme = (MatrixMiniMacsEnc)this->impl;
    bool r = 0;
    if (lmsg ==  matrix_getwidth(mmme->encoder)) {
      CHECK_POINT_S("[RSCODE] Validation Matrix");
      r = minimacs_validate_fast(mmme->encoder,code,matrix_getheight(mmme->encoder),  lmsg );
      CHECK_POINT_E("[RSCODE] Validation Matrix");
      return r;
    } else {
      CHECK_POINT_S("[RSCODE] Validation Matrix Schur Transform");
      r =  minimacs_validate_fast(mmme->big_encoder,code,matrix_getheight(mmme->big_encoder),  lmsg );
      CHECK_POINT_E("[RSCODE] Validation Matrix Schur Transform");
      return r;
    }
  }

void MiniMacsEnc_MatrixDestroy(MiniMacsEnc * mme) {
  MatrixMiniMacsEnc mmme = 0;
  if (!mme) return;
  if (!*mme) return ;

  mmme = (MatrixMiniMacsEnc)(*mme)->impl;
  if (mmme) {
    OE oe = mmme->oe;
    COO_detach( (*mme)->encode );
    COO_detach( (*mme)->validate );
    oe->putmem(*mme);
    destroy_matrix( mmme->encoder );
    destroy_matrix( mmme->big_encoder );
    zeromem(mmme, sizeof(*mmme));
    oe->putmem(mmme);
  }
  
}


MiniMacsEnc MiniMacsEnc_MatrixNew(OE oe, uint lcode, uint lmsg) {
    MiniMacsEnc enc = (MiniMacsEnc)oe->getmem(sizeof(*enc));
    MatrixMiniMacsEnc mmme = (MatrixMiniMacsEnc)oe->getmem(sizeof(*mmme));

    if (!enc) goto failure;

    if (!mmme) goto failure;

    mmme->encoder = minimacs_generate_encoder(oe,lmsg, lcode );
    mmme->big_encoder = minimacs_generate_encoder(oe,2*lmsg, lcode );
    mmme->oe = oe;
    
    enc->encode = COO_attach(enc, MiniMacsEnc_encode);
    enc->validate = COO_attach(enc, MiniMacsEnc_validate);
    enc->impl = mmme;

    return enc;
  failure:
    if (!mmme) oe->putmem(enc);enc=0;
    MiniMacsEnc_MatrixDestroy(&enc);
    return 0;
  }

#ifdef __cplusplus
}
#endif
