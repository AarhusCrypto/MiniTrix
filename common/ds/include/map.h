#ifndef MAP_H
#define MAP_H

#include <osal.h> 
#include <list.h>

typedef struct _map_entry_ {
  OE oe;
  void * key;
  void * elm;
} *MapEntry;

MapEntry MapEntry_new(OE oe, void * key, void * elm);
void MapEntry_destroy(MapEntry * entry);

typedef struct _map_ {

  /*!
   * Return the number of elements in this map.
   */
  uint (*size)(void);
  /*
   * Add key value pair to hash map.
   */
  void (*put)(void * key, void * elm);
  
  /*
   * Acquire element stored for the given key, {key}.
   *
   * (void*)0 if no element is filed under {key}.
   */
  void * (*get)(void * key);

  /*
   * True if key is in this map.
   */
  bool (*contains)(void * key);

  /*
   * The map does not contain any key, value pair with key after
   * invoking {rem}.
   */
  void * (*rem)(void * key);

  /*!
   *
   * Return the keys in this map.
   */
  List (*get_keys)(void);
  void * impl;
} * Map;

#endif
