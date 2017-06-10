/************************************************************************* */
/**
 * DashExtractorAdaptationLayer.cpp
 * @brief Implementation of DashExtractorAdaptationLayer.
 *  DashExtractorAdaptationLayer class implements the dash adaptation
 *  Layer for Media Extractor
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */

/* =======================================================================
**               Include files for DashExtractorAdaptationLayer.cpp
** ======================================================================= */
#include "DashExtractorAdaptationLayer.h"
#include "DEALUtils.h"
#include "OMX_Types.h"

#include "MMMemory.h"
#include "QOMX_StreamingExtensions.h"
#include "QOMX_IVCommonExtensions.h"
#include "QOMX_StreamingExtensionsPrivate.h"

namespace android {
  /* =======================================================================
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

  const char *DASH_MEDIA_MIMETYPE_AUDIO_AAC = "audio/mp4a-latm";

  /* -----------------------------------------------------------------------
  ** Global Data Declarations
  ** ----------------------------------------------------------------------- */

  /* -----------------------------------------------------------------------
  ** Local Object Definitions
  ** ----------------------------------------------------------------------- */

  /* =======================================================================
  **                        Class & Function Definitions
  ** ======================================================================= */

  /** @brief   Constructor of DEAL Interface.
  *  @return
  */
  DEALInterface::DEALInterface(bool &bOk)
  {
    bOk = true;

    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALInterface::DEALInterface");

    memset(m_pDEALStates, 0, sizeof(m_pDEALStates));

    m_hDEALStateLock     = NULL;

    for(int i=0; i<MAX_NUM_TRACKS; i++)
    {
      m_hDEALTrackInfoReadLock[i] = NULL;
    }

    if(MM_CriticalSection_Create(&m_hDEALStateLock) != 0)
    {
      bOk = false;
    }

    for(int i=0; i<MAX_NUM_TRACKS && bOk; i++)
    {
      if(MM_CriticalSection_Create(&m_hDEALTrackInfoReadLock[i]) != 0)
      {
        bOk = false;
      }
    }

    if(bOk && (MM_CriticalSection_Create(&m_hDEALSeekLock) != 0))
    {
      bOk = false;
    }

    if(bOk && (MM_CriticalSection_Create(&m_hDEALFlushLock) != 0))
    {
      bOk = false;
    }

    m_pCurrentDEALState = NULL;
    mSMTPTimedTextDimensionsIndex = OMX_IndexComponentStartUnused;
    mSMTPTimedTextSubInfoIndex = OMX_IndexComponentStartUnused;
    mWatermarkExtIndex = OMX_IndexComponentStartUnused;
    mWatermarkStatusExtIndex = OMX_IndexComponentStartUnused;
    mIndexDrmExtraSampleInfo = OMX_IndexComponentStartUnused;
    mIndexDrmParamPsshInfo= OMX_IndexComponentStartUnused;

    if(bOk)
    {
      bOk = InitializeDEALStates();
    }

    if(bOk)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        "HTTP ALL Initialized");
      // Initialize the HTTP ALL State to Idle
      m_pCurrentDEALState = m_pDEALStates[DEALInterfaceStateIdle];
    }

    memset(mTrackList, 0x00, sizeof(TrackInfo) * MAX_NUM_TRACKS);

    m_handle = NULL;
    notify = NULL;
    cbData = NULL;
    m_bFlushPending = false;
    n_flushCount = 0;
    cachedSeekValue = -1;
    m_bAudioDiscontinuity = false;
    m_bVideoDiscontinuity = false;
    m_bTextDiscontinuity = false;

