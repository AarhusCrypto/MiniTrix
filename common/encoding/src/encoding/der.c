#include "encoding/der.h"
#include "encoding/int.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <coov4.h>

#define UNI ( 0x00 << 6 )
#define APP ( 0x01 << 6 )
#define CSP ( 0x02 << 6 ) // context specific
#define PRV ( 0x03 << 6 )// private

#define PRI ( 0x00 << 5 )
#define CON ( 0x01 << 5 )

/*!
 * \brief given an encoded length in {buf} at {*off} write the length
 * of the remaining data to {l}. E.g. extract the length and write it to {*l}...
 */
DerRC get_len(uint * l, byte * buf, uint * off, uint lbuf) {
  uint i = 0;

  if (!l) return DER_ARG;

  if (!buf) return DER_ARG;

  if (!off) return DER_ARG;

  if (lbuf < *off) return DER_ARG;

  // short form
  if (buf[*off] < 0x80) {
    *l = buf[*off];
    *off += 1;
    return DER_OK;
  } 

  // long form
  if (buf[*off] > 0x80) {
    uint len = 0;
    uint lenlen = buf[*off] - 0x80;
    if (lbuf < *off + 1 + lenlen ) return DER_ARG;
   
    for(i = 0; i < lenlen; i++ ) {
      len += (buf[*off+i+1] << 8*(lenlen-i-1));
    }
    *l = len;
    *off += lenlen+1;
  }

  return DER_OK;
}

typedef struct _der_output_stream_ {
	RC(*write_int)(ull v);
	RC(*write_str)(byte * str, uint lstr);
	RC(*write_cstr)(const char * cstr);
	RC(*begin_seq)();
	RC(*leave_seq)();
	void * impl;
} *DerOutputStream;

#define CYCLIC_BUF_SIZ 8192
typedef struct _der_input_stream_impl_ {
	OE oe;
	FD fd;

	// cyclic buffer
	byte cbuf[CYCLIC_BUF_SIZ];
	uint cbuf_start;
	uint cbuf_len;

	List seqs;

} *DerInputStreamImpl;

static RC cyclic_take(DerInputStreamImpl s, byte * b) {
	RC rc = RC_OK;
	if (s->cbuf_len == 0) {
		byte buf[CYCLIC_BUF_SIZ] = { 0 };
		uint lbuf = sizeof(buf);
		uint i = 0;

		rc = s->oe->read(s->fd, buf, &lbuf);
		if (rc != RC_OK) return rc;

		for (i = 0; i < lbuf; ++i) {
			uint idx = (s->cbuf_start + i) % sizeof(s->cbuf);
			s->cbuf[idx] = buf[i];
		}

		s->cbuf_len = lbuf;
	}

	if (s->cbuf_len > 0) {
		if (b) {
			*b = s->cbuf[s->cbuf_start];
		}
		s->cbuf_start = (s->cbuf_start + 1) % sizeof(s->cbuf);
		s->cbuf_len -= 1;
	}
	else {
		return RC_BAD_DATA;
	}
	return rc;
}

static void cyclic_rewind(DerInputStreamImpl s) {
	s->cbuf_len += 1;
	s->cbuf_start = (s->cbuf_start - 1) % sizeof(s->cbuf);
}

static RC read_thelen(DerInputStreamImpl impl, uint * len, uint * _lenlen) {
	byte l = 0;
	RC rc = RC_OK;
	uint thelen = 0;
	uint j = 0;

	rc = cyclic_take(impl, &l);
	if (rc != RC_OK) return rc;

	if (l > 0x80) {
		byte lenbyt = 0;
		uint lenlen = l - 0x80;

		for (j = 0; j < lenlen; ++j) {
			rc = cyclic_take(impl, &lenbyt);
			if (rc != RC_OK) return rc;

			thelen += (lenbyt << 8 * (lenlen - 1 - j));
		}
		if (_lenlen) *_lenlen = lenlen+1;
	}
	else {
		thelen = l;
		if (_lenlen) *_lenlen = 1;
	}


	if (len) {
		*len = thelen;
	}

	return RC_OK;
}

typedef struct _seq_entry_ {
	ull offset;
	ull length;
	ull lenlen;
} *SeqEnt;

