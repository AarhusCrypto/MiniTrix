/*
 * coov4.c
 *
 *
 * Created on: February 19, 2015
 *     Author: Rasmus Winther Zakarias
 */
#include <coov4.h>
#include <mutex.h>
/*
 * Allocate read/write/execute memory
 */
static void * coo_internal_special_memory(uint siz);

/*
 * CooV4 static state
 */
static MUTEX coov4_lock;
static byte * coov4_mem;
static uint coov4_fkt_max;

// ############################################################
// ------------------------------------------------------------
//                          OSX/LINUX
// ------------------------------------------------------------
#ifdef LINUX
#include <sys/mman.h>
#include <stdlib.h>
// on linux we use mprotect to achieve this, it requires memory be 4K
// aligned.
static void * coo_internal_special_memory(uint siz) {
  void * m = malloc(siz);
  unsigned long long pageaddr = 0, r = 0;
for(r = 0;r < siz;++r) ((byte*)m)[r] = 0;
  pageaddr = (unsigned long long)m;
  pageaddr = pageaddr - (pageaddr % 4096);
  if (mprotect((void*)pageaddr,siz,PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
    return 0;
  }
  m = (void*)pageaddr;
  return m;
}


/*
 * The stub function which is actually no longer a function as it
 * never exit. It is nine asm-instructions erasing its own stack frame
 * jumping to the actual function after computing the this pointer.
 * 
 * Operations:
 *
 *   * Get the RIP into RAX
 *
 *   * compute Ce entry address aligned 
 *     at 64 byte address before RIP in RAX
 *     TODO(rwz): improve using ANDQ and bit pattern?
 *
 *    * restore stack as if call didn't happen
 *
 *    * store this point at 8(RAX) into reg R10
 *
 *    * jmp directly into function at (RAX)
 *
 * We require r10 is not used during function calls, this is safe when
 * following the System V x86-64 calling convension see
 * http://www.x86-64.org/documentation/abi.pdf page 22.
 */
static inline void generic_stub() {
  __asm__("call unique_label\n"
	  "unique_label:\n"
	  "popq %%rax\n" // get the RIP into RAX
	  "shrq $5, %%rax\n" // compute Ce address
	  "shlq $5, %%rax\n" // now Ce address is in RAX
	  "movq %%rbp, %%rsp\n"// restore stack as if call never
			       // happened
	  "popq %%rbp\n"
	  "movq 8(%%rax), %%r10\n" // this pointer in R10
	  "movq (%%rax), %%rax\n" // function to call in RAX
	  "jmpq *%%rax\n" // jump as if the stub never existed
	  : /* no output */
	  : /* no input */
	  : 
	  );
}

// on the amd64 architecture we pass the this argument in R10
// register. This function fetches that register for you.
void * COO_internal_getThis() {
  void * p = 0;
  __asm__( "movq %%r10,%0\n"
	   : "=r" (p) /* out */
	   : /* in  */
	   :);
    return p;
}


// ------------------------------------------------------------
//                           WINDOWS
// ------------------------------------------------------------
#elif WINDOWS
#include <windows.h>
static void * coo_internal_special_memory(uint siz) { return 0; }
// ------------------------------------------------------------
//                            OTHERS
// ------------------------------------------------------------
#else
static void * coo_internal_special_memory(uint siz) { return 0; }
#endif
// ############################################################

/*
 * A COO-entry is an allocation per member function. The {fkt} member for
 * an instance of {Ce} points to the actual implementation that takes als0
 * the "this" pointer. The {ths} member is the "this" pointer and code is
 * the copied stub-code for this particular instance.
 *
 * This struct must have size a power of 2.
 */
typedef struct _coo_entry_ {
  void * fkt;
  void * ths;
  byte code[48]; // 48 makes the whole thing 64 bytes on x86_64
} * Ce;


/*
 * Find the next 64 bytes aligned free Ce slot.
 *
 */
static Ce find_free_slot() {
  ull i = 0;
  void * curadr = coov4_mem;
  if (!curadr) {
    //    printf("Coo hasn't been initialized :(\n");
    return 0;
  }

  do {
    Ce ce = (Ce)curadr;
    if (ce->fkt == 0) return ce;
    curadr = (void*)(((byte*)curadr) + sizeof(*ce));
  } while(++i < coov4_fkt_max);

  //  printf("Coo has reached the maximal number of functions that can be allocated.");
  return 0;
}

/* 
 * Free the given ce slot.
 */
static void free_slot(Ce ce) {
  uint i = 0;
  byte * b = 0;
  if (ce == 0) return;

  b = (byte*)ce;

  // TODO(rwz): show ce point is which in bounds
  // [coov4_mem;coov4_mem+coov4_fnt_max*sizeof(ce)].

  for(i = 0;i < sizeof(*ce); ++i) {
    b[i] = 0;
  }
}

/*
 * Allocate special memory 
 * 
 * 1) assign this pointer
 * 2) assign function pointer
 * 3) copy generic stub
 *
 */
void * COO_attach(void * instance, void * function) {
  int i = 0 ;
  Ce ent = 0;
  
  Mutex_lock(coov4_lock);
  ent = find_free_slot();

  if (!ent) { 
    Mutex_unlock(coov4_lock);
    return 0;
  }

  ent->ths = instance;
  ent->fkt = function;
  for(i  = 0 ; i < sizeof(ent->code) ;++i) {
    ent->code[i] = (byte)((byte*)generic_stub)[i];
  }
  Mutex_unlock(coov4_lock);
  return ent->code;
}

/**
 * The {coov4_mem} has room for {coov4_fkt_max} {Ce}-entries. An entry
 * is taken if its {ce->fkt} pointer is non-zero. This function takes
 * a member function pointer and computes from that its {Ce}
 * entry. Then it zeros out the {fkt}, {ths} and {code} members of the
 * entry making it available for a new allocation.
 *
 * @Note this function fails silently when {function} is out of bounds.
 *
 * @param function - member function pointer to be free up.
 *
 */
void COO_detach(void * function) {
  ull fp = (ull)function;
  ull cm = (ull)coov4_mem;
  uint i = 0;

  // check bounds
  if ( fp > cm && fp < cm+sizeof(Ce)*coov4_fkt_max ) {

    // compute Ce entry address
    byte * f = (byte*)function;
    Ce ent = (Ce)(f-sizeof(ent->fkt)-sizeof(ent->ths));

    // Get exclusive access to special mem (coov4_mem) and free ent.
    Mutex_lock(coov4_lock);
    ent->fkt = ent->ths = 0;
    for(i = 0;i < sizeof(ent->code);++i) ent->code[i] = 0;
    Mutex_unlock(coov4_lock);
  }
}

bool COO_setup(OE oe, int cfnt) {
  
  // TODO(rwz): setup mutex
  
  // TODO(rwz): function called below is platform
  // specific should be moved into osal.c allowing coo to living in
  // src (not in posix_src) makeing the whole thing more portable.

  // setup exclusive access
  coov4_lock = Mutex_new( MUTEX_TAKE );

  // setup up coov4 mem 
  coov4_mem = coo_internal_special_memory(sizeof(Ce)*cfnt);

  // we are setup of {cfnt} concurrent allocations.
  coov4_fkt_max = cfnt;  

  // release resourceses
  Mutex_unlock( coov4_lock );

  return True;
}
