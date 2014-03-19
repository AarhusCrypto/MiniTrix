#include "caes.h"
#include "minimacs/minimacs_rep.h"
#include <stats.h>
#include <time.h>
#include <carena.h>

static inline replica_private_input(byte val, uint dst, MiniMacs mm) {
  uint rep = mm->get_ltext();
  uint i = 0;
  byte * r = malloc(rep);
  uint player = 0;
  uint nplayers = mm->get_no_peers()+1;

  printf("ltext = %u\n", mm->get_ltext());
  for(i = 0;i < rep;++i) {
    r[i] = val;
  }

  for(player = 0; player < nplayers;++player) {
    mm->secret_input(player,128,Data_shallow(r,rep));
    mm->add(dst,128,dst);
  }
  free(r);
}

static inline
void replica_public_input(byte val, uint dst, MiniMacs mm) {
  uint rep = mm->get_ltext();
  uint i = 0;
  byte * r = malloc(rep);
  for(i = 0;i < rep;++i) {
    r[i] = val;
  }
  mm->public_input( dst, Data_shallow(r,rep));
  free(r);
}


unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    printf("Failed to get time\n");
    exit(-1);
  }
}


static
void mission_control_start(MpcPeer mission_control) {
  byte raw_pid[256] = {0};
  Data _pid = Data_shallow(raw_pid, sizeof(raw_pid));
  uint pid = getpid();
  i2b(pid, _pid->data);
  if (mission_control) {
    mission_control->send(_pid);
  }
}

static
void mission_control_stop(MpcPeer mission_control) {
  byte raw_pid[256] = {0};
  Data _pid = Data_shallow(raw_pid, sizeof(raw_pid));
  uint pid = getpid();
  i2b(pid, _pid->data);
  if (mission_control) {
    mission_control->send(_pid);
  }
}


byte * mpc_aes(MiniMacs mm, byte * plaintext, byte * key, MpcPeer mission_control) {
  uint i=0, player=0;
  uint nplayers = mm->get_no_peers() + 1;
  unsigned long long start = 0, stop = 0;

#include "AES.cir"
  for(i = 33744;i < 33744+128;++i) {
    MiniMacsRep rep = mm->heap_get(i);
    if ( (i-33744) > 0 && ((i-33744) % 16) == 0) printf("\n");
    if (rep) 
      printf("%u ",rep->codeword[0]);
    else 
      printf("x ");
  }
  printf("\n");
  return 0;
}
