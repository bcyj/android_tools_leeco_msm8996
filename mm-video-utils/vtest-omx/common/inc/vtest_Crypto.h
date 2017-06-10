/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_CRYPTO_H
#define _VTEST_CRYPTO_H

#include "OMX_Core.h"
#include "vtest_Mutex.h"

#ifdef SECURE_COPY_ENABLED

#include "content_protection_copy.h"

namespace vtest {

typedef SampleClientResult(*Content_Protection_Copy_Init)(struct QSEECom_handle **);
typedef SampleClientResult(*Content_Protection_Copy_Terminate)(struct QSEECom_handle **);
typedef SampleClientResult(*Content_Protection_Copy)(struct QSEECom_handle *,
        OMX_U8 *, const uint32, uint32, uint32, uint32 *, SampleClientCopyDir);

class Crypto {

public:
    Crypto();
    ~Crypto();

    OMX_ERRORTYPE Init();
    OMX_ERRORTYPE Terminate();
    OMX_ERRORTYPE Copy(SampleClientCopyDir eCopyDir,
            OMX_U8 *pBuffer, unsigned long nBufferFd, OMX_U32 nBufferSize);

private:
    OMX_ERRORTYPE LoadCryptoLib();
    void UnloadCryptoLib();

    void *m_pLibHandle;
    QSEECom_handle *m_pWVHandle;
    Content_Protection_Copy_Init m_fOEMCryptoInit;
    Content_Protection_Copy_Terminate m_fOEMCryptoTerminate;
    Content_Protection_Copy m_fOEMCryptoCopy;
    Mutex *m_pLock;
};

}
#else //SECURE_COPY_DISABLED

namespace vtest {

enum SampleClientCopyDir {
    SAMPLECLIENT_COPY_NONSECURE_TO_SECURE = 0,
    SAMPLECLIENT_COPY_SECURE_TO_NONSECURE,
    SAMPLECLIENT_COPY_INVALID_DIR
}; //Taken from content_protection_copy.h

class Crypto {

public:
    Crypto();
    ~Crypto();

    OMX_ERRORTYPE Init();
    OMX_ERRORTYPE Terminate();
    OMX_ERRORTYPE Copy(SampleClientCopyDir eCopyDir,
            OMX_U8 *pBuffer, unsigned long nBufferFd, OMX_U32 nBufferSize);
    Mutex *m_pLock;
};

}

#endif //SECURE_COPY_ENABLED

#endif //#ifndef _VTEST_CRYPTO_H
