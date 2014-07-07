
#include <stdio.h>
#include <minimacs/minimacs_rep.h>
#include <minimacs/generic_minimacs.h>
#include <osal.h>
#include <carena.h>
#include <stats.h>
#include <caes.h>

int main(int c, char **a) {

  printf("Aarhus University - Multiparty Computation AES\n");
  printf("All rights reserved (C)\n");

  if (c == 2 || c == 3 || c == 4) {
    MiniMacsRep * singles = 0;
    MiniMacsRep ** pairs = 0;
    MiniMacsTripleRep * triples;
    uint lsingles = 0;
    uint lpairs = 0;
    uint ltriples = 0;
    MiniMacs comp = 0;
    CArena arena = 0;
    OE oe = 0;
    uint myid = 0;
    MiniMacsEnc fftenc = 0;
    char * ipaddr = "any";
    uint port = 2020;
    

    init_polynomial();
    oe = OperatingEnvironment_LinuxNew();
    arena = CArena_new(oe);
    fftenc = MiniMacsEnc_FFTNew(oe);

    InitStats(oe);


    printf("Loading material from file %s ... \n", a[1]);
    load_shares(a[1],
		&triples, &ltriples,
		&singles, &lsingles,
		&pairs, &lpairs );

    comp = GenericMiniMacs_New(oe,arena, fftenc,
                               singles, lsingles, 
                               pairs, lpairs, 
                               triples, ltriples );
    
    if (ltriples == 0 && lpairs == 0 && lsingles == 0) {
      printf("Loading %s failed ...\n", a[1]);
      return -1;
    }

    myid = minimacs_rep_whoami(singles[0]);
    if(myid == 0) {
      if (c >= 3) {
        port = atoi(a[2]);
      }


      uint no = minimacs_rep_no_players(singles[0]);
      if (no == 0) return -1;
      no--;
      printf("Waiting for %u players to connect on %s:%u.\n",no,ipaddr,port);
      comp->invite(no,port);
    } else {
    if (c >= 3) {
      ipaddr = a[2];
    } else {
      ipaddr = "127.0.0.1";
    }

    if (c >= 4) {
      port = atoi(a[3]);
    }

      comp->connect(ipaddr,port);
    }
    
    {
      byte key[128] = {0};
      byte pltxt[128] = {0};
      mpc_aes(comp,pltxt,key,0,0,0);
    }

    PrintMeasurements(oe);

  } else {
    printf("Usage %s <preprocessed material>\n",a[0]);
    return 0;
  }
  return 0;
}
