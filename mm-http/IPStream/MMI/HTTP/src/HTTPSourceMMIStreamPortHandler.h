#ifndef __HTTPSOURCEMMISTREAMPORTHANDLER_H__
#define __HTTPSOURCEMMISTREAMPORTHANDLER_H__
/************************************************************************* */
/**
 * HTTPSourceMMIStreamPortHandler.h
 * @brief Header file for HTTPSourceMMIStreamPortHandler.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIStreamPortHandler.h#4 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMIStreamPortHandler.h
** ======================================================================= */
#include <stddef.h>
#include <sys/types.h>
#include <DataSourcePort.h>

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class HTTPSourceMMIStreamPortHandler : public iStreamPort
{
//Constructors/destructor
public:
  explicit HTTPSourceMMIStreamPortHandler()
           : m_pHTTPDataBuffer(NULL){ };
  virtual ~HTTPSourceMMIStreamPortHandler(){ };

//iStreamPort methods - Streaming specific data delivery
public:
  virtual DataSourceReturnCode GetNumBytesAvailable(int64* pNumBytesAvailable);
  virtual DataSourceReturnCode GetAvailableOffset(int64* pAvailableOffset,
                                                  bool* pbEOS);
  virtual DataSourceReturnCode GetStartOffset(int64* pStartOffset);

//iSourcePort methods - Generic data delivery and port control
public:
  virtual DataSourceReturnCode Read(unsigned char* pBuf, ssize_t nBufSize, ssize_t* pnRead);
  virtual DataSourceReturnCode Readable(iReadNotificationHandler const* pNotificationHandler);
  virtual DataSourceReturnCode Write(const unsigned char* /* pBuf */,
                                     ssize_t /* nBufSize */,
                                     ssize_t* /* pnWritten */)
  {
    return DS_FAILURE;
  };
  virtual DataSourceReturnCode Seek(const int64 nOffset, const int nWhence, int64* pnOutOffset);
  virtual DataSourceReturnCode Close();
  virtual DataSourceReturnCode GetContentLength(int64* pContentLength);
  virtual DataSourceReturnCode GetSourceType(DataSourceType* pSourceType);

  virtual void* QueryInterface(const AEEIID iid);
  virtual uint32 AddRef();
  virtual uint32 Release();

public:
  void SetDataDownloadBuffer(iStreamPort* pBuffer)
  {
    m_pHTTPDataBuffer = pBuffer;
  };

private:
  iStreamPort* m_pHTTPDataBuffer;
};

}/* namespace video */

#endif /* __HTTPSOURCEMMISTREAMPORTHANDLER_H__ */
