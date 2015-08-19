#include <reedsolomon/reedsolomon.h>
#include <reedsolomon/minimacs_enc_fft.h>
#include <math/fft.h>
#include <coov4.h>
#include <stats.h>
static void ex(byte * s, byte * o, uint l) {
  uint i = 0;
  for(i = 0;i < l;++i) {
    if (i % 3 == 0) {
      o[i/3] = s[ (198*i) % 255];
    }
  }
}

typedef struct _fft_minimacs_enc_ {
  OE oe;
  // P for an Nth root of unity such that N=PQ
  uint P;
  // Q for an Nth root of unity such that N=PQ
  uint Q;
  // q for an nth root of unity such that n=Pq
  uint q;
  // nth root of unity 
  byte nroot;
  // qth root of unity 
  byte qroot;
  // Nth root of unity
  byte Nroot;
  // Qth root of unity
  byte Qroot;
  // Shur transform generator
  MATRIX * big_enc;
} * FFTMiniMacsEnc;


static 
void pr(char * s, byte * r, uint lr) {
  uint i = 0;
  printf("%s:\n",s);
  for(i = 0;i<lr;++i) {
    if (i > 0 && i % 8 == 0) printf("\n");
    printf("%u=%3u ",i,r[i]);
  }
  printf("\n");
}



static void swap(byte * s, uint i, uint j) {
  byte tmp = s[i];
  s[i] = s[j];
  s[j] = tmp;
}

/*!
 *
 * The FFT-form is where entry clear text byte {o[i]} is stored
 * {s[(198*i)%255]} because of the way of the Fast fourier transform
 * works.
 *
 * The outout {o} will have the same codeword however now in front
 * form where the clear text bytes are the first k bytes.
 *
 * {s} must have length 255
 */
static void rearrange_to_fft_form( byte * s ) {
  uint i = 0;
  byte tmp[255] = {0};
  
  for(i = 0;i < 85;++i) {
    uint r = 0;
    uint idx = (198*i)%255;
    for(r = 0;r < 3;++r) {
      tmp[idx+r] = s[i+r*85];
    }
  }
  mcpy(s,tmp,255);
}


/*!
 *
 * The front form is where the clear text bytes are stored in the
 * first {k} positions. 
p j
 *
 * After invocation {o} will have {s} rearranged such that the first
 * {k} bytes new are placed in FFT-form.
 */
static void rearrange_to_front_form(byte * s) {
  uint i = 0, k =0;
  byte tmp[255] = {0};

  tmp[0] = s[0];

  for(i =0; i < 85;++i) {
    uint r = 0;
    uint idx = (198*i)%255;
    for(r = 0;r<3;++r) {
      tmp[i+r*85] = s[idx+r];
    }
  }
  
  mcpy(s,tmp,255);
}


static MATRIX * load(OE oe, byte * s, uint ls) {
  uint i = 0;
  MATRIX * r = new_matrix(oe, ls,1);
  for(i = 0;i < ls;++i) matrix_setentry(r,i,0,s[i]);
  return r;
}

MATRIX * build_119_encoder(OE oe) {
  byte nroot = 0x03;
  MATRIX * m = 0, * small = 0, * res = 0;
  uint i=0,j=0;
  byte omega1 = 1, omega2 = nroot;
  byte val1 = 1, val2 = 1;
  m = new_matrix(oe,119,119);
  for(i = 0;i<119;++i) { // row
    val1 = 1;val2 = 1;
    for(j =0; j < 119;++j) { // column
      matrix_setentry(m,i,j,val1);
      val1 = multiply(omega1,val1);
    }
    omega1 = pol_pow(nroot,(i+1)*198);
  }

  small = LUInverse(m);
  printf("Small:\n");
  print_matrix(small);
  return small;

}

