/************************************************************************* */
/**
 * @file wdsm_mm_source_interface.cpp
 * @brief Implementation of the wireless display session manager MM inteface.
 *
 * Copyright (c) 2011 - 2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 ************************************************************************* */
/* =======================================================================
                                                 PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/wfd/source/main/latest/interface/src/wdsm_mm_source_interface.cpp#2 $
$DateTime: 2012/02/10 05:45:30 $
$Changes:$
========================================================================== */
/* =======================================================================
                             Edit History

========================================================================== */
/* =======================================================================
**               Include files for wdsm_mm_source_interface.cpp
** ======================================================================= */
#include "wdsm_mm_source_interface.h"
#include "WFDMMSource.h"
#include "MMMemory.h"
#include "MMMalloc.h"
#include "MMDebugMsg.h"
#include "MMFile.h"
#include <stdio.h>
#include "wfd_cfg_parser.h"
#include "WFD_HdcpCP.h"

#define FEATURE_CONFIG_FROM_FILE
#define ALTERNATE_WFD_CFG_FILE "/system/etc/capability.xml"
#define WFD_CAPABILITY_TAG "Capability"

/* ==========================================================================

   =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */


uint32 idr_interval_source;
extern uint32 av_format_chnage_interval_source;
extern uint32 video_frame_skipping_start_delay;

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/*Needed for negotiation algorithm to get the best resolution and refresh rate */

#define INIT_VALUE         1  // this value can be changed if there is any new resolution more than this gets added in spec
#define VESA_1920x1200p60  INIT_VALUE            //1
#define VESA_1920x1200p30  VESA_1920x1200p60+1   //2
#define CEA_1920x1080p60   VESA_1920x1200p30+1   //3
#define CEA_1920x1080p50   CEA_1920x1080p60+1    //4
#define CEA_1920x1080p30   CEA_1920x1080p50+1    //5
#define CEA_1920x1080i60   CEA_1920x1080p30+1    //6
#define CEA_1920x1080p25   CEA_1920x1080i60+1    //7
#define CEA_1920x1080i50   CEA_1920x1080p25+1    //8
#define CEA_1920x1080p24   CEA_1920x1080i50+1    //9
#define VESA_1600x1200p60  CEA_1920x1080p24+1    //10
#define VESA_1600x1200p30  VESA_1600x1200p60+1   //11
#define VESA_1680x1050p60  VESA_1600x1200p30+1   //12
#define VESA_1680x1050p30  VESA_1680x1050p60+1   //13
#define VESA_1680x1024p60  VESA_1680x1050p30+1   //14
#define VESA_1680x1024p30  VESA_1680x1024p60+1   //15
#define VESA_1600x900p60   VESA_1680x1024p30+1   //16
#define VESA_1600x900p30   VESA_1600x900p60+1    //17
#define VESA_1400x1050p60  VESA_1600x900p30+1    //18
#define VESA_1400x1050p30  VESA_1400x1050p60+1   //19
#define VESA_1440x900p60   VESA_1400x1050p30+1   //20
#define VESA_1440x900p30   VESA_1440x900p60+1    //21
#define VESA_1366x768p60   VESA_1440x900p30+1    //22
#define VESA_1366x768p30   VESA_1366x768p60+1    //23
#define VESA_1360x768p60   VESA_1366x768p30+1    //24
#define VESA_1360x768p30   VESA_1360x768p60+1    //25
#define VESA_1280x1024p60  VESA_1360x768p30+1    //26
#define VESA_1280x1024p30  VESA_1280x1024p60+1   //27
#define VESA_1280x800p60   VESA_1280x1024p30+1   //28
#define VESA_1280x800p30   VESA_1280x800p60+1    //29
#define VESA_1280x768p60   VESA_1280x800p30+1    //30
#define VESA_1280x768p30   VESA_1280x768p60+1    //31
#define CEA_1280x720p60    VESA_1280x768p30+1    //32
#define CEA_1280x720p50    CEA_1280x720p60+1     //33
#define CEA_1280x720p30    CEA_1280x720p50+1     //34
#define CEA_1280x720p25    CEA_1280x720p30+1     //35
#define CEA_1280x720p24    CEA_1280x720p25+1     //36
#define VESA_1152x864p60   CEA_1280x720p24+1     //37
#define VESA_1152x864p30   VESA_1152x864p60+1    //38
#define VESA_1024x768p60   VESA_1152x864p30+1    //39
#define VESA_1024x768p30   VESA_1024x768p60+1    //40
#define HH_960x540p60      VESA_1024x768p30+1    //41
#define HH_960x540p30      HH_960x540p60+1       //42
#define VESA_800x600p60    HH_960x540p30+1       //43
#define VESA_800x600p30    VESA_800x600p60+1     //44
#define HH_864x480p60      VESA_800x600p30+1     //45
#define HH_864x480p30      HH_864x480p60+1       //46
#define HH_854x480p60      HH_864x480p30+1       //47
#define HH_854x480p30      HH_854x480p60+1       //48
#define HH_848x480p60      HH_854x480p30+1       //49
#define HH_848x480p30      HH_848x480p60+1       //50
#define HH_800x480p60      HH_848x480p30+1       //51
#define HH_800x480p30      HH_800x480p60+1       //52
#define CEA_720x576p50     HH_800x480p30+1       //53
#define CEA_720x576i60     CEA_720x576p50+1      //54
#define CEA_720x576i50     CEA_720x576i60+1      //55
#define CEA_720x480p60     CEA_720x576i50+1      //56
#define CEA_720x480i60     CEA_720x480p60+1      //57
#define CEA_720x480i50     CEA_720x480i60+1      //58
#define CEA_640x480p60     CEA_720x480i50+1      //59
#define HH_640x360p60      CEA_640x480p60+1      //60
#define HH_640x360p30      HH_640x360p60+1       //61
#define MODES_MAX_VALUE    HH_640x360p30+1       //62

static int cea_modes[] = { CEA_640x480p60,    //BIT0
                           CEA_720x480p60,    //BIT1
                           CEA_720x480i60,    //BIT2
                           CEA_720x576p50,    //BIT3
                           CEA_720x576i50,    //BIT4
                           CEA_1280x720p30,   //BIT5
                           CEA_1280x720p60,   //BIT6
                           CEA_1920x1080p30,  //BIT7
                           CEA_1920x1080p60,  //BIT8
                           CEA_1920x1080i60,  //BIT9
                           CEA_1280x720p25,   //BIT10
                           CEA_1280x720p50,   //BIT11
                           CEA_1920x1080p25,  //BIT12
                           CEA_1920x1080p50,  //BIT13
                           CEA_1920x1080i50,  //BIT14
                           CEA_1280x720p24,   //BIT15
                           CEA_1920x1080p24,  //BIT16
                      };

