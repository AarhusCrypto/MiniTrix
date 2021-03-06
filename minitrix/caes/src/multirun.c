#include <stdio.h>
#include <osal.h>
#include <minimacs/minimacs_rep.h>
#include <minimacs/minimacs.h>
#include <minimacs/generic_minimacs.h>
#include <unistd.h>
#include <time.h>
#include <stats.h>
#include <coov3.h>


static 
MiniMacs GenericFFTMiniMacs_new(OE oe, char * filename) {

    MiniMacsRep * singles = 0;
    MiniMacsRep ** pairs = 0;
    MiniMacsTripleRep * triples;
    uint lsingles = 0;
    uint lpairs = 0;
    uint ltriples = 0;
    MiniMacs comp = 0;
    CArena arena = 0;
    MiniMacsEnc fftenc = 0;
    arena = CArena_new(oe);
    fftenc = MiniMacsEnc_FFTNew(oe);
    load_shares(oe,filename,
		&triples, &ltriples,
		&singles, &lsingles,
		&pairs, &lpairs );

    printf("Loading material from file %s ... \n", filename);

    comp = GenericMiniMacs_New(oe,arena, fftenc,
                               singles, lsingles, 
                               pairs, lpairs, 
                               triples, ltriples );
    

    return comp;
}

static 
int run(char * material, char * ip, uint myid, uint count, OE oe, MiniMacs mm) {
  CArena mc = CArena_new(oe);
  MpcPeer mission_control = 0;
     
  mc->connect("10.11.105.67", 65000);
  mission_control = mc->get_peer(0);
  
  if (!mission_control) {
    oe->p("Failed connection to mission control. aborting.\n");
    return -1;
  }
 
  if (mm->get_id() == 0) {
    if (mm->invite(1,2020+myid) != 0) {
      byte d[128] = {0};
      char m[128] = {0};
      osal_sprintf(m,"Failed to invite %u peers on port %u",1,2020+myid);
      oe->p(m);
      i2b(myid, d);
      osal_sprintf(d+4,"error");
      mission_control->send(d);
      return 0;
    }
  } else {
    if (mm->connect(ip,2020+myid) != 0) {
      char m[128] = {0};
      osal_sprintf(m,"Failed to connect to peer %s:%u",ip,2020+myid);
      oe->p(m);
      return 0;
    }
  }

  {
    byte key[128] = {0};
    byte ptxt[128] = {0};
    mpc_aes(mm,ptxt, key,myid,count,mission_control);
    CArena_destroy(&mc);
  }
  PrintMeasurements(oe);
  return 0;
}


int main(int c, char **a) {
  char * material = 0;
  char * ip = "127.0.0.1";
  uint count = 0, i = 0;
  OE oe = OperatingEnvironment_New();
  MiniMacs mm = 0;
  int * pids = 0;

  InitStats(oe);
  init_polynomial();
  if (c < 2 || c > 4) {
    printf("multirun <material> <count> [< server >]\n");
    return -1;
  }

  if ( c >= 2 ) {
    material = a[1];
  }

  if (c >= 3) {
    count = atoi(a[2]);
  }
  
  if (c >= 4) {
    ip =a[3];
  }

#ifndef FFT
  mm=GenericMiniMacs_DefaultLoadNew(oe, material);
#else
  mm = GenericFFTMiniMacs_new(oe, material);
#endif

  printf("Multirun CAES\n");
  printf("material taken from: %s\n",material);
  printf("ip: %s\n", ip);
  printf("count: %u\n",count);
  pids = (int*)oe->getmem(sizeof(int)*count);

  for( i = 0; i < count; ++i) {
    pids[i] = fork();
    if (pids[i] == 0) {
      return run(material,ip,i,count,oe,mm);
    }
  }
  CHECK_POINT_S("TOTAL");
  for(i = 0;i < count;++i) {
    wait(pids[i]);
  }
  CHECK_POINT_E("TOTAL");
  PrintMeasurements(oe);
}
