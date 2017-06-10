/*==============================================================================
*       WFDMMSourceStatistics.cpp
*
*  DESCRIPTION:
*       Central repository for all statistics during a session on a WFD Source
*
*
*
*  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
05/6/2014                 InitialDraft
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/
#include "WFDMMSourceStatistics.h"
#include "MMMemory.h"
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "MMMemory.h"
#include "WFDMMLogs.h"
#include "WFDUtils.h"
#include <cutils/properties.h>

/* =============================================================================

                              MACROS

================================================================================
*/

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "WFDMMSRCSTATS"

#define LOCK_MUTEX do {\
    if(pthread_mutex_lock(&sSrcStatLock))\
    {\
        LOGE("Failed to acquire mutex due to %s", strerror(errno));\
    }\
}while(0);


#define UNLOCK_MUTEX do {\
    if(pthread_mutex_unlock(&sSrcStatLock))\
    {\
        LOGE("Failed to acquire mutex due to %s", strerror(errno));\
    }\
}while(0);

#define MAX_STAT_SLOTS 100
#define WFD_SRC_STAT_ENABLE_PROFILING (1<<0)

/* =============================================================================

                              DATA DECLARATIONS

================================================================================
*/


/* -----------------------------------------------------------------------------
** Local Data Declarations
** -------------------------------------------------------------------------- */

WFDMMSourceStatistics        *WFDMMSourceStatistics::s_pMe= NULL;

static pthread_mutex_t sSrcStatLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;

/* ============================================================================
**                            Function Definitions
** ==========================================================================*/

/*=============================================================================

         FUNCTION:          WFDMMSourceStatistics

         DESCRIPTION:
*//**       @brief          CTOR
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

WFDMMSourceStatistics::WFDMMSourceStatistics()
{
    initData();
}

/*=============================================================================

         FUNCTION:          ~WFDMMSourceStatistics

         DESCRIPTION:
*//**       @brief          DTOR
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

WFDMMSourceStatistics::~WFDMMSourceStatistics()
{
    printStatistics();
    releaseResources();
}

/*=============================================================================

         FUNCTION:          isProfilingEnabled

         DESCRIPTION:
*//**       @brief          Helper method to check if profiling is enabled
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            OMX_TRUE if profiling is enabled, else OMX_FALSE

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

OMX_BOOL WFDMMSourceStatistics::isProfilingEnabled()
{
    int ret = 0;
    char szTemp[PROPERTY_VALUE_MAX];
    const char* propString = "persist.debug.wfd.profile";
    ret = property_get(propString,szTemp,NULL);
    if (ret > 0)
    {
        int i = atoi(szTemp);
        if(i>0)
        {
            if(i&WFD_SRC_STAT_ENABLE_PROFILING)
            {
                WFDMMLOGE2("Found %s with value %d",propString, i);
            }
            return OMX_TRUE;
        }
    }
    else
    {
        WFDMMLOGE1("Unable to find profiling property %s",propString);
    }
    return OMX_FALSE;
}

/*=============================================================================

         FUNCTION:          getInstance

         DESCRIPTION:
*//**       @brief          provides an instance of WFDMMSourceStatistics to
                            client, creating one if needed
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            Instance of WFDMMSourceStatistics

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

WFDMMSourceStatistics* WFDMMSourceStatistics::getInstance()
{

    LOCK_MUTEX;

    if(!s_pMe)
    {
        if(isProfilingEnabled())
        {
            s_pMe = MM_New(WFDMMSourceStatistics);
            if(s_pMe)
            {
                if(s_pMe->createResources())
                {
                    WFDMMLOGE("Failed to allocate resources!!!");
                    MM_Delete(s_pMe);
                    s_pMe = NULL;
                }
                else//everything's hunky-dory
                {
                    WFDMMLOGE("Enabling profiling");
                }
            }
        }
    }

    UNLOCK_MUTEX;

    return s_pMe;

}

/*=============================================================================

         FUNCTION:          deleteInstance

         DESCRIPTION:
*//**       @brief          Deletes the instance of WFDMMSourceStatistics if
                            present

*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIDE EFFECTS:
                            WFDMMSourceStatistics instance is no more legal to
                            access, calling any API of WFDSourceStatistics post
                            this are no-ops, without call to getInstance
*//*=========================================================================*/

