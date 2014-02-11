#include "hashmap.h"
#include <singlelinkedlist.h>
#include <coo.h>

typedef struct _hash_map_ {
  List * buckets;
  uint lbuckets;
  HashFN hfn;
  CompareFN cfn;
  OE oe;
} * HashMap;

COO_DCL(Map, void, put, void * key, void * elm)
COO_DEF_NORET_ARGS(Map, put, void * key; void * elm;,key,elm) {
  uint hash_code = 0;
  HashMap hmap = (HashMap)this->impl;
  uint bucket = 0;
  MapEntry ent = 0;
  uint i = 0;

  if (!elm) return;

  ent = MapEntry_new(hmap->oe, key, elm);

  hash_code = hmap->hfn(key);
  bucket = hash_code % hmap->lbuckets;
  if (!hmap->buckets[bucket]) {
    hmap->buckets[bucket] = SingleLinkedList_new(hmap->oe);
    if (!hmap->buckets[bucket]) goto failure;
  }
  for(i = 0;i < hmap->buckets[bucket]->size(); ++i) {
    MapEntry e = hmap->buckets[bucket]->get_element(i);
    if (ent) {
      if ( hmap->cfn(e->key,key) == 0) goto failure;
    }
  }
  hmap->buckets[bucket]->add_element(ent);
  return;
 failure:
  MapEntry_destroy ( &ent );
}}

COO_DCL(Map, void *, get, void * key) 
COO_DEF_RET_ARGS(Map, void *, get, void * key;,key) {
  HashMap hmap = (HashMap)this->impl;
  uint hash_code = 0, bucket = 0, i =0;
  
  hash_code = hmap->hfn(key);
  bucket = hash_code % hmap->lbuckets;
  
  if (hmap->buckets[bucket] == 0) return 0;

  for( i = 0 ; i < hmap->buckets[bucket]->size(); ++i) {
    MapEntry ent = hmap->buckets[bucket]->get_element(i);
    if (hmap->cfn(ent->key,key) == 0) { 
      return ent->elm;
    }
  }
  return 0;
}}

COO_DCL(Map, bool, contains, void * key)
COO_DEF_RET_ARGS(Map, bool, contains, void * key;,key) {
  HashMap hmap = (HashMap)this->impl;
  uint hash_code = 0;
  uint bucket = 0, i = 0;

  hash_code = hmap->hfn(key);
  bucket = hash_code % hmap->lbuckets;
  
  if (hmap->buckets[bucket] == 0) return False;
  for(i = 0;i < hmap->buckets[bucket]->size();++i) {
    MapEntry ent = (MapEntry)hmap->buckets[bucket]->get_element(i);
    if (ent) {
      if (hmap->cfn(ent->key, key) == 0) return True;
    }
  }
  
  return False;
}}

COO_DCL(Map, uint, size )
COO_DEF_RET_NOARGS(Map, uint, size) {
  HashMap hmap = (HashMap)this->impl;
  uint res = 0;
  uint bucket = 0;
  for(bucket =0;bucket < hmap->lbuckets;++bucket) {
    List b = hmap->buckets[bucket];
    if (b) {
      res += b->size();
    }
  }
  return res;
}}

COO_DCL(Map, void *, rem, void * key) 
COO_DEF_RET_ARGS(Map, void *, rem, void * key;,key) {
  HashMap hmap = (HashMap)this->impl;
  uint hash_code=0, bucket=0,i=0;
  hash_code = hmap->hfn(key);
  bucket = hash_code % hmap->lbuckets;
  
  if (hmap->buckets[bucket] == 0) return 0;

  for(i = 0;i < hmap->buckets[bucket]->size();++i) {
    MapEntry ent = (MapEntry)hmap->buckets[bucket]->get_element(i);
    if (ent) {
      if (hmap->cfn(ent->key, key) == 0) {
	hmap->buckets[bucket]->rem_element(i);
	void * elm = ent->elm;
	MapEntry_destroy(&ent);
	return elm;
      }
    }
  }
  return 0;
}}

Map HashMap_new(OE oe, HashFN hfn, CompareFN cfn, uint buckets ) {
  Map map = 0;
  HashMap hmap = 0;
  if (!oe) return 0;
  if (!hfn) return 0;
  if (!cfn) return 0;

  {
    uint i = 0;
    map = (Map)oe->getmem(sizeof(*map));
    hmap = (HashMap)oe->getmem(sizeof(*hmap));

    if (!hmap) goto failure;
    if (!map) goto failure;

    
    map->impl = hmap;
    COO_ATTACH(Map, map, put);
    COO_ATTACH(Map, map, get);
    COO_ATTACH(Map, map, contains);
    COO_ATTACH(Map, map, size);
    COO_ATTACH(Map, map, rem);

    hmap->cfn = cfn;
    hmap->hfn = hfn;
    hmap->buckets = oe->getmem(sizeof(List)*buckets);
    if (!buckets) goto failure;
    hmap->lbuckets = buckets;
    hmap->oe = oe;
    if (!hmap->buckets) goto failure;
    for(i = 0;i<buckets;++i) {
      hmap->buckets[i] = SingleLinkedList_new(oe);
      if (!hmap->buckets[i]) goto failure;
    }
    
  }
  return map;
 failure:
  HashMap_destroy( &map );
  return 0;
}

void HashMap_destroy( Map * map ) {
  HashMap hmap = 0;
  OE oe = 0;
  uint i =0;
  if (!map) return;
  if (!*map) return;

  hmap = (HashMap)(*map)->impl;
  oe = hmap->oe;

  for(i = 0;i < hmap->lbuckets;++i) {
    if (hmap->buckets[i]) {
      SingleLinkedList_destroy ( &hmap->buckets[i] ); 
    }
  }
  oe->putmem(hmap->buckets);
  hmap->buckets = 0;
  hmap->lbuckets = 0;

  oe->putmem(hmap);
  (*map)->impl = 0;
  COO_DETACH((*map), get);
  COO_DETACH((*map), put);
  COO_DETACH((*map), contains);
  COO_DETACH((*map), rem);
  oe->putmem(*map);
  *map = 0;
}
