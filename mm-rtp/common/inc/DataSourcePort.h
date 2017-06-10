#ifndef __DATASOURCEPORT_H__
#define __DATASOURCEPORT_H__
/************************************************************************* */
/**
 * @file DataSourcePort.h
 * @brief Header file for iSourcePort, iStreamPort interface definitions.
 *
 * Copyright (c) 2008 - 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/API/DataSourcePort/main/latest/inc/DataSourcePort.h#7 $
$DateTime: 2011/04/20 11:29:31 $
$Change: 1710564 $

========================================================================== */
/* =======================================================================
**               Include files for DataSourcePort.h
** ======================================================================= */
#include <AEEStdDef.h>
#include <stdlib.h>
#include "SourceBase.h"

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */
#define UNUSED(x) ((void)x)

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
/**
iSourcePort
The iSourcePort interface is a generic data source interface that provides
uni-directional data transfer (only read). This is designed to be generic
across all kinds of data sources (e.g. File Source, Network Source etc).
*/
class iSourcePort : public iSourceBase
{
public:
  /** iSourcePort interface ID
  */
  static const AEEIID SOURCEPORT_IID = 0x010732E0;

  /**
  Whence value for Seek() specifying the offset is relative to the
  start of the buffer.
  */
  static const int DS_SEEK_SET = 0;

  /**
  Whence value for Seek() specifying the offset is relative to the
  current position in the buffer.
  */
  static const int DS_SEEK_CUR = 1;

  /**
  Whence value for Seek() specifying the offset is relative to the
  end of the buffer.
  */
  static const int DS_SEEK_END = 2;

  /**
  Generic return codes for all iSourcePort APIs.
  */
  enum DataSourceReturnCode
  {
    DS_SUCCESS,
    DS_FAILURE,
    DS_WAIT
  };

  /**
  Data source type.
  */
  enum DataSourceType
  {
    DS_FILE_SOURCE,
    DS_STREAMING_SOURCE
  };

  /**
  iReadNotificationHandler is the interface that is registered with the data
  source, if the client chooses to be notified asynchronously when data becomes
  available after a buffer underrun.
  */
  class iReadNotificationHandler
  {
  public:
    virtual void Notify() = 0;
    virtual ~iReadNotificationHandler() {};
  };

  /**
  Read data into the specified buffer (of given size).
  return DS_SUCCESS - *pnRead indicates the amount of data read
                      (*pnRead = 0 and nBufSize > 0 signifies EOS)
         DS_WAIT    - no data available at the moment call Readable() to wait
         DS_FAILURE - generic read failure
  */
  virtual DataSourceReturnCode Read(/*rout*/ unsigned char* pBuf,
                                    /*in*/ ssize_t nBufSize,
                                    /*rout*/ ssize_t* pnRead) = 0;

  /**
  Register a callback interface to be invoked when data is available to be read
  (i.e. Read() would return something other than DS_WAIT).
  */
  virtual DataSourceReturnCode Readable(/*in*/ iReadNotificationHandler const* pNotificationHandler) = 0;

  /**
  Write data from the specified buffer (of given size).
  return DS_SUCCESS - *pnWritten indicates the amount of data written
         DS_FAILURE - generic write failure
  */
  virtual DataSourceReturnCode Write(/*in*/ const unsigned char* pBuf,
                                     /*in*/ ssize_t nBufSize,
                                     /*rout*/ ssize_t* pnWritten) = 0;

  /**
  Write data from the specified buffer (of given size). This API
   provides additional information if the current frame carries
   and end of a frame.This is particularly used in RTP where
   RTP provides framing for bitstreams
   return DS_SUCCESS
         - pnWritten indicates the amount of data written
           DS_FAILURE
         - generic write failure
  */
  virtual DataSourceReturnCode WriteBlockData(/*in*/ const unsigned char* pBuf,
                                     /*in*/ int nBufSize,
                                     /*in*/ int nTimeStamp,
                                     /*in*/ bool bEOF,
                                     /*rout*/ int64* pnWritten)
  {
    UNUSED(pBuf);
    UNUSED(nBufSize);
    UNUSED(nTimeStamp);
    UNUSED(bEOF);
    UNUSED(pnWritten);
    return video::iSourcePort::DS_FAILURE;};

  /**
  Repositions the read/write point in an open port to the specified offset.
  *pnOutOffset gives the offset after a successful seek.
  */
  virtual DataSourceReturnCode Seek(/*in*/ const int64 nOffset,
                                    /*in*/ const int nWhence,
                                    /*rout*/ int64* pnOutOffset) = 0;

  /**
  Close the data source port - port becomes unusable after this call.
  */
  virtual DataSourceReturnCode Close() = 0;

  /**
  Get the content length (in bytes).
  */
  virtual DataSourceReturnCode GetContentLength(/*rout*/ int64* pContentLength) = 0;

  /**
  Get the underlying data source type.
  */
  virtual DataSourceReturnCode GetSourceType(/*rout*/ DataSourceType* pSourceType) = 0;
  virtual ~iSourcePort(){};
};

/**
iStreamPort
The iStreamPort interface is the data source interface that derives from
iSourcePort and is specific to a stream source. The byte stream could be
delivered over a network (e.g. Progressive Download) or a file buffer.
*/
class iStreamPort : public iSourcePort
{
public:
  /** iStreamPort interface ID
  */
  static const AEEIID STREAMPORT_IID = 0x010732D3;

  /**
  Get the current available data size (in bytes).
  e.g. number of bytes downloaded for a network source.
  */
  virtual DataSourceReturnCode GetNumBytesAvailable(/*rout*/ int64* pNumBytesAvailable) = 0;

  /**
  Get the max available offset to read (in bytes).
  e.g. This is the same as number of bytes downloaded if data download is
  sequential (e.g. Progressive Download) else (e.g. FastTrack) it is the
  farthest download point in the buffer.
  *pbEOS will tell whether to expect any more data, avoiding a Read() call
  to learn EOS.
  */
  virtual DataSourceReturnCode GetAvailableOffset(/*rout*/ int64* pAvailableOffset,
                                                  /*rout*/ bool* pbEOS) = 0;

  /**
  Get the Lower Bound avaialble offset to read (in bytes).
  e.g. This is useful in case of Live streaming scenarios, where
  Download component will overwrite data into the buffer. It gives
  lower bound valid offset value.
   pbEOS will tell whether to expect any more data, avoiding a
  Read() call to learn EOS.
  */
  virtual DataSourceReturnCode GetBufferLowerBound(/*rout*/ int64* pAvailableOffset,
                                                  /*rout*/ bool* pbEOS) = 0;//{return DS_FAILURE;};

 /**
  Get the start offset to read (in bytes).
  e.g. This is 0 if data is stored in a linear buffer or file but can have
  a value > 0 in case of circular buffer kind of storage.*/
  virtual DataSourceReturnCode GetStartOffset(int64* pStartOffset)=0;

};

}/* namespace video */
#endif /* __DATASOURCEPORT_H__ */