static int vesa_modes[] = {VESA_800x600p30,      //BIT0
                           VESA_800x600p60,      //BIT1
                           VESA_1024x768p30,     //BIT2
                           VESA_1024x768p60,     //BIT3
                           VESA_1152x864p30,     //BIT4
                           VESA_1152x864p60,     //BIT5
                           VESA_1280x768p30,     //BIT6
                           VESA_1280x768p60,     //BIT7
                           VESA_1280x800p30,     //BIT8
                           VESA_1280x800p60,     //BIT9
                           VESA_1360x768p30,     //BIT10
                           VESA_1360x768p60,     //BIT11
                           VESA_1366x768p30,     //BIT12
                           VESA_1366x768p60,     //BIT13
                           VESA_1280x1024p30,    //BIT14
                           VESA_1280x1024p60,    //BIT15
                           VESA_1400x1050p30,    //BIT16
                           VESA_1400x1050p60,    //BIT17
                           VESA_1440x900p30,     //BIT18
                           VESA_1440x900p60,     //BIT19
                           VESA_1600x900p30,     //BIT20
                           VESA_1600x900p60,     //BIT21
                           VESA_1600x1200p30,    //BIT22
                           VESA_1600x1200p60,    //BIT23
                           VESA_1680x1024p30,    //BIT24
                           VESA_1680x1024p60,    //BIT25
                           VESA_1680x1050p30,    //BIT26
                           VESA_1680x1050p60,    //BIT27
                           VESA_1920x1200p30,    //BIT28
                           VESA_1920x1200p60,    //BIT29
                        };
