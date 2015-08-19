#include "encoding/hex.h"
#include <stdio.h>
#include <mutex.h>

static MUTEX _screen_m = 0;

byte h2b(char * hex) {
  byte h = hex[0];
  byte l = hex[1];
  byte res = 0;
  
  if (h >= 'a' && h <= 'f') { res += ((h-'a')+10) << 4; }
  if (h >= 'A' && h <= 'F') { res += ((h-'A')+10) << 4; }
  if (h >= '0' && h <= '9') { res += (h-'0') << 4; }

  if (l >= 'a' && l <= 'f') { res += ((l-'a')+10); }
  if (l >= 'A' && l <= 'F') { res += ((l-'A')+10); }
  if (l >= '0' && l <= '9') { res += (l-'0'); }
  
  return res;
}


void b2h(byte b, char * hex) {
  static char * hexchars = {"0123456789ABCDEFG"};
  byte h = (b & 0xF0) >> 4;
  byte l = (b & 0x0F);
  hex[0] = hexchars[h];
  hex[1] = hexchars[l];
}


 void hs2bs(char * hex, byte * res, uint lres) {
  uint i = 0;
  for(i = 0; i < 2*lres;i+=2) {
    res[i/2] = h2b( hex+i );
  }
}

void bs2hs(byte * bytes, char * res, uint lres) {
  uint i = 0;
  for(i = 0; i < lres/2;i++) {
    b2h( bytes[i], res+(2*i) );
  }
}

static void dump_data_as_text( byte * b, uint lb ) {
  uint i = 0;
  for(i = 0;i <lb;i++) {
    if (b[i] >= 32 && b[i] <= 140) {
      printf("%c",b[i]);
    } else {
      printf(".",b[i]);
    }
  }
}

void dump_data_as_hex(byte * data, uint ldata, uint width) {
  uint off = 0, i = 0;
  for(off = 0; off < ldata; ++off){

    if (off > 0 && off % width == 0) {
      printf(" | ");
      dump_data_as_text( data + (off - width), width );
      printf(" |\n");
    }
    
    if (off % width == 0) {
      printf("[0x%04x] ",off);
    }

    printf("%2x ", data[off]);
  }

  if (ldata%width != 0) { // there is a rest
    for(i=0;i<width-(ldata % width);++i) { printf("   "); }
    i = ldata % width;
    printf(" | ");
    dump_data_as_text( data + (off-i),i);
    for(i = 0;i<width-(ldata % width);++i) { 
      printf(" ");
    }
    printf(" |\n");
  } else {
    printf(" | ");
    dump_data_as_text( data + (off-width),width);
    printf(" |\n");
  }
}

void _p(const char * s,byte * d, uint ld, uint w) {
  if (!_screen_m) {
     _screen_m = Mutex_new(0);
  }
  
  Mutex_lock ( _screen_m );
  printf(" - - - [%s] - - -\n",s);
  dump_data_as_hex(d,ld,w);  
  printf(" - - - - - - - - \n");
  Mutex_unlock(_screen_m);
}