/*    mECALInterface = new ECALInterface();
    if (mECALInterface != NULL)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
       "Extractor Crypto Layer Initialized");
    } */
}

  /** @brief   Destructor of DEAL Interface.
  *  @return
  */
  DEALInterface::~DEALInterface()
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_DEBUG,
      "~DEALInterface");

    if(m_pCurrentDEALState)
    {
      DEALInterfaceState state = m_pCurrentDEALState->GetState();
      if(state != DEALInterfaceStateIdle)
      {
        SetDEALState(DEALInterfaceStateClosing);
      }
    }

    if(m_handle)
    {
      HTTPMMIDeviceClose(m_handle);
      m_handle = NULL;
    }

    SetDEALState(DEALInterfaceStateIdle);

    //destruct states
    for (int i=0; i< DEALInterfaceStateMax; i++)
    {
      if(m_pDEALStates[i])
      {
        MM_Delete(m_pDEALStates[i]);
        m_pDEALStates[i] = NULL;
      }
    }

    if(m_hDEALStateLock)
    {
      MM_CriticalSection_Release(m_hDEALStateLock);
      m_hDEALStateLock = NULL;
    }

    for(int i=0; i < MAX_NUM_TRACKS; i++)
    {
      MM_CriticalSection_Release(m_hDEALTrackInfoReadLock[i]);
      m_hDEALTrackInfoReadLock[i] = NULL;
    }

    if(m_uri)
    {
      MM_Free(m_uri);
      m_uri = NULL;
    }
  }

  /*
  * Register notify callback
  *
  * @param[in] NotifyCallback callback
  * @param[in] callback data
  *
  * @return
  */
  void DEALInterface::RegisterCallBack(NotifyCallback cb, void *cbdata)
  {
    notify = cb;
    cbData = cbdata;
    return;
  }

  /*
  * Stores http url and headers at DEAL Layer
  *
  * @param[in] paht http url
  * @param[in] http headers
  *
  * @return
  */
  bool DEALInterface::setDataSource(const char *path, uint32 size, const KeyedVector<String8, String8> *headers)
  {
    m_uri = NULL;

    if (path)
    {
      m_uri = (OMX_STRING) MM_Malloc((size+1) * sizeof(OMX_U8));
      if (m_uri)
      {
        std_strlcpy(m_uri, path, size+1);
      }
    }

    if(headers)
    {
      mUriHeaders = *headers;
    }

    SetDEALState(DEALInterfaceStateInit);

    return true;
  }

  /*
  * returns max tracks count
  *
  * @return int
  */
  size_t DEALInterface::countTracks()
  {
    return MAX_NUM_TRACKS;
  }

  sp<MediaSource> DEALInterface::getTrack(size_t index)
  {
    sp<MediaSource> src = NULL;
    if(index < (size_t)MAX_NUM_TRACKS)
    {
      return mTrackList[index].getOutBufferQ();
    }

    return src;
  }

  /*
  * returns track metadata
  *
  * @param[in] index : track index
  * @param[in] flags : flags associated with the track metadata
  *
  * @return
  */
  sp<MetaData> DEALInterface::getTrackMetaData(size_t index, uint32_t /*flags*/)
  {
    sp<MetaData> meta = NULL;

    if(index < (size_t)MAX_NUM_TRACKS)
    {
      MM_CriticalSection_Enter(m_hDEALTrackInfoReadLock[index]);
      meta = mTrackList[index].getMetaData();
      MM_CriticalSection_Leave(m_hDEALTrackInfoReadLock[index]);
    }

    if(meta == NULL)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "DEALInterface::getTrackMetaData portIndex %d returning meta NULL", index);
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "DEALInterface::getTrackMetaData portIndex %d returning valid meta", index);
    }

    return meta;
  }

  /*
  * get params from MMI
  *
  * @param[in] key : param key
  * @param[in/out] data : out params
  * @param[in/out] size : out param size
  *
  * @return
  */
  bool DEALInterface::getParameter(int key, void *data, size_t *size)
  {
    bool bOk = true;
    OMX_INDEXTYPE extIndex;
    MMI_GetExtensionCmdType ext;

    switch(key)
    {
    case KEY_DASH_PARAM_PREROLL:
    case KEY_DASH_PARAM_HWM:
      {
        MMI_OmxParamCmdType cmd;
        QOMX_BUFFERINGWATERMARKTYPE watermark;
        watermark.nSize = sizeof(QOMX_BUFFERINGWATERMARKTYPE);
        watermark.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
        watermark.eWaterMark = QOMX_WATERMARK_NORMAL;
        watermark.eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;
        watermark.nLevel = 0;//MAX_UINT32;
        watermark.bEnable = OMX_TRUE;

        cmd.nParamIndex = mWatermarkExtIndex;
        cmd.pParamStruct = &watermark;
        HTTPMMIDeviceCommand(m_handle, MMI_CMD_GET_STD_OMX_PARAM, &cmd);

        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "DEALInterface::getParameter KEY_DASH_PARAM_PREROLL/HWM AUDIO PORT %lu\n", watermark.nLevel);

        MMI_OmxParamCmdType cmd1;
        QOMX_BUFFERINGWATERMARKTYPE watermark1;
        watermark1.nSize = sizeof(QOMX_BUFFERINGWATERMARKTYPE);
        watermark1.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX;
        watermark1.eWaterMark = QOMX_WATERMARK_NORMAL;
        watermark1.eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;
        watermark1.nLevel = 0;//MAX_UINT32;
        watermark1.bEnable = OMX_TRUE;

        cmd1.nParamIndex = mWatermarkExtIndex;
        cmd1.pParamStruct = &watermark1;
        HTTPMMIDeviceCommand(m_handle, MMI_CMD_GET_STD_OMX_PARAM, &cmd1);
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "DEALInterface::getParameter KEY_DASH_PARAM_PREROLL/HWM VIDEO PORT %lu\n", watermark1.nLevel);

        int64 *preroll = (int64 *)data;
        *preroll  = watermark1.nLevel;
      }
      break;
    case KEY_DASH_PARAM_LWM:
      {
        int64 *lwm = (int64 *)data;

        MMI_OmxParamCmdType cmd;
        QOMX_BUFFERINGWATERMARKTYPE watermark;
        watermark.nSize = sizeof(QOMX_BUFFERINGWATERMARKTYPE);
        watermark.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
        watermark.eWaterMark = QOMX_WATERMARK_UNDERRUN;
        watermark.eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;
        watermark.nLevel = 0;
        watermark.bEnable = OMX_TRUE;

        cmd.nParamIndex = mWatermarkExtIndex;
        cmd.pParamStruct = &watermark;
        HTTPMMIDeviceCommand(m_handle, MMI_CMD_GET_STD_OMX_PARAM, &cmd);

        MMI_OmxParamCmdType cmd1;
        QOMX_BUFFERINGWATERMARKTYPE watermark1;
        watermark1.nSize = sizeof(QOMX_BUFFERINGWATERMARKTYPE);
        watermark1.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX;
        watermark1.eWaterMark = QOMX_WATERMARK_UNDERRUN;
        watermark1.eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;
        watermark1.nLevel = 0;
        watermark1.bEnable = OMX_TRUE;

        cmd1.nParamIndex = mWatermarkExtIndex;
        cmd1.pParamStruct = &watermark1;
        HTTPMMIDeviceCommand(m_handle, MMI_CMD_GET_STD_OMX_PARAM, &cmd1);

        *lwm = watermark1.nLevel;
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "DEALInterface::getParameter KEY_DASH_PARAM_LWM ALL PORT %lld\n", *lwm);
      }
      break;
    case KEY_DASH_PARAM_BUF_DURATION:
      {
        int64 *duration = (int64 *)data;

        MMI_OmxParamCmdType cmd;
        QOMX_BUFFERED_DURATION buf_dur;

        buf_dur.nPortIndex = OMX_ALL;
        buf_dur.bufDuration = 0;

        cmd.nParamIndex = mBuffDurationExtIndex;
        cmd.pParamStruct = &buf_dur;
        HTTPMMIDeviceCommand(m_handle, MMI_CMD_GET_STD_OMX_PARAM, &cmd);

        *duration = buf_dur.bufDuration;
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "DEALInterface::getParameter KEY_DASH_PARAM_BUF_DURATION ALL PORT %lld\n", *duration);

      }
      break;

    case KEY_DASH_ADAPTION_PROPERTIES :
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH, "DEALInterface::getParameter OMX_QUALCOMM_INDEX_PARAM_COMPLETE_DASH_ADAPTATION_PROPERTIES");
        ext.cParamName = (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_COMPLETE_DASH_ADAPTATION_PROPERTIES;
        break;
      }

    case KEY_DASH_MPD_QUERY:
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH, "DEALInterface::getParameter OMX_QUALCOMM_INDEX_PARAM_DASH_MPD");
        ext.cParamName = (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_DASH_MPD;
        break;
      }

    case KEY_DASH_QOE_PERIODIC_EVENT:
      //todo
      break;

    default:
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DEALInterface::getParameter unsupported key");
      bOk = false; //Unsupported getParameter call. Return false.
    }

    if(bOk && (KEY_DASH_ADAPTION_PROPERTIES == key || KEY_DASH_MPD_QUERY == key || KEY_DASH_QOE_PERIODIC_EVENT == key))
    {
      ext.pIndex = &extIndex;
      OMX_U32 err = HTTPMMIDeviceCommand(m_handle, MMI_CMD_GET_EXTENSION_INDEX, &ext);
      if (err == MMI_S_COMPLETE)
      {
        MMI_OmxParamCmdType param;
        param.nParamIndex = extIndex;

        if(data == NULL /*query size of data to allocate data memory*/)
        {
          QOMX_DASHPROPERTYINFO paramSize;

          paramSize.nSize = (sizeof(QOMX_DASHPROPERTYINFO));
          paramSize.nPropertiesSize = 0;
          paramSize.nPortIndex = OMX_ALL;
          param.pParamStruct = (OMX_PTR)(&paramSize);
          (void)HTTPMMIDeviceCommand(m_handle, MMI_CMD_GET_STD_OMX_PARAM, &param);

          (*size) = paramSize.nPropertiesSize;
        }
        else
        {
          QOMX_DASHPROPERTYINFO *paramData = (QOMX_DASHPROPERTYINFO*) malloc(sizeof(QOMX_DASHPROPERTYINFO) + (*size) + 1);

          if (paramData == NULL)
          {
            QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
              "DEALInterface::getParameter Out of memory\n");
            bOk = false;
          }
          else
          {
            paramData->nSize = sizeof(QOMX_DASHPROPERTYINFO) + (*size) + 1;
            paramData->nPropertiesSize = *size;
            paramData->nPortIndex = OMX_ALL;
            param.pParamStruct = (OMX_PTR)(paramData);
            (void)HTTPMMIDeviceCommand(m_handle, MMI_CMD_GET_STD_OMX_PARAM, &param);

            memcpy(data, (void*)paramData->cDashProperties, *size);

            free(paramData);

            QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH, "DEALInterface::getParameter %s", (char*) data);
          }
        }
      }
      else
      {
        bOk = false;
      }
    }

    return bOk;
  }



  bool DEALInterface::setParameter(int key, void *data, size_t size)
  {
    bool bOk = true;

    OMX_U32 err = MMI_S_EBADPARAM;
    OMX_INDEXTYPE extIndex;
    MMI_GetExtensionCmdType ext;

    switch(key)
    {
    case KEY_DASH_PARAM_PREROLL:
    case KEY_DASH_PARAM_HWM:
      {
        int64 *preroll = (int64 *)data;
        MMI_OmxParamCmdType cmd;
        QOMX_BUFFERINGWATERMARKTYPE watermark;
        watermark.nSize = sizeof(QOMX_BUFFERINGWATERMARKTYPE);
        watermark.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
        watermark.eWaterMark = QOMX_WATERMARK_NORMAL;
        watermark.eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;
        watermark.nLevel = OMX_U32(*preroll);//MAX_UINT32;
        watermark.bEnable = OMX_TRUE;

        cmd.nParamIndex = mWatermarkExtIndex;
        cmd.pParamStruct = &watermark;
        HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_STD_OMX_PARAM, &cmd);

        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "DEALInterface::setParameter KEY_DASH_PARAM_PREROLL/HWM AUDIO PORT %lu\n", watermark.nLevel);

        MMI_OmxParamCmdType cmd1;
        QOMX_BUFFERINGWATERMARKTYPE watermark1;
        watermark1.nSize = sizeof(QOMX_BUFFERINGWATERMARKTYPE);
        watermark1.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX;
        watermark1.eWaterMark = QOMX_WATERMARK_NORMAL;
        watermark1.eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;
        watermark1.nLevel = OMX_U32(*preroll);//MAX_UINT32
        watermark1.bEnable = OMX_TRUE;

        cmd1.nParamIndex = mWatermarkExtIndex;
        cmd1.pParamStruct = &watermark1;
        HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_STD_OMX_PARAM, &cmd1);
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "DEALInterface::setParameter KEY_DASH_PARAM_PREROLL/HWM VIDEO PORT %lu\n", watermark1.nLevel);

      }
      break;
    case KEY_DASH_PARAM_LWM:
      {
        int64 *lwm = (int64 *)data;

        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "DEALInterface::setParameter KEY_DASH_PARAM_LWM ALL PORT can not set %lld\n", *lwm);

        MMI_OmxParamCmdType cmd;
        QOMX_BUFFERINGWATERMARKTYPE watermark;
        watermark.nSize = sizeof(QOMX_BUFFERINGWATERMARKTYPE);
        watermark.nPortIndex = OMX_ALL;
        watermark.eWaterMark = QOMX_WATERMARK_UNDERRUN;
        watermark.eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;
        watermark.nLevel = OMX_U32(*lwm);//MAX_UINT32
        watermark.bEnable = OMX_TRUE;

        cmd.nParamIndex = mWatermarkExtIndex;
        cmd.pParamStruct = &watermark;
        HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_STD_OMX_PARAM, &cmd);
      }
      break;
    case KEY_DASH_ADAPTION_PROPERTIES :
      {
        ext.cParamName = (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_SELECTED_DASH_ADAPTATION_PROPERTIES;
        ext.pIndex = &extIndex;
        err = HTTPMMIDeviceCommand(m_handle, MMI_CMD_GET_EXTENSION_INDEX, &ext);
        if (err == MMI_S_COMPLETE)
        {
          MMI_OmxParamCmdType param;
          param.nParamIndex = extIndex;

          QOMX_DASHPROPERTYINFO * paramData = (QOMX_DASHPROPERTYINFO*) malloc(sizeof(QOMX_DASHPROPERTYINFO) + (size) + 1);

          if (paramData == NULL)
          {
            QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
              "DEALInterface::setParameter Out of memory\n");
            bOk = false;
          }
          else
          {
            paramData->nPropertiesSize = size;
            paramData->nSize = (sizeof(QOMX_DASHPROPERTYINFO) + (size) + 1);
            paramData->nPortIndex = OMX_ALL;
            memcpy((void *)paramData->cDashProperties, data, size + 1);

            param.pParamStruct = (OMX_PTR)paramData;
            (void)HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_STD_OMX_PARAM, &param);
            free(paramData);
          }
        }
        else
        {
          bOk = false;
        }
        break;
      }

    case KEY_DASH_QOE_EVENT:
      // ****** TODO *******
      break;

    default:
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DEALInterface::setParameter unsupported key");
      bOk = false;
    }

    return bOk;
  }

  /*
  * process prepare Async
  *
  * @return true if cmd is succesfually posted
  */
  bool DEALInterface::prepareAsync()
  {
    bool ret = false;
    ret = ProcessCmd(DEALInterface::DEAL_PREPARE);

    if(!ret)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
        "DEALInterface::prepareAsync failed");
    }

    return ret;
  }

  /*
  * process process start
  *
  * @return true if cmd is succesfually posted
  */
  bool DEALInterface::start()
  {
    bool bOk = false;
    bOk = ProcessCmd(DEALInterface::DEAL_START);

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
      "DEALInterface::start isSuccess %d", bOk);

    return bOk;
  }

  /*
  * process seek
  *
  * @return true if cmd is succesfually posted
  */
  bool DEALInterface::seekTo(int64_t seekTimeUs)
  {
    bool bOk = false;

    bOk = ProcessCmd(DEALInterface::DEAL_SEEK, 0, 0, seekTimeUs);

    if(notify && bOk)
    {
      notify(DEAL_SEEK_RSP, 1, cbData, NULL);
    }

    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
      "DEALInterface::seekTo called seektime %lld isSuccess %d", seekTimeUs, bOk);
    return bOk;
  }

  /*
  * process stop
  *
  * @return true if cmd is succesfually posted
  */
  bool DEALInterface::stop()
  {
    bool bOk = false;

    bOk = ProcessCmd(DEALInterface::DEAL_STOP);

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
      "DEALInterface::stop isSuccess %d", bOk);


    return bOk;
  }

  /*
  * process pause
  *
  * @return true if cmd is succesfually posted
  */
  bool DEALInterface::pause()
  {
    bool bOk = false;

    bOk = ProcessCmd(DEALInterface::DEAL_PAUSE);

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
      "DEALInterface::pause isSuccess %d", bOk);

    return bOk;
  }

  /*
  * process resume cmd
  *
  * @return true if cmd is succesfually posted
  */
  bool DEALInterface::resume()
  {
    bool bOk = false;

    bOk = ProcessCmd(DEALInterface::DEAL_RESUME);

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
      "DEALInterface::resume isSuccess %d", bOk);


    return bOk;
  }


  DEALInterface::InBufferQ::InBufferQ() {}

  DEALInterface::InBufferQ::~InBufferQ() {}

  void DEALInterface::InBufferQ::enqueue(sp<DEALInterface::InBufferQItem> buf)
  {
    Mutex::Autolock Lock(mInBufQLock);
    mBuffers.push_back(buf);
  }

  status_t DEALInterface::InBufferQ::dequeue(sp<DEALInterface::InBufferQItem> *buf)
  {
    Mutex::Autolock Lock(mInBufQLock);
    if (!mBuffers.empty()) {
      *buf = *mBuffers.begin();
      mBuffers.erase(mBuffers.begin());
      return OK;
    }
    *buf = NULL;
    return WOULD_BLOCK;
  }

  uint32 DEALInterface::InBufferQ::getQueueSize()
  {
    return (uint32)mBuffers.size();
  }


  bool DEALInterface::InitializeDEALStates()
  {
    bool bOk = true;

    m_pDEALStates[DEALInterfaceStateIdle] = MM_New_Args( DEALStateIdle, (*this));
    if(NULL == m_pDEALStates[DEALInterfaceStateIdle])
    {
      bOk = false;
    }

    if(true == bOk)
    {
      m_pDEALStates[DEALInterfaceStateInit] = MM_New_Args( DEALStateInit, (*this));
      if(NULL == m_pDEALStates[DEALInterfaceStateInit])
      {
        bOk = false;
      }
    }

    if(true == bOk)
    {
      m_pDEALStates[DEALInterfaceStateLoading] = MM_New_Args( DEALStateLoading, (*this));
      if(NULL == m_pDEALStates[DEALInterfaceStateLoading])
      {
        bOk = false;
      }
    }

    if(true == bOk)
    {
      m_pDEALStates[DEALInterfaceStateLoaded] = MM_New_Args( DEALStateLoaded, (*this));
      if(NULL == m_pDEALStates[DEALInterfaceStateLoaded])
      {
        bOk = false;
      }
    }

    if(true == bOk)
    {
      m_pDEALStates[DEALInterfaceStateOpening] = MM_New_Args( DEALStateOpening, (*this));
      if(NULL == m_pDEALStates[DEALInterfaceStateOpening])
      {
        bOk = false;
      }
    }

    if(true == bOk)
    {
      m_pDEALStates[DEALInterfaceStatePlaying] = MM_New_Args( DEALStatePlaying, (*this));
      if(NULL == m_pDEALStates[DEALInterfaceStatePlaying])
      {
        bOk = false;
      }
    }

    if(true == bOk)
    {
      m_pDEALStates[DEALInterfaceStateSeeking] = MM_New_Args( DEALStateSeeking, (*this));
      if(NULL == m_pDEALStates[DEALInterfaceStateSeeking])
      {
        bOk = false;
      }
    }

    if(true == bOk)
    {
      m_pDEALStates[DEALInterfaceStatePausing] = MM_New_Args( DEALStatePausing, (*this));
      if(NULL == m_pDEALStates[DEALInterfaceStatePausing])
      {
        bOk = false;
      }
    }

    if(true == bOk)
    {
      m_pDEALStates[DEALInterfaceStatePaused] = MM_New_Args( DEALStatePaused, (*this));
      if(NULL == m_pDEALStates[DEALInterfaceStatePaused])
      {
        bOk = false;
      }
    }

    if(true == bOk)
    {
      m_pDEALStates[DEALInterfaceStateResuming] = MM_New_Args( DEALStateResuming, (*this));
      if(NULL == m_pDEALStates[DEALInterfaceStateResuming])
      {
        bOk = false;
      }
    }

    if(true == bOk)
    {
      m_pDEALStates[DEALInterfaceStateClosing] = MM_New_Args( DEALStateClosing, (*this));
      if(NULL == m_pDEALStates[DEALInterfaceStateClosing])
      {
        bOk = false;
      }
    }

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALInterface State initialize return %d", bOk);
    return bOk;
  }

  /*
  * handles asynchronous events from MMI
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  * @param[in] pClientData client data provided by AAL at registration
  *
  * @return
  */
  void DEALInterface::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData,
    OMX_IN OMX_PTR pClientData)
  {
    DEALInterface *pDataCache = (DEALInterface*)pClientData;

    if (pDataCache)
    {
      DEALStateBase *pCurrState = pDataCache->GetCurrentDEALState();
      QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
        "EventHandler Process event %lu %lu by state %d",
        nEvtCode, nEvtStatus, pCurrState->GetState());

      pCurrState->EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
    }
    else
    {
      QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
        "EventHandler Drop event %lu %lu %x",
        nEvtCode, nEvtStatus, pEvtData );
    }
  }

  /*
  * Returns the current HTTP DEAL State
  *
  * return refernce to DEALStateBase derived State
  */
  DEALInterface::DEALStateBase *DEALInterface::GetCurrentDEALState()
  {
    DEALStateBase *pDEALCurrentState = NULL;
    MM_CriticalSection_Enter(m_hDEALStateLock);
    pDEALCurrentState = m_pCurrentDEALState;
    MM_CriticalSection_Leave(m_hDEALStateLock);
    return pDEALCurrentState;
  }


  /*
  * Initiates the transtion to the new state
  *
  * @param[in] eDEALState DEAL MMI State to transtion to
  * @param[in] DEALRsp MMI response that is sent after the transition is
  * complete
  *
  * @return true on success else failure
  */
  bool DEALInterface::SetDEALState(DEALInterfaceState eDEALState)
  {
    bool bReturn = false;
    DEALStateBase *pDEALCurrentState = GetCurrentDEALState();
    if(pDEALCurrentState)
    {
      QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        "SetDEALState from %d to %d",
        pDEALCurrentState->GetState(), eDEALState);
      bReturn = pDEALCurrentState->ExitHandler();
      if (bReturn)
      {
        MM_CriticalSection_Enter(m_hDEALStateLock);
        m_pCurrentDEALState = m_pDEALStates[eDEALState];
        pDEALCurrentState = m_pCurrentDEALState;
        MM_CriticalSection_Leave(m_hDEALStateLock);
        bReturn = pDEALCurrentState->EntryHandler();
      }
    }
    return bReturn;
  }

  /*
  * Get MMI Handle
  *
  * @param[in]
  *
  * @return MMI handle
  */
  OMX_HANDLETYPE DEALInterface::GetMMIHandle()
  {
    return m_handle;
  }

  /*
  * Set MMI Handle
  *
  * @param[in] handle
  *
  * @return none
  */
  void DEALInterface::SetMMIHandle(OMX_HANDLETYPE handle)
  {
    m_handle = handle;
  }

  /*
  * process commands given by framework
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  bool DEALInterface::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 arg1, int32 arg2, int64 arg3)
  {
    OMX_U32 nReturn = 0;

    DEALStateBase *pCurrState = GetCurrentDEALState();
    if (pCurrState)
    {
      nReturn = pCurrState->ProcessCmd(cmd, arg1, arg2, arg3);
    }

    return IS_SUCCESS(nReturn);
  }

  /**
  * C'tor
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateBase::DEALStateBase(DEALInterface      &pDataCache,
    DEALInterfaceState eDEALState):
  m_pMMHTTPDataCache(pDataCache),
    m_eDEALState(eDEALState)
  {
  }

  /**
  * D'tor
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateBase::~DEALStateBase()
  {
  }

  /*
  * EntryHandler, called when enters state
  *
  * @param[in] none
  *
  * @return true when successful, false otherwise
  */
  bool DEALInterface::DEALStateBase::EntryHandler()
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "DEALInterface::DEALStateBase::EntryHandler");
    return true;
  }

  /*
  * process commands given by framework
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  * @param[in] arg3 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStateBase::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 /*arg1*/, int32 /*arg2*/, int64 /*arg3*/)
  {
    OMX_U32 ret = MMI_S_EFAIL;

    switch(cmd)
    {

    case DEAL_STOP:
      {
        m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateClosing);
      }
      break;

    default:
      // Not Supported
      break;

    }

    return ret;
  }

  /*
  * handles asynchronous events from MMI in AAL state base
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStateBase::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 /*nPayloadLen*/,
    OMX_IN OMX_PTR pEvtData)
  {
    switch(nEvtCode)
    {

    case MMI_RESP_FLUSH:
      {
        // Ignore flush in this state
        m_pMMHTTPDataCache.incFlushCount();
        if(MAX_NUM_TRACKS+1 == m_pMMHTTPDataCache.getFlushCount())
        {
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStateBase::EventHandler flush response status %d", DEALUtils::MapMMIToDEALStatus(nEvtStatus));

          // Reset the variables
          m_pMMHTTPDataCache.setFlushPending(false);
          m_pMMHTTPDataCache.resetFlushCount();
        }
      }
      break;
    case MMI_EVT_QOMX_EXT_SPECIFIC:
      {
        if (nEvtStatus == MMI_S_COMPLETE)
        {
          MMI_ExtSpecificMsgType *pExtSpecificInfo = (MMI_ExtSpecificMsgType *)pEvtData;
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH, "DEALStateBase::EventHandler - ext specific");
          if ((pExtSpecificInfo != NULL) &&
            (pExtSpecificInfo->nData2 == (OMX_U32)m_pMMHTTPDataCache.mWatermarkStatusExtIndex))
          {
            QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "DEALStateBase::EventHandler, buffering status received in port %lu ", pExtSpecificInfo->nData1);

            OMX_U32 ret = MMI_S_EFAIL;
            MMI_OmxParamCmdType cmd;
            QOMX_BUFFERINGSTATUSTYPE bufferingStatus;
            bufferingStatus.nSize = sizeof(bufferingStatus);
            bufferingStatus.nPortIndex = pExtSpecificInfo->nData1;
            cmd.nParamIndex = m_pMMHTTPDataCache.mWatermarkStatusExtIndex;
            cmd.pParamStruct = &bufferingStatus;
            ret = HTTPMMIDeviceCommand(m_pMMHTTPDataCache.GetMMIHandle(), MMI_CMD_GET_STD_OMX_PARAM, &cmd);

            DEALInterface::DASHSourceTrackType nTrackId = DEALInterface::eTrackNone;
            nTrackId = DEALUtils::MapPortIndexToTrackId((uint32)pExtSpecificInfo->nData1);

            QTV_MSG_PRIO4(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "DEALStateBase::EventHandler, track %d buffering status  %d, unit type %d, level %d", (int)nTrackId, (int)bufferingStatus.eCurrentWaterMark, (int)bufferingStatus.eUnitsType, (int)bufferingStatus.nCurrentLevel);

            if(nTrackId != DEALInterface::eTrackNone)
            {
              int NotifyUnderRun = -1;
              if (bufferingStatus.eCurrentWaterMark == QOMX_WATERMARK_UNDERRUN)
              {
                NotifyUnderRun = DEAL_BUFFERING_START;
              }
              else if(bufferingStatus.eCurrentWaterMark == QOMX_WATERMARK_NORMAL)
              {
                NotifyUnderRun = DEAL_BUFFERING_END;
              }

              if(m_pMMHTTPDataCache.notify)
              {
                m_pMMHTTPDataCache.notify(NotifyUnderRun, nTrackId, m_pMMHTTPDataCache.cbData, NULL);
              }
            }
          }
        }
      }
      break;

    case MMI_EVT_RESOURCES_LOST:
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "DEALStateBase::EventHandler MMI_EVT_RESOURCES_LOST");
        m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
      }
      break;

    default:
      //ignore
      break;
    }

  }

  /*
  * ExitHandler, called when state exits
  *
  * @param[in] none
  *
  * @return true when successful, false otherwise
  */
  bool DEALInterface::DEALStateBase::ExitHandler()
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "DEALStateBase::ExitHandler");
    return true;
  }


  /**
  * C'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateIdle::DEALStateIdle(DEALInterface &pDataCache):
  DEALStateBase(pDataCache, DEALInterfaceStateIdle)
  {
  }

  /**
  * D'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateIdle::~DEALStateIdle()
  {
  }

  /*
  * process commands given by framework in state IDLE
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStateIdle::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 /*arg1*/, int32 /*arg2*/, int64 /*arg3*/)
  {
    OMX_U32 ret = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();

    switch(cmd)
    {
    case DEALInterface::DEAL_STOP:
      {
        //just be sure close the handle
        if (handle)
        {
          HTTPMMIDeviceClose(handle);
          m_pMMHTTPDataCache.SetMMIHandle(NULL);
        }

        if(m_pMMHTTPDataCache.notify)
        {
          m_pMMHTTPDataCache.notify(DEAL_STOP_RSP, 1, m_pMMHTTPDataCache.cbData, NULL);
        }

        ret = MMI_S_COMPLETE;
      }
      break;

    default:
      // Not supported
      break;
    }

    return ret;
  }

  /*
  * handles asynchronous events from MMI in DEAL state IDLE
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStateIdle::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    switch(nEvtCode)
    {
    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
      break;
    }
  }

  /**
  * C'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateInit::DEALStateInit(DEALInterface &pDataCache):
  DEALStateBase(pDataCache, DEALInterfaceStateInit)
  {
  }

  /**
  * D'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateInit::~DEALStateInit()
  {
  }

  /*
  * process commands given by framework in state IDLE
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStateInit::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 /*arg1*/, int32 /*arg2*/, int64 /*arg3*/)
  {
    OMX_U32 ret = MMI_S_EFAIL;

    switch(cmd)
    {
    case DEALInterface::DEAL_PREPARE :
      {
        ret = m_pMMHTTPDataCache.initialize();
        if(IS_SUCCESS(ret))
        {
          ret = HTTPMMIDeviceCommand(m_pMMHTTPDataCache.GetMMIHandle(), MMI_CMD_LOAD_RESOURCES, NULL);
          if(IS_SUCCESS(ret))
          {
            m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateLoading);
          }
        }
      }
      break;

    case DEALInterface::DEAL_STOP :
      {
        // Not supported in this state. As after stop, DEAL should be in Initial stat e.
      }
      break;

    default:
      // Not supported
      break;
    }

    return ret;
  }

  /*
  * handles asynchronous events from MMI in DEAL state IDLE
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStateInit::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    switch(nEvtCode)
    {
    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
      break;
    }
  }

  status_t DEALInterface::initialize()
  {
    OMX_U32 ret = MMI_S_EFAIL;
    if (NULL == m_handle)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
        "calling HTTPMMIDeviceOpen");
      ret = HTTPMMIDeviceOpen(&m_handle);

      if (IS_SUCCESS(ret))
      {
        ret = HTTPMMIRegisterEventHandler(m_handle, EventHandler, this);
//        if (mECALInterface != NULL)
  //      {
    //      mECALInterface->SetSourceMMIHandle(m_handle);
      //  }
      }

      if (IS_SUCCESS(ret))
      {
        //uri
        MMI_OmxParamCmdType uriData;
        OMX_PARAM_CONTENTURITYPE *paramStruct;
        uint32 uriLen = std_strlen(m_uri) + 1;
        paramStruct = (OMX_PARAM_CONTENTURITYPE *)
          MM_Malloc(sizeof(OMX_PARAM_CONTENTURITYPE) + uriLen);

        if (paramStruct == NULL)
        {
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
            "Out of memory\n");
          return NO_MEMORY;
        }

        QOMX_STRUCT_INIT(*paramStruct, OMX_PARAM_CONTENTURITYPE);
        paramStruct->nSize += uriLen;
        std_strlcpy((char *)paramStruct->contentURI, m_uri, uriLen);
        uriData.nParamIndex = OMX_IndexParamContentURI;
        uriData.pParamStruct = paramStruct;

        ret = HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_STD_OMX_PARAM, &uriData);

        MM_Free(paramStruct);
        paramStruct = NULL;
      }

      if ((IS_SUCCESS(ret)))
      {
        ret = OpenMMI();
      }
    }
    else
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
        "m_handle is valid %lu, prepareAsync not possible again", ret);
    }
    return ret;
}


/*
 * Open HTTP MMI device
 *
 * @param[in]
 *
 * @return status of the operation
 */
  OMX_U32 DEALInterface::OpenMMI()
  {
    OMX_U32 ret = MMI_S_COMPLETE;

    DEALUtils::AddOemHeaders(&mUriHeaders, *this);
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Add OEM Headers)");

    if (IS_SUCCESS(ret))
    {
      //set audio port to auto detect
      MMI_CustomParamCmdType autoDetect;
      MMI_ParamDomainDefType paramAutoDetect;
      paramAutoDetect.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
      paramAutoDetect.format.audio.eEncoding = OMX_AUDIO_CodingAutoDetect;
      autoDetect.nParamIndex = MMI_IndexDomainDef;
      autoDetect.pParamStruct = &paramAutoDetect;
      ret = HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_CUSTOM_PARAM, &autoDetect);

      //set video port to auto detect
      paramAutoDetect.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX;
      paramAutoDetect.format.video.eCompressionFormat = OMX_VIDEO_CodingAutoDetect;
      autoDetect.nParamIndex = MMI_IndexDomainDef;
      autoDetect.pParamStruct = &paramAutoDetect;
      ret = HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_CUSTOM_PARAM, &autoDetect);

      //set text port to auto detect
      paramAutoDetect.nPortIndex = MMI_HTTP_OTHER_PORT_INDEX;
      paramAutoDetect.format.other.eFormat = (OMX_OTHER_FORMATTYPE)QOMX_OTHER_CodingAutoDetect;
      autoDetect.nParamIndex = MMI_IndexDomainDef;
      autoDetect.pParamStruct = &paramAutoDetect;
      ret = HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_CUSTOM_PARAM, &autoDetect);
    }

    GetIndexForDRMExtensions();
    if (IS_SUCCESS(ret))
    {
       MMI_GetExtensionCmdType ext;
       ext.cParamName = (OMX_STRING)OMX_QUALCOMM_INDEX_CONFIG_WATERMARK;

       ret = GetOmxIndexByExtensionString(ext);
       if (IS_SUCCESS(ret))
       {
         MMI_OmxParamCmdType cmd;
         QOMX_BUFFERINGWATERMARKTYPE watermark;
         watermark.nSize = sizeof(QOMX_BUFFERINGWATERMARKTYPE);
         watermark.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
         watermark.eWaterMark = QOMX_WATERMARK_NORMAL;
         watermark.eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;
         watermark.nLevel = MAX_UINT32;
         watermark.bEnable = OMX_TRUE;

         cmd.nParamIndex = mWatermarkExtIndex;
         cmd.pParamStruct = &watermark;
         HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_STD_OMX_PARAM, &cmd);

         watermark.nLevel = MAX_UINT32;
         watermark.eWaterMark = QOMX_WATERMARK_UNDERRUN;
         watermark.bEnable = OMX_TRUE;
         cmd.nParamIndex = mWatermarkExtIndex;
         cmd.pParamStruct = &watermark;
         ret = HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_STD_OMX_PARAM, &cmd);


         MMI_OmxParamCmdType cmd1;
         QOMX_BUFFERINGWATERMARKTYPE watermark1;
         watermark1.nSize = sizeof(QOMX_BUFFERINGWATERMARKTYPE);
         watermark1.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX;
         watermark1.eWaterMark = QOMX_WATERMARK_NORMAL;
         watermark1.eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;
         watermark1.nLevel = MAX_UINT32;
         watermark1.bEnable = OMX_TRUE;

         cmd1.nParamIndex = mWatermarkExtIndex;
         cmd1.pParamStruct = &watermark1;
         HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_STD_OMX_PARAM, &cmd1);

         watermark.nLevel = MAX_UINT32;
         watermark1.eWaterMark = QOMX_WATERMARK_UNDERRUN;
         watermark.bEnable = OMX_TRUE;
         cmd1.nParamIndex = mWatermarkExtIndex;
         cmd1.pParamStruct = &watermark1;
         ret = HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_STD_OMX_PARAM, &cmd1);


         MMI_GetExtensionCmdType ext;
         ext.cParamName = (OMX_STRING)OMX_QUALCOMM_INDEX_CONFIG_WATERMARKSTATUS;
         ret = GetOmxIndexByExtensionString(ext);
         if (IS_SUCCESS(ret))
         {
            QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                         "mWatermarkStatusExtIndex val (%d)",mWatermarkStatusExtIndex);
         }
       }
    }

    if(IS_SUCCESS(ret))
    {
      MMI_GetExtensionCmdType paramStructbufduration;
      memset(&paramStructbufduration,0x00,sizeof(MMI_GetExtensionCmdType));

      char* str = (char *)OMX_QUALCOMM_INDEX_PARAM_BUFFERED_DURATION;
      paramStructbufduration.cParamName = str;
      ret = GetOmxIndexByExtensionString(paramStructbufduration);
    }

    if(IS_SUCCESS(ret))
    {
      MMI_GetExtensionCmdType paramStructSMTPTimedTextDimensions;
      memset(&paramStructSMTPTimedTextDimensions,0x00,sizeof(MMI_GetExtensionCmdType));

      char* str = (char *)OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_DIMENSIONS;
      paramStructSMTPTimedTextDimensions.cParamName = str;
      ret = GetOmxIndexByExtensionString(paramStructSMTPTimedTextDimensions);
    }

    if(IS_SUCCESS(ret))
    {
      MMI_GetExtensionCmdType paramStructSMTPTimedTextSubInfo;
      memset(&paramStructSMTPTimedTextSubInfo,0x00,sizeof(MMI_GetExtensionCmdType));

      char* str1 = (char *)OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_SUBINFO;
      paramStructSMTPTimedTextSubInfo.cParamName = str1;
      ret = GetOmxIndexByExtensionString(paramStructSMTPTimedTextSubInfo);
    }

    return ret;
  }

  OMX_U32 DEALInterface::GetOmxIndexByExtensionString(MMI_GetExtensionCmdType &nMMICmdType)
  {
    OMX_U32 ret = MMI_S_EFAIL;

    if (strncmp((char *)OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_DIMENSIONS,
      nMMICmdType.cParamName,
      strlen(nMMICmdType.cParamName)) == 0)
    {
      nMMICmdType.pIndex = &mSMTPTimedTextDimensionsIndex;

      ret = HTTPMMIDeviceCommand(GetMMIHandle(), MMI_CMD_GET_EXTENSION_INDEX, &nMMICmdType);
      if (IS_SUCCESS(ret))
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALStateOpening GetOmxIndexByExtensionString (OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_DIMENSIONS) Index success (0x%x)",
          (OMX_EXTRADATATYPE)*nMMICmdType.pIndex);
        ret = MMI_S_COMPLETE;
      }
      else
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "DEALStateOpening GetOmxIndexByExtensionString (OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_DIMENSIONS) Index Failed");
      }
    }
    else if (strncmp((char *)OMX_QUALCOMM_INDEX_CONFIG_WATERMARK,
      nMMICmdType.cParamName,
      strlen(nMMICmdType.cParamName)) == 0)
    {
      nMMICmdType.pIndex = &mWatermarkExtIndex;
      ret = HTTPMMIDeviceCommand(GetMMIHandle(), MMI_CMD_GET_EXTENSION_INDEX, &nMMICmdType);
      if (IS_SUCCESS(ret))
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALStateOpening GetOmxIndexByExtensionString (OMX_QUALCOMM_INDEX_CONFIG_WATERMARK) success (0x%x)",
          (OMX_EXTRADATATYPE)*nMMICmdType.pIndex);
        ret = MMI_S_COMPLETE;
      }
      else
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "DEALStateOpening GetOmxIndexByExtensionString (OMX_QUALCOMM_INDEX_CONFIG_WATERMARK) Index Failed");
      }
    }
    else if (strncmp((char *)OMX_QUALCOMM_INDEX_CONFIG_WATERMARKSTATUS,
      nMMICmdType.cParamName,
      strlen(nMMICmdType.cParamName)) == 0)
    {
      nMMICmdType.pIndex = &mWatermarkStatusExtIndex;
      ret = HTTPMMIDeviceCommand(GetMMIHandle(), MMI_CMD_GET_EXTENSION_INDEX, &nMMICmdType);
      if (IS_SUCCESS(ret))
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALStateOpening GetOmxIndexByExtensionString (OMX_QUALCOMM_INDEX_CONFIG_WATERMARKSTATUS) success (0x%x)",
          (OMX_EXTRADATATYPE)*nMMICmdType.pIndex);
        ret = MMI_S_COMPLETE;
      }
      else
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "DEALStateOpening GetOmxIndexByExtensionString (OMX_QUALCOMM_INDEX_CONFIG_WATERMARKSTATUS) Index Failed");
      }
    }
    else if (strncmp((char *)OMX_QUALCOMM_INDEX_PARAM_BUFFERED_DURATION,
      nMMICmdType.cParamName,
      strlen(nMMICmdType.cParamName)) == 0)
    {
      nMMICmdType.pIndex = &mBuffDurationExtIndex;
      ret = HTTPMMIDeviceCommand(GetMMIHandle(), MMI_CMD_GET_EXTENSION_INDEX, &nMMICmdType);
      if (IS_SUCCESS(ret))
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALStateOpening GetOmxIndexByExtensionString (OMX_QUALCOMM_INDEX_PARAM_BUFFERED_DURATION) success (0x%x)",
          (OMX_EXTRADATATYPE)*nMMICmdType.pIndex);
        ret = MMI_S_COMPLETE;
      }
      else
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "DEALStateOpening GetOmxIndexByExtensionString (OMX_QUALCOMM_INDEX_PARAM_BUFFERED_DURATION) Index Failed");
      }
    }
    else if (strncmp((char *)OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_SUBINFO,
      nMMICmdType.cParamName,
      strlen(nMMICmdType.cParamName)) == 0)
    {
      char *str = (char *)OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_SUBINFO;
      nMMICmdType.cParamName = str;
      nMMICmdType.pIndex = &mSMTPTimedTextSubInfoIndex;
      ret = HTTPMMIDeviceCommand(GetMMIHandle(), MMI_CMD_GET_EXTENSION_INDEX, &nMMICmdType);
      if (IS_SUCCESS(ret))
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALStateConnectin::GetOmxIndexByExtensionString GetTimedTextSubInfo Index success (0x%x)",
          (OMX_EXTRADATATYPE)*nMMICmdType.pIndex);
        ret = MMI_S_COMPLETE;
      }
      else
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "DEALStateOpening::GetOmxIndexByExtensionString GetTimedTextSubInfo Index Failed");
      }
    }
    return ret;
  }

  /**
  * C'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateLoading::DEALStateLoading(DEALInterface &pDataCache):
  DEALStateBase(pDataCache, DEALInterfaceStateLoading)
  {
  }

  /**
  * D'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateLoading::~DEALStateLoading()
  {
  }

  /*
  * process commands given by framework in state IDLE
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStateLoading::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 /*arg1*/, int32 /*arg2*/, int64 /*arg3*/)
  {
    OMX_U32 ret = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();

    switch(cmd)
    {
    case DEALInterface::DEAL_STOP :
      {
        //just be sure close the handle
        m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateInit);

        if(m_pMMHTTPDataCache.notify)
        {
          m_pMMHTTPDataCache.notify(DEAL_STOP_RSP, 1, m_pMMHTTPDataCache.cbData, NULL);
        }

        ret = MMI_S_COMPLETE;
      }
      break;

    default:
      // Not supported
      break;
    }

    return ret;
  }

  /*
  * handles asynchronous events from MMI in DEAL state IDLE
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStateLoading::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    switch(nEvtCode)
    {
    case MMI_RESP_LOAD_RESOURCES:
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "MMI_RESP_LOAD_RESOURCES in DEALStateLoading is success %d", IS_SUCCESS(nEvtStatus));

        if(IS_SUCCESS(nEvtStatus))
        {
          m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateLoaded);
          uint32 portIndex;

          //Create InBufferQ and OutBufferQ for each track and populate port and track indeces.
          //OutBufferQ is the mediaSource object which player uses to read samples.
          for(int i=0; i < MAX_NUM_TRACKS; i++)
          {
            TrackInfo& trackInfo = m_pMMHTTPDataCache.mTrackList[i];

            if(i == DEALInterface::eTrackAudio)
            {
              portIndex = MMI_HTTP_AUDIO_PORT_INDEX;
            }
            else if(i == DEALInterface::eTrackVideo)
            {
              portIndex = MMI_HTTP_VIDEO_PORT_INDEX;
            }
            else if(i == DEALInterface::eTrackText)
            {
              portIndex = MMI_HTTP_OTHER_PORT_INDEX;
            }

            trackInfo.m_nPortIndex = portIndex;
            trackInfo.m_nTrackId = i;

            m_pMMHTTPDataCache.QueryAndUpdateMetaData(portIndex);
            QOMX_DRM_TYPE trackDrmType = QOMX_NO_DRM;
            ECALInterface::DASHEncryptionStatus retType = ECALInterface::NOT_ENCRYPTED;
//                 m_pMMHTTPDataCache.mECALInterface->QueryDrmInfo(
  //                               trackInfo.m_nPortIndex, (int &)trackDrmType);
            if (ECALInterface::ENCRYPTED == retType)
            {
               trackInfo.SetEncrypted(true);
               QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                      "QueryDrmInfo :: track %d Encrypted DrmType %d", trackInfo.m_nTrackId, trackDrmType);
            }
            else if (ECALInterface::NOT_ENCRYPTED == retType)
            {
              QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                      "QueryDrmInfo :: track %d Non Encrypted", trackInfo.m_nTrackId);
            }
            else
            {
              // todo notify cb error;
              QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                      "CreateCryptoSession Failed for Encrypted port %d", trackInfo.m_nPortIndex);
              m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
            }
            m_pMMHTTPDataCache.QueryAndUpdateMetaData(portIndex);

            trackInfo.createInBufferQ();
            trackInfo.createOutBufferQ(&m_pMMHTTPDataCache);
          }
        }

        if(m_pMMHTTPDataCache.notify)
        {
          m_pMMHTTPDataCache.notify(DEAL_PREPARE_RSP, IS_SUCCESS(nEvtStatus)? 1 : 0, m_pMMHTTPDataCache.cbData, NULL);
        }
      }
      break;

    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
      break;
    }
  }

  /**
  * C'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateLoaded::DEALStateLoaded(DEALInterface &pDataCache):
  DEALStateBase(pDataCache, DEALInterfaceStateLoaded)
  {
  }

  /**
  * D'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateLoaded::~DEALStateLoaded()
  {
  }

  /*
  * process commands given by framework in state IDLE
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStateLoaded::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 /*arg1*/, int32 /*arg2*/, int64 /*arg3*/)
  {
    OMX_U32 ret = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();

    switch(cmd)
    {
    case DEALInterface::DEAL_START :
      {
        //issue start to mmi
        ret = HTTPMMIDeviceCommand(handle, MMI_CMD_START, NULL);
        if(IS_SUCCESS(ret))
        {
          m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateOpening);
        }
      }
      break;
    case DEALInterface::DEAL_STOP :
      {
        m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateInit);

        if(m_pMMHTTPDataCache.notify)
        {
          m_pMMHTTPDataCache.notify(DEAL_STOP_RSP, 1, m_pMMHTTPDataCache.cbData, NULL);
        }

        ret = MMI_S_COMPLETE;

      }
      break;

    default:
      // Not supported
      break;
    }

    return ret;
  }

  /*
  * handles asynchronous events from MMI in DEAL state IDLE
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStateLoaded::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    switch(nEvtCode)
    {
    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
      break;
    }
  }


  /**
  * C'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateOpening::DEALStateOpening(DEALInterface &pDataCache):
  DEALStateBase(pDataCache, DEALInterfaceStateOpening),
    m_nNumTracksReady(0)
  {
  }

  /**
  * D'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateOpening::~DEALStateOpening()
  {
  }

  /*
  * process commands given by framework in state IDLE
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStateOpening::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 /*arg1*/, int32 /*arg2*/, int64 /*arg3*/)
  {
    OMX_U32 ret = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();

    switch(cmd)
    {
    case DEAL_STOP:
      m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateClosing);
      ret = MMI_S_COMPLETE;
      break;
    default:
      // Not supported
      break;
    }

    return ret;
  }

  /*
  * handles asynchronous events from MMI in DEAL state IDLE
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStateOpening::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    switch(nEvtCode)
    {
    case MMI_RESP_START:
      {
        if(MMI_S_COMPLETE == nEvtStatus)
        {
          DEALUtils::QueryStreamType(OMX_ALL, m_pMMHTTPDataCache);
        }
        else
        {
          if(m_pMMHTTPDataCache.notify)
          {
            m_pMMHTTPDataCache.notify(DEAL_START_RSP, 0, m_pMMHTTPDataCache.cbData, NULL);
          }
        }

        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALStateOpening start response status %d", DEALUtils::MapMMIToDEALStatus(nEvtStatus));
      }
      break;
    case MMI_EVT_PORT_CONFIG_CHANGED:
      {

        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALStateOpening config changed event");

        if (MMI_S_COMPLETE == nEvtStatus)
        {
          MMI_PortMsgType *pPort = (MMI_PortMsgType *) pEvtData;
          DEALInterface::DASHSourceTrackType nTrackId = DEALInterface::eTrackNone;

          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStateOpening config changed on port %lu success", pPort->nPortIndex);

          if(pPort)
          {
            nTrackId = DEALUtils::MapPortIndexToTrackId(pPort->nPortIndex);

            QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "DEALStateOpening config changed on port %lu track %d", pPort->nPortIndex, nTrackId);

            if(nTrackId != DEALInterface::eTrackNone)
            {
              TrackInfo& trackInfo = m_pMMHTTPDataCache.mTrackList[nTrackId];

              QOMX_DRM_TYPE trackDrmType = QOMX_NO_DRM;

              ECALInterface::DASHEncryptionStatus retType = ECALInterface::NOT_ENCRYPTED;

//                          m_pMMHTTPDataCache.mECALInterface->QueryTrackEncInfo(
  //                              trackInfo.m_nPortIndex, (int &)trackDrmType);
               QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Port %d after QueryTrackEncInfo %d",
                        trackInfo.m_nPortIndex, retType);

              if (ECALInterface::ENCRYPTED == retType)
              {
                 trackInfo.SetEncrypted(true);
                 status_t drmErr  = 0;
    //                   m_pMMHTTPDataCache.mECALInterface->CreateCryptoSession(trackInfo.m_nPortIndex);
                 if (DRM_NO_ERROR == drmErr)
                 {
                   QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Success ::CreateCryptoSession For Encrypted port %d",
                        trackInfo.m_nPortIndex);
                 }
                 else
                 {
                   QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                        "Error::CreateCryptoSession for Encrypted port %d",
                        trackInfo.m_nPortIndex);
                 }
              }
              else if (ECALInterface::NOT_ENCRYPTED == retType)
              {
                 //default it is NOT_ENCRYPTED
                 QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Port %d is NOT_ENCRYPTED",
                        trackInfo.m_nPortIndex);
              }
              else
              {
                QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                        "QueryTrackEncInfo Failed for port %d",
                        trackInfo.m_nPortIndex);
                m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
              }

              //update TrackInfo (trackId, portIndex, InBufferQ and outBufferQ, buffer reqs) for both Valid and Invalid tracks.
              //For Invalid tracks metaData set to NULL.

              QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                "DEALStateOpening config changed on port update trackInfo trackId=%d portIndex=%lu", nTrackId, pPort->nPortIndex);

              //track list lock
              MM_CriticalSection_Enter(m_pMMHTTPDataCache.m_hDEALTrackInfoReadLock[nTrackId]);

              DEALUtils::UpdateBufReq(trackInfo, pPort->nPortIndex, m_pMMHTTPDataCache);
              m_pMMHTTPDataCache.QueryAndUpdateMetaData(pPort->nPortIndex);

              MM_CriticalSection_Leave(m_pMMHTTPDataCache.m_hDEALTrackInfoReadLock[nTrackId]);
              //track list unlock
            }
          }

          m_nNumTracksReady++;

          if(MAX_NUM_TRACKS == m_nNumTracksReady)
          {
            m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStatePlaying);

            QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "DEALStateOpening config changed on port MAX_NUM_TRACKS == m_nNumTracksReady");

            //Post FTB calls on all active ports
            for(int i=0; i < MAX_NUM_TRACKS; i++)
            {
              TrackInfo& trackInfo = m_pMMHTTPDataCache.mTrackList[i];
              if(trackInfo.getMetaData() !=NULL) //valid track
              {
                for(uint32 j=0; j < trackInfo.getMaxBufferCount(); j++)
                {

                  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                    "DEALStateOpening calling readframeasync");

                  bool bReturn = m_pMMHTTPDataCache.ReadFrameAsync(trackInfo.m_nPortIndex);

                  if(!bReturn)
                  {
                    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                      "DEALStateOpening readframeasync failed");
                    m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
                  }
                }
              }
            }

            if(m_pMMHTTPDataCache.notify)
            {
              m_pMMHTTPDataCache.notify(DEAL_START_RSP, 1, m_pMMHTTPDataCache.cbData, NULL); // notify start resp success
            }
          }
        }
        else
        {
          if(m_pMMHTTPDataCache.notify)
          {
            m_pMMHTTPDataCache.notify(DEAL_START_RSP, 0, m_pMMHTTPDataCache.cbData, NULL);
          }
        }
      }
      break;
    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
      break;
    }
  }


  void DEALInterface::QueryAndUpdateMetaData(uint32 nPortIndex)
  {
    DASHSourceTrackType nTrackId = DEALUtils::MapPortIndexToTrackId(nPortIndex);

      if(nTrackId != DEALInterface::eTrackNone)
    {
      TrackInfo& trackInfo = mTrackList[nTrackId];
      DEALUtils::QueryAndUpdateMetaData(nPortIndex, trackInfo.setMetaData(),  trackInfo.IsEncrypted(), *this);
    }
    else
    {
      //error event
    }
  }


  bool DEALInterface::ReadFrameAsync(uint32 nPortIndex)
  {

    bool bOk = true;

    DASHSourceTrackType nTrackId = DEALUtils::MapPortIndexToTrackId(nPortIndex);

    if(nTrackId != DEALInterface::eTrackNone)
    {
      TrackInfo& trackInfo = mTrackList[nTrackId];

      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        "DEALInteface::readframeasync Valid track");

      if (trackInfo.getMetaData() !=NULL && trackInfo.getInBufferQ() !=NULL && trackInfo.getOutBufferQ() != NULL) //kbhima? check for metadata. fine by design. check with praveen.
      {

        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALInteface::readframeasync maxtrackbuffercount %lu", trackInfo.getMaxBufferCount());

        if ((trackInfo.getInBufferQ()->getQueueSize() + trackInfo.getOutBufferQ()->getQueueSize()) > trackInfo.getMaxBufferCount())
        {
          QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALInterface::ReadFrameAsync Already requested max %lu frames on track%d", trackInfo.getMaxBufferCount(), trackInfo.m_nTrackId);
        }
        else
        {
          sp<ABuffer> mBuffer = new ABuffer(trackInfo.getMaxBufferSize());
          OMX_HANDLETYPE handle = GetMMIHandle();

          if (handle == NULL || mBuffer == NULL )
          {
            bOk = false;
            QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
              "DEALInterface::ReadFrameAsync  MMI Handle or ABuffer NULL!");
          }
          else
          {
            sp<DEALInterface::InBufferQItem> pInBufferQItem = new DEALInterface::InBufferQItem();
            pInBufferQItem->mBuffer = mBuffer;

            MM_CriticalSection_Enter(m_hDEALTrackInfoReadLock[nTrackId]);

            QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "DEALInterface::ReadFrameAsync  port %d track %d",
              trackInfo.m_nPortIndex, trackInfo.m_nTrackId);

            mBuffer->setRange(0, trackInfo.getMaxBufferSize());

            QOMX_STRUCT_INIT(pInBufferQItem->mBufHdr, OMX_BUFFERHEADERTYPE);
            MMI_BufferCmdType dataCmd;
            dataCmd.nPortIndex = trackInfo.m_nPortIndex;
            pInBufferQItem->mBufHdr.nOutputPortIndex = trackInfo.m_nPortIndex;
            pInBufferQItem->mBufHdr.nAllocLen = (OMX_U32) mBuffer->size();
            pInBufferQItem->mBufHdr.pBuffer = (OMX_U8 *) mBuffer->data();
            dataCmd.pBufferHdr = &pInBufferQItem->mBufHdr;

            OMX_U32 nEvtStatus = MMI_S_COMPLETE;
            nEvtStatus = HTTPMMIDeviceCommand(handle, MMI_CMD_FILL_THIS_BUFFER, &dataCmd);

            QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
              "DEALInterface::ReadFrameAsync - FILL THIS BUFFER Posted to MMI Device %lu",nEvtStatus);

            if (nEvtStatus == MMI_S_PENDING || nEvtStatus == MMI_S_COMPLETE)
            {
              trackInfo.getInBufferQ()->enqueue(pInBufferQItem);
            }
            else
            {
              pInBufferQItem = NULL;
              //todo bOk = false?
            }
            MM_CriticalSection_Leave(m_hDEALTrackInfoReadLock[nTrackId]);
          }// if (handle == NULL || mBuffer == NULL )
        }// if ((trackInfo.getInBufferQ()->getQueueSize() /*+ trackInfo.getOutBufferQ().getQueueSize()*/) > trackInfo.getMaxBufferCount())
      }// if (trackInfo.getMetaData() !=NULL && trackInfo.getInBufferQ() !=NULL)
    }//if(nTrackId != DEALInterface::eTrackNone)
    else
    {
      bOk = false;
    }

    return bOk;
  }


  OMX_U32 DEALInterface::postMMICmd(OMX_HANDLETYPE handle, OMX_U32 nCode, void *pData)
  {
    return HTTPMMIDeviceCommand(handle, nCode, pData);
  }


  DEALInterface::DashStreamType DEALInterface::GetStreamType()
  {
    return eStreamType;
  }


  void DEALInterface::SetStreamType(DEALInterface::DashStreamType lStreamType)
  {
    eStreamType = lStreamType;
  }

  /**
  * C'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStatePlaying::DEALStatePlaying(DEALInterface &pDataCache):
  DEALStateBase(pDataCache, DEALInterfaceStatePlaying)
  {
  }

  /**
  * D'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStatePlaying::~DEALStatePlaying()
  {
  }

  /*
  * process commands given by framework in state Playing
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStatePlaying::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 arg1, int32 arg2, int64 arg3)
  {
    uint32 ret = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();
    bool allset = true;

    switch(cmd)
    {
    case DEALInterface::DEAL_PAUSE:
      {
        //MM_CriticalSection_Enter(m_pMMHTTPDataCache.m_hDEALStateLock);
        //ret = HTTPMMIDeviceCommand(handle, MMI_CMD_PAUSE, NULL);
        //MM_CriticalSection_Leave(m_pMMHTTPDataCache.m_hDEALStateLock);
        bool bOk = m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStatePausing);
        if(bOk)
        {
          ret = MMI_S_PENDING;
        }
      }

      break;

    case DEALInterface::DEAL_RESUME:
      // Not supported in this playing state
      break;

    case DEALInterface::DEAL_SEEK:
      {
        m_pMMHTTPDataCache.storeSeekValue(arg3);
        bool bOk = m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateSeeking);
        if(bOk)
        {
          ret = MMI_S_PENDING;
        }
      }
      break;

    case DEALInterface::DEAL_STOP:
      ret = DEALStateBase::ProcessCmd(cmd, arg1, arg2, arg3);
      break;

    default:
      break;
    }

    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALStatePlaying::ProcessCmd - %d - result = %lu", cmd, ret);

    return ret;
  }

  /*
  * handles asynchronous events from MMI in DEAL state IDLE
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStatePlaying::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    switch(nEvtCode)
    {
    case MMI_EVT_PORT_CONFIG_CHANGED:
      {
        if (MMI_S_COMPLETE == nEvtStatus)
        {
          MMI_PortMsgType *pPort = (MMI_PortMsgType *) pEvtData;

          //For 1. track removal or
          //    2. port setting change on existing track use cases
          //QueryMetaData and trackInfo update can only be done later after discontinuity read by player

          //For 3. addition of new track it is done below after notifying DEAL_FORMAT_CHANGE

          DEALInterface::DASHSourceTrackType nTrackId = DEALUtils::MapPortIndexToTrackId(pPort->nPortIndex);

          QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "CHTTPAALStatePlaying config changed in playing state for port %lu track %d", pPort->nPortIndex, nTrackId);

          if(nTrackId != DEALInterface::eTrackNone)
          {
            TrackInfo &trackInfo = m_pMMHTTPDataCache.mTrackList[nTrackId];

            MM_CriticalSection_Enter(m_pMMHTTPDataCache.m_hDEALTrackInfoReadLock[nTrackId]);

            if (trackInfo.getMetaData() == NULL)
            {
              //New track was added(like A-AV case) Send DEAL_FORMAT_CHANGE event above.

              m_pMMHTTPDataCache.QueryAndUpdateMetaData(pPort->nPortIndex);
              m_pMMHTTPDataCache.notify(DEAL_FORMAT_CHANGE, trackInfo.m_nTrackId, m_pMMHTTPDataCache.cbData, NULL);
            }
            else
            {
              //For port setting change on existing track or track removal, queueDiscontinuity will happen in read path
              m_pMMHTTPDataCache.ResetDiscontinuity(trackInfo.m_nTrackId, true);
            }

            MM_CriticalSection_Leave(m_pMMHTTPDataCache.m_hDEALTrackInfoReadLock[nTrackId]);
          }
        }
      }
      break;

    case MMI_RESP_FILL_THIS_BUFFER:
      {
        MMI_BufferCmdType *pFillBuffCmdResp = NULL;
        pFillBuffCmdResp = (MMI_BufferCmdType *) pEvtData;
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALStatePlaying::EventHandler EventCode : 0x%x port index 0x%x",(int)nEvtCode,(int)pFillBuffCmdResp->nPortIndex);

        DASHSourceTrackType nTrackId = DEALUtils::MapPortIndexToTrackId(pFillBuffCmdResp->nPortIndex);

        if(nTrackId != DEALInterface::eTrackNone)
        {
          TrackInfo &trackInfo = m_pMMHTTPDataCache.mTrackList[nTrackId];

          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStatePlaying::EventHandler resp_fill_this_buffer valid track");

          sp<DEALInterface::InBufferQItem> buffer;

          MM_CriticalSection_Enter(m_pMMHTTPDataCache.m_hDEALTrackInfoReadLock[trackInfo.m_nTrackId]);
          //dequeue from InBufferQ and queue in OutBufferQ

          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStatePlaying::EventHandler dequeue inputbuffer Q");

          trackInfo.getInBufferQ()->dequeue(&buffer);
          MM_CriticalSection_Leave(m_pMMHTTPDataCache.m_hDEALTrackInfoReadLock[trackInfo.m_nTrackId]);
          if (buffer != NULL && buffer->mBuffer != NULL)
          {
            QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "DEALStatePlaying::EventHandler calling ProcessFrameNotify");

            //bool bReturn = m_pMMHTTPDataCache.ProcessFrameNotify(buffer, trackInfo.m_nPortIndex, nEvtStatus);
            bool bReturn = m_pMMHTTPDataCache.ProcessFrameNotify(buffer->mBuffer, pFillBuffCmdResp, trackInfo.m_nPortIndex, nEvtStatus);
            buffer->mBuffer = NULL;
            buffer = NULL;

            if(!bReturn)
            {
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                           "DEALStatePlaying::EventHandler ProcessFrameNotify failed");
              m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
            }
          }
        }
        else
        {
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                        "DEALStatePlaying::EventHandler Unknown track type %d", nTrackId);
          m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
        }
      }

      break;
    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
      break;
    }
  }


  //bool DEALInterface::ProcessFrameNotify(sp<DEALInterface::InBufferQItem> pInBufferQItem, uint32 nPortIndex, OMX_U32 nEvtStatus)
  bool DEALInterface::ProcessFrameNotify(sp<ABuffer> mABuffer, MMI_BufferCmdType *pMMICmdBuf, uint32 nPortIndex, OMX_U32 nEvtStatus)

  {

    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALInterface::ProcessFrameNotify");

    bool bOk = true;
    status_t nStatus = OK;
    //sp<ABuffer> mABuffer;
    OMX_BUFFERHEADERTYPE *pBufHdr = NULL;

    /*if(pInBufferQItem != NULL)
    {
    sp<ABuffer> mABuffer = pInBufferQItem->mBuffer;
    OMX_BUFFERHEADERTYPE *pBufHdr = &pInBufferQItem->mBufHdr;
    }*/

    pBufHdr = pMMICmdBuf->pBufferHdr;

    if (pBufHdr == NULL)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        "DEALInterface::ProcessFrameNotify pBufHdr null");
    }
    else if(mABuffer == NULL)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        "DEALInterface::ProcessFrameNotify mABuffer null");
    }



    nStatus = DEALUtils::MapMMIToDEALStatus(nEvtStatus);

    DEALInterface::DASHSourceTrackType nTrackId = DEALUtils::MapPortIndexToTrackId(nPortIndex);

    if(nTrackId != DEALInterface::eTrackNone)
    {
      TrackInfo &trackInfo = mTrackList[nTrackId];

      QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        "DEALInterface::ProcessFrameNotify - nEventStatus %lu, ret %d",nEvtStatus, nStatus);

      if(trackInfo.getOutBufferQ() == NULL)
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALInterface::ProcessFrameNotify trackInfo.getOutBufferQ null");
      }

      if (pBufHdr != NULL && mABuffer != NULL && trackInfo.getOutBufferQ() != NULL)
      {
        if(isTrackDiscontinuity(nTrackId))
        {
          QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALInterface::ProcessFrameNotify DISCONTINUITY being queued on port %d track %d, due to codec change ", trackInfo.m_nPortIndex, trackInfo.m_nTrackId);

          trackInfo.getOutBufferQ()->queueDiscontinuity();

          mABuffer = NULL;
          ResetDiscontinuity(nTrackId, false);
        }
        else if(!(pBufHdr->nFlags & OMX_BUFFERFLAG_EOS) &&
          !(pBufHdr->nFlags & OMX_BUFFERFLAG_DATACORRUPT) &&
          pBufHdr->nFilledLen == 0 && (!isTrackDiscontinuity(nTrackId)))
        {
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALInterface::ProcessFrameNotify Flushed Buffer Returned");
        }
        else if (pBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
        {
          QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALInterface::ProcessFrameNotify - EOS on port %d track %d",
            trackInfo.m_nPortIndex, trackInfo.m_nTrackId);

          trackInfo.getOutBufferQ()->signalEOS(ERROR_END_OF_STREAM);

        }
        else if ((pBufHdr->nFlags & OMX_BUFFERFLAG_DATACORRUPT))
        {
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
            "DEALInterface::ProcessFrameNotify - recv. OMX_BUFFERFLAG_DATACORRUPT");
          // there is an eror with in the steam. Hence signal eos for that stream.
          SignalEOSonError(ERROR_END_OF_STREAM);
        }
        else if(nStatus != OK)
        {
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
            "DEALInterface::ProcessFrameNotify - recv. UNKNOWN_ERROR");
          // received UNKNOWN_ERROR response status. Hence signal eos for that stream.

          //kbhima? do we want to signal eos on all ports here as it is currently in AAL?
          SignalEOSonError(ERROR_END_OF_STREAM);
        }
        else
        {
        QOMX_PARAM_STREAMING_PSSHINFO *psshInfo       = NULL;
        QOMX_EXTRA_SAMPLE_INFO        *extraSampleInfo = NULL;
        if (trackInfo.IsEncrypted())
        {
#if 0
          mABuffer->meta()->setInt32("encrypted", 1);
          if (mECALInterface != NULL)
          {
            QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
             "Track Encrypted, Parse Extra SampleInfo");
            if(pBufHdr)
            {
              OMX_OTHER_EXTRADATATYPE *pExtraData = NULL;
              uint32 ulAddr = (uint32)(intptr_t)( pBufHdr->pBuffer) +  pBufHdr->nFilledLen;
              // Aligned address to DWORD boundary
              ulAddr = (ulAddr + 0x3) & (~0x3);
              pExtraData = (OMX_OTHER_EXTRADATATYPE *)(intptr_t)ulAddr;

              // Traverse the list of extra data sections
              while(pExtraData->eType != OMX_ExtraDataNone)
              {
                QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Processing pExtraData->eType(0x%x)",
                        (OMX_EXTRADATATYPE)pExtraData->eType);
                if(pExtraData->eType == (OMX_EXTRADATATYPE)mIndexDrmParamPsshInfo)
                {
                  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING,
                                QTVDIAG_PRIO_HIGH, "Found PSSH");
                  psshInfo = (QOMX_PARAM_STREAMING_PSSHINFO *)&pExtraData->data;
                  if (psshInfo)
                  {
#if DRM_DEBUG_CONTENT_PRINT
                    PrintPSSH(psshInfo);
#endif
                    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Setting PSSHInfo into meta()");
                    mABuffer->meta()->setInt32("uniqueid",psshInfo->nUniqueID);
                  }
                }
                if(pExtraData->eType == (OMX_EXTRADATATYPE)mIndexDrmExtraSampleInfo)
                {
                  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING,
                                QTVDIAG_PRIO_HIGH, "Found ExtraSample");
                  extraSampleInfo = (QOMX_EXTRA_SAMPLE_INFO *)&pExtraData->data;
                  if (extraSampleInfo)
                  {
#if DRM_DEBUG_CONTENT_PRINT
                    PrintExtraSampleInfo(extraSampleInfo);
#endif
                    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Setting extrasampleInfo into meta()");
                    mABuffer->meta()->setInt32("encbuffersz", pBufHdr->nFilledLen);
                    mABuffer->meta()->setInt32("encsampleisenc",extraSampleInfo->nIsEncrypted);
                    mABuffer->meta()->setInt32("encsamplecount",extraSampleInfo->nSubSampleCount);
                    mABuffer->meta()->setInt32("encsampleivsz", extraSampleInfo->nIVSize);
                    mABuffer->meta()->setInt32("encsamplekidsz", MAX_KID_SIZE);
                    mABuffer->meta()->setInt32("encsampleinfosz",  MAX_ENCRYPTED_ENTRY*sizeof(QOMX_ENCRYPTED_SUBSAMPLE_INFO));
                    mABuffer->meta()->setString("encsampleiv",(const char*)extraSampleInfo->nInitVector,MAX_IV_SIZE);
                    mABuffer->meta()->setString("encsamplekid",(const char*)extraSampleInfo->nKeyID,MAX_KID_SIZE);
                    mABuffer->meta()->setString("encsampledefaultkid",(const char*)extraSampleInfo->nDefaultKeyID,MAX_KID_SIZE);
                    mABuffer->meta()->setString("encsampleinfo",(const char*)extraSampleInfo->sEncSubsampleInfo,
                                                MAX_ENCRYPTED_ENTRY*sizeof(QOMX_ENCRYPTED_SUBSAMPLE_INFO));

                  }
                }
                ulAddr = ulAddr + pExtraData->nSize;
                ulAddr = (ulAddr + 0x3) & (~0x3);
                pExtraData = (OMX_OTHER_EXTRADATATYPE*)(intptr_t)ulAddr;
              }
            }
          }
