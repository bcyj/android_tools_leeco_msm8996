/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_ENCODER_H
#define _VTEST_ENCODER_H

#include "OMX_Core.h"
#include "OMX_Component.h"
#include "OMX_QCOMExtns.h"
#include "vtest_ComDef.h"
#include "vtest_Thread.h"
#include "vtest_ISource.h"
#include "vtest_SignalQueue.h"
#include "vtest_BufferManager.h"

namespace vtest {

class Signal;

/**
 * @brief Class that wraps/simplifies OMX IL encoder component interactions.
 */
class Encoder : virtual public ISource {

public:
    Encoder(CodecConfigType *pConfig);
    ~Encoder();

    virtual PortBufferCapability GetBufferRequirements(OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE Start();
    virtual OMX_ERRORTYPE Stop();
    virtual OMX_ERRORTYPE Configure(CodecConfigType *pConfig,
            BufferManager *pBufManager, ISource *pSource, ISource *pSink);
    virtual OMX_ERRORTYPE SetBuffer(
            BufferInfo *pBufferInfo, ISource *pSource);
    virtual OMX_ERRORTYPE AllocateBuffers(BufferInfo **pBuffers,
            OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
            OMX_U32 nBufferSize, OMX_U32 ePortIndex, OMX_U32 nBufferUsage = 0);
    virtual OMX_ERRORTYPE UseBuffers(BufferInfo **pBuffers,
            OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
            OMX_U32 nBufferSize, OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE PortReconfig(OMX_U32 ePortIndex,
            OMX_U32 nWidth, OMX_U32 nHeight, const OMX_CONFIG_RECTTYPE& sRect);

private:
    virtual OMX_ERRORTYPE ThreadRun(OMX_PTR pThreadData);
    virtual OMX_ERRORTYPE FreeBuffer(
            BufferInfo *pBuffer, OMX_U32 ePortIndex);
    /**
     * @brief Set the encoder state
     *
     * This method can be asynchronous or synchronous. If asynchonous,
     * WaitState can be called to wait for the corresponding state
     * transition to complete.
     *
     * @param eState The state to enter
     * @param bSynchronous If OMX_TRUE, synchronously wait for the state transition to complete
     */
    OMX_ERRORTYPE SetState(OMX_STATETYPE eState, OMX_BOOL bSynchronous);
    /**
     * @brief Wait for the corresponding state transition to complete
     *
     * @param eState The state to wait for
     */
    OMX_ERRORTYPE WaitState(OMX_STATETYPE eState);
    /**
     * @brief Deliver an input (yuv) buffer to the encoder.
     *
     * @param pBuffer The populated input buffer.
     */
    OMX_ERRORTYPE DeliverInput(OMX_BUFFERHEADERTYPE *pBuffer);
    /**
     * @brief Deliver an output (bitstream) buffer to the encoder.
     *
     * @param pBuffer The un-populated output buffer.
     */
    OMX_ERRORTYPE DeliverOutput(OMX_BUFFERHEADERTYPE *pBuffer);
    /**
      * @brief Request for an iframe to be generated.
      */
    OMX_ERRORTYPE RequestIntraVOP();
    /**
     * @brief Set the intra period. It is valid to change this configuration at run-time
     *
     * @param nIntraPeriod The iframe interval in units of frames
     */
    OMX_ERRORTYPE SetIntraPeriod(OMX_S32 nIntraPeriod);
    /**
     * @brief Set the intra Refresh MB's. It is valid to change this configuration at run-time
     *
     * @param nIntraMBs The intra  MB's in a p Frame
     */
    OMX_ERRORTYPE SetIDRPeriod(OMX_S32 nIntraPeriod, OMX_S32 nIDRPeriod);
    /**
     * @brief Set the IDR period for AVC. It is valid to change
     *        this configuration at run-time
     *
     * @param nIntraMBs The intra period and IDR period
     */
    OMX_ERRORTYPE SetIntraRefresh(OMX_S32 nIntraMBs);
    /**
    * @brief Set the Hier P Num Layers. It is valid to change
    *        this configuration at run-time
    *
    * @param nHierPNumLayers the count of Hier P layers
    */
    OMX_ERRORTYPE SetHierPNumLayers(OMX_U32 nHierPNumLayers);
    /**
    * @brief Set Encoder Performance mode.
    *
    * @param nPerfMode, performance mode for encoding
    */
    OMX_ERRORTYPE SetVencPerfMode(OMX_U32 nPerfMode);
    /**
     * @brief Set port frame size and get resulting buffer
     *        requirements
     *
     * @param ePortIndex input/output port
     * @param nWidth frame width
     * @param nHeight frame height
     * @param nBufferCountMin min requested buffers
     * @param nFrameRate frame rate (output port only)
     * @param nBufferSize [out] required buffer size
     * @param nBufferCount [out] required buffer count
     */
    OMX_ERRORTYPE SetPortParams(OMX_U32 ePortIndex,
            OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCountMin,
            OMX_U32 nFrameRate, OMX_U32 *nBufferSize, OMX_U32 *nBufferCount);
    /**
     * @brief Change the encoding quality
     *
     * @param nFramerate The updated frame rate
     * @param nBitrate The updated bitrate
     */
    OMX_ERRORTYPE ChangeQuality(OMX_S32 nFramerate, OMX_S32 nBitrate);

    OMX_ERRORTYPE ProcessDynamicConfiguration(
            DynamicConfig *dynamic_config, int index);

    static OMX_ERRORTYPE EventCallback(
            OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_EVENTTYPE eEvent, OMX_IN OMX_U32 nData1,
            OMX_IN OMX_U32 nData2, OMX_IN OMX_PTR pEventData);
    static OMX_ERRORTYPE EmptyDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData, OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);
    static OMX_ERRORTYPE FillDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData, OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

    OMX_ERRORTYPE GetComponentRole(OMX_BOOL bSecureSession,
            OMX_VIDEO_CODINGTYPE eCodec, OMX_STRING *role);
    OMX_ERRORTYPE Flush(OMX_U32 nPortIndex);

private:
    OMX_BOOL m_bInputEOSReached;
    SignalQueue *m_pSignalQueue;
    OMX_BOOL m_bSecureSession;
    CodecConfigType m_sConfig;
    OMX_VIDEO_CODINGTYPE m_eCodec;
    OMX_HANDLETYPE m_hEncoder;
    OMX_STATETYPE m_eState;
    OMX_STATETYPE m_eStatePending;
    OMX_U32 m_nInputBufferCount;
    OMX_U32 m_nOutputBufferCount;
    OMX_U32 m_nInputBufferSize;
    OMX_U32 m_nOutputBufferSize;
    OMX_U32 m_LTRId;
    OMX_S32 m_nDynamicIndexCount;
    DynamicConfigType m_sDynamicConfig;
    OMX_BOOL m_bPortReconfig;
    SignalQueue *m_pFreeBufferQueue;
    OMX_BOOL m_bMetaMode;
    MetaBufferType m_eMetaBufferType;
    OMX_U32 m_nFrameWidth;
    OMX_U32 m_nFrameHeight;
};

} // namespace vtest


#endif // #ifndef  _VTEST_ENCODER_H
