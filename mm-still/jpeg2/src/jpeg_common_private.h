/*========================================================================

*//** @file jpeg_common_private.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
07/23/10   sigu    Fixing compiler warnings
08/03/09   vma     Switched to use the os abstraction layer (os_*)
09/07/08   vma     Created file.
========================================================================== */

#ifndef __JPEG_COMMON_PRIVATE_H__
#define __JPEG_COMMON_PRIVATE_H__

#include "jpeg_common.h"
#include "jpeg_debug.h"
#include "os_thread.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#ifndef JPEG_FREE
#define JPEG_FREE(p)               {if (p) jpeg_free(p); p = NULL;}
#endif /*JPEG_FREE*/

#ifndef JPEG_MALLOC
#define JPEG_MALLOC(s)             jpeg_malloc(s, __FILE__, __LINE__)
#endif /*JPEG_MALLOC*/

#define STD_MEMSET(p,i,n)          memset(p, i, n)

#ifndef STD_MIN
#define STD_MIN(a,b)               ((a < b) ? a : b)
#endif /*STD_MIN*/

#ifndef STD_MAX
#define STD_MAX(a,b)               ((a > b) ? a : b)
#endif /*STD_MAX*/

#define CLAMP(x,min,max)           {if ((x) < (min)) (x) = (min); \
                                    if ((x) > (max)) (x) = (max);}
#define STD_MEMMOVE(dest,src,size) memcpy(dest, src, size)

#define ROUND_TO_8(x)          ((((x) + 7) >> 3) << 3)
#define ROUND_TO_16(x)         ((((x) + 15) >> 4) << 4)

#ifndef STD_ZEROAT
#define STD_ZEROAT(p)               memset((p), 0, sizeof(*p))
#endif /*STD_ZEROAT*/



#endif // __JPEG_COMMON_PRIVATE_H__
