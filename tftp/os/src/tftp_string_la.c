/***********************************************************************
 * tftp_string_la.c
 *
 * Short description.
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Verbose Description
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-10-14   rp    Use asserts for control-logic, debug-asserts for data-logic
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_string.h"
#include "tftp_assert.h"
#include "tftp_log.h"

#if !defined (TFTP_LA_BUILD)
  #error "This file should only be compiled in LA builds"
#endif

#ifdef TFTP_LE_BUILD_ONLY
  #ifdef USE_GLIB
      #include <glib.h>
      #define strlcpy g_strlcpy
      #define strlcat g_strlcat
  #endif
#endif /* #ifdef TFTP_LE_BUILD_ONLY */

size_t
tftp_strlcat(char *dst, const char *src, size_t siz)
{
  size_t new_len;

  new_len =  strlcat(dst, src, siz);
  TFTP_ASSERT(new_len < siz);

  return new_len;
}


size_t
tftp_strlcpy(char *dst, const char *src, size_t siz)
{
  size_t new_len;

  new_len =  strlcpy(dst, src, siz);
  TFTP_ASSERT(new_len < siz);

  return new_len;
}

void
tftp_memscpy(void *dst, size_t dst_size, const void *src, size_t src_size)
{
  TFTP_ASSERT (dst != NULL);
  TFTP_ASSERT (src != NULL);

  if ((dst == NULL) || (src == NULL))
  {
    return; /* TODO: Maybe FATAL?? */
  }

  TFTP_ASSERT (dst_size >= src_size);
  if (dst_size < src_size)
  {
    src_size = dst_size;
  }

  /* Check memory region overlap. We use the src_size for checks as that is the
   * max size that will be copied. Replace memscpy with memmove if you
   * encounter this ASSERT.  */
  if ((((uint8*)src <= (uint8*)dst) &&
       ((uint8*)dst < ((uint8*)src + src_size))) ||
      (((uint8*)dst <= (uint8*)src) &&
       ((uint8*)src < ((uint8*)dst + src_size))))
  {
    TFTP_LOG_ERR ("Memscpy on overlapping regions!");
    TFTP_ASSERT (0);
  }

  memcpy (dst, src, src_size);
}


void
tftp_memsmove(void *dst, size_t dst_size, const void *src, size_t src_size)
{
  TFTP_ASSERT (dst != NULL);
  TFTP_ASSERT (src != NULL);

  if ((dst == NULL) || (src == NULL))
  {
    return; /* TODO: Maybe FATAL?? */
  }

  TFTP_ASSERT (dst_size >= src_size);

  if (dst_size < src_size)
  {
    src_size = dst_size;
  }

  memmove (dst, src, src_size);
}
