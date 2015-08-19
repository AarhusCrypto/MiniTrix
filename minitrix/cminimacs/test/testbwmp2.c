#include <osal.h>
#include <stats.h>
#include <minimacs/bitwisemulpar2_minimacs.h>
/*
 * Testing the flag-ship bitwisemulpar2_minimacs.
 */


typedef struct _test_ {
  char * name;
  bool (*test)(void * arg);
} Test;

static MUTEX hw_l1 = 0;


typedef struct _hw_t1_arg_ {
  OE oe;
  MiniMacs mm;
} HWT1A;

static void circuit(MiniMacs mm) {
  MiniMacsRep result = 0;
  byte d1[85] = {0};
  byte d2[85] = {0};
  int i  = 0;

  d2[0] = 0xFF;
  d1[0] = 0xFF;
  
  mm->init_heap(8);
  
  mm->secret_input(1,0,Data_shallow(d1,85));
  mm->secret_input(1,1,Data_shallow(d2,85));
  
  mm->mulpar(2);
  mm->mul(2,0,1);
  mm->mul(3,0,1);
  
  for(i = 0;i < 8;++i) {
    mm->open(i);
    if (mm->get_id() == 0) {
      MiniMacsRep r = mm->heap_get(i);
      if (r) {
        printf("%02x ",r->codeword[0]);
      } else printf("xx ");
    }
  }

}

static void * hw_t1(void *a) {
  HWT1A * b = (HWT1A*)a;
  b->mm->invite(1,2020);
  circuit(b->mm);
  printf("-------------------- LEAVING hw t1 --------------------\n");
  return 0;
}


static bool hw(void * arg) {
  OE oe = (OE)arg;
  const char * filename0 = "../../minimacs_85_255_2_0_fft.rep";
  const char * bdt_filename0 = "../../bdt_85_255_0of2_2048_fft.rep";
                                      
  const char * filename1 = "../../minimacs_85_255_2_1_fft.rep";
  const char * bdt_filename1 = "../../bdt_85_255_1of2_2048_fft.rep";
  HWT1A a = {0};
  MiniMacs mm0 = 0, mm1 = 0;
  ThreadID tid = 0;

  a.oe = oe;


  oe->newmutex(&hw_l1);
  oe->lock(hw_l1);
  oe->setloglevel(OSAL_LOGLEVEL_WARN);
  mm0 = BitWiseMulPar2MiniMacs_DefaultLoadFFTNew(oe,
                                              filename0, 
                                              bdt_filename0, 
                                              True);
  a.mm = mm0;
  if (!mm0) goto failure;
  mm1 = BitWiseMulPar2MiniMacs_DefaultLoadFFTNew(oe,
                                              filename1, 
                                              bdt_filename1, 
                                              True);
  if (!mm1) goto failure;

  printf("mm0: My id = %u %p\n",mm0->get_id(),mm0);
  printf("mm1: My id = %u %p\n",mm1->get_id(),mm1);
  
  oe->newthread(&tid,hw_t1, &a);

  usleep(200);
  if (mm1->connect("127.0.0.1", 2020) != 0) goto failure;
  
  circuit(mm1);

  oe->jointhread(tid);

  BitWiseMulPar2MiniMacs_destroy ( & mm0 );
  BitWiseMulPar2MiniMacs_destroy ( & mm1 );
  oe->setloglevel(OSAL_LOGLEVEL_TRACE);  
  return True;
 failure:
  BitWiseMulPar2MiniMacs_destroy ( & mm0 );
  BitWiseMulPar2MiniMacs_destroy ( & mm1 );
  oe->setloglevel(OSAL_LOGLEVEL_TRACE);
  return False;
}

static Test tests[] = { 
  {"Hello world", hw}
};

int main(int c, char **a, char **e) {
  
    OE oe = OperatingEnvironment_New();
  uint i = 0, j = 0;
  char txt[64] = {0};
  init_polynomial();
  InitStats(oe);

  for(i = 0; i < sizeof(tests)/sizeof(Test);++i) {
    uint l = 0;
    while(tests[i].name[l]) ++l;
    for(j = 0;j < 63;++j) {
      txt[j] = ' ';
    }
    txt[63] = 0;
    sprintf(txt,"%s",tests[i].name);
    txt[l] = ' ';
    printf("[ ] Running test \"%s\" ... ",txt);
    if (tests[i].test(oe)) { 
      printf(" [ success ]\n"); 
    } else {
      printf(" [ failed ]\n");
    }
  }
  PrintMeasurements(oe);
  OperatingEnvironment_Destroy(&oe);
  teardown_polynomial();
  return 0;

  
}
