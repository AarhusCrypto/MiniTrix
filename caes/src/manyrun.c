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
int run(char * material, char * ip, uint count, OE oe, MiniMacs mm) {
  uint no_players = mm->get_no_players();
  uint myid = mm->get_id();
  uint id = 0;

  printf("number of players: %u\n",no_players);
  printf("I am: %u\n",myid);
  printf("Waiting for %u\n",no_players-(myid+1));

  if (myid < no_players-1) {
    mm->invite(no_players-(myid+1),2020+100*myid);
  }

  for(id = 0;id < no_players;++id) {
    printf("connecting to peer at %s:%u\n",ip,2020+100*id);
    mm->connect(ip,2020+100*id);
  }

  {
    byte key[128] = {0};
    byte ptxt[128] = {0};
    ull start = _nano_time();
    mpc_aes(mm,ptxt, key);
    printf("Total time %llu ns\n",_nano_time()-start);
  }

  return 0;
}

int main(int c, char **a) {
  char * material = 0;
  char * ip = "127.0.0.1";
  uint count = 0, i = 0;
  OE oe = OperatingEnvironment_LinuxNew();
  MiniMacs mm = 0;

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

  mm =GenericMiniMacs_DefaultLoadNew(oe, material);
  printf("Multirun CAES\n");
  printf("material taken from: %s\n",material);
  printf("ip: %s\n", ip);
  printf("count: %u\n",count);

  for( i = 0; i < count; ++i) {
    uint pid;
    pid = fork();
    if (pid != 0) {
      return run(material,ip,i,oe,mm);
    }
  }
}
