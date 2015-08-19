/*
 * rnd.h
 *
 *  Created on: Jan 9, 2015
 *      Author: rwl
 */

#ifndef OSAL_INCLUDE_RND_H_
#define OSAL_INCLUDE_RND_H_
#include <common.h>
#include <osal.h>

typedef struct _random_source_{
	/**
	 * Generate {ldata} random bytes and put then in {data}.
	 *
	 */
	void (*rand)(byte * data, uint ldata);

	void * impl;
} *Rnd;

Rnd LibcWeakRandomSource_New(OE oe);

Rnd IntelRandomSource_New(OE oe);

void LibcWeakRandomSource_Destroy(Rnd * p_rnd);

void IntelRandomSource_Destroy(Rnd * p_rnd);

#endif /* OSAL_INCLUDE_RND_H_ */
