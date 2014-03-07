#ifndef INTERP_H
#define INTERP_H
#include "ast.h"
#include <minimacs/minimacs.h>

Visitor mpc_circuit_interpreter(OE oe, AstNode root, MiniMacs mm);

#endif
