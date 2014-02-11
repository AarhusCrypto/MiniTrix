#include "minimacs/minimacs_pre.h"
#include <common.h>
#include <stdlib.h>
#include "minimacs/minimacs_rep.h"
#include <encoding/der.h>

#define FAILURE(A,B) { printf("Error: %s\n",A);return 0;}

#define NOTIFY(KIND,COUNT,TOTAL) {\
    if (listener) { listener( (KIND), (COUNT), (TOTAL) ); } }; 

static void rand_bytes( byte * b, uint lb ) {
  uint i = 0;
  
  while(i+4 < lb) {
    int r = rand();
    i2b(r,b+i);
    i+=4;
  }

  for(;i<lb;i++) {
    b[i] = (byte)rand();
  } 
}


MiniMacsRep * minimacs_fake_setup(
                                  OE oe,
                                  MiniMacsEnc encoder,
                                  uint ltext, uint nplayers, uint codelength, 
                                  MiniMacsTripleRep *** triples, uint ltriples,
                                  MiniMacsRep *** singles, uint lsingles,
                                  MiniMacsRep **** pairs, uint lpairs,
                                  SetupListener listener) {
  
  uint ssingles=0, spairs=0, striples=0;
  uint i = 0, j = 0;
  uint count=0,player=0;
  byte * txt = (byte*)malloc(2*ltext);
  MiniMacsRep * current = 0;
  MiniMacsRep * currentB = 0;
  MiniMacsRep * compat = 0;
  
  if (!singles) {
    FAILURE("Singles was null","");
  }

  if (!pairs) {
    FAILURE("Pairs was null","");
  }

  if (!triples) {
    FAILURE("Triples was null","");
  }

  if (!txt) {
    FAILURE("Out of memory",""); 
  }

  //  DBG_P("Generating Reedsolomon polynomial");
  
  NOTIFY(SINGLES,0,lsingles);
  // Create singles
  ssingles = nplayers * sizeof(**singles);
  *singles = (MiniMacsRep**)malloc(ssingles);
  if (!*singles) {
    FAILURE("Failed to allocate singles","");
  }
  memset(*singles,0,ssingles);

  for(player=0;player<nplayers;++player) {
    (*singles)[player] = (MiniMacsRep*)malloc(lsingles*sizeof(MiniMacsRep));
    if (! (*singles)[player] ) {
      FAILURE("Failed to allocate singles, out of memory","");
    }
    memset( (*singles)[player],0,lsingles*sizeof(MiniMacsRep));
  }


  for(count = 0; count < lsingles;++count) {
    memset(txt,0,ltext);
    rand_bytes(txt,ltext);

    current = minimacs_create_rep_from_plaintext_f(encoder, txt,ltext,
						   nplayers,codelength,
						   compat);
    if (!current) {
      FAILURE("Failed to create random representation","");
    }

    for(player = 0;player<nplayers;++player) {
      (*singles)[player][count] = current[player];
    }
    
    if (!compat) {
      compat = current;
    } else {
      if (current) {
	(free(current),current=0);
      }
    }

    NOTIFY(SINGLES,count,lsingles);
  }

  NOTIFY(PAIRS, count, lpairs );
  // ********************
  // Pairs
  // ********************
  spairs = nplayers * sizeof(**pairs);
  *pairs = (MiniMacsRep***)malloc(spairs);
  if (!*pairs) {
    FAILURE("Failed to create singles star pairs, out of memory","");
  }
  memset(*pairs,0,spairs);

  for(player=0;player<nplayers;++player) {
    (*pairs)[player] = (MiniMacsRep**)malloc(lpairs*sizeof( MiniMacsRep * ) );
    if (!(*pairs)[player]) {
      FAILURE("Failed to create list of pairs, out of memory","");
    }
    memset ( (*pairs)[player],0,lpairs*sizeof (MiniMacsRep*));
  }

  for(count = 0;count<lpairs;++count) {

    memset(txt,0,2*ltext);
    rand_bytes(txt,2*ltext);

    // allocate
    for(player = 0; player < nplayers; ++player) {
      (*pairs)[player][count] = (MiniMacsRep*)malloc(2*sizeof(MiniMacsRep));
      if (! (*pairs)[player][count]  ) {
	FAILURE("Failed to allocate pair, out of memory","");
      }
      memset((*pairs)[player][count],0,2*sizeof(MiniMacsRep));
    }

    // first
    current = minimacs_create_rep_from_plaintext_f(encoder,txt,ltext, 
						   nplayers,codelength,compat);
    if (!current) {
      FAILURE("Failed to create single in pair %d",i);
    }

    for(player = 0; player < nplayers; ++player) {
      (*pairs)[player][count][0] = current[player];
    }

    if (current) {
      free(current);
      current=0;
    }
    
    // second
    current = minimacs_create_rep_from_plaintext_f(encoder,txt,2*ltext, 
						       nplayers,codelength,compat);

    if (!current) {
      FAILURE("Failed to create star in pair %d",i);
    }
  
    for(player = 0; player < nplayers; ++player) {   
      (*pairs)[player][count][1] = current[player];
    }
   
    if (current) {
      free(current);
      current=0;
    }

    NOTIFY(PAIRS,count,lpairs);
  }

  NOTIFY(TRIPLES, count, ltriples);
  
  // Triples
  striples = nplayers * sizeof(**triples);
  *triples = (MiniMacsTripleRep**)malloc(striples);

  if (!*triples) {
    FAILURE("Out of memory","");
  }
  memset(*triples,0,sizeof(**triples));

  for(player=0; player < nplayers;++player) {
    (*triples)[player] = (MiniMacsTripleRep*) malloc( ltriples * sizeof(MiniMacsTripleRep));
    if (!(*triples)[player]) {
      FAILURE("Out of memory","");
    }
    memset((*triples)[player],0,ltriples*sizeof(MiniMacsTripleRep));
  }

  for(count = 0; count < ltriples; ++ count ) {
    MiniMacsRep * tmp = 0;


    for(player=0;player<nplayers;++player) {
      (*triples)[player][count] = (MiniMacsTripleRep)malloc(sizeof(*(*triples)[player][count]));
      if (!(*triples)[player][count]) {
	FAILURE("Out of memory","");
      }
      memset((*triples)[player][count],0,sizeof(*(*triples)[player][count]));
    }
 
    // create a
    memset(txt,0,2*ltext);
    rand_bytes(txt,ltext);
    current = minimacs_create_rep_from_plaintext_f(encoder,txt,ltext,nplayers,codelength,compat);
    if (!current) {
      FAILURE("Count not create rep","");
    }
    
    for(player=0;player<nplayers;++player) {
      (*triples)[player][count]->a = current[player];
    }


    // create b
    memset(txt,0,2*ltext);
    rand_bytes(txt,ltext);
    currentB = minimacs_create_rep_from_plaintext_f(encoder,txt,ltext,nplayers,codelength,compat);
    if (!currentB) {
      FAILURE("Count not create rep","");
    }
    
    for(player=0;player<nplayers;++player) {
      (*triples)[player][count]->b = currentB[player];
    }
    
    // create c=ab
    tmp = current;
    current = minimacs_rep_mul_fast(encoder,
				    current,
				    currentB,
				    nplayers);
    if (!current) {
      FAILURE("Representation multiplication failed","");
    }
    (free(tmp),tmp=0);
    (free(currentB),currentB=0);
    
    for(player=0; player < nplayers; ++player) {
      (*triples)[player][count]->cstar = current[player];
    }  
    (free(current),current=0);

    NOTIFY(TRIPLES,count+1,ltriples);
  }
  
  if(txt) {
    free(txt);
    txt=0;
  }

  if (current) {
    free(current);
    current=0;
  }


  return compat;
 failure:
  MiniMacsEnc_MatrixDestroy( &encoder );

  if (current) {
    for(player=0;player<nplayers;++player) {
      minimacs_rep_clean_up(& (current[player] ) );
    }
    (free(current),current=0);
  }

  if (currentB) {
    for(player=0;player<nplayers;++player) {
      minimacs_rep_clean_up(& (currentB[player] ) );
    }
    (free(currentB),currentB=0);
  }

  if (txt) {
    free(txt);
    txt=0;
  }
  
  if (*singles) {
    for(player=0; player < nplayers; ++player) {
      for(count = 0; count < lsingles; ++count ) {
	minimacs_rep_clean_up( &  (*singles)[player][count] );
      }
      free( (*singles)[player] );
      (*singles)[player] = 0;
    }
    free( (*singles) );
    (*singles) = 0;
  }

  if (*pairs) {
    for(player = 0; player < nplayers;++player) {
      if ( (*pairs)[player] ) {
	for(count = 0; count < lpairs;++pairs) {
	  if ( (*pairs)[player][count] ) {
	    
	    // first
	    if ( (*pairs)[player][count][0] ) {
	      minimacs_rep_clean_up( & (*pairs)[player][count][0] );
	    }
	    
	    // second
	    if ( (*pairs)[player][count][1] ) {
	      minimacs_rep_clean_up( & (*pairs)[player][count][1] );
	    }
	    
	    // pair
	    free((*pairs)[player][count]),(*pairs)[player][count]=0;
	  }
	}
	// list
	free( (*pairs)[player] ); 
	(*pairs)[player] = 0;
      }
    }
    (free((*pairs)),(*pairs)=0);
  }

  if (*triples) {
    for(player = 0; player < nplayers; ++player) {
      if ( (*pairs)[player] ) {
	for(count = 0; count < ltriples; ++count ) {
	  if ((*pairs)[player][count] ) {
	    // a
	    if ((*triples)[player][count]->a) {
	      minimacs_rep_clean_up( & (*triples)[player][count]->a );
	    }
	    
	    // b
	    if ((*triples)[player][count]->b) {
	      minimacs_rep_clean_up( & (*triples)[player][count]->b );
	    }

	    // c
	    if ((*triples)[player][count]->cstar) {
	      minimacs_rep_clean_up( & (*triples)[player][count]->cstar );
	    }
	    
	    free((*triples)[player][count]); 
	    (*triples)[player][count] = 0;
	  }
	}
	free((*triples)[player]);
	(*triples)[player] = 0;
      }
    }
    free(*triples);
    *triples = 0;
  }

  if (compat) {
    for(player=0;player<nplayers;++player) {
      minimacs_rep_clean_up( & compat[player] );
    }
    free(compat);
    compat = 0;
  }

  return compat;
}


