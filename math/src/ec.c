/*!
 * Fun with EC over GF 2^8.
 *
 *
 */


/************************************************************
 GALOIS 2^8
************************************************************/
typedef unsigned char polynomial;

static 
polynomial add(polynomial p1, polynomial p2) {
  return p1 ^ p2;
}

static 
polynomial multiply(polynomial p1, polynomial p2) {
  int i = 0;
  polynomial res = 0;
  for(i = 0;i < 8;++i) {
    polynomial h = 0;
    if (p1 & 0x01) {
      res = add(res,p2);
    }
    
    h = p2 & 0x80; // high bit
    p2 << 1; // push one bit up into the x^8 position, hence if (h !=
    // 0) a reduction module x^8 + x^4 + x^3 + x + 1 must take place. 
    if (h) p2 = add(p2,0x1b); // add x^4 + x^3 + x + 1
  }
}

/************************************************************/


/************************************************************
  Elliptic Curve Arith - Binary Edwards Curves
************************************************************/
typedef unsigned int uint;

typedef struct _ec_point_ {
  polynomial x,y;
} P;

typedef struct _curve_ {
  polynomial d1, d2;
} EdwardsCurve;

static 
void ec_edwards_add(EdwardsCurve * E, P * r, P a, P b) {
  {
    polynomial t1 = multiply(E->d1,add(a.x,b.x));
    polynomial t2 = multiply(E->d2,multiply(add(a.x,a.y),add(b.x,b.y)));
    polynomial f1 = add(a.x,multiply(a.x,a.x));
    polynomial ff1 = multiply(b.x,add(a.y,add(b.y,1)));
    polynomial ff2 = multiply(a.y,b.y);
    polynomial f2 = add(ff1,ff2);
    polynomial t3 = multiply(f1,f2);
    polynomial numerator = add(t1,add(t2,t3));
    polynomial denominator = add(E->d1,
                                 multiply(
                                          add(a.y,
                                              multiply(a.y,a.y)),
                                          add(b.x,b.y)));
    r->x = multiply(numerator,inverse(denominator));
  }

  {
    polynomial t1 = multiply(E->d1,add(a.x,b.x));
    polynomial t2 = multiply(E->d2,multiply(add(a.x,a.y),add(b.x,b.y)));
    polynomial f1 = add(a.y,multiply(a.y,a.y));
    polynomial f2 = add(multiply(a.y,b.y), multiply(b.x,add(a.y,add(b.y,1))));
    polynomial t3 = multiply(f1,f2);
    polynomial numerator = add(t1, add( t2, t3 ));
    polynomial denominator = add(E->d1,
                                 multiply(add(a.y,multiply(a.y,a.y)),add(b.x,b.y)));
    r->y = multiply(numerator,inverse(denominator));
    
  }
}


static
polynomial ec_edwards_double(EdwardsCurve * E, P * r, P a) {
  
  {
    polynomial nominator = multiply(E->d1,add(1,add(a.x,a.y)));
    polynomial t1 = multiply(a.x,a.y);
    polynomial t2 = multiply(multiply(a.x,a.x),add(1,add(a.x, a.y)));
    polynomial denominator = add(E->d1,add(t1,t2));
    
    r->x = add(1,multiply(nominator,inverse(denominator)));
  }

  {
    polynomial numinator = multiply(E->d1,add(1,add(a.x,a.y)));
    polynomial t1 = multiply(a.x,a.y);
    polynomial t2 = multiply(multiply(a.y,a.y),add(1,add(a.x, a.y)));
    polynomial denominator = add(E->d1,add(t1,t2));
    r->y = add(1,multiply(numinator,inverse(denominator)));
  }
}

static 
polynomial ec_edwards_pow(EdwardsCurve * E, P * r, P a, uint exp) {
  uint count = exp;
  P res = {0};
  while ( count ) {
    ec_edwards_double(E,&res,a);
    count = count >> 1;
  }

  if (exp % 2 == 1) {
    ec_edwards_add(E, r, res, a);
  }
}

// return 1 if {p} is on the curve
static
int curve_check(EdwardsCurve * E, P p) {

  polynomial xsq = multiply(p.x,p.x);
  polynomial ysq = multiply(p.y,p.y);
  polynomial left = add(multiply(E->d1,add(p.x,p.y)),
                        multiply(E->d2,add(xsq,ysq)));

  polynomial right = add(
                         multiply(p.x,p.y),
                         add(multiply(multiply(p.x,p.y),add(p.x,p.y)),
                             multiply(xsq,ysq)));

  if (left == right) return 1;
  
  return 0;
}

static 
find_order_n(EdwardsCurve * E, uint n) {
  polynomial x=0;
  polynomial y=0;
  uint i = 0;

  // search all elements for one where the n-fold add gives 1.
  for(x = 0;x < 255;++x) {
    for(y = 0;y < 255;++y) {
      P p = {x,y};
      P r = {0};
      ec_edwards_pow(E, &r,p,n);
      
    }
  }
  
  
}

/************************************************************/


/*************************************************************
 ElGamal 
************************************************************/
/*
static
ElGamalKey gen(EdwardsCurve E,int n) {
  
  

}

static 
polynomial enc(byte plaintext) {

}

static
polynomial dec(byte plaintext) {

}
*/
/************************************************************/

int main(int c, char **a) {
  printf("Binary Elliptic Curves over Galois 2^8 just for fun\n");
  printf("All rights reserved (c) Rasmus Winther Lauritsen "
         "@ Aarhus University\n");
  
  
  {
    uint i = 0;
    polynomial x = 0,y = 0;
    EdwardsCurve E = {129,130};
    for(x = 0;x < 255;++x) {
      for(y = 0;y < 255;++y) {
        P p = {x,y};
        if (curve_check(&E,p)) {
          if (i > 0 && i % 16 == 0) printf("\n");
          ++i;
          printf("(%3u,%3u)",p.x,p.y);
        }
      }
    }
    printf("\n%u elements on the curve\n");
  }
  
}
