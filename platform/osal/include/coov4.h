/**
 * C Object Orientation Library 4th incarnation (aka CooV4).
 *
 *
 * In this fourth version we replace many of the earlier macro's with 
 * custom assembly routines called as functions.
 *
 * @author: Rasmus Winther Zakarias
 * @date: Feb 2015
 * @copyright: Aarhus University
 *
 *
 * The C-Object Orientation Library is a simple yet powerful library
 * that offers a {this} pointer in struct member functions. If a
 * member function is defined with COO_DEF and bound to a struct
 * function pointer by COO_attach the function body will have access 
 * to the struct-instances through a {this}-pointer.
 *
 * Example:
 *
 * typedef struct _my_car {
 *    float fuel;
 *    void (*drive)(uint km);
 * } * MyCar;
 *
 * COO_DEF(MyCar,void,drive,uint km) {
 *  this->fuel -= 0.3*km; // <--- The this argument is automatically 
 *  // computed in this context by COO_DEF.
 * }}
 *
 * The motivation for using COO rather than C++ is that some HSM
 * implementations do not have full C++ support if any. Also,
 * experimental FPGA systems can use this software with only a
 * C-compiler and a few lines of inline assembly.
 *
 */
#include <common.h>
#include <osal.h>

/**
 * Setup COO library to have be able to attach {cfnt}
 * functions simultaneously. (takes 64*{cfnt} bytes of memory).
 *
 * @param cfnt - function count 
 * @param oe   - Operating Environment
 *
 * @return returns True if the library was successfully setup.
 */
bool COO_setup(OE oe, int cfnt);

/**
 * Attach the function at {function} to a struct function pointer
 * having {instance} as its this pointer.
 *
 * @param instance - the object instance
 * @param function - the static c-function
 *
 * @return - address of callable code that first sets up the
 * environment and then invokes {function} such that
 * {COO_internal_getThis} can acquire the this point.
 */
void * COO_attach(void * instance, void * function);

/**
 * Free the memory allocated for the given function.
 *
 * @param function - member function that was attached with
 * COO_attach.
 *
 */
void COO_detach(void * function);

/**
 * Short hand macro for defining a member function in which {this}
 * points to a struct to which this function was attached.
 *
 * @CLZ - The struct pointer that is the class 
 * @RET - The return type
 * @NAM - Function name
 * @ARGS- veriadic argument list 
 */
#define COO_DEF(CLZ,RET,NAM,...)\
  static RET CLZ##_##NAM(__VA_ARGS__) {\
  extern void * COO_internal_getThis();\
  CLZ this = (CLZ)COO_internal_getThis();

