#include <osal.h>
#include <tinyot.h>
#include <carena.h>
#include <aes.h>

static
void mission_control_start(MpcPeer mission_control, Data _pid) {
  byte _d[256] = {0};
  Data d = Data_shallow(_d,sizeof(_d));
  if (!mission_control || !_pid) return;
  mission_control->send(_pid);
  mission_control->receive(d);
}

static
void mission_control_stop(MpcPeer mission_control, Data _pid) {
  byte _d[256] = {0};
  Data d = Data_shallow(_d,sizeof(_d));
  if (!mission_control || !_pid) return;
  mission_control->send(_pid);
}

void
mpc_aes(OE oe, TinyOT tot, byte * plaintext, tinyotshare ** key, byte * ciphertext, Data _pid, MpcPeer mission_control) {
#include <AES_sorted.spaclc>
}
