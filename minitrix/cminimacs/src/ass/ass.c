#include <ass/ass.h>
#include <math.h> // need rand
#include <stdlib.h> // malloc/free
#include <string.h> // memset
#include <math/polynomial.h>

byte ** ass_create_shares( byte * msg, uint lmsg, uint nshares ) {
  byte * sums = (byte*)0;
  int i=0,j=0;
  byte ** result = (byte**)0;

  if (!msg) return (byte**)0;

  // smart buffer for remembering the sum of each entry over the
  // shares.
  sums = malloc(lmsg);
  if (!sums) return (byte**)0;
  memset(sums,0,lmsg);

  // allocate pointers for the result
  result = (byte**)malloc(nshares*sizeof(byte*));
  if (!result) return (byte**)0;
  memset(result,0,nshares*sizeof(byte*));

  // allocate vectors, one for each pointer above
  for(i = 0;i<nshares;i++) {
    result[i] = (byte*)malloc(lmsg);
    if (!result[i]) goto failure;
    memset(result[i],0,lmsg);
  }

  // randomize vectors (except the last one)
  for(i=0;i<nshares-1;i++) {
    for(j=0;j<lmsg;j++) {
      result[i][j] = (byte)rand();
      sums[j] = add(result[i][j],sums[j]);
    }
  }

  // compute the last vector
  for(j=0;j<lmsg;j++) {
    result[nshares-1][j] = add(msg[j],sums[j]);
  }
    
  // free sums
  if (sums) {
    memset(sums,0,lmsg);
    (free(sums),sums=0);
  }
    
  //hey we are done
  return result;
 failure:
  if (result) {
    for(i=0;i<nshares;i++) {
      if (result[i]) { 
	for(j=0;j<lmsg;j++) result[i][j] = 0; 
	(free(result[i]),result[i]=0);
      }
    }
  }
  
  if (sums) {
    for(i=0;i<lmsg;i++) { 
      sums[i] = 0; 
    }
    (free(sums),sums=0);
  }
  
  // damn !
  return 0;
}


void ass_clean_shares(byte ** shares, uint lshares) {
  
  int i = 0, j = 0;
  
  if (!shares) return;

  for(i=0;i<lshares;i++) {
    free(shares[i]);shares[i]=0;
  }

  free(shares);
  
}


byte* ass_reconstruct(byte**shares,uint lshare, uint nshares ) {
  
  int i = 0, j = 0;

  byte * res = malloc(lshare);

  if (!res) return 0;
  memset(res,0,lshare);
  
  for(i=0;i<nshares;i++) {
    for(j=0;j<lshare;j++) {
      res[j] = add(res[j],shares[i][j]);
    }
  }
  

  return res;
}
