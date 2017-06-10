/*==============================================================================
  *       RTPStreamPort.cpp
  *
  *  DESCRIPTION:
  *       RTP stream port implementation
  *
  *
  *  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Inc. All Rights Reserved.
  *  Qualcomm Technologies Proprietary and Confidential.
  *==============================================================================*/

/* =======================================================================
                                   Edit History
      ========================================================================== */

/* =======================================================================
      **               Includes and Public Data Declarations
      ** ======================================================================= */

/* ==========================================================================

                           INCLUDE FILES FOR MODULE

      ========================================================================== */

#include "RTPStreamPort.h"
#include "MMMemory.h"

/* ==========================================================================

                              DATA DECLARATIONS

      ========================================================================== */
/* -----------------------------------------------------------------------
      ** Constant / Define Declarations
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

/* =======================================================================
      **                          Macro Definitions
      ** ======================================================================= */

/* =======================================================================
 **                            Function Definitions
 ** ======================================================================= */
RTPStreamPort::RTPStreamPort(int32 rtpPort, bool bTCPConnection, int rtpSock, int rtcpsock)
 :m_pRTPDataSource(NULL),
  mOffset(0)
{
  UNUSED(rtcpsock);
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPStreamPort:constructor");
  CreateRTPDataSource(rtpPort, bTCPConnection, rtpSock);
}

RTPStreamPort::~RTPStreamPort()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPStreamPort:destructor");

  if(m_pRTPDataSource != NULL)
  {
    MM_Delete(m_pRTPDataSource);
    m_pRTPDataSource = NULL;
  }
}

iStreamPort::DataSourceReturnCode RTPStreamPort::Start()
{
  iStreamPort::DataSourceReturnCode nRet = DS_FAILURE;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPStreamPort:Start");
  if( m_pRTPDataSource != NULL )
  {
    android::status_t eRet = m_pRTPDataSource->start();
    if ( android::OK == eRet )
    {
      nRet = DS_SUCCESS;
    }
  }
  return nRet;
}

iStreamPort::DataSourceReturnCode RTPStreamPort::Read(/*rout*/ unsigned char* pBuf,
                              /*in*/ ssize_t nBufSize,
                              /*rout*/ ssize_t* pnRead)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "RTPStreamPort:Read");
  if (!pBuf)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPStreamPort:Read:Buffer is NULL");
    return DS_FAILURE;
  }
  //memset(pBuf, 0, nBufSize);
  ssize_t nRet  = m_pRTPDataSource->readAt(mOffset, (void*)pBuf, nBufSize);
  *pnRead = (int)nRet;
  return DS_SUCCESS;
}

void RTPStreamPort::updateRTPPortVars()
{
    m_pRTPDataSource->updateRTPPortVars();
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"RTPStreamPort: Reset parser variables");
}

iStreamPort::DataSourceReturnCode RTPStreamPort::Seek(/*in*/ const int64 nOffset,
                                                   /*in*/ const int nWhence,
                                                   /*rout*/ int64* pnOutOffset) {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "RTPStreamPort:Seek with nwhence %d", nWhence);
    int64 nOrigin = 0;
    switch (nWhence) {
    case DS_SEEK_SET:
        nOrigin = 0;
        break;
    case  DS_SEEK_CUR:
        nOrigin = mOffset;
        break;
    case DS_SEEK_END:
        m_pRTPDataSource->getSize(&nOrigin);
        break;
    default:
        break;
    }

    *pnOutOffset = mOffset = (nOffset + nOrigin);
    return DS_SUCCESS;
}

iStreamPort::DataSourceReturnCode RTPStreamPort::GetRTPBaseTimeUs(/*rout*/ int64* pBaseTimeUs)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "RTPStreamPort:GetRTPBaseTimeUs");
  android::status_t eRet = m_pRTPDataSource->getRTPBaseTime(pBaseTimeUs);
  return (android::OK == eRet ? DS_SUCCESS : DS_FAILURE);
}

