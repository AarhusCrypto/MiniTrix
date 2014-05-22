#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <string.h>

typedef unsigned char gf8;
gf8 read_number(char * s, int * index);
gf8 read_op(char op, char * s, int * index);
gf8 do_read( char * s, int * index );

static inline void skip(char c, char * s, int * index) {
  int idx = *index;
  while(s[idx] == c) ++idx;
  *index=idx;
}

static inline int is_num(char c) {
  return c >= '0' && c <= '9';
}

static inline int is_op(char c) {
  static char ops[] = {'+','*','/','-','^','o','s'};
  int i = 0;
  for(i=0;i<sizeof(ops);++i) {
    if (c == ops[i]) return 1;
  }
  return 0;
}

static inline gf8 do_op(char op, gf8 l, gf8 r) {
  if (op == '+' || op == '-') return add(l,r);
  if (op == '*') return multiply(l,r);
  if (op == '/') return multiply(l,inverse(r));
  if (op == '^') return pol_pow(l,r);
  if (op == 'o') return  smallest_nth_primitive_root_of_unity(l);
  if (op == 's') {
    uint i = 0;
    for (i = 0;i < r;++i) {
      printf("%u ",pol_pow(l,i));
    }
    return pol_pow(l,i);
  }
  printf("unknown operator %c\n",op);
  exit(-1);
}

gf8 do_read( char * s, int * index ) {
  char op = 0;
  skip(' ',s,index);
  if (is_num(s[*index])) return read_number(s,index);
  if (is_op(s[*index])) { 
    op = s[*index];*index = (*index) + 1;
    return read_op(op,s,index);
  }
  if (s[*index] == 0) return 0;

  if (s[*index] == '\n') {
    *index = *index + 1;
    return do_read(s,index);
  }

  printf("Unknown character %c\n",s[*index]);
  exit(-1);
}

gf8 read_number(char * s, int * index) {
  int idx = *index;
  int val = 0;
  int len = 0;

  while(s[idx+len] != ' ') ++len;
  val = (256 + atoi(s+idx)) % 256;
  idx+=len;
  *index = idx;
  return (gf8)val;  
}

gf8 read_op(char op, char * s, int * index) {
  gf8 left = do_read(s,index);
  gf8 right = do_read(s,index);
  return do_op(op, left, right);
}

static int read_line(char *s,int ls) {
  int i = 0;
  char c = 0;
  // zero
  for(i = 0; i < ls;++i) s[i] = 0;
  i = 0;
  // read until newline or end of s
  while(i<ls && c !='\n') {
    c = getchar();
    s[i] = c;
    ++i;
  }
  // return number of bytes read
  return i;
}

int main(int argc, char ** args) {
  char s[256] = {0};
  int i = 0;
  uint ls = 0;
  printf("Galois Field 2^8 Polish Notation Calculator.\n");
  init_polynomial();
  while(ls < 4 || memcmp("quit",s,4) != 0) {
    gf8 res = 0;
    int i = 0;
    printf("gf8>");fflush(stdout);
    ls = read_line(s,sizeof(s));
    res = do_read(s,&i);
    printf("%02u\n",res);
  }
    
}
