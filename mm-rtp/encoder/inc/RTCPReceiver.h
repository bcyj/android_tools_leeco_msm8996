#ifndef _RTCPRECEIVER_H
#define _RTCPRECEIVER_H
/* =======================================================================
                              RTCPReceiver.h
DESCRIPTION
  Test class definition of IStreamPort based output interface


  Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */


/* =======================================================================
                             Edit History

$Header:

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <stdio.h>
#include "RTPEncoder.h"
#include "MMDebugMsg.h"
#include "MMMalloc.h"
#include "MMThread.h"

typedef enum RTCPState
{
    RTCP_STATE_RUNNING,
    RTCP_STATE_ERROR,
    RTCP_STATE_STOPPED,
    RTCP_STATE_IDLE
}RTCPStateType;



class CRTCPReceiver
{
public:
    CRTCPReceiver(int nSocket, RTPCallbackType pCallback, void *pUserData);


    ~CRTCPReceiver();

private:
    static int RTCPThreadEntry(void *);
    int RTCPThread(void);
    int RecvPacket(uint8 *pData, uint32 nLen);

    RTCPStateType   m_eState;
    RTPCallbackType m_pCallback;
    void*           m_pUserData;
    MM_HANDLE       m_pRTCPThread;
    int32           m_nSockfd;
};

#endif //_RTCPRECEIVER_H
