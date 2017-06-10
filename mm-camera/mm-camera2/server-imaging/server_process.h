/* server_process.h
 * 
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SERVER_PROCESS_H__
#define __SERVER_PROCESS_H__
#define MAX_DEV_NAME_SIZE 32
#define TOTAL_RAM_SIZE_512MB 536870912

#include "mtype.h"
#include "mct_list.h"
#include <linux/videodev2.h>

/**
 *
 *
 *
 **/
typedef enum _serv_proc_result {
  RESULT_SUCCESS,
  RESULT_FAILURE,
  RESULT_NEW_SESSION,
  RESULT_DEL_SESSION
} serv_proc_result_t;

/**
 * Every Session should have one corresponding MediaController
 **/
typedef struct _serv_proc_session_info {
  int session_idx;
  int hal_ds_fd;
  int mct_msg_rd_fd;
  int mct_msg_wt_fd;
} serv_proc_session_info_t;

/**
 *
 **/
typedef enum _serv_ret_to_hal_type {
  SERV_RET_TO_HAL_CMDACK,
  SERV_RET_TO_HAL_NOTIFY,
  SERV_RET_TO_KERNEL_NOTIFY_POSSIBLE_FREEZE,
  SERV_RET_TO_HAL_NOTIFY_ERROR,
} serv_ret_to_hal_type_t;

/**
 *
 *
 *
 **/
typedef struct _serv_ret_to_hal {
  boolean                 ret;
  serv_ret_to_hal_type_t  ret_type;
  struct v4l2_event       ret_event;
} serv_ret_to_hal_t;

/**
 * HAL command processing return
 **/
typedef struct _serv_proc_ret {
  serv_proc_result_t       result;
  boolean                  new_session;
  serv_proc_session_info_t new_session_info;
  serv_ret_to_hal_t        ret_to_hal;
} serv_proc_ret_t;

boolean server_process_module_init(void);
boolean server_process_module_sensor_init(void);
serv_proc_ret_t server_process_hal_event(struct v4l2_event *event);

serv_proc_ret_t server_process_hal_ds_packet(const int fd,
  const int session);

serv_proc_ret_t server_process_mct_msg(const int fd,
  const unsigned int session);
#endif /* __SERVER_PROCESS_H__ */
