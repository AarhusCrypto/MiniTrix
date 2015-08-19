#include <osal.h>
#include <singlelinkedlist.h>




int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();
  List list = SingleLinkedList_new(oe);
  uint number = 2;
  ull i = 1024*1024*64;
  ull max = i;

  oe->print("Adding %lu e.g. %lu MEGA items to list ... ",i,i/(1024*1024));
  oe->print("%3d%%",0);
  while(i--) {
    list->add_element((void*)(ull)number++);
    if (i % (max/100) == 0) {
      oe->print("\b\b\b\b");
      oe->print("%3d%%",(max-i)*100/max);
    }
  }
  oe->print(" done list size %lu\n",list->size());
  
  return 0;

}
