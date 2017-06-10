#ifndef __HTTPMMICOMPONENT_H__
#define __HTTPMMICOMPONENT_H__
/************************************************************************* */
/**
 * HTTPMMIComponent.h
 * @brief Header file for HTTPMMIComponent.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/inc/HTTPMMIComponent.h#5 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPMMIComponent.h
** ======================================================================= */
#include "OMX_Types.h"
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

#define MMI_HTTP_COMPONENT_NAME  "OMX.qcom.source.httpstream"
#define MMI_HTTP_COMPONENT_UUID   "UUID_OMX.QCOM.SOURCE.HTTPSTREAM.1.0.0"

/* Numbers of port defines for different domain */
#define MMI_HTTP_NUM_VIDEO_PORTS (0x1)
#define MMI_HTTP_NUM_AUDIO_PORTS (0x1)
#define MMI_HTTP_NUM_IMAGE_PORTS (0x1)
#define MMI_HTTP_NUM_OTHER_PORTS (0x1)

/* port index definition for mmi demuxer */
#define MMI_HTTP_PORT_START_INDEX (0x01)
#define MMI_HTTP_AUDIO_PORT_INDEX MMI_HTTP_PORT_START_INDEX
#define MMI_HTTP_VIDEO_PORT_INDEX (MMI_HTTP_NUM_AUDIO_PORTS + \
                                   MMI_HTTP_AUDIO_PORT_INDEX)
#define MMI_HTTP_IMAGE_PORT_INDEX (MMI_HTTP_NUM_VIDEO_PORTS + \
                                   MMI_HTTP_VIDEO_PORT_INDEX)
#define MMI_HTTP_OTHER_PORT_INDEX (MMI_HTTP_NUM_IMAGE_PORTS + \
                                   MMI_HTTP_IMAGE_PORT_INDEX)

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
#ifdef __cplusplus
extern "C" {
#endif
  OMX_ERRORTYPE QOMX_MMIV_HTTP_ComponentInit(OMX_IN OMX_HANDLETYPE hComponent);
#ifdef __cplusplus
}
#endif

#endif /* __HTTPMMICOMPONENT_H__ */
