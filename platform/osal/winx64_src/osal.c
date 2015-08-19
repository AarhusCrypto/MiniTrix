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
#include "list.h"
#include "singlelinkedlist.h"
#include <common.h>
#include <config.h>
#include <memory.h>

// libc (stdandard libc functions)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>

#ifndef WINDOWS
#error This is OSAL for Windows x64 and WINDOWS in not defined
#endif

// Windows x64 System
#include <Windows.h>
#include <socketapi.h>
#pragma comment(lib, "WS2_32.lib")
#define NOT_YET_MSG "TODO(rwz): win x64 does not support this yet."

// Library names for names which can be expected of the standard platform.
const char * DATE_TIME_LIBRARY = "OSAL_DATE_TIME_LIBRARY";
const char * TERMINAL_LIBRARY = "OSAL_TERMINAL_LIBRARY";
const char * NETWORK_LIBRARY = "OSAL_NETWORK_LIBRARY";
const char * GRAPHICS_LIBRARY = "OSAL_GRAPHICS_LIBRARY";
extern char *strerror (int __errnum);

static DefaultConstructor dateTimeDefaultConstructor = 0;

// static structure boot-strapping the osal system
static MUTEX _static_lock;
static uint no_osal_instance;

// informative return value
typedef struct _osal_ret_val_ {
  uint err;
  int rval;
} retval;

void usleep(long long usec) {
	HANDLE timer = 0;
	LARGE_INTEGER ft = { 0 };
	ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time
	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

// Handle Windows API ReadFile
static
retval __read(HANDLE hFile, byte * buf, uint lbuf) {
  uint bytes_read = 0;
  retval res = {0};
  Mutex_lock(_static_lock);
 
  ReadFile(hFile, buf, lbuf, &bytes_read, NULL);
  res.rval = bytes_read;
  res.err = GetLastError();
  SetLastError(0);

  Mutex_unlock(_static_lock);
  return res;
}

// Handle Windows API WriteFile
static
retval __write(HANDLE hFile, byte * buf, uint lbuf) {
	uint bytes_written = 0;
  retval res = {0};
  Mutex_lock(_static_lock);

  // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365747%28v=vs.85%29.aspx	
  WriteFile(hFile, buf, lbuf,&bytes_written, NULL);
  res.err = GetLastError(); SetLastError(0);
  res.rval = bytes_written;

  Mutex_unlock(_static_lock);
  return res;
}


// ------------------------------------------------------------
Cmaphore Cmaphore_new(OE oe, uint count) {
  Cmaphore res = (Cmaphore)oe->getmem(sizeof(*res));
  RC rc = RC_OK;
  if (!res) return 0;
  
  rc = oe->newmutex(&(res->lock));
  if (rc != RC_OK) return 0;

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


// ----------------------------------------
// OE implementation defails
// ----------------------------------------
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
  IntKeyMap filedescriptors;
  
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

  uint fdpool;
} * SimpleOE;


static HANDLE FDEntry_remove(OE oe, IntKeyMap entries, uint fd) {
  if (!entries) return 0;
  if (!entries->contains((ull)fd)) return 0;
  return entries->rem((ull)fd);
}

static HANDLE FDEntry_lookup(IntKeyMap entries, uint fd) {
  if (!entries) return 0;
  if (!entries->contains((ull)fd)) return 0;
  return entries->get((ull)fd); 
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

COO_DEF(OE, RC, read, uint fd, byte * buf, uint * lbuf)
  SimpleOE soe = (SimpleOE)this->impl;

  if (!lbuf) return RC_BAD_ARGS;

  if (!buf) return RC_BAD_ARGS;

  if (fd > 0x80000000) {
	  SOCKET os_fd = 0;
	  int rc = 0, count = 0;

	// lookup System specific Handle
	  this->lock(soe->lock);
	  os_fd = (SOCKET)soe->filedescriptors->get(fd);
	  this->unlock(soe->lock);
	  if (os_fd == 0) return RC_FAIL;
  
	  do {
		  rc = recv(os_fd, buf, *lbuf, 0);
		  count++;
		  this->usleep(25);
	  } while (count < 5 && rc == -1);


	  if (rc > 0) {
		  // data received
		  *lbuf = rc;
		  return RC_OK;
	  }
	  else if (rc == 0) {
		  // Connection close by the other end
		  *lbuf = 0;
		  return RC_DISCONN;
	  }
	  else {
		  int err = GetLastError();
		  *lbuf = 0;

		  if (err == WSAEWOULDBLOCK)
			  return RC_OK;

		  // Negative error
		  return RC_FAIL;
	  }
  }
  else {
	  // We have a HANDLE not a SOCKET
	  retval ret = { 0 };
	  HANDLE os_fd = 0;

	  // lookup System specific handle
	  this->lock(soe->lock);
	  os_fd = FDEntry_lookup(soe->filedescriptors, fd);
	  this->unlock(soe->lock);
	  if (os_fd == 0) return RC_FAIL;

	  ret = __read(os_fd, buf, *lbuf);
	  *lbuf = ret.rval;
	  return (ret.err == 0) ? RC_OK : RC_FAIL;
  }
}

COO_DEF(OE, RC, write, uint fd, byte * buf, uint * lbuf)
  SimpleOE soe = (SimpleOE)this->impl;
  retval ret = { 0 };

  if (!buf) return RC_BAD_ARGS;

  if (!lbuf) return RC_BAD_ARGS;


  if (fd > 0x80000000) {
	  SOCKET os_fd = 0;
	  int sentbytes = 0;
	  int err = 0;
	  // lookup System specific handle
	  this->lock(_static_lock);
	  os_fd = (SOCKET)soe->filedescriptors->get(fd);
	  this->unlock(_static_lock);
	 
	  // Win API send 
	  do {
		  this->lock(_static_lock);
		  sentbytes = send(os_fd, buf, *lbuf, 0);
		  err = GetLastError();
		  this->unlock(_static_lock);
	  } while (err == WSAEWOULDBLOCK);


	  if (sentbytes == SOCKET_ERROR) {
		  err = GetLastError();
		  if (err == 10057)
			  this->print("[OSal] Sending failed because socket is disconnected.");
		  else
			  this->print("Error: %d", err);
		  *lbuf = 0;
		  return RC_FAIL;
	  }
	  *lbuf = sentbytes;
	  return RC_OK;
  }
  else {
	  HANDLE os_fd = { 0 };
	  // lookup handle
	  this->lock(_static_lock);
	  os_fd = soe->filedescriptors->get(fd);
	  this->unlock(_static_lock);

	  ret = __write(os_fd, buf, *lbuf);
	  *lbuf = ret.rval;
	  return ret.err == 0 ? RC_FAIL : RC_OK;
  }
}

