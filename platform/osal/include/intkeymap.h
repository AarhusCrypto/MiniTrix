#ifndef INT_KEY_MAP_H
#define INT_KEY_MAP_H
#include <list.h>
  // ----------------------------------------
  // Hardcoded int key map
  // ----------------------------------------
  typedef struct _int_key_map_ {
	  /*
	  * Get a list with all keys in the map
	  */
	  List(*keys)();

	  /*
	  * Insert {value} into the map under {key}
	  */
	  void(*put)(ull key, void * value);

	  /*
	  * Get element inserted under {key}
	  */
	  void * (*get)(ull key);

	  /*
	  * Remove the element stored under {key} and result the element.
	  */
	  void * (*rem)(ull key);

	  /*
	   * Return the size of this guy
	   */
	  uint(*size)();

	  /*
	   * Returns True of the map constains an entry with the given key.
	   */
	  bool(*contains)(ull key);

	  void * impl;
  } *IntKeyMap;
#endif
