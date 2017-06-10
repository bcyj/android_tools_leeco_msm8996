/*========================================================================

*//** @file os_timer_sp.c

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

#include "os_timer.h"

int os_timer_start(os_timer_t *p_timer)
{
    if (!p_timer)
        return JPEGERR_ENULLPTR;

    if (clock_gettime(CLOCK_REALTIME, p_timer))
        return JPEGERR_EFAILED;

    return JPEGERR_SUCCESS;
}

int os_timer_get_elapsed(os_timer_t *p_timer, int *elapsed_in_ms, uint8_t reset_start)
{
    os_timer_t now;
    long diff;
    int rc = os_timer_start(&now);

    if (JPEG_FAILED(rc))
        return rc;

    diff = (long)(now.tv_sec - p_timer->tv_sec) * 1000;
    diff += (long)(now.tv_nsec - p_timer->tv_nsec) / 1000000;
    *elapsed_in_ms = (int)diff;

    if (reset_start)
        *p_timer = now;

    return JPEGERR_SUCCESS;
}

