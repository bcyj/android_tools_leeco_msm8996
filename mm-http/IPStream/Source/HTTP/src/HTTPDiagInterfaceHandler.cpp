/************************************************************************* */
/**
 * HTTPDiagInterfaceHandler.cpp
 * @brief Implementation of HTTPDiagInterfaceHandler. This class implements methods
 * that log HTTP log packets with diag component.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPDiagInterfaceHandler.cpp#6 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDiagInterfaceHandler.cpp
** ======================================================================= */
#include "HTTPDiagInterfaceHandler.h"

#include <qtv_msg.h>
#include <IPStreamSourceUtils.h>
#include "string.h"

/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */
#define LOG_QTV_RTP_AUDIO_SAMPLE_C                      (0x172 + LOG_1X_BASE_C)
#define LOG_QTV_RTP_VIDEO_SAMPLE_C                      (0x173 + LOG_1X_BASE_C)
#define LOG_QTV_RTP_TEXT_SAMPLE_C                       (0x1B0 + LOG_1X_BASE_C)

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
/** @brief Log HTTP media sample.
  *
  * @param[in] codecType - Media codec type
  * @param[in] pBuf - Reference to sample data buffer
  * @param[in] size - Sample size
  * @param[in] sampleInfo - Reference to sample info
  */
void HTTPDiagInterfaceHandler::LogHTTPMediaSample
(
 const eCodecType codecType,
 const char* pBuf,
 const uint32 size,
 const tHTTPSampleInfo& sampleInfo
)
{
  if (pBuf == NULL || size == 0)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                   "Error: LogHTTPMediaSample %d - invalid input",
                   codecType );
  }
  else
  {
    tHTTPSampleLog* pLogPkt = NULL;

    //Reusing RTP log codes
    switch (codecType)
    {
    case AUDIO:
      pLogPkt = (tHTTPSampleLog *)LogAlloc(LOG_QTV_RTP_AUDIO_SAMPLE_C, size);
      break;

    case VIDEO:
      pLogPkt = (tHTTPSampleLog *)LogAlloc(LOG_QTV_RTP_VIDEO_SAMPLE_C, size);
      break;

    case TEXT:
      pLogPkt = (tHTTPSampleLog *)LogAlloc(LOG_QTV_RTP_TEXT_SAMPLE_C, size);
      break;

    default:
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Error: LogHTTPMediaSample - bad codec type %d", codecType );
      break;
    }

    if (pLogPkt)
    {
      //Fill the log packet (copy to log/diag memory)
      memcpy(&pLogPkt->sampleInfo, &sampleInfo, sizeof(sampleInfo));
      pLogPkt->size = size;
      memcpy(pLogPkt->content, pBuf, size);

      //Commit log packet
      LogCommit(pLogPkt);
    }
  }
}

/** @brief Log HTTP media sample.
  *
  * @param[in] stats - Reference to HTTP stats
  */
void HTTPDiagInterfaceHandler::LogHTTPStats
(
 const tHTTPStats& /* stats */
)
{
  //ToDo: Log HTTP stats
}

/** @brief Allocate memory for log packet.
  *
  * @param[in] logCode - Log packet code
  * @param[in] cnt - Number of bytes to log
  * @return Log packet
  */
void* HTTPDiagInterfaceHandler::LogAlloc
(
 const log_code_type logCode,
 const uint32 cnt
)
{
  void* pLogPkt = NULL;
  size_t logPktSize = 0;

  //Compute log pkt size based on log code
  switch (logCode)
  {
  case LOG_QTV_RTP_AUDIO_SAMPLE_C:
  case LOG_QTV_RTP_VIDEO_SAMPLE_C:
  case LOG_QTV_RTP_TEXT_SAMPLE_C:
    {
      logPktSize = FPOS(tHTTPSampleLog, content[0]);
      logPktSize += cnt * FSIZ(tHTTPSampleLog, content[0]);
      break;
    }
  case LOG_QTV_PDS2_STATS:
    {
      logPktSize = sizeof(tHTTPStatsLog);
      break;
    }
  default:
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Error: LogAlloc - bad log code %d", logCode );
      break;
    }
  }

  //Allocate log pkt memory
  if (logPktSize > 0)
  {
    pLogPkt = (void *) MM_Log_Alloc(logCode, logPktSize);
  }

  return pLogPkt;
}

/** @brief Commit log packet into diag.
  *
  * @param[in] pLogPkt - Reference to log packet to be committed
  */
void HTTPDiagInterfaceHandler::LogCommit
(
 void* pLogPkt
)
{
  (void)pLogPkt;
  MM_Log_Commit(pLogPkt);
}
