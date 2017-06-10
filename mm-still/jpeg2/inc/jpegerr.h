/*========================================================================

*//** @file jpegerr.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-09 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/03/09   vma     Switched to use the os abstraction layer (os_*)
04/23/09   vma     Added one error code: thumbnail is too big and is dropped. 
07/05/08   vma     Created file.

========================================================================== */

#ifndef _JPEGERR_H
#define _JPEGERR_H

/******************************************************************************
 * This file defines the standard errors thrown by the functions in the Jpeg 
 * library.
 *****************************************************************************/

#define JPEGERR_SUCCESS              0
#define JPEGERR_EFAILED              1
#define JPEGERR_EMALLOC              2
#define JPEGERR_ENULLPTR             3
#define JPEGERR_EBADPARM             4
#define JPEGERR_EBADSTATE            5
#define JPEGERR_EUNSUPPORTED         6
#define JPEGERR_EUNINITIALIZED       7
#define JPEGERR_TAGABSENT            8
#define JPEGERR_TAGTYPE_UNEXPECTED   9
#define JPEGERR_THUMBNAIL_DROPPED    10
#define JPEGERR_ETIMEDOUT            11

// MACRO for checking function return values
#define JPEG_FAILED(r)  (r != JPEGERR_SUCCESS)
#define JPEG_SUCCEEDED(r) (r == JPEGERR_SUCCESS)

#endif /* _JPEGERR_H */
