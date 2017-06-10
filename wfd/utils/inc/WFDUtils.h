#ifndef _WFD_UTILS_H_
#define _WFD_UTILS_H_

/*==============================================================================
*       WFDUtils.h
*
*  DESCRIPTION:
*       Header file for WFDUtils
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

#include "OMX_Core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------
 A structure for storing custom information in an OMX_BUFFERHEADERTYPE
------------------------------------------------------------------------
*/

struct buff_hdr_extra_info
{
    OMX_TICKS   nEncDelTime;
    OMX_TICKS   nEncRcvTime;
    OMX_TICKS   nEncryptTime;
    OMX_TICKS   nMuxDelTime;
    OMX_TICKS   nMuxRcvTime;
    OMX_S64     nFrameNo;
    OMX_BOOL    bPushed;
};

void GetCurTime(OMX_TICKS& lTtime);

#ifdef __cplusplus
}
#endif

#endif //_WFD_UTILS_H_
