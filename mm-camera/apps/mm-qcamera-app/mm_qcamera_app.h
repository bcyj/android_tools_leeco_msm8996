/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MM_QCAMERA_APP_H__
#define __MM_QCAMERA_APP_H__

#include "camera.h"
#include "mm_qcamera_main_menu.h"
#include "mm_camera_interface2.h"
#ifndef DISABLE_JPEG_ENCODING
#include "mm_omx_jpeg_encoder.h"
#endif

#define MM_APP_MAX_DUMP_FRAME_NUM 1000
typedef struct {
	int num;
	uint32_t frame_len;
	struct msm_frame frame[MM_CAMERA_MAX_NUM_FRAMES];
} mm_camear_app_buf_t;

typedef struct {
	mm_camera_t *cam;
	int8_t my_id;
	mm_camera_op_mode_type_t op_mode;
	cam_ctrl_dimension_t dim;
	int open_flag;
    int ionfd;
	mm_camear_app_buf_t preview_buf;
	mm_camear_app_buf_t video_buf;
	mm_camear_app_buf_t snapshot_buf;
	mm_camear_app_buf_t thumbnail_buf;
    mm_camear_app_buf_t jpeg_buf;
	mm_camear_app_buf_t raw_snapshot_buf;
} mm_camera_app_obj_t;

typedef struct {
  void *ptr;
  mm_camera_t *(*mm_camera_query) (uint8_t *num_cameras);
  uint8_t *(*mm_camera_do_mmap)(uint32_t size, int *pmemFd);
  int (*mm_camera_do_munmap)(int pmem_fd, void *addr, size_t size);
  uint8_t *(*mm_camera_do_mmap_ion)(int ion_fd, struct ion_allocation_data *alloc,
		     struct ion_fd_data *ion_info_fd, int *mapFd);
  int (*mm_camera_do_munmap_ion) (int ion_fd, struct ion_fd_data *ion_info_fd,
                   void *addr, size_t size);
  uint32_t (*mm_camera_get_msm_frame_len)(cam_format_t fmt_type,
                                            camera_mode_t mode,
                                            int width,
                                            int height,
                                            int image_type,
                                            uint8_t *num_planes,
                                            uint32_t planes[]);
#ifndef DISABLE_JPEG_ENCODING
  void (*set_callbacks)(jpegfragment_callback_t fragcallback,
    jpeg_callback_t eventcallback, void* userdata, void* output_buffer,
    int * outBufferSize);
  int8_t (*omxJpegOpen)(void);
  void (*omxJpegClose)(void);
  int8_t (*omxJpegStart)(void);
  int8_t (*omxJpegEncode)(omx_jpeg_encode_params *encode_params);
  void (*omxJpegFinish)(void);
  int8_t (*mm_jpeg_encoder_setMainImageQuality)(uint32_t quality);
#endif
} hal_interface_lib_t;

typedef struct {
	mm_camera_t *cam;
	uint8_t num_cameras;
	mm_camera_app_obj_t obj[MSM_MAX_CAMERA_SENSORS];
	int use_overlay;
	int use_user_ptr;
    hal_interface_lib_t hal_lib;
} mm_camera_app_t;

extern mm_camera_app_t my_cam_app;
extern USER_INPUT_DISPLAY_T input_display;
extern int mm_app_dl_render(int frame_fd, struct crop_info * cropinfo);
extern mm_camera_app_obj_t *mm_app_get_cam_obj(int8_t cam_id);
extern int mm_app_load_hal();
extern int mm_app_init();
extern void mm_app_user_ptr(int use_user_ptr);
extern int mm_app_open_ch(int cam_id, mm_camera_channel_type_t ch_type);
extern void mm_app_close_ch(int cam_id, mm_camera_channel_type_t ch_type);
extern int mm_app_set_dim(int8_t cam_id, cam_ctrl_dimension_t *dim);
extern int mm_app_set_op_mode(int cam_id, mm_camera_op_mode_type_t op_mode);
extern int mm_app_run_unit_test();
extern int mm_app_unit_test_entry(mm_camera_app_t *cam_app);
extern int mm_app_unit_test();
extern void mm_app_set_dim_def(cam_ctrl_dimension_t *dim);
extern int mm_app_open(int8_t cam_id, mm_camera_op_mode_type_t op_mode);
extern int mm_app_close(int8_t cam_id);
extern int mm_app_start_preview(int cam_id);
extern int mm_app_stop_preview(int cam_id);
extern int mm_app_start_video(int cam_id);
extern int mm_app_stop_video(int cam_id);
extern int mm_app_take_picture(int cam_id);
extern int mm_app_take_raw_picture(int cam_id);
extern int mm_app_get_dim(int8_t cam_id, cam_ctrl_dimension_t *dim);

#endif /* __MM_QCAMERA_APP_H__ */









