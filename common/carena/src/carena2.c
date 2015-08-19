/*
  Copyright (c) 2013, Rasmus Lauritsen, Aarhus University
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
  This product includes software developed by the Aarhus University.
  4. Neither the name of the Aarhus University nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY Rasmus Lauritsen at Aarhus University ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL Rasmus Lauritsen at Aarhus University BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


  Author: Rasmus Winther Lauritsen, rwl@cs.au.dk

  Changes: 
  2014-02-28  Initial version created
*/
#include "carena.h"
#include <list.h>
#include <coo.h>
#include <blockingqueue.h>
#include <errno.h>
#include <singlelinkedlist.h>
#include <common.h>
#include <encoding/hex.h>
#include <encoding/int.h>
#include <stats.h>
#include <unistd.h>
#include <time.h>
#include <encoding/int.h>


#include <errno.h>

static
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
}

int _c_;



typedef struct _carena_impl_ {

  uint server_fd;
  
  OE oe;

  List peers;

  uint peer_ids;

  ThreadID worker;

  bool running;

  MUTEX lock;

  List conn_obs;

  MUTEX listen_ready;

} * CArenaImpl;

typedef struct _mpcpeer_impl_ {

  /* Operating environment available to this peer.
   *
   */
  OE oe;

  char ip[16];

  uint port;

  // file descriptor
  uint fd_in, fd_out;

  // Thread id of the receiver
  ThreadID receiver;
  
  // Therad id of the sender
  ThreadID sender;

  // Incoming ITC
  BlkQueue incoming;

  // Outgoing ITC (inter thread comm)
  BlkQueue outgoing;

  /*
   * Keep running flag 
   */
  bool running;

  /*
   * Shutdown flag
   */
  bool die;

  /*
   * Buffer for in between data, when receiving more data than asked
   * for.
   */
  Data drem;

  /*
   * Initially locked by the arena until the peer is completely
   * initialised.
   */
  MUTEX receive_lock;
  MUTEX send_lock;

  Data die_package;
} * MpcPeerImpl;




COO_DCL(MpcPeer, CAR, send, Data data)
COO_DEF_RET_ARGS(MpcPeer, CAR, send, Data data;, data) {
  CAR c = {{0}};
  MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
  RC rc = 0;

  if (peer_i->die) return;

  
  peer_i->oe->write(peer_i->fd_out, data->data,data->ldata);

  return c;
}}

static
void local_read(OE oe, int fd, byte * buf, uint lbuf) {
  uint sofar = 0;
  RC rc = 0;
  
  while(sofar < lbuf) {
    uint read_last = lbuf - sofar;
    rc = oe->read(fd, buf+sofar,&read_last);
    if (rc != RC_OK) { 
      oe->p("Read failure");
      return;
    }
    if (read_last == 0) printf("Reading\n");
    if (read_last > 0) sofar += read_last;
  }
}

COO_DCL(MpcPeer, CAR, receive, Data data);
COO_DEF_RET_ARGS(MpcPeer, CAR, receive, Data data;, data) {
    MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
    OE oe = peer_i->oe;
    byte len[4] = {0};
    uint to_read = 0;
    local_read(oe,peer_i->fd_in, data->data, data->ldata);
}}


COO_DCL(MpcPeer, char *, get_ip)
COO_DEF_RET_NOARGS(MpcPeer, char *, get_ip) {
  MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
  return peer_i->ip;
}}

COO_DCL(MpcPeer, uint, get_port)
COO_DEF_RET_NOARGS(MpcPeer, uint, get_port) {
  MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
  return peer_i->port;
}}

