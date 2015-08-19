#include <minimacs/bitwiseand_minimacs.h>
#include <osal.h>
#include <config.h>

int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();
  MiniMacs mm = 0;
  byte in1[120] = {0};
  byte in2[120] = {0};
  byte one[120] = {0};
  byte zer[120] = {0};
  int i  =0;
  printf("MiniMacs Aarhus University (C)\n");
  init_polynomial();// GFeight intialization 
  mm = BitWiseANDMiniMacs_DefaultLoadNew(oe,a[1],a[2]);
  for(i = 0; i < sizeof(one);++i) {
    one[i] = 0xFF;
  }

  if (!mm) {
    oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to create BitWiseAND Mini Macs instance\n");
    return -1;
  }

  if (mm->get_id() == 0) {
    if (mm->invite(1,2020) != 0) {
      oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to listen for parties");
      return -1;
    };
  } else {
    if (mm->connect("127.0.0.1",2020) != 0) {
      oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to connect to party");
      return -1;
    }
  }
  mm->init_heap(8);
  printf("Heap initializes\n");
  in1[0] = 0xff;
  in1[3] = 0xaa;
  in2[0] = 0x55;
  in2[42] = 0xAA;
  mm->public_input(6,Data_shallow(zer,120));
  mm->public_input(7,Data_shallow(one,120));
  mm->secret_input(0,0,Data_shallow(in1,120));
  mm->secret_input(1,1,Data_shallow(in2,120));
  printf("Done inputting Rasmus\n");
  mm->mul(2,1,0);
  mm->add(6,7,0);
  mm->open(0);
  mm->open(1);
  mm->open(2);
  mm->open(6);

  dump_data_as_hex(mm->heap_get(0)->codeword, 256,16);
  printf("------------------------------------------------------------\n");
  dump_data_as_hex(mm->heap_get(1)->codeword, 256, 16);
  printf("------------------------------------------------------------\n");
  dump_data_as_hex(mm->heap_get(2)->codeword, 256, 16);
  printf("------------------------------------------------------------\n");
  dump_data_as_hex(mm->heap_get(6)->codeword, 256, 16);
  
  return 0;
}
