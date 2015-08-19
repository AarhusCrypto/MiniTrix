#include <carena.h>
#include <stdio.h>

int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();
  CArena arena = CArena_new(oe);
  MpcPeer peer = 0;
  Data id = Data_new(oe, 256);
  char * ip = "87.104.238.146";

  if (c == 2) 
    ip = a[1];
  
  
  if (arena->connect(ip,65000).rc != RC_OK) {
    printf("Connection failed\n");
    return -1;
  }

  peer = arena->get_peer(0);
  peer->send(id);
  peer->receive(id);
  peer->send(id);
  CArena_destroy(&arena);
  OperatingEnvironment_Destroy(&oe);

  printf("Press any key to terminate.\n");
  getchar();
  return -1;
}
