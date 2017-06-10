/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_ISOURCE_H
#define _VTEST_ISOURCE_H

#include "OMX_Core.h"
#include "vtest_Debug.h"
#include "vtest_ComDef.h"
#include "vtest_Thread.h"
#include "vtest_BufferManager.h"
#include "vtest_SignalQueue.h"

#define OMX_PORT_NAME(ePort) (ePort == PORT_INDEX_IN ? "IN_PORT" : "OUT_PORT")
#define OMX_STATE_NAME(eState)                              \
   (eState == OMX_StateInvalid ? "OMX_StateInvalid" :       \
   (eState == OMX_StateLoaded ? "OMX_StateLoaded" :         \
   (eState == OMX_StateIdle ? "OMX_StateIdle" :             \
   (eState == OMX_StateExecuting ? "OMX_StateExecuting" :   \
   (eState == OMX_StatePause ? "OMX_StatePause" :           \
    "Unknown")))))

namespace vtest {

class ISource : virtual public IThreaded {

public:
    /**
     * @brief Destructor
     */
    virtual ~ISource();

    /**
     * @brief get buffer requirements for this source, only valid
     *        after Configure has been called
     *
     * @param ePortIndex - input/output
     */
    virtual PortBufferCapability GetBufferRequirements(OMX_U32 ePortIndex) = 0;

    /**
     * @brief Starts the delivery of buffers.
     *
     * Source will continue to deliver buffers at the specified
     * rate until the specified number of frames have been delivered.
     *
     * Note that Source will not deliver buffers unless it has ownership
     * of atleast 1 buffer. If Source does not have ownership of buffer when
     * timer expires, it will block until a buffer is given to the component.
     * Free buffers should be given to the Source through SetFreeBuffer
     * function.
     */
    virtual OMX_ERRORTYPE Start();

    /**
     * @brief Wait for the thread to finish.
     *
     * Function will block until the Source has delivered all frames.
     */
    virtual OMX_ERRORTYPE Finish();

    /**
     * @brief Force the thread to stop processing frames
     *
     * Function will block until the Source has delivered all frames.
     */
    virtual OMX_ERRORTYPE Stop();

    /**
     * @brief Generic configuration function for all ISource objects
     */
    virtual OMX_ERRORTYPE Configure(CodecConfigType *pConfig,
            BufferManager *pBufManager, ISource *pSource, ISource *pSink);

    /**
     * @brief Gives ownership of the buffer to the source.
     *
     * All buffers must be registered with the Source prior to
     * invoking Start. This will give initial ownership to the
     * source.
     *
     * After Start is called, buffers must be given ownership
     * when yuv buffers are free.
     *
     * @param pBuffer - buffer being passed
     * @param pSource - caller should be sink or source
     */
    virtual OMX_ERRORTYPE SetBuffer(
            BufferInfo *pBuffer, ISource *pSource);

    /**
     * @brief Make any required state transitions and allocate
     *        buffers.  Must be called BEFORE any start calls.
     *
     * @param pBuffers - a preallocated array to be filled in
     * @param nBufferCount - number of buffers to alloc
     * @param nBufferSize - size of buffers to alloc
     * @param nWidth - frame width
     * @param nHeight - frame height
     *  @param ePortIndex - input or output
     *  @param nBufferUsage - used to pass buffer requirements
     */
    virtual OMX_ERRORTYPE AllocateBuffers(BufferInfo **pBuffers,
            OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
            OMX_U32 nBufferSize, OMX_U32 ePortIndex, OMX_U32 nBufferUsage = 0);

    /**
     * @brief Make any required state transitions and buffer
     *        registrations.   Must be called BEFORE any start
     *        calls.
     *
     * @param pBuffers - a preallocated array to be filled in
     * @param nBufferCount - number of buffers to alloc
     * @param nBufferSize - size of buffers to alloc
     * @param nWidth - frame width
     * @param nHeight - frame height
     * @param pBufferHandles - buffer handles (only valid in
     *                       non-secure mode)
     * @param ePortIndex - input or output
     * @param nBufferUsage - used to pass buffer requirements
     */
    virtual OMX_ERRORTYPE UseBuffers(BufferInfo **pBuffers,
            OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
            OMX_U32 nBufferSize, OMX_U32 ePortIndex);

    virtual OMX_ERRORTYPE FreeAllocatedBuffers(
            BufferInfo **pBuffers, OMX_U32 nBufferCount, OMX_U32 ePortIndex);

    virtual OMX_ERRORTYPE FreeUsedBuffers(
            BufferInfo **pBuffers, OMX_U32 nBufferCount, OMX_U32 ePortIndex);

    /**
     * @brief enable/disable port for reconfigure on the fly, allows
     *        reconfigure without leaving executing state.  The
     *        receiver should flush all buffers then disable the
     *        port to allow reallocation.
     *
     * @param ePortIndex - input/output port
     * @param bEnable - enable/disable port
     * @param nWidth - updated frame width (if changed)
     * @param nHeight - updated frame height (if changed)
     * @param sRect - updated crop (for smooth streaming)
     */
    virtual OMX_ERRORTYPE PortReconfig(OMX_U32 ePortIndex,
            OMX_U32 nWidth, OMX_U32 nHeight, const OMX_CONFIG_RECTTYPE& sRect);

    /**
     * @brief Text formated object name, for readable logging
     */
    virtual OMX_STRING Name();

    /**
     * @brief Total buffers passed out via SetBuffer calls
     * NOTE: for terminal sink/source objects Input=Ouput
     */
    virtual OMX_S32 OutputBufferCount();

    /**
     * @brief Total bytes passed out via SetBuffer calls
     * NOTE: for terminal sink/source objects Input=Ouput
     */
    virtual OMX_S32 OutputByteCount();

    /**
     * @brief get input source
     */
    virtual ISource* Source();

    /**
     * @brief get output sink
     */
    virtual ISource* Sink();

    virtual void SetError();

    virtual void StopThreadOnError();

    virtual OMX_BOOL ThreadStoppedOnError();

protected:
    ISource();

    virtual OMX_ERRORTYPE ThreadRun(OMX_PTR pThreadData);

    virtual OMX_ERRORTYPE FreeBuffer(
            BufferInfo *pBuffer, OMX_U32 ePortIndex);

protected:
    char m_pName[PROPERTY_FILENAME_MAX];            // friendly object name to help with debugging
    OMX_S32 m_nInBufferCount;     // buffers pushed through input-port
    OMX_U32 m_nInBytes;           // total bytes pushed through input-port
    Thread *m_pThread;
    OMX_BOOL m_bThreadStop;
    ISource *m_pSink;
    ISource *m_pSource;
    BufferManager *m_pBufferManager;
    Mutex *m_pLock;
    SignalQueue *m_pBufferQueue;

private:
    OMX_BOOL m_bSourceStop;
    OMX_ERRORTYPE m_bSourceResult;
};

} // namespace vtest

#endif // #ifndef _VTEST_ISOURCE_H
