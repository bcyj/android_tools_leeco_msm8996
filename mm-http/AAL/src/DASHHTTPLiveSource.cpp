/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "DASHHTTPLiveSource"
#define SRMax 30
#include "common_log.h"
#include <utils/Log.h>

#include "DASHHTTPLiveSource.h"
#include "DashPacketSource.h"
#include "ATSParser.h"
#include "qtv_msg.h"
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaBuffer.h>

#include "MMTimer.h"

namespace android {

#define  DASH_TIMED_TEXT_NOTIFICATION_PREFETCH  (2000 * 1000) //in us = 2secc


DashPlayer::DASHHTTPLiveSource::DASHHTTPLiveSource(const char *urlPath                          ,
                                                 const KeyedVector<String8, String8> *headers ,
                                                 OMX_U32 &nReturn                             ,
                                                 bool /*bValidUid*/                               ,
                                                 uid_t /*nUid*/)
    : mEOS(false),
      mSRid(0),
      mFinalResult(OK),
      mPrepareDone(OK)
{
     (void)MM_Debug_Initialize();

     QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                  "DASHHTTPLiveSource Constructor");
     nReturn = MMI_S_EFAIL;
     mNotifySourceStatus = NULL;
     mQOEEventNotify = NULL;
     mUri = urlPath;
     if (headers) {
         mUriHeaders = *headers;
     }

     mAudioMediaSource  = NULL;
     mVideoMediaSource  = NULL;
     mTextMediaSource  = NULL;

     mAudioPacketSource = NULL;
     mVideoPacketSource = NULL;
     mTextPacketSource = NULL;

     mVideoMetaData = NULL;
     mAudioMetaData = NULL;
     mTextMetaData = NULL;

     mLastAudioTimeUs = -1; //last audio timestamp read
     mLastVideoTimeUs = -1; // last video timestamp read
     mLastTextTimeUs = -1; //last text timestamp read
     mLastAudioReadTimeUs = -1; //last audio timestamp read
     mLastVideoReadTimeUs = -1; // last video timestamp read
     mLastTextReadTimeUs = -1; //last text timestamp read
     bForcedAudioEOS = false;
     bForcedVideoEOS = false;
     bForcedTextEOS = false;

     mAudioPacketQueueSize = 0;
     mVideoPacketQueueSize = 0;
     mTextPacketQueueSize = 0;

     //Timed Text Timer specific
     mTimedTextTimerHandle = NULL;
     mTimedTextTimerStarted = 0;
     mPrevTimedTextDuration = 0;

     mCurrentRenderingPosition = -1;
     bPaused = false;
     mLastTextSampleDuration = -1;
     mLastTextSampleTSNotified = -1;

     mNotifySourceStatus = NULL;
     mPostTextFrameNotify = NULL;
     mSeekedPositionUs = -1;
     bSeeked = false;

     mCachedBufferingNotification.mEvent = 0;
     mCachedBufferingNotification.mTrack = eTrackNone;
     mAudioBufQ = NULL;
     mVideoBufQ = NULL;
     mTextBufQ  = NULL;

     mMMIInterface = new DASHMMIInterface(urlPath, headers, nReturn); /* MMI Device would be opened here */
     if (!IS_SUCCESS(nReturn))
     {
         if (mMMIInterface != NULL)
         {
             //smart pointer deletes the object we just return NULL
             mMMIInterface = NULL;
         }
     }
     else
     {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "DASHHTTPLiveSource Constructor, DASHMMIInterface object created successfully");
        mMMIInterface->setHttpLiveSourceObj(this);
        /* create the media info class object*/
        mMediaInfo = new DASHMMIMediaInfo(mMMIInterface,nReturn);
        if (!IS_SUCCESS(nReturn))
        {
           QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                        "DASHHTTPLiveSource Constructor, DASHMediaInfo nReturn invalid");
           if (mMediaInfo != NULL)
           {
             //smart pointer deletes the object we just return NULL
             mMediaInfo = NULL;
           }
           if (mMMIInterface != NULL)
           {
             //smart pointer deletes the object we just return NULL
             mMMIInterface = NULL;
           }
        }
     }
     QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "DASHMMIMediaInfo object created successfully");

}

DashPlayer::DASHHTTPLiveSource::~DASHHTTPLiveSource() {

     QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "~DASHHTTPLiveSource destructor ");

/* delete all the objects which might have been created */
     if (mMMIInterface != NULL)
     {
       mMMIInterface->resetAudioSource();
       mMMIInterface->resetVideoSource();
       mMMIInterface->resetTextSource();
     }
     if (mAudioMediaSource != NULL)
     {
        mAudioMediaSource = NULL;
     }
     if (mVideoMediaSource != NULL)
     {
        mVideoMediaSource = NULL;
     }
     if (mTextMediaSource != NULL)
     {
        mTextMediaSource = NULL;
     }

     if (mVideoPacketSource != NULL)
     {
        flush(0); // flush all the video ABuffers
        mVideoPacketSource = NULL;
     }
     if (mAudioPacketSource != NULL)
     {
        flush(1); // flush all the audio ABuffers from Audio Packet source
        mAudioPacketSource = NULL;
     }
     if (mTextPacketSource != NULL)
     {
        flush(2); // flush all the text ABuffers
        mTextPacketSource = NULL;
     }

     if (mMediaInfo != NULL)
     {
         //smart pointer deletes the object we just return NULL
         mMediaInfo = NULL;
     }
     if (mMMIInterface != NULL)
     {
         //smart pointer deletes the object we just return NULL
         mMMIInterface = NULL;
     }


     mVideoMetaData = NULL;
     mAudioMetaData = NULL;
     mTextMetaData = NULL;
     mNotifySourceStatus = NULL;
     mPostTextFrameNotify = NULL;
     mQOEEventNotify = NULL;
     mSeekedPositionUs = -1;
     bSeeked = false;
     mCachedBufferingNotification.mEvent = 0;
     mCachedBufferingNotification.mTrack = eTrackNone;
    //mm-osal log de-init
    (void)MM_Debug_Deinitialize();
}

void DashPlayer::DASHHTTPLiveSource::start() {

    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                 "DASHHTTPLiveSource::start() ");
    if (mMMIInterface != NULL)
    {
      mMMIInterface->OpenMMI(&mUriHeaders);
    }
}

/**
 *  loop through list of  buffers to and return index of free buffer
 */
int32_t DashPlayer::DASHHTTPLiveSource::BufInfoQ::getReusableBufIndex()
{
  for(int32_t i = 0; i < (int32_t)bufQ.size(); ++i)
  {
    if(bufQ[i] != NULL)
    {
      QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_LOW,
      "getStrongCount mediatype %d strongcnt %d buf[%p]", track, bufQ[i]->getStrongCount(), bufQ[i]->data());
      if(bufQ[i]->getStrongCount() == 1)
      {
        // we're the only owner
        return i;
      }
    }
  }

  // Check if new Buffer can be allocated
  if(bufQ.size() < maxBufCnt)
  {
    sp<ABuffer> buf = new ABuffer(maxBufSize);
    if(buf != NULL)
    {
      bufQ.push_back(buf);
      QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
         "DASHHTTPLiveSource::BufInfoQ::getReusableBufIndex mediaType %d, buffer id: %d : %p allocated ", track, (int32_t)(bufQ.size() - 1) , buf->data());
      return (int32_t)(bufQ.size() - 1);
    }
    else
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
         "DASHHTTPLiveSource::BufInfoQ::getReusableBufIndex failed to allocate new buffer for mediaType %d", track);
    }
  }

  return -1;
}

