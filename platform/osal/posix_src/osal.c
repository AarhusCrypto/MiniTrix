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
2014-10-26 14:30: Initial version created
*/

/*!
 * Linux Operating System Abstraction Layer
 * 
 * Author: Rasmus Winther Lauritsen
 * 
 * 
 */

// Framework
#include "osal.h"
#include "coov4.h"
#include "singlelinkedlist.h"
#include <common.h>
#include <config.h>
#include <memory.h>

// libc (stdandard libc functions)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

// Linux System
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <netinet/in.h>

List SingleLinkedList_new(OE oe);

// Library names for names which can be expected of the standard platform.
const char * DATE_TIME_LIBRARY = "OSAL_DATE_TIME_LIBRARY";
const char * TERMINAL_LIBRARY = "OSAL_TERMINAL_LIBRARY";
const char * NETWORK_LIBRARY = "OSAL_NETWORK_LIBRARY";
const char * GRAPHICS_LIBRARY = "OSAL_GRAPHICS_LIBRARY";

extern char *strerror (int __errnum);

// static structure boot-strapping the osal system
MUTEX _static_lock;
static uint no_osal_instance;

// informative return value
typedef struct _osal_ret_val_ {
  uint err;
  int rval;
} retval;

// static read libstdc is static in structure
// _static_lock protects libc.
static
retval __read(int fd, byte * buf, uint lbuf) {
  retval res = {0};
  Mutex_lock(_static_lock);
  res.rval = read(fd, buf, lbuf);
  res.err = errno;
  Mutex_unlock(_static_lock);
  return res;
}

// static write libstdc is static in structure
// _static_lock protects libc.
static
retval __write(int fd, byte * buf, uint lbuf) {
  retval res = {0};
  Mutex_lock(_static_lock);
  res.rval = write(fd, buf, lbuf);
  res.err = errno;
  Mutex_unlock(_static_lock);
  return res;
}


// ------------------------------------------------------------
Cmaphore Cmaphore_new(OE oe, uint count) {
  Cmaphore res = (Cmaphore)oe->getmem(sizeof(*res));
  if (!res) return 0;
  oe->newmutex(&(res->lock));
  res->count = count;
  res->oe = oe;
  return res;
};

void Cmaphore_up(Cmaphore c) {
  OE oe = 0;
  if (!c) return;
  if (!c->lock) return;
  c->oe->lock(c->lock);
  ++(c->count);
  c->oe->unlock(c->lock);
}

void Cmaphore_down(Cmaphore c) {
  OE oe = 0;
  if (!c) return;
  if (!c->lock) return;

  oe = c->oe;
  if (!oe) return;

  oe->lock(c->lock);
  while(c->count <= 0) {
    oe->unlock( c->lock );
    oe->yieldthread();
    oe->lock( c->lock );
    if (!c->oe)  { 
      oe->unlock(c->lock);
      return; // semaphore destroyed, leave !
    }
  }
  --(c->count);
  oe->unlock( c->lock );
}

void Cmaphore_destroy(Cmaphore * c) {
  OE oe = 0;
  Cmaphore s = 0;
  if (!c) return;
  if (!*c) return;

  s = *c;
  oe = s->oe;
  
  // inform all down's that we are destroying this semaphore
  oe->lock(s->lock);
  s->oe = 0;
  oe->unlock(s->lock);

  // give all other threads a chance to run.
  oe->yieldthread();

  // Really now we destroy it !
  oe->lock(s->lock);
  s->count = 0;
  oe->destroymutex(&s->lock);
  zeromem(s, sizeof(*s));
  *c = 0;
  oe->putmem(s);
}


// ------------------------------------------------------------


static
void set_non_blocking(int fd);

static
unsigned long long _nano_time() {
 return 0; }
#if 0
static
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
}
#endif

typedef struct _simple_oe_ {
  /*!
   * mm = main memory
   */
  Memory mm;
  /*!
   * sm = special memory in which writing and executing the same
   * addresses are allowed.
   */
  Memory sm;
  /*!
   * Threads allocated by this instance.
   */
  IntKeyMap threads;
  /*!
   * List of all open file descriptors
   */
  List filedescriptors;
  
  /*!
   * Lock that protects the state of this instance.
   */
  MUTEX lock;

  /*!
   * The current log level.
   */
  LogLevel loglevel;

  /*!
   * File descriptor to which logs are written. Stdout is zero.
   */
  int log_file;

} * SimpleOE;


