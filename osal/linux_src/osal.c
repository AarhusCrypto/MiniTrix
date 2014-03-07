#include "osal.h"
#include "coo.h"
#include "list.h"
#include "singlelinkedlist.h"


#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <syscall.h>
#include <errno.h>
#include "config.h"

Cmaphore Cmaphore_new(OE oe, uint count);
void Cmaphore_up(Cmaphore c);
void Cmaphore_down(Cmaphore c);
void Cmaphore_destroy(Cmaphore * c);

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
  List threads;
  /*!
   * List of all open file descriptors
   */
  List filedescriptors;
  
  /*!
   * Lock that protects the state of this instance.
   */
  MUTEX lock;

} * SimpleOE;



COO_DCL( OE, void *, getmem, uint size)
COO_DEF_RET_ARGS(OE, void *, getmem, uint i;,i) {
  uint j = 0;
  byte * res = (byte*) malloc(i);
  for(j=0;j<i;++j) res[j] = 0;
  return res;
}}

COO_DCL( OE, void, putmem, void * p)
COO_DEF_NORET_ARGS(OE, putmem, void * p;,p) {
  if (p != 0) {
    free(p); 
  }
}}

COO_DCL(OE, RC, read, uint fd, byte * buf, uint * lbuf)
COO_DEF_RET_ARGS(OE, RC, read, uint fd;byte *buf;uint * lbuf;, fd, buf, lbuf ) {
  int r = 0;
  SimpleOE soe = (SimpleOE)this->impl;
  int os_fd = 0;

  if (!lbuf) return RC_FAIL;

  if (fd < 1) return RC_FAIL;

  if (soe->filedescriptors->size()+1 <= fd) {
    return RC_FAIL;
  }
  
  // get exclusive access to file descriptors
  this->lock(soe->lock);
  os_fd = (int)(unsigned long long)soe->filedescriptors->get_element(fd-1);
  
  
  while(1) {
    r = read(os_fd,buf, *lbuf); 
    if (r <= 0) {
      if (errno == EAGAIN) {
        *lbuf = 0;
        this->unlock(soe->lock);
        return RC_OK;
      } else {
        this->unlock(soe->lock);
        return RC_FAIL; // unexpected error or end of file
      }
    } else {
       printf("%d got it from read\n",r);
      *lbuf = r;
      break; // successful read
    }
  }
  this->unlock(soe->lock);
  return RC_OK;
}}

COO_DCL(OE, RC, write, uint fd, byte * buf, uint lbuf)
COO_DEF_RET_ARGS(OE, RC, write, uint fd; byte*buf;uint lbuf;,fd,buf,lbuf) {
  SimpleOE soe = (SimpleOE)this->impl;
  int writesofar = 0;
  int lastwrite = 0;
  int os_fd = (int)(long long)soe->filedescriptors->get_element(fd-1);

  if (fd >= soe->filedescriptors->size()+1) return RC_FAIL;

  while(writesofar < lbuf && lastwrite >= 0) {
    lastwrite = write(os_fd, buf+writesofar, lbuf-writesofar);
    if ( lastwrite == -1) {
      if (errno == EAGAIN) {
	lastwrite = 0;
	continue;
      } 
	
      return RC_FAIL;
    }
    writesofar += lastwrite;
  }
  
  
  if (lastwrite < 0) {
    this->p("ERROR Failed to write");
    return RC_OK;
  }

  return writesofar;

}}
extern char *strerror (int __errnum);
/* proposal:
 *
 * file <path>          - open a file
 * net <ip or dns name> - make a client connection
 * listen://[<ip>:]port   - open server socket
 *
 * Returns non-zero on success, zero on failure.
 */
