#include "map.h"

MapEntry MapEntry_new(OE oe, void * key, void * elm) {
  MapEntry ent = (MapEntry)oe->getmem(sizeof(*ent));
  if (!ent) return 0;

  if (!oe) return 0;

  ent->key = key;
  ent->elm = elm;
  ent->oe = oe;

  return ent;
}


void MapEntry_destroy(MapEntry * entry) {
  OE oe = 0;
  if (!entry) return;
  if (!*entry) return;
  oe = (*entry)->oe;
  oe->putmem(*entry);
}

