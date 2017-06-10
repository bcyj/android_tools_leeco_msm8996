/******************************************************************************
  @file:  .SlimProviderTimeSyncFilter.h
  @brief: 1-State KF w/outlier detection

  DESCRIPTION

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2011,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------

******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
07/22/11   jb       Initial version

======================================================================*/
#ifndef __SLIM_PROVIDER_TIME_SYNC_FILTER_H__
#define __SLIM_PROVIDER_TIME_SYNC_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct slim_time_sync_kf_meas
{
   double d_Meas;
   double d_MeasUnc;
} slim_time_sync_kf_meas;

typedef double (*noise_func)(double d_deltaTime);

typedef struct slim_time_sync_kf_ctor_type
{
   uint8_t outlier_limit;           /* Number of outliers before a reset */
   uint8_t n_var_thresh;            /* Expect/Meas variance threshold scalar */
   uint32_t min_unc_to_init;        /* Min unc allowed to initialize filter */
   uint32_t diverge_threshold;      /* Min Residual diff to check for outlier */
   uint64_t time_update_interval;   /* How often to update the filter */
   noise_func proc_noise_func;      /* Func used to calculate process noise */
   noise_func meas_noise_func;      /* Func used to calculate measurement noise */
} slim_time_sync_kf_ctor_type;

/*===========================================================================
FUNCTION    slim_time_sync_kf_ctor

DESCRIPTION
   Creates space and initializes the KF with the provided parameters.

DEPENDENCIES
   N/A

RETURN VALUE
   0: Success
   else failure

SIDE EFFECTS
   N/A

===========================================================================*/
int slim_time_sync_kf_ctor(void** slim_time_sync_kf_state, slim_time_sync_kf_ctor_type* init_params);

/*===========================================================================
FUNCTION    slim_time_sync_kf_destroy

DESCRIPTION
   Removes space allocated for the KF.

DEPENDENCIES
   N/A

RETURN VALUE
   0: Success
   Otherwise failure

SIDE EFFECTS
   N/A

===========================================================================*/
int slim_time_sync_kf_destroy(void** slim_time_sync_kf_state);

/*===========================================================================
FUNCTION    slim_time_sync_kf_init_filter

DESCRIPTION
   Initializes the filter with the given value if it is not initialized yet.

   unfilt_meas: Unfiltered measurement and uncertainty
   time_update: Time of this measurement in any time base as long as consistent.

DEPENDENCIES
   N/A

RETURN VALUE
   0: Success
   Otherwise failure

SIDE EFFECTS
   N/A

===========================================================================*/
int slim_time_sync_kf_init_filter(void* slim_time_sync_kf_state, slim_time_sync_kf_meas* unfilt_meas, uint64_t time_update);

/*===========================================================================
FUNCTION    slim_time_sync_kf_filter_update

DESCRIPTION
   Updates the filter given the provided unfiltered measurement

   unfilt_meas: Unfiltered measurement and uncertainty
   time_update: Time of this measurement in any time base as long as consistent.

DEPENDENCIES
   N/A

RETURN VALUE
   0: Success
   Otherwise failure

SIDE EFFECTS
   N/A

===========================================================================*/
int slim_time_sync_kf_filter_update(void* slim_time_sync_kf_state, slim_time_sync_kf_meas* unfilt_meas, uint64_t time_update);

/*===========================================================================
FUNCTION    slim_time_sync_kf_get_filt_meas

DESCRIPTION
   Utility function to return the current system time in microseconds.

   filt_meas: Output param for the current filtered measurement

DEPENDENCIES
   N/A

RETURN VALUE
   0: Success
   Otherwise failure

SIDE EFFECTS
   N/A

===========================================================================*/
int slim_time_sync_kf_get_filt_meas(void* slim_time_sync_kf_state, double* filt_meas);

/*===========================================================================
FUNCTION    slim_time_sync_kf_get_filt_meas_unc

DESCRIPTION
   Utility function to return the current system time in microseconds.

   filt_meas_unc: Output param for the current filtered measurement uncertainty

DEPENDENCIES
   N/A

RETURN VALUE
   0: Success
   Otherwise failure

SIDE EFFECTS
   N/A

===========================================================================*/
int slim_time_sync_kf_get_filt_meas_unc(void* slim_time_sync_kf_state, double* filt_meas_unc);

#ifdef __cplusplus
}
#endif

#endif /* __SLIM_PROVIDER_TIME_SYNC_FILTER_H__ */
