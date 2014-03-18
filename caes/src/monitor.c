#include <carena.h>
#include <map.h>
#include <hashmap.h>
#include <osal.h>
#include <stdio.h>
#include <string.h>
#include <coo.h>

MUTEX lock;
Map clients;

typedef struct _c_arg_ {
  OE oe ;
  MpcPeer peer ;
} *CArg;

typedef struct _measure_ {
  ull start;
  ull stop;
} * Measure;

typedef struct _key_ {
  char * ip;
  uint pid;
} * Key;

static
uint hashcode_key_fh(void * a) {
  Key k = (Key)a;
  uint lip = 0;
  uint hashcode = 0;
  uint i = 0;

  if (k->ip) {
    lip=strlen(k->ip);
    for(i = 0;i < lip;++i) {
      hashcode += 65537*k->ip[i];
    }
  }

  hashcode += 2147483647*k->pid;
  
  return hashcode;
}

static 
int compare_key_fn(void * a, void * b) {
  Key ka = (Key)a;
  Key kb = (Key)b;
  int res = 0;
  uint kalip = 0;
  uint kblip = 0,lip=0;

  if (!a && !b) return 0;
  if (!a && b) return -1;
  if (a && !b) return 1;
  
  kalip = strlen(ka->ip);
  kblip = strlen(kb->ip);

  lip = kalip > kblip ? kblip : kalip;

  res = memcmp(ka->ip, kb->ip,lip);
  if (res) return res;

  return ka->pid > kb->pid ? 1 : (ka->pid < kb->pid ? -1 : 0);
}

static 
void * handle_client(void  * a) {
  CArg arg = (CArg)a;
  MpcPeer peer = arg->peer;
  OE oe = arg->oe;
  Data in = Data_new(oe, 4);
  uint pid = 0;

  peer->receive(in);
  pid = b2i(in->data);

  oe->lock(lock);
  //  clients->put(

  oe->putmem(a);
}

COO_DCL(ConnectionListener, void, client_connected, MpcPeer peer);
COO_DEF_NORET_ARGS(ConnectionListener, client_connected, MpcPeer peer;, peer) {
  CArg arg = this->oe->getmem(sizeof(*arg));
  arg->oe = this->oe;
  arg->peer = peer;
  this->oe->newthread(handle_client, arg);
}}


COO_DCL(ConnectionListener, void, client_disconnected, MpcPeer peer);
COO_DEF_NORET_ARGS(ConnectionListener, client_disconnected, MpcPeer peer;, peer) {
  
  // nothing !
  
}}

ConnectionListener CL_new(OE oe) {
  ConnectionListener cl = (ConnectionListener)oe->getmem(sizeof(*cl));
  COO_ATTACH(ConnectionListener, cl, client_connected);
  COO_ATTACH(ConnectionListener, cl , client_disconnected);
  cl->oe = oe;
  return cl;
}

int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
  CArena arena = CArena_new(oe);
  clients = HashMap_new(oe, hashcode_key_fh, compare_key_fn, 8);

  printf("MiniTrix Monitor version 0.1\n");

  if (arena->listen(65000).rc == 0) {
    printf("Monitor active\n"); 
  }

  printf("Press any key to terminate.");
  getchar();

  CArena_destroy(&arena);
  OperatingEnvironment_LinuxDestroy(&oe);
  return 0;
}
