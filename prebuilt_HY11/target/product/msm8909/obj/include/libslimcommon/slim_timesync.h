#ifndef SLIM_TIMESYNC_H
#define SLIM_TIMESYNC_H
/*============================================================================
  @file slim_timesync.h

  SLIM time synchronization filter implementation.

               Copyright (c) 2013-2014 QUALCOMM Atheros, Inc.
               All Rights Reserved.
               Qualcomm Atheros Confidential and Proprietary
============================================================================*/
/* $Header: //components/rel/gnss.mpss/6.0/gnss/slim/common/core/inc/slim_timesync.h#2 $ */

#ifdef __cplusplus
 extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "slim_utils.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

typedef enum
{
  /*! Filter state is invalid */
  SLIM_TIME_SYNC_UPDATE_STATUS_INVALID,

  /*! Filter update was successful */
  SLIM_TIME_SYNC_UPDATE_STATUS_SUCCESS,

  /*! Filter update was an outlier */
  SLIM_TIME_SYNC_UPDATE_STATUS_OUTLIER,

  /*! Filter was reset */
  SLIM_TIME_SYNC_UPDATE_STATUS_RESET
} slim_TimeSyncUpdateStatusEnumType;

/*! Struct for SLIM time sync filter configuration */
typedef struct
{
  DBL d_ProcessNoiseDensity[2];

  /* Outlier detection thresholds */
  DBL d_OutlierThreshold;
  DBL d_OutlierSigmaThreshold;
  uint32 q_MaxOutlierCount;
} slim_TimeSyncFilterConfigStructType;

/*! Struct for SLIM time sync filter */
typedef struct
{
  /* Timetick of last filter update */
  uint64 t_LastUpdatedTimeTick;

  uint32 q_OutlierCount;

  /* TRUE if time sync is valid, FALSE if not */
  boolean b_Valid;

  /* Filter state and covariance matrix
     [0] = Remote to local offset
     [1] = Remote to local offset drift
   */
  DBL d_KfState[2];

  /* Upper diagonal of the KF state covariance matrix as: */
  DBL d_KfStateCov[3];

  /* Filter state as integer offset */
  int64 r_FilteredRemoteToLocalOffset;
  int64 r_FilteredRemoteToLocalOffsetUnc;

  slim_TimeSyncFilterConfigStructType z_Config;
} slim_TimeSyncFilterStateStructType;

/*! Struct for monotonous offset for service time sync */
typedef struct
{
  boolean b_Valid;
  int64 r_MonotonousRemoteToLocalOffset;
} slim_TimeSyncMtOffsetStructType;

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

extern slim_TimeSyncFilterConfigStructType slimz_DefaultTimeSyncFilterConfig;

/**
@brief Initialize time sync filter state.

This function initializes time sync filter state using given filter
configuration. The configuration is copied to the filter state.

@param  pz_Filter: Pointer to time sync filter state
@param  pz_Config: Pointer to time sync filter config

@return None
*/
void slim_TimeSyncInit(slim_TimeSyncFilterStateStructType *pz_Filter,
                       const slim_TimeSyncFilterConfigStructType *pz_Config);

/**
@brief Update time sync filter state with RTT measurement.

This function computes the offset between remote and local
times from RTT (round-trip-time) measurements, and updates
the time sync filter to improve offset estimate. If the
filter has detected too many outliers, it will reset and
and return TRUE to inform the user about a time jump.

@param  pz_Filter: Pointer to time sync filter state
@param  q_LocalTxTimeMsec: Local transmit time in milliseconds
@param  q_RemoteRxTimeMsec: Remote receive time in milliseconds
@param  q_RemoteTxTimeMsec: Remote transmit time in milliseconds
@param  q_LocalRxTimeMsec: Local receive time in milliseconds
@param  t_CurrentTimeTick: Timetick at update (msec)

@return SLIM_TIME_SYNC_UPDATE_STATUS_INVALID: when filter state
        was, or has become invalid

        SLIM_TIME_SYNC_UPDATE_STATUS_SUCCESS: when measurement was
        successfully applied to filter state

        SLIM_TIME_SYNC_UPDATE_STATUS_OUTLIER: when measurement was
        discarded due to being an outlier

        SLIM_TIME_SYNC_UPDATE_STATUS_RESET: when filter has been
        reset due to consecutive outliers
*/
slim_TimeSyncUpdateStatusEnumType slim_TimeSyncRttUpdate(
  slim_TimeSyncFilterStateStructType *pz_Filter,
  uint32 q_LocalTxTimeMsec,
  uint32 q_RemoteRxTimeMsec,
  uint32 q_RemoteTxTimeMsec,
  uint64 t_LocalRxTimeMsec);

/**
@brief Propagate time sync filter to current time.

This function propagates the time sync filter to current
time. Essentially this is a KF propagation step.

@param pz_Filter: Pointer to time sync filter state
@param t_CurrentTimeTick: Current time tick (msec)

@return None
*/
void slim_TimeSyncTimeUpdate(slim_TimeSyncFilterStateStructType *pz_Filter,
                             uint64 t_CurrentTimeTick);

/**
@brief Helper function for determining if the filter should be time updated.

This function determines if the remote-to-local offset could have
drifted more than half a millisecond since last filter update. To do
this, it estimates the amount of drift based on the offset drift
estimate, and compares it against the difference between current
time and last filter update time.

@param  pz_Filter: Pointer to time sync filter state
@param  t_CurrentTimeTick: Current time tick (msec)

@return TRUE if offset might have drifted enough that update is needed
        FALSE otherwise
*/
boolean slim_TimeSyncShouldTimeUpdate(slim_TimeSyncFilterStateStructType *pz_Filter,
                                      uint64 t_CurrentTimeTick);

