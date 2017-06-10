/*==============================================================================
*       WFDMMUtils.cpp
*
*  DESCRIPTION:
*       Source file for WFDUtils
*
*
*  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
02/19/2014                    InitialDraft
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/

#include "WFDUtils.h"
#include "WFDMMLogs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=============================================================================

         FUNCTION:          GetCurTime

         DESCRIPTION:
*//**       @brief          Helper method to query current system time in micro
                            sec precision

*//**
@par     DEPENDENCIES:
                            None

*//*
         PARAMETERS:
*//**       @param[out]     lTime  Variable which will hold the current sys time

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

void GetCurTime(OMX_TICKS& lTime)
{
    static const OMX_S32 WFD_TIME_NSEC_IN_MSEC = 1000000;
    static const OMX_S32 WFD_TIME_NSEC_IN_USEC = 1000;
    struct timespec tempTime;
    clock_gettime(CLOCK_MONOTONIC, &tempTime);
    lTime =(OMX_TICKS)(((unsigned long long)tempTime.tv_sec *
                                            WFD_TIME_NSEC_IN_MSEC)
                     + ((unsigned long long)tempTime.tv_nsec /
                                             WFD_TIME_NSEC_IN_USEC));
}

#ifdef __cplusplus
}
#endif
