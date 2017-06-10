/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file defines the media/module/master controller's interface with the
   DSPS modules. The functionalities od this module include:

   1. Control communication with the sensor module
   2. Process data received from the sensors

============================================================================*/
#include "dsps_hw.h"
#ifdef FEATURE_GYRO
#include <sensor1.h>
#include <sns_smgr_api_v01.h>
#include <sns_sam_gyroint_v01.h>
#include <sns_time_api_v02.h>
#include <sns_reg_api_v02.h>

void dsps_callback(intptr_t *data, sensor1_msg_header_s *msg_hdr,
  sensor1_msg_type_e msg_type, void *msg_ptr);
#endif
/* Helper functions */
/*===========================================================================
 * FUNCTION      dsps_cirq_init
 *
 * DESCRIPTION   Initialize the gyro queue
 *=========================================================================*/

void dsps_cirq_init(void *sensor_config)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)sensor_config;
  memset(dsps_config->queue, 0, MAX_Q_SIZE * sizeof(cirq_node));
  dsps_config->q_pos = 0;
  dsps_config->q_size = 0;
}

/*===========================================================================
 * FUNCTION      dsps_cirq_deinit
 *
 * DESCRIPTION   Initialize the gyro queue
 *=========================================================================*/
void dsps_cirq_deinit(sensor1_config_t *dsps_config)
{
  memset(dsps_config->queue, 0, MAX_Q_SIZE * sizeof(cirq_node));
  dsps_config->q_pos = 0;
  dsps_config->q_size = 0;
}

/*===========================================================================
 * FUNCTION      dsps_cirq_add
 *
 * DESCRIPTION   Add sensor data to the queue
 *=========================================================================*/
void dsps_cirq_add(void *config, void *data, uint8_t seqnum)
{
#ifdef FEATURE_GYRO
  sns_sam_gyroint_report_ind_msg_v01 *sensor_data =
      (sns_sam_gyroint_report_ind_msg_v01 *) data;
  sensor1_config_t * dsps_config = (sensor1_config_t *)config;

  pthread_mutex_lock(&(dsps_config->data_mutex));

  dsps_config->queue[dsps_config->q_pos].data.seqnum = seqnum;
  dsps_config->queue[dsps_config->q_pos].data.x =
      sensor_data->frame_info.angle[0];
  dsps_config->queue[dsps_config->q_pos].data.y =
      sensor_data->frame_info.angle[1];
  dsps_config->queue[dsps_config->q_pos].data.z =
      sensor_data->frame_info.angle[2];
  dsps_config->q_pos = (dsps_config->q_pos + 1) % MAX_Q_SIZE;
  if (dsps_config->q_size <  MAX_Q_SIZE) {
    dsps_config->q_size = dsps_config->q_size++;
  }

  pthread_mutex_unlock(&(dsps_config->data_mutex));
#endif
}

/*===========================================================================
 * FUNCTION      dsps_cirq_get_last
 *
 * DESCRIPTION   Get last available sample
 *=========================================================================*/
int dsps_cirq_get_last(void *config, void *data)
{
  int i;
  struct cirq_node  q_data;
  sensor1_config_t * dsps_config = (sensor1_config_t *)config;
  buffer_data_t *sensor_data = (buffer_data_t *)data;

  pthread_mutex_lock(&(dsps_config->data_mutex));

  if (!dsps_config->q_size) {
    pthread_mutex_unlock(&(dsps_config->data_mutex));
    return -1;
  }

  memcpy(sensor_data, &dsps_config->queue[dsps_config->q_pos - 1].data,
      sizeof(buffer_data_t));

  pthread_mutex_unlock(&(dsps_config->data_mutex));
  return 0;
}

/*===========================================================================
 * FUNCTION      dsps_cirq_search
 *
 * DESCRIPTION   Search for sensor data based on unique id.(frame id)
 *=========================================================================*/