#endif
        }
        else
        {
          mABuffer->meta()->setInt32("encrypted", 0);
        }
        if (IS_AUDIO_PORT(trackInfo.m_nPortIndex)) //kbhima? picked from AAL. check with satish.
        {
            const char *mime = NULL;
            sp<MetaData> md = trackInfo.getMetaData();
            if( md !=NULL )
            {
              md->findCString(kKeyMIMEType, &mime);
              if (mime && !std_strnicmp(mime, DASH_MEDIA_MIMETYPE_AUDIO_AAC,
                std_strlen(DASH_MEDIA_MIMETYPE_AUDIO_AAC)))
              {
                if ( (pBufHdr->pBuffer[0] == 0xFF ) && ((pBufHdr->pBuffer[1] & 0xF0 ) == 0xF0))
                {
                  if (pBufHdr->nFilledLen > 7)
                  {
                    pBufHdr->nFilledLen -= 7;
                    std_memmove(pBufHdr->pBuffer, pBufHdr->pBuffer+7, pBufHdr->nFilledLen);
                  }
                }
              }
            }
          }
          else if (IS_VIDEO_PORT(trackInfo.m_nPortIndex))
          {
            mABuffer->meta()->setInt32("sync", ((pBufHdr->nFlags & OMX_BUFFERFLAG_SYNCFRAME)!=0) );

            if((pBufHdr->nFlags & OMX_BUFFERFLAG_SYNCFRAME)!=0)
            {
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                "DEALInterface::ProcessFrameNotify video setInt32 sync frame flag");
            }
          }

          mABuffer->setRange(0, pBufHdr->nFilledLen);
          mABuffer->meta()->setInt64("timeUs",pBufHdr->nTimeStamp);

          //kbhima? is this Read sample for media 2 - status 8 buffer or next buffer? probably next buffer.
          mABuffer->meta()->setInt32("conf", ((pBufHdr->nFlags & OMX_BUFFERFLAG_CODECCONFIG)!=0));

        if (trackInfo.IsEncrypted() && !psshInfo)
        {
          //Notify Error
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                        "pssh not present for encrypted stream !!");
          trackInfo.SetEncrypted(UNKNOWN_ERROR);
          SignalEOSonError(ERROR_END_OF_STREAM);
        }

        if((pBufHdr->nFlags & OMX_BUFFERFLAG_CODECCONFIG)!=0)
        {
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                            "DEALInterface::ProcessFrameNotify video setInt32 codec config flag");
          if (trackInfo.IsEncrypted() && psshInfo)
          {
            mABuffer->meta()->setInt32("encbuffersz", pBufHdr->nFilledLen);
            mABuffer->meta()->setString("encsampledefaultkid",(const char*)psshInfo->cDefaultKeyID,MAX_KID_SIZE);
            mABuffer->meta()->setInt32("encsamplekidsz", MAX_KID_SIZE);
          }
        }
        else
        {
          mABuffer->meta()->setInt32("conf", 0);
          if (trackInfo.IsEncrypted() && !extraSampleInfo)
          {
            //Notify Error
            QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                        "Error::ExtraSampleInfo not present"
                        "in encrypted steam (Not A CODECCONFIG Frame)!!");
            trackInfo.SetEncrypted(UNKNOWN_ERROR);
            SignalEOSonError(ERROR_END_OF_STREAM);
          }
        }

  if((IS_TEXT_PORT(trackInfo.m_nPortIndex)) && (pBufHdr->nFlags & OMX_BUFFERFLAG_EXTRADATA))
  {
     QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "ProcessFrameNotify TextPort Extradata");

     OMX_OTHER_EXTRADATATYPE *pExtraDataSampleDimensions = NULL;
     OMX_OTHER_EXTRADATATYPE *pExtraData = NULL;

     uint32 ulAddr = (uint32)(intptr_t)( pBufHdr->pBuffer) +  pBufHdr->nFilledLen;
     // Aligned address to DWORD boundary
     ulAddr = (ulAddr + 0x3) & (~0x3);
     pExtraData = (OMX_OTHER_EXTRADATATYPE *)(intptr_t)ulAddr;

            while ( pExtraData && pExtraData->eType != OMX_ExtraDataNone)
            {
              QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                "ProcessFrameNotify pExtraData->eType(0x%x)",(OMX_EXTRADATATYPE)pExtraData->eType);

              if((OMX_INDEXTYPE)pExtraData->eType == mSMTPTimedTextDimensionsIndex)
              {
                pExtraDataSampleDimensions = pExtraData;
                QOMX_SUBTITILE_DIMENSIONS *sampleDimensions = (QOMX_SUBTITILE_DIMENSIONS *)pExtraDataSampleDimensions->data;
                mABuffer->meta()->setInt32("height",sampleDimensions->nHeight);
                mABuffer->meta()->setInt32("width",sampleDimensions->nWidth);
                mABuffer->meta()->setInt32("duration",sampleDimensions->nDuration);
                mABuffer->meta()->setInt32("startoffset",(int32)sampleDimensions->nStartOffset);
                ulAddr = ulAddr + pExtraDataSampleDimensions->nSize;
                ulAddr = (ulAddr + 0x3) & (~0x3);
                pExtraData = (OMX_OTHER_EXTRADATATYPE*)(intptr_t)ulAddr;
              }

              if ((OMX_INDEXTYPE)pExtraData->eType == mSMTPTimedTextSubInfoIndex)
              {
                OMX_OTHER_EXTRADATATYPE *pExtraDataSubs = pExtraData;
                QOMX_SUBTITLE_SUB_INFOTYPE *subInfo = (QOMX_SUBTITLE_SUB_INFOTYPE *)pExtraDataSubs->data;
                mABuffer->meta()->setInt32("subSc",subInfo->subSampleCount);
                mABuffer->meta()->setInt32("subSt",subInfo->eSubCodingType);
                mABuffer->meta()->setInt32("subSz",subInfo->nSubtitleSubInfoSize);
                //mABuffer->meta()->setString("subSi",(const char*)subInfo->cSubtitleSubInfo,subInfo->nSubtitleSubInfoSize);
                ulAddr = ulAddr + pExtraDataSubs->nSize;
                ulAddr = (ulAddr + 0x3) & (~0x3);
                pExtraData = (OMX_OTHER_EXTRADATATYPE*)(intptr_t)ulAddr;
              }
            } //while
          }

          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALInterface::ProcessFrameNotify queueaccessunit on outbufferQ");

          // queue the access unit to video Dash Packet source
          trackInfo.getOutBufferQ()->queueAccessUnit(mABuffer);
        }
      }
    }
    else
    {
      bOk = false;
    }

    return bOk;
  }