COO_DEF(DerInputStream, RC, peek_tag, ull * tagout) {
	DerInputStreamImpl impl = (DerInputStreamImpl)this->impl;
	OE oe = impl->oe;
	RC rc = RC_OK;
	byte tag = 0;
	uint thelen = 0, j = 0;

	rc = cyclic_take(impl, &tag);
	if (rc != RC_OK) return rc;

	cyclic_rewind(impl);
	
	if (tagout) {
		*tagout = tag;
	}

	return RC_OK;
}}

COO_DEF(DerInputStream, RC, leave_seq) {
	DerInputStreamImpl impl = (DerInputStreamImpl)this->impl;
	OE oe = impl->oe;
	RC rc = RC_OK;
	byte tag = 0;
	uint thelen = 0, j = 0;
	SeqEnt ent = 0;
	ull len = 0, thelenlen = 0;


	if (impl->seqs->size() == 0) return RC_BAD_DATA;

	ent = impl->seqs->get_element(impl->seqs->size() - 1);
	if (!ent) return RC_FAIL;

	impl->seqs->rem_element(impl->seqs->size() - 1);

	for (j = 0; j < ent->length - ent->offset; ++j) {
		rc = cyclic_take(impl, 0);
		if (rc != RC_OK) return rc;
	}
	len = ent->length;
	thelenlen = ent->lenlen;
	oe->putmem(ent);

	ent = impl->seqs->get_element(impl->seqs->size() - 1);
	if (ent) {
		ent->offset += ( len + thelenlen + 1);
	}

	return RC_OK;

}}

COO_DEF(DerInputStream, RC, begin_seq) {
	DerInputStreamImpl impl = (DerInputStreamImpl)this->impl;
	OE oe = impl->oe;
	RC rc = RC_OK;
	byte tag = 0;
	uint thelen = 0, j = 0, thelenlen = 0;
	SeqEnt ent = oe->getmem(sizeof(*ent));

	rc = cyclic_take(impl, &tag);
	if (rc != RC_OK) return rc;

	if (tag != 0x30) {
		cyclic_rewind(impl);
		return RC_BAD_DATA;
	}

	rc = read_thelen(impl, &thelen,&thelenlen);
	if (rc != RC_OK) return rc;

	ent->offset = 0;
	ent->length = thelen;
	ent->lenlen = thelenlen;

	impl->seqs->add_element(ent);

	return RC_OK;
}}

COO_DEF(DerInputStream, RC, read_str, byte ** str, uint *lstr) {
	DerInputStreamImpl impl = (DerInputStreamImpl)this->impl;
	OE oe = impl->oe;
	RC rc = RC_OK;
	byte tag = 0;
	uint thelen = 0,j=0, thelenlen = 0;
	byte *res = 0;
	SeqEnt ent = 0;

	rc = cyclic_take(impl, &tag);
	if (rc != RC_OK) return rc;

	if (tag != 0x04) {
		cyclic_rewind(impl);
		return RC_BAD_DATA;
	}

	rc = read_thelen(impl, &thelen,&thelenlen);
	if (rc != RC_OK) return rc;

	ent = impl->seqs->get_element(impl->seqs->size() - 1);
	if (ent) {
		if (ent->length - ent->offset < thelen) return RC_BAD_DATA;
		ent->offset += (thelen  + 1 + thelenlen);
	}

	res = impl->oe->getmem(thelen);
	if (!res) return RC_NOMEM;
		
	for (j = 0; j < thelen; ++j) {
		rc = cyclic_take(impl, res + j);
		if (rc != RC_OK) {
			oe->putmem(res);
			return rc;
		}
	}
	*lstr = thelen;
	*str = res;


	return RC_OK;

}}


