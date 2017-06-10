/**********************************************************************
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
 * Qualcomm Technologies Proprietary and Confidential.                 *
 **********************************************************************/

#ifndef __MODULE_HDR_LIB_H__
#define __MODULE_HDR_LIB_H__

#include "mct_pipeline.h"
#include "mct_port.h"

/** HDR_LIB_IN_BUFFS:
 *
 * Defines number of input buffers for HDR library
 *
 * Returns number of input buffers for HDR library
 **/
#ifdef HDR_LIB_GHOSTBUSTER
#define HDR_LIB_IN_BUFFS (3)
#else
#define HDR_LIB_IN_BUFFS (2)
#endif

/** HDR_LIB_OUT_BUFFS:
 *
 * Defines number of output buffers for HDR library
 *
 * Returns number of output buffers for HDR library
 **/
#define HDR_LIB_OUT_BUFFS 0

/** HDR_LIB_INPLACE_BUFFS:
 *
 * Defines number of inplace processing buffers for HDR library
 *
 * Returns number of inplace processing buffers for HDR library
 **/
#define HDR_LIB_INPLACE_BUFFS 1

/** module_mms_hdr_buf_t
 *    @subdev_fd: buffer manager file descriptor
 *    @identity: stream identity for current buffer
 *    @is_native: flag indicating whether buffer is native
 *    @img_frame: image frame descriptor
 *    @channel_id: holds the channel id
 *    @meta_data: holds the pointer for meta data associated with this buffer
 *
 *  hdr buffer handlers
 **/
typedef struct
{
  int32_t subdev_fd;
  uint32_t identity;
  boolean is_native;
  img_frame_t* img_frame;
  int32_t channel_id;
  void *meta_data;
  boolean is_skip_pproc;
} module_hdr_buf_t;

/** module_hdr_crop_t
 *    @start_x: start X
 *    @start_y: start Y
 *    @width: width
 *    @height: height
 *
 *  hdr crop type
 **/
typedef struct
{
  uint32_t start_x;
  uint32_t start_y;
  uint32_t width;
  uint32_t height;
} module_hdr_crop_t;

/** module_hdr_lib_notify_cb
 *    @user_data: user data
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @out_crop: output crop
 *
 * Module hdr library process done callback
 *
 * Returns Nothing
 **/
typedef void (*module_hdr_lib_notify_cb)(void* user_data,
  module_hdr_buf_t** out_buff, module_hdr_buf_t** in_buff,
  module_hdr_crop_t* out_crop);

/** module_hdr_lib_load
 *
 * Loads hdr library
 *
 * Returns library handle in case of success or NULL
 **/
void* module_hdr_lib_load();

/** module_hdr_lib_init
 *    @lib_handle: library handle
 *
 * Initializes hdr library
 *
 * Returns Library handle instance in case of success or NULL
 **/
void* module_hdr_lib_init(void* lib_handle);

/** module_hdr_lib_query_mod
 *    @buf: querry capabilities data
 *
 * Requests library capabilities data for specified session
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_query_mod(mct_pipeline_imaging_cap_t *buf);

/** module_hdr_lib_get_output_inplace_index:
 *    @number: sequential number for inplace buffers
 *    @index: output inplace index
 *
 * Function to process image data
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_get_output_inplace_index(uint32_t number,
  uint32_t* index);

/** module_hdr_lib_process:
 *    @lib_instance: library handle instance
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @metadata_buff: metadata buffer handler
 *    @user_data: user data
 *    @cam_hdr_param: hdr library configuration
 *    @cb: notification cb
 *
 * Function to process image data
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_process(void* lib_instance, module_hdr_buf_t** out_buff,
  module_hdr_buf_t** in_buff, void* metadata_buff, void* user_data,
  cam_hdr_param_t *cam_hdr_param, module_hdr_lib_notify_cb cb);

/** module_hdr_lib_abort
 *    @lib_instance: library handle instance
 *
 * Aborts hdr library processing
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_abort(void* lib_instance);

/** module_hdr_lib_deinit
 *    @lib_instance: library handle instance
 *
 * Deinitializes hdr library
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_deinit(void* lib_instance);

/** module_hdr_lib_unload
 *    @lib_handle: library handle
 *
 * Unloads hdr library
 *
 * Returns TRUE in case of success
 **/
boolean module_hdr_lib_unload(void* lib_handle);

#endif //__MODULE_HDR_LIB_H__