sp<MetaData> DashPlayer::DASHHTTPLiveSource::getFormat(int pTrack) {

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "DASHHTTPLiveSource getFormat for track : %d", pTrack);

     sp<MetaData> meta;
    int nAudioTrackID = 0;
    int nVideoTrackID = 0;
    int nTextTrackID = 0;

    if (mMediaInfo == NULL)
    {
       QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                    "DASHHTTPLiveSource getFormat, mMediaInfo is NULL hence return NULL");
       return NULL;
    }

    // pTrack values are audio = 1 , video = 0, text = 2
    if (pTrack == eTrackAudio)
    {
       meta = mMediaInfo->getAudioMetadata(nAudioTrackID);
       if (meta != NULL)
       {
           QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                        "DASHHTTPLiveSource::start() -> Audio MetaData Found  ");
           if (mAudioMediaSource != NULL)
           {
               mAudioMediaSource = NULL;
           }

           /* Create Audio Media source */
           mAudioMediaSource = new DASHMMIMediaSource(meta,nAudioTrackID,mMediaInfo);
           if (mAudioMediaSource != NULL)
           {
               mAudioMediaSource->start();//start the media source
               if (mMMIInterface != NULL)  // register audio notify
               {
                  mMMIInterface->setAudioSource(mAudioMediaSource);
               }
               QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                            "DASHHTTPLiveSource::start() -> Audio Media Source created  ");

               if(mAudioBufQ == NULL)
               {
                 mAudioBufQ = new BufInfoQ(eTrackAudio, (uint32_t)mAudioMediaSource->getMaxBufferSize(), (uint32_t)AUDIO_PACKET_QUEUE_MAX_SIZE);
                 if(mAudioBufQ == NULL)
                 {
                   QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                                "DASHHTTPLiveSource::start() -> Audio Buffer info Q creation failed ");
                 }
               }
           }
           else
           {
               QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                            "DASHHTTPLiveSource::start() -> Audio Media Source creation failed ");
           }
        }
    }
    else if (pTrack == eTrackVideo)
    {
       meta = mMediaInfo->getVideoMetadata(nVideoTrackID);
       if (meta != NULL)
       {
           QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                        "DASHHTTPLiveSource::start() -> Video MetaData Found  ");

           if (mVideoMediaSource != NULL)
           {
               mVideoMediaSource = NULL;
           }

       /* Create Video media source */
           mVideoMediaSource = new DASHMMIMediaSource(meta,nVideoTrackID,mMediaInfo);
           if (mVideoMediaSource != NULL)
           {
               mVideoMediaSource->start();//start the media source
               if (mMMIInterface != NULL)  // register video notify
               {
                  mMMIInterface->setVideoSource(mVideoMediaSource);
                  if (mMMIInterface-> getDrmType() != QOMX_NO_DRM) {
                      meta->setInt32(kKeyIsDRM, 1);
                  }
               }
               QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                            "DASHHTTPLiveSource::start() -> Video Media Source created  ");

               if(mVideoBufQ == NULL)
               {
                 mVideoBufQ = new BufInfoQ(eTrackVideo, (uint32_t)mVideoMediaSource->getMaxBufferSize(), (uint32_t)VIDEO_PACKET_QUEUE_MAX_SIZE);
                 if(mVideoBufQ == NULL)
                 {
                   QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                                "DASHHTTPLiveSource::start() -> Video Buffer info Q creation failed ");
                 }
               }
           }
           else
           {
               QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                            "DASHHTTPLiveSource::start() -> Video Media Source creation failed ");
           }
        }
    }
    else if (pTrack == eTrackText)
    {
       meta = mMediaInfo->getTextMetadata(nTextTrackID);
       if (meta != NULL)
       {
           QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                        "DASHHTTPLiveSource::start() -> Text MetaData Found  ");
           if (mTextMediaSource != NULL)
           {
               mTextMediaSource = NULL;
           }

       /* Create Text media source */
           mTextMediaSource = new DASHMMIMediaSource(meta,nTextTrackID,mMediaInfo);
           if (mTextMediaSource != NULL)
           {
              mTextMediaSource->start();//start the media source
              if (mMMIInterface != NULL)  // register text notify
              {
                 mMMIInterface->setTextSource(mTextMediaSource);
              }
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                           "DASHHTTPLiveSource::start() -> Text Media Source created  ");

              if(mTextBufQ == NULL)
              {
                mTextBufQ = new BufInfoQ(eTrackText, (uint32_t)mTextMediaSource->getMaxBufferSize(), (uint32_t)TEXT_PACKET_QUEUE_MAX_SIZE);
                if(mTextBufQ == NULL)
                {
                  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                              "DASHHTTPLiveSource::start() -> Text Buffer info Q creation failed ");
                }
              }
           }
           else
           {
               QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                            "DASHHTTPLiveSource::start() -> Text Media Source creation failed ");
           }
        }
    }
    return meta;
}

void DashPlayer::DASHHTTPLiveSource::stop() {

   if (mTimedTextTimerHandle != NULL)
   {
     MM_Timer_Stop(mTimedTextTimerHandle);
     MM_Timer_Release(mTimedTextTimerHandle);
     mTimedTextTimerHandle = NULL;
   }

   if (mMMIInterface != NULL)
   {
      //Call flush on the mmi interface
      mMMIInterface->flush();
   }
}

status_t DashPlayer::DASHHTTPLiveSource::feedMoreTSData() {

    status_t nReturn = OK;

    if (mFinalResult != OK) {
      return mFinalResult;
    }

    //read audio
    nReturn = fillAudioPacketSource();
    if ( (nReturn != ((status_t)OK)))
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                    "DASHHTTPLiveSource::feedMoreTSData fail for Audio status %d", nReturn);
      return UNKNOWN_ERROR;
    }

    // continue reading video frame
    nReturn = fillVideoPacketSource();
    if ( (nReturn != ((status_t)OK)))
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                    "DASHHTTPLiveSource::feedMoreTSData fail for Video status %d", nReturn);
      return UNKNOWN_ERROR;
    }

    // continue reading Text
    nReturn = fillTextPacketSource();
    if ( (nReturn != ((status_t)OK)))
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                    "DASHHTTPLiveSource::feedMoreTSData fail for Text status %d", nReturn);
      return UNKNOWN_ERROR;
    }

    return OK;
}

status_t DashPlayer::DASHHTTPLiveSource::dequeueAccessUnit(
        int pTrack, sp<ABuffer> *accessUnit) {
   status_t nReturn;
   sp<DashPacketSource> packetSourceHndl = NULL;
   int64_t nTimeUs = -1;
   int nSize = -1;

   if(accessUnit == NULL)
   {
     return -EINVAL;
   }

   accessUnit->clear();

   if (mMMIInterface != NULL && !mMMIInterface->isLiveStream())
   {
       if (!allowDequeueAccessUnit(pTrack))
       {
         QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "DASHHTTPLiveSource::dequeueAccessUnit -> Hold Track %d!!",pTrack);
         return -EWOULDBLOCK;
       }
   }

   if ((mMediaInfo != NULL) && mMediaInfo->HasVideo() && mMediaInfo->HasAudio() && mMediaInfo->HasText())
   {
     switch (pTrack)
     {
        case eTrackAudio : //audio track
        {
           if (mLastAudioTimeUs != -1           &&
              (mLastVideoTimeUs == -1  ||
               mLastTextTimeUs == -1) && (mFinalResult == OK))
               {
                  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                               "DASHHTTPLiveSource::dequeueAccessUnit -> Hold Audio as Video/Text Not Started Yet!!");
                  return -EWOULDBLOCK;
               }
               break;
        }
        case eTrackText: // text track
        {
           if (mLastTextTimeUs  != -1           &&
              (mLastVideoTimeUs == -1  ||
               mLastAudioTimeUs == -1)  && (mFinalResult == OK))
               {
                  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                               "DASHHTTPLiveSource::dequeueAccessUnit -> Hold Text as Audio/Video Not Started Yet!!");
                  return -EWOULDBLOCK;
               }
               break;
        }
        case eTrackVideo: // video track
        {
           if (mLastVideoTimeUs  != -1 &&
               ( mLastTextTimeUs == -1          ||
                 mLastAudioTimeUs == -1)        &&
               (mFinalResult == OK))
           {
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                           "DASHHTTPLiveSource::dequeueAccessUnit -> Hold Video as Audio/Text Not Started Yet!!");
              return -EWOULDBLOCK;
           }
           break;
        }
        default :
        {
          // nothing to do here
          break;
        }
     }
   }
   else if ((mMediaInfo != NULL) && mMediaInfo->HasVideo() && mMediaInfo->HasAudio())
   {
     if ((pTrack == eTrackAudio) && (mLastAudioTimeUs != -1) && (mLastVideoTimeUs == -1) && (mFinalResult == OK))
     {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                     "DASHHTTPLiveSource::dequeueAccessUnit -> Hold Audio as Video Not Started Yet!!");
        return -EWOULDBLOCK;
     }
   }
   else if ((mMediaInfo != NULL) && mMediaInfo->HasVideo() && mMediaInfo->HasText())
   {
     if ((pTrack == eTrackText) && ((mLastTextTimeUs != -1) && (mLastVideoTimeUs == -1)) && (mFinalResult == OK))
     {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                     "DASHHTTPLiveSource::dequeueAccessUnit -> Hold Text as Video Not Started Yet!!");
        return -EWOULDBLOCK;
     }
     else if ((pTrack == eTrackVideo) && ((mLastTextTimeUs == -1) && (mLastVideoTimeUs != -1)) && (mFinalResult == OK))
     {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                     "DASHHTTPLiveSource::dequeueAccessUnit -> Hold Video as Text Not Started Yet!!");
        return -EWOULDBLOCK;
     }
   }
   else if ((mMediaInfo != NULL) && mMediaInfo->HasAudio() && mMediaInfo->HasText())
   {
     if ((pTrack == eTrackText) && (mLastTextTimeUs != -1) && (mLastAudioTimeUs == -1) && (mFinalResult == OK))
     {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                     "DASHHTTPLiveSource::dequeueAccessUnit -> Hold Text as Audio Not Started Yet!!");
        return -EWOULDBLOCK;
     }
     else if ((pTrack == eTrackAudio) && (mLastTextTimeUs == -1) && (mLastAudioTimeUs != -1) && (mFinalResult == OK))
     {
         QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                      "DASHHTTPLiveSource::dequeueAccessUnit -> Hold Audio as Text Not Started Yet!!");
          return -EWOULDBLOCK;
     }
   }

    // DashPlayer sends pTrack values as audio = 1 , video = 0, text = 2
    if (pTrack == eTrackVideo) //video
    {
        packetSourceHndl  =  mVideoPacketSource;
    }
    else if (pTrack == eTrackAudio) //audio
    {
        packetSourceHndl  =  mAudioPacketSource;
    }
    else if (pTrack == eTrackText) //text
    {
        packetSourceHndl  =  mTextPacketSource;
    }

    if (packetSourceHndl == NULL) {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                     "DASHHTTPLiveSource dequeueAccessUnit -> Audio/Video/Text Another Packet Source not found");
        return -EWOULDBLOCK;
    }

    status_t finalResult;
    // if access unit is there @ packet source
    if (!packetSourceHndl->hasBufferAvailable(&finalResult)) {
        return finalResult == OK ? -EWOULDBLOCK : finalResult;
    }

    // return the Access unt
    nReturn = packetSourceHndl->dequeueAccessUnit(accessUnit);

    if(pTrack == eTrackVideo)
    {
        mVideoPacketQueueSize--;
        nSize = mVideoPacketQueueSize;
        (*accessUnit)->meta()->findInt64("timeUs", &mLastVideoReadTimeUs);
        nTimeUs = mLastVideoReadTimeUs;
    }
    else if (pTrack == eTrackAudio)
    {
        mAudioPacketQueueSize--;
        nSize = mAudioPacketQueueSize;
        (*accessUnit)->meta()->findInt64("timeUs", &mLastAudioReadTimeUs);
        nTimeUs = mLastAudioReadTimeUs;
    }
    else if(pTrack == eTrackText)
    {
        mTextPacketQueueSize--;
        nSize = mTextPacketQueueSize;
        (*accessUnit)->meta()->findInt64("timeUs", &mLastTextReadTimeUs);
        nTimeUs = mLastTextReadTimeUs;
    }

    QTV_MSG_PRIO4( MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "DASHHTTPLiveSource dequeueAccessUnit -> Track %d, Status %d, Sample TS %.2f secs, Remaining samples %d",
                   pTrack, nReturn, (double)nTimeUs / 1E6, nSize );

    return nReturn;
}


