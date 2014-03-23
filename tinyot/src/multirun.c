#include <stdio.h>
#include <osal.h>
#include <tinyot.h>
#include <carena.h>
#include <unistd.h>
#include <time.h>
#include <stats.h>
#include <coo.h>

void
mpc_aes(OE oe, TinyOT tot, byte * plaintext, tinyotshare ** key, byte * ciphertext, Data _pid, MpcPeer mission_control);

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
int run(char * material, char * ip, uint myid, uint count, OE oe, TinyOT tot) {
  CArena mc = CArena_new(oe);
  MpcPeer mission_control = 0;
  tinyotshare ** key = 0;
  byte plaintext[128] = {0};
  byte ciphertext[128] = {0};
  Data d = Data_new(oe,256);
  Data _pid = Data_new(oe,256);
  uint i = 0;

 
  i2b(myid,_pid->data);
  i2b(myid,_pid->data+4);
  i2b(count,_pid->data+8);

  key = (tinyotshare**)oe->getmem(sizeof(*key)*128);
  for(i = 0;i < 128;++i) {
    key[i] = oe->getmem(sizeof(*key[i]));
  }

  mc->connect("87.104.238.146", 65000);
  mission_control = mc->get_peer(0);


  
  if (!mission_control) {

    oe->p("Failed connection to mission control. aborting.\n");
    return -1;
  }


 
  if (tot->isAlice() == 0) {
    if (tot->invite(2020+myid) != 0) {
      byte d[128] = {0};
      char m[128] = {0};
      osal_sprintf(m,"Failed to invite %u peers on port %u",1,2020+myid);
      oe->p(m);
      i2b(myid, d);
      osal_sprintf(d+4,"error");
      mission_control->send(Data_shallow(d,sizeof(d)));
      return 0;
    }
  } else {
    if (tot->connect(ip,2020+myid) != 0) {
      char m[128] = {0};
      osal_sprintf(m,"Failed to connect to peer %s:%u",ip,2020+myid);
      oe->p(m);
      return 0;
    }
  }

  {
    mpc_aes(oe, tot, plaintext, key, ciphertext, _pid, mission_control);
    printf("Circuit done\n");
    CArena_destroy(&mc);
  }
  PrintMeasurements(oe);
  return 0;
}


int main(int c, char **a) {
  char * material = 0;
  char * ip = "127.0.0.1";
  uint count = 1, i = 0;
  OE oe = OperatingEnvironment_LinuxNew();
  TinyOT tot = 0;
  int * pids = 0;
  int alice = 0;

  InitStats(oe);

  if (c < 2 || c > 3) {
    printf("multirun <1=alice|0=bob> [count] [ip]\n");
    return -1;
  }

  if ( c >= 2 ) {
    alice = atoi(a[1]);
  }

  if (c >= 3) {
    count = atoi(a[2]);
  }
  
  if (c >= 4) {
    ip =a[3];
  }

  tot = TinyOT_new(oe,alice);

  printf("Multirun CAES\n");
  printf("I am: %s\n",alice == 1 ? "Alice" : "Bob");
  printf("ip: %s\n", ip);
  printf("count: %u\n",count);
  pids = (int*)oe->getmem(sizeof(int)*count);

  for( i = 0; i < count; ++i) {
    pids[i] = fork();
    if (pids[i] == 0) {
      return run(material,ip,i,count,oe,tot);
    }
  }
  CHECK_POINT_S("TOTAL");
  for(i = 0;i < count;++i) {
    wait(pids[i]);
  }
  CHECK_POINT_E("TOTAL");
  PrintMeasurements(oe);
}
