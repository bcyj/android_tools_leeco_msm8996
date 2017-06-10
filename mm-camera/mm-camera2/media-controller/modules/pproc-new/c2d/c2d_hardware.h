/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef C2D_HARDWARE_H
#define C2D_HARDWARE_H

#include <pthread.h>
#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "cam_types.h"
#include "camera_dbg.h"
#include "mtype.h"
#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "c2d_hw_params.h"

#define MAX_C2D_DEVICES       2
#define SUBDEV_NAME_SIZE_MAX  32

#define C2D_MAX_FW_VERSIONS   8
#define C2D_MAX_FW_NAME_LEN   32

/* hw version info:
  31:28  Major version
  27:16  Minor version
  15:0   Revision bits
**/
#define C2D_HW_VERSION_1_0_0  0x10000000
#define C2D_HW_VERSION_1_1_0  0x10010000
#define C2D_HW_VERSION_2_0_0  0x20000000

#define C2D_CAPS_DENOISE      (1 << 0)
#define C2D_CAPS_SCALE        (1 << 1)
#define C2D_CAPS_SHARPENING   (1 << 2)
#define C2D_CAPS_CROP         (1 << 3)
#define C2D_CAPS_ROTATION     (1 << 4)
#define C2D_CAPS_FLIP         (1 << 5)
#define C2D_CAPS_COLOR_CONV   (1 << 6)

#define ROTATION_90       (1 << 0)
#define ROTATION_180      (2 << 1)
#define ROTATION_270      (3 << 2)

#define C2D_TOTAL_SHARPNESS_LEVELS  6
#define C2D_MAX_SHARPNESS           6
#define C2D_MIN_SHARPNESS           0
#define C2D_DEFAULT_SHARPNESS       2

/* maximum no. of unique streams supported */
#define C2D_HARDWARE_MAX_STREAMS      8
/* how many buffers hardware can hold at a time */
#define C2D_HARDWARE_MAX_PENDING_BUF  2


typedef uint32_t c2d_rotation_caps_t;

typedef struct _c2d_scaling_caps_t{
  float min_scale_factor;
  float max_scale_factor;
} c2d_scaling_caps_t;

typedef struct _c2d_color_conv_caps_t{
  cam_format_t src_fmt;
  cam_format_t dest_fmt;
} c2d_color_conv_caps_t;

typedef struct _c2d_flip_caps_t{
  uint8_t h_flip;
  uint8_t v_flip;
} c2d_flip_caps_t;

typedef struct _c2d_sharpness_caps_t{
  int32_t min_value;
  int32_t max_value;
  int32_t def_value;
  int32_t step;
} c2d_sharpness_caps_t;

typedef enum {
  C2D_HW_STATUS_INVALID,
  C2D_HW_STATUS_FW_LOADED,
  C2D_HW_STATUS_READY,
  C2D_HW_STATUS_BUSY,
} c2d_hardware_status_t;

typedef struct {
  uint32_t caps_mask;
  c2d_scaling_caps_t    scaling_caps;
  c2d_rotation_caps_t   rotation_caps;
  c2d_color_conv_caps_t color_conv_caps;
  c2d_flip_caps_t       filp_caps;
  c2d_sharpness_caps_t  sharpness_caps;
} c2d_hardware_caps_t;

/* HW can send only one type of v4l2 event and actually type
   of event can be abstracted in event data */
typedef struct _c2d_hardware_event_subscribe_info {
  boolean valid;
  uint32_t id;
  uint32_t type;
} c2d_hardware_event_subscribe_info_t;

typedef struct _c2d_hardware_info_t {
	uint32_t version;
	uint32_t caps;
} c2d_hardware_info_t;

typedef struct _c2d_hardware_stream_status_t {
  boolean   valid;
  int32_t   pending_buf;
  boolean   stream_off_pending;
  uint32_t  identity;
} c2d_hardware_stream_status_t;

typedef struct _c2d_hardware_t {
  uint32_t                            subdev_ids[MAX_C2D_DEVICES];
  int                                 num_subdev;
  //int                                 subdev_fd;
  //boolean                             subdev_opened;
  uint32_t                            inst_id;
  c2d_hardware_caps_t                 caps;
  c2d_hardware_info_t                 hwinfo;
  //c2d_hardware_status_t               status;
  c2d_hardware_event_subscribe_info_t event_subs_info;
  c2d_hardware_stream_status_t        stream_status[C2D_HARDWARE_MAX_STREAMS];
  pthread_cond_t                      no_pending_cond;
  pthread_mutex_t                     mutex;
} c2d_hardware_t;

typedef enum {
  C2D_HW_CMD_GET_CAPABILITIES,
  C2D_HW_CMD_STREAMON,
  C2D_HW_CMD_STREAMOFF,
  C2D_HW_CMD_PROCESS_FRAME,
  C2D_HW_CMD_RELEASE_FRAME
} c2d_hardware_cmd_type_t;

typedef struct _c2d_hardware_event_data_t {
  uint32_t  frame_id;
  uint32_t  identity;
  uint32_t  buf_idx;
  void     *cookie;
} c2d_hardware_event_data_t;

typedef struct _c2d_hardware_cmd_t {
  c2d_hardware_cmd_type_t type;
  union {
    uint32_t                         streamoff_identity;
    c2d_hardware_event_data_t       *event_data;
    c2d_hardware_params_t           *hw_params;
    c2d_hardware_stream_buff_info_t *stream_buff_list;

  } u;
} c2d_hardware_cmd_t;

/* interface functions */
c2d_hardware_t* c2d_hardware_create();
//int32_t c2d_hardware_open_subdev(c2d_hardware_t *c2dhw);
//int32_t c2d_hardware_close_subdev(c2d_hardware_t *c2dhw);
int32_t c2d_hardware_destroy(c2d_hardware_t *c2dhw);
int32_t c2d_hardware_process_command(c2d_hardware_t *c2dhw,
  c2d_hardware_cmd_t cmd);
c2d_hardware_status_t c2d_hardware_get_status(c2d_hardware_t *c2dhw);

/* static functions */
static int32_t c2d_hardware_find_subdev(c2d_hardware_t *c2dhw);
static int32_t c2d_hardware_subcribe_v4l2_event(c2d_hardware_t *c2dhw);
static int32_t c2d_hardware_notify_event_get_data(c2d_hardware_t *c2dhw,
  c2d_hardware_event_data_t *event_data);
static int32_t c2d_hardware_get_capabilities(c2d_hardware_t *c2dhw);
static int32_t c2d_hardware_load_firmware(c2d_hardware_t *c2dhw);
static int32_t c2d_hardware_process_streamon(c2d_hardware_t *c2dhw,
  c2d_hardware_stream_buff_info_t *hw_strm_buff_info);
static int32_t c2d_hardware_process_streamoff(c2d_hardware_t *c2dhw,
  uint32_t identity);
static int32_t c2d_hardware_process_frame(c2d_hardware_t *c2dhw,
  c2d_hardware_params_t *hw_params);
static int32_t c2d_hardware_release_frame(c2d_hardware_t *c2dhw,
  c2d_hardware_params_t *hw_params);
c2d_hardware_stream_status_t*
  c2d_hardware_get_stream_status(c2d_hardware_t* c2dhw, uint32_t identity);
static int32_t c2d_hardware_subcribe_v4l2_event(c2d_hardware_t *c2dhw);
static int32_t c2d_hardware_unsubcribe_v4l2_event(c2d_hardware_t *c2dhw);
double c2d_get_sharpness_ratio(int32_t sharpness);

#endif
