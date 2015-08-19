/*!
 * \file Double linked list queue implementation.
 *
 */
#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <osal.h>

typedef void (*FreeFunction)(void*);

/*!
 * \struct Blocking Queue structure.
 *
 * \param {elements}   - an array of elements stored in this Queue
 * \param {lelements}  - length of elements
 * \param {inserts}    - semaphore to count taken places in the queue
 * \param {takeouts}   - semahpore to count free places in the queue
 * \param {free}       - function that free one element in elements.
 * 
 * Example(With elements that need not be freed):
 *
 * typedef struct _complex_ {
 *   double real;
 *   double imaginal;
 * } Complex;
 *
 * int main(int c, char **a) {
 *   Complex a={0},b={0};
 *   Complex * c = 0;
 * 
 *   a.real = 42.0;a.imaginal=43.0;
 *
 *   BlkQueue Q = BlkQueue_new(2);
 *
 *   BlkQueue_push(Q,&b);   
 *   BlkQueue_push(Q,&a);
 *   
 *   c = BlkQueue_take(Q);
 *   if (c != &b) { printf("Oh, no the queue is not working\n");exit(-1); }
 *
 *   BlkQueue_destroy(&Q); // this leaves the pointer (&b) in the
 *                         // queue as is.  
 * }
 *
 * Example(With elements that need to be freed):
 *
 * Complex * Complex_new(double real, double imaginal) {
 *   Complex * r = malloc(sizeof(*r));
 *   if (!r) return 0;
 *   r->real =real;
 *   r->imaginal = imaginal;
 *   return r;
 * }
 *
 * void free_complex(void * c) {
 *   Complex * r = (Complex*)c;
 *   if (r) {
 *    memset(c,0,sizeof(*c));
 *    free(c);
 *   }
 * }
 *
 * int main(int c, char **a) {
 *  Complex * a = Complex_new(42.0,42.0);
 *  Complex * b = Complex_new(43.0,43.0);
 *  Complex * c = 0;
 *
 *  BlkQueue Q = BlkQueue_newown(2, free_complex);
 *
 *  if (!BlkQueue_push(Q,a)) { // the queue failed to take ownership of {a}
 *    free_complex(a); 
 *    BlkQueue_destroy(Q);
 *    return -1;
 *  }
 *
 *  if (!BlkQueue_push(Q,b)) { // the queue failed to take ownership of {b}
 *    free_complex(b);
 *    BlkQueue_destroy(Q);
 *    return -2;
 *  }
 *  
 *  c = BlkQueue_take(Q);
 *  if (c != a) { printf("Oh no the Queue is not working properly.\n");exit(-1); }
 *
 *  free_complex(c);a=c=0; // and now a is free too, because coming here means a == c
 *
 *  BlkQueue_destroy(Q); // This will free b as a free function was
 *                       // provided to BlkQueue_newown as 2nd arg.
 *  
 * }
 */
typedef struct _queue_ {
  OE oe;
  void * elements;
  uint lelements;
  uint front,back;
  Cmaphore inserts;
  Cmaphore takeouts;
  MUTEX lock;
  FreeFunction ff;
  void (*put)(void * elm);
  void *(*get)(void);
  uint (*size)(void);
} * BlkQueue;

/*!
 * \brief create a new blocking queue instance.
 *
 * \param {size} - the initial {size} of the queue must be greater
 * than zero.
 *
 * \return A non-zero pointer to struct _queue_ on success. Otherwise
 * Null (this includes when invoked with {size} equal null).
 */
  BlkQueue BlkQueue_new(OE oe, uint size);

/*!
 * \brief create a new blocking queue instance.
 *
 * \param {size} - the initial {size} of the queue must be greater
 * than zero.
 *
 * \param {ff}   - a free function
 *
 * \return A non-zero pointer to struct _queue_ on success. Otherwise
 * Null (this includes when invoked with {size} equal null).
 */
  BlkQueue BlkQueue_newown(OE oe, uint size,FreeFunction ff);


  /*!
   * \brief get a copy of the pointers in the queue. This copy is
   * created atomically and represents a consistent view of the
   * content of the queue at some point in time after the invocation
   * of {getcopy}.
   *
   * \param {Q}   - the queue to extract a copy of elements from.
   * \param {size}- the size of the returned array of elements.
   *
   * \return a list of elements *{size} long on success. Null
   * otherwise and the *{size} is not changed.
   */
  void * BlkQueue_getcopy(BlkQueue Q, uint * size);

/*!
 * \brief block the calling thread until there is room for adding an
 * element to the queue.
 *
 * \param {q}       - the queue to add {element} to
 * \param {element} - an element to add
 *
 * Memory: This queue takes ownership of the memory pointed to by
 * element and free's it 
 *
 * \return non zero if {q} takes ownership of {element}.
 */
int BlkQueue_push(BlkQueue q, void * element);

/*!
 * \brief block the calling thread until there is something in the
 * queue to take out. The returned element is removed from the queue.
 *
 * \param {q}    - the queue to take an element from.
 *
 * \return the element in front of the Queue.
 *
 */
void * BlkQueue_take(BlkQueue q);

/*!
 * \brief frees the memory taken by this queue. If a free-function was
 * provided to the BlkQueue_new when {Q} was created all present
 * non-null elements will be passed to that function before the queue
 * is freed.
 *
 * Threading: The inserts and takeouts are both depleted when destroy
 * starts thus all takes and push's will hang until the completion of
 * this function. Then both inserts and takeouts are destroyed and
 * then all threads are free to continue in which case the take
 * function will return Null and push will return zero to indicate
 * that ownership of the given element wasn't accomplished.
 *
 * \param {Q}   - the queue to destroy
 *
 */
void BlkQueue_destroy(BlkQueue * Q);

#ifdef __cplusplus
}
#endif

#endif
