/*!
 * TinyOT 
 *
 * Based on original work by Jesper Buus Nielsen jbn@cs.au.dk.
 *
 *
 */
#ifndef TINYOT_H
#define TiNYOT_H
#include <common.h>
#include <osal.h>

typedef uint hptr;

typedef struct {
  unsigned char shr;  // My share of the shared bit, a bit.
  unsigned long mac;  // My mac on my share,
  unsigned long key;  // My key under which his share is mac'ed
} tinyotshare;

typedef struct {
  tinyotshare a;
  tinyotshare b;
  tinyotshare ab;
} tinyotand;

typedef struct {
  tinyotshare X;
  tinyotshare Y;
  tinyotshare *left;
  tinyotshare *right;
  tinyotshare *res;
  tinyotshare *ab;
  tinyotshare *a;
} opbuffer;

typedef struct _tinyot_ {
  
  void (*init_heap)(uint size);

  tinyotshare * (*heap_get)(hptr addr);

  void (*heap_set)(hptr addr, tinyotshare * share);

  void (*ZERO)(hptr addr);
  
  void (*ONE)(hptr addr);
  
  void (*XOR)(hptr res, hptr left, hptr right);

  void (*AND)(hptr res, hptr left, hptr right, uint pos, uint no);

  void (*end_layer_and)(uint width);

  //
  void (*begin_layer_public_common_store)(uint len);
  //
  void (*end_layer_public_common_store)(uint len);
  //
  void (*public_common_store)(byte * s, uint out, uint pos);
  //
  void (*no_of_ands)(uint size);
  //
  void (*max_width_AND)(uint size);
  //
  void (*max_width_public_common_store)(uint len);
  //
  void (*private_common_load)(tinotshare * in, uint res, uint _);
  // 
  void (*public_common_load)(byte val, uint res, uint _);
  //
  void (*end_layer_AND)(uint width);

  

  void (*INV)(hptr res, hptr op);

  void * impl;
  
} *TinyOT;

TinyOT TinyOT_new(OE oe, bool isAlice);
void TinyOT_destroy(TinyOT * instance);

#endif
