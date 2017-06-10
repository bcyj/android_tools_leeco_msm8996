/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __MODULE_DENOISE_LIB_H__
#define __MODULE_DENOISE_LIB_H__

#include "mct_pipeline.h"
#include "mct_port.h"

/** module_denoise_buf_t
 *    @subdev_fd: buffer manager file descriptor
 *    @frame_id: frame id
 *    @img_frame: image frame descriptor
 *
 *  denoise buffer handlers
 **/
typedef struct {
  int32_t subdev_fd;
  uint32_t frame_id;
  img_frame_t* img_frame;
} module_denoise_buf_t;

/** module_denoise_lib_notify_cb
 *    @user_data: user data
 *    @ret_val: library return value
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handlere
 *
 * Module denoise library process done callback
 *
 * Returns Nothing
 **/
typedef void (*module_denoise_lib_notify_cb) (void* user_data,
  module_denoise_buf_t *out_buff, module_denoise_buf_t *in_buff);

/** module_denoise_lib_load
 *
 * Loads denoise library
 *
 * Returns library handle in case of success or NULL
 **/
void* module_denoise_lib_load();

/** module_denoise_lib_init
 *    @lib_handle: library handle
 *
 * Initializes denoise library
 *
 * Returns Library handle instance in case of success or NULL
 **/
void* module_denoise_lib_init(void* lib_handle);

/** module_denoise_lib_query_mod
 *    @buf: querry capabilities data
 *
 * Requests library capabilities data for specified session
 *
 * Returns TRUE in case of success
 **/
boolean module_denoise_lib_query_mod(mct_pipeline_cap_t *buf);

/** module_denoise_lib_process:
 *    @lib_instance: library handle instance
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @metadata_buff: metadata buffer handler
 *    @user_data: user data
 *    @cam_denoise_param: denoise library configuration
 *    @cb: notification cb
 *
 * Function to process image data
 *
 * Returns TRUE in case of success
 **/
boolean module_denoise_lib_process(void* lib_instance,
  module_denoise_buf_t *out_buff, module_denoise_buf_t *in_buff,
  void* metadata_buff, void* user_data, cam_denoise_param_t *cam_denoise_param,
  module_denoise_lib_notify_cb cb);

/** module_denoise_lib_abort
 *    @lib_instance: library handle instance
 *
 * Aborts denoise library processing
 *
 * Returns TRUE in case of success
 **/
boolean module_denoise_lib_abort(void* lib_instance);

/** module_denoise_lib_deinit
 *    @lib_instance: library handle instance
 *
 * Deinitializes denoise library
 *
 * Returns TRUE in case of success
 **/
boolean module_denoise_lib_deinit(void* lib_instance);

/** module_denoise_lib_unload
 *    @lib_handle: library handle
 *
 * Unloads denoise library
 *
 * Returns TRUE in case of success
 **/
boolean module_denoise_lib_unload(void* lib_handle);

#endif //__MODULE_DENOISE_LIB_H__
