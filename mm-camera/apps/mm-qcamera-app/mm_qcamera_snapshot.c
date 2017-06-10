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

static mm_camera_ch_data_buf_t *mCurrentFrameEncoded;
static int JpegOffset = 0;
static int raw_snapshot_cnt = 0;
static int snapshot_cnt = 0;
static pthread_mutex_t g_s_mutex;
static int g_status = 0;
static pthread_cond_t g_s_cond_v;

static void mm_app_snapshot_done()
{
  pthread_mutex_lock(&g_s_mutex);
  g_status = TRUE;
  pthread_cond_signal(&g_s_cond_v);
  pthread_mutex_unlock(&g_s_mutex);
}

static int mm_app_dump_snapshot_frame(struct msm_frame *frame,
									  uint32_t len, int is_main, int is_raw)
{
	char bufp[BUFF_SIZE_128];
	int file_fdp;
	int rc = 0;

	if(is_raw) {
		snprintf(bufp, BUFF_SIZE_128, "/data/main_raw_%d.yuv", raw_snapshot_cnt);
	} else {
		if(is_main) {
			snprintf(bufp, BUFF_SIZE_128, "/data/main_%d.yuv", snapshot_cnt);
		} else {
			snprintf(bufp, BUFF_SIZE_128, "/data/thumb_%d.yuv", snapshot_cnt);
		}
	}

	file_fdp = open(bufp, O_RDWR | O_CREAT, 0777);

	if (file_fdp < 0) {
		CDBG("cannot open file %s\n", bufp);
		rc = -1;
		goto end;
	}
	CDBG("%s:dump snapshot frame to '%s'\n", __func__, bufp);
	write(file_fdp,
		(const void *)frame->buffer, len);
	close(file_fdp);
end:
	return rc;
}


static void mm_app_dump_jpeg_frame(const void * data, uint32_t size, char* name, char* ext, int index)
{
    char buf[32];
    int file_fd;
    if ( data != NULL) {
        char * str;
        snprintf(buf, sizeof(buf), "/data/%s_%d.%s", name, index, ext);
        CDBG("%s size =%d", buf, size);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        write(file_fd, data, size);
        close(file_fd);
    }
}

