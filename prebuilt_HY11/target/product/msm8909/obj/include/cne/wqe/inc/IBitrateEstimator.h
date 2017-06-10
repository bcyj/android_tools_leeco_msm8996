#ifndef _IBitrateEstimator_h_
#define _IBitrateEstimator_h_

/*==============================================================================
  FILE:         IBitrateEstimator.h

  OVERVIEW:     Abstract class.


  DEPENDENCIES: Logging, Ppc, C++ STL

                Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
                All Rights Reserved.
                Qualcomm Technologies Confidential and Proprietary
==============================================================================*/


/*==============================================================================
  EDIT HISTORY FOR MODULE

  when        who      what, where, why
  ----------  ---      --------------------------------------------------------
  05-13-2014  npoddar  First revision.
==============================================================================*/


/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/

#include <arpa/inet.h>
#ifdef __unused
#undef __unused
#endif
#include <linux/netfilter.h>
#include <linux/netlink.h>
#include <map>
#include <queue>
#include <set>
#include <hash_map>
#include <CneDefs.h>
#include "WqePolicyTypes.h"
#include "SwimNetlinkSocket.h"

using namespace std;

/*------------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Forward Class Declarations
 * ---------------------------------------------------------------------------*/
class SwimWlanLinkMgr;
class SwimBQEActiveProbe;
class SwimByteCounter;
class SwimInterfaceManager;
class SwimDefaultInterfaceSelector;

/*------------------------------------------------------------------------------
 * Type Declarations
 * ---------------------------------------------------------------------------*/
typedef enum
{
  SWIM_BQE_RET_CODE_SUCCESS         = 0,
  SWIM_BQE_RET_CODE_FAILURE         = -1,
  SWIM_BQE_RET_CODE_ALREADY_STARTED = -2,
  SWIM_BQE_RET_CODE_BQE_DISABLED    = -3
} SwimBqeRetCodeType_t;

typedef enum
{
  BEE_POLLING_MODE_IDLE,
  BEE_POLLING_MODE_SAMPLING,
  BEE_POLLING_MODE_BURST,
  BEE_POLLING_MODE_DISABLE
} BEEPollingMode_t;

typedef enum
{
  BEE_PROBE_STATE_BQE_PASSIVE,
  BEE_PROBE_STATE_BQE_ACTIVE,
  BEE_PROBE_STATE_CNE
} BEEProbeState_t;

enum
{
  SWIM_BQE_STATUS_STARTED         = 0x00000001,
  SWIM_BQE_STATUS_STARTED_PENDING = 0x00000002,
  SWIM_BQE_STATUS_FORCE_BQE       = 0x00000004,
  SWIM_BQE_STATUS_CANCEL_PENDING  = 0x00000008,
  SWIM_BQE_STATUS_WAITING_FOR_JRTT = 0x00000010,
  SWIM_BQE_STATUS_ACTIVE_BQE_PENDING = 0x00000020,
  SWIM_BQE_STATUS_WAITING_FOR_SSID = 0x00000040,
  SWIM_BQE_STATUS_WAITING_FOR_ICD_PARAMS = 0x00000080,
  SWIM_BQE_STATUS_PASSIVE_BQE_FAILED = 0x00000100,
  SWIM_BQE_STATUS_WAITING_FOR_CQE_OUTCOME = 0x00000200,
  SWIM_BQE_STATUS_ACTIVE_BQE_PASSED = 0x00000400,
};

typedef enum
{
  SWIM_BEE_RET_CODE_SUCCESS = 0,
  SWIM_BEE_RET_CODE_FAILURE = -1,
  SWIM_BEE_RET_CODE_NOT_SUPPORTED = -2,
  SWIM_BEE_RET_CODE_IN_PROGRESS = -3,
  SWIM_BEE_RET_CODE_BQE_DISABLED = -4
} SwimBeeRetCodeType_t;

struct SwimBqeFileSizeCompParams {

  SwimBqeFileSizeCompParams(
    uint32_t currentRttMillis,
    uint32_t mssBytes,
    uint32_t numParallelStreams,
    uint32_t congToSlowRatio):
    _currentRttMillis(currentRttMillis),
    _mssBytes(mssBytes),
    _numParallelStreams(numParallelStreams),
    _congToSlowRatio(congToSlowRatio)
  {

  }

  SwimBqeFileSizeCompParams():
    _currentRttMillis(SWIM_DEFAULT_RTT * 1000),
    _mssBytes(SWIM_DEFAULT_MSS),
    _numParallelStreams(SWIM_DEFAULT_NUM_PARALLEL_STREAM),
    _congToSlowRatio(SWIM_DEFAULT_CONG_TO_SLOW)
  {

  }

  uint32_t _currentRttMillis;
  uint32_t _mssBytes;
  uint32_t _numParallelStreams;
  uint32_t _congToSlowRatio;//ratio
};        /* ----------  end of struct SwimBqeFileSizeCompParams  ---------- */

