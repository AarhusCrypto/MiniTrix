/*
 * TinyOT Implementation based on work by Jesper Buus Nielsen.
 *
 */
#include <tinyot.h>
#include <coo.h>
#include <osal.h>
#include <carena.h>
#include <stdio.h>

/*
 * UTILITIES FROM tinyot.cpp
 */

inline
tinyotshare * tshr_new(OE oe) {
  tinyotshare * res = (tinyotshare*)oe->getmem(sizeof(*res));
  return res;
}


static
int lenInLongs(int len) {
  int lenInLongs = len / 64;
  if (lenInLongs*64 < len) {
    lenInLongs++;
  }
  return lenInLongs;
}

static 
int longFoldLen(int len) {
  const unsigned int lenInBits = 64+len-1;
  unsigned int lenInLongs = lenInBits / 64;
  if (lenInLongs*64 < lenInBits) {
    lenInLongs++;
  }
  return lenInLongs;
}

static
void longFold(unsigned long* list, unsigned long* res, long len) {
  const int resLen = longFoldLen(len);
  int i = 0;
  for (i=0; i<resLen; i++) {
    res[i]=0;
  }
  
  for (i=0; i<len; i++) {
    const unsigned long wordi = list[i];
    if (i % 64 == 0) {
      /* The easy case where word i fits into a word in res. */
      res[i/64] = res[i/64] ^ wordi;
    } else {
      const unsigned long word1 = wordi >> (i%64);
      const unsigned long word2 = wordi << (64-(i%64));
      /* Now word1 concat word2 is wordi shifted i%64 position right, 
       * with infinite position. We shift it i/64 words more. */
      res[i/64] = res[i/64] ^ word1;
      res[(i/64)+1] = res[(i/64)+1] ^ word2;
    }
  }
}

static 
int sendBits(MpcPeer peer, char* buf, unsigned long* wordBuf, int len) {
  int lenInWords = lenInLongs(len);
  int i = 0;
  for (i=0; i<lenInWords; i++) {
    wordBuf[i]=0;
  }
  for ( i=0; i<len; i++) {
    wordBuf[i/64] = (wordBuf[i/64]<<1) | buf[i];
  }

  peer->send(Data_shallow((byte*)wordBuf, lenInWords*8));
  return 0;
}

static
int receiveBits(MpcPeer peer, char * buf, unsigned long * wordBuf, int len) {
  int lenInWords = lenInLongs(len);
  int i  =0;
  peer->receive(Data_shallow((byte*)wordBuf, lenInWords*8));
  for(i = len-1;i >= 0;--i) {
    buf[i] = wordBuf[i/64] & 1;
    wordBuf[i/64] = wordBuf[i/64] >> 1;
  }
  return 0;
}

static 
void _COPY(tinyotshare * res, tinyotshare * left) {
  res->shr = left->shr;
  res->mac = left->mac;
  res->key = left->key;
}



typedef struct _tinyot_impl_ {
  
  tinyotshare ** heap;
  
  unsigned long delta;

  uint noOfAnds;

  tinyotand * ands;

  unsigned long *hisMacsExpected;
  unsigned long *hisMacsExpectedFolded;
  unsigned long *myMacs;
  unsigned long *myMacsFolded;
  unsigned long *hisMacsFolded;

  opbuffer * andBuffer;
  unsigned int noToVerify;

  char * tmpBuf;
  char * buf;
  unsigned long *wordBuf;
  unsigned int commBufSize;

  byte ** pcsBuf;
  int * outBuf;

  bool isAlice;

  int evalCode ;

  int sizeHeap;
  

  bool freeAnds;
  
  void (*AliceShare)(tinyotshare * res, byte s);

  void (*BobShare)(tinyotshare* res, char s);

  void (*swapBits)(int len);

  void (*verify)(void);

  void (*checkMACs)(uint len);

  void (*alloc_comm_buf)(uint size);

  CArena arena;
  OE oe;

} * TinyOTImpl;

