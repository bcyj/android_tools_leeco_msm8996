/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <pthread.h>
#include "camera_dbg.h"
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <dlfcn.h>
#include "mm_qcamera_main_menu.h"
#include "mm_qcamera_app.h"

mm_camera_app_t my_cam_app;

mm_camera_app_obj_t *mm_app_get_cam_obj(int8_t cam_id)
{
	return &my_cam_app.obj[cam_id];
}

void mm_app_user_ptr(int use_user_ptr)
{
	my_cam_app.use_user_ptr = use_user_ptr;
}

void mm_app_set_dim_def(cam_ctrl_dimension_t *dim)
{
	dim->display_width = WVGA_WIDTH;
	dim->display_height = WVGA_HEIGHT;
	input_display.user_input_display_width = dim->display_width;
	input_display.user_input_display_height = dim->display_height;
	dim->video_width = WVGA_WIDTH;
	dim->video_width = CEILING32(dim->video_width);
	dim->video_height = WVGA_HEIGHT;
	dim->orig_video_width = dim->video_width;
	dim->orig_video_height = dim->video_height;
  dim->picture_width = MP1_WIDTH;
  dim->picture_height = MP1_HEIGHT;
  dim->orig_picture_dx = dim->picture_width;
  dim->orig_picture_dy = dim->picture_height;
  dim->ui_thumbnail_height = QVGA_HEIGHT;
  dim->ui_thumbnail_width = QVGA_WIDTH;
  dim->thumbnail_height = dim->ui_thumbnail_height;
  dim->thumbnail_width = dim->ui_thumbnail_width;
  dim->orig_picture_width = dim->picture_width;
  dim->orig_picture_height = dim->picture_height;
  dim->orig_thumb_width = dim->thumbnail_width;
  dim->orig_thumb_height = dim->thumbnail_height;
  dim->raw_picture_height = MP1_HEIGHT;
  dim->raw_picture_width = MP1_WIDTH;
  dim->hjr_xtra_buff_for_bayer_filtering;
  dim->prev_format = CAMERA_YUV_420_NV21;
  dim->enc_format = CAMERA_YUV_420_NV21;
  dim->thumb_format = CAMERA_YUV_420_NV21;
  dim->main_img_format = CAMERA_YUV_420_NV21;
  dim->prev_padding_format = CAMERA_PAD_TO_4K;
  dim->display_luma_width = dim->display_width;
  dim->display_luma_height = dim->display_height;
  dim->display_chroma_width = dim->display_width;
  dim->display_chroma_height = dim->display_height;
  dim->video_luma_width = dim->orig_video_width;
  dim->video_luma_height = dim->orig_video_height;
  dim->video_chroma_width = dim->orig_video_width;
  dim->video_chroma_height = dim->orig_video_height;
  dim->thumbnail_luma_width = dim->thumbnail_width;
  dim->thumbnail_luma_height = dim->thumbnail_height;
  dim->thumbnail_chroma_width = dim->thumbnail_width;
  dim->thumbnail_chroma_height = dim->thumbnail_height;
  dim->main_img_luma_width = dim->picture_width;
  dim->main_img_luma_height = dim->picture_height;
  dim->main_img_chroma_width = dim->picture_width;
  dim->main_img_chroma_height = dim->picture_height;
}