void minimacs_fake_teardown(uint nplayers,
			    MiniMacsTripleRep *** triples, uint ltriples,
			    MiniMacsRep *** singles, uint lsingles,
			    MiniMacsRep **** pairs, uint lpairs,
			    MiniMacsRep * compat) {
  uint player = 0;
  uint count  = 0;
  if (triples) {
    if (*triples) {
      for(player=0;player<nplayers;++player) {
	if ( (*triples)[player]) {
	  for(count = 0; count < ltriples;++count) {
	    
	    minimacs_rep_clean_up( & (*triples)[player][count]->a );
	    minimacs_rep_clean_up( & (*triples)[player][count]->b );
	    minimacs_rep_clean_up( & (*triples)[player][count]->cstar );
	    free( (*triples)[player][count] );
	    (*triples)[player][count] = 0;
	    
	  }
	  free((*triples)[player]);(*triples)[player] = 0;
	}
      }
      free( (*triples) ); (*triples = 0);
    }
  }

  if (singles) {
    if (*singles) {
      for(player=0;player < nplayers;++player) {
	if ( (*singles)[player] ) {
	  for(count = 0; count < lsingles; ++count) {
	    minimacs_rep_clean_up( & (*singles)[player][count] );
	  }
	  free((*singles)[player]);(*singles)[player] = 0;
	}
      }
      free( (*singles) );
      *singles = 0;
    }
  }

  if (pairs) {
    if (*pairs) {
      for(player=0;player<nplayers;++player) {
	if ( (*pairs)[player] ) {
	  for(count=0;count<lpairs;++count) {
	    if ( (*pairs)[player][count][0]) {
	      minimacs_rep_clean_up( & (*pairs)[player][count][0] );
	    }
	    if ( (*pairs)[player][count][1] ) {
	      minimacs_rep_clean_up( & (*pairs)[player][count][1] );
	    }
	    free((*pairs)[player][count]);
	    (*pairs)[player][count]=0;
	  }
	  free( (*pairs)[player]);
	  (*pairs)[player] = 0;
	}
      }
      free(*pairs);*pairs = 0;
    }
  }

  if (compat) {
    free(compat);compat=0;
  }
}