COO_DEF(DerInputStream, RC, read_int, uint * v) {
	DerInputStreamImpl impl = (DerInputStreamImpl)this->impl;
	OE oe = impl->oe;
	RC rc = RC_OK;
	byte tag = 0, len = 0;
	byte ints[4] = { 0 };
	SeqEnt ent = 0;

	rc = cyclic_take(impl, &tag);
	if (rc != RC_OK) return rc;

	if (tag != 0x02) return RC_BAD_DATA;

	rc = cyclic_take(impl, &len);
	if (rc != RC_OK) return rc;

	if (len != 4) {
		return RC_BAD_DATA;
	}

	ent = impl->seqs->get_element(impl->seqs->size() - 1);
	if (ent) {
		if (ent->length - ent->offset < len) return RC_BAD_DATA;
		ent->offset += len + 1 + 1;
	}


	rc = cyclic_take(impl, &ints[0]);
	if (rc != RC_OK) return rc;
	rc = cyclic_take(impl, &ints[1]);
	if (rc != RC_OK) return rc;
	rc = cyclic_take(impl, &ints[2]);
	if (rc != RC_OK) return rc;
	rc = cyclic_take(impl, &ints[3]);
	if (rc != RC_OK) return rc;

	// Big Endian Integer
	if (v) {
	  *v = 0;
		*v += ints[0] << 24;
		*v += ints[1] << 16;
		*v += ints[2] << 8;
		*v += ints[3];
	}

	return RC_OK;
}}
List SingleLinkedList_new(OE oe);
DerInputStream DerInputStream_New(OE oe, FD in) {
	DerInputStream res = 0;
	DerInputStreamImpl impl = 0;

	res = oe->getmem(sizeof(*res));
	if (!res) return 0;

	impl = oe->getmem(sizeof(*impl));
	if (!impl) goto fail;

	res->read_int = COO_attach(res, DerInputStream_read_int);
	res->read_str = COO_attach(res, DerInputStream_read_str);
	res->begin_seq = COO_attach(res, DerInputStream_begin_seq);
	res->leave_seq = COO_attach(res, DerInputStream_leave_seq);
	res->peek_tag = COO_attach(res, DerInputStream_peek_tag);
	

	res->impl = impl;
	impl->fd = in;
	impl->oe = oe;
	impl->seqs = SingleLinkedList_new(oe);

	return res;
fail:
	DerInputStream_Destroy(&res);
	return 0;
}

void DerInputStream_Destroy(DerInputStream * s) {

}

DerOutputStream DerOutputStream_New(OE oe, FD out);


static DerRC len_len(uint ldata, byte * len, uint * llen) {
  
  if (!llen) return DER_ARG;


  // the length can be encoded in one byte
  if (ldata < 0x80) {
    *llen = 1;
    if (len) {
      *len = ldata;
    }
  }

  // the length needs to be in multiple bytes
  if (ldata >= 0x80) {
    int i = 0;
    int lenlen = 1;
    
    int tmp = ldata;
    while ( (tmp >>= 8) >0) {
      lenlen++;
    }

    *llen = lenlen + 1;
    if (len) {
      len[0] = 0x80 + lenlen;
      for(i = 0;i<lenlen;i++) {
	len[lenlen-i] = ldata % 256;
	ldata >>= 8;
      }
    }
  } 
  return DER_OK;
}


enum UniversalDerTags {
  EOC = UNI + PRI + 0x00,
  BOOLEAN_P = UNI + PRI +  0x01,
  INT_P = UNI + PRI + 0x02,
  BIT_STR_P = UNI + PRI + 0x03,
  BIT_STR_C = UNI + CON + 0x03,
  OCTET_STR_P = UNI + PRI + 0x04,
  OCTET_STR_C = UNI + CON + 0x04,
  NUL = UNI + PRI + 0x05,
  OBJ_ID = UNI + PRI + 0x06,
  OBJ_DES_P= UNI + PRI + 0x07,
  OBJ_DES_C= UNI + CON + 0x07,
  EXT = UNI + CON + 0x08,
  REAL = UNI + PRI + 0x09,
  ENUM = UNI + PRI + 0x0A,
  UTF8_STR_P = UNI + PRI + 0x0C,
  UTF8_STR_C = UNI + CON + 0x0C,
  REL_OID = UNI + PRI + 0x0D,
  SEQ = UNI + CON + 0x10,
  SET = UNI + CON + 0x11,
  NUM_STR = 0x12,
  PRINT_STR = 0x13,
  UTC_TIME = UNI + PRI + 0x17,
  GRAPHICS_STR = UNI + PRI + 0x19
};



DerRC der_encode_eoc( byte * out, uint * off, uint lout) {

  if (!off) return DER_ARG;
  
  if (out)  {
    if (lout < *off + 2) { 
      return DER_ARG;
    }
  }
 
  if (out) {
    out[*off] = EOC; // tag
    out[*off+1]  = 0; // len
  }

  *off += 2;  

  return DER_OK;
}