int mm_app_load_hal()
{
  memset(&my_cam_app, 0, sizeof(my_cam_app));
  memset(&my_cam_app.hal_lib, 0, sizeof(hal_interface_lib_t));
#if defined(_MSM7630_)
   my_cam_app.hal_lib.ptr = dlopen("/usr/lib/hw/camera.msm7630.so", RTLD_LAZY);
#elif defined(_MSM7627A_)
   my_cam_app.hal_lib.ptr = dlopen("/usr/lib/hw/camera.msm7627A.so", RTLD_LAZY);
#else
  my_cam_app.hal_lib.ptr = dlopen("hw/camera.msm8960.so", RTLD_NOW);
#endif
  if (!my_cam_app.hal_lib.ptr) {
    CDBG_ERROR("%s Error opening HAL library %s\n", __func__, dlerror());
    return -1;
  }
  *(void **)&(my_cam_app.hal_lib.mm_camera_query) =
        dlsym(my_cam_app.hal_lib.ptr,
              "mm_camera_query");
  *(void **)&(my_cam_app.hal_lib.mm_camera_do_mmap) =
        dlsym(my_cam_app.hal_lib.ptr,
              "mm_camera_do_mmap");
  *(void **)&(my_cam_app.hal_lib.mm_camera_do_munmap) =
        dlsym(my_cam_app.hal_lib.ptr,
              "mm_camera_do_munmap");
#ifdef USE_ION
  *(void **)&(my_cam_app.hal_lib.mm_camera_do_mmap_ion) =
        dlsym(my_cam_app.hal_lib.ptr,
              "mm_camera_do_mmap_ion");
  *(void **)&(my_cam_app.hal_lib.mm_camera_do_munmap_ion) =
        dlsym(my_cam_app.hal_lib.ptr,
              "mm_camera_do_munmap_ion");
#endif
  *(void **)&(my_cam_app.hal_lib.mm_camera_get_msm_frame_len) =
        dlsym(my_cam_app.hal_lib.ptr,
              "mm_camera_get_msm_frame_len");
#ifndef DISABLE_JPEG_ENCODING
  *(void **)&(my_cam_app.hal_lib.set_callbacks) =
        dlsym(my_cam_app.hal_lib.ptr,
              "set_callbacks");
  *(void **)&(my_cam_app.hal_lib.omxJpegEncode) =
        dlsym(my_cam_app.hal_lib.ptr,
              "omxJpegEncode");
  *(void **)&(my_cam_app.hal_lib.omxJpegFinish) =
        dlsym(my_cam_app.hal_lib.ptr,
              "omxJpegFinish");
  *(void **)&(my_cam_app.hal_lib.omxJpegStart) =
        dlsym(my_cam_app.hal_lib.ptr,
              "omxJpegStart");
  *(void **)&(my_cam_app.hal_lib.mm_jpeg_encoder_setMainImageQuality) =
        dlsym(my_cam_app.hal_lib.ptr,
              "mm_jpeg_encoder_setMainImageQuality");
  *(void **)&(my_cam_app.hal_lib.omxJpegOpen) =
        dlsym(my_cam_app.hal_lib.ptr,
              "omxJpegOpen");
  *(void **)&(my_cam_app.hal_lib.omxJpegClose) =
        dlsym(my_cam_app.hal_lib.ptr,
              "omxJpegClose");
#endif
  return 0;
}