COO_DCL(TinyOT, void, init_heap, uint size);
COO_DEF_NORET_ARGS(TinyOT, init_heap, uint size;, size) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  impl->heap = (tinyotshare**)impl->oe->getmem(sizeof(*impl->heap)*size);
  if (impl->heap != 0)
    impl->sizeHeap = size;
  else
    impl->oe->p("Unable to initialize heap.");
}

COO_DCL(TinyOT, tinyotshare *, heap_get, hptr addr);
COO_DEF_RET_ARGS(TinyOT, tinyotshare *, heap_get, hptr addr;,addr) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  if (addr < impl->sizeHeap) return impl->heap[addr];
  else { 
    char m[64] = {0};
    osal_sprintf(m, "Heap addresse %u out of range [0-%u].", addr, impl->sizeHeap-1);
    impl->oe->p(m);
    return 0;
  }
}

COO_DCL(TinyOT, void, heap_set, hptr addr, tinyotshare*share);
COO_DEF_NORET_ARGS(TinyOT, heap_set, hptr addr; tinyotshare*share;,addr,share) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  if (addr < impl->sizeHeap) {
    char m[64] = {0};
    if (!share)  { 
      osal_sprintf(m,"Warning setting %u to null.",addr);
      impl->oe->p(m);
    }
    impl->heap[addr] = share;
  } else {
    char m[64] = {0};
    osal_sprintf(m,"Address %u is out of range [0-%u]",addr, impl->sizeHeap-1);
    impl->oe->p(m);
  }
}


COO_DCL(TinyOT, void, ZERO, hptr addr);
COO_DEF_NORET_ARGS(TinyOT, ZERO, hptr addr;,addr) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;

  if (!impl->heap[addr]) {
    char m[64] = {0};
    osal_sprintf(m,"Trying to zero out an unset address %u.", addr);
    impl->oe->p(m);
    return;
  }

  if (!impl->heap[addr]) {
    impl->heap[addr] = tshr_new(impl->oe);
  }
  impl->AliceShare(impl->heap[addr],0);
  impl->BobShare(impl->heap[addr],0);
}


COO_DCL(TinyOTImpl, void, AliceShare, tinyotshare * res, char s);
COO_DEF_NORET_ARGS(TinyOTImpl, AliceShare, tinyotshare* res; char s;,res,s) {
  
  if (!res) {
    this->oe->p("Oh no AliceShare was given null.");
  }

  if (this->isAlice) {
    res->shr = s;
    res->mac = 0;
  } else {
    res->key = s*this->delta;
  }
}

COO_DCL(TinyOTImpl, void, BobShare, tinyotshare * res, char s);
COO_DEF_NORET_ARGS(TinyOTImpl, BobShare, tinyotshare * res; char s;,res,s) {

  if (!res) {
    this->oe->p("On Bob share was given null.");
  }

  if (!this->isAlice) {
    res->shr = s;
    res->mac = 0;
  } else {
    res->key = s*this->delta;
  }
}

COO_DCL(TinyOT, void, ONE, hptr addr);
COO_DEF_NORET_ARGS(TinyOT, ONE, hptr addr;,addr) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;

  if (!impl->heap[addr]) {
    char m[64] = {0};
    osal_sprintf(m,"Trying to set one with unset address %u.", addr);
    impl->oe->p(m);
    return;
  }
  

  impl->AliceShare(impl->heap[addr],0);
  impl->BobShare(impl->heap[addr],1);
}


static 
void _XOR(tinyotshare * res, tinyotshare * left, tinyotshare * right) {
  res->shr = left->shr ^ right->shr;
  res->mac = left->mac ^ right->mac;
  res->key = left->key ^ right->key;
}


