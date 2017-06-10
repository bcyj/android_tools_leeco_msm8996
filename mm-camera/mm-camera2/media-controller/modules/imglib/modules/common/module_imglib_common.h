/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#ifndef __MODULE_IMGLIB_COMMON_H__
#define __MODULE_IMGLIB_COMMON_H__

#include <linux/media.h>
#include <pthread.h>
#include "img_common.h"
#include "img_comp.h"
#include "img_dbg.h"
#include "img_queue.h"
#include "mct_port.h"
#include "mct_stream.h"
#include "mct_pipeline.h"
#include "modules.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/** GET_STATUS
 *
 * Macro to convert img_status_t to boolean
 **/
#define GET_STATUS(v) (((v) == IMG_SUCCESS) ? TRUE : FALSE)

/** IMGLIB_SESSIONID
 *
 * Macro to get the session id
 **/
#define IMGLIB_SESSIONID(id) (((id) & 0xFFFF0000) >> 16)

/** IMGLIB_STREAMID:
 *
 * Macro to get the stream id
 **/
#define IMGLIB_STREAMID(id) ((id) & 0x0000FFFF)

/** IMGLIB_PACK_IDENTITY:
 *
 * Macro to pack the stream and session id
 **/
#define IMGLIB_PACK_IDENTITY(session_id, stream_id) \
  ((((session_id) & 0x0000FFFF) << 16) + ((stream_id) & 0x0000FFFF))

/** IMGLIB_ARRAY_SIZE:
 *    @a: array to be processed
 *
 * Returns number of elements in array
 **/
#define IMGLIB_ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

/** IMGLIB_GUARDED_CALL:
 *    @condition: condition
 *    @func: function to be called
 *
 * Calls specified function in case the condition is TRUE
 *
 * Returns FALSE in case condition is FALSE or
 *   function return value in case the condition is TRUE
 **/
#define IMGLIB_GUARDED_CALL(condition, function) \
  ((condition) ? (function) : FALSE)

/** PORT_NAME_LEN
 *
 * Max length of port name
 **/
#define PORT_NAME_LEN 32

/** MODULE_FACEPROC_DEBUG
 *
 * Enable logging for faceproc module
 **/
#define MODULE_FACEPROC_DEBUG 1

/** imglib_state_t
 *   @IMGLIB_STATE_IDLE: idle state
 *   @IMGLIB_STATE_INIT: init state
 *   @IMGLIB_STATE_ACTIVE: active state
 *
 *   buffer ack message data
 **/
typedef enum _imglib_state_t {
  IMGLIB_STATE_IDLE,
  IMGLIB_STATE_INIT,
  IMGLIB_STATE_STARTED,
  IMGLIB_STATE_PROCESSING,
} imglib_state_t;

/** mod_img_msg_type_t
 *   @MOD_IMG_MSG_BUF_ACK: buffer acknowledge message
 *   @MOD_IMG_MSG_DIVERT_BUF: divert message
 *   @MOD_IMG_MSG_EXIT: exit message
 *
 *   buffer ack message data
 **/
typedef enum {
  MOD_IMG_MSG_BUF_ACK, /* mod_img_msg_buf_ack_t */
  MOD_IMG_MSG_DIVERT_BUF, /* mod_img_msg_buf_divert_t */
  MOD_IMG_MSG_EXEC_INFO, /* mod_img_msg_exec_t */
  MOD_IMG_MSG_EXIT, /* int */
} mod_img_msg_type_t;

/** mod_img_msg_buf_ack_t
 *   @frame_id: frame id
 *   @identity: MCT identity
 *
 *   buffer ack message data
 **/
typedef struct {
  int frame_id;
  uint32_t identity;
} mod_img_msg_buf_ack_t;

/** divert_buf_exec
 *
 *   divert buf function
 **/
typedef void (*divert_buf_exec)(void *user_data, void *data);

