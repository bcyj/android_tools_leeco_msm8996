/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __MCTL_PP_H__
#define __MCTL_PP_H__

#include <sys/socket.h>
//#include <linux/un.h>
#include "camera.h"
#include "cam_list.h"
#include "c2d_client_intf.h"
#include "vpe_api.h"
#include "frameproc_interface.h"
#include "mctl_pp_node.h"

#define QCAM_PP_MAX_NUM                8

/* forward declaration */
struct ion_flush_data;

typedef enum {
  /* ask the channel to flash out the queued frames. */
  QCAM_MCTL_CMD_DATA,
  QCAM_MCTL_CMD_STREAMON,
  QCAM_MCTL_CMD_STREAMOFF,
  QCAM_MCTL_CMD_ACQUIRE_HW,
  QCAM_MCTL_CMD_RELEASE_HW,
  QCAM_MCTL_CMD_SET_CROP,
  QCAM_MCTL_CMD_SET_DIS,
  QCAM_MCTL_CMD_CONFIG_SRC,
  QCAM_MCTL_CMD_RESET_SRC,
  QCAM_MCTL_CMD_CONFIG_DEST,
  QCAM_MCTL_CMD_RESET_DEST,
  QCAM_MCTL_CMD_SHUTDOWN,
  QCAM_MCTL_CMD_EXIT,
  QCAM_MCTL_CMD_MAX
} qcam_mctl_poll_cmd_type_t;

struct mctl_pp_stream_info {
  int image_mode;
  int path;
};

struct mctl_pp_crop_info {
  struct msm_pp_crop crop;
  int path;
};

struct mctl_pp_crop_cmd {
  uint32_t  src_w;
  uint32_t  src_h;
  uint32_t  dst_w;
  uint32_t  dst_h;
  int src_path;
};

#define MCTL_PP_MAX_SRC_NUM             2
#define MCTL_PP_MAX_DEST_NUM            2

#define MCTL_PP_SRC_IDX_0               0
#define MCTL_PP_DEST_IDX_0              0
#define MCTL_PP_DEST_IDX_1              1

typedef enum {
  MCTL_PP_VPE_CROP_AND_DIS,
  MCTL_PP_C2D_CROP_2D,
  MCTL_PP_S3D_3D_CORE,
  MCTL_PP_S3D_C2D_VPE,
} mctl_pp_proc_type_t;

enum {
  MCTL_PP_INPUT_FROM_KERNEL,
  MCTL_PP_INPUT_FROM_USER,
} mctl_pp_input_type;

typedef struct {
  int path;
  int dis_enable;
  int image_mode;
  int image_width;
  int image_height;
  plane_info_t plane[MAX_PLANES];
  cam_format_t format;
} mctl_pp_src_cfg_params_t;

typedef struct {
  cam_format_t format;
  int image_mode;
  int action_flag;
  int path;
  int dis_enable;
  /* If rotation is set then it is client's responsibility to set
   * dest_image_width, dest_image_height accordingly.
   */
  int rotation;
  int image_width;
  int image_height;
  plane_info_t plane[MAX_PLANES];
  mctl_pp_proc_type_t proc_type;
  tgt_pp_hw_t hw_type;
} mctl_pp_dest_cfg_params_t;

typedef struct {
  int src_idx;
  int dest_idx;
  mctl_pp_dest_cfg_params_t parms;
} mctl_pp_dest_cfg_cmd_t;

typedef struct {
  int num_dest;
  int op_mode;
  int num_src;
  int src_idx;
  cam_ctrl_dimension_t dimInfo;
  mctl_pp_src_cfg_params_t parms;
} mctl_pp_src_cfg_cmd_t;

typedef struct {
  int num;
  tgt_pp_hw_t pp_hw[PP_HW_TYPE_MAX_NUM];
} mctl_pp_hw_t;

typedef struct {
  int cmd_type;
  int evt_type;
  union {
    mctl_pp_hw_t hw;
    struct msm_cam_evt_divert_frame div_frame;
    struct mctl_pp_stream_info stream_info;
    struct mctl_pp_crop_cmd crop_info;
    cam_dis_info_t dis_cmd;
    struct msm_mctl_pp_event_info pp_event;
    mctl_pp_dest_cfg_cmd_t dest_cfg;
    mctl_pp_src_cfg_cmd_t src_cfg;
  };
} mctl_pp_cmd_t;

