#include <carena.h>
#include <map.h>
#include <hashmap.h>
#include <osal.h>
#include <stdio.h>
#include <string.h>
#include <coo.h>
#include <time.h>
#include <singlelinkedlist.h>
#include <stdarg.h>
#include <unistd.h>
MUTEX lock;
List measures;
CArena arena;
FILE * _log;
uint no_connected;

typedef struct _c_arg_ {
  OE oe ;
  MpcPeer peer ;
  CArena arena;

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
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
}

static 
void writeln(const char * fmt, ... ) {
  va_list l = {{0}};
  byte buf[256] = {0};
  if (!_log) return;
  va_start(l,fmt);
  vfprintf(_log,fmt,l);
  va_end(l);
  fflush(_log);
}

static 
void * handle_client(void  * a) {
  CArg arg = (CArg)a;
  MpcPeer peer = arg->peer;
  OE oe = arg->oe;
  Data in = Data_new(oe, 256);
  uint pid = 0, cid = 0, count = 0;
  
  Key key = oe->getmem(sizeof(*key));
  ull start = 0, end = 0, dur = 0;
  Measure m = oe->getmem(sizeof(*m));

  peer->receive(in);
  pid = b2i(in->data);
  cid = b2i(in->data+4);
  count = b2i(in->data+8);
  m->start = _nano_time();
  printf(" [Measuring %u %u/%u]\n",pid,cid,count);
  writeln("[Measuring %u %u/%u]\n",pid,cid,count);

  oe->lock(lock);
  measures->add_element(m);
  no_connected++;
  printf("%u connected so far\n",no_connected);
  oe->unlock(lock);

  oe->lock(lock);
  while(no_connected < 2*count) { // one connection for client and server
    oe->unlock(lock);
    usleep(200);
    oe->lock(lock);
  }
  writeln("[All Ready] BANG !\n");
  printf( "[All Ready] BANG !\n");
  peer->send(in);
  printf("Sendt BANG\n");
  oe->unlock(lock);
  

  peer->receive(in);
  pid = b2i(in->data);
  m->stop = _nano_time();

  printf("[Peer done %u]\n",pid);

  oe->lock(lock);
  dur = m->stop - m->start;
  printf( "[Completed run %u total runs %u\n",pid, measures->size());
  writeln("[Completed run %u total runs %u\n",pid, measures->size());
  writeln("%u,%llu,%llu,%llu\n",pid,m->start, m->stop, dur / 1000000);
  oe->unlock(lock);
  

  oe->putmem(a);
  return 0;
}

COO_DCL(ConnectionListener, void, client_connected, MpcPeer peer);
COO_DEF_NORET_ARGS(ConnectionListener, client_connected, MpcPeer peer;, peer) {
  CArg arg = this->oe->getmem(sizeof(*arg));
  arg->oe = this->oe;
  arg->peer = peer;
  this->oe->newthread(handle_client, arg);
}


COO_DCL(ConnectionListener, void, client_disconnected, MpcPeer peer);
COO_DEF_NORET_ARGS(ConnectionListener, client_disconnected, MpcPeer peer;, peer) {
  
  // nothing !
  
}

ConnectionListener CL_new(OE oe) {
  ConnectionListener cl = (ConnectionListener)oe->getmem(sizeof(*cl));
  COO_ATTACH(ConnectionListener, cl, client_connected);
  COO_ATTACH(ConnectionListener, cl , client_disconnected);
  cl->oe = oe;
  return cl;
}

int main(int c, char **a) {
  uint i  =0;
  OE oe = OperatingEnvironment_LinuxNew();
  ConnectionListener cl = CL_new(oe);
  _log = fopen("mission_control.log","a+");
  measures = SingleLinkedList_new(oe);
  arena = CArena_new(oe);  
  lock = oe->newmutex();

  arena->add_conn_listener(cl);

  printf("MiniTrix Monitor version 0.2\n");

  if (arena->listen(65000).rc == 0) {
    printf("Monitor active\n"); 
  }
  
  while(1) {
    char c = getchar();
    if (c == 'q') {
      fclose(_log);
      return 0;
    }
    oe->lock(lock);
    for(i = 0; i < measures->size();++i) {
      Measure m = measures->get_element(i);
      ull dur = m->stop - m->start;
      printf("%u,%llu,%llu,%llu\n",i,m->start, m->stop, dur / 1000000);
    }
    printf("\n\n");
    oe->unlock(lock);
    
  }

  CArena_destroy(&arena);
  OperatingEnvironment_LinuxDestroy(&oe);
  return 0;
}
