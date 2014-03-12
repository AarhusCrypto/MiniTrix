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
  uint fd;

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

} * MpcPeerImpl;


COO_DCL(MpcPeer,CAR, send3, Data data);
COO_DEF_RET_ARGS(MpcPeer, CAR, send3, Data data;, data) {
  CAR c = {{0}};
  MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
  
  
  
  return c;
}}

COO_DCL(MpcPeer,CAR, send2, Data data);
COO_DEF_RET_ARGS(MpcPeer, CAR, send2, Data data;, data) {
  CAR c = {{0}};
  MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
  byte len[4] = {0};
  peer_i->oe->write(peer_i->fd, data->data, data->ldata);
  return c;
}}


COO_DCL(MpcPeer, CAR, send, Data data)
COO_DEF_RET_ARGS(MpcPeer, CAR, send, Data data;, data) {
  CAR c = {{0}};
  MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
  RC rc = 0;
  //  rc = peer_i->oe->write(peer_i->fd, data->data, &data->ldata);
  //  printf("SEND %llu ns",_nano_time());
  CHECK_POINT_S(__FUNCTION__);
  peer_i->outgoing->put(Data_copy(peer_i->oe,data));
  CHECK_POINT_E(__FUNCTION__);
  c.rc = rc;
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

COO_DCL(MpcPeer, CAR, receive2, Data data);
COO_DEF_RET_ARGS(MpcPeer, CAR, receive2, Data data;, data) {
    MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
    OE oe = peer_i->oe;
    byte len[4] = {0};
    uint to_read = 0;
    local_read(oe,peer_i->fd, data->data, data->ldata);
}}

COO_DCL(MpcPeer, CAR, receive, Data data)
COO_DEF_RET_ARGS(MpcPeer, CAR, receive, Data data;, data) {
  MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
  CAR c = {{0}};
  RC rc = 0;

  // ldata is the number of bytes successfully read so far
  uint ldata = 0;
  Data d = 0;

  CHECK_POINT_S(__FUNCTION__);
  if (!data) { 
    osal_sprintf(c.msg,"CArena receive, bad input data is null");
    return c;
  }
  
  while(ldata < data->ldata) {
    Data chunk = 0;
    if (peer_i->drem != 0) {
      //      printf("DREM in use\n");
      chunk = peer_i->drem;
      peer_i->drem = 0;
    } else {
      chunk = peer_i->incoming->get();
    }

    if (chunk) {
      // data->ldata-ldata >= chunk->ldata
      // we have enough to fill {data}
      if (data->ldata-ldata >= chunk->ldata) {
        mcpy(data->data + ldata, chunk->data, chunk->ldata);
        ldata += chunk->ldata;
        Data_destroy(peer_i->oe,&chunk);
        continue;
      }
     
      // more data in chunk than asked for 
      // data->ldata-ldata < chunk->ldata
      if (data->ldata-ldata < chunk->ldata) {
        uint remaining = data->ldata-ldata;
        Data residue = 0;
        mcpy(data->data+ldata, chunk->data, remaining);
        ldata += chunk->ldata;
        residue = Data_new(peer_i->oe, chunk->ldata-remaining);
        mcpy(residue->data, chunk->data+remaining, chunk->ldata-remaining);
        Data_destroy(peer_i->oe,&chunk);
        peer_i->drem = residue;
      }
      
    
    } else {
      RETERR("Shutting down or out of memory", NO_MEM);
    }
  }

  CHECK_POINT_E(__FUNCTION__);

  return c;
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
  while(mei->running) {
    uint sofar = 0;
    byte len[4] = {0};
    uint lenlen = 4;
    RC rc = 0 ; 
    char _o[64] = {0};
    ull start = 0;
    ull waste = 0;

    //    if (_c_) waste = _nano_time();
    rc = mei->oe->read(mei->fd,len,&lenlen);
    //    if (_c_) printf("Time returning read %llu\n",_nano_time()-waste);
    if (lenlen <= 3) { 
      continue;
    }

    if (rc != RC_OK) {
      oe->p("Failed to read leaving");
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
        mei->incoming->put(buf);
        break;
      }

      // read_bytes will never be more than {buf->ldata} as we would
      // never ask for more. !! Invariant !!
      start = _nano_time();
      rc = mei->oe->read(mei->fd, buf->data+sofar, &read_bytes);
      tmp = (_nano_time()-start);
      if (read_bytes == 0) waste += tmp;
      if (rc == RC_OK) {

        // if we are shutting down better quit now !
        if (mei->die) goto out; // are we shutting down, then leave

        // okay was anything read?
        if (read_bytes > 0) { // yes okay update sofar !
          sofar += read_bytes;
          //    printf("read %u bytes sofar %u :)\n",read_bytes,sofar);
        }

      } else { // rc != RC_OK failure connection probably lost
        mei->oe->syslog(OSAL_LOGLEVEL_FATAL, 
                        "MpcPeer peer_rec_function failure to read."	\
                        " Peer will not receive anymore data.");
        return 0;
      }
    } // while(1)
  }
  
  
 out:
  mei->oe->p("Mpc Peer leaving receiver thread.\n");
  return 0;
}



