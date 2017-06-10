/************************************************************************* */
/**
 * DashMediaSource.cpp
 * @brief Implementation of DashMediaSource.
 *  DashMediaSource class implements the media source for DashExtractor
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */

/* =======================================================================
**               Include files for DashMediaSource.cpp
** ======================================================================= */

#include "DashMediaSource.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <utils/Vector.h>

#include "DashExtractorAdaptationLayer.h"
#include "ExtendedMediaExtractor.h"


namespace android {

  DashMediaSource::DashMediaSource(const int32_t portIndex, const int32_t trackId, const sp<DEALInterface> &deal, const sp<MetaData> &meta)
    : mPortIndex(portIndex),
    mTrackId(trackId),
    mEOSResult(OK),
    mPaused(false),
    mFormat(meta),
    mDEAL(deal)
  {

  }

  DashMediaSource::~DashMediaSource() {
  }

  void DashMediaSource::setFormat(const sp<MetaData> &meta) {
    Mutex::Autolock autoLock(mLock);
    mFormat = meta;
  }

  sp<MetaData> DashMediaSource::getFormat() {
    Mutex::Autolock autoLock(mLock);
    return mFormat;
  }


  status_t DashMediaSource::read(
    MediaBuffer **out, const ReadOptions *) {

      status_t ret = OK;

      Mutex::Autolock autoLock(mLock);

      if( (NULL == out) || (NULL == (*out)))
      {
        // TODO error code to be decided
        //ret = ?
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "DashMediaSource::read error out or *out is NULL trackId=%d", mTrackId);

      }
      else if(mEOSResult == OK)
      {
        if (!mBuffers.empty() && !mPaused)
        {
          sp<ABuffer> buffer = *mBuffers.begin();
          mBuffers.erase(mBuffers.begin());

          int32_t discontinuity;
          if (buffer->meta()->findInt32("discontinuity", &discontinuity))
          {
            // return INFO_DISCONTINUITY if either port settings changed on track or the track was removed
            ret = INFO_DISCONTINUITY;

            QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
              "DashMediaSource::read trackId=%d INFO_DISCONTINUITY", mTrackId);

            //query and update metadata in trackInfo
            mFormat.clear();
            mDEAL->QueryAndUpdateMetaData(mPortIndex);
            mFormat = mDEAL->getTrackMetaData(mTrackId);
          }
          else
          {
            int64_t timeUs;
            CHECK(buffer->meta()->findInt64("timeUs", &timeUs));

            (*out)->meta_data()->setInt64(kKeyTime, timeUs);

            int32_t bIsEncrypted;

            buffer->meta()->findInt32("encrypted", &bIsEncrypted);

            int32_t sync;
            if(buffer->meta()->findInt32("sync", &sync))
            {
              (*out)->meta_data()->setInt32(kKeyIsSyncFrame, sync ? true:false);
            }

            int32_t conf;
            if(buffer->meta()->findInt32("conf", &conf))
            {
              (*out)->meta_data()->setInt32(kKeyIsCodecConfig, conf ? true:false);
            }

#if 1 // to be enabled for text track once Keys gets finalized
            if(IS_TEXT_PORT(mPortIndex))
            {
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "read for text track");
              int32_t value = 0;

              if(buffer->meta()->findInt32("height", &value))
              {
                (*out)->meta_data()->setInt32(SMPTheight, value);
              }

              value = 0;
              if(buffer->meta()->findInt32("width", &value))
              {
                (*out)->meta_data()->setInt32(SMPTwidth, value);
              }

              value = 0;
              if(buffer->meta()->findInt32("duration", &value))
              {
                (*out)->meta_data()->setInt32(SMPTduration, value);
              }

              value = 0;
              if(buffer->meta()->findInt32("startoffset", &value))
              {
                (*out)->meta_data()->setInt32(SMPTstartoffset, value);
              }


              value = 0;
              if(buffer->meta()->findInt32("subSc", &value))
              {
                (*out)->meta_data()->setInt32(SMPTsubSc, value);
              }

              value = 0;
              if(buffer->meta()->findInt32("subSt", &value))
              {
                (*out)->meta_data()->setInt32(SMPTsubSt, value);
              }

              int32_t subInfoSize = 0;
              if(buffer->meta()->findInt32("subSz", &subInfoSize))
              {
                (*out)->meta_data()->setInt32(SMPTsubSz, subInfoSize);
              }

              AString subInfo;
              if(buffer->meta()->findString("subSi", &subInfo))
              {
                //(*out)->meta_data()->setData(SMPTsubSi, MetaData::TYPE_C_STRING, (const uint8_t *)subInfo.c_str(), subInfoSize);
                //(*out)->meta_data()->setCString(SMPTsubSi,(const uint8_t *)subInfo.c_str());
                (*out)->meta_data()->setCString(SMPTsubSi,(const char *)subInfo.c_str());
              }
            }
#endif
            if (bIsEncrypted == 1)
            {
              int Fd = -1; //(int)((*out)->data());
              if (Fd > 0)
              {
                QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Encrypted...Proceed for Decryption with FD 0X%x with size %d",
                   (unsigned int)Fd, buffer->size());
                ret = mDEAL->DoDecyrpt(buffer,Fd, mPortIndex);
                (*out)->set_range(0, buffer->size());
                ret = OK;

              }
            }
            else if(bIsEncrypted == 0 && (*out)->size() >= buffer->size())
            {
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Non-Encrypted Track...");
              memcpy((*out)->data(), buffer->data(), buffer->size());
              (*out)->set_range(0, buffer->size());

               //return OK if buffer read successfully
              ret = OK;
              QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                             "DashMediaSource::read trackId=%d OK timeUs %llu", mTrackId, timeUs);
              buffer = 0;
            }
            else
            {
              QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR," DashMediaSource::read trackId=%d error not enough memory for frame mem size is %d and req is %d ", mTrackId, (*out)->size(), buffer->size());
              mBuffers.push_front(buffer);
              buffer = 0;
              ret = ERROR_BUFFER_TOO_SMALL;
            }
          }
        }
        else
        {
          // return under-run since buffer queue is empty or reads are paused from DEAL
          ret = -EWOULDBLOCK;

          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
            "DashMediaSource::read trackId=%d UNDER_RUN", mTrackId);
        }
      }
      else
      {
        //return ERROR_END_OF_STREAM when eos or UNKNOWN_ERROR when any error occured
        ret = ERROR_END_OF_STREAM;

        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
          "DashMediaSource::read trackId=%d ERROR_EOS_OF_STREAM ..", mTrackId);
      }

      if((ret == OK || ret == -EWOULDBLOCK) && !mPaused)
      {
        if(mDEAL != NULL)
        {
          if(!(mDEAL->ReadFrameAsync(mPortIndex)))
          {
            ret = ERROR_END_OF_STREAM;
            QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
              "DashMediaSource::read trackId=%d ReadFramAsync failed ERROR_EOS_OF_STREAM", mTrackId);
          }
        }
        else
        {
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
            "DashMediaSource::read trackId=%d mDEAL object is NULL", mTrackId);
          ret = ERROR_END_OF_STREAM;
        }
      }

      return ret;
  }

  status_t DashMediaSource::pause()
  {
    mPaused = true;
    return OK;
  }

  status_t DashMediaSource::resume()
  {
    mPaused = false;
    return OK;
  }

  status_t DashMediaSource::flush()
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
      "DashMediaSource::flush flushing all access units");
    Mutex::Autolock autoLock(mLock);
    mBuffers.clear();
    mEOSResult = OK;
    return OK;
  }

  void DashMediaSource::queueAccessUnit(const sp<ABuffer> &buffer) {

    int64_t timeUs;
    CHECK(buffer->meta()->findInt64("timeUs", &timeUs));
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH, "DashMediaSource::queueAccessUnit timeUs=%lld us (%.2f secs)", timeUs, (double)timeUs / 1E6);

    Mutex::Autolock autoLock(mLock);
    mBuffers.push_back(buffer);
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH," DashMediaSource::queueAccessUnit buffer queue size is %d ",mBuffers.size() );
  }

  uint32_t DashMediaSource::getQueueSize() {
    return mBuffers.size();
  }

  void DashMediaSource::queueDiscontinuity() {
    Mutex::Autolock autoLock(mLock);

    mEOSResult = OK;

    sp<ABuffer> buffer = new ABuffer(0);
    buffer->meta()->setInt32("discontinuity", 1);
    mBuffers.push_back(buffer);
  }

  void DashMediaSource::signalEOS(status_t result) {
    CHECK(result != OK);

    Mutex::Autolock autoLock(mLock);
    mEOSResult = result;
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
      "DashMediaSource::mEOSResult %d for trackId=%d",mEOSResult, mTrackId);
  }

}  // namespace android