void
DEALInterface::SignalEOSonError(status_t result)
{
  for(int i=0; i < MAX_NUM_TRACKS; i++)
  {
    TrackInfo& trackInfo = mTrackList[i];
    if(trackInfo.getMetaData() !=NULL) //valid track
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
      "Posting ERROR_END_OF_STREAM for port %d", trackInfo.m_nPortIndex);
      trackInfo.getOutBufferQ()->signalEOS(result);
    }
  }

}
#if DRM_DEBUG_CONTENT_PRINT
void
DEALInterface::printBuf(const char */*buffer*/, int /*bufLength*/)
{
/*
  for (int i = 0; i< bufLength; i++)
  {
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "[%d]->[%x]", i, buffer[i]);
  }
*/
}

void
DEALInterface::PrintPSSH
(
  QOMX_PARAM_STREAMING_PSSHINFO */*psshInfo*/
)
{
/*
  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "----DefaultKID:Port[%d]:UniqueId[%d]----",
      (int)psshInfo->nPortIndex,(int)psshInfo->nUniqueID);
  printBuf((const char*)psshInfo->cDefaultKeyID,MAX_KID_SIZE);

  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "----PSSH:Size[%d]----",(int)psshInfo->nPsshDataBufSize);
  printBuf((const char*)psshInfo->cPSSHData,(int)psshInfo->nPsshDataBufSize);
*/
}

