#include <minimacs/minimacs.h>
#include <minimacs/generic_minimacs.h>
#include <osal.h>
#include <stats.h>

#include <unistd.h>
typedef struct _cli_arg_ {
  char * file;
  OE oe;
} * CliArg;

#define COUNT 32

static
void * client(void * a) {
  CliArg arg = (CliArg)a;
  OE oe = arg->oe;
  MiniMacs mm = GenericMiniMacs_DefaultLoadNew(oe,arg->file);
  uint count = 0;
  
  if (!mm) {
    oe->p("Client: Error could not create instance of MiniMacs");
    return 0;
  }
  mm->init_heap(2);
  mm->connect("127.0.0.1",8080);

  mm->secret_input(0,0,0);

  for(count = 0;count < COUNT;++count) {
    mm->mul(1,0,0);
  }
}

int main(int c, char **a) {
  OE oe = (OE)OperatingEnvironment_LinuxNew();
  MiniMacs mm = 0;
  Data input = 0;
  uint count=0,i=0;
  init_polynomial();

  mm = GenericMiniMacs_DefaultLoadNew(oe,a[1]);

  InitStats(oe);

  if (!mm) {
    oe->p("Error could not create instance of MiniMacs");
    OperatingEnvironment_LinuxDestroy(&oe);
    return -1;
  }
  
  {
    CliArg arg = (CliArg)oe->getmem(sizeof(*arg));
    arg->file = a[2];
    arg->oe = oe;
    oe->newthread(client,arg);
  }

  mm->init_heap(2);
  mm->invite(1,8080);
  printf("Got client ... \n");

  input = Data_new(oe, mm->get_ltext());
  for(i = 0;i < mm->get_ltext();++i) {
    input->data[i] = 'r';
  }

  mm->secret_input(0,0,input);

  for(count = 0;count < COUNT;++count) {
    mm->mul(1,0,0);
  }

  usleep(5);
  PrintMeasurements(oe);

  return 0;
}