typedef struct _file_descriptor_entry_ {
  int osfd;
  uint fd;
} * FDEntry;

uint fd_pool;
static uint FDEntry_new(OE oe, List entries, int osfd) {
  FDEntry r = (FDEntry)oe->getmem(sizeof(*r));
  if (!r) return 0;

  r->osfd = osfd;
  r->fd = ++fd_pool;

  entries->add_element(r);

  return r->fd;
}

static void FDEntry_destroy(OE oe, FDEntry * ent) {
  FDEntry e = 0;
  if (!ent) return;
  if (!*ent) return;
  e = *ent;

  oe->putmem(e);
  *ent = 0;
}

static int FDEntry_remove(OE oe, List entries, uint fd) {
  uint siz = 0;
  int i  = 0;

  if (!entries) return 0;
  
  siz = entries->size();
  for(i = 0;i < siz;++i) {
    FDEntry c = (FDEntry)entries->get_element(i);
    if (c && c->fd == fd) {
      int r = c->osfd;
      entries->rem_element(i);
      FDEntry_destroy(oe, &c);
      return r;
    }
  }
  return -1;
}

static int FDEntry_lookup(List entries, uint fd) {
  uint siz = 0;
  int i  = 0;

  if (!entries) return 0;
  
  siz = entries->size();
  for(i = 0;i < siz;++i) {
    FDEntry c = (FDEntry)entries->get_element(i);
    if (c && c->fd == fd) return c->osfd;

  }

  return -1;
}


COO_DEF( OE, void *, getmem, uint size)
  uint j = 0;
  byte * res = (byte*) malloc(size);
  if (!res) { 
    this->syslog(OSAL_LOGLEVEL_FATAL, "Out of memory");
    return 0;
  }
  for(j=0;j<size;++j) res[j] = 0;
  return res;
}

COO_DEF( OE, void, putmem, void * p)
  if (p != 0) {
    free(p); 
  }
}

COO_DEF(OE, RC, read, FD fd, byte * buf, uint * lbuf)
  int r = 0;
  SimpleOE soe = (SimpleOE)this->impl;
  int os_fd = 0;
  ull start = 0;
  retval ret = {0};

if (!lbuf) return RC_BAD_ARGS;

  if (fd < 1) return RC_BAD_ARGS;

  this->lock(soe->lock);
  os_fd = FDEntry_lookup(soe->filedescriptors,fd);
  this->unlock(soe->lock);

  if (os_fd < 0) return RC_FAIL; 

  {
    fd_set read_set = {0};
    struct timeval timeout = {0};
    FD_ZERO(&read_set);
    timeout.tv_sec = 0;
    timeout.tv_usec = 230; // latency on Giga bit LAN
    FD_SET(os_fd, &read_set);
    if (select(os_fd+1, &read_set, 0,0,&timeout) <= 0) { 
      *lbuf = 0;
      return RC_OK;
    } else {
      // OK CONTINUE
    }
  }

  start = _nano_time();
  ret = __read(os_fd,buf, *lbuf); 
  r = ret.rval;
  if (r == 0) {
    return RC_DISCONN;
  }

  if (r < 0) {
    if (ret.err == EAGAIN || ret.err == EWOULDBLOCK) {
      *lbuf = 0;
      return RC_OK;
    } else {
      return RC_FAIL; // unexpected error or end of file
    }
  } else {
    //    printf("Result return in %llu ns\n",_nano_time()-start);
    *lbuf = r;
  }

  return RC_OK;
}