static void * peer_snd_function(void * a) {
  MpcPeer me = (MpcPeer)a;

  MpcPeerImpl mei = (MpcPeerImpl)me->impl;
  
  byte len[4] = {0};

  while(mei->running) {
    Data item = (Data)mei->outgoing->get();
    if (mei->die) goto out;
    if (item) {
      i2b(item->ldata,len);
      int written = 0;
      int sofar = 0;
      ull start = _nano_time();
      uint lbuf = item->ldata+4;
      byte * buf = mei->oe->getmem(lbuf);

      
      mcpy(buf, len, 4);
      mcpy(buf+4,item->data,item->ldata);
      Data_destroy( mei->oe, &item);

      //      if (_c_) printf("[snd] Sending %u bytes ... ",lbuf);
      written = 0;sofar = 0;
      while(sofar < lbuf && written >= 0) {
        written = mei->oe->write(mei->fd, buf, lbuf);
        sofar += written;
        //if (_c_) printf("[snd] sofar %u,%u \n",sofar,written);
      }

      {
        //        ull now = _nano_time();
        //        printf("Done Sending took %llu at time %llu \n",(_nano_time()-start),now);
      }
      //      printf(" done \n");

      if (written < 0) {
        mei->oe->p("Error writting to file descriptor." \
                   "This peer cannot send anymore.");
        break;
      }
    }
  }
 out:
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

  if (peer_i->fd) {
    char m[32] = {0};
    osal_sprintf(m,"Closing fd=%d", peer_i->fd);
    oe->p(m);
    oe->close(peer_i->fd);
  }

  BlkQueue_destroy( & (peer_i->outgoing) );
  BlkQueue_destroy( & (peer_i->incoming) );

  oe->jointhread(peer_i->receiver);

  oe->jointhread(peer_i->sender);

}

COO_DCL(MpcPeer, bool, has_data)
COO_DEF_RET_NOARGS(MpcPeer, bool, has_data) {
  MpcPeerImpl mei = 0;
  mei = (MpcPeerImpl)this->impl;

  return mei->incoming->size() > 0 || mei->drem;
}} 

COO_DCL(CArena, void, add_conn_listener, ConnectionListener l)
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