static void * peer_rec_function(void * a) {
  MpcPeer me = (MpcPeer)a;
  MpcPeerImpl mei = 0;
  Data buf = 0; 
  OE oe = 0;
  int l = 0;

  if (!me) goto out;

  mei = (MpcPeerImpl)me->impl;
  oe = mei->oe;

  oe->lock(mei->receive_lock);
  
  while(mei->running) {
    uint sofar = 0;
    byte len[4] = {0};
    uint lenlen = 4;
    RC rc = 0 ; 
    char _o[64] = {0};
    ull start = 0;
    ull waste = 0;

    //    if (_c_) waste = _nano_time();
    rc = mei->oe->read(mei->fd_in,len,&lenlen);
    
    if (mei->die) goto out;

    //    if (_c_) printf("Time returning read %llu\n",_nano_time()-waste);
    if (lenlen <= 3) { 
      continue;
    }

    if (rc != RC_OK) {
      oe->unlock(mei->receive_lock);
      return 0;
    }

    l = b2i(len);

    //    osal_sprintf(_o,"expecting %u bytes to come",l);
    //    oe->p(_o);

    if (l > 1024*1024*10) {
      char m[32] = {0};
      osal_sprintf(m,"More than 10 MB %u\n",l);
      oe->p(m);
    }
    
    buf = (Data)Data_new(mei->oe,l);  
    if (!buf) {
      char mmm[64] = {0};
      osal_sprintf(mmm,"Allocation of size %u failed",l);
      mei->oe->p(mmm);
      continue;
    }

    sofar = 0;
    CHECK_POINT_S("Read data");
    waste = 0;
    while(1) {
      uint read_bytes = 0;
      ull tmp = 0;

      // read_bytes is how many bytes to be read in this round
      // before {read} and the number of bytes read after.
      read_bytes = l - sofar;

      // if we need to read zero bytes in this round we'r done
      // add the buffer to the queue.
      if(read_bytes == 0) {
        CHECK_POINT_E("Read data");
        //        printf("Adding data to incoming\n");
        mei->incoming->put(buf);
        break;
      }

      // read_bytes will never be more than {buf->ldata} as we would
      // never ask for more. !! Invariant !!

      rc = mei->oe->read(mei->fd_in, buf->data+sofar, &read_bytes);
      //      printf("Die 2\n");
      if (mei->die) goto out; // are we shutting down, then leave      
      
      if (rc == RC_DISCONN) {
        break;
      }

      if (rc == RC_OK) {

        // if we are shutting down better quit now !

        // okay was anything read?
        if (read_bytes > 0) { // yes okay update sofar !
          sofar += read_bytes;
          //    printf("read %u bytes sofar %u :)\n",read_bytes,sofar);
        }

      } else { // rc != RC_OK failure connection probably lost
        mei->oe->syslog(OSAL_LOGLEVEL_FATAL, 
                        "MpcPeer peer_rec_function failure to read."	\
                        " Peer will not receive anymore data.");
        oe->unlock(mei->receive_lock);
        return 0;
      }
    } // while(1)
  }
  
  
 out:
  oe->unlock(mei->receive_lock);
  //  printf("Mpc Peer leaving receiver thread. fd_in=%u\n",mei->fd_in);
  return 0;
}



static void * peer_snd_function(void * a) {
  MpcPeer me = (MpcPeer)a;

  MpcPeerImpl mei = (MpcPeerImpl)me->impl;
  
  byte len[4] = {0};

  //  printf("Waiting for SEND LOCK\n");
  mei->oe->lock(mei->send_lock);
  //  printf("Sender thread ready fd_out=%u\n",mei->fd_out);
  while(1) {
    //    printf("Hanging on queue outgoing\n");
    Data item = 0; 
    

    item = (Data)mei->outgoing->get();
    if (item == mei->die_package) {
      Data_destroy(mei->oe,&item);
      mei->die_package = 0;
      item = 0;
      //      printf("The blue pill\n");
    }

    if (item) {
      i2b(item->ldata,len);
      int written = 0;
      int sofar = 0;
      uint lbuf = item->ldata+4;
      byte * buf = mei->oe->getmem(lbuf);

      
      mcpy(buf, len, 4);
      mcpy(buf+4,item->data,item->ldata);
      Data_destroy( mei->oe, &item);

      written = 0;sofar = 0;
      while(sofar < lbuf && written >= 0) {
        written = mei->oe->write(mei->fd_out, buf, lbuf);
        sofar += written;
      }
      
      //      printf("fd_out = %u\n",mei->fd_out);
      
      if (written < 0) {
        mei->oe->p("Error writting to file descriptor." \
                   "This peer cannot send anymore.");
        break;
      }
    } else break;
  }
 out:
  mei->oe->unlock(mei->send_lock);
  mei->oe->p("Leaving sender thread");
  return 0;
}

