#ifndef COO_H
#define COO_H
#include <memory.h>
#define STUB_SIZE 512
#include<stdio.h>

void set_rbx();
/* COO ATTACH
 *
 *
 * Given the name of a struct-pointer type {CLZ} and an instance (a
 * pointer) of {OBJ} of that type assign the function defined and
 * declared to a member of that name. E.g. setup the function pointer
 * for {NAME} in {OBJ} of type {CLZ-pointer}.
 *
 * The coo_patch function searches for a special pointer in the code
 * generated for a static function named stub_{CLZ}_{NAME} and
 * replaces this special pointer with {OBJ}. In effect the patched
 * function returned invokes the function {CLZ}_{NAME} with a pointer
 * to {OBJ} (called this) as its first argument. 
 *
 * printf("FATAL: Unable to attach %s\n", #NAME);
 */
#define COO_ATTACH(CLZ,OBJ,NAME) {					\
    OBJ->NAME = coo_patch((byte*)stub_##CLZ##_##NAME, STUB_SIZE, OBJ ); \
    if ( !( (OBJ->NAME) ) ) {                                           \
                                                                        \
    }}


#define COO_ATTACH_FN(CLZ,OBJ,NAME,FN) {                                \
    OBJ->NAME = coo_patch((byte*)stub_##CLZ##_##FN, STUB_SIZE , OBJ );  \
    if ( !( (OBJ->NAME) ) ) {                                           \
                                                                        \
    }}                                                                  

/* COO_DCL
 *
 * Declare a member function for class {CLZ} returning {RET} having
 * name {NAME} taking argument __VA_ARGS__.
 *
 */
#define COO_DCL(CLZ, RET, NAME, ...)		\
  static RET CLZ##_##NAME(void * t, ##__VA_ARGS__);

/* COO_DEF_RET_ARGS
 *
 * Define a member function that returns a value and takes arguments.
 *
 * First the stub "RET stub_{CLZ}_{NAME}"-function is defined having a
 * variable with the magic pointer 0xDEADBEEFDEADBEEF. coo_patch will
 * look for an repleace this pointer when this stub-code is copied to
 * rwx memory and assignmed to a function pointer with COO_ATTACH.
 *
 * The {regsiter} keyword is used to prohibit compiler optimizations !
 */
#define COO_DEF_RET_ARGS(CLZ, RET, NAME, TYPES, ...)	\
  static __attribute__((optimize("O0"))) RET stub_##CLZ##_##NAME(__VA_ARGS__) TYPES {                   \
    asm("\n");                                                          \
    register void * ths = (void*)0xDEADBEEFDEADBEEFL;                   \
    register RET (*k)(void * t, ...) = (void*)&CLZ##_##NAME;            \
    asm("\n");                                                          \
    RET val = k(ths, __VA_ARGS__ );                                     \
    return val;                                                         \
  }                                                                     \
                                                                        \
  static RET CLZ##_##NAME(t, ##__VA_ARGS__) void * t; TYPES             \
  {                                                                     \
  register  CLZ this = (CLZ)t;

/* COO_DEF_RET_NOARGS
 *
 * Defines a member function that does return something but takes no
 * arguments.
 *
 * Otherwise as COO_DEF_RET_ARGS above.
 */
#define COO_DEF_RET_NOARGS(CLZ, RET, NAME)                              \
  static __attribute__((optimize("O0"))) RET stub_##CLZ##_##NAME(void)  \
  {                                                                     \
    asm("\n");                                                          \
    volatile register void * ths = (void*)0xDEADBEEFDEADBEEFL;          \
    register RET (* k)(void * t) = (void*)&CLZ##_##NAME;                \
    asm("\n");                                                            \
    RET val = k((void*)ths);                                            \
  return val;                                                           \
  }                                                                     \
  static RET CLZ##_##NAME(void * t)                                     \
  {                                                                     \
  register CLZ this = (CLZ)t;

/* COO_DEF_NORET_NOARGS
 *
 * Defines a member function that doesn't return anything nor takes
 * any arguments.
 *
 *
 * Otherwise as COO_DEF_RET_ARGS above. 
 */
#define COO_DEF_NORET_NOARGS(CLZ, NAME)                                 \
  static __attribute__((optimize("O0"))) void stub_##CLZ##_##NAME(void) \
  {                                                                     \
    asm("\n");                                                          \
    register void * ths = (void*)0xDEADBEEFDEADBEEFL;                   \
    register void (*k)(void * t) = (void*)&CLZ##_##NAME;                \
    asm("\n");                                                          \
    k(ths);                                                             \
  }                                                                     \
  static void CLZ##_##NAME(void * t)                                    \
  {                                                                     \
  register CLZ this = (CLZ)t;

/* COO_DEF_NORET_ARGS
 *
 * Defines a member function that doesn't return anything but takes
 * arguments.
 *
 * Otherwise as COO_DEF_RET_ARGS above.
 */


#define COO_DEF_NORET_ARGS(CLZ, NAME, TYPES, ...)	\
  static __attribute__((optimize("O0"))) void stub_##CLZ##_##NAME(__VA_ARGS__) TYPES \
  {                                                           \
    asm("\n");                                                \
    register void * ths = (void*)0xDEADBEEFDEADBEEFL;         \
    register void (*k)(void * t, ...) = (void*)&CLZ##_##NAME; \
    asm("\n");                                                \
    k(ths, __VA_ARGS__ );                                     \
    return;                                                   \
  }                                                           \
  static void CLZ##_##NAME(t, ##__VA_ARGS__) void * t; TYPES	\
  {							\
  register CLZ this = (CLZ)t;

/* COO_DETACH
 *
 * Remove member function and free up special rwx memory.
 */
#define COO_DETACH(OBJ, NAME) {		\
    byte * patch = (byte*)OBJ->NAME;            \
    coo_depatch(patch);                         \
}

/*
 * Initialise COO with {m} being a rwx memory.
 */
void coo_init(Memory m);

/*
 * Teardown COO
 */
void coo_end();

/*
 * coo_patch function. Takes a piece of code pointed to by stub and
 * searches for the pattern 0xDEADBEEFDEADBEEF and replaces that with
 * the value stored in this in little endian.
 */ 
void * coo_patch(byte * stub, uint lstub, void * this );

/*
 * Free stub function from (special) rwx memory.
 */
void coo_depatch(byte * stub);

#ifdef LINUX
Memory LinuxSpecialMemoryNew(Memory m) ;
Memory LinuxMemoryNew(void);
#endif

#endif
