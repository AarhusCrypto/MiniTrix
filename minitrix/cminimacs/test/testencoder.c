#include <osal.h>
#include <reedsolomon/reedsolomon.h>
#include <reedsolomon/minimacs_enc_fft.h>
#include <math/fft.h>
#include <math/matrix.h>

static 
void pr(char * s, byte * r, uint lr) {
  uint i = 0;
  printf("%s:\n",s);
  for(i = 0;i<lr;++i) {
    if (i > 0 && i % 8 == 0) printf("\n");
    printf("%3u ",r[i]);
  }
  printf("\n");
}



static void matrix_fft(OE oe, uint N, byte w, byte * f, byte * r) {
  MATRIX * V = 0, * Fvec = 0, * R = 0;
  uint i = 0;
  byte * rr = 0;
  Fvec = new_matrix(oe, N,1);
  V = build_nth_matrix(oe,N,N,w);
  for(i = 0;i < N;++i) {
    matrix_setentry(Fvec,i,0,f[i]);
  }
  R = matrix_multiplication(V,Fvec);
  rr = matrix_to_flatmem(R);
  memcpy(r,rr,N+1);
  free(rr);
  destroy_matrix(Fvec);
  destroy_matrix(V);
  destroy_matrix(R);
}

static MATRIX * load(OE oe,byte * s, uint ls) {
  uint i = 0;
  MATRIX * r = new_matrix(oe,ls,1);
  for(i = 0;i < ls;++i) matrix_setentry(r,i,0,s[i]);
  return r;
}

static void rev(byte * s, uint ls) {
  uint i = 0;
  for( i = 0 ; i < ls/2; ++i) {
    s[i] ^= s[ls-1-i]; // a ^= b = a^b
    s[ls-1-i] ^= s[i]; // b ^= a^b = a
    s[i] ^= s[ls-1-i]; // a^b ^= a = b
  }
}
int U = 42;
static void pr_i(byte * b) {
  uint i = 0;
  for(i = 0;i < 256;++i) {
    if (i > 0 && i % 16 ==0) printf("\n");
    if (b[i] == U) 
      printf("%03d(%03u) ",i,b[i]);
    else
      printf("%03d|%03u| ",i,b[i]);
  }
}

static void extract(byte * s) {
  uint i = 0;
  uint j = 0;
  do {
    i = (i + 28) % 256;
    printf("%d %u\n",j,s[i]);
    ++j;
  }while(i);
  
}

static void ex(byte * s, byte * o, uint l) {
  uint i = 0;
  for(i = 0;i < l/3;++i) {
    o[ i ] = s[ (198*i) % 255 ];
      //      o[ (i/3) * 76 % 85 ] = s[ i ]; 
         //    }
  }
}

static void print(OE oe, byte * d, uint ld) {

  uint i = 0;
  for(i = 0; i < ld;++i) {
    if (i>0&&i%16==0) oe->print("\n");
    oe->print("%02x ",d[i]);
  }
  oe->print("\n");
}



static byte omega =3;
int main(int argc, char **args) {
  OE oe = OperatingEnvironment_New();
  MiniMacsEnc fftenc = 0, mtxenc=0 ;
  MiniMacsEnc fft119 = 0;

  byte a[256] = {0}, a_[256]={0};
  byte c[256] = {0};
  byte b[256] = {0};
  byte mr[256] = {0}, ir[256] = {0};
  MATRIX * V = 0, * iV = 0, * R = 0, * Fvec  = 0, * e, * I = 0, * _I = 0;
  uint i = 0;
  byte * ee1= 0, * ee2 = 0;
  MATRIX  * T = 0, * iV85 = 0;
  byte omega85 = 7;
  byte qroot85 = 0;
  extern int _print_matrix_dim;
  init_polynomial();
  
  /*  
  for(i = 0;i < 170;++i) {
    b[i] = ((101*i+65537) % 255) + 1;
  }
  */
  b[0] = 1;
  _print_matrix_dim = 32;

  fftenc = MiniMacsEnc_FFTNew(oe );

  ee2 = fftenc->encode(b,85);

  print(oe,ee2,255);

  if (fftenc->validate(ee2, 170)) {
    printf("Valid\n");
  } else {
    printf("inValid\n");
  }

  
}
