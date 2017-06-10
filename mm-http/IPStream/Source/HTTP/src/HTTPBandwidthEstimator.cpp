/************************************************************************* */
/**
 * HTTPBandwidthEstimator.cpp
 * @brief implementation of the HTTPBandwidthEstimator.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPBandwidthEstimator.cpp#10 $
$DateTime: 2013/08/14 12:00:06 $
$Change: 4274722 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPBandwidthEstimator.h"
#include "SourceMemDebug.h"
#include "qtv_msg.h"

namespace video {

/**
 * c'tor
 */
HTTPBandwidthAggregate::HTTPBandwidthAggregate(uint32 timeToAddOnStart):
  m_pClock(NULL),
  m_PrevBucketIdx(-1),
  m_PrevBucketStartTime(-1),
  m_LastCalculatedBandwidth(-1),
  m_nBucketDuration(BUCKET_DURATION_MS)
{
  for (int i = 0; i < NUM_BUCKETS; ++i)
  {
    m_BucketArray[i].Reset();
  }
  m_CummulativeDataDownloaded = 0;
  m_CummulativeTimeElapsed = 0;
  m_TimeToAddOnStart = timeToAddOnStart;
  refCount = 0;
  timerRunning = FALSE;

  MM_CriticalSection_Create(&m_UpdateTimeLock);
  MM_CriticalSection_Create(&m_UpdateSizeLock);
}

/**
 * d'tor
 */
HTTPBandwidthAggregate::~HTTPBandwidthAggregate()
{
  if(m_UpdateTimeLock)
  {
    MM_CriticalSection_Release(m_UpdateTimeLock);
    m_UpdateTimeLock = NULL;
  }
  if(m_UpdateSizeLock)
  {
    MM_CriticalSection_Release(m_UpdateSizeLock);
    m_UpdateSizeLock = NULL;
  }
}

/**
 * Initialize with ptr to StreamSourceClock.
 */
bool HTTPBandwidthAggregate::Initialize(StreamSourceClock *pClock)
{
  m_pClock = pClock;
  return (m_pClock ? true : false);
}

/**
 * Returns the estimated bandwidth. If bandwidth is unknown,
 * then will use last calculated bandwidth. Returns '-1' when
 * not able to estimate the bandwidth.
 */
int HTTPBandwidthAggregate::GetEstimatedBandwidth() const
{
  return m_LastCalculatedBandwidth;
}

/**
 * c'tor. Reset's the bucket.
 */
HTTPBandwidthAggregate::Bucket::Bucket()
{
  Reset();
}

/**
 * Needs to be called when a bucket needs to start collecting
 * statistics.
 */
HTTPBandwidthAggregate::Bucket::~Bucket()
{

}

/**
 * Bucket initialization. Also, should be called when a bucket
 * is invalidated, so furthur updates on the bucket will be
 * ignored.
 */
void HTTPBandwidthAggregate::Bucket::Reset()
{
  m_NumBytesRequested = -1;
  m_NumBytesRead = -1;
}

/**
 * Needs to be called when a bucket needs to start collecting
 * statistics.
 */
void HTTPBandwidthAggregate::Bucket::Start()
{
  m_NumBytesRequested = 0;
  m_NumBytesRead = 0;
}

HTTPBandwidthEstimator::HTTPBandwidthEstimator() :
  HTTPBandwidthAggregate(TIME_TO_ADD_ON_START),
  m_eState(IDLE),
  m_NumOutstaningRequests(0)
{

}

HTTPBandwidthEstimator::~HTTPBandwidthEstimator()
{

}

void HTTPBandwidthEstimator::RequestSent()
{
  ++m_NumOutstaningRequests;
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "RequestSent: %u", m_NumOutstaningRequests);
}

void HTTPBandwidthEstimator::ResponseHeaderReceived()
{
  if (IDLE == m_eState)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                  "ResponseHeaderReceived num %u", m_NumOutstaningRequests);
    m_eState = BUSY;
    StartTimer();
  }
}


void HTTPBandwidthEstimator::ResponseDataReceived()
{
  --m_NumOutstaningRequests;

  if (0 == m_NumOutstaningRequests && BUSY == m_eState)
  {
    StopTimer();
    m_eState = IDLE;
  }

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
               "ResponseDataReceived num %u", m_NumOutstaningRequests);
}

} // end namespace video
