#include <common.h>
#include <osal.h>
#ifndef DER_HH
#define DER_HH

#ifdef __cplusplus
extern "C" {
#endif

typedef uint DerRC;

  // everything is fine you can depend on output
#define DER_OK     ((DerRC)0)
  // one or more arguments were inconsistent with the required operation
#define DER_ARG    ((DerRC)1)
  // ran out of memory during operation
#define DER_MEM    ((DerRC)2)
  // bad data encountered for the given operation
#define DER_BAD    ((DerRC)3)
  // a output buffer is too short
#define DER_SHORT  ((DerRC)4)



/*!
 * encode end of content (eoc). write a zero into {out} at {off}. 
 *
 * \param out        - output buffer
 *
 * \param off        - offset into output buffer, after invocation off points
 *                     into out after the eoc sequence.
 *
 * \param lout       - length of the output buffer
 *
 * Notice we fail with DER_ARG if off >= lout
 */
DerRC der_encode_eoc(byte * out, uint * off, uint lout);

/*!
 * decode end of content (eof). read a zero from {out}+{off}.
 *
 * \param in         - output buffer
 *
 * \param off        - offset into output buffer, points after the eoc
 *                     sequence upon return.
 
 *
 * \param lin        - the length of the output buffer
 *
 * Notice we fail with DER_ARG if off >= lin We fail with DER_BAD if
 * the eoc sequence is not recognized at address {in}+{off}.
 */
DerRC der_decode_eoc(byte * in, uint * off, uint lin);

/*!
 * encode a boolean.
 *
 * \param value    - the value to encode
 *
 * \param out      - output where the encoded boolean will be
 *
 * \param off      - the offset 
 *
 * \param lout     - length of out
 *
 * \return DER_OK on succeesss, DER_ARG if lengths doesn't check out
 * or NULL pointer given, except when out is NULL then only off is
 * updated and may go beoynd {lout} to stipulate how much space is
 * required.
*/
DerRC der_encode_boolean(bool value, byte * out, uint * off, uint lout);

/*!
 * decode a boolean.
 *
 * \param value          - if successful *value is non zero if the boolean is
 *                         true, zero otherwise.
 *
 * \param in             - input
 *
 * \param off            - off into in
 *
 * \param lin            - length of in
 *
 * \return DER_OK on succeesss, DER_BAD if no boolean tag is found,
 * DER_ARG if lengths doesn't check out or NULL pointer given.
 */
DerRC der_decode_boolean(bool * value, byte *in, uint * off, uint lin);

/*!
 * encode an integer.
 *
 * \param value    - the value to encode
 *
 * \param out      - output where the encoded boolean will be
 *
 * \param off      - the offset 
 *
 * \param lout     - length of out
 *
 * \return DER_OK on succeesss, DER_ARG if lengths doesn't check out
 * or NULL pointer given, except when out is NULL then only off is
 * updated and may go beoynd {lout} to stipulate how much space is
 * required.
*/
DerRC der_encode_integer(int value, byte * out, uint * off, uint lout );

/*!
 * decode an integer.
 *
 * \param value          - if successful *value is the encoded integer
 *
 * \param in             - input
 *
 * \param off            - off into in
 *
 * \param lin            - length of in
 *
 * \return DER_OK on succeesss, DER_BAD if no boolean tag is found,
 * DER_ARG if lengths doesn't check out or NULL pointer given.
 */
DerRC der_decode_integer(int * value, byte * in, uint * off, uint lin );

/*!
 * encode a bitstring.
 *
 * \param v        - the value to encode
 *
 * \param lv       - the length of v
 *
 * \param out      - output where the encoded boolean will be
 *
 * \param off      - the offset 
 *
 * \param lout     - length of out
 *
 * \return DER_OK on succeesss, DER_ARG if lengths doesn't check out
 * or NULL pointer given, except when out is NULL then only off is
 * updated and may go beoynd {lout} to stipulate how much space is
 * required.
*/
DerRC der_encode_bitstring(byte * v, uint lv, byte * out, uint * off, uint lout );

/*!
 * decode a bit string.
 *
 * \param value          - if successful value points to the encoded bitstring
 *
 * \param in             - input
 *
 * \param off            - off into in
 *
 * \param lin            - length of in
 *
 * \return DER_OK on succeesss, DER_BAD if no boolean tag is found,
 * DER_ARG if lengths doesn't check out or NULL pointer given.
 */
DerRC der_decode_bitstring(byte * v, uint lv, byte * out, uint *off, uint lout);

/*!
 * encode a octet string.
 *
 * \param v        - the value to encode
 *
 * \param lv       - the length of v
 *
 * \param out      - output where the encoded boolean will be
 *
 * \param off      - the offset 
 *
 * \param lout     - length of out
 *
 * \return DER_OK on succeesss, DER_ARG if lengths doesn't check out
 * or NULL pointer given, except when out is NULL then only off is
 * updated and may go beoynd {lout} to stipulate how much space is
 * required.
*/
DerRC der_encode_octetstring(byte * v, uint lv, byte * out, uint * off, uint lout);

/*!
 * decode a octet (byte) string.
 *
 * \param value          - if successful value points to the encoded octet string
 *
 * \param in             - input
 *
 * \param off            - off into in
 *
 * \param lin            - length of in
 *
 * \return DER_OK on succeesss, DER_BAD if no boolean tag is found,
 * DER_ARG if lengths doesn't check out or NULL pointer given.
 */
DerRC der_decode_octetstring(byte * v, uint * lv, byte * in, uint * off, uint lin);

  /*!
   * \brief encode a DER sequence.
   *
   * \param result     - pointer to the result buffer (optional)
   * \param lresult    - pointer to the length of {result} will be updated
   * \param offset     - pointer to offset in result to start from
   * \param ...        - variable number of pairs of byte * and uint pointing 
   *                     to buffers that should go into this sequence.
   *
   * \return DER_OK on success. If result is null/zero only {lresult}
   * will be updated with the length required of {result} to encode
   * the given buffers to encode from the position of {offset}.
   *
   */
  DerRC der_encode_seq(byte *result, uint * lresult, uint *offset, ... );
  
  /*!
   * \brief decode a single item from a DER sequence.
   *
   * \param seq       - pointer to a DER sequence.
   * \param lseq      - the length of {seq}.
   * \param idx       - the item to decode.
   * \param offset    - the offset in {seq} to start looking for a sequence.
   * \param buffer    - (optional) will point into {seq} where the ith item starts
   * \param lbuffer   - the length of {buffer}.
   *
   * \return DER_OK on success. If {buffer} is null/zero only
   * {lbuffer} will be updated with the required length for {buffer}
   * to hold the item index'ed by idx.
   *
   */
  DerRC der_decode_seq(byte *seq,uint lseq, uint idx, 
		       uint * offset, byte ** buffer, uint *lbuffer);

  /*!
   * \brief update/append a sequence with data.
   *
   * \param result    - the sequence to update
   * \param offset    - will be updated to point after the last used byte
   * \param lresult   - the length of {result}
   * \param data      - the data to append
   * \param ldata     - length of {data}
   *
   *\return DER_OK on success
   */
  DerRC der_upd_seq( byte * result, uint * offset, uint lresult, 
		     byte * data, uint ldata );


  /*
   * Context Based Der Sequence builder
   */

  /*!
   * \brief datastructure DerData is the combination of a data pointer
   * and its length. Convinient when allocating a list of fragments as
   * in {der_begin_seq}.
   */
  typedef struct _der_data_ {
    byte * data;
    uint ldata;
    uint index;
  } DerData;

  /*!
   * \brief datastructure DerCtx which is a context for intermediate
   * state to which data items for a DER sequence are added until
   * finally collapsed into a single array with {der_final}.
   *
   */
  typedef struct _der_ctx_ {
    struct _der_ctx_ * parent;
    DerData * subs;
    uint lsubs;
  } DerCtx;

  /*!
   * \brief helper to free a DerCtx with all its members
   * Mostly for intertal use of this DER-module... 
   */
  void der_ctx_free( DerCtx ** ctx );
  
  /*!
   * \brief function is initiate a DER sequence on the given context.
   *
   * \param ctx - the current DER context, a new context may be
   *              returned.
   * \return DER_OK on success.
   */
  DerRC der_begin_seq(DerCtx ** ctx);

  /*!
   * \brief finalise a sequence. All the data fragments constituding
   * the sequence are collapsed into a freshly allocated array.
   *
   * \param ctx -  the current DER context.
   * 
   */
  DerRC der_end_seq( DerCtx ** ctx );

  /*!
   * \brief add and serialise an integer to the current context. The
   * 0x02 tag for int will be used.
   * 
   * \param ctx - the current DER context.
   * \param val - the integer value to add.
   *
   * \return DER_OK on success. 
   */
  DerRC der_insert_uint( DerCtx * ctx, uint val);

  /*!
   * \brief add and serialise an octetstring (aka byte array) to the
   * sequence. The data is copied and {d} can be free or zeroed after
   * this invocation with out disrupting later actions with the
   * context, {ctx}.
   *
   * \param ctx  - the current DER context.
   * \param d    - pointer to data.
   * \param ld   - length of {d}
   *
   * \return DER_OK on success.
   */
  DerRC der_insert_octetstring( DerCtx * ctx, byte * d, uint ld);
  DerRC der_insert_cstr(DerCtx * ctx, const char * str);

  /*!
   * \brief this function allocates a fresh context to start with.
   * 
   * \param ctx  - the context {ctx} to which we can add stuff.
   *
   * \return on success DER_OK.
   */
  DerRC der_begin( DerCtx ** ctx);

  /*!
   * \brief this function finalises the context {ctx} and frees it if
   * data is not zero and data will point to the resulting DER
   * sequence. If data is zero, the length required to serialise all
   * elements inserted into {ctx} is written to *{ldata}.
   *
   * \param ctx  - the context to finalise.
   * \param data - pointer to buffer when the result is written.
   * \param ldata- pointer to the length of {data}. After invocation {ldata} 
   *               points to the number of bytes acutally used by {data}.
   *
   * \return DER_OK on success.
   */
  DerRC der_final( DerCtx ** ctx, byte * data, uint * ldata);

  /*!
   *
   * \breif reads the bytes in {data} and constructs a DER context for
   * use with the take, leave and enter functions follwing.
   *
   * \param ctx   - *ctx will point to a context after invocation.
   * \param data  - the data source to decode from
   * \param ldata - the length of {data}.
   *
   * \return DER_OK on success.
   */
  DerRC der_begin_read(DerCtx ** ctx, byte * data, uint ldata);

  /*!
   * \brief decodes an unsigned integer from the context {ctx}. 
   *
   * \param ctx - the DerCtx to decode from
   * \param res - point to an int when the result will be stored on succes.
   *
   * \return DER_OK on success
   */
  DerRC der_take_uint(DerCtx * ctx, uint idx, uint * res);


  /*!
   * \brief take an octet string from the context.
   *
   * \param ctx - the der context to read an octet string from.
   * \param data - pointer to the data read on success.
   * \param ldata- the length of {data}
   *
   * \return DER_OK on success.
   */
  DerRC der_take_octetstring(DerCtx * ctx, uint idx, byte ** data, uint * ldata);



  /*!
   *
   * \brief enters a sequence.
   *
   * \param ctx - updates the *{ctx} for the sequence entered.
   *
   * \return DER_OK on success.
   */
  DerRC der_enter_seq(DerCtx ** ctx, uint idx);

  /*!
   * \breif leaves a sequence.
   *
   * \param ctx - the *{ctx} is updated for the parent sequence.
   *
   * \return DER_OK on success.
   */
  DerRC der_leave_seq(DerCtx ** ctx);

  /*!
   * \brief free resources allocated for *{ctx} and set *{ctx} to zero.
   *
   * \param ctx - the DER context to free.
   */
  DerRC der_end_read(DerCtx ** ctx);


  typedef struct _der_input_stream_ {
	  RC(*read_int)(ull * v);
	  RC(*read_str)(byte ** str, uint * lstr);
	  RC(*peek_tag)(ull * tag);
	  RC(*begin_seq)();
	  RC(*leave_seq)();
	  void * impl;
  } *DerInputStream;



  /*!
   * Destroy the CooV4 version of the DER reader.
   *
   */
  DerInputStream DerInputStream_New(OE oe, FD in);
  void DerInputStream_Destroy(DerInputStream * s);
#ifdef __cplusplus
}
#endif

#endif
