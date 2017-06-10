/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CPP_HARDWARE_H
#define CPP_HARDWARE_H

#include <pthread.h>
#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "cam_types.h"
#include "camera_dbg.h"
#include "mtype.h"
#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "cpp_hw_params.h"
#include "chromatix_metadata.h"

#define MAX_CPP_DEVICES       2
#define SUBDEV_NAME_SIZE_MAX  32

#define CPP_MAX_FW_VERSIONS   8
#define CPP_MAX_FW_NAME_LEN   32

/* hw version info:
  31:28  Major version
  27:16  Minor version
  15:0   Revision bits
**/
#define CPP_HW_VERSION_1_0_0  0x10000000
#define CPP_HW_VERSION_1_1_0  0x10010000
#define CPP_HW_VERSION_1_1_1  0x10010001
#define CPP_HW_VERSION_2_0_0  0x20000000
#define CPP_HW_VERSION_4_0_0  0x40000000
#define CPP_HW_VERSION_4_1_0  0x40010000
#define CPP_HW_VERSION_4_2_0  0x40020000


#define CPP_CAPS_DENOISE      (1 << 0)
#define CPP_CAPS_SCALE        (1 << 1)
#define CPP_CAPS_SHARPENING   (1 << 2)
#define CPP_CAPS_CROP         (1 << 3)
#define CPP_CAPS_ROTATION     (1 << 4)
#define CPP_CAPS_FLIP         (1 << 5)
#define CPP_CAPS_COLOR_CONV   (1 << 6)

#define ROTATION_90       (1 << 0)
#define ROTATION_180      (2 << 1)
#define ROTATION_270      (3 << 2)

#define CPP_TOTAL_SHARPNESS_LEVELS  6
#define CPP_MAX_SHARPNESS           36
#define CPP_MIN_SHARPNESS           0
#define CPP_DEFAULT_SHARPNESS       12

/* maximum no. of unique streams supported */
#define CPP_HARDWARE_MAX_STREAMS      8
/* how many buffers hardware can hold at a time */
#define CPP_HARDWARE_MAX_PENDING_BUF  1


typedef uint32_t cpp_rotation_caps_t;

typedef struct _cpp_scaling_caps_t{
  float min_scale_factor;
  float max_scale_factor;
} cpp_scaling_caps_t;

typedef struct _cpp_color_conv_caps_t{
  cam_format_t src_fmt;
  cam_format_t dest_fmt;
} cpp_color_conv_caps_t;

typedef struct _cpp_flip_caps_t{
  uint8_t h_flip;
  uint8_t v_flip;
} cpp_flip_caps_t;

typedef struct _cpp_sharpness_caps_t{
  int32_t min_value;
  int32_t max_value;
  int32_t def_value;
  int32_t step;
} cpp_sharpness_caps_t;

typedef enum {
  CPP_HW_STATUS_INVALID,
  CPP_HW_STATUS_FW_LOADED,
  CPP_HW_STATUS_READY,
  CPP_HW_STATUS_BUSY,
} cpp_hardware_status_t;

typedef struct {
  uint32_t caps_mask;
  cpp_scaling_caps_t    scaling_caps;
  cpp_rotation_caps_t   rotation_caps;
  cpp_color_conv_caps_t color_conv_caps;
  cpp_flip_caps_t       filp_caps;
  cpp_sharpness_caps_t  sharpness_caps;
} cpp_hardware_caps_t;

/* HW can send only one type of v4l2 event and actually type
   of event can be abstracted in event data */
typedef struct _cpp_hardware_event_subscribe_info {
  boolean valid;
  uint32_t id;
  uint32_t type;
} cpp_hardware_event_subscribe_info_t;

typedef struct _cpp_hardware_info_t {
  uint32_t version;
  uint32_t caps;
  unsigned long freq_tbl[MAX_FREQ_TBL];
  uint32_t freq_tbl_count;
} cpp_hardware_info_t;

typedef struct _cpp_hardware_stream_status_t {
  boolean   valid;
  int32_t   pending_buf;
  boolean   stream_off_pending;
  uint32_t  identity;
} cpp_hardware_stream_status_t;