int dsps_cirq_search(void *config, void *data, uint8_t id)
{
  int i;
  struct cirq_node  q_data;
  sensor1_config_t *dsps_config = (sensor1_config_t *)config;
  buffer_data_t *sensor_data = (buffer_data_t *)data;

  pthread_mutex_lock(&(dsps_config->data_mutex));

  if (!dsps_config->q_size) {
    pthread_mutex_unlock(&(dsps_config->data_mutex));
    return -1;
  }
  for (i = 0; i < dsps_config->q_size; i++) {
    if (id == dsps_config->queue[i].data.seqnum) {
      memcpy(sensor_data, &(dsps_config->queue[i].data),
        sizeof(buffer_data_t));
      pthread_mutex_unlock(&(dsps_config->data_mutex));
      return 0;
    }
  }

  pthread_mutex_unlock(&(dsps_config->data_mutex));
  return -1;
}

/*===========================================================================
 * FUNCTION      dsps_close
 *
 * DESCRIPTION   Close a connection with the sensor framework
 *==========================================================================*/
int dsps_close(sensor1_config_t *dsps_config)
{
  if (dsps_config == NULL)
    return -1;
#ifdef FEATURE_GYRO
  if (sensor1_close(dsps_config->handle) != SENSOR1_SUCCESS)
    return -1;
#else
  return -1;
#endif
  return 0;
}
/*===========================================================================
 * FUNCTION      dsps_disconnect
 *
 * DESCRIPTION   Deregister an mctl client with the DSPS Thread
 *=========================================================================*/
int dsps_disconnect(void * sensor_config)
{
  int rc = 0;
  sensor1_config_t * dsps_config = (sensor1_config_t *)sensor_config;

  if (dsps_close(dsps_config) < 0) {
    CDBG_ERROR("%s Error in closing sensor connection", __func__);
    rc = -1;
  }
  dsps_cirq_deinit(dsps_config);
  pthread_mutex_destroy(&(dsps_config->data_mutex));

  return rc;
}

/*===========================================================================
 * FUNCTION      dsps_open
 *
 * DESCRIPTION   Open a new connection with the sensor framework
 *==========================================================================*/
int dsps_open(void *sensor_config)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)sensor_config;
#ifdef FEATURE_GYRO
  /* Open sensor1 port */
  if (sensor1_open(&dsps_config->handle,
      (sensor1_notify_data_cb_t)&dsps_callback,
      (intptr_t)dsps_config) == SENSOR1_SUCCESS)
    return 0;
#endif
  return -1;
}

/*===========================================================================
 * FUNCTION      dsps_set_expiry_time
 *
 * DESCRIPTION   Set the expiry time for timed wait by adding timeout
 *               value to current time.
 *==========================================================================*/
void dsps_set_expiry_time(struct timespec *expiry_time)
{
  struct timeval current_time;

  gettimeofday(&current_time, NULL);
  expiry_time->tv_sec = current_time.tv_sec;
  expiry_time->tv_nsec = current_time.tv_usec * USEC_TO_NSEC;
  expiry_time->tv_sec += SENSOR_TIME_OUT * MSEC_TO_SEC;
  expiry_time->tv_sec += (expiry_time->tv_nsec + (SENSOR_TIME_OUT % SEC_TO_MSEC)
    * MSEC_TO_NSEC) / SEC_TO_NSEC;
  expiry_time->tv_nsec += (SENSOR_TIME_OUT % SEC_TO_MSEC) * MSEC_TO_NSEC;
  expiry_time->tv_nsec %= SEC_TO_NSEC;
}

/*===========================================================================
 * FUNCTION      dsps_wait_for_response
 *
 * DESCRIPTION   Wait for response from sensor until timer expires.
 *               Condition variable here is signaled from
 *               dsps_process_response()
 *==========================================================================*/
int dsps_wait_for_response(sensor1_config_t *dsps_config)
{
  int ret;
  struct timespec expiry_time;

  dsps_set_expiry_time(&expiry_time);

  pthread_mutex_lock(&(dsps_config->callback_mutex));

  /* Check if callback has already arrived */
  if (dsps_config->callback_arrived == 1) {
    CDBG_DSPS("%s: Callback received before wait\n", __func__);
    ret = 0;
    goto end;
  }

  /* Timed wait for callback */
  ret = pthread_cond_timedwait(&(dsps_config->callback_condvar),
    &(dsps_config->callback_mutex), &expiry_time);

  if ((!ret) && (dsps_config->callback_arrived == 0)) {
    CDBG_ERROR("Error! Timed wait returned without callback.\n");
    ret = -1;
  }

end:
  pthread_mutex_unlock(&(dsps_config->callback_mutex));
  return ret;
}