static int hh_modes[] = { HH_800x480p30,      //BIT0
                          HH_800x480p60,      //BIT1
                          HH_854x480p30,      //BIT2
                          HH_854x480p60,      //BIT3
                          HH_864x480p30,      //BIT4
                          HH_864x480p60,      //BIT5
                          HH_640x360p30,      //BIT6
                          HH_640x360p60,      //BIT7
                          HH_960x540p30,      //BIT8
                          HH_960x540p60,      //BIT9
                          HH_848x480p30,      //BIT10
                          HH_848x480p60,      //BIT11
                        };

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief Get H264 capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_h264_codec_config_t - pointer to H264 capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
extern "C" {

  static int wfd_mm_negotiated_value(uint32 value);
   static int  wfd_mm_negotiated_cea_value(uint32 value,int *priority);
   static int  wfd_mm_negotiated_vesa_value(uint32 value,int *priority);
   static int  wfd_mm_negotiated_hh_value(uint32 value,int *priority);

  /** @brief Get extended capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle (not used)
               WFD_extended_capability_config_t* - pointer to extended capability structure
  *
  * @return  WFD_status_t - status
  */
  WFD_status_t wfd_get_extended_capability_source (WFD_MM_HANDLE hHandle,
                                                   WFD_extended_capability_config_t *ext_capability)
  {
    (void)hHandle;
    WFD_status_t ret_status = WFD_STATUS_BADPARAM;
    if (ext_capability != NULL)
    {
      ext_capability->sub_element_id         = WFD_EXTENDED_CAPABILITY_SUB_ELEMENT_ID;
      ext_capability->length                 = WFD_EXTENDED_CAPABILITY_SUB_ELEMENT_LENGTH;
      ext_capability->ext_capability_bitmap  = WFD_EXTENDED_CAPABILITY_UIBC; //as of now

      ret_status = WFD_STATUS_SUCCESS;
    }

    return ret_status;
  }

/** @brief Get video capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_video_codec_config_t - pointer to video capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_video_capability_source(WFD_MM_HANDLE, WFD_video_codec_config_t* video_codec_config, WFD_device_t)
{
  if((video_codec_config) && (video_codec_config->h264_codec))
  {
   // memset(video_codec_config->h264_codec, 0, (sizeof(WFD_3d_h264_codec_config_t) * WFD_MAX_NUM_H264_PROFILES));
    video_codec_config->native_bitmap = 2;
    video_codec_config->preferred_display_mode_supported = 0;
    //Currently we are populating only one profile we will add all the profiles later
    video_codec_config->num_h264_profiles = 1;
    //populate the first profile details
    ((video_codec_config->h264_codec) + 0)->supported_cea_mode = 1;
    ((video_codec_config->h264_codec) + 0)->supported_vesa_mode = 0;
    ((video_codec_config->h264_codec) + 0)->supported_hh_mode = 0;
    ((video_codec_config->h264_codec) + 0)->h264_level = 2;//Level 3.2
    ((video_codec_config->h264_codec) + 0)->h264_profile = 1;//Profile CBP
    ((video_codec_config->h264_codec) + 0)->min_slice_size = 0;
    ((video_codec_config->h264_codec) + 0)->decoder_latency = 0;
    ((video_codec_config->h264_codec) + 0)->frame_rate_control_support = 0;
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
WFD_status_t wfd_mm_get_3d_video_capability_source(WFD_MM_HANDLE, WFD_3d_video_codec_config_t* threed_video_codec_config, WFD_device_t)
{
  (void)threed_video_codec_config;
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
WFD_status_t wfd_mm_get_lpcm_capability_source(WFD_MM_HANDLE, WFD_lpcm_codec_config_t *lpcm_codec_config, WFD_device_t wfd_device_type)
{
  (void) wfd_device_type;
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
WFD_status_t wfd_mm_get_aac_capability_source(WFD_MM_HANDLE, WFD_aac_codec_config_t *aac_codec_config, WFD_device_t wfd_device_type)
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

/** @brief Get dts codec capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_dts_codec_config_t - pointer to dts codec capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
/**
WFD_status_t wfd_mm_get_dts_codec_capability_source
  (WFD_MM_HANDLE, WFD_dts_codec_config_t*, WFD_device_t)
{
      return WFD_STATUS_SUCCESS;
   }
 **/

/** @brief Get dolby digital codec capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_dolby_digital_codec_config_t - pointer to dolby digital codec capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_dolby_digital_codec_capability_source(WFD_MM_HANDLE, WFD_dolby_digital_codec_config_t *dolby_digital_codec, WFD_device_t wfd_device_type)
{
   (void) wfd_device_type;
   if(dolby_digital_codec)
   {
      dolby_digital_codec->supported_modes_bitmap = 0;//DOLBY_DIGITAL_48_16_2_AC3
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
WFD_status_t wfd_mm_get_content_protection_capability_source(WFD_MM_HANDLE, WFD_content_protection_capability_config_t * content_protection_capability_config, WFD_device_t wfd_device_type)
{
   (void) content_protection_capability_config;
   (void) wfd_device_type;
   return WFD_STATUS_SUCCESS;
}
/** @brief Get transport capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_transport_capability_config_t - pointer to transport capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_transport_capability_source(WFD_MM_HANDLE, WFD_transport_capability_config_t *transport_capability_config, WFD_device_t wfd_device_type)
{
   (void) transport_capability_config;
   (void) wfd_device_type;
   return WFD_STATUS_SUCCESS;
}
/** @brief get negotiated capability
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_MM_capability_t - pointer to local capability structure
               WFD_MM_capability_t - pointer to remote capability structure
               WFD_MM_capability_t - pointer to negotiated capability structure
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_negotiated_capability_source(WFD_MM_HANDLE hHandle,
                                        WFD_MM_capability_t* pMMCfg_local,
                                        WFD_MM_capability_t* pMMCfg_remote,
                                        WFD_MM_capability_t* pMMCfg_negotiated,
                                        WFD_MM_capability_t* pMMCfg_common)
{
 (void) hHandle;
 if((pMMCfg_negotiated))
 {
    //memset(pMMCfg_negotiated, 0, (sizeof(WFD_MM_capability_t)));
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "get Negotiated Capability_source");
    /* This code  can be used to override negotiated capability from file */
    {
       int negValue = 0;
       int ceaModeWeight = MODES_MAX_VALUE,ceaIndexValue = 0;
       int vesaModeWeight = MODES_MAX_VALUE, vesaIndexValue = 0;
       int hhModeWeight = MODES_MAX_VALUE, hhIndexValue = 0;
       int ceaPrevModeWeight = 0;
       int vesaPrevModeWeight = 0;
       int hhPrevModeWeight = 0;

       int nBestWeight = 0;
       int nCurrBestIndex = 0;
       int nCurrBestWeight = 0;

       int profile_index = 0;
       int comIdx = 0;

       int locIdx = 0;


       /*
         The src and remote (sink) capabilities are compared and a optimal set is derived
       */

      if(  pMMCfg_local->audio_method > WFD_AUDIO_UNK &&
            pMMCfg_local->audio_method < WFD_AUDIO_INVALID &&
            pMMCfg_remote->audio_method > WFD_AUDIO_UNK &&
            pMMCfg_remote->audio_method < WFD_AUDIO_INVALID
         )
      {
           pMMCfg_negotiated->audio_method = (pMMCfg_local->audio_method > pMMCfg_remote->audio_method)?
                                          pMMCfg_local->audio_method : pMMCfg_remote->audio_method;
           pMMCfg_common->audio_method = pMMCfg_negotiated->audio_method;
      }
      else
      {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Invalid Audio Method");
      }
      pMMCfg_negotiated->audio_method = WFD_AUDIO_UNK;



      if(pMMCfg_local->audio_config.lpcm_codec.supported_modes_bitmap &
           pMMCfg_remote->audio_config.lpcm_codec.supported_modes_bitmap)
      {
          pMMCfg_negotiated->audio_method = WFD_AUDIO_LPCM;
      }

      if(pMMCfg_local->audio_config.aac_codec.supported_modes_bitmap &
           pMMCfg_remote->audio_config.aac_codec.supported_modes_bitmap)
      {
          pMMCfg_negotiated->audio_method = WFD_AUDIO_AAC;
      }

      if(pMMCfg_local->audio_config.dolby_digital_codec.supported_modes_bitmap &
          pMMCfg_remote->audio_config.dolby_digital_codec.supported_modes_bitmap)
      {
          pMMCfg_negotiated->audio_method = WFD_AUDIO_DOLBY_DIGITAL;
      }

      if(pMMCfg_negotiated->audio_method == WFD_AUDIO_UNK)
      {
          //No audio format matches Fallback to LPCM mandatory support
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " No audio bitmaps in common. choosing default LPCM");
         pMMCfg_negotiated->audio_method = WFD_AUDIO_LPCM;
      }

      pMMCfg_common->audio_method = pMMCfg_negotiated->audio_method;
        // AAC
      if(pMMCfg_negotiated->audio_method == WFD_AUDIO_AAC)
      {
         pMMCfg_negotiated->audio_config.aac_codec.decoder_latency = 0;
         pMMCfg_common->audio_config.aac_codec.decoder_latency = 0;
         pMMCfg_common->audio_config.aac_codec.supported_modes_bitmap =(pMMCfg_local->audio_config.aac_codec.supported_modes_bitmap &
                                                                        pMMCfg_remote->audio_config.aac_codec.supported_modes_bitmap);

         negValue = wfd_mm_negotiated_value(pMMCfg_common->audio_config.aac_codec.supported_modes_bitmap);
         pMMCfg_negotiated->audio_config.aac_codec.supported_modes_bitmap =(uint16)(negValue ? ( 1 <<  (negValue - 1) ) : 0) ;
      }

      if(pMMCfg_negotiated->audio_method == WFD_AUDIO_LPCM)
      {
         //LPCM
         pMMCfg_negotiated->audio_config.lpcm_codec.decoder_latency =
         pMMCfg_common->audio_config.lpcm_codec.decoder_latency = 0;
         pMMCfg_common->audio_config.lpcm_codec.supported_modes_bitmap = (pMMCfg_local->audio_config.lpcm_codec.supported_modes_bitmap &
                                                                          pMMCfg_remote->audio_config.lpcm_codec.supported_modes_bitmap);

         negValue = wfd_mm_negotiated_value(pMMCfg_common->audio_config.lpcm_codec.supported_modes_bitmap);

         pMMCfg_negotiated->audio_config.lpcm_codec.supported_modes_bitmap = negValue ? ( 1 <<  (negValue - 1) ) : 0 ;

         if(!pMMCfg_negotiated->audio_config.lpcm_codec.supported_modes_bitmap)
         {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " No audio bitmaps for LPCM, choosing 48k");
           pMMCfg_negotiated->audio_config.lpcm_codec.supported_modes_bitmap = 2;
           pMMCfg_common->audio_config.lpcm_codec.supported_modes_bitmap = 2;
         }
      }

      if(pMMCfg_negotiated->audio_method == WFD_AUDIO_DOLBY_DIGITAL)
      {
         //AC3
         pMMCfg_negotiated->audio_config.dolby_digital_codec.decoder_latency =
         pMMCfg_common->audio_config.dolby_digital_codec.decoder_latency = 0;

         pMMCfg_common->audio_config.dolby_digital_codec.supported_modes_bitmap = (pMMCfg_local->audio_config.dolby_digital_codec.supported_modes_bitmap &
                                                                                   pMMCfg_remote->audio_config.dolby_digital_codec.supported_modes_bitmap);

         negValue = wfd_mm_negotiated_value(pMMCfg_common->audio_config.dolby_digital_codec.supported_modes_bitmap);

         pMMCfg_negotiated->audio_config.dolby_digital_codec.supported_modes_bitmap = negValue ? ( 1 <<  (negValue - 1) ) : 0 ;

      }

       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " negotiated audio method :%d ",pMMCfg_negotiated->audio_method);

       pMMCfg_common->video_method = pMMCfg_negotiated->video_method = WFD_VIDEO_H264;


       // Only one profile tuple
       pMMCfg_negotiated->video_config.video_config.num_h264_profiles = 1;


       pMMCfg_negotiated->video_config.video_config.native_bitmap =
       pMMCfg_common->video_config.video_config.native_bitmap = 0;

       // Not supported
       pMMCfg_negotiated->video_config.video_config.preferred_display_mode_supported =
       pMMCfg_common->video_config.video_config.preferred_display_mode_supported = 0;


       for(int remIdx = 0 ; remIdx < pMMCfg_remote->video_config.video_config.num_h264_profiles; remIdx ++)
       {
         //local and remote profiles may be in reverse order need to match profile to profile.
         for(locIdx = 0; locIdx < pMMCfg_local->video_config.video_config.num_h264_profiles; locIdx ++)
         {
           if(pMMCfg_local->video_config.video_config.h264_codec[locIdx].h264_profile == 
             pMMCfg_remote->video_config.video_config.h264_codec[remIdx].h264_profile)
           {
              break;
           }
         }

         if(locIdx < pMMCfg_local->video_config.video_config.num_h264_profiles)
         {
             pMMCfg_common->video_config.video_config.num_h264_profiles++;
             MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "H264 profiles.. common found  ");
         }
         else
         {
             MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "H264 profiles done. Num common found = %d",
                          pMMCfg_common->video_config.video_config.num_h264_profiles);
             continue;
         }


          // CEA mode
         (pMMCfg_common->video_config.video_config.h264_codec[comIdx]).supported_cea_mode =
                                            ((pMMCfg_local->video_config.video_config.h264_codec[locIdx]).supported_cea_mode &
                                            (pMMCfg_remote->video_config.video_config.h264_codec[remIdx]).supported_cea_mode);
          if((pMMCfg_common->video_config.video_config.h264_codec[comIdx]).supported_cea_mode)
          {
              ceaModeWeight = MODES_MAX_VALUE;
              ceaIndexValue =  wfd_mm_negotiated_cea_value((pMMCfg_common->video_config.video_config.h264_codec[comIdx]).supported_cea_mode,&ceaModeWeight);
              MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,"nego alog ,nego cea value = %d ceaIndexValue = %d",
                  (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_cea_mode,ceaIndexValue);
          }


         // VESA mode
          (pMMCfg_common->video_config.video_config.h264_codec[comIdx]).supported_vesa_mode =
                                            ((pMMCfg_local->video_config.video_config.h264_codec[locIdx]).supported_vesa_mode &
                                            (pMMCfg_remote->video_config.video_config.h264_codec[remIdx]).supported_vesa_mode);
          if((pMMCfg_common->video_config.video_config.h264_codec[comIdx]).supported_vesa_mode)
          {
              vesaModeWeight = MODES_MAX_VALUE;
              vesaIndexValue =  wfd_mm_negotiated_vesa_value((pMMCfg_common->video_config.video_config.h264_codec[comIdx]).supported_vesa_mode,&vesaModeWeight);
              MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,"nego alog ,commom vesa value = %d vesaIndexValue = %d",
                  (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_vesa_mode,vesaIndexValue);
          }


        // HH mode
           (pMMCfg_common->video_config.video_config.h264_codec[comIdx]).supported_hh_mode =
                                            ((pMMCfg_local->video_config.video_config.h264_codec[locIdx]).supported_hh_mode &
                                            (pMMCfg_remote->video_config.video_config.h264_codec[remIdx]).supported_hh_mode);
           if((pMMCfg_common->video_config.video_config.h264_codec[comIdx]).supported_hh_mode)
           {
              hhModeWeight = MODES_MAX_VALUE;
              hhIndexValue =  wfd_mm_negotiated_hh_value((pMMCfg_common->video_config.video_config.h264_codec[comIdx]).supported_hh_mode,&hhModeWeight);
              MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,"nego alog ,commom hh value = %d hhIndexValue = %d",
                  (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_hh_mode,hhIndexValue);
           }


           pMMCfg_common->video_config.video_config.h264_codec[comIdx].h264_level =
                     pMMCfg_local->video_config.video_config.h264_codec[locIdx].h264_level >
                     pMMCfg_remote->video_config.video_config.h264_codec[remIdx].h264_level ?
                     pMMCfg_remote->video_config.video_config.h264_codec[remIdx].h264_level :
                     pMMCfg_local->video_config.video_config.h264_codec[locIdx].h264_level;

           (pMMCfg_common->video_config.video_config.h264_codec[comIdx]).h264_profile =
                     pMMCfg_remote->video_config.video_config.h264_codec[remIdx].h264_profile;


           (pMMCfg_common->video_config.video_config.h264_codec[comIdx]).frame_rate_control_support = 0;
           // Frame skipping support bits 0 - 3
           if(((pMMCfg_remote->video_config.video_config.h264_codec[remIdx]).frame_rate_control_support & 0x1)
               &
              ((pMMCfg_local->video_config.video_config.h264_codec[locIdx]).frame_rate_control_support & 0x1) )
           {
             uint8 uSrcFrameSkipInterval =
                            ((pMMCfg_local->video_config.video_config.h264_codec[locIdx]).frame_rate_control_support >> 1) & 0x7;

             uint8 uSinkFrameSkipinterval =
                            ((pMMCfg_remote->video_config.video_config.h264_codec[remIdx]).frame_rate_control_support >> 1) & 0x7;

             uint8 uMinFrameSkipInterval = MIN(uSrcFrameSkipInterval,uSinkFrameSkipinterval);

             (pMMCfg_common->video_config.video_config.h264_codec[comIdx]).frame_rate_control_support = (uint8)(((uMinFrameSkipInterval << 1)| 0x1) & 0xF );
           }
           // Dynamic refresh rate support bit 4
           if(((pMMCfg_remote->video_config.video_config.h264_codec[remIdx]).frame_rate_control_support & 0x10)
               &
              ((pMMCfg_local->video_config.video_config.h264_codec[locIdx]).frame_rate_control_support & 0x10) )
           {
             ((pMMCfg_common->video_config.video_config.h264_codec[comIdx]).frame_rate_control_support) |= 0x10;
           }


           nBestWeight = ceaModeWeight;

           if(vesaModeWeight > nBestWeight)
           {
             nBestWeight = vesaModeWeight;
           }

           if(hhModeWeight > nBestWeight)
           {
             nBestWeight = hhModeWeight;
           }

           if(nCurrBestWeight > nBestWeight)
           {
             comIdx++;
             continue;
           }

           nCurrBestIndex = comIdx;

           nCurrBestWeight = nBestWeight;

           if(ceaModeWeight < vesaModeWeight && ceaModeWeight < hhModeWeight)
           {
              (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_cea_mode = ( 1 << ceaIndexValue );
              (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_vesa_mode = 0 ;
              (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_hh_mode = 0 ;
               MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"nego alog video format cea = %d",
                  (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_cea_mode);
           }
           else if(vesaModeWeight < ceaModeWeight && vesaModeWeight < hhModeWeight)
           {
              (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_cea_mode = 0 ;
              (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_vesa_mode = ( 1 << vesaIndexValue ) ;
              (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_hh_mode = 0 ;
               MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"nego alog video format ves = %d",
                  (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_vesa_mode);
           }
           else if(hhModeWeight < ceaModeWeight && hhModeWeight < vesaModeWeight)
           {
              (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_cea_mode = 0 ;
              (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_vesa_mode = 0 ;
              (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_hh_mode = ( 1 << hhIndexValue ) ;
               MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"nego alog video format hh = %d",
                  (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_hh_mode);
           }

           comIdx++;
      }

      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, " BestProfileIdx %d, %d",comIdx,nCurrBestIndex);

      if(!(pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_hh_mode &&
          !(pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_cea_mode &&
           !(pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_vesa_mode)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, " No video bitmaps for anything, choosing CEA 1");
        (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_cea_mode =
        (pMMCfg_common->video_config.video_config.h264_codec[0]).supported_cea_mode = 1;
      }

     (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).h264_level =
       (pMMCfg_common->video_config.video_config.h264_codec[nCurrBestIndex]).h264_level;

     (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).h264_profile =
       (pMMCfg_common->video_config.video_config.h264_codec[nCurrBestIndex]).h264_profile;	 

     (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).frame_rate_control_support =
     (pMMCfg_common->video_config.video_config.h264_codec[nCurrBestIndex]).frame_rate_control_support;

     (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).max_vres = 0;

     (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).max_hres = 0;


     // Currently not modified
     (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).min_slice_size = 0;

     (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).decoder_latency = 0;

     (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).slice_enc_params = 0;

      // Currently not supporting stand by resume

      pMMCfg_negotiated->standby_resume_support = FALSE;
      if(pMMCfg_local->standby_resume_support && pMMCfg_remote->standby_resume_support)
      {
          pMMCfg_negotiated->standby_resume_support = TRUE;
      }
      pMMCfg_common->standby_resume_support = pMMCfg_negotiated->standby_resume_support;
      // Negotiated port type will be source port type
      pMMCfg_negotiated->transport_capability_config.eRtpPortType = pMMCfg_local->transport_capability_config.eRtpPortType;
      // Common port type will be source port type
      pMMCfg_common->transport_capability_config.eRtpPortType = pMMCfg_local->transport_capability_config.eRtpPortType;
      if( pMMCfg_remote->content_protection_config.content_protection_capability
          &&
          pMMCfg_local->content_protection_config.content_protection_capability )
      {
        pMMCfg_negotiated->content_protection_config.content_protection_capability = pMMCfg_remote->content_protection_config.content_protection_capability;
        pMMCfg_negotiated->content_protection_config.content_protection_ake_port = pMMCfg_remote->content_protection_config.content_protection_ake_port;
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,"Debug:HDCP Port = %d, Capability = %d",pMMCfg_negotiated->content_protection_config.content_protection_ake_port,
                                            pMMCfg_negotiated->content_protection_config.content_protection_capability);
        pMMCfg_common->content_protection_config.content_protection_capability = pMMCfg_negotiated->content_protection_config.content_protection_capability;
        pMMCfg_common->content_protection_config.content_protection_ake_port = pMMCfg_negotiated->content_protection_config.content_protection_ake_port;
      }

      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "dumping local capabilities");
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty pMMCfg_local->audio_method = %d", pMMCfg_local->audio_method);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty pMMCfg_local->audio_config.aac_codec.supported_modes_bitmap = %d", pMMCfg_local->audio_config.aac_codec.supported_modes_bitmap);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty pMMCfg_local->video_method = %d", pMMCfg_local->video_method);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty pMMCfg_local->video_config.video_config.num_h264_profiles = %d",pMMCfg_local->video_config.video_config.num_h264_profiles);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty pMMCfg_local->video_config.video_config.preferred_display_mode_supported = %d",pMMCfg_local->video_config.video_config.preferred_display_mode_supported);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty (pMMCfg_local->video_config.video_config.h264_codec[0]).supported_cea_mode = %d", (pMMCfg_local->video_config.video_config.h264_codec[0]).supported_cea_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty (pMMCfg_local->video_config.video_config.h264_codec[0]).supported_vesa_mode = %d", (pMMCfg_local->video_config.video_config.h264_codec[0]).supported_vesa_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty (pMMCfg_local->video_config.video_config.h264_codec[0]).supported_hh_mode = %d", (pMMCfg_local->video_config.video_config.h264_codec[0]).supported_hh_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty (pMMCfg_local->video_config.video_config.h264_codec[0]).h264_level = %d", (pMMCfg_local->video_config.video_config.h264_codec[0]).h264_level);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty (pMMCfg_local->video_config.video_config.h264_codec[0]).h264_profile = %d", (pMMCfg_local->video_config.video_config.h264_codec[0]).h264_profile);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty (pMMCfg_local->video_config.video_config.h264_codec[0]).frame_rate_control_support = %d", (pMMCfg_local->video_config.video_config.h264_codec[0]).frame_rate_control_support);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty (pMMCfg_local->video_config.video_config.h264_codec[0]).max_vres = %d", (pMMCfg_local->video_config.video_config.h264_codec[0]).max_vres);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty (pMMCfg_local->video_config.video_config.h264_codec[0]).max_hres = %d", (pMMCfg_local->video_config.video_config.h264_codec[0]).max_hres);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty pMMCfg_local->standby_resume_support = %d", pMMCfg_local->standby_resume_support);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty pMMCfg_local->transport_capability_config.eRtpPortType = %d", pMMCfg_local->transport_capability_config.eRtpPortType);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "local capabilty pMMCfg_local->content_protection_config.content_protection_capability = %d", pMMCfg_local->content_protection_config.content_protection_capability);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "local capabilty pMMCfg_local->content_protection_config.content_protection_ake_port = %d", pMMCfg_local->content_protection_config.content_protection_ake_port);

      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "dumping remote capabilities");
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty pMMCfg_remote->audio_method = %d", pMMCfg_remote->audio_method);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty pMMCfg_remote->audio_config.aac_codec.supported_modes_bitmap = %d", pMMCfg_remote->audio_config.aac_codec.supported_modes_bitmap);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty pMMCfg_remote->video_method = %d", pMMCfg_remote->video_method);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty pMMCfg_remote->video_config.video_config.num_h264_profiles = %d",pMMCfg_remote->video_config.video_config.num_h264_profiles);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty pMMCfg_remote->video_config.video_config.preferred_display_mode_supported = %d",pMMCfg_remote->video_config.video_config.preferred_display_mode_supported);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty (pMMCfg_remote->video_config.video_config.h264_codec[0]).supported_cea_mode = %d", (pMMCfg_remote->video_config.video_config.h264_codec[0]).supported_cea_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty (pMMCfg_remote->video_config.video_config.h264_codec[0]).supported_vesa_mode = %d", (pMMCfg_remote->video_config.video_config.h264_codec[0]).supported_vesa_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty (pMMCfg_remote->video_config.video_config.h264_codec[0]).supported_hh_mode = %d", (pMMCfg_remote->video_config.video_config.h264_codec[0]).supported_hh_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty (pMMCfg_remote->video_config.video_config.h264_codec[0]).h264_level = %d", (pMMCfg_remote->video_config.video_config.h264_codec[0]).h264_level);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty (pMMCfg_remote->video_config.video_config.h264_codec[0]).h264_profile = %d", (pMMCfg_remote->video_config.video_config.h264_codec[0]).h264_profile);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty (pMMCfg_remote->video_config.video_config.h264_codec[0]).frame_rate_control_support = %d", (pMMCfg_remote->video_config.video_config.h264_codec[0]).frame_rate_control_support);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty (pMMCfg_remote->video_config.video_config.h264_codec[0]).max_vres = %d", (pMMCfg_remote->video_config.video_config.h264_codec[0]).max_vres);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty (pMMCfg_remote->video_config.video_config.h264_codec[0]).max_hres = %d", (pMMCfg_remote->video_config.video_config.h264_codec[0]).max_hres);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty pMMCfg_remote->standby_resume_support = %d", pMMCfg_remote->standby_resume_support);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty pMMCfg_remote->transport_capability_config.eRtpPortType = %d", pMMCfg_remote->transport_capability_config.eRtpPortType);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty pMMCfg_remote->content_protection_config.content_protection_capability = %d", pMMCfg_remote->content_protection_config.content_protection_capability);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "remote capabilty pMMCfg_remote->content_protection_config.content_protection_ake_port = %d", pMMCfg_remote->content_protection_config.content_protection_ake_port);


      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "dumping negotiated capabilities");
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty pMMCfg_negotiated->audio_method = %d", pMMCfg_negotiated->audio_method);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty pMMCfg_negotiated->audio_config.aac_codec.supported_modes_bitmap = %d", pMMCfg_negotiated->audio_config.aac_codec.supported_modes_bitmap);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty pMMCfg_negotiated->video_method = %d", pMMCfg_negotiated->video_method);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty pMMCfg_negotiated->video_config.video_config.num_h264_profiles = %d",pMMCfg_negotiated->video_config.video_config.num_h264_profiles);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty pMMCfg_negotiated->video_config.video_config.preferred_display_mode_supported = %d",pMMCfg_negotiated->video_config.video_config.preferred_display_mode_supported);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_cea_mode = %d", (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_cea_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_vesa_mode = %d", (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_vesa_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_hh_mode = %d", (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_hh_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).h264_level = %d", (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).h264_level);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).h264_profile = %d", (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).h264_profile);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).frame_rate_control_support = %d", (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).frame_rate_control_support);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).max_vres = %d", (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).max_vres);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).max_hres = %d", (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).max_hres);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty pMMCfg_negotiated->standby_resume_support = %d", pMMCfg_negotiated->standby_resume_support);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty pMMCfg_negotiated->transport_capability_config.eRtpPortType = %d", pMMCfg_negotiated->transport_capability_config.eRtpPortType);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty pMMCfg_negotiated->content_protection_config.content_protection_capability = %d", pMMCfg_negotiated->content_protection_config.content_protection_capability);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "negotiated capabilty pMMCfg_negotiated->content_protection_config.content_protection_ake_port = %d", pMMCfg_negotiated->content_protection_config.content_protection_ake_port);
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "dumping common capabilities");
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty pMMCfg_common->audio_method = %d", pMMCfg_common->audio_method);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty pMMCfg_common->audio_config.aac_codec.supported_modes_bitmap = %d", pMMCfg_common->audio_config.aac_codec.supported_modes_bitmap);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty pMMCfg_common->video_method = %d", pMMCfg_common->video_method);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty pMMCfg_common->video_config.video_config.num_h264_profiles = %d",pMMCfg_common->video_config.video_config.num_h264_profiles);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty pMMCfg_common->video_config.video_config.preferred_display_mode_supported = %d",pMMCfg_common->video_config.video_config.preferred_display_mode_supported);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty (pMMCfg_common->video_config.video_config.h264_codec[0]).supported_cea_mode = %d", (pMMCfg_common->video_config.video_config.h264_codec[0]).supported_cea_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty (pMMCfg_common->video_config.video_config.h264_codec[0]).supported_vesa_mode = %d", (pMMCfg_common->video_config.video_config.h264_codec[0]).supported_vesa_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty (pMMCfg_common->video_config.video_config.h264_codec[0]).supported_hh_mode = %d", (pMMCfg_common->video_config.video_config.h264_codec[0]).supported_hh_mode);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty (pMMCfg_common->video_config.video_config.h264_codec[0]).h264_level = %d", (pMMCfg_common->video_config.video_config.h264_codec[0]).h264_level);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty (pMMCfg_common->video_config.video_config.h264_codec[0]).h264_profile = %d", (pMMCfg_common->video_config.video_config.h264_codec[0]).h264_profile);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty (pMMCfg_common->video_config.video_config.h264_codec[0]).frame_rate_control_support = %d", (pMMCfg_common->video_config.video_config.h264_codec[0]).frame_rate_control_support);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty (pMMCfg_common->video_config.video_config.h264_codec[0]).max_vres = %d", (pMMCfg_common->video_config.video_config.h264_codec[0]).max_vres);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty (pMMCfg_common->video_config.video_config.h264_codec[0]).max_hres = %d", (pMMCfg_common->video_config.video_config.h264_codec[0]).max_hres);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty pMMCfg_common->standby_resume_support = %d", pMMCfg_common->standby_resume_support);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty pMMCfg_common->transport_capability_config.eRtpPortType = %d", pMMCfg_common->transport_capability_config.eRtpPortType);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty pMMCfg_common->content_protection_config.content_protection_capability = %d", pMMCfg_common->content_protection_config.content_protection_capability);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "common capabilty pMMCfg_common->content_protection_config.content_protection_ake_port = %d", pMMCfg_common->content_protection_config.content_protection_ake_port);
   }

   return WFD_STATUS_SUCCESS;
  }
  return WFD_STATUS_BADPARAM;
}