typedef struct _cpp_hardware_t {
  uint32_t                            subdev_ids[MAX_CPP_DEVICES];
  int                                 num_subdev;
  int                                 subdev_fd;
  boolean                             subdev_opened;
  uint32_t                            inst_id;
  cpp_hardware_caps_t                 caps;
  cpp_hardware_info_t                 hwinfo;
  cpp_hardware_status_t               status;
  uint32_t                            fw_version;
  cpp_hardware_event_subscribe_info_t event_subs_info;
  cpp_hardware_stream_status_t        stream_status[CPP_HARDWARE_MAX_STREAMS];
  pthread_cond_t                      no_pending_cond;
  pthread_mutex_t                     mutex;
  int                                 num_iommu_cnt;
} cpp_hardware_t;

typedef enum {
  CPP_HW_CMD_GET_CAPABILITIES,
  CPP_HW_CMD_SUBSCRIBE_EVENT,
  CPP_HW_CMD_UNSUBSCRIBE_EVENT,
  CPP_HW_CMD_NOTIFY_EVENT,
  CPP_HW_CMD_STREAMON,
  CPP_HW_CMD_STREAMOFF,
  CPP_HW_CMD_LOAD_FIRMWARE,
  CPP_HW_CMD_PROCESS_FRAME,
  CPP_HW_CMD_QUEUE_BUF,
  CPP_HW_CMD_GET_CUR_DIAG,
  CPP_HW_CMD_BUF_UPDATE,
  CPP_HW_CMD_SET_CLK
} cpp_hardware_cmd_type_t;

typedef struct _cpp_hardware_event_data_t {
  uint8_t        is_buf_dirty;
  uint32_t       frame_id;
  uint32_t       identity;
  uint32_t       buf_idx;
  uint32_t       out_buf_idx;
  int            out_fd;
  struct timeval timestamp;
  void          *cookie;
} cpp_hardware_event_data_t;

typedef struct _cpp_hardware_streamoff_event{
  uint32_t                         streamoff_identity;
  uint32_t                         duplicate_identity;
} cpp_hardware_streamoff_event_t;

typedef struct _cpp_hardware_clock_settings_t {
  long                             clock_rate;
  uint64_t                         avg;
  uint64_t                         inst;
} cpp_hardware_clock_settings_t;

typedef struct _cpp_hardware_cmd_t {
  cpp_hardware_cmd_type_t type;
  union {
    cpp_hardware_streamoff_event_t streamoff_data;
    cpp_hardware_event_data_t       *event_data;
    cpp_hardware_params_t           *hw_params;
    cpp_hardware_stream_buff_info_t *stream_buff_list;
    cpp_hardware_clock_settings_t   clock_settings;
  } u;
} cpp_hardware_cmd_t;

/* interface functions */
cpp_hardware_t* cpp_hardware_create();
int32_t cpp_hardware_open_subdev(cpp_hardware_t *cpphw);
int32_t cpp_hardware_close_subdev(cpp_hardware_t *cpphw);
int32_t cpp_hardware_destroy(cpp_hardware_t *cpphw);
int32_t cpp_hardware_process_command(cpp_hardware_t *cpphw,
  cpp_hardware_cmd_t cmd);
cpp_hardware_status_t cpp_hardware_get_status(cpp_hardware_t *cpphw);

/* static functions */
static int32_t cpp_hardware_find_subdev(cpp_hardware_t *cpphw);
static int32_t cpp_hardware_subcribe_v4l2_event(cpp_hardware_t *cpphw);
static int32_t cpp_hardware_notify_event_get_data(cpp_hardware_t *cpphw,
  cpp_hardware_event_data_t *event_data);
static int32_t cpp_hardware_get_capabilities(cpp_hardware_t *cpphw);
static int32_t cpp_hardware_load_firmware(cpp_hardware_t *cpphw);
static int32_t cpp_hardware_process_streamon(cpp_hardware_t *cpphw,
  cpp_hardware_stream_buff_info_t *hw_strm_buff_info);
static int32_t cpp_hardware_process_streamoff(cpp_hardware_t *cpphw,
  cpp_hardware_streamoff_event_t streamoff_data);
static int32_t cpp_hardware_process_frame(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params);
static cpp_hardware_stream_status_t*
  cpp_hardware_get_stream_status(cpp_hardware_t* cpphw, uint32_t identity);
static int32_t cpp_hardware_subcribe_v4l2_event(cpp_hardware_t *cpphw);
static int32_t cpp_hardware_unsubcribe_v4l2_event(cpp_hardware_t *cpphw);
double cpp_get_sharpness_ratio(int32_t sharpness);

#endif
