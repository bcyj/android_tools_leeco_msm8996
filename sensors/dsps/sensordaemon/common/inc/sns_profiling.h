#ifndef SNS_PROFILING_H
#define SNS_PROFILING_H

/*============================================================================
  @file sns_profiling.h

  The profiling code header file

  Copyright (c) 2012-2014 by Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential
============================================================================*/

#ifdef SNS_QDSS_SWE
#include <stdio.h>
#include <qurt.h>
#include "qurt_timer.h"
#include "qurt_elite.h"
#include "tracer.h"
#include "tracer_event_ids.h"
#include "qurt_island.h"
#endif /* SNS_QDSS_SWE */

#define SNS_PWR_PROFILING_BUF_SIZE 4096

/* SMGR process latency profiling markers */
#define SNS_SMGR_LATENCY_PROFILE_START  0x11111111
#define SNS_SMGR_LATENCY_PROFILE_END    0x66666666

/* SAM algorithm execution time profiling markers */
#define SNS_SAM_ALGO_EXEC_PROFILE_START 0x77777777
#define SNS_SAM_ALGO_EXEC_PROFILE_END   0x55555555

/* SMGR & driver function makers for QDSS */
#define SNS_QDSS_DD_GET_DATA 0
#define SNS_QDSS_DD_HANDLE_TIMER 1
#define SNS_QDSS_DD_HANDLE_IRQ 2

#ifdef SNS_QDSS_SWE
#define sns_profiling_log_qdss(a, b, ... ) \
        (b==0)? \
        tracer_event_simple( (tracer_event_id_t)a ): \
        tracer_event_simple_vargs( (tracer_event_id_t)a, b, ## __VA_ARGS__ )
typedef tracer_event_id_t sns_tracer_event;
#else
#define sns_profiling_log_qdss(a, b, ... )
typedef uint8_t sns_tracer_event;

typedef enum
{
  SNS_SAM_ENTER,
  SNS_SAM_EXIT,
  SNS_SAM_ALGO_UPDATE_ENTER,
  SNS_SAM_ALGO_UPDATE_EXIT,
  SNS_SAM_ALGO_PROCESS_INPUT_ENTER,
  SNS_SAM_ALGO_PROCESS_INPUT_EXIT,
  SNS_SAM_ALGO_REPORT_ENTER,
  SNS_SAM_ALGO_REPORT_EXIT,
  SNS_SAM_PROCESS_IND_START,
  SNS_SAM_PROCESS_IND_END,
  SNS_SAM_PROCESS_IND_ENTER,
  SNS_SAM_PROCESS_IND_EXIT,
  SNS_SAM_ALGO_RUN_ENTER,
  SNS_SAM_ALGO_RUN_EXIT,
  SNS_SAM_UIMAGE_ENTER,
  SNS_SAM_UIMAGE_EXIT,
  SNS_SAM_ALGO_ENABLE_ENTER,
  SNS_SAM_ALGO_ENABLE_EXIT,
  SNS_SAM_ALGO_CB_ADD_INPUT_ENTER,
  SNS_SAM_ALGO_CB_ADD_INPUT_EXIT,
  SNS_SAM_ALGO_CB_ADD_OUTPUT_ENTER,
  SNS_SAM_ALGO_CB_ADD_OUTPUT_EXIT,
  SNS_SAM_ALGO_CB_LOG_SUBMIT_ENTER,
  SNS_SAM_ALGO_CB_LOG_SUBMIT_EXIT
} sns_sam_qdss_event;

#endif /* SNS_QDSS_SWE */

typedef struct {
  uint64_t code_loc;
  uint64_t timestamp;
}sns_pwr_prof_struct;

typedef struct {
  boolean  ltcy_measure_enabled;
  uint32_t polling_cb_ts;
  uint32_t polling_get_data_start_ts;
  uint32_t polling_get_data_end_ts;
  uint32_t dri_notify_irq_ts;
  uint32_t dri_get_data_start_ts;
  uint32_t dri_get_data_end_ts;
}sns_profiling_latency_s;

extern sns_profiling_latency_s sns_latency;

/* Get the current location and timestamp */
void sns_profiling_log_timestamp(uint64_t curr_loc_tag);

/* Initiate the profiling code */
void sns_profiling_init(void);

/* Sampling latency measurement for Polling mode */
void sns_profiling_log_latency_poll(
        sns_profiling_latency_s    latency,
        int32_t                    sensor_id);

/* Sampling latency measurement for DRI mode */
void sns_profiling_log_latency_dri(
        sns_profiling_latency_s    latency,
        int32_t                    sensor_id);

#endif /* SNS_PROFILING_H */