COO_DCL(TinyOT, void, XOR, hptr dst, hptr left, hptr right,uint _);
COO_DEF_NORET_ARGS(TinyOT, XOR, hptr dst; hptr left; hptr right; uint _;, dst, left, right,_) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  tinyotshare * res = 0;
  char m[64] = {0};
  osal_sprintf(m,"xor %u",dst);
  impl->oe->p(m);
  if (dst > impl->sizeHeap) {
    char m[64] = {0};
    osal_sprintf(m,"Destination of XOR is out of range %u.",dst);
    impl->oe->p(m);
    return;
  }

  if (left > impl->sizeHeap || impl->heap[left] == 0) {
    char m[64] = {0};
    osal_sprintf(m,"left operand %u is either out of range or not set.",left);
    impl->oe->p(m);
    return;
  }

  if (right > impl->sizeHeap || impl->heap[right] == 0) {
    char m[64]= {0};
    osal_sprintf(m, "right operand %u is either out of range or not set.",right);
    impl->oe->p(m);
    return;
  }
  
  res = tshr_new(impl->oe);
  _XOR(res,impl->heap[left],impl->heap[right]);
  impl->heap[dst] = res;

}


COO_DCL(TinyOT, void, AND, hptr dst, hptr left, hptr right, uint pos, uint no);
COO_DEF_NORET_ARGS(TinyOT, AND, hptr dst; hptr left; hptr right; uint pos; uint no;
                   , dst, left, right, pos, no) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  char m[64] = {0};
  osal_sprintf(m,"mul %u",dst);
  impl->oe->p(m);
  if (dst > impl->sizeHeap) {
    char m[64] = {0};
    osal_sprintf(m,"Destination of XOR is out of range %u.",dst);
    impl->oe->p(m);
    return;
  }

  if (left > impl->sizeHeap || impl->heap[left] == 0) {
    char m[64] = {0};
    osal_sprintf(m,"left operand %u is either out of range or not set.",left);
    impl->oe->p(m);
    return;
  }

  if (right > impl->sizeHeap || impl->heap[right] == 0) {
    char m[64]= {0};
    osal_sprintf(m, "right operand %u is either out of range or not set.",right);
    impl->oe->p(m);
    return;
  }
  
  {
    tinyotand * pre = impl->ands + no;
    _XOR(&impl->andBuffer[pos].X, impl->heap[left], &pre->a);
    _XOR(&impl->andBuffer[pos].Y, impl->heap[right], &pre->b);
    
    impl->buf[2*pos+0] = impl->andBuffer[pos].X.shr;
    impl->myMacs[impl->noToVerify+2*pos+0] = impl->andBuffer[pos].X.mac;
    
    impl->buf[2*pos+1] = impl->andBuffer[pos].Y.shr;
    impl->myMacs[impl->noToVerify+2*pos+1] = impl->andBuffer[pos].Y.mac;

    impl->heap[dst] = tshr_new(impl->oe);

    impl->andBuffer[pos].left = impl->heap[left];
    impl->andBuffer[pos].right = impl->heap[right];
    impl->andBuffer[pos].res = impl->heap[dst];
    impl->andBuffer[pos].a = &pre->a;
    impl->andBuffer[pos].ab = &pre->ab;
  }
  // TODO(rwl): The end_and_layer is needed
}

