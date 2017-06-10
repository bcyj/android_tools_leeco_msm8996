#ifndef _HTTP_BANDWIDTH_ESTIMATOR_H_
#define _HTTP_BANDWIDTH_ESTIMATOR_H_

/************************************************************************* */
/**
 * HTTPBandwidthEstimator.h
 * @brief Header file for HTTPBandwidthEstimator.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPBandwidthEstimator.h#13 $
$DateTime: 2013/08/14 18:14:12 $
$Change: 4277631 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPBandwidthEstimator.h
** ======================================================================= */

#include "AEEStdDef.h"
#include "StreamSourceClock.h"
#include "MMCriticalSection.h"
#include "SourceMemDebug.h"
#include <qtv_msg.h>

namespace video {

/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

/**
 * HTTPBandwidthEstimator
 *  2 parts to it:
 *  (i) Data collection: Data is collected into buckets of equal
 *    sized intervals. The appropriate bucket is calculated based
 *    on the current time of the update.
 *  (ii) Bandwidth estimation based on collected data. A simple
 *    average is taken on the data associated with valid buckets.
 *    A valid bucket has 'numBtesRequested' > 0.
 */
class HTTPBandwidthAggregate
{
public:
  /**
   * c'tor
   */
  HTTPBandwidthAggregate(uint32 timeToAddOnStart = 0);

  /**
   * d'tor
   */
  ~HTTPBandwidthAggregate();

  /**
   * Initialize with ptr to StreamSourceClock.
   */
  bool Initialize(StreamSourceClock *pClock);

  /**
   * Updates cummulative data downloaded with amount passed as argument.
   */
  void UpdateSize(uint64 bytesDownloaded)
  {
    MM_CriticalSection_Enter(m_UpdateSizeLock);
    m_CummulativeDataDownloaded+=bytesDownloaded;
    MM_CriticalSection_Leave(m_UpdateSizeLock);
  };

  /**
   * To get start time of first request.
   */
  void StartTimer()
  {
    MM_CriticalSection_Enter(m_UpdateTimeLock);
    UpdateRefCount(TRUE);
    if(!timerRunning)
    {
      startTime = m_pClock->GetTickCount();
      m_CummulativeTimeElapsed += m_TimeToAddOnStart;
      timerRunning = TRUE;
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                     "Start timer 0x%p  = %d", this, (uint32)startTime );
    }

    MM_CriticalSection_Leave(m_UpdateTimeLock);
  };


  /**
   * Updates number of requests are in process.
   * For 'val' - TRUE : it increments number of request
   * For 'val' - FALSE : it decrements number of request
   */
  void UpdateRefCount(bool val)
  {
    if(val)
      refCount++;
    else
      refCount--;
  };

 /**
   * Decrements number of request by one.
   * And if timer is running and number of request under process
   * is 0 then it will update cummulative time taken with
   * time elapsed during processing all requests.
   */
  void StopTimer()
  {
    uint32 timeElapsed = 0;
    MM_CriticalSection_Enter(m_UpdateTimeLock);
    UpdateRefCount(FALSE);
    if(timerRunning && !refCount)
    {
      endTime = m_pClock->GetTickCount();
      timeElapsed = endTime - startTime;
      m_CummulativeTimeElapsed+=timeElapsed;
      timerRunning = FALSE;
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                   "Stop timer %p = %d,elapsed time %d",
                   this, (uint32)endTime,(uint32)timeElapsed);
    }

    MM_CriticalSection_Leave(m_UpdateTimeLock);
  };
  /*  This will provide time taken even for partially downloaded segments.
   *  If number of request is 0 then "m_CummulativeTimeElapsed" is updated with the time taken.
   *  Else it has time till last completed request.
   *  So in that case , we will return updated "time elapsed"
   *
   */
  uint32 TimeElapsed()
  {
      uint32 curTime = m_pClock->GetTickCount();
      return ((refCount == 0) ? m_CummulativeTimeElapsed
                              : (m_CummulativeTimeElapsed + (curTime-startTime)));
  }
  uint64 GetCummulativeDataDownloaded()
  {
    uint64 nDataDownloaded = 0;
    MM_CriticalSection_Enter(m_UpdateSizeLock);
    nDataDownloaded = m_CummulativeDataDownloaded;
    MM_CriticalSection_Leave(m_UpdateSizeLock);
    return nDataDownloaded;
  };

  uint32 GetCummulativeTimeTaken()
  {
    uint32 nTimeTaken;
    MM_CriticalSection_Enter(m_UpdateTimeLock);
    nTimeTaken= TimeElapsed();
    MM_CriticalSection_Leave(m_UpdateTimeLock);
    return nTimeTaken;
  };

  /**
   * Indicates number of active Download requests currently
   *
   */
  int GetNumActiveDownloadRequests()
  {
    int nActiveDownloadRequests = 0;
    MM_CriticalSection_Enter(m_UpdateTimeLock);
    nActiveDownloadRequests = refCount;
    MM_CriticalSection_Leave(m_UpdateTimeLock);
    return nActiveDownloadRequests;
  }

  /**
   * Returns the estimated bandwidth. If bandwidth is unknown,
   * then will use last calculated bandwidth. Returns '-1' when
   * not able to estimate the bandwidth.
   */
  int GetEstimatedBandwidth() const;

