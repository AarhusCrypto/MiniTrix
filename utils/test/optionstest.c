#include <osal.h>
#include <utils/options.h>


#include <stdio.h>

int main(int argc, char **args) {
  OE oe = OperatingEnvironment_LinuxNew();
  Map a = Options_New(oe,argc,args);
  List keys = a->get_keys();
  int i = 0;

  for(i = 0;i < keys->size();++i) {
    char * key = (char*)keys->get_element(i);
    char * val = (char*)a->get(key);
    printf("%s: \"%s\"\n",key,val);
  }
  

  if (a->contains("rasmus")) {
    printf("The king has spoken: %s\n",a->get("rasmus"));
  }
  
  
  //  SingleLinkedList_destroy( &keys );
  Options_Destroy(&a);
  OperatingEnvironment_LinuxDestroy(&oe);

  return 0;
}