COO_DCL(TinyOT, void, INV, hptr dst, hptr op, uint _);
COO_DEF_NORET_ARGS(TinyOT, INV, hptr dst; hptr op; uint _;,dst,op,_) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  char m[64] = {0};
  osal_sprintf(m,"inv %u",dst);
  impl->oe->p(m);

  if (dst > impl->sizeHeap) {
    char m[64] = {0};
    osal_sprintf(m,"destination %u is out of range.",dst);
    impl->oe->p(m);
    return;
  }

  if (op > impl->sizeHeap || impl->heap[op] == 0) {
    char m[64] = {0};
    osal_sprintf(m,"left operand %u is either out of range or not set.",op);
    impl->oe->p(m);
    return;
  }

  impl->heap[dst] = tshr_new(impl->oe);

  printf("is alice = %u",impl->isAlice);

  if (impl->isAlice) {
    impl->heap[dst]->shr = impl->heap[op]->shr;
    impl->heap[dst]->mac = impl->heap[op]->mac;
    impl->heap[dst]->key = impl->heap[op]->key ^ impl->delta;
  } else {
    impl->oe->p("Bob does\n");
    impl->heap[dst]->shr = impl->heap[op]->shr ^ 1;
    impl->heap[dst]->mac = impl->heap[op]->mac;
    impl->heap[dst]->key = impl->heap[op]->key;
  }

}

COO_DCL(TinyOTImpl, void, swapBits, int len);
COO_DEF_NORET_ARGS(TinyOTImpl, swapBits, int len;,len){ 

  MpcPeer peer = this->arena->get_peer(0);
  if (!peer) {
      this->oe->p("No Peer connected");
      return;
  }

  
  if (this->isAlice) {
    sendBits(peer, this->buf, this->wordBuf, len);
    receiveBits(peer, this->buf, this->wordBuf, len);
  } else {
    receiveBits(peer, this->tmpBuf, this->wordBuf, len);
    sendBits(peer, this->buf, this->wordBuf, len);
    char * tmp = this->tmpBuf;
    this->tmpBuf = this->buf;
    this->buf = tmp;
  }
}


COO_DCL(TinyOT, void, end_layer_AND, uint width);
COO_DEF_NORET_ARGS(TinyOT, end_layer_AND, uint width;, width) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  uint pos = 0;
  impl->swapBits(2*width);
  for (pos=0; pos<width; ++pos) {
    /*
     * Record the received share along with the keys and mac
     * for later verification.
     */

    impl->hisMacsExpected[impl->noToVerify+2*pos+0] 
      = impl->andBuffer[pos].X.key ^ impl->buf[2*pos+0]*impl->delta;

    impl->hisMacsExpected[impl->noToVerify+2*pos+1] 
      = impl->andBuffer[pos].Y.key ^ impl->buf[2*pos+1]*impl->delta;

    impl->andBuffer[pos].X.shr ^= impl->buf[2*pos+0];
    impl->andBuffer[pos].Y.shr ^= impl->buf[2*pos+1];

    
    _COPY(impl->andBuffer[pos].res,impl->andBuffer[pos].ab);  
    if (impl->andBuffer[pos].X.shr==1) {
      _XOR(impl->andBuffer[pos].res,impl->andBuffer[pos].res,impl->andBuffer[pos].right);
    }
    if (impl->andBuffer[pos].Y.shr==1) {
      _XOR(impl->andBuffer[pos].res,impl->andBuffer[pos].res,impl->andBuffer[pos].a);
    }
  }
  impl->noToVerify+=2*width;


  
}

COO_DCL(TinyOT, void, begin_layer_public_common_store, uint len);
COO_DEF_NORET_ARGS(TinyOT, begin_layer_public_common_store, uint len;, len) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  
  impl->verify();
 
}


COO_DCL(TinyOT, void, end_layer_public_common_store, uint len);
COO_DEF_NORET_ARGS(TinyOT, end_layer_public_common_store, uint len;, len) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  int pos = 0;
  impl->swapBits(len);
  
  for(pos = 0; pos < len;++pos) {
    impl->hisMacsExpected[impl->noToVerify-len+pos]=impl->hisMacsExpected[impl->noToVerify-len+pos] ^
      impl->buf[pos] * impl->delta;
    
  }

  impl->verify();

  for(pos = 0;pos < len;++pos) {
    *impl->pcsBuf[pos] = impl->heap[impl->outBuf[pos]]->shr ^ (impl->buf[pos]);
  }
}