/* proposal:
 *
 * file <path>          - open a file
 * net <ip or dns name> - make a client connection
 * listen://[<ip>:]port   - open server socket
 *
 * Returns non-zero on success, zero on failure.
 *
 *
 * 
 */

COO_DEF(OE, RC, open, const char * name, FD * fdout)
	uint lname = 0;
	SimpleOE soe = 0;

	if (!name) return RC_BAD_ARGS;
	lname = osal_strlen(name);

	soe = (SimpleOE)this->impl;
	if (!soe) return RC_IMPL_NOTSET;
  
	if (!fdout) {
		this->p("[Open] Called with null {fdout}");
		return RC_FAIL;
	}

	if (soe->filedescriptors->size() > 0x0000F000) {
		this->p("[Open] Too many file descriptors open");
		return RC_FAIL;
	}

	if (lname > 5 && (mcmp((void*)"file ",(void*)name, 5) == 0)) {  
		HANDLE hFile = 0;
		DWORD error = 0;
		char * filename = (char*)name + 5;
		this->lock(_static_lock); // protect Last Error
		hFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
		error = GetLastError();
		SetLastError(0);
		this->unlock(_static_lock);
	
		if (hFile == INVALID_HANDLE_VALUE) {
			// maybe the file already existed open for read write always instead.
			hFile = 0;
			this->lock(_static_lock);
			hFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			error = GetLastError();
			SetLastError(0);
			this->unlock(_static_lock);
			if (hFile == INVALID_HANDLE_VALUE) {
				this->p("[Open] WIN API Last error: %d\n",error); 
				return RC_FAIL;
			} // else fall through we have a valid handle
		} // else fall through we have a valid handle
		
 	    this->lock(soe->lock);
		do {
			*fdout = soe->fdpool;
			soe->fdpool += 1;
		} while (soe->filedescriptors->contains(*fdout));

		soe->filedescriptors->put((ull)*fdout, (void*)hFile);
		this->unlock(soe->lock);
		return RC_OK;
  }

 
  // listen for incoming connections
  if (lname > 7 && mcmp(( void*)"listen ",( void *)name,7) == 0)   {
	SOCKET server_fd = 0;
    char port[6] = {0};
    int reuse_addr_option = 1;
    struct sockaddr_in serv_addr = {0};

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == INVALID_SOCKET) {
		int error = GetLastError();
		this->p("[Open] Socket failed WIN API Error: %d\n", error);
		return RC_FAIL;
	}

    if (setsockopt(server_fd, SOL_SOCKET,
		   SO_REUSEADDR,
		   (char *)&reuse_addr_option, sizeof(reuse_addr_option)) < 0 ) {
      closesocket(server_fd);
	  this->p("[Open] setsockopt fail WIN API Error: %d",GetLastError());
      return RC_FAIL;
    }

	
    sscanf_s(name+7,"%s", port, 6);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(port)); 
    {
      // socklen_t lserv_addr = sizeof(serv_addr);
      if ( bind ( server_fd, (struct sockaddr *)&serv_addr,	
		   sizeof(serv_addr)) < 0) {
        closesocket(server_fd);
		this->p("[Open] bind fail WIN API Error: %d", GetLastError());
		return RC_FAIL;
	  }
    }

    if (listen(server_fd, 20) != 0) {
		this->p("[Open] listen fail WIN API Error: %d", GetLastError());
		return RC_FAIL;
    }

		{
			ull v = 1;
			ioctlsocket(server_fd, FIONBIO, &v);
		}

	this->lock(soe->lock);
	do {
		*fdout = soe->fdpool;
		soe->fdpool += 1;
	} while (soe->filedescriptors->contains(*fdout));
	*fdout |= 0x80000000;
	soe->filedescriptors->put(*fdout, (void*)server_fd);
	this->unlock(soe->lock);
	return RC_OK;
  }


  // ip address
  if (lname > 3 && mcmp((void *) "ip ", (void*)name, 3) == 0) {
    SOCKET socket_fd = 0;
    char ip[80]={0},port[32]={0};
    struct sockaddr_in addr = {0};
	sscanf_s(name + 3, "%s", ip,sizeof(ip));
	sscanf_s(name + 3 + osal_strlen(ip), "%s", port,sizeof(port));
	int iResult = 0;

	socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd == INVALID_SOCKET){
		this->p("[Open} ip: create socket failed WIN API Error: %d", GetLastError());
		return RC_FAIL;
	}

    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(atoi(port));

    if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		this->p("[Open} ip: create connect failed WIN API Error: %d", GetLastError());
		closesocket(socket_fd);
		return RC_FAIL;
    }

	{
		ull v = 1;
		ioctlsocket(socket_fd, FIONBIO, &v);
	}

	this->lock(soe->lock);
	*fdout = soe->fdpool;
	soe->fdpool += 1;
	*fdout = *fdout | 0x80000000;
	soe->filedescriptors->put((ull)(*fdout), (void*)socket_fd);
	this->unlock(soe->lock);
	return RC_OK;
  }
  return 0;
}



