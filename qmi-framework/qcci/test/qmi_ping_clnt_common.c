/******************************************************************************
  -----------------------------------------------------------------------------
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include "qmi_cci_target.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_common.h"
#include "qmi_ping_clnt_common.h"
#include "qmi_ping_api_v01.h"

static volatile int test_completed = 0;
static struct itimerval  timer_test_completion;
static int pending_async = 0;
pthread_mutex_t async_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t async_count_cond = PTHREAD_COND_INITIALIZER;


static inline int zero_timer(struct itimerval *val)
{
  val->it_value.tv_sec = 0;
  val->it_value.tv_usec = 0;
  val->it_interval.tv_sec = 0;
  val->it_interval.tv_usec = 0;

  return setitimer(ITIMER_REAL, val, NULL);
}

static inline int set_timer_ms(struct itimerval *val, int ms)
{
  val->it_value.tv_sec = ms/1000;
  val->it_value.tv_usec = (ms*1000) % 1000000;
  val->it_interval.tv_sec = 0;
  val->it_interval.tv_usec = 0;

  return setitimer(ITIMER_REAL, val, NULL);
}

/*===========================================================================
  FUNCTION   test_completion_timer_cb_func
===========================================================================*/
/*!
@brief
  Test is complete
@return
  N/A

@note

*/
/*=========================================================================*/
static void test_completion_timer_cb_func(int a)
{
  int ret;
  test_completed=1;

  ret = zero_timer(&timer_test_completion);
  if(ret)
  {
    perror("setitimer");
  }
}


/*=============================================================================
  CALLBACK FUNCTION ping_clnt_error_cb
=============================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an REMOVE SERVER message from the modem
*/
void ping_error_cb(
  qmi_client_type clnt,
  qmi_client_error_type error,
  void *error_cb_data)
{
  printf("%s: with %d called for %p\n", __func__, error, (void *)clnt);
  pthread_mutex_lock(&async_count_mutex);
  pending_async = 0;
  pthread_cond_signal(&async_count_cond);
  pthread_mutex_unlock(&async_count_mutex);
}


/*=============================================================================
  CALLBACK FUNCTION ping_rx_cb
=============================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an asynchronous response for this client

@param[in]   user_handle         Opaque handle used by the infrastructure to
                                                                 identify different services.

@param[in]   msg_id              Message ID of the response

@param[in]   buf                 Buffer holding the decoded response

@param[in]   len                 Length of the decoded response

@param[in]   resp_cb_data        Cookie value supplied by the client

@param[in]   transp_err          Error value

*/
/*=========================================================================*/
void ping_rx_cb
(
 qmi_client_type                user_handle,
 unsigned long                  msg_id,
 void                           *buf,
 int                            len,
 void                           *resp_cb_data,
 qmi_client_error_type          transp_err
 )
{
  int i;
  uint32_t rx_csum = 0, calc_csum = 0;
  ping_data_resp_msg_v01 *data_resp = (ping_data_resp_msg_v01 *)buf;
  uint8_t *byte_data = (uint8_t *)(data_resp->data);

  memcpy(&rx_csum, byte_data, sizeof(uint32_t));
#ifdef QMI_PING_CHECKSUM
  for (i = 0; i < data_resp->data_len; i++)
    calc_csum = calc_csum ^ byte_data[i + 4];

  if (rx_csum != calc_csum) {
    printf("rx_csum (%d) != calc_csum (%d)\n", rx_csum, calc_csum);
    printf("RX_data\n");
    for(i = 0; i < data_resp->data_len; i++)
    {
      printf("%02x ", byte_data[i]);
    }
  }
#endif

  pthread_mutex_lock(&async_count_mutex);
  pending_async--;
  pthread_cond_signal(&async_count_cond);
  pthread_mutex_unlock(&async_count_mutex);
}

