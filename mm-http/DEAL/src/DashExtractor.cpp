/************************************************************************* */
/**
 * DashExtractor.cpp
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */

/* =======================================================================
**               Include files for DashExtractor.cpp
** ======================================================================= */

#include "DashExtractor.h"
#include "binder/Parcel.h"


namespace android {

  extern "C" sp<ImplMediaExtractor> CreateDashExtractor()
  {

    return new DashExtractor;
  }


  DashExtractor::DashExtractor()
    :mListener(NULL)
  {
    bool bOk;

    mDEAL = new DEALInterface(bOk);


    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DashExtractor::DashExtractor");

    if(!bOk)
    {
      EventHandler(DEAL_UNKNOWN_ERROR, 0, NULL, NULL);
    }
  }


  DashExtractor::~DashExtractor()
  {
    mListener = NULL;
    mDEAL = NULL;
  }


  size_t DashExtractor::countTracks()
  {
    return mDEAL->countTracks();
  }


  sp<MediaSource> DashExtractor::getTrack(size_t index)
  {
    return mDEAL->getTrack(index);
  }


  sp<MetaData> DashExtractor::getTrackMetaData(
    size_t index, uint32_t flags)
  {
    return mDEAL->getTrackMetaData(index, flags);
  }


  bool DashExtractor::sendCommand(int nCmdId)
  {
    bool ret = true;

    switch(nCmdId)
    {
    case UE_CMD_START:
      ret = mDEAL->start();
      break;

    case UE_CMD_PAUSE:
      ret = mDEAL->pause();
      break;

    case UE_CMD_RESUME:
      ret = mDEAL->resume();
      break;

    case UE_CMD_STOP:
      ret = mDEAL->stop();
      break;

    case UE_CMD_PREPARE_ASYNC:
      ret = mDEAL->prepareAsync();
      break;

    default:
      break;
    }

    return ret;
  }


  bool DashExtractor::setParameter(int nParamId, void* data)
  {
    bool ret = true;
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                  "setParameter param %d, data 0x%x", nParamId, data);

    switch(nParamId)
    {
    case UE_PARAM_SEEK:
      {
        UESeekInfo* seekInfo = (UESeekInfo*) data;
        int64_t seekTimeUs = seekInfo->mTSeekTime * 1000;

        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "DashExtractor::setParameter seekTimeUs %lld", seekTimeUs);
        ret = mDEAL->seekTo(seekTimeUs);

        break;
      }

    case UE_PARAM_CALLBACK:
      {
        mListener = ((UEListenerInfo*)data)->mListener;
        mDEAL->RegisterCallBack(EventHandler, this);
        break;
      }

