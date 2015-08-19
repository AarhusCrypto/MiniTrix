#include <osal.h>
#include <carena.h>


int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();

  CArena arena = CArena_new(oe);

  arena->listen(2020);

  CArena_destroy(&arena);

  OperatingEnvironment_Destroy(&oe);
  return 0;

}