void WFDMMSourceStatistics::deleteInstance()
{

    LOCK_MUTEX;

    if(s_pMe)
    {
        MM_Delete(s_pMe);
        WFDMMLOGE("Deleted WFDMMSourceStatistics instance");
        s_pMe = NULL;
    }

    UNLOCK_MUTEX;

}

/*=============================================================================

         FUNCTION:          printStatistics

         DESCRIPTION:
*//**       @brief          Prints the statistics of a session on WFD source
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

void WFDMMSourceStatistics::printStatistics() const
{
    if(s_pMe)
    {

        WFDMMLOGE("******************SOURCE STATISTICS**********************");

        if(mWfdProfStats.nFramesEncoded)
        {
            WFDMMLOGE1("Number of video frames encoded = %lld",
                mWfdProfStats.nFramesEncoded);
            WFDMMLOGE1("Minumim Encode Time = %lf ms",
                mWfdProfStats.nMinEncodeTime/1000.0);
            WFDMMLOGE1("Maximum Encode Time = %lf ms",
                mWfdProfStats.nMaxEncodeTime/1000.0);
            WFDMMLOGE1("Average Encode Time = %lf ms",
            mWfdProfStats.nTotEncodeTime/(mWfdProfStats.nFramesEncoded*1000.0));
        }

        if(mWfdProfStats.nFramesDropped)
        {
            WFDMMLOGE1("Number of frames dropped by encoder = %lld",
                mWfdProfStats.nFramesDropped);
        }

        if(mWfdProfStats.nFramesEncrypted)
        {
            WFDMMLOGE1("Number of video frames encrypted = %lld",
                mWfdProfStats.nFramesEncrypted);
            WFDMMLOGE1("Minimum Encrypt time = %lf ms",
                mWfdProfStats.nMinEncryptTime/1000.0);
            WFDMMLOGE1("Maximum Encrypt time = %lf ms",
                mWfdProfStats.nMaxEncryptTime/1000.0);
            WFDMMLOGE1("Average Encrypt time = %lf ms",
            mWfdProfStats.nTotEncryptTime/(mWfdProfStats.nFramesEncrypted*1000.0));
        }

        if(mWfdProfStats.nFramesMuxed)
        {
            WFDMMLOGE1("Number of video frames muxed = %lld",
                mWfdProfStats.nFramesMuxed);
            WFDMMLOGE1("Minumim Muxing Time = %lf ms",
                mWfdProfStats.nMinMuxTime/1000.0);
            WFDMMLOGE1("Maximum Muxing Time = %lf ms",
                mWfdProfStats.nMaxMuxTime/1000.0);
            WFDMMLOGE1("Average Muxing Time = %lf ms",
            mWfdProfStats.nTotMuxTime/(mWfdProfStats.nFramesMuxed*1000.0));
            WFDMMLOGE1("Average Round Trip Time = %lf ms",
            mWfdProfStats.nTotRdTripTime/(mWfdProfStats.nFramesMuxed*1000.0));
        }

        WFDMMLOGE("*********************************************************");

    }
}

/*=============================================================================

         FUNCTION:          getStatistics

         DESCRIPTION:
*//**       @brief          Retrieve the statistics for WFD source
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            An array containing the statistics. Onus on client
                            to delete the array once it's done with it

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/
OMX_S64* WFDMMSourceStatistics::getStatistics()const
{
    OMX_S64* arr = new OMX_S64[WFD_MM_SRC_MAX_STAT_KEYS];
    if(arr)
    {
        arr[FRAMES_DELIVERED] = mWfdProfStats.nFramesDelivered;
        arr[FRAMES_ENCODED]   = mWfdProfStats.nFramesEncoded;
        arr[FRAMES_DROPPED]   = mWfdProfStats.nFramesDropped;
        arr[FRAMES_ENCRYPTED] = mWfdProfStats.nFramesEncrypted;
        arr[FRAMES_MUXED]     = mWfdProfStats.nFramesMuxed;
        arr[MIN_ENCODE_TIME]  = mWfdProfStats.nMinEncodeTime;
        arr[MAX_ENCODE_TIME]  = mWfdProfStats.nMaxEncodeTime;
        arr[AVG_ENCODE_TIME]  = mWfdProfStats.nFramesEncoded ?
        mWfdProfStats.nTotEncodeTime/mWfdProfStats.nFramesEncoded:0;
        arr[MIN_ENCRYPT_TIME] = mWfdProfStats.nMinEncryptTime;
        arr[MAX_ENCRYPT_TIME] = mWfdProfStats.nMaxEncryptTime;
        arr[AVG_ENCRYPT_TIME] = mWfdProfStats.nFramesEncrypted?
        mWfdProfStats.nTotEncryptTime/mWfdProfStats.nFramesEncrypted:0;
        arr[MIN_MUX_TIME]     = mWfdProfStats.nMinMuxTime;
        arr[MAX_MUX_TIME]     = mWfdProfStats.nMaxMuxTime;
        arr[AVG_MUX_TIME]     = mWfdProfStats.nFramesMuxed?
        mWfdProfStats.nTotMuxTime/mWfdProfStats.nFramesMuxed:0;
        arr[MIN_RDTRIP_TIME]  = mWfdProfStats.nMinRdTripTime;
        arr[MAX_RDTRIP_TIME]  = mWfdProfStats.nMaxRdTripTime;
        arr[AVG_RDTRIP_TIME]  = mWfdProfStats.nFramesMuxed?
        mWfdProfStats.nTotRdTripTime/mWfdProfStats.nFramesMuxed:0;
    }
    return arr;
}

/*=============================================================================

         FUNCTION:          initData

         DESCRIPTION:
*//**       @brief          initializes data members of WFDMMSourceStatistics
                            instance
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

void WFDMMSourceStatistics::initData()
{
    mStatSlots    = NULL;
    memset(&mWfdProfStats,0,sizeof(mWfdProfStats));

    /*-----------------------------------------------------------------
     Here's how the Q indexes work:

     mStatSlotHead --> points to the latest frame that has been found

     mStatSlotTail --> points to the location in the Q where a frame
                       is to be pushed.
    -------------------------------------------------------------------
    */

    mStatSlotHead = 0;
    mStatSlotTail = 0;
}

