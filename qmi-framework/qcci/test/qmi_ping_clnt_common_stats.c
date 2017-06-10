/******************************************************************************
  -----------------------------------------------------------------------------
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "qmi_cci_target.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_common.h"
#include "qmi_ping_clnt_common_stats.h"
#include "qmi_ping_api_v01.h"


/*===========================================================================
  FUNCTION  qmi_ping_clnt_common_calc_stats
===========================================================================*/
/*!
@brief
  Calculate Stats of test

@return
  returns statistics of test in  qmi_ping_clnt_results_type

@note
  - Dependencies  qmi_ping_clnt_results_type must be pre-allocated

  - Side Effects
*/
/*=========================================================================*/
void qmi_ping_clnt_common_calc_stats(
  uint32_t num_loops,
  uint32_t data_size_bytes,
  uint32_t ret_data_size_bytes,
  uint32_t test_duration_ms,
  qmi_ping_clnt_results_type *results)
{
  uint32_t num_bytes;
  num_bytes = (data_size_bytes + ret_data_size_bytes + 8) * num_loops;
  results->data_transfer_rate_kbps = (double)(num_bytes) / (double)test_duration_ms;
  results->num_bytes_transfered = (data_size_bytes + ret_data_size_bytes) * num_loops;
  results->num_fwd_bytes_per_txn = data_size_bytes;
  results->num_loops_completed = num_loops;
  results->num_ret_bytes_per_txn = ret_data_size_bytes;
  results->latency_ms = (double)test_duration_ms/(double)num_loops;
  results->test_duration_ms = test_duration_ms;
}


/*===========================================================================
  FUNCTION  qmi_ping_clnt_common_print_stats
===========================================================================*/
/*!
@brief
  Print Test Results

@return
  N/A

@note
  - Dependencies  qmi_ping_clnt_results_type must be pre-allocated

  - Side Effects
*/
/*=========================================================================*/
void qmi_ping_clnt_common_print_stats(qmi_ping_clnt_results_type *results,
  qmi_ping_print_format_enum format)
{
  if( results )
    switch( format )
    {
      case   QMI_PING_STANDARD_PRINT_FORMAT:
        printf("Test Duration          :%d ms\n",(int)results->test_duration_ms);
        printf("Num QMI Transactions:%d\n",(int)results->num_loops_completed);
        printf("Num fwd bytes per transaction :%d\n",(int)results->num_fwd_bytes_per_txn);
        printf("Num ret bytes per call :%d\n",(int)results->num_ret_bytes_per_txn);
        printf("Num bytes transferred  :%d\n",(int)results->num_bytes_transfered);
        printf("Latency      :%05.3f ms\n",results->latency_ms);
        printf("Data Transfer Rate     :%05.3f kBytes/sec\n",results->data_transfer_rate_kbps);
        break;
      case QMI_PING_SINGLE_LINE_HEADER_PRINT_FORMAT:
        printf("Test(ms) #Txns  Latency(ms) FwdBytes   RetBytes  TotKBytes     DataRate(kBps) \n");
        break;
      case QMI_PING_SINGLE_LINE_PRINT_FORMAT:
        printf("%5d    %5d   %8.3f      %5d    %5d  %10d       %8.3f \n",
          (int)results->test_duration_ms,
          (int)results->num_loops_completed,
           results->latency_ms,
          (int)results->num_fwd_bytes_per_txn,
          (int)results->num_ret_bytes_per_txn,
          (int)results->num_bytes_transfered/1000,
        results->data_transfer_rate_kbps);
        break;
      default:
        printf("ERROR default case not handled in function %s \n",__FUNCTION__);
        break;
    }
};