int mm_app_init()
{
	int rc = MM_CAMERA_OK;

	CDBG("%s:BEGIN\n", __func__);
    my_cam_app.cam = (mm_camera_t *)my_cam_app.hal_lib.mm_camera_query(&my_cam_app.num_cameras);
    if(my_cam_app.cam && my_cam_app.num_cameras > 0) {
		my_cam_app.use_overlay = TRUE;  /* deafult use overlay */
		my_cam_app.use_user_ptr = TRUE; /* default use usr ptr */
	} else rc = -1;
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_open(int8_t cam_id, mm_camera_op_mode_type_t op_mode)
{
	int rc = MM_CAMERA_OK;
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);

	CDBG("%s:BEGIN\n", __func__);
	if(pme->cam) {
		CDBG("%s:cam already open.nop\n",__func__);
		goto end;
	}
	pme->cam = &my_cam_app.cam[cam_id];
	pme->my_id = cam_id;
	pme->op_mode = op_mode;
	if(MM_CAMERA_OK != (rc = pme->cam->ops->open(pme->cam, op_mode))) {
		CDBG("%s:dev open error=%d\n", __func__, rc);
		memset(pme,0, sizeof(*pme));
		return -1;
	}
	pme->open_flag = TRUE;
	mm_app_set_dim_def(&pme->dim);
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_close(int8_t cam_id)
{
	int rc = MM_CAMERA_OK;
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	mm_camera_channel_type_t ch;

	CDBG("%s:BEGIN\n", __func__);
	if(!pme->cam) {
		CDBG("%s:cam already closed. nop\n",__func__);
		goto end;
	}
	pme->cam = &my_cam_app.cam[cam_id];
	pme->my_id = cam_id;
	pme->op_mode = 0;
	for(ch = 0; ch < MM_CAMERA_CH_MAX; ch++) {
		mm_app_close_ch(cam_id, ch);
		CDBG("%s:cam_id = %d, channel %d relaesed\n",__func__,cam_id, ch);
	}
	pme->cam->ops->close(pme->cam);
	pme->open_flag = FALSE;
	pme->cam = NULL;
	pme->my_id = 0;
	pme->op_mode = 0;
	memset(&pme->dim, 0, sizeof(pme->dim));
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}
int mm_app_set_op_mode(int cam_id, mm_camera_op_mode_type_t op_mode)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s:BEGIN\n", __func__);
	if(MM_CAMERA_OK != (rc = pme->cam->cfg->set_parm(pme->cam,
											MM_CAMERA_PARM_OP_MODE, &op_mode))) {
		CDBG("%s:set mode MM_CAMERA_OP_MODE_VIDEO err=%d\n", __func__, rc);
		return rc;
	}
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_set_dim(int8_t cam_id, cam_ctrl_dimension_t *dim)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s:BEGIN\n", __func__);
	if(pme->open_flag != TRUE) {
		CDBG("%s: dev not open yet\n", __func__);
		rc = -MM_CAMERA_E_INVALID_OPERATION;
		goto end;
	}
	/* now we only use the upper portion. TBD: needs to be fixed later */
	memcpy(&pme->dim, dim, sizeof(cam_ctrl_dimension_t));
	if(MM_CAMERA_OK != (rc = pme->cam->cfg->set_parm(pme->cam,
													MM_CAMERA_PARM_DIMENSION, &pme->dim))) {
		CDBG("%s: set dimension err=%d\n", __func__, rc);
	}
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_get_dim(int8_t cam_id, cam_ctrl_dimension_t *dim)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s:BEGIN\n", __func__);
	if(pme->open_flag != TRUE) {
		CDBG("%s: dev not open yet\n", __func__);
		rc = -MM_CAMERA_E_INVALID_OPERATION;
		goto end;
	}
	/* now we only use the upper portion. TBD: needs to be fixed later */
	//memcpy(&pme->dim, dim, sizeof(cam_ctrl_dimension_t));
	if(MM_CAMERA_OK != (rc = pme->cam->cfg->get_parm(pme->cam,
													MM_CAMERA_PARM_DIMENSION, &pme->dim))) {
		CDBG("%s: set dimension err=%d\n", __func__, rc);
	}
	CDBG("%s: raw_w=%d,raw_h=%d\n",
			 __func__, pme->dim.orig_picture_width, pme->dim.orig_picture_height);
	if(dim)
		memcpy(dim, &pme->dim, sizeof(cam_ctrl_dimension_t));

end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_open_ch(int cam_id, mm_camera_channel_type_t ch_type)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	mm_camera_channel_type_t i;
	int rc = MM_CAMERA_OK;

	CDBG("%s:BEGIN\n", __func__);
	if(ch_type < MM_CAMERA_CH_MAX) {
		rc = pme->cam->ops->ch_acquire(pme->cam, ch_type);
			CDBG("%s:cam ch_open rc=%d\n",__func__, rc);
			goto end;
	}
	else {
		/* here we open all available channels */
		for(i = 0; i < MM_CAMERA_CH_MAX; i++) {
			if( MM_CAMERA_OK != (rc = pme->cam->ops->ch_acquire(pme->cam, i))) {
				CDBG("%s:cam ch_open err=%d\n",__func__, rc);
				goto end;
			}
		}
	}
end:
	CDBG("%s:END, rc=%d\n", __func__, rc);
	return rc;
}
void mm_app_close_ch(int cam_id, mm_camera_channel_type_t ch_type)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	mm_camera_channel_type_t i;

	CDBG("%s:BEGIN,cam_id = %d, ch = %d\n", __func__, cam_id, ch_type);
	if(ch_type < MM_CAMERA_CH_MAX) {
		pme->cam->ops->ch_release(pme->cam, ch_type);
		CDBG("%s:cam ch_release %d\n",__func__, ch_type);
	}
	else {
		/* here we open all available channels */
		for(i = 0; i < MM_CAMERA_CH_MAX; i++) {
			pme->cam->ops->ch_release(pme->cam, i);
			CDBG("%s:cam ch_release %d\n",__func__, i);
		}
	}
	CDBG("%s:END,cam_id = %d, ch = %d\n", __func__, cam_id, ch_type);
}

int mm_app_unit_test()
{
#if 0 
   //TODO: mm_qcamera_unit_test is out of date.
   return mm_app_unit_test_entry(&my_cam_app);
#endif
   return 0;
}

