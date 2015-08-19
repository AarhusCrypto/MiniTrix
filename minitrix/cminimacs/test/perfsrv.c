#include <minimacs/minimacs.h>
#include <minimacs/symmetric_minimacs.h>
#include <osal.h>
#include <stats.h>


#define COUNT 256

int main(int c, char **a) {
  OE oe = (OE)OperatingEnvironment_New();
  MiniMacs mm = 0;
  Data input = 0;
  uint count=0,i=0;
  init_polynomial();

  mm = SymmetricMiniMacs_DefaultLoadNew(oe,a[1]);

  InitStats(oe);

  if (!mm) {
    oe->p("Error could not create instance of MiniMacs");
    OperatingEnvironment_Destroy(&oe);
    return -1;
  }
  

  mm->init_heap(2);
  mm->invite(1,8080);
  printf("Got client ... \n");

  input = Data_new(oe, mm->get_ltext());
  for(i = 0;i < mm->get_ltext();++i) {
    input->data[i] = 'r';
  }

  mm->secret_input(0,0,input);

  for(count = 0;count < COUNT;++count) {
    CHECK_POINT_S("Mul server");
    mm->mul(1,0,0);
    CHECK_POINT_E("Mul server");
  }

  usleep(5);
  PrintMeasurements(oe);

  return 0;
}