COO_DEF(OE, RC, write, FD fd, byte * buf, uint * lbuf) {
  SimpleOE soe = (SimpleOE)this->impl;
  int writesofar = 0;
  int lastwrite = 0;
  int os_fd = 0;
  ull start = _nano_time();
  uint lb = 0;
  
  this->lock(soe->lock);
  os_fd = FDEntry_lookup(soe->filedescriptors,fd);
  this->unlock(soe->lock);
  
  if (os_fd < 0) return RC_FAIL;

  if (!lbuf) return RC_BAD_ARGS;

  if (!buf) return RC_BAD_ARGS;

  lb = *lbuf;

  while(writesofar < lb && lastwrite >= 0) {
    struct timeval t = {0};
    fd_set wfds = {0};
    retval r = {0};
    FD_SET(os_fd, &wfds);
    t.tv_usec = 230;
    select(os_fd+1, 0, &wfds, 0, &t);
    r = __write(os_fd, buf+writesofar, lb-writesofar);
    lastwrite = r.rval;
    if ( lastwrite == -1) {
      if (r.err == EAGAIN) {
        lastwrite = 0;
        continue;
      } 
      this->p("Error: %u %s os_fd=%u\n",r.err, strerror(r.err),os_fd);
      *lbuf = 0;
      return RC_FAIL;
    } 
    writesofar += lastwrite;
  }

  if (lastwrite < 0) {
    this->p("ERROR Failed to write");
    *lbuf = 0;
    return RC_FAIL;
  }

  *lbuf = writesofar;

  return RC_OK;
}}

/* proposal:
 *
 * file <path>          - open a file
 * net <ip or dns name> - make a client connection
 * listen://[<ip>:]port   - open server socket
 *
 * Returns non-zero on success, zero on failure.
 */
COO_DEF(OE, RC, open, const char * name, FD * pfd) {
  uint lname = 0;
  SimpleOE soe = 0;
  uint res = 0;

  if (!name) return RC_BAD_ARGS;
  lname = osal_strlen(name);

  soe = (SimpleOE)this->impl;
  if (!soe) return RC_NOMEM;
  
  if (!pfd) return RC_BAD_ARGS;

  if (lname > 5 && (mcmp((void*)"file ",(void*)name, 5) == 0)) {
    int fd = open(name+5, O_RDWR|O_CREAT|O_APPEND,S_IRWXU|S_IRGRP|S_IROTH);
    if (fd < 0) goto failure;
    
    this->lock(soe->lock);
    res = FDEntry_new(this,soe->filedescriptors,fd);
    *pfd = res;
    this->unlock(soe->lock);
    return RC_OK;
  }
  
  // listen for incoming connections
  if (lname > 7 && mcmp(( void*)"listen ",( void *)name,7) == 0)   {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    char port[6] = {0};
    int reuse_addr_option = 1;
    struct sockaddr_in serv_addr = {0};

    
    if (setsockopt(server_fd, SOL_SOCKET,
		   SO_REUSEADDR,
		   (char *)&reuse_addr_option, sizeof(reuse_addr_option)) < 0 ) {
      close(server_fd);
      return RC_FAIL;
    }

    sscanf(name+7,"%s", port);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(port)); 
    {
      socklen_t lserv_addr = sizeof(serv_addr);
      if ( bind ( server_fd, (struct sockaddr *)&serv_addr,	
		  lserv_addr) < 0) {
        close(server_fd);
        return RC_FAIL;
      }
    }

    {
      uint flags = fcntl(server_fd, F_GETFL, 0);
      fcntl(server_fd, F_SETFL, flags | O_NONBLOCK  );
    }

    if (listen(server_fd, 20) != 0) {
      return RC_FAIL;
    }
    this->lock(soe->lock); 
    res = FDEntry_new(this, soe->filedescriptors, server_fd);
    *pfd = res;
    this->unlock(soe->lock);
    return RC_OK;
  }


  // ip address
  if (lname > 3 && mcmp((void *) "ip ", (void*)name, 3) == 0) {
    int socket_fd = 0;
    char ip[20]={0},port[6]={0};
    struct sockaddr_in addr = {0};
    sscanf(name+3,"%s %s", ip, port);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) return RC_FAIL;

    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(atoi(port));

    if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      close(socket_fd);
      return RC_FAIL;
    }

    set_non_blocking(socket_fd);

    this->lock(soe->lock);
    res = FDEntry_new(this, soe->filedescriptors, socket_fd);
    *pfd =res;
    this->unlock(soe->lock);
    
    return RC_OK;
  }

 failure:
  {
    char m[128] = {0};
    char * e = (char*)strerror(errno);
    this->syslog(OSAL_LOGLEVEL_FATAL, 
		 "Failed to create file descriptor, \"%s\" see error below:",
		 name);
    this->syslog(OSAL_LOGLEVEL_FATAL ,e);
  }
  if (soe) {
    this->unlock(soe->lock);
  }
  return RC_FAIL;
}}