status_t DashPlayer::DASHHTTPLiveSource::getDuration(int64_t *pDurationUs) {

    return ( (mMediaInfo != NULL) ? mMediaInfo->getDuration(pDurationUs) : (uint32_t)BAD_VALUE); /* gets the duration of the clip, in case of live it will be 0 */
}

status_t DashPlayer::DASHHTTPLiveSource::seekTo(int64_t nSeekTimeUs) {
    status_t ret = (status_t)UNKNOWN_ERROR;
    mSeekedPositionUs = -1;

    if( mFinalResult != OK  )
    {
      if( mFinalResult == ERROR_END_OF_STREAM )
      {
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "Allow seek even though EOS is set");
          mFinalResult = OK;
      }
      else
      {
         QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Error state %d, Ignore this seek", mFinalResult);
         return mFinalResult;
      }
    }
    if (mMMIInterface != NULL)
    {
       // Post a seek command to DASH
       ret = mMMIInterface->seekTo(nSeekTimeUs);

       if (ret == OK) // seek success
       {

          if (mTimedTextTimerHandle != NULL)
          {
             MM_Timer_Stop(mTimedTextTimerHandle);
             MM_Timer_Release(mTimedTextTimerHandle);
             mTimedTextTimerHandle = NULL;
             mTimedTextTimerStarted = 0;
          }

          if ( (mVideoPacketSource != NULL) && ((mMediaInfo != NULL) &&  mMediaInfo->HasVideo()))
          {
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                           "Flushing Video PacketSource");
              mVideoPacketSource->queueDiscontinuity(ATSParser::DISCONTINUITY_TIME,NULL); // flush all the frames
              mVideoPacketQueueSize = 0;
              mMediaInfo->setMediaPresence(eTrackVideo,false);
          }
          if ( (mAudioPacketSource != NULL) && ((mMediaInfo != NULL) && mMediaInfo->HasAudio()))
          {
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                           "Flushing Audio PacketSource");
              mAudioPacketSource->queueDiscontinuity(ATSParser::DISCONTINUITY_TIME,NULL); // flush all the frames
              mAudioPacketQueueSize = 0;
              mMediaInfo->setMediaPresence(eTrackAudio,false);
          }
          if ( (mTextPacketSource != NULL) && ((mMediaInfo != NULL) && mMediaInfo->HasText()))
          {
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                           "Flushing Text PacketSource");
              mTextPacketSource->queueDiscontinuity(ATSParser::DISCONTINUITY_TIME,NULL); // flush all the frames
              mTextPacketQueueSize = 0;
              mMediaInfo->setMediaPresence(eTrackText,false);
          }
          mLastAudioTimeUs = -1; //reset last audio timestamp read
          mLastVideoTimeUs = -1; // reset last video timestamp read
          mLastTextTimeUs = -1; //reset last text timestamp read
          bForcedAudioEOS = false;
          bForcedVideoEOS = false;
          bForcedTextEOS  = false;
          mLastAudioReadTimeUs = -1; //reset last audio timestamp read
          mLastVideoReadTimeUs = -1; // reset last video timestamp read
          mLastTextReadTimeUs = -1; //reset last text timestamp read
          mPrevTimedTextDuration = 0;
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "Reset EOS Bit as new seek received");
          mMMIInterface->m_bAudEos = false;
          mMMIInterface->m_bVidEos = false;
          mMMIInterface->m_bTextEos = false;
          bSeeked = true;
       }
       else if (ret == INVALID_OPERATION)
       {
         if ( mVideoPacketSource != NULL)
         {
            VideoNotifyCB(NULL,ret);
         }
         if ( mAudioPacketSource != NULL)
         {
            AudioNotifyCB(NULL,ret);
         }
         if ( mTextPacketSource != NULL)
         {
            TextNotifyCB(NULL,ret);
         }
       }

    }
    return ret;
}

bool DashPlayer::DASHHTTPLiveSource::isSeekable() {
  return ((mMMIInterface!=NULL) ? mMMIInterface->IsSeekable() : false);
      }

status_t DashPlayer::DASHHTTPLiveSource::getRepositionRange(uint64_t* pMin, uint64_t* pMax, uint64_t* pMaxDepth) {
  return ((mMMIInterface!=NULL) ? mMMIInterface->getRepositionRange(pMin, pMax, pMaxDepth) : (status_t)UNKNOWN_ERROR);
}

bool DashPlayer::DASHHTTPLiveSource::isPlaybackDiscontinued() {
  return ((mMMIInterface!=NULL) ? mMMIInterface->isPlaybackDiscontinued() : false);
}

status_t DashPlayer::DASHHTTPLiveSource::getTrackInfo(Parcel *reply) {
  return ((mMMIInterface!=NULL) ? mMMIInterface->getTrackInfo(reply) : (status_t)UNKNOWN_ERROR);
}

status_t DashPlayer::DASHHTTPLiveSource::prepareAsync() {

    status_t ret = (status_t)UNKNOWN_ERROR;
    if (mMMIInterface != NULL)
    {
      mPrepareDone = -EWOULDBLOCK;
      ret = mMMIInterface->prepareAsync();
    }
    return ret;
}


status_t DashPlayer::DASHHTTPLiveSource::isPrepareDone() {

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "PrepareDone %d ", mPrepareDone);
    return mPrepareDone;
}

status_t DashPlayer::DASHHTTPLiveSource::getParameter(int key, void **data, size_t *size) {

    status_t ret = (status_t)UNKNOWN_ERROR;
    if (mMMIInterface != NULL)
    {
      ret = mMMIInterface->getParameter(key, data, size);
    }
    return ret;
}

status_t DashPlayer::DASHHTTPLiveSource::setParameter(int key, void *data, size_t size) {

    status_t ret = (status_t)UNKNOWN_ERROR;
    if (mMMIInterface != NULL)
    {
      ret = mMMIInterface->setParameter(key, data, size);
    }
    return ret;
}
status_t DashPlayer::DASHHTTPLiveSource::getMediaPresence(bool &audio, bool &video, bool &text)
{

   if (mMediaInfo != NULL)
   {
      audio = mMediaInfo->HasAudio();
      video = mMediaInfo->HasVideo();
      text = mMediaInfo->HasText();
   }
   return OK;
}

// DASHHTTPLiveSource calss private function definition starts here
status_t DashPlayer::DASHHTTPLiveSource::flush(int pTrack)
{
   QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "flush");

   if(pTrack == eTrackVideo)
   {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "Flushing Video Packet source queue");
      sp<ABuffer> accessUnit;
      while(dequeueAccessUnit(pTrack,&accessUnit) == OK)
      {
        accessUnit = NULL;
        mVideoPacketQueueSize--;
      }
   }
   else if (pTrack == eTrackAudio)
   {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "Flushing Audio Packet source queue");
      sp<ABuffer> accessUnit;
      while(dequeueAccessUnit(pTrack,&accessUnit) == OK)
      {
        accessUnit = NULL;
        mAudioPacketQueueSize--;
      }
   }
   else if(pTrack == eTrackText)
   {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "Flushing Text Packet source queue");
      sp<ABuffer> accessUnit;
      while(dequeueAccessUnit(pTrack,&accessUnit) == OK)
      {
        accessUnit = NULL;
        mTextPacketQueueSize--;
      }
   }
   return OK;
}

status_t DashPlayer::DASHHTTPLiveSource::queueBuffer(int pTrack, bool &bCodecConfig)
{
     if (pTrack == eTrackVideo) //queue video stream
     {
        return getVideoStream(bCodecConfig);
     }
     else if (pTrack == eTrackAudio) //queue audio stream
     {
        return getAudioStream(bCodecConfig);
     }
     else if (pTrack == eTrackText) // //queue text stream
     {
        return getTextStream(bCodecConfig);
     }
     return (status_t)UNKNOWN_ERROR;
}

