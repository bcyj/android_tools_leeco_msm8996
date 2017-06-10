/******************************************************************************
  @file: SlimProviderTimeSyncFilter.c
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

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "SlimProviderTimeSyncFilter.h"

#define LOG_TAG "slim_time_sync_filter"
#include "log_util.h"

typedef struct
{
   bool b_FilterInit;              /* true:  Filter initialized */
                                   /* false: Filter not initialized */
   uint64_t t_TimeUpdate;          /* Sys time of the last filter update */
   uint8_t  u_OutlierCount;
   double  d_FiltMeas;             /* Filtered Measurement - given when retrieved */
                                   /* Not used for calculations because this value */
                                   /* is used even when filter is not properly initialized */

   double  d_IntFiltMeas;          /* Internal Filtered Measurement. */
                                   /* Used for all calculations */
   double  d_FiltMeasVar2;         /* Variance of filtered measurement */

   uint8_t outlier_limit;          /* Number of outliers before reset */
   noise_func meas_noise_func;     /* Func provided to calculate measurement noise */
   noise_func proc_noise_func;     /* Func provided to calculate process noise */
   uint8_t n_var_thresh;           /* Expect/Meas variance scalar to determine outliers */
   uint32_t min_unc_to_init;       /* Minimum unc allowed to initialize filter */
   uint32_t diverge_threshold;     /* Min Residual used to check for outliers */
   uint64_t time_update_interval;  /* How often to update the filter */
} FilterDataStructType;

/*===========================================================================

  FUNCTION:   slim_time_sync_kf_ctor

  ===========================================================================*/
int slim_time_sync_kf_ctor(void** slim_time_sync_kf_state, slim_time_sync_kf_ctor_type* init_params)
{
    if ( slim_time_sync_kf_state == NULL || init_params == NULL)
    {
        return -1;
    }

    FilterDataStructType* tmp_slim_time_sync_kf_state = (FilterDataStructType*)calloc(1, sizeof(FilterDataStructType));
    if ( tmp_slim_time_sync_kf_state == NULL )
    {
        return -2;
    }

    tmp_slim_time_sync_kf_state->meas_noise_func = init_params->meas_noise_func;
    tmp_slim_time_sync_kf_state->proc_noise_func = init_params->proc_noise_func;
    tmp_slim_time_sync_kf_state->outlier_limit = init_params->outlier_limit;
    tmp_slim_time_sync_kf_state->n_var_thresh = init_params->n_var_thresh;
    tmp_slim_time_sync_kf_state->min_unc_to_init = init_params->min_unc_to_init;
    tmp_slim_time_sync_kf_state->diverge_threshold = init_params->diverge_threshold;
    tmp_slim_time_sync_kf_state->time_update_interval = init_params->time_update_interval;

    *slim_time_sync_kf_state = tmp_slim_time_sync_kf_state;

    return 0;
}

/*===========================================================================

  FUNCTION:   slim_time_sync_kf_destroy

  ===========================================================================*/
int slim_time_sync_kf_destroy(void** slim_time_sync_kf_state)
{
    if ( slim_time_sync_kf_state == NULL )
    {
        return -1;
    }

    free(*slim_time_sync_kf_state);
    *slim_time_sync_kf_state = NULL;

    return 0;
}

/*===========================================================================

  FUNCTION:   slim_time_sync_kf_init_filter

  ===========================================================================*/
int slim_time_sync_kf_init_filter(void* slim_time_sync_kf_state, slim_time_sync_kf_meas* unfilt_meas, uint64_t t_TimeUpdate)
{
    if ( slim_time_sync_kf_state == NULL || unfilt_meas == NULL)
    {
        return -1;
    }

    FilterDataStructType* kf_state = (FilterDataStructType*)slim_time_sync_kf_state;

    if ( (!kf_state->b_FilterInit) &&
         (unfilt_meas->d_MeasUnc <= kf_state->min_unc_to_init) )
    {
        LOC_LOGI("%s: Filter initialized", __FUNCTION__);

        kf_state->t_TimeUpdate = t_TimeUpdate;
        kf_state->u_OutlierCount = 0;
        kf_state->b_FilterInit = true;
        kf_state->d_IntFiltMeas = unfilt_meas->d_Meas;
        kf_state->d_FiltMeas = unfilt_meas->d_Meas;
        kf_state->d_FiltMeasVar2 = (unfilt_meas->d_Meas*unfilt_meas->d_Meas);

        LOC_LOGI("%s: Filtered timeMs=%llu, FiltMeas=%f", __FUNCTION__,
                 t_TimeUpdate, kf_state->d_FiltMeas);
    }

    return 0;
}

/*===========================================================================

  FUNCTION:   slim_time_sync_kf_filter_update

  ===========================================================================*/