/*===========================================================================
  FUNCTION  qmi_ping_clnt_common_sync_basic_test
===========================================================================*/
/*!
@brief
  Common sync basic test
  Runs a qmi ping sync basic test for with parameters:
  test_length : max number of qmi transactions
  data_size_bytes: size of data block to send
  test_duration_ms: duration of test

@return
  0 for pass, -1 for fail
  returns statistics of test in  qmi_ping_clnt_results_type

@note
  - Dependencies  qmi_ping_clnt_results_type must be pre-allocated

  - Side Effects
*/
/*=========================================================================*/
int qmi_ping_clnt_common_sync_basic_test(
  qmi_client_type *clnt,
  uint32_t test_length,
  uint32_t test_duration_ms,
  uint32_t verbose,
  qmi_ping_clnt_results_type *results
  )
{
  uint32_t i, ret, errcnt = 0;
  struct timespec time_info_start;
  struct timespec time_info_end;
  struct timespec time_info_last;
  struct timespec time_info_current;
  uint32_t num_loops = 0;
  ping_req_msg_v01 req;
  ping_resp_msg_v01 resp;

  test_completed = 0;

  req.client_name_valid = 0;
  memcpy(&req, "ping", 4);
  /* If the test is specified with a test_duration_ms
     Then execute the test for this duration, if not,
     loop for test_length */

  clock_gettime(CLOCK_REALTIME,&time_info_last);
  if((0 == test_duration_ms) && (0 == test_length))
  {
    return 0;
  }
  else if(0 == test_duration_ms)
  {
    clock_gettime(CLOCK_REALTIME,&time_info_start);
    for(i = 0;i < test_length; i++)
    {
      ret = qmi_client_send_msg_sync(*clnt, QMI_PING_REQ_V01, &req, sizeof(req),
                                      &resp, sizeof(resp), 0);
      if (ret != 0)
      {
        printf("PING: send_msg_sync error: %d\n",ret);
        errcnt++;
      }
    }
    clock_gettime(CLOCK_REALTIME,&time_info_end);
    qmi_ping_clnt_common_calc_stats_timespec((test_length - errcnt), 4, 4,
                                           &time_info_start, &time_info_end, results);
  }
  else
  {
    signal(SIGALRM, test_completion_timer_cb_func);
    ret = set_timer_ms(&timer_test_completion, test_duration_ms);
    if(ret) {
      perror("setitimer");
      errcnt++;
      return -errcnt;
    }

    /* Test ran for a fixed period of time*/
    clock_gettime(CLOCK_REALTIME,&time_info_start);
    while((0 == test_completed) && (0 == errcnt))
    {
      ret = qmi_client_send_msg_sync(*clnt, QMI_PING_REQ_V01, &req, sizeof(req),
                                      &resp, sizeof(resp), 0);
      if (ret != 0)
      {
        printf("PING: send_msg_sync error: %d\n",ret);
        errcnt++;
      }
      num_loops++;

      if( verbose )
      {
        clock_gettime(CLOCK_REALTIME,&time_info_current);
        if( time_info_current.tv_sec > (time_info_last.tv_sec ) )
        {
          time_info_last.tv_sec  = time_info_current.tv_sec ;
          printf("QMI txns completed: %5d Num Errors:%d\n",(int)(num_loops - errcnt),(int)errcnt);
        }
      }

#ifdef FEATURE_QMI_PING_TIMER_BASED_TEST
      if( num_loops >= test_length )
      {
        test_completed = 1;
      }
#endif
    }
    clock_gettime(CLOCK_REALTIME,&time_info_end);
    if( errcnt == 0 )
    {
      qmi_ping_clnt_common_calc_stats_timespec(num_loops, 4, 4,
                                             &time_info_start, &time_info_end, results);
      results->rc = -errcnt;
    }
    else
    {
      memset(results, 0, sizeof(qmi_ping_clnt_results_type));
      results->rc = -errcnt;
    }
    ret = zero_timer(&timer_test_completion);
    if(ret) {
      perror("setitimer");
      errcnt++;
      return -errcnt;
    }
  }
  return(- errcnt );
}


