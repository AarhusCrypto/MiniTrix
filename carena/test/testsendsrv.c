
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
  uint port = 0;

  InitStats(oe);

  if (c == 1) {
    printf("testsendsrv <port\n");
    return -1;
  }

  port = atoi(a[1]);

  arena = CArena_new(oe);

  printf("Accepting client on port: %u\n",port);
  if (arena->listen(port).rc == 0) {
    
    printf("Client accepted\n");

  }

}