static 
void set_non_blocking(int fd) {
  int keep_alice_opt = 1;
  int tcp_nodelay_opt = 1;
  int lopt = sizeof(int);
  int flags = fcntl(fd, F_GETFL, &flags, sizeof(flags));
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


COO_DEF(OE, int, close, FD _fd)
  int fd = 0;
  SimpleOE soe = (SimpleOE)this->impl;
  FDEntry ent = 0;
  
  if (!soe) { 
    return RC_IMPL_NOTSET;
  }

  this->lock(soe->lock);
  fd = FDEntry_remove(this, soe->filedescriptors, _fd);
  if (fd >= 0) {
    close(fd);
  }
  this->unlock(soe->lock);

  return RC_OK;
}


COO_DEF(OE, RC, accept, FD server_fd, FD * client_fd_out) {
  SimpleOE soe = (SimpleOE)this->impl;
  int os_fd = 0;
  int os_client_fd = 0;
  uint res = 0;
  fd_set rd = {0};
  struct timeval timeout = {0};

  this->lock(soe->lock);
  os_fd = FDEntry_lookup(soe->filedescriptors, server_fd);
  this->unlock(soe->lock);

  if (os_fd < 0) return RC_FAIL;

  FD_ZERO(&rd);
  FD_SET(os_fd, &rd);
  timeout.tv_usec=100000;
  
  while(1) {
    select(os_fd+1,&rd,0,0,&timeout);
    os_client_fd = accept(os_fd,0,0);
    if (os_client_fd < 0) { 
      if (errno == EAGAIN) {
        usleep(0);
        this->yieldthread();
        continue;
      } else {
        return 0; // report failure
      } 
    } else {
      set_non_blocking(os_client_fd);
      break; // we have a client
    }
  }
  // RX ERR 4060083


  this->lock(soe->lock);
  res = FDEntry_new(this, soe->filedescriptors, os_client_fd);
  this->unlock(soe->lock);
  *client_fd_out = res;
  return RC_OK;
}}


COO_DEF(OE, RC, newthread, ThreadID * ptid, ThreadFunction tf, void * args) {
  static ThreadID idpool = 0xBABE0000;
  pthread_t * t = this->getmem(sizeof(*t));
  SimpleOE soe = (SimpleOE)this->impl;
  ThreadID tid = 0;

  if (pthread_create(t, 0, tf, args) == 0) {
    pthread_t tt = pthread_self();
    this->lock(soe->lock);
    tid = idpool;
    idpool += 1;
    soe->threads->put(tid,t);
    *ptid = tid;
    this->unlock(soe->lock);
    return RC_OK;
  }
  this->syslog(OSAL_LOGLEVEL_WARN, "newthread: failed to create thread");
  return RC_FAIL;
  }}

COO_DEF(OE, RC, usleep, uint usec) {
  usleep(usec);
  return RC_OK;
}}

COO_DEF(OE, RC, yieldthread)
  sched_yield();
  return RC_OK;
}

COO_DEF(OE, RC, jointhread, ThreadID tid){
  SimpleOE soe = (SimpleOE)this->impl;
  pthread_t * t = (pthread_t *)soe->threads->get(tid);
  void * p = 0;

  if (t) {
    pthread_join(*t,&p);
    this->lock(soe->lock);
    soe->threads->rem(tid);
    this->unlock(soe->lock);
  } else {
    this->p("Auch ! Thread not found");
    return RC_FAIL;
  }
  
return RC_OK;
}}

COO_DEF(OE, uint, number_of_threads);
  SimpleOE soe = (SimpleOE)this->impl;
  uint answer = 0;
  this->lock(soe->lock);
  answer = soe->threads->size()+1; // +1 is the main thread
  this->unlock(soe->lock);
  return answer;
}

COO_DEF(OE, RC, newmutex, MUTEX * m)
if (m) { 
  *m = Mutex_new(MUTEX_FREE);
  if (!*m) return RC_NOMEM;
 }

return RC_OK;
}

COO_DEF(OE, RC, destroymutex, MUTEX * m)
  Mutex_destroy(*m);
return RC_OK;
}

