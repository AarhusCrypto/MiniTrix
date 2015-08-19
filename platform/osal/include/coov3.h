/*
 * coov3.h
 *
 *  Created on: Dec 8, 2014
 *      Author: rwl
 */

#ifndef OSAL_INCLUDE_COOV3_H_
#define OSAL_INCLUDE_COOV3_H_
#define STUB_SIZE 1024
#include <memory.h>

/**
 * TODO(rwz): Find a way to figure out the size of the assembly of an emitted c-function.
 *
 * Allocate an COO-Entry (Ce) in special rwx-memory copying the code given by {static_stub}
 * for execution in this memory. The allocated (Ce) is setup with {invoke_fkt} and {ths} for
 * its {fkt}-member and {ths}-member respectively.
 *
 * Returned is a pointer the {code} member containing a copy of the code in {static_stub}.
 *
 */
void * allocate_stub_fun(byte * static_stub, byte * invoke_fkt, void * ths);

/**
 *
 * Deallocate an COO-Entry (Ce) in special rwz-memory such that it can be occupied by
 * another member function.
 *
 */
void deallocate_stub_function(void * fkt);

/**
 * Memory allocation is handled by this {Memory} instance created by {LinuxMemory}
 * on Linux. This function should be called the first time in a single-threaded
 * context. E.g. make sure this function runs to completion before creating more threads.
 */
Memory LinuxMemoryNew();


/**
 * ASMSNIPPET
 *
 * Makes a unique symbol based on {CLZ} and {NAME} and outputs Assembly code
 * that fetches the instruction pointer into ADRVAR.
 *
 */
#define ASMSNIPPET(CLZ,NAME,ADRVAR) asm(\
   "call "#CLZ"_"#NAME"_jmp\n\t"        \
     #CLZ"_"#NAME"_jmp: popq %%rax\n\t" \
     "movq %%rax, %0\n"                 \
  	 : "=r" (ADRVAR)                    \
	 :                                  \
  	 : "%rax"                           \
);


/** COO_ATTACH_FN
 *
 * Attach the function given by {FN} to the member {NAME} on object {OBJ} of class {CLZ}.
 *
 */
#define COO_ATTACH_FN(CLZ,OBJ,NAME,FN) { \
	OBJ->NAME = allocate_stub_fun((byte*)stub_##CLZ##_##FN, (byte*)CLZ##_##FN,OBJ);\
}
/**
 *
 * Shorthand for the default case where the implemented function is named after the member.
 *
 */
#define COO_ATTACH(CLZ,OBJ,NAME) {\
	COO_ATTACH_FN(CLZ,OBJ,NAME,NAME); }

/** COO_DCL
 *
 * Declare a member function for class {CLZ} returning {RET} having
 * name {NAME} taking argument __VA_ARGS__.
 *
 */
#define COO_DCL(CLZ, RET, NAME, ...)		\
  static RET CLZ##_##NAME(CLZ t,##__VA_ARGS__);

/**
 * COO_DEF_RET_ARGS
 *
 * Define the function that will be a member on instances of {CLZ}
 *
 * 1) Define the function "stub_CLZ_NAME(__VA_ARGS__)
 * 1_a) Compute the current instruction address
 * 1_b) Compute the address of the {Ce} we are in
 * 1_c) Assign into k the address of the {fkt} member (record[0])
 * 1_d) Assign into this the address of the {ths} member (record[1])
 * 1_e) Invoke k(this,__VA_ARGS__) and return the result
 * 2) Define the CLZ_NAME(this,__VA_ARGS__) function headers
 *    and expect its implementation to follow.
 */
#define COO_DEF_RET_ARGS(CLZ,RET,NAME,TYPES,...) \
	static RET stub_##CLZ##_##NAME(__VA_ARGS__) TYPES {\
	unsigned long long instr_ptr = 0;\
	ASMSNIPPET(CLZ,NAME,instr_ptr)\
	unsigned long long t = ((instr_ptr)/STUB_SIZE)*STUB_SIZE;\
	unsigned long long * record = (unsigned long long *)(t);\
	register RET (*k)(void *, ...) = 0; \
	void * this = 0; \
	k = (void*)record[0];\
	this = (void*)record[1]; \
	return k(this,__VA_ARGS__); }\
	static RET CLZ##_##NAME(this, ##__VA_ARGS__) CLZ this; TYPES

/**
 * Same as COO_DEF_RET_ARGS except there is not return type.
 */
#define COO_DEF_NORET_ARGS(CLZ,NAME,TYPES,...) \
	static void stub_##CLZ##_##NAME(__VA_ARGS__) TYPES {\
	unsigned long long instr_ptr = 0;\
	ASMSNIPPET(CLZ,NAME,instr_ptr); \
	unsigned long long t = ((instr_ptr)/STUB_SIZE)*STUB_SIZE;\
	unsigned long long * record = (unsigned long long *)(t);\
	register void (*k)(void *, ...) = 0; \
	void * this = 0; \
	k = (void*)record[0];\
	this = (void*)record[1]; \
	k(this,__VA_ARGS__); }\
	static void CLZ##_##NAME(this, ##__VA_ARGS__) CLZ this; TYPES

/**
 * Same as COO_DEF_RET_ARGS except there is no arguments.
 */
#define COO_DEF_RET_NOARGS(CLZ,RET,NAME) \
	static RET stub_##CLZ##_##NAME() {\
	unsigned long long instr_ptr = 0;\
	ASMSNIPPET(CLZ,NAME,instr_ptr); \
	unsigned long long t = ((instr_ptr)/STUB_SIZE)*STUB_SIZE;\
	unsigned long long * record = (unsigned long long *)(t);\
	register RET (*k)(void *) = 0; \
	void * this = 0; \
	k = (void*)record[0];\
	this = (void*)record[1]; \
	return (RET)k(this);}\
	static RET CLZ##_##NAME(this) CLZ this;

/**
 * Same as COO_DEF_RET_ARGS except there are no arguments and return type.
 */
#define COO_DEF_NORET_NOARGS(CLZ,NAME) \
	static void stub_##CLZ##_##NAME() {\
	unsigned long long instr_ptr = 0;\
	ASMSNIPPET(CLZ,NAME,instr_ptr); \
	unsigned long long t = ((instr_ptr)/STUB_SIZE)*STUB_SIZE;\
	unsigned long long * record = (unsigned long long *)(t);\
	register void (*k)(void *) = 0; \
	void * this = 0; \
	k = (void*)record[0];\
	this = (void*)record[1]; \
	k(this); }\
static void CLZ##_##NAME(this) CLZ this;

/**
 * Detach from {OBJ} the member {NAME} and free the corresponding COO-entry.
 */
#define COO_DETACH(OBJ,NAME) {\
	deallocate_stub_function(OBJ->NAME);}

/**
 * Initialize COO
 */
bool InitializeCOO(uint no_fkt, Memory m);

/**
 * TODO(rwz): Need to be implemented.
 * Teardown COO
 */
void TeardownCOO();

#endif /* OSAL_INCLUDE_COOV3_H_ */