/*===========================================================================
  FUNCTION  qmi_ping_clnt_common_sync_data_test
===========================================================================*/
/*!
@brief
  Common sync data test
  Runs a qmi ping sync data test for with parameters:
  test_length : max number of qmi transactions
  data_size_bytes: size of data block to send
  test_duration_ms: duration of test

@return
  0 for pass, -1 for fail
  returns statistics of test in  qmi_ping_clnt_results_type

@note
  - Dependencies  qmi_ping_clnt_results_type must be pre-allocated

  - Side Effects
*/
/*=========================================================================*/
int qmi_ping_clnt_common_sync_data_test(
  qmi_client_type *clnt,
  uint32_t test_length,
  uint32_t data_size_bytes,
  uint32_t test_duration_ms,
  uint32_t verbose,
  qmi_ping_clnt_results_type *results
  )
{
  uint32_t sum = 0, ret_sum;
  uint32_t i, ret, errcnt = 0;
  struct timespec time_info_start;
  struct timespec time_info_end;
  struct timespec time_info_last;
  struct timespec time_info_current;
  uint32_t num_loops = 0;
  uint8_t byte_data = 0;
  ping_data_req_msg_v01 *data_req;
  ping_data_resp_msg_v01 *data_resp;

  test_completed = 0;
  data_req = (ping_data_req_msg_v01 *)calloc(1, sizeof(ping_data_req_msg_v01));
  if(!data_req)
    return -1;
  data_resp = (ping_data_resp_msg_v01 *)calloc(1, sizeof(ping_data_resp_msg_v01));
  if(!data_resp)
  {
    free(data_req);
    return -1;
  }

  /* Setup req data block & checksum to it*/
  for(i = 0; i < data_size_bytes; i++)
  {
    byte_data = (uint8_t)(rand() & 0xff);
    data_req->data[i + 4] = byte_data;
    sum = sum ^ byte_data;
  }
  memcpy(data_req->data, &sum, sizeof(uint32_t));
  data_req->data_len = (data_size_bytes + 4);

  /* If the test is specified with a test_duration_ms
     Then execute the test for this duration, if not,
     loop for test_length */

  clock_gettime(CLOCK_REALTIME,&time_info_last);
  if((0 == test_duration_ms) && (0 == test_length))
  {
    free(data_req);
    free(data_resp);
    return 0;
  }
  else if(0 == test_duration_ms)
  {
    clock_gettime(CLOCK_REALTIME,&time_info_start);
    for(i = 0;i < test_length; i++)
    {
      ret = qmi_client_send_msg_sync(*clnt, QMI_PING_DATA_REQ_V01, data_req, sizeof(ping_data_req_msg_v01),
                                      data_resp, sizeof(ping_data_resp_msg_v01), 0);
      if (ret != 0)
      {
        printf("PING: send_msg_sync error: %d\n",ret);
        errcnt++;
      }
      memcpy(&ret_sum, data_resp->data, sizeof(uint32_t));
#ifdef QMI_PING_CHECKSUM
      if (ret_sum != sum)
      {
        printf("PING: send_msg_sync_error: Checksum mismatch\n");
        printf("tx_csum (%d) != rx_csum(%d)\n", sum, ret_sum);
        errcnt++;
      }
#endif
    }
    clock_gettime(CLOCK_REALTIME,&time_info_end);
    qmi_ping_clnt_common_calc_stats_timespec((test_length - errcnt), data_size_bytes, data_size_bytes,
                                           &time_info_start, &time_info_end, results);
  }
  else
  {
    signal(SIGALRM, test_completion_timer_cb_func);
    ret = set_timer_ms(&timer_test_completion, test_duration_ms);
    if(ret) {
      perror("setitimer");
      errcnt++;
      free(data_req);
      free(data_resp);
      return -errcnt;
    }

    /* Test ran for a fixed period of time*/
    clock_gettime(CLOCK_REALTIME,&time_info_start);
    while((0 == test_completed) && (0 == errcnt))
    {
      ret = qmi_client_send_msg_sync(*clnt, QMI_PING_DATA_REQ_V01, data_req, sizeof(ping_data_req_msg_v01),
                                      data_resp, sizeof(ping_data_resp_msg_v01), 0);
      if (ret != 0)
      {
        printf("PING: send_msg_sync error: %d\n",ret);
        errcnt++;
      }
      memcpy(&ret_sum, data_resp->data, sizeof(uint32_t));
#ifdef QMI_PING_CHECKSUM
      if (ret_sum != sum)
      {
        printf("PING: send_msg_sync_error: Checksum mismatch\n");
        printf("tx_csum (%d) != rx_csum(%d)\n", sum, ret_sum);
        errcnt++;
      }
#endif
      num_loops++;

      if( verbose )
      {
        clock_gettime(CLOCK_REALTIME,&time_info_current);
        if( time_info_current.tv_sec > (time_info_last.tv_sec ) )
        {
          time_info_last.tv_sec  = time_info_current.tv_sec ;
          printf("QMI txns completed: %5d Num Errors:%d\n",(int)(num_loops - errcnt),(int)errcnt);
        }
      }

#ifdef FEATURE_QMI_PING_TIMER_BASED_TEST
      if( num_loops >= test_length )
      {
        test_completed = 1;
      }
#endif
    }
    clock_gettime(CLOCK_REALTIME,&time_info_end);
    if( errcnt == 0 )
    {
      qmi_ping_clnt_common_calc_stats_timespec(num_loops, data_size_bytes , data_size_bytes,
                                             &time_info_start, &time_info_end, results);
      results->rc = -errcnt;
    }
    else
    {
      memset(results, 0, sizeof(qmi_ping_clnt_results_type));
      results->rc = -errcnt;
    }
    ret = zero_timer(&timer_test_completion);
    if(ret) {
      perror("setitimer");
      errcnt++;
      free(data_req);
      free(data_resp);
      return -errcnt;
    }
  }
  free(data_req);
  free(data_resp);

  return(- errcnt );
}


