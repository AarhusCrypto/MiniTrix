#include <osal.h>
#include <utf8string.h>

#include <stdio.h>

#define TEST(NAME,T,MSG) { printf("TEST [%s] ... ",(NAME));  \
  if (!(T)) { printf("Failure: %s\n", (MSG)); }              \
  else                                                       \
    { printf("Success\n"); }                                 \
  }
  


int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
  String s = UTF8_String_cstrnew(oe, "Rasmus");
  uint len = 0;
  String t = 0;

  TEST("Create UTF8_String_cstrnew - valid arg", s != 0, 
       "Failed to craete from c-string");
  UTF8_String_destroy (&s);

  s = UTF8_String_cstrnew(oe,0);
  TEST("Create UTF8_String_cstrnew - Null", s == 0,"Expected null");
  UTF8_String_destroy (&s);

  s = UTF8_String_cstrnew(0,"Rasmus");
  TEST("Create UTF8_String_cstrnew - Null OE", s == 0, "Expected null");
  UTF8_String_destroy (&s);

  s = UTF8_String_cstrnew(oe, "Rasmus");
  len = s->length();
  TEST("Length of UTF8 string - given \"Rasmus\"", len == 6, "Expected 6");
  UTF8_String_destroy (&s);

  s = UTF8_String_cstrnew(oe, "Rasmus");
  t = UTF8_String_cstrnew(oe, "mus");
  TEST("Suffix mus of Rasmus", s->endswith(t) == True, "Expected true");
  UTF8_String_destroy(&s);
  UTF8_String_destroy(&t);

  OperatingEnvironment_LinuxDestroy ( &oe );
}