/*===========================================================================
 * FUNCTION      dsps_prepare_req_header
 *
 * DESCRIPTION   Prepare header for a request message
 *==========================================================================*/
#ifdef FEATURE_GYRO
void dsps_prepare_req_header(sensor1_msg_header_s *req_hdr,
    sensor1_req_data_t *msg_data)
{
  /* Prepare Message Header */
  switch (msg_data->msg_type) {
    case DSPS_ENABLE_REQ:
      req_hdr->service_number = SNS_SAM_GYROINT_SVC_ID_V01;
      req_hdr->msg_id = SNS_SAM_GYROINT_ENABLE_REQ_V01;
      req_hdr->msg_size = sizeof(sns_sam_gyroint_enable_req_msg_v01);
      req_hdr->txn_id = 0;
      break;
    case DSPS_DISABLE_REQ:
      req_hdr->service_number = SNS_SAM_GYROINT_SVC_ID_V01;
      req_hdr->msg_id = SNS_SAM_GYROINT_DISABLE_REQ_V01;
      req_hdr->msg_size = sizeof(sns_sam_gyroint_disable_req_msg_v01);
      req_hdr->txn_id = 0;
      break;  
    case DSPS_GET_REPORT:
      req_hdr->service_number = SNS_SAM_GYROINT_SVC_ID_V01;
      req_hdr->msg_id = SNS_SAM_GYROINT_GET_REPORT_REQ_V01;
      req_hdr->msg_size = sizeof(sns_sam_gyroint_get_report_req_msg_v01);
      req_hdr->txn_id = msg_data->seqnum;
      break;
    default:
      CDBG_ERROR("%s Invalid type", __func__);
  }
}
#endif

/*===========================================================================
 * FUNCTION      dsps_prepare_req_msg
 *
 * DESCRIPTION   Prepare body of a request message
 *==========================================================================*/
#ifdef FEATURE_GYRO
void dsps_prepare_req_msg(sensor1_config_t *dsps_config, void* req_msg,
    sensor1_req_data_t *msg_data)
{
  sns_sam_gyroint_enable_req_msg_v01 *enable_req_msg;
  sns_sam_gyroint_disable_req_msg_v01 *disable_req_msg;
  sns_sam_gyroint_get_report_req_msg_v01 *get_report_req_msg;

  switch (msg_data->msg_type) {
    case DSPS_ENABLE_REQ:
      enable_req_msg = (sns_sam_gyroint_enable_req_msg_v01 *)req_msg;
      enable_req_msg->sample_rate = SENSOR_SAMPLE_RATE;
      enable_req_msg->extra_sample = 1;
      enable_req_msg->enable_angle = 1;
      enable_req_msg->enable_sample = 0;
      break;
    case DSPS_DISABLE_REQ:
      disable_req_msg = (sns_sam_gyroint_disable_req_msg_v01 *)req_msg;
      disable_req_msg->instance_id = dsps_config->instance_id;
      break;
    case DSPS_GET_REPORT:
      get_report_req_msg = (sns_sam_gyroint_get_report_req_msg_v01 *)req_msg;
      get_report_req_msg->instance_id = dsps_config->instance_id;
      get_report_req_msg->seqnum = msg_data->seqnum;
       /* Use microseconds */
      get_report_req_msg->t_start = msg_data->t_start;
      get_report_req_msg->t_end = msg_data->t_end;
      get_report_req_msg->frame_info_valid = 0;
      break;
    default:
      CDBG_ERROR("%s Invalid type", __func__);
  }
}
#endif

/*===========================================================================
 * FUNCTION      allocate_req_msg_buffer
 *
 * DESCRIPTION   Allocate request msg buffer
 *==========================================================================*/
