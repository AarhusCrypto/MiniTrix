#include <math/fft.h>
#include <stdio.h>
#include <math/matrix.h>
#include <osal.h>

static byte omega = 3;

static 
void print_pol(byte * a) {
  uint i = 0;
  for(i = 0;i < 256;++i) {
    if (i > 0 && i % 16 == 0) { printf("\n"); }
    printf("%04x ",a[i]);
  }
}

 MATRIX * build_matrix(uint h, uint w) {
    MATRIX * m = new_matrix(h,w);
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
  
static 
void pr(char * s, byte * r) {
  uint i = 0;
  printf("%s:\n",s);
  for(i = 0;i<255;++i) {
    if (i > 0 && i % 32 == 0) printf("\n");
    printf("%3u ",r[i]);
  }
  printf("\n");
}

static void matrix_fft(uint N, byte w, byte * f, byte * r) {
  MATRIX * V = 0, * Fvec = 0, * R = 0;
  uint i = 0;
  byte * rr = 0;
  Fvec = new_matrix(N,1);
  V = build_nth_matrix(N,N,w);
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

static int test(uint p, uint q,byte * f) {
  byte r[256] = {0}, mr[256] = {0};
  uint n = p*q;
  efft(p,q,f,r);
  matrix_fft(n,omega,f,mr);
  return memcmp(r,mr,n) == 0;
}

static int test2(uint p, uint q, byte * f) {
  byte r[256] = {0}, mr[256] = {0};
  byte ir[256] = {0};
  uint n = p*q, i = 0;
  //  pr("f",f);
  efft(p,q,f,r);
  // pr("r",r);
  efftinv2(p,q,r,ir);
  /*
  for(i = 0;i < 255/2;++i){
    ir[i] ^= ir[254-i]; // r[i] = a^b
    ir[254-i] ^= ir[i]; // r[255-i] = b^a^b = a
    ir[i] = ir[i] ^ ir[254-i]; // a^b ^ a = b
  }
  */
  // pr("ir",ir);

  return memcmp(ir,f,p*q) == 0;
}

static void test_all_bits(uint p, uint q) {
  uint v=0,i=0,k=0;
  for(i = 1;i < 256;++i) {
    for (v = 0;v < 256;++v) {
      byte f[256] = {1,2,3,225};
      memset(f, v,p*q);
      printf("Test %u\t",i);
      if (test2(p,q,f)) { printf("[ok]"); } 
      else { ++k;printf("[fail]"); }
      printf("\n");
    }
  }
  printf("\nFailed for %u positions.\n",k);
}

int main(int c, char ** a) {
  OE oe = OperatingEnvironment_LinuxNew();
  byte f[256] = {0};
  byte n = 0;
  byte r[512] = {0}, mr[512] = {0}, ir[512]={0}, im[512]={0};
  uint idx = 0;
  uint k=0,p=0,q=0,i=0,j=0,v=0;
  byte imr[256] = {0};
  MATRIX *V=0,*iV=0;


  init_polynomial();
  if (c < 3) {
    printf("Needs argument\n");
    return -1;
  }
  

  q = atoi(a[1]);
  p = atoi(a[2]);
  n = p*q;
  
  f[2] = 1;
  efft(p,q,f,r);
  efftinv2(p,q,r,ir);
  matrix_fft(p*q,omega,f,mr);
  matrix_fft(p*q,inverse(omega),mr,im);

  pr("r",r);
  pr("ir",ir);
  pr("mr",mr);
  pr("im", im);

  test2(p,q,f);

  OperatingEnvironment_LinuxDestroy( &oe );
  return 0;
}
