/*==============================================================================
*        @file wdsm_mm_sink_interface.cpp
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


/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "wdsm_mm_sink_interface.h"
#include "MMMemory.h"
#include "MMDebugMsg.h"
#include "MMTimer.h"
#include "WFDMMSink.h"
#include "wfd_netutils.h"

#include <media/mediaplayer.h>
#include <binder/ProcessState.h>
#ifdef WFD_ICS
#include <surfaceflinger/Surface.h>
#else
#include <Surface.h>
#endif

#define WFD_SINK_RTP_PORT  1234
#define WFD_SINK_HDCP_PORT 9876

#include "wfd_cfg_parser.h"
#define FEATURE_CONFIG_FROM_FILE
uint32 idr_interval_sink;
uint32 av_format_chnage_interval_sink;

using namespace android;
MM_HANDLE sinkTimerHandle = NULL;


typedef struct {
    WFD_MM_capability_t negotiated_capability;
    wfd_mm_capability_change_cb pCapability_change_cb;
    wfd_mm_request_IDRframe_cb pIDR_Request_cb;
    wfd_mm_stream_play_cb pStream_play_cb;
    wfd_mm_stream_pause_cb pStream_pause_cb;
    WFD_AV_select_t av_select;
    sp<MediaPlayer> pMP;
} wfd_mm_sink_context_t;


extern "C" {

/*Declare utility functions*/
static int32 wfd_mm_construct_session_url( char* url, int32 urlSize, WFD_MM_capability_t* pMMCfg_negotiated);
static void wfd_mm_session_cleanup(wfd_mm_sink_context_t* pCtx);


