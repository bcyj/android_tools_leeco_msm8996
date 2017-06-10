/*
 * Copyright (c) 2010 - 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "QComDivXDecrypt"
#include <utils/Log.h>

#include <OMX_Core.h>
#include <DrmApi.h>
#include <DrmApiExt.h>
#include <DivXDrmDecrypt.h>
#include "common_log.h"
 
class QComDivXDecrypt : public DivXDrmDecrypt
{
public:
    QComDivXDecrypt(){};
    virtual OMX_ERRORTYPE Init();
    virtual OMX_ERRORTYPE Decrypt(OMX_BUFFERHEADERTYPE* buffer);
    virtual ~QComDivXDecrypt();
private:
    uint8_t* iDrmContext;
};

//Exported factory function.
extern "C" DivXDrmDecrypt* createDivXDrmDecrypt() {
      return new QComDivXDecrypt;
}

OMX_ERRORTYPE QComDivXDecrypt::Init() {
    //Todo - check we're not already Init-ed
    uint32_t drmContextLength = 0;
    drmErrorCodes_t err = drmInitSystemEx(drmContextRoleDecryption, NULL, &drmContextLength);
    if((err!=DRM_SUCCESS) ||
        (drmContextLength==0)) {
        LOGE("Unable to get size of drmContext, err = %d", err);
        return OMX_ErrorUndefined;
    }

    iDrmContext = (uint8_t*)malloc(drmContextLength*sizeof(uint8_t));
    if(!iDrmContext) {
        LOGE("Unable to alloc drmContext");
        return OMX_ErrorUndefined;
    }

    err = drmInitSystemEx(drmContextRoleDecryption, iDrmContext, &drmContextLength);
    if(err!=DRM_SUCCESS) {
        LOGE("Unable to init drmContext, err = %d", err);
        return OMX_ErrorUndefined;
    }

    err = drmInitPlayback(iDrmContext, NULL);
    if(err!=DRM_SUCCESS) {
        LOGE("Unable to init playback, err = %d", err);
        return OMX_ErrorUndefined;
    }

    err = drmCommitPlayback(iDrmContext);
    if(err!=DRM_SUCCESS) {
        LOGE("Unable to commit playback, err = %d", err);
      return OMX_ErrorUndefined;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE QComDivXDecrypt::Decrypt(OMX_BUFFERHEADERTYPE* buffer) {
    //Todo - check input params.
    drmErrorCodes_t err = drmDecryptVideoEx(iDrmContext, (uint8_t*)(buffer->pBuffer), (uint32_t)(buffer->nSize));
    if(err!=DRM_SUCCESS) {
        //TODO - if we fail at decryting, should we stop decryting for this session?
        return OMX_ErrorUndefined;
    }

    LOGV("Successfully decrypted!");
    return OMX_ErrorNone;
}

QComDivXDecrypt::~QComDivXDecrypt() {
    if(iDrmContext) {
        drmFinalizePlayback(iDrmContext);
        delete iDrmContext;
    }
}