static void MpcPeerImpl_destroy( MpcPeer * peer ) {
  MpcPeerImpl peer_i = 0;
  OE oe = 0;
  if (!peer) return ;
  if (!*peer) return ;


  peer_i = (MpcPeerImpl)(*peer)->impl;

  peer_i->running = 0;
  peer_i->die = 1;
  oe = peer_i->oe;

  peer_i->die_package = Data_new(oe,4);
  
  //  printf("Destroying peer, waiting for send lock\n");
  peer_i->outgoing->put(peer_i->die_package);
  oe->lock(peer_i->send_lock);
  //  printf("Acquired send lock, tearing down\n");

  if (peer_i->fd_in) {
    char m[32] = {0};
    //    osal_sprintf(m,"Closing fd_in=%d", peer_i->fd_in);
    //    oe->p(m);
    oe->close(peer_i->fd_in);
  }

  if (peer_i->fd_out) {
    char m[32] = {0};
    //    osal_sprintf(m,"Closing fd_out=%d", peer_i->fd_out);
    //    oe->p(m);
    oe->close(peer_i->fd_out);
  }
  


  oe->destroymutex(&peer_i->send_lock);
  oe->destroymutex(&peer_i->receive_lock);

  oe->jointhread(peer_i->receiver);
  //  printf("Done joining receive thread\n");

  //  printf("Joining sender thread\n");
  oe->jointhread(peer_i->sender);
  //  printf("Done joining sender thread\n");


  oe->putmem(peer_i);
  oe->putmem(*peer);
}

COO_DCL(MpcPeer, bool, has_data)
COO_DEF_RET_NOARGS(MpcPeer, bool, has_data) {
  MpcPeerImpl mei = 0;
  mei = (MpcPeerImpl)this->impl;

  return mei->incoming->size() > 0 || mei->drem;
}} 

COO_DCL(CArena, void, add_conn_listener, ConnectionListener l);
COO_DEF_NORET_ARGS(CArena, add_conn_listener, ConnectionListener l;,l) {
  CArenaImpl arena_i = (CArenaImpl)this->impl;
  OE oe = arena_i->oe;
  if (l){
    oe->lock(arena_i->lock);
    arena_i->conn_obs->add_element(l);
    oe->unlock(arena_i->lock);
  }
  return;
}}


COO_DCL(CArena, void, rem_conn_listener, ConnectionListener l);
COO_DEF_NORET_ARGS(CArena, rem_conn_listener, ConnectionListener l;,l) {
  CArenaImpl arena_i = (CArenaImpl)this->impl;
  OE oe = arena_i->oe;
  uint i = 0;
  if (l){
    int dead_index = -1;
    oe->lock(arena_i->lock);
    for(i = 0; i < arena_i->conn_obs->size();++i) {
      if (l == arena_i->conn_obs->get_element(i)) { 
        dead_index = i;
      };
    }
    if (dead_index >= 0) 
      arena_i->conn_obs->rem_element(dead_index);
    oe->unlock(arena_i->lock);
  }
  return;
  
}}

static MpcPeer MpcPeerImpl_new(OE oe, uint fd_in, char * ip, uint port) {
  MpcPeer peer = (MpcPeer)oe->getmem(sizeof(*peer));
  MpcPeerImpl peer_i = 0;
  if (!peer) return 0;

  peer_i = peer->impl = (MpcPeerImpl)oe->getmem(sizeof(*peer_i));
  if (!peer->impl) goto failure;

  peer_i->oe = oe;
  peer_i->port = port;
  peer_i->fd_in = fd_in;
  peer_i->running = 1;
  peer_i->die = 0;

  COO_ATTACH(MpcPeer, peer, send);
  COO_ATTACH(MpcPeer, peer, receive);
  COO_ATTACH(MpcPeer, peer, get_ip);
  COO_ATTACH(MpcPeer, peer, get_port);
  COO_ATTACH(MpcPeer, peer, has_data);

  return peer;
 failure:
  return 0;
}



static 
void send_int(OE oe, uint fd, int i) {
  byte d[4] = {0};
  uint sofar = 0;
  i2b(i,d);
  if ( oe->write(fd,d,4) != RC_OK) {
    //    oe->p("Write failed sending int");
  }
}

static 
int read_int(OE oe, uint fd) {
  byte d[4] = {0};
  uint read = 4;
  uint sofar = 0;
  while(sofar < 4) {
    if (oe->read(fd, d, &read) != RC_OK) {
      oe->p("Failure doing read int");
      return 0;
    }
    sofar += read;
    read = 4 - sofar;
  }
  return b2i(d);
}


