/*!
 *
 *
 * \file This file contains utilities for handling files.
 *
 *
 */
#ifndef FS_H
#define FS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <osal.h>

/*!
 * \brief returns the size of the behind filename. 
 *
 * Note: Zero can mean two things, either an error occurred or the
 * file is actually empty (containing zero bytes).
 *
 * \param filename - the filename to probe the size of
 *
 * \return the size in bytes of the file, or zero on failure.
 */
uint read_file_size(const char * filename);

/*!
 * \brief read entire file.
 *
 * \return the number of bytes actually read.
 */
uint read_entire_file(const char * filename, byte * buf, uint lbuf);

#ifdef __cplusplus
}
#endif

#endif
