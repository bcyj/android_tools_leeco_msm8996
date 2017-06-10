#ifndef __WFDMMSINKSTATISTICS_H
#define  __WFDMMSINKSTATISTICS_H
/*==============================================================================
*       WFDMMSinkStatistics.h
*
*  DESCRIPTION:
*       Class declaration for WFDMMSinkStatistics. This provides audio video
        statisctics for WFD mm sink module
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
03/25/2013         SK            InitialDraft
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
#include "WFDMMSinkCommon.h"

typedef struct sinkStats
{
    OMX_TICKS nFrameTimeStamp;
    uint64 nFrameIPTime;
}sinkStatsType;

typedef struct rendererStats
{
    bool nIsVideoFrame;
    bool nIsLate;
    int64 nTimeDecider;
    uint64 nSessionTime;
    uint64 nArrivalTime;
}rendererStatsType;

class WFDMMSinkStatistics
{
public:

    ~WFDMMSinkStatistics();
    int PrintStatistics(void);

    static WFDMMSinkStatistics* CreateInstance();
    static void DeleteInstance();

    bool PushStatsInfo(sinkStatsType nStatObj);
    uint64 GetFrameIPTime(OMX_TICKS nFramePTS);
    void SetVideoStatistics(int nCurrDecodeTime);
    void UpdateDropsOrLateby(rendererStatsType nRendererObj);
    void SetVideoDecryptStatistics(uint64 nDecryptTime);

private:
    //Datastructure
    sinkStatsType* mStatsArray;
    int mHeadStatsArray;
    int mTailStatsArray;

    WFDMMSinkStatistics();
    static WFDMMSinkStatistics* m_pMe;

    //decoder statistics
    uint64 mVideoAverageDecodeTimeMs;
    uint64 mAudioAverageDecodeTimeMs;
    uint64 mPeakVideoDecodeTimeMs;
    uint64 mPeakAudioDecodeTimeMs;
    uint64 mMinVideoDecodeTimeMs;
    uint64 mMinAudioDecodeTimeMs;
    uint64 mNumberOfAudioFrames;
    uint64 mNumberOfVideoFrames;


    //render statistics

    int64 mTotalVideoFrames;
    int64 mTotalAudioFrames;
    int64 mVideoArrivalTimeStamp;
    int64 mAudioArrivalTimeStamp;

    int32 mTotalVideoFramesDropped;
    int32 mTotalAudioFramesDropped;

    int64 mMaxAudioLateByMs;
    int64 mMaxVideoLateByMs;

    int64 mAverageAudioLateByMs;
    int64 mAverageVideoLateByMs;

    int32 mTotalVideoFramesDroppedAfterFiveSeconds;
    int32 mTotalAudioFramesDroppedAfterFiveSeconds;

    int64 mMaxDiffInArrivalOfSuccessiveAudioFrames;
    int64 mMaxDiffInArrivalOfSuccessiveVideoFrames;

    int64 mAverageVideoFrameArrival;
    int64 mAverageAudioFrameArrival;

    int64 mTotalVideoFramesArrived;
    int64 mTotalAudioFramesArrived;

    bool mFirstVideoFrame;
    bool mFirstAudioFrame;
    uint64 mFirstVideoFrameArrivalTime;
    uint64 mFirstAudioFrameArrivalTime;

    //Decrypt Statistics
    uint64 mVideoAvgDecryptTimeMs;
    uint64 mVideoMinDecryptTimeMs;
    uint64 mVideoMaxDecryptTimeMs;
    uint64 mNumberOfVideoFramesDecrypted;
};
#endif /*__WFDMMSINKSTATISTICS_H*/
