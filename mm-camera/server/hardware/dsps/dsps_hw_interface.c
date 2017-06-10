/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file defines the media/module/master controller's interface with the
   DSPS modules.

============================================================================*/
#include "dsps_hw.h"

static sensor1_config_t *dsps_config;  /* Pointer to the DSPS Struct */

/*==========================================================================
 * FUNCTION    - copy_dsps_data -
 *
 * DESCRIPTION: Helper fn
 *=========================================================================*/
void copy_dsps_data(dsps_get_data_t *data, buffer_data_t *sensor_info)
{
  if (data->format == TYPE_Q16) {
    data->output_data.q16[0] = (long) sensor_info->x;
    data->output_data.q16[1] = (long) sensor_info->y;
    data->output_data.q16[2] = (long) sensor_info->z;
    data->output_data.seq_no = sensor_info->seqnum;
  } else if (data->format == TYPE_FLOAT) {
    FLOAT_FROM_Q16(data->output_data.flt[0], sensor_info->x);
    FLOAT_FROM_Q16(data->output_data.flt[1], sensor_info->y);
    FLOAT_FROM_Q16(data->output_data.flt[2], sensor_info->z);
    data->output_data.seq_no = sensor_info->seqnum;
  }
}

/*==========================================================================
 * FUNCTION    - dsps_proc_init -
 *
 * DESCRIPTION: To be called by mctl only each instance of camera
 *=========================================================================*/
int dsps_proc_init()
{
  CDBG_DSPS("%s: E\n", __func__);
  if (dsps_config != NULL) {
    CDBG_ERROR("DSPS has already been initialized");
    if (dsps_config->status == DSPS_RUNNING)
      return 0;
  }
  dsps_config = (sensor1_config_t *)malloc(sizeof(sensor1_config_t));
  if (dsps_config == NULL) {
    CDBG_ERROR("%s: malloc error", __func__);
    return -1;
  }

  memset((void *)dsps_config, 0, sizeof(sensor1_config_t));
#ifdef FEATURE_GYRO
  dsps_config->handle = (sensor1_handle_s *)-1;
#endif

  dsps_config->callback_arrived = 0;
  dsps_config->status = DSPS_RUNNING;
  dsps_config->num_clients = 0;
  dsps_config->error = 0;
  dsps_config->instance_id = -1;
  dsps_cirq_init(dsps_config);
  pthread_mutex_init(&(dsps_config->callback_mutex), NULL);
  pthread_cond_init(&(dsps_config->callback_condvar), NULL);
  pthread_mutex_init(&(dsps_config->data_mutex), NULL);
  if (dsps_open((void *)dsps_config) != 0)
    goto error;
  if (dsps_config->status == DSPS_RUNNING)
    dsps_config->num_clients++;
  CDBG_DSPS("%s: X", __func__);
  return 0;

error:
  CDBG_ERROR("Failed to open sensor1 port\n");
  free(dsps_config);
  dsps_config = NULL;
  return -1;
}

/*===========================================================================
 * FUNCTION    - dsps_proc_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
void dsps_proc_deinit()
{
  if (dsps_config == NULL)
    return;
  if (dsps_config->status == DSPS_RUNNING)
    dsps_config->num_clients--;
  if (dsps_config->num_clients == 0) {
    dsps_disconnect((void *)dsps_config);
    pthread_mutex_destroy(&(dsps_config->callback_mutex));
    pthread_cond_destroy(&(dsps_config->callback_condvar));
    free(dsps_config);
    dsps_config = NULL;
  }
}

/*===========================================================================
 * FUNCTION    - dsps_proc_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int dsps_proc_set_params(dsps_set_data_t *data)
{
  if (dsps_config == NULL ||dsps_config->status != DSPS_RUNNING) {
    CDBG_DSPS("%s: DSPS is not running or is deinitialized\n",
        __func__);
    return -1;
  }

  int rc = 0;
  sensor1_req_data_t msg_data;
  msg_data.msg_type = data->msg_type;

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
      break;
    case DSPS_DISABLE_REQ:
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
  if (dsps_send_request((void *)dsps_config, &msg_data) < 0) {
    CDBG_ERROR("%s Error sending request", __func__);
    return -1;
  }
#endif
  return rc;
}  /* dsps_proc_set_params */

/*===========================================================================
 * FUNCTION    - dsps_proc_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int dsps_get_params(dsps_get_data_t *data)
{
  buffer_data_t sensor_info;

  if (dsps_config == NULL ||dsps_config->status != DSPS_RUNNING) {
    CDBG_DSPS("%s: DSPS is not running or is deinitialized\n",
      __func__);
    return -1;
  }

  switch (data->type) {
    case GET_DATA_FRAME:
      if (dsps_cirq_search(dsps_config, &sensor_info, data->id))
        return -1;
      copy_dsps_data(data, &sensor_info);
      break;
    case GET_DATA_LAST:
      if (dsps_cirq_get_last(dsps_config, &sensor_info))
        return -1;
      copy_dsps_data(data, &sensor_info);
      break;
    default:
      CDBG_ERROR("Invalid DSPS Get Params Type");
      return -1;
  }

  return 0;
}  /* dsps_proc_get_params */
