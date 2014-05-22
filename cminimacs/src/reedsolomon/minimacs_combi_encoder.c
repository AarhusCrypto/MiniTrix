#include <reedsolomon/minimacs_combi_encoder.h>


typedef struct _combi_encoder_ {
  uint ltext,lcode;
} * MiniMacsCombiEncoder;

MiniMacsEnc MiniMacsEnc_CombiNew(OE oe, uint lcode, uint ltext) {
  MiniMacsEnc res = 0;
  MiniMacsCombiEncoder impl = (MiniMacsCombiEncoder)oe->getmem(sizeof(*impl));

  // allocate the interface
  res = (MiniMacsEnc)oe->getmem(sizeof(*res));
  if (!res) return 0;
}