COO_DCL(CArena, CAR, connect,  char * hostname, uint port)
COO_DEF_RET_ARGS(CArena, CAR, connect,  char * hostname; uint port;, hostname, port) {
  CAR c = {{0}};
  CArenaImpl arena_i = (CArenaImpl)this->impl;
  MpcPeer peer = 0;
  uint fd_out = 0;
  uint fd_in = 0;
  char addr[128] = {0};
  uint i = 0;
  uint myid = 0;
  MpcPeerImpl peer_i = 0;
  OE oe = arena_i->oe;
  
  osal_sprintf(addr, "ip %s %d", hostname, port);

  fd_out = arena_i->oe->open(addr);
  if (!fd_out) {
    arena_i->oe->p("Could not connect in");
    RETERR("Could not connect.", NO_CONN);
  }
  send_int(oe,fd_out,1024);
  myid = read_int(oe,fd_out);

  fd_in = arena_i->oe->open(addr);
  if (!fd_in) {
    arena_i->oe->p("Could not connect out");
    RETERR("Could not connect.", NO_CONN);
  }
  send_int(oe,fd_in,myid);

  peer = MpcPeerImpl_new( arena_i->oe, fd_in, hostname, port );
  if (!peer) RETERR("Unable to connect to peer", NO_CONN);

  peer_i = (MpcPeerImpl)peer->impl;
  peer_i->fd_out = fd_out;

  oe->lock(arena_i->lock);
  arena_i->peers->add_element(peer);
  oe->unlock(arena_i->lock);
  oe->unlock(peer_i->send_lock);
  oe->unlock(peer_i->receive_lock);
  for(i = 0;i < arena_i->conn_obs->size();++i) {
    ConnectionListener cl = (ConnectionListener)arena_i->conn_obs->get_element(i);
    if (cl) {
      cl->client_connected(peer);
    }
  }

  return c;
}}


static void * carena_listener_thread(void * a) {
  uint i = 0;
  CArena arena = (CArena)a;
  CArenaImpl arena_i = (CArenaImpl)arena->impl;
  OE oe = arena_i->oe;
  while(arena_i->running) {
    char mm[128] = {0};
    uint peer_id = 0;
    MpcPeer peer = 0;
    uint client_fd = 0;
    arena_i->oe->unlock(arena_i->listen_ready);
    oe->yieldthread();

    client_fd = arena_i->oe->accept(arena_i->server_fd);
    if (!client_fd) {
      arena_i->oe->p("Listening for clients failed");
      arena_i->running = 0;
      return 0;
    }

    peer_id = read_int(oe, client_fd);
    if (peer_id == 1024) {
      arena_i->oe->lock(arena_i->lock);
      peer_id = arena_i->peers->size();
      send_int(oe,client_fd,peer_id);
      peer = MpcPeerImpl_new(arena_i->oe,client_fd,0,0);
      arena_i->peers->add_element(peer);
      arena_i->oe->unlock(arena_i->lock);
      //      osal_sprintf(mm,"added client with id %u",peer_id);
      //      oe->p(mm);
      continue;
    }

    if (peer_id < 1024) {
      MpcPeerImpl peer_i = 0;
      //      osal_sprintf(mm,"incoming id was %u",peer_id);
      //      oe->p(mm);
      oe->lock(arena_i->lock);
      peer = arena_i->peers->get_element(peer_id);
      oe->unlock(arena_i->lock);
      if (peer == 0) {
        oe->p("Out Stream from client not registered...");
        close(client_fd);
        continue;
      }
      peer_i = (MpcPeerImpl)peer->impl;
      peer_i->fd_out = client_fd;
      
      oe->unlock(peer_i->send_lock);
      oe->unlock(peer_i->receive_lock);
      for(i = 0;i < arena_i->conn_obs->size();++i) {
        ConnectionListener cur = (ConnectionListener)
          arena_i->conn_obs->get_element(i);
        if (cur) {
          cur->client_connected(peer);
        }
      }
    } 
    
    arena_i->oe->yieldthread();
  }
  return 0;
}

typedef struct _default_connection_listener_ {
  MUTEX lock;
  int count;
  MUTEX hold;
  OE oe;
  void (*wait)(void);
} *DefaultConnectionListener;

