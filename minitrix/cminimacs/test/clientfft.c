#include <minimacs/generic_minimacs.h>
#include <encoding/hex.h>
#include <reedsolomon/minimacs_enc_fft.h>
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

  load_shares( oe,raw_material_file, &triples, &ltriples,
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

  encoder = MiniMacsEnc_FFTNew(oe);
  res = GenericMiniMacs_New(oe, arena, encoder,singles, lsingles, pairs, lpairs, triples, ltriples);
  return res;
}

#define C( CALL ) {				\
  mr = (CALL);					\
  if (mr != 0) {printf("Error: See above");return mr;}}



int main(int c, char **a) {

  OE oe = OperatingEnvironment_New();
  init_polynomial();
  if (oe) {
    MR mr = 0;
    MiniMacs mm = setup_generic_minimacs(oe, a[1]);
    mr = mm->connect("127.0.0.1",2020);
    if (mr != 0) {
      printf("Error: See above.\n");
      return 0;
    }
    mm->init_heap(6); 
    
    C(mm->secret_input(0,0,0));
    C(mm->open(0));

    C(mm->secret_input(0,0,0));
    C(mm->secret_input(0,1,0));
    C(mm->secret_input(0,2,0));
    C(mm->mul(3,0,1));
    C(mm->mul(4,2,3));
    C(mm->open(4));
    printf("Done\n");
  }

 failure:
  return 0;
}
