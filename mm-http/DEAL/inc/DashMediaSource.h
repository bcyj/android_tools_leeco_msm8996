/************************************************************************* */
/**
 * DashMediaSource.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */

#ifndef DASH_PACKET_SOURCE_H_

#define DASH_PACKET_SOURCE_H_

/* =======================================================================
**               Include files for DashMediaSource.h
** ======================================================================= */

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/MediaSource.h>
#include <utils/threads.h>
#include <utils/List.h>


namespace android {

struct ABuffer;
struct AMessage;
class DEALInterface;

class DashMediaSource : public MediaSource {
public:
    DashMediaSource(const int32_t portIndex, const int32_t trackId, const sp<DEALInterface> &deal, const sp<MetaData> &meta);

    virtual status_t start(MetaData */*params = NULL*/) { return OK; }
    virtual status_t stop() { return OK; }

    virtual status_t read(
            MediaBuffer **buffer, const ReadOptions *options = NULL);

    void queueAccessUnit(const sp<ABuffer> &buffer);

    void queueDiscontinuity();

    void signalEOS(status_t result);

    status_t pause();
    status_t resume();
    status_t flush();

    status_t dequeueAccessUnit(sp<ABuffer> *buffer);

    uint32_t getQueueSize();

    void setFormat(const sp<MetaData> &meta);
    virtual sp<MetaData> getFormat();

protected:
    virtual ~DashMediaSource();

private:
    Mutex mLock;

    int32_t mPortIndex;
    int32_t mTrackId;
    List<sp<ABuffer> > mBuffers;
    status_t mEOSResult;
    bool mPaused;
    sp<MetaData> mFormat;
    sp<DEALInterface> mDEAL;

    //bool wasFormatChange(int32_t discontinuityType) const;

    DISALLOW_EVIL_CONSTRUCTORS(DashMediaSource);
};


}  // namespace android

#endif  // DASH_PACKET_SOURCE_H_