/**
@brief Is time sync valid.

This function returns TRUE if time sync is valid, and FALSE
otherwise. If time sync is valid, then it can be used to
convert timestamps from remote time to local time. One
might also want to check the time sync uncertainty.

@param  pz_Filter: Pointer to time sync filter state

@return TRUE if time sync is valid
        FALSE otherwise
*/
boolean slim_TimeSyncIsValid(const slim_TimeSyncFilterStateStructType *pz_Filter);

/**
@brief Time sync uncertainty.

This function returns the uncertainty estimate of remote to
local timestamp conversion.

@param  pz_Filter: Pointer to time sync filter state

@return Uncertainty of remote to local time conversion
*/
uint64 slim_TimeSyncUnc(const slim_TimeSyncFilterStateStructType *pz_Filter);

/**
@brief Convert remote time to local time.

This function converts remote time to local time using
time sync filter.

@param  pz_Filter: Pointer to time sync filter state
@param  q_RemoteTime: 32bit remote timestamp in milliseconds
@param  pt_LocalTime: Pointer to 64bit local time in milliseconds

@return TRUE if conversion was successful (time sync was valid)
        FALSE if conversion failed (time sync was invalid)
*/
boolean slim_TimeSyncRemoteToLocal(const slim_TimeSyncFilterStateStructType *pz_Filter,
                                   uint32 q_RemoteTime, uint64 *pt_LocalTime);

/**
@brief Initializes monotonous offset.

This function initializes monotonous offset with the given time sync filter.

@param  pz_Filter: Pointer to time sync filter state
@param  pz_MtOffset: Pointer to monotonous offset structure.
*/
void slim_TimeSyncInitMtOffset
(
  const slim_TimeSyncFilterStateStructType *pz_Filter,
  slim_TimeSyncMtOffsetStructType *pz_MtOffset
);

/**
@brief Introduces offset to monotonous offset.

This function is used to introduce offset to monotonous
offset, in order to get it closer to KF estimated offset.
This function should be called every now and then to keep
monotonous offset in sync with KF offset.

Monotonous offset guarantees that when converting monotonous
timestamps from remote time to local time, the timestamps
are monotonous also in local time. When a gap in timestamps
(for example between sensor samples) is known, this function
can be used to synchronize monotonous offset, but only by
q_MaxOffsetToIntroduceMsec at a time. For example, if gap
between sensor samples is 10ms, then by calling this between
the samples with q_MaxOffsetToIntroduceMsec = 9, we change
the offset by maximum of 9ms, and the relative ordering of
sensor samples is maintained.

@param  pz_Filter: Pointer to time sync filter state
@param  pz_MtOffset: Pointer to monotonous offset structure.
@param  q_MaxOffsetToIntroduceMsec: Maximum number of milliseconds
                                    that the offset can change
*/
void slim_TimeSyncIntroduceMtOffset
(
  const slim_TimeSyncFilterStateStructType *pz_Filter,
  slim_TimeSyncMtOffsetStructType *pz_MtOffset,
  uint32 q_MaxOffsetToIntroduceMsec
);

/**
@brief Is monotonous offset valid.

This function returns TRUE if monotonous offset is valid, and FALSE
otherwise. If monotonous offset is valid, then it can be used to
convert timestamps from remote time to local time. One
might also want to check the monotonous offset uncertainty.

@param  pz_MtOffset: Pointer to monotonous offset structure.

@return TRUE if monotonous offset is valid
        FALSE otherwise
*/
boolean slim_TimeSyncMtOffsetIsValid
(
  const slim_TimeSyncMtOffsetStructType *pz_MtOffset
);

/**
@brief Convert remote time to local time using monotonous offset.

This function converts remote time to local time using
time sync filter monotonous offset. Note that the offset itself
is not monotonous, but it will change only when
slim_TimeSyncIntroduceMtOffset() is called, and only by specified
amount, which enables conversion of monotonous timestamps from
remote time to local time such that also converted timestamps
are monotonous.

@param  pz_Filter: Pointer to time sync filter MtOffset was derived
                   from
@param  pz_MtOffset: Pointer to monotonous offset structure.
@param  q_RemoteTime: 32bit remote timestamp in milliseconds
@param  pt_LocalTime: Pointer to 64bit local time in milliseconds

@return TRUE if conversion was successful (time sync was valid)
        FALSE if conversion failed (time sync was invalid)
*/
boolean slim_TimeSyncMtRemoteToLocal
(
  const slim_TimeSyncFilterStateStructType *pz_Filter,
  const slim_TimeSyncMtOffsetStructType *pz_MtOffset,
  uint32 q_RemoteTime,
  uint64 *pt_LocalTime
);

/**
@brief Time sync monotonous offset uncertainty.

This function returns the uncertainty estimate of remote to
local timestamp conversion using monotonous offset.

@param  pz_Filter: Pointer to time sync filter state
@param  pz_MtOffset: Pointer to monotonous offset structure.

@return Uncertainty of remote to local time conversion using
        monotonous offset
*/
uint64 slim_TimeSyncMtUnc
(
  const slim_TimeSyncFilterStateStructType *pz_Filter,
  const slim_TimeSyncMtOffsetStructType *pz_MtOffset
);

#ifdef __cplusplus
}
#endif

#endif
