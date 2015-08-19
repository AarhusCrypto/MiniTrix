#include "utf8string.h"
#include "coov3.h"

#include <string.h> 

typedef struct _utf8string_ {
  Data content;
  OE oe;
} * UTF8Str;


// 1 1 1 1  
// F        
// 1 1 1 0
// E
// 1 1 0 0
// C
// 1 0 0 0
// 8

static uint code_point_length( byte * s ) {
  byte len = 0;
  if (!s) return 0;

  len = *s;
  
  if ( (len & 0xF0) == 0xF0) return 4;
  if ( (len & 0xF0) == 0xE0) return 3;
  if ( (len & 0xF0) == 0xC0) return 2;
  if (len < 0x80) return 1;
  return 0;
}

static void utf8_value_code_point(uint cp, byte * s) {

  if (cp < 0x80) {
    s[0] = (byte)cp;
    return;
  }

  if (cp > 0x7F && cp < 0x800) {
    // 0001 0010 0011 0100 0101 0110 0111 1000 
    // bits 7-11 both incl.
    // 0000 0|000 00 00 0000
    //        7   C  3  F
    s[0] = 0xC0 + ((cp & 0x000007C0) >> 6);
    s[1] = 0x80 + (cp & 0x0000003F);
    return; 
  }

  if (cp > 0x07FF && cp < 0x10000) {
    // C=1100 D=1101 E=1110 F=1111
    //
    // 0000 0000...0|0000 0000 0000 0000
    //
    s[0] = 0xE0 + ((cp & 0xF000) >> 12);
    s[1] = 0x80 + ((cp & 0x0FC0) >> 6);
    s[2] = 0x80 + ((cp & 0x003F));
    return;
  }

  if (cp > 0xFFFF && cp < 0x1FFFFF) {
    // 0000 0000 0000 0000 0000 0000 0000 0000
    // 0    0       1 F    F    F    F    F
    s[0] = 0xF0 + ((cp & 0x001C0000) >> 18);
    s[1] = 0x80 + ((cp & 0x0003F000) >> 12);
    s[2] = 0x80 + ((cp & 0x00000FC0) >> 6);
    s[3] = 0x80 + ((cp & 0x0000003F));
    return;
  }

  return;
}

static uint utf8_code_point_value(byte * s, uint ls) {
  switch(code_point_length(s)) {
  case 1: 
    return s[0];
  case 2:
    /*
     * 110xxxxx 10xxxxxx
     * 00011111 00111111
     *
     * 00011111 = 1+2+4+8+16 = 15+16 = 31 hex(31) = 0x1F
     * 00111111 = 1+2+4+8+16+32 = 31+32 = 63 = 0x3F
     */
    return (((s[0] & 0x1F) << 6) + ((s[1] & 0x3F)));
  case 3:
    /*
     * 1110xxxx 10xxxxxx 10xxxxxx
     * 00001111 00111111 00111111
     * = 0x0F   = 0x3F   = 0x3F
     *
     * 
     */
    return ((s[0] & 0x0F) << 12)+((s[1] & 0x3F) << 6) + ((s[2] & 0x3F));
  case 4:
    /*
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     * = 0x07   = 0x3F   = 0x3F   = 0x3F
     *
     */
    return ((s[0] & 0x07) << 18) + ((s[1] & 0x3F) << 12) + ((s[2] & 0x3F) << 6)
      + ((s[3] & 0x3F));
  default:
    return 0;
  }
}

/* If {s} points to an UTF8 string this function returns a pointer
 * into {s} where the next UTF8-code-point starts. If {s} does not
 * point to a valid UTF8 code point, 0 is returned.
 *
 */
static byte * next_code_point_boundary(byte * s, uint ls) {
  uint i = 0;
  if(s) {
    if (s[0] < 0x80) return s+1;
    if ( (s[0] & 0xE0) == 0xC0 ||
	 (s[0] & 0xF0) == 0xE0 || 
         (s[0] & 0xF8) == 0xF0) ++i; 
    while((s[i] & 0xC0) == 0x80 && i < ls) ++i;
    return s + i;
  }
  return 0;
}

/* Returns True if {s} contains a valid UTF8 encoded string.
 *
 */
static bool utf8_is_valid(byte * s, uint ls) {
  uint i = 0;

  while(i < ls) {
    
    // is it an ASCII character? (<128)
    if (s[i] < 0x80) {
      ++i; continue; 
    }

    // is it a two byte UTF8
    if ((s[i] & 0xE0) == 0xC0) {
      if (ls < i+1) return False;
      if ((s[i+1] & 0xC0) != 0x80) return False;
      i+=2; continue;
    }

    // is it a three byte UTF8
    if ((s[i] & 0xF0) == 0xE0) {
      if (ls < i+2) return False;
      if ((s[i+1] & 0xC0) != 0x80) return False;
      if ((s[i+2] & 0xC0) != 0x80) return False;
      i+=3;continue;
    }

    // is it a four byte UTF8
    if ((s[i] & 0xF8) == 0xF0) {
      if (ls < i+3) return False;
      if ((s[i+1] & 0xC0) != 0x80) return False;
      if ((s[i+2] & 0xC0) != 0x80) return False;
      if ((s[i+3] & 0xC0) != 0x80) return False;
      i+=4;continue;
    }

    // Oops not supported or invalid UTF8 encoding.
    return False;
  }
  return False;
}

