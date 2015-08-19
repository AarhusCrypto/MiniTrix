#include <carena.h>
#include <osal.h>
#include <stdio.h>
#include <coov4.h>


MUTEX m = 0;
void * t_connect(void * a) {
  OE oe = (OE)a;
  CArena arena = CArena_new(oe);
  MpcPeer peer = 0;

  oe->p("Connecting with CArena");
  arena->connect("127.0.0.1", 2021);
  
  oe->p("Getting peer");
  peer = arena->get_peer(0);

  if (peer) {
    oe->p("Got peer");
    peer->send(Data_shallow((byte*)"Rasmus",7));
  } else {
    oe->p("No peer");
  }

  oe->p("Leaving and destroying CArena");

  return 0;
}

COO_DCL(ConnectionListener, void, client_connected, MpcPeer peer) 
COO_DEF_NORET_ARGS(ConnectionListener, client_connected, MpcPeer peer;,peer) {
  ConnectionListener l = (ConnectionListener)this;
  OE oe = l->oe;
  oe->unlock(m);
  oe->p("Client unlocking");
  return;
}

COO_DCL(ConnectionListener, void, client_disconnected, MpcPeer peer)
COO_DEF_NORET_ARGS(ConnectionListener, client_disconnected, MpcPeer peer;,peer) {
  ConnectionListener l = (ConnectionListener)this;
  OE oe = l->oe;
  oe->p("Peer disconnected");
  return ;
}

ConnectionListener MyCL_new(OE oe) {
  ConnectionListener l = oe->getmem(sizeof(*l));
  l->oe = oe;
  COO_ATTACH(ConnectionListener,l,client_connected);
  COO_ATTACH(ConnectionListener,l,client_disconnected);
  return l;
}

int main(int c, char **a) {
  CArena arena = 0;
  OE oe = 0;
  ThreadID tid = 0;
  Data in = 0;
  ConnectionListener cl = 0;
  oe = OperatingEnvironment_New();

  oe->syslog(OSAL_LOGLEVEL_WARN, "Testing");
  if (!oe) {
    printf("Failure could not create OE\n");return -1;
  }
  arena = CArena_new(oe);
  
  if (!arena) {
    printf("Failure could not create Arena.\n");return -2;
  } else {
    oe->syslog(OSAL_LOGLEVEL_WARN,"Created arena");
  }
  cl = MyCL_new(oe);
  printf("%p\n",&cl);
  m = oe->newmutex();
  arena->add_conn_listener(cl);
  arena->listen(2021);
  oe->p("After listen 2021");
  sched_yield();
  sleep(1);
  tid = oe->newthread(t_connect, oe);

  in = Data_new(oe,7);

  oe->lock(m);
  oe->lock(m);
  oe->p("Beoynd the great lock");
  arena->get_peer(0)->receive(in);
  printf("%s\n",in->data);
  oe->jointhread(tid);

  printf("%d %p\n", tid, in);

  CArena_destroy( &arena );

  OperatingEnvironment_Destroy( &oe );
  return 0;
}