/*=============================================================================

         FUNCTION:          createResources

         DESCRIPTION:
*//**       @brief          creates resources for WFDMMSourceStatistics
                            instance
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            0 on success,else other value in case of failure

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/
int WFDMMSourceStatistics::createResources()
{
    int nRet = 0;
    mStatSlots = MM_New_Array(statSlotInfo,MAX_STAT_SLOTS);
    if(!mStatSlots)
    {
        WFDMMLOGE("Failed to allocate memory for statistics array!!!");
        nRet = 1;
    }
    return nRet;
}


/*=============================================================================

         FUNCTION:          releaseResources

         DESCRIPTION:
*//**       @brief          releases resources of WFDMMSourceStatistics
                            instance
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            0 on success,else other value in case of failure

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/
int WFDMMSourceStatistics::releaseResources()
{
    int nRet = 0;
    if(mStatSlots)
    {
        MM_Delete_Array(mStatSlots);
        mStatSlots = NULL;
    }
    return nRet;
}

/*=============================================================================

         FUNCTION:          recordEncIP

         DESCRIPTION:
*//**       @brief          records relevant details from the buffer header
                            being delivered to encoder
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param[in]     pEncIPBuffHdr buffer header being delivered to
                                          encoder

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

void WFDMMSourceStatistics::recordEncIP(
const OMX_BUFFERHEADERTYPE* pEncIPBuffHdr)
{

    if(s_pMe && pEncIPBuffHdr)
    {
        mWfdProfStats.nFramesDelivered++;
        buff_hdr_extra_info* tempExtra = NULL;

        tempExtra = static_cast<buff_hdr_extra_info*>
            (pEncIPBuffHdr->pPlatformPrivate);

        if(tempExtra)
        {

            if(((mStatSlotTail + 1)% MAX_STAT_SLOTS) == mStatSlotHead)
            {

                /*----------------------------------------------------------------------
                !!!THIS BLOCK SHOULD NEVER BE HIT IDEALLY!!!

                If the current slot has a frame thats still un-encoded, its an act
                of overwriting due to array being full. Hold off any more pushes to
                the statisticsQ till encoder gives outputs to make the situation
                amenable again.
                ------------------------------------------------------------------------
                */

                WFDMMLOGE("!!! Statistics Q is full !!!");
                return;
            }

            WFDMMLOGE4("Sending frame number %lld with TS %lld at time %lld, slot %d",
                tempExtra->nFrameNo, pEncIPBuffHdr->nTimeStamp,
                tempExtra->nEncDelTime,mStatSlotTail);

            mStatSlots[mStatSlotTail].nKey          = pEncIPBuffHdr->nTimeStamp;
            //Encoder delivery time might not be same as the TS of Buffer Header
            mStatSlots[mStatSlotTail].nEncDelTime   = tempExtra->nEncDelTime;
            mStatSlots[mStatSlotTail].nFrameNo      = tempExtra->nFrameNo;
            mStatSlots[mStatSlotTail].bFrameEncoded = OMX_FALSE;

            mStatSlotTail = (mStatSlotTail + 1) % MAX_STAT_SLOTS;

        }

    }

}