COO_DCL(String, uint, length )
COO_DEF_RET_NOARGS(String, uint, length) {
  UTF8Str ustr = (UTF8Str)this->impl;
  byte * d = ustr->content->data, * t=0;
  uint ld = ustr->content->ldata;
  uint i = 0;
  uint len = 0;
  
  while(t != d && ld > 0) {
    t = d;
    d=next_code_point_boundary(d,ld);
    ld -= (d-t);
    ++len;
  }
  
  return len;
}


COO_DCL(String, bool, endswith, String suffix)
COO_DEF_RET_ARGS(String, bool, endswith, String suffix;,suffix) {
  
  return False;
}

COO_DCL(String, bool, startswith, String prefix)
COO_DEF_RET_ARGS(String, bool, startswith, String prefix;,prefix) {
  return False;
}

COO_DCL(String, void, concat, String other)
COO_DEF_NORET_ARGS(String, concat, String other;,other) {
  UTF8Str uother = 0;
  UTF8Str uthis = (UTF8Str)this->impl;
  String res = 0;
  Data dres = 0;
  if (!other) return;
  uother = (UTF8Str)other->impl;
  {
    uint i = 0;
    uint newlen = uother->content->ldata +
      uthis->content->ldata;

    dres = Data_new(uthis->oe, newlen);
    for(i = 0;i<uthis->content->ldata;++i) {
      dres->data[i] = uthis->content->data[i];
    }

    for(;i < uthis->content->ldata+uother->content->ldata;++i) {
      dres->data[i] = uother->content->data[i-uthis->content->ldata];
    }
  }
  return;
}

COO_DCL(String, String, substr, uint start, uint end)
COO_DEF_RET_ARGS(String, String, substr, uint start; uint end;,start,end) {
  return 0;
}

COO_DCL(String, const char *, cstr)
COO_DEF_RET_NOARGS(String, const char *, cstr) {
  return 0;
}

String UTF8_String_datanew(OE oe, Data data) {
  String s = (String)oe->getmem(sizeof(*s));
  UTF8Str ustr = 0;

  if (!data) return 0;
  if (!oe) return 0;

  if (!data->data) return 0;
  if (!utf8_is_valid(data->data, data->ldata)) return 0;
  
  if (!s) return 0;
  
  ustr = (UTF8Str)oe->getmem(sizeof(*ustr));
  if (!ustr) goto failure;
  
  ustr->content = Data_copy(oe,data);
  ustr->oe = oe;
  s->impl = ustr;
  return s;
 failure:
  if (s) oe->putmem(s);
  if (ustr) oe->putmem( ustr);
  return s;
}

String UTF8_String_cstrnew(OE oe, const char * s) {
  String res = 0;
  UTF8Str ures = 0;
  uint ls = 0;

  if (!oe) return 0;
  if (!s) return 0;

  ls = osal_strlen(s);

  res = (String)oe->getmem(sizeof(*res));
  if (!res) return 0;

  ures = (UTF8Str)oe->getmem(sizeof(*ures));
  if (!res) goto failure;
  
  res->impl = ures;
  ures->content = Data_new(oe,ls+1);
  if (!ures->content) goto failure;
  ures->content->ldata = ls;
  ures->oe = oe;

  while(--ls) {
    ures->content->data[ls] = s[ls];
  }

  COO_ATTACH(String, res, length );
  COO_ATTACH(String, res, endswith);
  COO_ATTACH(String, res, startswith);
  COO_ATTACH(String, res, concat);
  COO_ATTACH(String, res, substr);
  COO_ATTACH(String, res, cstr);

  return res;
 failure:
  UTF8_String_destroy(&res);
  return 0;
}

void UTF8_String_destroy(String * s) {
  OE oe = 0;
  if (s) {
    if (*s) {
      UTF8Str ustr = (UTF8Str)(*s)->impl;
      oe = ustr->oe;
      if (ustr) {
        if (ustr->content) {
          Data_destroy(ustr->oe, &ustr->content);
        }
      }
      oe->putmem(ustr);
      COO_DETACH( (*s), length);
      COO_DETACH( (*s), endswith);
      COO_DETACH( (*s), startswith);
      COO_DETACH( (*s), concat);
      COO_DETACH( (*s), substr);
      COO_DETACH( (*s), cstr);
      oe->putmem( (*s) );
    }
  }

}
