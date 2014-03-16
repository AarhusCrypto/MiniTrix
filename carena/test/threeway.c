#include <osal.h>
#include <carena.h>
#include <stdio.h>
#include <unistd.h>
#include <coo.h>
#include <mutex.h>
#include <time.h>
#include <stats.h>


int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
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
  sleep(2);
  CArena_destroy(&arena);
  OperatingEnvironment_LinuxDestroy(&oe);

  return 0;
}
