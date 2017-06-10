/*==============================================================================
*        @file RTPDataSource.h
*
*  @par DESCRIPTION:
*       This is the RTPDataSource class header file
*
*
*  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:

================================================================================
*/
#ifndef RTP_DATA_SOURCE_H

#define RTP_DATA_SOURCE_H

#define RTP_THREAD_STACK_SIZE 16384

#include "MMMemory.h"
#include "AEEstd.h"

#include <utils/Errors.h>

namespace android
{
  class RTPParser;

  class RTPDataSource
  {
   public:

    RTPDataSource(int32 port, int32 payloadType, boolean isTCPConnection = false, int rtpSock = -1);
    virtual ~RTPDataSource();

    virtual ssize_t readAt(int64 offset, void *data, size_t size);
    virtual status_t initCheck() const;
    status_t setCacheSize(size_t cacheSize);
    status_t start();
    status_t stop();
    status_t pause();
    status_t resume();
    void updateRTPPortVars();

    virtual status_t getSize(int64 *size);
    virtual status_t getHeadOffset(int64 *size);
    int64_t getRTPRefereceTime();
    status_t getRTPBaseTime(int64 *baseTimeUs);
    bool IsBaseTimeAdjusted(bool *bDiscontinuity, uint64 *pnTSBefore, uint64* pnTSAfter);

   private:

    enum RTPState
    {
      RTP_NO_INIT = 0,     //Component is not yet initialized
      RTP_INITIALIZED,      //Component is initialized
      RTP_FETCHING_DATA,   //Component starts fetching data
      RTP_STOPPED,         //Component stopped fetching data
    };

    RTPState m_eRTPState;

    //holds RTP packet data. Points to only header if recvmsg is being used
    uint8* m_pRTPPacket;

    RTPParser* m_pRTPParser;

    MM_HANDLE m_pRTPThread;

    MM_HANDLE m_pSignalQ;
    MM_HANDLE m_pFetchDataSignal;
    MM_HANDLE m_pExitSignal;
    MM_HANDLE m_pPauseSignal;
    MM_HANDLE m_pResumeSignal;


    /*
         *Following values are the user data corresponding to m_pFetchDataSignal, m_pExitSignal respectively
         */
    static const uint32 FETCH_DATA_SIGNAL; //user data for m_pFetchDataSignal
    static const uint32 THREAD_EXIT_SIGNAL;
    static const uint32 THREAD_PAUSE_SIGNAL;
    static const uint32 THREAD_RESUME_SIGNAL;



    //True indicates connection is over TCP else UDP
    boolean m_bTCPConnection;
    int32 m_nRTPSocket;
    int32 m_nTCPDataSocket;

    //Cache size in bytes
    int32 m_nCacheSize;

    //As part of Debug feature RTP_READ_FROM_FILE
    FILE *m_pLocalFile;

    static int rtpThreadEntry( void* ptr );
    void rtpThreadWorker();
    int32 fetchData();
    int32 receiveRTPPacket(uint8* rtpPacket);

    void init(int32 port, int32 payloadType, int rtpSock, bool tcp);
    int32 createSocket(int32 port);
    int32 setSocketOptions(int32 socket);
    int32 acceptTCPConnection(int32 tcpSocket);

    void closeDataSource();
  };
} //name space android

#endif /*RTP_DATA_SOURCE_H*/