COO_DCL(TinyOTImpl, void, alloc_comm_buf, uint size);
COO_DEF_NORET_ARGS(TinyOTImpl, alloc_comm_buf, uint size;,size) {
  if (this->commBufSize < size) {
    if (this->commBufSize > 0) {
      this->oe->putmem(this->buf);
      this->oe->putmem(this->tmpBuf);
      this->oe->putmem(this->wordBuf);
    }
    this->commBufSize = size;
    this->buf = this->oe->getmem(size);
    this->tmpBuf = this->oe->getmem(size);
    this->wordBuf = this->oe->getmem(size*sizeof(unsigned long int));
  }
}


COO_DCL(TinyOT, void, public_common_store, byte*s, uint out, uint pos);
COO_DEF_NORET_ARGS(TinyOT, public_common_store, byte * s; uint out; uint pos;,
                   s, out, pos) {
  
  TinyOTImpl impl = (TinyOTImpl)this->impl;

  impl->pcsBuf[pos] = s;
  impl->outBuf[pos] = out;
  impl->buf[pos] = impl->heap[out]->shr;

  impl->myMacs[impl->noToVerify] = impl->heap[out]->mac;
  impl->hisMacsExpected[impl->noToVerify] = impl->heap[out]->key;
  impl->noToVerify++;
  

}

COO_DCL(TinyOT, void, no_of_ands, uint size);
COO_DEF_NORET_ARGS(TinyOT, no_of_ands, uint size;, size) {
  
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  
  if (impl->hisMacsExpected==NULL) {
    impl->noOfAnds = size;
    
    if (impl->ands==NULL) {
      int i = 0;
      impl->ands = impl->oe->getmem(sizeof(tinyotand)*size);
      for (i=0; i<size; i++) {
	impl->ands[i].a.shr = 0;
	impl->ands[i].a.key = 0;
	impl->ands[i].a.mac = 0;
	
	impl->ands[i].b.shr = 0;
	impl->ands[i].b.key = 0;
	impl->ands[i].b.mac = 0;
	
	impl->ands[i].ab.shr = 0;
	impl->ands[i].ab.key = 0;
	impl->ands[i].ab.mac = 0;
      }
    }

    impl->hisMacsExpected = impl->oe->getmem(sizeof(unsigned long)*(2*size));
    impl->hisMacsExpectedFolded = impl->oe->getmem(sizeof(unsigned long)*longFoldLen(2*size));
    impl->myMacs = impl->oe->getmem(sizeof(unsigned long)*(2*size));
    impl->myMacsFolded = impl->oe->getmem(sizeof(unsigned long)*(longFoldLen(2*size)));
    impl->hisMacsFolded = impl->oe->getmem(sizeof(unsigned long)*(longFoldLen(2*size)));
  }

}

COO_DCL(TinyOT, void, max_width_AND, uint size);
COO_DEF_NORET_ARGS(TinyOT, max_width_AND, uint size;, size) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  if (impl->andBuffer==NULL) {
    impl->andBuffer = impl->oe->getmem(sizeof(opbuffer)*size);
    impl->alloc_comm_buf(2*size);
  }
}


COO_DCL(TinyOT, void, max_width_public_common_store, uint len);
COO_DEF_NORET_ARGS(TinyOT, max_width_public_common_store, uint len;, len) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  
  impl->alloc_comm_buf(len);
  impl->pcsBuf = impl->oe->getmem(sizeof(byte*)*len);
  impl->outBuf = impl->oe->getmem(len*sizeof(int));

}

COO_DCL(TinyOT, void, private_common_load, tinyotshare * in, uint res, uint _);
COO_DEF_NORET_ARGS(TinyOT, private_common_load, 
                   tinyotshare * in; uint res; uint _;, in, res, _) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  if (!in) {
    char m[80] = {0};
    osal_sprintf(m,"private common load, setting address %u to null.\n",res);
    impl->oe->p(m);
  }
  
  impl->heap[res] = tshr_new(impl->oe);
  *(impl->heap[res]) = *in;
}