static
MATRIX * build_big_encoder(OE oe) {
  byte nroot = 0x03;
  MATRIX * m = 0, * small = 0, * res = 0;
  uint i=0,j=0;
  byte omega1 = 1, omega2 = nroot;
  byte val1 = 1, val2 = 1;
  m = new_matrix(oe,170,170);
  for(i = 0;i<85;++i) { // row
    val1 = 1;val2 = 1;
    for(j =0; j < 170;++j) { // column
      matrix_setentry(m,i,j,val1);
      matrix_setentry(m,85+i,j, val2);
      val1 = multiply(omega1,val1);
      val2 = multiply(omega2,val2);
    }
    omega1 = pol_pow(nroot,(i+1)*198);
    omega2 = multiply(nroot,omega1);
  }

  small = LUInverse(m);
//  printf("Small:\n");
//  print_matrix(small);
  return small;
  /*
  destroy_matrix(m);
  m = new_matrix(255,170); 
  omega1 = 1;
  for(i = 0;i < 255;++i) {
    val1 = 1;
    for(j = 0;j < 170;++j) {
      matrix_setentry(m,i,j,val1);
      val1 = multiply(omega1,val1);
    }
    omega1 = multiply(nroot,omega1);
  }
  
  res = matrix_multiplication(m,small);
  printf("RES:\n");
  print_matrix(res);
  
  res = m;
  return res;
  */
}


COO_DEF(MiniMacsEnc, byte *, encode, byte * msg, uint lmsg)
  FFTMiniMacsEnc fftmme = (FFTMiniMacsEnc)this->impl;
  OE oe = fftmme->oe;
  uint N = fftmme->Q * fftmme->P, i = 0;
  uint n = fftmme->q * fftmme->P;
  byte * r = 0, * k = 0;
  byte f[512] = {0};
  byte temp[512] = {0};



  if (lmsg > N) { 
    oe->p("Error in FFTMiniMacsEnc encode, lmsg is too long for the transform.");
    return 0;
  }

  r=oe->getmem(N);
  if (!r) goto failure;

  if (lmsg == 2*n) {
    /* The big_enc matrix is constructed wrongly. The first 85 entries
     * must be every 198 mod 255 rows of the Vandermonde matrix with
     * 255th root of unity as generator. E.g. the first 85 rows goes:
     *
     * \left[ w_{255}^{198*i*j} \right] 
     *
     * The remaining rows can be arbitrary.
     *
     */
    MATRIX * res = 0;
    MATRIX * vec = load(oe, msg,lmsg);
    byte a[255] = {0};
    byte tmp[512] = {0};
    CHECK_POINT_S("[RSCODE] Encoding FFT Schur Transform");
    //    oe->p("Encoding in the Schur transform, inefficiently");
    res = matrix_multiplication(fftmme->big_enc, vec);
    k = matrix_to_flatmem(res);
    mcpy(tmp,k,lmsg);
    composite_fft2(fftmme->P, fftmme->Q, fftmme->Qroot, fftmme->Nroot, tmp, r);

    rearrange_to_front_form(r);
    free(k);
    destroy_matrix(res);
    destroy_matrix(vec);
    CHECK_POINT_E("[RSCODE] Encoding FFT Schur Transform");
    return r;
  }


  CHECK_POINT_S("[RSCODE] Encoding FFT");
  mcpy(f,msg,n);
  //  pr("f",f,n);

  //17,15,176,209,0x04 * 85, 000...
  composite_fft2(fftmme->P, fftmme->q, 
                 inverse(fftmme->qroot), inverse(fftmme->nroot), 
                 f, temp);

  //  pr("Temp",temp,255);

  composite_fft2(fftmme->P, fftmme->Q, fftmme->Qroot, fftmme->Nroot, temp, r); 

  //  pr("r before arrange:", r, 255);

  rearrange_to_front_form(r);

  //  pr("r after arrange: ", r, 255);
  CHECK_POINT_E("[RSCODE] Encoding FFT");
  return r;
 failure:
  oe->putmem(r);
  return 0;
 } 

/*
 * if a == b return true else false
 */
static bool identical(polynomial * a, polynomial *b, uint la) {
  uint i = la;
  if (!a && b) { 
    return 0;
  }
  if (a && !b) { 
    return 0;
  }
  if (!a && !b) { 
    return 1;
  }
  if (la == 0) {
    return 1;
  }

  while(i--) { 
    if (a[i] != b[i]) {
      return 0; 
    }
  }
  return 1;
}


