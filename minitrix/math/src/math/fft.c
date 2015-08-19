#include <math/fft.h>
#include <math/matrix.h>

static byte omega = 3;

static 
void pol_assign(byte * dst, byte * src) {
  uint i = 0;
  for(i = 0;i<256;++i) {
    dst[i] = src[i];
  }
}



static 
int pol_cmp(byte *a, byte *b) {
  uint i = 0;
  for(i = 0; i < 256;++i) {
    if (a[i] != b[i]) return (a[i] > b[i] ? 1 : -1);
  }
  return 0;
}



static 
void pol_add(byte * a, byte * b, byte * res) {
  uint i = 0;
  for(i = 0;i < 256;++i) {
    res[i] = add(a[i],b[i]);
  }
  return;
}

static byte zero_pol[256] = {0};
static byte one_pol[256] = {1};

/*!
 * Poly GCD
 *
 * \param f         - polynomial in GF2^8[x]
 * \param g         - polynomial in GF2^8[x]
 * \param gcd [out] - gcd(f,g)
 * \param a [out]   - polynomial f quotient
 * \param b [put]   - polynomial f quotient
 *
 * s.t. gdc(f,g) = a*f + b*g
 */
void
polynomial_gcd(byte * f, byte * g, byte * gcd, byte * a, byte * b) {
  byte s[256]={0},t[256]={0},r[256]={0};
  byte os[256]={0},ot[256]={0},or[256]={0};
  
  // s = 0;
  pol_assign(os,one_pol); // old_s = 1
  pol_assign(t,one_pol); // t = 1
  // old_t = 0
  pol_assign(r,f); // r = b
  pol_assign(or,g); // old_r = a
  
  

  while ( pol_cmp( r, zero_pol) != 0 ) { // r != 0
    byte tmp[256] = {0};
    // (old_r, r) := (r, old_r - r)
    pol_assign(tmp,r);
    pol_add(or,r,r); // r = old_r - r
    pol_assign(or,tmp); // ord_r = r

    //  (old_s, s) := (s, old_s - s)
    pol_assign(tmp,s);
    pol_add(os,s,s);
    pol_assign(os,tmp);

    //  (old_t, t) := (t, old_t - t)       
    pol_assign(tmp,t);
    pol_add(ot,t,t);
    pol_assign(os,tmp);
  }

  
  pol_assign(gcd,or);
  pol_assign(a,t);
  pol_assign(b,s);
}


/*
 * Naiv poly mul.
 *
 */
void
polynomial_mul(byte * f, byte * g, uint degree, byte * res) {
  uint i = 0, j = 0;
  for(i = 0; i < degree;++i) {
    for(j = 0;j < degree;++j) {
      res[i+j] = add(res[i+j],multiply(f[j],g[i]));
    }
  }
  return;
}


/*!
 * Evaluates the polynomial f[0]+f[1]x+...+f[lf-1]x^{lf-1} in the
 * point x.
 */  
uint order_of(byte e) {
  uint ord = 0;

  for(ord = 1;ord < 255;++ord) {
    if (pol_pow(e,ord) == 1) return ord;
  }
  return 0;
}

// TODO(rwl): Remove this is stupid will only give you the parity.

byte n_to_pol(uint n) {
  uint i = 0;
  byte res = 0;
  for(i = 0;i < n;++i) {
    res = add(res,0x01);
  }
  return res;
}



void trickfftinv(uint P, uint Q, byte * f, byte * res) {
  byte rev[256] = {0};
  uint i = 0;

  for(i = 0; i < P*Q+1;++i) {
    rev[i] = f[P*Q-i];
  }
  printf(" F\tREV\n");
  for(i = 0;i < P*Q+1;++i) {
    printf("|%u\t|%u|\n",f[i],rev[i]);
  }
  printf("\n");
  efft(P,Q,rev,res);
}


