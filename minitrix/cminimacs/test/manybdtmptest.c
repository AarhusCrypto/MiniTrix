/*

<<Test code>>

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
2014-07-03 06:53: Initial version created
*/

#include <minimacs/generic_minimacs.h>
#include <minimacs/bitwisemulpar2_minimacs.h>
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
  char msg[64] = {0};
  printf("MiniMacs Aarhus University (C)\n");
  init_polynomial();// GFeight intialization 

  if (c != 3) {
    printf("%s <singles> <bdt triples>\n",a[0]);
    return 0;
  }

  mm = BitWiseMulPar2MiniMacs_DefaultLoadFFTNew(oe, a[1], a[2], True);
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
  in1[0] = 0x02;
  in2[0] = 0x03;
  mm->secret_input(0,0,Data_shallow(in1,mm->get_ltext()));
  mm->secret_input(0,1,Data_shallow(in2,mm->get_ltext()));
  mm->mul(2,1,0);

  if (mm->open(2) != 0) {
    printf("Open failed\n");
    return -1;
  };
  dump_data_as_hex(mm->heap_get(2)->codeword, mm->get_ltext(),16);

  return 0;
}
