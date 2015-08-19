#include "fs.h"
#include <sys/stat.h>
#include <stdio.h>

uint read_file_size(const char * filename) {
  struct stat st = {0};
  stat(filename,&st);
  return st.st_size;
}

uint read_entire_file(const char * filename, byte * buf, uint lbuf) {
  ull read = 0, intotal=0;
  FILE * file = 0; 

  if (!filename) return 0;
  file=fopen(filename,"rb");
  if (!file) return 0;
  if (!buf) return 0;
  
  do {
    read = fread(buf+intotal, 1, lbuf-intotal, file);
    intotal += read;
  } while(read);

  fclose(file);
  return intotal;
}