COO_DEF(OE, RC, lock, MUTEX m)
  //pid_t tid;
  //  char msg[64] = {0};
  //tid = syscall(SYS_gettid);
  //  sprintf(msg,"LOCK(%p) taken by THREAD(%d)",m,tid);
  //this->p(msg);
Mutex_lock(m);
return RC_OK;
}

COO_DEF(OE,RC, unlock,  MUTEX m) 
  //pid_t tid;
  //char msg[64] = {0};
  //tid = syscall(SYS_gettid);
  //sprintf(msg,"LOCK(%p) released by THREAD(%d)",m,tid);
  //this->p(msg);
  Mutex_unlock(m);  
return RC_OK;
}

COO_DEF(OE, RC, newsemaphore,Cmaphore * s, uint count)
if (s) {
*s = Cmaphore_new(this,count);
 if (!*s) return RC_NOMEM;
 }
return RC_OK;
}

COO_DEF(OE, RC, up, Cmaphore c)
  Cmaphore_up(c);
return RC_OK;
}

COO_DEF(OE, RC, down, Cmaphore c)
Cmaphore_down(c);
return RC_OK;
}

COO_DEF(OE, void, setloglevel, LogLevel level);
  SimpleOE soe = (SimpleOE)this->impl;
  soe->loglevel = level;
}

COO_DEF(OE, void, set_log_file, char * filename) {
  SimpleOE soe = (SimpleOE)this->impl;
  char fname[512] = {0};
  FD fd = 0;
  RC rc = RC_OK;
  osal_sprintf(fname,"file %s", filename);
  rc = this->open(fname, &fd);
  if (rc == RC_OK) {
    soe->log_file = fd;
  }
}}

#define SECONDS_PER_MINUTE 60

#define SECONDS_PER_HOUR 60*SECONDS_PER_MINUTE

#define SECONDS_PER_DAY 24*SECONDS_PER_HOUR

#define S_P_YEAR 365*SECONDS_PER_DAY

#define S_P_LYEAR 366*SECONDS_PER_DAY

static bool is_leap_year(uint year) {
  if ((year % 4 ==0 && year % 100 != 0) || year % 400 == 0) {
    return True;
  }
  return False;
}

static uint days_per_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};


static uint seconds_per_month(uint month,uint year) {
  uint days = days_per_month[month];
  if (month == 2 && is_leap_year(year)) days += 1;
  return SECONDS_PER_DAY*days;
}

// precondition cstr has at least 22 bytes available including 
// terminating zero.
static void unixtime_to_cstr(ull ut, char * cstr, uint utcoffset) {
  uint year = 1970;
  uint month = 0;
  uint day = 0;
  uint hour = 0;
  uint minute = 0;
  uint second = 0;
  
  // find the year
  while(ut >= (is_leap_year(year) ? S_P_LYEAR : S_P_YEAR)) {
    ut -= (is_leap_year(year) ? S_P_LYEAR : S_P_YEAR);
    year ++;
  }

  // find the month
  while(ut > days_per_month[month]*SECONDS_PER_DAY + (is_leap_year(year) ? 1 : 0)) {
    ut -= seconds_per_month(month,year);
    month += 1;
  }

  // find the day
  while (ut > SECONDS_PER_DAY) {
    ut -= SECONDS_PER_DAY;
    day += 1;
  }

  // find the hour
  while(ut > SECONDS_PER_HOUR) {
    ut-=SECONDS_PER_HOUR;
    hour += 1;
  }

  // find minute
  while(ut > SECONDS_PER_MINUTE) {
    ut -= SECONDS_PER_MINUTE;
    minute+= 1;
  }

  second = ut;
  
  osal_sprintf(cstr,"%04d-%02d-%02d %02d:%02d:%02d",
	       year,month+1,day+1,(hour+utcoffset)%24,
	       minute,second);
}

