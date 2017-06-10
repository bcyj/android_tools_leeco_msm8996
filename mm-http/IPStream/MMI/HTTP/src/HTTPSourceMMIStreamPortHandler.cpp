/************************************************************************* */
/**
 * HTTPSourceMMIStreamPortHandler.cpp
 * @brief Implementation of HTTPSourceMMIStreamPortHandler.
 *  HTTPSourceMMIStreamPortHandler is a helper class for HTTPSourceMAPI that
 *  implements iStreamPort interface and uses Data Manager to retrieve data.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIStreamPortHandler.cpp#4 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMIStreamPortHandler.cpp
** ======================================================================= */
#include "HTTPSourceMMIStreamPortHandler.h"
#include "HTTPCommon.h"

namespace video {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief Get the number of bytes available (downloaded) for the current
  * HTTP session - delegate to data manager.
  *
  * @param[out] pNumBytesAvailable - Reference to number of bytes available
  * @return
  * DS_SUCCESS - Number of bytes downloaded obtained
  * DS_FAILURE - Otherwise
  */
iSourcePort::DataSourceReturnCode HTTPSourceMMIStreamPortHandler::GetNumBytesAvailable
(
 int64* pNumBytesAvailable
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIStreamPortHandler::GetNumBytesAvailable" );
  DataSourceReturnCode result = DS_FAILURE;

  if (pNumBytesAvailable)
  {
    *pNumBytesAvailable = 0;
    if (m_pHTTPDataBuffer)
    {
      result = m_pHTTPDataBuffer->GetNumBytesAvailable(pNumBytesAvailable);
    }
  }

  if (result != DS_SUCCESS)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Error: GetNumBytesAvailable failed %d", result );
  }

  return result;
}
/** @brief Get the start offset from the http download buffer
  * - delegate to data manager.
  *
  * @param[out] pStartOffset- Reference to start offser
  * @return
  * DS_SUCCESS - Start Offset obtained
  * DS_FAILURE - Otherwise
  */

iSourcePort::DataSourceReturnCode HTTPSourceMMIStreamPortHandler::GetStartOffset
(
 int64* pStartOffset
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIStreamPortHandler::GetStartOffset" );
  DataSourceReturnCode result = DS_FAILURE;

  if (pStartOffset)
  {
    *pStartOffset = 0;
    if (m_pHTTPDataBuffer)
    {
      result = m_pHTTPDataBuffer->GetStartOffset(pStartOffset);
    }
  }

  if (result != DS_SUCCESS)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Error: GetStartOffset failed %d", result );
  }

  return result;
}
/** @brief Get the max available (download) offset from the HTTP download
  * buffer - delegate to data manager.
  *
  * @param[out] pAvailableOffset - Reference to max available offset
  * @param[out] pbEOS - Reference to EOS flag
  * @return
  * DS_SUCCESS - Max download offset obtained
  * DS_FAILURE - Otherwise
  */
iSourcePort::DataSourceReturnCode HTTPSourceMMIStreamPortHandler::GetAvailableOffset
(
 int64* pAvailableOffset,
 bool* pbEOS
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIStreamPortHandler::GetAvailableOffset" );
  DataSourceReturnCode result = DS_FAILURE;

  if (pAvailableOffset && pbEOS)
  {
    *pAvailableOffset = 0;
    *pbEOS = false;
    if (m_pHTTPDataBuffer)
    {
      result = m_pHTTPDataBuffer->GetAvailableOffset(pAvailableOffset, pbEOS);
    }
  }

  if (result != DS_SUCCESS)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Error: GetAvailableOffset failed %d", result );
  }

  return result;
}

/** @brief Read specified number of bytes from the byte stream
  * - delegate to data manager.
  *
  * @param[out] pBuf - Read buffer
  * @param[in] nBufSize - Read buffer size
  * @param[out] pnRead - Reference to number of bytes read
  * @return
  * DS_SUCCESS - Read successful
  * DS_WAIT - Call Readable() and check back later
  * DS_FAILURE - Otherwise
  */
iSourcePort::DataSourceReturnCode HTTPSourceMMIStreamPortHandler::Read
(
 unsigned char* pBuf,
 ssize_t nBufSize,
 ssize_t* pnRead
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIStreamPortHandler::Read" );
 DataSourceReturnCode result = DS_FAILURE;

  if (pBuf && pnRead)
  {
    *pnRead = 0;
    if (m_pHTTPDataBuffer)
    {
      result = m_pHTTPDataBuffer->Read(pBuf, nBufSize, pnRead);
    }
  }

  if (result != DS_SUCCESS && result != DS_WAIT)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Error: Read failed %d", result );
  }

  return result;
}