COO_DEF(MiniMacsEnc, bool, validate, byte * code, uint lmsg)
  FFTMiniMacsEnc fftmme = (FFTMiniMacsEnc)this->impl;
  OE oe = fftmme->oe;
  uint N = fftmme->Q * fftmme->P; // degree is N-1 
  uint n = fftmme->P * fftmme->q;
  byte * f = 0, * r = 0; 
  bool res  = 0;
  byte tmp[512] = {0};
  if (lmsg > N) { 
    oe->p("Error in FFTMiniMacsEnc encode, lmsg is too long for the transform.");
    return 0;
  }

  f = oe->getmem(512);
  if (!f) return False;

  CHECK_POINT_S("[RSCODE] Validation FFT");
  mcpy(tmp, code, 255);

  rearrange_to_fft_form( tmp );

  composite_fft2(fftmme->P, fftmme->Q, inverse(fftmme->Qroot), inverse(fftmme->Nroot), tmp, f); 

  //  pr("Validate f", f, 255);

  {
    uint i = 0;
    for(i = lmsg;i < 255;++i) {
      if (f[i] != 0) { 
        oe->p("code validation check failed");
        return 0;
      }
    }
  }
  CHECK_POINT_E("[RSCODE] Validation FFT");
  return 1;
 failure:
  oe->putmem(f);
  oe->putmem(r);
  return res;
}

void   MiniMacsEnc_FFTDestroy( MiniMacsEnc * enc) {

}


COO_DEF(MiniMacsEnc,byte *, encode119, byte * data, uint ldata) {
  return data;
}}


COO_DEF(MiniMacsEnc, bool, validate119, byte * data, uint ldata) {
  return False;
}}


MiniMacsEnc MiniMacsEnc_FFT119New(OE oe) {
	MiniMacsEnc res = (MiniMacsEnc)oe->getmem(sizeof(*res));

	FFTMiniMacsEnc fftmme = (FFTMiniMacsEnc)oe->getmem(sizeof(*fftmme));

	if (!res) goto failure;

	if (!fftmme) goto failure;

	res->impl = fftmme;
	fftmme->P = 17;
	fftmme->Q = 15;
	fftmme->q = 7;
	fftmme->oe = oe;
	fftmme->nroot = smallest_nth_primitive_root_of_unity(fftmme->P*fftmme->q);
	fftmme->Nroot = smallest_nth_primitive_root_of_unity(fftmme->P*fftmme->Q);
	fftmme->qroot = pol_pow(fftmme->nroot, fftmme->P);
	fftmme->Qroot = pol_pow(fftmme->Nroot, fftmme->P);
	fftmme->big_enc = build_big_encoder(oe);

	res->encode = COO_attach(res, MiniMacsEnc_encode119);
	res->validate = COO_attach(res, MiniMacsEnc_validate119);
	return res;
failure:
	MiniMacsEnc_FFTDestroy(&res);
	return 0;

}

MiniMacsEnc MiniMacsEnc_FFTNew(OE oe) {

  MiniMacsEnc res = (MiniMacsEnc)oe->getmem(sizeof(*res));

  FFTMiniMacsEnc fftmme = (FFTMiniMacsEnc)oe->getmem(sizeof(*fftmme));
  
  if (!res) goto failure;
  
  if (!fftmme) goto failure;

  res->impl = fftmme;
  fftmme->P = 17;
  fftmme->Q = 15;
  fftmme->q = 5;
  fftmme->oe = oe;
  fftmme->nroot = smallest_nth_primitive_root_of_unity(fftmme->P*fftmme->q);
  fftmme->Nroot = smallest_nth_primitive_root_of_unity(fftmme->P*fftmme->Q);
  fftmme->qroot = pol_pow(fftmme->nroot,fftmme->P);
  fftmme->Qroot = pol_pow(fftmme->Nroot,fftmme->P);
  fftmme->big_enc = build_big_encoder(oe);

  res->encode = COO_attach(res,MiniMacsEnc_encode);
  res->validate = COO_attach(res,MiniMacsEnc_validate);
  return res;
 failure:
  MiniMacsEnc_FFTDestroy( & res );
  return 0;
}


