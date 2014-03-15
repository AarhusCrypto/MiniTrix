#include <stdio.h>
#include <osal.h>
#include <minimacs/minimacs.h>
#include <minimacs/generic_minimacs.h>
#include <unistd.h>


static 
int run(char * material, char * ip, uint count) {
  
  OE oe = OperatingEnvironment_LinuxNew();
  MiniMacs mm = GenericMiniMacs_DefaultLoadNew(oe, material);
  
  
  if (mm->get_id() == 0) {
    mm->invite(1,2020+count);
  } else {
    mm->connect(ip,2020+count);
  }
  {
    byte key[128] = {0};
    byte ptxt[128] = {0};
    mpc_aes(mm,ptxt, key);
  }

  return 0;
}

int main(int c, char **a) {
  char * material = 0;
  char * ip = "127.0.0.1";
  uint count = 0, i = 0;

  init_polynomial();
  if (c < 2 || c > 4) {
    printf("multirun <material> <count> [< server >]\n");
    return -1;
  }

  if ( c >= 2 ) {
    material = a[1];
  }

  if (c >= 3) {
    count = atoi(a[2]);
  }
  
  if (c >= 4) {
    ip =a[3];
  }


  printf("Multirun CAES\n");
  printf("material taken from: %s\n",material);
  printf("ip: %s\n", ip);
  printf("count: %u\n",count);

  for( i = 0; i < count; ++i) {
    uint pid;
    pid = fork();
    if (pid != 0) {
      return run(material,ip,i);
    }
  }
}
