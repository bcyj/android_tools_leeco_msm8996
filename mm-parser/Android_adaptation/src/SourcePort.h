/*
 * Copyright (c) 2010 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#ifndef DATA_PORT_H_
#define DATA_PORT_H_

#include <media/stagefright/DataSource.h>
#include "DataSourcePort.h"


using namespace video;

namespace android {

class SourcePort : public iStreamPort {
public:
    SourcePort(const sp<DataSource> &source);

    virtual void* QueryInterface(const AEEIID iid);
    virtual uint32 AddRef();
    virtual uint32 Release();

     /**
  Read data into the specified buffer (of given size).
  return DS_SUCCESS - *pnRead indicates the amount of data read
                      (*pnRead = 0 and nBufSize > 0 signifies EOS)
         DS_WAIT    - no data available at the moment call Readable() to wait
         DS_FAILURE - generic read failure
  */
  virtual DataSourceReturnCode Read(/*rout*/ unsigned char* pBuf,
                                    /*in*/ ssize_t nBufSize,
                                    /*rout*/ ssize_t* pnRead);

  /**
  Register a callback interface to be invoked when data is available to be read
  (i.e. Read() would return something other than DS_WAIT).
  */
  virtual DataSourceReturnCode Readable(/*in*/ iReadNotificationHandler const* pNotificationHandler);

  /**
  Write data from the specified buffer (of given size).
  return DS_SUCCESS - *pnWritten indicates the amount of data written
         DS_FAILURE - generic write failure
  */
  virtual DataSourceReturnCode Write(/*in*/ const unsigned char* pBuf,
                                     /*in*/ ssize_t nBufSize,
                                     /*rout*/ ssize_t* pnWritten);
     /**
  Repositions the read/write point in an open port to the specified offset.
  *pnOutOffset gives the offset after a successful seek.
  */
  virtual DataSourceReturnCode Seek(/*in*/ const int64 nOffset,
                                    /*in*/ const int nWhence,
                                    /*rout*/ int64* pnOutOffset);

   /**
  Close the data source port - port becomes unusable after this call.
  */
  virtual DataSourceReturnCode Close();

  /**
  Get the content length (in bytes).
  */
  virtual DataSourceReturnCode GetContentLength(/*rout*/ int64* pContentLength);

  /**
  Get the underlying data source type.
  */
  virtual DataSourceReturnCode GetSourceType(/*rout*/ DataSourceType* pSourceType);

  /**
  Get the current available data size (in bytes).
  e.g. number of bytes downloaded for a network source.
  */
  virtual DataSourceReturnCode GetNumBytesAvailable(/*rout*/ int64* pNumBytesAvailable);

  /**
  Get the max available offset to read (in bytes).
  e.g. This is the same as number of bytes downloaded if data download is
  sequential (e.g. Progressive Download) else (e.g. FastTrack) it is the
  farthest download point in the buffer.
  *pbEOS will tell whether to expect any more data, avoiding a Read() call
  to learn EOS.
  */
  virtual DataSourceReturnCode GetAvailableOffset(/*rout*/ int64* pAvailableOffset,
                                                  /*rout*/ bool* pbEOS);

 /**
  Get the start offset to read (in bytes).
  e.g. This is 0 if data is stored in a linear buffer or file but can have
  a value > 0 in case of circular buffer kind of storage.*/
  virtual DataSourceReturnCode GetStartOffset(int64* pStartOffset);

  virtual ~SourcePort();

private:
    sp<DataSource> mDataSource;
    off64_t mOffset;
    unsigned int mRefCnt;
};

}

#endif  // DATA_PORT_H_