/*===========================================================================
  FUNCTION  qmi_ping_clnt_common_async_data_test
===========================================================================*/
/*!
@brief
  Common async data test
  Runs a qmi ping async data test for with parameters:
  test_length : max number of qmi transactions
  data_size_bytes: size of data block to send
  test_duration_ms: duration of test

@return
  0 for pass, -1 for fail
  returns statistics of test in  qmi_ping_clnt_results_type

@note
  - Dependencies  qmi_ping_clnt_results_type must be pre-allocated

  - Side Effects
*/
/*=========================================================================*/
int qmi_ping_clnt_common_async_data_test(
  qmi_client_type *clnt,
  qmi_txn_handle *txn,
  uint32_t test_length,
  uint32_t data_size_bytes,
  uint32_t test_duration_ms,
  uint32_t verbose,
  qmi_ping_clnt_results_type *results
  )
{
  uint32_t sum = 0;
  uint32_t i, ret, errcnt = 0;
  struct timespec time_info_start;
  struct timespec time_info_end;
  struct timespec time_info_last;
  struct timespec time_info_current;
  uint32_t num_loops = 0;
  uint8_t byte_data = 0;
  ping_data_req_msg_v01 *data_req;
  ping_data_resp_msg_v01 *data_resp;

  test_completed = 0;
  data_req = (ping_data_req_msg_v01 *)calloc(1, sizeof(ping_data_req_msg_v01));
  if(!data_req)
    return -1;
  data_resp = (ping_data_resp_msg_v01 *)calloc(1, sizeof(ping_data_resp_msg_v01));
  if(!data_resp)
  {
    free(data_req);
    return -1;
  }

  /* Setup req data block & checksum to it*/
  for(i = 0; i < data_size_bytes; i++)
  {
    byte_data = (uint8_t)(rand() & 0xff);
    data_req->data[i + 4] = byte_data;
    sum = sum ^ byte_data;
  }
  memcpy(data_req->data, &sum, sizeof(uint32_t));
  data_req->data_len = (data_size_bytes + 4);

  /* If the test is specified with a test_duration_ms
     Then execute the test for this duration, if not,
     loop for test_length */

  clock_gettime(CLOCK_REALTIME,&time_info_last);
  if((0 == test_duration_ms) && (0 == test_length))
  {
    free(data_req);
    free(data_resp);
    return 0;
  }
  else if(0 == test_duration_ms)
  {
    clock_gettime(CLOCK_REALTIME,&time_info_start);
    for(i = 0;i < test_length; i++)
    {
      ret = qmi_client_send_msg_async(*clnt, QMI_PING_DATA_REQ_V01, data_req, sizeof(ping_data_req_msg_v01),
                                      data_resp, sizeof(ping_data_resp_msg_v01), ping_rx_cb, 2, txn);
      if (ret != 0)
      {
        printf("PING: send_msg_async error: %d\n",ret);
        errcnt++;
      }
      else
      {
        pthread_mutex_lock(&async_count_mutex);
        pending_async++;
        pthread_mutex_unlock(&async_count_mutex);
      }
    }
    pthread_mutex_lock(&async_count_mutex);
    while (pending_async)
      pthread_cond_wait(&async_count_cond, &async_count_mutex);
    pthread_mutex_unlock(&async_count_mutex);

    clock_gettime(CLOCK_REALTIME,&time_info_end);
    qmi_ping_clnt_common_calc_stats_timespec((test_length - errcnt), data_size_bytes, data_size_bytes,
                                           &time_info_start, &time_info_end, results);
  }
  else
  {
    signal(SIGALRM, test_completion_timer_cb_func);
    ret = set_timer_ms(&timer_test_completion, test_duration_ms);
    if(ret) {
      perror("setitimer");
      errcnt++;
      free(data_req);
      free(data_resp);
      return -errcnt;
    }

    /* Test ran for a fixed period of time*/
    clock_gettime(CLOCK_REALTIME,&time_info_start);
    while((0 == test_completed) && (0 == errcnt))
    {
      ret = qmi_client_send_msg_async(*clnt, QMI_PING_DATA_REQ_V01, data_req, sizeof(ping_data_req_msg_v01),
                                      data_resp, sizeof(ping_data_resp_msg_v01), ping_rx_cb, 2, txn);
      if (ret != 0)
      {
        printf("PING: send_msg_async error: %d\n",ret);
        errcnt++;
      }
      else
      {
        pthread_mutex_lock(&async_count_mutex);
        pending_async++;
        pthread_mutex_unlock(&async_count_mutex);
      }
      num_loops++;

      if( verbose )
      {
        clock_gettime(CLOCK_REALTIME,&time_info_current);
        if( time_info_current.tv_sec > (time_info_last.tv_sec ) )
        {
          time_info_last.tv_sec  = time_info_current.tv_sec ;
          printf("QMI txns completed: %5d Num Errors:%d\n",(int)(num_loops - (errcnt + pending_async)),(int)errcnt);
        }
      }

#ifdef FEATURE_QMI_PING_TIMER_BASED_TEST
      if( num_loops >= test_length )
      {
        test_completed = 1;
      }
#endif
    }
    pthread_mutex_lock(&async_count_mutex);
    while (pending_async)
      pthread_cond_wait(&async_count_cond, &async_count_mutex);
    pthread_mutex_unlock(&async_count_mutex);

    clock_gettime(CLOCK_REALTIME,&time_info_end);

    if( errcnt == 0 )
    {
      qmi_ping_clnt_common_calc_stats_timespec(num_loops, data_size_bytes , data_size_bytes,
                                             &time_info_start, &time_info_end, results);
      results->rc = -errcnt;
    }
    else
    {
      memset(results, 0, sizeof(qmi_ping_clnt_results_type));
      results->rc = -errcnt;
    }
    ret = zero_timer(&timer_test_completion);
    if(ret) {
      perror("setitimer");
      errcnt++;
      free(data_req);
      free(data_resp);
      return -errcnt;
    }
  }
  free(data_req);
  free(data_resp);

  return(- errcnt );
}


