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
  uint pid = getpid();
  Data _pid = Data_new(oe,4);
  MpcPeer mission_control = 0;
  i2b(pid, _pid->data);
     
  mc->connect("127.0.0.1", 65000);
  mission_control = mc->get_peer(0);
  
  if (!mission_control) {
    oe->p("Failed connection to mission control. aborting.\n");
    Data_destroy(oe,& _pid);
    return -1;
  }
    mc->get_peer(0)->send(_pid);

  
  
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
    mpc_aes(mm,ptxt, key);
    mission_control->send(_pid);
    CArena_destroy(&mc);
  }
  Data_destroy(oe,& _pid);
  return 0;
}




COO_DCL(ConnectionListener, void, client_connected, MpcPeer peer);
COO_DEF_NORET_ARGS(ConnectionListener, client_connected, MpcPeer peer;, peer) {
  uint pid = 0;
  Data _pid = Data_new(this->oe, 4);
  char m[128] = {0};
  peer->receive(_pid);
  pid = b2i(_pid->data);
  Data_destroy(this->oe,&_pid);
  osal_sprintf(m,"[MissionControl] Peer with pid %u checked in and started computation.\n",pid);
  this->oe->p(m);
}}


COO_DCL(ConnectionListener, void, client_disconnected, MpcPeer peer);
COO_DEF_NORET_ARGS(ConnectionListener, client_disconnected, MpcPeer peer;,peer) {
  
  this->oe->p("[MissionControl] Peer disconnected.");

}}


// skriv til Svend
ConnectionListener MCCL_new(OE oe) {
  ConnectionListener cl = (ConnectionListener)oe->getmem(sizeof(*cl));
  COO_ATTACH(ConnectionListener, cl, client_connected);
  COO_ATTACH(ConnectionListener, cl, client_disconnected);
  cl->oe = oe;
  return cl;
}


int main(int c, char **a) {
  char * material = 0;
  char * ip = "127.0.0.1";
  uint count = 0, i = 0;
  OE oe = OperatingEnvironment_LinuxNew();
  MiniMacs mm = 0;
  int * pids = 0;
  CArena mission_control = 0;
  ConnectionListener cl = MCCL_new(oe);
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
  mission_control = CArena_new(oe);
  mission_control->add_conn_listener(cl);
  mission_control->listen(65000);

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
