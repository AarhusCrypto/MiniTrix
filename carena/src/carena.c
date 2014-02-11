#include "carena.h"
#include <list.h>
#include <coo.h>
#include <blockingqueue.h>
#include <errno.h>
#include <singlelinkedlist.h>
#include <common.h>
#include <encoding/hex.h>

typedef struct _carena_impl_ {

  uint server_fd;
  
  OE oe;

  List peers;

  ThreadID worker;

  bool running;

  MUTEX lock;

  List conn_obs;

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

COO_DCL(MpcPeer, CAR, send, Data data)
COO_DEF_RET_ARGS(MpcPeer, CAR, send, Data data;, data) {
  CAR c = {{0}};

  MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
  peer_i->outgoing->put(Data_copy(peer_i->oe,data));
  
  return c;
}}

COO_DCL(MpcPeer, CAR, receive, Data data)
COO_DEF_RET_ARGS(MpcPeer, CAR, receive, Data data;, data) {
  MpcPeerImpl peer_i = (MpcPeerImpl)this->impl;
  CAR c = {{0}};
  uint ldata = 0;
  if (!data) { 
    osal_sprintf(c.msg,"CArena receive, bad input data is null");
    return c;
  }
  while(ldata < data->ldata) {
    Data chunk = 0;
    if (peer_i->drem != 0) {
      chunk = peer_i->drem;
      peer_i->drem = 0;
    } else {
      chunk = peer_i->incoming->get();
    }
    if (chunk) {
      // data->ldata-ldata >= chunk->ldata
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
  if (!me) goto out;

  mei = (MpcPeerImpl)me->impl;
  buf = (Data)Data_new(mei->oe,1024);  
  if (!buf) goto out;
  
  while(mei->running) {
    RC rc = mei->oe->read(mei->fd, buf->data, &buf->ldata);
    if (rc == RC_OK) {
      if (mei->die) goto out;
      if (buf->ldata > 0) {
        mei->incoming->put(buf);
        buf=Data_new(mei->oe,1024);
      } else {
        buf->ldata = 1024; 
      }
    } else {
      mei->oe->syslog(OSAL_LOGLEVEL_FATAL, 
                      "MpcPeer peer_rec_function failure to read."	\
                      " Peer will not receive anymore data.");
      break;
    }
  }
  
  
 out:
  mei->oe->p("Mpc Peer leaving receiver thread.\n");
  return 0;
}

static void * peer_snd_function(void * a) {
  MpcPeer me = (MpcPeer)a;

  MpcPeerImpl mei = (MpcPeerImpl)me->impl;
  

  while(mei->running) {
    Data item = (Data)mei->outgoing->get();
    if (mei->die) goto out;
    if (item) {
      int written = mei->oe->write(mei->fd, item->data, item->ldata);
      Data_destroy( mei->oe, &item);
      if (written < 0) {
	mei->oe->p("Error writting to file descriptor."\
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


  peer_i->oe = oe;
  peer_i->port = port;
  peer_i->fd = fd;
  peer_i->running = 1;
  peer_i->die = 0;
  peer_i->incoming = BlkQueue_new(oe,8);
  peer_i->outgoing = BlkQueue_new(oe,8);
  peer_i->receiver = oe->newthread( peer_rec_function, peer );
  peer_i->sender = oe->newthread( peer_snd_function, peer );

  if (!peer_i->fd) goto failure;

  COO_ATTACH(MpcPeer, peer, send );
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

COO_DCL(CArena, CAR,listen, uint port)
COO_DEF_RET_ARGS(CArena, CAR, listen, uint port;, port) {

  

  char addr[64] = {0};

  CAR c = {{0}};
 
  CArenaImpl arena_i = (CArenaImpl)this->impl;

  uint server_fd = 0;

  osal_sprintf(addr, "listen %d", port);

  server_fd = arena_i->oe->open(addr);
  if (!server_fd) {
    arena_i->oe->p("Failed to open server socket for listening");
    RETERR("Failed to open server socket for listening.", NO_CONN);
  }
  
  arena_i->server_fd = server_fd;

  arena_i->worker = arena_i->oe->newthread(carena_listener_thread, this);
 
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
    SingleLinkedList_destroy( & arena_i->peers );
  }

  if (arena_i->lock) {
    oe->destroymutex(&arena_i->lock);
  }

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

  COO_ATTACH(CArena, arena, connect);
  COO_ATTACH(CArena, arena, listen);
  COO_ATTACH(CArena, arena, get_peer);
  COO_ATTACH(CArena, arena, get_no_peers);
  COO_ATTACH(CArena, arena, add_conn_listener);
  return arena;
 failure:
  return 0;
}