/*===========================================================================
  FUNCTION  qmi_ping_clnt_common_calc_stats_timespec
===========================================================================*/
/*!
@brief
  Calculate Stats of test using timespec of start and end times

@return
  returns statistics of test in  qmi_ping_clnt_results_type

@note
  - Dependencies  qmi_ping_clnt_results_type must be pre-allocated

  - Side Effects
*/
/*=========================================================================*/
void qmi_ping_clnt_common_calc_stats_timespec(
  uint32_t num_loops,
  uint32_t data_size_bytes,
  uint32_t ret_data_size_bytes,
  struct timespec *start_time_info,
  struct timespec *end_time_info,
  qmi_ping_clnt_results_type *results)
{

  double start_time_us;
  double end_time_us;
  double delta_us;
  double test_duration_ms;
  uint32_t num_bytes;
  num_bytes = (data_size_bytes + ret_data_size_bytes + 8) * num_loops;

  end_time_us = (double)end_time_info->tv_nsec / (double)1000 + (double)end_time_info->tv_sec * (double)1000000;
  start_time_us = (double)start_time_info->tv_nsec / (double)1000 + (double)start_time_info->tv_sec * (double)1000000;

  delta_us = end_time_us - start_time_us;
  test_duration_ms = (delta_us)/1000;

  results->data_transfer_rate_kbps = (double)(num_bytes ) / (double)test_duration_ms;
  results->num_bytes_transfered = (data_size_bytes + ret_data_size_bytes) * num_loops;
  results->num_fwd_bytes_per_txn = data_size_bytes;
  results->num_loops_completed = num_loops;
  results->num_ret_bytes_per_txn = ret_data_size_bytes;
  results->latency_ms = (double)test_duration_ms/(double)num_loops;
  results->test_duration_ms = test_duration_ms;

}
