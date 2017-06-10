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
#include "mm_qcamera_app.h"

#define BUFF_SIZE_128 128
static int num_run = 0;
static int mm_app_dump_video_frame(struct msm_frame *frame,
								   uint32_t len)
{
	static int v_cnt = 0;
	char bufp[BUFF_SIZE_128];
	int file_fdp;
	int rc = 0;

	return rc; /* disable dump */

	v_cnt++;
	if(0 == (v_cnt % 10))
		snprintf(bufp, BUFF_SIZE_128, "/data/v_%d.yuv", v_cnt);
	else
		return 0;

	file_fdp = open(bufp, O_RDWR | O_CREAT, 0777);

	if (file_fdp < 0) {
		CDBG("cannot open file %s\n", bufp);
		rc = -1;
		goto end;
	}
	CDBG("%s:dump frame to '%s'\n", __func__, bufp);
	write(file_fdp,
		(const void *)frame->buffer, len);
	close(file_fdp);
end:
	return rc;
}

static int mm_app_set_video_fmt(int cam_id)
{
	/* now we hard code format */
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;
	mm_camera_ch_image_fmt_parm_t fmt;

	CDBG("%s: BEGIN\n", __func__);
	/* here we set both preview and video */
	memset(&fmt, 0, sizeof(mm_camera_ch_image_fmt_parm_t));
	fmt.ch_type = MM_CAMERA_CH_VIDEO;
	fmt.video.video.fmt = pme->dim.enc_format;
	fmt.video.video.dim.width = pme->dim.video_width;
	fmt.video.video.dim.height = pme->dim.video_height;
	rc = pme->cam->cfg->set_parm(pme->cam, MM_CAMERA_PARM_CH_IMAGE_FMT, &fmt);
	if(rc != MM_CAMERA_OK) {
		CDBG("%s:set video format err=%d\n", __func__, rc);
		goto end;
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

static int mm_app_prepare_video_buf(int cam_id, int num_video_buf)
{
	/* now we hard code format */
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int i, rc = MM_CAMERA_OK;
	mm_camera_reg_buf_t reg_buf;
	uint32_t frame_offset[8];
	int fd;
	uint32_t pmem_addr = 0;
	mm_camera_channel_type_t ch_type;
	uint8_t num_planes;
	uint32_t planes[VIDEO_MAX_PLANES];

	CDBG("%s: BEGIN\n", __func__);
	memset(&reg_buf,  0,  sizeof(reg_buf));
	memset(&pme->video_buf, 0, sizeof(pme->video_buf));
	pme->video_buf.frame_len = my_cam_app.hal_lib.mm_camera_get_msm_frame_len(pme->dim.enc_format,
							CAMERA_MODE_2D, pme->dim.video_width,
							pme->dim.video_height, OUTPUT_TYPE_V, &num_planes, planes);
	pme->video_buf.num = num_video_buf;
	for(i = 0; i < pme->video_buf.num; i++) {
		pme->video_buf.frame[i].buffer = (unsigned long)  my_cam_app.hal_lib.mm_camera_do_mmap(
			pme->video_buf.frame_len, &pme->video_buf.frame[i].fd);
		if (!pme->video_buf.frame[i].buffer) {
			CDBG("%s:no mem for video buf index %d\n", __func__, i);
			rc = -MM_CAMERA_E_NO_MEMORY;
			goto end;
		}
		frame_offset[i] = 0;
		pme->video_buf.frame[i].path = OUTPUT_TYPE_V;
		pme->video_buf.frame[i].cbcr_off = planes[0];
	}
	reg_buf.ch_type = MM_CAMERA_CH_VIDEO;
	reg_buf.video.video.num = pme->video_buf.num;
	reg_buf.video.video.buf.sp.frame_offset = frame_offset;
	reg_buf.video.video.buf.sp.frame = &pme->video_buf.frame[0];
	rc = pme->cam->cfg->prepare_buf(pme->cam, &reg_buf);
	if(rc != MM_CAMERA_OK) {
		CDBG("%s:reg video buf err=%d\n", __func__, rc);
		goto end;
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}
static int mm_app_unprepare_video_buf(int cam_id)
{
	/* now we hard code format */
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int i, rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	/* this is unreg case */
	rc = pme->cam->cfg->unprepare_buf(pme->cam, MM_CAMERA_CH_VIDEO);
	if(rc) {
		CDBG("%s:unreg video buf err=%d\n", __func__, rc);
		goto end;
	}
	for(i = 0; i < pme->video_buf.num; i++) {
		rc = my_cam_app.hal_lib.mm_camera_do_munmap(pme->video_buf.frame[i].fd, (void *)pme->video_buf.frame[i].buffer,
											 pme->video_buf.frame_len);
		if(rc != MM_CAMERA_OK) {
		  CDBG("%s: mm_camera_do_munmap err, pmem_fd = %d, rc = %d",
			   __func__, pme->video_buf.frame[i].fd, rc);
		  goto end;
		}

	}
	memset(&pme->video_buf, 0, sizeof(pme->video_buf));
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}
static void mm_app_video_notify_cb(mm_camera_ch_data_buf_t *bufs,
	void *user_data)
{
	mm_camera_app_obj_t *pme = user_data;

	CDBG("%s: BEGIN\n", __func__);
    mm_app_dump_video_frame(bufs->video.video.frame, pme->video_buf.frame_len);
	pme->cam->evt->buf_done(pme->cam, bufs);
	CDBG("%s: END\n", __func__);
}
static int mm_app_reg_video_data_cb(int cam_id, int is_reg)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(is_reg) {
		rc = pme->cam->evt->register_buf_notify(pme->cam,
				MM_CAMERA_CH_VIDEO,
				mm_app_video_notify_cb,
				MM_CAMERA_REG_BUF_CB_INFINITE, 0,
				pme);
		if(rc != MM_CAMERA_OK) {
			CDBG("%s:register video data notify cb err=%d\n",
			     __func__, rc);
			goto end;
		}
	} else {
		rc = pme->cam->evt->register_buf_notify(pme->cam,
					 MM_CAMERA_CH_VIDEO,
					 NULL,
					 (mm_camera_register_buf_cb_type_t)NULL,
					 0, pme);
		if(rc != MM_CAMERA_OK) {
			CDBG("%s:unregister video data notify cb err=%d\n",
			      __func__, rc);
			goto end;
		}
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}
static int mm_app_streamon_video(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;
	CDBG("%s: launch_camframe_fb_thread done\n", __func__);
	if(MM_CAMERA_OK != (rc = pme->cam->ops->action(pme->cam,
				TRUE, MM_CAMERA_OPS_VIDEO, 0))) {
		CDBG("%s:video streaming err=%d\n", __func__, rc);
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}


static int mm_app_streamoff_video(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(MM_CAMERA_OK != (rc = pme->cam->ops->action(pme->cam, FALSE, MM_CAMERA_OPS_VIDEO, 0))) {
		CDBG("%s:video streaming err=%d\n", __func__, rc);
	}
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}
static int mm_app_open_video(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(MM_CAMERA_OK != (rc = mm_app_open_ch(cam_id, MM_CAMERA_CH_VIDEO))) {
		CDBG("%s:open video channel err=%d\n", __func__, rc);
		goto end;
	}
end:
CDBG("%s: END, rc = %d\n", __func__, rc);
	return rc;
}
static int mm_app_close_video(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	mm_app_close_ch(cam_id, MM_CAMERA_CH_VIDEO);
end:
CDBG("%s: END, rc = %d\n", __func__, rc);
	return rc;
}
int mm_app_start_video(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(MM_CAMERA_OK != (rc = mm_app_open_video(cam_id))) {
		CDBG("%s:mm_app_open_preview err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_set_video_fmt(cam_id))) {
		CDBG("%s:set preview format err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_prepare_video_buf(cam_id,
						VIDEO_FRAMES_NUM))) {
		CDBG("%s:reg preview buf err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_reg_video_data_cb(cam_id, TRUE))) {
		CDBG("%s:reg preview data cb err=%d\n", __func__, rc);
	}
	if(MM_CAMERA_OK != (rc = mm_app_streamon_video(cam_id))) {
		CDBG("%s:preview streaming on err=%d\n", __func__, rc);
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_stop_video(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);

	if(MM_CAMERA_OK != (rc = mm_app_streamoff_video(cam_id))) {
		CDBG("%s:streamoff video err=%d\n", __func__, rc);
		return rc;
	}
	if(MM_CAMERA_OK != (rc = mm_app_reg_video_data_cb(cam_id, FALSE))) {
		CDBG("%s:reg video data cb err=%d\n", __func__, rc);
		return rc;
	}
	if(MM_CAMERA_OK != (rc = mm_app_unprepare_video_buf(cam_id))) {
		CDBG("%s:treg video buf err=%d\n", __func__, rc);
		return rc;
	}
	if(MM_CAMERA_OK != (rc = mm_app_close_video(cam_id))) {
		CDBG("%s:mm_app_close_video err=%d\n", __func__, rc);
		goto end;
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

