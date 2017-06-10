/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef VPE_HARDWARE_H
#define VPE_HARDWARE_H

#include <pthread.h>
#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "cam_types.h"
#include "camera_dbg.h"
#include "mtype.h"
#include "modules.h"
#include "vpe_hw_params.h"

#define MAX_VPE_DEVICES       2
#define SUBDEV_NAME_SIZE_MAX  32

#define VPE_CAPS_SCALE        (1 << 0)
#define VPE_CAPS_CROP         (1 << 1)

/* maximum no. of unique streams supported */
#define VPE_HARDWARE_MAX_STREAMS      8
/* how many buffers hardware can hold at a time */
#define VPE_HARDWARE_MAX_PENDING_BUF  2


typedef struct _vpe_scaling_caps_t{
  float min_scale_factor;
  float max_scale_factor;
} vpe_scaling_caps_t;

typedef enum {
  VPE_HW_STATUS_INVALID,
  VPE_HW_STATUS_READY,
  VPE_HW_STATUS_BUSY,
} vpe_hardware_status_t;

typedef struct {
  uint32_t caps_mask;
  vpe_scaling_caps_t    scaling_caps;
} vpe_hardware_caps_t;

/* HW can send only one type of v4l2 event and actually type
   of event can be abstracted in event data */
typedef struct _vpe_hardware_event_subscribe_info {
  boolean valid;
  uint32_t id;
  uint32_t type;
} vpe_hardware_event_subscribe_info_t;

typedef struct _vpe_hardware_info_t {
	uint32_t version;
	uint32_t caps;
} vpe_hardware_info_t;

typedef struct _vpe_hardware_stream_status_t {
  boolean   valid;
  int32_t   pending_buf;
  boolean   stream_off_pending;
  uint32_t  identity;
} vpe_hardware_stream_status_t;

typedef struct _vpe_hardware_t {
  uint32_t                            subdev_ids[MAX_VPE_DEVICES];
  int                                 num_subdev;
  int                                 subdev_fd;
  boolean                             subdev_opened;
  uint32_t                            inst_id;
  vpe_hardware_caps_t                 caps;
  vpe_hardware_info_t                 hwinfo;
  vpe_hardware_status_t               status;
  vpe_hardware_event_subscribe_info_t event_subs_info;
  vpe_hardware_stream_status_t        stream_status[VPE_HARDWARE_MAX_STREAMS];
  pthread_cond_t                      no_pending_cond;
  pthread_mutex_t                     mutex;
} vpe_hardware_t;

typedef enum {
  VPE_HW_CMD_GET_CAPABILITIES,
  VPE_HW_CMD_SUBSCRIBE_EVENT,
  VPE_HW_CMD_UNSUBSCRIBE_EVENT,
  VPE_HW_CMD_NOTIFY_EVENT,
  VPE_HW_CMD_STREAMON,
  VPE_HW_CMD_STREAMOFF,
  VPE_HW_CMD_PROCESS_FRAME
} vpe_hardware_cmd_type_t;

typedef struct _vpe_hardware_event_data_t {
  uint32_t  frame_id;
  uint32_t  identity;
  uint32_t  buf_idx;
  void     *cookie;
} vpe_hardware_event_data_t;

typedef struct _vpe_hardware_cmd_t {
  vpe_hardware_cmd_type_t type;
  union {
    uint32_t                         streamoff_identity;
    vpe_hardware_event_data_t       *event_data;
    vpe_hardware_params_t           *hw_params;
    vpe_hardware_stream_buff_info_t *stream_buff_list;

  } u;
} vpe_hardware_cmd_t;

/* interface functions */
vpe_hardware_t* vpe_hardware_create();
int32_t vpe_hardware_open_subdev(vpe_hardware_t *vpehw);
int32_t vpe_hardware_close_subdev(vpe_hardware_t *vpehw);
int32_t vpe_hardware_destroy(vpe_hardware_t *vpehw);
int32_t vpe_hardware_process_command(vpe_hardware_t *vpehw,
  vpe_hardware_cmd_t cmd);
vpe_hardware_status_t vpe_hardware_get_status(vpe_hardware_t *vpehw);

/* static functions */
static int32_t vpe_hardware_find_subdev(vpe_hardware_t *vpehw);
static int32_t vpe_hardware_subcribe_v4l2_event(vpe_hardware_t *vpehw);
static int32_t vpe_hardware_notify_event_get_data
(vpe_hardware_t *vpehw,
 vpe_hardware_event_data_t *event_data);
static int32_t vpe_hardware_get_capabilities(vpe_hardware_t *vpehw);
static int32_t vpe_hardware_process_streamon
(vpe_hardware_t *vpehw, vpe_hardware_stream_buff_info_t *hw_strm_buff_info);
static int32_t vpe_hardware_process_streamoff(vpe_hardware_t *vpehw,
                                              uint32_t identity);
static int32_t vpe_hardware_process_frame(vpe_hardware_t *vpehw,
                                          vpe_hardware_params_t *hw_params);
static vpe_hardware_stream_status_t* vpe_hardware_get_stream_status
(vpe_hardware_t* vpehw, uint32_t identity);
static int32_t vpe_hardware_subcribe_v4l2_event(vpe_hardware_t *vpehw);
static int32_t vpe_hardware_unsubcribe_v4l2_event(vpe_hardware_t *vpehw);

#endif
