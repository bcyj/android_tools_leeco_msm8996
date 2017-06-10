/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ISP_BUF_MGR_H__

#define __ISP_BUF_MGR_H__
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/media.h>

#include "camera_dbg.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "isp_def.h"

#define ISP_MAX_NUM_BUF_QUEUE 28

typedef struct {
  uint32_t buf_handle;
  uint32_t session_id;
  uint32_t stream_id;
  int vfe_fd;
} isp_buf_register_t;

typedef struct {
  uint32_t buf_handle;
  uint32_t session_id;
  uint32_t stream_id;
  uint32_t use_native_buf;
  int current_num_buf;
  int total_num_buf;
  cam_frame_len_offset_t buf_info;
  enum msm_isp_buf_type buf_type;
  int cached;
  mct_list_t *img_buf_list;
} isp_buf_request_t;

typedef struct {
  uint32_t user_bufq_handle;
  uint32_t kernel_bufq_handle;
  uint32_t session_id;
  uint32_t stream_id;
  int current_num_buffer;
  int total_num_buffer; /*For def buf allocation, we need to know total count*/
  int open_cnt;
  uint32_t use_native_buf;
  enum msm_isp_buf_type buf_type;
  isp_frame_buffer_t image_buf[ISP_MAX_IMG_BUF];
  uint32_t used;
  int vfe_fds[2];
  int num_vfe_fds;
  pthread_mutex_t mutex;
} isp_bufq_t;

typedef struct {
  pthread_mutex_t mutex;
  pthread_mutex_t req_mutex;
  uint32_t use_cnt;
  int ion_fd;
  uint32_t bufq_handle_count;
  isp_bufq_t bufq[ISP_MAX_NUM_BUF_QUEUE];
} isp_buf_mgr_t;

int isp_init_native_buffer(isp_frame_buffer_t *buf, int buf_idx,
          int ion_fd, cam_frame_len_offset_t *len_offset, int cached);

int isp_do_cache_inv_ion(int ion_fd, isp_frame_buffer_t *image_buf);

void isp_deinit_native_buffer(isp_frame_buffer_t *buf, int ion_fd);

int isp_request_buf(isp_buf_mgr_t *buf_mgr,
                    isp_buf_request_t *buf_request);
int isp_register_buf(isp_buf_mgr_t *buf_mgr,
                    uint32_t bufq_handle, int vfe_fd);
int isp_register_buf_list_update(isp_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, isp_buf_request_t *buf_request, int vfe_fd);
int isp_queue_buf(isp_buf_mgr_t *buf_mgr,
        uint32_t bufq_handle, int buf_idx, uint32_t dirty_buf, int vfe_fd);
void *isp_get_buf_addr(isp_buf_mgr_t *buf_mgr,
        uint32_t bufq_handle, int buf_idx);
isp_frame_buffer_t *isp_get_buf_by_idx(isp_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx);
int isp_unregister_buf(isp_buf_mgr_t *buf_mgr,
        uint32_t bufq_handle, int vfe_fd);
void isp_release_buf(isp_buf_mgr_t *buf_mgr,
        uint32_t bufq_handle);
uint32_t isp_find_matched_bufq_handle(isp_buf_mgr_t *buf_mgr,
  uint32_t session_id, uint32_t stream_id);
int isp_open_ion(void);
void isp_close_ion(int ion_fd);
int isp_open_buf_mgr(isp_buf_mgr_t *buf_mgr);
void isp_close_buf_mgr(isp_buf_mgr_t *buf_mgr);
int isp_init_buf_mgr(isp_buf_mgr_t *buf_mgr);
void isp_deinit_buf_mgr(isp_buf_mgr_t *buf_mgr);

#endif /* __ISP_BUF_MGR_H__ */
