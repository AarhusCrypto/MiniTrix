#include <minimacs/minimacs_rep.h>
#include <minimacs/minimacs_pre.h>
#include <reedsolomon/minimacs_enc_fft.h>

void ui_print(SetupEventKind kind, uint count, uint total) {
  
  switch(kind) {

  case SINGLES:
    printf("s(%u/%u)\n",count, total);
    break;
  case TRIPLES:
    printf("t(%u/%u)\n",count, total);
    break;
  case PAIRS:
    printf("p(%u/%u)\n",count, total);
    break;
  default:
    return;
  }
  return;
}



  
int main(int c, char ** a) {
  
  MiniMacsRep * compats = 0; 
  MiniMacsRep ** singles = 0;
  MiniMacsRep *** pairs  = 0;
  MiniMacsTripleRep ** triples = 0;
  BitDecomposedTriple ** btriples = 0;
  
  uint ltext = 0;
  uint nplayers = 0;
  uint ncount = 0;
  uint codelength = 0;
  uint ltriples = 0, lpairs = 0;
  
  uint player = 0;
  uint count = 0;
  OE oe = 0;
  MiniMacsEnc encoder = 0;
  char * postfix = "mxt";
  
  init_polynomial();  
  oe = OperatingEnvironment_New();

  if (c < 6 || c > 7) {
    printf("Usage <ltext> <codelength> <nplayers> <count> <mxt|fft> [<bwa|sba>]\n");
    printf("\tmxt - proprocessing using Vandermonde matrix encoding.\n" \
           "\tfft - preprocessing using Fast Fourier encoding.\n"       \
           "\n\n"                                                       \
           "\tbwa - generate bit decomposed triples with [a],[b]\n"     \
           "        and [c]* with [a_i] and [b_i].\n"                   \
           "\tsba - smart bit decomposed triples saving one round of\n" \
           "        communication. A triple is not [a]*,[b]*,[c]* and\n"\
           "        [a_i], [b_i].\n");

    return 1;
  }
  
  ltext = atoi(a[1]);
  codelength = atoi(a[2]);
  nplayers = atoi(a[3]);
  ltriples = lpairs = ncount = atoi(a[4]);
  
  printf("-- Generation report --\n");
  printf("message length          : %d\n",ltext);
  printf("reedsolomon code length : %d\n",codelength);
  printf("number of players       : %d\n",nplayers);
  printf("number of items         : %d\n",ncount);

  //  encoder = MiniMacsEnc_MatrixNew(oe, codelength, ltext);
  //  encoder = MiniMacsEnc_MatrixNew(oe, 256, 120);

  do {
    if (strlen(a[5]) ==3) {
      if (memcmp("mxt",a[5],3) == 0) {
        encoder = MiniMacsEnc_MatrixNew(oe, codelength, ltext );
        break;
      } 

      if (memcmp("fft",a[5],3) == 0) {
        if (ltext == 85 && codelength == 255) {
          encoder = MiniMacsEnc_FFTNew(oe);
          postfix = "fft";
          break;
        } else {
          printf("Oh, for fft we only support 85 bytes messages over 255 byte codewords\n");
          return -1;
        }
      }
    }
    printf("Error: the fifth argument must be either mxt or fft\n");
    return -1;
  } while(0);

  if (c == 7) {
    uint l = 0;
    while(a[6][l]) ++l;
    if (l != 3) {
      printf("Error: Expected the 6th argument to be either bwa or sba\n");
      return 0;
    }

    if (memcmp("bwa",a[6],3) == 0) {
      ltriples = 0;
    }

    if (memcmp("sba",a[6],3) == 0) {
      ltriples = 0;
      lpairs = 0;
    }
  }

  compats = minimacs_fake_setup(oe,encoder,
                                ltext,nplayers,codelength,
                                &triples,ltriples,
                                &singles,ncount,
                                &pairs,lpairs,
                                ui_print );


  if (c == 7) {
    if (memcmp("bwa",a[6],3) == 0) {
      minimacs_fake_bdt(oe, encoder,
                        compats, &btriples,
                        ncount);
      
    }

    if (memcmp("sba",a[6],3) == 0) {
      minimacs_fake_bdt(oe, encoder,
                        compats, &btriples,
                        ncount);
    }
  }

  if (!triples || !singles || !pairs ) {
    printf("Generation of preprocessing materials failed.\n");
    return -1;
  }

  save_shares(postfix, nplayers, triples, ltriples, singles, ncount, pairs, lpairs );
  if (btriples) 
    save_bdt(postfix, ltext, codelength, nplayers, btriples, ncount);

  OperatingEnvironment_Destroy( & oe );
  return 0;
}
