#include <osal.h>
#include <carena.h>
#include <stdio.h>
#include <unistd.h>
#include <coo.h>
#include <mutex.h>
#include <time.h>
#include <stats.h>


/*
 C                S
     SEND           REC 
     REC            SEND
     SEND           REC 
 */

/*



 */

// 256 bytes

MUTEX m = 0;
#define COUNT 128
#define SIZE 256



#define S_SCENARIO srv_scenario_7
#define C_SCENARIO cli_scenario_7

static
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
}


// ------------------------------------------------------------
// scenario 1: cli receive, send
//             src send, receive
//
static
cli_scenario_1(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
  peer->send(s);
}

static 
srv_scenario_1(OE oe, MpcPeer peer, Data s, Data r) {
  peer->send(s);
  peer->receive(r);
}


// ------------------------------------------------------------
// scenario 2: cli receives twice
//             srv send twice
// ------------------------------------------------------------
static 
cli_scenario_2(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
  peer->receive(r);
}

static 
srv_scenario_2(OE oe, MpcPeer peer, Data s, Data r) {
  peer->send(s);
  peer->send(s);
}

// ------------------------------------------------------------
// scenario 3: cli send twice
//             srv receive twice
// ------------------------------------------------------------
static 
cli_scenario_3(OE oe, MpcPeer peer, Data s, Data r) {
  peer->send(s);
  peer->send(s);
}

static 
srv_scenario_3(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
  peer->receive(r);
}

// ------------------------------------------------------------
// scenario 4: cli send, receive
//             srv receive, send
// ------------------------------------------------------------
static 
cli_scenario_4(OE oe, MpcPeer peer, Data s, Data r) {
  peer->send(s);
  peer->receive(r);
}

static 
srv_scenario_4(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
  peer->send(s);
}

// ------------------------------------------------------------
static 
cli_scenario_5(OE oe, MpcPeer peer, Data s, Data r) {
  peer->send(s);
}

static 
srv_scenario_5(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
}

// ------------------------------------------------------------
static 
cli_scenario_6(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
}

static 
srv_scenario_6(OE oe, MpcPeer peer, Data s, Data r) {
  peer->send(s);
}

// ------------------------------------------------------------
static
cli_scenario_7(OE oe, MpcPeer peer, Data s, Data r) {
  peer->send(s);
  peer->receive(r);
  peer->send(s);
}

static srv_scenario_7(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
  peer->send(s);
  peer->receive(r);
}

struct scenario {
  int (*cli)(OE, MpcPeer, Data, Data);
  int (*srv)(OE, MpcPeer, Data, Data);
};

struct scenario scenarios[] = { 
  {cli_scenario_1, srv_scenario_1},
  {cli_scenario_2, srv_scenario_2},
  {cli_scenario_3, srv_scenario_3},
  {cli_scenario_4, srv_scenario_4},
  {cli_scenario_5, srv_scenario_5},
  {cli_scenario_6, srv_scenario_6},
  {cli_scenario_7, srv_scenario_7},
};
  
int scenario = 0;

static 
void * client (void * a) {
  OE oe = (OE)a;
  CArena arena = CArena_new(oe);
  MpcPeer peer = 0;
  Data s = Data_new(oe,SIZE);
  Data r = Data_new(oe,SIZE);
  int i = 0;
  
  for(i = 0;i < s->ldata;++i) {
    s->data[i] = 0x42;
  }

  arena->connect("127.0.0.1",2021);
  
  peer = arena->get_peer(0);
  for(i = 0;i < COUNT;++i){
    CHECK_POINT_S("Client");
    // ------------------------------------------------------------
    scenarios[scenario].cli(oe,peer,s,r);
    // ------------------------------------------------------------
    CHECK_POINT_E("Client");
  }    

  return arena;
}

COO_DCL(ConnectionListener, void, client_connected, MpcPeer peer) 
COO_DEF_NORET_ARGS(ConnectionListener, client_connected, MpcPeer peer;,peer) {
  ConnectionListener l = (ConnectionListener)this;
  OE oe = l->oe;
  oe->unlock(m);
  oe->p("Client unlocking");
  return;
}}

COO_DCL(ConnectionListener, void, client_disconnected, MpcPeer peer)
COO_DEF_NORET_ARGS(ConnectionListener, client_disconnected, MpcPeer peer;,peer) {
  ConnectionListener l = (ConnectionListener)this;
  OE oe = l->oe;
  oe->p("Peer disconnected");
  return ;
}}

ConnectionListener MyCL_new(OE oe) {
  ConnectionListener l = oe->getmem(sizeof(*l));
  l->oe = oe;
  COO_ATTACH(ConnectionListener,l,client_connected);
  COO_ATTACH(ConnectionListener,l,client_disconnected);
  return l;
}



int main(int c, char **a) {
  uint i = 0;
  OE oe = OperatingEnvironment_LinuxNew();
  CArena arena = CArena_new(oe);
  ConnectionListener cl = MyCL_new(oe);
  ThreadID tid = 0;
  CArena other = 0;
  MpcPeer peer = 0;
  Data s = Data_new(oe,SIZE);
  Data r = Data_new(oe,SIZE);
  ull start = 0;

  InitStats(oe);

  if (c == 2) {
    int val = atoi(a[1]);
    if (val > 0 && val < 8) {
      scenario = val - 1;
    }
  }

  for(i = 0; i < COUNT; ++i ) {
    CHECK_POINT_S("Sanity");
    usleep(500);
    CHECK_POINT_E("Sanity");
  }

  arena->add_conn_listener(cl);

  m = oe->newmutex();

  oe->lock(m);
  arena->listen(2021);
  oe->p("Listening on 2021 for clients");
  tid = oe->newthread(client, oe);
  oe->lock(m);

  peer = arena->get_peer(0);

  for(i = 0;i < s->ldata;++i) {
    s->data[i] = 0x61;
  }

  printf("-------------------- SCENARIO --------------------\n");
  { 
    ull ms = 0;
    start = _nano_time();
    for(i = 0; i < COUNT;++i) {
      CHECK_POINT_S("Server");
      // ------------------------------------------------------------
      scenarios[scenario].srv(oe,peer,s,r);
      // ------------------------------------------------------------
      CHECK_POINT_E("Server");
    }
    ms = (_nano_time() - start)/1000000L;
 
    printf("Communication Complexity: %u bytes in total, %u ms %u bytes/ms\n", COUNT*SIZE, ms, (ms > 0 ? (COUNT*SIZE)/ms : 0));
 
  }

  
  other = (CArena)oe->jointhread(tid);

  printf("%p \n", other);

  PrintMeasurements(oe);

  printf(oe->get_version());printf("\n");


  return 0;
}