COO_DCL(OE, int, open, const char * name)
COO_DEF_RET_ARGS(OE, int, open , const char * name;, name) {
  uint lname = 0;
  SimpleOE soe = 0;
  uint res = 0;

  if (!name) return -1;
  lname = osal_strlen(name);

  soe = (SimpleOE)this->impl;
  if (!soe) return -128;
  
  if (lname > 5 && (memcmp("file ", name, 5) == 0)) {
    int fd = open(name+5, O_RDWR);
    if (fd < 0) goto failure;
    
    this->lock(soe->lock);
    soe->filedescriptors->add_element( (void*)(long long)fd);
    res = soe->filedescriptors->size();
    this->unlock(soe->lock);
    return res;
  }
  
  // listen for incoming connections
  if (lname > 7 && memcmp("listen ",name,7) == 0)   {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    char port[6] = {0};
    int reuse_addr_option = 1;
    struct sockaddr_in serv_addr = {0};

    
    if (setsockopt(server_fd, SOL_SOCKET,
		   SO_REUSEADDR,
		   (char *)&reuse_addr_option, sizeof(reuse_addr_option)) < 0 ) {
      close(server_fd);
      return 0;
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
        return 0;
    }}

    {
      uint flags = fcntl(server_fd, F_GETFL, 0);
      fcntl(server_fd, F_SETFL, flags | O_NONBLOCK );
    }

    if (listen(server_fd, 20) != 0) {
      return 0;
    }
    this->lock(soe->lock); 
    soe->filedescriptors->add_element((void *)(long long)server_fd);
    res = soe->filedescriptors->size();
    this->unlock(soe->lock);
    return res;
  }


  // ip address
  if (lname > 3 && memcmp("ip ", name, 3) == 0) {
    int socket_fd = 0;
    char ip[20]={0},port[6]={0};
    struct sockaddr_in addr = {0};
    sscanf(name+3,"%s %s", ip, port);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) return 0;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(atoi(port));

    if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      close(socket_fd);
      return 0;
    }

    // set keep alive such that we detect failures and lost
    // connections faster.
    {
      int keep_alive_option = 1;
      int lkeep_alive_option = sizeof(keep_alive_option);
      if(setsockopt(socket_fd, 
		    SOL_SOCKET, 
		    SO_KEEPALIVE, 
		    &keep_alive_option, lkeep_alive_option) < 0) {
	close(socket_fd);
	return 0;
      }
    }

    // set non-blocking
    {
      int flags = fcntl( socket_fd, F_GETFL, &flags, sizeof(flags) );
      
      fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    }

    this->lock(soe->lock);
    soe->filedescriptors->add_element( (void*)(long long)socket_fd);
    res = soe->filedescriptors->size();
    this->unlock(soe->lock);
    return res;
  }

 failure:
  {
    char m[128] = {0};
    char * e = (char*)strerror(errno);
    sprintf(m,"Failed to create file descriptor, \"%s\" see error below:",name);
    this->syslog(OSAL_LOGLEVEL_FATAL, m);
    this->syslog(OSAL_LOGLEVEL_FATAL ,e);
  }
  if (soe) {
    this->unlock(soe->lock);
  }
  return 0;
}}

static void set_non_blocking(int fd) {
  int flags = fcntl(fd, F_GETFL, &flags, sizeof(flags));
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

COO_DCL(OE, int, accept, uint fd)
COO_DEF_RET_ARGS(OE, int, accept, uint fd;,fd) {
  SimpleOE soe = (SimpleOE)this->impl;
  int os_fd = 0;
  int os_client_fd = 0;
  uint res = 0;
  os_fd = (int)(long long) soe->filedescriptors->get_element( fd-1 );
  if (fd > soe->filedescriptors->size()+1) return 0;

  while(1) {
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



  this->lock(soe->lock);
  soe->filedescriptors->add_element( (void*)(long long)os_client_fd);
  res = soe->filedescriptors->size();
  this->unlock(soe->lock);

  return res;
}}

COO_DCL(OE, int, close, uint fd)
COO_DEF_RET_ARGS(OE, int, close, uint fd;, fd) {
  int fd = 0;
  SimpleOE soe = (SimpleOE)this->impl;
  
  if (!soe) { 
    return -1;
  }

  fd = (int)(long long)soe->filedescriptors->get_element(fd);
  soe->filedescriptors->rem_element(fd);

  return close(fd);
}}

COO_DCL(OE, ThreadID, newthread, ThreadFunction tf, void * args)
COO_DEF_RET_ARGS(OE, ThreadID, newthread, ThreadFunction tf; void * args;, tf, args) {

  pthread_t * t = this->getmem(sizeof(*t));
  SimpleOE soe = (SimpleOE)this->impl;
  ThreadID tid = 0;

  if (pthread_create(t, 0, tf, args) == 0) {
    this->lock(soe->lock);
    soe->threads->add_element(t);
    tid = soe->threads->size();
    this->unlock(soe->lock);
    return tid;
  }
  this->syslog(OSAL_LOGLEVEL_WARN, "newthread: failed to create thread");
  return 0;
}}

COO_DCL(OE, void, yieldthread)
COO_DEF_NORET_NOARGS(OE, yieldthread) {
  sched_yield();
  //  this->syslog(OSAL_LOGLEVEL_FATAL,"Yield thread is *NOT* implemented");
}}

COO_DCL(OE, void *, jointhread, ThreadID tid)
COO_DEF_RET_ARGS(OE, void *, jointhread, ThreadID tid;,tid) {
  
  SimpleOE soe = (SimpleOE)this->impl;
  pthread_t * t = (pthread_t *)soe->threads->get_element(tid-1);
  void * p = 0;
  if (tid > soe->threads->size()+1) { 
    this->syslog(OSAL_LOGLEVEL_FATAL, "Failed to join thread"
		 ", thread id is greater than any active thread id.");
    return;
  }
  if (t) {
    pthread_join(*t,&p);
  } else {
    this->p("Auch ! Thread not found");
  }
  
  return p;
}}

COO_DCL(OE, MUTEX, newmutex )
COO_DEF_RET_NOARGS(OE, MUTEX, newmutex) {
  return Mutex_new(MUTEX_FREE);
}}

COO_DCL(OE, void, destroymutex, MUTEX * m)
COO_DEF_NORET_ARGS(OE, destroymutex, MUTEX * m;,m) {
  Mutex_destroy(*m);
}}

