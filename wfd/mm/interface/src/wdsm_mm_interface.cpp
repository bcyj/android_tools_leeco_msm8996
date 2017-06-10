/*==============================================================================
 * @file wdsm_mm_interface.cpp
 *
*  @par DESCRIPTION:
*       Implementation of the wireless display session manager MM inteface
 *
*
*  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/mm-wfd-interface/src/wdsm_mm_interface.cpp#7 $
$DateTime: 2011/11/14 18:28:18 $

===============================================================================*/


/* =======================================================================
**               Include files for wdsm_mm_interface.cpp
** ======================================================================= */
#include "wdsm_mm_interface.h"
#include "wdsm_mm_sink_interface.h"
#include "wdsm_mm_source_interface.h"
#include "MMDebugMsg.h"
#include "WFD_HdcpCP.h"
/* ==========================================================================

   =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */
#define WFD_MM_MAX_HANDLES (4)
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */
static struct {
	WFD_MM_HANDLE hHandle;
	WFD_device_t tDevice;
} g_wfd_mm_handle_list[WFD_MM_MAX_HANDLES];

static uint32 g_wfd_mm_handle_count = 0;



/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

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

  /** @brief add MM handle to handle index
    *
    * @param[in] hHandle - WFD MM  instance handle
    * @param[in] eDeviceType - WFD Device type
    *
    * @return None
    */