void
DEALInterface::PrintExtraSampleInfo
(
  QOMX_EXTRA_SAMPLE_INFO *extraSampleInfo
)
{
  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "----ExtraSampleInfo:SubsampleCount [%d]----",
               extraSampleInfo->nSubSampleCount);
  //QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
    //  "----Init Vector----");
  //printBuf((const char*)extraSampleInfo->nInitVector,MAX_IV_SIZE);
  PrintSubSampleInfo(extraSampleInfo->nSubSampleCount,
                     extraSampleInfo->sEncSubsampleInfo);
}

void
DEALInterface::PrintSubSampleInfo
(
  int subSampleCount,
  QOMX_ENCRYPTED_SUBSAMPLE_INFO *encSubsampleInfo
)
{
  if (subSampleCount > 0)
  {
    //QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      //              "----Encrypted Sample Info----") ;
    for (int i = 0; encSubsampleInfo && i < subSampleCount ; i++)
    {
      QTV_MSG_PRIO5(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
         "[%d]SizeOfClearData [%d] OffsetClearData [%lu] SizeOfEncryptedData"
         "[%lu] OffsetEncryptedData[%lu]", i,
         encSubsampleInfo[i].nSizeOfClearData,
         encSubsampleInfo[i].nOffsetClearData,
         encSubsampleInfo[i].nSizeOfEncryptedData,
         encSubsampleInfo[i].nOffsetEncryptedData);
     }
  }
}

