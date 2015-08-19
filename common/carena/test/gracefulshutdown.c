#include <carena.h>

#include <stdio.h>
#include <unistd.h>
int main(int c, char **a) {
  
  OE oe = OperatingEnvironment_New();
  CArena arena = CArena_new(oe);

  arena->listen(2020);
  
  while(!arena->get_no_peers()) {
    char m[32] = {0};
    sprintf(m,"peer %d\n",arena->get_no_peers());
    oe->p(m);
    sleep(1);
  }

  CArena_destroy( & arena );
  OperatingEnvironment_Destroy(&oe);
  return 0;
}
