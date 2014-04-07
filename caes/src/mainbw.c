
#include <stdio.h>
#include <minimacs/minimacs_rep.h>
#include <minimacs/bitwiseand_minimacs.h>
#include <osal.h>
#include <carena.h>
#include <caes.h>
#include <stats.h>


int main(int c, char **a) {

  printf("Aarhus University - Multiparty Computation AES\n");
  printf("All rights reserved (C)\n");

  if (c == 2 || c == 3 || c == 4) {
    MiniMacsRep * singles = 0;
    MiniMacsRep ** pairs = 0;
    MiniMacsTripleRep * triples;
    uint lsingles = 0;
    uint lpairs = 0;
    uint ltriples = 0;
    MiniMacs comp = 0;
    CArena arena = 0;
    OE oe = 0;
    uint myid = 0;
    char * ipaddr = "any";
    uint port = 2020;

    init_polynomial();

    if (c == 1) {
      printf("caes <raw_material> <bdt triples> [<client>] \n");
      return 0;
    }

    oe = OperatingEnvironment_LinuxNew();
    
    comp = BitWiseANDMiniMacs_DefaultLoadNew(oe,a[1],a[2]);
    
    if (!comp) {
      oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to create comp instance.");
      return -1;
    }
  
    if (comp->get_id() == 0) {
      if (comp->invite(1,2020) != 0) {
        oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to listen for parties");
        return -1;
      };
    } else {
      if (comp->connect("127.0.0.1",2020) != 0) {
        oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to connect to party");
        return -1;
      }
    }
    {
      byte key[128] = {0};
      byte pltxt[128] = {0};
      
      mpc_aes(comp,pltxt,key,0,0,0);
    }
  }
  return 0;
}
