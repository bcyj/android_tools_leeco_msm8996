/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#ifndef WFD_SOURCE_H_

#define WFD_SOURCE_H_

#include <NuPlayer.h>
#include <NuPlayerSource.h>
#include <fcntl.h>
#include <stdlib.h>

#include "AnotherPacketSource.h"
#include "OMX_Core.h"
#include "OMX_Types.h"
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/MediaErrors.h>
#include "filesource.h"
#include "filesourcetypes.h"
#include "OMX_Audio.h"
#include "OMX_Video.h"
#include "RTPStreamPort.h"




namespace android {
struct MediaExtractor;
struct DataSource;


struct NuPlayer::WFDSource : public NuPlayer::Source {


    WFDSource(const char *urlPath                          ,
              const KeyedVector<String8, String8> *headers ,
              OMX_U32 &nReturn                             ,
              bool bValidUid = false                       ,
              uid_t nUid = 0);

    virtual status_t feedMoreTSData();
    virtual sp<MetaData> getFormat(int iMedia);
    virtual status_t getDuration(int64_t *pDurationUs);
    virtual status_t seekTo(int64_t nSeekTimeUs);
    virtual void start();
    virtual bool isSeekable();
    virtual status_t dequeueAccessUnit(int iMedia, sp<ABuffer> *accessUnit);
    virtual void stop();
    virtual status_t getNewSeekTime(int64_t *pNewSeekTime);
    virtual void notifyRenderingPosition(int64_t nRenderingTS);
    virtual void pause();
    virtual void resume();
    void setStatus(FileSourceCallBackStatus status);



protected:
    virtual ~WFDSource();

private:
    bool mEOS;
    bool mAudioEOS;
    bool mVideoEOS;
    bool mResponseReceived;
    bool mResponseAcknoledge;
    FileSourceCallBackStatus mOpenFileSourceStatus;

    FileSource *m_pFileSource;
    RTPStreamPort *m_pRTPStreamPort;
    int32 mAudioTrackId;
    int32 mVideoTrackId;
    int32_t mAudioTimescale;
    int32_t mVideoTimescale;
    int32   mTracksCount;
    size_t mVideoMaxBufferSize;
    size_t mAudioMaxBufferSize;
    FILE* fp1;
    FILE* fp2;

    bool mPauseIndication;
/* Another packet source queue */
    sp<AnotherPacketSource> mAudioPacketSource;
    sp<AnotherPacketSource> mVideoPacketSource;

    status_t mFinalResult;

    // audio and video metadata
    sp<MetaData> mVideoMetaData;
    sp<MetaData> mAudioMetaData;

    // flushes the locat audio/video packet source queues
    status_t flush(bool audio);
    status_t createTracks();
    static sp<MediaExtractor> CreateExtractor(const sp<DataSource> &source, const char *mime);
    status_t fillAudioPacketSource();
    status_t fillVideoPacketSource();

    status_t getAudioStream();
    status_t getVideoStream();
    static void fileSourceEventCallback(FileSourceCallBackStatus status, void *pClientData);
    status_t checkThenStart();

    void writeToFile(bool audio, void* buffer, size_t size);
    DISALLOW_EVIL_CONSTRUCTORS(WFDSource);

};

}  // namespace android

#endif  // WFD_SOURCE_H_
