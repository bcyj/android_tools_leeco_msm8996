/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_MDP_SOURCE_H
#define _VTEST_MDP_SOURCE_H

#include "OMX_Core.h"
#include "vtest_ISource.h"

namespace vtest {

/**
* @brief Delivers YUV data from the MDP for testing WFD.
*
*/
class MdpSource : virtual public ISource {

public:
    /**
     * @brief constructor
     *
     * @param pConfig - configuration struct
     * @param aSemaphore - sync semaphore
     * @param nFrameWidth - screen width, for now frame width
     * @param nFrameHeight - screen hight, for now frame height
     */
    MdpSource(CodecConfigType *pConfig);
    ~MdpSource();

    virtual PortBufferCapability GetBufferRequirements(OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE Configure(CodecConfigType *pConfig,
            BufferManager *pBufManager, ISource *pSource, ISource *pSink);
    virtual OMX_ERRORTYPE SetBuffer(
            BufferInfo *pBuffers, ISource *pSource);
private:
    virtual OMX_ERRORTYPE ThreadRun(OMX_PTR pThreadData);
    void GetFrameBufferDev(char fb_dev[128]);

private:
    static const OMX_S32 MAX_BUF_COUNT = 32;

    OMX_S32 m_nFrames;
    OMX_S32 m_nFramerate;
    OMX_S32 m_nFrameWidth;
    OMX_S32 m_nFrameHeight;
    OMX_S32 m_nDVSXOffset;
    OMX_S32 m_nDVSYOffset;
    OMX_S32 m_pFbPanel;   // frame-buffer device
    OMX_S32 m_nHeaderCount;
    OMX_BUFFERHEADERTYPE *m_pHeaders[MAX_BUF_COUNT];  // buffer list
};

} // namespace vtest

#endif // #ifndef _VTEST_MDP_SOURCE_H