/** @brief Get video capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_video_codec_config_t - pointer to video capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_video_capability_sink(WFD_MM_HANDLE,
                                              WFD_video_codec_config_t* video_codec_config, WFD_device_t)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_get_video_capability_sink");
  if((video_codec_config) && (video_codec_config->h264_codec))
  {
    //std_memset(video_codec_config->h264_codec, 0, (sizeof(WFD_3d_h264_codec_config_t) * WFD_MAX_NUM_H264_PROFILES));
    video_codec_config->native_bitmap = 0;
    video_codec_config->preferred_display_mode_supported = 1;
    //Currently we are populating only one profile we will add all the profiles later
    video_codec_config->num_h264_profiles = 1;
    //populate the first profile details
    ((video_codec_config->h264_codec) + 0)->supported_cea_mode = 0;
    ((video_codec_config->h264_codec) + 0)->supported_vesa_mode = 0;
    ((video_codec_config->h264_codec) + 0)->supported_hh_mode = 1;//WVGA
    ((video_codec_config->h264_codec) + 0)->h264_level = 2;//Level 3.2
    ((video_codec_config->h264_codec) + 0)->h264_profile = 1;//Profile CBP
    ((video_codec_config->h264_codec) + 0)->min_slice_size = 0;
    ((video_codec_config->h264_codec) + 0)->decoder_latency = 0;
    ((video_codec_config->h264_codec) + 0)->frame_rate_control_support = 17;
    ((video_codec_config->h264_codec) + 0)->max_vres = 480;
    ((video_codec_config->h264_codec) + 0)->max_hres = 800;
    ((video_codec_config->h264_codec) + 0)->slice_enc_params = 0;

    return WFD_STATUS_SUCCESS;
  }
  return WFD_STATUS_BADPARAM;
}

/** @brief Get 3d capability parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_3d_video_codec_config_t - pointer to 3d video capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_3d_video_capability_sink(WFD_MM_HANDLE, WFD_3d_video_codec_config_t* threed_video_codec_config, WFD_device_t)
{
  (void) threed_video_codec_config;
  return WFD_STATUS_SUCCESS;
}


/** @brief Get lpcm capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_lpcm_codec_config_t - pointer to lpcm capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_lpcm_capability_sink(WFD_MM_HANDLE, WFD_lpcm_codec_config_t *lpcm_codec_config, WFD_device_t wfd_device_type)
{
  (void)wfd_device_type;
  if( lpcm_codec_config )
  {
   lpcm_codec_config->decoder_latency = 0;
   lpcm_codec_config->supported_modes_bitmap = 2;  //LPCM_48_16_2
   return WFD_STATUS_SUCCESS;
  }
  return WFD_STATUS_BADPARAM;
}

/** @brief Get aac capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_aac_codec_config_t - pointer to aac capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_aac_capability_sink(WFD_MM_HANDLE, WFD_aac_codec_config_t *aac_codec_config, WFD_device_t wfd_device_type)
{
  (void) wfd_device_type;
  if(aac_codec_config)
  {
    aac_codec_config->decoder_latency = 0;
    aac_codec_config->supported_modes_bitmap = 0; //AAC_48_16_2
    return WFD_STATUS_SUCCESS;
  }
  return WFD_STATUS_BADPARAM;
}

/** @brief Get dolby digital codec capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_dolby_digital_codec_config_t - pointer to dolby digital codec capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_dolby_digital_codec_capability_sink(WFD_MM_HANDLE, WFD_dolby_digital_codec_config_t *dolby_digital_codec, WFD_device_t wfd_device_type)
{
   (void) wfd_device_type;
   if(dolby_digital_codec)
   {
      dolby_digital_codec->supported_modes_bitmap = 1;//DOLBY_DIGITAL_48_16_2_AC3
      dolby_digital_codec->decoder_latency = 0;
      return WFD_STATUS_SUCCESS;
   }
   return WFD_STATUS_BADPARAM;
}

/** @brief Get content protection capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_content_protection_capability_config_t - pointer to content protection capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_content_protection_capability_sink(WFD_MM_HANDLE, WFD_content_protection_capability_config_t * content_protection_capability_config, WFD_device_t wfd_device_type)
{
   (void) wfd_device_type;
   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "wfd_mm_get_content_protection_capability_sink");
   WFD_status_t ret_status = WFD_STATUS_BADPARAM;

   if( content_protection_capability_config )
   {
       memset(content_protection_capability_config, 0,sizeof(WFD_content_protection_capability_config_t) );
       content_protection_capability_config->content_protection_ake_port = WFD_SINK_HDCP_PORT;
       content_protection_capability_config->content_protection_capability = WFD_HDCP_2_1;
       ret_status = WFD_STATUS_SUCCESS;
   }
   return ret_status;
}

/** @brief Get transport capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_transport_capability_config_t - pointer to transport capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_transport_capability_sink(WFD_MM_HANDLE, WFD_transport_capability_config_t *transport_capability_config, WFD_device_t wfd_device_type)
{
   (void) transport_capability_config;
   (void) wfd_device_type;
   return WFD_STATUS_SUCCESS;
}

/** @brief Get local MM capability structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Sink instance handle
               WFD_MM_capability_t - pointer to local capability structure
                WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_local_capability_sink(WFD_MM_HANDLE hHandle,
                                                          WFD_MM_capability_t* pMMCfg,
                                                          WFD_device_t tDevice)
{
   (void)hHandle;
   (void)tDevice;
   resetCfgItems();
  if(pMMCfg)
  {

   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_get_local_capability_sink entry pMMCfg =%x",
                pMMCfg);
 #ifdef FEATURE_CONFIG_FROM_FILE
    FILE * fp = NULL;
    fp = fopen(WFD_CFG_FILE_SINK,"r");
    if(fp)
    {
      readConfigFile readCfgCapabilities;

      readCfgCapabilities.pCfgCapabilities = pMMCfg;
      parseCfg(WFD_CFG_FILE_SINK,&readCfgCapabilities);

      pMMCfg = readCfgCapabilities.pCfgCapabilities;

      if( readCfgCapabilities.idrRequestCapability.idrReqValid)
      {
         idr_interval_sink = readCfgCapabilities.idrRequestCapability.idrIntvl;
      }
      fclose(fp);
    }
    else
#endif
    {
      pMMCfg->audio_config.aac_codec.decoder_latency = 0;
      pMMCfg->audio_config.aac_codec.supported_modes_bitmap = 1;  //AAC 48, 16, 2
      pMMCfg->audio_config.lpcm_codec.decoder_latency = 0;
      pMMCfg->audio_config.lpcm_codec.supported_modes_bitmap = 2;  //LPCM_48_16_2
      // PF3_SUPPORT  AC3 for testing the decoder code
      pMMCfg->audio_config.dolby_digital_codec.decoder_latency = 0 ;
      pMMCfg->audio_config.dolby_digital_codec.supported_modes_bitmap = 1;  //AC3_48_16_2
      pMMCfg->video_method = WFD_VIDEO_H264;
      pMMCfg->video_config.video_config.num_h264_profiles = 1;
      pMMCfg->video_config.video_config.native_bitmap = 2;
      pMMCfg->video_config.video_config.preferred_display_mode_supported = 0;
      //populate the first profile details
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->supported_cea_mode = 1;
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->supported_vesa_mode = 0;
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->supported_hh_mode = 1; //WVGA
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->h264_level = 2;//Level 3.2
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->h264_profile = 1;//Profile CBP
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->min_slice_size = 0;
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->decoder_latency = 0;
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->frame_rate_control_support = 17;
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->max_vres = 480;
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->max_hres = 800;
      ((pMMCfg->video_config.video_config.h264_codec) + 0)->slice_enc_params = 0;
       pMMCfg->standby_resume_support = TRUE;
       pMMCfg->content_protection_config.content_protection_capability = 0;
    }

    if ( ((WFD_HDCP_version_t)pMMCfg->content_protection_config.content_protection_capability >=
                               WFD_HDCP_VERSION_2_0) &&
          ((WFD_HDCP_version_t)pMMCfg->content_protection_config.content_protection_capability <=
                                WFD_HDCP_VERSION_MAX))
    {
        pMMCfg->content_protection_config.content_protection_ake_port = WFD_SINK_HDCP_PORT;
    }

    int tempRtpPortNo = 0;
    int tempRtcpPortNo = 0;
    getIPSockPair(true, &pMMCfg->transport_capability_config.rtpSock,
                        &pMMCfg->transport_capability_config.rtcpSock,
                        &tempRtpPortNo,
                        &tempRtcpPortNo, false );
    pMMCfg->transport_capability_config.port1_id = (uint16)tempRtpPortNo;
    pMMCfg->transport_capability_config.port1_rtcp_id = (uint16)tempRtcpPortNo;
    close(pMMCfg->transport_capability_config.rtpSock);
    close(pMMCfg->transport_capability_config.rtcpSock);
    pMMCfg->transport_capability_config.rtpSock = -1;
    pMMCfg->transport_capability_config.rtcpSock = -1;

    if(pMMCfg->transport_capability_config.port1_id == 0)
    {
        pMMCfg->transport_capability_config.port1_id = WFD_SINK_RTP_PORT;
    }
    return WFD_STATUS_SUCCESS;
  }
  return WFD_STATUS_BADPARAM;
}

/** @brief Get extended capabity parameters structure
*
* @param[in] WFD_MM_HANDLE - WFD MM Sink instance handle (not used)
             WFD_extended_capability_config_t* - pointer to extended capability structure
*
* @return  WFD_status_t - status
*/
WFD_status_t wfd_get_extended_capability_sink (WFD_MM_HANDLE hHandle,
                                                 WFD_extended_capability_config_t *ext_capability)
{
  (void)hHandle;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "wfd_get_extended_capability_sink");
  WFD_status_t ret_status = WFD_STATUS_BADPARAM;
  if (ext_capability != NULL)
  {
    ext_capability->sub_element_id         = WFD_EXTENDED_CAPABILITY_SUB_ELEMENT_ID; //fixed for extended capability
    ext_capability->length                 = WFD_EXTENDED_CAPABILITY_SUB_ELEMENT_LENGTH; //fixed 2 bytes
    ext_capability->ext_capability_bitmap  = WFD_EXTENDED_CAPABILITY_UIBC; //enable only UIBC, based on Table  5.27, spec 1.20

    ret_status = WFD_STATUS_SUCCESS;
  }

  return ret_status;
}