    case UE_PARAM_URI:
      {
        URIInfo* uri = (URIInfo*) data;
        QTV_MSG_SPRINTF_PRIO_2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                               "uri->path %s, uri->mPathLength %d",
                               uri->path, uri->mPathLength);
        ret = mDEAL->setDataSource(uri->path, uri->mPathLength, uri->headers);
        break;
      }

    case UE_PARAM_DASH_MPD_ATTRIBUTES:
      {
        size_t len = 0;
        Parcel *parcel = ((UEDashMPDAttributesInfo*)data)->mDashMpdAttributes;

        if(parcel)
        {
          const char16_t* str = parcel->readString16Inplace(&len);
          void * data = malloc(len + 1);
          if (data == NULL)
          {
            QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::setParameter Out of memory\n");
            ret = false;
          }

          if(ret)
          {
            utf16_to_utf8(str, len, (char*) data);
            ret = mDEAL->setParameter(KEY_DASH_ADAPTION_PROPERTIES, data, len);
            free(data);
          }
        }
        else
        {
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::setParameter UE_PARAM_DASH_MPD_ATTRIBUTES null parcel object\n");
          ret = false;
        }
        break;
      }
    case UE_PARAM_PREROLL:
      {
        int64 val = 0;
        val = *((int64 *)data);
        val = val * 1000;
        ret = mDEAL->setParameter(KEY_DASH_PARAM_PREROLL, &val, sizeof(int64));
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::setParameter UE_PARAM_PREROLL %llu, status %d\n", val, ret);
        break;
      }
    case UE_PARAM_BUFFERING_HWM:
      {
        int64 val = 0;
        val = *((int64 *)data);
        val = val * 1000;
        ret = mDEAL->setParameter(KEY_DASH_PARAM_HWM, &val, sizeof(int64));
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::setParameter UE_PARAM_BUFFERING_HWM %llu, status %d\n", val, ret);
        break;
      }
    case UE_PARAM_BUFFERING_LWM:
      {
        int64 val = 0;
        val = *((int64 *)data);
        val = val * 1000;

        ret = mDEAL->setParameter(KEY_DASH_PARAM_LWM, &val, sizeof(int64));
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::setParameter UE_PARAM_BUFFERING_LWM %llu, status %d\n", val, ret);
        break;
      }
      //case UE_PARAM_QOE_EVENT:
      //todo

    default:
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::setParameter unsupported key");
      ret = false; //Unsupported setParameter key. Return false.
    }

    return ret;
  }


  bool DashExtractor::getParameter(int nParamId, void* data)
  {
    bool ret = true;

    void* data_8 = NULL;
    size_t data_8_Size = 0;
    void* data_16 = NULL;
    size_t data_16_Size = 0;

    Parcel *reply = NULL;

    int key = -1;

    switch(nParamId)
    {
    case UE_PARAM_DASH_MPD:
      key = KEY_DASH_MPD_QUERY;
      reply = ((UEDashMPDInfo*)data)->mDashMPD;
      break;

    case UE_PARAM_DASH_MPD_ATTRIBUTES:
      key = KEY_DASH_ADAPTION_PROPERTIES;
      reply = ((UEDashMPDAttributesInfo*)data)->mDashMpdAttributes;
      break;
    case UE_PARAM_PREROLL:
      {
        int64 value = 0;
        ret = mDEAL->getParameter(KEY_DASH_PARAM_PREROLL, &value, 0);
        if(!ret)
        {
          value = -1;
        }
        else
        {
          value = value / 1000; // in mili sec
        }

        ((Parcel *)data)->setDataPosition(0);
        ((Parcel *)data)->writeInt64(value); // value already in mili sec
        ((Parcel *)data)->setDataPosition(0);


        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::getParameter UE_PARAM_PREROLL %lld, status %d\n", value, ret);
      }
      break;

    case UE_PARAM_BUFFERING_HWM:
      {
        int64 value = 0;
        ret = mDEAL->getParameter(KEY_DASH_PARAM_HWM, &value, 0);
        if(!ret)
        {
          value = -1;
        }
        else
        {
          value = value / 1000; // in mili sec
        }

        ((Parcel *)data)->setDataPosition(0);
        ((Parcel *)data)->writeInt64(value); // value already in mili sec
        ((Parcel *)data)->setDataPosition(0);

        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::getParameter UE_PARAM_BUFFERING_HWM %lld, status %d\n", value, ret);
        break;
      }
    case UE_PARAM_BUFFERING_LWM:
      {
        int64 value = 0;
        ret = mDEAL->getParameter(KEY_DASH_PARAM_LWM, &value, 0);
        if(!ret)
        {
          value = -1;
        }
        else
        {
          value = value / 1000; // in mili sec
        }

        ((Parcel *)data)->setDataPosition(0);
        ((Parcel *)data)->writeInt64(value); // value already in mili sec
        ((Parcel *)data)->setDataPosition(0);

        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::getParameter UE_PARAM_BUFFERING_LWM %lld, status %d\n", value, ret);
        break;
      }

    case UE_BUFFERED_DURATION:
      {
        int64 value = 0;
        ret = mDEAL->getParameter(KEY_DASH_PARAM_BUF_DURATION, &value, 0);
        if(!ret)
        {
          value = -1;
        }

        ((Parcel *)data)->setDataPosition(0);
        ((Parcel *)data)->writeInt64(value); // value already in mili sec
        ((Parcel *)data)->setDataPosition(0);
        int64_t val = ((Parcel *)data)->readInt64();
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "checkpost:In DashExtractor val %lld status %d\n",val,ret);
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::getParameter KEY_DASH_PARAM_BUF_DURATION %lld, status %d\n", value, ret);
        break;
      }

      //case UE_PARAM_QOE_PERIODIC_EVENT:
      //todo

    default:
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR, "DashExtractor::getParameter unsupported key");
      ret = false; //Unsupported getParameter key. Return false.
    }

    if(ret && ((UE_PARAM_DASH_MPD_ATTRIBUTES == nParamId) || (UE_PARAM_DASH_MPD == nParamId)))
    {
      ret = mDEAL->getParameter(key, data_8, &data_8_Size);

      if(ret)
      {
        data_8 = malloc(data_8_Size);

        if (data_8 == NULL)
        {
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH, "Out of memory in getParameter\n");
          ret = false;
        }
        else
        {
          ret = mDEAL->getParameter(key, data_8, &data_8_Size);
        }
      }

      if(ret && data_8 != NULL)
      {
        data_16_Size = data_8_Size * sizeof(char16_t);

        data_16 = malloc(data_16_Size);
        if (data_16 == NULL)
        {
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH, "Out of memory in getParameter\n");
          ret = false;
        }
        else
        {
          utf8_to_utf16_no_null_terminator((uint8_t *)data_8, data_8_Size, (char16_t *) data_16);

          if(reply == NULL)
          {
            ret = false;
          }
          else
          {
          status_t err = reply->writeString16((char16_t *)data_16, data_8_Size);
          if(err != OK)
          {
            ret = false;
          }
          }

          free(data_16);
        }

        free(data_8);
      }
    }

    return ret;
  }


  void DashExtractor::EventHandler(int evntCode, int evntData, void *cbData, const Parcel */*obj*/)
  {
    int msg;

    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
      "DashExtractor::EventHandler evntCode %d evntData %d", evntCode, evntData);
    Parcel parcel;
    parcel.setDataPosition(0);
    parcel.writeInt32(static_cast<int32_t> (evntData));

    switch(evntCode)
    {
    case DEAL_PREPARE_RSP:
      msg = UE_PREPARE_RESP;
      break;

    case DEAL_START_RSP:
      msg = UE_START_RESP;
      break;

    case DEAL_SEEK_RSP:
      msg = UE_SEEK_RESP;
      break;

    case DEAL_PAUSE_RSP:
      msg = UE_PAUSE_RESP;
      break;

    case DEAL_RESUME_RSP:
      msg = UE_RESUME_RESP;
      break;

    case DEAL_STOP_RSP:
      msg = UE_STOP_RESP;
      break;

    case DEAL_FORMAT_CHANGE:
      msg = UE_FORMAT_CHANGE;
      break;

    case DEAL_BUFFERING_START:
      msg = UE_BUFFERING_START;
      break;

    case DEAL_BUFFERING_END:
      msg = UE_BUFFERING_END;
      break;

      //todo qoe_event_keys

    case DEAL_UNKNOWN_ERROR:
    default:
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
        "DashExtractor::EventHandler Unsupported key %d", evntCode);
      msg = UE_UNKNOWN_ERROR;
    }

    if(cbData !=NULL && ((DashExtractor*)cbData)->mListener !=NULL)
    {
      QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        "DashExtractor::EventHandler notify callback resp_code: %d resp_status %d", msg, parcel.readInt32());

      ((DashExtractor*)cbData)->mListener->notify(msg, 1, 1, &parcel);
    }

  }

 void DashExtractor::SetSelectedTrackID(int /*id*/)
 {
   return;
 }

 int32_t  DashExtractor::getTrackId(int /*trackIndex*/)
 {
   return 0;
 }

 int DashExtractor::GetBufferedDuration(unsigned int /*nTrackID*/, long long  /*nBytes*/,
                                      unsigned long long */*pDuration*/)
 {
   return 1;
 }

 int DashExtractor::GetBufferedDuration(unsigned int /*nTrackID*/, unsigned long long */*pDuration*/)
 {
  return 1;
 }

}
