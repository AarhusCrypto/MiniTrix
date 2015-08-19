/*

Copyright (c) 2013, Rasmus Zakarias, Aarhus University
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software

   must display the following acknowledgement:

   This product includes software developed by Rasmus Winther Zakarias 
   at Aarhus University.

4. Neither the name of Aarhus University nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY Rasmus Zakarias at Aarhus University 
''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Rasmus Zakarias at Aarhus University BE 
LIABLE FOR ANY, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Created: 2014-10-26

Author: Rasmus Winther Zakarias, rwl@cs.au.dk

Changes: 
2014-10-26 14:27: Initial version created
*/

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
#include <intkeymap.h>

#ifdef __cplusplus
extern "C" {
#endif


// ----------------------------------------
// Forward declarations
// ----------------------------------------
struct _operating_environment_;

// ----------------------------------------
// Native Library support
// ----------------------------------------
typedef void * (*DefaultConstructor)(struct _operating_environment_ * oe);
extern const char * DATE_TIME_LIBRARY;
extern const char * TERMINAL_LIBRARY;
extern const char * NETWORK_LIBRARY;
extern const char * GRAPHICS_LIBRARY;
  
// ----------------------------------------
// Useful Data encapsulation 
// ----------------------------------------
typedef struct _data_ {
    uint ldata;
    byte * data;
  } * Data;

// ----------------------------------------
// Semaphore Definition
// ----------------------------------------
typedef struct _cmaphore_ {
    struct _operating_environment_ * oe;
    int count;
    MUTEX lock;
  } * Cmaphore;

// ----------------------------------------
//                 ON ERRORS
//
// OSal reports errors with a return code (RC),
// the reason for errors are written to the logs.
// 
// 
// ----------------------------------------

// ----------------------------------------
// Error codes from OSAL all clients are 
// welcome to adapt these error codes.
// ----------------------------------------
  typedef enum _Return_Code {
	// everything is fine operation successful
    RC_OK,
	// file descriptor disconnected
    RC_DISCONN,
	// operation fail for unspecified reasons
    RC_FAIL,
	// no more memory
    RC_NOMEM,
	// arguments are inconsistent or wrong
    RC_BAD_ARGS,
    // data expected in some format, but wasn't
    RC_BAD_DATA,
    // blocking function interrupted.
    RC_INT,
    // instance impl not set
    RC_IMPL_NOTSET,
    // Try again with file descriptor
    RC_TRY_AGAIN,
  } RC;

 // ----------------------------------------
 // Log Levels for instance OE oe; {oe->p} function
 // the default trace-log function and for use
 // with syslog.
 // ----------------------------------------
  typedef enum {
    /* All that crap implementors wants to see that makes no sense to
     * any one else.
     */
    OSAL_LOGLEVEL_TRACE=0x00,

    /* Messages that might make sense for technical personal.
     */
    OSAL_LOGLEVEL_DEBUG=0x01,

    /* Warnings that end users should see. These indicates reduced
     * functionality or eminent occouring of that.
     */
    OSAL_LOGLEVEL_WARN=0x02,

    /* OK we died, tell the world why !
     */
    OSAL_LOGLEVEL_FATAL=0x04,

    /*
     * Print to user. Message here should arrive in the hands of the
     * end user not only in the log. Examples include Low level message 
	 * that the end-user cannot ignore, use this carefully.
     */
    OSAL_LOGLEVEL_USER=0x08,

  } LogLevel;

  // ----------------------------------------
  // Type definitions
  // ----------------------------------------
  typedef void* (*ThreadFunction)(void *);
  typedef uint ThreadID;
  typedef uint FD;


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
  // Input/Output through file descriptors
  // ------------------------------------------------------------
  /*!
   * read from {fd} into {buf} up to {lbuf} bytes. 
   *
   * Return RC_OK on success otherwise a failure occurred.
   *
   * @param fd  - osal file descriptor
   * @param buf - to which data are written
   * @param lbuf- initially hold the memory available in buf 
   *              upon success lbuf holds the number of bytes 
   *              read.
   *
   * @return RC_OK on success, *{lbuf} holdes the numbe of bytes read
   * into {buf}.
   */
  RC (*read)(FD fd, byte * buf, uint * lbuf);

  /*!
   * write to {fd} *{lbuf} bytes from {buf}.
   * 
   * Return RC_OK on success, and *{lbuf} will 
   * contain the number of bytes written.
   *
   */
  RC (*write)(FD fd, byte * buf, uint * lbuf);

  /*!
   * open file descriptor for the resource identified by {name}.
   * 
   * \param fd - out parameter that will hold the allocated 
   *             file descriptor. open fails if fd is null.
   * 
   * ex: if {name} is "ip 192.168.2.1:42" opens an file descriptor on
   * ethernet to that ip address to the given port after colon.
   *
   * if {name} is "file home/rwl/memory.txt" opens a file descriptor
   * to the specified file on the file system.
   *
   * \return RC_OK if allocation of resource was successfull
   */
  RC (*open)(const char * name, FD * fd);

  /*!
   * free resources allocated for {fd}.
   *
   * \return RC_OK on success 
   */
  RC (*close)(FD fd);
  
  /*!
   * if the given file descriptor {fd} is a listening socket {accept}
   * will return the first incoming connection after invocation
   * waiting.
   */
  RC (*accept)(FD server_fd, FD * client_fd);


  // ------------------------------------------------------------
  // Threads
  // ------------------------------------------------------------
  
  /*!
   * start a new thread with the given args and argument.
   *
   * \param {tid} - therad id out 
   * \param {tf}  - entry point for this thread
   * \param {args}- optional arguments for {tf}
   *
   * \return RC_OK on success.
   */
  RC (*newthread)(ThreadID * tid, ThreadFunction tf, void * args);

  /*
   * Blocks or preferably reschedules the current thread for {usec}
   * micro seconds 1/10^6 of a second.
   *
   * \param {usec} - number of micro (1/16^6) seconds to block
   *
   * \returns RC_OK on success sleeping the specified amount of time
   * may return RC_INT if the calling thread were interrupted.
   */
  RC (*usleep)(uint usec);


  /*!
   * Put the calling thread in the back of the scheduling queue and run
   * the scheduler.
   *
   * \return RC_OK always
   */
  RC (*yieldthread)(void);

  /*!
   * join with the thread given id {tid}.
   * 
   * \param {tid} - thread identifier to wait for
   *
   * \return RC_OK when thread {tid} has terminated. 
   */
  RC (*jointhread)(ThreadID tid);

  /*!
   * Return the number of active threads created by this 
   * OSal instance. Note this excludes the main-thread 
   * and threads created by other instances.
   * 
   * Cannot fail
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
   * \param {fresh_mutex} - handle for fresh mutex
   * 
   * \return RC_OK on success.
   */
  RC (*newmutex)(MUTEX * fresh_mutex);

  /*!
   * Acquire the given MUTEX {m} or block the calling thread until {m}
   * becomes available.
   *
   * \param {m} - the mutex to acquire
   *
   * \return RC_OK if lock acquired. 
   */
  RC (*lock)(volatile MUTEX m);

  /*! 
   * Release the given MUTEX {m}. 
   * 
   * \return RC_OK if lock on {m} freed.
   */
  RC (*unlock)(volatile MUTEX m);

  /*!
   * Destroy mutex {m} releasing its resources.
   * 
   */
  RC (*destroymutex)(MUTEX * m);

  /*!
   * Create a new semaphore with the given count
   *
   * \param {s}     - the semaphore to create
   * \param {count} - number of permits before blocking
   * 
   * \return RC_OK of success.
   */
  RC (*newsemaphore)(Cmaphore * s, uint count);

  /*!
   * Count Semaphore down one, block the calling thread if the given
   * semaphore {c} is zero.
   *
   * \return RC_OK on success
   */
  RC (*down)(Cmaphore c);

  /*!
   * Destroy semaphore releasing its resources.
   *
   * \return RC_OK on success
   */
  RC (*destroysemaphore)(Cmaphore * c);

  /*!
   * Count semaphore up, releasing on thread waiting on {c}.
   *
   * \param {c} - semaphore to add permit to.
   * 
   *
   * \return RC_OK on success
   */
  RC (*up)(Cmaphore c);


  // ------------------------------------------------------------
  // helpers
  // ------------------------------------------------------------


  /*!
   * set log level. All levels below the set log level will be
   * ignored and never reach the user.
   */
  void (*setloglevel)(LogLevel level);

  /*!
   * write a trace message to the system log.
   */
  void (*syslog)(LogLevel level, const char * fmt,...);

  /*!
   * print to the syslog on warn. equivalent to syslog(OSAL_LOGLEVEL_WARN, msg);
   */
  void (*p)(const char * fmt,...);

  /*!
   * Print to the user on OSAL_LOGLEVEL_USER behaving like {printf}.
   */
  void (*print)(const char * fmt, ...);

  /*!
   * The name of some file to log instead of stdout.
   */
  void (*set_log_file)(char * filename);

  /*!
   * Get instance of registered library object.
   */
  void * (*getSystemLibrary)(const char * name);

  /*!
   * Register a library with {name} constructed by {f}.
   */
  void * (*provideSystemLibrary)(const char * name, DefaultConstructor f);

  void * impl;

} * OE;

// ----------------------------------------
// int key map constructor and destructor
// ----------------------------------------
IntKeyMap IntKeyMap_New(OE oe);
void IntKeyMap_Destroy(IntKeyMap * map);

// ----------------------------------------
// Create platform default operating system 
// abstraction layer.
// ----------------------------------------
OE OperatingEnvironment_New();
void OperatingEnvironment_Destroy(OE * oe);
  

// ----------------------------------------
// Data helper
// ----------------------------------------
  Data Data_new(OE oe,uint size);
  Data Data_copy(OE oe, Data other);
  void Data_destroy(OE oe, Data * d);
  Data Data_shallow(byte * d, uint ld);
  bool Data_equal(Data a, Data b);
#define Data_Shallow(A,B) Data_shallow((byte*)(A),(uint)(B))

#ifdef __cplusplus
}
#endif


#endif
