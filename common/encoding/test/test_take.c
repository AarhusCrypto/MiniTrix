#include <encoding/der.h>
#include <encoding/hex.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(MSG) {                            \
  if (rc != DER_OK) { \ 
printf("[FAIL] (%u) %s\n",__LINE__, (MSG));     \
return -1;                                      \
}}



int main(int c, char **a) {
  DerCtx * out = 0;
  DerCtx * in = 0;
  DerRC rc = DER_OK;
  byte * data;
  uint ldata = 0, lname = 0, lgade = 0;
  byte * name = 0;
  byte * gade = 0;
  uint age = 0;

  printf("DER Test\n");

  rc = der_begin(&out);
  CHECK("Failed to der begin");

  rc = der_begin_seq(&out);
  CHECK("der begin failed");

  rc = der_insert_cstr(out, "Rasmus Lauritsen");
  CHECK("der insert cstr failed");

  rc = der_insert_uint(out, 31);
  CHECK("der insert uint failed");

  rc = der_insert_cstr(out, "Thorshavnsgade 4B");
  CHECK("der insert octetstring failed");

  rc = der_end_seq(&out);
  CHECK("der end deq failed");

  rc = der_final(&out, 0, &ldata);
  CHECK("der final failed");

  data = malloc(ldata);
  
  rc = der_final(&out, data, &ldata);
  CHECK("der final failed, actual");

  rc = der_begin_read(&in, data, ldata);
  CHECK("der read failed");

  rc = der_take_octetstring(in, 0, &name, &lname);
  CHECK("take octetstring failed");

  rc = der_take_uint(in, 1, &age);
  CHECK("take uint failed");

  rc = der_take_octetstring(in, 2, &gade, &lgade);
  CHECK("take octetstring failed");

  rc = der_end_read(&in);
  CHECK("der end read failed");

  if (name) {
    printf("%s er %u aar gammel og bor paa %s\n",name,age,gade);
    free(data);
  }
  return 0;
}