/** @brief Get local MM capability structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_MM_capability_t - pointer to local capability structure
                WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_local_capability_source(WFD_MM_HANDLE hHandle,
                                                WFD_MM_capability_t* pMMCfg_negotiated,
                                                WFD_device_t tDevice)
{
 (void)hHandle;
 (void)tDevice;
 if((pMMCfg_negotiated))
 {
  //  memset(pMMCfg_negotiated, 0, (sizeof(WFD_MM_capability_t)));
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "get Negotiated Capability");
    /* This code is for PF2 */
    resetCfgItems();
#ifdef FEATURE_CONFIG_FROM_FILE
    FILE * fp = NULL;
    int32 nRetVal;
    MM_HANDLE pFileHandle = NULL;
    //Check whether /system/etc/wfdconfig.xml is there or not
    nRetVal = MM_File_Create(WFD_CFG_FILE, MM_FILE_CREATE_R, &pFileHandle);
    if(!nRetVal)
    {
      if(MM_File_Release(pFileHandle))
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parserCfg:File release failed");
      }
    }
    pFileHandle = NULL;
   if(nRetVal == 0)
    {
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
            "Reading capabilities from %s",WFD_CFG_FILE);
     const char str[]=WFD_CFG_FILE;
     readConfigFile readCfgCapabilities;
     readCfgCapabilities.pCfgCapabilities = pMMCfg_negotiated;
     parseCfg(str,&readCfgCapabilities);
     pMMCfg_negotiated = readCfgCapabilities.pCfgCapabilities;
   }
   else if(nRetVal == 1)//MM_File_Create returns 1 on failure
   {
     if ((fp = fopen(ALTERNATE_WFD_CFG_FILE,"r")))
     {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                    "Reading capabilities from %s",ALTERNATE_WFD_CFG_FILE);
        const char str[]=ALTERNATE_WFD_CFG_FILE;
        readConfigFile readCfgCapabilities;
        readCfgCapabilities.pCfgCapabilities = pMMCfg_negotiated;
        parseCfg(str,&readCfgCapabilities);
        pMMCfg_negotiated = readCfgCapabilities.pCfgCapabilities;
        fclose(fp);
    }
    else
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                  "Alternative file %s is not present",ALTERNATE_WFD_CFG_FILE);
    }
  }
   else