/*=============================================================================

         FUNCTION:          recordEncOP

         DESCRIPTION:
*//**       @brief          records relevant details from the buffer header
                            received from encoder
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param[in]     pEncOPBuffHdr buffer header received from encoder

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

void WFDMMSourceStatistics::recordEncOP(
const OMX_BUFFERHEADERTYPE* pEncOPBuffHdr)
{
    /*----------------------------------------------------------------------
    If component returns buffers after it has signalled an EOS, it will be
    giving ones with 0 filled length. Take care of that while recording
    statistics.
    ------------------------------------------------------------------------
    */

    if(s_pMe && pEncOPBuffHdr && pEncOPBuffHdr->nFilledLen)
    {
        buff_hdr_extra_info* tempExtra = NULL;
        OMX_TICKS encTime = 0;

        tempExtra = static_cast<buff_hdr_extra_info*>
            (pEncOPBuffHdr->pPlatformPrivate);

        if(tempExtra)
        {
            GetCurTime(tempExtra->nEncRcvTime);

            /*-----------------------------------------------------------------
            Retrieve the frame no from the stat array using the delivery TS as
            the key and use this to track the frame at output port
            -------------------------------------------------------------------
            */

            int i = mStatSlotHead;
            for(; i!= mStatSlotTail; i =(i+ 1) % MAX_STAT_SLOTS)
            {

                /*-------------------------------------------------------------
                The frame no at head should always match the TS of buffer header
                received from encoder, unless it actually drops frames since WFD
                doesn't use B-Frames and timestamps should always be in order.

                Any concurrency concerns can be dismissed because the TS match
                should be a direct hit or else a few indexes ahead. One thread
                T1 will be updating the tail index, essentially pushing frames
                to be matched with later, and another thread T2 is going to
                search for a frame in this list. In the worst case T1 might be
                updating the tail index while T2 is iterating over the array,
                but this should not be a problem because the head index should
                always be lagging the tail index so it might just need to go a
                few slots ahead (that too when the frame isn't there at all in
                the array, which again is a case that shouldn't ever arise)
                ---------------------------------------------------------------
                */

                if(mStatSlots[i].nKey != pEncOPBuffHdr->nTimeStamp)
                {
                    WFDMMLOGE3("Frame number %lld with TS %lld at %d dropped by encoder",
                        mStatSlots[i].nFrameNo, mStatSlots[i].nKey, i);
                    mWfdProfStats.nFramesDropped++;
                }
                else
                {
                    mStatSlots[i].bFrameEncoded = OMX_TRUE;

                    /*---------------------------------------------------------
                    Now populate the buffer header at output port with the info
                    received after hashing the stat array, to enable tracking
                    of the frame.
                    -----------------------------------------------------------
                    */

                    tempExtra->nFrameNo = mStatSlots[i].nFrameNo;
                    tempExtra->nEncDelTime = mStatSlots[i].nEncDelTime;
                    WFDMMLOGE4("Received frame number %lld with TS %lld at time %lld, at slot %d",
                        tempExtra->nFrameNo, pEncOPBuffHdr->nTimeStamp,tempExtra->nEncRcvTime,i);

                    mStatSlotHead = (i+ 1) % MAX_STAT_SLOTS;
                    break;
                }
            }

            if(i == mStatSlotTail)
            {
                WFDMMLOGE1("Unable to find frame with TS %lld",
                    pEncOPBuffHdr->nTimeStamp);
                /*--------------------------------------------------------------
                 The entire list of frames doesn't have the buffer header with
                 the TS of the buffer header received from encoder. It's most
                 probably a spurious buffer header. No point in going ahead.
                ---------------------------------------------------------------
                */
                return;
            }

            mWfdProfStats.nFramesEncoded++;

            encTime = tempExtra->nEncRcvTime - tempExtra->nEncDelTime;

            mWfdProfStats.nTotEncodeTime += encTime;

            if(mWfdProfStats.nMaxEncodeTime < encTime)
            {
                mWfdProfStats.nMaxEncodeTime = encTime;
            }

            if(mWfdProfStats.nMinEncodeTime == 0)
            {
                mWfdProfStats.nMinEncodeTime = encTime;
            }

            if(mWfdProfStats.nMinEncodeTime > encTime)
            {
                mWfdProfStats.nMinEncodeTime = encTime;
            }

            /*-----------------------------------------------------------------
            Setting MUX delivery time to when we receive it from encoder.
            Also setting MUX receive time to the same value to ignore cases
            when the frame isn't delivered to MUX, by ignoring frames that
            have a MUX diff time of 0 while recording MUX stats
            -------------------------------------------------------------------
            */
            tempExtra->nMuxDelTime = tempExtra->nMuxRcvTime =
                tempExtra->nEncRcvTime;

        }

    }

}

