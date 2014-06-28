#include "coo.h"
#include <memory.h>
#ifdef LINUX
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#endif


static Memory mem;
static byte * freelist;

void * get_rip() {
  void * result = 0;
  asm("movq (%%ebp), %%rax\n"
      "movq %%rax, %0\n"
      :"=r" (result)
      :
      : "%rax"
      );
  return result;
}

static byte match( byte * data, byte * pattern, uint lpattern )
{
  uint j = 0;
  for(j = 0;j<lpattern;j++)
    if (pattern[j] != data[j]) return 0;
  return 1;
}

static void mset(void * dest, byte val, uint l)
{
  while(l) ((byte*)dest)[--l] = val;
}



static void i2b(uint l, byte * out)
{
  out[0] = (l & 0x000000FF);
  out[1] = (l & 0x0000FF00) >> 8;
  out[2] = (l & 0x00FF0000) >> 16;
  out[3] = (l & 0xFF000000) >> 24;
}

// DANGER
static unsigned long b2i(byte * in)
{
  uint res = 0;
  res += in[0];
  res += in[1] << 8;
  res += in[2] << 16;
  res += in[3] << 24;
  return res;
}


void coo_depatch( byte * stub ){
  mem->free(stub);
}

void * coo_patch2( byte * stub, uint lstub, void * ths) {
  byte * fun = mem->alloc(lstub+1);
  unsigned long long v = (unsigned long long)ths;
  mcpy(fun, stub, lstub);
  l2b(v,fun+10);
  return fun;
}


void * coo_patch(byte * stub_fun, byte * fun, void * ths) {
  byte * result = mem->alloc(STUB_SIZE);

  if (!result) {
    printf("[COO FATAL] Out of special memory !\n");
    exit(-1);
  }

  mcpy(result+2*sizeof(void*),stub_fun,STUB_SIZE-2*sizeof(void*));

  l2b((ull)fun,(byte*)result);
  l2b((ull)ths,result+sizeof(void*));

  //  printf("Patch Address %p @ %p\n",result,ths);
  return result+2*sizeof(void*);
}

/*
void * coo_patch( byte * stub, uint lstub, void * this ){
  uint i = 0;
  byte pattern[] = {0xEF, 0xBE, 0xAD, 0xDE, 0xEF, 0xBE, 0xAD, 0xDE};
  uint lpattern = sizeof(pattern);
  byte * fun = mem->alloc(lstub+1);
  byte found = 0;
  mcpy(fun, stub, lstub);
  for(i = 0;i<lstub-lpattern;i++){
    if (match(fun+i, pattern, lpattern)) {
      byte serv[8] = {0};
      unsigned long long v = (unsigned long long)this;
      l2b(v, serv);
      mcpy(fun+i, serv, sizeof(serv));
      found =1; break;
    }
  }
  if (!found) {
    int i = 0;
    printf("----------------------------------------------------------------\n");
    for(i =0 ; i < lstub; ++i) {
      if (i > 0 && i % 16 == 0) printf("\n");
      printf("%2x ",*(fun+i));
    }
    printf("\n----------------------------------------------------------------\n\n");
    printf("[COO] FATAL ERROR, FAILED TO PATCH \n");
    return 0;
  }
  return fun;
}
*/

#ifdef LINUX
static uint last_percentage;
static byte * special_mem;
static uint lspecial_mem;
static uint idx;
static void * special_alloc_impl(uint size)
{
  void * res = 0;
  /* TODO(rwl): Free list implementation not working */
  if (lspecial_mem > idx + size) { 
    /*
    unsigned long long used = lspecial_mem-idx;
    uint percentage = (100*used)/lspecial_mem;
    if (percentage > last_percentage) {
      last_percentage = percentage;
      printf("%u %u %u %% speciial_mem free\n",idx, lspecial_mem, percentage);
    }
    */
    res = special_mem + idx; idx += size; 
  }
  else { 
    printf("FATAL: COO Library ran out of memory.");
    exit(-1);
  }
  return res;
}

static void special_free_impl(void * memory)
{
  unsigned long m = (unsigned long)memory;
  if (m >= (unsigned long)(special_mem) && 
      m <= (unsigned long)(special_mem+idx)) // we own that pointer ...
    {
      if (m % STUB_SIZE == 0) // and it is aligned 
        {
          byte r[4] = {0};
          i2b((unsigned long)freelist,memory);
          freelist =  memory;
        }
    }
}

static void special_free_all_impl(void)
{
  return;
}


#define SPECIAL_MEM_SIZ 4*1024*1024*256

Memory LinuxSpecialMemoryNew(Memory m) 
{
  Memory res = m->alloc(sizeof(*res));
  unsigned long pageaddr = 0;
  mset(res, 0, sizeof(*res));
  res->alloc = special_alloc_impl;
  res->free  = special_free_impl;

  res->free_all = special_free_all_impl;
  special_mem = m->alloc(SPECIAL_MEM_SIZ);
  pageaddr = (unsigned long)special_mem;
  pageaddr = pageaddr - (pageaddr % 4096);
  if (mprotect( (void*)pageaddr, SPECIAL_MEM_SIZ, PROT_READ | PROT_WRITE | PROT_EXEC ) != 0)
    return NULL;
  lspecial_mem = SPECIAL_MEM_SIZ;
  return res;
  
}

static void * linux_memory_alloc( uint size )
{
  return malloc(size);
}

static void linux_memory_free( void * m ) {
  if (m)
    free(m);
}

static void linux_memory_free_all( void )
{
  return;
}

Memory LinuxMemoryNew(void)
{
 Memory res = malloc(sizeof(*res));
  if (res == NULL) return res;
  res->alloc = linux_memory_alloc;
  res->free  = linux_memory_free;
  res->free_all=linux_memory_free_all;
  return res;
}
#else
#error "this platform is not supported yet by COO lib"
#endif

void coo_init(Memory special_m) {
  if (!mem) {
    mem = special_m;
  } // otherwise we are already initialised
}

void coo_end() {
  if (mem) { 
    mem->free_all();
    free(mem);mem=0;
    free(special_mem);
    special_mem=0;
  }
}