COO_DEF(OE, uint, accept, FD fd, FD * client_fd_out)
  SimpleOE soe = (SimpleOE)this->impl;
  SOCKET os_fd = 0;
  SOCKET os_client_fd = { 0 };

if (fd < 0x80000000) {
	this->p("[Accept] Non network socket descriptor given.");
	return RC_BAD_ARGS;
}

this->lock(_static_lock);
os_fd = (SOCKET)(ull)soe->filedescriptors->get((void*)(ull)fd);
this->unlock(_static_lock);


while (1)  {

	fd_set rd = { 0 };
	struct timeval timeout = { 0 };
	timeout.tv_usec = 100;
	select(os_fd + 1, &rd, 0, 0, &timeout);
	os_client_fd = accept(os_fd, 0, 0);
	if (os_client_fd == SOCKET_ERROR) {
		int err = GetLastError();
		if (err == WSAEWOULDBLOCK) {
			this->yieldthread(); continue;
		}
		else {
			this->p("[Accept] accept failed Win API Error: %d osfd:%lu fd:%lu", err, os_fd, fd);
			return RC_FAIL; // report failure
		}
	}
	else {
		break; // we have a client
	}
}
// RX ERR 4060083


this->lock(soe->lock);
{
	uint newfd = soe->fdpool;
	newfd |= 0x80000000;
	soe->fdpool += 1;
	soe->filedescriptors->put((void*)(ull)newfd, (void*)os_client_fd);
	*client_fd_out = newfd;
}
this->unlock(soe->lock);

return RC_OK;
}


COO_DEF(OE, RC, close, uint _fd)
  HANDLE fd = 0;
  SimpleOE soe = (SimpleOE)this->impl;
  
  if (!soe) { 
	  return RC_IMPL_NOTSET;
  }

  this->lock(soe->lock);

  fd = FDEntry_remove(this, soe->filedescriptors, _fd);
  if (fd) {
    CloseHandle(fd);
  }

  this->unlock(soe->lock);
  
  return RC_OK;
}

