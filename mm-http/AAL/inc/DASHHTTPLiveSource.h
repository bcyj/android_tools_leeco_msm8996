/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef DASH_HTTP_LIVE_SOURCE_H_

#define DASH_HTTP_LIVE_SOURCE_H_

#include <DashPlayer.h>
#include <DashPlayerSource.h>


#include "DASHMMIInterface.h"
#include "DASHMMIMediaInfo.h"
#include "DASHMMIMediaSource.h"
#include "DashPacketSource.h"

#define AUDIO_PACKET_QUEUE_MAX_SIZE 4
#define VIDEO_PACKET_QUEUE_MAX_SIZE 4
#define TEXT_PACKET_QUEUE_MAX_SIZE  4

namespace android {

struct DashPlayer::DASHHTTPLiveSource : public DashPlayer::Source {

    DASHHTTPLiveSource(const char *urlPath                          ,
                       const KeyedVector<String8, String8> *headers ,
                       OMX_U32 &nReturn                             ,
                       bool bValidUid = false                       ,
                       uid_t nUid = 0);

    virtual status_t feedMoreTSData();
    virtual sp<MetaData> getFormat(int pTrack);
    virtual status_t getDuration(int64_t *pDurationUs);
    virtual status_t seekTo(int64_t nSeekTimeUs);
    virtual void start();
    virtual bool isSeekable();
    virtual status_t dequeueAccessUnit(int pTrack, sp<ABuffer> *accessUnit);
    virtual void stop();
    virtual status_t prepareAsync();
    virtual status_t isPrepareDone();
    virtual status_t getParameter(int key, void **data, size_t *size);
    virtual status_t setParameter(int key, void *data, size_t size);
    virtual status_t setupSourceData(const sp<AMessage> &msg, int iTrack);
    virtual status_t postNextTextSample(sp<ABuffer> accessUnit, const sp<AMessage> &msg, int iTrack);
    virtual status_t getMediaPresence(bool &audio, bool &video, bool &text);
    virtual void notifyRenderingPosition(int64_t nRenderingTS);
    virtual status_t pause();
    virtual status_t resume();

    virtual status_t getRepositionRange(uint64_t* pMin, uint64_t* pMax, uint64_t* pMaxDepth);
    virtual bool isPlaybackDiscontinued();
    virtual status_t getTrackInfo(Parcel *reply);

    static void timedTextTimerCB(void *arg);

    enum eDASHSourceTrackType
    {
      eTrackVideo = 0,
      eTrackAudio    ,
      eTrackText     ,
      eTrackAll      ,
      eTrackNone     ,
    };

    enum eDASHSourceNotifications
    {
       DASH_ERROR_NOTIFY_BUFFERING_START = 1,
       DASH_ERROR_NOTIFY_BUFFERING_END      ,
       DASH_SOURCE_NOTIFY_RESUME            ,
       DASH_ERROR_LAST = DASH_SOURCE_NOTIFY_RESUME,
    };


protected:
    virtual ~DASHHTTPLiveSource();

private:
    bool mEOS;
    int32_t mSRid;
/* interface classes to DASH MMI Device */
    sp<DASHMMIInterface>   mMMIInterface;      /* Interface to DASH MMI and handles all the open/close/load/Response/Event handling */
    sp<DASHMMIMediaInfo>   mMediaInfo;         /* provides the information about the media */

    /* Audio Media source responsible for getting/creating the AU's and queueing/De-queueing frames to/from Another packet source */
    sp<DASHMMIMediaSource> mAudioMediaSource;
    sp<DASHMMIMediaSource> mVideoMediaSource;
    sp<DASHMMIMediaSource> mTextMediaSource;

/* Dash packet source queue */
    sp<DashPacketSource> mAudioPacketSource;
    sp<DashPacketSource> mVideoPacketSource;
    sp<DashPacketSource> mTextPacketSource;

    status_t mFinalResult;

    // stores the last read (from source) audio and video timestamp
    int64_t mLastAudioTimeUs;
    int64_t mLastVideoTimeUs;
    int64_t mLastTextTimeUs;

