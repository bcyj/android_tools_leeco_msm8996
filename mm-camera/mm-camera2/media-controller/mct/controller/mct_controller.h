/* mct_controller.h
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_CONTROLLER_H__
#define __MCT_CONTROLLER_H__

#include "mct_object.h"
#include "mct_queue.h"
#include "mct_pipeline.h"
#include <pthread.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#define MCT_DEBUG_MASK                 (1<<0)
typedef struct _mct_controller mct_controller_t;

struct _mct_controller {
  mct_queue_t   *serv_cmd_q;
  mct_queue_t   *bus_cmd_q;
  pthread_mutex_t  serv_msg_q_lock;

  pthread_cond_t mctl_thread_started_cond;
  pthread_mutex_t mctl_thread_started_mutex;

  /* 1. Server communicates with Media Controller through signal(servSignal);
   * 2. Media Controller communicate with Server through Pipe(servFd) */
  int         serv_fd;

  pthread_t   mct_tid;

  pthread_cond_t mctl_cond;
  pthread_mutex_t mctl_mutex;

  /*This counter is increased with 1 when message pushed to Q */
  unsigned int serv_cmd_q_counter;

  /* Media Controller incorporte Pipeline from here */
  mct_pipeline_t *pipeline;
};

typedef enum _mct_serv_msg_type {
  SERV_MSG_DS,
  SERV_MSG_HAL,
  SERV_MSG_MAX
} mct_serv_msg_type;

/** _mct_serv_ds_msg:
 *    @buf_type:
 *    @session: session index
 *    @stream:  stream index
 *    @size:    mapped buffer size
 *    @index:   mapped buffer index
 *    @fd:      buffer's file descriptor
 *              from domain socket
 *
 *  This structure defines the message received
 *  via domain socket
 **/
typedef struct _mct_serv_ds_msg {
  uint32_t buf_type;
  uint32_t operation;
  uint32_t session;
  uint32_t  stream;
  size_t size;
  uint32_t index;
  int32_t plane_idx;
  int fd;
} mct_serv_ds_msg_t;

/** _mct_server_msg:
 *    @msg_type: defines whether it is HAL message or Domain
 *               Socket message
 *    @ds_msg:   content of message received from Domain Socket
 *    @hal_msg:  content of message received from HAL, it's
 *               definition is in msmb_camera.h
 *
 * Message sent to Media Controller from Imaging Server
 **/
typedef struct _mct_serv_msg {
  mct_serv_msg_type msg_type;

  union {
    mct_serv_ds_msg_t ds_msg;
    struct v4l2_event hal_msg;
  } u;
} mct_serv_msg_t;

/** _mct_process_ret_type:
 *
 *
 *
 **/
typedef enum _mct_process_ret_type {
  MCT_PROCESS_RET_SERVER_MSG,
  MCT_PROCESS_RET_BUS_MSG,
  MCT_PROCESS_DUMP_INFO,
  MCT_PROCESS_RET_ERROR_MSG,
} mct_process_ret_type;

typedef struct _mct_proc_serv_msg_ret {
  boolean error;
  mct_serv_msg_t msg;
} mct_proc_serv_msg_ret;

/** mct_proc_bus_msg_type
 *
 *
 *
 **/
typedef enum _mct_proc_bus_msg_type {
  MCT_PROC_BUS_METADATA,
  MCT_PROC_BUS_ISP_SOF,
  MCT_PROC_BUS_ISP_ERROR,
} mct_proc_bus_msg_type;

/** mct_proc_bus_msg_ret:
 *   @msg_type: Bus message type
 *   @metadata_buf_idx: meta data buffer
 *      index
 *   @session: session index
 *   @stream:  stream index
 *
 * Bus message return value sent to imaging server
 **/
typedef struct _mct_proc_bus_msg_ret {
  boolean error;
  mct_bus_msg_type_t msg_type;
  int metadata_buf_idx;
  unsigned int session;
  unsigned int stream;
} mct_proc_bus_msg_ret;

/** _mct_process_ret: Media Controller process return type
 *    @type: server messagre or bus message
 *      - server HAL message
 *        use SERV_RET_TO_HAL_CMDACK for control command
 *        use SERV_RET_TO_HAL_NOTIFY for DS Buf  mapping
 *      - bus message use SERV_RET_TO_HAL_NOTIFY
 *
 *    @serv_msg_ret: return value after processsed server message
 *    @bus_msg_ret: return value after processed bus message
 **/
typedef struct _mct_process_ret {
  mct_process_ret_type type;

  union {
    mct_proc_serv_msg_ret serv_msg_ret;
    mct_proc_bus_msg_ret  bus_msg_ret;
  } u;

} mct_process_ret_t;

boolean mct_controller_new(mct_list_t *mods,
  unsigned int session_idx, int servFd);

boolean mct_controller_destroy(unsigned int session_idx);

boolean mct_controller_proc_serv_msg(mct_serv_msg_t *msg);

#endif /* __MCT_CONTROLLER_H__ */
