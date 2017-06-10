/*========================================================================

*//** @file os_timer_sp.h

OS abstracted timer functionality specific for linux

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
04/27/09   vma     Created file.

========================================================================== */

#ifndef _OS_TIMER_SP_H
#define _OS_TIMER_SP_H

#include <time.h>
#include "jpeg_common.h"
#include "os_types.h"
#include "os_int.h"
#include "jpeg_debug.h"

// Typedefs
typedef struct timespec os_timer_t;

#endif // _OS_TIMER_SP_H      
