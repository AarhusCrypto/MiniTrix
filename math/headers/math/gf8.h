/*
 * Galouis field with 256 elements implemented on top of polynomial.h
 */
#ifndef GF8_H
#define GF8_H

Field GF8_new();

void GF8_destroy(Field * f);

#endif