DerRC der_decode_eoc( byte * out, uint * off, uint lout) {

  if (!out) return DER_ARG;

  if (!off) return DER_ARG;

  if (lout < *off + 2) return DER_ARG;

  
  if (out[*off] != EOC) {
    return DER_BAD;
  }

  if (out[*off + 1] != 0) {
    return DER_BAD;
  }

  *off += 1;

  return DER_OK;
}

DerRC der_encode_boolean( bool value, byte * out, uint * off, uint lout) {
  
  if (!off) return DER_ARG;

  if (out) {
    if (  lout < *off + 3) {
      return DER_ARG;
    }
  }

  if (out) {
    out[*off+0] = BOOLEAN_P; // tag
    out[*off+1] = 1; // len
    out[*off+2] = value ? 0x01 : 0x00; // content
  }
  
  *off += 3;

  return DER_OK;
}

DerRC der_decode_boolean(bool * value, byte *in, uint * off, uint lin) {

  if (!off) return DER_ARG;
  
  if (!in) return DER_ARG;

  if (lin < *off+3) return DER_ARG;

  if (in[*off+0] != BOOLEAN_P) return DER_BAD;

  if (in[*off+1] != 1) return DER_BAD;

  if (value) {
    *value = (in[ (*off)+2 ] == 0 ? 0 : 1);
  }

  return DER_OK;
}

DerRC der_encode_integer(int value, byte * out, uint * off, uint lout ) {
  
  if (!off) return DER_ARG;

  if (out) {
    if (lout < *off + 6) return DER_ARG;
  }

  if (out) {
    out[*off+0] = INT_P; // tag
    out[*off+1] = 4;
    i2b(value, out+*off+2);
  }

  *off += 6;

  return DER_OK;
}

DerRC der_decode_integer(int * value, byte * in, uint * off, uint lin ) {
  
  if (!off) return DER_ARG;

  if (!in) return DER_ARG;

  if (lin < *off + 6) return DER_ARG;

  if (in[*off+0] != INT_P) return DER_BAD;

  if (in[*off+1] != 4) return DER_BAD;

  if (value) {
    *value = b2i(in+2);
  }
  
  *off += 5;

  return DER_OK;
}

DerRC der_encode_octetstring(byte * v, uint lv, byte * out, uint * off, uint lout) {

  uint lenlen = 0;

  if (!off) return DER_ARG;

  if (!v) return DER_ARG;
  
  if (len_len( lv, 0, &lenlen) != DER_OK) return DER_ARG;

  if (out) {

    if (lout < *off + lenlen + 1 + lv) return DER_ARG;

    out[*off + 0] = OCTET_STR_P; // encode tag
    if (len_len(lv,out+1,&lenlen) != DER_OK) return DER_ARG;  // encode len
    mcpy(out+lenlen+1,v,lv); // content
  }

  *off += lenlen + 1 + lv;
  
  return DER_OK;
}


DerRC der_decode_octetstring(byte *v, uint * lv, byte * in, uint * off, uint lin ) {
  
  uint len = 0;

  if (!off) return DER_ARG;

  if (!in) return DER_ARG;

  if (!lv) return DER_ARG;

  if (lin < *off+2) return DER_ARG;

  if (in[*off] != OCTET_STR_P) return DER_BAD;
  
  *off = 1;
  get_len(&len, in, off, lin);

  if (lin < *off + len) return DER_BAD;
  
  if (!v) {
    *lv = len;
    *off = *off + len;
    return DER_OK;
  }

  if (*lv < len) return DER_ARG;

  mcpy(v,in + *off,len);

  *lv = len;

  *off += len;
  
  return DER_OK;
}