#endif
 {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "No Capability file found: not parsing capabilities from file");
       pMMCfg_negotiated->audio_method = WFD_AUDIO_LPCM;
       pMMCfg_negotiated->audio_config.lpcm_codec.decoder_latency = 0;
       pMMCfg_negotiated->audio_config.lpcm_codec.supported_modes_bitmap = 2;  //LPCM_48_16_2
       pMMCfg_negotiated->audio_config.aac_codec.decoder_latency = 0;
       pMMCfg_negotiated->audio_config.aac_codec.supported_modes_bitmap = 1;  //AAC-LC, 48, 16, 2 channels
       pMMCfg_negotiated->video_method = WFD_VIDEO_H264;
       pMMCfg_negotiated->video_config.video_config.num_h264_profiles = 1;
       pMMCfg_negotiated->video_config.video_config.native_bitmap = 2;
       pMMCfg_negotiated->video_config.video_config.preferred_display_mode_supported = 0;
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_cea_mode = 1;
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_vesa_mode = 0;
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).supported_hh_mode = 1;
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).h264_level = 1;//Level 3.2
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).h264_profile = 1;//Profile CBP
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).min_slice_size = 0;
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).decoder_latency = 0;
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).frame_rate_control_support = 17;
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).max_vres = 480;
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).max_hres = 800;
       (pMMCfg_negotiated->video_config.video_config.h264_codec[0]).slice_enc_params = 0;
       pMMCfg_negotiated->standby_resume_support = TRUE;
    }
   return WFD_STATUS_SUCCESS;
  }
  return WFD_STATUS_BADPARAM;
}