/*=============================================================================

         FUNCTION:          recordVideoEncryptStat

         DESCRIPTION:
*//**       @brief          records encryption statistics
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param[in]     pBuffHdr buffer header submitted for encryption

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

void WFDMMSourceStatistics::recordVideoEncryptStat(const OMX_BUFFERHEADERTYPE* pBuffHdr)
{

    if(s_pMe && pBuffHdr)
    {

        buff_hdr_extra_info* tempExtra = NULL;

        tempExtra = static_cast<buff_hdr_extra_info*>
            (pBuffHdr->pPlatformPrivate);

        if(tempExtra)
        {
            GetCurTime(tempExtra->nEncryptTime);

            /*-----------------------------------------------------------------
            Just before calling encrypt, we receive the buffer from encoder and
            set the MUX delivery time and encoder ouptut time. Use the same for
            recording the encryption start time.
            -------------------------------------------------------------------
            */

            tempExtra->nEncryptTime -= tempExtra->nEncRcvTime;

            if(tempExtra->nEncryptTime)//Ignore spurious frames
            {

                mWfdProfStats.nFramesEncrypted++;

                mWfdProfStats.nTotEncryptTime+= tempExtra->nEncryptTime;

                if(mWfdProfStats.nMinEncryptTime == 0)
                {
                    mWfdProfStats.nMinEncryptTime = tempExtra->nEncryptTime;
                }

                if(mWfdProfStats.nMinEncryptTime > tempExtra->nEncryptTime)
                {
                    mWfdProfStats.nMinEncryptTime = tempExtra->nEncryptTime;
                }

                if(mWfdProfStats.nMaxEncryptTime < tempExtra->nEncryptTime)
                {
                    mWfdProfStats.nMaxEncryptTime = tempExtra->nEncryptTime;
                }

            }

        }
    }

}