void efftinv(uint P, uint Q, byte * f, byte *res ) {
  byte oN = smallest_nth_primitive_root_of_unity(P*Q);
  byte oQ = pol_pow(oN,P);
  uint pc = 0, qc = 0, k = 0;
  byte Qroot = 1;  
  byte Nroot = 1;
  byte rowFFT[256][256] = {{0}}; // P x Q 
  byte rootK = 0;
  oN = inverse(oN);
  oQ = inverse(oQ);

  for(qc = 0;qc < Q;++qc) {
    for(pc = 0;pc < P;++pc) {
      rootK = 1;
      for(k = 0;k < Q;++k) { // rowFFT[pc,qc] = SUM_{k=0}^{Q-1} 
	byte term = 0;
	
	// A[k,pc] each term: f_{pc*Q+k}
	term = multiply(f[k*P+pc],rootK);
	
	rowFFT[pc][qc] = add(rowFFT[pc][qc],term);
	rootK = multiply(rootK, Qroot);
      }
    }
    Qroot = multiply(Qroot,oQ);
  }
  
  for(pc = 0;pc < P;++pc) {
    for(qc = 0;qc < Q;++qc) {
      rootK = 1;
      for(k = 0;k < P;++k) {
	byte term = 0;
	uint res_idx = pc*Q+qc;	
	term = 
	  multiply(rowFFT[k][qc],rootK);

	res[res_idx] = add(res[res_idx],term);
	rootK = multiply(rootK, Nroot);
      }
      Nroot = multiply(Nroot,oN);
    }
  }
}

/*!
 *
 */
static
void _efft(uint P, uint Q, byte qroot, byte nroot, byte * f, byte * res) {
  uint qhat = 0, rho = 0, q = 0, rhohat = 0;
  uint N = P*Q, i =0;
  byte oN = nroot;
  byte oq = qroot;
  byte z = 1, y = 1;
  byte Y[256][256] = {{0}};
  
  if (oN == 0) {
    printf("Failed to find %u root of unity\n", N);
  }

  if (oq == 0) {
    printf("Failed to find %u root of unity\n",q);
  }

  
  for(qhat = 0; qhat <= Q;++qhat) {
    for(rho = 0; rho <= P; ++rho) {
      for(q = 0; q < Q;++q) {
        uint fidx = q*P+rho;
        Y[rho][qhat] = add(Y[rho][qhat],multiply(f[fidx],pol_pow(y,q)));
        if (qhat == 15) {
          printf("Y[%u][%u]=%u\n", rho, qhat, Y[rho][qhat]);
        }

        if (qhat == Q-1)
          printf("fidx=%u ",fidx);
      }
    }
    y = multiply(y,oq);
  }
  printf("\n");
  
  for(rhohat = 0 ; rhohat <= P;++rhohat ) {
    for(qhat = 0; qhat <= Q; ++qhat ) {// qhat loop
      for(rho = 0;rho < P;++rho) { // SUM
        uint residx = rhohat*Q + qhat;
        res[rhohat*Q + qhat] = add(res[rhohat*Q+qhat],
				   multiply(Y[rho][qhat],
                    pol_pow(z,rho)));
        if (residx == 255) {
          printf("Y[%u][%u]=%u, pow(%u,%u)\n",rho,qhat,Y[rho][qhat],z,rho);
        }

        if (rho == P-1) {
          printf("residx = %u ",residx);

        }
      }
      z = multiply(z,oN);
    }
  }
  printf("\n");
}



/*!
 * DFT_w^{-1}(f) = DFT_{w^{-1}}(f)
 */
void efftinv2(uint P, uint Q, byte * y, byte * res) {
  uint N = P*Q,i=0;
  byte nroot = omega;
  byte qroot = pol_pow(nroot,P);
  byte nrootinv = inverse(nroot);
  byte qrootinv = inverse(qroot);
  composite_fft2(P,Q,qrootinv, nrootinv, y, res);
}