COO_DCL(ConnectionListener, void, client_connected, MpcPeer peer) 
COO_DEF_NORET_ARGS(ConnectionListener, client_connected, MpcPeer peer;,peer) {
  ConnectionListener l = (ConnectionListener)this;
  DefaultConnectionListener dcl = (DefaultConnectionListener)this->impl;
  OE oe = l->oe;
  uint c = 0;
  
  oe->lock(dcl->lock);
  c = (dcl->count = dcl->count - 1);
  oe->unlock(dcl->lock);
  
  if (c <= 0) oe->unlock(dcl->hold);
  
  return;
}}

COO_DCL(ConnectionListener, void, client_disconnected, MpcPeer peer)
COO_DEF_NORET_ARGS(ConnectionListener, client_disconnected, MpcPeer peer;,peer) {
  ConnectionListener l = (ConnectionListener)this;
  OE oe = l->oe;
  return ;
}}

COO_DCL(DefaultConnectionListener, void, wait);
COO_DEF_NORET_NOARGS(DefaultConnectionListener, wait) {
  OE oe = this->oe;
  oe->lock(this->hold);
}}

ConnectionListener DefaultConnectionListener_new(OE oe, uint count) {
  ConnectionListener l = oe->getmem(sizeof(*l));
  DefaultConnectionListener dcl = oe->getmem(sizeof(*dcl));
  
  l->oe = oe;
  l->impl = dcl;
  COO_ATTACH(ConnectionListener,l,client_connected);
  COO_ATTACH(ConnectionListener,l,client_disconnected);
  COO_ATTACH(DefaultConnectionListener, dcl, wait);
  dcl->oe =oe;
  dcl->count = count;
  dcl->lock = oe->newmutex();
  dcl->hold = oe->newmutex();
  dcl->oe->lock(dcl->hold);
  return l;
}

void DefaultConnectionListener_destroy(ConnectionListener * cl) {
  DefaultConnectionListener dcl = 0;
  OE oe = 0;
  if (!cl) return;
  if (!*cl) return;
  
  oe = (*cl)->oe;

  dcl = (DefaultConnectionListener)(*cl)->impl;
  COO_DETACH(dcl, wait);
  COO_DETACH( (*cl),client_connected);
  COO_DETACH( (*cl),client_disconnected);
  oe->destroymutex(&dcl->lock);
  oe->destroymutex(&dcl->hold);
  oe->putmem(dcl);
  oe->putmem(*cl);
}             
  

COO_DCL(CArena, CAR, listen_wait, uint no, uint port);
COO_DEF_RET_ARGS(CArena, CAR, listen_wait, uint no; uint port;, no, port) {
  char addr[64] = {0};

  CAR c = {{0}};

  CArenaImpl arena_i = (CArenaImpl)this->impl;

  OE oe = arena_i->oe;

  ConnectionListener cl = DefaultConnectionListener_new(oe,no);
  DefaultConnectionListener dcl = (DefaultConnectionListener)cl->impl;

  if (no == 0) goto failure;

  this->add_conn_listener(cl);

  c = this->listen(port);
  if (c.rc != 0) goto failure;

  dcl->wait();

  this->rem_conn_listener(cl);

 failure:
  DefaultConnectionListener_destroy(&cl);
  return c;
}}

COO_DCL(CArena, CAR,listen, uint port)
COO_DEF_RET_ARGS(CArena, CAR, listen, uint port;, port) {

  char addr[64] = {0};

  CAR c = {{0}};
 
  CArenaImpl arena_i = (CArenaImpl)this->impl;

  uint server_fd = 0;

  osal_sprintf(addr, "listen %u", port);
  
  server_fd = arena_i->oe->open(addr);
  if (!server_fd) {
    arena_i->oe->p("Failed to open server socket for listening");
    RETERR("Failed to open server socket for listening.", NO_CONN);
  }
  
  arena_i->server_fd = server_fd;
  arena_i->oe->lock(arena_i->listen_ready);
  arena_i->worker = arena_i->oe->newthread(carena_listener_thread, this);
  arena_i->oe->lock(arena_i->listen_ready);
 
  return c;
}}


