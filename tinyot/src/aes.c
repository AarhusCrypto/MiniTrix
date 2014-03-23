#include <osal.h>
#include <tinyot.h>
#include <carena.h>

void
mpc_aes(OE oe, TinyOT tot, byte * plaintext, tinyotshare ** key, byte * ciphertext, Data _pid, MpcPeer mission_control) {
  byte _d[256] = {0};
  Data d = Data_shallow(_d,sizeof(_d));
#include <AES_sorted.spaclc>
}