/** @brief create MM session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_device_t - device type either primary/secondary
               WFD_MM_capability_t - pointer to negotiated capabilty structure
  *            wfd_mm_capability_change_cb  - Callback to inform session manager
  *             if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_create_session_sink(
                    WFD_MM_HANDLE *pHandle,
                    WFD_device_t eWFD_device,
                    WFD_MM_capability_t *pWFD_negotiated_capability,
                    WFD_MM_callbacks_t *pCallBack)
{
    WFD_status_t nErr = WFD_STATUS_FAIL;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_create_session_sink");

    //ToDo:Need to check why the check is only for primary sink
    if(pHandle && (eWFD_device == WFD_DEVICE_PRIMARY_SINK ||
                 eWFD_device == WFD_DEVICE_SECONDARY_SINK) &&
                 pWFD_negotiated_capability && pCallBack)
    {
        *pHandle = (WFD_MM_HANDLE)WFDMMSink::CreateInstance();

        if(pWFD_negotiated_capability->content_protection_config.
                                               content_protection_ake_port)
        {
            static char sIpAddr[24] = {0};

            getLocalIpAddress(sIpAddr, 24);

            WFDMMSink *pMMSink = WFDMMSink::CreateInstance();
            if(!pMMSink)
            {
                return nErr;
            }
            nErr = pMMSink->setupHDCPSession(sIpAddr,
                      pWFD_negotiated_capability->content_protection_config.
                      content_protection_ake_port);
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
               "wfd_mm_create_session_sink:completed HDCP with status %d",
               nErr);
        }
        else
        {
            WFDMMSink *pMMSink = WFDMMSink::CreateInstance();
            if(pMMSink)
            {
                pMMSink->teardownHDCPSession();
            }
        }

        nErr = ((WFDMMSink*)(*pHandle))->setupMMSession(
                                    WFD_DEVICE_PRIMARY_SINK,
                                    pWFD_negotiated_capability,
                                    pCallBack);
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
               "wfd_mm_create_session_sink:completed with status %d",
               nErr);
    return nErr;
}

/** @brief destroy MM session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_destroy_session_sink(WFD_MM_HANDLE hHandle)
{
    if(hHandle)
    {
        ((WFDMMSink*)hHandle)->teardownMMSession();

        ((WFDMMSink*)hHandle)->teardownHDCPSession();

        if(hHandle)
        {
            WFDMMSink::DeleteInstance();
        }

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                     "wfd_mm_destroy_session_sink:completed");

        return WFD_STATUS_SUCCESS;
    }
    return WFD_STATUS_BADPARAM;
}

/** @brief Play
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the playing of Audio + Video or Audio only or Video only
               wfd_mm_capability_change_cb  - Callback to inform session manager if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_play_sink(WFD_MM_HANDLE hHandle,
                                     WFD_AV_select_t av_select,
                                     wfd_mm_stream_play_cb pCallback )
{
    WFD_status_t nErr = WFD_STATUS_FAIL;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "wfd_mm_stream_play");
    if(hHandle)
    {
        nErr = ((WFDMMSink*)hHandle)->play(av_select, pCallback);
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                 "wfd_mm_stream_play_sink:completed with status %d",
                 nErr);
    return nErr;
}

/** @handler for the timer created in play to request IDR frame
  *
  * @param[in] void pointer
  *
  * @return  WFD_status_t - status
  */
