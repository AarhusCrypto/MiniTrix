/*
  Copyright (c) 2013, Rasmus Lauritsen, Aarhus University
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
  This product includes software developed by the Aarhus University.
  4. Neither the name of the Aarhus University nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY Rasmus Lauritsen at Aarhus University ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL Rasmus Lauritsen at Aarhus University BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


  Author: Rasmus Winther Lauritsen, rwl@cs.au.dk

  Changes: 
  2014-02-28 Initial version created
*/
#include <osal.h>
#include <minimacs/minimacs.h>
#include <minimacs/generic_minimacs.h>
#include <minimacs/bitwisemulpar2_minimacs.h>
#include <math/polynomial.h>
#include <utils/options.h>
#include "ast.h"
#define YYSTYPE AstNode
#include "y.tab.h"
#include "interp.h"
#include "config.h"
extern AstNodeFactory anf;
extern FILE * yyin;
extern int error;
extern AstNode root;
#define VARIANT_GENERIC_MINIMACS 1
#define VARIANT_GENEFFT_MINIMACS 2
#define VARIANT_BWMPMXT_MINIMACS 3
#define VARIANT_BWMPFFT_MINIMACS 4

static
uint cstr_len(char * a){ 
  uint i = 0;
  if (!a) return 0;
  while( *(a+i) ) ++i;
  return i;
}

static 
bool cstr_eq(char *a, char *b) {
  uint la = cstr_len(a);
  uint lb = cstr_len(b);

  if (la != lb) return False;

  la = 0;
  while(la < lb) {
    if (a[la] != b[la]) return False;
    ++la;
  }

  return True;
}

static 
int get_variant(Map a) {
  
  if (a->contains("kind")) {
    char * v = a->get("kind");
    if (cstr_eq("generic",v)) return VARIANT_GENERIC_MINIMACS;
    if (cstr_eq("genefft",v)) return VARIANT_GENEFFT_MINIMACS;
    if (cstr_eq("bwmpfft",v)) return VARIANT_BWMPFFT_MINIMACS;
    if (cstr_eq("bwmpmxt",v)) return VARIANT_BWMPMXT_MINIMACS;
    a->rem("kind");
  }

  // default variant
  return VARIANT_GENERIC_MINIMACS;
}

static 
char * get_arg(char * key, Map arguments) {
  if (arguments->contains(key)) {
    char * res = (char *)arguments->get(key);
    arguments->rem(key);
    return res;
  }
  return 0;
}

static
void _error(OE oe, char * m) {
  oe->syslog(OSAL_LOGLEVEL_FATAL,m);
}


static
MiniMacs make_minimacs(OE oe, Map arguments) {
  int variant = get_variant(arguments);
  MiniMacs mm = 0;
  char * std_pre_file = get_arg("pre", arguments);
  char * bdt_pre_file = get_arg("bdt", arguments);
  char * str_do_bit_enc = get_arg("bitenc", arguments);
  bool do_bit_enc = False;

  switch(variant) {

    /* ------------------------------------------------------------ */
  case VARIANT_GENERIC_MINIMACS: 
    /* ------------------------------------------------------------ */
    if (!std_pre_file) {
      _error(oe,"Missing -pre argument for file path to preprocessing material.");
      return 0;
    }

    if (bdt_pre_file) {
      _error(oe,"Generic Minimacs variant does not take bit decomposed triples (bdt).");
      return 0;
    }

    mm = GenericMiniMacs_DefaultLoadNew(oe, std_pre_file);
    return mm;
    /* ------------------------------------------------------------ */
  case VARIANT_GENEFFT_MINIMACS:
    /* ------------------------------------------------------------ */    
    if (!std_pre_file) {
      _error(oe,"Missing -pre argument for file path to preprocessing material.");
      return 0;
    }

    if (bdt_pre_file) {
      _error(oe,"Generic Minimacs variant does not take bit decomposed triples (bdt).");
      return 0;
    }

    _error(oe,"Not implemented as there is no factory method for this in generic_minimacs.c.");
    return 0;
    /* ------------------------------------------------------------ */
  case VARIANT_BWMPMXT_MINIMACS:
    /* ------------------------------------------------------------ */
    if (!std_pre_file) {
      _error(oe,"Missing -pre argument for file path to preprocessing material.\n"
	    "run genpre 120 256 2 6800 mxt for two players with 6800 singles.\n");
      return 0;
    }

    if (!bdt_pre_file) {
      _error(oe,"Missing -bdt argument for file pwht to bit decomposed triples."
	    "run genpre 120 256 2 6800 mxt for two players with 6800 singles.\n");
      return 0;
    }

    if (cstr_eq("yes",str_do_bit_enc)) {
      do_bit_enc = True;
    }

    if (str_do_bit_enc && do_bit_enc) {
      _error(oe,"Argument -bitenc is either \"yes\" or omitted");
      return 0;
    }

    mm = BitWiseMulPar2MiniMacs_DefaultLoadNew(oe, std_pre_file, bdt_pre_file, do_bit_enc);
    return mm;
    
    /* ------------------------------------------------------------ */
  case VARIANT_BWMPFFT_MINIMACS:
    /* ------------------------------------------------------------ */
    if (!std_pre_file) {
      _error(oe, "Missing -pre argument for file path to preprocessing material.\n"
	    "run genpre 85 255 2 6800 fft sba for two players with 6800 singles.\n");
      return 0;
    }

    if (!bdt_pre_file) {
      _error(oe, "Missing -bdt argument for file pwht to bit decomposed triples."
	    "run genpre 85 255 2 6800 fft sba for two players with 6800 singles.\n");
      return 0;
    }

    if (cstr_eq("yes",str_do_bit_enc)) {
      do_bit_enc = True;
    }

    if (str_do_bit_enc && do_bit_enc) {
      _error(oe,"Argument -bitenc is either \"yes\" or omitted");
      return 0;
    }

    mm = BitWiseMulPar2MiniMacs_DefaultLoadFFTNew(oe, std_pre_file, bdt_pre_file, do_bit_enc);
    return mm;
    

  default:
    _error(oe,"This should never happen. Programming error cannot instantiate supported MiniMac variant.");
    return 0;
  }

  return mm;
  
}