#endif

void
DEALInterface::GetIndexForDRMExtensions
(
)
{

  OMX_HANDLETYPE handle = GetMMIHandle();
  MMI_GetExtensionCmdType ext;

  ext.cParamName = (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_PSSH_INFO;
  ext.pIndex = &mIndexDrmParamPsshInfo;
  uint32 mmiret = MMI_S_EFAIL;
  mmiret = HTTPMMIDeviceCommand(handle, MMI_CMD_GET_EXTENSION_INDEX, &ext);
  if(mmiret != MMI_S_COMPLETE)
  {
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
            "Error: Get Extension Index for '%s' is %lu", ext.cParamName,
            (long unsigned int)ext.pIndex);
  }
  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "Success: Get Extension Index for '%s' is  0x%x", ext.cParamName,
               mIndexDrmParamPsshInfo);


  ext.cParamName = (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_EXTRASAMPLE_INFO;
  ext.pIndex = &mIndexDrmExtraSampleInfo;
  mmiret = HTTPMMIDeviceCommand(handle, MMI_CMD_GET_EXTENSION_INDEX, &ext);
  if(mmiret != MMI_S_COMPLETE)
  {
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
             "Error: Get Extension Index for '%s' is %lu",
             ext.cParamName, (long unsigned int)ext.pIndex);
  }
  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "Success: Get Extension Index for '%s' is  0x%x",
              ext.cParamName, mIndexDrmExtraSampleInfo);
  return;
}


