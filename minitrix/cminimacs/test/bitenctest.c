#include <reedsolomon/minimacs_bit_encoder.h>
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

  MiniMacsEnc enc = MiniMacsEnc_BitNew(oe, 32, 16);

  res = (enc != 0);

  MiniMacsEnc_BitDestroy(&enc);

  return res;
}


static bool _test_not_null(void*a) {
  OE oe = (OE)a;
  bool res = False;

  MiniMacsEnc enc = MiniMacsEnc_BitNew(oe, 256, 120);

  res = (enc != 0);

  MiniMacsEnc_BitDestroy(&enc);

  return res;
}

static bool _test_encode_not_null(void * a) {
  OE oe =  (OE)a;
  byte * codeword = 0;
  byte msg[120] = {0};
  bool res = False;

  MiniMacsEnc enc = MiniMacsEnc_BitNew(oe, 256, 120 );

  codeword = enc->encode(msg,sizeof(msg));
  if (!codeword) goto fail;

  res = True;
 fail:
  MiniMacsEnc_BitDestroy(&enc);
  oe->putmem(codeword);
  return res;
}

static bool _test_zero_codeword_validates(void * a) {
  OE oe = (OE)a;
  byte * codeword = 0;
  byte msg[120] = {0};
  bool res = False;
  MiniMacsEnc enc = MiniMacsEnc_BitNew(oe, 256, 120);

  codeword = enc->encode(msg,sizeof(msg));
  if (!codeword) goto fail;

  if (!enc->validate(codeword, sizeof(msg))) goto fail;
      
  res = True;
 fail:
  MiniMacsEnc_BitDestroy(&enc);
  oe->putmem(codeword);
  return res;
}

static bool _test_zero_codeword_fail(void * a) {
  OE oe = (OE)a;
  byte * codeword = 0;
  byte msg[120] = {0};
  bool res = False;
  MiniMacsEnc enc = MiniMacsEnc_BitNew(oe,  256, 120);

  codeword = enc->encode(msg,sizeof(msg));
  if (!codeword) goto fail;

  codeword[1] = 0x42; // invalidate codeword
  if (enc->validate(codeword, sizeof(msg))) goto fail;

  res = True;
 fail:
  MiniMacsEnc_BitDestroy(&enc);
  oe->putmem(codeword);
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


static bool _test_1000_random(void * a) {
  OE oe = (OE)a;
  byte * codeword = 0;
  byte msg[120] = {0};
  bool res = False;
  MiniMacsEnc enc = MiniMacsEnc_BitNew(oe, 256, 120);
  uint i = 0 ;
  
  for(i = 0; i < 1000;++i) {
    if (codeword) oe->putmem(codeword);

    rand_bytes(msg, sizeof(msg));

    codeword = enc->encode(msg, sizeof(msg));
    if (!codeword) goto fail;

    if (!enc->validate(codeword, sizeof(msg))) { 
      char m[32] = {0};
      osal_sprintf(m,"i=%u",i);
      oe->p(m);
      goto fail;
    }


  }

  res = True;
 fail:
  MiniMacsEnc_BitDestroy(&enc);
  oe->putmem(codeword);
  return res;
}



static bool _test_1200_random(void * a) {
  OE oe = (OE)a;
  byte * codeword = 0;
  byte msg[120] = {0};
  bool res = False;
  MiniMacsEnc enc = MiniMacsEnc_BitNew(oe, 256, 120);
  uint i = 0 ;
  
  for(i = 0; i < 1200;++i) {
    if (codeword) oe->putmem(codeword);

    rand_bytes(msg, sizeof(msg));

    CHECK_POINT_S("[RSCODE] Encode only");
    codeword = enc->encode(msg, sizeof(msg));
    if (!codeword) goto fail;
    CHECK_POINT_E("[RSCODE] Encode only");

    if (!enc->validate(codeword, sizeof(msg))) { 
      char m[32] = {0};
      osal_sprintf(m,"i=%u",i);
      oe->p(m);
      goto fail;
    }
  }

  res = True;
 fail:
  MiniMacsEnc_BitDestroy(&enc);
  oe->putmem(codeword);
  return res;
}


static bool _test_1000_tampered_random(void * a) {
  OE oe = (OE)a;
  byte * codeword = 0;
  byte msg[120] = {0};
  bool res = False;
  MiniMacsEnc enc = MiniMacsEnc_BitNew(oe, 256, 120);
  uint i = 0 ;
  
  for(i = 0; i < 1000;++i) {
    if (codeword) oe->putmem(codeword);

    rand_bytes(msg, sizeof(msg));

    codeword = enc->encode(msg, sizeof(msg));
    if (!codeword) goto fail;

    codeword[0] ++ ;
    codeword[42] ++;
    if (enc->validate(codeword, sizeof(msg))) { 
      char m[32] = {0};
      osal_sprintf(m,"i=%u",i);
      oe->p(m);
      goto fail;
    }
  }

  res = True;
 fail:
  MiniMacsEnc_BitDestroy(&enc);
  oe->putmem(codeword);
  return res;
}

static
bool _test_matrix_comp(void * a) {
  OE oe = (OE)a;
  byte * codeword = 0;
  byte * expected_codeword = 0;
  byte msg[120] = {0};
  bool res = False;
  uint ltext = 120;
  uint lcode = 256;
  MiniMacsEnc enc = MiniMacsEnc_BitNew(oe, lcode, ltext);
  MiniMacsEnc mxt = MiniMacsEnc_MatrixNew(oe, lcode, ltext);
  uint i = 0 ;

  for(i = 0;i < 120;++i) {
    msg[i] = (i % 3 == 0);
  }
  
  expected_codeword = mxt->encode(msg,ltext);

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
  MiniMacsEnc_MatrixDestroy( &mxt );
  MiniMacsEnc_BitDestroy( & enc );
  oe->putmem(codeword);
  oe->putmem(expected_codeword);
  return res;

}





static
bool _test_matrix_comp_schur(void * a) {
  OE oe = (OE)a;
  byte * codeword = 0;
  byte * expected_codeword = 0;
  byte msg[240] = {0};
  bool res = False;
  MiniMacsEnc enc = MiniMacsEnc_BitNew(oe, 256, 120);
  MiniMacsEnc mxt = MiniMacsEnc_MatrixNew(oe, 256, 120);
  uint i = 0 ;

  for(i = 0;i < 240;++i) {
    msg[i] = (i % 3 == 0);
  }
  
  expected_codeword = mxt->encode(msg,240);

  codeword = enc->encode(msg, 240);
  
  for(i = 0; i < 256; ++i) {
    if (expected_codeword[i] != codeword[i]) goto fail;
  }

  res = True;
 fail:
  MiniMacsEnc_MatrixDestroy( &mxt );
  MiniMacsEnc_BitDestroy( & enc );
  oe->putmem(codeword);
  oe->putmem(expected_codeword);
  return res;

}

static Test tests[] = {
  
  {"Small", _test_small},
  {"Creation not null", _test_not_null},
  {"Encode not null", _test_encode_not_null},
  {"Check zero codeword validates", _test_zero_codeword_validates},
  {"Tampered zero codeword validate fail", _test_zero_codeword_fail},
  {"1000 Random message", _test_1000_random},
  {"Tampered 1000 Random messages fail.", _test_1000_tampered_random},
  {"1200 Random messages", _test_1200_random},
  {"Correctness comparison in Schur transform to Matrix", _test_matrix_comp_schur}, 
  {"Correctness comparison to Matrix", _test_matrix_comp},

  
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