static void wfd_mm_add_handle(WFD_MM_HANDLE hHandle, WFD_device_t eDeviceType)
{
    if(hHandle)
    {
      /* Search if the handle is already stored in Handle Index */
      for( int i =0; i<WFD_MM_MAX_HANDLES; i++)
      {
        if( g_wfd_mm_handle_list[i].hHandle == hHandle )
        {
          return;
        }
      }

      /* store valid handles.*/
      for( int i =0; i<WFD_MM_MAX_HANDLES; i++)
      {
        LOGD("Adding new handle %p",hHandle);
        if( g_wfd_mm_handle_list[i].hHandle == 0 )
        {
          g_wfd_mm_handle_list[i].hHandle = hHandle;
          g_wfd_mm_handle_list[i].tDevice = eDeviceType;
          g_wfd_mm_handle_count++;
          break;
        }
      }
    }

}

  /** @brief Get extended capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_extended_capability_config_t - pointer to video capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_get_extended_capability (WFD_MM_HANDLE hHandle,
                                          WFD_extended_capability_config_t *ext_capability,
                                          WFD_device_t wfd_device_type)
{
  WFD_status_t ret_status = WFD_STATUS_INVALID;
  if (wfd_device_type == WFD_DEVICE_SOURCE)
  {
    ret_status = wfd_get_extended_capability_source(hHandle,ext_capability);
  }
  else if (wfd_device_type == WFD_DEVICE_PRIMARY_SINK || wfd_device_type == WFD_DEVICE_SECONDARY_SINK )
  {
    ret_status = wfd_get_extended_capability_sink(hHandle,ext_capability);
  }
  else
  {
    ret_status = WFD_STATUS_BADPARAM;
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
WFD_status_t wfd_mm_get_video_capability(WFD_MM_HANDLE hHandle, WFD_video_codec_config_t* pVCfg, WFD_device_t tDevice)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if(tDevice == WFD_DEVICE_SOURCE)
  {
	nErr = wfd_mm_get_video_capability_source(hHandle, pVCfg,tDevice);
  }
  else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
  {
    nErr = wfd_mm_get_video_capability_sink(hHandle, pVCfg,tDevice);
  }
  return nErr;
}

/** @brief Get 3d capability parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_3d_video_codec_config_t - pointer to 3d video capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_3d_video_capability(WFD_MM_HANDLE hHandle, WFD_3d_video_codec_config_t* pVCfg, WFD_device_t tDevice)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if(tDevice == WFD_DEVICE_SOURCE)
  {
	nErr = wfd_mm_get_3d_video_capability_source(hHandle, pVCfg,tDevice);
  }
  else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
  {
    nErr = wfd_mm_get_3d_video_capability_sink(hHandle, pVCfg,tDevice);
  }
  return nErr;
}


/** @brief Get lpcm capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_lpcm_codec_config_t - pointer to lpcm capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_lpcm_capability(WFD_MM_HANDLE hHandle, WFD_lpcm_codec_config_t *pLPCM_Cfg, WFD_device_t tDevice)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if(tDevice == WFD_DEVICE_SOURCE)
  {
	nErr = wfd_mm_get_lpcm_capability_source(hHandle, pLPCM_Cfg,tDevice);
  }
  else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
  {
    nErr = wfd_mm_get_lpcm_capability_sink(hHandle, pLPCM_Cfg,tDevice);
  }
  return nErr;
}
/** @brief Get aac capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_aac_codec_config_t - pointer to aac capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_aac_capability(WFD_MM_HANDLE hHandle, WFD_aac_codec_config_t *pAAC_Cfg, WFD_device_t tDevice)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if(tDevice == WFD_DEVICE_SOURCE)
  {
	nErr = wfd_mm_get_aac_capability_source(hHandle, pAAC_Cfg,tDevice);
  }
  else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
  {
    nErr = wfd_mm_get_aac_capability_sink(hHandle, pAAC_Cfg,tDevice);
  }
  return nErr;
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
WFD_status_t wfd_mm_get_dts_codec_capability (WFD_MM_HANDLE hHandle, WFD_dts_codec_config_t* pDTS_Cfg, WFD_device_t tDevice)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if(tDevice == WFD_DEVICE_SOURCE)
  {
	nErr = wfd_mm_get_dts_codec_capability_source(hHandle, pDTS_Cfg, tDevice);
  }
  else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
  {
    nErr = wfd_mm_get_dts_codec_capability_sink(hHandle, pDTS_Cfg, tDevice);
  }
  return nErr;
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
WFD_status_t wfd_mm_get_dolby_digital_codec_capability(WFD_MM_HANDLE hHandle, WFD_dolby_digital_codec_config_t *pDolby_Cfg, WFD_device_t tDevice)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if(tDevice == WFD_DEVICE_SOURCE)
  {
	nErr = wfd_mm_get_dolby_digital_codec_capability_source(hHandle, pDolby_Cfg,tDevice);
  }
  else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
  {
    nErr = wfd_mm_get_dolby_digital_codec_capability_sink(hHandle, pDolby_Cfg,tDevice);
  }
  return nErr;
}
/** @brief Get content protection capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_content_protection_capability_config_t - pointer to content protection capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_content_protection_capability(WFD_MM_HANDLE hHandle, WFD_content_protection_capability_config_t * pCP_Cfg, WFD_device_t tDevice)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if(tDevice == WFD_DEVICE_SOURCE)
  {
	nErr = wfd_mm_get_content_protection_capability_source(hHandle, pCP_Cfg,tDevice);
  }
  else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
  {
    nErr = wfd_mm_get_content_protection_capability_sink(hHandle, pCP_Cfg,tDevice);
  }
  return nErr;
}
/** @brief Get transport capabity parameters structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_transport_capability_config_t - pointer to transport capabilty structure
               WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_transport_capability(WFD_MM_HANDLE hHandle, WFD_transport_capability_config_t * pTS_Cfg, WFD_device_t tDevice)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if(tDevice == WFD_DEVICE_SOURCE)
  {
	nErr = wfd_mm_get_transport_capability_source(hHandle, pTS_Cfg,tDevice);
  }
  else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
  {
    nErr = wfd_mm_get_transport_capability_sink(hHandle, pTS_Cfg,tDevice);
  }
  return nErr;
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
WFD_status_t wfd_mm_create_session(WFD_MM_HANDLE *phHandle, WFD_device_t tDevice, WFD_MM_capability_t *pNegCap, WFD_MM_callbacks_t* pCallback)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if( phHandle && (g_wfd_mm_handle_count < WFD_MM_MAX_HANDLES) )
  {
    *phHandle = NULL;
    if(tDevice == WFD_DEVICE_SOURCE)
    {
      nErr = wfd_mm_create_session_source(phHandle, tDevice, pNegCap,pCallback);
    }
    else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
    {
      nErr = wfd_mm_create_session_sink(phHandle, tDevice, pNegCap,pCallback);
    }

    if(*phHandle)
    {
      wfd_mm_add_handle(*phHandle,tDevice);
    }
  }
  return nErr;
}

/** @brief destroy MM session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */

WFD_status_t wfd_mm_destroy_session(WFD_MM_HANDLE hHandle)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
  int32 nHandleIndex = -1;

  /* Search Handle Index */
  for( int i =0; i<WFD_MM_MAX_HANDLES; i++)
  {
    if( g_wfd_mm_handle_list[i].hHandle == hHandle )
    {
      nHandleIndex = i;
      break;
    }
  }

  if( nHandleIndex >=0  )
  {
    if(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SOURCE)
    {
      nErr = wfd_mm_destroy_session_source(hHandle);
    }
    else if( (g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_PRIMARY_SINK)||(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SECONDARY_SINK) )
    {
      nErr = wfd_mm_destroy_session_sink(hHandle);
	}

	  /* Remove the Handle */
      g_wfd_mm_handle_list[nHandleIndex].hHandle = 0;
      g_wfd_mm_handle_count--;
  }


  return nErr;
}

/** @brief Play
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the playing of Audio + Video or Audio only or Video only
               wfd_mm_capability_change_cb  - Callback to inform session manager if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_play(WFD_MM_HANDLE hHandle, WFD_AV_select_t av_select, wfd_mm_stream_play_cb pCallback )
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
  int32 nHandleIndex = -1;

  /* Search Handle Index */
  for( int i =0; i<WFD_MM_MAX_HANDLES; i++)
  {
    if( g_wfd_mm_handle_list[i].hHandle == hHandle )
    {
	  nHandleIndex = i;
	  break;
    }
  }
  if( nHandleIndex >=0  )
  {
    if(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SOURCE)
    {
	  nErr = wfd_mm_stream_play_source(hHandle, av_select,pCallback );
    }
    else if( (g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_PRIMARY_SINK)||(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SECONDARY_SINK) )
    {
      nErr = wfd_mm_stream_play_sink(hHandle, av_select,pCallback );
    }
  }

  return nErr;
}

/** @brief Pause the session
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the pausing of Audio + Video or Audio only or Video only
               wfd_mm_capability_change_cb  - Callback to inform session manager if there is any change in the MM capabilities
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_pause(WFD_MM_HANDLE hHandle, WFD_AV_select_t av_select, wfd_mm_stream_pause_cb pCallback)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
    int32 nHandleIndex = -1;

  /* Search Handle Index */
  for( int i =0; i<WFD_MM_MAX_HANDLES; i++)
  {
    if( g_wfd_mm_handle_list[i].hHandle == hHandle )
    {
	  nHandleIndex = i;
	  break;
    }
  }
  if( nHandleIndex >=0  )
  {
    if(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SOURCE)
    {
	  nErr = wfd_mm_stream_pause_source(hHandle,av_select,pCallback);
    }
    else if( (g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_PRIMARY_SINK)||(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SECONDARY_SINK) )
    {
      nErr = wfd_mm_stream_pause_sink(hHandle,av_select,pCallback);
    }
  }
  return nErr;
}


/** @brief standby
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the pausing of Audio + Video or Audio only or Video only
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_standby(WFD_MM_HANDLE hHandle, wfd_mm_stream_pause_cb pCallback)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
    int32 nHandleIndex = -1;

  /* Search Handle Index */
  for( int i =0; i<WFD_MM_MAX_HANDLES; i++)
  {
    if( g_wfd_mm_handle_list[i].hHandle == hHandle )
    {
	  nHandleIndex = i;
	  break;
    }
  }
  if( nHandleIndex >=0  )
  {
    if(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SOURCE)
    {
	  nErr = wfd_mm_stream_pause_source(hHandle,WFD_STREAM_AV,pCallback);
    }
    else if( (g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_PRIMARY_SINK)||(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SECONDARY_SINK) )
    {
      nErr = wfd_mm_stream_pause_sink(hHandle,WFD_STREAM_AV,pCallback);
    }
  }
  return nErr;
}