typedef struct {
  int (*acquire_hw)(void *p_poll_cb, int src_idx, int dest_idx);
  int (*release_hw)(void *p_poll_cb, int src_idx, int dest_idx);
  int (*config_src)(void *p_poll_cb, mctl_pp_src_cfg_cmd_t *src_cfg);
  void (*reset_src)(void *p_poll_cb, int src_idx);
  int (*config_dest)(void *p_poll_cb, mctl_pp_dest_cfg_cmd_t *dest_cfg);
  void (*reset_dest)(void *p_poll_cb, mctl_pp_dest_cfg_cmd_t *dest_cfg);
  void (*divert)(void *p_poll_cb, struct msm_cam_evt_divert_frame *div_frame,
    int src_idx);
  int (*streamon)(void *p_poll_cb, int src_idx, int dest_idx);
  int (*streamoff)(void *p_poll_cb, int src_idx, int dest_idx);
  void (*deinit)(void *p_poll_cb, int src_idx);
  void (*crop)(void *p_poll_cb, struct mctl_pp_crop_info *crop_info,
    int src_idx, int dest_idx);
  void (*dis)(void *p_poll_cb, cam_dis_info_t *dis_info, int src_idx,
    int dest_idx);
  int (*handle_ack)(void *p_poll_cb, int src_idx);
} mctl_pp_src_ops_t;

typedef struct {
  int (*pp_error)(void *src_ptr, uint32_t frame_id, int dest_idx);
  int (*pp_no_op)(void *src_ptr, int send_to_app, uint32_t frame_id,
    int dest_idx);
  void (*ack)(void *src_ptr, uint32_t frame_id, int dest_idx);
  void (*stop)(void *src_ptr, uint32_t frame_id, int dest_idx);
} mctl_pp_src_cb_ops_t;

typedef struct {
  int (*buf_error) (void *user_data,
    struct msm_cam_evt_divert_frame *div_frame, int err);
  int (*buf_no_op) (void *user_data,
    struct msm_cam_evt_divert_frame *div_frame, int send_to_app);
  void (*buf_done) (void *user_data,
    struct msm_cam_evt_divert_frame *div_frame);
  void (*pipe_stop) (void *user_data,
    struct msm_cam_evt_divert_frame *div_frame);
} mctl_pp_cur_to_prev_pipe_ops_t;

typedef struct {
  int (*mctl_pp_cmd) (void *p_poll_cb, mctl_pp_cmd_t *cmd);
} mctl_pp_prev_to_cur_pipe_ops_t;

typedef struct {
  int (*acquire_hw)(void *p_poll_cb, int src_idx, int dest_idx);
  int (*release_hw)(void *p_poll_cb, int src_idx, int dest_idx);
  int (*config_dest)(void *p_poll_cb, mctl_pp_dest_cfg_cmd_t *dest_cfg);
  void (*reset_dest)(void *p_poll_cb, mctl_pp_dest_cfg_cmd_t *dest_cfg);
  void (*divert)(void *p_poll_cb, struct msm_cam_evt_divert_frame *div_frame,
    int src_idx, int dest_idx);
  void (*free_frame)(void *p_poll_cb, struct msm_cam_evt_divert_frame *div_frame,
    int src_idx, int dest_idx);
  int (*streamon)(void *p_poll_cb, int src_idx, int dest_idx);
  int (*streamoff)(void *p_poll_cb, int src_idx, int dest_idx);
  void (*deinit)(void *p_poll_cb, int src_idx, int dest_idx);
  void (*crop)(void *p_poll_cb, struct mctl_pp_crop_info *crop_info,
    int src_idx, int dest_idx);
  void (*dis)(void *p_poll_cb, cam_dis_info_t *dis_info, int src_idx,
    int dest_idx);
  int (*handle_ack)(void *p_poll_cb, int src_idx, int dest_idx);
} mctl_pp_dest_ops_t;

typedef enum {
  MCTL_PP_FRAME_NULL = 0,
  MCTL_PP_FRAME_QUEUED_NO_FREE_FRAME,
  MCTL_PP_FRAME_QUEUED,
  MCTL_PP_FRAME_DOING
} mctl_frame_state_t;

#define MCTL_PP_MAX_EVENT 16

typedef struct {
  struct cam_list list;
  mctl_frame_state_t state;
  struct msm_cam_evt_divert_frame src_frame;
  struct msm_cam_evt_divert_frame dest_frame;
  struct msm_pp_crop crop;
  int send_frame_back;
} mctl_pp_dest_cmd_node_t;