DerRC der_upd_seq( byte * result, uint * offset, uint lresult, byte * data, uint ldata ) {

  uint olen = 0;
  uint nlen = 0;
  uint olenlen = 0;
  uint nlenlen = 0;
  uint ntotallen = 0;
  byte * swap = 0;
  uint lswap = 0;
  uint off = 0;
  DerRC rc = DER_OK;

  if (!result) {
    return DER_ARG;
  }

  if (!data) {
    return DER_ARG;
  }

  if (ldata-2 > lresult) {
    return DER_ARG;
  }

  if (result[0] != SEQ) {
    return DER_BAD;
  }

  // read current length
  off = 1;
  rc = get_len(&olen, result, &off, lresult );
  if (rc != DER_OK) return rc;

  olenlen = result[1] > 0x80 ? result[1] - 0x7F : 1;

  if (olen > lresult) {
    return DER_BAD;
  }

  // compute new len and new len len
  nlen = olen + ldata;
  rc = len_len(nlen, NULL, &nlenlen);
  if (rc != DER_OK) return rc;

  ntotallen = nlen + nlenlen + 1; // 1 for the 0x30 tag

  if (ntotallen > lresult) { // is there room?
    return DER_SHORT;
  }

  // swap up or not
  if (nlenlen > olenlen) {
    uint swap_i = 0;
    uint swap_u = 0;
    uint j = 0;

    // swap up because the length of the length increased
    uint lswap = nlenlen - olenlen;
    swap = (byte*)malloc(lswap);
    if (!swap) return DER_MEM;
    zeromem(swap,lswap);

    
    do {
      swap_u = (swap_i < off+olen);
      for ( j = 0; j <lswap;++j) {
	byte t = swap[j];
	swap[j] = result[off+swap_i+j];
	result[off+swap_i+j] = t;
      }
      swap_i += lswap;
    } while( swap_u );
  } 

  off = 1+nlenlen+olen;

  mcpy(result+off,data,ldata);
  rc = len_len(nlen,result+1,&nlenlen);
  if (rc != DER_OK) return rc;

  if (offset) {
    *offset = off+ldata;
  }

  return DER_OK;
}

DerRC der_encode_seq(byte * result, uint *lresult, uint *offset,...) {
  DerRC rc = DER_OK;
  va_list args = {0};
  uint i = 0;
  uint nargs = 0;
  byte * buffer = 0;
  uint lbuffer = 0;
  uint off = 0;
  uint result_lenlen = 0;
  uint required_result_len = 0;
  
  if (!lresult) {
    return DER_ARG;
  }

  if (offset) {
    off = *offset;
  }
  
  uint payload_length_required = off;

  // compute the required length of the payload
  va_start(args,offset);
  buffer = va_arg(args,byte *);
  while(buffer != 0) {
    lbuffer = va_arg(args,uint);
    // here we have a pair of buffer and length
    payload_length_required += lbuffer;
    buffer = va_arg(args,byte *);
    ++nargs;
  }
  va_end(args);
  
  // compute the length of the length
  rc = len_len( payload_length_required, 0, &result_lenlen);
  if (rc != DER_OK) return rc;

  // compute the actual length required to hold the result
  required_result_len = payload_length_required + 1 + result_lenlen; // 1 for the tag byte

  // make the result
  if (result) {
    if (required_result_len > *lresult) return DER_SHORT;
    result[0] = 0x30; off += 1;
    len_len(payload_length_required,result+off,&result_lenlen); off += result_lenlen;
    va_start(args, offset);
    for(i = 0;i < nargs;++i) {
      buffer  = va_arg(args,byte *);
      lbuffer = va_arg(args,uint );
      //      for(k = 0;k < lbuffer;++k) {
      //	result[off] = buffer[k];
      //      }
      memmove(result+off,buffer,lbuffer);off += lbuffer;
    }
  }

  *lresult = required_result_len;

  if (offset) {
    *offset = off;
  }
  
  return DER_OK;
}


DerRC der_decode_seq(byte * seq, uint lseq, 
		     uint idx, uint * offset, 
		     byte **buffer, uint * lbuffer) {
  DerRC rc = DER_OK;
  uint i = 0;
  uint nlen = 0;
  uint all_len = 0;
  uint off = 0;
  uint tmp_off = 0;
  
  if (offset) { 
    off = *offset;
  } 

  if (!lbuffer) {
    return DER_ARG;
  }

  if (!seq) {
    return DER_ARG;
  }

  if (off+2 > lseq) {
     return DER_ARG;
  }
  
  if (seq[0] != SEQ) {
    return DER_BAD;
  } 

  ++off; // jump tag

  // get length
  rc = get_len(&all_len,seq,&off,lseq);
  if (rc != DER_OK) return rc;

  
  // the stipulated length is longer than the remaining length
  if (lseq-off+1 < all_len) {
    return DER_BAD;
  }
  
  for(i = 0; i < idx;++i) {
    ++off; // jump tag
    nlen = 0;

    // read subsequence length
    rc = get_len(&nlen,seq,&off,lseq);
    if(rc != DER_OK) return rc;
    
    // increase offset
    off += nlen;
    if (off > lseq) return DER_ARG;
  }

  tmp_off = off;
  
  ++off; // jump tag

  rc = get_len(&nlen,seq,&off,lseq);
  if (rc != DER_OK) return rc;

  // rewind to the beginning of 
  nlen += (off-tmp_off);
  off = tmp_off;

  if (buffer) {   
    *lbuffer = nlen;
    *buffer = seq+off;
  } else { *lbuffer = nlen; }
  
  
  return rc;
}