void wfd_mm_idr_request_sink(void *hHandle)
{
   wfd_mm_sink_context_t* sinkCtx = (wfd_mm_sink_context_t* )hHandle;
   if(sinkCtx && sinkCtx->pIDR_Request_cb)
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_idr_request_sink: requesting IDR");
      sinkCtx->pIDR_Request_cb((WFD_MM_HANDLE)hHandle);
   }
}


/** @brief Pause the session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the pausing of Audio + Video or Audio only or Video only
               wfd_mm_capability_change_cb  - Callback to inform session manager if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_pause_sink(WFD_MM_HANDLE hHandle, WFD_AV_select_t av_select, wfd_mm_stream_pause_cb pCallback)
{
    WFD_status_t nErr = WFD_STATUS_FAIL;
    //(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_stream_pause");
    if(hHandle)
    {
        nErr = ((WFDMMSink*)hHandle)->pause(av_select, pCallback);
    }
    return nErr;
}

/**
  *wfd_mm_construct_session_url:Utility function to construct WFD session URL.
  *
  *@parm [out]:url:contains the wfd session url
  *       [in]:urlSize:size of url
  *       [in]:pMMCfg_negotiated:negotiated capabiltiy
  *
  *@return:length of url constructed
  */
static int32 wfd_mm_construct_session_url( char* url, int32 urlSize, WFD_MM_capability_t* pMMCfg_negotiated)
{
  int32 nBytes = 0;
  if(url != NULL && pMMCfg_negotiated != NULL)
  {
    uint16 rtpPort = pMMCfg_negotiated->transport_capability_config.port1_id;
    uint16 hdcpPort = pMMCfg_negotiated->content_protection_config.content_protection_ake_port;

    /***RTP URL as rtp://wfd/<port>/<<T(TCP)/any other char(UDP)>/<CacheSizeinKB>/<HDCP port>****/

    char connType = 'E';//'T' is for TCP
    nBytes = snprintf(url, urlSize,"rtp://wfd/%hu/%c/10240/hdcp/%hu", rtpPort, connType, hdcpPort);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_construct_session_url: WFD session url is %s", url);

  }
  return nBytes;
}

