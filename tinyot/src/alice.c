#include <osal.h>
#include <tinyot.h>
#include <stdio.h>
#include <stats.h>

void sim(TinyOT tot) {
  tinyotshare s = {0};
  byte v = 0;
  
  tot->init_heap(16);
  tot->no_of_ands(1);
  tot->max_width_AND(1);
  tot->max_width_private_common_load(1);
  tot->max_width_public_common_store(1);

  tot->begin_layer_private_common_load(1);
  tot->private_common_load(&s,0,0);
  tot->end_layer_private_common_load(1);

  tot->begin_layer_INV(1);
  tot->INV(0,0,0);
  tot->end_layer_INV(1);
  
  tot->begin_layer_public_common_store(1);
  tot->public_common_store(&v,0,0);
  tot->end_layer_public_common_store(1);

  printf("res = %02x \n",v);
}


void mpc_aes(OE oe, TinyOT tot, byte * plaintext, tinyotshare ** key, byte * ciphertext);

int main(int c, char **args) {

  OE oe = OperatingEnvironment_LinuxNew();
  TinyOT alice = TinyOT_new(oe, 1);
  byte plaintext[128] = {0};
  byte ciphertext[128] = {0};
  tinyotshare ** key = oe->getmem(sizeof(tinyotshare *)*128);
  int i = 0;
  alice->invite(2020);

  InitStats(oe);

  for (i=0; i<128; i++) {
    plaintext[i] = 0;
    ciphertext[i] = -1;
    key[i] = oe->getmem(sizeof(*key[i]));
    key[i]->shr = 0;
    key[i]->mac = 0;
    key[i]->key = 0;
  }


  // sim(alice);
  CHECK_POINT_S("AES");
  mpc_aes(oe, alice, plaintext, key,ciphertext);
  CHECK_POINT_E("AES");
  
  for(i = 0;i < 128;++i) {
    if (i > 0 && i % 16 == 0) printf("\n");
    printf("%02x ",ciphertext[i]);
  }
  printf("\n");

  PrintMeasurements(oe);

  TinyOT_destroy(&alice);
  oe->p("DONE press any key to terminate.\n");
  getchar();
  //  OperatingEnvironment_LinuxDestroy(&oe);

  return 0;
}
