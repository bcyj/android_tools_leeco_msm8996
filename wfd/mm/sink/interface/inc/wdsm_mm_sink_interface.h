
/*==============================================================================
*        @file wdsm_mm_sink_interface.h
*
*  @par DESCRIPTION:
*       Definition of the wireless display session manager MM Sink inteface
*
*
*  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:

===============================================================================*/
#ifndef __WDSM_MM_SINK_INTERFACE_H__
#define __WDSM_MM_SINK_INTERFACE_H__


/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "wdsm_mm_interface.h"

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#ifdef __cplusplus
extern "C" {
#endif

/* query capability */
WFD_status_t wfd_mm_get_video_capability_sink(WFD_MM_HANDLE hHandle, WFD_video_codec_config_t* pVCfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_3d_video_capability_sink(WFD_MM_HANDLE hHandle, WFD_3d_video_codec_config_t* pVCfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_lpcm_capability_sink(WFD_MM_HANDLE, WFD_lpcm_codec_config_t *pLPCM_Cfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_aac_capability_sink(WFD_MM_HANDLE, WFD_aac_codec_config_t *pAAC_Cfg, WFD_device_t tDevice);
//WFD_status_t wfd_mm_get_dts_codec_capability_sink(WFD_MM_HANDLE hHandle, WFD_dts_codec_config_t* pDTS_Cfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_dolby_digital_codec_capability_sink(WFD_MM_HANDLE hHandle, WFD_dolby_digital_codec_config_t *pDolby_Cfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_content_protection_capability_sink(WFD_MM_HANDLE hHandle, WFD_content_protection_capability_config_t * pCP_Cfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_transport_capability_sink(WFD_MM_HANDLE hHandle, WFD_transport_capability_config_t * pTS_Cfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_local_capability_sink(WFD_MM_HANDLE hHandle, WFD_MM_capability_t* pMMCfg, WFD_device_t);

/*query extended capability*/
WFD_status_t wfd_get_extended_capability_sink(WFD_MM_HANDLE hHandle, WFD_extended_capability_config_t *ext_capability);

/* create/destroy */
WFD_status_t wfd_mm_create_session_sink(WFD_MM_HANDLE *hHandle, WFD_device_t tDevice, WFD_MM_capability_t *pNegCap, WFD_MM_callbacks_t* pCallback);
WFD_status_t wfd_mm_destroy_session_sink(WFD_MM_HANDLE hHandle);

/* stream control */
WFD_status_t wfd_mm_stream_play_sink(WFD_MM_HANDLE hHandle, WFD_AV_select_t av_select, wfd_mm_stream_play_cb pCallback );
WFD_status_t wfd_mm_stream_pause_sink(WFD_MM_HANDLE hHandle, WFD_AV_select_t av_select, wfd_mm_stream_pause_cb pCallback);


WFD_status_t wfd_mm_create_hdcp_session_sink(WFD_MM_HANDLE *hHandle,WFD_MM_capability_t* pMMCfg_local,
                                              WFD_MM_capability_t* pMMCfg_remote,WFD_MM_capability_t* pMMCfg_negotiated );

WFD_status_t wfd_mm_destroy_hdcp_session_sink(WFD_MM_HANDLE hHandle);

/*Timer Handler for IDR Req*/
void wfd_mm_idr_request_sink(void *);

WFD_status_t wfd_mm_update_session_sink
(       WFD_MM_HANDLE hHandle,
        WFD_MM_capability_t *WFD_negotiated_capability,
        wfd_mm_update_session_cb
);

WFD_status_t wfd_mm_av_stream_control_sink( WFD_MM_HANDLE, WFD_MM_AV_Stream_Control_t, int64);
#ifdef __cplusplus
}
#endif

#endif //__WDSM_MM_SINK_INTERFACE_H__