struct SwimBqeDownloadParams {

  SwimBqeDownloadParams():
    _fileSizeMegaBytes(0),
    _fileDurationMillis(0)
  {

  }

  float _fileSizeMegaBytes;
  uint32_t  _fileDurationMillis;
};        /* ----------  end of struct SwimBqeDownloadParams  ---------- */

typedef enum SwimDefaultInterfaceSelectorEvent_e {
  CNE_NIMS_DEF_SEL_CHANGE_EVENT,
  CNE_NIMS_DEF_SEL_WWAN_ADDRESS_AVAILABLE,
  CNE_NIMS_DEF_SEL_WLAN_ADDRESS_AVAILABLE,
  CNE_NIMS_WLAN_STATE_CHANGE_EVENT,
  CNE_NIMS_ICD_OUTCOME,
  CNE_NIMS_WLAN_TEST_START_EVENT,
  CNE_NIMS_CQE_OUTCOME,
} SwimDefaultInterfaceSelectorEvent_t;

/*
* event_cb type should be the same as that defined by EventDispatcher.h
*/
typedef void (*event_cb)(SwimDefaultInterfaceSelectorEvent_t event,
                         const void *event_data, void *cbdata);

struct SwimBPSClientRecord_t {
  SwimBPSClientRecord_t(unsigned int clientId, BEEPollingMode_t mode,
      BEEProbeState_t state, cne_rat_type type, void *ptr,
      unsigned int wticks = 0):
    jrttMillis(0),
    getTsMillis(0),
    cleanupScheduled(false)
  {
    id = clientId;
    pollingMode = mode;
    probeState = state;
    ratType = type;
    elapsedTimeTicks = 0;
    elapsedTimeTicksSinceBurst = 0;
    initialByteRead = 0;
    initialBurstByteRead = 0;
    totalBurstBytesRead = 0;
    totalMbToDownload = 0;
    context = ptr;
    windowTicks = wticks;
    pollInterval = 0;
    burstDuration = 0;
    jrttTimeoutId = 0;
    icdTimeoutId = 0;
    bqeExpiredId = 0;
    cleanupScheduled = false;
  }
  unsigned int id;
  BEEPollingMode_t pollingMode;
  BEEProbeState_t probeState;
  cne_rat_type ratType;
  queue<uint64_t> byteCounterQ;
  multiset<unsigned int> sampleSet;
  uint64_t initialByteRead;
  uint64_t initialBurstByteRead;
  uint64_t totalBurstBytesRead;
  float totalMbToDownload;
  unsigned int elapsedTimeTicks;
  unsigned int elapsedTimeTicksSinceBurst;
  void *context;
  uint32_t jrttMillis;
  uint32_t getTsMillis;
  // Window duration for webtech instantaneous bitrate
  // in multiples of poll intervals
  unsigned int windowTicks;
  // Poll interval to sample bitrate
  uint16_t pollInterval;
  // Burst duration in multiples of poll intervals
  uint8_t burstDuration;
  // timer id for the timer that we start when we send the active probe start
  // message to java
  int jrttTimeoutId;
  // timer id for the timer that we start when we have finished long term
  // history analysis and it has returned bad/unknown.
  int icdTimeoutId;
  // timer if for the timer which is runs periodically in order to do
  // byte counting and check whether to schedule evaluation
  int bqeExpiredId;
  bool cleanupScheduled;
  // client callback which gets notified of ICD pass/fail events
  // this callback must call SwimBitrateEstimator::IcdOutcomeHandler
  event_cb clientIcdOutcomeHandler;
  // client callback which gets notified of CQE pass/fail events
  // this callback must call SwimBitrateEstimator::CqeOutcomeHandler
  event_cb clientCqeOutcomeHandler;
};

typedef std::map <int, SwimBPSClientRecord_t *> SwimBPSClientMap_t;

enum
{
  RESULT_ESTIMATED = 0,
  RESULT_FORCE_STOPPED_AND_FAILED = 1,
  RESULT_FORCE_STOPPED_AND_PASSED = 2
};
//
// This is the type for BQE callback with the result of BQE
// Estimated bitrate is passed in the second argument of the callback
typedef void ( *SwimBQEResultCb_t )( int type, unsigned long bps, void *arg );

/*------------------------------------------------------------------------------
 * Function Definition
 * ---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * Class Definition
 * ---------------------------------------------------------------------------*/

class IBitrateEstimator
{
public:
  /*----------------------------------------------------------------------------
   * Public Types
   * -------------------------------------------------------------------------*/


  /*----------------------------------------------------------------------------
   * Public Method Specifications
   * -------------------------------------------------------------------------*/

