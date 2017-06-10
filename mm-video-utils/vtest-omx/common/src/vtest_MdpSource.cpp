/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#include <fcntl.h>
#include <linux/msm_mdp.h>
#include <linux/videodev2.h>
#include <media/msm_media_info.h>
#include "vt_semaphore.h"
#include "vtest_ComDef.h"
#include "vtest_Debug.h"
#include "vtest_Thread.h"
#include "vtest_SignalQueue.h"
#include "vtest_Sleeper.h"
#include "vtest_Time.h"
#include "vtest_MdpSource.h"
#include "vtest_File.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_MDP_SOURCE"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MdpSource::MdpSource(CodecConfigType *pConfig)
    : ISource(),
      m_nFrames(0),
      m_nFramerate(0),
      m_nFrameWidth(0),
      m_nFrameHeight(0),
      m_nDVSXOffset(0),
      m_nDVSYOffset(0),
      m_pFbPanel(0),
      m_nHeaderCount(0) {

    char fb_dev[128];
    struct fb_var_screeninfo sVar;

    snprintf(m_pName, PROPERTY_FILENAME_MAX, "MdpSource");
    VTEST_MSG_LOW("%s created", Name());

    // open device
    GetFrameBufferDev(fb_dev);
    m_pFbPanel = open(fb_dev, O_RDWR);
    if (m_pFbPanel < 0) {
        VTEST_MSG_ERROR("fb_panel open failed (%s)", fb_dev);
        return;
    }
    VTEST_MSG_LOW("fb_panel open (%s)", fb_dev);

    // get scren resolution
    if (ioctl(m_pFbPanel, FBIOGET_VSCREENINFO, &sVar) < 0) {
        VTEST_MSG_ERROR("error FBIOGET_VSCREENINFO");
        return;
    }

    m_nFrameWidth = sVar.xres = sVar.xres_virtual = pConfig->nFrameWidth;
    m_nFrameHeight = sVar.yres = sVar.yres_virtual = pConfig->nFrameHeight;
    if (ioctl(m_pFbPanel, FBIOPUT_VSCREENINFO, &sVar) < 0) {
        VTEST_MSG_ERROR("error FBIOPUT_VSCREENINFO");
        return;
    }

    VTEST_MSG_LOW("MpdSource: screen res %u x %u", sVar.xres, sVar.yres);
    VTEST_MSG_LOW("MpdSource: screen vir-res %u x %u",
            sVar.xres_virtual, sVar.yres_virtual);
    if (m_nFrameWidth <= 0 || m_nFrameHeight <= 0) {
       VTEST_MSG_ERROR("frame size invalid (%u x %u)",
                (unsigned int)m_nFrameWidth, (unsigned int)m_nFrameHeight);
        return;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MdpSource::~MdpSource() {

    if (m_pFbPanel != 0) {
        close(m_pFbPanel);
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PortBufferCapability MdpSource::GetBufferRequirements(OMX_U32 ePortIndex) {

    PortBufferCapability sBufCap;

    Mutex::Autolock autoLock(m_pLock);
    memset(&sBufCap, 0, sizeof(PortBufferCapability));

    if (ePortIndex == PORT_INDEX_OUT) {
        sBufCap.bAllocateBuffer = OMX_FALSE;
        sBufCap.bUseBuffer = OMX_TRUE;
        sBufCap.pSource = this;
        sBufCap.ePortIndex = ePortIndex;
        sBufCap.nMinBufferSize = 0x1000;
        sBufCap.nMinBufferCount = 3;
        sBufCap.nExtraBufferCount = 0;
        sBufCap.nBufferUsage = 0;
    } else {
        VTEST_MSG_ERROR("Error: invalid port selection");
    }
    return sBufCap;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MdpSource::Configure(CodecConfigType *pConfig,
    BufferManager *pBufManager, ISource *pSource, ISource *pSink) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    result = ISource::Configure(pConfig, pBufManager, pSource, pSink);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("MDPSource configure failed");
        return result;
    }

    Mutex::Autolock autoLock(m_pLock);

    m_nFrames = pConfig->nFrames;
    m_nFramerate = pConfig->nFramerate;
    m_nDVSXOffset = pConfig->nDVSXOffset;
    m_nDVSYOffset = pConfig->nDVSYOffset;

    if (m_nFrames <= 0 || m_nFramerate <= 0 ||
        m_nDVSXOffset < 0 || m_nDVSYOffset < 0) {
        VTEST_MSG_ERROR("MdpSource: bad cfg params");
        return OMX_ErrorBadParameter;
    }

    result = (OMX_ERRORTYPE)ioctl(m_pFbPanel, MSMFB_WRITEBACK_INIT, NULL);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("MSMFB_WRITEBACK_INIT failed %d", result);
        return OMX_ErrorIncorrectStateTransition;
    }

    //call write back start ioctl
    result = (OMX_ERRORTYPE)ioctl(m_pFbPanel, MSMFB_WRITEBACK_START, NULL);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("MSMFB_WRITEBACK_START failed %d", result);
        return OMX_ErrorIncorrectStateTransition;
    }

    int hint = MDP_WRITEBACK_MIRROR_ON;
    if (ioctl(m_pFbPanel, MSMFB_WRITEBACK_SET_MIRRORING_HINT, &hint) < 0) {
        VTEST_MSG_ERROR("MpdSource: Couldn't start/stop mirroring");
        return OMX_ErrorUnsupportedSetting;
    }
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MdpSource::SetBuffer(BufferInfo *pBuffer, ISource *pSource) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *pHeader;
    OMX_S32 index = -1;
    struct msmfb_data fbdata;

    result = ISource::SetBuffer(pBuffer, pSource);
    if (result != OMX_ErrorNone) {
        return result;
    }

    pHeader = pBuffer->pHeaderOut;

    // find buffer index
    for (index = 0; index < m_nHeaderCount; index++) {
        if (pHeader == m_pHeaders[index]) {
            break;
        }
    }
    if (index < MAX_BUF_COUNT) {
        m_pHeaders[index] = pHeader;
        m_nHeaderCount++;
        VTEST_MSG_LOW("buffer registered %p, ct %u", pHeader, (unsigned int)m_nHeaderCount);
    } else {
        VTEST_MSG_ERROR("Error: too many buffers registered %p", pBuffer);
        return OMX_ErrorBadParameter;
    }

    // queue buffer in hardware
    memset(&fbdata, 0, sizeof(fbdata));
    fbdata.offset = 0;
    fbdata.memory_id = pBuffer->pHandle;
    fbdata.id = index;
    fbdata.flags = 0;
    fbdata.iova = 0; //specify fbtest is using mixer-2

    //call write back queue buffer ioctl
    VTEST_MSG_LOW("queueing buffer...");
    if (ioctl(m_pFbPanel, MSMFB_WRITEBACK_QUEUE_BUFFER, &fbdata) < 0) {
        VTEST_MSG_ERROR("MdpSource: MSMFB_WRITEBACK_QUEUE_BUFFER failure");
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MdpSource::ThreadRun(OMX_PTR pThreadData) {

    (void)pThreadData;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *pHeader = NULL;
    BufferInfo *pBuffer = NULL;
    OMX_TICKS nTimeStamp = 0;
    struct msmfb_data fbdata;
    enum v4l2_buf_type type;
    int rv;

    // get frames in a loop
    for (OMX_S32 i = 1; i <= m_nFrames && !m_bThreadStop; i++) {

        VTEST_MSG_LOW("MdpSource: dequeue frame...");
        fbdata.flags = MSMFB_WRITEBACK_DEQUEUE_BLOCKING;
        if (ioctl(m_pFbPanel, MSMFB_WRITEBACK_DEQUEUE_BUFFER, &fbdata) < 0) {
            VTEST_MSG_ERROR("MdpSource: MSMFB_WRITEBACK_DEQUEUE_BUFFER failure");
            continue;
        }

        VTEST_MSG_LOW("MdpSource: dequeue fd %d ind %d size %d\n",
                      fbdata.memory_id, fbdata.id, fbdata.offset);
        pHeader = m_pHeaders[fbdata.id];
        result = m_pBufferManager->GetBuffer(
            this, PORT_INDEX_OUT, pHeader, &pBuffer);
        FAILED2(result, SetError(), "buffer %p not found!!!", pHeader);

        //nTimeStamp = (OMX_TICKS) Time::GetTimeMicrosec();
        nTimeStamp = nTimeStamp + (OMX_TICKS)(1000000 / m_nFramerate);

        pHeader->nTimeStamp = nTimeStamp;
        pHeader->nFilledLen = pHeader->nAllocLen; //(m_nFrameWidth * m_nFrameHeight * 3) / 2;
        pHeader->nOffset = fbdata.offset;         //((m_nFrameWidth * m_nDVSYOffset) + m_nDVSXOffset) * 3 / 2;

        // set the EOS flag if this is the last frame
        pHeader->nFlags = 0;
        if (i >= m_nFrames) {
            pHeader->nFlags = OMX_BUFFERFLAG_EOS;
            VTEST_MSG_HIGH("enable OMX_BUFFERFLAG_EOS on frame %u", (unsigned int)i);
            m_bThreadStop = OMX_TRUE;
        }

        VTEST_MSG_LOW("MdpSource: delivering frame #%u, bytes %u, time %llu)...",
                      (unsigned int)i, (unsigned int)pHeader->nFilledLen, pHeader->nTimeStamp);
        m_pSink->SetBuffer(pBuffer, this);
    } //end for loop

    int hint = MDP_WRITEBACK_MIRROR_OFF;
    if (ioctl(m_pFbPanel, MSMFB_WRITEBACK_SET_MIRRORING_HINT, &hint) < 0) {
        VTEST_MSG_ERROR("MpdSource: Couldn't start/stop mirroring");
        result = OMX_ErrorUnsupportedSetting;
    }
    VTEST_MSG_HIGH("Stopping MDP mirroring");

    //call write back stop ioctl
    rv = ioctl(m_pFbPanel, MSMFB_WRITEBACK_STOP, NULL);
    if (rv < 0) {
        VTEST_MSG_ERROR("MSMFB_WRITEBACK_STOP failed %d", rv);
        result = OMX_ErrorUnsupportedSetting;
    }
    VTEST_MSG_HIGH("MDP writeback stop");

    //call write back terminate ioctl
    rv = ioctl(m_pFbPanel, MSMFB_WRITEBACK_TERMINATE, NULL);
    if (rv < 0) {
        VTEST_MSG_ERROR("MSMFB_WRITEBACK_TERMINATE failed %d", rv);
        result = OMX_ErrorUnsupportedSetting;
    }
    VTEST_MSG_HIGH("MDP writeback terminate");

    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void MdpSource::GetFrameBufferDev(char fb_dev[128]) {

    int c = 0;
    bool try_next_fb = true;

    while (try_next_fb) {
        char temp_fb_type[128], temp[32] = { 0 }, wanted_fb_type[] = "writeback panel";
        FILE *temp_file = NULL;

        snprintf(temp_fb_type, sizeof(temp_fb_type),
                 "/sys/class/graphics/fb%d/msm_fb_type", c);
        temp_file = fopen(temp_fb_type, "r");
        if (!temp_file) break;

        int size = fread(&temp, sizeof(temp), 1, temp_file);
        /* XXX avoid using strcmp below as temp is not null terminated */
        if (!memcmp(temp, wanted_fb_type, strlen(wanted_fb_type))) {
            snprintf(fb_dev, 128 * sizeof(char), "/dev/graphics/fb%d", c);
            try_next_fb = false;
        }

        fclose(temp_file);
        ++c;
    }
}

} // namespace vtest