status_t DashPlayer::DASHHTTPLiveSource::fillAudioPacketSource()
{
    Mutex::Autolock Lock(mLockAudioFrame);

    status_t nQueueStatus = OK;

    sp<MetaData> meta = NULL;

    if ( ((mMediaInfo != NULL) && !mMediaInfo->HasAudio())       || // support audio  only clips
         ((mMMIInterface != NULL) && (mMMIInterface->m_bAudEos)) ||
         (bForcedAudioEOS == true)                               ||
         (mAudioMediaSource == NULL)
       )
    {
       // if Audio EOS is reached or clip dosen't have audio then we should not read frames ...
       return nQueueStatus;
    }

    // if meta data is not available then get that
    if (mAudioMetaData == NULL)
    {
       mAudioMetaData = getFormat(1);
    }

    // if audio is available from the content then read the data from audio media source
    meta = mAudioMetaData;

    if (meta != NULL) //audio available
    {
      if (mAudioPacketSource == NULL)
      {
         mAudioPacketSource = new DashPacketSource(meta); // !warning : Make sure to update the meta information with in Another packet source when CODEC Change happens
      }
      bool bCodecConfig = false;
      nQueueStatus = queueBuffer(eTrackAudio, bCodecConfig);
    }
    meta = NULL;
    return nQueueStatus;
}

status_t DashPlayer::DASHHTTPLiveSource::fillVideoPacketSource()
{
    Mutex::Autolock Lock(mLockVideoFrame);

    status_t nQueueStatus = OK;

    sp<MetaData> meta = NULL;

    if (((mMediaInfo != NULL) && !mMediaInfo->HasVideo())       ||  // support video only clips
        ((mMMIInterface != NULL) && (mMMIInterface->m_bVidEos)) ||
        (bForcedVideoEOS == true)                               ||
        (mVideoMediaSource == NULL)
       )
    {
      // if video EOS is reached or clip dosen't have video we should not read the frames any more...
      return nQueueStatus;
    }
    // if video meta data not available then get that
    if (mVideoMetaData == NULL)
    {
      mVideoMetaData = getFormat(0);
    }
    // if video is available from the content then read the data from video media source
    meta = mVideoMetaData;
    if (meta != NULL) //video available
    {
      if (mVideoPacketSource == NULL)
      {
        mVideoPacketSource = new DashPacketSource(meta);  // !warning : Make sure to update the meta information with in Another packet source when CODEC Change happens
      }
      status_t finalResult;
      bool bCodecConfig = false;
      // queues the frame into Video Packet source
      nQueueStatus = queueBuffer(eTrackVideo, bCodecConfig);

    }
    meta = NULL;
    return nQueueStatus;
}

status_t DashPlayer::DASHHTTPLiveSource::fillTextPacketSource()
{
    Mutex::Autolock Lock(mLockTextFrame);

    status_t nQueueStatus = OK;

    sp<MetaData> meta = NULL;

    if (((mMediaInfo != NULL) && !mMediaInfo->HasText())       ||  // support video only clips
        ((mMMIInterface != NULL) && (mMMIInterface->m_bTextEos)) ||
        (bForcedTextEOS == true)                               ||
        (mTextMediaSource == NULL)
       )
    {
      // if Text EOS is reached or clip dosen't have Text we should not read the frames any more...
      return nQueueStatus;
    }
    // if text meta data not available then get that
    if (mTextMetaData == NULL)
    {
      mTextMetaData = getFormat(2);
    }
    // if text is available from the content then read the data from text media source
    meta = mTextMetaData;
    if (meta != NULL) //video available
    {
      if (mTextPacketSource == NULL)
      {
        mTextPacketSource = new DashPacketSource(meta);
      }
      status_t finalResult;
      bool bCodecConfig = false;
      // queues the frame into Text Packet source
      nQueueStatus = queueBuffer(eTrackText, bCodecConfig);

    }
    meta = NULL;
    return nQueueStatus;
}

status_t DashPlayer::DASHHTTPLiveSource::getAudioStream(bool &bCodecConfig)
{

  status_t nReturn = OK;
  bCodecConfig = false;
  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Get Audio Stream");

  if(mAudioBufQ == NULL)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
               "NULL BufQ for Audio Stream");
    return (status_t) UNKNOWN_ERROR;
  }

  int32_t index = mAudioBufQ->getReusableBufIndex();
  sp<ABuffer> accessUnit = NULL;

  if((index >= 0 && index < (int32_t)mAudioBufQ->bufQ.size()) &&
     ((accessUnit = mAudioBufQ->bufQ[index]) != NULL))
  {
     accessUnit->meta()->clear();
     QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Get Audio Stream %d %p", index, accessUnit->data());

    if (mMMIInterface != NULL)
    {
      // read the one access unit from MediaSource
      nReturn = mMMIInterface->readFrameAsync(0, accessUnit);  // read audio
    }

    if (nReturn != OK)
    {
      accessUnit = NULL;
    }
  }
  else if(index >= 0)
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Get Audio Stream Error NULL buffer or invalid buffer index %d ", index);
    nReturn = (status_t)UNKNOWN_ERROR;
  }
  else
  {
    // index = -1, non of the pre allocated buffers available for reuse currently
    nReturn = OK;
  }

  return nReturn;
}

status_t DashPlayer::DASHHTTPLiveSource::getVideoStream(bool &bCodecConfig)
{
  status_t nReturn = OK;
  bCodecConfig = false;
  // this function needs to be executed when video media source and video packet source is available
  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Get Video Stream");

  if(mVideoBufQ == NULL)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
               "NULL BufQ for Video Stream");
    return (status_t)UNKNOWN_ERROR;;
  }

  int32_t index = mVideoBufQ->getReusableBufIndex();
  sp<ABuffer> accessUnit = NULL;

  if((index >= 0 && index < (int32_t)mVideoBufQ->bufQ.size()) &&
     ((accessUnit = mVideoBufQ->bufQ[index]) != NULL))
  {
    accessUnit->meta()->clear();
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Get Video Stream %d %p", index, accessUnit->data());

    if (mMMIInterface != NULL)
    {
     // read video access unit from Media Source
     nReturn  =  mMMIInterface->readFrameAsync(1, accessUnit); // read video
    }
    if (nReturn != OK)
    {
     accessUnit = NULL;
    }
  }
  else if(index >= 0)
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Get Video Stream Error NULL buffer or invalid buffer index %d ", index);
    nReturn = (status_t)UNKNOWN_ERROR;
  }
  else
  {
    // index = -1, non of the pre allocated buffers available for reuse currently
    nReturn = OK;
  }

  return nReturn;

}

status_t DashPlayer::DASHHTTPLiveSource::getTextStream(bool &bCodecConfig)
{
  status_t nReturn = OK;
  bCodecConfig = false;
  // this function needs to be executed when text media source and text packet source is available
  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Get Text Stream");

  if(mTextBufQ == NULL)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
               "NULL BufQ for Text Stream");
    return (status_t)UNKNOWN_ERROR;;
  }

  int32_t index = mTextBufQ->getReusableBufIndex();
  sp<ABuffer> accessUnit = NULL;

  if((index >= 0 && index < (int32_t)mTextBufQ->bufQ.size()) &&
     ((accessUnit = mTextBufQ->bufQ[index]) != NULL))
  {
    accessUnit->meta()->clear();
    QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "Get Text Stream %d %p %s", index, accessUnit->data(), accessUnit->meta()->debugString().c_str());

    if (mMMIInterface != NULL)
    {
      // read text access unit from Media Source
      nReturn  =  mMMIInterface->readFrameAsync(2, accessUnit); // read video
    }

    if (nReturn != OK)
    {
       accessUnit = NULL;
    }
  }
  else if(index >= 0)
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Get Text Stream Error NULL buffer or invalid buffer index %d ", index);
    nReturn = (status_t)UNKNOWN_ERROR;
  }
  else
  {
    // index = -1, non of the pre allocated buffers available for reuse currently
    nReturn = OK;
  }

  return nReturn;
}