COO_DCL(TinyOT, void, public_common_load, byte val, uint res, uint _);
COO_DEF_NORET_ARGS(TinyOT, public_common_load, 
                   byte val; uint res; uint _;, val, res, _) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  tinyotshare * ress = tshr_new(impl->oe);
  impl->AliceShare(ress, 0);
  impl->BobShare(ress,val);
  impl->heap[res] = ress;
}

/*
COO_DCL(TinyOT, void, end_layer_AND, uint width);
COO_DEF_NORET_ARGS(TinyOT, end_layer_AND, uint width;, width) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  int pos = 0;
  impl->swapBits(2*width);
  for (pos=0; pos<width; pos++) {
  //
  //   * Record the received share along with the keys and mac
  //   * for later verification.
  //   *

    impl->hisMacsExpected[impl->noToVerify+2*pos+0] 
      = impl->andBuffer[pos].X.key ^ impl->buf[2*pos+0]*impl->delta;

    impl->hisMacsExpected[impl->noToVerify+2*pos+1] 
      = impl->andBuffer[pos].Y.key ^ impl->buf[2*pos+1]*impl->delta;

    impl->andBuffer[pos].X.shr ^= impl->buf[2*pos+0];
    impl->andBuffer[pos].Y.shr ^= impl->buf[2*pos+1];

    _COPY(andBuffer[pos].res,andBuffer[pos].ab);  
    if (andBuffer[pos].X.shr==1) {
      _XOR(andBuffer[pos].res,andBuffer[pos].res,andBuffer[pos].right);
    }
    if (andBuffer[pos].Y.shr==1) {
      _XOR(andBuffer[pos].res,andBuffer[pos].res,andBuffer[pos].a);
    }
  }
  noToVerify+=2*width;


}}
*/

COO_DCL(TinyOTImpl, void, verify);
COO_DEF_NORET_NOARGS(TinyOTImpl, verify) {
  this->checkMACs(this->noToVerify);
  this->noToVerify = 0;
}


COO_DCL(TinyOTImpl, void, checkMACs,uint len);
COO_DEF_NORET_ARGS(TinyOTImpl, checkMACs, uint len;, len) {
  int foldedLen = longFoldLen(len);
  int i = 0;

  MpcPeer peer = 0;

  peer = this->arena->get_peer(0);
  if (!peer) {
    this->oe->p("Communications failure");
    return;
  }

  longFold(this->myMacs, this->myMacsFolded,len);
  longFold(this->hisMacsExpected, this->hisMacsExpectedFolded, len);
  
  if (this->isAlice) {
    peer->send(Data_shallow((byte*)this->myMacsFolded, foldedLen * sizeof(unsigned long)));
    peer->receive(Data_shallow((byte*)this->hisMacsFolded, foldedLen * sizeof(unsigned long)));
  } else {
    peer->send(Data_shallow((byte*)this->myMacsFolded, foldedLen * sizeof(unsigned long)));
    peer->receive(Data_shallow((byte*)this->hisMacsFolded, foldedLen * sizeof(unsigned long)));
  }

  for(i = 0;i < foldedLen;++i) {
    if (this->hisMacsFolded[i] != this->hisMacsExpectedFolded[i]) {
      this->oe->p("Cheating detected !!!!");
    }
  }
}



COO_DCL(TinyOT, uint, invite, uint port);
COO_DEF_RET_ARGS(TinyOT, uint, invite, uint port;, port) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  return impl->arena->listen_wait(1,port).rc;
}

COO_DCL(TinyOT, uint, connect, char * ip, uint port);
COO_DEF_RET_ARGS(TinyOT, uint, connect, char * ip; uint port;, ip, port) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  return impl->arena->connect(ip,port).rc;
}

