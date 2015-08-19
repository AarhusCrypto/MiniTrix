#include "minimacs/minimacs_pre.h"
#include <stdio.h>
#include <osal.h>

int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();
  MiniMacsRep * singles = 0;
  MiniMacsRep ** pairs  = 0;
  MiniMacsTripleRep * triples = 0;
  uint lsingles=0, ltriples=0, lpairs=0,i=0;
  uint player = 0;

  if (c != 2) {
    printf("Usage <filename>\n");
    return 0;
  }

  load_shares(oe,a[1],
	      &triples, &ltriples, 
	      &singles, &lsingles,
	      &pairs, &lpairs );

  for( i = 0;i < singles[0]->lmac;++i) {
    if (singles[0]->mac[i] == 0) { player = i; break; }
  }
  
  // validate first single
  {

  }

  printf(" --- MiniMacs representation ---\n");
  printf("for player            : %d\n", player);
  printf("nplayers              : %d\n", singles[0]->lmac);
  printf("message length        : %d\n", singles[0]->lval);
  printf("codelength            : %d\n", singles[0]->lcodeword);
  printf("ncount                : %d\n", lsingles );
  OperatingEnvironment_Destroy(&oe);
  return 0;
}