void DashPlayer::DASHHTTPLiveSource::AudioNotifyCB(sp<ABuffer> accessUnit, status_t nStatus)
{

        Mutex::Autolock Lock(mLockAudioFrame);
        switch(nStatus)
        {
           case DASH_ERROR_NOTIFY_BUFFERING_START:
           {
                if (isPaused())
                {
                   QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                                "AudioNotifyCB:: Caching DASH_ERROR_NOTIFY_BUFFERING_START in paused state");
                   mCachedBufferingNotification.mEvent = DASH_ERROR_NOTIFY_BUFFERING_START;
                   mCachedBufferingNotification.mTrack = eTrackAudio;
                   break;
                 }
                 QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                              "DashPlayer::DASHHTTPLiveSource::AudioNotifyCB -> DASH_ERROR_NOTIFY_BUFFERING_START");
                    sp<AMessage> sourceRequest = new AMessage;

                    if (sourceRequest != NULL)
                    {
                      sourceRequest->setInt32("what", kWhatBufferingStart);
                      sourceRequest->setInt64("track", eTrackAudio);
                   if (SourceNotifyCB(sourceRequest))
                   {
                      mCachedBufferingNotification.mEvent = 0;
                      mCachedBufferingNotification.mTrack = eTrackNone;
                   }
                 }
                 break;
           }
           case DASH_ERROR_NOTIFY_BUFFERING_END:
           {
                 if (isPaused())
                 {
                    if (mCachedBufferingNotification.mEvent == DASH_ERROR_NOTIFY_BUFFERING_START)
                    {
                        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                                     "AudioNotifyCB:: Resetting Cached DASH_ERROR_NOTIFY_BUFFERING_START as its in paused state");
                        mCachedBufferingNotification.mEvent = 0;
                        mCachedBufferingNotification.mTrack = eTrackNone;
                        break;
                    }
                    else if(mCachedBufferingNotification.mEvent == 0)
                    {
                        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                                    "AudioNotifyCB:: Caching DASH_ERROR_NOTIFY_BUFFERING_END in paused state");
                        mCachedBufferingNotification.mEvent = DASH_ERROR_NOTIFY_BUFFERING_END;
                        mCachedBufferingNotification.mTrack = eTrackAudio;
                        break;
                    }
                 }
                 QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                              "DashPlayer::DASHHTTPLiveSource::AudioNotifyCB -> DASH_ERROR_NOTIFY_BUFFERING_END");
                    sp<AMessage> sourceRequest = new AMessage;
                    if (sourceRequest != NULL)
                    {
                      sourceRequest->setInt32("what", kWhatBufferingEnd);
                      sourceRequest->setInt64("track", eTrackAudio);
                 if (SourceNotifyCB(sourceRequest))
                 {
                      mCachedBufferingNotification.mEvent = 0;
                      mCachedBufferingNotification.mTrack = eTrackNone;
                   }
                 }
                 break;
           }
     case OK: // if access unit got
                  int64_t timeUs;
                  int32_t aCodecConfig;
                  if (accessUnit != NULL)
                  {
                    CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));
                    accessUnit->meta()->findInt32("conf", &aCodecConfig);
                  }

                  //QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                  //             "During Seek CodecConfigValue: %d",aCodecConfig);

                  if (aCodecConfig == 0)
                  {
                    mLastAudioTimeUs = timeUs; // save the current/last audio time stamp,
                    if (bSeeked == true)
                    {
                      if (mSeekedPositionUs == -1)
                      {
                         mSeekedPositionUs = timeUs;
                         QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                                      "mSeekedPositionUs From Audio (%.2f) sec",(double)mSeekedPositionUs / 1E6);
                      }
                      bSeeked = false;
                    }
                  }
                  else
                  {
                    //bCodecConfig = true;
                  }

                  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                               " Received AudioBuf[%p] Queuing AudioTimeStamp: %.2f secs", accessUnit->data(), (double)timeUs / 1E6);
                  mAudioPacketSource->queueAccessUnit(accessUnit);
                  mAudioPacketQueueSize++;
                  break;
           case ERROR_END_OF_STREAM: // if EOS is reached
                  // EOS case
                  if ((mMediaInfo != NULL) && mMediaInfo->HasVideo() && mMediaInfo->HasText())
                  {
                     mEOS = ((mMMIInterface != NULL) && mMMIInterface->m_bVidEos && mMMIInterface->m_bAudEos && mMMIInterface->m_bTextEos); // EOS reached
                  }
                  else if ((mMediaInfo != NULL) && mMediaInfo->HasVideo())
                  {
                     mEOS = ((mMMIInterface != NULL) && mMMIInterface->m_bVidEos && mMMIInterface->m_bAudEos); // EOS reached
                  }
                  else
                  {
                     mEOS = ((mMMIInterface != NULL) && mMMIInterface->m_bAudEos && true);
                  }

                  if (mEOS || mMediaInfo == NULL)
                  {
                    mFinalResult = ERROR_END_OF_STREAM;
                  }

                  accessUnit = NULL;
                  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                               "Audio EOS reached: %d",mEOS);
                  mLastAudioTimeUs = 0;
                  mAudioPacketSource->signalEOS(ERROR_END_OF_STREAM);
                  break;
          case INFO_DISCONTINUITY:  // codec change not handled

                  accessUnit = NULL;
                  mAudioMetaData = NULL;
                  /* INFO_DISCONTINUITY is trigged when there is a codec change/format change
                   * incase of resolution change there won't be any discontinuty queued instead decoder will autometically detect the resolution
                   * change and accordingly renderer will render the frames.
                   */
                  // Only incase of Codec change, we should get INFO_DISCONTINUITY and in this case we need to shut down and reinit the decoder
                  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                               "Audio DISCONTINUITY being queued, due to codec change ");
                  mAudioPacketSource->queueDiscontinuity(ATSParser::DISCONTINUITY_AUDIO_FORMAT, NULL);

                  break;
          case ERROR_IO:  // some error in the bit stream may be
                  accessUnit = NULL;
                  // signal error.. so that PB can be stopped for that stream
                  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                               "Audio Read ERROR_IO, queueing ERROR_END_OF_STREAM for audio streams");
                  mAudioPacketSource->signalEOS(ERROR_END_OF_STREAM);  //ERROR_IO
                  // there is an eror with in the steam and hence Forced EOS is queued
                  bForcedAudioEOS = true;
                  break;
            case ERROR_DRM_CANNOT_HANDLE: // ABORT has been issued
                 accessUnit = NULL;
                 // signal error.. so that PB can be stopped
                 QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                                "Audio Read Error, queueing ERROR_DRM_CANNOT_HANDLE for both the streams %d",nStatus);
                 mAudioPacketSource->signalEOS(ERROR_DRM_CANNOT_HANDLE); //ERROR_IO
                 bForcedAudioEOS = true;
                 if (mVideoPacketSource != NULL)
                 {
                     mVideoPacketSource->signalEOS(ERROR_DRM_CANNOT_HANDLE); //ERROR_IO
                     bForcedVideoEOS = true;
                 }
                 if (mTextPacketSource != NULL)
                 {
                     bForcedTextEOS = true;
                 }
                 //As EOS is queued on both the streames we should update mFinalResult.
                 mFinalResult = ERROR_DRM_CANNOT_HANDLE;
                 break;

          case UNKNOWN_ERROR: // ABORT has been issued
          accessUnit = NULL;
          // signal error.. so that PB can be stopped
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Video Read Error, queueing UNKNOWN_ERROR for all the streams %d",nStatus);
          mAudioPacketSource->signalEOS(UNKNOWN_ERROR); //ERROR_IO
          bForcedAudioEOS = true;
          if (mVideoPacketSource != NULL)
          {
              mVideoPacketSource->signalEOS(UNKNOWN_ERROR); //ERROR_IO
              bForcedVideoEOS = true;
          }
          if (mTextPacketSource != NULL)
          {
              mTextPacketSource->signalEOS(UNKNOWN_ERROR);
              bForcedTextEOS = true;
          }
          //As EOS is queued on both the streames we should update mFinalResult.
          mFinalResult = UNKNOWN_ERROR;
          break;
          case INVALID_OPERATION:
          default:
                  accessUnit = NULL;
                  // signal error.. so that PB can be stopped
                  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                               "Audio Read Error (%d) , queueing ERROR_END_OF_STREAM for both the streams",nStatus);
                  mAudioPacketSource->signalEOS(ERROR_END_OF_STREAM);
                  bForcedAudioEOS = true;
                  if (mVideoPacketSource != NULL)
                  {
                     mVideoPacketSource->signalEOS(ERROR_END_OF_STREAM);
                     bForcedVideoEOS = true;
                  }
                  if (mTextPacketSource != NULL)
                  {
                     mTextPacketSource->signalEOS(ERROR_END_OF_STREAM);
                     bForcedTextEOS = true;
                  }
                  //As EOS is queued on both the streames we should update mFinalResult.
                  mFinalResult = ERROR_END_OF_STREAM;
                  break;
        } // switch

}

