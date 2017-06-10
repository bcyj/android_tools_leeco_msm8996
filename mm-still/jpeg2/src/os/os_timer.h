/*========================================================================

*//** @file os_timer.h

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

#ifndef _OS_TIMER_H
#define _OS_TIMER_H

// include the os specific header file
#include "os_timer_sp.h"

// os timer methods
int os_timer_start(os_timer_t *p_timer);
int os_timer_get_elapsed(os_timer_t *p_timer, int *elapsed_in_ms, uint8_t reset_start);

#endif // _OS_TIMER_H      
