#ifndef QMI_TEST_CLNT_COMMON_STATS_H
#define QMI_TEST_CLNT_COMMON_STATS_H
/******************************************************************************
  -----------------------------------------------------------------------------
  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef enum
{
  QMI_TEST_STANDARD_PRINT_FORMAT,
  QMI_TEST_SINGLE_LINE_HEADER_PRINT_FORMAT,
  QMI_TEST_SINGLE_LINE_PRINT_FORMAT
}qmi_test_print_format_enum;

typedef struct
{
  uint32_t rc;
  uint32_t test_duration_ms;
  uint32_t num_loops_completed;
  uint32_t num_fwd_bytes_per_txn;
  uint32_t num_ret_bytes_per_txn;
  uint32_t num_bytes_transfered;
  double data_transfer_rate_kbps;
  double latency_ms;
}qmi_test_clnt_results_type;


/*===========================================================================
  FUNCTION  qmi_test_clnt_common_calc_stats
===========================================================================*/
/*!
@brief
  Calculate Stats of test

@return
  returns statistics of test in  qmi_test_clnt_results_type

@note
  - Dependencies  qmi_test_clnt_results_type must be pre-allocated

  - Side Effects
*/
/*=========================================================================*/
void qmi_test_clnt_common_calc_stats(
  uint32_t num_loops,
  uint32_t data_size_bytes,
  uint32_t ret_data_size_bytes,
  uint32_t test_duration_ms,
  qmi_test_clnt_results_type *results);


/*===========================================================================
  FUNCTION  qmi_test_clnt_common_calc_stats_timespec
===========================================================================*/
/*!
@brief
  Calculate Stats of test using timespec of start and end times

@return
  returns statistics of test in  qmi_test_clnt_results_type

@note
  - Dependencies  qmi_test_clnt_results_type must be pre-allocated

  - Side Effects
*/
/*=========================================================================*/
void qmi_test_clnt_common_calc_stats_timespec(
  uint32_t num_loops,
  uint32_t data_size_bytes,
  uint32_t ret_data_size_bytes,
  struct timespec *start_time_info,
  struct timespec *end_time_info,
  qmi_test_clnt_results_type *results);


/*===========================================================================
  FUNCTION  qmi_test_clnt_common_print_stats
===========================================================================*/
/*!
@brief
  Print Test Results

@return
  N/A

@note
  - Dependencies  qmi_test_clnt_results_type must be pre-allocated

  - Side Effects
*/
/*=========================================================================*/
void qmi_test_clnt_common_print_stats(qmi_test_clnt_results_type *results, qmi_test_print_format_enum format);

#endif /* QMI_TEST_CLNT_COMMON_STATS_H */
