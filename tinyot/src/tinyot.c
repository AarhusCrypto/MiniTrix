/*
 * TinyOT Implementation based on work by Jesper Buus Nielsen.
 *
 */
#include <tinyot.h>
#include <coo.h>
#include <osal.h>
#include <carena.h>

/*
 * UTILITIES FROM tinyot.cpp
 */

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

  CArena arena;
  OE oe;

} * TinyOTImpl;

COO_DCL(TinyOT, void, init_heap, uint size);
COO_DEF_NORET_ARGS(TinyOT, init_heap, uint size;, size) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  impl->heap = (tinyotshare**)impl->oe->getmem(sizeof(*impl->heap)*size);
  if (impl->heap != 0)
    impl->sizeHeap = size;
}}

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
}}

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
}}


COO_DCL(TinyOT, void, ZERO, hptr addr);
COO_DEF_NORET_ARGS(TinyOT, ZERO, hptr addr;,addr) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;

  if (!impl->heap[addr]) {
    char m[64] = {0};
    osal_sprintf(m,"Trying to zero out an unset address %u.", addr);
    impl->oe->p(m);
    return;
  }

  impl->AliceShare(impl->heap[addr],0);
  impl->BobShare(impl->heap[addr],0);
}}


COO_DCL(TinyOTImpl, void, AliceShare, tinyotshare * res, hptr addr, char s);
COO_DEF_NORET_ARGS(TinyOTImpl, AliceShare, tinyotshare* res; hptr addr; char s;,res,addr,s) {
  
  if (!res) return;

  if (this->isAlice) {
    res->shr = s;
    res->mac = 0;
  } else {
    res->key = s*this->delta;
  }
}}

COO_DCL(TinyOTImpl, void, BobShare, tinyotshare * res, hptr addr, char s);
COO_DEF_NORET_ARGS(TinyOTImpl, BobShare, tinyotshare * res; hptr addr; char s;,res,addr,s) {

  if (!res) return;

  if (!this->isAlice) {
    res->shr = s;
    res->mac = 0;
  } else {
    res->key = s*this->delta;
  }
}}

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
}}


static 
void _XOR(tinyotshare * res, tinyotshare * left, tinyotshare * right) {
  res->shr = left->shr ^ right->shr;
  res->mac = left->mac ^ right->mac;
  res->key = left->key ^ right->key;
}

inline
tinyotshare * tshr_new(OE oe) {
  tinyotshare * res = (tinyotshare*)oe->getmem(sizeof(*res));
  return res;
}

COO_DCL(TinyOT, void, XOR, hptr dst, hptr left, hptr right);
COO_DEF_NORET_ARGS(TinyOT, XOR, hptr dst; hptr left; hptr right;, dst, left, right) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  tinyotshare * res;

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


COO_DCL(TinyOT, void, AND, hptr dst, hptr left, hptr right, uint pos, uint no);
COO_DEF_NORET_ARGS(TinyOT, AND, hptr dst; hptr left; hptr right; uint pos; uint no;
                   , dst, left, right, pos, no) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;

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

    impl->andBuffer[pos].left = impl->heap[left];
    impl->andBuffer[pos].right = impl->heap[right];
    impl->andBuffer[pos].res = impl->heap[dst];
    impl->andBuffer[pos].a = &pre->a;
    impl->andBuffer[pos].ab = &pre->ab;
  }
  // TODO(rwl): The end_and_layer is needed
}}

COO_DCL(TinyOT, void, INV, hptr dst, hptr op);
COO_DEF_NORET_ARGS(TinyOT, INV, hptr dst; hptr op;,dst,op) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  
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


  if (impl->isAlice) {
    impl->heap[dst]->shr = impl->heap[op]->shr;
    impl->heap[dst]->mac = impl->heap[op]->mac;
    impl->heap[dst]->key = impl->heap[op]->key ^ impl->delta;
  } else {
    impl->heap[dst]->shr = impl->heap[op]->shr ^ 1;
    impl->heap[dst]->mac = impl->heap[op]->mac;
    impl->heap[dst]->key = impl->heap[op]->key;
  }

}}

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
    receiveBits(peer, this->buf, this->wordBuf, len);
    sendBits(peer, this->buf, this->wordBuf, len);
    char * tmp = this->tmpBuf;
    this->tmpBuf = this->buf;
    this->buf = tmp;
  }
}}