void der_ctx_free( DerCtx ** ctx ) {
  if (!ctx) return;
  
  if (!*ctx) return;

  if ( (*ctx)->subs ) {
    uint i = 0;
    for(i = 0;i<(*ctx)->lsubs;++i) {
      if ( (*ctx)->subs[i].data) {
	free((*ctx)->subs[i].data);
	(*ctx)->subs[i].data = 0;
	(*ctx)->subs[i].ldata = 0;
      }
    }
    free( (*ctx)->subs);
    (*ctx)->subs=0;
  }
  
  zeromem(*ctx,sizeof(**ctx));
  free(*ctx);
  *ctx = 0;
  
}

DerRC der_begin( DerCtx ** ctx) {
  DerCtx * new = 0;
  
  if (!ctx) return DER_ARG;
  new = (DerCtx*)malloc(sizeof(*new));
  if (!new) return DER_MEM;
  zeromem(new,sizeof(*new));
  *ctx = new;

  return DER_OK;
}

 DerRC der_begin_seq(DerCtx ** ctx) {
  DerCtx * new = 0;
  if (!ctx) return DER_ARG;
  
  new = (DerCtx*)malloc(sizeof(*new));
  if (!new) return DER_MEM;
  zeromem(new,sizeof(*new));

  new->parent = *ctx;
  *ctx = new;

  return DER_OK;
};


DerRC der_end_seq( DerCtx ** ctx ) {

  DerRC rc = DER_OK;
  DerCtx * mine = 0;
  DerCtx * parent = 0;
  byte * mine_collapsed = 0;
  DerData * tmp;
  
  uint off = 0;
  uint tlen = 0;
  uint i = 0,idx=0;
  uint lres = 0;
  
  // check input
  if (!ctx) return DER_ARG;
  mine = *ctx;
  
  parent = mine->parent;

  // compute total length
  for(i = 0;i<mine->lsubs;++i) {
    tlen += mine->subs[i].ldata;
  }

  mine_collapsed = (byte*)-1;
  rc = der_encode_seq( 0, &lres, &off, mine_collapsed, tlen,0);
  if (rc != DER_OK) goto failure;
  mine_collapsed = 0;
  
  // allocate
  mine_collapsed = (byte*)malloc(lres);
  if (!mine_collapsed) {
    return DER_MEM;
  }
  zeromem(mine_collapsed,lres);

  idx = (lres-tlen);
  for(i = 0;i<mine->lsubs;++i) {
    mcpy(mine_collapsed+idx,mine->subs[i].data, mine->subs[i].ldata);
    idx += mine->subs[i].ldata;
  }
  
  off = 0;
  rc = der_encode_seq( mine_collapsed, &lres, &off, mine_collapsed+(lres-tlen), tlen, 0);
  if (rc != DER_OK) {
    goto failure;
  }

  tmp = parent->subs;
  parent->lsubs++;
  parent->subs = (DerData*)malloc(sizeof(DerData)*parent->lsubs);
  if (!parent->subs) {
    rc= DER_MEM;
    goto failure;
  }
  zeromem(parent->subs,  sizeof(DerData)*parent->lsubs);

  for(i = 0;i<parent->lsubs-1;++i) {
    parent->subs[i]=tmp[i];
  }

  if (tmp) {
    free(tmp);
    tmp = 0;
  }
  
  parent->subs[parent->lsubs-1].data = mine_collapsed;
  parent->subs[parent->lsubs-1].ldata= lres;

  *ctx = parent;
  der_ctx_free( &mine );

  return DER_OK;
 failure:
  if (mine_collapsed) {
    free(mine_collapsed);
    mine_collapsed = 0;
  }

  return rc;
};


 DerRC der_insert_uint( DerCtx * ctx, uint val) {

  uint i = 0;
  DerData * tmp = 0;
  DerRC rc = DER_OK;
  uint off = 0;                                 
  if (!ctx) return DER_ARG;

  tmp = ctx->subs;
  ++ctx->lsubs;
  ctx->subs = (DerData*)malloc(sizeof(DerData)*ctx->lsubs);
  if (!ctx->subs) {
    rc = DER_MEM;
    goto failure;
  }
  zeromem(ctx->subs, sizeof(DerData)*ctx->lsubs);

  for(i = 0;i<ctx->lsubs-1;++i) {
    ctx->subs[i]=tmp[i];
  }

  free(tmp);
  ctx->subs[ctx->lsubs-1].data = (byte*)malloc(6);
  if (!ctx->subs[ctx->lsubs-1].data) {
    rc = DER_MEM;
    goto failure;
  }
  zeromem(ctx->subs[ctx->lsubs-1].data,6);
  ctx->subs[ctx->lsubs-1].ldata = 6;

  rc = der_encode_integer(val, 
			  ctx->subs[ctx->lsubs-1].data, 
			  &off,
			  ctx->subs[ctx->lsubs-1].ldata);

  return rc;
 failure:
  return rc;
}

