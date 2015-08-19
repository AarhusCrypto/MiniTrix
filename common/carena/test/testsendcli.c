#include <osal.h>
#include <carena.h>
#include <stdio.h>
#include <coov3.h>
#include <mutex.h>
#include <time.h>
#include <stats.h>


#define SIZE 2048
#define COUNT 64


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
  
int scenario = 6;


int main(int c, char **a) {
  uint i = 0;
  OE oe = OperatingEnvironment_New();
  CArena arena = CArena_new(oe);
  //  ConnectionListener cl = MyCL_new(oe);
  Data s = Data_new(oe,SIZE);
  Data r = Data_new(oe,SIZE);
  ull start = 0;
  uint port = 2020;
  char * ip = "127.0.0.1";
  MpcPeer peer = 0;


  if (s) {
    for(i = 0; i < SIZE;++i) {
      s->data[i] = 0x42;
    }
  }

  InitStats(oe);

  printf("testsendsrv [<ip>] [<port>] default is localhost:2020\n");

  if (c == 2) {
    ip = a[1];
  }

  if (c >= 3) {
    ip = a[1];
    port = atoi(a[2]);
  }

  if (c == 4) {
    scenario = atoi(a[3]);
    if (scenario < 0 || scenario > 6) scenario = 0;
  } 

  printf("Running scenario %u\n", scenario);

  if (arena->connect(ip,port).rc == 0) {
    
    peer = arena->get_peer(0);
    
    for(i = 0;i < COUNT; ++i) {
      printf("%8u",i);fflush(stdout);
      scenarios[scenario].cli(oe, peer, s, r);
      printf("\b\b\b\b\b\b\b\b");
    }
  } else {
    printf("Connection failed\n");
  }
  PrintMeasurements(oe);


  printf("Press any key to terminate\n");
  getchar();
  return 0;

}
