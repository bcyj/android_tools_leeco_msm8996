/*
 * Copyright (c) 2010 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "SourcePort"
#include <utils/Log.h>
#include "common_log.h"


#include "SourcePort.h"

namespace android {

SourcePort::SourcePort(const sp<DataSource> &source)
    : mDataSource(source) {
    mOffset = 0;
    mRefCnt = 0;
}

SourcePort::~SourcePort() {
}

/**
 Read data into the specified buffer (of given size).
 return DS_SUCCESS - *pnRead indicates the amount of data read
                     (*pnRead = 0 and nBufSize > 0 signifies EOS)
        DS_WAIT    - no data available at the moment call Readable() to wait
        DS_FAILURE - generic read failure
 */
iStreamPort::DataSourceReturnCode SourcePort::Read(/*rout*/ unsigned char* pBuf,
                                                   /*in*/ ssize_t nBufSize,
                                                   /*rout*/ ssize_t* pnRead){
    if (!pBuf) {
        LOGE(" Error :: Buffer is NULL \n");
        return DS_FAILURE;
    }
    memset(pBuf, 0, nBufSize);
    ssize_t nRet  = mDataSource->readAt(mOffset, (void*)pBuf, (size_t)nBufSize);
    if (nRet == -EAGAIN) {
        *pnRead = 0;
        LOGV("SourcePort::read under-run @ %lld ",mOffset );
        return DS_WAIT;
    }
    else if (nRet <= 0) {
        *pnRead = 0;
        LOGV("SourcePort::read failed @ %lld with error code %ld ",mOffset, nRet);
        return DS_FAILURE;
    }
    *pnRead = (ssize_t)nRet;
    return DS_SUCCESS;
}

/**
Repositions the read/write point in an open port to the specified offset.
*pnOutOffset gives the offset after a successful seek.
*/
iStreamPort::DataSourceReturnCode SourcePort::Seek(/*in*/ const int64 nOffset,
                                                   /*in*/ const int nWhence,
                                                   /*rout*/ int64* pnOutOffset) {
    // TODO : check whence
    int64 nOrigin = 0;
    switch (nWhence) {
    case DS_SEEK_SET:
        nOrigin = 0;
        break;
    case  DS_SEEK_CUR:
        nOrigin = mOffset;
        break;
    case DS_SEEK_END:
        mDataSource->getSize((off64_t*)nOrigin);
        break;
    default:
        break;
    }

    *pnOutOffset = mOffset = (nOffset + nOrigin);
    //LOGV("SourcePort::Seek nOffset = %lld nWhence = %d pnOutOffset = %lld\n",nOffset, nWhence, *pnOutOffset );
    return DS_SUCCESS;
}

/**
 Get the content length (in bytes).
 */
iStreamPort::DataSourceReturnCode SourcePort::GetContentLength(/*rout*/ int64* pContentLength){
   status_t eRet = mDataSource->getSize((off64_t*)pContentLength);
   return (OK == eRet ? DS_SUCCESS : DS_FAILURE);

}

/**
Get the underlying data source type.
*/
iStreamPort::DataSourceReturnCode SourcePort::GetSourceType(/*rout*/ DataSourceType* pSourceType){
    if (mDataSource->flags() &
        (DataSource::kWantsPrefetching | DataSource::kIsCachingDataSource)) {
        *pSourceType = DS_STREAMING_SOURCE;
    } else {
        *pSourceType = DS_FILE_SOURCE;
    }
    return DS_SUCCESS;
}

iStreamPort::DataSourceReturnCode SourcePort::GetNumBytesAvailable(/*rout*/ int64* pNumBytesAvailable){
    status_t eRet = mDataSource->getSize((off64_t*)pNumBytesAvailable);
    return (OK == eRet ? DS_SUCCESS : DS_FAILURE);
}

iStreamPort::DataSourceReturnCode SourcePort::GetStartOffset(/*rout*/ int64* pStartOffset){
   if(pStartOffset == NULL)
   {
      return DS_FAILURE;
   }
   else
   {
      *pStartOffset = 0;
   }
   return DS_SUCCESS;
}

iStreamPort::DataSourceReturnCode SourcePort::GetAvailableOffset(/*rout*/ int64* pAvailableOffset,
                                        /*rout*/ bool* pbEOS){

   // NOTE: chaning, reason being if the pbEOS is false the caller thinks the entire data is not available
   // in android file playback is treated as streaming, as streamport is used.
   //*pbEOS = false;
    *pbEOS = true;

    status_t eRet = mDataSource->getSize((off64_t*)pAvailableOffset);
    LOGV(" SourcePort::GetAvailableOffset pAvailableOffset = %ld \n", *pAvailableOffset);
    // If DataSource is not able to provide stream length, then reset with -1
    if((OK != eRet) || (0 == *pAvailableOffset))
    {
        eRet = OK;
        *pAvailableOffset = -1;
    }
    return (OK == eRet ? DS_SUCCESS : DS_FAILURE);
}

 /**
  Register a callback interface to be invoked when data is available to be read
 *(i.e. Read() would return something other than DS_WAIT).
  */
iStreamPort::DataSourceReturnCode SourcePort::Readable(
  /*in*/ iReadNotificationHandler const* /*pNotificationHandler*/){
    return DS_SUCCESS;
}

  /**
  Write data from the specified buffer (of given size).
  return DS_SUCCESS - *pnWritten indicates the amount of data written
  *      DS_FAILURE - generic write failure
  */
iStreamPort::DataSourceReturnCode SourcePort::Write(/*in*/ const unsigned char* /*pBuf*/,
                                     /*in*/ ssize_t /*nBufSize*/,
                                     /*rout*/ ssize_t* /*pnWritten*/){
    return DS_SUCCESS;
}

void* SourcePort::QueryInterface(const AEEIID /*iid*/){

    return this;
}

uint32 SourcePort::AddRef(){
    return ++mRefCnt;
}

uint32 SourcePort::Release(){
    return --mRefCnt;
}

 /**
  Close the data source port - port becomes unusable after this call.
  */
iStreamPort::DataSourceReturnCode SourcePort::Close(){
    return DS_SUCCESS;
}

}  // namespace android