  /*----------------------------------------------------------------------------
   * FUNCTION      SwimBitrateEstimator Constructor
   *
   * DESCRIPTION
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
   IBitrateEstimator( ){}

  /*----------------------------------------------------------------------------
   * FUNCTION      SwimBitrateEstimator Destructor
   *
   * DESCRIPTION
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  virtual ~IBitrateEstimator( ){}

  /*----------------------------------------------------------------------------
   * FUNCTION      init
   *
   * DESCRIPTION   Initializes SwimBitrateEstimator
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  True if successful, false otherwise
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  virtual bool init() = 0;

  /*----------------------------------------------------------------------------
   * FUNCTION      InitializeLinks
   *
   * DESCRIPTION   Setup links to other used objects
   *
   * DEPENDENCIES  None
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  virtual void InitializeLinks(SwimDefaultInterfaceSelector *a_defsel, SwimWlanLinkMgr *a_wlanmgr,
			SwimNetlinkSocket *a_nl) = 0;

  /*----------------------------------------------------------------------------
   * FUNCTION      startBPSMeasurementWithActiveProbe
   *
   * DESCRIPTION   Starts BPS measurement for BQE
   *
   * DEPENDENCIES  Logging
   *
   * PARAMETERS
   *    bpsThreshold - threshold to be used to evaluate quality of the passive BQE (in bps)
   *    bqePassiveDuration  - Passive BQE test duration (in msec)
   *    bqeResultCb  - BQE result callback
   *    udata    - pointer to the user data for the BQE result callback
   *
   * RETURN VALUE  Error code
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  virtual SwimBeeRetCodeType_t startBPSMeasurementWithActiveProbe
    (unsigned int bpsThreshold,
     unsigned int bqePassiveDuration,
     SwimBQEResultCb_t bqeResultCb,
     void *udata) = 0;

  /*----------------------------------------------------------------------------
   * FUNCTION      disableBQE
   *
   * DESCRIPTION   Disables ongoing BPS measurement for BQE if it is active
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  Error code
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  virtual void disableBQE() = 0;

  /*----------------------------------------------------------------------------
   * FUNCTION      pushBqeThreshold
   *
   * DESCRIPTION   Pushes 3G threshold for ongoing passive BQE
   *
   * DEPENDENCIES  Logging
   *
   * PARAMETERS     bpsThreshold - threshold to be used to evaluate quality of
   * the passive BQE (in bps)
   *
   * RETURN VALUE  Error code
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  virtual void pushBqeThreshold(unsigned int bpsThreshold) = 0;

  /*----------------------------------------------------------------------------
   * FUNCTION      startBPSMeasurement
   *
   * DESCRIPTION   Starts BPS measurement for a geSwimBeeRetCodeType_t stopBPSMeasurement(int clientId);
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  Error code
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  virtual SwimBeeRetCodeType_t startBPSMeasurement(unsigned int windowDuration,
      cne_rat_type ratType, int& clientId) = 0;

  /*----------------------------------------------------------------------------
   * FUNCTION      stopBPSMeasurement
   *
   * DESCRIPTION   Stops BPS measurement for a generic client
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  Error code
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  virtual SwimBeeRetCodeType_t stopBPSMeasurement(int clientId) = 0;


  /*----------------------------------------------------------------------------
   * FUNCTION      getBPSEstimate
   *
   * DESCRIPTION   Gets BPS estimate for a generic client
   *
   * DEPENDENCIES  Logging
   *
   * PARAMETERS:
   *    clientId (input) - client id
   *    ltRate (output)  - long term bitrate since the start of the BPS measurement
   *    maxRate (output) - maximum burst rate
   *    instRate (output) - instantaneous rate at the point of the estimate
   *    over the window duration since the last poll
   * RETURN VALUE  Error code
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  virtual SwimBeeRetCodeType_t getBPSEstimate(int clientId, unsigned int *ltRate,
      unsigned int *maxRate, unsigned int *instRate) = 0;

  /*
   * ---------------------------------------------------------------------------
   *  FUNCTION:          PerformBitrateCleanUpCb
   *
   *  DESCRIPTION:   A callback to perform cleanup of SwimBitrateEstimator
   *                 member variables
   *
   *  DEPENDENCIES:  logging
   *
   *  PARAMETERS:    none
   *
   *  RETURN VALUE:  None
   *
   *  SIDE EFFECTS:  None
   *
   * ---------------------------------------------------------------------------
   */
  virtual void PerformBitrateCleanUpCb () = 0;

  /*----------------------------------------------------------------------------
   * FUNCTION      bqeEnable
   *
   * DESCRIPTION   Starts BQE
   *
   * DEPENDENCIES  Logging
   *
   * PARAMETERS
   *    forceBqe - if true BQE will proceed without prior checking for short/long
   *    term history and load control
   *    bqeResultCb  - BQE result callback
   *    udata    - pointer to the user data for the BQE result callback
   *
   * RETURN VALUE  Error code
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
//  virtual SwimBqeRetCodeType_t bqeEnable(bool forceBqe,
//      SwimBQEResultCb_t bqeResultCb, void *udata) = 0;

  /*----------------------------------------------------------------------------
   * Public Attributes
   * -------------------------------------------------------------------------*/

};

#endif /* _IBitrateEstimator_h_ */



