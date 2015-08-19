#include <osal.h>
#include <utils/options.h>


/* Main file to run any variant.
 *
 *
 *
 *
 */


int main(int c, char ** a) {
  OE oe = OperatingEnvironment_New();
  Map args = Options_New(oe,c,a);

  

  return 0; 
}
