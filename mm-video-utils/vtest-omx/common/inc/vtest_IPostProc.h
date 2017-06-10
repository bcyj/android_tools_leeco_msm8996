/*-------------------------------------------------------------------
Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_IPOSTPROC_H
#define _VTEST_IPOSTPROC_H

#include "OMX_Core.h"
#include "vtest_Debug.h"
#include "vtest_ComDef.h"
#include "vtest_Crypto.h"

namespace vtest {

class IPostProc;
IPostProc *CreatePostProcModule(PostProcType ePostProcType);

struct PostProcSession {
    OMX_U32 nFrameWidth;
    OMX_U32 nFrameHeight;
    OMX_BOOL bSecureSession;
    OMX_U32 nInputColorFormat;
    OMX_U32 nOutputColorFormat;
};

class IPostProc {

public:
    IPostProc();
    /**
     * @brief Destructor
     */
    virtual ~IPostProc();

    /**
     * @brief init session
     *
     * @param pSession - PostProcSession structure containing
     * session information
     */
    virtual OMX_ERRORTYPE Init(PostProcSession *pSession);

    /**
     * @brief terminate session
     */
    virtual void Terminate();

    /**
     * @brief perform post processing
     *
     * @param pBufferIn - Input buffer
     * @param pBufferOut - Output buffer
     */
    virtual OMX_ERRORTYPE Perform(BufferInfo *pBufferIn, BufferInfo *pBufferOut);

    /**
     * @brief get buffer requirements, for the buffer to post process in
     */
    virtual void GetBufferRequirements(OMX_U32 ePortIndex, OMX_U32 *nBufferSize);

    /**
     * @brief Text formatted object name, for readable logging
     */
    virtual OMX_STRING Name();

     /**
     * @brief Update output buffer geometry
     *
     * @param pBuffer - Output buffer
     */
    virtual void UpdateOutputBufferGeometry(BufferInfo *pBuffer);

protected:
    OMX_U32 GetFrameSize(OMX_U32 nFormat, OMX_U32 nWidth, OMX_U32 nHeight);
    OMX_U32 GetFrameStride(OMX_U32 nFormat, OMX_U32 nWidth);
    OMX_U32 GetFrameSlice(OMX_U32 nFormat, OMX_U32 nHeight);

protected:
    char m_pName[PROPERTY_FILENAME_MAX];
    OMX_U32 m_nFrameWidth;
    OMX_U32 m_nFrameHeight;
    OMX_BOOL m_bSecureSession;
    void *m_pSessionHandle;
    void *m_pLibHandle;
    OMX_U32 m_nInputColorFormat;
    OMX_U32 m_nOutputColorFormat;
};

} // namespace vtest

#endif // #ifndef _VTEST_IPOSTPROC_H
