#include <hashmap.h>
#include <osal.h>

static uint hfn(void * a) {
  uint an = (uint)(ull)a;
  return an * 2903 + 1009; 
}

static int cfn(void * a, void * b) {
  uint an = (uint)(ull)a;
  uint bn = (uint)(ull)b;
  return ( an == bn ? 0 : (an > bn ? 1 : -1));
}

int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
  OE tmp = 0;
  Map hmap = HashMap_new(oe,hfn,cfn,5);
  hmap->put( (void*)(ull)1,oe);
  
  if (hmap->size() == 1) oe->p("Testing size one [ok]");
  else oe->p("Testing size one [fail]");

  tmp = (OE)hmap->get( (void *)(ull)1 );
  if (tmp == oe) oe->p("Testing get [ok]"); 
  else oe->p("Testing get [fail]");

  tmp = hmap->rem( (void*)(ull) 1);
  if (tmp == oe) oe->p("Testing rem return [ok]"); 
  else oe->p("Testing rem return [fail]");

  if (hmap->size() == 0) oe->p("Testing rem size [ok]"); 
  else {oe->p("Testing rem size [fail]"); }

  
  
  OperatingEnvironment_LinuxDestroy( &oe );
  return 0;
}