/** @brief create MM session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_device_t - device type either primary/secondary
               WFD_MM_capability_t - pointer to negotiated capabilty structure
               wfd_mm_capability_change_cb  - Callback to inform session manager if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_create_session_source(WFD_MM_HANDLE *handle, WFD_device_t wfd_device_type, WFD_MM_capability_t *WFD_negotiated_capability, WFD_MM_callbacks_t* pCallback)
{
  if((wfd_device_type == WFD_DEVICE_SOURCE) && WFD_negotiated_capability && pCallback)
  {
    *handle = (WFD_MM_HANDLE) new WFDMMSource(wfd_device_type, WFD_negotiated_capability,pCallback);
    return WFD_STATUS_SUCCESS;
  }
  return WFD_STATUS_BADPARAM;
}

/** @brief destroy MM session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */

WFD_status_t wfd_mm_destroy_session_source(WFD_MM_HANDLE WFD_MM_Handle)
{
  if(WFD_MM_Handle)
  {
    MM_Delete((WFDMMSource*)WFD_MM_Handle);
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
WFD_status_t wfd_mm_stream_play_source(WFD_MM_HANDLE handle, WFD_AV_select_t av_select, void (*wfd_mm_stream_play_cb)(WFD_MM_HANDLE, WFD_status_t status))
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "wfd_mm_stream_play_source");
  OMX_ERRORTYPE result = OMX_ErrorNone;
  if( handle && wfd_mm_stream_play_cb)
  {
    WFDMMSource *WFDMMSourceHandle = (WFDMMSource *)handle;
    result = WFDMMSourceHandle->WFDMMSourcePlay(handle, av_select, wfd_mm_stream_play_cb);
    if(result == OMX_ErrorNone)
    {
       return WFD_STATUS_SUCCESS;
    }
  }
  return WFD_STATUS_FAIL;
}

