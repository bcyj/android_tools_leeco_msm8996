/* =======================================================================
                              wdsm_mm_source_interface.h
DESCRIPTION
  Definition of the wireless display session manager MM Source inteface.

   Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                                                 PERFORCE HEADER
   $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/mm-wfd-interface/inc/wdsm_mm_source_interface.h

========================================================================== */
/* =======================================================================
                             Edit History


========================================================================== */

#ifndef __WDSM_MM_SOURCE_INTERFACE_H__
#define __WDSM_MM_SOURCE_INTERFACE_H__
/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "wdsm_mm_interface.h"
#include "MMDebugMsg.h"
#include "OMX_Core.h"

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/**-----------------------------------------------------------------------------
    Macro definitions of the init params for file mux layer
--------------------------------------------------------------------------------  */
#ifndef MAX
#define         MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif

#ifndef MIN
#define         MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif

#ifdef __cplusplus
extern "C" {
#endif

//* query capability */
WFD_status_t wfd_get_extended_capability_source (WFD_MM_HANDLE hHandle, WFD_extended_capability_config_t *ext_capability);

WFD_status_t wfd_mm_get_video_capability_source(WFD_MM_HANDLE hHandle, WFD_video_codec_config_t* pVCfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_3d_video_capability_source(WFD_MM_HANDLE hHandle, WFD_3d_video_codec_config_t* pVCfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_lpcm_capability_source(WFD_MM_HANDLE, WFD_lpcm_codec_config_t *pLPCM_Cfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_aac_capability_source(WFD_MM_HANDLE, WFD_aac_codec_config_t *pAAC_Cfg, WFD_device_t tDevice);
//WFD_status_t wfd_mm_get_dts_codec_capability_source(WFD_MM_HANDLE hHandle, WFD_dts_codec_config_t* pDTS_Cfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_dolby_digital_codec_capability_source(WFD_MM_HANDLE hHandle, WFD_dolby_digital_codec_config_t *pDolby_Cfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_content_protection_capability_source(WFD_MM_HANDLE hHandle, WFD_content_protection_capability_config_t * pCP_Cfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_transport_capability_source(WFD_MM_HANDLE hHandle, WFD_transport_capability_config_t * pTS_Cfg, WFD_device_t tDevice);
WFD_status_t wfd_mm_get_negotiated_capability_source(WFD_MM_HANDLE hHandle, WFD_MM_capability_t* pMMCfg_local, WFD_MM_capability_t* pMMCfg_remote, WFD_MM_capability_t* pMMCfg_negotiated,WFD_MM_capability_t* pMMCfg_common);
//WFD_status_t wfd_mm_get_negotiated_capability_source(WFD_MM_HANDLE hHandle, WFD_MM_capability_t* pMMCfg_local, WFD_MM_capability_t* pMMCfg_remote, WFD_MM_capability_t* pMMCfg_negotiated);
WFD_status_t wfd_mm_get_local_capability_source(WFD_MM_HANDLE hHandle, WFD_MM_capability_t* pMMCfg, WFD_device_t);
WFD_status_t wfd_mm_get_proposed_capability_source(WFD_MM_HANDLE hHandle, WFD_MM_capability_t* pMMCfg_local, WFD_MM_capability_t* pMMCfg_remote, WFD_MM_capability_t* pMMCfg_proposed);
WFD_status_t wfd_mm_av_format_change_timing_source(WFD_MM_HANDLE, uint32*, uint32*);
WFD_status_t wfd_mm_update_session_source(WFD_MM_HANDLE, WFD_MM_capability_t *, wfd_mm_update_session_cb);

/* create/destroy */
WFD_status_t wfd_mm_create_session_source(WFD_MM_HANDLE *hHandle, WFD_device_t tDevice, WFD_MM_capability_t *pNegCap, WFD_MM_callbacks_t* pCallback);
WFD_status_t wfd_mm_destroy_session_source(WFD_MM_HANDLE hHandle);
WFD_status_t wfd_mm_send_IDRframe_source(WFD_MM_HANDLE hHandle);
WFD_status_t wfd_mm_set_framerate_source(WFD_MM_HANDLE hHandle,OMX_U32 nFrameRate);
WFD_status_t wfd_mm_set_bitrate_source(WFD_MM_HANDLE hHandle, OMX_U32 nBitRate);
WFD_status_t wfd_mm_send_runtime_command_source(WFD_MM_HANDLE hHandle,WFD_runtime_cmd_t eCommand);


/* stream control */
WFD_status_t wfd_mm_stream_play_source(WFD_MM_HANDLE hHandle, WFD_AV_select_t av_select, wfd_mm_stream_play_cb pCallback );
WFD_status_t wfd_mm_stream_pause_source(WFD_MM_HANDLE hHandle, WFD_AV_select_t av_select, wfd_mm_stream_pause_cb pCallback);

WFD_status_t wfd_mm_get_current_PTS_source(WFD_MM_HANDLE handle,uint64 *timeStamp);


WFD_status_t wfd_SendIframeNext(WFD_MM_HANDLE handle);
WFD_status_t wfd_ChangeBitRate(WFD_MM_HANDLE handle, OMX_S32 nBitRate);
/* hdcp session creation */
WFD_status_t wfd_mm_create_hdcp_session_source(WFD_MM_HANDLE *hHandle,WFD_MM_capability_t* pMMCfg_local,WFD_MM_capability_t* pMMCfg_remote,WFD_MM_capability_t* pMMCfg_negotiated );
#ifdef __cplusplus
}
#endif

#endif //__WDSM_MM_SOURCE_INTERFACE_H__