/** mod_img_msg_buf_divert_t
 *   @frame_id: frame id
 *   @buf_idx: buffer index
 *   @identity: MCT identity
 *   @p_exec: thread exec function
 *   @userdata: user data pointer
 *
 *   buffer divert message data
 **/
typedef struct {
  isp_buf_divert_t buf_divert;
  uint32_t identity;
  divert_buf_exec p_exec;
  void *userdata;
} mod_img_msg_buf_divert_t;

/** mod_img_msg_exec_t
 *   @p_exec: thread exec function
 *   @p_userdata: user data pointer
 *   @data: function parameter
 *
 *   exec message data
 **/
typedef struct {
  divert_buf_exec p_exec;
  void *p_userdata;
  void *data;
} mod_img_msg_exec_t;

/** mod_img_msg_t
 *   @type: message type
 *   @port: pointer to the port
 *   @buf_ack: buffer ack data
 *   @buf_divert: buffer divert data
 *
 *   Imaging message structure
 **/
typedef struct {
  mod_img_msg_type_t type;
  mct_port_t *port;

  union {
    mod_img_msg_buf_ack_t buf_ack;
    mod_img_msg_buf_divert_t buf_divert;
    mod_img_msg_exec_t exec_info;
  } data;
} mod_img_msg_t;

/** mod_img_buffer_t
 *   @frame: Frame structure used by imglib components
 *   @map_buf: buffer map provided by MCT
 *
 *   Imaging buffer structure
 **/
typedef struct {
  img_frame_t frame;
  mct_stream_map_buf_t map_buf;
} mod_img_buffer_t;

/** mod_img_buffer_info_t
 *   @p_buffer: array of buffers
 *   @buf_count: buffer count
 *   @total_count: total number of buffers allocates
 *   @fd_buf_count: number of fd buffers
 *
 *   This structure represents the buffer information for
 *   imaging module
 **/
typedef struct {
  mod_img_buffer_t *p_buffer;
  uint32_t buf_count;
  uint32_t total_count;
  uint32_t fd_buf_count;
} mod_img_buffer_info_t;

/** mod_imglib_msg_th_t
 *   @threadid: thread id
 *   @msg_q: message queue
 *   @is_ready: flag to indicate when thread is created
 *   @abort_flag: flag to indicate if abort is issued
 *
 *   This structure represents the generic message thread for
 *   imaging module
 **/
typedef struct {
  pthread_t threadid;
  img_queue_t msg_q;
  int is_ready;
  int abort_flag;
} mod_imglib_msg_th_t;

/** module_imglib_destroy_msg_thread
 *
 * Enable logging for faceproc module
 **/
int module_imglib_destroy_msg_thread(mod_imglib_msg_th_t *p_msg_th);

int module_imglib_send_msg(mod_imglib_msg_th_t *p_msg_th,
  mod_img_msg_t *p_msg);

int module_imglib_create_msg_thread(mod_imglib_msg_th_t *p_msg_th);

boolean mod_imglib_map_fd_buffer(void *data, void *user_data);
boolean mod_imglib_map_fr_buffer(void *data, void *user_data);

/** mod_imglib_dump_stream_info
 *    @info: stream info configuration
 *
 * Prints stream info configuration
 *
 * Returns TRUE in case of success
 **/
void mod_imglib_dump_stream_info(mct_stream_info_t* info);

/** mod_imglib_dump_frame
 *    @img_frame: frame handler
 *    @number: number to be appended at the end of the file name
 *
 * Saves specified frame to folder /data/
 *
 * Returns TRUE in case of success
 **/
boolean mod_imglib_dump_frame(img_frame_t *img_frame, char* file_name,
  uint32_t number);

/** mod_imglib_find_module_parent
 *    @identity: required identity
 *    @module: module, whichs parents will be serached
 *
 * Finds module parent (stream) with specified identity
 *
 * Returns Pointer to stream handler in case of cucess
 *   or NULL in case of failure
 **/
mct_stream_t* mod_imglib_find_module_parent(uint32_t identity,
  mct_module_t* module);

