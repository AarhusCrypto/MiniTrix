#include <osal.h>
#include <carena.h>
#include <stdio.h>
#include <mutex.h>
#include <stats.h>


int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();
  CArena arena = CArena_new(oe);
  uint myid = 0;
  uint count = 0;
  uint id = 0;

  if (c != 3) {
    printf("threeway <myid> <count>\n");
    return -1;
  }

  myid = atoi(a[1]);
  count = atoi(a[2]);

  // listen for count-1-myid connections
  if (arena->listen_wait(count-1-myid, 2020+myid).rc != OK) {
    printf("Failed to listen on port %u\n",2020+myid);
    return -1;
  }

  for(id = 0;id < myid;++id) {
    if (arena->connect("127.0.0.1",2020+id).rc != OK) {
      printf("Failed to connect to port %u\n",2020+id);
      return -1;
    }
  }

  printf("All peers connected\n");

  for(id = 0;id < count-1;++id) {
    byte b[4] = {0};
    MpcPeer peer = arena->get_peer(id);
    if (!peer) {
      oe->syslog(OSAL_LOGLEVEL_FATAL,"Error peer is null ;(");
      return -1;
    }
    peer->send(Data_shallow("1234", 4));
    peer->receive(Data_shallow(b, 4));
  }

  oe->p("Everything done");

  CArena_destroy(&arena);
  OperatingEnvironment_Destroy(&oe);

  return 0;
}
