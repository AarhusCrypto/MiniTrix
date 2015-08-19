/*
 * coov3.c
 *
 *  Created on: Dec 8, 2014
 *      Author: rwl
 */


#include <coov3.h>
#include <mutex.h>
#include <common.h>

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * A COO-entry is an allocation per member function. The {fkt} member for
 * an instance of {Ce} points to the actual implementation that takes als0
 * the "this" pointer. The {ths} member is the "this" pointer and code is
 * the copied stub-code for this particular instance.
 *
 */
typedef struct _coo_entry_ {
	void * fkt;
	void * ths;
	byte code[STUB_SIZE-2*sizeof(void *)];
} * Ce;


/// _coo_lock ensure consistent concurrent access.
static MUTEX _coo_lock;
/// coo_base_address for Ce allocations
static void * coov3_base_address;
/// max number of Ce allocations we can do
static unsigned long coov3_fkt_max;

static void * __malloc__(unsigned int size) {
	return malloc(size);
}

static void __free__(void * ptr) {
	if (ptr != 0)
		free(ptr);
}

static Memory mem_instance;
Memory LinuxMemoryNew() {
	if (mem_instance) return mem_instance;

	mem_instance = malloc(sizeof(*mem_instance));
	if (!mem_instance) return 0;

	mem_instance->alloc = __malloc__;
	mem_instance->free = __free__;

	return mem_instance;
}

// not thread safe in it self remember to own the _coo_lock
static Ce find_free_slot() {
	ull i = 0;
	void * curadr = coov3_base_address;
	if (!curadr) {
		printf("Coo hasn't been initialized :(\n");
		return 0;
	}

	do {
		Ce ce = (Ce)curadr;
		if (ce->fkt == 0) return ce;
		curadr = (void*)(((byte*)coov3_base_address) + STUB_SIZE*i);
	} while(++i < coov3_fkt_max);

	printf("Coo has reached the maximal number of functions that can be allocated.");
	return 0;
}

// debug function
static void dump_hex(byte * d, uint ld) {
	uint i = 0;
	printf("\n\n");
	do {
		printf("%2x ",d[i]);
		if (i > 0 && i % 32 ==0) {
			printf("\n");
		}
	} while(++i < ld);
	printf("\n\n");
}

void * allocate_stub_fun(byte * static_stub, byte * invoke_fkt, void * ths) {
	Mutex_lock(_coo_lock);
	Ce free_slot = find_free_slot();
	if (free_slot) {
		free_slot->fkt = invoke_fkt;
		free_slot->ths = ths;
		mcpy(free_slot->code,static_stub,STUB_SIZE-2*sizeof(void*));
		Mutex_unlock(_coo_lock);
		return &(free_slot->code[0]);

	}
	Mutex_unlock(_coo_lock);
	return 0;
}


bool InitializeCOO(uint no_fkt, Memory m) {
	// compute size needed
	Ce a = 0;
	unsigned long long special_mem_size = sizeof(*a)*no_fkt+4096;

	// Sanity check
	if (sizeof(*a) != STUB_SIZE) {
		return False;
	}

	// allocate special memory for {no_fkt}
	coov3_base_address = m->alloc(special_mem_size);
	if (!coov3_base_address) {
		printf("Alloc failed\n");
		return False;
	}
	zeromem(coov3_base_address,sizeof(Ce)*no_fkt);
	coov3_fkt_max = no_fkt;

	// make special memory special.
	{
		unsigned long long pageaddr = 0, r = 0;
		pageaddr = (unsigned long long)coov3_base_address;
		pageaddr = pageaddr - (pageaddr % 4096);
		if (mprotect((void*)pageaddr,special_mem_size,PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
			printf("mprotect failed\n");
			return False;
		}

		// make sure {coov3_base_address} is STUB_SIZE aligned.
		//   pageaddr % STUB_SIZE == r != 0 ==> pageaddr = t*STUB_SIZE + r
		//   meaning pageaddr + (STUB_SIZE-r) == (t+1)*STUB_SIZE
		pageaddr = (unsigned long long)coov3_base_address;
		r = pageaddr % STUB_SIZE;
		if (r != 0) {
			pageaddr = pageaddr + (STUB_SIZE-r);
		}
		coov3_base_address = (void*)pageaddr;

		// setup global lock
		_coo_lock = Mutex_new( MUTEX_FREE );
		return True;
	}

 return False;
}

void deallocate_stub_function(void * fkt) {
	unsigned long long adr = ((ull)fkt)-2*sizeof(void*);
	Ce ce = (fkt-2*sizeof(void*));
	if (adr % STUB_SIZE == 0) {
		Mutex_lock(_coo_lock);
		zeromem(ce,sizeof(*ce));
		Mutex_unlock(_coo_lock);
	}
}

void TeardownCOO() {

}
