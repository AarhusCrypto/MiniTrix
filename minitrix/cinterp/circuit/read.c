#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef unsigned int uint;
typedef unsigned char byte;
typedef struct _token_ {
  uint start;
  uint end;
  byte * tok;
} * Token;



typedef enum {
  PRC_OK = 0,
  PRC_SYN, // Syntax error in input
  PRC_PAR, // Parser error 
  PRC_ARG
} PRC;

typedef enum {
  ADD, MUL, SMUL, SADD, MOV
} OP;

typedef struct _location_ {
  uint start, end;
  uint line, ch;
} Loc;


typedef struct _parser_res_ {
  PRC rc;
  Loc loc;
  char * err;
  union {

    struct {
      uint lop;
      uint rop;
      uint dst;
      OP op;
    } BinOp;
    
    struct {
      char * name;
      uint value;
    } ConstVal;

    struct {
      uint fst, snd, trd, fth;
    } FourNum;

  } Res;
} * ParserRes;



static uint compute_line(byte * buf, uint lbuf, uint off, uint * last_line_off) {
  uint res = 1;
  uint i = 0;
  uint line_off = 0;
  while( i < lbuf && i < off) {
    if (buf[i] == '\n') { 
      line_off = i;
      ++res;
    }
    ++i;
  }

  if (last_line_off) {
    *last_line_off = line_off;
  }

  return res;
}




static ParserRes ParserRes_new( PRC rc, byte * buf, uint lbuf, uint start, uint end, const char * msg ) {
  ParserRes res = (ParserRes)malloc(sizeof(*res));
  uint line_off = 0;
  if (!res) {
    return 0;
  }
  memset(res, 0, sizeof(*res));
  
  res->rc = rc;
  res->loc.start = start;
  res->loc.end=end;
  res->loc.line = compute_line(buf, lbuf, start, &line_off);
  res->loc.ch = start - line_off;
  if (msg) {
    uint lmsg = strlen(msg);
    res->err = (char *)malloc(lmsg+1);
    if (res->err) {
      memset(res->err, 0, lmsg+1);
      memcpy(res->err, msg, lmsg);
    }
  }

  return res;
}



static void ParserRes_destroy( ParserRes * r ) {

  if (!r) return;
  if (!*r) return ;


  if ((*r)->err) {
    free((*r)->err);
  }

  memset(*r, 0, sizeof(**r));
  *r = 0;
}


inline static ParserRes ParserRes_add( uint lop, uint top, uint dst ) {
  //  ParseRes res = ParseRes_new( PRC_OK, 
  
}


#define PRES_ERR( RC, MSG, TOK ) ParserRes_new( RC, buf, lbuf, (TOK)->start , (TOK)->end, MSG )

#define PRES_OK( ADDTOK ) \
  ParserRes_new( PRC_OK, buf, lbuf, ADDTOK->start, ADDTOK->end, 0 );

#define PRES_INT( VAL ) 
// ------------------------------------------------------------
// TOKEN
// ------------------------------------------------------------
static void Token_destroy( Token * token) {
  if (!token) return;
  if (!*token) return;

  if ( (*token)->tok) {
    free( (*token)->tok);
  }
  
  free(*token);
  *token = 0;
}



static Token Token_new(byte * buf, uint lbuf, uint start, uint end) {
  Token res = (Token)malloc(sizeof(*res));
  if (!(end-start)) return 0;
  if (!res) return 0;
  memset(res, 0, sizeof(*res));
  res->tok = (byte*)malloc(end-start+1);
  if (!res->tok) goto failure;
  memset(res->tok, 0, end-start+1);
  memcpy(res->tok, buf+start, end-start);
  res->start = start;
  res->end = end;
  return res;
 failure:
  if (res) Token_destroy( &res );
  return 0;
}
// ------------------------------------------------------------





static Token do_parse_token(byte * buf, uint lbuf, uint * idx_) {
  Token res = 0;
  uint i = 0;
  uint start = 0;
  uint idx;

  if (!idx_) {
    idx = 0;
  } else {
    idx = *idx_;
  }

  if (idx>=lbuf) return 0;
  

  // skip whites
  while( i + idx < lbuf &&  (buf[idx+i] == ' '  || buf[idx+i] == '\t' )) {
    ++i;
  }
  
  if (buf[idx+i] == '\n') {
    if (idx_) {
      *idx_ = idx+i+1;
    }
    return Token_new(buf, lbuf, idx+i, idx+i+1);
  }


  // eat until white
  start = idx+i;
  while( i + idx < lbuf && (buf[idx+i] != ' ' && buf[idx+i] != '\t' && buf[idx+i] != '\n') ) 
    ++i;
  
  
  if (idx_){
    *idx_ = idx+i;
  }

  res = Token_new(buf, lbuf, start, idx+i);
  return res;
}



inline static Token next(byte * buf, uint lbuf, uint * idx) {
  return do_parse_token( buf, lbuf, idx );
}

static uint line;

static int is_token(Token tok, const char * s) {
  uint ls = 0;
  uint l = 0;
  if(!tok) return 0;
  if (!tok->tok) return 0;
  if (!s) return 0;
  ls = strlen(s);
  l = strlen(tok->tok);
  l = l > ls ? ls : l;
  return memcmp(tok->tok, s, l) == 0;
}




