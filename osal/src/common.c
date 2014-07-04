#include "common.h"
#include "osal.h"
#include <stdio.h>
#include <stdarg.h>

bool True = 1;
bool False = 0;

void l2b(unsigned long long l, byte * out) {
  out[0] = (l & 0x00000000000000FFL);
  out[1] = (l & 0x000000000000FF00L) >> 8;
  out[2] = (l & 0x0000000000FF0000L) >> 16;
  out[3] = (l & 0x00000000FF000000L) >> 24;
  out[4] = (l & 0x000000FF00000000L) >> 32;
  out[5] = (l & 0x0000FF0000000000L) >> 40;
  out[6] = (l & 0x00FF000000000000L) >> 48;
  out[7] = (l & 0xFF00000000000000L) >> 56;
}


unsigned long long b2l(byte * in) {
  unsigned long long res = 0;
  res += in[0];
  res += in[1] << 8;
  res += in[2] << 16;
  res += in[3] << 24;
  res += ((unsigned long long)in[4]) << 32;
  res += ((unsigned long long)in[5]) << 40;
  res += ((unsigned long long)in[6]) << 48;
  res += ((unsigned long long)in[7]) << 56;
  return res;
}


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

int mcmp(void * a, void * b, uint l) {
  byte * ba = (byte*)a;
  byte * bb = (byte*)b;
  int i = 0;
  while(i < l) {
    if (ba[i] > bb[i]) return 1;
    if (ba[i] < bb[i]) return -1;
    ++i;
  }
  return 0;
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