/** module_imglib_common_get_buffer:
 *  @subdev_fd: buffer mgr fd
 *  @identity: stream/session id
 *
 * Function to get buffer for denoise port
 *
 * Returns buffer index
 **/
int module_imglib_common_get_buffer(int subdev_fd, uint32_t identity);

/** module_imglib_common_release_buffer_idx:
 *  @subdev_fd: buffer mgr fd
 *  @identity: stream/session id
 *
 * Function to get buffer for denoise port
 *
 * Returns buffer index
 **/
int module_imglib_common_release_buffer(int subdev_fd, uint32_t identity,
  uint32_t idx, uint32_t frame_id, boolean buff_done);

/** module_imglib_common_get_bfr_mngr_subdev:
 *  @buf_mgr_fd: buffer manager file descriptor
 *
 * Function to get buffer manager file descriptor
 *
 * Returns TRUE in case of success
 **/
int module_imglib_common_get_bfr_mngr_subdev(int *buf_mgr_fd);

/** mod_imglib_find_module_parent
 *    @identity: required identity
 *    @module: module, whichs parents will be serached
 *
 * Finds module parent (stream) with specified identity
 *
 * Returns Pointer to stream handler in case of cucess
 *   or NULL in case of failure
 **/
mct_stream_t* mod_imglib_find_module_parent(uint32_t identity,
  mct_module_t* module);
/** module_imglib_get_next_from_list
 *    @data1: not used
 *    @data2: not used
 *
 *  Gets next element from the list
 *
 *  Return TRUE always
 **/
boolean module_imglib_get_next_from_list(void *data1, void *data2);

/** mod_imglib_get_timestamp
 *  @timestamp: pointer to a char buffer. The buffer should be
 *    allocated by the caller
 *
 *  Get the current timestamp and convert it to a string
 *
 *  Return: None.
 **/
void mod_imglib_get_timestamp_string(char *timestamp);

/**
 * Function: module_imglib_common_post_bus_msg
 *
 * Description: post a particular message to media bus
 *
 * Arguments:
 *   @p_mct_mod - media controller module
 *   @identity - Stream identity
 *   @msg_id - bus message id
 *   @msg_data - bus message data
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_imglib_common_post_bus_msg(mct_module_t *p_mct_mod,
    unsigned int identity, mct_bus_msg_type_t msg_id, void *msg_data);

/**
 * Function: module_imglib_common_get_zoom_ratio
 *
 * Description: This function is returning current zoom ratio
 *
 * Arguments:
 *   @p_mct_mod: mct_module
 *   @zoom_level: hal zoom level
 *
 * Return values:
 *     zoom ratio, 0 if error
 *
 * Notes: none
 **/
float module_imglib_common_get_zoom_ratio(mct_module_t *p_mct_mod,
  int zoom_level);

/**
 * Function: module_imglib_common_get_zoom_level
 *
 * Description: This function is returning current zoom ratio
 *
 * Arguments:
 *   p_mct_cap - capababilities
 *   @zoom_ratio: zoom ratio
 *
 * Return values:
 *     zoom level, -1 if error
 *
 * Notes: none
 **/
int module_imglib_common_get_zoom_level(mct_pipeline_cap_t *p_mct_cap,
  float zoom_ratio);

/** module_imglib_common_get_metadata_buffer:
 *  @info: Stream info
 *  @meta_index: Metadata buffer index
 *
 * Function to get metadata buffer pointer
 *
 * Returns Pointer to metadata buffer / NULL on fail
 **/
cam_metadata_info_t *module_imglib_common_get_metadata(mct_stream_info_t *info,
  uint8_t meta_index);

/**
 * Function: process_fd_on_frame
 *
 * Description: Appliies FD on a given frame
 *
 * Input parameters:
 *   p_frame - input image frame
 *   p_meta - imglib meta data structure
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
int process_fd_on_frame(img_frame_t *p_frame, img_meta_t *p_meta);

#endif //__MODULE_IMGLIB_COMMON_H__