typedef struct {
  struct cam_list list;
  mctl_frame_state_t state;
  struct msm_cam_evt_divert_frame div_frame;
  int count;
  int dest_delivered_count;
  int send_frame_back;
} mctl_pp_src_node_t;

typedef struct {
  cam_format_t format;
  int image_mode; /* app's image mode */
  int image_width;
  int image_height;
  plane_info_t plane[MAX_PLANES];
  int path;
  int dis_enable;
  uint32_t vpe_action_flag;
  uint32_t c2d_action_flag;
  struct mctl_pp_crop_info crop_info;
  cam_dis_info_t dis_info;
  camera_rotation_type rotation;
  mctl_pp_dest_cmd_node_t cmd_list;
  mctl_pp_dest_cmd_node_t free_cmd_node[MCTL_PP_MAX_EVENT];
  struct msm_cam_evt_divert_frame *p_free_frame;
  struct msm_cam_evt_divert_frame free_frame;
  int stream_on;
  void *src_ptr;
  mctl_pp_proc_type_t proc_type;
  tgt_pp_hw_t hw_type;
  int32_t pipe_fds[2];
  uint32_t dest_handle;
  int frame_cnt;
} mctl_pp_dest_data_t;

typedef struct {
  int my_idx;
  mctl_pp_dest_ops_t *ops;
  mctl_pp_src_cb_ops_t *src_cb_ops;
  mctl_pp_dest_data_t data;
  module_ops_t hw_ops;
} mctl_pp_dest_t;

typedef struct {
  cam_format_t format;
  int image_mode; /* app's image mode */
  int image_width;
  int image_height;
  plane_info_t plane[MAX_PLANES];
  int path; /* which vfe path diverted here */
  int num_dest;
  int active_dest;
  int num_dest_configured;
  int dis_enable;
  cam_dis_info_t dis_info;
  mctl_pp_src_node_t src_list;
  mctl_pp_src_node_t *dis_waiting_node;
  mctl_pp_src_node_t free_node[MCTL_PP_MAX_EVENT];
  int stream_on;
  struct timeval streamon_timestamp;
} mctl_pp_divert_src_data_t;

typedef struct {
  int my_idx;
  void *p_poll_cb;
  mctl_pp_src_ops_t *ops;
  mctl_pp_dest_t dest[MCTL_PP_MAX_DEST_NUM];
  mctl_pp_divert_src_data_t data;
} mctl_pp_divert_src_t;

typedef struct {
  cam_ctrl_dimension_t dimInfo;
  int op_mode;
  int num_src;
  mctl_pp_divert_src_t src[MCTL_PP_MAX_SRC_NUM];
  /* ops table containing functions that can be invoked
   * by the previous pp pipeline dest node to the current
   * pp pipeline src node. */
  mctl_pp_prev_to_cur_pipe_ops_t *p2c_ops;
  /* ops table containing functions that can be invoked
   * by the current pp pipeline src node to the previous
   * pp pipeline dest node. */
  mctl_pp_cur_to_prev_pipe_ops_t *c2p_ops;
} mctl_pp_ctrl_t;

#define MCTL_PP_MAX_FRAME_NUM      16

typedef struct {
  int active_mapping_count;
  mm_camera_frame_map_type remote_frame_info[MCTL_PP_MAX_FRAME_NUM];
  mctl_pp_local_buf_info_t  local_frame_info[MCTL_PP_MAX_FRAME_NUM];
} mctl_pp_buf_info_t;

typedef struct {
  struct pollfd poll_fd;
  int in_use;
  int src_idx;
  int dest_idx;
} mctl_pp_hw_poll_list_t;

typedef struct {
  int32_t pfds[2];
  int num_poll_fds;
  /* Add the hw whose ACK needs to be polled by mctl pp
   * into this list and set in_use flag to true.
   * While releasing the hw, reset the in_use flag.*/
  mctl_pp_hw_poll_list_t poll_fds[PP_HW_TYPE_MAX_NUM];
  int used;
  pthread_t pid;
  int32_t release;
  int vnode_idx;
  int timeoutms;
  mctl_pp_cmd_t cmd;
  void *cfg_ctrl;
  mctl_pp_ctrl_t pp_ctrl;
} mctl_pp_super_pipe_node_t;

typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond_v;
  int32_t status;
  int pp_idx;
  /* mctl_pp_input_type */
  int input_type;
  mctl_pp_super_pipe_node_t data;
} mctl_pp_t;

