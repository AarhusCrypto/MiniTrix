/*!
 * \file
 * Author: Rasmus Winther Lauritsen
 *
 * Operating System Abstraction Layer
 *
 * This module abstracts operating system calls and structures.
 *
 */
#ifndef OSAL_H
#define OSAL_H

#include "common.h"
#include "mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

  struct _operating_environment_;

  typedef struct _data_ {
    uint ldata;
    byte * data;
  } * Data;

  /*
   * A semaphore, and what we tell the world about it.
   */
  typedef struct _cmaphore_ {
    struct _operating_environment_ * oe;
    int count;
    MUTEX lock;
  } * Cmaphore;

  typedef enum _Return_Code {
    RC_OK,
    RC_FAIL,
  } RC;

  typedef enum {
    /* All that crap implementors wants to see that makes no sense to
     * any one else.
     */
    OSAL_LOGLEVEL_TRACE,
    /* Messages that might make sense for technical personal.
     */
    OSAL_LOGLEVEL_DEBUG,
    /* Warnings that end users should see. These indicates reduced
     * functionality or eminent occouring of that.
     */
    OSAL_LOGLEVEL_WARN,
    /* OK we died, tell the world why !
     */
    OSAL_LOGLEVEL_FATAL
  } LogLevel;

  typedef void* (*ThreadFunction)(void *);
  typedef uint ThreadID;
  

/*!
 * \struct an instance of struct _operating_environment_ provides
 * access to core functionality on the system. This includes:
 *
 * 0] Allocation and freeing of memory
 * 1] Open and closing I/O
 * 2] Read and writing I/O
 * 3] Starting and Stoping threads
 * 4] Locking and Blocking
 */
typedef struct _operating_environment_ {

  // ------------------------------------------------------------
  // Core functionality 
  // ------------------------------------------------------------

  char * (*get_version)(void);

  // ------------------------------------------------------------
  // Memory management
  // ------------------------------------------------------------
  /*!
   * Allocation of {size} memory
   * 
   */
  void *(*getmem)(uint size);

  /*!
   * Freeing memory allocated at {p}
   *
   */
  void (*putmem)(void * p);
  

  // ------------------------------------------------------------
  // Input/Output thriugh file descriptors
  // ------------------------------------------------------------
  /*!
   * read from {fd} into {buf} up to {lbuf} bytes. 
   *
   * Return the number of bytes actually read.
   *
   * This function only returns zero if lbuf is zero or an error
   * occurred (connection closed, or some other kind of read error).
   */
  RC (*read)(uint fd, byte * buf, uint * lbuf);

  /*!
   * write to {fd} {lbuf} bytes from {buf}.
   * 
   * Return the number of bytes actually written.
   */
  RC (*write)(uint fd, byte * buf, uint lbuf);


  /*!
   * open file descriptor for the resource identified by {name}.
   * 
   * \return file descriptor
   * 
   * ex: if {name} is "ip 192.168.2.1:42" opens an file descriptor on
   * ethernet to that ip address to the given port after colon.
   *
   * if {name} is "file home/rwl/memory.txt" opens a file descriptor
   * to the specified file on the file system.
   */
  uint (*open)(const char * name);

  /*!
   * free resources allocated for {fd}.
   *
   * \return 0 on success 
   */
  int (*close)(uint fd);
  
  /*!
   * if the given file descriptor {fd} is a listening socket {accept}
   * will return the first incoming connection after invocation
   * waiting.
   */
  uint (*accept)(uint fd);
  // ------------------------------------------------------------
  // Threads
  // ------------------------------------------------------------

  /*!
   * start a new thread with the given args and argument.
   *
   */
  ThreadID (*newthread)(ThreadFunction tf, void * args);

  /*!
   * Put the calling thread in the back of the scheduling queue and run
   * the schenduler.
   */
  void (*yieldthread)(void);

  /*!
   * join with the thread given id {tid}.
   * 
   */
  void* (*jointhread)(ThreadID tid);

  /*!
   * Return the number of active threads.
   */
  uint (*number_of_threads)(void);

  /*!
   * Return id of the calling thread.
   */
  ThreadID (*get_thread_id)(void);

  // ------------------------------------------------------------
  // Locking
  // ------------------------------------------------------------

  /*!
   * Create a new MUTEX which is free/unlocked.
   *
   * \return (void*)0 on failure, non zero otherwise.
   */
  MUTEX (*newmutex)(void);

  /*!
   * Acquire the given MUTEX {m} or block the calling thread until {m}
   * becomes available.
   */
  void (*lock)(volatile MUTEX m);

  /*! 
   * Release the given MUTEX {m}. 
   *  
   */
  void (*unlock)(volatile MUTEX m);


  /*!
   * Destroy mutex {m} releasing its resources.
   * 
   */
  void (*destroymutex)(MUTEX * m);

  /*!
   * Create a new semaphore with the given count
   *
   */
  Cmaphore (*newsemaphore)(uint count);

  /*!
   * Count Semaphore down one, block the calling thread if the given
   * semaphore {c} is zero.
   *
   */
  void (*down)(Cmaphore c);

  /*!
   * Destroy semaphore releasing its resources.
   *
   */
  void (*destroysemaphore)(Cmaphore * c);

  /*!
   * Count semaphore up, releasing on thread waiting on {c}.
   */
  void (*up)(Cmaphore c);

  // ------------------------------------------------------------
  // helpers
  // ------------------------------------------------------------
  /*!
   * write a trace message to the system log.
   *
   */
  void (*syslog)(LogLevel level, const char * msg);

  /*!
   * print to the syslog on warn. equivalent to syslog(OSAL_LOGLEVEL_WARN, msg);
   */
  void (*p)(const char * msg);

  void * impl;
} * OE;
   /*
   * Create a linux based operating environment abstraction layer.
   */
  OE OperatingEnvironment_LinuxNew();

  void OperatingEnvironment_LinuxDestroy( OE * oe );

  Data Data_new(OE oe,uint size);
  Data Data_copy(OE oe, Data other);
  Data Data_destroy(OE oe, Data * d);
  Data Data_shallow(byte * d, uint ld);

#ifdef __cplusplus
}
#endif


#endif