COO_DCL(CArena, MpcPeer, get_peer, uint pid)
COO_DEF_RET_ARGS(CArena, MpcPeer, get_peer, uint pid;,pid) {
  CArenaImpl arena_i = (CArenaImpl)this->impl;

  if (arena_i->peers->size()+1 > pid) {
    MpcPeer res = 0;
    arena_i->oe->lock(arena_i->lock);
    res = (MpcPeer)arena_i->peers->get_element(pid);
    arena_i->oe->unlock(arena_i->lock);
    return res;
  }

  return 0;
}}

COO_DCL(CArena, uint, get_no_peers)
COO_DEF_RET_NOARGS(CArena, uint, get_no_peers) {
  CArenaImpl arena_i = (CArenaImpl)this->impl;
  uint res = 0;
  arena_i->oe->lock(arena_i->lock);
  res = arena_i->peers->size();
  arena_i->oe->unlock(arena_i->lock);
  return res;
  
}}

void CArena_destroy( CArena * arena) {
  CArenaImpl arena_i = 0;
  OE oe = 0;

  if (!arena) return;
  if (!*arena) return;

  arena_i = (CArenaImpl)(*arena)->impl;
  oe = arena_i->oe;
  arena_i->running = 0;

  if (arena_i->worker) {
    oe->jointhread(arena_i->worker);
  }
  //  printf("Done joining with worker\n");

  if (arena_i->peers) {
    uint i = 0;
    for(i = 0;i<arena_i->peers->size();++i) {
      MpcPeer peer = (MpcPeer)arena_i->peers->get_element(i);
      if (peer) {
        MpcPeerImpl_destroy( &peer );
      }
    }
    SingleLinkedList_destroy( &arena_i->peers );
  }

  oe->close(arena_i->server_fd);
  //  printf("Done destroying peers\n");

  if (arena_i->conn_obs) {
    SingleLinkedList_destroy( &arena_i->conn_obs );
  }
  
  if (arena_i->lock) {
    oe->destroymutex(&arena_i->lock);
  }

  if (arena_i->listen_ready) {
    oe->destroymutex(&arena_i->listen_ready);
  }

  
  oe->putmem(arena_i);
  oe->putmem(*arena);
}

static
uint uint_hfn(void *a) {
  uint ai = (uint)(ull)a;
  uint hash = 65536*ai+4294967297;
  return hash;
}

static
int uint_cmp(void * a, void * b) {
  uint ai = (uint)(ull)a;
  uint bi = (uint)(ull)b;
  return ( ai > bi ? 1 : ( ai == bi ? 0 : -1));
}


COO_DCL(CArena, void, disconnect, uint peerid);
COO_DEF_NORET_ARGS(CArena, disconnect, uint peerid;, peerid) {
  
  MpcPeer peer = this->get_peer(peerid);

  if (peer) {
    MpcPeerImpl peer_i = (MpcPeerImpl)peer->impl;
    peer_i->oe->close(peer_i->fd_in);
    peer_i->oe->close(peer_i->fd_out);
  }

}}


#include "config.h"
CArena CArena_new(OE oe) {
  CArena arena = 0;
  CArenaImpl arena_i = 0;

  {
    oe->p("************************************************************");
    oe->p(" [2]"PACKAGE_STRING" - "CODENAME);
    oe->p("   "BUILD_TIME);
    oe->p("************************************************************");
  }

  arena = oe->getmem(sizeof(*arena));
  if (!arena) goto failure;

  arena_i = arena->impl = oe->getmem(sizeof(*arena_i));
  if (!arena->impl) goto failure;

  arena_i->oe = oe;
  arena_i->worker = 0;
  arena_i->lock = oe->newmutex();
  arena_i->running = 1;
  arena_i->peers = SingleLinkedList_new(oe);
  arena_i->conn_obs = SingleLinkedList_new(oe);
  arena_i->listen_ready = oe->newmutex();
  arena_i->peer_ids = 1024;

  COO_ATTACH(CArena, arena, connect);
  COO_ATTACH(CArena, arena, listen);
  COO_ATTACH(CArena, arena, listen_wait);
  COO_ATTACH(CArena, arena, get_peer);
  COO_ATTACH(CArena, arena, get_no_peers);
  COO_ATTACH(CArena, arena, add_conn_listener);
  COO_ATTACH(CArena, arena, rem_conn_listener);
  COO_ATTACH(CArena, arena, disconnect);
  return arena;
 failure:
  return 0;
}
