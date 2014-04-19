#ifndef AES_H
#define AES_H
#include <tinyot.h>
#include <common.h>
#include <carena.h>

void 
mpc_aes(OE oe, TinyOT tot, 
        byte * plaintext, tinyotshare ** key, byte * ciphertext, 
        Data _pid, MpcPeer mission_control);


#endif