void do_sys_log(OE this,LogLevel level, const char * msg, va_list args) {
	SimpleOE soe = (SimpleOE)this->impl;
	char * buf = 0;
	uint lbuf = 1024*1024;
	uint o = 0;
	uint l = 0;
	char tstamp[25] = {0};

	if (!msg) msg = "(null)";

	if (level < soe->loglevel) return;

	buf = this->getmem(lbuf);
	if (buf == 0) {
	  return ;
	}

	unixtime_to_cstr(time(0),tstamp,2);

	switch (level) {
	case OSAL_LOGLEVEL_TRACE: {
	  o = sprintf(buf, "[%s] ",tstamp);
	} break;
	case OSAL_LOGLEVEL_DEBUG: {
	  o = sprintf(buf, "[%s] ",tstamp);
	} break;
	case OSAL_LOGLEVEL_WARN: {
	  o = sprintf(buf, "[%s] ",tstamp);
	} break;
	case OSAL_LOGLEVEL_FATAL: {
	  o = sprintf(buf, "[%s] ",tstamp);
	} break;

	default: {
	  o = sprintf(buf, "[%s] ",tstamp);
	} break;
	}

	l += o;
	o = vsprintf(buf + l, msg, args);
	l += o;
	o = sprintf(buf + l, "\n");
	l += o;

	if (soe->log_file != 0) {
	  uint w = l+1;
	  this->write(soe->log_file, (byte*)buf, &w);
	  this->putmem(buf);
	  return;
	}


	buf[lbuf-1] = 0;
	printf("%s",buf);
	this->putmem(buf);
}


COO_DEF(OE, void, syslog, LogLevel level, const char * msg,...) {
  va_list args = {0};
  va_start(args, msg);
  do_sys_log(this, level, msg, args);
  va_end(args);
  return ;
}}


COO_DEF(OE, void, p, const char * msg,...) {
  va_list args = { 0 };
  va_start(args, msg);
  do_sys_log(this, OSAL_LOGLEVEL_WARN, msg, args);
  va_end(args);
}}

COO_DEF(OE, RC, destroysemaphore, Cmaphore * s)
  Cmaphore_destroy(s);
return RC_OK;
}


COO_DEF(OE, char *, get_version)
char * version_str = this->getmem(512);
  osal_sprintf(version_str,"%s %s %s",PACKAGE_STRING, CODENAME, BUILD_TIME);
  return version_str;
}

COO_DEF(OE, void, print,const char * fmt,...)
va_list arg = { 0 };
va_start(arg, fmt);
vprintf(fmt, arg);
fflush(stdout);
va_end(arg);
}



COO_DEF(OE, ThreadID, get_thread_id);
  SimpleOE simpleOE = (SimpleOE)this->impl;
  uint i = 0;
  pthread_t __t = pthread_self();
  this->lock(simpleOE->lock);
  for(i = 0;i < simpleOE->threads->size();++i) {
    pthread_t * cur = (pthread_t *)simpleOE->threads->get(i);
    if (cur) {
      if (pthread_equal(*cur,__t)) {
        this->unlock(simpleOE->lock);
        return i;
      }
    } else {
      printf("NULL :(\n");
    }
  }
  this->unlock(simpleOE->lock);
  return 0; // the main thread
}


COO_DEF(OE, void *, getSystemLibrary, const char * name);
	this->syslog(OSAL_LOGLEVEL_WARN,"System libraries are not implemented yet.");
	return 0;
}

COO_DEF(OE, void, provideSystemLibrary, const char * name, DefaultConstructor f);

}

extern Memory LinuxMemoryNew();

