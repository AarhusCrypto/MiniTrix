#include <minimacs/minimacs.h>
#include <minimacs/generic_minimacs.h>
#include <osal.h>
#include <carena.h>
#include <math/polynomial.h>
#include <stats.h>
#include <minimacs/minimacs_rep.h>

static MiniMacs setup_generic_minimacs(OE oe, const char * raw_material_file) {
  CArena arena = CArena_new(oe);
  MiniMacs res = 0;
  MiniMacsTripleRep * triples = 0;
  MiniMacsRep * singles = 0;
  MiniMacsRep ** pairs = 0;
  uint ltriples=0,lsingles=0,lpairs=0;
  uint player = 0;
  uint i = 0;
  load_shares( raw_material_file, &triples, &ltriples,
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

  res = GenericMiniMacs_DefaultNew(oe, arena, singles, lsingles, pairs, lpairs, triples, ltriples);
  return res;
}

#define C( CALL ) {				\
  mr = (CALL);					\
  if (mr.rc != 0) {printf("Error: %s",mr.msg);return mr.rc;}}

int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
  init_stats(oe);
  init_polynomial();
  if (oe) {
    MR mr = {{0}};
    MiniMacs mm = setup_generic_minimacs(oe, a[1]);
    printf("Inviting 1 party to computate on port 2020\n");
    mm->invite(1,2020);
    mm->init_heap(6);
    
    C(mm->secret_input(0,0,Data_shallow("Rasmus",7)));
    C(mm->open(0));
    
    C(mm->secret_input(0,1,Data_shallow("\001",1)));
    C(mm->secret_input(0,2,Data_shallow("\001",1)));
    C(mm->mul(3,2,1));
    C(mm->add(3,3,3));
    C(mm->open(3));

    GenericMiniMacs_destroy( & mm );
  }

  PrintMeasurements(oe);

 failure:
  return 0;
}