int slim_time_sync_kf_filter_update(void* slim_time_sync_kf_state, slim_time_sync_kf_meas* unfilt_meas, uint64_t t_TimeUpdate)
{
    double d_DeltaTime = 0;
    double d_FiltVarProp2 = 0;
    double d_Residual = 0;
    double d_MeasVar2 = 0;
    double d_ExpectedVar2 = 0;
    double d_KfGain = 0;
    bool b_UpdateFilter = true;

    if ( slim_time_sync_kf_state == NULL || unfilt_meas == NULL)
    {
        return -1;
    }

    FilterDataStructType* kf_state = (FilterDataStructType*)slim_time_sync_kf_state;

    /* Filter not initialized. Use the unfiltered meas. */
    if ( !kf_state->b_FilterInit )
    {
        LOC_LOGI("%s: Filter not initialized. Using unfiltered meas.", __FUNCTION__);
        kf_state->d_FiltMeas = unfilt_meas->d_Meas;
        return 0;
    }

    LOC_LOGI("%s: Unfiltered timeMs=%llu, unFiltMeas=%f, unFiltMeasUnc=%f", __FUNCTION__,
             t_TimeUpdate, unfilt_meas->d_Meas, unfilt_meas->d_MeasUnc);

    /* A filter update is possible. Set as true initially, and then lower to
       FALSE if a condition below is met. */
    b_UpdateFilter = true;

    /* Compute time difference between time of current TsOffset estimate and last
       filter update */
    d_DeltaTime = (double)(t_TimeUpdate - kf_state->t_TimeUpdate);

    /* Only update the filter when specified. */
    if ( d_DeltaTime >= kf_state->time_update_interval )
    {
        d_FiltVarProp2 = kf_state->d_FiltMeasVar2;

        /* Form Kalman filter measurement residual and measured variance */
        d_Residual = unfilt_meas->d_Meas - kf_state->d_IntFiltMeas;
        d_MeasVar2 = d_Residual * d_Residual;

        /* Compute the expected variance */
        d_ExpectedVar2 = d_FiltVarProp2 +
            (unfilt_meas->d_MeasUnc * unfilt_meas->d_MeasUnc);

        /* Perform chi-squared test and residual threshold test for outlier
         * rejection.
         */
        LOC_LOGI("%s: Filter d_Residual=%f d_ExpectedVar2=%f, d_MeasVar2=%f",
                 __FUNCTION__,
                 d_Residual,
                 d_ExpectedVar2,
                 d_MeasVar2);
        if ( (fabs(d_Residual) > kf_state->diverge_threshold) &&
             (d_MeasVar2 > ( kf_state->n_var_thresh * d_ExpectedVar2)) )
        {
            b_UpdateFilter = false;
            //kf_state->t_TimeUpdate = t_TimeUpdate;
            kf_state->u_OutlierCount++;

            LOC_LOGI("%s: Filter outlier detection. timeMs=%llu, d_MeasVar2=%.3f, d_ExpectedVar2=%.3f", __FUNCTION__,
                     t_TimeUpdate, d_MeasVar2, d_ExpectedVar2);

            if (kf_state->outlier_limit <= kf_state->u_OutlierCount)
            {
                /* Reset KF */
                kf_state->b_FilterInit = false;

                LOC_LOGI("%s: Filter reset. timeMs=%llu, outlier count=%d", __FUNCTION__,
                         t_TimeUpdate, kf_state->u_OutlierCount);

                /* Populate the WLS output with the TsOffset filter state */
                kf_state->d_IntFiltMeas = unfilt_meas->d_Meas;
                kf_state->d_FiltMeas = unfilt_meas->d_Meas;
                kf_state->u_OutlierCount = 0;

                LOC_LOGI("%s: Filtered timeMs=%llu, FiltMeas=%f", __FUNCTION__,
                         t_TimeUpdate, kf_state->d_FiltMeas);

                /* return early */
                return 0;
            }
        }
        else
        {
            /* filter reset only if consecutive outliers */
            kf_state->u_OutlierCount = 0;
        }

        /* If the above test passes, then update the Kalman filter */
        if ( b_UpdateFilter )
        {
            /* Add process noise/measurement noise to input uncertainty. */
            d_FiltVarProp2 += kf_state->proc_noise_func(d_DeltaTime);
            d_ExpectedVar2 += kf_state->meas_noise_func(d_DeltaTime);

            /* Compute Kalman gain */
            d_KfGain = d_FiltVarProp2 / d_ExpectedVar2;

            LOC_LOGV("%s: Filter. d_DeltaTimeMs=%d, d_FiltVarProp2=%d, d_KfGain(x1000)=%d", __FUNCTION__,
                     (int32_t)(d_DeltaTime), (int32_t)(d_FiltVarProp2), (int32_t)(1000*d_KfGain));

            /* Update filter state */
            kf_state->d_IntFiltMeas += d_KfGain * d_Residual;

            /* Update state error covariance */
            kf_state->d_FiltMeasVar2 = ( 1.0 - d_KfGain ) * d_FiltVarProp2;

            /* Update filter time to the time of the raw data */
            kf_state->t_TimeUpdate = t_TimeUpdate;

            kf_state->d_FiltMeas = kf_state->d_IntFiltMeas;
        }

        LOC_LOGI("%s: Filtered timeMs=%llu, FiltMeas=%f", __FUNCTION__,
                 t_TimeUpdate, kf_state->d_FiltMeas);

    }

    return 0;
}

/*===========================================================================

  FUNCTION:   slim_time_sync_kf_get_filt_meas

  ===========================================================================*/
int slim_time_sync_kf_get_filt_meas(void* slim_time_sync_kf_state, double* filt_meas)
{
    if ( slim_time_sync_kf_state == NULL || filt_meas == NULL)
    {
        return -1;
    }

    FilterDataStructType* kf_state = (FilterDataStructType*)slim_time_sync_kf_state;
    *filt_meas = kf_state->d_FiltMeas;

    return 0;
}

/*===========================================================================

  FUNCTION:   slim_time_sync_kf_get_filt_meas_unc

  ===========================================================================*/
int slim_time_sync_kf_get_filt_meas_unc(void* slim_time_sync_kf_state, double* filt_meas_unc)
{
    if ( slim_time_sync_kf_state == NULL || filt_meas_unc == NULL)
    {
        return -1;
    }

    FilterDataStructType* kf_state = (FilterDataStructType*)slim_time_sync_kf_state;
    *filt_meas_unc = sqrt(kf_state->d_FiltMeasVar2);

    return 0;
}