#ifdef FEATURE_GYRO
sensor1_error_e allocate_req_msg_buffer(sensor1_handle_s *handle,
  dsps_req_msg_type_t msg_type,
  void **req_msg)
{
  sensor1_error_e error;
  sns_sam_gyroint_enable_req_msg_v01 *enable_req = NULL;
  sns_sam_gyroint_disable_req_msg_v01 *disable_req = NULL;
  sns_sam_gyroint_get_report_req_msg_v01 *get_req = NULL;
  int size;

  switch (msg_type) {
    case DSPS_ENABLE_REQ:
      size = sizeof(sns_sam_gyroint_enable_req_msg_v01);
      error = sensor1_alloc_msg_buf(handle,size,
        (void **)&enable_req);
      *req_msg = (void *)enable_req;
      break;
    case DSPS_DISABLE_REQ:
      size = sizeof(sns_sam_gyroint_disable_req_msg_v01);
      error = sensor1_alloc_msg_buf(handle,size,
        (void **)&disable_req);
      *req_msg = (void *)disable_req;
      break;
    case DSPS_GET_REPORT:
      size = sizeof(sns_sam_gyroint_get_report_req_msg_v01);
      error = sensor1_alloc_msg_buf(handle,size,
        (void **)&get_req);
      *req_msg = (void *)get_req;
      break;
    default:
      CDBG_ERROR("%s Invalid type", __func__);
      return SENSOR1_EFAILED;
  }
  return error;
}
#endif

/*===========================================================================
 * FUNCTION      dsps_send_request
 *
 * DESCRIPTION   Send a request message to the sensor framework.
 *               Typically used for adding and deleting reports
 *==========================================================================*/
#ifdef FEATURE_GYRO
int dsps_send_request(void *sensor_config,
    void *req_data)
{
  sensor1_error_e error;
  sensor1_msg_header_s req_hdr;
  void *req_msg = NULL;
  sensor1_config_t *dsps_config = (sensor1_config_t *)sensor_config;
  sensor1_req_data_t *msg_data = (sensor1_req_data_t *)req_data;

#ifdef FEATURE_GYRO
  error = allocate_req_msg_buffer(dsps_config->handle,
      msg_data->msg_type, &req_msg);
#else
  return -1;
#endif

  if (error != SENSOR1_SUCCESS) {
    CDBG_ERROR("%s: Error allocating sensor1 message buffer %d\n", __func__,
        error);
    return -1;
  }

  dsps_prepare_req_msg(dsps_config, req_msg, msg_data);
  dsps_prepare_req_header(&req_hdr, msg_data);

  dsps_config->error = 0;
  dsps_config->callback_arrived = 0;

  error = sensor1_write(dsps_config->handle, &req_hdr, req_msg);
  if (error != SENSOR1_SUCCESS) {
    CDBG_ERROR("%s: Error writing request message\n", __func__);
    sensor1_free_msg_buf(dsps_config->handle, req_msg);
    return -1;
  }

  /* Wait for a response */
  if (dsps_wait_for_response(dsps_config) != 0) {
    CDBG_ERROR("%s: Request response timed out\n", __func__);
    return -1;
  }

  if (dsps_config->error)
    return -1;

  return 0;
}
#endif

/*===========================================================================
 * FUNCTION      dsps_handle_broken_pipe
 *
 * DESCRIPTION   Handle error condition of broken pipe with the sensor
 *               framework
 *==========================================================================*/
void dsps_handle_broken_pipe(sensor1_config_t *dsps_config)
{
  CDBG_ERROR("%s: Broken Pipe Exception\n", __func__);
  dsps_config->status = DSPS_BROKEN_PIPE;
  dsps_disconnect((void *)dsps_config);
}

/*===========================================================================
 * FUNCTION      dsps_process_repsonse
 *
 * DESCRIPTION   Process response received from sensor framework.
 *               A response message is in response to a message sent to the
 *               sensor framework. Signal waiting condition variable in
 *               dsps_wait_for_response()
 *=========================================================================*/
