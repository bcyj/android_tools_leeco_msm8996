/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#ifndef __FRAMEPROC_H__
#define __FRAMEPROC_H__

#include "frameproc_interface.h"
#include "camera_dbg.h"

#define FRAME_PROC_DEBUG              0
#define FRAME_PROC_AFD_DEBUG          0
#define FRAME_PROC_HJR_DEBUG          0
#define FRAME_PROC_DENOISE_DEBUG      0
#define FRAME_PROC_FACE_DETECT_DEBUG  0
#define FRAME_PROC_HDR_DEBUG          0

#undef CDBG_HIGH
#if(FRAME_PROC_DEBUG)
  #ifdef _ANDROID_
  #include <utils/Log.h>
  #endif
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "CAMERA FRAME_PROC"
  #define CDBG_FRAME_PROC(fmt, args...) LOGE(fmt, ##args)
  #define CDBG_HIGH(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_FRAME_PROC(fmt, args...) do{}while(0)
  #define CDBG_HIGH(fmt, args...) do{}while(0)
#endif

#ifndef LOGV
#define LOGV(fmt, args...) CDBG(fmt, ##args)
#endif

#define MAX_INSTANCES 8
#define FRAMEPROC_MAX_CLIENT_NUM    2
typedef enum {
NUM_PLANE_1,
NUM_PLANE_2,
NUM_PLANE_3,
}frame_plane_type;

typedef struct {
  uint32_t handle;
  frame_proc_interface_t *frame_proc_intf;
} frame_proc_parms;

typedef struct {
  uint32_t handle;
  int exit_flag;
  int fd_init_flag;
  uint32_t obj_idx_mask;
  uint8_t client_idx;
  uint8_t my_comp_id;
  uint32_t vfe_version;
  /* frame to be ingored for hdr */
  uint8_t ignore_snap_frame;
  mctl_ops_t *ops;
  frame_proc_interface_input_t input;
  frame_proc_interface_output_t output;
} frame_proc_t;

typedef struct {
  uint8_t refcount;
} frame_proc_obj_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t frame_proc_handle_cnt;
  frame_proc_t client[FRAMEPROC_MAX_CLIENT_NUM];
  frame_proc_obj_t obj[FRAMEPROC_MAX_CLIENT_NUM];
} frame_proc_comp_root_t;

/*******************************
 AFD APIs
*******************************/
int afd_init(frame_proc_t *frameCtrl);
int afd_exit(frame_proc_t *frameCtrl);
int frame_proc_afd_execute(frame_proc_t *frameCtrl);
int afd_set_params(frame_proc_t *frameCtrl, frame_proc_set_afd_data_t *data);


/*******************************
 Face Detection APIs
*******************************/
int face_proc_init(frame_proc_t *frameCtrl,fd_mode_t mode);
int face_proc_exit(frame_proc_t *frameCtrl);
int face_proc_execute(frame_proc_t *frameCtrl);
int face_proc_set_params(frame_proc_t *frameCtrl, frame_proc_set_fd_data_t *data);

/*******************************
 HJR APIs
*******************************/
int hjr_execute(frame_proc_t *frameCtrl);
int hjr_set_params(frame_proc_t *frameCtrl, frame_proc_set_hjr_data_t *data);

/*******************************
 WAVELET DENOISE APIs
*******************************/
int wavelet_denoise_init(frame_proc_t *frameCtrl);
int wavelet_denoise_exit(frame_proc_t *frameCtrl);
int wavelet_denoise_execute(frame_proc_t *frameCtrl);
int wavelet_denoise_set_params(frame_proc_t *frameCtrl, frame_proc_set_wd_data_t *data);

/*******************************
 FRAME PROC APIs
*******************************/
int hdr_execute(frame_proc_t *frameCtrl);
int hdr_set_params(frame_proc_t *frameCtrl, frame_proc_set_hdr_data_t *data);;

#endif /* __FRAMEPROC_H__ */