static MpcPeer MpcPeerImpl_new(OE oe, uint fd, char * ip, uint port) {
  MpcPeer peer = (MpcPeer)oe->getmem(sizeof(*peer));
  MpcPeerImpl peer_i = 0;
  if (!peer) return 0;

  peer_i = peer->impl = (MpcPeerImpl)oe->getmem(sizeof(*peer_i));
  if (!peer->impl) goto failure;

  peer->ffd = fd;

  peer_i->oe = oe;
  peer_i->port = port;
  peer_i->fd = fd;
  peer_i->running = 1;
  peer_i->die = 0;
  peer_i->incoming = BlkQueue_new(oe,256);
  peer_i->outgoing = BlkQueue_new(oe,256);
  peer_i->receiver = oe->newthread( peer_rec_function, peer );
  peer_i->sender = oe->newthread( peer_snd_function, peer );

  if (!peer_i->fd) goto failure;

  COO_ATTACH(MpcPeer, peer, send);
  COO_ATTACH(MpcPeer, peer, receive);
  COO_ATTACH(MpcPeer, peer, get_ip);
  COO_ATTACH(MpcPeer, peer, get_port);
  COO_ATTACH(MpcPeer, peer, has_data);

  

  return peer;
 failure:
  return 0;
}


COO_DCL(CArena, CAR, connect,  char * hostname, uint port)
COO_DEF_RET_ARGS(CArena, CAR, connect,  char * hostname; uint port;, hostname, port) {
  CAR c = {{0}};
  CArenaImpl arena_i = (CArenaImpl)this->impl;
  MpcPeer peer = 0;
  uint fd = 0;
  char addr[128] = {0};
  uint i = 0;
  
  osal_sprintf(addr, "ip %s %d", hostname, port);

  fd = arena_i->oe->open(addr);
  if (!fd) {
    arena_i->oe->p("Could not connect");
    RETERR("Could not connect.", NO_CONN);
  }

  peer = MpcPeerImpl_new( arena_i->oe, fd, hostname, port );
  if (!peer) RETERR("Unable to connect to peer", NO_CONN);

  arena_i->peers->add_element(peer);
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

  while(arena_i->running) {
    
    MpcPeer peer = 0;
    arena_i->oe->unlock(arena_i->listen_ready);
    uint client_fd = arena_i->oe->accept(arena_i->server_fd);
    if (!client_fd) {
      arena_i->oe->p("Listening for clients failed");
      arena_i->running = 0;
      return 0;
    }


    arena_i->oe->p("Client joining");
    peer = MpcPeerImpl_new(arena_i->oe,client_fd,0,0);
    arena_i->oe->lock(arena_i->lock);
    arena_i->peers->add_element(peer);
    arena_i->oe->unlock(arena_i->lock);
    for(i = 0;i < arena_i->conn_obs->size();++i) {
      ConnectionListener cur = (ConnectionListener)
        arena_i->conn_obs->get_element(i);
      if (cur) {
        cur->client_connected(peer);
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
  c = dcl->count = dcl->count - 1;
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
  COO_ATTACH(ConnectionListener,l,client_connected);
  COO_ATTACH(ConnectionListener,l,client_disconnected);
  COO_ATTACH(DefaultConnectionListener, dcl, wait);
  dcl->oe =oe;
  dcl->count = count;
  dcl->lock = oe->newmutex();
  dcl->hold = oe->newmutex();
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

  this->listen(port);

  dcl->wait();

  DefaultConnectionListener_destroy(&cl);
  
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

  oe->close(arena_i->server_fd);

  if (arena_i->worker) {
    oe->jointhread(arena_i->worker);
  }

  if (arena_i->server_fd) {
    oe->close(arena_i->server_fd);
  }

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

#include "config.h"
CArena CArena_new(OE oe) {
  CArena arena = 0;
  CArenaImpl arena_i = 0;

  {
    oe->p("************************************************************");
    oe->p("   "PACKAGE_STRING" - "CODENAME);
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

  COO_ATTACH(CArena, arena, connect);
  COO_ATTACH(CArena, arena, listen);
  COO_ATTACH(CArena, arena, get_peer);
  COO_ATTACH(CArena, arena, get_no_peers);
  COO_ATTACH(CArena, arena, add_conn_listener);
  return arena;
 failure:
  return 0;
}
