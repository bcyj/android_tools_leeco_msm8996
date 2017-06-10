/* =============================================================================
                             RTPEncoder.cpp
  DESCRIPTION
  Implementation of RTPEncoder class


  Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
================================================================================
*/


/* =============================================================================
                             Edit History

$Header:

================================================================================
*/

/* =============================================================================
**               Includes and Public Data Declarations
** =============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/
#include "RTPEncoder.h"
#include "RTPPacketizer.h"
#include "MMMemory.h"
#include "MMDebugMsg.h"



/*==============================================================================

         FUNCTION:         CRTPPacketizer

         DESCRIPTION:
*//**       @brief         CRTPEncoder constructor
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         Port Number
                           Destination IP address as uint32
                           Stream bitrate

*//*     RETURN VALUE:
*//**       @return
                           DS_FAILURE
                           DS_SUCCESS


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
CRTPEncoder::CRTPEncoder(uint32 nPortNum, uint32 nDestIP, uint32 nStreamBitrate,uint8 bRtpPortTypeUdp)
{
    m_pPacketizer =
          MM_New_Args(CRTPPacketizer,(nPortNum, nDestIP, nStreamBitrate,bRtpPortTypeUdp));

    if(m_pPacketizer && m_pPacketizer->IsOK())
    {
        m_eRTPEncState = RTP_ENC_READY;
    }
    else
    {
        m_eRTPEncState = RTP_ENC_ERROR;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                 "CRTPEncoder::  RTPPacketizer failed to Init");
    }
}

/*==============================================================================

         FUNCTION:         CRTPEncoder

         DESCRIPTION:
*//**       @brief         CRTPEncoder constructor
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         Port Number
                           Destination IP address as uint32
                           Stream bitrate

*//*     RETURN VALUE:
*//**       @return
                           DS_FAILURE
                           DS_SUCCESS


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
CRTPEncoder::CRTPEncoder(RTPNetworkInfoType *pNetWorkInfo, uint32 nStreamBitrate)
{
    networkInfoType networkInfo = {0,0,0,0};

    networkInfo.bRtpPortTypeUdp = pNetWorkInfo->bRtpPortTypeUdp;
    networkInfo.nIPAddress      = pNetWorkInfo->DestIp;
    networkInfo.PortNum0        = pNetWorkInfo->Port0;
    networkInfo.nSocket         = pNetWorkInfo->nSocket;

    m_pPacketizer =
          MM_New_Args(CRTPPacketizer,(&networkInfo, nStreamBitrate));


    if(m_pPacketizer && m_pPacketizer->IsOK())
    {
        m_eRTPEncState = RTP_ENC_READY;

    }
    else
    {
        m_eRTPEncState = RTP_ENC_ERROR;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                 "CRTPEncoder::  RTPPacketizer failed to Init");
    }
}


/*==============================================================================

         FUNCTION:         ~CRTPPacketizer

         DESCRIPTION:
*//**       @brief         CRTPPacketizer destructor
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           DS_FAILURE
                           DS_SUCCESS


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
CRTPEncoder::~CRTPEncoder()
{
    if(m_pPacketizer)
    {
        MM_Delete(m_pPacketizer);
    }
}


/*==============================================================================

         FUNCTION:         Write

         DESCRIPTION:
*//**       @brief         Write the stream of TS packets
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         Buffer
                           Buffer size
                           Num bytes written

*//*     RETURN VALUE:
*//**       @return
                           DS_FAILURE
                           DS_SUCCESS


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
iSourcePort::DataSourceReturnCode CRTPEncoder::Write(
                                     /*in*/   const unsigned char* pBuf,
                                     /*in*/   ssize_t nBufSize,
                                     /*rout*/ ssize_t* pnWritten)
{


    *pnWritten = 0;

    if(!pBuf || !nBufSize)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "CRTPEncoder::Write Invalid arguments");
        return DS_FAILURE;
    }

    if(m_eRTPEncState != RTP_ENC_READY)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "CRTPEncoder::Write RTPEncoder Bad state");
        return DS_FAILURE;
    }

    *pnWritten = (int)m_pPacketizer->Encode(pBuf, (uint32)nBufSize, true);

    if(!*pnWritten)
    {
        return DS_FAILURE;
    }
    return DS_SUCCESS;
}

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
iSourcePort::DataSourceReturnCode CRTPEncoder::WriteBlockData(/*in*/ const unsigned char* pBuf,
                                     /*in*/ int nBufSize,
                                     /*in*/ int nTimeStamp,
                                     /*in*/ bool bEOF,
                                     /*rout*/ int64* pnWritten)
{

    UNUSED(nTimeStamp);
    *pnWritten = 0;

    if(!pBuf || !nBufSize)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "CRTPEncoder::Write Invalid arguments");
        return DS_FAILURE;
    }

    if(m_eRTPEncState != RTP_ENC_READY)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "CRTPEncoder::Write RTPEncoder Bad state");
        return DS_FAILURE;
    }

    *pnWritten = (int)m_pPacketizer->Encode(pBuf, (uint32)nBufSize, bEOF);

    if((int32)(*pnWritten) <= 0)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "CRTPEncoder::WriteBlockData encode is failed" );
        return DS_FAILURE;
    }
    return DS_SUCCESS;
}
/*==============================================================================

         FUNCTION:         Close

         DESCRIPTION:
*//**       @brief         Close the encode session
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         Port Number
                           Destination IP address as uint32
                           Stream bitrate

*//*     RETURN VALUE:
*//**       @return
                           DS_FAILURE
                           DS_SUCCESS


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
iSourcePort::DataSourceReturnCode CRTPEncoder::Close()
{

    m_eRTPEncState = RTP_ENC_CLOSED;
    return DS_SUCCESS;
}

