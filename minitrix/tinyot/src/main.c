#include <osal.h>
#include <tinyot.h>


int main(int c, char **args) {

  OE oe = OperatingEnvironment_New();
  TinyOT alice = TinyOT_new(oe, 1);
  TinyOT bob = TinyOT_new(oe, 0);

  alice->invite(2020);
  bob->connect("127.0.0.1",2020);
  

  return 0;
}
