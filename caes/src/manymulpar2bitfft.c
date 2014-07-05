/*

Copyright (c) 2013, Rasmus Zakarias, Aarhus University
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software

   must display the following acknowledgement:

   This product includes software developed by Rasmus Winther Zakarias 
   at Aarhus University.

4. Neither the name of Aarhus University nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY Rasmus Zakarias at Aarhus University 
''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Rasmus Zakarias at Aarhus University BE 
LIABLE FOR ANY, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Created: 2014-07-03

Author: Rasmus Winther Zakarias, rwl@cs.au.dk

Changes: 
2014-07-03 22:21: Initial version created


This test is hardcoded to run on AU computers with IP address
10.11.82.X where X \in {1,2,3,...,N}. {N} is the number of parties the
preprocessing material provided is prepared for. At the time if
writting this were the addresses of fresh-horses known as llama01,
llama02, llama03 up to llamaN (notice N as bounded from above by 16,
the number of fresh-horses). An AES circuit is executed between the
participating computers such that none of them knows the key.


 */


#include <stdio.h>
#include <osal.h>
#include <minimacs/minimacs_rep.h>
#include <minimacs/minimacs.h>
#include <minimacs/bitwisemulpar2_minimacs.h>
#include <unistd.h>
#include <time.h>
#include <stats.h>
#include <coo.h>


static char * bitlab = "87.104.238.146";
static char * llama01= "10.11.82.1";

/*
 * ] Connect to the monitor
 *
 * ] Listen for clients with ids greater than this client.
 *
 * ] Connect to clients with ids less than this client. (in this way
 *   client 1 connects to no one and listens for every one, vice verse
 *   client N connects to everyone and listens for no one.)
 * 
 * ] Execute mpc_aes with the connected peers
 *
 * ] Destroy the CArena connected to comm with the monitor and leave.
 */
static 
int run(char * ip, uint myid, uint count, OE oe, MiniMacs mm) {
  CArena mc = CArena_new(oe);
  MpcPeer mission_control = 0;

  // connect to monitor 
  if (mc->connect(bitlab, 65000).rc != 0) {
    oe->syslog(OSAL_LOGLEVEL_FATAL,"Failed to connect to the performance monitor.");
    return -1;
  };
  mission_control = mc->get_peer(0);
  
  if (!mission_control) {
    oe->p("Failed connection to mission control. aborting.\n");
    return -1;
  }
 
  // listen for all parties with id greater than mm->myid
  {
    byte msg[92] = {0};
    uint port = 2020+100*mm->get_id();
    uint wait4=mm->get_no_players()-(mm->get_id()+1);
    osal_sprintf(msg,"Waiting for %u players to connect.",wait4);
    oe->p(msg);
    if (wait4 > 0) {
      if (mm->invite(wait4,port) != 0) {
        byte d[256] = {0};
        char m[128] = {0};
        osal_sprintf(m,"Failed to invite %u peers on port %u",wait4,2020+myid);
        oe->syslog(OSAL_LOGLEVEL_FATAL,m);
        i2b(myid, d);
        osal_sprintf(d+4,"error");
        mission_control->send(Data_shallow(d,128));
        return 0;
      };
    }
  }

  // connect to all parties with id less than mm->myid
  {
    int id = 0;
    for(id = mm->get_id()-1;id >= 0;--id) {
      byte address[16] = {0};
      byte msg[92] = {0};
      uint port = 2020+100*id;
      osal_sprintf(msg,"connecting to %u ...",port);
      oe->p(msg);
      osal_sprintf(address,"10.11.82.%d",id+1);
      if (mm->connect(address,port) != 0) {
        byte d[256] = {0};
        char m[128] = {0};
        osal_sprintf(m,"Failed to connect to %s peers on port %u",address,port);
        oe->syslog(OSAL_LOGLEVEL_FATAL,m);
        i2b(myid, d);
        osal_sprintf(d+4,"error");
        mission_control->send(Data_shallow(d,128));
        return 0;
      }
    }
  }

  // invoke AES circuit with zero plaintext and zero key
  {
    byte key[128] = {0};
    byte ptxt[128] = {0};
    mpc_aes(mm,ptxt, key,myid,count,mission_control);
    CArena_destroy(&mc);
  }

  // print time measurements if compiled in
  PrintMeasurements(oe);
  return 0;
}


int main(int c, char **a) {
  char * material = 0, * bdt_material=0;
  char * ip = "127.0.0.1";
  uint count = 0, i = 0;
  OE oe = OperatingEnvironment_LinuxNew();
  MiniMacs mm = 0;
  int * pids = 0;

  InitStats(oe);
  init_polynomial();
  if (c < 3 || c > 5) {
    printf("multirun <material> <bdt_material> <count> [< server >]\n");
    return -1;
  }

  if ( c >= 3 ) {
    material = a[1];
    bdt_material = a[2];
  }

  if (c >= 4) {
    count = atoi(a[3]);
  }
  
  if (c >= 5) {
    ip =a[4];
  }

  // loads the preprocessing material making MiniMac ready 
  mm=BitWiseMulPar2MiniMacs_DefaultLoadFFTNew(oe, material, bdt_material, True);

  printf("Multirun CAES\n");
  printf("material taken from: %s\n",material);
  printf("ip: %s\n", ip);
  printf("count: %u\n",count);
  pids = (int*)oe->getmem(sizeof(int)*count);

  // create processes for parallel execution ({count} of them)
  for( i = 0; i < count; ++i) {
    pids[i] = fork();
    if (pids[i] == 0) {
      return run(ip,i,count,oe,mm);
    }
  }

  // wait for everybody to complete
  CHECK_POINT_S("TOTAL");
  for(i = 0;i < count;++i) {
    wait(pids[i]);
  }
  CHECK_POINT_E("TOTAL");
  PrintMeasurements(oe);
}
