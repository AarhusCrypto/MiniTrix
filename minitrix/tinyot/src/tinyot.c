/*
 * TinyOT Implementation based on work by Jesper Buus Nielsen.
 *
 */
#include <tinyot.h>
#include <coov4.h>
#include <osal.h>
#include <carena.h>
#include <stdio.h>

/*
 * UTILITIES FROM tinyot.cpp
 */


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

COO_DEF(TinyOT, void, init_heap, uint size) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  impl->heap = (tinyotshare**)impl->oe->getmem(sizeof(*impl->heap)*size);
  if (impl->heap != 0)
    impl->sizeHeap = size;
  else
    impl->oe->p("Unable to initialize heap.");
}}

COO_DEF(TinyOT, tinyotshare *, heap_get, hptr addr) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  if (addr < impl->sizeHeap) return impl->heap[addr];
  else { 
    char m[64] = {0};
    osal_sprintf(m, "Heap addresse %u out of range [0-%u].", addr, impl->sizeHeap-1);
    impl->oe->p(m);
    return 0;
  }
}}

COO_DEF(TinyOT, void, heap_set, hptr addr, tinyotshare*share) {
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
}}


COO_DEF(TinyOT, void, ZERO, hptr addr) {
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
}}


COO_DEF(TinyOTImpl, void, AliceShare, tinyotshare * res, char s) {
  if (!res) {
    this->oe->p("Oh no AliceShare was given null.");
  }

  if (this->isAlice) {
    res->shr = s;
    res->mac = 0;
  } else {
    res->key = s*this->delta;
  }
}}

COO_DEF(TinyOTImpl, void, BobShare, tinyotshare * res, char s) {

  if (!res) {
    this->oe->p("On Bob share was given null.");
  }

  if (!this->isAlice) {
    res->shr = s;
    res->mac = 0;
  } else {
    res->key = s*this->delta;
  }
}}

COO_DEF(TinyOT, void, ONE, hptr addr){
  TinyOTImpl impl = (TinyOTImpl)this->impl;

  if (!impl->heap[addr]) {
    char m[64] = {0};
    osal_sprintf(m,"Trying to set one with unset address %u.", addr);
    impl->oe->p(m);
    return;
  }
  

  impl->AliceShare(impl->heap[addr],0);
  impl->BobShare(impl->heap[addr],1);
}}


static 
void _XOR(tinyotshare * res, tinyotshare * left, tinyotshare * right) {
  res->shr = left->shr ^ right->shr;
  res->mac = left->mac ^ right->mac;
  res->key = left->key ^ right->key;
}


COO_DEF(TinyOT, void, XOR, hptr dst, hptr left, hptr right,uint _) {
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
}}


COO_DEF(TinyOT, void, AND, hptr dst, hptr left, hptr right, uint pos, uint no) {

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

}}

COO_DEF(TinyOT, void, INV, hptr dst, hptr op, uint _) {
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

}}

COO_DEF(TinyOTImpl, void, swapBits, int len) {

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
}}


COO_DEF(TinyOT, void, end_layer_AND, uint width) {
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
}}

COO_DEF(TinyOT, void, begin_layer_public_common_store, uint len) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  
  impl->verify();
 
}}


COO_DEF(TinyOT, void, end_layer_public_common_store, uint len) {
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
}}

COO_DEF(TinyOTImpl, void, alloc_comm_buf, uint size) {
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
}}


COO_DEF(TinyOT, void, public_common_store, byte*s, uint out, uint pos) {
  
  TinyOTImpl impl = (TinyOTImpl)this->impl;

  impl->pcsBuf[pos] = s;
  impl->outBuf[pos] = out;
  impl->buf[pos] = impl->heap[out]->shr;

  impl->myMacs[impl->noToVerify] = impl->heap[out]->mac;
  impl->hisMacsExpected[impl->noToVerify] = impl->heap[out]->key;
  impl->noToVerify++;
  

}}

COO_DEF(TinyOT, void, no_of_ands, uint size) {
  
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

}}

COO_DEF(TinyOT, void, max_width_AND, uint size) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  if (impl->andBuffer==NULL) {
    impl->andBuffer = impl->oe->getmem(sizeof(opbuffer)*size);
    impl->alloc_comm_buf(2*size);
  }
}}


COO_DEF(TinyOT, void, max_width_public_common_store, uint len) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  
  impl->alloc_comm_buf(len);
  impl->pcsBuf = impl->oe->getmem(sizeof(byte*)*len);
  impl->outBuf = impl->oe->getmem(len*sizeof(int));

}}