/*=============================================================================

         FUNCTION:          recordMuxStat

         DESCRIPTION:
*//**       @brief          records Mux statistics and serves as the point for
                            calculation of round trip time since all info is
                            now available.
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//*         @param[in]     pBuffHdr buffer header received from MUX

*//*     RETURN VALUE:
*//**       @return
                            None

@par     SIDE EFFECTS:
                            None
*//*=========================================================================*/

void WFDMMSourceStatistics::recordMuxStat(
const OMX_BUFFERHEADERTYPE* pBuffHdr)
{
    if(s_pMe && pBuffHdr)
    {

        buff_hdr_extra_info* tempExtra = NULL;

        tempExtra = static_cast<buff_hdr_extra_info*>
            (pBuffHdr->pPlatformPrivate);

        if(tempExtra)
        {

            GetCurTime(tempExtra->nMuxRcvTime);

            mWfdProfStats.nFramesMuxed++;

            //Encoding time
            OMX_TICKS encTime = tempExtra->nEncRcvTime -
                                tempExtra->nEncDelTime;

            if(tempExtra->nEncryptTime)
            {
                /*------------------------------------------------------------------
                MUX delivery time is set when it is received from encoder.
                But in case of encryption, it is actually delivered to MUX once
                encrypted, so just offset MUX delivery time by the encrypt time
                --------------------------------------------------------------------
                */
                tempExtra->nMuxDelTime +=
                tempExtra->nEncryptTime;
            }

            //Mux Time
            OMX_TICKS muxTime = tempExtra->nMuxRcvTime -
                                tempExtra->nMuxDelTime;

            mWfdProfStats.nTotMuxTime += muxTime;

            if(mWfdProfStats.nMinMuxTime == 0)
            {
                mWfdProfStats.nMinMuxTime = muxTime;
            }

            if(mWfdProfStats.nMinMuxTime > muxTime)
            {
                mWfdProfStats.nMinMuxTime = muxTime;
            }

            if(mWfdProfStats.nMaxMuxTime < muxTime)
            {
                mWfdProfStats.nMaxMuxTime = muxTime;
            }

            //Round trip time calculations
            OMX_TICKS rndTripTime = encTime + tempExtra->nEncryptTime +
                                                            muxTime;

            mWfdProfStats.nTotRdTripTime += rndTripTime;

            if(mWfdProfStats.nMinRdTripTime == 0)
            {
                mWfdProfStats.nMinRdTripTime = rndTripTime;
            }

            if(mWfdProfStats.nMinRdTripTime > rndTripTime)
            {
                mWfdProfStats.nMinRdTripTime = rndTripTime;
            }

            if(mWfdProfStats.nMaxRdTripTime < rndTripTime)
            {
                mWfdProfStats.nMaxRdTripTime = rndTripTime;
            }

            LOGE("Frame num %lld encoding: %lld us encryption: %lld us muxing: %lld us rndTrip: %lld us",
                tempExtra->nFrameNo, encTime, tempExtra->nEncryptTime, muxTime, rndTripTime);
            mWfdProfStats.nTotMuxTime += muxTime;

            if(mWfdProfStats.nFramesMuxed % 100 == 0)
            {
                printStatistics();
            }

            //Reset all times
            tempExtra->nEncDelTime  = tempExtra->nEncRcvTime =
            tempExtra->nEncryptTime = tempExtra->nMuxDelTime =
            tempExtra->nMuxRcvTime  = 0;
        }

    }
}
