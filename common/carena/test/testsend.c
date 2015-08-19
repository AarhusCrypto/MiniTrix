#include <osal.h>
#include <carena.h>
#include <stdio.h>
#include <coov4.h>
#include <mutex.h>
#include <time.h>
#include <stats.h>
#include <datetime.h>
#include <testcase.h>

#ifndef WINDOWS
#include <unistd.h>
#endif

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
bool check_rcv(Data r) {
  bool res = True;
  uint i = 0;
  if (!r) return False;
  if (r->ldata < SIZE) return False;
  for(i = 0;i < SIZE;++i) {
    if (r->data[i] != 0x42) return False;
  }
  return True;
}

/*
static
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
}
*/


// ------------------------------------------------------------
// scenario 1: cli receive, send
//             src send, receive
//
static
cli_scenario_1(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
  peer->send(s);
}

static 
srv_scenario_1(OE oe, MpcPeer peer, Data s, Data r) {
  peer->send(s);
  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
}


// ------------------------------------------------------------
// scenario 2: cli receives twice
//             srv send twice
// ------------------------------------------------------------
static 
cli_scenario_2(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
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
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
}

// ------------------------------------------------------------
// scenario 4: cli send, receive
//             srv receive, send
// ------------------------------------------------------------
static 
cli_scenario_4(OE oe, MpcPeer peer, Data s, Data r) {
  peer->send(s);
  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
}

static 
srv_scenario_4(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
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
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
}

// ------------------------------------------------------------
static 
cli_scenario_6(OE oe, MpcPeer peer, Data s, Data r) {
  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
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
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
  peer->send(s);
}

static srv_scenario_7(OE oe, MpcPeer peer, Data s, Data r) {

  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }

  peer->send(s);

  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }

}



static
cli_scenario_8(OE oe, MpcPeer peer, Data s, Data r) {
  peer->send(s);
  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
  peer->send(s);
  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }
}

static srv_scenario_8(OE oe, MpcPeer peer, Data s, Data r) {

  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }

  peer->send(s);

  peer->receive(r);
  if (check_rcv(r) != True) { printf("Crap wrong data\n"); }

  peer->send(s);

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
  {cli_scenario_8, srv_scenario_8}
};
  
int scenario = 0;


static 
void * client (void * a) {
  OE oe = (OE)a;
  CArena arena = CArena_new(oe);
  MpcPeer peer = 0;
  Data s = Data_new(oe,SIZE);
  Data r = Data_new(oe,SIZE);
  uint i = 0;
  
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

  CArena_destroy(&arena);
  return 0;
}

COO_DEF(ConnectionListener, void, client_connected, MpcPeer peer) 
  ConnectionListener l = (ConnectionListener)this;
  OE oe = l->oe;
  oe->unlock(m);
  oe->p("Client unlocking");
  return;
}

COO_DEF(ConnectionListener, void, client_disconnected, MpcPeer peer)
  ConnectionListener l = (ConnectionListener)this;
  OE oe = l->oe;
  oe->p("Peer disconnected");
  return ;
}

ConnectionListener MyCL_new(OE oe) {
  ConnectionListener l = oe->getmem(sizeof(*l));
  l->oe = oe;
  l->client_connected = COO_attach(l, ConnectionListener_client_connected);
  l->client_disconnected = COO_attach(l, ConnectionListener_client_disconnected);
  return l;
}



static int test_send_recv(OE oe) {
  uint i = 0;
  CArena arena = CArena_new(oe);
  ConnectionListener cl = MyCL_new(oe);
  ThreadID tid = 0;
  CArena other = 0;
  MpcPeer peer = 0;
  Data s = Data_new(oe,SIZE);
  Data r = Data_new(oe,SIZE);
  ull start = 0;

  InitStats(oe);

  arena->add_conn_listener(cl);

  if (oe->newmutex(&m) != RC_OK) goto fail;

  oe->lock(m);
  oe->newthread(&tid, client, oe);
  arena->listen(2021);
  oe->p("Listening on 2021 for clients");
  oe->lock(m);

  peer = arena->get_peer(0);

  printf("Got client ... \n");
  for(i = 0;i < s->ldata;++i) {
    s->data[i] = 0x42;
  }

  
  printf("-------------------- SCENARIO --------------------\n");
  {
	  DateTime dt = DateTime_New(oe);
    ull ms = 0;
    start = dt->getNanoTime();
    for(i = 0; i < COUNT;++i) {
      CHECK_POINT_S("Server");
      // ------------------------------------------------------------
      scenarios[scenario].srv(oe,peer,s,r);
      // ------------------------------------------------------------
      CHECK_POINT_E("Server");
    }
    ms = (dt->getNanoTime()- start)/1000000L;
 
    printf("Communication Complexity: %u bytes in total, %u ms %u bits/s\n", COUNT*SIZE, ms, (ms > 0 ? (COUNT*SIZE*8000)/ms : 0));
 
  }

  
  (CArena)oe->jointhread(tid);

  printf("%p \n", other);

  PrintMeasurements(oe);

  printf(oe->get_version());printf("\n");

  CArena_destroy(&arena);
  return 1;
fail:
  CArena_destroy(&arena);
  return 0;
}

static Test tests[] = {
	{ "Test send and receive", test_send_recv }
};

static TestSuit test_and_send_suit = {
	"Test send and receive",
	0, 0,
	tests, sizeof(tests) / sizeof(Test)
};

TestSuit * get_test_and_send_suit(OE oe) {
	return &test_and_send_suit;
}