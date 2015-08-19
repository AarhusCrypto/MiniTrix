#include <common.h>
#ifndef HEX_H
#define HEX_H

#ifdef __cplusplus
extern "C" {
#endif




/*!
 * Convert a hex string of length 2 into one byte.
 *
 * \param hex - two hex characters in ASCII both upper and lower case
 *              are accepted.
 *
 * \return byte representing the value stipulated by {hex}
 */
byte h2b(char * hex);

/*!
 * Convert a byte into a two character ASCII Hex string.
 *
 * \param b      - byte to convert.
 * \param hex    - at least two bytes of memory for output.
 *
 */
void b2h(byte b, char * hex);


/*!
 * Convert the even length hex string in {hex} into a byte sequence
 * written to {res}.
 *
 * hex must have length 2*{lres}
 *
 * \param hex    - at least 2*{lres} hex character string in ASCII
 * \param res    - output 
 * \param lres   - length of {res}
 *
 */
void hs2bs(char * hex, byte * res, uint lres);

/*!
 * Convert the given byte sequence of at least {lres}/2 bytes into a
 * hex string in {res}.
 *
 * \param bytes      - data to convert to hex
 * \param res        - output
 * \param lres       - length of {res}
 *
 */
void bs2hs(byte * bytes, char * res, uint lres);

/*!
 * Dump the given {data} on stdout with width bytes per line.
 */
void dump_data_as_hex(byte * data, uint ldata, uint width);

/*!
 * Print data as a hex string to stdout.
 */
void print_data_as_hex(byte * data, uint ldata);

void _p(const char * s, byte * d, uint ld, uint w);

#ifdef __cplusplus
}
#endif

#endif