//status_t DEALInterface::DoDecyrpt(sp<ABuffer> buffer, int Fd, uint32 portIndex, int len)
status_t DEALInterface::DoDecyrpt(sp<ABuffer> /*buffer*/, int /*Fd*/, uint32 /*portIndex*/)
{
  OMX_U8 *pBufferEncData =NULL;
  status_t drmErr = DRM_ERROR_UNKNOWN;
#if 0
  if (buffer != NULL)
  {
    pBufferEncData = (OMX_U8 *) buffer->data();
    int32_t uniqueId;
    int32_t encBufferSz;
    int32_t encSampleIsEnc;
    int32_t encSampleCount;
    int32_t encSampleIvSz;
    int32_t encSampleKidSz;
    int32_t encSampleInfoSz;
    AString psshInfo;
    AString defaultKid;
    AString sampleIV;
    AString sampleKID;
    AString sampleDefaultKID;
    AString encSampleInfo;
    QOMX_ENCRYPTED_SUBSAMPLE_INFO *sEncSubsampleInfo = NULL;
    buffer->meta()->findInt32("uniqueid", &uniqueId);
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                    "uniqueId '%d' ", uniqueId) ;
    if(mECALInterface != NULL)
    {
      drmErr = mECALInterface->HasDrmMetaInfoChanged(portIndex, uniqueId);
      if (drmErr == DRM_NO_ERROR)
      {
        int32_t conf = 0;
        buffer->meta()->findInt32("conf", &conf);
        if (conf == 1)
        {
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "CODECCONFIG frame");
          encSampleCount = 1;
          buffer->meta()->findInt32("encbuffersz", &encBufferSz);
          encSampleIvSz = 8;
          char *iv = (char *)MM_Malloc(encSampleIvSz);
          memset (iv, 0x00,encSampleIvSz );
          sampleIV.setTo(iv, encSampleIvSz);
          buffer->meta()->findString("encsampledefaultkid",&sampleKID); //reading default KID in sample KID
          sEncSubsampleInfo = (QOMX_ENCRYPTED_SUBSAMPLE_INFO *)
                               MM_Malloc(sizeof(QOMX_ENCRYPTED_SUBSAMPLE_INFO));
          if (sEncSubsampleInfo)
          {
            sEncSubsampleInfo->nOffsetClearData  = 0 ;
            sEncSubsampleInfo->nOffsetEncryptedData = encBufferSz;
            sEncSubsampleInfo->nSizeOfClearData = encBufferSz;
            sEncSubsampleInfo->nSizeOfEncryptedData = 0;
          }
          QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "encBufferSz '%d', encSampleCount '%d',encSampleIvSz '%d'",
               encBufferSz, encSampleCount , encSampleIvSz);
        }
        else
        {
          buffer->meta()->findInt32("encbuffersz", &encBufferSz);
          buffer->meta()->findInt32("encsampleisenc", &encSampleIsEnc);
          buffer->meta()->findInt32("encsamplecount", &encSampleCount);
          buffer->meta()->findInt32("encsampleivsz", &encSampleIvSz);
          buffer->meta()->findInt32("encsamplekidsz", &encSampleKidSz);
          buffer->meta()->findInt32("encsampleinfosz", &encSampleInfoSz);
          buffer->meta()->findString("encsampleiv",&sampleIV);
          buffer->meta()->findString("encsamplekid",&sampleKID);
          buffer->meta()->findString("encsampledefaultkid",&sampleDefaultKID);
          buffer->meta()->findString("encsampleinfo",&encSampleInfo);

          sEncSubsampleInfo =
                  (QOMX_ENCRYPTED_SUBSAMPLE_INFO *)encSampleInfo.c_str();

          QTV_MSG_PRIO6(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "encBufferSz '%d'encSampleIsEnc '%d', encSampleCount '%d',"
               "encSampleIvSz '%d'encSampleKidSz '%d', encSampleInfoSz '%d'",
                encBufferSz, encSampleIsEnc , encSampleCount , encSampleIvSz,
                encSampleKidSz, encSampleInfoSz);

        }

        if (sEncSubsampleInfo)
        {
#if DRM_DEBUG_CONTENT_PRINT
          //QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            //            "----EncryptedSample IV----") ;
          printBuf((const char*)sampleIV.c_str(), encSampleIvSz);
          //QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
           //             "----EncryptedSample KID----") ;
          printBuf((const char*)sampleDefaultKID.c_str(), encSampleKidSz);
          PrintSubSampleInfo(encSampleCount, sEncSubsampleInfo);
#endif

          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Calling mECALInterface->DoDecyrpt");
          drmErr = mECALInterface->DoDecyrpt(portIndex,
                                              encBufferSz,
                                              encSampleCount,
                                              encSampleIvSz,
                                              (const char*)sampleIV.c_str(),
                                              (const char*)sampleKID.c_str(),
                                              sEncSubsampleInfo,
                                              (const char *)pBufferEncData,
                                              Fd);
        }
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                   "DEALInterface::DoDecyrpt Input Buffer is NULL");
  }
#endif
  return drmErr;
}



/**
 * C'tor of DEALStateSeeking
 * @param[in]
 *
 * @return
 */
DEALInterface::DEALStateSeeking::DEALStateSeeking(DEALInterface &pDataCache):
                                 DEALStateBase(pDataCache, DEALInterfaceStateSeeking)
{
}

/**
 * D'tor of DEALStateSeeking
 * @param[in]
 *
 * @return
 */
DEALInterface::DEALStateSeeking::~DEALStateSeeking()
{
}

/*
 * EntryHandler, called when enters state
 *
 * @param[in] none
 *
 * @return true when successful, false otherwise
 */
