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
#include <math/polynomial.h>
#include "ast.h"
#define YYSTYPE AstNode
#include "y.tab.h"
#include "interp.h"
#include "config.h"
extern AstNodeFactory anf;
extern FILE * yyin;
extern int error;

int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
  extern MiniMacs mm;  
  extern AstNode root;

  if (oe == 0) {
    return -1;
  }
  init_polynomial();
  anf = AstNodeFactory_New(oe);

  
  mm = GenericMiniMacs_DefaultLoadNew(oe,a[2]);
  if (!mm) return -2;

  oe->p("------------------------------------------------------------");
  oe->p("  " PACKAGE_STRING " - " SVN_REVISION " - " CODENAME);
  oe->p(" build: " TIMEDATE);
  oe->p("------------------------------------------------------------");
  
  yyin = fopen(a[1],"rb");
  if (yyin) {
    Visitor interp = 0;

    // parse
    oe->p("Parsing circuit...");
    yyparse();
    if (error) {   printf("\n");return -1; }

    // connect peers now that we read a circuit
    if (mm->get_id() == 0) {
      oe->p("One party invited, waiting ... ");
      mm->invite(1,2020);
    } else {
      mm->connect(a[3],2020);
    }

    oe->p("Interpreting circuit...");
    // create interpreter
    interp = mpc_circuit_interpreter( oe,  root, mm);
    if (root) 
      root->visit(interp);

    printf("Done %p\n",root);
  } else {
    printf("Error: Unable to open file\n");
  }
  
  return 0 ;
}