COO_DCL(TinyOT, void, end_layer_and, uint width);
COO_DEF_NORET_ARGS(TinyOT, end_layer_and, uint width;, width) {
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

COO_DCL(TinyOT, void, begin_layer_public_common_store, uint len);
COO_DEF_NORET_ARGS(TinyOT, begin_layer_public_common_store, uint len;, len) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  
  impl->verify();
  
  
}}


COO_DCL(TinyOT, void, end_layer_public_common_store, uint len);
COO_DEF_NORET_ARGS(TinyOT, end_layer_public_common_store, uint len;, len) {
  TinyOTImpl impl = (TinyOTImpl)this->impl;
  int pos = 0;
  swapBits(len);
  
  for(pos = 0; pos < len;++pos) {
    hisMacsExpected[impl->noToVerify-len+pos]=hisMacsExpected[noToVerify-len+pos] ^
      buf[pos] * impl->delta;
    
  }

  impl->verify();

  for(pos = 0;pos < len;++pos) {
    *impl->pcsBuf[pos] = impl->heap[impl->outBuf[pos]]->shr ^ (impl->buf[pos]);
  }
}}


COO_DCL(TinyOT, void, public_common_store, byte*s, uint out, uint pos);
COO_DEF_NORET_ARGS(TinyOT, public_common_store, byte * s; uint out; uint pos;,
                   s, out, pos) {

  

}}

COO_DCL(TinyOT, void, no_of_ands, uint size);
COO_DEF_NORET_ARGS(TinyOT, no_of_ands, uint size;, size) {
  

}}

COO_DCL(TinyOT, void, max_with_AND, uint size);
COO_DEF_NORET_ARGS(TinyOT, max_width_AND, uint size;, size) {

  
}}


COO_DCL(TinyOT, void, max_width_public_common_store, uint len);
COO_DEF_NORET_ARGS(TinyOT, max_width_public_common_store, uint len;, len) {
  
}}

COO_DCL(TinyOT, void, private_common_load, tinyotshare * in, uint res, uint _);
COO_DEF_NORET_ARGS(TinyOT, private_common_load, 
                   tinyotshare * in; uint res; uint _;, in, res, _) {


}}

COO_DCL(TinyOT, void, public_common_load, byte val, uint res, uint _);
COO_DEF_NORET_ARGS(TinyOT, void, public_common_load, 
                   byte val; uint res; uint _;, val, res, _) {

}}

COO_DCL(TinyOT, void, end_layer_AND, uint width);
COO_DEF_NORET_ARGS(TinyOT, void, end_layer_AND, uint width;, width) {

}}

TinyOT TinyOT_new(OE oe, bool isAlice) {
  TinyOT res = (TinyOT)oe->getmem(sizeof(*res));
  TinyOTImpl impl = 0;
  if (!res) return 0;

  impl = (TinyOTImpl)oe->getmem(sizeof(*res));
  if (!impl) return 0;

  COO_ATTACH(TinyOT, res, init_heap);
  COO_ATTACH(TinyOT, res, heap_get);
  COO_ATTACH(TinyOT, res, heap_set);
  COO_ATTACH(TinyOT, res, ZERO);
  COO_ATTACH(TinyOT, res, ONE);
  COO_ATTACH(TinyOT, res, XOR);
  COO_ATTACH(TinyOT, res, AND);
  COO_ATTACH(TinyOT, res, INV);
  COO_ATTACH(TinyOT, res, end_layer_and);
  COO_ATTACH(TinyOT, res, begin_layer_public_common_store);
  COO_ATTACH(TinyOT, res, end_layer_public_common_store);
  COO_ATTACH(TinyOT, res, public_common_store);
  COO_ATTACH(TinyOT, res, no_of_ands);
  COO_ATTACH(TinyOT, res, max_width_AND);
  COO_ATTACH(TinyOT, res, max_width_public_common_store);
  COO_ATTACH(TinyOT, res, private_common_load);
  COO_ATTACH(TinyOT, res, public_common_load);
  COO_ATTACH(TinyOT, res, end_layer_AND);

  COO_ATTACH(TinyOTImpl, impl, AliceShare);
  COO_ATTACH(TinyOTImpl, impl, BobShare);
  COO_ATTACH(TinyOTImpl, impl, swapBits);
  
  return res;
}

void TinyOT_destroy(TinyOT * instance) {

}