bool DEALInterface::DEALStateSeeking::EntryHandler()
{
  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "DEALInterface::DEALStateSeeking::EntryHandler");
  if(!m_pMMHTTPDataCache.isFlushPending())
  {
    m_pMMHTTPDataCache.flush();
  }

  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "DEALInterface::DEALStateSeeking::EntryHandler isFlushPending %d", m_pMMHTTPDataCache.isFlushPending());

  return true;
}

  /*
  * process commands given by framework in state Pausing
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStateSeeking::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 arg1, int32 arg2, int64 arg3)
  {
    uint32 ret = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();
    bool allset = true;

    switch(cmd)
    {
    case DEALInterface::DEAL_PAUSE:
      {
        bool bOk = m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStatePausing);
        if(bOk)
        {
          ret = MMI_S_COMPLETE;
        }
      }
      break;

    case DEALInterface::DEAL_RESUME:
      // Invalid cmd in this state
      break;

    case DEALInterface::DEAL_SEEK:
      {
        //Cache current seek and send notificaion success to previous
        m_pMMHTTPDataCache.storeSeekValue(arg3);

        //seek cmd is posted after flush response,
        //latest seek cmd is cached if seek happens with flushpending flag true,
        if(m_pMMHTTPDataCache.isFlushPending())
        {
          ret = MMI_S_PENDING;
        }
        else
        {
          int64 seekVal = m_pMMHTTPDataCache.getSeekValue();

          //Post seek directly if flush cmd response is already arrived
          ret = m_pMMHTTPDataCache.postSeek(seekVal);
          m_pMMHTTPDataCache.resetSeek();
        }
      }
      break;

    case DEALInterface::DEAL_STOP:
      ret = DEALStateBase::ProcessCmd(cmd, arg1, arg2, arg3);
      break;

    default:
      break;
    }

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALStatePlaying::ProcessCmd - seek/pause processed - result = %lu", ret);

    return ret;

  }

  status_t DEALInterface::postSeek(int64 timeUs)
  {
    uint32 ret = MMI_S_EFAIL;
    MMI_OmxParamCmdType data;
    OMX_TIME_CONFIG_TIMESTAMPTYPE paramTSStruct;

    QOMX_STRUCT_INIT(paramTSStruct, OMX_TIME_CONFIG_TIMESTAMPTYPE);
    paramTSStruct.nTimestamp = timeUs;
    paramTSStruct.nPortIndex = OMX_ALL;
    data.nParamIndex = OMX_IndexConfigTimePosition;
    data.pParamStruct = &paramTSStruct;

    ret = HTTPMMIDeviceCommand(m_handle, MMI_CMD_SET_STD_OMX_PARAM, &data);
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALInterface::seek - sent to stack - result = %lu", ret);

    return ret;
  }

  /*
  * handles asynchronous events from MMI in DEAL state Seeking
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStateSeeking::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    OMX_U32 nReturn = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();

    switch(nEvtCode)
    {
    case MMI_RESP_FLUSH:
      {
        m_pMMHTTPDataCache.incFlushCount();
        if(m_pMMHTTPDataCache.isFlushPending() && (MAX_NUM_TRACKS+1 == m_pMMHTTPDataCache.getFlushCount()))
        {
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStateSeeking flush response status %d", DEALUtils::MapMMIToDEALStatus(nEvtStatus));

          // Reset the variables
          m_pMMHTTPDataCache.setFlushPending(false);
          m_pMMHTTPDataCache.resetFlushCount();

          int64 seekVal = m_pMMHTTPDataCache.getSeekValue();

          if(seekVal != (int64)-1)
          {
            nReturn = m_pMMHTTPDataCache.postSeek(seekVal);
          }

          m_pMMHTTPDataCache.resetSeek();

          m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStatePlaying);

          //Post FTB calls on all active ports
          for(int i=0; i < MAX_NUM_TRACKS; i++)
          {
            TrackInfo& trackInfo = m_pMMHTTPDataCache.mTrackList[i];
            if(trackInfo.getMetaData() !=NULL) //valid track
            {
              for(uint32 j=0; j < trackInfo.getMaxBufferCount(); j++)
              {
                QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                  "DEALStateSeeking calling readframeasync");

                bool bReturn = m_pMMHTTPDataCache.ReadFrameAsync(trackInfo.m_nPortIndex);
                if(!bReturn)
                {
                  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                    "DEALStateOpening readframeasync failed");
                  m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
                }
              }
            }
          }
        }

        break;
      }

    case MMI_RESP_FILL_THIS_BUFFER:
      {
        MMI_BufferCmdType *pFillBuffCmdResp = NULL;
        pFillBuffCmdResp = (MMI_BufferCmdType *) pEvtData;
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALStateSeeking::EventHandler EventCode : 0x%x port index 0x%x",(int)nEvtCode,(int)pFillBuffCmdResp->nPortIndex);

        DASHSourceTrackType nTrackId = DEALUtils::MapPortIndexToTrackId(pFillBuffCmdResp->nPortIndex);

        if(nTrackId != DEALInterface::eTrackNone)
        {
          TrackInfo &trackInfo = m_pMMHTTPDataCache.mTrackList[nTrackId];

          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStateSeeking::EventHandler resp_fill_this_buffer valid track");

          sp<DEALInterface::InBufferQItem> buffer;

          MM_CriticalSection_Enter(m_pMMHTTPDataCache.m_hDEALTrackInfoReadLock[trackInfo.m_nTrackId]);
          //dequeue from InBufferQ and queue in OutBufferQ

          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStateSeeking::EventHandler dequeue inputbuffer Q");

          trackInfo.getInBufferQ()->dequeue(&buffer);
          MM_CriticalSection_Leave(m_pMMHTTPDataCache.m_hDEALTrackInfoReadLock[trackInfo.m_nTrackId]);
          if (buffer != NULL && buffer->mBuffer != NULL)
          {
            QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "DEALStateSeeking::EventHandler calling ProcessFrameNotify");

            //bool bReturn = m_pMMHTTPDataCache.ProcessFrameNotify(buffer, trackInfo.m_nPortIndex, nEvtStatus);
            bool bReturn = m_pMMHTTPDataCache.ProcessFrameNotify(buffer->mBuffer, pFillBuffCmdResp, trackInfo.m_nPortIndex, nEvtStatus);
            buffer->mBuffer = NULL;
            buffer = NULL;

            if(!bReturn)
            {
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                           "DEALStateSeeking::EventHandler ProcessFrameNotify failed");
              m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
            }
          }
        }
        else
        {
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                        "DEALStateSeeking::EventHandler Invalid track type %d", nTrackId);
          m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
        }
        break;
      }

    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
    }
  }

  /**
  * C'tor of DEALStatePausing
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStatePausing::DEALStatePausing(DEALInterface &pDataCache):
  DEALStateBase(pDataCache, DEALInterfaceStatePausing)
  {
  }

  /**
  * D'tor of DEALStatePausing
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStatePausing::~DEALStatePausing()
  {
  }

  /*
  * EntryHandler, called when enters state
  *
  * @param[in] none
  *
  * @return true when successful, false otherwise
  */
  bool DEALInterface::DEALStatePausing::EntryHandler()
  {
    uint32 ret = MMI_S_EFAIL;
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "DEALInterface::DEALStatePausing::EntryHandler");

    //Set mPaused flag on each of mediasource track objects
    for(int index=0; index<MAX_NUM_TRACKS; index++)
    {
      if(m_pMMHTTPDataCache.mTrackList[index].getOutBufferQ() != NULL)
      {
        m_pMMHTTPDataCache.mTrackList[index].getOutBufferQ()->pause();
      }
    }

    ret = HTTPMMIDeviceCommand(m_pMMHTTPDataCache.GetMMIHandle(), MMI_CMD_PAUSE, NULL);
    if(!IS_SUCCESS(ret))
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        "DEALInterface::DEALStatePausing::EntryHandler posting pause failed");
    }

    return IS_SUCCESS(ret);
  }

  /*
  * process commands given by framework in state Pausing
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStatePausing::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 arg1, int32 arg2, int64 arg3)
  {
    uint32 ret = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();
    bool allset = true;

    switch(cmd)
    {
    case DEALInterface::DEAL_PAUSE:
      //  Not allowed in this state
      break;

    case DEALInterface::DEAL_RESUME:
      // TODO
      break;

    case DEALInterface::DEAL_SEEK:
      {
        //Cache current seek and send notificaion success to previous
        m_pMMHTTPDataCache.storeSeekValue(arg3);
        ret = MMI_S_PENDING;
      }
      break;

    case DEALInterface::DEAL_STOP:
      ret = DEALStateBase::ProcessCmd(cmd, arg1, arg2, arg3);
      break;

    default:
      break;
    }

    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALStatePausing::ProcessCmd - %d - result = %lu", cmd, ret);

    return ret;

  }

  /*
  * handles asynchronous events from MMI in DEAL state Pausing
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStatePausing::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    OMX_U32 nReturn = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();

    switch(nEvtCode)
    {
    case MMI_RESP_PAUSE:
      {
        bool bOk = false;
        if (MMI_S_COMPLETE == nEvtStatus)
        {
          bOk = m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStatePaused);
        }

        if(m_pMMHTTPDataCache.notify)
        {
          m_pMMHTTPDataCache.notify(DEAL_PAUSE_RSP, bOk ? 1 : 0, m_pMMHTTPDataCache.cbData, NULL);
        }

      }
      break;

    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
      break;
    }
  }

  /**
  * C'tor of DEALStatePaused
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStatePaused::DEALStatePaused(DEALInterface &pDataCache):
  DEALStateBase(pDataCache, DEALInterfaceStatePaused)
  {
  }

  /**
  * D'tor of DEALStatePausing
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStatePaused::~DEALStatePaused()
  {
  }

  /*
  * process commands given by framework in state Pausing
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStatePaused::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 arg1, int32 arg2, int64 arg3)
  {
    uint32 ret = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();
    bool allset = true;

    switch(cmd)
    {
    case DEALInterface::DEAL_PAUSE:
      //  Not allowed in this state
      break;

    case DEALInterface::DEAL_RESUME:
      {
        bool bOk = m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateResuming);
        if(bOk)
        {
          ret = MMI_S_COMPLETE;
        }
      }
      break;

    case DEALInterface::DEAL_SEEK:
      {
        //Cache current seek and send notificaion success to previous
        m_pMMHTTPDataCache.storeSeekValue(arg3);
        ret = MMI_S_PENDING;
      }
      break;

    case DEALInterface::DEAL_STOP:
      ret = DEALStateBase::ProcessCmd(cmd, arg1, arg2, arg3);
      break;

    default:
      break;
    }

    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALStatePaused::ProcessCmd - %d - result = %lu", cmd, ret);

    return ret;
  }

  /*
  * handles asynchronous events from MMI in DEAL state Paused
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStatePaused::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    OMX_U32 nReturn = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();

    switch(nEvtCode)
    {
    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
      break;
    }
  }

  /**
  * C'tor of DEALStateResuming
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateResuming::DEALStateResuming(DEALInterface &pDataCache):
  DEALStateBase(pDataCache, DEALInterfaceStateResuming)
  {
  }

  /**
  * D'tor of DEALStateResuming
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateResuming::~DEALStateResuming()
  {
  }

  /*
  * EntryHandler, called when enters state
  *
  * @param[in] none
  *
  * @return true when successful, false otherwise
  */
  bool DEALInterface::DEALStateResuming::EntryHandler()
  {
    uint32 ret = MMI_S_EFAIL;
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "DEALInterface::DEALStateResuming::EntryHandler");

    //Reset mPaused flag on each of mediasource track objects
    for(int index=0; index<MAX_NUM_TRACKS; index++)
    {
      if(m_pMMHTTPDataCache.mTrackList[index].getOutBufferQ() != NULL)
      {
        m_pMMHTTPDataCache.mTrackList[index].getOutBufferQ()->resume();
      }
    }

    ret = HTTPMMIDeviceCommand(m_pMMHTTPDataCache.GetMMIHandle(), MMI_CMD_RESUME, NULL);
    if(!IS_SUCCESS(ret))
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        "DEALInterface::DEALStateResuming::EntryHandler posting resume failed");
    }

    return IS_SUCCESS(ret);
  }

  /*
  * process commands given by framework in state Resuming
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStateResuming::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 arg1, int32 arg2, int64 arg3)
  {
    uint32 ret = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();
    bool allset = true;

    switch(cmd)
    {
    case DEALInterface::DEAL_PAUSE:
      //  Not allowed in this state
      break;

    case DEALInterface::DEAL_RESUME:
      // Not allowed in this state
      break;

    case DEALInterface::DEAL_SEEK:
      {
        //Cache current seek and send notificaion success to previous
        m_pMMHTTPDataCache.storeSeekValue(arg3);
        ret = MMI_S_COMPLETE;
      }
      break;

    case DEALInterface::DEAL_STOP:
      ret = DEALStateBase::ProcessCmd(cmd, arg1, arg2, arg3);
      break;

    default:
      break;
    }

    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALStateResuming::ProcessCmd - %d - result = %lu", cmd, ret);

    return ret;
  }

  /*
  * handles asynchronous events from MMI in DEAL state Paused
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStateResuming::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    OMX_U32 nReturn = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();

    switch(nEvtCode)
    {
    case MMI_RESP_RESUME:
      {
        bool bOk = false;
        if (MMI_S_COMPLETE == nEvtStatus)
        {
          if(m_pMMHTTPDataCache.getSeekValue() != (int64)-1)
          {
            bOk = m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateSeeking);
          }
          else
          {
            bOk = m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStatePlaying);
          }
        }

        if(m_pMMHTTPDataCache.notify)
        {
          m_pMMHTTPDataCache.notify(DEAL_RESUME_RSP, bOk ? 1 : 0, m_pMMHTTPDataCache.cbData, NULL);
        }
      }
      break;

    case MMI_RESP_FILL_THIS_BUFFER:
      {
        MMI_BufferCmdType *pFillBuffCmdResp = NULL;
        pFillBuffCmdResp = (MMI_BufferCmdType *) pEvtData;
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DEALStatePlaying::EventHandler EventCode : 0x%x port index 0x%x",(int)nEvtCode,(int)pFillBuffCmdResp->nPortIndex);

        DASHSourceTrackType nTrackId = DEALUtils::MapPortIndexToTrackId(pFillBuffCmdResp->nPortIndex);

        if(nTrackId != DEALInterface::eTrackNone)
        {
          TrackInfo &trackInfo = m_pMMHTTPDataCache.mTrackList[nTrackId];

          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStatePlaying::EventHandler resp_fill_this_buffer valid track");

          sp<DEALInterface::InBufferQItem> buffer;

          MM_CriticalSection_Enter(m_pMMHTTPDataCache.m_hDEALTrackInfoReadLock[trackInfo.m_nTrackId]);
          //dequeue from InBufferQ and queue in OutBufferQ

          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStatePlaying::EventHandler dequeue inputbuffer Q");

          trackInfo.getInBufferQ()->dequeue(&buffer);
          MM_CriticalSection_Leave(m_pMMHTTPDataCache.m_hDEALTrackInfoReadLock[trackInfo.m_nTrackId]);
          if (buffer != NULL && buffer->mBuffer != NULL)
          {
            QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "DEALStatePlaying::EventHandler calling ProcessFrameNotify");

            //bool bReturn = m_pMMHTTPDataCache.ProcessFrameNotify(buffer, trackInfo.m_nPortIndex, nEvtStatus);
            bool bReturn = m_pMMHTTPDataCache.ProcessFrameNotify(buffer->mBuffer, pFillBuffCmdResp, trackInfo.m_nPortIndex, nEvtStatus);
            buffer->mBuffer = NULL;
            buffer = NULL;

            if(!bReturn)
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "DEALStatePlaying::EventHandler ProcessFrameNotify failed");
              m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
            }
          }
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "DEALStatePlaying::EventHandler Invalid track");
          m_pMMHTTPDataCache.notify(DEAL_UNKNOWN_ERROR, 0, m_pMMHTTPDataCache.cbData, NULL);
        }
      }

    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
      break;
    }

  }

  void DEALInterface::flush()
  {
    if(m_handle)
    {
      MMI_PortCmdType flushCmd;
      flushCmd.nPortIndex = OMX_ALL;

      //Flush access unit queues on each of mediasource track objects
      for(int index=0; index<MAX_NUM_TRACKS; index++)
      {
        if(mTrackList[index].getOutBufferQ() != NULL)
        {
          mTrackList[index].getOutBufferQ()->flush();
        }
      }

      OMX_U32 nret = MMI_S_EFAIL;

      setFlushPending(true);

      nret = postMMICmd(m_handle, MMI_CMD_FLUSH, &flushCmd);
      if (MMI_S_PENDING == nret)
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "Flush all ports Done");
      }
      else
      {
        setFlushPending(false);
      }
    }
  }

  /**
  * C'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateClosing::DEALStateClosing(DEALInterface &pDataCache):
  DEALStateBase(pDataCache, DEALInterfaceStateClosing)
  {
  }

  /**
  * D'tor of DEALStateIdle
  * @param[in]
  *
  * @return
  */
  DEALInterface::DEALStateClosing::~DEALStateClosing()
  {
  }

  /*
  * EntryHandler, called when enters state
  *
  * @param[in] none
  *
  * @return true when successful, false otherwise
  */
  bool DEALInterface::DEALStateClosing::EntryHandler()
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "DEALInterface::DEALStateClosing::EntryHandler");

    if(!m_pMMHTTPDataCache.isFlushPending())
    {
      m_pMMHTTPDataCache.flush();
    }

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALInterface::DEALStateClosing::EntryHandler isFlushPending %d", m_pMMHTTPDataCache.isFlushPending());

    return true;
  }

  /*
  * process commands given by framework in state Playing
  *
  * @param[in] cmd command to be processed
  * @param[in] arg1 valid only if requires args (now seek uses this)
  * @param[in] arg2 valid only if requires args (now seek uses this)
  *
  * @return status of the command processing
  */
  uint32 DEALInterface::DEALStateClosing::ProcessCmd(DEALInterface::DEALCmd cmd,
    int32 arg1, int32 arg2, int64 arg3)
  {
    uint32 ret = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();
    bool allset = true;

    switch(cmd)
    {
    case DEALInterface::DEAL_STOP:
      ret = MMI_S_PENDING;
      break;

    default:
      ret = DEALStateBase::ProcessCmd(cmd, arg1, arg2, arg3);
      break;
    }

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DEALStateStoping::ProcessCmd - STOP - result = %lu", ret);

    return ret;
  }

  /*
  * handles asynchronous events from MMI in DEAL state IDLE
  *
  * @param[in] nEvtCode event code
  * @param[in] nEvtStatus event status
  * @param[in] nPayloadLen length of pEvtData if not null
  * @param[in] pEvtData event payload
  *
  * @return
  */
  void DEALInterface::DEALStateClosing::EventHandler(OMX_IN OMX_U32 nEvtCode,
    OMX_IN OMX_U32 nEvtStatus,
    OMX_IN OMX_U32 nPayloadLen,
    OMX_IN OMX_PTR pEvtData)
  {
    OMX_U32 nReturn = MMI_S_EFAIL;
    OMX_HANDLETYPE handle = m_pMMHTTPDataCache.GetMMIHandle();

    switch(nEvtCode)
    {
    case MMI_RESP_FLUSH:
      {
        // Ignore flush in this state
        m_pMMHTTPDataCache.incFlushCount();
        if(MAX_NUM_TRACKS+1 == m_pMMHTTPDataCache.getFlushCount())
        {
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStateBase::EventHandler flush response status %d", DEALUtils::MapMMIToDEALStatus(nEvtStatus));

          // Reset the variables
          m_pMMHTTPDataCache.setFlushPending(false);
          m_pMMHTTPDataCache.resetFlushCount();

          // Issue stop cmd
          nReturn = HTTPMMIDeviceCommand(handle, MMI_CMD_STOP, NULL);
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "DEALStateBase::EventHandler stop cmd posted status %d", DEALUtils::MapMMIToDEALStatus(nReturn));

        }
      }

      break;

    case MMI_RESP_STOP:
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        "CHTTPAALStateBase::EventHandler, "
        "recved stop rsp, closing device");
      if(m_pMMHTTPDataCache.notify)
      {
        m_pMMHTTPDataCache.notify(DEAL_STOP_RSP, IS_SUCCESS(nEvtStatus) ? 1 : 0, m_pMMHTTPDataCache.cbData, NULL);
      }

      m_pMMHTTPDataCache.SetDEALState(DEALInterfaceStateInit);
      break;

    default:
      DEALStateBase::EventHandler(nEvtCode, nEvtStatus, nPayloadLen, pEvtData);
      break;
    }
  }

  void DEALInterface::ResetDiscontinuity(uint32 nTrackId, bool resetTo)
  {
    if (nTrackId == DEALInterface::eTrackAudio)
    {
      m_bAudioDiscontinuity = resetTo;
    }
    else if (nTrackId == DEALInterface::eTrackVideo)
    {
      m_bVideoDiscontinuity = resetTo;
    }
    else if (nTrackId == DEALInterface::eTrackText)
    {
      m_bTextDiscontinuity = resetTo;
    }
  }


  bool DEALInterface::isTrackDiscontinuity(uint32 nTrackId)
  {
    bool bOk = false;
    if (nTrackId == DEALInterface::eTrackAudio)
    {
      bOk = m_bAudioDiscontinuity;
    }
    else if (nTrackId == DEALInterface::eTrackVideo)
    {
      bOk = m_bVideoDiscontinuity;
    }
    else if (nTrackId == DEALInterface::eTrackText)
    {
      bOk = m_bTextDiscontinuity;
    }

    return bOk;
  }

}/* namespace android */
