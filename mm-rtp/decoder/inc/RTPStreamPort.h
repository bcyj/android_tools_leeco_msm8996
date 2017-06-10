#ifndef __RTP_STREAM_PORT_H__
#define __RTP_STREAM_PORT_H__
/*==============================================================================
  *       RTPStreamPort.h
  *
  *  DESCRIPTION:
  *
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
#include "MMDebugMsg.h"
#include "AEEstd.h"

#include "DataSourcePort.h"
#include "RTPDataSource.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
using namespace video;
/* -----------------------------------------------------------------------
** Forward Declarations
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

#define UNUSED(x) ((void)x)

/* =======================================================================
**                        Class Declarations
** ======================================================================= */
class RTPStreamPort : public iStreamPort
{
  public:
     RTPStreamPort(int32 rtpPort, bool bTCPConnection = false, int rtpsock = -1, int rtcpsock = -1);

     virtual ~RTPStreamPort();
     virtual DataSourceReturnCode Read(/*rout*/ unsigned char* pBuf,
                                    /*in*/ ssize_t nBufSize,
                                    /*rout*/ ssize_t* pnRead);

     DataSourceReturnCode GetRTPBaseTimeUs(/*rout*/ int64* pBaseTimeUs);
     bool IsBaseTimeAdjusted(bool *bDiscontinuity, uint64 *pnTSBefore, uint64* pnTSAfter);
     virtual DataSourceReturnCode GetContentLength(/*rout*/ int64* pContentLength);
     virtual DataSourceReturnCode GetNumBytesAvailable(/*rout*/ int64* pNumBytesAvailable);
     virtual DataSourceReturnCode GetAvailableOffset(/*rout*/ int64* pAvailableOffset,
                                                /*rout*/ bool* pbEOS);
     virtual DataSourceReturnCode GetBufferLowerBound(/*rout*/ int64* pAvailableOffset,
                                                /*rout*/ bool* pbEOS);

     virtual DataSourceReturnCode GetStartOffset(int64* pStartOffset);
     virtual DataSourceReturnCode GetSourceType(
                       /*rout*/ DataSourceType* pSourceType);
     virtual DataSourceReturnCode Close();
     virtual DataSourceReturnCode Seek(/*in*/ const int64 nOffset,
                                    /*in*/ const int nWhence,
                                    /*rout*/ int64* pnOutOffset);



     void* QueryInterface(const AEEIID iid) {UNUSED(iid);return NULL;};
     uint32 AddRef() {return 0;};
     uint32 Release() {return 0;};

     virtual DataSourceReturnCode Readable(
      /*in*/ iReadNotificationHandler const* pNotificationHandler)
     {
      UNUSED(pNotificationHandler);
      return DS_FAILURE;
     };

     virtual DataSourceReturnCode Write(/*in*/   const unsigned char* pBuf,
                             /*in*/   ssize_t nBufSize,
                             /*rout*/ ssize_t* pnWritten)
     {
       UNUSED(pBuf);
       UNUSED(nBufSize);
       UNUSED(pnWritten);
       return DS_FAILURE;
     };

     DataSourceReturnCode Start();
     DataSourceReturnCode Pause();
     DataSourceReturnCode Resume();
     void updateRTPPortVars();

  private:
     android::RTPDataSource* m_pRTPDataSource;
     int64 mOffset;
     void CreateRTPDataSource(int32 rtpPort, bool bTCPConnection, int rtpSock);
};
#endif /*__RTP_STREAM_PORT_H__*/
