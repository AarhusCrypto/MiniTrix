#include <stats.h>


int main(int c, char **args) {
  OE oe = OperatingEnvironment_New();
  int i=0,a=0;
  InitStats(oe);

  CHECK_POINT_S(__FUNCTION__);

  for( i = 0; i < 1024;++i) {
    a+=i*i;
  }

  
  CHECK_POINT_E(__FUNCTION__);

  PrintMeasurements(oe);

  OperatingEnvironment_Destroy(&oe);

  return 0;
}