/** @brief getcurrent VideoTimeStamp
  *
  * @param[in]  WFD_MM_HANDLE - WFD MM instance handle
    @param[out] timeStamp     - current Video Timestamp in MS
  *
  * @return  WFD_status_t - status
  */

WFD_status_t wfd_mm_get_current_PTS_source(WFD_MM_HANDLE handle,uint64 *timeStamp)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "wfd_mm_getCurrent_VideoTimeStamp_source");
  OMX_ERRORTYPE result = OMX_ErrorNone;
  if( handle && timeStamp)
  {
    WFDMMSource *WFDMMSourceHandle = (WFDMMSource *)handle;
    result = WFDMMSourceHandle->GetCurrentPTS(handle,(OMX_U64*)timeStamp);

    if(result == OMX_ErrorNone)
    {
      return WFD_STATUS_SUCCESS;
    }
  }
  return WFD_STATUS_FAIL;
}


/** @brief Pause the session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the pausing of Audio + Video or Audio only or Video only
               wfd_mm_capability_change_cb  - Callback to inform session manager if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_pause_source(WFD_MM_HANDLE handle, WFD_AV_select_t av_select, void (*wfd_mm_stream_pause_cb)(WFD_MM_HANDLE, WFD_status_t status))
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "wfd_mm_stream_pause_source");
  OMX_ERRORTYPE result = OMX_ErrorNone;
  if( handle && wfd_mm_stream_pause_cb)
  {
    WFDMMSource *WFDMMSourceHandle = (WFDMMSource *)handle;
    result = WFDMMSourceHandle->WFDMMSourcePause(handle, av_select, wfd_mm_stream_pause_cb);
    if(result == OMX_ErrorNone)
    {
       return WFD_STATUS_SUCCESS;
    }
  }
  return WFD_STATUS_FAIL;
}

/** @brief update the session with new capabilty parameters
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_MM_capability_t - pointer to negotiated capabilty structure
               wfd_mm_update_session_cb  - Callback to inform session manager if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_update_session_source(WFD_MM_HANDLE handle, WFD_MM_capability_t *WFD_negotiated_capability, void (*wfd_mm_update_session_cb)(WFD_MM_HANDLE, WFD_status_t status))
{
   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "wfd_mm_update_session_source");
  OMX_ERRORTYPE result = OMX_ErrorNone;
  if( handle)
  {
     WFDMMSource *WFDMMSourceHandle = (WFDMMSource *)handle;
     //need to add interface to call to pass the new changed capability and right now we need to get the new frame rate out of that
     result = WFDMMSourceHandle->WFDMMSourceUpdateSession(WFD_negotiated_capability,wfd_mm_update_session_cb) ;
     if(result == OMX_ErrorNone)
     {
        return WFD_STATUS_SUCCESS;
     }
     else
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_update_session_source status = %x",result);
  }
   return WFD_STATUS_FAIL;
}

/** @brief send IDR frame as next frame from source
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_send_IDRframe_source(WFD_MM_HANDLE handle)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "wfd_mm_send_IDRframe_source");
  OMX_ERRORTYPE result = OMX_ErrorNone;
  if( handle)
  {
     WFDMMSource *WFDMMSourceHandle = (WFDMMSource *)handle;
     result = WFDMMSourceHandle->GenerateIFrameNext(handle);
     if(result == OMX_ErrorNone)
     {
        return WFD_STATUS_SUCCESS;
     }
     else
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_send_IDRframe_source status = %x",result);
  }
   return WFD_STATUS_FAIL;
}

/** @brief set the bit rate at source
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  *@param[in] OMX_U32 - bitrate
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_set_bitrate_source(WFD_MM_HANDLE handle, uint32 nBitRate)
{
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "wfd_mm_set_bitrate_source = %ld", nBitRate);
  OMX_ERRORTYPE result = OMX_ErrorNone;
  if( handle)
  {
     WFDMMSource *WFDMMSourceHandle = (WFDMMSource *)handle;
     result = WFDMMSourceHandle->ChangeBitrate(handle, nBitRate);
     if(result == OMX_ErrorNone)
     {
        return WFD_STATUS_SUCCESS;
     }
     else
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_set_bitrate_source status = %x",result);
  }
   return WFD_STATUS_FAIL;
}

/** @brief notify audio source to open proxy device
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_send_runtime_command_source(WFD_MM_HANDLE handle,WFD_runtime_cmd_t eCommand)
{
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_send_runtime_command_source %d ", eCommand);
  OMX_ERRORTYPE result = OMX_ErrorNone;
  if( handle)
  {
     WFDMMSource *WFDMMSourceHandle = (WFDMMSource *)handle;
     result = WFDMMSourceHandle->ExecuteRunTimeCommand(handle,eCommand);
     if(result == OMX_ErrorNone)
     {
        return WFD_STATUS_SUCCESS;
     }
     else
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_send_runtime_command_source status = %x",result);
  }
   return WFD_STATUS_FAIL;
}


/** @brief set the frame rate at source
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  *@param[in] OMX_U32 - Frame rate
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_set_framerate_source(WFD_MM_HANDLE handle, uint32 nFrameRate)
{
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "wfd_mm_set_framerate_source = %ld", nFrameRate);
  OMX_ERRORTYPE result = OMX_ErrorNone;
  if( handle)
  {
     WFDMMSource *WFDMMSourceHandle = (WFDMMSource *)handle;
     result = WFDMMSourceHandle->ChangeFramerate(handle, nFrameRate);
     if(result == OMX_ErrorNone)
     {
        return WFD_STATUS_SUCCESS;
     }
     else
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "wfd_mm_set_framerate_source status = %x",result);
  }
   return WFD_STATUS_FAIL;
}

/** @brief get proposed capability
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_MM_capability_t - pointer to local capability structure
               WFD_MM_capability_t - pointer to remote capability structure
               WFD_MM_capability_t - pointer to proposed capability structure
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_proposed_capability_source(WFD_MM_HANDLE hHandle,
                                                     WFD_MM_capability_t* pMMCfg_local,
                                                     WFD_MM_capability_t* pMMCfg_remote,
                                                     WFD_MM_capability_t* pMMCfg_proposed)
{
 if((pMMCfg_proposed) && pMMCfg_local && pMMCfg_proposed && hHandle)
 {
   OMX_ERRORTYPE result;
   WFDMMSource *WFDMMSourceHandle = (WFDMMSource *)hHandle;

   result = WFDMMSourceHandle->WFDMMGetProposedCapability(hHandle,
                                                          pMMCfg_local,
                                                          pMMCfg_remote,
                                                          pMMCfg_proposed);

   if(result == OMX_ErrorNone)
    {
     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WDSMMM Found proposed capability ");
     return WFD_STATUS_SUCCESS;
    }
   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WDSMMM No available proposed capability ");
    }
    else
    {
    return WFD_STATUS_BADPARAM;
     }

      return WFD_STATUS_NOTSUPPORTED;

}

/** @brief query AV format chnage timing parameters
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  *@param[in] uint64 - uint32* pointing to 64 bit PTS
  *@param[in] uint64 - uint32* pointing to 64 bit DTS
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_av_format_change_timing_source(WFD_MM_HANDLE hHandle, uint32* pts, uint32* dts)
{
  if((pts) && (dts))
  {
     uint64 nTimeMs;
     OMX_ERRORTYPE result;
     WFDMMSource *WFDMMSourceHandle = (WFDMMSource *)hHandle;

     result = WFDMMSourceHandle->GetCurrentPTS(hHandle, &nTimeMs) ;
     //Note this gives latest PTS irrespective of audio or video. Revisit during
     //implementation of explicit AV format change
     if(result == OMX_ErrorNone)
     {
      *((uint32*)pts) = (uint32)nTimeMs;
      *((uint32*)dts) = (uint32)nTimeMs;
     }
     else
     {
      *pts = 0;
      *dts = 0;
     }
  }
  return WFD_STATUS_SUCCESS;
}

/** @brief create a hdcp source session
  *
  * @param[in] pMMCfg_local  -     local capability
  * @param[in] pMMCfg_remote -     remote capability
  * @param[in] pMMCfg_negotiated - Negotiated capability
  *
  * @return  WFD_status_t - status
  */

