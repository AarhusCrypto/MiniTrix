/*
 * rnd.c
 *
 *  Created on: Jan 9, 2015
 *      Author: rwl
 */
#include <rnd.h>
#include <osal.h>
#include <stdlib.h>
#include <coov4.h>
#include <time.h>

COO_DEF(Rnd,void,libc_rand,byte * data, uint ldata)
	srand(time(0));
	if (data != 0) {
		uint i = 0;
		for (i = 0;i < ldata;i+=4) {
			uint r = rand();
			uint j = 0;
			for(j = 0;j < 4 && i+j < ldata;++j) {
				uint mask = (0xFF << j*8);
				data[i+j] = (byte)((r & mask) >> j*8);
			}
		}
	}
}

Rnd LibcWeakRandomSource_New(OE oe) {
	Rnd rnd = (Rnd)oe->getmem(sizeof(*rnd));

	rnd->rand = COO_attach(rnd, Rnd_libc_rand);
	rnd->impl = oe;

	return rnd;
}

Rnd IntelRandomSource_New(OE oe) {
	oe->syslog(OSAL_LOGLEVEL_FATAL,"Intel random source not implemented yet !");
	return 0;
}

void LibcWeakRandomSource_Destroy(Rnd * p_rnd) {
	Rnd r = 0;
	OE oe = 0;
	if (p_rnd == 0) return;

	r = *p_rnd;
	oe = r->impl;

	COO_detach(rand);

	oe->putmem(r);
	*p_rnd = 0;

}

void IntelRandomSource_Destroy(Rnd * p_rnd) {

}