int mm_app_set_snapshot_fmt(int cam_id)
{
	/* now we hard code format */
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;
	mm_camera_ch_image_fmt_parm_t fmt;

	CDBG("%s: BEGIN\n", __func__);
	memset(&fmt, 0, sizeof(mm_camera_ch_image_fmt_parm_t));
	fmt.ch_type = MM_CAMERA_CH_SNAPSHOT;
	fmt.snapshot.main.fmt = pme->dim.main_img_format;
	fmt.snapshot.main.dim.width = pme->dim.picture_width;
	fmt.snapshot.main.dim.height = pme->dim.picture_height;
	fmt.snapshot.thumbnail.fmt = pme->dim.thumb_format;
	fmt.snapshot.thumbnail.dim.width = pme->dim.ui_thumbnail_width;
	fmt.snapshot.thumbnail.dim.height = pme->dim.ui_thumbnail_height;
	rc = pme->cam->cfg->set_parm(pme->cam, MM_CAMERA_PARM_CH_IMAGE_FMT, &fmt);
	if(rc != MM_CAMERA_OK) {
		CDBG("%s:set snapshot format err=%d\n", __func__, rc);
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_set_raw_snapshot_fmt(int cam_id)
{
	/* now we hard code format */
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;
	mm_camera_ch_image_fmt_parm_t fmt;

	CDBG("%s: BEGIN\n", __func__);
	memset(&fmt, 0, sizeof(mm_camera_ch_image_fmt_parm_t));
	fmt.ch_type = MM_CAMERA_CH_RAW;
	fmt.def.fmt = CAMERA_BAYER_SBGGR10;
	fmt.def.dim.width = pme->dim.raw_picture_width;
	fmt.def.dim.height = pme->dim.raw_picture_height;
	rc = pme->cam->cfg->set_parm(pme->cam, MM_CAMERA_PARM_CH_IMAGE_FMT, &fmt);
	if(rc != MM_CAMERA_OK) {
		CDBG("%s:set raw snapshot format err=%d\n", __func__, rc);
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_prepare_snapshot_buf(int cam_id)
{
	int j;
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;
	mm_camera_reg_buf_t reg_buf;
    uint8_t num_planes_main;
    uint32_t planes_main[VIDEO_MAX_PLANES];
    uint8_t num_planes_thumbnail;
    uint32_t planes_thumbnail[VIDEO_MAX_PLANES];

	CDBG("%s: BEGIN\n", __func__);
	memset(&reg_buf,  0,  sizeof(reg_buf));
    reg_buf.snapshot.main.buf.mp = malloc(sizeof(mm_camera_mp_buf_t));
    if (!reg_buf.snapshot.main.buf.mp) {
	  CDBG_ERROR("%s Error allocating memory for mplanar struct ", __func__);
	  rc = -MM_CAMERA_E_NO_MEMORY; 
	  goto end;
    }
	reg_buf.snapshot.thumbnail.buf.mp = malloc(sizeof(mm_camera_mp_buf_t));
    if (!reg_buf.snapshot.thumbnail.buf.mp) {
	  CDBG_ERROR("%s Error allocating memory for mplanar struct ", __func__);
	  rc = -MM_CAMERA_E_NO_MEMORY; 
	  goto end;
    }
	memset(&pme->snapshot_buf, 0, sizeof(pme->snapshot_buf));
	memset(&pme->thumbnail_buf, 0, sizeof(pme->thumbnail_buf));

	pme->snapshot_buf.num = 1;
	pme->thumbnail_buf.num = 1;

	/*setup main buffer*/
	pme->snapshot_buf.frame_len = my_cam_app.hal_lib.mm_camera_get_msm_frame_len(pme->dim.main_img_format,
			 CAMERA_MODE_2D, pme->dim.picture_width,
			pme->dim.picture_height, OUTPUT_TYPE_S, &num_planes_main, planes_main);
#ifdef USE_ION
  	pme->snapshot_buf.frame[0].ion_alloc.len = pme->snapshot_buf.frame_len;
	pme->snapshot_buf.frame[0].ion_alloc.flags = ION_FLAG_CACHED;
	pme->snapshot_buf.frame[0].ion_alloc.heap_mask =
			(0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
	pme->snapshot_buf.frame[0].ion_alloc.align = 4096;
	pme->snapshot_buf.frame[0].buffer = (unsigned long) my_cam_app.hal_lib.mm_camera_do_mmap_ion(pme->ionfd,
                       &(pme->snapshot_buf.frame[0].ion_alloc), &(pme->snapshot_buf.frame[0].fd_data),
                       &pme->snapshot_buf.frame[0].fd);
#else
	pme->snapshot_buf.frame[0].buffer = (unsigned long) my_cam_app.hal_lib.mm_camera_do_mmap(
		pme->snapshot_buf.frame_len, &pme->snapshot_buf.frame[0].fd);
#endif
	if (!pme->snapshot_buf.frame[0].buffer) {
		CDBG("%s:no mem for snapshot buf\n", __func__);
		rc = -MM_CAMERA_E_NO_MEMORY;
		goto end;
	}
	pme->snapshot_buf.frame[0].path = OUTPUT_TYPE_S;
	pme->preview_buf.frame[0].y_off = 0;
	pme->snapshot_buf.frame[0].cbcr_off = planes_main[0];

	/*setup thumbnail buffer*/
	pme->thumbnail_buf.frame_len = my_cam_app.hal_lib.mm_camera_get_msm_frame_len(pme->dim.thumb_format,
				   CAMERA_MODE_2D, pme->dim.ui_thumbnail_width,
				  pme->dim.ui_thumbnail_height, OUTPUT_TYPE_T, &num_planes_thumbnail, planes_thumbnail);
#ifdef USE_ION
  	pme->thumbnail_buf.frame[0].ion_alloc.len = pme->thumbnail_buf.frame_len;
	pme->thumbnail_buf.frame[0].ion_alloc.flags = ION_FLAG_CACHED;
	pme->thumbnail_buf.frame[0].ion_alloc.heap_mask =
			(0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
	pme->thumbnail_buf.frame[0].ion_alloc.align = 4096;
	pme->thumbnail_buf.frame[0].buffer = (unsigned long) my_cam_app.hal_lib.mm_camera_do_mmap_ion(pme->ionfd,
                       &(pme->thumbnail_buf.frame[0].ion_alloc), &(pme->thumbnail_buf.frame[0].fd_data),
                       &pme->thumbnail_buf.frame[0].fd);
#else
	pme->thumbnail_buf.frame[0].buffer = (unsigned long)  my_cam_app.hal_lib.mm_camera_do_mmap(
		pme->thumbnail_buf.frame_len, &pme->thumbnail_buf.frame[0].fd);
#endif
	if (!pme->thumbnail_buf.frame[0].buffer) {
		CDBG("%s:no mem for thumbnail buf\n", __func__);
		rc = -MM_CAMERA_E_NO_MEMORY;
		goto end;
	}
	pme->thumbnail_buf.frame[0].path = OUTPUT_TYPE_T;
	pme->thumbnail_buf.frame[0].y_off = 0;
	pme->thumbnail_buf.frame[0].cbcr_off = planes_thumbnail[0];

	/*setup jpeg buffer*/
	pme->jpeg_buf.frame_len = pme->snapshot_buf.frame_len;
#ifdef USE_ION
  	pme->jpeg_buf.frame[0].ion_alloc.len = pme->snapshot_buf.frame_len;
	pme->jpeg_buf.frame[0].ion_alloc.flags = ION_FLAG_CACHED;
	pme->jpeg_buf.frame[0].ion_alloc.heap_mask =
			(0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
	pme->jpeg_buf.frame[0].ion_alloc.align = 4096;
	pme->jpeg_buf.frame[0].buffer = (unsigned long) my_cam_app.hal_lib.mm_camera_do_mmap_ion(pme->ionfd,
                       &(pme->jpeg_buf.frame[0].ion_alloc), &(pme->jpeg_buf.frame[0].fd_data),
                       &pme->jpeg_buf.frame[0].fd);
#else
	pme->jpeg_buf.frame[0].buffer = (unsigned long) my_cam_app.hal_lib.mm_camera_do_mmap(
		pme->jpeg_buf.frame_len, &pme->jpeg_buf.frame[0].fd);
#endif
	if (!pme->jpeg_buf.frame[0].buffer) {
		CDBG("%s:no mem for snapshot buf\n", __func__);
		rc = -MM_CAMERA_E_NO_MEMORY;
		goto end;
	}

	/*setup registration buffer*/
	reg_buf.snapshot.main.buf.mp[0].frame = pme->snapshot_buf.frame[0];
    reg_buf.snapshot.main.buf.mp[0].frame_offset = 0;
    reg_buf.snapshot.main.buf.mp[0].num_planes = num_planes_main;

    reg_buf.snapshot.main.buf.mp[0].planes[0].length = planes_main[0];
    reg_buf.snapshot.main.buf.mp[0].planes[0].m.userptr = pme->snapshot_buf.frame[0].fd;
    reg_buf.snapshot.main.buf.mp[0].planes[0].data_offset = 0;
    reg_buf.snapshot.main.buf.mp[0].planes[0].reserved[0] =
    reg_buf.snapshot.main.buf.mp[0].frame_offset;
	for (j = 1; j < num_planes_main; j++) {
      reg_buf.snapshot.main.buf.mp[0].planes[j].length = planes_main[j];
      reg_buf.snapshot.main.buf.mp[0].planes[j].m.userptr =
        pme->snapshot_buf.frame[0].fd;
	  reg_buf.snapshot.main.buf.mp[0].planes[j].data_offset = 0;
      reg_buf.snapshot.main.buf.mp[0].planes[j].reserved[0] =
      reg_buf.snapshot.main.buf.mp[0].planes[j-1].reserved[0] +
      reg_buf.snapshot.main.buf.mp[0].planes[j-1].length;
    }

	reg_buf.snapshot.thumbnail.buf.mp[0].frame = pme->thumbnail_buf.frame[0];
    reg_buf.snapshot.thumbnail.buf.mp[0].frame_offset = 0;
    reg_buf.snapshot.thumbnail.buf.mp[0].num_planes = num_planes_thumbnail;

    reg_buf.snapshot.thumbnail.buf.mp[0].planes[0].length = planes_thumbnail[0];
    reg_buf.snapshot.thumbnail.buf.mp[0].planes[0].m.userptr = pme->thumbnail_buf.frame[0].fd;
    reg_buf.snapshot.thumbnail.buf.mp[0].planes[0].data_offset = 0;
    reg_buf.snapshot.thumbnail.buf.mp[0].planes[0].reserved[0] =
    reg_buf.snapshot.thumbnail.buf.mp[0].frame_offset;
	for (j = 1; j < num_planes_thumbnail; j++) {
      reg_buf.snapshot.thumbnail.buf.mp[0].planes[j].length = planes_thumbnail[j];
      reg_buf.snapshot.thumbnail.buf.mp[0].planes[j].m.userptr =
        pme->thumbnail_buf.frame[0].fd;
	  reg_buf.snapshot.thumbnail.buf.mp[0].planes[j].data_offset = 0;
      reg_buf.snapshot.thumbnail.buf.mp[0].planes[j].reserved[0] =
      reg_buf.snapshot.thumbnail.buf.mp[0].planes[j-1].reserved[0] +
      reg_buf.snapshot.thumbnail.buf.mp[0].planes[j-1].length;
    }

	reg_buf.ch_type = MM_CAMERA_CH_SNAPSHOT;
	reg_buf.snapshot.main.num = pme->snapshot_buf.num;
	reg_buf.snapshot.thumbnail.num = pme->thumbnail_buf.num;

	rc = pme->cam->cfg->prepare_buf(pme->cam, &reg_buf);
	if(rc != MM_CAMERA_OK) {
		CDBG("%s:reg snapshot buf err=%d\n", __func__, rc);
		goto end;
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}
int mm_app_prepare_raw_snapshot_buf(int cam_id)
{
   int j;
   mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
   int rc = MM_CAMERA_OK;
   mm_camera_reg_buf_t reg_buf;
   uint32_t y_off, cbcr_off;
   uint8_t num_planes_main;
   uint32_t planes_main[VIDEO_MAX_PLANES];
   
   CDBG("%s: BEGIN, raw_w=%d, raw_h=%d\n",
          __func__, pme->dim.raw_picture_width, pme->dim.raw_picture_height);
   memset(&reg_buf,  0,  sizeof(reg_buf));
   reg_buf.def.buf.mp = malloc(sizeof(mm_camera_mp_buf_t));
   if (!reg_buf.def.buf.mp) {
     CDBG_ERROR("%s Error allocating memory for mplanar struct ", __func__);
     rc = -MM_CAMERA_E_NO_MEMORY; 
     goto end;
   }
   
   // setup main buffer
   memset(&pme->raw_snapshot_buf, 0, sizeof(pme->raw_snapshot_buf));
   pme->raw_snapshot_buf.num = 1;
   pme->raw_snapshot_buf.frame_len = 
         my_cam_app.hal_lib.mm_camera_get_msm_frame_len(CAMERA_BAYER_SBGGR10,
                                                        CAMERA_MODE_2D,
                                                        pme->dim.raw_picture_width,
                                                        pme->dim.raw_picture_height,
                                                        OUTPUT_TYPE_S,
                                                        &num_planes_main,
                                                        planes_main);
#ifdef USE_ION
   pme->raw_snapshot_buf.frame[0].ion_alloc.len = pme->raw_snapshot_buf.frame_len;
   pme->raw_snapshot_buf.frame[0].ion_alloc.flags = (0x1 << CAMERA_ION_HEAP_ID);
   pme->raw_snapshot_buf.frame[0].ion_alloc.align = 4096;
#endif
   pme->raw_snapshot_buf.frame[0].buffer = (unsigned long) my_cam_app.hal_lib.mm_camera_do_mmap(
      pme->raw_snapshot_buf.frame_len, &pme->raw_snapshot_buf.frame[0].fd);
   
   
   if (!pme->raw_snapshot_buf.frame[0].buffer) {
      CDBG("%s:no mem for snapshot buf\n", __func__);
      rc = -MM_CAMERA_E_NO_MEMORY;
      goto end;
   }
   pme->raw_snapshot_buf.frame[0].path = OUTPUT_TYPE_S;
   pme->preview_buf.frame[0].y_off = 0;
   pme->raw_snapshot_buf.frame[0].cbcr_off = planes_main[0];
   
   /*setup registration buffer*/
   reg_buf.def.buf.mp[0].frame = pme->raw_snapshot_buf.frame[0];
   reg_buf.def.buf.mp[0].frame_offset = 0;
   reg_buf.def.buf.mp[0].num_planes = num_planes_main;

   reg_buf.def.buf.mp[0].planes[0].length = planes_main[0];
   reg_buf.def.buf.mp[0].planes[0].m.userptr = pme->raw_snapshot_buf.frame[0].fd;
   reg_buf.def.buf.mp[0].planes[0].data_offset = 0;
   reg_buf.def.buf.mp[0].planes[0].reserved[0] = reg_buf.def.buf.mp[0].frame_offset;
   for (j = 1; j < num_planes_main; j++) {
      reg_buf.def.buf.mp[0].planes[j].length = planes_main[j];
      reg_buf.def.buf.mp[0].planes[j].m.userptr = pme->raw_snapshot_buf.frame[0].fd;
      reg_buf.def.buf.mp[0].planes[j].data_offset = 0;
      reg_buf.def.buf.mp[0].planes[j].reserved[0] = reg_buf.def.buf.mp[0].planes[j-1].reserved[0] +
                                                               reg_buf.def.buf.mp[0].planes[j-1].length;
    }
   
   reg_buf.ch_type = MM_CAMERA_CH_RAW;
   reg_buf.def.num = pme->raw_snapshot_buf.num;
   rc = pme->cam->cfg->prepare_buf(pme->cam, &reg_buf);
   if(rc != MM_CAMERA_OK) {
      CDBG("%s:reg snapshot buf err=%d\n", __func__, rc);
      goto end;
   }
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}
static int mm_app_unprepare_snapshot_buf(int cam_id)
{
	/* now we hard code format */
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int i, rc = MM_CAMERA_OK;
	CDBG("%s: BEGIN\n", __func__);
	rc = pme->cam->cfg->unprepare_buf(pme->cam, MM_CAMERA_CH_SNAPSHOT);
#ifdef USE_ION
    rc = my_cam_app.hal_lib.mm_camera_do_munmap_ion (pme->ionfd, &(pme->snapshot_buf.frame[0].fd_data),
                   (void *)pme->snapshot_buf.frame[0].buffer, pme->snapshot_buf.frame_len);
#else
	rc = my_cam_app.hal_lib.mm_camera_do_munmap(pme->snapshot_buf.frame[0].fd,
							 (void *)pme->snapshot_buf.frame[0].buffer,
							 pme->snapshot_buf.frame_len);
#endif
	if(rc != MM_CAMERA_OK) {
	  CDBG("%s: main image, mm_camera_do_munmap err, pmem_fd = %d, rc = %d",
		   __func__, pme->snapshot_buf.frame[0].fd, rc);
	  goto end;
	}
#ifdef USE_ION
    rc = my_cam_app.hal_lib.mm_camera_do_munmap_ion (pme->ionfd, &(pme->thumbnail_buf.frame[0].fd_data),
                   (void *)pme->thumbnail_buf.frame[0].buffer, pme->thumbnail_buf.frame_len);
#else
	rc = my_cam_app.hal_lib.mm_camera_do_munmap(pme->thumbnail_buf.frame[0].fd,
							 (void *)pme->thumbnail_buf.frame[0].buffer,
							pme->thumbnail_buf.frame_len);
#endif
#ifdef USE_ION
    rc = my_cam_app.hal_lib.mm_camera_do_munmap_ion (pme->ionfd, &(pme->jpeg_buf.frame[0].fd_data),
                   (void *)pme->jpeg_buf.frame[0].buffer, pme->jpeg_buf.frame_len);
#else
	rc = my_cam_app.hal_lib.mm_camera_do_munmap(pme->jpeg_buf.frame[0].fd,
							 (void *)pme->jpeg_buf.frame[0].buffer,
							pme->jpeg_buf.frame_len);
#endif
	if(rc != MM_CAMERA_OK) {
	  CDBG("%s: thumbnail image, mm_camera_do_munmap err, pmem_fd = %d, rc = %d",
		   __func__, pme->thumbnail_buf.frame[0].fd, rc);
	  goto end;
	}
	/* zero out the buf stuct */
	memset(&pme->snapshot_buf, 0, sizeof(pme->snapshot_buf));
	memset(&pme->thumbnail_buf, 0, sizeof(pme->thumbnail_buf));
	memset(&pme->jpeg_buf, 0, sizeof(pme->jpeg_buf));
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}
static int mm_app_unprepare_raw_snapshot_buf(int cam_id)
{
	/* now we hard code format */
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int i, rc = MM_CAMERA_OK;
	CDBG("%s: BEGIN\n", __func__);
	rc = pme->cam->cfg->unprepare_buf(pme->cam, MM_CAMERA_CH_RAW);
	rc = my_cam_app.hal_lib.mm_camera_do_munmap(pme->raw_snapshot_buf.frame[0].fd,
							 (void *)pme->raw_snapshot_buf.frame[0].buffer,
							 pme->raw_snapshot_buf.frame_len);
	rc = my_cam_app.hal_lib.mm_camera_do_munmap(pme->jpeg_buf.frame[0].fd,
							 (void *)pme->jpeg_buf.frame[0].buffer,
							pme->jpeg_buf.frame_len);
	/* zero out the buf stuct */
	memset(&pme->raw_snapshot_buf, 0, sizeof(pme->raw_snapshot_buf));
	memset(&pme->jpeg_buf, 0, sizeof(pme->jpeg_buf));
	
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

#ifndef DISABLE_JPEG_ENCODING
/* Once we give frame for encoding, we get encoded jpeg image
   fragments by fragment. We'll need to store them in a buffer
   to form complete JPEG image */
static void snapshot_jpeg_fragment_cb(uint8_t *ptr,
                                      uint32_t size,
                                      void *user_data)
{
	mm_camera_app_obj_t *pme = user_data;

    CDBG("%s: E",__func__);
	if (pme) {
		memcpy((uint8_t *)((uint32_t)pme->jpeg_buf.frame[0].buffer + JpegOffset), ptr, size);
		JpegOffset += size;
	}
    CDBG("%s: X",__func__);
}

/* This callback is received once the complete JPEG encoding is done */
static void snapshot_jpeg_cb(jpeg_event_t event, void *user_data)
{
	mm_camera_app_obj_t *pme = user_data;
    CDBG("%s: E ",__func__);

    if (pme) {
      mm_app_dump_jpeg_frame((void *)pme->jpeg_buf.frame[0].buffer, JpegOffset, (char *)"snapshot", (char *)"jpg", snapshot_cnt);
      //reset jpeg_offset
      JpegOffset = 0;
	}
    /* this will free up the resources used for previous encoding task */
    if(mCurrentFrameEncoded) {
        free(mCurrentFrameEncoded);
        mCurrentFrameEncoded = NULL;
    }
    CDBG("%s: Before omxJpegFinish", __func__);
    my_cam_app.hal_lib.omxJpegFinish();
    CDBG("%s: After omxJpegFinish", __func__);
	mm_app_snapshot_done();
    CDBG("%s: X",__func__);

}

static int encodeData(mm_camera_ch_data_buf_t* recvd_frame,
           int frame_len,
           int enqueued,
		   mm_camera_app_obj_t *pme)
{
    int ret = -1;
    cam_ctrl_dimension_t dimension;
    struct msm_frame *postviewframe;
    struct msm_frame *mainframe;
    common_crop_t crop;
    cam_point_t main_crop_offset;
    cam_point_t thumb_crop_offset;
    int width, height;
    uint8_t *thumbnail_buf;
    uint32_t thumbnail_fd;

    omx_jpeg_encode_params encode_params;
    postviewframe = recvd_frame->snapshot.thumbnail.frame;
    mainframe = recvd_frame->snapshot.main.frame;
    dimension.orig_picture_dx = pme->dim.picture_width;
    dimension.orig_picture_dy = pme->dim.picture_height;
    dimension.thumbnail_width = pme->dim.ui_thumbnail_width;
    dimension.thumbnail_height = pme->dim.ui_thumbnail_height;
	dimension.main_img_format = pme->dim.main_img_format;
	dimension.thumb_format = pme->dim.thumb_format;

    CDBG("Setting callbacks, initializing encoder and start encoding.");
    my_cam_app.hal_lib.set_callbacks(snapshot_jpeg_fragment_cb, snapshot_jpeg_cb, pme,
             (void *)pme->jpeg_buf.frame[0].buffer, &JpegOffset);
    my_cam_app.hal_lib.omxJpegStart();
    my_cam_app.hal_lib.mm_jpeg_encoder_setMainImageQuality(85);

    /*TBD: Pass 0 as cropinfo for now as v4l2 doesn't provide
      cropinfo. It'll be changed later.*/
    memset(&crop,0,sizeof(common_crop_t));
    memset(&main_crop_offset,0,sizeof(cam_point_t));
    memset(&thumb_crop_offset,0,sizeof(cam_point_t));

    /*Fill in the encode parameters*/
    encode_params.dimension = (const cam_ctrl_dimension_t *)&dimension;
    encode_params.thumbnail_buf = (uint8_t *)postviewframe->buffer;
    encode_params.thumbnail_fd = postviewframe->fd;
    encode_params.thumbnail_offset = postviewframe->phy_offset;
    encode_params.snapshot_buf = (uint8_t *)mainframe->buffer;
    encode_params.snapshot_fd = mainframe->fd;
    encode_params.snapshot_offset = mainframe->phy_offset;
    encode_params.scaling_params = &crop;
    encode_params.exif_data = NULL;
    encode_params.exif_numEntries = 0;
    encode_params.a_cbcroffset = -1;
    encode_params.main_crop_offset = &main_crop_offset;
    encode_params.thumb_crop_offset = &thumb_crop_offset;

    if (!my_cam_app.hal_lib.omxJpegEncode(&encode_params)){
        CDBG_ERROR("%s: Failure! JPEG encoder returned error.", __func__);
        ret = -1;
        goto end;
    }

    /* Save the pointer to the frame sent for encoding. we'll need it to
       tell kernel that we are done with the frame.*/
    mCurrentFrameEncoded = recvd_frame;

end:
    CDBG("%s: X", __func__);
    return ret;
}

static int encodeDisplayAndSave(mm_camera_ch_data_buf_t* recvd_frame,
                     int enqueued, mm_camera_app_obj_t *pme)
{
    int ret = -1;

	CDBG("%s: Send frame for encoding", __func__);
    ret = encodeData(recvd_frame, pme->snapshot_buf.frame_len,
                     enqueued, pme);
    if (!ret) {
        CDBG_ERROR("%s: Failure configuring JPEG encoder", __func__);
    }

    LOGD("%s: X", __func__);
    return ret;
}
#endif //DISABLE_JPEG_ENCODING
static void mm_app_snapshot_notify_cb(mm_camera_ch_data_buf_t *bufs,
	void *user_data)
{
	mm_camera_app_obj_t *pme = user_data;
	int rc;

	CDBG("%s: BEGIN\n", __func__);
	snapshot_cnt++;
	mm_app_dump_snapshot_frame(bufs->snapshot.main.frame, pme->snapshot_buf.frame_len, TRUE, 0);
	mm_app_dump_snapshot_frame(bufs->snapshot.thumbnail.frame, pme->thumbnail_buf.frame_len, FALSE, 0);
#ifndef DISABLE_JPEG_ENCODING
	/* The recvd_frame structre we receive from lower library is a local
    variable. So we'll need to save this structure so that we won't
    be later pointing to garbage data when that variable goes out of
    scope */
    mm_camera_ch_data_buf_t* frame =
      (mm_camera_ch_data_buf_t *)malloc(sizeof(mm_camera_ch_data_buf_t));
    if (frame == NULL) {
      CDBG_ERROR("%s: Error allocating memory to save received_frame structure.", __func__);
	  goto error1;
	}
    memcpy(frame, bufs, sizeof(mm_camera_ch_data_buf_t));
    rc = encodeDisplayAndSave(frame, 0, pme);
	if (!rc) {
      CDBG_ERROR("%s: Error encoding buffer.", __func__);
	  goto error;
	}
#endif //DISABLE_JPEG_ENCODING
	/* return buffer back for taking next snapshot */
	pme->cam->evt->buf_done(pme->cam, bufs);
    mm_app_snapshot_done();
/*
	CDBG("%s: calling mm_app_snapshot_done()\n", __func__);
	mm_app_snapshot_done();
*/
	CDBG("%s: END\n", __func__);
	return;
error:
  /*if (frame != NULL)
    free(frame);*/
error1:
  pme->cam->evt->buf_done(pme->cam, bufs);
  mm_app_snapshot_done();
  return;
}

static void mm_app_raw_snapshot_notify_cb(mm_camera_ch_data_buf_t *bufs,
	void *user_data)
{
	mm_camera_app_obj_t *pme = user_data;
	static int loop = 0;

	CDBG("%s: BEGIN\n", __func__);
	raw_snapshot_cnt++;
	mm_app_dump_snapshot_frame(bufs->def.frame, pme->raw_snapshot_buf.frame_len, TRUE, 1);
	/* return buffer back for taking next snapshot */
	pme->cam->evt->buf_done(pme->cam, bufs);
	CDBG("%s: calling mm_app_snapshot_done()\n", __func__);
	mm_app_snapshot_done();
	CDBG("%s: END\n", __func__);
}
static int mm_app_reg_snapshot_data_cb(int cam_id, int is_reg)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(is_reg) {
		rc = pme->cam->evt->register_buf_notify(pme->cam,
						 MM_CAMERA_CH_SNAPSHOT,
						 mm_app_snapshot_notify_cb,
                         MM_CAMERA_REG_BUF_CB_INFINITE, 0,
						 pme);
		if(rc != MM_CAMERA_OK) {
			CDBG("%s:register snapshot data notify cb err=%d\n",
			     __func__, rc);
			goto end;
		}
	} else {
		rc = pme->cam->evt->register_buf_notify(pme->cam,
					 MM_CAMERA_CH_SNAPSHOT,
					 NULL,
                                         (mm_camera_register_buf_cb_type_t)NULL,
					 0, pme);
		if(rc != MM_CAMERA_OK) {
			CDBG("%s:unregister snapshot data notify cb err=%d\n",
			     __func__, rc);
			goto end;
		}
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}
static int mm_app_reg_raw_snapshot_data_cb(int cam_id, int is_reg)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(is_reg) {
		rc = pme->cam->evt->register_buf_notify(pme->cam,
					 MM_CAMERA_CH_RAW,
					 mm_app_raw_snapshot_notify_cb,
                     MM_CAMERA_REG_BUF_CB_INFINITE, 0,
					 pme);
		if(rc != MM_CAMERA_OK) {
			CDBG("%s:register raw snapshot data notify cb err=%d\n",
                             __func__, rc);
			goto end;
		}
	} else {
		rc = pme->cam->evt->register_buf_notify(pme->cam,
				   MM_CAMERA_CH_RAW,
				   NULL,
                   (mm_camera_register_buf_cb_type_t)NULL, 0, pme);
		if(rc != MM_CAMERA_OK) {
			CDBG("%s:unregister raw snapshot data notify cb err=%d\n",
                             __func__, rc);
			goto end;
		}
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_start_snapshot(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	/*start OMX Jpeg encoder*/
#ifndef DISABLE_JPEG_ENCODING
	my_cam_app.hal_lib.omxJpegOpen();
#endif

	CDBG("%s:BEGIN\n", __func__);
	if(MM_CAMERA_OK != (rc = mm_app_set_op_mode(cam_id, MM_CAMERA_OP_MODE_CAPTURE))) {
		CDBG("%s:mm_app_set_op_mode(op_mode=%d) err=%d\n", __func__,
				 MM_CAMERA_OP_MODE_CAPTURE, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_open_ch(cam_id, MM_CAMERA_CH_SNAPSHOT))) {
		CDBG("%s:open snapshot channel err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_set_snapshot_fmt(cam_id))) {
		CDBG("%s:set video format err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_prepare_snapshot_buf(cam_id))) {
		CDBG("%s:treg video/preview buf err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_reg_snapshot_data_cb(cam_id, TRUE))) {
		CDBG("%s:reg video/preview data cb err=%d\n", __func__, rc);
	}
	CDBG("%s:MM_CAMERA_OPS_PREPARE_SNAPSHOT done\n", __func__);
	if(MM_CAMERA_OK != (rc = pme->cam->ops->action(pme->cam, TRUE, MM_CAMERA_OPS_SNAPSHOT, 0))) {
		CDBG("%s:snapshot streaming err=%d\n", __func__, rc);
		goto end;
	}
	CDBG("%s:MM_CAMERA_OPS_SNAPSHOT started\n", __func__);

end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}


int mm_app_stop_snapshot(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(MM_CAMERA_OK != (rc = pme->cam->ops->action(pme->cam, FALSE, MM_CAMERA_OPS_SNAPSHOT, 0))) {
		CDBG("%s:stop snapshot streaming err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_unprepare_snapshot_buf(cam_id))) {
		CDBG("%s:mm_app_unprepare_snapshot_buf err=%d\n", __func__, rc);
		return rc;
	}
	if(MM_CAMERA_OK != (rc = mm_app_reg_snapshot_data_cb(cam_id, FALSE))) {
		CDBG("%s:mm_app_reg_snapshot_data_cb err=%d\n", __func__, rc);
		return rc;
	}
	mm_app_close_ch(cam_id, MM_CAMERA_CH_SNAPSHOT);
#ifndef DISABLE_JPEG_ENCODING
	my_cam_app.hal_lib.omxJpegClose();
#endif
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_start_raw_snapshot(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	mm_camera_channel_attr_t attr;
	int rc = MM_CAMERA_OK;

	attr.type = MM_CAMERA_CH_ATTR_RAW_STREAMING_TYPE;
	attr.raw_streaming_mode = MM_CAMERA_RAW_STREAMING_CAPTURE_SINGLE;

	if(MM_CAMERA_OK != (rc = mm_app_set_op_mode(cam_id, MM_CAMERA_OP_MODE_CAPTURE))) {
		CDBG("%s:mm_app_set_op_mode(op_mode=%d) err=%d\n", __func__,
				 MM_CAMERA_OP_MODE_CAPTURE, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_open_ch(cam_id, MM_CAMERA_CH_RAW))) {
		CDBG("%s:open raw snapshot channel err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_set_raw_snapshot_fmt(cam_id))) {
		CDBG("%s:set raw snapshot format err=%d\n", __func__, rc);
		goto end;
	}
	mm_app_get_dim(cam_id, NULL);
	if(MM_CAMERA_OK != (rc = mm_app_prepare_raw_snapshot_buf(cam_id))) {
		CDBG("%s:reg raw snapshot buf err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_reg_raw_snapshot_data_cb(cam_id, TRUE))) {
		CDBG("%s:reg raw snapshot data cb err=%d\n", __func__, rc);
	}
	if(MM_CAMERA_OK != (rc = pme->cam->ops->ch_set_attr(pme->cam, MM_CAMERA_CH_RAW, &attr))) {
		CDBG("%s:set raw capture attribute err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = pme->cam->ops->action(pme->cam, TRUE, MM_CAMERA_OPS_RAW, 0))) {
		CDBG("%s:snapshot streaming err=%d\n", __func__, rc);
		goto end;
	}
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

int mm_app_stop_raw_snapshot(int cam_id)
{
	mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc = MM_CAMERA_OK;

	CDBG("%s: BEGIN\n", __func__);
	if(MM_CAMERA_OK != (rc = pme->cam->ops->action(pme->cam, FALSE, MM_CAMERA_OPS_RAW, 0))) {
		CDBG("%s:stop raw snapshot streaming err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_unprepare_raw_snapshot_buf(cam_id))) {
		CDBG("%s:mm_app_unprepare_raw_snapshot_buf err=%d\n", __func__, rc);
		return rc;
	}
	if(MM_CAMERA_OK != (rc = mm_app_reg_raw_snapshot_data_cb(cam_id, FALSE))) {
		CDBG("%s:mm_app_reg_raw_snapshot_data_cb err=%d\n", __func__, rc);
		return rc;
	}
	mm_app_close_ch(cam_id, MM_CAMERA_CH_RAW);
end:
	CDBG("%s: END, rc=%d\n", __func__, rc);
	return rc;
}

static void mm_app_snapshot_wait(int cam_id)
{
	pthread_mutex_lock(&g_s_mutex);
	if(FALSE == g_status) pthread_cond_wait(&g_s_cond_v, &g_s_mutex);
	pthread_mutex_unlock(&g_s_mutex);
}

int mm_app_take_picture(int cam_id)
{
    mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc;
	CDBG("%s:BEGIN, cam_id=%d\n",__func__,cam_id);
	g_status = FALSE;
	if(MM_CAMERA_OK != (rc = pme->cam->ops->action(pme->cam, TRUE, MM_CAMERA_OPS_PREPARE_SNAPSHOT, 0))) {
		CDBG("%s:prepare snapshot err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_stop_preview(cam_id))) {
	  CDBG("%s:mm_app_stop_preview err=%d\n", __func__, rc);
	  goto end;
	}
	if(MM_CAMERA_OK != mm_app_start_snapshot(cam_id))
	  goto preview;
	CDBG("%s:waiting images\n",__func__);
	mm_app_snapshot_wait(cam_id);
	CDBG("%s:calling mm_app_stop_snapshot() \n",__func__);
	mm_app_stop_snapshot(cam_id);
preview:
    mm_app_start_preview(cam_id);
end:
	CDBG("%s:END, cam_id=%d\n",__func__,cam_id);
	return rc;
}

int mm_app_take_raw_picture(int cam_id)
{
    mm_camera_app_obj_t *pme = mm_app_get_cam_obj(cam_id);
	int rc;
	CDBG("%s:BEGIN, cam_id=%d\n",__func__,cam_id);
	g_status = FALSE;
	if(MM_CAMERA_OK != (rc = pme->cam->ops->action(pme->cam, TRUE, MM_CAMERA_OPS_PREPARE_SNAPSHOT, 0))) {
		CDBG("%s:prepare snapshot err=%d\n", __func__, rc);
		goto end;
	}
	if(MM_CAMERA_OK != (rc = mm_app_stop_preview(cam_id))) {
	  CDBG("%s:mm_app_stop_preview err=%d\n", __func__, rc);
	  goto end;
	}
	if(MM_CAMERA_OK != mm_app_start_raw_snapshot(cam_id))
	  goto preview;
	CDBG("%s:waiting images\n",__func__);
	mm_app_snapshot_wait(cam_id);
	CDBG("%s:calling mm_app_stop_snapshot() \n",__func__);
	mm_app_stop_raw_snapshot(cam_id);
preview:
    mm_app_start_preview(cam_id);
end:
	CDBG("%s:END, cam_id=%d\n",__func__,cam_id);
	return rc;
}

