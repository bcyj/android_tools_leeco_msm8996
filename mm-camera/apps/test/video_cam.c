/*============================================================================

   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ioctl.h>
struct file;
struct inode;
#include <linux/android_pmem.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>

#include <linux/fb.h>
#include <linux/videodev2.h>

#include <pthread.h>

#include "cam_fifo.h"
#include "camera.h"
#include "camera_dbg.h"
#include "cam_mmap.h"

#define DBG_DUMP_YUV_FRAME  0

static int frameCnt = 0;
static common_crop_t *crop;
extern int ZoomDump;
extern cam_ctrl_dimension_t *dimension;

extern void test_app_mmcamera_videoframe_return(struct msm_frame * p);

static struct fifo_queue g_busy_videoframe_queue =
{0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, "video_busy_q"};

/*===========================================================================
FUNCTION      cam_frame_post_video

DESCRIPTION    this function add a busy video frame to the busy queue tails
===========================================================================*/
static void cam_frame_post_video (struct msm_frame *p)
{
  add_frame(&g_busy_videoframe_queue, p);
  signal_queue(&g_busy_videoframe_queue);
}

/*===========================================================================
FUNCTION      cam_frame_get_video

DESCRIPTION    this function returns a video frame from the head
===========================================================================*/
static struct msm_frame *cam_frame_get_video() {
  common_crop_t *loc_crop;
  CDBG("!!cam_frame_get_video... in\n");
  struct msm_frame *p = get_frame(&g_busy_videoframe_queue);
  if (p) {
    loc_crop = (common_crop_t *) (p->cropinfo);

    CDBG("in2_w = %d \n",loc_crop->in2_w);
    CDBG("in2_h = %d \n",loc_crop->in2_h);
    CDBG("in1_w = %d \n",loc_crop->in1_w);
    CDBG("in1_h = %d \n",loc_crop->in1_h);
    CDBG("!!cam_frame_get_video... out = %lu\n", p->buffer);
  } else {
    CDBG_ERROR("%s Got NULL frame from videoframe_queue", __func__);
  }

  return p;
}

/*===========================================================================
 * FUNCTION     video_frame
 * DESCRIPTION  video thread
 *==========================================================================*/
static int videoframe_exit = 0;
static pthread_t video_frame_thread, video_enc_thread;

static void *video_enc_delay(void *data)
{
  uint32_t check = FALSE;
  unsigned long write_buf = 0;
  struct msm_frame *newFrame = (struct msm_frame *)data;
  CDBG("!!video_enc_delay... in = %lu\n", newFrame->buffer);

  if (newFrame) {
    common_crop_t *loc_crop = (common_crop_t *) (newFrame->cropinfo);

    if ((crop->in2_w != loc_crop->in2_w) ||
      (crop->in2_h != loc_crop->in2_h) ||
      (crop->in1_w != loc_crop->in1_w) ||
      (crop->in1_h != loc_crop->in1_h) ||
      (crop->out2_w != loc_crop->out2_w) ||
      (crop->out2_h != loc_crop->out2_h) ||
      (crop->out1_w != loc_crop->out1_w) ||
      (crop->out1_h != loc_crop->out1_h)) {

      check = TRUE;
      *crop = *loc_crop;
    } else
      check = FALSE;

    CDBG("DUMP VIDEO FRAME");
#if DBG_DUMP_YUV_FRAME
    CDBG("Recording Width = %d, Height = %d\n",
         dimension->orig_video_width, dimension->orig_video_height);
    char buf[128];

    if (ZoomDump) {
      ZoomDump = 0;
      sprintf(buf, "/data/%d_v_%d_z.yuv", frameCnt,loc_crop->in2_w);

      cam_dump_yuv((void *)newFrame->buffer, dimension->orig_video_width *
                   dimension->orig_video_height, buf);
    } else if (frameCnt >= 31 && frameCnt < 34) {
#define TOP_DOWN_FULL 0
#if TOP_DOWN_FULL
      uint32_t w, h;
      w = dimension->orig_video_width;
      h = dimension->orig_video_height;

      sprintf(buf, "/data/%d_v_%d_Left.yuv", frameCnt,loc_crop->in2_w);
      cam_dump_yuv2((void *)newFrame->buffer,
                    (void *)newFrame->buffer + (w * h * 2), (w * h), buf);

      sprintf(buf, "/data/%d_v_%d_Right.yuv", frameCnt,loc_crop->in2_w);
      cam_dump_yuv2((void *)newFrame->buffer + (w * h),
                    (void *)newFrame->buffer + (w * h * 2) + ((w * h) / 2),
                    (w * h), buf);
#else
      uint32_t w, h;
      w = dimension->orig_video_width/2;
      h = dimension->orig_video_height;

      sprintf(buf, "/data/%d_v_%d_Full.yuv", frameCnt,loc_crop->in2_w);
      cam_dump_yuv((void *)newFrame->buffer, (w * h * 2), buf);
#endif
    }
    frameCnt++;
#endif

  } else {
    CDBG_ERROR("%s Input frame is NULL ", __func__);
  }
  CDBG("!!video_enc_delay... out = %lu\n", newFrame->buffer);
  return 0;
}

