/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
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

static int mm_app_set_preview_fmt(int cam_id)
{
	/* now we hard code format */
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;
	mm_camera_ch_image_fmt_parm_t fmt;

	CDBG("%s: BEGIN\n", __func__);
	memset(&fmt, 0, sizeof(mm_camera_ch_image_fmt_parm_t));
	fmt.ch_type = MM_CAMERA_CH_PREVIEW;
	fmt.def.fmt = pme->dim.prev_format;
	fmt.def.dim.width = pme->dim.display_width;
	fmt.def.dim.height = pme->dim.display_height;
	rc = pme->cam->cfg->set_parm(pme->cam, MM_CAMERA_PARM_CH_IMAGE_FMT, &fmt);
	if(rc != MM_CAMERA_OK) {
		CDBG("%s:set preview format err=%d\n", __func__, rc);
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

static int mm_app_prepare_preview_buf(int cam_id, int num_preview_buf)
{
	/* now we hard code format */
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int i, j, rc = MM_CAMERA_OK;
	mm_camera_reg_buf_t reg_buf;
	uint32_t frame_offset[8];
	int fd;
	uint32_t pmem_addr = 0;
	mm_camera_channel_type_t ch_type;
    uint8_t num_planes;
    uint32_t planes[VIDEO_MAX_PLANES];

	CDBG("%s: BEGIN\n", __func__);

	if(pme->preview_buf.num) {
		/* already registered. err */
		CDBG("%s:video/preview buf alraedy registered\n", __func__);
		rc = -MM_CAMERA_E_INVALID_OPERATION;
		goto end;
	}

	memset(&reg_buf,  0,  sizeof(reg_buf));
    reg_buf.preview.buf.mp = malloc(sizeof(mm_camera_mp_buf_t) * num_preview_buf);
    if (!reg_buf.preview.buf.mp) {
	  CDBG_ERROR("%s Error allocating memory for mplanar struct ", __func__);
	  rc = -MM_CAMERA_E_NO_MEMORY; 
	  goto end;
    }
    memset(reg_buf.preview.buf.mp, 0,
    num_preview_buf * sizeof(mm_camera_mp_buf_t));
	memset(&pme->preview_buf, 0, sizeof(pme->preview_buf));

	pme->video_buf.num = num_preview_buf;
    pme->preview_buf.num = num_preview_buf;
	pme->preview_buf.frame_len = my_cam_app.hal_lib.mm_camera_get_msm_frame_len(pme->dim.prev_format,
									CAMERA_MODE_2D, pme->dim.display_width,
									pme->dim.display_height, OUTPUT_TYPE_P, &num_planes, planes);

	for(i = 0; i < pme->preview_buf.num; i++) {
#ifdef USE_ION
		pme->preview_buf.frame[i].ion_alloc.len = pme->preview_buf.frame_len;
		pme->preview_buf.frame[i].ion_alloc.flags = ION_FLAG_CACHED;
		pme->preview_buf.frame[i].ion_alloc.heap_mask =
			(0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
		pme->preview_buf.frame[i].ion_alloc.align = 4096;
		pmem_addr = (unsigned long) my_cam_app.hal_lib.mm_camera_do_mmap_ion(pme->ionfd,
                       &(pme->preview_buf.frame[i].ion_alloc), &(pme->preview_buf.frame[i].fd_data),
                       &pme->preview_buf.frame[i].fd);
#else
		pmem_addr = (unsigned long) my_cam_app.hal_lib.mm_camera_do_mmap(
				pme->preview_buf.frame_len,
				&pme->preview_buf.frame[i].fd);
#endif
		if (!pmem_addr) {
			CDBG_ERROR("%s:no mem for preview buf index %d\n", __func__, i);
				rc = -MM_CAMERA_E_NO_MEMORY;
				goto end1;
		}
		pme->preview_buf.frame[i].buffer = pmem_addr;
		pme->preview_buf.frame[i].path = OUTPUT_TYPE_P;
		pme->preview_buf.frame[i].y_off = 0;
		pme->preview_buf.frame[i].cbcr_off = planes[0];

		reg_buf.preview.buf.mp[i].frame = pme->preview_buf.frame[i];
        reg_buf.preview.buf.mp[i].frame_offset = 0;
        reg_buf.preview.buf.mp[i].num_planes = num_planes;

		reg_buf.preview.buf.mp[i].planes[0].length = planes[0];
        reg_buf.preview.buf.mp[i].planes[0].m.userptr = pme->preview_buf.frame[i].fd;
        reg_buf.preview.buf.mp[i].planes[0].data_offset = 0;
        reg_buf.preview.buf.mp[i].planes[0].reserved[0] =
          reg_buf.preview.buf.mp[i].frame_offset;
		for (j = 1; j < num_planes; j++) {
          reg_buf.preview.buf.mp[i].planes[j].length = planes[j];
          reg_buf.preview.buf.mp[i].planes[j].m.userptr =
            pme->preview_buf.frame[i].fd;
		  reg_buf.preview.buf.mp[i].planes[j].data_offset = 0;
          reg_buf.preview.buf.mp[i].planes[j].reserved[0] =
            reg_buf.preview.buf.mp[i].planes[j-1].reserved[0] +
            reg_buf.preview.buf.mp[i].planes[j-1].length;
        }

	}

	reg_buf.ch_type = MM_CAMERA_CH_PREVIEW;
	reg_buf.preview.num = pme->preview_buf.num;
	rc = pme->cam->cfg->prepare_buf(pme->cam, &reg_buf);
	if(rc != MM_CAMERA_OK) {
		CDBG("%s:reg preview buf err=%d\n", __func__, rc);
	}
end1:
    if (reg_buf.def.buf.mp)
      free(reg_buf.def.buf.mp);
end:
	CDBG("%s: END, num_preview_buf = %d, rc=%d\n", __func__, pme->preview_buf.num, rc);
	return rc;
}
static int mm_app_unprepare_preview_buf(int cam_id)
{
	/* now we hard code format */
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int i, rc = MM_CAMERA_OK;
	rc = pme->cam->cfg->unprepare_buf(pme->cam, MM_CAMERA_CH_PREVIEW);
	if(rc) {
	  CDBG("%s:unreg preview buf err=%d\n", __func__, rc);
	  goto end;
	}
	for(i = 0; i < pme->preview_buf.num; i++) {
#ifdef USE_ION
        rc = my_cam_app.hal_lib.mm_camera_do_munmap_ion (pme->ionfd, &(pme->preview_buf.frame[i].fd_data),
                   (void *)pme->preview_buf.frame[i].buffer, pme->preview_buf.frame_len);
#else
		rc = my_cam_app.hal_lib.mm_camera_do_munmap(pme->preview_buf.frame[i].fd,
								(void *)pme->preview_buf.frame[i].buffer,
								pme->preview_buf.frame_len);
#endif
		if(rc != MM_CAMERA_OK) {
		  CDBG("%s: mm_camera_do_munmap err, pmem_fd = %d, rc = %d",
			   __func__, pme->preview_buf.frame[i].fd, rc);
		  goto end;
		}
	}
	/* zero out the buf stuct */
	memset(&pme->preview_buf, 0, sizeof(pme->preview_buf));
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}
static void mm_app_preview_notify_cb(mm_camera_ch_data_buf_t *bufs,
	void *user_data)
{
	mm_camera_app_obj_t *pme = user_data;
	int rc;
	CDBG("%s: BEGIN - ch=%d, frame idx = %d\n", __func__, bufs->type, bufs->def.idx);
	if(0 != (rc = mm_app_dl_render(bufs->def.frame->fd, NULL))) {
		CDBG("%s:DL rendering err=%d, frame fd=%d,frame idx = %d\n",
				 __func__, rc, bufs->def.frame->fd, bufs->def.idx);
	}
	pme->cam->evt->buf_done(pme->cam, bufs);
	CDBG("%s: END\n", __func__);
}
static int mm_app_reg_preview_data_cb(int cam_id, int is_reg)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(is_reg) {
		rc = pme->cam->evt->register_buf_notify(pme->cam,
		   MM_CAMERA_CH_PREVIEW,
		   mm_app_preview_notify_cb,
		   MM_CAMERA_REG_BUF_CB_INFINITE, 0,
		   pme);
		if(rc != MM_CAMERA_OK) {
			CDBG("%s:register preview data notify cb err=%d\n", __func__, rc);
			return rc;
		}
	} else {
		rc = pme->cam->evt->register_buf_notify(pme->cam,
					 MM_CAMERA_CH_PREVIEW,
					 NULL,
                                         (mm_camera_register_buf_cb_type_t)NULL,
                                         0, NULL);
		if(rc != MM_CAMERA_OK) {
			CDBG("%s:unregister preview data notify cb err=%d\n", __func__, rc);
			goto end;
		}
		if(rc != MM_CAMERA_OK) {
			CDBG("%s:unregister video data notify cb err=%d\n", __func__, rc);
			goto end;
		}
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

static int mm_app_open_preview(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(MM_CAMERA_OK != (rc = mm_app_set_op_mode(cam_id, MM_CAMERA_OP_MODE_VIDEO))) {
		CDBG("%s:mm_app_set_op_mode(op_mode=%d) err=%d\n", __func__,
				 MM_CAMERA_OP_MODE_VIDEO, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_open_ch(cam_id, MM_CAMERA_CH_PREVIEW))) {
		CDBG("%s:open preview channel err=%d\n", __func__, rc);
		goto end;
	}
end:
CDBG("%s: END, rc = %d\n", __func__, rc);
	return rc;
}
static int mm_app_close_preview(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	mm_app_close_ch(cam_id, MM_CAMERA_CH_PREVIEW);
end:
CDBG("%s: END, rc = %d\n", __func__, rc);
	return rc;
}

static int mm_app_streamon_preview(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

  use_overlay_fb_display_driver();
  if (launch_camframe_fb_thread()) {
    CDBG("%s:launch_camframe_fb_thread failed!\n", __func__);
    rc = -MM_CAMERA_E_GENERAL;
		goto end;
  }
	CDBG("%s: launch_camframe_fb_thread done\n", __func__);

	if(MM_CAMERA_OK != (rc = pme->cam->ops->action(pme->cam,
												   TRUE, MM_CAMERA_OPS_PREVIEW, 0))) {
		CDBG("%s:preview streaming err=%d\n", __func__, rc);
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}


static int mm_app_streamoff_preview(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(MM_CAMERA_OK != (rc = pme->cam->ops->action(pme->cam, FALSE, MM_CAMERA_OPS_PREVIEW, 0))) {
		CDBG("%s:preview streaming err=%d\n", __func__, rc);
		goto end;
	}
	/* stop the display thread */
	release_camframe_fb_thread();
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_start_preview(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(MM_CAMERA_OK != (rc = mm_app_open_preview(cam_id))) {
		CDBG("%s:mm_app_open_preview err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_set_preview_fmt(cam_id))) {
		CDBG("%s:set preview format err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_prepare_preview_buf(cam_id,
						PREVIEW_FRAMES_NUM))) {
		CDBG("%s:reg preview buf err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_reg_preview_data_cb(cam_id, TRUE))) {
		CDBG("%s:reg preview data cb err=%d\n", __func__, rc);
	}
	if(MM_CAMERA_OK != (rc = mm_app_streamon_preview(cam_id))) {
		CDBG("%s:preview streaming on err=%d\n", __func__, rc);
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_stop_preview(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);

	if(MM_CAMERA_OK != (rc = mm_app_streamoff_preview(cam_id))) {
		CDBG("%s:streamoff preview err=%d\n", __func__, rc);
		return rc;
	}
	if(MM_CAMERA_OK != (rc = mm_app_unprepare_preview_buf(cam_id))) {
		CDBG("%s:treg preview buf err=%d\n", __func__, rc);
		return rc;
	}
	if(MM_CAMERA_OK != (rc = mm_app_close_preview(cam_id))) {
		CDBG("%s:mm_app_close_preview err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_reg_preview_data_cb(cam_id, FALSE))) {
		CDBG("%s:reg preview data cb err=%d\n", __func__, rc);
		return rc;
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}


