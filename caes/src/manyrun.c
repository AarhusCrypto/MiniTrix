#include <caes.h>
#include <stdio.h>
#include <osal.h>
#include <minimacs/minimacs.h>
#include <minimacs/generic_minimacs.h>
#include <unistd.h>
#include <time.h>
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
int run(char * material, char * ip, OE oe, MiniMacs mm) {
  uint no_players = mm->get_no_players();
  uint myid = mm->get_id();
  int id = 0;

  printf("number of players: %u\n",no_players);
  printf("I am: %u\n",myid);
  printf("Waiting for %u\n",no_players-(myid+1));

  if (myid < no_players-1) {
    mm->invite(no_players-(myid+1),2020+100*myid);
  }

  for(id = myid-1;id >= 0;--id) {
    printf("connecting to peer at %s:%u\n",ip,2020+100*id);
    mm->connect(ip,2020+100*id);
  }

  {
    byte key[128] = {0};
    byte ptxt[128] = {0};
    ull start = _nano_time();
    mpc_aes(mm,ptxt, key,0,0,0);
    printf("Total time %llu ns\n",_nano_time()-start);
  }

  return 0;
}

int main(int c, char **a) {
  char * material = 0;
  char * ip = "127.0.0.1";
  uint  i = 0;
  OE oe = OperatingEnvironment_LinuxNew();
  MiniMacs mm = 0;

  init_polynomial();
  if (c < 2 || c > 4) {
    printf("multirun <material> [< server >]\n");
    return -1;
  }

  if ( c >= 2 ) {
    material = a[1];
  }
 
  if (c >= 3) {
    ip =a[3];
  }

  mm =GenericMiniMacs_DefaultLoadNew(oe, material);
  printf("Multirun CAES\n");
  printf("material taken from: %s\n",material);
  printf("ip: %s\n", ip);
  
  return run(material,ip,oe,mm);
}
