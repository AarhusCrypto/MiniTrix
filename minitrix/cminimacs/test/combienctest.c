#include <osal.h>
#include <stats.h>
#include <reedsolomon/minimacs_combi_encoder.h>


typedef struct _test_ {
  char * name;
  bool (*test)(void * arg);
} Test;



static 
MATRIX * build_e(OE oe) {
  uint i = 0, j = 0;
  byte nroot = 0x03; // 255th root of unity in GF_{2^8} with reduction pol rijndael.
  byte omega = 1;
  MATRIX * small = new_matrix(oe,120,120);
  MATRIX * big = new_matrix(oe,120,255);
  MATRIX * res = 0, * smallI = 0;

  for(i = 0;i < 255;++i) {
    byte val = 1;
    int row = ((198*i) % 255);
    printf("Row %u\n",row);
    if (row < 120) {
      for(j = 0;j < 120;++j) {
        matrix_setentry(small,row,j,val);
        val = multiply(omega,val);
      }
    }
    omega = multiply(omega,nroot);
  }

  print_matrix(small);

  //  smallI = LUInverse(small);

  /*

  omega = 1;
  for(i = 0;i < 255;++i) {
    byte val = 1;
    for(j = 0; j < 120;++j) {
      matrix_setentry(big,(i*198) % 255,j,val);
      val = multiply(omega,val);
    }
    omega = multiply(omega,nroot);
  }
  
  res = matrix_multiplication(big, smallI);

  destroy_matrix(small);
  destroy_matrix(smallI);
  destroy_matrix(big);
  */
  return res;
}

MATRIX * build_small_encoder(OE oe, uint dim) {
  byte nroot = 0x03;
  MATRIX * m = 0, * small = 0, * res = 0;
  uint i=0,j=0;
  byte omega1 = 1, omega2 = nroot;
  byte val1 = 1, val2 = 1;
  m = new_matrix(oe,dim,dim);
  for(i = 0;i<85;++i) { // row
    val1 = 1;val2 = 1;
    for(j =0; j < dim;++j) { // column
      matrix_setentry(m,i,j,val1);
      if (85+i < dim) {
        matrix_setentry(m,85+i,j, val2);
        val2 = multiply(omega2,val2);
      }
      val1 = multiply(omega1,val1);
    }
    omega1 = pol_pow(nroot,(i+1)*198);
    omega2 = multiply(nroot,omega1);
  }

  small = LUInverse(m);
  printf("Small:\n");
  print_matrix(small);
  return small;
}


/*
 * Build encoder matrix that has dimension 120 using 255th root of
 * unity such that it is checkable by FFT.
 */
static 
MATRIX * build_combi_enc_matrix(OE oe) {
  MATRIX * M = build_nth_matrix(oe, 255,120,3);
  MATRIX * m = build_nth_matrix(oe, 120,120,3);
  MATRIX * t1 =0 ,*t2 = 0;
  t1 = LUInverse(m);
  t2 = matrix_multiplication(M,t1);
  destroy_matrix(m);
  destroy_matrix(t1);
  destroy_matrix(M);
  return t2;
}

static 
void dump_redun_matrix_as_c_code(MATRIX * enc, uint ltext) {
  
  uint w,h;
  uint i,j;

  w = matrix_getwidth(enc);
  h = matrix_getheight(enc);

  printf("byte R[%u][%u] = {\n",h-ltext,w);
  for(i = ltext; i < h;++i) {
    bool f = True;
    printf("  {");
    for(j = 0;j < w; ++j) {
      if (j > 0 && j % 16 == 0) {
        printf("\n ");
      }
      if (f) {
        printf("0x%02x",matrix_getentry(enc,i,j));
        f = False;
      } else {
        printf(", 0x%02x",matrix_getentry(enc,i,j));
      }
    }
    printf("},\n");
  }
  printf("};\n");
}


static bool print_encoder_matrix_code(void * arg) {
  MATRIX * enc = build_e((OE)arg);
  /*
  printf("\n\n");
  dump_redun_matrix_as_c_code( enc, 120 );
  printf("\n\n");
  destroy_matrix(enc);
  */
  return True;
}

static bool can_create(void * arg) {
  bool result = False;
  OE oe = (OE)arg;
  MiniMacsEnc enc = MiniMacsEnc_CombiNew(oe, 120, 255);
  if (enc) result = True;
  MiniMacsEnc_CombiDestroy(&enc);
  return result;
}


static bool all_zeros(void * arg) {
  OE oe = (OE)arg;
  byte msg[120] = {0};
  MiniMacsEnc enc = MiniMacsEnc_CombiNew(oe, 120, 255);
  byte * code = 0;
  bool result = False;
  
  code = enc->encode(msg, 120);
  if (code) result = True;
  
  if (result == True) {
    result = enc->validate(code, 120);
  }

  oe->putmem(code);
  MiniMacsEnc_CombiDestroy(&enc);
  return result;
}

static bool all_ones(void * arg) {
  OE oe = (OE)arg;
  byte msg[120] = {0};
  MiniMacsEnc enc = MiniMacsEnc_CombiNew(oe, 120, 255);
  MiniMacsEnc fft = MiniMacsEnc_FFTNew(oe);
  byte * code = 0;
  bool result = False;
  uint i = 0;
  
  msg[0] = 1;
  /*
  for(i = 0; i < sizeof(msg); ++i) {
    msg[i] = 1;
  }
  */
  
  code = enc->encode(msg, 120);
  if (code) result = True;

  for(i = 0;i < 255;++i) {
    printf("%02x\n",code[i]);
    if(i == 119) printf("----\n");
  }
  
  if (result == True) {
    result = fft->validate(code, 85);
  }

  oe->putmem(code);
  MiniMacsEnc_CombiDestroy(&enc);
  return result;
}


static Test tests[] = {
  {"simple test",print_encoder_matrix_code},
  {"Can Create", can_create},
  {"Encoding all zeros",all_zeros},
  {"Encoding all ones", all_ones}
};



int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();
  uint i = 0, j = 0;
  char txt[64] = {0};
  init_polynomial();
  InitStats(oe);

  for(i = 0; i < sizeof(tests)/sizeof(Test);++i) {
    uint l = 0;
    while(tests[i].name[l]) ++l;
    for(j = 0;j < 63;++j) {
      txt[j] = ' ';
    }
    txt[63] = 0;
    sprintf(txt,"%s",tests[i].name);
    txt[l] = ' ';
    printf("[ ] Running test \"%s\" ... ",txt);
    if (tests[i].test(oe)) { 
      printf(" [ success ]\n"); 
    } else {
      printf(" [ failed ]\n");
    }
  }
  PrintMeasurements(oe);
  OperatingEnvironment_Destroy(&oe);
  teardown_polynomial();
  return 0;

}