/*!
 *
 *
 */
void efft(uint P, uint Q, byte * f, byte * res) {
  uint N = P*Q;
  byte nroot = omega;
  byte qroot = pol_pow(nroot,P);
  composite_fft2(P,Q,qroot,nroot,f,res);
}




/*!
 *
 *
 */
static byte eval_pol(byte * f, uint lf, byte x) {
  
  uint idx = 0;
  byte res = 0;
  byte x_pow_idx = 1;
  for(idx = 0;idx < lf; ++idx) {
    res = add(res,multiply(f[idx],x_pow_idx));
    x_pow_idx = multiply(x_pow_idx,x);
  }
  return res;
}


/*!
 * Build the h by w Van Der Monde matrix on the form:

 * +----------------------+
 * | 1 1  1  ...       1  |
 * | 1 r r^2 ...  2^{w-1} |
 * | 1 r^2 r^3 ... 2^{w-2}|
 * .                      .
 * | etc...               |
 * +----------------------+
 */
MATRIX * build_nth_matrix(OE oe, uint h, uint w, byte root) {
  int i = 0, j = 0;
  byte curroot = 1;
  MATRIX * m = new_matrix(oe,h,w);
  for(i=0;i<h;++i) {
    byte val = 1;
    for(j=0;j<w;++j) {
      matrix_setentry(m,i,j,val);
      val = multiply(val,curroot);
    }
    curroot = multiply(root,curroot);
  }
  return m;
}


MATRIX * minimacs_fft_shurgenerator(OE oe, uint lmsg, uint lcode, byte root) {
  
  MATRIX * res = 0;
  MATRIX * small = build_nth_matrix(oe,lmsg,lmsg,root);
  MATRIX * smallinv = LUInverse(small);
  MATRIX * big = build_nth_matrix(oe,lcode, lmsg, root);
  res = matrix_multiplication( big, smallinv);
  return res;
}


/*!
 * {n} is the nth root number and f has that degree.
 *
 * {f} is an n vector of gf2^8 elements
 *
 * {omega} is root of unity of length n with 1, omega, omega^2 etc...
 *
 * TODO(rwl): 
 *
 * I)   Optimize the 1 of the omega list no need to have it there.
 * II)  
 */    
void txt_book_fft(OE oe, uint n, byte * f, byte * omegas, byte * result) {
  byte * r0 = 0;
  byte * r1star = 0;
  byte * new_omegas = 0;
  byte * r0_result = 0;
  byte * r1star_result = 0;
  uint idx = 0;
  byte omega = omegas[1];
  byte q = 0;

  // base case
  if (n == 1) { 
    result[0] = f[0];
    return;
  }
  
  r0 = oe->getmem(n/2+1);
  r1star = oe->getmem(n/2+1);
  

  // divide 
  for(idx = 0;idx<n/2;++idx) {
    q = add(f[idx],f[idx+n/2]);
    r0[idx] = q;
    r1star[idx] = multiply(q,omegas[idx]);
  }

  if (n%2) {
    r0[n/2] = f[n/2];
    r1star[n/2]=multiply(f[n/2],omegas[n/2]);
  }

  new_omegas = oe->getmem(n);
  r0_result = oe->getmem(n);
  r1star_result = oe->getmem(n);
  for(idx = 1;idx<n;++idx) {
    new_omegas[idx] = multiply(omegas[idx],omega);
  }
  new_omegas[idx] = 1;
  
  txt_book_fft(oe, n/2, r0, new_omegas, r0_result);
  txt_book_fft(oe, n/2, r1star, new_omegas,r1star_result);
  
  for(idx = 0;idx < 2*(n/2);idx+=2) {
    result[idx]  = eval_pol(r0_result,n/2,omegas[idx/2]);
    result[idx+1]= eval_pol(r1star_result,n/2,omegas[idx/2]); 
  }
  oe->putmem(r0);
  oe->putmem(r1star);
  oe->putmem(r0_result);
  oe->putmem(r1star_result);
  oe->putmem(new_omegas);
  return;
}






