/*============================================================================

   Copyright (c) 2012 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file defines the media/module/master controller's interface with the
   DSPS modules.

============================================================================*/
#include "dsps_hw.h"
#include "camera_dbg.h"


static int num_gyroint_instance = 0;

/*==========================================================================
 * FUNCTION    - copy_dsps_data -
 *
 * DESCRIPTION: Helper fn
 *=========================================================================*/
static void copy_dsps_data(dsps_get_data_t *data, buffer_data_t *sensor_info,
  int include_gyro_samples)
{
  uint8_t i;
  if (data->format == TYPE_Q16) {
    data->output_data.q16[0] = (long) sensor_info->x;
    data->output_data.q16[1] = (long) sensor_info->y;
    data->output_data.q16[2] = (long) sensor_info->z;
    data->output_data.seq_no = sensor_info->seqnum;
    data->output_data.sample_len = sensor_info->sample_len;
  } else if (data->format == TYPE_FLOAT) {
    FLOAT_FROM_Q16(data->output_data.flt[0], sensor_info->x);
    FLOAT_FROM_Q16(data->output_data.flt[1], sensor_info->y);
    FLOAT_FROM_Q16(data->output_data.flt[2], sensor_info->z);
    data->output_data.seq_no = sensor_info->seqnum;
    data->output_data.sample_len = sensor_info->sample_len;
  }

  if (sensor_info->sample_len > 32)  // This is a bug
  {
    sensor_info->sample_len = 0;
  }

  data->output_data.gyro_samples_included = 0;

  if (include_gyro_samples) {
    //LOGE("%s, SAMPLE LEN %d", __FUNCTION__, sensor_info->sample_len);
    data->output_data.gyro_samples_included = 1;
    for (i = 0; i< sensor_info->sample_len; i++) {
      data->output_data.sample[i] = sensor_info->sample[i];
    }
  }
  else
  {
    sensor_info->sample_len = 0;
  }
}


/** dsps_proc_init:
 *    @port: gyro port (opaque to dsps layer)
 *    @dsps_callback: callback to upper layer client
 *
 * Currently (December 2013), gyro sensors supports only one instance of the
 * gyroint service.
 *
 * Returns dsps client handle on success, 0 otherwise.
 **/
void *dsps_proc_init(void *port, dsps_callback_func dsps_callback)
{
  sensor1_config_t *dsps_config;

  CDBG_DSPS("%s: E\n", __func__);
  if (num_gyroint_instance != 0) {
    CDBG_ERROR("%s: DSPS has already been initialized", __func__);
    /* Return NULL for now since gyro sensor can only support a single instance
       of GYROINT */
    return NULL;
  }

  dsps_config = (sensor1_config_t *)malloc(sizeof(sensor1_config_t));
  if (dsps_config == NULL) {
    CDBG_ERROR("%s:%d malloc error", __func__, __LINE__);
    return NULL;
  }

  memset((void *)dsps_config, 0, sizeof(sensor1_config_t));
  dsps_config->handle = (sensor1_handle_s *)-1;
  dsps_config->callback_arrived = 0;
  dsps_config->status = DSPS_RUNNING;
  dsps_config->error = 0;
  dsps_config->instance_id = -1;
  dsps_config->port = port;
  dsps_config->dsps_callback = dsps_callback;
  dsps_cirq_init(dsps_config);
  pthread_mutex_init(&(dsps_config->callback_mutex), NULL);
  pthread_cond_init(&(dsps_config->callback_condvar), NULL);
  pthread_mutex_init(&(dsps_config->thread_mutex), NULL);
  pthread_cond_init(&(dsps_config->thread_condvar), NULL);
  pthread_mutex_init(&(dsps_config->data_mutex), NULL);
  if (dsps_open((void *)dsps_config) != 0) {
    goto dsps_open_error;
  }
  num_gyroint_instance++;
  CDBG_DSPS("%s: X", __func__);
  return dsps_config;

dsps_open_error:
  CDBG_ERROR("%s: Failed to open sensor1 port", __func__);
  free(dsps_config);
  return NULL;
}


/*===========================================================================
 * FUNCTION    - dsps_proc_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
void dsps_proc_deinit(void *dsps_client_handle)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)dsps_client_handle;

  if (dsps_config == NULL) {
    CDBG_ERROR("%s: DSPS client handle is NULL", __func__);
    return;
  }

  dsps_disconnect((void *)dsps_config);
  free(dsps_config);
  num_gyroint_instance--;
}


/*===========================================================================
 * FUNCTION    - dsps_proc_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int dsps_proc_set_params(void *dsps_client_handle, dsps_set_data_t *data)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)dsps_client_handle;

  if (dsps_config == NULL || dsps_config->status != DSPS_RUNNING) {
    CDBG_DSPS("%s: DSPS is not running, deinitialized or not connected",
      __func__);
    return -1;
  }

  int rc = 0;
  sensor1_req_data_t msg_data;
  msg_data.msg_type = data->msg_type;
  int wait = 0;

  CDBG_DSPS("%s: msg_type %d", __func__, data->msg_type);
  switch (data->msg_type) {
    case DSPS_ENABLE_REQ:
      if (data->sensor_feature.mode == ENABLE_ALL) {
        msg_data.enable_buffered_samples = true;
        msg_data.enable_frame_stats = false;
        msg_data.enable_integrate_angle = true;
      } else {
        CDBG_ERROR("%s Selected feature not supported %d", __func__,
          data->sensor_feature.mode);
        return -1;
      }
      wait = 1;
      break;

    case DSPS_DISABLE_REQ:
      wait = 1;
      break;

    case DSPS_GET_REPORT:
      msg_data.seqnum = data->data.id;
      msg_data.t_start = data->data.t_start;
      msg_data.t_end = data->data.t_end;
      break;

    default:
      CDBG_ERROR("%s Invalid param",__func__);
      return -1;
  }
#ifdef FEATURE_GYRO
  if (dsps_send_request((void *)dsps_config, &msg_data, wait) < 0) {
    ALOGE("%s Error sending request", __func__);
    return -1;
  }
#endif
  return rc;
}  /* dsps_proc_set_params */

/*===========================================================================
 * FUNCTION    - dsps_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int dsps_get_params(void *dsps_client_handle, dsps_get_data_t *data,
  int include_gyro_samples)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)dsps_client_handle;
  buffer_data_t sensor_info;

  if (dsps_config == NULL || dsps_config->status != DSPS_RUNNING) {
    CDBG_DSPS("%s: DSPS is not running, deinitialized or not connected",
      __func__);
    return -1;
  }

  switch (data->type) {
    case GET_DATA_FRAME:
      if (dsps_cirq_search(dsps_config, &sensor_info, data->id))
      {
        CDBG("%s failed search",__FUNCTION__);
        return -1;
      }
      copy_dsps_data(data, &sensor_info, include_gyro_samples);
      break;
    case GET_DATA_LAST:
      if (dsps_cirq_get_last(dsps_config, &sensor_info))
      {
        CDBG_DSPS("%s failed get_last",__FUNCTION__);
        return -1;
      }
      copy_dsps_data(data, &sensor_info, include_gyro_samples);
      break;
    default:
      CDBG_ERROR("Invalid DSPS Get Params Type");
      return -1;
  }

  return 0;
}  /* dsps_get_params */