COO_DEF(OE, RC, newthread, ThreadID * id, ThreadFunction tf, void * args)
SimpleOE soe = (SimpleOE)this->impl;
static ThreadID idpool = 1;
ThreadID tid = 0;
// ---- critical ----
this->lock(_static_lock);
tid = idpool++;
HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)tf, args, 0, 0);
if (!hThread) return RC_FAIL;
soe->threads->put(tid, hThread);
this->unlock(_static_lock);
*id = tid;
// ----- done -----
return RC_OK;
}

COO_DEF(OE, void, yieldthread)
	usleep(0);
}

COO_DEF(OE, RC, jointhread, ThreadID tid)
SimpleOE soe = (SimpleOE)this->impl;
HANDLE hThread = soe->threads->get((ull)tid);
if (hThread != 0) {
	WaitForSingleObject(hThread, INFINITE);
}
this->lock(_static_lock);
soe->threads->rem(tid);
this->unlock(_static_lock);
return RC_OK;
}

COO_DEF(OE, uint, number_of_threads);
  SimpleOE soe = (SimpleOE)this->impl;
  uint answer = 0;
  this->lock(soe->lock);
  answer = soe->threads->size()+1; // +1 is the main thread
  this->unlock(soe->lock);
  return answer;
}

COO_DEF(OE, RC, newmutex, MUTEX * mout)
if (!mout) return RC_BAD_ARGS;

*mout = Mutex_new(MUTEX_FREE);

return RC_OK;
}

COO_DEF(OE, RC, destroymutex, MUTEX * m)
  Mutex_destroy(*m);
return RC_OK;
}

COO_DEF(OE, void, lock, MUTEX m)
//pid_t tid;
//  char msg[64] = {0};
//tid = syscall(SYS_gettid);
//  sprintf(msg,"LOCK(%p) taken by THREAD(%d)",m,tid);
//this->p(msg);
if (!m) return RC_BAD_ARGS;
  Mutex_lock(m);
return RC_OK;
}

COO_DEF(OE, void, unlock, MUTEX m)
//pid_t tid;
//char msg[64] = {0};
//tid = syscall(SYS_gettid);
//sprintf(msg,"LOCK(%p) released by THREAD(%d)",m,tid);
//this->p(msg);
if (!m) return RC_BAD_ARGS;
  Mutex_unlock(m);  
  return RC_OK;
}

COO_DEF(OE, RC, newsemaphore, Cmaphore * cout, uint count)
if (!cout) return RC_BAD_ARGS;
*cout = Cmaphore_new(this, count);
return RC_OK;
}

COO_DEF(OE, RC, up, Cmaphore c)
if (!c) return RC_BAD_ARGS;
  Cmaphore_up(c);
  return RC_OK;
}

COO_DEF(OE, void, down, Cmaphore c)
if (!c) return RC_BAD_ARGS;
  Cmaphore_down(c);
  return RC_OK;
}

COO_DEF(OE, void, setloglevel, LogLevel level);
  SimpleOE soe = (SimpleOE)this->impl;
  soe->loglevel = level;
}

COO_DEF(OE, void, set_log_file, char * filename);
  SimpleOE soe = (SimpleOE)this->impl;
  char fname[512] = {0};
  FD fd = 0;
  RC rc = RC_OK;
  uint lfilename = osal_strlen(filename);
  osal_sprintf(fname,"file %s", filename);
  rc = this->open(fname,&fd);
  if (rc != RC_OK) {
    soe->log_file = fd;
  }
}

static uint arglen(char c) {
	switch (c) {
	case 'd': 
	case 'x':
	case 'i':
	case 'u':
	case 'o':
	case 'X':
	case 'p':
		return 11;
	case 'f':
	case 'F':
	case 'g':
	case 'G':
		return 12;
	case 'e':
	case 'E':
		return 14;
	case 'a':
	case 'A':
		return 16;
	case 'c':
		return 1;
	case '%':
		return 1;

	}
}

static void do_sys_log(OE this,LogLevel level, const char * msg, va_list args) {
	SimpleOE soe = (SimpleOE)this->impl;
	byte * buf = 0;
	uint lbuf = 128;
	uint o = 0;
	uint l = 0;

	buf = this->getmem(lbuf);

	switch (level) {
	case OSAL_LOGLEVEL_TRACE: {
		o = sprintf_s(buf, lbuf, "- Trace log - ");
	} break;
	case OSAL_LOGLEVEL_DEBUG: {
		uint o = 0;
		o = sprintf_s(buf, lbuf, "- Debug log - ");
	} break;
	case OSAL_LOGLEVEL_WARN: {
		o = sprintf_s(buf, lbuf, "- Warn log  - ");
	} break;
	case OSAL_LOGLEVEL_FATAL: {
		o = printf(buf, lbuf, "- Fatal log- ");
	} break;

	default: {
		o = sprintf_s(buf, lbuf, " - log - ");
	} break;
	}

	l += o;
	o = vsprintf_s(buf + o, lbuf - o, msg, args);
	l += o;
	o = sprintf_s(buf + l, lbuf - l, "\n");
	l += o;

	if (soe->log_file != 0) {
	  uint w = l+1;
	  this->write(soe->log_file, (byte*)buf, &w);
	  this->putmem(buf);
	  return;
	}


	printf(buf);
	this->putmem(buf);

}