COO_DEF(TinyOT, void, private_common_load, tinyotshare * in, uint res, uint _) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  if (!in) {
    char m[80] = {0};
    osal_sprintf(m,"private common load, setting address %u to null.\n",res);
    impl->oe->p(m);
  }
  
  impl->heap[res] = tshr_new(impl->oe);
  *(impl->heap[res]) = *in;
}}

COO_DEF(TinyOT, void, public_common_load, byte val, uint res, uint _) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  tinyotshare * ress = tshr_new(impl->oe);
  impl->AliceShare(ress, 0);
  impl->BobShare(ress,val);
  impl->heap[res] = ress;
}}

/*
COO_DEF(TinyOT, void, end_layer_AND, uint width);
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

COO_DEF(TinyOTImpl, void, verify) {
  this->checkMACs(this->noToVerify);
  this->noToVerify = 0;
}}


COO_DEF(TinyOTImpl, void, checkMACs,uint len) {
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
}}



COO_DEF(TinyOT, uint, invite, uint port) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  return impl->arena->listen_wait(1,port).rc;
}}

COO_DEF(TinyOT, uint, connect, char * ip, uint port) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  return impl->arena->connect(ip,port).rc;
}}

COO_DEF(TinyOT, void, dummy, uint w) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  impl->oe->p("");
}}


COO_DEF(TinyOT, bool, isAlice) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  return impl->isAlice;
}}

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

  res->isAlice   = COO_attach(res, TinyOT_isAlice);
  res->invite    = COO_attach(res, TinyOT_invite);
  res->connect   = COO_attach(res, TinyOT_connect);
  res->init_heap = COO_attach(res, TinyOT_init_heap);
  res->heap_get  = COO_attach(res, TinyOT_heap_get);
  res->heap_set  = COO_attach(res, TinyOT_heap_set);
  res->ZERO      = COO_attach(res, TinyOT_ZERO);
  res->ONE       = COO_attach(res, TinyOT_ONE);
  res->XOR       = COO_attach(res, TinyOT_XOR);
  res->AND       = COO_attach(res, TinyOT_AND);
  res->INV       = COO_attach(res, TinyOT_INV);
  res->end_layer_AND = COO_attach(res, TinyOT_end_layer_AND);
  res->begin_layer_public_common_store 
                 = COO_attach(res, TinyOT_begin_layer_public_common_store);
  res->end_layer_public_common_store 
                 = COO_attach(res, TinyOT_end_layer_public_common_store);
  res->public_common_store 
                 = COO_attach(res, TinyOT_public_common_store);
  res->no_of_ands= COO_attach(res, TinyOT_no_of_ands);
  res->max_width_AND 
                 = COO_attach(res, TinyOT_max_width_AND);
  res->max_width_public_common_store 
                 = COO_attach(res, TinyOT_max_width_public_common_store);
  res->private_common_load 
                 =   COO_attach(res, TinyOT_private_common_load);
  res->public_common_load 
                 =   COO_attach(res, TinyOT_public_common_load);
  res->end_layer_INV = COO_attach(res, TinyOT_dummy);
  res->begin_layer_XOR = COO_attach(res, TinyOT_dummy);
  res->end_layer_XOR =  COO_attach(res, TinyOT_dummy);
  res->begin_layer_INV = COO_attach(res, TinyOT_dummy);
  res->begin_layer_AND = COO_attach( res, TinyOT_dummy);
  res->max_width_XOR = COO_attach(res, TinyOT_dummy);
  res->max_width_INV = COO_attach(res, TinyOT_dummy);
  res->max_width_private_common_load = COO_attach(res, TinyOT_dummy);
  res->max_width_public_common_load = COO_attach(res, TinyOT_dummy);
  res->end_layer_private_common_load = COO_attach(res, TinyOT_dummy);
  res->begin_layer_public_common_load = COO_attach(res, TinyOT_dummy);
  res->begin_layer_private_common_load = COO_attach(res, TinyOT_dummy);
  res->end_layer_public_common_load = COO_attach(res, TinyOT_dummy);

  impl->AliceShare = COO_attach(impl, TinyOTImpl_AliceShare);
  impl->BobShare = COO_attach(impl, TinyOTImpl_BobShare);
  impl->swapBits = COO_attach(impl, TinyOTImpl_swapBits);
  impl->verify = COO_attach(impl, TinyOTImpl_verify);
  impl->checkMACs = COO_attach(impl, TinyOTImpl_checkMACs);
  impl->alloc_comm_buf = COO_attach(impl, TinyOTImpl_alloc_comm_buf);
  
  oe->p("************************************************************");
  oe->p("   " PACKAGE_STRING " - " CODENAME );
  oe->p("   " BUILD_TIME);
  oe->p("************************************************************");

  return res;
}

void TinyOT_destroy(TinyOT * instance) {

}