    // stores the last read (by decoder) audio and video timestamp
    int64_t mLastAudioReadTimeUs;
    int64_t mLastVideoReadTimeUs;
    int64_t mLastTextReadTimeUs;

    sp<ABuffer> mVideoConfigBuffer;

    // audio and video metadata
    sp<MetaData> mVideoMetaData;
    sp<MetaData> mAudioMetaData;
    sp<MetaData> mTextMetaData;

    // To store uri and headers
    String8 mUri;
    KeyedVector<String8, String8> mUriHeaders;

    /* in case of error reported from DASH, we should queue EOS and that will be a forced EOS,
         after that there should not be any fill this buffer to DASH
    */
    bool bForcedAudioEOS;
    bool bForcedVideoEOS;
    bool bForcedTextEOS;

    // flushes the locat audio/video packet source queues
    status_t flush(int pTrack);

    // queues the buffer in to packet source queue
    status_t queueBuffer(int pTrack, bool &bCodecConfig);

    status_t fillAudioPacketSource();
    status_t fillVideoPacketSource();
    status_t fillTextPacketSource();

    //posts fill this buffer to DASHMMI and get the frame from DASH
    status_t getAudioStream(bool &bCodecConfig);
    status_t getVideoStream(bool &bCodecConfig);
    status_t getTextStream(bool &bCodecConfig);

    //call backs from the DASH Thread
    Mutex mLockAudioFrame;
    Mutex mLockVideoFrame;
    Mutex mLockTextFrame;

    int mAudioPacketQueueSize;
    int mVideoPacketQueueSize;
    int mTextPacketQueueSize;
    status_t mPrepareDone;

    bool bPaused;
    int64_t mCurrentRenderingPosition; // keeps track of current rendering position
    int64_t mLastTextSampleTSNotified; // keeps track of Last notified TT TimeStamp
    int32_t mLastTextSampleDuration; // keeps track of last notified TT Duration

    int32_t mPrevTimedTextDuration;
    MM_HANDLE mTimedTextTimerHandle;
    int64_t mTimedTextTimerStarted;
    int64_t mSeekedPositionUs;
    bool bSeeked;

    sp<AMessage> mNotifySourceStatus;
    sp<AMessage> mPostTextFrameNotify;
    sp<AMessage> mQOEEventNotify;

    typedef struct BufferingEventNotification {
        status_t  mEvent;
        int       mTrack;
    }T_stBufferingEventNotification;

    T_stBufferingEventNotification mCachedBufferingNotification;

   struct BufInfoQ : public RefBase
   {
     public:
       BufInfoQ(const int32_t trackID, const uint32_t maxBufSz, const uint32_t maxBufCt)
       {
         track = trackID;
         maxBufSize = maxBufSz;
         maxBufCnt = maxBufCt;
       }

       virtual ~BufInfoQ() {;}
       int32_t getReusableBufIndex();
       Vector<sp<ABuffer> > bufQ;

     private:
       int32_t track;
       uint32_t maxBufSize;
       uint32_t maxBufCnt;

    DISALLOW_EVIL_CONSTRUCTORS(BufInfoQ);
   };

   sp<BufInfoQ> mVideoBufQ;
   sp<BufInfoQ> mAudioBufQ;
   sp<BufInfoQ> mTextBufQ;

public:
    void AudioNotifyCB(sp<ABuffer> accessUnit, status_t nStatus);
    void VideoNotifyCB(sp<ABuffer> accessUnit, status_t nStatus);
    void TextNotifyCB(sp<ABuffer> accessUnit, status_t nStatus);
    void QOENotifyCB(sp<AMessage> amessage);
    bool SourceNotifyCB(sp<AMessage> amessage);
    void setFinalResult(status_t nStatus);

    void PrepareDoneNotifyCB(status_t status);
    bool isMiddleOfPlayback();
    bool isPaused() {return bPaused;}
    bool allowDequeueAccessUnit(int iTrack);

    DISALLOW_EVIL_CONSTRUCTORS(DASHHTTPLiveSource);
};

}  // namespace android

#endif  // DASH_HTTP_LIVE_SOURCE_H_
