#ifndef __HTTPDIAGINTERFACEHANDLER_H__
#define __HTTPDIAGINTERFACEHANDLER_H__
/************************************************************************* */
/**
 *HTTPDiagInterfaceHandler.h
 * @brief Header file for HTTPDiagInterfaceHandler.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPDiagInterfaceHandler.h#4 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDiagInterfaceHandler.h
** ======================================================================= */
#include <AEEstd.h>
#include <MMDebugMsg.h>

/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data and type Declarations
** ----------------------------------------------------------------------- */
//Log code type - currently a 16 bit unsigned integer
typedef uint16 log_code_type;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

class HTTPDiagInterfaceHandler
{
public:

  enum eCodecType
  {
    UNKNOWN,
    AUDIO,
    VIDEO,
    TEXT
  };

#if defined(WINCE)
  #pragma pack(push, log_h )
  #pragma pack(1)
#endif

  //Structure definitions for HTTP sample log(s)
  struct tHTTPSampleInfo
  {
    uint8 trackID; //Media track ID
    uint32 absTS; //Absolute time (in media time scale)
    uint32 dispTS;  //Display time (converted absTS in msec)
    uint8 layerID;  //Layer ID (non-zero for samples from enhanced (video) layer else set to 0
    uint32 duration; //Sample duration (msec)
  };

  struct tHTTPSampleLog
  {
    log_hdr_type hdr; //Log header (length, code, timestamp)
    tHTTPSampleInfo sampleInfo; //Sample info
    uint32 size;  //Sample size
    char content[1]; //Sample data
  };

  //Structure definitions for HTTP stats log(s)
  struct tHTTPStats
  {
    int dataRate; //Incoming data rate (in bps)
    int clipStopTime; //Total playback time of the clip (in msec)
    int downloadTime; //Current download time - current buffered data in time scale (in msec)
    int bufCap; //Buffer capacity (in bytes)
    int currBufSize; //Current amount of data present in the download buffer (in bytes)
  };

  struct tHTTPStatsLog
  {
    log_hdr_type hdr; //Log header (length, code, timestamp)
    tHTTPStats stats; //Stats info
  };

#if defined(WINCE)
  #pragma pack( pop, log_h )
#endif

  static void LogHTTPMediaSample(const eCodecType codecType,
                                 const char* pBuf,
                                 const uint32 size,
                                 const tHTTPSampleInfo& sampleInfo);
  static void LogHTTPStats(const tHTTPStats& stats);

private:
  static void* LogAlloc(const log_code_type logCode, const uint32 cnt = 0);
  static void LogCommit(void* pLogPkt);
};
#endif /* __HTTPDIAGINTERFACEHANDLER_H__ */
