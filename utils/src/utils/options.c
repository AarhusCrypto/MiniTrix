#include <utils/options.h>
#include <hashmap.h>
#include <osal.h>
#include <stdio.h>

inline static int
slen(char * s) {
  uint i = 0;
  while(s[i]) ++i;
  return i;
}

static
uint hash_fn(void * a) {
  uint j = 0, res=524287;
  char * s = (char*)a;
  uint ls = slen(s); 
  for(j = 0;j < ls;++j) {
    res += 2147483647*s[j];
  }
  return res;
}

inline static 
int compare_fn(void * a, void * b) {
  char * sa = (char*)a;
  char * sb = (char*)b;
  int lsa = slen(sa);
  int lsb = slen(sb);
  int lmin= lsa > lsb ? lsb : lsa;
  uint j = 0;
  for(j = 0; j < lmin;++j) {
    if (sa[j] < sb[j]) return -1;
    if (sa[j] > sb[j] ) return 1;
  }

  if (lsa < lsb) return -1;
  if (lsa > lsb) return 1;

  return 0;
}

Map Options_New(OE oe, int argc, char **args) {
  int i = 0;
  Map res = HashMap_new(oe, hash_fn, compare_fn, 8);
  for(i = 0;i < argc;++i) {
    if (args[i][0] == '-') {
      char * key = args[i]+1;
      char * val = "";
      if (i+1 < argc) {
        if (args[i+1][0] != '-') {
          val = args[i+1];
          ++i;
        }
      }
      res->put(key,val);
    }
  }
  return res;
}

void Options_Destroy(Map * m) {
  HashMap_destroy(m);
}