#ifdef FEATURE_GYRO
void dsps_process_response(sensor1_config_t *dsps_config,
    sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
  sns_sam_gyroint_enable_resp_msg_v01 *enable_resp_msg;
  sns_sam_gyroint_disable_resp_msg_v01 *disable_resp_msg;
  sns_sam_gyroint_get_report_resp_msg_v01 *get_report_resp_msg;

  switch (msg_hdr->msg_id) {
    case SNS_SAM_GYROINT_ENABLE_RESP_V01:
      enable_resp_msg = (sns_sam_gyroint_enable_resp_msg_v01*)msg_ptr;
      if (enable_resp_msg->instance_id_valid)
        dsps_config->instance_id = enable_resp_msg->instance_id;
      break;
    case SNS_SAM_GYROINT_DISABLE_RESP_V01:
      disable_resp_msg = (sns_sam_gyroint_disable_resp_msg_v01*)msg_ptr;
      if (disable_resp_msg->instance_id_valid)
        dsps_config->instance_id = INVALID_INSTANCE_ID;
      break;
    case SNS_SAM_GYROINT_GET_REPORT_RESP_V01:
      get_report_resp_msg = (sns_sam_gyroint_get_report_resp_msg_v01 *)msg_ptr;
      if (get_report_resp_msg->resp.sns_result_t == SNS_RESULT_SUCCESS_V01) {
        CDBG_DSPS("%s: Report Request Accepted\n", __func__);
      } else {
        CDBG_ERROR("%s: Report Request Denied\n", __func__);
      }
      break;
    default:
      CDBG_ERROR("%s Response not valid",__func__);
      break;
  }
  pthread_mutex_lock(&(dsps_config->callback_mutex));
  dsps_config->callback_arrived = 1;
  pthread_mutex_unlock(&(dsps_config->callback_mutex));
  pthread_cond_signal(&(dsps_config->callback_condvar));
}
#endif

/*===========================================================================
 * FUNCTION      dsps_process_indication
 *
 * DESCRIPTION   Process indication received from sensor framework.
 *=========================================================================*/
#ifdef FEATURE_GYRO
void dsps_process_indication(sensor1_config_t *dsps_config,
    sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
  sns_sam_gyroint_report_ind_msg_v01 *indication =
      (sns_sam_gyroint_report_ind_msg_v01 *) msg_ptr;

  switch(msg_hdr->msg_id) {
    case SNS_SAM_ALGO_REPORT_IND:
      if (!indication->frame_info_valid) {
        CDBG_ERROR("%s: Invalid Indication Frame\n", __func__);
        break;
      }

      if (indication->frame_info_valid) {
        dsps_cirq_add((void *)dsps_config, (void *)indication,
            indication->seqnum);
      }
      else
        CDBG_ERROR("%s: Invalid Frame Info\n", __func__);
      break;
    default:
      CDBG_ERROR("%s: Invalid Indication ID\n", __func__);
      break;
  }
}
#endif

/*===========================================================================
 * FUNCTION      dsps_callback
 *
 * DESCRIPTION   Callback function to be registered with the sensor framework.
 *               This will be called in context of the sensor framework.
 *==========================================================================*/
#ifdef FEATURE_GYRO
void dsps_callback(intptr_t *data,
  sensor1_msg_header_s *msg_hdr,
  sensor1_msg_type_e msg_type,
  void *msg_ptr)
{
  sensor1_config_t * dsps_config =(sensor1_config_t *)data;
  sensor1_handle_s *handle = dsps_config->handle;

  switch (msg_type) {
    case SENSOR1_MSG_TYPE_RESP:
      if (msg_hdr->service_number == SNS_SMGR_SVC_ID_V01) {
        /* Ignore */
      } else if (msg_hdr->service_number == SNS_SAM_GYROINT_SVC_ID_V01) {
        CDBG_DSPS("DSPS Response Received\n");
        dsps_process_response(dsps_config, msg_hdr, msg_ptr);
      } else {
        CDBG_ERROR("%s: Response Msg Id %d not supported\n", __func__,
            msg_hdr->msg_id);
      }
      break;
    case SENSOR1_MSG_TYPE_IND:
      if (msg_hdr->service_number == SNS_SMGR_SVC_ID_V01) {
        /* Ignore */
      } else if (msg_hdr->service_number == SNS_SAM_GYROINT_SVC_ID_V01) {
        CDBG_DSPS("DSPS Response Received\n");
        dsps_process_indication(dsps_config, msg_hdr, msg_ptr);
      } else {
        CDBG_ERROR("%s Unexpected Indication Msg type received ",__func__);
      }
      break;
    case SENSOR1_MSG_TYPE_BROKEN_PIPE:
      dsps_handle_broken_pipe(dsps_config);
      break;
    default:
      CDBG_ERROR("%s: Invalid Message Type\n", __func__);
      break;
  }

  if (msg_ptr != NULL)
    sensor1_free_msg_buf(handle, msg_ptr);
}
#endif