DerRC der_insert_cstr(DerCtx * ctx, const char * str) {
  DerRC rc = DER_OK;
  uint lstr = 0;
  while(str[lstr]) ++lstr;

  rc = der_insert_octetstring(ctx, (byte*)str, lstr+1);

  return rc;
}


DerRC der_insert_octetstring( DerCtx * ctx, byte * d, uint ld) {

  DerRC rc = DER_OK;
  uint off = 0;
  uint lout = 0;
  byte * out = 0;
  uint i = 0;
  DerData * tmp = 0;

  rc = der_encode_octetstring( d, ld, out, &off, lout );
  if (rc != DER_OK) return rc;

  out = (byte*)malloc(off);
  if (!out) return DER_MEM;
  lout = off;
  zeromem(out,lout);

  off = 0;
  rc = der_encode_octetstring(d, ld, out, &off, lout);
  if (rc != DER_OK) {
    rc = DER_MEM;
    goto failure;
  }

  tmp = ctx->subs;
  ctx->lsubs++;
  ctx->subs = (DerData*)malloc(sizeof(DerData)*(ctx->lsubs));
  if (!ctx->subs) {
    rc = DER_MEM;
    goto failure;
  }
  zeromem(ctx->subs,sizeof(DerData)*(ctx->lsubs));


  for(i = 0;i<ctx->lsubs-1;++i) {
    ctx->subs[i] = tmp[i];
  }

  if (tmp) {
    free(tmp);tmp=0;
  }

  ctx->subs[ctx->lsubs-1].data = out;
  ctx->subs[ctx->lsubs-1].ldata = lout;
  return DER_OK;
 failure:
  if (tmp) free(tmp);
  return rc;
}

 DerRC der_final( DerCtx ** ctx, byte * data, uint * ldata) {
  uint i = 0;
  uint tlen = 0;
  DerCtx * mine = 0;
  
  if (!ldata) {
    return DER_ARG;
  }

  if (!ctx) return DER_ARG;

  mine = *ctx;

  if (mine->parent != 0) {
    return DER_ARG;
  }

  for(i = 0;i<mine->lsubs;++i) {
    tlen += mine->subs[i].ldata;
  }
  


  if (data) {
    uint idx = 0;
    if (*ldata < tlen) return DER_SHORT;
    for(i = 0;i<mine->lsubs;++i) {
      mcpy(data+idx,mine->subs[i].data, mine->subs[i].ldata);
      idx += mine->subs[i].ldata;
    }
    *ldata = tlen;
    der_ctx_free( ctx );
    mine=0;
  } else {
    *ldata = tlen;
  }

  
  return DER_OK;
}

static 
uint peek_tag(byte * d) {
  byte tag = 0;
  if (!d) return 0;
  tag = d[0];
  
  if ((tag & 0x1F /*00011111*/) == 31) {
    return 0;
  }

  return tag;
}


