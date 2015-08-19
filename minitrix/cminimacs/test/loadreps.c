#include <minimacs/minimacs.h>
#include <minimacs/minimacs_rep.h>
#include <minimacs/minimacs_pre.h>
#include <reedsolomon/reedsolomon.h>
#include <osal.h>

DerRC write_bdt(BitDecomposedTriple * t, uint lt, byte ** dout, uint * ldout );

int main(int c, char **s) {
  OE oe = OperatingEnvironment_New();
  uint ltext = 120, lcode = 256, nplayers = 2;
  MiniMacsEnc encoder = 0;
  MiniMacsTripleRep ** triples = 0;
  MiniMacsRep ** singles = 0;
  MiniMacsRep *** pairs = 0;
  BitDecomposedTriple ** t = 0;
  MiniMacsRep * compat = 0;
  uint ltriples=1, lsingles=1, lpairs=1, lbtriples=1;
  byte * buf = 0;
  uint lbuf = 0;
  init_polynomial();

  encoder = MiniMacsEnc_MatrixNew(oe,lcode, ltext);
  
  compat = minimacs_fake_setup(oe, encoder, 
                               ltext, nplayers, lcode, 
                               &triples, ltriples, 
                               &singles, lsingles, 
                               &pairs, lpairs,0);
  minimacs_fake_bdt( oe, encoder, compat, &t, lbtriples);
  
  

  printf("Save/Load BitDecomposedTriples\n");

  write_bdt(t[0], lbtriples, &buf, &lbuf);
  {
    BitDecomposedTriple * loaded_bdt = 0;
    uint lloaded_bdt = 0;
    DerRC rc = 0;
    uint i = 0;

    rc = read_bdt(buf, lbuf, &loaded_bdt, &lloaded_bdt);
    if (rc != DER_OK) {
      printf("Oh crap \n");
      exit(-1);
    }

    if (!loaded_bdt) {
      printf("Oh crap nothing there\n");
      return -2;
    }

    if (lloaded_bdt ==1 ){
      printf("Yes we loaded one\n");
    } else {
      printf("Oh, no this failes\n");
    }

    
    for (i = 0;i < lloaded_bdt;++i) {
      int j = 0;
      if (loaded_bdt[i]->a == 0) {
        printf("[%u] has no a\n",i);
      }

      if (loaded_bdt[i]->b == 0) {
        printf("[%u] has no b\n",i);
      }

      if (loaded_bdt[i]->c == 0) {
        printf("[%u] has no c\n",i);
      }

      for(j = 0;j < 8;++j) {
       
        if (loaded_bdt[i]->abits[j] == 0) {
          printf("[%u] has no abits[%u]\n",i,j);
        }

        if (loaded_bdt[i]->bbits[j] == 0) {
          printf("[%u] has no abits[%u]\n",i,j);
        }

      }
    }
  }

  {
    FD fd = 0;
    oe->open("file bdt.fat", &fd);
    oe->write(fd, buf, &lbuf);
    oe->close(fd);

  }
  
  //  minimacs_fake_bdt(oe, encoder, 
  
  return 0;

  
  

}