void DashPlayer::DASHHTTPLiveSource::VideoNotifyCB(sp<ABuffer> accessUnit, status_t nStatus)
{

  Mutex::Autolock Lock(mLockVideoFrame);

  switch(nStatus)
  {
    case DASH_ERROR_NOTIFY_BUFFERING_START:
    {
         if (isPaused())
         {
             QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                          "VideoNotifyCB:: Caching DASH_ERROR_NOTIFY_BUFFERING_START in paused state");
             mCachedBufferingNotification.mEvent = DASH_ERROR_NOTIFY_BUFFERING_START;
             mCachedBufferingNotification.mTrack = eTrackVideo;
             break;
         }
         QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "DashPlayer::DASHHTTPLiveSource::VideoNotifyCB -> DASH_ERROR_NOTIFY_BUFFERING_START");
            sp<AMessage> sourceRequest = new AMessage;
            if(sourceRequest != NULL)
            {
              sourceRequest->setInt32("what", kWhatBufferingStart);
              sourceRequest->setInt64("track", eTrackVideo);
         if (SourceNotifyCB(sourceRequest))
         {
              mCachedBufferingNotification.mEvent = 0;
              mCachedBufferingNotification.mTrack = eTrackNone;
           }
         }
         break;
    }
    case DASH_ERROR_NOTIFY_BUFFERING_END:
    {
         if (isPaused())
         {
            if (mCachedBufferingNotification.mEvent == DASH_ERROR_NOTIFY_BUFFERING_START)
            {
                QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                             "VideoNotifyCB:: Resetting Cached DASH_ERROR_NOTIFY_BUFFERING_START as its in paused state");
               mCachedBufferingNotification.mEvent = 0;
               mCachedBufferingNotification.mTrack = eTrackNone;
               break;
            }
            else if(mCachedBufferingNotification.mEvent == 0)
            {
               QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                            "VideoNotifyCB:: Caching DASH_ERROR_NOTIFY_BUFFERING_END in paused state");
               mCachedBufferingNotification.mEvent = DASH_ERROR_NOTIFY_BUFFERING_END;
               mCachedBufferingNotification.mTrack = eTrackVideo;
               break;
            }
         }
         QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "DashPlayer::DASHHTTPLiveSource::VideoNotifyCB -> DASH_ERROR_NOTIFY_BUFFERING_END");
            sp<AMessage> sourceRequest = new AMessage;
            if (sourceRequest != NULL)
            {
              sourceRequest->setInt32("what", kWhatBufferingEnd);
              sourceRequest->setInt64("track", eTrackVideo);
         if (SourceNotifyCB(sourceRequest))
         {
              mCachedBufferingNotification.mEvent = 0;
              mCachedBufferingNotification.mTrack = eTrackNone;
           }
         }
         break;
    }
    case OK:
          // how to get the metadata ifnormation to create a ABuffer packet here
          int64_t timeUs;
          int32_t vCodecConfig;
          if (accessUnit != NULL)
          {
            CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));
            accessUnit->meta()->findInt32("conf", &vCodecConfig);
          }

          //QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          //             "During Seek CodecConfigValue: %d",vCodecConfig);

          if (vCodecConfig == 0) // if its not a codec config (APS/PPS info)
          {
             mLastVideoTimeUs = timeUs;   // save the current video time stamp
             if (bSeeked == true)
             {
                if (mSeekedPositionUs == -1)
                {
                    mSeekedPositionUs = timeUs;
                    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                                 "mSeekedPositionUs From Video (%.2f) sec",(double)mSeekedPositionUs / 1E6);
                }
                bSeeked = false;
             }
          }
          else
          {
             //bCodecConfig = true;
             QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                          "Video Codec Config information");
          }

          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                                 "Received VideoBuf[%p]", accessUnit->data());

          // queue the access unit to video Another Packet source
          mVideoPacketSource->queueAccessUnit(accessUnit);
          mVideoPacketQueueSize++;

          break;
    case ERROR_END_OF_STREAM: // EOS detected
          // EOS case
          if ((mMediaInfo != NULL) && mMediaInfo->HasAudio() && mMediaInfo->HasText())
          {
             mEOS =  ((mMMIInterface != NULL) && mMMIInterface->m_bVidEos && mMMIInterface->m_bAudEos && mMMIInterface->m_bTextEos);// EOS reached
          }
          else if ((mMediaInfo != NULL) && mMediaInfo->HasAudio())
          {
             mEOS =  ((mMMIInterface != NULL) && mMMIInterface->m_bVidEos && mMMIInterface->m_bAudEos);// EOS reached
          }
          else
          {
             mEOS = ((mMMIInterface != NULL) && mMMIInterface->m_bVidEos && true);  //!Warning : no need to and wtih true
          }

          if (mEOS || mMediaInfo == NULL)
          {
             mFinalResult = ERROR_END_OF_STREAM;
          }

          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "Video EOS reached: %d",mEOS);
          mLastVideoTimeUs = 0;
          mVideoPacketSource->signalEOS(ERROR_END_OF_STREAM);
          break;
    case INFO_DISCONTINUITY: // codec change not handled
          break;
    case ERROR_IO:  // some error in the bit stream
          accessUnit = NULL;
          // signal error.. so that PB can be stopped for that stream
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Video Read ERROR_IO, queueing ERROR_END_OF_STREAM for the stream");
          mVideoPacketSource->signalEOS(ERROR_END_OF_STREAM); //ERROR_IO
          // there is an eror with in the steam and hence Forced EOS is queued
          bForcedVideoEOS = true;
          break;
     case ERROR_DRM_CANNOT_HANDLE: // ABORT has been issued
          accessUnit = NULL;
          // signal error.. so that PB can be stopped
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Video Read Error, queueing ERROR_DRM_CANNOT_HANDLE for both the streams %d",nStatus);
          mVideoPacketSource->signalEOS(ERROR_DRM_CANNOT_HANDLE); //ERROR_IO
          bForcedVideoEOS = true;
          if (mAudioPacketSource != NULL)
          {
              mAudioPacketSource->signalEOS(ERROR_DRM_CANNOT_HANDLE); //ERROR_IO
              bForcedAudioEOS = true;
          }
          if (mTextPacketSource != NULL)
          {
          //    mTextPacketSource->signalEOS(ERROR_END_OF_STREAM);
              bForcedTextEOS = true;
          }
          //As EOS is queued on both the streames we should update mFinalResult.
          mFinalResult = ERROR_DRM_CANNOT_HANDLE;
          break;
    case UNKNOWN_ERROR: // ABORT has been issued
          accessUnit = NULL;
          // signal error.. so that PB can be stopped
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Video Read Error, queueing UNKNOWN_ERROR for all the streams %d",nStatus);
          mVideoPacketSource->signalEOS(UNKNOWN_ERROR); //ERROR_IO
          bForcedVideoEOS = true;
          if (mAudioPacketSource != NULL)
          {
              mAudioPacketSource->signalEOS(UNKNOWN_ERROR); //ERROR_IO
              bForcedAudioEOS = true;
          }
          if (mTextPacketSource != NULL)
          {
              mTextPacketSource->signalEOS(UNKNOWN_ERROR);
              bForcedTextEOS = true;
          }
          //As EOS is queued on both the streames we should update mFinalResult.
          mFinalResult = UNKNOWN_ERROR;
          break;
    case INVALID_OPERATION:
    default:  // not sure when this case would arrive
          accessUnit = NULL;
          // signal error.. so that PB can be stopped
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Video Read Error, queueing ERROR_END_OF_STREAM for both the streams %d",nStatus);
          mVideoPacketSource->signalEOS(ERROR_END_OF_STREAM); //ERROR_IO
          bForcedVideoEOS = true;
          if (mAudioPacketSource != NULL)
          {
             mAudioPacketSource->signalEOS(ERROR_END_OF_STREAM); //ERROR_IO
             bForcedAudioEOS = true;
          }
          if (mTextPacketSource != NULL)
          {
             mTextPacketSource->signalEOS(ERROR_END_OF_STREAM);
             bForcedTextEOS = true;
          }

          //As EOS is queued on both the streames we should update mFinalResult.
          mFinalResult = ERROR_END_OF_STREAM;
          break;
          //-EWOULDBLOCK;
  } //switch
}

void DashPlayer::DASHHTTPLiveSource::TextNotifyCB(sp<ABuffer> accessUnit, status_t nStatus)
{

  Mutex::Autolock Lock(mLockTextFrame);

  switch(nStatus)
  {
    case DASH_ERROR_NOTIFY_BUFFERING_START:
    {
         QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "DashPlayer::DASHHTTPLiveSource::TextNotifyCB -> DASH_ERROR_NOTIFY_BUFFERING_START");
            sp<AMessage> sourceRequest = new AMessage;
            if(sourceRequest != NULL)
            {
              sourceRequest->setInt32("what", kWhatBufferingStart);
              sourceRequest->setInt64("track", eTrackText);
              (void)SourceNotifyCB(sourceRequest);
         }
         break;
    }
    case DASH_ERROR_NOTIFY_BUFFERING_END:
    {
         QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "DashPlayer::DASHHTTPLiveSource::TextNotifyCB -> DASH_ERROR_NOTIFY_BUFFERING_END");
            sp<AMessage> sourceRequest = new AMessage;
            if (sourceRequest != NULL)
            {
              sourceRequest->setInt32("what", kWhatBufferingEnd);
              sourceRequest->setInt64("track", eTrackText);
              (void)SourceNotifyCB(sourceRequest);
         }
         break;
    }
    case OK:

      // how to get the metadata ifnormation to create a ABuffer packet here
      int64_t timeUs;
      int32_t tCodecConfig;
      if (accessUnit != NULL)
      {
        CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));
        accessUnit->meta()->findInt32("conf", &tCodecConfig);

        //QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
        //             "During Seek CodecConfigValue: %d",vCodecConfig);

        if (tCodecConfig == 0) // if its not a codec config (APS/PPS info)
        {
          mLastTextTimeUs = timeUs;   // save the current video time stamp
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "DASHHTTPLiveSource::TextNotifyCB TS (%0.2f) sec",(double)timeUs / 1E6);
        }
        else
        {
          //tCodecConfig = true;
          QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "Text Codec Config information");
        }

        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                               "Received TextBuf[%p]", accessUnit->data());

        // queue the access unit to video Another Packet source
        mTextPacketSource->queueAccessUnit(accessUnit);
        mTextPacketQueueSize++;
      }
      break;
    case ERROR_END_OF_STREAM: // EOS detected
      // EOS case
      if ((mMediaInfo != NULL) && mMediaInfo->HasAudio() && mMediaInfo->HasVideo())
      {
         mEOS =  ((mMMIInterface != NULL) && mMMIInterface->m_bVidEos && mMMIInterface->m_bAudEos && mMMIInterface->m_bTextEos);// EOS reached
      }
      else if ((mMediaInfo != NULL) && mMediaInfo->HasAudio())
      {
         mEOS =  ((mMMIInterface != NULL) && mMMIInterface->m_bTextEos && mMMIInterface->m_bAudEos);// EOS reached
      }
      else if ((mMediaInfo != NULL) && mMediaInfo->HasVideo())
      {
         mEOS =  ((mMMIInterface != NULL) && mMMIInterface->m_bTextEos && mMMIInterface->m_bVidEos);// EOS reached
      }
      else
      {
        mEOS = ((mMMIInterface != NULL) && mMMIInterface->m_bTextEos && true);  //!Warning : no need to and wtih true
      }

      if (mEOS || mMediaInfo == NULL)
      {
          mFinalResult = ERROR_END_OF_STREAM;
      }

      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "Text EOS reached: %d",mEOS);
      mLastTextTimeUs = 0;
      mTextPacketSource->signalEOS(ERROR_END_OF_STREAM);
      break;
    case INFO_DISCONTINUITY: // codec change not handled
      break;
    case ERROR_IO:  // some error in the bit stream
      accessUnit = NULL;
      // signal error.. so that PB can be stopped for that stream
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Text Read ERROR_IO, queueing ERROR_END_OF_STREAM for the stream");
      mTextPacketSource->signalEOS(ERROR_END_OF_STREAM); //ERROR_IO
      // there is an eror with in the steam and hence Forced EOS is queued
      bForcedTextEOS = true;
      break;
    case ERROR_DRM_CANNOT_HANDLE: // ABORT has been issued
         accessUnit = NULL;
         // signal error.. so that PB can be stopped
         QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                        "Audio Read Error, queueing ERROR_END_OF_STREAM for both the streams %d",nStatus);
         mTextPacketSource->signalEOS(ERROR_END_OF_STREAM); //ERROR_IO
         bForcedTextEOS = true;
         if (mAudioPacketSource != NULL)
         {
             mVideoPacketSource->signalEOS(ERROR_DRM_CANNOT_HANDLE); //ERROR_IO
             bForcedVideoEOS = true;
         }
         if (mAudioPacketSource != NULL)
         {
             mAudioPacketSource->signalEOS(ERROR_DRM_CANNOT_HANDLE);
             bForcedAudioEOS = true;
         }
         //As EOS is queued on both the streames we should update mFinalResult.
         mFinalResult = ERROR_DRM_CANNOT_HANDLE;
         break;

    case UNKNOWN_ERROR: // ABORT has been issued
          accessUnit = NULL;
          // signal error.. so that PB can be stopped
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Text Read Error, queueing UNKNOWN_ERROR for all the streams %d",nStatus);
          mTextPacketSource->signalEOS(UNKNOWN_ERROR); //ERROR_IO
          bForcedTextEOS = true;
          if (mAudioPacketSource != NULL)
          {
              mAudioPacketSource->signalEOS(UNKNOWN_ERROR); //ERROR_IO
              bForcedAudioEOS = true;
          }
          if (mVideoPacketSource != NULL)
          {
              mVideoPacketSource->signalEOS(UNKNOWN_ERROR);
              bForcedVideoEOS = true;
          }
          //As EOS is queued on both the streames we should update mFinalResult.
          mFinalResult = UNKNOWN_ERROR;
          break;
    case INVALID_OPERATION:
    default:  // not sure when this case would arrive
      accessUnit = NULL;
      // signal error.. so that PB can be stopped
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Text Read Error, queueing ERROR_END_OF_STREAM for both the streams %d",nStatus);
      mTextPacketSource->signalEOS(ERROR_END_OF_STREAM); //ERROR_IO
      bForcedTextEOS = true;
      if (mAudioPacketSource != NULL)
      {
         mAudioPacketSource->signalEOS(ERROR_END_OF_STREAM); //ERROR_IO
         bForcedAudioEOS = true;
      }
      if (mVideoPacketSource != NULL)
      {
         mVideoPacketSource->signalEOS(ERROR_END_OF_STREAM); //ERROR_IO
         bForcedVideoEOS = true;
      }
      //As EOS is queued on both the streames we should update mFinalResult.
      mFinalResult = ERROR_END_OF_STREAM;
      break;
     //-EWOULDBLOCK;
  } //switch
}

