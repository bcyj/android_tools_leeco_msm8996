/*==============================================================================
*       WFDMMSinkStatistics.cpp
*
*  DESCRIPTION:
*       Calculates and Prints the audio video frames and
        average statistics in the session
*
*
*
*  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
07/26/2013         Darshan R     InitialDraft
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
#include "WFDMMSinkStatistics.h"
#include "WFDMMLogs.h"
#include "MMMemory.h"

#define MAX_STATS_ARRAY 100
#define ELAPSED_SESSION_TIME 5000000
WFDMMSinkStatistics *WFDMMSinkStatistics::m_pMe = NULL;

/*==============================================================================

         FUNCTION:         CreateInstance

         DESCRIPTION:
*//**       @brief         Use to create instance of this class. This class has
                           been implemented as a singleton class.
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
WFDMMSinkStatistics* WFDMMSinkStatistics::CreateInstance()
{
    if(!m_pMe)
        m_pMe = MM_New(WFDMMSinkStatistics);

    return m_pMe;
}

/*==============================================================================

         FUNCTION:         DeleteInstance

         DESCRIPTION:
*//**       @brief         Destroys the single instance of this class if exists
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
void WFDMMSinkStatistics::DeleteInstance()
{
    WFDMMLOGE("WFDMMSinkStatistics: Deleting the singleton instance");
    if(m_pMe)
    {
        MM_Delete(m_pMe);
        m_pMe = NULL;
    }
}

/*==============================================================================

         FUNCTION:         PushStatsInfo

         DESCRIPTION:
*//**       @brief         The function receives statistics information and uses
                           circular buffer to maintain this information
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:       [1] nStatObj: The structure that holds the required
                                         statistics info from other modules
*//**       @param

*//*     RETURN VALUE:     Boolean
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
bool WFDMMSinkStatistics::PushStatsInfo(sinkStatsType nStatObj)
{
    if((mHeadStatsArray + 1) % MAX_STATS_ARRAY== mTailStatsArray)
    {
         WFDMMLOGE("WFDMMSinkStatistics:Cannot push any more elements!");
         return false;
    }

    mStatsArray[mHeadStatsArray] = nStatObj;
    mHeadStatsArray = (mHeadStatsArray + 1) % MAX_STATS_ARRAY;
    return true;
}

/*==============================================================================

         FUNCTION:         GetStatInfo

         DESCRIPTION:
*//**       @brief         The function receives statistics information and uses
                           circular buffer to maintain this information
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
uint64 WFDMMSinkStatistics::GetFrameIPTime(OMX_TICKS nFramePTS)
{
    sinkStatsType nToRet;
    int i;
    for(i = mTailStatsArray; i != mHeadStatsArray; i = (i+1) % MAX_STATS_ARRAY)
    {
        if(mStatsArray[i].nFrameTimeStamp == nFramePTS)
        {
            nToRet = mStatsArray[i];
            mTailStatsArray = i;
            return nToRet.nFrameIPTime;
        }
    }
    WFDMMLOGE("WFDMMSinkStatistics:Failed to query Statistics Data structure");
    return -1;
}

/*==============================================================================

         FUNCTION:         SetVideoStatistics

         DESCRIPTION:
*//**       @brief         API used by other modules to report statistics info.
                           This function updates the required members of this
                           class.
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:       [1] nCurrDecodeTime: Decode time of frame received
                                                from video decoder module
*//**       @param

*//*     RETURN VALUE:
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
void WFDMMSinkStatistics::SetVideoStatistics(int nCurrDecodeTime)
{
    mNumberOfVideoFrames++;

    if(mMinVideoDecodeTimeMs == 0 && mPeakVideoDecodeTimeMs == 0)
    {
         mMinVideoDecodeTimeMs = nCurrDecodeTime;
         mPeakVideoDecodeTimeMs = nCurrDecodeTime;
    }
    else
    {
         if((uint64)nCurrDecodeTime < mMinVideoDecodeTimeMs)
            mMinVideoDecodeTimeMs = nCurrDecodeTime;
         else if((uint64)nCurrDecodeTime > mPeakVideoDecodeTimeMs)
            mPeakVideoDecodeTimeMs = nCurrDecodeTime;
    }
    mVideoAverageDecodeTimeMs += nCurrDecodeTime;
}

/*==============================================================================

         FUNCTION:         SetVideoDecryptStatistics

         DESCRIPTION:
*//**       @brief         API used by HDCP module to update HDCP stats
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param[in]nDecryptTime Decrypt time for video frame

*//*     RETURN VALUE:
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
void WFDMMSinkStatistics::SetVideoDecryptStatistics(uint64 nDecryptTime)
{
    mNumberOfVideoFramesDecrypted++;
    mVideoAvgDecryptTimeMs += nDecryptTime;

    if(mVideoMinDecryptTimeMs == 0)
    {
        mVideoMinDecryptTimeMs = nDecryptTime;
    }

    if(nDecryptTime > mVideoMaxDecryptTimeMs)
    {
        mVideoMaxDecryptTimeMs = nDecryptTime;
    }

    if(nDecryptTime < mVideoMinDecryptTimeMs)
    {
        mVideoMinDecryptTimeMs = nDecryptTime;
    }
}

/*==============================================================================

         FUNCTION:         UpdateDropsOrLateby

         DESCRIPTION:
*//**       @brief         Updates the renderer specific members of the class
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
void WFDMMSinkStatistics::UpdateDropsOrLateby(rendererStatsType nRendererObj)
{
    if(nRendererObj.nIsVideoFrame)
    {
        if(!mFirstVideoFrame)
        {
            mFirstVideoFrame = 1;
            mFirstVideoFrameArrivalTime = nRendererObj.nSessionTime;
        }

        mTotalVideoFramesArrived++;

        if(mVideoArrivalTimeStamp == 0)
            mVideoArrivalTimeStamp = nRendererObj.nArrivalTime;
        else if((nRendererObj.nArrivalTime - mVideoArrivalTimeStamp) >
                           (uint64)mMaxDiffInArrivalOfSuccessiveVideoFrames)
            mMaxDiffInArrivalOfSuccessiveVideoFrames =
                            nRendererObj.nArrivalTime - mVideoArrivalTimeStamp;

        mAverageVideoFrameArrival = mAverageVideoFrameArrival +
            (nRendererObj.nArrivalTime - mVideoArrivalTimeStamp);

        mVideoArrivalTimeStamp = nRendererObj.nArrivalTime;
    }
    else
    {
        if(!mFirstAudioFrame)
        {
            mFirstAudioFrame= 1;
            mFirstAudioFrameArrivalTime= nRendererObj.nSessionTime;
        }

        mTotalAudioFramesArrived++;

        if(mAudioArrivalTimeStamp == 0)
            mAudioArrivalTimeStamp = nRendererObj.nArrivalTime;
        else if((nRendererObj.nArrivalTime - mAudioArrivalTimeStamp) >
                           (uint64)mMaxDiffInArrivalOfSuccessiveAudioFrames)
            mMaxDiffInArrivalOfSuccessiveAudioFrames =
                            nRendererObj.nArrivalTime - mAudioArrivalTimeStamp;

        mAverageAudioFrameArrival = mAverageAudioFrameArrival +
            (nRendererObj.nArrivalTime - mAudioArrivalTimeStamp);

        mAudioArrivalTimeStamp = nRendererObj.nArrivalTime;
    }

    if(nRendererObj.nIsLate)
    {
        if(nRendererObj.nIsVideoFrame)
        {
            mTotalVideoFramesDropped++;
            if(nRendererObj.nSessionTime - mFirstVideoFrameArrivalTime
                >= ELAPSED_SESSION_TIME)
                mTotalVideoFramesDroppedAfterFiveSeconds++;
        }
        else
        {
            mTotalAudioFramesDropped++;
            if(nRendererObj.nSessionTime - mFirstAudioFrameArrivalTime
                >= ELAPSED_SESSION_TIME)
                mTotalAudioFramesDroppedAfterFiveSeconds++;
        }
    }
    else
    {
        if(nRendererObj.nIsVideoFrame)
        {
            if(nRendererObj.nTimeDecider < mMaxVideoLateByMs)
                mMaxVideoLateByMs = nRendererObj.nTimeDecider;

            mAverageVideoLateByMs += nRendererObj.nTimeDecider;
            mTotalVideoFrames++;
        }
        else
        {
            if(nRendererObj.nTimeDecider < mMaxAudioLateByMs)
                mMaxAudioLateByMs = nRendererObj.nTimeDecider;

            mAverageAudioLateByMs += nRendererObj.nTimeDecider;
            mTotalAudioFrames++;
        }
    }
}

/*==============================================================================

         FUNCTION:         WFDMMSinkStatistics

         DESCRIPTION:
*//**       @brief         Constructor of the class
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
WFDMMSinkStatistics::WFDMMSinkStatistics()
{
    WFDMMLOGE("WFDMMSinkStatistics:Constructor called");

    //New members
    mStatsArray = MM_New_Array(sinkStatsType,MAX_STATS_ARRAY);
    mHeadStatsArray = 0;
    mTailStatsArray = 0;

    //Video/Audio statistics
    mVideoAverageDecodeTimeMs = 0;
    mAudioAverageDecodeTimeMs = 0;
    mPeakVideoDecodeTimeMs = 0;
    mPeakAudioDecodeTimeMs = 0;
    mMinVideoDecodeTimeMs = 0;
    mMinAudioDecodeTimeMs = 0;
    mNumberOfAudioFrames = 0;
    mNumberOfVideoFrames=0;


    //render statistics
    mTotalVideoFrames = 0;
    mTotalAudioFrames = 0;
    mAudioArrivalTimeStamp = 0;
    mVideoArrivalTimeStamp = 0;

    mTotalVideoFramesDropped = 0;
    mTotalAudioFramesDropped = 0;

    mMaxAudioLateByMs = 0;
    mMaxVideoLateByMs = 0;

    mAverageAudioLateByMs = 0;
    mAverageVideoLateByMs = 0;

    mTotalVideoFramesDroppedAfterFiveSeconds = 0;
    mTotalAudioFramesDroppedAfterFiveSeconds = 0;

    mMaxDiffInArrivalOfSuccessiveAudioFrames = 0;
    mMaxDiffInArrivalOfSuccessiveVideoFrames = 0;

    mAverageAudioFrameArrival = 0;
    mAverageVideoFrameArrival = 0;

    mTotalVideoFramesArrived = 0;
    mTotalAudioFramesArrived = 0;

    mFirstVideoFrame = 0;
    mFirstAudioFrame = 0;
    mFirstVideoFrameArrivalTime = 0;
    mFirstAudioFrameArrivalTime = 0;

    mVideoAvgDecryptTimeMs = 0;
    mVideoMinDecryptTimeMs = 0;
    mVideoMaxDecryptTimeMs = 0;
    mNumberOfVideoFramesDecrypted = 0;

}

/*==============================================================================

         FUNCTION:         ~WFDMMSinkStatistics

         DESCRIPTION:
*//**       @brief         Destructor of the WFDMMThread class
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
int  WFDMMSinkStatistics::PrintStatistics(void)
{
    WFDMMLOGE("****************SINK STATISTICS******************************");
    WFDMMLOGE1("Minumim Decode Time = %llu",mMinVideoDecodeTimeMs);
    WFDMMLOGE1("Maximum Decode Time = %llu",mPeakVideoDecodeTimeMs);
    if(mNumberOfVideoFrames)
    {
        WFDMMLOGE1("Average Decode Time = %llu",
            mVideoAverageDecodeTimeMs/mNumberOfVideoFrames);
    }

    WFDMMLOGE1("Total Number of Video Frames dropped in the session= %d",
        mTotalVideoFramesDropped);
    WFDMMLOGE1("Total Number of Audio Frames dropped in the session= %d",
        mTotalAudioFramesDropped);
    WFDMMLOGE1("Number of Video Frames dropped after 5 seconds= %d",
        mTotalVideoFramesDroppedAfterFiveSeconds);
    WFDMMLOGE1("Number of Audio Frames dropped after 5 seconds = %d",
        mTotalAudioFramesDroppedAfterFiveSeconds);

    WFDMMLOGE1("Max Video Lateby = %lld",mMaxVideoLateByMs);
    WFDMMLOGE1("Max Audio Lateby = %lld",mMaxAudioLateByMs);
    if(mTotalVideoFrames)
    {
        WFDMMLOGE1("Average Video Lateby = % lld",
            mAverageVideoLateByMs/mTotalVideoFrames);
    }
    if(mTotalAudioFrames)
    {
        WFDMMLOGE1("Average Audio Lateby = %lld",
            mAverageAudioLateByMs/mTotalAudioFrames);
    }

    WFDMMLOGE1("Max difference in arrival of successive Video Frames = %lld",
        mMaxDiffInArrivalOfSuccessiveVideoFrames);
    WFDMMLOGE1("Max difference in arrival of successive Audio Frames = %lld",
        mMaxDiffInArrivalOfSuccessiveAudioFrames);

    if(mTotalVideoFramesArrived)
    {
        WFDMMLOGE1("Avg diff in arrival of successive   Video Frames = %lld",
            mAverageVideoFrameArrival / mTotalVideoFramesArrived);
    }
    if(mTotalAudioFramesArrived)
    {
        WFDMMLOGE1("Avg diff in arrival of successive Audio Frames = %lld",
            mAverageAudioFrameArrival / mTotalAudioFramesArrived);
    }

    if(mNumberOfVideoFramesDecrypted)
    {
        WFDMMLOGE1("Number of video frames decrypted = %lld",
            mNumberOfVideoFramesDecrypted);
        WFDMMLOGE1("Maximum Decrypt time = %lld", mVideoMaxDecryptTimeMs);
        WFDMMLOGE1("Minimum Decrypt time = %lld", mVideoMinDecryptTimeMs);
        WFDMMLOGE1("Average Decrypt time = %lf ms",
            mVideoAvgDecryptTimeMs*1.0/mNumberOfVideoFramesDecrypted);
    }

    WFDMMLOGE("**************************************************************");
    return 0;
}

/*==============================================================================

         FUNCTION:         ~WFDMMSinkStatistics

         DESCRIPTION:
*//**       @brief         Destructor of the WFDMMThread class
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
WFDMMSinkStatistics::~WFDMMSinkStatistics()
{
    WFDMMLOGE("WFDMMSinkStatistics:Destructor called");
    if(mStatsArray)
        MM_Delete_Array(mStatsArray);
}

