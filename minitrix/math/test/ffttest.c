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

MATRIX * build_matrix(OE oe,uint h, uint w) {
   MATRIX * m = new_matrix(oe,h,w);
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

static 
void matrix_fft(OE oe,uint h, uint b, byte w, byte * f, byte * r) {
  MATRIX * V = 0, * Fvec = 0, * R = 0;
  uint i = 0;
  byte * rr = 0;
  Fvec = new_matrix(oe,b,1);
  V = build_nth_matrix(oe,h,b,w);
  for(i = 0;i < b;++i) {
    matrix_setentry(Fvec,i,0,f[i]);
  }
  R = matrix_multiplication(V,Fvec);
  rr = matrix_to_flatmem(R);
  mcpy(r,rr,h);
  oe->putmem(rr);
  destroy_matrix(Fvec);
  destroy_matrix(V);
  destroy_matrix(R);
}

/*
static int test(OE oe, uint p, uint q,byte * f) {
  byte r[256] = {0}, mr[256] = {0};
  uint n = p*q;
  efft(p,q,f,r);
  matrix_fft(oe,n,omega,f,mr);
  return memcmp(r,mr,n) == 0;
}

static int test2(OE oe, uint p, uint q, byte * f) {
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
  /
  // pr("ir",ir);

  return memcmp(ir,f,p*q) == 0;
}

static void test_all_bits(OE oe, uint p, uint q) {
  uint v=0,i=0,k=0;
  for(i = 1;i < 256;++i) {
    for (v = 0;v < 256;++v) {
      byte f[256] = {1,2,3,225};
      zeromem(f, p*q);
      printf("Test %u\t",i);
      if (test2(oe,p,q,f)) { oe->p("[ok]"); } 
      else { ++k;oe->p("[fail]"); }
      printf("\n");
    }
  }
  printf("\nFailed for %u positions.\n",k);
}
*/
static void print(OE oe, byte * d, uint ld) {

  uint i = 0;
  for(i = 0; i < ld;++i) {
    if (i>0&&i%16==0) oe->print("\n");
    oe->print("%02x ",d[i]);
  }
  oe->print("\n");
}

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

int main(int c, char ** _) {
  OE oe = OperatingEnvironment_New();
  byte f[256] = {1};
  byte n = 0;
  byte r[512] = {0}, mr[512] = {0}, ir[512]={0}, im[512]={0};
  uint idx = 0;
  uint k=0,p=0,q=0,i=0,j=0;
  byte imr[256] = {0};
  MATRIX *V=0,*iV=0, * v=0, * a=0, * H = 0;
  byte root5 = smallest_nth_primitive_root_of_unity(5);
  byte rootn = smallest_nth_primitive_root_of_unity(17*5);

  init_polynomial();
  init_matrix();

  
  composite_fft2(17,5,inverse(root5),inverse(rootn),f,r);
  print(oe,r,255);
  composite_fft2(17,15,pol_pow(3,17),3,r,f);
  rearrange_to_front_form(f);
  oe->print("\n");
  print(oe,f,255);

  return 0;
  V = build_nth_matrix(oe,119,119,3);
  H = build_nth_matrix(oe,119,119,inverse(3));
  v = new_matrix(oe,119,1);
  matrix_setentry(v,0,0,5);
  matrix_setentry(v,1,0,5);
  iV = LUPInverse(V);
  a = matrix_multiplication(V,v);
  print_matrix(matrix_multiplication(iV,a));

  print_matrix(iV);
  oe->print("\n\n");
  print_matrix(H);

  OperatingEnvironment_Destroy( &oe );
  return 0;
}