typedef enum {
  FP_PREVIEW_SET = 0,
  FP_PREVIEW_RESET,
  FP_SNAPSHOT_SET,
  FP_SNAPSHOT_RESET,
  FP_RESET,
} frame_proc_key_t;

typedef struct {
  cam_format_t src_format;
  cam_format_t dest_format;
} mctl_pp_hw_init_data;

extern int mctl_pp_launch(mctl_pp_t *pp_ctrl, void *cfg_ctrl, int input_type);
extern int mctl_pp_release(mctl_pp_t *pp_ctrl);

int mctl_pp_get_free_pipeline(void *cctrl, int *free_idx);
int mctl_pp_put_free_pipeline(void *cctrl, int idx);

int8_t mctl_pp_divert_frame (void *parm1, void *parm2);
int8_t mctl_pp_proc_event   (void *parm1, void *parm2);

int mctl_pp_src_init(mctl_pp_t *poll_cb);
int mctl_pp_dest_init(mctl_pp_t *poll_cb, void *p_src, int src_idx,
  int dest_idx, mctl_pp_src_cb_ops_t *p_src_cb_ops);
int mctl_pp_dest_done_notify(void *userdata, pp_frame_data_t *frame);
int mctl_pp_src_frame_done(mctl_pp_t *poll_cb,
  struct msm_cam_evt_divert_frame *frame);
int mctl_pp_divert_done(void *parent,
  struct msm_cam_evt_divert_frame *div_frame);
int mctl_pp_add_poll_fd(mctl_pp_t *poll_cb, struct pollfd *poll_fd,
  int src_idx, int dest_idx);
int mctl_pp_remove_poll_fd(mctl_pp_t *poll_cb, int src_idx, int dest_idx);

/* VPE cmd functions */
uint32_t mctl_pp_acquire_vpe(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest);
int mctl_pp_gen_vpe_config_parm(mctl_pp_t *poll_cb, mctl_pp_divert_src_t *src,
  mctl_pp_dest_t *dest);
void mctl_pp_dest_vpe_resend_cmd(void *p_poll_cb, mctl_pp_dest_t *dest,
  mctl_pp_dest_cmd_node_t *node, int *del_node);
int mctl_pp_dest_vpe_ack_notify(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest,
  mctl_pp_dest_cmd_node_t *node);
int mctl_pp_release_vpe(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest);

/* C2D cmd functions */
uint32_t mctl_pp_acquire_c2d(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest);
int mctl_pp_gen_c2d_config_parm(mctl_pp_t *poll_cb, mctl_pp_divert_src_t *src,
  mctl_pp_dest_t *dest);
void mctl_pp_dest_c2d_resend_cmd(void *p_poll_cb, mctl_pp_dest_t *dest,
  mctl_pp_dest_cmd_node_t *node, int *del_node);
int mctl_pp_dest_c2d_ack_notify(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest,
  mctl_pp_dest_cmd_node_t *node);
int mctl_pp_release_c2d(mctl_pp_t *poll_cb, mctl_pp_dest_t *dest);

/* Helper cmd functions */
void mctl_pp_merge_crop_dis_offset(struct msm_pp_crop *crop_in,
  cam_dis_info_t *in_dis, struct msm_pp_crop *final_crop);
int mctl_pp_cache_ops(struct ion_flush_data *cache_data, int type,
                      int ion_dev_fd);
int32_t mctl_pp_cmd(mctl_pp_t *poll_cb, mctl_pp_cmd_t *cmd);

/* MCTL PP Node functions */
int mctl_pp_node_open(mctl_pp_node_obj_t *pp_node, char *dev_name);
int mctl_pp_node_prepare(void *ctrl, mctl_pp_node_obj_t *pp_node, int ion_dev_fd);
int mctl_pp_node_get_buffer_count(mctl_pp_node_obj_t *myobj);
int mctl_pp_node_get_buffer_info(mctl_pp_node_obj_t *myobj,
                                mctl_pp_local_buf_info_t *buf_data);
int mctl_pp_node_buf_done(mctl_pp_node_obj_t *pp_node,
                          struct msm_cam_evt_divert_frame *div_frame);
int mctl_pp_node_release(mctl_pp_node_obj_t *pp_node, int ion_dev_fd);
void mctl_pp_node_proc_evt(mctl_pp_node_obj_t *pp_node, struct pollfd *fds);
void mctl_pp_node_close(mctl_pp_node_obj_t *pp_node);

#endif /* __MCTL_PP_H__ */
