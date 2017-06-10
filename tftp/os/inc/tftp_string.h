/***********************************************************************
 * tftp_string.h
 *
 * TFTP String manupulation api's which are os agnostic.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * TFTP String manupulation api's which are os agnostic.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-06-04   rp    Switch to IPCRouter sockets.
2013-12-05   nr    Modify the server code to compile across all HLOS.
2013-11-21   nr    Abstract the OS layer across the OS's.
2013-11-19   nr    Create

===========================================================================*/

#ifndef __TFTP_STRING_H__
#define __TFTP_STRING_H__

#include "tftp_config_i.h"
#include "tftp_comdef.h"
#include "string.h"

#if defined (TFTP_WINDOWS_BUILD)
  #include <stdio.h>
  #define snprintf _snprintf
#endif


/**
  tftp_strlcat - Size bounded string concatenation.

  Concatenates the source string to destination string.

  This function ensures that the destination string will
  not be improperly terminated and that there will be
  no concatenation beyond the size of the destination buffer.

  @param[in,out]  dst   Destination buffer.
  @param[in]      src   Source string.
  @param[in]      siz   Size of the destination buffer in bytes.

  @return
  The length of the string that was attempted to be created,
  i.e. the sum of the source and destination strings.

  @dependencies
  None.
*/
size_t tftp_strlcat(char *dst, const char *src, size_t siz);


/**
  tftp_strlcpy - Size bounded string copy.

  Copies the source string to the destination buffer.

  This function ensures that the destination buffer will always
  be NULL terminated and that there will not be a copy beyond
  the size of the destination buffer.

  @param[out] dst   Destination buffer.
  @param[in]  src   Source String.
  @param[in]  siz   Size of the destination buffer in bytes.

  @return
  The length of the source string.

  @dependencies
  None.
*/
size_t tftp_strlcpy(char *dst, const char *src, size_t siz);

/**
  tftp_memscpy - Size bounded memory copy.

  Copies the source string to the destination buffer.

  This function ensures to copy only a maximum of destination buffer bytes.

  @param[out] dst       Destination buffer.
  @param[in]  dst_size  Destination buffer size.
  @param[in]  src       Source buffer.
  @param[in]  siz       Size of the source bytes to copy.

  @return
  The length of the copied bytes.

  @dependencies
  None.
*/
void tftp_memscpy(void *dst, size_t dst_size, const void *src, size_t src_siz);

/**
  tftp_memsmove - Size bounded memory move.

  Copies the source buffer to the destination buffer when the buffers could
  overlap.

  This function ensures to copy only a maximum of destination buffer bytes.

  @param[out] dst       Destination buffer.
  @param[in]  dst_size  Destination buffer size.
  @param[in]  src       Source buffer.
  @param[in]  siz       Size of the source bytes to copy.

  @return
  The length of the copied bytes.

  @dependencies
  None.
*/
void tftp_memsmove(void *dst, size_t dst_siz, const void *src, size_t src_siz);


#endif /* not __TFTP_STRING_H__ */
