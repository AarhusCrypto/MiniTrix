#include <stdio.h>
#include <osal.h>
#include <minimacs/minimacs.h>
#include <minimacs/generic_minimacs.h>
#include <unistd.h>
#include <time.h>
#include <stats.h>
#include <coo.h>


static
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
}


static 
int run(char * material, char * ip, uint count, OE oe, MiniMacs mm) {
  CArena mc = CArena_new(oe);
  Data _pid = Data_new(oe,1024);
  MpcPeer mission_control = 0;
  uint pid = getpid();
  i2b(pid, _pid->data);
     
  mc->connect("87.104.238.146", 65000);
  mission_control = mc->get_peer(0);
  
  if (!mission_control) {
    oe->p("Failed connection to mission control. aborting.\n");
    Data_destroy(oe,& _pid);
    return -1;
  }
 
  if (mm->get_id() == 0) {
    mm->invite(1,2020+count);
  } else {
    if (mm->connect(ip,2020+count) != 0) {
      Data_destroy(oe,& _pid);
      return 0;
    }
  }

  {
    byte key[128] = {0};
    byte ptxt[128] = {0};
    mission_control->send(_pid);
    usleep(500);
    //    mpc_aes(mm,ptxt, key);
    mission_control->send(_pid);

    CArena_destroy(&mc);
  }
  Data_destroy(oe,& _pid);
  return 0;
}


int main(int c, char **a) {
  char * material = 0;
  char * ip = "127.0.0.1";
  uint count = 0, i = 0;
  OE oe = OperatingEnvironment_LinuxNew();
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

  mm=GenericMiniMacs_DefaultLoadNew(oe, material);
  printf("Multirun CAES\n");
  printf("material taken from: %s\n",material);
  printf("ip: %s\n", ip);
  printf("count: %u\n",count);
  pids = (int*)oe->getmem(sizeof(int)*count);
  CHECK_POINT_S("TOTAL");
  for( i = 0; i < count; ++i) {
    pids[i] = fork();
    if (pids[i] == 0) {
      return run(material,ip,i,oe,mm);
    }
  }
  for(i = 0;i < count;++i) {
    wait(pids[i]);
  }
  CHECK_POINT_E("TOTAL");
  PrintMeasurements(oe);
}
