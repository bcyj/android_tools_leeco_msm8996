/*========================================================================

*//** @file jpeg_debug.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/03/09   vma     Switched to use the os abstraction layer (os_*)
09/07/08   vma     Created file.
========================================================================== */

#ifndef __JPEG_DEBUG_H__
#define __JPEG_DEBUG_H__

#include "os_types.h"
#include "os_int.h"

//#define _DEBUG

#ifdef _DEBUG

// memory leak related debugging methods
void *jpeg_malloc(uint32_t size, const char* file_name, uint32_t line_num);
void  jpeg_free(void *ptr);
void  jpeg_show_leak(void);

#else

#define jpeg_malloc(s,f,l)  malloc(s)
#define jpeg_free(p)    free(p)
#define jpeg_show_leak()

#endif

#endif // __JPEG_DEBUG_H__
