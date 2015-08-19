#include <osal.h>
#include <reedsolomon/reedsolomon.h>
#include <reedsolomon/minimacs_enc_fft.h>
#include <math/fft.h>
#include <math/matrix.h>

int main(int c, char **a) {
  OE oe = OperatingEnvironment_New();
  MiniMacsEnc fftenc = 0; 
  byte test[256] = {1};
  byte * enc = 0;
  uint i = 0;
  uint len = 119;

  init_polynomial();
  init_matrix();

  fftenc = MiniMacsEnc_FFT119New(oe);

  enc = fftenc->encode(test,len);
  
  for(i = 0;i < 256;++i) {
    if (i>0&&i%16==0) oe->print("\n");
    oe->print("%02x ",enc[i]);
  }
  oe->print("\n");
  
  return 0;
}