bool RTPStreamPort::IsBaseTimeAdjusted(bool *bDiscontinuity, uint64 *pnTSBefore, uint64* pnTSAfter)
{
  return m_pRTPDataSource? m_pRTPDataSource->IsBaseTimeAdjusted(bDiscontinuity, pnTSBefore, pnTSAfter): false;
}

iStreamPort::DataSourceReturnCode RTPStreamPort::GetContentLength(/*rout*/ int64* pContentLength)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "RTPStreamPort:GetContentLength");
  if(pContentLength != NULL)
  {
    //assigining all fs to size
    *pContentLength = -1;
  }
  return DS_SUCCESS;
}

iStreamPort::DataSourceReturnCode RTPStreamPort::GetNumBytesAvailable(/*rout*/ int64* pNumBytesAvailable)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "RTPStreamPort:GetNumBytesAvailable");
  android::status_t eRet = m_pRTPDataSource->getSize(pNumBytesAvailable);
  return (android::OK == eRet ? DS_SUCCESS : DS_FAILURE);
}

iStreamPort::DataSourceReturnCode RTPStreamPort::GetAvailableOffset(/*rout*/ int64* pAvailableOffset,
                                          /*rout*/ bool* pbEOS)
{
  android::status_t eRet = m_pRTPDataSource->getSize(pAvailableOffset);
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG,
               "RTPStreamPort:GetAvailableOffset [%lld]",*pAvailableOffset);
  *pbEOS = FALSE;
  return (android::OK == eRet ? DS_SUCCESS : DS_FAILURE);
}

iStreamPort::DataSourceReturnCode RTPStreamPort::GetBufferLowerBound(/*rout*/ int64* pAvailableOffset,
                                          /*rout*/ bool* pbEOS)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "RTPStreamPort:GetBufferLowerBound");
  android::status_t eRet = m_pRTPDataSource->getHeadOffset(pAvailableOffset);
  *pbEOS = FALSE;
  return (android::OK == eRet ? DS_SUCCESS : DS_FAILURE);
}

iStreamPort::DataSourceReturnCode RTPStreamPort::GetStartOffset(int64* pStartOffset)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "RTPStreamPort:GetStartOffset");
  if(pStartOffset != NULL)
  {
    *pStartOffset = 0;
  }
  return DS_SUCCESS;
}

iStreamPort::DataSourceReturnCode RTPStreamPort::GetSourceType(
                 /*rout*/ DataSourceType* pSourceType)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "RTPStreamPort:GetSourceType");
  *pSourceType = DS_STREAMING_SOURCE;
  return DS_SUCCESS;
}

iStreamPort::DataSourceReturnCode RTPStreamPort::Close()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPStreamPort:Close");
  return DS_SUCCESS;
}

/*==========================================================================
   FUNCTION     : createRTPDataSource

   DESCRIPTION:

   DEPENDENCIES:

   PARAMETERS :

   Return Value  :
  ===========================================================================*/

void RTPStreamPort::CreateRTPDataSource(int32 rtpPort, bool bTCPConnection, int rtpSock)
{
    //MPEG2TS payload format accoding to RTP Payload types specified in tables 4,5 of RFC 3551
    int payloadType = 33;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPStreamPort:createRTPDataSource");
    m_pRTPDataSource = MM_New_Args(android::RTPDataSource, (rtpPort, payloadType, bTCPConnection, rtpSock));
}

iStreamPort::DataSourceReturnCode RTPStreamPort::Pause()
{
  iStreamPort::DataSourceReturnCode nRet = DS_FAILURE;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPStreamPort:pause");
  if( m_pRTPDataSource != NULL )
  {
    android::status_t eRet = m_pRTPDataSource->pause();
    if ( android::OK == eRet )
    {
      nRet = DS_SUCCESS;
    }
  }
  return nRet;
}

iStreamPort::DataSourceReturnCode RTPStreamPort::Resume()
{
  iStreamPort::DataSourceReturnCode nRet = DS_FAILURE;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPStreamPort:resume");
  if( m_pRTPDataSource != NULL )
  {
    android::status_t eRet = m_pRTPDataSource->resume();
    if ( android::OK == eRet )
    {
      nRet = DS_SUCCESS;
    }
  }
  return nRet;
}


