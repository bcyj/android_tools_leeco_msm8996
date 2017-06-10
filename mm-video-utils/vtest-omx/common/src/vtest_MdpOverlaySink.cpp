/* ========================================================================= *
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#include "vtest_Debug.h"
#include "vtest_MdpOverlaySink.h"
#include "vtest_ISource.h"
#include "vtest_Sleeper.h"
#include <fcntl.h>

#undef LOG_TAG
#define LOG_TAG "VTEST_MDP_OVERLAY_SINK"
#define COLOR_BLACK_RGBA_8888 0x00000000

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MdpOverlaySink::MdpOverlaySink(OMX_BOOL bRotate)
    : ISource(),
      m_nFrames(0),
      m_nFrameWidth(0),
      m_nFrameHeight(0),
      m_bRotate(bRotate),
      m_pBufferQueue(new SignalQueue(100, sizeof(OMX_BUFFERHEADERTYPE*))),
      m_nFbFd(-1),
      m_nOverlayId(-1),
      m_nIonFd(-1),
      m_bSecureSession(OMX_FALSE) {

    snprintf(m_pName, PROPERTY_FILENAME_MAX, "MdpOverlaySink");
    VTEST_MSG_LOW("%s created", Name());

    memset(&m_sOverlay, 0, sizeof(m_sOverlay));
    memset(&m_sVinfo, 0, sizeof(m_sVinfo));
    memset(&m_sFinfo, 0, sizeof(m_sFinfo));
    memset(&m_sRotatorIonData, 0, sizeof(m_sRotatorIonData));
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MdpOverlaySink::~MdpOverlaySink() {

    if (m_pBufferQueue != NULL) {
        delete m_pBufferQueue;
        m_pBufferQueue = NULL;
    }
    if (m_nFbFd >= 0) {
        OverlayUnset();
        close(m_nFbFd);
        m_nFbFd = -1;
    }
    VTEST_MSG_ERROR("I'm getting free!!");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PortBufferCapability MdpOverlaySink::GetBufferRequirements(OMX_U32 ePortIndex) {

    PortBufferCapability sBufCap;

    Mutex::Autolock autoLock(m_pLock);
    memset(&sBufCap, 0, sizeof(PortBufferCapability));

    if (ePortIndex == PORT_INDEX_IN) {
        sBufCap.bAllocateBuffer = OMX_FALSE;
        sBufCap.bUseBuffer = OMX_TRUE;
        sBufCap.pSource = this;
        sBufCap.ePortIndex = ePortIndex;
        sBufCap.nMinBufferSize = 0x1000;
        sBufCap.nMinBufferCount = 1;
        sBufCap.nExtraBufferCount = 0;
    } else {
        VTEST_MSG_ERROR("Error: invalid port selection");
    }
    return sBufCap;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MdpOverlaySink::Configure(CodecConfigType *pConfig,
        BufferManager *pBufManager, ISource *pSource, ISource *pSink) {

    OMX_ERRORTYPE result;

    result = ISource::Configure(pConfig, pBufManager, pSource, pSink);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("MdpOverlaySink configure failed");
        return result;
    }

    Mutex::Autolock autoLock(m_pLock);

    m_nFrameWidth = pConfig->nFrameWidth;
    m_nFrameHeight = pConfig->nFrameHeight;
    m_nFrames = pConfig->nFrames;
    m_bSecureSession = pConfig->bSecureSession;

#ifdef _ANDROID_
    VTEST_MSG_HIGH("Opening /dev/graphics/fb0");
    m_nFbFd = open("/dev/graphics/fb0", O_RDWR);
#else
    VTEST_MSG_HIGH("Opening /dev/fb0");
    m_nFbFd = open("/dev/fb0", O_RDWR);
#endif

    if (m_nFbFd < 0) {
        VTEST_MSG_ERROR("Can't open framebuffer!");
        return OMX_ErrorBadParameter;
    }

    VTEST_MSG_HIGH("m_nFbFd = %d", (unsigned int)m_nFbFd);
    if (ioctl(m_nFbFd, FBIOGET_FSCREENINFO, &m_sFinfo) < 0) {
        VTEST_MSG_ERROR("Can't retrieve fscreenInfo!");
        close(m_nFbFd);
        m_nFbFd = -1;
        return OMX_ErrorUndefined;
    }
    if (ioctl(m_nFbFd, FBIOGET_VSCREENINFO, &m_sVinfo) < 0) {
        VTEST_MSG_ERROR("Can't retrieve vscreenInfo!");
        close(m_nFbFd);
        m_nFbFd = -1;
        return OMX_ErrorUndefined;
    }

    result = OverlaySet();
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MdpOverlaySink::SetBuffer(
        BufferInfo *pBuffer, ISource *pSource) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    result = ISource::SetBuffer(pBuffer, pSource);
    if (result != OMX_ErrorNone) {
        return result;
    }

    VTEST_MSG_LOW("queue push (%p %p)", pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    result = m_pBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MdpOverlaySink::PortReconfig(OMX_U32 ePortIndex,
        OMX_U32 nWidth, OMX_U32 nHeight, const OMX_CONFIG_RECTTYPE& sRect) {

    (void)sRect;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    VTEST_MSG_LOW("PortReconfig port %s", OMX_PORT_NAME(ePortIndex));

    if (m_nFrameWidth != nWidth || m_nFrameHeight != nHeight) {
        VTEST_MSG_LOW("PortReconfig WxH (%uX%u) ==> WxH (%uX%u)",
                (unsigned int)m_nFrameWidth, (unsigned int)m_nFrameHeight, (unsigned int)nWidth, (unsigned int)nHeight);
        m_nFrameWidth = nWidth;
        m_nFrameHeight = nHeight;
    }
    OverlayUnset();
    return OverlaySet();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MdpOverlaySink::ThreadRun(OMX_PTR pThreadData) {

    (void)pThreadData;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    BufferInfo *pBuffer = NULL;

    for (OMX_U32 i = 1; m_bThreadStop != OMX_TRUE; i++) {
        BufferInfo *pFreeBuffer = NULL;
        OMX_BUFFERHEADERTYPE *pHeader = NULL, *pFreeHeader = NULL;

        result = m_pBufferQueue->Pop(&pBuffer, sizeof(BufferInfo **), 0);
        VTEST_MSG_LOW("queue pop %u of %u (qsize %u)",
                      (unsigned int)i, (unsigned int)m_nFrames, (unsigned int)m_pBufferQueue->GetSize());

        if ((pBuffer == NULL) || (result != OMX_ErrorNone)) {
            /* Can only happen if stop is called or someone else ran into an
             * error */
            VTEST_MSG_HIGH("Stopping thread");
            result = OMX_ErrorNone;
            continue;
        }

        pHeader = pBuffer->pHeaderIn;
        if (pHeader->nFlags & OMX_BUFFERFLAG_ENDOFFRAME) {
            VTEST_MSG_HIGH("frames written %u", (unsigned int)i);
        }

        if (pHeader->nFlags & OMX_BUFFERFLAG_EOS) {
            VTEST_MSG_HIGH("got EOS for frame : %u", (unsigned int)i);
            m_bThreadStop = OMX_TRUE;
        }

        if (pHeader->nFilledLen > 0) {
            result = OverlayEnqueueBuffer(pHeader);
            FAILED2(result, SetError(), "Error while enquing buffer : %p", pHeader);
        } else  {
            VTEST_MSG_HIGH("skipping frame because of 0 length %u...", (unsigned int)i);
        }
        VTEST_MSG_MEDIUM("received frame %u...", (unsigned int)i);
        m_pSource->SetBuffer(pBuffer, this);
    }

    //clean up
    while(m_pBufferQueue->GetSize() > 0) {
        VTEST_MSG_LOW("cleanup: q-wait (qsize %u)", (unsigned int)m_pBufferQueue->GetSize());
        m_pBufferQueue->Pop(&pBuffer, sizeof(BufferInfo **), 0);
        m_pSource->SetBuffer(pBuffer, this);
    }

    VTEST_MSG_HIGH("thread exiting...");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void MdpOverlaySink::OverlayUnset() {

    FreeRotatorBuffer(); // free any existing memory
    if (ioctl(m_nFbFd, MSMFB_OVERLAY_UNSET, &m_nOverlayId))
    {
        VTEST_MSG_ERROR("MSMFB_OVERLAY_UNSET failed!");
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MdpOverlaySink::OverlaySet() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    struct mdp_overlay *pOverlay = &m_sOverlay;

    OMX_U32 nWidth = m_nFrameWidth;
    OMX_U32 nHeight = m_nFrameHeight;
    pOverlay->src.width  = nWidth;
    pOverlay->src.height = nHeight;
    VTEST_MSG_HIGH("width is %u, height is %u\n", (unsigned int)nWidth, (unsigned int)nHeight);

    pOverlay->src.format = MDP_Y_CBCR_H2V2_VENUS;
    pOverlay->src_rect.x = 0;
    pOverlay->src_rect.y = 0;
    pOverlay->src_rect.w = nWidth;
    pOverlay->src_rect.h = nHeight;
    pOverlay->z_order = 0;
	pOverlay->flags = 0;

    if (m_bRotate) {
        struct mdp_overlay rotator = m_sOverlay;

        rotator.dst_rect.w = nHeight;
        rotator.dst_rect.h = nWidth;

        if (m_bSecureSession) {
            rotator.flags = MDP_SECURE_OVERLAY_SESSION;
        }
        rotator.flags |= MDP_ROT_90 | MDSS_MDP_ROT_ONLY;
        rotator.id = MSMFB_NEW_REQUEST;

        if (ioctl(m_nFbFd, MSMFB_OVERLAY_SET, &rotator)) {
            VTEST_MSG_ERROR("ERROR: MSMFB_OVERLAY_SET rotator failed!");
            return OMX_ErrorUndefined;
        }

        if (AllocateRotatorBuffer()) {
            VTEST_MSG_ERROR("ERROR allocating memory for rotator");
            return OMX_ErrorUndefined;
        }

        /* swap width and height */
        nWidth = pOverlay->src_rect.h;
        nHeight = pOverlay->src_rect.w;
        pOverlay->src.width  = nWidth;
        pOverlay->src.height = nHeight;
        pOverlay->src_rect.w = nWidth;
        pOverlay->src_rect.h = nHeight;
        pOverlay->flags = MDP_SOURCE_ROTATED_90;
    }

    if (nWidth >= m_sVinfo.xres) {
        pOverlay->dst_rect.x = 0;
        pOverlay->dst_rect.w = m_sVinfo.xres;
    } else {
        pOverlay->dst_rect.x = (m_sVinfo.xres - nWidth)/2;
        pOverlay->dst_rect.w = nWidth;
    }

    if (nHeight >= m_sVinfo.yres) {
        pOverlay->dst_rect.h = (pOverlay->dst_rect.w * nHeight)/nWidth;
        pOverlay->dst_rect.y = 0;
        if (pOverlay->dst_rect.h < m_sVinfo.yres) {
            pOverlay->dst_rect.y = (m_sVinfo.yres - pOverlay->dst_rect.h)/2;
        }
    } else {
        pOverlay->dst_rect.y = (m_sVinfo.yres - nHeight)/2;
        pOverlay->dst_rect.h = nHeight;
    }

    pOverlay->alpha = 0x0;
    pOverlay->transp_mask = 0xFFFFFFFF;
    pOverlay->is_fg = 0;
    if (m_bSecureSession) {
        pOverlay->flags |= MDP_SECURE_OVERLAY_SESSION;
    }

    pOverlay->id = MSMFB_NEW_REQUEST;
    m_nOverlayId = ioctl(m_nFbFd, MSMFB_OVERLAY_SET, pOverlay);
    if (m_nOverlayId < 0) {
        VTEST_MSG_ERROR("ERROR: MSMFB_OVERLAY_SET failed!");
    }
    m_nOverlayId = pOverlay->id;
    VTEST_MSG_HIGH("Overlay_set m_nOverlayId = %d", (unsigned int)m_nOverlayId);

    void *fb_buf = mmap(NULL, m_sFinfo.smem_len, PROT_READ|PROT_WRITE,
            MAP_SHARED, m_nFbFd, 0);

    if (fb_buf == MAP_FAILED)
    {
        VTEST_MSG_ERROR("Framebuffer MMAP failed");
        return OMX_ErrorBadParameter;
    }

    m_sVinfo.yoffset = 0;
    long *p = (long *)fb_buf;

    for (OMX_U32 i = 0; i < m_sVinfo.xres * m_sVinfo.yres; i++) {
        *p++ = COLOR_BLACK_RGBA_8888;
    }

    if (ioctl(m_nFbFd, FBIOPAN_DISPLAY, &m_sVinfo) < 0) {
        VTEST_MSG_ERROR("FBIOPAN_DISPLAY failed");
        result = OMX_ErrorBadParameter;
    }
    munmap(fb_buf, m_sFinfo.smem_len);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MdpOverlaySink::OverlayEnqueueBuffer(struct OMX_BUFFERHEADERTYPE *pBufHdr) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *pPMEMInfo = NULL;
    struct msmfb_overlay_data s_OvFront;
    memset(&s_OvFront, 0, sizeof(struct msmfb_overlay_data));

#if 0
    OverlayUnset();
    result = OverlaySet();
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("Overlay set failed in EnqueueBuffer");
        return result;
    }
#endif

    s_OvFront.id = m_nOverlayId;
    if (pBufHdr->pPlatformPrivate == NULL) {
        VTEST_MSG_ERROR("overlay_fb: pPlatformPrivate is null");
        return OMX_ErrorBadParameter;
    }

    pPMEMInfo  = (OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *)
                ((OMX_QCOM_PLATFORM_PRIVATE_LIST *)
                pBufHdr->pPlatformPrivate)->entryList->entry;
    if (pPMEMInfo == NULL) {
        VTEST_MSG_ERROR("overlay_fb: pmem_info is null");
        return OMX_ErrorBadParameter;
    }

    VTEST_MSG_MEDIUM("pmem fd is : %lu", pPMEMInfo->pmem_fd);
    s_OvFront.data.memory_id = pPMEMInfo->pmem_fd;
    s_OvFront.data.offset = pPMEMInfo->offset;

    if (m_bRotate) {
        s_OvFront.id = MSMFB_NEW_REQUEST;
        s_OvFront.dst_data.memory_id = m_sRotatorIonData.fd;
        s_OvFront.dst_data.offset = 0;

        if (ioctl(m_nFbFd, MSMFB_OVERLAY_PLAY, (void*)&s_OvFront)) {
            VTEST_MSG_ERROR("MSMFB_OVERLAY_PLAY rotator failed at frame");
            return OMX_ErrorUndefined;
        }
        // set rotated output as input to display
        s_OvFront.data = s_OvFront.dst_data;
        s_OvFront.id =  m_nOverlayId;
    }

    VTEST_MSG_MEDIUM("s_OvFront.data.memory_id = %d", s_OvFront.data.memory_id);
    VTEST_MSG_MEDIUM("s_OvFront.data.offset = %u", s_OvFront.data.offset);
    if (ioctl(m_nFbFd, MSMFB_OVERLAY_PLAY, (void*)&s_OvFront)) {
        VTEST_MSG_ERROR("MSMFB_OVERLAY_PLAY failed at frame");
        return OMX_ErrorUndefined;
    }

    VTEST_MSG_MEDIUM("MSMFB_OVERLAY_PLAY successfull");
    if (ioctl(m_nFbFd, FBIOPAN_DISPLAY, &m_sVinfo) < 0) {
        VTEST_MSG_ERROR("FBIOPAN_DISPLAY failed");
        result = OMX_ErrorUndefined;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void MdpOverlaySink::FreeRotatorBuffer() {

    if (m_sRotatorIonData.handle) {
        ioctl(m_nIonFd, ION_IOC_FREE, &m_sRotatorIonData);
    }

    if (m_nIonFd >= 0) {
        close(m_nIonFd);
        m_nIonFd = -1;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MdpOverlaySink::AllocateRotatorBuffer() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    struct ion_allocation_data ionAllocData;
    FreeRotatorBuffer(); // free any existing memory

    if (m_nIonFd < 0) {
        m_nIonFd = open("/dev/ion", O_RDWR|O_DSYNC);
        if (m_nIonFd < 0) {
                VTEST_MSG_ERROR("Can't open ion device");
                return OMX_ErrorUndefined;
        }
    }

    ionAllocData.len = 13 * 1024 * 1024; // more than enough?
    ionAllocData.align = 1024 * 1024; // 1mb
    ionAllocData.heap_id_mask = ION_HEAP(ION_IOMMU_HEAP_ID);
    ionAllocData.flags = 0;
    if (m_bSecureSession) {
        ionAllocData.flags |= ION_SECURE;
        ionAllocData.heap_id_mask = ION_HEAP(ION_CP_MM_HEAP_ID);
    }

    if (ioctl(m_nIonFd, ION_IOC_ALLOC, &ionAllocData)) {
        VTEST_MSG_ERROR("Rotator buffer allocation failed");
        return OMX_ErrorUndefined;
    }

    m_sRotatorIonData.fd = -1;
    m_sRotatorIonData.handle = ionAllocData.handle;
    if (ioctl(m_nIonFd, ION_IOC_MAP, &m_sRotatorIonData)) {
        printf("Rotator buffer allocation - ION_IOC_MAP failed");
        result = OMX_ErrorUndefined;
    }
    return result;
}

}
