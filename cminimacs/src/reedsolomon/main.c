#include <terminal/terminal.h>
#include <reedsolomon/reedsolomon.h>

#include <time.h>
#include <stdio.h>

static void print_error(char * msg) {
  set_color(RED);
  printf("Error: %s.\n",msg);
  reset();
}

inline static void print_usage(char * progname) {
  printf("%s usage",progname);
  set_color(WHITE);set_bold(1);
  printf(" <cmd> <data>\n");reset();				\
  printf("The first argument, cmd, can be one of the following:\n");
  printf("\tenc - encode the given data as a MiniMacs codeword\n");
  printf("\tval - check if the given data as a valid MiniMacs codeword\n");
  printf("\n");
}

inline static void do_encode(char * data) {
  polynomial * codeword = 0;
  MATRIX * codewordvec = 0;
  uint i = 0,ldata=0;

  if (!data) {
    print_error("No message given");
    return;
  }

  ldata = strlen(data);

  if (ldata == 0) {
    print_error("Message of length zero is no acceptable");
    return;
  }

  codeword = minimacs_encode((polynomial*)data,ldata,2*ldata);
  
  if (!codeword) {
    print_error("Encoding algorithm failed");
    return;
  }

  codewordvec = new_matrix(1,2*ldata);
  if (!codewordvec) {
    print_error("Out of memory creating code word vector\n");
    return ;
  }

  for(i = 0; i < 2*ldata;i++)
    matrix_setentry(codewordvec,0,i,codeword[i]);

  set_color(GREEN);
  print_matrix(codewordvec);
  reset();

  // do not clean up we are leaving anyway, let the OS do that
}

static inline void skip_white_space(char ** p) {
  char * s = 0;
  if (!p) return;

  s = *p;
  while(*s == ' ') s++;

  *p = s;
}

static inline polynomial read_next_int(char **p) {

  char * s = 0;
  polynomial val = 0;
  uint num_len = 1;

  if (!p) {
    print_error("Null pointer given to read_next :(");
    return 0;
  }

  s = *p;s++;

  while(*s != ' ' && *s != '\0') { num_len *= 10;s++;}

  s = *p;

  while(*s != ' ' && *s != '\0') { val += num_len * ((*s) - '0'); num_len/=10;s++; }

  *p = s;

  return val;
}

static inline MATRIX * decode_code(char * code, uint lcode) {

  int i = 0;
  MATRIX * codevec = 0;
  uint num_count = 0;

  // check only spaces and numbers
  // count numbers
  {
    bool in_num = 0;

    for(i = 0; i < lcode;i++) {
      if (code[i] != ' ' && (code[i] < '0' || code[i] > '9')) {
	print_error("Invalid input");
	return 0;
      }
      
      if (code[i] == ' ') { 
	in_num = 0;
      } else {
	if (!in_num) num_count++;
	in_num = 1;
      }
    }
  }

  // read the numbers
  {
    uint pos = 0;
    codevec = new_matrix(num_count,1);
    if (!codevec) {
      print_error("Failed to allocate matrix for code");
      return 0;
    }

    for(i = 0; i < num_count;i++) {
      skip_white_space(&code);
      polynomial v = read_next_int(&code);
      matrix_setentry(codevec,i,0,v);
    }
  }

  return codevec;
}

static inline void do_validate(char * code) {
  
  uint lcode = 0;
  MATRIX * codevec = 0;
  bool valid = 0;

  polynomial * flat = 0;


  if (!code) {
    print_error("Code is null.");
    return;
  }

  lcode =strlen(code);
  
  if (lcode == 0) {
    print_error("Code cannot have length zero");
    return;
  }

  codevec = decode_code(code,lcode);
  if (!codevec) return;

  if (matrix_getheight(codevec) < 1 || matrix_getheight(codevec) & 0x01) {
    print_error("We assume codes are twice the length of any encoded message, thus the code must have even length");
    return;
  } 

  flat = matrix_topoly(codevec,&lcode);

  valid = minimacs_validate(flat,lcode,lcode/2);
  if (valid) {
    set_color(GREEN);
    printf("The code is valid\n");
  } else {
    set_color(RED);
    printf("The code is invalid\n");
  }
  reset();
}

inline static void do_performance(char * strnum, uint lmsg, uint lcode) {
  int no = 0;
  int i = 0;
  char * msg = malloc(lmsg+1);
  clock_t start = 0, stop = 0;

  if (!msg) {
    print_error("failed to allocate message");
    return;
  }
  
  memset(msg,0,lmsg+1);

  if (!strnum) {
    print_error("strnum was null :(");
    return;
  }

  
  // generate random data
  for(i=0;i<lmsg;i++)
    msg[i] = (char)(rand() % 256);

  no = atoi(strnum);

  // check input
  if (no <= 0) {
    print_error("invalid input, iteration count should be positive");
    return;
  }

  // running standard encoding
  set_color(YELLOW);
  printf("Running standard encoding with %d iterations\n",no);
  reset();
  start = clock();
  i = no;
  while(i--) {
    polynomial * code = minimacs_encode( (polynomial*)msg, lmsg, lcode);
    if (!code) {
      print_error("encoding of code failed");
      return;
    }
    {
      bool valid = minimacs_validate(code,lcode,lmsg);
      if (!valid) {
	print_error("an invalid code was generated :(");
	return;
      }
    }
    free(code);
  }
  stop = clock();
  set_color(GREEN);
  {
    long elapsedtime = (long)(((stop-start)*1000) / CLOCKS_PER_SEC);
    printf("Standard encoding took: %ld ms\n", elapsedtime);
    fprintf(stderr,"%ld ",elapsedtime);fflush(stderr);
  }
  reset();


  // running optimised encoding
  set_color(YELLOW);
  printf("Running optimised encoding with %d iterations\n",no);
  reset();
  start = clock();
  i = no;
  {
    MATRIX * encoder = minimacs_generate_encoder(lmsg,lcode);
    if (!encoder) {
      print_error("failed to generate encoder");
      return ;
    }

    while(i--) {
      polynomial * code = minimacs_encode_fast(encoder, msg, lmsg);
      if (!code) {
	print_error("failed to generated code (fast)");
	return;
      }
      {
	bool valid = minimacs_validate_fast(encoder,code,lcode,lmsg);
	if (!valid) {
	  print_error("an invalid code was generated");
	  return;
	}
      }
      free(code);
    }
    destroy_matrix(encoder);
  }
  stop = clock();
  set_color(GREEN);
  {
    long elapsedtime = (long)(((stop-start)*1000) / CLOCKS_PER_SEC);
    printf("Optimised encoding took: %ld ms\n", elapsedtime);
    fprintf(stderr,"%ld\n",elapsedtime);fflush(stderr);
  }
  reset();
  
}

int main(int c, char **argv, char ** env) {
  set_bold(1);
  set_color(WHITE);
  printf("\nReed Solomon over GF[2^8] (c) Aarhus University 2013\n\n");
  reset();

  init_polynomial();  
  init_matrix();


  // check no. args
  if (c != 3) {
    print_usage(argv[0]);
    return -1;
  }

  // check argv[1]
  if (strcmp(argv[1],"enc") == 0) do_encode(argv[2]);
  else if (strcmp(argv[1],"val") == 0) do_validate(argv[2]);
  else if (strcmp(argv[1],"perf")==0) do_performance(argv[2],40,80);
  else { print_usage(argv[0]); return -2;}
  
  reset();
  printf("\n");
  return 0;

}