static ParserRes do_parse_int(byte * buf, uint lbuf, uint * idx) {
  ParserRes res = 0;
  int v = 0;
  uint ltok = 0;
  Token t = next(buf, lbuf, idx );
  ltok = strlen(t->tok);
  while(ltok--) {
    if (t->tok[ltok] < '0' || t->tok[ltok] > '9') 
      return PRES_ERR(PRC_SYN, "Error: not an integer.",t);
  }
  v = atoi(t->tok);
  res = PRES_OK(t);
  res->Res.ConstVal.value = v;
  res->Res.ConstVal.name = 0;
  return res;
}



static int do_const(byte * buf, uint lbuf, uint * idx) {
  printf("CONST\n");
  return next(buf, lbuf, idx ) != 0;
}



static int do_add(byte * buf, uint lbuf, uint * idx) {
  Token dst=0,lop=0,rop=0;
  ParserRes res = 0;

  dst = next(buf, lbuf, idx );
  if (!dst) return 0;

  lop = next(buf, lbuf, idx );
  if (!lop) return 0;

  rop = next(buf, lbuf, idx);
  if (!rop) return 0;

  return 1;
}



static int do_comment(byte * buf, uint lbuf, uint * idx) {
  Token n = 0, tmp=0;
  do{
    if (tmp) Token_destroy(&tmp);
    n = next(buf, lbuf, idx);
    if (!n) break; else {
      tmp = n;
    }
  } while(!is_token(n, "\n"));
  if (is_token(n,"\n")) ++line;
  if (tmp) Token_destroy ( &tmp );
  return n != 0;
}



static int do_newline(byte * buf, uint lbuf, uint * idx) {
  
  ++line;
  return 1;
}



static ParserRes read_any_number_of_comments(byte * buf, uint lbuf, uint * idx) {
  Token t = 0;
  Token last = 0;
  do{
    t = next(buf, lbuf, idx);
    if (is_token(t, "#")) {
      last = t;
      if (do_comment(buf, lbuf, idx )) { 
        continue;
      } else return PRES_ERR(PRC_SYN, "Syntax error in comment", t);
    } else break;
  }   while(t);
  return PRES_OK( last );;
}



static ParserRes read_four_numbers( byte * buf, uint lbuf, uint * idx) {
  
  Token fst = next(buf, lbuf, idx);
  Token snd = next(buf, lbuf, idx);
  Token trd = next(buf, lbuf, idx);
  Token fth = next(buf, lbuf, idx);

  if (!fst || !snd || !trd || !fth ) {
    return PRES_ERR( PRC_SYN, "Syntax error at ",fst );
  }

  return PRES_OK(fst);

}


static int read_const_section(byte * buf, uint lbuf, uint *idx) {
  
}



static int read_circuit( byte * buf, uint lbuf, uint * idx) {

  Token t = 0;
  ParserRes res = 0;

  do {
    t = next(buf, lbuf, idx);
    if (t) {

      if (is_token(t, "add")) { 
        res = do_add(buf, lbuf, idx);
        continue;
      }

      if (is_token(t,"mul")) {
        res = do_mul(buf, lbuf, idx);
        continue;
      }

      if (is_token(t,"mulpar")) {
        res =  do_mulpar(buf, lbuf, idx);
        continue;
      }

      if (is_token(t,"smul")) {
        res = do_smul(buf, lbuf, idx);
        continue;
      }

      if (is_token(t,"sadd")) {
        res = do_sadd(buf, lbuf, idx);
        continue;
      }
        

      if (is_token(t, "\n")) {
        continue;
      }

    };
  } while (t);

}


/*
  while(t) {

    
  }

    if (is_token(t, "CONST")) { 
      if (do_const(buf, lbuf, idx)) {
        t = next(buf, lbuf, idx);continue;
      } else break;
    }
    
    if (is_token(t, "add")) { 
      if (do_add(buf, lbuf, idx)) {
        t = next(buf, lbuf, idx);continue;
      } else {
        printf("Failed to parse add\n");
        break;
      }
    }

    if (is_token(t, "\n")) {
      do_newline(buf, lbuf, idx);
      t = next(buf, lbuf, idx);continue;
      continue;
    }

    printf("Syntax Error at offset %d unrecognized token \"%s\"\n",t->start, t->tok);
    break;
  }

*/

static int top(byte * buf, uint lbuf, uint * idx) {

  ParserRes pr = 0;

  pr = read_any_number_of_comments(buf, lbuf, idx);
  if (pr->rc != PRC_OK) {
    printf("%d:%d - %s",pr->loc.line, pr->loc.ch, pr->err);return 0;
  }

  read_four_numbers( buf, lbuf, idx );

  read_const_section(buf, lbuf, idx );

  read_circuit(buf, lbuf, idx );

  return 0;
}


int main(int c, char ** a) {
  
  FILE * in = 0;
  if (c == 2) {
    uint lbuf = 1024*10;
    byte * buf = malloc(lbuf);
    Token tok = 0;
    uint idx = 0;

    in = fopen(a[1], "rb");

    if (!in) {
      printf("Cannot open file for reading\n");
      return -1;
    }

    lbuf = fread(buf, 1, lbuf, in);

    fclose(in);

    top(buf, lbuf, &idx );

  } else {
    printf("usage %s <file>\n", a[0]);
  }
  return 0;
}
