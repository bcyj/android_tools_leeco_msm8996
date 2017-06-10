/*==============================================================================
*        @file RTCPReceiver.cpp
*
*  @par DESCRIPTION:
*       This is the implementation of the RTCP Receiver.
*
*
*  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:

================================================================================
*/


#include "AEEstd.h"
#include "errno.h"
#include <unistd.h>
#include "RTCPReceiver.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include "MMTimer.h"

#define RTCP_STACK_SIZE 32768
#define RTCP_BUF_SIZE  65536

/*==============================================================================

         FUNCTION:         CRTCPReceiver

         DESCRIPTION:
*//**       @brief         constructor
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
CRTCPReceiver::CRTCPReceiver(int nSocket, RTPCallbackType pCallback, void *pUserData)
{
    m_nSockfd   = nSocket;
    m_pCallback = pCallback;
    m_pUserData = pUserData;

    /**-------------------------------------------------------------------------
       Create the thread for RTP packetization.
    ----------------------------------------------------------------------------
    */
    if ( 0 != MM_Thread_CreateEx( 20,
                                   0,
                                   CRTCPReceiver::RTCPThreadEntry,
                                   this,
                                   RTCP_STACK_SIZE,
                                   "RTCP", &m_pRTCPThread) )
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPReceiver::CRTPReceiver Thread create failed %x!!!",m_pRTCPThread);
        m_pRTCPThread = NULL;
        m_eState = RTCP_STATE_ERROR;
        return;
    }
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Thread create success!!!");
    }
    m_eState = RTCP_STATE_RUNNING;
}

/*==============================================================================

         FUNCTION:         RTCPThreadEntry

         DESCRIPTION:
*//**       @brief         Entry function to RTP Thread

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         void pointer to the const

*//*     RETURN VALUE:
*//**       @return
                           0


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
int CRTCPReceiver::RTCPThreadEntry( void* ptr )
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTCPReceiver::RTCPThreadEntry");
    CRTCPReceiver* pThis = (CRTCPReceiver *) ptr;

    if ( NULL != pThis )
    {
        pThis->RTCPThread();
    }
    return 0;
}

/*==============================================================================

         FUNCTION:         RTCPThread

         DESCRIPTION:
*//**       @brief         Actual processing function for RTCP thread

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
int CRTCPReceiver::RTCPThread( void )
{
    bool bRunning = true;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTCPReceiver::RTCPThread");
    uint32 *pEvent = NULL;

    if(m_eState != RTCP_STATE_RUNNING)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, 
                     "RTCPReceiver::RTCPThread Error, Exiting Early");
        MM_Thread_Exit( m_pRTCPThread, 0 );
        return 0;
    }

    uint8 *pRTCPBuffer = (uint8*)MM_Malloc(RTCP_BUF_SIZE);

    if(!pRTCPBuffer)
    {
        m_eState = RTCP_STATE_ERROR;
    }

    rtpGenericDatatype sData;
    int nBytesRead = 0;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000; 

    setsockopt((int)m_nSockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,(socklen_t)sizeof(struct timeval));

    while ( m_eState == RTCP_STATE_RUNNING )
    {
        nBytesRead = RecvPacket(pRTCPBuffer, RTCP_BUF_SIZE);

        /*RecvPacket attempts to read message from socket which returns
          number of bytes read or -1 on failure.*/
        if(nBytesRead > 0)
        {
            sData.pData = (char*)pRTCPBuffer;
            sData.nLen  = nBytesRead;

            if(RTCP_BUF_SIZE == nBytesRead)
            {
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTCP payload too large %d", nBytesRead);
                nBytesRead--;
            }
            pRTCPBuffer[nBytesRead] = 0;
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTCP payload recieved %d", nBytesRead);
            m_pCallback(RTP_RTCP_RR_EVENT, RTP_SUCCESS, (void *)(&sData), m_pUserData);
        }
        MM_Timer_Sleep(1);
    }

    if(pRTCPBuffer)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Reeing RTCP Buffer");
        MM_Free(pRTCPBuffer);
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Exiting RTCP thread");
    return 0;
}


/*==============================================================================

         FUNCTION:         RecvPacket

         DESCRIPTION:
*//**       @brief         Receive RTCP packets in UDP
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
int CRTCPReceiver::RecvPacket(uint8 *pData, uint32 nLen)
{
    int iRecvResult;

    iRecvResult = (int)recvfrom((int)m_nSockfd, pData, nLen, 0, NULL, NULL);

    return iRecvResult;

}


/*==============================================================================

         FUNCTION:         ~CRTCPReceiver

         DESCRIPTION:
*//**       @brief         Destructor cleans up threads and all
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
CRTCPReceiver::~CRTCPReceiver()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTCP destructor called");

    m_eState = RTCP_STATE_STOPPED;
    if(m_pRTCPThread)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTCP~Wait to exit thread");
        int exitCode = 0;
        MM_Thread_Join( m_pRTCPThread, &exitCode );
    }

    if(m_pRTCPThread)
    {
        MM_Thread_Release(m_pRTCPThread);
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTCP destructor done");
    return;
}