/** @brief resume
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_AV_select_t - to select the playing of Audio + Video or Audio only or Video only
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_stream_resume(WFD_MM_HANDLE hHandle)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
  int32 nHandleIndex = -1;

  /* Search Handle Index */
  for( int i =0; i<WFD_MM_MAX_HANDLES; i++)
  {
    if( g_wfd_mm_handle_list[i].hHandle == hHandle )
    {
	  nHandleIndex = i;
	  break;
    }
  }
  if( nHandleIndex >=0  )
  {
    if(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SOURCE)
    {
	  nErr = wfd_mm_stream_play_source(hHandle, WFD_STREAM_AV,NULL );
    }
    else if( (g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_PRIMARY_SINK)||(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SECONDARY_SINK) )
    {
      nErr = wfd_mm_stream_play_sink(hHandle, WFD_STREAM_AV,NULL );
    }
  }

  return nErr;
}



/** @brief getcurrent VideoTimeStamp
  *
  * @param[in]  WFD_MM_HANDLE - WFD MM instance handle
    @param[out] timeStamp     - current Video Timestamp in MS
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_current_PTS(WFD_MM_HANDLE hHandle, uint64 *timeStamp)
{
    WFD_status_t nErr = WFD_STATUS_FAIL;
    int32 nHandleIndex = -1;

    /* Search Handle Index */
    for( int i =0; i<WFD_MM_MAX_HANDLES; i++)
    {
      if( g_wfd_mm_handle_list[i].hHandle == hHandle )
      {
        nHandleIndex = i;
        break;
      }
    }

    if( nHandleIndex >=0  && timeStamp)
    {
        if(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SOURCE)
        {
            nErr = wfd_mm_get_current_PTS_source(hHandle,timeStamp);
            return nErr;
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
WFD_status_t wfd_mm_update_session(WFD_MM_HANDLE hHandle, WFD_MM_capability_t* pCapability, wfd_mm_update_session_cb pCallback)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
  int32 nHandleIndex = -1;

  /* Search Handle Index */
  for( int i =0; i<WFD_MM_MAX_HANDLES; i++)
  {
    if( g_wfd_mm_handle_list[i].hHandle == hHandle )
    {
      nHandleIndex = i;
      break;
    }
  }
  if( nHandleIndex >=0  )
  {
    if(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SOURCE)
    {
      nErr = wfd_mm_update_session_source(hHandle, pCapability,pCallback );
    }
    else if( (g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_PRIMARY_SINK)||(g_wfd_mm_handle_list[nHandleIndex].tDevice == WFD_DEVICE_SECONDARY_SINK) )
    {
      nErr = wfd_mm_update_session_sink(hHandle, pCapability,pCallback );
    }
  }

  return nErr;
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
WFD_status_t wfd_mm_get_negotiated_capability (WFD_MM_HANDLE hHandle,
                                               WFD_MM_capability_t* pMMCfg_local,
                                               WFD_MM_capability_t* pMMCfg_remote,
                                               WFD_MM_capability_t* pMMCfg_negotiated,
                                               WFD_MM_capability_t* pMMCfg_common)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
  nErr = wfd_mm_get_negotiated_capability_source(hHandle, pMMCfg_local, pMMCfg_remote, pMMCfg_negotiated,pMMCfg_common);
  return nErr;
}

/** @brief Get local MM capability structure
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
               WFD_MM_capability_t - pointer to local capability structure
                WFD_device_t  - WFD device type
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_local_capability
  (WFD_MM_HANDLE hHandle, WFD_MM_capability_t* pMMCfg, WFD_device_t tDevice)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if(tDevice == WFD_DEVICE_SOURCE)
  {
    nErr = wfd_mm_get_local_capability_source(hHandle, pMMCfg, tDevice);
  }
  else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
  {
    nErr = wfd_mm_get_local_capability_sink(hHandle, pMMCfg,tDevice);
  }
  return nErr;
}
/** @brief send IDR frame from source
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_send_IDRframe(WFD_MM_HANDLE hHandle)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
  nErr = wfd_mm_send_IDRframe_source(hHandle);
  return nErr;
}

/** @brief set the frame rate at source
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  *@param[in] uint32 - Frame rate
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_set_framerate( WFD_MM_HANDLE hHandle, uint32 nFrameRate)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
  nErr = wfd_mm_set_framerate_source(hHandle, nFrameRate);
  return nErr;
}

/** @brief set the bit rate at source
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  *@param[in] uint32 - bitrate
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_set_bitrate
  (WFD_MM_HANDLE hHandle, uint32 nBitRate)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
  nErr = wfd_mm_set_bitrate_source(hHandle, nBitRate);
  return nErr;
}

/** @brief notify MM layer with runtime command
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  * @return  WFD_status_t - status
  */

WFD_status_t wfd_mm_send_runtime_command(WFD_MM_HANDLE hHandle, WFD_runtime_cmd_t eCommand)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
  nErr = wfd_mm_send_runtime_command_source(hHandle, eCommand);
  return nErr;
}