OE OperatingEnvironment_New() {
  SimpleOE simpleOE = 0;
  OE oe = 0;
  Memory mem = (Memory)LinuxMemoryNew();

  if (_static_lock == 0) {
    _static_lock = Mutex_new(MUTEX_FREE);
  }

  simpleOE = mem->alloc(sizeof(*simpleOE));
  
  oe = (OE)mem->alloc(sizeof(*oe));
  if (!oe) return 0;
  zeromem(oe,sizeof(*oe));
  
  COO_setup(oe,1024*1024);

  oe->set_log_file = COO_attach( oe, OE_set_log_file);
  oe->get_version =  COO_attach( oe, OE_get_version);
  oe->destroysemaphore =  COO_attach( oe, OE_destroysemaphore);
  oe->yieldthread =  COO_attach( oe, OE_yieldthread);
  oe->accept =  COO_attach( oe, OE_accept);
  oe->getmem  =  COO_attach( oe, OE_getmem );
  oe->putmem  =  COO_attach( oe, OE_putmem );
  oe->read =  COO_attach( oe, OE_read);
  oe->write =  COO_attach( oe, OE_write);
  oe->open =  COO_attach( oe, OE_open);
  oe->close =  COO_attach( oe, OE_close);
  oe->newthread =  COO_attach( oe, OE_newthread);
  oe->jointhread =  COO_attach( oe, OE_jointhread);
  oe->newmutex = COO_attach( oe, OE_newmutex);
  oe->destroymutex =  COO_attach( oe, OE_destroymutex);
  oe->lock =  COO_attach( oe, OE_lock);
  oe->unlock =  COO_attach( oe, OE_unlock);
  oe->newsemaphore =  COO_attach( oe, OE_newsemaphore);
  oe->down =  COO_attach( oe, OE_down);
  oe->up =  COO_attach( oe, OE_up);
  oe->syslog =  COO_attach( oe, OE_syslog);
  oe->p =  COO_attach( oe, OE_p);
  oe->number_of_threads =  COO_attach( oe, OE_number_of_threads);
  oe->get_thread_id =  COO_attach( oe, OE_get_thread_id);
  oe->setloglevel =  COO_attach( oe, OE_setloglevel);
  oe->print =  COO_attach( oe, OE_print);
  oe->getSystemLibrary =  COO_attach( oe, OE_getSystemLibrary);
  oe->provideSystemLibrary =  COO_attach( oe, OE_provideSystemLibrary);
  oe->usleep = COO_attach(oe,OE_usleep);

  oe->impl = simpleOE;
  oe->newmutex(&(simpleOE->lock ));

  simpleOE->mm = mem;
  simpleOE->threads = IntKeyMap_New(oe);
  simpleOE->filedescriptors = SingleLinkedList_new(oe);
  simpleOE->loglevel = OSAL_LOGLEVEL_TRACE;
  simpleOE->log_file = 0;
  simpleOE->lock = Mutex_new(0);

  oe->lock(_static_lock);
  no_osal_instance += 1;
  oe->unlock(_static_lock);
  return oe;
}

void OperatingEnvironment_Destroy( OE * oe) {
  if (!oe) return;
  if (!*oe) return;
 
  if ((*oe)->impl) {
    SimpleOE soe = (SimpleOE)(*oe)->impl;
    Memory m = soe->mm;
    IntKeyMap_Destroy( &soe->threads );
    SingleLinkedList_destroy( &soe->filedescriptors );
    Mutex_destroy( soe->lock );
    m->free((*oe)->impl);
    m->free(*oe);

    free(m);
  }
  Mutex_lock(_static_lock);
  no_osal_instance -= 1;
  if (!no_osal_instance) {
  //  coo_end();
    Mutex_unlock(_static_lock);
    Mutex_destroy(_static_lock);
  } else {
    Mutex_unlock(_static_lock);
  }
  
}

Data Data_shallow(byte * d, uint ld) {
  static struct _data_ shallow = {0};
  shallow.data =d;
  shallow.ldata = ld;
  return &shallow;
}

bool Data_equal(Data a, Data b) {
  if (a == 0 && b == 0) return True;
  if (a == 0) return False;
  if (b == 0) return False;

  if (a->ldata != b->ldata) return False;

  return mcmp(a->data,b->data,a->ldata) == 0 ? True : False;

}

Data Data_new(OE oe, uint size) {
  Data res = 0;
  if (!oe) return 0;

  res = (Data)oe->getmem(sizeof(*res));
  if (!res) return 0;

  res->data = (byte*)oe->getmem(size);
  if (!res->data) goto failure;
  
  res->ldata = size;
  return res;
 failure:
  Data_destroy(oe,&res);
  return 0;
}

Data Data_copy(OE oe, Data other) {
  Data res = 0;
  uint i = 0;

  if (!oe) return 0;
  if (!other) return 0;
  if (!other->data) return 0;
  
  res = oe->getmem(sizeof(*res));
  if (!res) return 0;

  res->data = (byte*)oe->getmem(other->ldata);
  if (!res->data) goto failure;
  
  i=other->ldata;
  while(i--) res->data[i] = other->data[i];

  res->ldata = other->ldata;
  return res;
 failure:
  Data_destroy(oe, &res );
  return res;
  
}

void Data_destroy(OE oe, Data * d) {
  if (d) {
    if (*d) {
      if ((*d)->data) {
        oe->putmem((*d)->data);
      }
      oe->putmem((*d));
      *d = 0;
    }
  }
  
}
