#include "caes.h"
#include "minimacs/minimacs_rep.h"
#include <stats.h>
#include <time.h>
#include <carena.h>
#include <stdlib.h>

static void dump_heap(MiniMacs mm) {
  int i = 0;
  
  for(i = 0;i < 33873;++i) {
    mm->open(i);
    MiniMacsRep r = mm->heap_get(i);
    if (!r) break;
    if (i > 0  && i % 32 == 0) printf("\n");
    printf("%02x ",r->codeword[0]);
  }
  
  printf("\n******************** HEAP DUMPED ********************\n");
  exit(0) ;
}

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

/*
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    printf("Failed to get time\n");
    exit(-1);
  }
}
*/

static
void mission_control_start(MpcPeer mission_control, uint myid,uint count) {
  byte raw_pid[256] = {0};
  Data _pid = Data_shallow(raw_pid, sizeof(raw_pid));
  uint pid = getpid();
  i2b(pid, _pid->data);
  i2b(myid, _pid->data+4);
  i2b(count, _pid->data+8);

  if (mission_control) {
    mission_control->send(_pid);
  }

}

static
void mission_control_bang(MpcPeer mission_control) {
  byte _bang[256] = {0};
  Data bang = Data_shallow(_bang,sizeof(_bang));
  if (mission_control) {
    mission_control->receive(bang);
  }
}

static
void mission_control_stop(MpcPeer mission_control, uint myid) {
  byte raw_pid[256] = {0};
  Data _pid = Data_shallow(raw_pid, sizeof(raw_pid));
  uint pid = getpid();
  i2b(pid, _pid->data);
  i2b(myid, _pid->data+4);
  if (mission_control) {
    mission_control->send(_pid);
  }
}


byte * mpc_aes(MiniMacs mm, byte * plaintext, byte * key, 
               uint myid, uint count, MpcPeer mission_control) {
  uint i=0, player=0;
  uint nplayers = mm->get_no_peers() + 1;
  unsigned long long start = 0, stop = 0;

#include "AES_.sir"
  for(i = 33744;i < 33744+128;++i) {
    MiniMacsRep rep = mm->heap_get(i);
    if ( (i-33744) > 0 && ((i-33744) % 16) == 0) printf("\n");
    if (rep) 
      printf("%2x ",rep->codeword[0]);
    else 
      printf("x ");
  }
  printf("\n");
  return 0;
}
