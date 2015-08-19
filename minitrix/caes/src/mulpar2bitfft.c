#include <stdio.h>
#include <osal.h>
#include <minimacs/minimacs_rep.h>
#include <minimacs/minimacs.h>
#include <minimacs/bitwisemulpar2_minimacs.h>
#include <unistd.h>
#include <time.h>
#include <stats.h>
#include <coov3.h>

/*
static
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
}
*/

static char * bitlab = "87.104.238.146";
static char * llama01= "10.11.82.1";
static char * localhost = "127.0.0.1";
static char * sputnik = "10.11.105.67";

static 
int run(char * ip, uint myid, uint count, OE oe, MiniMacs mm) {
  CArena mc = CArena_new(oe);
  MpcPeer mission_control = 0;
     
  if (mc->connect(localhost, 65000).rc != 0) {
    oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to connect to the performance monitor.");
    return -1;
  };
  mission_control = mc->get_peer(0);
  
  if (!mission_control) {
    oe->p("Failed connection to mission control. aborting.\n");
    return -1;
  }
 
  if (mm->get_id() == 0) {
    if (mm->invite(1,2020+myid) != 0) {
      byte d[256] = {0};
      char m[128] = {0};
      osal_sprintf(m,"Failed to invite %u peers on port %u",1,2020+myid);
      oe->p(m);
      i2b(myid, d);
      osal_sprintf(d+4,"error");
      mission_control->send(Data_shallow(d,128));
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
  char * material = 0, * bdt_material=0;
  char * ip = "127.0.0.1";
  uint count = 0, i = 0;
  OE oe = OperatingEnvironment_New();
  MiniMacs mm = 0;
  int * pids = 0;

  InitStats(oe);
  init_polynomial();
  if (c < 3 || c > 5) {
    printf("multirun <material> <bdt_material> <count> [< server >]\n");
    return -1;
  }

  if ( c >= 3 ) {
    material = a[1];
    bdt_material = a[2];
  }

  if (c >= 4) {
    count = atoi(a[3]);
  }
  
  if (c >= 5) {
    ip =a[4];
  }
  mm=BitWiseMulPar2MiniMacs_DefaultLoadFFTNew(oe, material, bdt_material, True);

  printf("Multirun CAES\n");
  printf("material taken from: %s\n",material);
  printf("ip: %s\n", ip);
  printf("count: %u\n",count);
  pids = (int*)oe->getmem(sizeof(int)*count);

  if (count == 1) {
    return run(ip,0,1,oe,mm);
  }

  for( i = 0; i < count; ++i) {
    pids[i] = fork();
    if (pids[i] == 0) {
      return run(ip,i,count,oe,mm);
    }
  }
  CHECK_POINT_S("TOTAL");
  for(i = 0;i < count;++i) {
    wait(pids[i]);
  }
  CHECK_POINT_E("TOTAL");
  PrintMeasurements(oe);
}
