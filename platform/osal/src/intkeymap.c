#include <intkeymap.h>
#include <coov4.h>
#include <singlelinkedlist.h>

List SingleLinkedList_new(OE oe);
#define INT_KEY_MAP_BUCKETS 17
typedef struct _int_key_map_impl_ {
	List buckets[INT_KEY_MAP_BUCKETS];
	OE oe;
} *IntKeyMapImpl;

typedef struct _int_key_map_impl_entry_ {
	uint key;
	void * elm;
} *IkmiEnt;

static IkmiEnt IkmiEnt_New(OE oe, uint key, void * elm) {
	IkmiEnt res = (IkmiEnt)oe->getmem(sizeof(*res));
	if (!res) return 0;

	res->key = key;
	res->elm = elm;

	return res;
}

static void IkmiEnt_Destroy(OE oe, IkmiEnt * ent) {
	IkmiEnt e = 0;
	
	if (!ent) return;

	if (!*ent) return;

	e = *ent;
	e->key = 0;
	e->elm = 0;
	oe->putmem(e);
	*ent = 0;
}

static uint int_hash_fn(uint key) {
	return (101 * key + 65537) % INT_KEY_MAP_BUCKETS;
}

static bool int_compare_fn(uint key1, uint key2) {
	return ((key1 == key2) ? True : False);
}

COO_DEF(IntKeyMap,void,put,uint key, void * elm) 
	IntKeyMapImpl impl = (IntKeyMapImpl)this->impl;
    uint bucket_idx = int_hash_fn(key);
	List bucket = impl->buckets[bucket_idx];
	uint i = 0;

	for (i = 0; i < bucket->size(); ++i) {
		IkmiEnt cur = (IkmiEnt)bucket->get_element(i);
		if (int_compare_fn(key, cur->key) == True) return;
	}
    
	bucket->add_element(IkmiEnt_New(impl->oe, key, elm));
}

COO_DEF(IntKeyMap, void*, get, uint key)
IntKeyMapImpl impl = (IntKeyMapImpl)this->impl;
uint bucket_idx = int_hash_fn(key);
List bucket = impl->buckets[bucket_idx];
uint i = 0;

for (i = 0; i < bucket->size(); ++i) {
	IkmiEnt cur = (IkmiEnt)bucket->get_element(i);
	if (int_compare_fn(key, cur->key) == True) return cur->elm;
}
return 0;
}

COO_DEF(IntKeyMap,void *, rem,uint key) 
	IntKeyMapImpl impl = (IntKeyMapImpl)this->impl;
	uint bucket_idx = int_hash_fn(key);
	List bucket = impl->buckets[bucket_idx];
	uint i = 0;
	for (i = 0; i < bucket->size(); ++i) {
		IkmiEnt cur = (IkmiEnt)bucket->get_element(i);
		if (int_compare_fn(key, cur->key)) {
			void * res = cur->elm;
			bucket->rem_element(i);
			IkmiEnt_Destroy(impl->oe, &cur);
			return res;
		}
	}
	return 0;
}

COO_DEF(IntKeyMap,List,keys)
	IntKeyMapImpl impl = (IntKeyMapImpl)this->impl;
	List result = SingleLinkedList_new(impl->oe);
	uint i = 0, b=0;
	for (b = 0; b < INT_KEY_MAP_BUCKETS; ++b) {
		List bucket = impl->buckets[b];
		for (i = 0; i < bucket->size(); ++i) {
			IkmiEnt cur = (IkmiEnt)bucket->get_element(i);
			result->add_element((void*)(ull) cur->key);
		}
	}
	return result;
}

void IntKeyMap_Destroy(IntKeyMap * map) {
	IntKeyMap m = 0;
	IntKeyMapImpl mi = 0;
	uint b = 0, i = 0;
	OE oe = 0;

	if (!map) return;

	m = *map;
	if (!m) return;

	mi = m->impl;
	if (!mi) return;

	oe = mi->oe;
	for (b = 0; b < INT_KEY_MAP_BUCKETS; ++b) {
		List bucket = mi->buckets[b];
		for (i = 0; i < bucket->size(); ++i) {
			IkmiEnt cur = (IkmiEnt)bucket->get_element(i);
			IkmiEnt_Destroy(oe,&cur);
		}
		SingleLinkedList_destroy(&bucket);
		mi->buckets[b] = 0;
	}

	oe->putmem(mi);
	oe->putmem(m);
}

COO_DEF(IntKeyMap, bool, contains, ull key)
return this->get(key) != 0;
}

COO_DEF(IntKeyMap, uint, size)
	IntKeyMapImpl impl = (IntKeyMapImpl)this->impl;
uint i = 0, b = 0, r = 0;
for (b = 0; b < INT_KEY_MAP_BUCKETS; ++b) {
	List bucket = impl->buckets[b];
	r += bucket->size();
}
return r;
}

IntKeyMap IntKeyMap_New(OE oe) {
	IntKeyMap result = (IntKeyMap)oe->getmem(sizeof(*result));
	IntKeyMapImpl impl = 0;
	uint i = 0;
	if (!result) return 0;

	impl = (IntKeyMapImpl)oe->getmem(sizeof(*impl));
	if (!impl) goto error;

	impl->oe = oe;
	for (i = 0; i < INT_KEY_MAP_BUCKETS; ++i) {
		impl->buckets[i] = SingleLinkedList_new(oe);
	}

	result->impl = impl;

	result->put = COO_attach(result, IntKeyMap_put);
	result->get = COO_attach(result, IntKeyMap_get);
	result->rem = COO_attach(result, IntKeyMap_rem);
	result->keys = COO_attach(result, IntKeyMap_keys);
	result->contains = COO_attach(result, IntKeyMap_contains);
	result->size = COO_attach(result, IntKeyMap_size);

	return result;
error:
	IntKeyMap_Destroy(&result);
	return 0;
}

