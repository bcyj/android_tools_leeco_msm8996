/**********************************************************************
* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __EZTUNE_PREVIEW_H__
#define __EZTUNE_PREVIEW_H__

#pragma pack(push, 1)

#define EZTUNE_PREV_GET_INFO            0x0001
#define EZTUNE_PREV_CH_CNK_SIZE         0x0002
#define EZTUNE_PREV_GET_PREV_FRAME      0x0003
#define EZTUNE_PREV_GET_JPG_SNAP        0x0004
#define EZTUNE_PREV_GET_RAW_SNAP        0x0005
#define EZTUNE_PREV_GET_RAW_PREV        0x0006

#define EZTUNE_STATUS_MINUS_ONE    -1
#define EZTUNE_STATUS_ZERO          0
#define EZTUNE_STATUS_ONE           1
#define EZTUNE_STATUS_TWO           2

typedef enum {
  EZTUNE_FORMAT_JPG = 0,
  EZTUNE_FORMAT_YUV_422,
  EZTUNE_FORMAT_YUV_420,
  EZTUNE_FORMAT_YVU_422,
  EZTUNE_FORMAT_YVU_420,
  EZTUNE_FORMAT_YCrCb_422,
  EZTUNE_FORMAT_YCrCb_420,
  EZTUNE_FORMAT_YCbCr_422,
  EZTUNE_FORMAT_YCbCr_420,
  EZTUNE_FORMAT_INVALID
} eztune_prev_format_t;

typedef enum {
  EZTUNE_ORIGIN_TOP_LEFT = 1,
  EZTUNE_ORIGIN_BOTTOM_LEFT,
  EZTUNE_ORIGIN_INVALID
} eztune_prev_origin_t;

typedef enum {
  EZTUNE_PREV_COMMAND = 1,
  EZTUNE_PREV_NEW_CNK_SIZE,
  EZTUNE_PREV_INVALID
} eztune_prev_cmd_t;

typedef struct {
  uint8_t      major_ver;
  uint8_t      minor_ver;
  uint16_t     header_size;
} eztune_prev_get_info_ver_t;

typedef struct {
  uint8_t           target_type;
  uint8_t           capabilities;
  uint32_t          cnk_size;
} eztune_prev_get_info_caps_t;

typedef struct {
  uint8_t           status;
  uint32_t          cnk_size;
} eztune_prev_ch_size_t;

typedef struct {
  uint8_t           status;
  uint16_t          width;
  uint16_t          height;
  uint8_t           format;
  uint8_t           origin;
  uint32_t          frame_size;
} eztune_prev_get_frame_t;

typedef struct {
  char             *data_buf;
} eztune_prev_data_t;

typedef struct {
  uint16_t                    current_cmd;
  eztune_prev_cmd_t           next_recv_code;
  uint32_t                    next_recv_len;
  uint32_t                    new_cnk_size;
  eztune_prev_get_info_ver_t  get_info_ver;
  eztune_prev_get_info_caps_t get_info_caps;
  eztune_prev_ch_size_t       ch_cnk_size;
  eztune_prev_get_frame_t     get_frame;
  eztune_prev_data_t          prev_frame;
} eztune_prev_protocol_t;

typedef struct {
  uint32_t prev_clientsocket_id;
  uint32_t prev_pipewrite_fd;
  mctl_config_ctrl_t *mctl;
  eztune_prev_protocol_t *prev_protocol_ptr;
} eztune_prev_t;

int32_t eztune_init_preview_settings(eztune_prev_protocol_t *prev_data,
                    cam_ctrl_dimension_t *dim);
int32_t eztune_copy_preview_frame(struct msm_pp_frame *frame);
int32_t eztune_preview_server_run (
 eztune_prev_protocol_t *prev_proc,
  int client_socket);
void eztune_prev_init_protocol_data(
  eztune_prev_protocol_t *prev_proc);

#pragma pack(pop)
#endif /* __EZTUNE_PREVIEW_H__ */