COO_DCL(OE, void, lock, MUTEX m)
COO_DEF_NORET_ARGS(OE, lock,  MUTEX m;,m) {
  //pid_t tid;
  //  char msg[64] = {0};
  //tid = syscall(SYS_gettid);
  //  sprintf(msg,"LOCK(%p) taken by THREAD(%d)",m,tid);
  //this->p(msg);
  Mutex_lock(m);
}}

COO_DCL(OE, void, unlock,  MUTEX m) 
COO_DEF_NORET_ARGS(OE, unlock, MUTEX m;,m) {
  //pid_t tid;
  //char msg[64] = {0};
  //tid = syscall(SYS_gettid);
  //sprintf(msg,"LOCK(%p) released by THREAD(%d)",m,tid);
  //this->p(msg);
  Mutex_unlock(m);  
}}

COO_DCL(OE, Cmaphore, newsemaphore, uint count)
COO_DEF_RET_ARGS(OE, Cmaphore, newsemaphore, uint count;,count) {
  return Cmaphore_new(this,count);
}}

COO_DCL(OE, void, up, Cmaphore c)
COO_DEF_NORET_ARGS(OE, up, Cmaphore c;,c) {
  Cmaphore_up(c);
}}

COO_DCL(OE, void, down, Cmaphore c)
COO_DEF_NORET_ARGS(OE, down, Cmaphore c;, c) {
  Cmaphore_down(c);
}}

COO_DCL(OE, void, syslog, LogLevel level, const char * msg) 
COO_DEF_NORET_ARGS(OE, syslog, LogLevel level; const char * msg;, level, msg) {
  printf(" - log - %s\n", msg);
}}

COO_DCL(OE, void, p, const char * msg)
COO_DEF_NORET_ARGS(OE, p, const char * msg;,msg) {
  return this->syslog(OSAL_LOGLEVEL_WARN, msg);
}}

COO_DCL(OE, void, destroysemaphore, Cmaphore * s)
COO_DEF_NORET_ARGS(OE, destroysemaphore, Cmaphore * s;,s) {
  Cmaphore_destroy(s);
}}


OE OperatingEnvironment_LinuxNew() {
  SimpleOE simpleOE = 0;
  OE oe = 0;
  Memory m = LinuxMemoryNew();
  Memory special_mem = LinuxSpecialMemoryNew(m);
  coo_init(special_mem);

  simpleOE = m->alloc(sizeof(*simpleOE));
  
  oe = (OE)m->alloc(sizeof(*oe));
  if (!oe) return 0;
  zeromem(oe,sizeof(*oe));
  
  COO_ATTACH(OE, oe, destroysemaphore);
  COO_ATTACH(OE, oe, yieldthread);
  COO_ATTACH(OE, oe, accept);
  COO_ATTACH(OE, oe, getmem );
  COO_ATTACH(OE, oe, putmem );
  COO_ATTACH(OE, oe, read);
  COO_ATTACH(OE, oe, write);
  COO_ATTACH(OE, oe, open);
  COO_ATTACH(OE, oe, close);
  COO_ATTACH(OE, oe, newthread);
  COO_ATTACH(OE, oe, jointhread);
  COO_ATTACH(OE, oe, newmutex);
  COO_ATTACH(OE, oe, destroymutex);
  COO_ATTACH(OE, oe, lock);
  COO_ATTACH(OE, oe, unlock);
  COO_ATTACH(OE, oe, newsemaphore);
  COO_ATTACH(OE, oe, down);
  COO_ATTACH(OE, oe, up);
  COO_ATTACH(OE, oe, syslog);
  COO_ATTACH(OE, oe, p);

  oe->impl = simpleOE;
  simpleOE->lock = oe->newmutex();
  simpleOE->mm = m;
  simpleOE->sm = special_mem;
  simpleOE->threads = SingleLinkedList_new(oe);
  simpleOE->filedescriptors = SingleLinkedList_new(oe);

  oe->p("************************************************************");
  oe->p("   "PACKAGE_STRING" - "CODENAME );
  oe->p("************************************************************");
  {
    char m[64] = {0};
    osal_sprintf(m,"STUB SIZE %u",STUB_SIZE);
    oe->p(m);
  }
  return oe;
}

void OperatingEnvironment_LinuxDestroy( OE * oe) {
  if (!oe) return;
  if (!*oe) return;
 
  if ((*oe)->impl) {
    SimpleOE soe = (SimpleOE)(*oe)->impl;
    Memory m = soe->mm;
    SingleLinkedList_destroy( &soe->threads );
    SingleLinkedList_destroy( &soe->filedescriptors );
    Mutex_destroy( soe->lock );
    m->free((*oe)->impl);
    m->free(*oe);

    free(m);
  }
  coo_end();
}

Data Data_shallow(byte * d, uint ld) {
  static struct _data_ shallow = {0};
  shallow.data =d;
  shallow.ldata = ld;
  return &shallow;
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

Data Data_destroy(OE oe, Data * d) {
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