/*
void test(MiniMacsRep rep) {
  uint i = 0;
  DerCtx * c = 0;
  DerRC rc = 0;

  rc = der_begin_seq(&c);
  rc = der_insert_uint(c,rep->lval);
  rc = der_insert_octetstring(c,rep->dx_codeword, rep->ldx_codeword );
  rc = der_insert_octetstring(c,rep->codeword, rep->lcodeword );
  rc = der_begin_seq(&c);
  rc = der_insert_uint(c,rep->lmac);
  for(i = 0;i<rep->lmac;++i) { 
    if (rep->mac[i] != 0) {
      der_begin_seq(&c);
      der_insert_uint(c,rep->mac[i]->mid);
      der_insert_uint(c,rep->mac[i]->toid);
      der_insert_uint(c,rep->mac[i]->fromid);
      der_insert_octetstring(c, rep->mac[i], rep->mac[i]->lmac );
      der_end_seq(&c);
    }
  }
  der_end_seq(&c);

  der_begin_seq(&c);
  der_insert_uint(c,rep->lmac_keys_to_others);

  for(i = 0;i<rep->lmac_keys_to_others;++i) {
    if (rep->mac_keys_to_others[i]) {
      der_begin_seq(&c);
      der_insert_uint(c,rep->mac_keys_to_others[i]->mid);
      der_insert_uint(c,rep->mac_keys_to_others[i]->toid);
      der_insert_uint(c,rep->mac_keys_to_others[i]->fromid);

      der_insert_octetstring(c, rep->mac_keys_to_others[i]->alpha, 
			     rep->mac_keys_to_others[i]->lalpha);

      der_insert_octetstring(c,rep->mac_keys_to_others[i]->beta,
			     rep->mac_keys_to_others[i]->lbeta);
      der_end_seq(&c);
    }
  }
  der_end_seq(&c);
  der_end_seq(&c);
}

void minimacs_store_pre(uint nplayers,
			MiniMacsTripleRep * triples, uint ltriples,
			MiniMacsRep * singles, uint lsingles,
			MiniMacsRep ** pairs, uint lpairs,
			byte ** data_out, uint * ldata_out) {
  
  /*
    PRE = SEQUENCE ( 
            nplayers ::= INTEGER,
	    nTriples ::= INTEGER,
	    Triples  ::= SEQ(n) of TRIPLE,
	    nSingles ::= INTEGER,
	    Singles  ::= SEQ(nSingles) MiniMacsRep,
	    nPairs   ::= INTEGER,
	    Pairs    ::= SEQ(2*nPairs) MiniMacsRep
	    )

    PAIR = SEQUENCE( 
       first ::= MiniMacsRep,
       second::= MiniMacsRep
       ) 

    TRIPLE = SEQUENCE (
        a ::= MiniMacsRep,
	b ::= MiniMacsRep,
	c ::= MiniMacsRep
	)

	*
  
  uint ltmp = 1024;
  byte tmp[1024] = {0};
  uint otmp = 0;
  DerRC rc = DER_OK;

  rc = der_encode_integer(nplayers,tmp,&otmp, ltmp );
  if (rc != DER_OK) return rc;

  
  
  
  
}
*/