void DashPlayer::DASHHTTPLiveSource::QOENotifyCB(sp<AMessage> amessage)
{
    if (mQOEEventNotify != NULL)
    {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_LOW,
                        "DASHHTTPLiveSource::QOENotifyCB");
        mQOEEventNotify->setMessage("QOEData",amessage);
        mQOEEventNotify->post();
    }
}

bool DashPlayer::DASHHTTPLiveSource::SourceNotifyCB(sp<AMessage> amessage)
{
  bool bOK = false;

  if (mNotifySourceStatus != NULL)
  {
    char srName[] = "source-request00";
    (void)std_strlprintf(srName, sizeof(srName), "source-request%d%d", mSRid/10, mSRid%10);
    mSRid = (mSRid + 1) % SRMax;
    mNotifySourceStatus->setMessage(srName, amessage);
    mNotifySourceStatus->post();
    bOK = true;
  }
  return bOK;
}

void DashPlayer::DASHHTTPLiveSource::setFinalResult(status_t nStatus)
{
   mFinalResult = nStatus;
}

status_t DashPlayer::DASHHTTPLiveSource::setupSourceData(const sp<AMessage> &msg, int iTrack)
{
    if (msg != NULL)
    {
      if (iTrack == eTrackText)
      {
         msg->setInt64("timer-started",0);
         if (0 != MM_Timer_CreateEx(0, timedTextTimerCB, (AMessage *)&msg, &mTimedTextTimerHandle))
         {
           QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                        "MM_Timer Creation failed for Text Track, hence Text track won't be rendered");
           return (status_t)FAILED_TRANSACTION;
         }
         else
         {  // timer created successfully, post to pull timed text sample
            QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                          "mTimedTextTimerHandle 0x%x",(unsigned)(intptr_t)mTimedTextTimerHandle);
            msg->post();
         }
      }
      else if (iTrack == eTrackAll)
      {
         mNotifySourceStatus = msg;
      }
      else if (iTrack == -1)
      {
         mQOEEventNotify = msg;
      }
    }
    return OK;
}

status_t DashPlayer::DASHHTTPLiveSource::postNextTextSample(sp<ABuffer> accessUnit,
                                                          const sp<AMessage> &msg,
                                                          int iTrack)
{
   status_t nStatus = OK;
   int nTimedOut = 0;

   CHECK(iTrack == kText);

   if (accessUnit == NULL || msg == NULL)
   {
     nStatus = BAD_VALUE;
     return nStatus;
   }

   int32_t tCodecConfig;
   int64_t mCurrentTimestamp = 0;
   int64_t mLocalSeekPos = 0;

   int64_t mRealTimePrefetch = DASH_TIMED_TEXT_NOTIFICATION_PREFETCH;
   int64_t mAccumlatedDurationUs = 0;

   accessUnit->meta()->findInt32("conf", &tCodecConfig);
   accessUnit->meta()->findInt32("duration", &mPrevTimedTextDuration);
   accessUnit->meta()->findInt64("timeUs", &mCurrentTimestamp);
   // if its a codec config, no need to start a timer, rather post a message to pull the sample
   // and pass that to APP.
   if (tCodecConfig)
   {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "postNextTextSample notifyNextTextSample: Timed text codec config frame, post a new frame now");
      msg->post(); // post to retrive the next frame now itself
      return nStatus;
   }
   msg->findInt64("timer-started",&mTimedTextTimerStarted);
   mPostTextFrameNotify = msg;

   // if not codec config then we might have to look into the accessUnit for duration and
   // start the timer for the next frame
   if ((mTimedTextTimerStarted == 0) && (mTimedTextTimerHandle != NULL))
   {
      mLastTextSampleDuration = mPrevTimedTextDuration;
      mLastTextSampleTSNotified = mCurrentTimestamp;

      mAccumlatedDurationUs = mCurrentTimestamp + (mPrevTimedTextDuration * 1000); //micro sec

      QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "postNextTextSample DASH_TIMED_TEXT_NOTIFICATION_PREFETCH %.2f sec, mAccumlatedDurationUs %.2f ses",
                    (double)mRealTimePrefetch / 1E6,(double)mAccumlatedDurationUs / 1E6);

      if (mCurrentRenderingPosition < 0)
      {
        mCurrentRenderingPosition = 0;
      }
      mRealTimePrefetch +=  mCurrentRenderingPosition;

      if ((mSeekedPositionUs >= 0) && (mCurrentRenderingPosition < mSeekedPositionUs))
      {
          mLocalSeekPos = mSeekedPositionUs;
          mRealTimePrefetch = DASH_TIMED_TEXT_NOTIFICATION_PREFETCH;
      }

      if (mAccumlatedDurationUs <= (mRealTimePrefetch + mLocalSeekPos))
      {
        QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "postNextTextSample -> Immediately mAccumlatedDurationUs %.2f sec, mSeekedPositionUs (%.2f sec) mCurrentRenderingPosition (%.2f) sec",
                     (double)mAccumlatedDurationUs / 1E6,(double)mSeekedPositionUs / 1E6,(double)mCurrentRenderingPosition/1E6);
        msg->post();
        return nStatus;
      }
      else
      {
         if ( (mSeekedPositionUs >= 0) && (mCurrentRenderingPosition < mSeekedPositionUs))
         {
            nTimedOut = (int)(mAccumlatedDurationUs - (mSeekedPositionUs + DASH_TIMED_TEXT_NOTIFICATION_PREFETCH));
         }
         else
         {
           nTimedOut = (int)(mAccumlatedDurationUs - mRealTimePrefetch);
           mSeekedPositionUs = -1;
         }
         QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "postNextTextSample nTimedOut %d ms, mRealTimePrefetch %lld ms, mCurrentRenderingPosition %.2f ",
                       nTimedOut / 1000,
                       mRealTimePrefetch / 1000,
                       (double)mCurrentRenderingPosition/1E6);

      }

     // make sure Timeout is greater than oe equal to 0 then start the timer
     if (nTimedOut >= 0)
     {
       MM_Timer_Stop(mTimedTextTimerHandle); // !Warning : not sure to have this here, for now, let it be here
       if (nTimedOut == 0)
       {
          // just an optimization where in when timeout is 0,
          // do not start the timer insted do a post here itself.
          msg->post();
       }
       else
       {
         MM_Timer_Start(mTimedTextTimerHandle, (nTimedOut/1000));
         msg->setInt64("timer-started",1);
       }
       nStatus = OK;
     }
   }
   else
   {
     QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "May be a Stale Request, no need to process ");
   }

   //TODO: need to see what value to set in nStatus, when timer is already running
   return nStatus;
}

