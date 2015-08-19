#include <reedsolomon/minimacs_bitfft_encoder.h>
#include <reedsolomon/minimacs_enc_fft.h>
#include <osal.h>
#include <stats.h>

typedef struct _test_ {
  char * name;
  bool (*test)(void * arg);
} Test;

static bool _test_small(void*a) {
  OE oe = (OE)a;
  bool res = False;

  MiniMacsEnc enc = MiniMacsEnc_BitFFTNew(oe, 255, 85);

  res = (enc != 0);

  MiniMacsEnc_BitDestroy(&enc);

  return res;
}

static void rand_bytes( byte * b, uint lb ) {
  uint i = 0;
  
  while(i+4 < lb) {
    int r = rand();
    i2b(r,b+i);
    i+=4;
  }

  for(;i<lb;i++) {
    b[i] = (byte)rand();
  } 
}



static
bool _test_fftdim_comp(void * a) {
  OE oe = (OE)a;
  byte * codeword = 0;
  byte * expected_codeword = 0;
  byte msg[120] = {0};
  bool res = False;
  uint ltext = 85;
  uint lcode = 255;
  MiniMacsEnc enc = MiniMacsEnc_BitFFTNew(oe, lcode, ltext);
  MiniMacsEnc fft = MiniMacsEnc_FFTNew(oe);
  uint i = 0 ;

  for(i = 0;i < 120;++i) {
    msg[i] = (i % 3 == 0);
  }
  
  expected_codeword = fft->encode(msg,ltext);

  codeword = enc->encode(msg, ltext);
  
  for(i = 0; i < lcode; ++i) {
    if (expected_codeword[i] != codeword[i]) { 
      printf("Failed at %u\n",i);
      dump_data_as_hex(expected_codeword, lcode, 16);
      printf("\n");
      dump_data_as_hex(codeword, lcode, 16);
      goto fail;
    }
  }

  res = True;
 fail:
  MiniMacsEnc_FFTDestroy( &fft );
  MiniMacsEnc_BitDestroy( & enc );
  oe->putmem(codeword);
  oe->putmem(expected_codeword);
  return res;

}


static Test tests[] = {
  {"Small", _test_small},
  {"Correctness with fft", _test_fftdim_comp},
};

int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();
  uint i = 0, j = 0;
  char txt[64] = {0};
  byte msg[170] = {0};
  byte * codeword = 0;
  MiniMacsEnc fft = 0;
  init_polynomial();
  InitStats(oe);
  /*
 fft= MiniMacsEnc_FFTNew(oe);
 
  printf("byte t[%u][%u] = {",85,170);
  for(i = 0; i < 85;++i) {
    if (i > 0) msg[i-1] = 0;

    msg[i] = 1;
    codeword = fft->encode(msg, 85);

    printf("{");
    for(j = 85;j < 255;++j) {
      printf("0x%02x,",codeword[j]);
    }
    printf("},\n");
  }
  printf("}\n");
  return 0;
  */
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