WFD_status_t wfd_mm_create_hdcp_session_source(WFD_MM_HANDLE *hHandle,
                                                WFD_MM_capability_t* pMMCfg_local,
                                               WFD_MM_capability_t* pMMCfg_remote,
                                               WFD_MM_capability_t* pMMCfg_negotiated )
{
    if(hHandle)
    {
      *hHandle = NULL;
    }
    (void) pMMCfg_local;
    WFD_status_t nStatus = WFD_STATUS_FAIL;
    int nEncryptAudioDecision = 0;
    uint32 uEncryptAudioStream = 0;
    int32 nRetVal = -1;

  /**---------------------------------------------------------------------
       Decision to encrypt audio or not is made by application
       or user based on the WFD config file
      ------------------------------------------------------------------------
  */

    nRetVal =  getCfgItem(ENCRYPT_AUDIO_DECISION_KEY,(int*)(&nEncryptAudioDecision));
    if(nRetVal == 0)
    {
      uEncryptAudioStream = nEncryptAudioDecision ? STREAM_AUDIO : STREAM_INVALID;
    }

    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                     "wfd_mm_create_hdcp_session_source: Audio Encryption Flag m_bEncryptFlag = %d uEncryptAudioStream = %d",
                     nEncryptAudioDecision,uEncryptAudioStream);
  // This will be called when RTSPSession get M3 message response (get callback)
  // Create a HDCP Session heare itself.
    /**-----------------------------------------------------------------------
    *  HDCP Content Protection
    *  1) Init HDCP library
    *  2) Create a new HDCP session
    *  3) Connect to downstream HDCP device
    *  4) Open HDCP stream
    *-------------------------------------------------------------------------*/
    // Create HDCP session only if remote have capability of HDCP
    if ( ( pMMCfg_remote ) /*&& ( pMMCfg_local )*/ && ( pMMCfg_negotiated ) &&
         ( pMMCfg_negotiated->HdcpCtx.hHDCPSession == NULL ) &&
         ((pMMCfg_remote->content_protection_config.content_protection_capability >= (uint8)WFD_HDCP_VERSION_2_0) &&
           (pMMCfg_remote->content_protection_config.content_protection_capability <= (uint8)WFD_HDCP_VERSION_MAX)))
    {
      //pMMCfg_local->HdcpCtx.bHDCPSEssionValid = false;
      pMMCfg_negotiated->HdcpCtx.bHDCPSEssionValid = false;
      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                  "Peer IpAddr is =%d  Hdcp Ctrl Port=%d",
                  pMMCfg_remote->peer_ip_addrs.ipv4_addr1,
                  pMMCfg_remote->content_protection_config.content_protection_ake_port );

      pMMCfg_negotiated->HdcpCtx.hHDCPSession = (CWFD_HdcpCp*)new CWFD_HdcpCp(NULL,
                                  pMMCfg_remote->peer_ip_addrs.ipv4_addr1,
                                  pMMCfg_remote->content_protection_config.content_protection_ake_port,
                                  (uint8)STREAM_VIDEO,
                                  (uint8)uEncryptAudioStream, /*STREAM_AUDIO*/
                                  SOURCE_DEVICE,
                                  pMMCfg_remote->content_protection_config.content_protection_capability);

      if( pMMCfg_negotiated->HdcpCtx.hHDCPSession != NULL)
      {
        nStatus = WFD_STATUS_SUCCESS;
        pMMCfg_negotiated->HdcpCtx.bHDCPSEssionValid = true;
      }
      else
      {
        nStatus = WFD_STATUS_FAIL;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"HDCP Lib INIT failed");
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"wfd_mm_create_hdcp_session_source():Bad parameter");
    }
    return nStatus;
}

static int wfd_mm_negotiated_value(uint32 value)
{
   int count = 0;
   while( value)
   {
      value >>= 1;
      count++;
   }
   return count;
}

static int wfd_mm_negotiated_cea_value(uint32 value,int *priority)
{
   int index = 0;
   for(int count = 0; value ; value >>= 1,count++)
   {
      if(value & 1)
      {
         if( cea_modes[count] < *priority )
         {
            *priority = cea_modes[count];
            index = count;
         }
      }
    }
   return index;
}

static int wfd_mm_negotiated_vesa_value(uint32 value, int *priority)
{
   int index = 0;
   for(int count = 0; value ; value >>= 1,count++)
   {
      if(value & 1)
      {
         if( vesa_modes[count] < *priority )
         {
            *priority = vesa_modes[count];
            index = count;
         }
      }
    }
   return index;
}

static int wfd_mm_negotiated_hh_value(uint32 value, int *priority)
{
   int index = 0;
   for( int count = 0 ; value ; value >>= 1 , count++ )
   {
      if(value & 1)
      {
         if( hh_modes[count] < *priority )
         {
            *priority = hh_modes[count];
            index = count;
         }
      }
    }
   return index;
}

} //extern "C"