void DashPlayer::DASHHTTPLiveSource::timedTextTimerCB(void *arg)
{
  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "timedTextTimerCB, Timer Expired");

  sp<AMessage> *msg = (sp<AMessage> *)arg;
  if (*msg != NULL) {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "timedTextTimerCB call back called");
     (*msg)->setInt64("timer-started",0);
     (*msg)->post();
  }
}

void DashPlayer::DASHHTTPLiveSource::PrepareDoneNotifyCB(status_t status)
{
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "PrepareDone true");
    mPrepareDone = status;
}

bool DashPlayer::DASHHTTPLiveSource::isMiddleOfPlayback()
{
    bool nRet = false;
    if (mMediaInfo != NULL)
    {
      if (mMediaInfo->HasVideo() && mMediaInfo->HasAudio() && mMediaInfo->HasText())
      {
        if (mLastAudioTimeUs != -1 && mLastVideoTimeUs != -1 && mLastTextTimeUs != -1)
        {
          nRet = true;
        }
      }
      else if (mMediaInfo->HasVideo() && mMediaInfo->HasAudio())
      {
        if (mLastAudioTimeUs != -1 && mLastVideoTimeUs != -1)
        {
          nRet = true;
        }
      }
      else if (mMediaInfo->HasVideo() && mMediaInfo->HasText())
      {
        if (mLastVideoTimeUs != -1 && mLastTextTimeUs != -1)
        {
          nRet = true;
        }
      }
      else if (mMediaInfo->HasAudio() && mMediaInfo->HasText())
      {
        if (mLastAudioTimeUs != -1 && mLastTextTimeUs != -1)
        {
          nRet = true;
        }
      }
      else if (mMediaInfo->HasVideo())
      {
        if (mLastVideoTimeUs != -1)
        {
           nRet = true;
        }
      }
      else if (mMediaInfo->HasAudio())
      {
        if (mLastAudioTimeUs != -1)
        {
           nRet = true;
        }
      }
    }
    return nRet;
}

void DashPlayer::DASHHTTPLiveSource::notifyRenderingPosition(int64_t nRenderingTS)
{
   mCurrentRenderingPosition = nRenderingTS;
}

status_t DashPlayer::DASHHTTPLiveSource::pause()
{
   QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "Pause called from DashPlayer onto DASH Source");
   bPaused = true;

   status_t status = BAD_VALUE;
   if (mMMIInterface != NULL)
   {
     status = mMMIInterface->pause();
   }
   if (status != OK && status != WOULD_BLOCK)
   {
     QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                   "DASHHTTPLiveSource::pause() Failed, status %d",status);
   }

   // in case of text, we have to stop the timer so that there are no issues.
   if (mTimedTextTimerHandle != NULL)
   {
      MM_Timer_Stop(mTimedTextTimerHandle);
      if (mPostTextFrameNotify != NULL)
      {
        mPostTextFrameNotify->setInt64("timer-started",0);
      }
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "DashPlayer::DASHHTTPLiveSource::pause() -> Timer stopped, will be started again during resume");
   }

   return status;
}

status_t DashPlayer::DASHHTTPLiveSource::resume()
{
   QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "Resume called from DashPlayer onto DASH Source");
   int64_t nTimedOut = 0;
   int64_t mRealTimePrefetch = DASH_TIMED_TEXT_NOTIFICATION_PREFETCH;
   int64_t mTsPlusDurationUs = 0;

   status_t status = BAD_VALUE;
   if (mMMIInterface != NULL)
   {
     status = mMMIInterface->resume();
   }
   if (status != OK && status != WOULD_BLOCK)
   {
     QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                   "DASHHTTPLiveSource::resume() Failed, status %d",status);
   }

// there is a need start timed text timer here
   if (mPostTextFrameNotify != NULL)
   {
      mPostTextFrameNotify->findInt64("timer-started",&mTimedTextTimerStarted);
   }
   if (bPaused)
   {
      bPaused = false;
      if (mCachedBufferingNotification.mEvent != 0)
      {
         if (mAudioPacketSource != NULL && mCachedBufferingNotification.mTrack == eTrackAudio)
         {
            AudioNotifyCB(NULL,mCachedBufferingNotification.mEvent);
         }
         else if (mVideoPacketSource != NULL && mCachedBufferingNotification.mTrack == eTrackVideo )
         {
            VideoNotifyCB(NULL,mCachedBufferingNotification.mEvent);
         }
      }

      if ( (mPostTextFrameNotify != NULL) &&
           (mTimedTextTimerStarted == 0)  &&
           (mTimedTextTimerHandle != NULL))
      {
         mTsPlusDurationUs = mLastTextSampleTSNotified + (mLastTextSampleDuration * 1000); //micro sec

         QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "postNextTextSample DASH_TIMED_TEXT_NOTIFICATION_PREFETCH %.2f Us, mTsPlusDurationUs %.2f Us",
                        (double)mRealTimePrefetch / 1E6,(double)mTsPlusDurationUs / 1E6);

         if (mCurrentRenderingPosition < 0)
         {
           mCurrentRenderingPosition = 0;
         }
         mRealTimePrefetch +=  mCurrentRenderingPosition;

         nTimedOut = (mTsPlusDurationUs - mRealTimePrefetch);

         QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "postNextTextSample nTimedOut %lld ms, mRealTimePrefetch %lld ms, mCurrentRenderingPosition %.2f Us",
                       nTimedOut / 1000,
                       mRealTimePrefetch / 1000,
                       (double)mCurrentRenderingPosition/1E6);

        // make sure Timeout is greater than oe equal to 0 then start the timer
        if ((nTimedOut >= 0) && (mPostTextFrameNotify != NULL))
        {
           MM_Timer_Stop(mTimedTextTimerHandle); // !Warning : not sure to have this here, for now, let it be here
           if (nTimedOut == 0)
           {
             //optimization for not starting the timer for 0 timeout
             mPostTextFrameNotify->post();
           }
           else
           {
              MM_Timer_Start(mTimedTextTimerHandle, (int)(nTimedOut/1000));
              mPostTextFrameNotify->setInt64("timer-started",1);
           }
        }
      }
   }
   else
   {
     QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                  "DashPlayer::DASHHTTPLiveSource::resume not possible at this stage");
   }

   return status;
}

bool DashPlayer::DASHHTTPLiveSource::allowDequeueAccessUnit(int iTrack)
{
   bool bReturn = true;
   int64_t nNextSampleTime = -1;

   switch (iTrack)
   {
       case eTrackAudio :
       {
           if (isMiddleOfPlayback()                                       &&
               (mMMIInterface != NULL && mMMIInterface->getBufferingStatus()) &&
               ((mMediaInfo != NULL) && mMediaInfo->HasVideo()) && (mLastAudioReadTimeUs > mLastVideoReadTimeUs) &&
               (!bForcedAudioEOS))
           {
               QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                            "DASHHTTPLiveSource::allowDequeueAccessUnit -> Hold Audio as Audio/Video is in Buffering ..");
               return false;
           }
           break;
       }
       case eTrackVideo :
       {
           //Make sure all B-frames associated with the last given reference (closeset to last given audio sample)
           //are also not held
           if (mVideoPacketSource != NULL)
           {
             (void)mVideoPacketSource->nextBufferTime(&nNextSampleTime);
           }
           if (isMiddleOfPlayback()                                            &&
               (mMMIInterface != NULL && mMMIInterface->getBufferingStatus())  &&
               (!bForcedVideoEOS)                                              &&
               ((mMediaInfo != NULL) && mMediaInfo->HasAudio())                &&
                ((mLastVideoReadTimeUs > mLastAudioReadTimeUs) && ((nNextSampleTime > 0) && (nNextSampleTime > mLastAudioReadTimeUs))))
           {
               QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                            "DASHHTTPLiveSource::allowDequeueAccessUnit -> Hold Video as Audio/Video is in Buffering ..");
               return false;
           }
           break;
       }
       case eTrackText :
       {
           if ( (isMiddleOfPlayback()                                           &&
               (mMMIInterface != NULL && mMMIInterface->getBufferingStatus()) &&
               ((((mMediaInfo != NULL) && mMediaInfo->HasAudio()) && mLastTextReadTimeUs > mLastAudioReadTimeUs)   ||
                (((mMediaInfo != NULL) && mMediaInfo->HasVideo()) && mLastTextReadTimeUs > mLastVideoReadTimeUs))) &&
                (!bForcedTextEOS))
           {
               QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                            "DASHHTTPLiveSource::allowDequeueAccessUnit -> Hold Text as Audio/Video is in Buffering ..");
               return false;
           }
           break;
       }
       default:
           QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                        "DASHHTTPLiveSource::dequeueAccessUnit -> Invalid Track ..");
           return false;
           break;
   }
   QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_LOW,
                 "DASHHTTPLiveSource::dequeueAccessUnit -> valid Track Allow access.. %d",iTrack);
   return bReturn;
}

}  // namespace android

