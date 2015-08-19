#include <minimacs/minimacs.h>
#include <minimacs/symmetric_minimacs.h>
#include <osal.h>
#include <stats.h>

#include <unistd.h>

#define COUNT 256

int main(int c, char **a) {
  OE oe = (OE)OperatingEnvironment_New();
  MiniMacs mm = 0;
  Data input = 0;
  uint count=0,i=0;
  init_polynomial();
  char * ip = "127.0.0.1";

  mm = SymmetricMiniMacs_DefaultLoadNew(oe,a[1]);

  InitStats(oe);

  if (!mm) {
    oe->p("Error could not create instance of MiniMacs");
    OperatingEnvironment_Destroy(&oe);
    return -1;
  }


  mm->init_heap(2);
  if (c == 3)
    ip = a[2];

  mm->connect(ip,8080);

  mm->secret_input(0,0,0);

  for(count = 0;count < COUNT;++count) {
    CHECK_POINT_S("\033[01mMul Client\033[00m");
    mm->mul(1,0,0);
    CHECK_POINT_E("\033[01mMul Client\033[00m");
  }

  PrintMeasurements(oe);

  
  return 0;
}
