#include "common.h"
#include "osal.h"
#include <stdio.h>
#include <stdarg.h>

bool True = 1;
bool False = 0;

void zeromem(void * m, uint lm) {
  uint i = 0;
  byte*dm = (byte*)m;
  for(i = 0;i<lm;++i) {
    dm[i] = 0;
  }
}

void mcpy(void * d, void * s, uint l) {
  byte * dst = (byte*)d;
  byte * src = (byte*)s;
  uint i = 0;
  for(i=0;i<l;++i) dst[i]=src[i];
}

int osal_strlen(const char * s) {
  int i = 0;
  if (!s) return 0;
  while(s[i]) ++i;
  return i;
}


int osal_sprintf(char * b, const char * fmt, ... ) {
  va_list l = {{0}};
  int res = 0;
  va_start(l,fmt);
  res = vsprintf(b, fmt, l);
  va_end(l);
  return res;
}