static void *videoframe(void *data)
{
  struct msm_frame *pf = 0;
  CDBG("!!videoframe thread started\n");

  while (!videoframe_exit) {
    CDBG("!!videoframe thread waiting\n");

    wait_queue(&g_busy_videoframe_queue);
    // get a frame from the busy queue
    CDBG("!!videoframe before cam_frame_get_video\n");

    if (videoframe_exit) {
      break;
    }
    pf = cam_frame_get_video ();
    CDBG("!!videoframe after cam_frame_get_video\n");
    if (!pf) {
      CDBG("!!videoframe after cam_frame_get_video: NULL\n");
      continue;
    } else {
      // simulate a delay of encoder
      video_enc_delay(pf);

      // then return the the frame (to free queue)
      test_app_mmcamera_videoframe_return (pf);
    }
  }

  return 0;
}

/*===========================================================================
 * FUNCTION     test_app_launch_video_frame_thread
 * DESCRIPTION  entry point for main to launch the video frame
 *==========================================================================*/
int test_app_launch_video_frame_thread(void* parms)
{
  videoframe_exit = 0;

  pthread_create(&video_frame_thread, NULL, videoframe, parms);
  CDBG("alloc crop \n");
  crop = malloc(sizeof(common_crop_t));
  if (!crop) {
    CDBG("test_app_launch_video_frame_thread malloc failed\n");
    return 0;
  }
  memset(crop, 0, sizeof(common_crop_t));

  return 1;
}

/*===========================================================================
 * FUNCTION     test_app_release_video_frame_thread
 * DESCRIPTION  entry point for main to launch the video frame
 *==========================================================================*/
int test_app_release_video_frame_thread(void)
{
  videoframe_exit = 1;

  CDBG("Inside test_app_release_video_frame_thread \n");

  /* signal the video thread , and check in video thread if stop is called,
  if so exit video thread */
  signal_queue(&g_busy_videoframe_queue);

  /* by this time, camframe thread is already closed,
  and its safe to flush the busy queue, no more video frames will be added
  as camframe thread is closed */
  flush_queue(&g_busy_videoframe_queue);

  CDBG("test_app_release_video_frame_thread done\n");
  return 1;
}


/*===========================================================================
 * FUNCTION     test_app_mmcamera_videoframe_callback
 * DESCRIPTION  entry point for cam_frame thread to deliver a busy frame
 *==========================================================================*/
void test_app_mmcamera_videoframe_callback(struct msm_frame *frame)
{
  CDBG("!!testapp_mmcamera_videoframe_callback\n");
  // post busy frame
  if (frame) {
    cam_frame_post_video (frame);
  }
}

void test_receive_shutter_callback(common_crop_t *crop)
{
  CDBG("%s: \n", __func__);
}

void test_app_release_video()
{
  if (crop) {
    CDBG("free crop \n");
    free(crop);
    crop = NULL;
  }
}