/** @brief create a hdcp source session
  *
  * @param[in] pMMCfg_local  -     local capability
  * @param[in] pMMCfg_remote -     remote capability
  * @param[in] pMMCfg_negotiated - Negotiated capability
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_create_hdcp_session_sink(WFD_MM_HANDLE *hHandle,
                                                WFD_MM_capability_t* pMMCfg_local,
                                               WFD_MM_capability_t* pMMCfg_remote,
                                               WFD_MM_capability_t* pMMCfg_negotiated)
{
    (void) pMMCfg_remote;
    (void) pMMCfg_negotiated;
    WFD_status_t nErr = WFD_STATUS_FAIL;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_create_HDCP_session_sink");

    WFDMMSink *pMMSink = WFDMMSink::CreateInstance();

    if(hHandle)
    {
        *hHandle = pMMSink;
    }

    if(pMMSink)
    {
        static char sIpAddr[24] = {0};
        getLocalIpAddress(sIpAddr, 24);
        nErr = pMMSink->setupHDCPSession(sIpAddr,
                                  pMMCfg_local->content_protection_config.
                                  content_protection_ake_port);
    }
    return nErr;
}

WFD_status_t wfd_mm_update_session_sink(WFD_MM_HANDLE hHandle, WFD_MM_capability_t *WFD_negotiated_capability, void (*wfd_mm_update_session_cb)(WFD_MM_HANDLE, WFD_status_t status))
{
    WFD_status_t nErr = WFD_STATUS_FAIL;
    if( hHandle)
    {
        nErr = ((WFDMMSink*)hHandle)->\
        WFDMMSinkUpdateSession(WFD_negotiated_capability,wfd_mm_update_session_cb) ;
    }
    return nErr;
}

WFD_status_t wfd_mm_destroy_hdcp_session_sink(WFD_MM_HANDLE hHandle)
{
    (void)hHandle;
    WFD_status_t nErr = WFD_STATUS_FAIL;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"wfd_mm_destroy_hdcp_session_sink");

    WFDMMSink *pMMSink = WFDMMSink::CreateInstance();

    if(pMMSink)
    {
        nErr = pMMSink->teardownHDCPSession();
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"wfd_mm_destroy_hdcp_session_sink \
            teardown HDCP status = %d",nErr);
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"wfd_mm_destroy_hdcp_session_sink \
            Failed to get WFDMMSink instance!");
    }

    return nErr;
}

WFD_status_t wfd_mm_av_stream_control_sink(  WFD_MM_HANDLE hHandle,
                                        WFD_MM_AV_Stream_Control_t control, int64 controlValue)
{
    WFD_status_t nErr = WFD_STATUS_FAIL;
    if( hHandle)
    {
        nErr = ((WFDMMSink*)hHandle)->WFDMMSinkAVControl(control,controlValue);
    }
    return nErr;
}
} // extern C