/** @brief Register Read signal, to be called when data is available
  * to be read - delegate to data manager.
  *
  * @param[in] pNotificationHandler - Reference to Read notification
  *                                   handler iface
  * @return
  * DS_SUCCESS - Read notification handler successfully registered
  * DS_FAILURE - Otherwise
  */
iSourcePort::DataSourceReturnCode HTTPSourceMMIStreamPortHandler::Readable
(
 iReadNotificationHandler const* pNotificationHandler
)
{
   QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIStreamPortHandler::Readable" );
  DataSourceReturnCode result = DS_FAILURE;

  if (pNotificationHandler)
  {
    if (m_pHTTPDataBuffer)
    {
      result = m_pHTTPDataBuffer->Readable(pNotificationHandler);
    }
  }

  if (result != DS_SUCCESS)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Error: Readable failed %d", result );
  }

  return result;
}

/** @brief Seek to the specified offset in the byte stream
  * - delegate to data manager.
  *
  * @param[in] nOffset - Seek position in the byte stream
  * @param[in] nWhence - Relative position in the byte stream
  * @param[out] pnOutOffset - Reference to current (read) offset
  * @return
  * DS_SUCCESS - Seek successful
  * DS_FAILURE - Otherwise
  */
iSourcePort::DataSourceReturnCode HTTPSourceMMIStreamPortHandler::Seek
(
 const int64 nOffset,
 const int nWhence,
 int64* pnOutOffset
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIStreamPortHandler::Seek" );
  DataSourceReturnCode result = DS_FAILURE;

  if (pnOutOffset)
  {
    *pnOutOffset = 0;
    if (m_pHTTPDataBuffer)
    {
      result = m_pHTTPDataBuffer->Seek(nOffset, nWhence, pnOutOffset);
    }
  }

  if (result != DS_SUCCESS)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Error: Seek failed %d", result );
  }

  return result;
}

/** @brief Close the byte stream (must be called ONLY when no more data
  * is intended to be read) - delegate to data manager.
  *
  * @return
  * DS_SUCCESS - Port closed successfully
  * DS_FAILURE - Otherwise
  */
iSourcePort::DataSourceReturnCode HTTPSourceMMIStreamPortHandler::Close()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIStreamPortHandler::Close" );
  DataSourceReturnCode result = DS_FAILURE;

  if (m_pHTTPDataBuffer)
  {
    result = m_pHTTPDataBuffer->Close();
  }

  if (result != DS_SUCCESS)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Error: Close failed %d", result );
  }

  return result;
}

/** @brief Get total size of the file being downloaded (-1 if unknown)
  * - delegate to data manager.
  *
  * @param[out] pContentLength - Reference to content length
  * @return
  * DS_SUCCESS - File size obtained
  * DS_FAILURE - Otherwise
  */
iSourcePort::DataSourceReturnCode HTTPSourceMMIStreamPortHandler::GetContentLength
(
 int64* pContentLength
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIStreamPortHandler::GetContentLength" );
  DataSourceReturnCode result = DS_FAILURE;

  if (pContentLength)
  {
    if (m_pHTTPDataBuffer)
    {
      result = m_pHTTPDataBuffer->GetContentLength(pContentLength);
    }
  }

  if (result != DS_SUCCESS)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Error: GetContentLength failed %d", result );
  }

  return result;
}

/** @brief Get underlying data source type.
  *
  * @param[out] pSourceType - Reference to source type
  * @return
  * DS_SUCCESS - Data source type obtained
  * DS_FAILURE - Otherwise
  */
iSourcePort::DataSourceReturnCode HTTPSourceMMIStreamPortHandler::GetSourceType
(
 DataSourceType* pSourceType
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMIStreamPortHandler::GetSourceType" );
  DataSourceReturnCode result = DS_FAILURE;

  if (pSourceType)
  {
    *pSourceType = iSourcePort::DS_STREAMING_SOURCE;
    result = DS_SUCCESS;
  }

  if (result != DS_SUCCESS)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Error: GetSourceType failed %d", result );
  }

  return result;
}

void* HTTPSourceMMIStreamPortHandler::QueryInterface(const AEEIID /*iid*/)
{
  //stub
  return NULL;
}
uint32 HTTPSourceMMIStreamPortHandler::AddRef()
{
  //stub
  return 0;
}
uint32 HTTPSourceMMIStreamPortHandler::Release()
 {
   //stub
   return 0;
 }

}/* namespace video */
