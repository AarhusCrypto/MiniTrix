#include <osal.h>
#include <list.h>
#include <singlelinkedlist.h>
#include <stdio.h>

#define TEST(EXP,ACT) {                             \
  if ((EXP) != (ACT)) {                             \
    printf("Expected %s but got %s\n",(#EXP),(#ACT)); \
  } else { printf("%s \t [ok]\n", (#ACT)); } }





int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();
  List l = SingleLinkedList_new(oe);
  int i = 0;

  l->add_element((void*)(ull)42);
  TEST((void*)(ull)42,l->get_element(0));

  TEST(1,l->size());
  l->add_element((void*)(ull)43);
  TEST((void*)(ull)43, l->get_element(1));


  for(i = 2;i < 256;++i) {
    l->add_element((void*)(ull)i);
    TEST((void*)(ull)i, l->get_element(i));
  }

  for(i = 2;i < 256;++i) {
    TEST((void*)(ull)i, l->get_element(i));
  }

  return 0;
}