private:

  /**
   * class Bucket represents an interval of time which has
   * associated with it, the number of bytes requested and the
   * number of bytes read in the interval.
   */
  class Bucket
  {
  public:
    /**
     * c'tor. Reset's the bucket.
     */
    Bucket();

    /**
     * d'tor. No'op.
     */
    ~Bucket();

    /**
     * Bucket initialization. Also, should be called when a bucket
     * is invalidated, so furthur updates on the bucket will be
     * ignored.
     */
    void Reset();

    /**
     * Needs to be called when a bucket needs to start collecting
     * statistics.
     */
    void Start();

  private:
    // number of bytes requested in socket read. if -1, that means the bucket
    // was reset, and furthur updates to it should be ignored.
    int32 m_NumBytesRequested;

    // number of bytes read in socket read.
    int32 m_NumBytesRead;
  };

  static const int NUM_BUCKETS = 5;
  static const int BUCKET_DURATION_MS = 2000; // ms
  static const int THRESHOLD_ERROR_MS = 20; // 20 ms

  // Number of buckets for b/w grouping.
  Bucket m_BucketArray[NUM_BUCKETS];

  // Pointer to clock.
  StreamSourceClock *m_pClock;

  // To determine if switching to a new bucket.
  int m_PrevBucketIdx;

  // StartTime associated with bucket with idx 'm_PrevBucketIdx'.
  int m_PrevBucketStartTime;

  // Cached bandwidth value in the last Calculatebandwidth call.
  int m_LastCalculatedBandwidth;

  // Bucket duration in msec
  int m_nBucketDuration;
  /* Total amount of data that have been downloaded.
   * It is updated when each request is complete and we have
   * amount of data downloaded
   */
  uint64 m_CummulativeDataDownloaded;

  /* Total time spent in serving requests.
   * It is updated when all the requests that have
   * been queued were processed.
   */
  uint32 m_CummulativeTimeElapsed;

  /**
   * Amount to add as RTT whenever timer is started when the first
   * byte of http response is received.
   */
  uint32 m_TimeToAddOnStart;

  /* Time when request was made */
  uint32 startTime;

  /* Time when request was completely served */
  uint32 endTime;

  /* Information if timer has running and startime is updated */
  bool timerRunning;

  /* For updating total size data */
  MM_HANDLE m_UpdateSizeLock;

  /* For updating total time taken data */
  MM_HANDLE m_UpdateTimeLock;

  /* To keep track of number of request being
   * processed at current time.
   */
  int refCount;
};

/**
 * HTTPBandwidthEstimator is a wrapper on top of
 * HTTPBandwidthAggregate. It incorporates new logic to start
 * timer only when the first byte of the http-response header is
 * received. A preset value of TIME_TO_ADD_ON_START as
 * approximate rtt is added to the cumulative time each time it
 * is started. With this class is used, the refcnf in
 * HTTPBandwidthAggregate will only change between 0 and 1 due
 * to the state transitions of this class.
 */
class HTTPBandwidthEstimator : public HTTPBandwidthAggregate
{
public:
  HTTPBandwidthEstimator();
  ~HTTPBandwidthEstimator();

  void RequestSent();
  void ResponseHeaderReceived();
  void ResponseDataReceived();

private:
  static const uint32 TIME_TO_ADD_ON_START = 50; // Qsm recommendation

  enum BecState
  {
    IDLE, // waiting to receive the first byte of the first outstanding response.
    BUSY, // waiting to receive the last byte of the last outstanding response.
  };

  BecState m_eState;

  int m_NumOutstaningRequests;
};

} // end namespace video
#endif