/** @brief query AV format chnage timing parameters
  *
  * @param[in] WFD_MM_HANDLE - WFD MM Source instance handle
  *
  *@param[in] uint32 - PTS
  *@param[in] uint32 - DTS
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_av_format_change_timing(WFD_MM_HANDLE hHandle, uint32* pts, uint32* dts)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;
  nErr = wfd_mm_av_format_change_timing_source(hHandle, pts, dts );
  return nErr;
}

/** @brief get proposed capability
  *
  * @param[in] WFD_MM_HANDLE - WFD MM instance handle
               WFD_MM_capability_t - pointer to local capability structure
               WFD_MM_capability_t - pointer to remote capability structure
               WFD_MM_capability_t - pointer to proposed capability structure
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_get_proposed_capability
  (WFD_MM_HANDLE hHandle, WFD_MM_capability_t* pMMCfg_local, WFD_MM_capability_t* pMMCfg_remote, WFD_MM_capability_t* pMMCfg_proposed)
{
   WFD_status_t nErr = WFD_STATUS_FAIL;
  nErr = wfd_mm_get_proposed_capability_source(hHandle, pMMCfg_local, pMMCfg_remote, pMMCfg_proposed);
  return nErr;
}
/** @brief
  *
  * @param[in] WFD_MM_HANDLE - WFD MM instance handle
  *
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_create_HDCP_session(WFD_MM_HANDLE *pHandle,
                                        WFD_device_t tDevice,
                                        WFD_MM_capability_t* pMMCfg_local,
                                        WFD_MM_capability_t* pMMCfg_remote,
                                        WFD_MM_capability_t* pMMCfg_negotiated)
{
   WFD_status_t nErr = WFD_STATUS_FAIL;

   if(pHandle)
   {
     *pHandle = NULL;
   }

   if(tDevice == WFD_DEVICE_SOURCE)
   {
     nErr = wfd_mm_create_hdcp_session_source(pHandle,pMMCfg_local,pMMCfg_remote,pMMCfg_negotiated);
   }
   else if( (tDevice == WFD_DEVICE_PRIMARY_SINK)||(tDevice == WFD_DEVICE_SECONDARY_SINK) )
   {
     nErr = wfd_mm_create_hdcp_session_sink(pHandle,pMMCfg_local,pMMCfg_remote,pMMCfg_negotiated);
   }

   if(pHandle && *pHandle)
   {
     wfd_mm_add_handle(*pHandle,tDevice);
   }

   return nErr;
}

WFD_status_t wfd_mm_destroy_hdcp_session(WFD_MM_HANDLE hHandle,
                                         WFD_device_t tDevice)
{
  WFD_status_t nErr = WFD_STATUS_FAIL;

  if(tDevice != WFD_DEVICE_SOURCE)
  {
    nErr = wfd_mm_destroy_hdcp_session_sink(hHandle);
  }
  return nErr;
}
/** @brief
  *
  * @param[in] WFD_MM_HANDLE - WFD MM  instance handle
  * @param[i] control        - stream control command
  *
  * @return  WFD_status_t - status
  */
WFD_status_t wfd_mm_av_stream_control( WFD_MM_HANDLE hHandle,WFD_device_t eDeviceType,  WFD_MM_AV_Stream_Control_t eControl, int64 nControlValue)
{
  if(eDeviceType == WFD_DEVICE_PRIMARY_SINK || eDeviceType == WFD_DEVICE_SECONDARY_SINK)
  {
    return wfd_mm_av_stream_control_sink(hHandle, eControl, nControlValue);
  }
  return WFD_STATUS_NOTSUPPORTED;
}

}//extern "C"
