#ifndef __WFD_MM_SINK_COMMON_H__
#define __WFD_MM_SINK_COMMON_H__
/*==============================================================================
*       WFDMMLogs.h
*
*  DESCRIPTION:
*       Provides logging mechanism to WFD MM Modules
*
*
*  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
03/25/2013         SK            InitialDraft
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

/* =============================================================================

                     MACRO DEFINITIONS AND ENUMS

================================================================================
*/
#include "OMX_Core.h"
#include "OMX_IVCommon.h"
#include "AEEstd.h"

#define SINK_VIDEO_TRACK_ID  0
#define SINK_AUDIO_TRACK_ID  1

/*------------------------------------------------------------------------------
 For audio decoders Tunnel mode supercedes other modes
--------------------------------------------------------------------------------
*/
#ifdef USE_AUDIO_TUNNEL_MODE
#undef USE_OMX_AAC_CODEC
#endif

#define  FRAME_DROP_DROP_NONE 0
#define  FRAME_DROP_DROP_CURRENT_FRAME 1
#define  FRAME_DROP_DROP_TILL_IDR 2

/*------------------------------------------------------------------------------
 Can't find a documentation on this requirement from native window
--------------------------------------------------------------------------------
*/
#define NUM_BUFS_HELD_IN_NATIVE_WINDOW   2


#define CHECK_ARGS_RET_OMX_ERR_3(a, b, c)              \
if(!(a) || !(b) || !(c))                               \
    return OMX_ErrorBadParameter;                      \
/*ends CHECK_ARGS_RET_OMX_ERR_3 */

#define CHECK_ARGS_RET_OMX_ERR_4(a, b, c, d)           \
if(!(a) || !(b) || !(c) || !(d))                       \
    return OMX_ErrorBadParameter;                      \
/*ends CHECK_ARGS_RET_OMX_ERR_4 */

#define CHECK_ARGS_RET_OMX_ERR_5(a, b, c, d, e)        \
if(!(a) || !(b) || !(c) || !(d) || !(e))               \
    return OMX_ErrorBadParameter;                      \
/*ends CHECK_ARGS_RET_OMX_ERR_5 */

#define CHECK_ARGS_RET_VOID_1(a)                       \
if(!(a))                                               \
    return;                      \
/*ends CHECK_ARGS_RET_VOID_1 */

#define UNUSED(x) ((void)x)

typedef struct pictureInfo
{
    OMX_COLOR_FORMATTYPE eColorFmt;
    void                *pGraphicBuffer;
    OMX_CONFIG_RECTTYPE  rect;
    uint32               nWidth;
    uint32               nHeight;
}pictureInfoType;

typedef struct avInfo
{
    uint64  nBaseTime;
    uint64  nBaseTimeStream;
}avInfoType;

typedef enum WFDMMSinkEvent
{
    WFDMMSINK_ERROR,
    WFDMMSINK_PACKETLOSS,
    WFDMMSINK_DECRYPT_FAILURE
}WFDMMSinkEventType;

typedef enum WFDMMIDRTimerStatusType
{
    TIMER_RELEASED,
    TIMER_CREATED,
    TIMER_EXPIRED
}WFDMMIDRTimerStatus;

typedef enum videoFrameInfo
{
    FRAME_INFO_UNAVAILABLE,
    FRAME_INFO_I_FRAME,
    FRAME_INFO_P_FRAME
}videoFrameInfoType;

typedef void (*WFDMMSinkHandlerFnType)(void *pThis, OMX_U32 nModuleId,
                               WFDMMSinkEvent nEvent,
                               OMX_ERRORTYPE nStatus, int nData);
typedef void (*WFDMMSinkFBDType) (void* handle, int moduleIdx, int trackID,
                             OMX_BUFFERHEADERTYPE *pBuffer);

typedef void (*WFDMMSinkEBDType) (void* handle, int moduleIdx, int trackID,
                             OMX_BUFFERHEADERTYPE *pBuffer);



#endif /*__WFD_MM_SINK_COMMON_H__*/
