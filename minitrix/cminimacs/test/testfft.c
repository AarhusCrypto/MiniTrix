#include <minimacs/minimacs.h>
#include <minimacs/generic_minimacs.h>
#include <osal.h>
#include <carena.h>
#include <math/polynomial.h>
#include <reedsolomon/minimacs_enc_fft.h>
#include <minimacs/minimacs_rep.h>

#define C( CALL ) {				\
  mr = (CALL);					\
  if (mr != 0) {printf("Error: See above.");return mr;}}


static MiniMacs setup_generic_minimacs(OE oe, const char * raw_material_file) {
  CArena arena = CArena_new(oe);
  MiniMacs res = 0;
  MiniMacsTripleRep * triples = 0;
  MiniMacsRep * singles = 0;
  MiniMacsRep ** pairs = 0;
  uint ltriples=0,lsingles=0,lpairs=0;
  uint player = 0;
  uint i = 0;
  MiniMacsEnc encoder = 0;

  load_shares( oe, raw_material_file, &triples, &ltriples,
	       &singles, &lsingles, &pairs, &lpairs);

  if(lsingles) {

    for(i = 0;i < singles[0]->lmac;++i) {
      if (singles[0]->mac[i] == 0) { player = i; break; }
    }


    printf(" --- MiniMacs representation ---\n");
    printf("for player            : %d\n", player);
    printf("nplayers              : %d\n", singles[0]->lmac);
    printf("message length        : %d\n", singles[0]->lval);
    printf("codelength            : %d\n", singles[0]->lcodeword);
    printf("ncount                : %d\n", lsingles );
  }

  encoder = MiniMacsEnc_FFTNew( oe );

  res = GenericMiniMacs_New(oe, arena, encoder, singles, lsingles, pairs, lpairs, triples, ltriples);
  return res;
}


int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();
  init_polynomial();
  if (oe) {
    MR mr = 0;
    MiniMacs mm = setup_generic_minimacs(oe, a[1]);
    printf("Inviting 1 party to computate on port 2020\n");
    mm->invite(1,2020);
    mm->init_heap(6);
    

    // test secret input and open
    {
      MiniMacsRep rep = 0;
      byte data[85] = {0};
      uint i = 0;
      for(i = 0;i < 85;++i) {
        data[i] = (byte)((i*101+65537)%255);
      }
      C(mm->secret_input(0,0,Data_shallow(data,85)));
      C(mm->open(0));
      rep = mm->heap_get(0);
      if (!rep) {
        printf("Nothing to open\n");
        return -1;
      }

      for(i = 0;i < 85;++i) {
        if (rep->codeword[i] != data[i]) {
          printf("Failure at possition %u\n",i);
          return -1;
        }
      }
      printf("Secret Input Done\n");
    }

    // test multiply
    {
      byte data1[85] = {0};
      byte data2[85] = {0};
      byte p[85] = {0};
      MiniMacsRep rep = 0;
      uint i = 0;

      for(i = 0; i < 85;++i) {
        data1[i] = (i*101+65537) % 255;
        data2[i] = (i*31+257) % 255;
        p[i] = multiply(multiply(data1[i], data2[i]), 42);
      }

      C(mm->secret_input(0,0,Data_shallow(data1,85)));
      C(mm->secret_input(0,1,Data_shallow(data2,85)));
      memset(data1,42,85);
      C(mm->secret_input(0,2,Data_shallow(data1,85)));
      C(mm->mul(3,0,1));
      C(mm->mul(4,2,3));
      
      C(mm->open(4));
      rep = mm->heap_get(4);
      if (!rep) { 
        printf("Rep is null.\n");
        return -1;
      }
      for(i = 0;i < 85;++i) {
        if (rep->codeword[i] != p[i]) {
          printf("Failed at pos %u expected 0x04 got %u\n",i,rep->codeword[i]);
          return -1;
        }
      }
    }
    

    GenericMiniMacs_destroy( & mm );
  }

 failure:
  return 0;
}
