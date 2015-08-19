#include "reedsolomon/minimacs_bit_encoder.h"
#include <stats.h>
#include <coov4.h>

typedef struct _bit_encoder_ {
  OE oe;
  ull * small; // bit * col * row
  ull * big; // bit * col * row
  uint lcode;
  uint ltext;
  uint rem;
} * MiniMacsBitEncoder;



COO_DEF(MiniMacsEnc, polynomial *, encode, byte * msg, uint lmsg);
  MiniMacsBitEncoder impl = (MiniMacsBitEncoder)this->impl;
  OE oe = impl->oe;
  uint i = 0;
  unsigned long long result[256] = {0};
  byte * r = 0; 
  ull * m = 0;
  uint bit = 0, col = 0, k = 0;
  CHECK_POINT_S("[RSCODE] Bit Encode");
  if (!result) return 0;

  if (lmsg == impl->ltext) m = impl->small;
  if (lmsg == impl->ltext*2) m = impl->big;
  if (m == 0) {
    oe->p("[BitEnc] Error: Invalid encoding length.");
    return 0;
  }
  

  /*

   1   0   0   0   0   0   0   0
   0   1   0   0   0   0   0   0
   0   0   1   0   0   0   0   0
   0   0   0   1   0   0   0   0
   0   0   0   0   1   0   0   0
   0   0   0   0   0   1   0   0
   0   0   0   0   0   0   1   0
   0   0   0   0   0   0   0   1
  94 104  92 232 210  66  93  78
 104  34 184 231 147  25 205  83
  92 184 185   3  79 131 201  90
 232 231   3 170 101  50   7 247
 210 147  79 101 215 158 106  73
  66  25 131  50 158 201 213 105
  93 205 201   7 106 213 130  98
 236 181 203  19  27 180 164 139


  */
  for(col = 0; col < lmsg; ++col) {
    if (msg[col] == 1) {
      ull * pcol = m+((col*impl->lcode) >> 3);
      for(k = 0; k < (impl->lcode >> 3);++k) {
        result[k] ^= pcol[k];
      }
    }
  }

  r = oe->getmem(impl->lcode*sizeof(*r));
  
  for(k = 0;k < impl->lcode;++k) {
    uint ullno = k/sizeof(ull);
    uint bytno = k % sizeof(ull);
    r[k] = (result[ullno] >> bytno*8) & 0xFF;
  }
  
  CHECK_POINT_E("[RSCODE] Bit Encode");
  return r;
}


COO_DEF(MiniMacsEnc, bool, validate, byte * code, uint lmsg)
  MiniMacsBitEncoder impl = (MiniMacsBitEncoder)this->impl;
  uint i = 0;
  polynomial * expected = this->encode(code,lmsg);
  
  for(i = 0; i < impl->lcode;++i) {
    if (code[i] != expected[i]) return False;
  }

  impl->oe->putmem(expected);

  return True;
}



MiniMacsEnc MiniMacsEnc_BitNew(OE oe, uint lcode, uint ltext) {
  MiniMacsEnc res = (MiniMacsEnc)oe->getmem(sizeof(*res));
  MiniMacsBitEncoder impl = (MiniMacsBitEncoder)oe->getmem(sizeof(*impl));
  MATRIX * small = 0, * big = 0;
  uint bit=0,col=0,row=0,k = 0;
  uint rem = 0 ;
  if (!res) {
    return 0;
  }

  rem = lcode % sizeof(ull);

  lcode += sizeof(ull)- rem; // now its a multiplum 

  small = minimacs_generate_encoder(oe,ltext, lcode);

  big = minimacs_generate_encoder(oe,2*ltext, lcode);

  impl->small = oe->getmem(lcode*ltext);
  
  for(col = 0; col < ltext;++col) {
    for(row = 0; row < lcode;++row) {
      byte ent = matrix_getentry(small,row,col);
      int col_idx = (col*lcode) >> 3; 
      int ull_idx = (row >> 3);
      int byt_idx = row - (ull_idx<<3);
      ull * pull = impl->small + ( (col_idx) + ull_idx);
      ull _ent = ent;
      *pull ^= (_ent << byt_idx*8 );
    }
  }


  impl->big = oe->getmem(lcode*2*ltext*8);
  
  for(col = 0; col < 2*ltext;++col) {
    for(row = 0; row < lcode;++row) {
      byte ent = matrix_getentry(big,row,col);
      int col_idx = (col*lcode) >> 3; 
      int ull_idx = (row >> 3);
      int byt_idx = row - (ull_idx<<3);
      ull * pull = impl->big + ( (col_idx) + ull_idx);
      ull _ent = ent;
      *pull ^= (_ent << byt_idx*8 );
    }
  }

  destroy_matrix(small);
  destroy_matrix(big);

  res->impl = impl;
  impl->oe = oe;
  impl->ltext = ltext;
  impl->lcode = lcode;
  impl->rem = rem;
  res->encode = COO_attach(res, MiniMacsEnc_encode);
  res->validate = COO_attach(res, MiniMacsEnc_validate);
  return res;
 failure:
  MiniMacsEnc_BitDestroy( &res );
  return 0;
}

void MiniMacsEnc_BitDestroy( MiniMacsEnc * enc ) {
  OE oe = 0;
  MiniMacsBitEncoder impl = 0;
  uint i = 0, j = 0;
  if (!enc) return;
  if (!*enc) return;
  
  impl = (*enc)->impl;
  oe = impl->oe;

  oe->putmem(impl->small);
  oe->putmem(impl->big);

  COO_detach( (*enc)->encode);
  COO_detach( (*enc)->validate);
  oe->putmem( (*enc)->impl);
  oe->putmem(*enc);
  *enc = 0;
}