void fft_gf8(byte * ab, byte * a, byte *b, uint len) {
  if (!ab) return;
  if (!a) return;
  if (!b) return ;
}

byte smallest_nth_primitive_root_of_unity(uint n){
  byte res = 0;
  byte b = 2;
  uint j = 0;

  while(j < 257) {
    uint i = 0;
    // is it a nth root?
	if ((res = pol_pow(b, n)) == 1) {
		bool ok = 1;
		// okay, but is it a primitive root?
		for (i = 2; i < n; ++i) {
			if (pol_pow(b, i) == 1) {
				ok = 0;
			}
		}

		if (ok) {
			return b;
		}
	}
    
    ++b;
    ++j;
  }
  return 0;
}

uint nth_root_of_unity(byte * r, uint n) {
  int b = 0;
  uint count = 0;
  for(b = 2;b<256;++b) {
    if (pol_pow( (byte)b, n) == 1) {
      r[count++] = b;
    }
  }
  
  
  return count;
}

static void dump(char * s, byte  mxm[17][17], uint P, uint Q) {
  uint i = 0,j=0;
  printf("\n------------------------------------------------------------\n");
  printf("%s (%ux%u):\n",s,P,Q);
  for(i = 0;i<P;++i) {
    for(j = 0;j<Q;++j) {
      printf("%3u ",mxm[i][j]);
    }
    printf("\n");
  }
  printf("\n------------------------------------------------------------\n");
}

/*
 Now let k = w*P where w < Q 
                          l=j+iQ
 Zr = SUM_l^{k} a_l W_N^l =  
 Zr = SUM_i^P SUM_j^Q a_{j+iQ} W^{r(j+iQ)} = SUM_i^P SUM_j^Q a_{j+iQ} W{(s+tP)(j+iQ)}
    = SUM_i^P SUM_j^Q a_{j+iQ} W^{(sj+siQ+tPj+tPiQ)} = SUM_j^Q ( SUM_i^P a_{j+iQ} W_P^{si} ) W_N^{(sj+tPj)}}


 */


/**
 * Compute the Fast Fourier Transform given N=PQ for roots of order P and N.
 *
 * 
 *
 */
void fft(uint P, uint Q, byte proot, byte nroot, byte * a, uint la /* k */, byte * z) {
  uint i = 0, j = 0;
  byte Y[255][255]= {0};

  // Computing inner sums
  
}

void rwz_thefft_(uint P, uint Q, uint qcut, byte Qroot, byte Nroot, const byte * f, byte * y) {
  uint kp = 0, kq = 0, t = 0, s = 0;
  byte Y[20][20] = {{0}};
  byte qroot_to_t = 1;
  byte qroot_to_tkq = 1;
  byte nroot_to_m = 1;
  byte nroot_to_mkp = 1;
  zeromem(y,P*qcut);
  for(t = 0; t < Q;++t) {
    for(kp = 0;kp < P;++kp) {
      qroot_to_tkq = 1;
      for(kq = 0; kq < qcut;++kq) {
	byte term = multiply(f[kq*P+kp],qroot_to_tkq);
	Y[t][kp] = add(Y[t][kp],term);
	qroot_to_tkq = multiply(qroot_to_tkq,qroot_to_t);
      }
    }
    qroot_to_t = multiply(qroot_to_t,Qroot);
  }

  for(s = 0; s < P;++s) {
    for(t = 0; t < Q;++t) {
      nroot_to_mkp = 1;
      for(kp = 0; kp < P;++kp) {
	byte term = multiply(Y[t][kp],nroot_to_mkp);
	y[s*Q+t] = add(y[s*Q+t],term);
	nroot_to_mkp = multiply(nroot_to_mkp,nroot_to_m);
      }
      nroot_to_m = multiply(nroot_to_m,Nroot);
    }
  }

}

