#include <minimacs/generic_minimacs.h>
#include <minimacs/bitwiseandmulpar_minimacs.h>
#include <osal.h>
#include <config.h>

int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
  MiniMacs mm = 0;
  byte in1[120] = {0};
  byte in2[120] = {0};
  byte one[120] = {0};
  byte zer[120] = {0};
  int i  =0;
  char msg[64] = {0};
  printf("MiniMacs Aarhus University (C)\n");
  init_polynomial();// GFeight intialization 
  mm = GenericMiniMacs_DefaultLoadNew(oe,a[1]);//,a[2]);
  for(i = 0; i < sizeof(one);++i) {
    one[i] = 0xFF;
  }

  if (!mm) {
    oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to create BitWiseAND Mini Macs instance\n");
    return -1;
  }

  {
    uint port = 2020+100*mm->get_id();
    uint wait4=mm->get_no_players()-(mm->get_id()+1);
    osal_sprintf(msg,"Waiting for %u players to connect.",wait4);
    oe->p(msg);
    if (wait4 > 0) {
      if (mm->invite(wait4,port) != 0) {
        oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to listen for parties");
        return -1;
      };
    }
  }

  {
    int id = 0;
    for(id = mm->get_id()-1;id >= 0;--id) {
      uint port = 2020+100*id;
      osal_sprintf(msg,"connecting to %u ...",port);
      oe->p(msg);
      if (mm->connect("127.0.0.1",port) != 0) {
        oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to connect to party");
        return -1;
      }
    }
  }

  mm->init_heap(12);
  printf("Heap initializes\n");

  in1[0] = 0x01;
  //  in2[0] = 0x02;
  
  mm->secret_input(0,0,Data_shallow(in1,mm->get_ltext()));

  //  mm->secret_input(1,1,Data_shallow(in2,mm->get_ltext()));
    
  printf("Done inputting Rasmus\n");
  //  mm->mul(2,0,1);

  mm->open(0);
  //  mm->open(1);
  //x  mm->open(2); 

  dump_data_as_hex(mm->heap_get(0)->codeword, mm->get_lcode(),16);
  /*
  printf("------------------------------------------------------------\n");
  dump_data_as_hex(mm->heap_get(1)->codeword, mm->get_lcode(), 16);
  printf("------------------------------------------------------------\n");
  dump_data_as_hex(mm->heap_get(2)->codeword, mm->get_lcode(), 16);
  */
  return 0;
}