COO_DEF(OE, void, syslog, LogLevel level, const char * msg,...) 
  SimpleOE soe = (SimpleOE)this->impl;
  va_list args = { 0 };
  if (level < soe->loglevel) { return RC_OK; };

  if (!msg) {
	  return RC_FAIL;
  }
  va_start(args, msg);
  do_sys_log(this, level, msg, args);
  va_end(args);
  return ;
}

COO_DEF(OE, void, p, const char * msg,...)
SimpleOE soe = (SimpleOE)this->impl;
va_list args = { 0 };
if (!msg) {
	return RC_BAD_ARGS;
}

va_start(args, msg);
do_sys_log(this, OSAL_LOGLEVEL_WARN, msg, args);
va_end(args);

}

COO_DEF(OE, void, destroysemaphore, Cmaphore * s)
  Cmaphore_destroy(s);
}


COO_DEF(OE, char *, get_version)
  static char version_str[256] = {0};
  osal_sprintf(version_str,"OSAL %s %s %s",PACKAGE_STRING, CODENAME, BUILD_TIME);
  return version_str;
}

COO_DEF(OE, void, print, const char * fmt, ...)
va_list arg = { 0 };
va_start(arg, fmt);
vprintf(fmt, arg);
va_end(arg);
}


COO_DEF(OE, ThreadID, get_thread_id) {
  SimpleOE simpleOE = (SimpleOE)this->impl;
  uint id = GetCurrentThreadId();
  uint i = 0;
  
  this->lock(simpleOE->lock);
  for(i = 0;i < simpleOE->threads->size();++i) {
    uint cur = (uint)(ull)simpleOE->threads->get(i);
    if (id == cur) {
      this->unlock(simpleOE->lock);
      return i;
    }
  }

  this->unlock(simpleOE->lock);
  return 0;
}}


static int strcmp(const char * s1, const char * s2) {
	uint ls1 = 0, ls2 = 0;
	uint i = 0;

	while (s1[i] == s2[i] && s1[i] != 0) {
		++i;
	}

	if (s1[i] == s2[i]) return 1;
	return 0;
}

COO_DEF(OE, void *, getSystemLibrary, const char * name);

if (strcmp(name, DATE_TIME_LIBRARY) == 1 
    && 
    dateTimeDefaultConstructor) 
  return dateTimeDefaultConstructor(this);
return 0;
}

COO_DEF(OE, void, provideSystemLibrary, const char * name, DefaultConstructor f);
	
if (strcmp(name, DATE_TIME_LIBRARY) == 1) {
	dateTimeDefaultConstructor = f;
}
}

static void * libc_malloc_wrap(uint size) {
	return malloc(size);
}

static Memory LibC_Memory() {
	Memory res = (Memory)malloc(sizeof(*res));
	res->alloc = libc_malloc_wrap;
	res->free = free;
	return res;
}


COO_DEF(OE, void, usleep, ull usec)
usleep(usec);
}

OE OperatingEnvironment_New() {
  SimpleOE simpleOE = 0;
  OE oe = 0;
  Memory mem = LibC_Memory();

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
  oe->usleep = COO_attach(oe, OE_usleep);

  if (!oe->newmutex) {
	  return 0;
  }

  oe->impl = simpleOE;
  oe->newmutex(&simpleOE->lock);

  simpleOE->fdpool = 0x00000010;
  simpleOE->mm = mem;
  simpleOE->threads = IntKeyMap_New(oe);
  simpleOE->filedescriptors = IntKeyMap_New(oe);
  simpleOE->loglevel = OSAL_LOGLEVEL_TRACE;
  simpleOE->log_file = 0;
  // Initialize networking
  {
	  WSADATA wsaData = { 0 };
	  int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	  if (iResult != 0) {
		  printf("getaddrinfo failed: %d\n", iResult);
		  WSACleanup();
		  return 0;
	  }
  }


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
    IntKeyMap_Destroy( &soe->filedescriptors );
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
