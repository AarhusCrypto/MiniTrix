#include "hashmap.h"
#include <singlelinkedlist.h>
#include <coov4.h>

List SingleLinkedList_new(OE oe);
typedef struct _hash_map_ {
  List * buckets;
  uint lbuckets;
  HashFN hfn;
  CompareFN cfn;
  OE oe;
} * HashMap;

COO_DEF(Map, void, put, void * key, void * elm)
  uint hash_code = 0;
  HashMap hmap = (HashMap)this->impl;
  uint bucket = 0;
  MapEntry ent = 0;
  uint i = 0;

  if (!elm) return ;
  
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
      if ( hmap->cfn(e->key,key) == 0) {
        e->elm = elm;
      }
    }
  }
  hmap->buckets[bucket]->add_element(ent);
  return;
 failure:
  MapEntry_destroy ( &ent );
}

COO_DEF(Map, void *, get, void * key)
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
}

COO_DEF(Map,bool, contains, void * key)
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
}

COO_DEF(Map, int, size )
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
}

COO_DEF(Map, List, get_keys)
  HashMap hmap = (HashMap)this->impl;
  uint bi = 0;
  List res = SingleLinkedList_new(hmap->oe);
  for(bi = 0; bi < hmap->lbuckets;++bi) {
    if (hmap->buckets[bi]) {
      List bucket = hmap->buckets[bi];
      uint ei = 0;
      for(ei = 0; ei < bucket->size();++ei) {
        MapEntry e = (MapEntry)bucket->get_element(ei);
        res->add_element(e->key);
      }
    }
  }
  return res;
}

COO_DEF(Map, void *, rem, void * key) 
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
}

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
    map->put = COO_attach(map, Map_put);
    map->get = COO_attach(map, Map_get);
    map->contains = COO_attach(map, Map_contains);
    map->size = COO_attach(map, Map_size);
    map->rem = COO_attach(map, Map_rem);
    map->get_keys = COO_attach(map, Map_get_keys);

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
      int k = 0;
      for(k = 0;k < hmap->buckets[i]->size();++k) {
        MapEntry ent = (MapEntry)hmap->buckets[i]->get_element(k);
        if (ent) {
          MapEntry_destroy(&ent);
        }
      }
      SingleLinkedList_destroy ( &hmap->buckets[i] ); 
    }
  }
  oe->putmem(hmap->buckets);
  hmap->buckets = 0;
  hmap->lbuckets = 0;

  oe->putmem(hmap);
  (*map)->impl = 0;
  COO_detach((*map)->get);
  COO_detach((*map)->put);
  COO_detach((*map)->contains);
  COO_detach((*map)->rem);
  oe->putmem(*map);
  *map = 0;
}

static uint int_hash_fn(void * _key) {
  uint key = (uint)(ull)_key;
  return (101 * key + 65537);
}

static int int_compare_fn(void * _key1, void * _key2) {
  uint key1 = (uint)(ull)_key1;
  uint key2 = (uint)(ull)_key2;  
  return ((key1 == key2) ? 0 :key1 > key2 ? 1 : -1);
}


Map HashMap_IntKey_New(OE oe, uint buckets) {
  Map result = 0;
  result = HashMap_new(oe,int_hash_fn, int_compare_fn, buckets);
  return result;
}


static int cstr_compare_fn(void * a, void * b) {
	char * as = (const char *)a;
	char * bs = (const char *)b;
	uint las = 0;
	uint lbs = 0;
	uint i = 0;
	while(as[las++]);
	while(bs[lbs++]);

	if (las > lbs) return 1;
	if (las < lbs) return -1;

	while( (i < las) && (as[i] == bs[i])) ++i;

	if (i == las) return 0;
	if (as[i] > bs[i]) return 1;
	else return -1;
}

static uint cstr_hash_fn(void * a) {
	const char * as = a;
	uint i = 0;
	uint hash = 65537;
	while(as[i]) {
		hash += 101*as[i];
		++i;
	}

	return hash;
}

Map HashMap_StrKey_New(OE oe,uint buckets) {
	Map result = 0;
	result = HashMap_new(oe,cstr_hash_fn,cstr_compare_fn,buckets);
	return result;
}