COO_DCL(TinyOT, void, dummy, uint w);
COO_DEF_NORET_ARGS(TinyOT, dummy, uint w;, w) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  impl->oe->p("");
}


COO_DCL(TinyOT, bool, isAlice);
COO_DEF_RET_NOARGS(TinyOT, bool, isAlice) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  return impl->isAlice;
}

#include <config.h>
TinyOT TinyOT_new(OE oe, bool isAlice) {
  TinyOT res = (TinyOT)oe->getmem(sizeof(*res));
  TinyOTImpl impl = 0;
  if (!res) return 0;

  impl = (TinyOTImpl)oe->getmem(sizeof(*impl));
  if (!impl) return 0;

  res->impl = impl;
  impl->arena = CArena_new(oe);
  impl->oe = oe;
  impl->isAlice = isAlice;

  COO_ATTACH(TinyOT, res, isAlice);
  COO_ATTACH(TinyOT, res, invite);
  COO_ATTACH(TinyOT, res, connect);
  COO_ATTACH(TinyOT, res, init_heap);
  COO_ATTACH(TinyOT, res, heap_get);
  COO_ATTACH(TinyOT, res, heap_set);
  COO_ATTACH(TinyOT, res, ZERO);
  COO_ATTACH(TinyOT, res, ONE);
  COO_ATTACH(TinyOT, res, XOR);
  COO_ATTACH(TinyOT, res, AND);
  COO_ATTACH(TinyOT, res, INV);
  COO_ATTACH(TinyOT, res, end_layer_AND);
  COO_ATTACH(TinyOT, res, begin_layer_public_common_store);
  COO_ATTACH(TinyOT, res, end_layer_public_common_store);
  COO_ATTACH(TinyOT, res, public_common_store);
  COO_ATTACH(TinyOT, res, no_of_ands);
  COO_ATTACH(TinyOT, res, max_width_AND);
  COO_ATTACH(TinyOT, res, max_width_public_common_store);
  COO_ATTACH(TinyOT, res, private_common_load);
  COO_ATTACH(TinyOT, res, public_common_load);
  COO_ATTACH_FN(TinyOT, res, end_layer_INV, dummy);
  COO_ATTACH_FN(TinyOT, res, begin_layer_XOR, dummy);
  COO_ATTACH_FN(TinyOT, res, end_layer_XOR, dummy);
  COO_ATTACH_FN(TinyOT, res, begin_layer_INV, dummy);
  COO_ATTACH_FN(TinyOT, res, begin_layer_AND, dummy);
  COO_ATTACH_FN(TinyOT, res, max_width_XOR, dummy);
  COO_ATTACH_FN(TinyOT, res, max_width_INV, dummy);
  COO_ATTACH_FN(TinyOT, res, max_width_private_common_load, dummy);
  COO_ATTACH_FN(TinyOT, res, max_width_public_common_load, dummy);
  COO_ATTACH_FN(TinyOT, res, end_layer_private_common_load, dummy);
  COO_ATTACH_FN(TinyOT, res, begin_layer_public_common_load, dummy);
  COO_ATTACH_FN(TinyOT, res, begin_layer_private_common_load, dummy);
  COO_ATTACH_FN(TinyOT, res, end_layer_public_common_load, dummy);

  COO_ATTACH(TinyOTImpl, impl, AliceShare);
  COO_ATTACH(TinyOTImpl, impl, BobShare);
  COO_ATTACH(TinyOTImpl, impl, swapBits);
  COO_ATTACH(TinyOTImpl, impl, verify);
  COO_ATTACH(TinyOTImpl, impl, checkMACs);
  COO_ATTACH(TinyOTImpl, impl, alloc_comm_buf);
  
  oe->p("************************************************************");
  oe->p("   " PACKAGE_STRING " - " CODENAME );
  oe->p("   " BUILD_TIME);
  oe->p("************************************************************");

  return res;
}

void TinyOT_destroy(TinyOT * instance) {

}
