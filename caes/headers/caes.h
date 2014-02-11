#ifndef CAES_H
#define CAES_H
#include <minimacs/minimacs.h>

/*
 * This is the AES circuit.
 *
 * Computes the AES encryption of {plaintext} where the key is
 * additively shared between the parties.
 */
byte * mpc_aes(MiniMacs c, byte * plaintext, byte *keyshare);

#endif