void composite_fft2(uint P, uint Q, byte qroot, byte nroot, byte * f, byte * y) {
  rwz_thefft_(P,Q,Q,qroot,nroot,f,y);
}

// Transforms {f} into {y}
/*
void composite_fft2(uint P, uint Q, byte qroot, byte nroot, byte * f, byte * y) {
  uint i = 0; 
  uint j = 0;
  uint k = 0;
  byte qroot_to_j = 1;
  byte qroot_to_jk = 1;
  byte nroot_to_sigma = 1;
  byte nroot_to_sigma_k = 1;
  byte Y[18][18] = {{0}};

  for(i = 0; i < P;++i) {
    qroot_to_j = 1;
    for(j = 0; j < Q;++j){ 
      qroot_to_jk = 1;
      for(k = 0;k < P;++k) {
        byte term = multiply(f[k*P+i], qroot_to_jk);
        Y[i][j] = add(Y[i][j],term);
        qroot_to_jk = multiply(qroot_to_jk,qroot_to_j);
      }
      qroot_to_j = multiply(qroot_to_j,qroot);
    }
  }
  
  for(i = 0;i < P;++i) {
    for(j = 0;j < Q;++j) {
      nroot_to_sigma_k = 1;
      for(k = 0;k < P;++k) {
        byte term = multiply(Y[k][j],nroot_to_sigma_k);
        y[i*Q+j] = add(y[i*Q+j],term);
        //        printf("%u == %u\n",nroot_to_sigma_k, pol_pow(nroot,(i*Q+j)*k));
        nroot_to_sigma_k = multiply(nroot_to_sigma_k,nroot_to_sigma);
      }
      nroot_to_sigma = multiply(nroot_to_sigma,nroot);
    }
  }

  return;
}
*/

void composite_fft(uint P, uint Q, byte qroot, byte nroot, byte * f, byte * res) {
  byte A[32][32] = {{0}}; // P x Q
  byte Y[32][32] = {{0}}; // P x Q
  byte R[32][32] = {{0}}; // Q x P
  uint p=0,q=0,k=0;
  uint N = p*q;
  uint lf = N+1;
  byte qroot_to_qk = 1;
  byte qroot_to_q = 1;
  byte nroot_to_sigma = 1; // sigma = pQ+q
  byte nroot_to_sigma_k = 1;
  

  // layout f in A column wise
  for(q=0;q < Q;++q) {
    for(p = 0; p < P;++p) {
      A[p][q] = f[q*P+p];
    }
  }

  dump("A[PxQ]",A,P,Q);

  // inner sums
  for(q = 0;q<Q;++q) {
    for(p = 0;p<P;++p) {
      qroot_to_qk = 1;
      for( k = 0; k < Q;++k) {
        byte val = multiply(f[k*P+p],qroot_to_qk);
        Y[q][p] = add(Y[q][p],val);
        qroot_to_qk = multiply(qroot_to_qk,qroot_to_q);
      }
    }
    qroot_to_q = multiply(qroot_to_q,qroot);
  }
  
  dump("Y[QxP]",Y,Q,P);

  // outer sums
  for(p = 0;p < P;++p) {
    for(q = 0;q < Q;++q) {
      nroot_to_sigma_k = 1;
      for( k = 0; k < P;++k) {
        byte val = multiply(Y[q][k],nroot_to_sigma_k);
        R[q][p] = add(R[q][p],val);
        nroot_to_sigma_k = multiply(nroot_to_sigma_k,nroot_to_sigma);
      }
      nroot_to_sigma = multiply(nroot_to_sigma,nroot); // sigma = pQ+q
    }
  }

  //dump("R",R,Q,P);
  for(q=0;q<Q;++q)
    for(p=0;p<P;++p)
      res[p*Q+q] = R[q][p];

  //dump("res",res,1,P*Q+1);
}