DerRC der_begin_read(DerCtx ** ctx, byte * data, uint ldata) {
  DerCtx * res = 0;
  uint tag = 0;
  uint data_read = 0;
  DerRC rc = DER_OK;
  uint nsubs = 0;
  uint sub_no = 0;
  uint off = 0, ltmp = 0;
  byte * tmp = 0;
  

  if (!ctx) return DER_ARG;
  if (!data) return DER_ARG;

  res =  malloc(sizeof(*res));
  if (!res) return DER_MEM;

  tag = peek_tag(data);
  if (tag != SEQ) return DER_ARG;

  // compute number of subs
  while(rc == DER_OK) {
    rc = der_decode_seq(data, ldata, nsubs,
                        &off, 0, &ltmp);
    if (rc == DER_OK)
      ++nsubs;
  }

  res->subs = malloc(sizeof(DerData)*nsubs);
  if (!res->subs) return DER_MEM;
  res->lsubs = nsubs;

  off = 0;
  for(sub_no = 0; sub_no < nsubs;++sub_no) {
    rc = der_decode_seq(data, ldata, sub_no,
                        &off, 
                        &res->subs[sub_no].data,
                        &res->subs[sub_no].ldata);
	if (rc != DER_OK) goto failure;
  }

  rc = DER_OK;

  *ctx = res;
  return DER_OK;
 failure:
  return rc;
}



DerRC der_take_octetstring(DerCtx * ctx, uint idx, byte ** data, uint * ldata) {
  DerRC rc = 0;
  uint len = 0;
  uint off = 0, ltmp = 0, tag = 0;
  byte * tmp = 0;
  
  if (!ctx) return DER_ARG;
  if (!data) return DER_ARG;
  if (!ldata) return DER_ARG;
  if (ctx->lsubs < idx) return DER_ARG;
  
  tmp = ctx->subs[idx].data;
  ltmp = ctx->subs[idx].ldata;

  tag = peek_tag(tmp);
  if (tag != OCTET_STR_P) return DER_ARG; 

  off = 1;
  rc = get_len(&len, tmp, &off, ltmp);
  if (rc != DER_OK) return rc;
  
  *data = (tmp+off);
  *ldata = len;
  
  return DER_OK;
}


DerRC der_take_uint(DerCtx * ctx, uint idx, uint * v) {
  DerRC rc = DER_OK;
  uint tag = 0, ltmp = 0, off = 0;
  byte *tmp = 0;
  uint len = 0;
  

  if (!ctx) return DER_ARG;
  if (!v) return DER_ARG;
  if (idx > ctx->lsubs) return DER_ARG;
  
  tmp = ctx->subs[idx].data;
  ltmp = ctx->subs[idx].ldata;

  tag = peek_tag(tmp);
  if (tag != INT_P) return DER_ARG;
  
  off = 1;
  rc = get_len(&len, tmp, &off, ltmp);
  if (rc != DER_OK) return rc;

  if (len != 4) return DER_ARG;
  
  *v = b2i(tmp+2);
  return rc;
}



DerRC der_enter_seq(DerCtx**ctx, uint idx) {
  DerCtx * c =0;
  DerRC rc = DER_OK;
  uint tag = 0;
  DerCtx * result = 0;


  if (!ctx) return DER_ARG;
  if (!*ctx) return DER_ARG;
  
  c = *ctx;
  
  if (idx > c->lsubs) return DER_ARG;
  
  tag = peek_tag(c->subs[idx].data);
  if (tag != 0x30) return DER_ARG;

  rc = der_begin_read(&result,c->subs[idx].data, c->subs[idx].ldata);
  if (rc != DER_OK) return rc;

  result->parent = *ctx;

  *ctx = result;
  
  return rc;
}



DerRC der_leave_seq(DerCtx **ctx) {

  if (!ctx) return DER_ARG;
  if (!*ctx) return DER_ARG;

  *ctx = (*ctx)->parent;

  return DER_OK;
}



DerRC der_end_read(DerCtx ** ctx) {
  DerCtx * c = 0;
  if (!*ctx) return DER_OK;

  c = *ctx;
  if (c->subs) free(c->subs);
  free(c);

  *ctx = 0;
  return DER_OK;
}
