/************************************************************************* */
/**
 * DashExtractor.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */

#ifndef DASH_EXTRACTOR_H_

#define DASH_EXTRACTOR_H_

/* =======================================================================
**               Include files for DashExtractor.h
** ======================================================================= */

#include "DashExtractorAdaptationLayer.h"
#include "ExtendedMediaExtractor.h"

namespace android {

class DashExtractor : public ImplMediaExtractor {
public:

    DashExtractor();

    //static sp<MediaExtractor> Create(
      //      const sp<DataSource> &source, const char *mime = NULL);

    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);

    virtual sp<MetaData> getTrackMetaData(
            size_t index, uint32_t flags = 0);

    //// Return container specific meta-data. The default implementation
    //// returns an empty metadata object.
    //virtual sp<MetaData> getMetaData();

    //// If subclasses do _not_ override this, the default is
    //// CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_SEEK | CAN_PAUSE
    //virtual uint32_t flags() const;

    //// for DRM
    //void setDrmFlag(bool flag) {
    //    mIsDrm = flag;
    //};
    //bool getDrmFlag() {
    //    return mIsDrm;
    //}

    //virtual char* getDrmTrackInfo(size_t trackID, int *len) {
    //    return NULL;
    //}
    virtual void SetSelectedTrackID(int id);
    virtual int32_t getTrackId(int trackIndex);
    virtual int GetBufferedDuration(unsigned int nTrackID, long long  nBytes,
                                      unsigned long long *pDuration);
    virtual int GetBufferedDuration(unsigned int nTrackID, unsigned long long *pDuration);

    virtual bool sendCommand(int nCmdId);
    virtual bool setParameter(int nParamId, void* data);
    virtual bool getParameter(int nParamId, void* data);

    static void EventHandler(int evntCode, int evntData, void *cbData, const Parcel *obj);


protected:
    virtual ~DashExtractor();

private:
  sp<DEALInterface> mDEAL;
  sp<ExtractorListener> mListener;

};

}  //namespace android

#endif // DASH_EXTRACTOR_H_
