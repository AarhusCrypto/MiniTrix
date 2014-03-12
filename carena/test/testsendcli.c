#include <osal.h>
#include <carena.h>
#include <stdio.h>
#include <unistd.h>
#include <coo.h>
#include <mutex.h>
#include <time.h>
#include <stats.h>

#define SIZE 1024
#define COUNT 16

int main(int c, char **a) {
  uint i = 0;
  OE oe = OperatingEnvironment_LinuxNew();
  CArena arena = CArena_new(oe);
  //  ConnectionListener cl = MyCL_new(oe);
  Data s = Data_new(oe,SIZE);
  Data r = Data_new(oe,SIZE);
  ull start = 0;
  uint port = 2020;
  char * ip = "127.0.0.1";
  MpcPeer peer = 0;

  InitStats(oe);

  printf("testsendsrv [<ip>] [<port>] default is localhost:2020\n");

  if (c == 2) {
    ip = a[1];
  }

  if (c == 3) {
    ip = a[1];
    port = atoi(a[2]);
  }


  arena = CArena_new(oe);
 
  if (arena->connect(ip,port).rc == 0) {

    peer = arena->get_peer(0);
    
    for(i = 0;i < COUNT; ++i) {
      peer->send(s);
      peer->receive(r);
    peer->send(s);
    }
  } else {
    printf("Connection failed\n");
  }
  PrintMeasurements(oe);

  return 0;

}