static 
void print_banner(OE oe) {
  oe->p("------------------------------------------------------------");
  oe->p("  " PACKAGE_STRING " - " SVN_REVISION " - " CODENAME);
  oe->p(" build: " TIMEDATE);
  oe->p("------------------------------------------------------------");
}


static 
int load_circuit(OE oe, Map args) {

  if (!args->contains("circuit")) {
    _error(oe, "No -circuit argument given.");
    return -1;
  }
  
  yyin = fopen(get_arg("circuit",args),"rb");
  if (!yyin) {
    _error(oe, "Unable to load circuit file.");
    return -1;
  }
  args->rem("circuit");
  
  yyparse();
  if (error != 0 || root == 0) {
    _error(oe, "Unable to parse circuit.");
    return -2;
  }

  return 0;
}

static int
connect_peers(OE oe, MiniMacs mm, Map args) {
  byte msg[512] = {0};
  uint no_peers = 0;
  uint myid = 0;
  uint id = 0;
  char * _port = "2020";
  uint port = 2020;

  // get port argument
  if (args->contains("lport")) {
    _port = get_arg("lport",args);
  }
  port = atoi(_port);

  // print info about expected connections
  no_peers = mm->get_no_players();
  myid = mm->get_id();
  osal_sprintf(msg, "Peer %u of %u ready.", myid+1,no_peers);
  oe->p(msg);

  // listen for peers connecting to me
  if (no_peers-myid-1 > 0) {
    zeromem(msg, 512);
    osal_sprintf(msg, "Listening for %u parties to connect.",myid+1, no_peers,no_peers-(myid+1));
    if (mm->invite(no_peers-myid-1, port) != 0) return -1;
  }
  
  for(id = 0;id < myid;++id) {
    char ipkey[20] = {0};
    char * pip = 0;
    uint pp = 2020;
    osal_sprintf(ipkey,"peer%uip", id);
    if (!args->contains(ipkey)) {
      zeromem(msg,512);
      osal_sprintf(msg, "No ip address given for peer with id %u (-peer%uip) not provided.",id,id);
      _error(oe,msg);
      return -1;
    }
    
    pip = args->get(ipkey);
    if (mm->connect(args->get(ipkey),port) != 0) {
      osal_sprintf(msg, "Unable to establish connection to peer %u (%s)\n", id+1, args->get(ipkey));
      _error(oe, msg);
      return -1;
    }
  }
  return 0;
}

static 
void report_unused_args(OE oe, Map args) {
  uint i = 0;
  List keys = args->get_keys();

  for( i = 0; i < keys->size(); ++i ) {
    byte msg[92] = {0};
    osal_sprintf(msg, "Unused argument \"%s\".",(char*)args->get(keys->get_element(i)));
  }
}


int main(int c, char **a) {
  OE oe = 0;
  Map args = 0;
  extern MiniMacs mm;  
  Visitor interp = 0; 

  // check runtime 
  oe = OperatingEnvironment_LinuxNew();
  if (oe == 0) { 
    printf("Failed to instantiate Linux runtime\n");
    return -1; 
  }

  // initialize
  args = Options_New(oe,c,a);
  init_polynomial();
  anf = AstNodeFactory_New(oe);
  if (!anf) { 
    _error(oe,"Unable to instantiate AstNode Factory.");
    return -1;
  }
  mm = make_minimacs(oe, args);
  if (!mm) return -2;


  // load circuit
  oe->p("Loading circuit...");
  if (load_circuit(oe,args) != 0) return -1;
  
  // connect peers
  if (connect_peers(oe, mm, args) != 0) return -1;

  // print warnings to the user about unused arguments
  report_unused_args(oe,args);

  // create interpreter

  oe->p("Interpreting circuit...");
  interp = mpc_circuit_interpreter( oe,  root, mm, args->contains("replicate"));
  root->visit(interp);
  
  printf("Done %p\n",root);
  
  return 0 ;
}
