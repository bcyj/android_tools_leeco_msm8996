/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
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
#include <poll.h>

#ifdef _ANDROID_
#define LIKELY(exp)   __builtin_expect(!!(exp), 1)
#define UNLIKELY(exp) __builtin_expect(!!(exp), 0)
#else
#define LIKELY(exp)   exp
#define UNLIKELY(exp) exp
#endif
#include "camera.h"
#include "camera_dbg.h"
#include "mm_camera_interface.h"
#include "cam_mmap.h"
#ifndef DISABLE_JPEG_ENCODING
#include "jpeg_encoder.h"
#endif /* DISABLE_JPEG_ENCODING */

#define MAX_PREVIEW_FRAMES_NUM  6
#define MAX_VIDEO_FRAMES_NUM  9
#define DBG_DUMP_YUV_FRAME 0
#define BUFF_SIZE_128 128
#define BUFF_SIZE_256 256

static int terminatefd[2];
static int camframe_exit;
static int camframe_v4l2_exit;
int camerafd1 = 0;
static int display_fd = -1;
static struct msm_frame videoFrame[MAX_VIDEO_FRAMES_NUM];
static struct msm_frame previewFrame[MAX_PREVIEW_FRAMES_NUM];
static int videoIndex = 0;
static int previewIndex = 0;

struct fifo_queue g_free_video_frame_queue =
{0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, "video_free_q"};
struct fifo_queue g_free_preview_frame_queue =
{0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, "preview_free_q"};

static int is_camframe_thread_ready;
static int is_camframe_v4l2_thread_ready;

static pthread_cond_t  sub_thread_ready_cond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t sub_thread_ready_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t frame_thread;
static struct timeval td1, td2, tdv1, tdv2;
static struct timezone tz;

unsigned long preview_frames_buf = 0;
extern cam_3d_frame_format_t video_format_3d;
extern camera_mode_t mode;
int8_t dump_restart_flag = false;

pthread_mutex_t jpeg_encoder_ready_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *jpege_fout = NULL;

extern void set_liveshot_frame(struct msm_frame* liveshot_frame);

static int32_t (*eztune_preview_callback)(const char *, uint32_t) = NULL;

/*===========================================================================
 *  * FUNCTION     camframe_add_frame
 *  * DESCRIPTION
 *==========================================================================*/
int8_t camframe_add_frame(cam_frame_type_t type, struct msm_frame* p_frame)
{
  struct fifo_queue* queue = NULL;
  switch(type) {
  case CAM_VIDEO_FRAME:
    queue = &g_free_video_frame_queue;
    break;
  case CAM_PREVIEW_FRAME:
    queue = &g_free_preview_frame_queue;
    break;
  default:
    return FALSE;
  }
  return add_frame(queue, p_frame);
}

/*===========================================================================
 *  * FUNCTION     camframe_release_all_frames
 *  * DESCRIPTION
 *==========================================================================*/
int8_t camframe_release_all_frames(cam_frame_type_t type)
{
  struct fifo_queue* queue = NULL;
  switch(type) {
  case CAM_VIDEO_FRAME:
    queue = &g_free_video_frame_queue;
    break;
  case CAM_PREVIEW_FRAME:
    queue = &g_free_preview_frame_queue;
    break;
  default:
    return FALSE;
  }
  flush_queue(queue);
  return TRUE;
}

/*===========================================================================
 *  * FUNCTION     calc_fps
 *   * DESCRIPTION
 *==========================================================================*/
void calc_fps(int is_video_frame)
{
  if (is_video_frame) {
    gettimeofday(&tdv2, &tz);
    int tmp =
      1000000 / ((tdv2.tv_sec - tdv1.tv_sec) * 1000000 + (tdv2.tv_usec -
      tdv1.tv_usec));
    CDBG("cam_frame Profiling: video frame rate is %f\n",
      (1000000.0 / ((tdv2.tv_sec - tdv1.tv_sec) * 1000000 + (tdv2.tv_usec -
      tdv1.tv_usec))));
    tdv1 = tdv2;
  } else {
    gettimeofday(&td2, &tz);
    int tmp =
      1000000 / ((td2.tv_sec - td1.tv_sec) * 1000000 + (td2.tv_usec -
      td1.tv_usec));
    CDBG("cam_frame Profiling: preview frame rate is %f\n",
      (1000000.0 / ((td2.tv_sec - td1.tv_sec) * 1000000 + (td2.tv_usec -
      td1.tv_usec))));
    td1 = td2;
  }
}

/*===========================================================================
 *  * FUNCTION     dump_frame
 *   * DESCRIPTION
 *==========================================================================*/
#ifdef _ANDROID_
#include <cutils/properties.h>
void dump_frame(struct msm_frame* newFrame, int is_video)
{
  static int frameCnt = 0;
  static int skip_cnt = 0;
  int32_t enabled = 0;
  int frm_num;
  uint32_t  skip_mode;
  char value[PROPERTY_VALUE_MAX];
  char buf[BUFF_SIZE_128];
  property_get("persist.camera.dumpimg", value, "0");
  enabled = atoi(value);

  frm_num = ((enabled & 0xffff0000) >> 16);
  if(frm_num == 0) frm_num = 10; /*default 10 frames*/
  if(frm_num > 256) frm_num = 256; /*256 buffers cycle around*/
  skip_mode = ((enabled & 0x0000ff00) >> 8);
  if(skip_mode == 0) skip_mode = 1; /*no -skip */
  enabled = (enabled & 0x000000ff);
  if (dump_restart_flag) {
    dump_restart_flag = false;
    frameCnt = 0;
  }

  if(enabled == 3 /*both preview & video*/ ||
     (enabled == 1 && !is_video) /*preview only */ ||
     (enabled == 2 && is_video) /*video only */ ) {
    if( skip_cnt % skip_mode == 0) {
      if (frameCnt >= 0 && frameCnt <= frm_num) {
        if(is_video) {
          snprintf(buf, BUFF_SIZE_128, "/data/misc/camera/%d_v.yuv", frameCnt);
        } else {
          snprintf(buf, BUFF_SIZE_128, "/data/misc/camera/%d.yuv", frameCnt);
        }
        int file_fd = open(buf, O_RDWR | O_CREAT, 0777);

        if (file_fd < 0) {
          CDBG("%s: cannot open file\n", __func__);
        }

        write(file_fd, (const void *)newFrame->buffer,
          newFrame->planar1_off * 3 / 2);
        close(file_fd);
        CDBG_ERROR("Dump %s", buf);
      } else if(frm_num == 256){
        frameCnt = 0;
  }
  frameCnt++;
}
    skip_cnt++;
  }  else {
    frameCnt = 0;
  }
}
#endif
/*===========================================================================
 *  * FUNCTION     launch_camframe_thread
 *   * DESCRIPTION
 *==========================================================================*/
int launch_camframe_thread(cam_frame_start_parms* parms)
{
  camframe_exit = 0;
  is_camframe_thread_ready = 0;

  //pthread_create(&frame_thread, NULL, cam_frame, frame_holding);
  pthread_create(&frame_thread, NULL, cam_frame, (void *)parms);

  /* Waiting for launching sub thread ready signal. */
  CDBG("launch_camframe_thread(), call pthread_cond_wait\n");

  pthread_mutex_lock(&sub_thread_ready_mutex);
  if (!is_camframe_thread_ready) {
    pthread_cond_wait(&sub_thread_ready_cond, &sub_thread_ready_mutex);
  }
  pthread_mutex_unlock(&sub_thread_ready_mutex);

  CDBG("launch_camframe_thread(), call pthread_cond_wait done cam_exit %d \n",camframe_exit);
  return camframe_exit;
}

/*===========================================================================
 *  * FUNCTION     release_camframe_thread
 *   * DESCRIPTION
 *==========================================================================*/
void release_camframe_thread(void)
{
  camframe_terminate();

  CDBG("pthread_join(frame_thread, NULL)\n");
  if (pthread_join(frame_thread, NULL) != 0) {
    CDBG("frame_thread exit failure!\n");
  }
  CDBG("pthread_join succeeded on frame_thread, \n");
}

/*===========================================================================
 *  * FUNCTION     camframe_thread_ready_signal
 *   * DESCRIPTION
 *==========================================================================*/
void camframe_thread_ready_signal(void)
{
  /*
   * Send signal to control thread to indicate that camframe thread is
   * ready.
   */
  CDBG("cam_frame() is ready, call pthread_cond_signal\n");

  pthread_mutex_lock(&sub_thread_ready_mutex);
  is_camframe_thread_ready = 1;
  pthread_cond_signal(&sub_thread_ready_cond);
  pthread_mutex_unlock(&sub_thread_ready_mutex);

  CDBG("cam_frame() is ready, call pthread_cond_signal done\n");
}

/* camframe_terminate shall be replaced by camframe_release. The latter provides
 * threads sync mechanism
 */
/*===========================================================================
 *  * FUNCTION     camframe_terminate
 *   * DESCRIPTION
 *==========================================================================*/
void camframe_terminate(void)
{
  int rc;
  char end = 'y';

  CDBG("In camframe_terminate\n");

  rc = ioctl(camerafd1, MSM_CAM_IOCTL_UNBLOCK_POLL_FRAME);
  if (rc < 0) {
    CDBG("MSM_CAM_IOCTL_UNBLOCK_POLL_FRAME failed (%s), frame thread may not exit.",
      strerror(errno));
  }

  camframe_exit = 1;
  rc = write(terminatefd[1],&end,sizeof(end));

  if (rc <0)
    CDBG("camframe termination failed : Failed\n");
}

void *cam_frame_v4l2(void *data);
pthread_t frame_v4l2_thread;
int launch_camframe_v4l2_thread(cam_frame_v4l2_start_parms* parms)
{
  camframe_v4l2_exit = 0;
  is_camframe_v4l2_thread_ready = 0;

  pthread_create(&frame_v4l2_thread, NULL, cam_frame_v4l2, (void *)parms);

  /* Waiting for launching sub thread ready signal. */
  CDBG("launch_camframe_v4l2_thread(), call pthread_cond_wait\n");

  pthread_mutex_lock(&sub_thread_ready_mutex);
  if (!is_camframe_v4l2_thread_ready) {
    pthread_cond_wait(&sub_thread_ready_cond, &sub_thread_ready_mutex);
  }
  pthread_mutex_unlock(&sub_thread_ready_mutex);

  CDBG("launch_camframe_v4l2_thread(), pthread_cond_wait done cam_exit %d \n",
    camframe_v4l2_exit);
  return camframe_v4l2_exit;
}

void release_camframe_v4l2_thread(void)
{
  camframe_v4l2_exit = 1;
    CDBG("pthread_join(frame_v4l2_thread, NULL)\n");

  if (pthread_join(frame_v4l2_thread, NULL) != 0) {
    CDBG("frame_v4l2_thread exit failure!\n");
  }
  CDBG("pthread_join succeeded on frame_v4l2_thread, \n");
}

void wait_cam_frame_thread_ready()
{
  is_camframe_thread_ready = 0;
  pthread_mutex_lock(&sub_thread_ready_mutex);
  CDBG_ERROR("Waiting for frame thread to start ! \n");
  if (!is_camframe_thread_ready) {
    pthread_cond_wait(&sub_thread_ready_cond, &sub_thread_ready_mutex);
 }
  CDBG_ERROR("Wait over, frame thread ready !!!! \n");
  pthread_mutex_unlock(&sub_thread_ready_mutex);
}

void camframe_v4l2_thread_ready_signal(void)
{
  /*
   * Send signal to control thread to indicate that camframe thread is
   * ready.
   */
  CDBG("cam_frame_v4l2() is ready, call pthread_cond_signal\n");

  pthread_mutex_lock(&sub_thread_ready_mutex);
  is_camframe_v4l2_thread_ready = 1;
  pthread_cond_signal(&sub_thread_ready_cond);
  pthread_mutex_unlock(&sub_thread_ready_mutex);

  CDBG("cam_frame_v4l2() is ready, call pthread_cond_signal done\n");
}

/*===========================================================================
 * FUNCTION     cam_frame
 * DESCRIPTION  Frame thread
 *==========================================================================*/
void *cam_frame(void *data)
{
  fd_set fds;
  int retval;
  int ioctlRetVal;
  int nfds = 0;
  common_crop_t cropinfo;
  int is_video_frame = 1;

  cam_frame_start_parms* parms = (cam_frame_start_parms*)data;

  struct msm_frame newFrame;
  struct msm_frame* cbFrame = NULL;
  videoIndex = previewIndex = 0;

  struct msm_frame currentFrame;
  struct msm_frame currentVideoFrame;
  struct fd_roi_t *p_fd_roi, fd_roi;

  memset(&cropinfo, 0, sizeof(common_crop_t));

  /* Need TODO:
   * 1. open the device file;
   * 2. select on the new fd and ioctl */
  /* open device driver /dev/msm_camera */
  if (pipe(terminatefd)< 0) {
    CDBG("cam_frame : *****************  pipe creation failed\n");
    return NULL;
  }

  char device[MAX_DEV_NAME_LEN];
  snprintf(device, MAX_DEV_NAME_LEN, MSM_CAMERA_FRAME, get_device_id());
  camerafd1 = open(device, O_RDWR);
  if (camerafd1 < 0) {
    camframe_exit = -1;
    camframe_thread_ready_signal();
    return NULL;
  }

  camframe_thread_ready_signal();

  newFrame.cropinfo = &cropinfo;
  newFrame.croplen = sizeof(common_crop_t);
  newFrame.roi_info.info = (void *)&(fd_roi);
  /* stereo quality */

  gettimeofday(&td1, &tz);
  gettimeofday(&tdv1, &tz);

  CDBG("%s : start frameloop\n", __func__);
  do {
    struct timeval timeout;
    timeout.tv_usec = 0;
    timeout.tv_sec = 6;
    FD_ZERO(&fds);
    FD_SET(terminatefd[0], &fds);
    FD_SET(camerafd1, &fds);

    nfds = (((camerafd1) > (terminatefd[0])) ? (camerafd1) : (terminatefd[0]));
    retval = select(nfds + 1, &fds, NULL, NULL, &timeout);
    if (retval == 0) {
      if (camframe_exit != 0) {
        CDBG("cam_frame: exit 1\n");
        break;
      }
      if (get_notify_obj()->on_error_event) {
        get_notify_obj()->on_error_event(CAMERA_ERROR_TIMEOUT);
      }
    } else if (retval < 0) {
      CDBG("SELECT ERROR %s \n", strerror(errno));
      if (camframe_exit != 0) break;
      usleep(1000 * 10);
      continue;
    } else if (retval) {
      if (camframe_exit != 0) break;

      if (FD_ISSET(camerafd1, &fds)) {
        ioctlRetVal = ioctl(camerafd1, MSM_CAM_IOCTL_GETFRAME, &newFrame);
        if (ioctlRetVal < 0) {
          CDBG
            ("cam_frame: MSM_CAM_IOCTL_GETFRAME ioctl failed. ioctl return value is %d \n",
            ioctlRetVal);
          if (camframe_exit != 0) break;
          usleep(1000 * 10);
          continue;
        }

        if (newFrame.error_code) {
          CDBG("%s: errornous frame. error_code = %d\n", __func__,
               newFrame.error_code);
          if (get_notify_obj()->on_error_event) {
            get_notify_obj()->on_error_event(CAMERA_ERROR_ESD);
          }
          continue;
        }

        if (camframe_exit != 0) break;

        CDBG("%s: newFrame.path  = %d \n",__func__, newFrame.path);

        if (newFrame.path & OUTPUT_TYPE_V) {
          if (newFrame.path & OUTPUT_TYPE_L) {
              set_liveshot_frame(&newFrame);
          }
          is_video_frame = 1;
          CDBG ("!!!cam_frame: a video frame \n");
        } else
          is_video_frame = 0;

        CDBG
          ("cam_frame: MSM_CAM_IOCTL_GETFRAME  buffer=%lu fd=%d y_off=%d cbcr_off=%d \n",
          (unsigned long)newFrame.buffer, newFrame.fd, newFrame.planar0_off,
          newFrame.planar1_off);

#if DBG_DUMP_YUV_FRAME
#ifdef _ANDROID_
        dump_frame(&newFrame, is_video_frame);
#endif /*_ANDROID_*/
#endif /*DBG_DUMP_YUV_FRAME*/

#ifdef ENABLE_DISPLAY
        draw_rectangles(&newFrame);
#endif

#if DBG_PERFORMANCE_FPS
        calc_fps(is_video_frame);
#endif

        if (is_video_frame) {
          struct msm_frame *tmp = get_frame(&g_free_video_frame_queue);
          if (tmp) {
            currentVideoFrame = *tmp;
            videoFrame[videoIndex] = newFrame;
            cbFrame = &videoFrame[videoIndex];
            videoIndex = (videoIndex+1)%MAX_VIDEO_FRAMES_NUM;
          } else {
            currentVideoFrame = newFrame;
            cbFrame = NULL;
            CDBG("%s: Video frame dropped", __func__);
          }
        } else { /* preview frame */
          struct msm_frame *tmp;
          if (parms->cammode == CAMERA_MODE_3D)
            tmp = NULL;
          else
            tmp = get_frame(&g_free_preview_frame_queue);

          if (tmp) {
            currentFrame = *tmp;
            previewFrame[previewIndex] = newFrame;
            cbFrame = &previewFrame[previewIndex];
            previewIndex = (previewIndex+1)%MAX_PREVIEW_FRAMES_NUM;
          } else {
            currentFrame = newFrame;
            cbFrame = NULL;
            CDBG("%s: Preview frame dropped", __func__);
          }
          if(UNLIKELY(eztune_preview_callback)) {
            (*eztune_preview_callback)((const void *)newFrame.buffer,
                   newFrame.planar1_off * 3 / 2);
          }
        }

        if ((ioctlRetVal =
          ioctl(camerafd1, MSM_CAM_IOCTL_RELEASE_FRAME_BUFFER,
          is_video_frame ? &currentVideoFrame : &currentFrame)) < 0) {
          CDBG
            ("cam_frame: MSM_CAM_IOCTL_RELEASE_FRAME_BUFFER ioctl failed. ioctl return value is %d \n",
            ioctlRetVal);
          CDBG("cam_frame: release frame failed buffer=%lu fd=%d y_off=%d cbcr_off=%d \n",
            (unsigned long)currentVideoFrame.buffer, currentVideoFrame.fd,
            currentVideoFrame.planar0_off, currentVideoFrame.planar1_off);
          break;
        }

        if (camframe_exit != 0) break;

        if (parms->cammode == CAMERA_MODE_3D)
          CDBG("%s: 3D Convergence Value for this frame = %d\n", __func__,
            cbFrame->stcam_conv_value);

        if (is_video_frame && cbFrame) {
          if (LIKELY(get_notify_obj()->video_frame_cb))
            get_notify_obj()->video_frame_cb(cbFrame);
          else
            camframe_add_frame(CAM_VIDEO_FRAME, cbFrame);
        } else if (cbFrame) { /* preview */
          if (LIKELY(get_notify_obj()->preview_frame_cb))
            get_notify_obj()->preview_frame_cb(cbFrame);
          else
            camframe_add_frame(CAM_PREVIEW_FRAME, cbFrame);
        }
      }/*if (FD_ISSET(camerafd1, &fds)) */
    } /*else if (retval) */
  } while (camframe_exit == 0);

  done2:
  close(camerafd1);
  close(terminatefd[0]);
  close(terminatefd[1]);
  CDBG("Exiting camframe thread\n - %d",camerafd1);
  return NULL;
}
void *cam_frame_set_exit_flag(int flag) {
   CDBG("__func__ Setting camfram_exit to 0");
   camframe_exit = flag;
   is_camframe_thread_ready = flag;
   return NULL;
}

/*===========================================================================
 * FUNCTION     cam_frame_v4l2
 * DESCRIPTION  V4L2 Frame thread
 *==========================================================================*/
void *cam_frame_v4l2(void *data)
{
  int retval;
  int ioctlRetVal;
  struct pollfd fds;
  int timeoutms;
  struct pollfd fds2[2];

  cam_frame_v4l2_start_parms* parms = (cam_frame_v4l2_start_parms*)data;
  struct v4l2_frame_buffer *frames_pointer_p = &parms->framesP[0];
  struct v4l2_frame_buffer *frames_pointer_v = &parms->framesV[0];
  int camfds[2];
  int camfdP = parms->camfdP;
  int camfdV = parms->camfdV;
  int numFramesP = parms->numFramesP;
  int numFramesV = parms->numFramesV;
  struct v4l2_buffer bufInUseP;
  struct v4l2_buffer bufInUseV;
  struct v4l2_buffer bufToPutP = frames_pointer_p[numFramesP-1].buffer;
  struct v4l2_buffer bufToPutV = frames_pointer_v[numFramesV-1].buffer;
  struct v4l2_crop *crop = parms->crop;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
  cam_format_t image_format = parms->image_format;

  static unsigned int vcount = 0;
  static int frameCnt = 0;
  unsigned long display_buf = 0;
  int main_ion_fd;
  struct ion_allocation_data display_alloc;
  struct ion_fd_data display_fd_data;
  static int nLoop = 0;
  int frameCntp = 0;
  struct timeval td1, td2, tdv1, tdv2;
  struct timezone tz;
  int koff = 0;
  int buff_size = 0;
  camframe_v4l2_exit = 0;
  errno = 0;
  nLoop++;
  bufInUseP.index = 0;
  bufInUseV.index = 0;

  /* Need TODO:
   * 1. open the device file;
   * 2. select on the new fd and ioctl */
  /* open device driver /dev/msm_camera */
  if(pipe(terminatefd)< 0) {
    CDBG("cam_frame : *****************  pipe creation failed\n");
    return NULL;
  }

  CDBG("cam_frame_v4l2: allocating display buffer memory\n");
  if (frames_pointer_p[0].buffer.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
    buff_size = frames_pointer_p[0].buffer.m.planes[0].length +
                  frames_pointer_p[0].buffer.m.planes[1].length;
    if (image_format == CAMERA_YUV_420_YV12)
      buff_size += frames_pointer_p[0].buffer.m.planes[2].length;
  } else {
    buff_size = frames_pointer_p[0].buffer.length;
  }
#ifdef USE_ION
  main_ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
  display_alloc.len = buff_size;
  display_alloc.flags = 0;
  display_alloc.heap_mask = (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
  display_alloc.align = 4096;
  display_buf = (unsigned long) do_mmap_ion(main_ion_fd, &display_alloc, &display_fd_data, &display_fd);
#else
  display_buf = (unsigned long) do_mmap(buff_size,
    &display_fd);
#endif
  if (!display_buf) {
    close(terminatefd[0]);
    close(terminatefd[1]);
    CDBG("cam_frame : *****************  memory map failed\n");
    return NULL;
  }
  memset((uint8_t *)display_buf, 0, buff_size);
  CDBG("%s display_fd = %d\n", __func__, display_fd);

  camframe_v4l2_thread_ready_signal();

  gettimeofday(&td1, &tz);

  timeoutms = 6000;
  do {
    struct timeval timeout;
    int has_p_buf = 0;
    int has_v_buf = 0;
    fds2[0].fd = camfdP;
    fds2[1].fd = camfdV;
    int num_fds = 1;
    fds2[0].events = POLLIN | POLLRDNORM;
    fds2[1].events = POLLIN | POLLRDNORM;
    if(camfdV > 0)
      num_fds = 2;
    else
      num_fds = 1;
    retval = poll(fds2, num_fds, timeoutms);

    CDBG("cam_frame_v4l2: select rc = 0x%x\n", retval);
    CDBG("cam_frame_v4l2: poll fd0 return revents:0x%x\n",
      fds2[0].revents);
    if(num_fds == 2) {
      CDBG("cam_frame_v4l2: poll fd1 return revents:0x%x\n",
        fds2[1].revents);
    }
    if (retval == 0) {
      if (camframe_v4l2_exit != 0) {
        CDBG("cam_frame: exit 1\n");
        break;
      }
      usleep(1000 * 100);
      continue;
    }
    else if (retval < 0) {
      CDBG("SELECT ERROR %s \n", strerror(errno));
      if (camframe_v4l2_exit != 0)
        break;
      usleep(1000 * 100);
      continue;
    }
    else {
      if (camframe_v4l2_exit != 0)
        break;

      if ((fds2[0].revents & POLLERR) || (fds2[0].revents & POLLERR)) {
        CDBG("Revents return POLLERR\n");
        if (camframe_v4l2_exit != 0)
          break;
          usleep(1000 * 100);
          continue;
      }

      if ((fds2[0].revents & POLLIN) && (fds2[0].revents & POLLRDNORM)) {
        bufInUseP.type = bufToPutP.type;
        bufInUseP.memory = bufToPutP.memory;
        if (bufInUseP.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
          bufInUseP.m.planes = &planes[0];
          if (image_format == CAMERA_YUV_420_YV12)
            bufInUseP.length = 3; // 3 planes.
          else
            bufInUseP.length = 2;
        }
        ioctlRetVal = ioctl(camfdP, VIDIOC_DQBUF, &bufInUseP);
        if (ioctlRetVal < 0) {
          CDBG("cam_frame_v4l2: VIDIOC_DQBUF ioctl failed; ioctlRetVal = %d \n",
            ioctlRetVal);
          if (camframe_v4l2_exit != 0)
            break;
          usleep(100);
          continue;
        }
        CDBG("cam_frame_v4l2: VIDIOC_DQBUF ioctl call done\n");
        CDBG("cam_frame_v4l2: bufInUseP.index = %d\n", bufInUseP.index);
        uint32_t j;
        for (j = 0; j < bufInUseP.length; j++) {
          CDBG("%s plane %d addr offset: %d data offset:%d\n",
               __func__, j, bufInUseP.m.planes[j].reserved[0],
               bufInUseP.m.planes[j].data_offset);
          frames_pointer_p[bufInUseP.index].buffer.m.planes[j].reserved[0] =
            bufInUseP.m.planes[j].reserved[0];
          frames_pointer_p[bufInUseP.index].buffer.m.planes[j].data_offset =
            bufInUseP.m.planes[j].data_offset;
        }

#if DBG_DUMP_YUV_FRAME
          if (frameCntp % 10 == 0 && frameCntp <= 100 ) {
            char bufp[BUFF_SIZE_128];
            snprintf(bufp, BUFF_SIZE_128, "/data/p_%d_%d.yuv",
              nLoop, frameCntp);
            int file_fdp = open(bufp, O_RDWR | O_CREAT, 0777);

            if (file_fdp < 0) {
              CDBG("cannot open file %s\n", bufp);
              goto done2;
            }
            CDBG("cam_frame_v4l2: writing recording dump image\n");
            CDBG("cam_frame_v4l2: recording buffer address = 0x%p\n",
              (const void *)frames_pointer_p[bufInUseP.index].addr[0]);
            CDBG("cam_frame_v4l2: v buffer length = %d\n",
              frames_pointer_p[bufInUseP.index].buffer.length);
            if (frames_pointer_p[bufInUseP.index].buffer.type ==
                                   V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
              write(file_fdp,
                (const void *)frames_pointer_p[bufInUseP.index].addr[0],
                frames_pointer_p[bufInUseP.index].buffer.m.planes[0].bytesused);
              koff = lseek(file_fdp,
                frames_pointer_p[bufInUseP.index].buffer.m.planes[0].bytesused,
                SEEK_SET);
              write(file_fdp,
                (const void *)frames_pointer_p[bufInUseP.index].addr[1],
                frames_pointer_p[bufInUseP.index].buffer.m.planes[1].bytesused);
              if (image_format == CAMERA_YUV_420_YV12) {
                koff = lseek(file_fdp,
                  frames_pointer_p[bufInUseP.index].buffer.m.planes[0].bytesused +
                  frames_pointer_p[bufInUseP.index].buffer.m.planes[1].bytesused,
                  SEEK_SET);
                write(file_fdp,
                  (const void *)frames_pointer_p[bufInUseP.index].addr[2],
                  frames_pointer_p[bufInUseP.index].buffer.m.planes[2].bytesused);
              }
              CDBG("%s:%d: done recordingwriting dump image, size=%d, %s\n",
                 __func__, __LINE__, buff_size, bufp);
            } else {
              write(file_fdp,
                (const void *)frames_pointer_p[bufInUseP.index].addr[0],
                frames_pointer_v[bufInUseP.index].buffer.length);
            }
            close(file_fdp);
          }
          frameCntp++;
  #endif
#if DBG_PERFORMANCE_FPS
        if (is_video_frame) {
          gettimeofday(&tdv2, &tz);
          int tmp =
           1000000 / ((tdv2.tv_sec - tdv1.tv_sec) *
           1000000 + (tdv2.tv_usec -
           tdv1.tv_usec));
          CDBG("cam_frame Profiling: video frame rate is %f\n",
           (1000000.0 / ((tdv2.tv_sec - tdv1.tv_sec) *
            1000000 + (tdv2.tv_usec -
           tdv1.tv_usec))));
          tdv1 = tdv2;
        } else {
          gettimeofday(&td2, &tz);
          int tmp =
                    1000000 / ((td2.tv_sec - td1.tv_sec) *
                    1000000 + (td2.tv_usec -
          td1.tv_usec));
         CDBG("cam_frame Profiling: preview frame rate is %f\n",
          (1000000.0 / ((td2.tv_sec - td1.tv_sec) * 1000000 + (td2.tv_usec -
          td1.tv_usec))));
          td1 = td2;
        }
#endif
        CDBG("cam_frame_v4l2: VIDIOC_QBUF ioctl call\n");
        ioctlRetVal = ioctl(camfdP, VIDIOC_QBUF, &bufToPutP);
        if (ioctlRetVal < 0) {
          CDBG ("cam_frame: VIDIOC_QBUF ioctl failed. return value is %d \n",
            ioctlRetVal);
          break;
        }
        if (camframe_v4l2_exit == 0) {
          bufToPutP = frames_pointer_p[bufInUseP.index].buffer;
          CDBG("cam_frame_v4l2: about to copy to display buffer\n");
          CDBG("cam_frame_v4l2: display buffer address is 0x%p\n",
            (uint8_t *)display_buf);
          if (frames_pointer_p[bufInUseP.index].buffer.type ==
                               V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            /* copy the buffer data for display purpose */
            memcpy((uint8_t *)display_buf,
              (const void *)frames_pointer_p[bufInUseP.index].addr[0],
              frames_pointer_p[bufInUseP.index].buffer.m.planes[0].bytesused);
            memcpy((uint8_t *)display_buf +
              frames_pointer_p[bufInUseP.index].buffer.m.planes[0].bytesused,
              (const void *)frames_pointer_p[bufInUseP.index].addr[1],
              frames_pointer_p[bufInUseP.index].buffer.m.planes[1].bytesused);
            if (image_format == CAMERA_YUV_420_YV12) {
              memcpy((uint8_t *)display_buf +
                frames_pointer_p[bufInUseP.index].buffer.m.planes[0].bytesused +
                frames_pointer_p[bufInUseP.index].buffer.m.planes[1].bytesused,
                (const void *)frames_pointer_p[bufInUseP.index].addr[2],
                frames_pointer_p[bufInUseP.index].buffer.m.planes[2].bytesused);
            }
            CDBG("cam_frame_v4l2: done copying to display buffer\n");

            } else {
              memcpy((uint8_t *)display_buf,
              (const void *)frames_pointer_p[bufInUseP.index].addr[0],
              frames_pointer_p[bufInUseP.index].buffer.length);
            }
          v4l2_render(display_fd, &bufInUseP, crop);
        } /*if (camframe_exit == 0)*/
      }/*if ((fds.revents & POLLIN) && (fds.revents & POLLRDNORM)) */
      if(num_fds == 2) {
        if ((fds2[1].revents & POLLIN) && (fds2[1].revents & POLLRDNORM)) {
          CDBG("%s:video buf dequeue,fd=%d\n",__func__,fds2[1].fd);
          bufInUseV.type = bufToPutV.type;
          bufInUseV.memory = bufToPutV.memory;
          CDBG("cam_frame_v4l2: VIDIOC_DQBUF ioctl call\n");
          ioctlRetVal = ioctl(camfdV, VIDIOC_DQBUF, &bufInUseV);
          if (ioctlRetVal < 0) {
            CDBG(
              "cam_frame_v4l2: VIDIOC_DQBUF ioctl failed; ioctlRetVal = %d \n",
              ioctlRetVal);
            if (camframe_v4l2_exit != 0)
              break;
            usleep(1000 * 100);
            continue;
          }
          CDBG("cam_frame_v4l2: VIDIOC_DQBUF ioctl call done\n");
          CDBG("cam_frame_v4l2: bufInUseV.index = %d\n", bufInUseV.index);
#if DBG_DUMP_YUV_FRAME
          static int frameCnt = 0;
          if (frameCnt%10 == 0 && frameCnt <= 100 ) {
            char buf[BUFF_SIZE_128];
            snprintf(buf, BUFF_SIZE_128, "/data/v_%d_%d.yuv",
              nLoop, frameCnt);
            int file_fd = open(buf, O_RDWR | O_CREAT, 0777);

            if (file_fd < 0) {
              CDBG("cannot open file %s\n", buf);
              goto done2;
            }
            CDBG("cam_frame_v4l2: writing recording dump image\n");
            CDBG("cam_frame_v4l2: recording buffer address = 0x%p\n",
              (const void *)frames_pointer_v[bufInUseV.index].addr[0]);
            if (frames_pointer_v[bufInUseV.index].buffer.type ==
                                V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
              write(file_fd,
                (const void *)frames_pointer_v[bufInUseV.index].addr[0],
                frames_pointer_v[bufInUseV.index].buffer.m.planes[0].bytesused);

              koff = lseek(file_fd,
                frames_pointer_v[bufInUseV.index].buffer.m.planes[0].bytesused,
                SEEK_SET);
              write(file_fd,
                (const void *)frames_pointer_v[bufInUseV.index].addr[1],
                frames_pointer_v[bufInUseV.index].buffer.m.planes[1].bytesused);
              CDBG("%s:%d: done recordingwriting dump image, size=%d, %s\n",
                 __func__, __LINE__,
                 frames_pointer_v[bufInUseV.index].buffer.m.planes[0].bytesused +
                 frames_pointer_v[bufInUseV.index].buffer.m.planes[1].bytesused, buf);
            } else {
              CDBG("cam_frame_v4l2: v buffer length = %d\n",
                 frames_pointer_v[bufInUseV.index].buffer.length);
              write(file_fd, (const void *)frames_pointer_v[bufInUseV.index].addr[0],
                frames_pointer_v[bufInUseV.index].buffer.length);
              CDBG("%s:%d: done recordingwriting dump image, size=%d, %s\n",
                 __func__, __LINE__,
                 frames_pointer_v[bufInUseV.index].buffer.length, buf);
            }
            close(file_fd);
          }
          frameCnt++;
#endif
          ioctlRetVal = ioctl(camfdV, VIDIOC_QBUF, &bufToPutV);
          CDBG_ERROR("%s:video recording enqueue buf fd=%d buf index=%d\n",
             __func__, fds2[1].fd, bufToPutV.index);
          if (ioctlRetVal < 0) {
            CDBG("cam_frame: VIDIOC_QBUF ioctl failed. return value is %d \n",
              ioctlRetVal);
            break;
          }
          if (camframe_v4l2_exit == 0) {
            bufToPutV = frames_pointer_v[bufInUseV.index].buffer;
          CDBG_ERROR("%s:video_fd=%d,next enqueue buf index=%d\n",
            __func__, fds2[1].fd, bufInUseV.index);
          }
        }/*if ((fds[1].revents & POLLIN) && (fds[1].revents & POLLRDNORM)) */
      }
    } /*else if (retval) */
    CDBG("%s exit flag = %d\n", __func__, camframe_v4l2_exit);
  } while (camframe_v4l2_exit == 0);
done2:
  close(terminatefd[0]);
  close(terminatefd[1]);
#ifdef USE_ION
  do_munmap_ion(main_ion_fd, &display_fd_data, (void *) display_buf, buff_size);
#else
  do_munmap(display_fd, (void *) display_buf, buff_size);
#endif
#ifdef USE_ION
  close(main_ion_fd);
#endif
  CDBG("Exiting camframe thread\n - preview_fd=%d", camfdP);
  return NULL;
}


static int v4l2_test_app_jpeg_fopen(char *filename)
{
  pthread_mutex_lock(&jpeg_encoder_ready_mutex);
  if (jpege_fout) {
    fclose(jpege_fout);
  }
  jpege_fout = fopen(filename, "wb");
  if (!jpege_fout) {
    pthread_mutex_unlock(&jpeg_encoder_ready_mutex);
    return -1;
  }
  return 0;
}

void v4l2_test_app_jpeg_fwrite(uint8_t *buf_ptr, uint32_t buf_size)
{
  fwrite(buf_ptr, 1, buf_size, jpege_fout);
}

void v4l2_test_app_jpeg_fclose(void)
{
  fclose(jpege_fout);
  jpege_fout = NULL;
  pthread_mutex_unlock(&jpeg_encoder_ready_mutex);
}

static void v4l2_test_app_jpeg_wait()
{
  pthread_mutex_lock(&jpeg_encoder_ready_mutex);
  pthread_mutex_unlock(&jpeg_encoder_ready_mutex);
}

#ifndef DISABLE_JPEG_ENCODING
static int do_jpeg_encoder(int t_fd, unsigned long t_buf, uint32_t t_len,
                           int s_fd, unsigned long s_buf, uint32_t s_len,
                           cam_ctrl_dimension_t *dimension, char* filename,
                           struct v4l2_crop *crop_t, struct v4l2_crop *crop_s)
{
  int rc = -1;
  common_crop_t crop_info;

  memset(&crop_info, 0, sizeof(common_crop_t));
  crop_info.in1_w = crop_t->c.width;
  crop_info.out1_w = crop_info.in1_w + 2 * crop_t->c.left;
  crop_info.in1_h = crop_t->c.height;
  crop_info.out1_h = crop_info.in1_h + 2 * crop_t->c.top;
  crop_info.in2_w = crop_s->c.width;
  crop_info.out2_w = crop_info.in2_w + 2 * crop_s->c.left;
  crop_info.in2_h = crop_s->c.height;
  crop_info.out2_h = crop_info.in2_h + 2 * crop_s->c.top;

  /* init */
  if (!jpeg_encoder_init()) {
    CDBG("jpeg encoder init failed\n");
  } else {
    rc = v4l2_test_app_jpeg_fopen(filename);
    if (rc < 0)
      return rc;
    /* encode */
    rc = jpeg_encoder_encode(dimension, (void*)t_buf, t_fd, (void*)s_buf, s_fd,
                             &crop_info, NULL, 0, -1, NULL, NULL,0);
  }
  /* dump to jpeg file: done through callbacks */
  v4l2_test_app_jpeg_wait();
  jpeg_encoder_join();
  return rc;
}
#endif /* DISABLE_JPEG_ENCODING */

int cam_get_raw_images(int fd, struct v4l2_frame_buffer *buffers, int buffer_num)
{
  struct pollfd fds;
  int retval, ioctlRetval;
  int timeoutms;
  int cnt = 0;
  struct v4l2_buffer buffer;
  char path[BUFF_SIZE_128];
  struct v4l2_plane planes[VIDEO_MAX_PLANES];

  fds.fd = fd;
  fds.events = POLLIN | POLLRDNORM;
  timeoutms = 6000;

  do {
    retval = poll(&fds, 1, timeoutms);
    if (retval == 0) {
      usleep(1000 * 100);
      continue;
    } else if (retval < 0) {
      CDBG_ERROR("%s: poll error %s\n", __func__, strerror(errno));
      usleep(1000 * 100);
      continue;
    } else {
      if (fds.revents & POLLERR) {
        CDBG_ERROR("%s: Revents return POLLERR\n", __func__);
        usleep(1000 * 100);
        continue;
      }

      if ((fds.revents & POLLIN) && (fds.revents & POLLRDNORM)) {
        int dump_fd;
        ssize_t bytes_written = 0;

        buffer.type = buffers[cnt].buffer.type;
        buffer.memory = buffers[cnt].buffer.memory;
        buffer.m.planes = &planes[0];
        buffer.length = 1;
        ioctlRetval = ioctl(fd, VIDIOC_DQBUF, &buffer);
        if (ioctlRetval < 0) {
          CDBG_ERROR("%s: ioctl VIDIOC_DQBUF failed, rc = %d\n",
            __func__, ioctlRetval);
          continue;
        }

        snprintf(path, BUFF_SIZE_128, "/data/%d.raw", cnt);
        dump_fd = open(path, O_RDWR | O_CREAT, 0777);
        if (dump_fd < 0) {
          CDBG_ERROR("%s: cannot open file %s: %s\n", __func__, path, strerror(errno));
          continue;
        }
        bytes_written = write(dump_fd, (const void *)buffers[buffer.index].addr[0],
          buffers[0].buffer.m.planes[0].length);
        if (bytes_written != (ssize_t)buffers[buffer.index].buffer.m.planes[0].length) {
          CDBG_ERROR("%s: bytes written to file %d is not equal to buffer size %d\n",
            __func__, (int)bytes_written, buffers[buffer.index].buffer.length);
        }

        close(dump_fd);
        cnt++;
      }
    }
  } while (cnt < buffer_num);
  return 0;
}

int cam_get_snapshot_images(void *data, cam_ctrl_dimension_t *dimension)
{
  int retval;
  int ioctlRetVal;
  struct pollfd fds;
  int timeoutms;
  struct pollfd fds2[2];
  int thumbnail_cnt = 0;
  int snapshot_cnt = 0;
  cam_frame_v4l2_start_parms* parms = (cam_frame_v4l2_start_parms*)data;
  struct v4l2_frame_buffer *frames_pointer_t = &parms->framesT[0];
  struct v4l2_frame_buffer *frames_pointer_s = &parms->framesS[0];
  int camfds[2];
  int camfdT = parms->camfdT;
  int camfdS = parms->camfdS;
  int num_fds = 2;
  int numFramesT = parms->numFramesT;
  int numFramesS = parms->numFramesS;
  struct v4l2_buffer bufInUseT;
  struct v4l2_buffer bufInUseS;
  struct v4l2_buffer bufToPutT = frames_pointer_t[numFramesT-1].buffer;
  struct v4l2_buffer bufToPutS = frames_pointer_s[numFramesS-1].buffer;
  int main_ion_fd;
  int s_fd = 0;
  int t_fd = 0;
  unsigned long s_buf = 0;
  unsigned long t_buf = 0;
  struct ion_allocation_data s_alloc;
  struct ion_allocation_data t_alloc;
  struct ion_fd_data s_fd_data;
  struct ion_fd_data t_fd_data;
  char jpegfilename[BUFF_SIZE_256] = {0};
  static unsigned int vcount = 0;
  static int frameCnt = 0;
  camframe_v4l2_exit = 0;
  errno = 0;
  struct timeval td1, td2, tdv1, tdv2;
  struct timezone tz;
  int i, lengthT = 0, lengthS = 0;
  int koff = 0;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
  gettimeofday(&td1, &tz);
#ifdef USE_ION
  main_ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
#endif
  if (frames_pointer_t[0].buffer.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
    lengthT = frames_pointer_t[0].buffer.m.planes[0].length +
                frames_pointer_t[0].buffer.m.planes[1].length;
  } else
    lengthT = frames_pointer_t[0].buffer.length;
#ifdef USE_ION
  t_alloc.len = lengthT;
  t_alloc.flags = 0;
  t_alloc.heap_mask = (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
  t_alloc.align = 4096;
  t_buf = (unsigned long) do_mmap_ion(main_ion_fd, &t_alloc, &t_fd_data, &t_fd);
#else
  t_buf = (unsigned long) do_mmap(lengthT, &t_fd);
#endif
  if (t_buf)
    CDBG(" Thumbnail buffer mapped addr %p fd %d length %d ",
          (void *)t_buf, t_fd, lengthT);
  if (frames_pointer_s[0].buffer.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
    lengthS = frames_pointer_s[0].buffer.m.planes[0].length +
                frames_pointer_s[0].buffer.m.planes[1].length;
  } else
    lengthS = frames_pointer_s[0].buffer.length;

#ifdef USE_ION
  s_alloc.len = lengthS;
  s_alloc.flags = 0;
  s_alloc.heap_mask = (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
  s_alloc.align = 4096;
  s_buf = (unsigned long) do_mmap_ion(main_ion_fd, &s_alloc, &s_fd_data, &s_fd);
#else
  s_buf = (unsigned long) do_mmap(lengthS, &s_fd);
#endif
  if (s_buf)
    CDBG(" MainImage buffer mapped addr %p fd %d length %d ",
          (void *)s_buf, s_fd, lengthS);

  timeoutms = 6000;
  do {
    struct timeval timeout;
    int has_p_buf = 0;
    int has_v_buf = 0;
    struct v4l2_crop cropT, cropM;
    fds2[0].fd = camfdT;
    fds2[1].fd = camfdS;
    fds2[0].events = POLLIN | POLLRDNORM;
    fds2[1].events = POLLIN | POLLRDNORM;
    cropT.c.left = 0;
    cropT.c.top = 0;
    cropT.c.width = 0;
    cropT.c.height = 0;

    cropM.c.left = 0;
    cropM.c.top = 0;
    cropM.c.width = 0;
    cropM.c.height = 0;

    retval = poll(fds2, 2, timeoutms);
    CDBG("cam_frame_v4l2: select rc = 0x%x\n", retval);
    CDBG("cam_frame_v4l2: poll return revents:0x%x|0x%x\n",
      fds2[0].revents, fds2[1].revents);
    if (retval == 0) {
      if (camframe_v4l2_exit != 0) {
        CDBG("cam_frame: exit 1\n");
        break;
      }
      usleep(1000 * 100);
      continue;
    } else if (retval < 0) {
      CDBG_ERROR("SELECT ERROR %s \n", strerror(errno));
      if (camframe_v4l2_exit != 0)
        break;
      usleep(1000 * 100);
      continue;
    } else {
      if ((fds2[0].revents & POLLERR) || (fds2[1].revents & POLLERR)) {
        CDBG_ERROR("Revents return POLLERR\n");
        if (camframe_v4l2_exit != 0)
          break;
          usleep(100);
          continue;
      }
      if ((thumbnail_cnt < numFramesT) && (fds2[0].revents & POLLIN) &&
        (fds2[0].revents & POLLRDNORM)) {
        bufInUseT.type = bufToPutT.type;
        bufInUseT.memory = bufToPutT.memory;
        if (bufInUseT.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
          bufInUseT.m.planes = &planes[0];
          bufInUseT.length = 2;
        }
        ioctlRetVal = ioctl(camfdT, VIDIOC_DQBUF, &bufInUseT);
        if (ioctlRetVal < 0) {
          CDBG_ERROR("cam_frame_v4l2: VIDIOC_DQBUF ioctl failed; ioctlRetVal = %d \n",
            ioctlRetVal);
          usleep(100);
        }
        if (t_buf) {
          ioctlRetVal = ioctl(camfdT, VIDIOC_G_CROP, &cropT);
          if (ioctlRetVal < 0) {
            CDBG_ERROR("%s: VIDIOC_G_CROP failed %s\n", __func__, strerror(errno));
          }
          if (bufInUseT.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            memcpy((uint8_t *)t_buf,
              (const void *)frames_pointer_t[bufInUseT.index].addr[0],
              frames_pointer_t[bufInUseT.index].buffer.m.planes[0].bytesused);
            memcpy((uint8_t *)t_buf + frames_pointer_t[bufInUseT.index].buffer.m.planes[0].bytesused,
              (const void *)frames_pointer_t[bufInUseT.index].addr[1],
              frames_pointer_t[bufInUseT.index].buffer.m.planes[1].bytesused);
          } else {
            CDBG(" Copying the thumbnail data ");
            memcpy((uint8_t *)t_buf,
              (const void *)frames_pointer_t[bufInUseT.index].addr[0],
              frames_pointer_t[bufInUseT.index].buffer.length);
          }
        }

#if DBG_DUMP_YUV_FRAME
        static int TCnt = 0, koff = 0;
        char buf[BUFF_SIZE_128];
        snprintf(buf, BUFF_SIZE_128, "/data/t_%d.yuv", TCnt);
        int file_fdt = open(buf, O_RDWR | O_CREAT, 0777);

        if (file_fdt < 0) {
          CDBG("%s:cannot open file %s\n", __func__, buf);
          goto check_1;
        }
        if (bufInUseT.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
          CDBG("cam_frame_v4l2: thumbnail buffer address = 0x%p\n",
            (const void *)frames_pointer_t[bufInUseT.index].addr[0]);
          CDBG("cam_frame_v4l2: thumbnail buffer length = %d\n",
            frames_pointer_t[bufInUseT.index].buffer.m.planes[0].bytesused);
          write(file_fdt, (const void *)frames_pointer_t[bufInUseT.index].addr[0],
            frames_pointer_t[bufInUseT.index].buffer.m.planes[0].bytesused);

          koff = lseek(file_fdt, frames_pointer_t[bufInUseT.index].buffer.m.planes[0].bytesused,
            SEEK_SET);

          write(file_fdt, (const void *)frames_pointer_t[bufInUseT.index].addr[1],
            frames_pointer_t[bufInUseT.index].buffer.m.planes[1].bytesused);
        } else {
          CDBG("cam_frame_v4l2: thumbnail buffer address = 0x%p\n",
            (const void *)frames_pointer_t[bufInUseT.index].addr[0]);
          CDBG("cam_frame_v4l2: thumbnail buffer length = %d\n",
            frames_pointer_t[bufInUseT.index].buffer.length);
          write(file_fdt, (const void *)frames_pointer_t[bufInUseT.index].addr[0],
            frames_pointer_t[bufInUseT.index].buffer.length);
        }
        CDBG("%s:%d: done writing thumbnail dump image %d, %s\n",
          __func__, __LINE__,
          lengthT, buf);
        close(file_fdt);

        TCnt++;

#endif
        thumbnail_cnt++;
        CDBG("cam_frame_v4l2: VIDIOC_DQBUF ioctl call done, fd = %d,\
          thumbnail_cnt=%d\n", camfdT, thumbnail_cnt);
      }
check_1:
      if ((snapshot_cnt < numFramesS) && (fds2[1].revents & POLLIN) && (fds2[1].revents & POLLRDNORM)) {
        CDBG("%s:snapshot buf dequeue,fd=%d\n",__func__,fds2[1].fd);
        bufInUseS.type = bufToPutS.type;
        bufInUseS.memory = bufToPutS.memory;
        if (bufInUseS.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
          bufInUseS.m.planes = &planes[0];
          bufInUseS.length = 2;
        }

        CDBG("cam_frame_v4l2: VIDIOC_DQBUF ioctl call\n");
        ioctlRetVal = ioctl(camfdS, VIDIOC_DQBUF, &bufInUseS);
        if (ioctlRetVal < 0) {
          CDBG("cam_frame_v4l2: VIDIOC_DQBUF ioctl failed; ioctlRetVal = %d \n",
            ioctlRetVal);
          usleep(100);
          continue;
        }

        CDBG("cam_frame_v4l2: VIDIOC_DQBUF ioctl call done, fd=%d,\
          snapshot_cnt=%d\n", camfdS, snapshot_cnt);
        static int frameCnt = 0;
        if(s_buf) {
          ioctlRetVal = ioctl(camfdS, VIDIOC_G_CROP, &cropM);
          if (ioctlRetVal < 0) {
            CDBG_ERROR("%s: VIDIOC_G_CROP failed %s\n", __func__, strerror(errno));
          }
          if (bufInUseS.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            memcpy((uint8_t *)s_buf,
              (const void *)frames_pointer_s[bufInUseS.index].addr[0],
              frames_pointer_s[bufInUseS.index].buffer.m.planes[0].bytesused);
            memcpy((uint8_t *)s_buf +
              frames_pointer_s[bufInUseS.index].buffer.m.planes[0].bytesused,
              (const void *)frames_pointer_s[bufInUseS.index].addr[1],
              frames_pointer_s[bufInUseS.index].buffer.m.planes[1].bytesused);
          } else {
            memcpy((uint8_t *)s_buf,
              (const void *)frames_pointer_s[bufInUseS.index].addr[0],
              frames_pointer_s[bufInUseS.index].buffer.length);
          }
        }
#if DBG_DUMP_YUV_FRAME
        char buf[BUFF_SIZE_128];
        snprintf(buf, BUFF_SIZE_128, "/data/s_%d.yuv", frameCnt);
        int file_fd = open(buf, O_RDWR | O_CREAT, 0777);

        if (file_fd < 0) {
          CDBG("%s:cannot open file\n", __func__);
          continue;
        }
        CDBG("cam_frame_v4l2: writing snapshot dump image\n");
        if (bufInUseS.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
          CDBG("cam_frame_v4l2: v buffer length = %d\n",
            frames_pointer_s[bufInUseS.index].buffer.length);
          write(file_fd, (const void *)frames_pointer_s[bufInUseS.index].addr[0],
            frames_pointer_s[bufInUseS.index].buffer.m.planes[0].bytesused);

          koff = lseek(file_fd, frames_pointer_s[bufInUseS.index].buffer.m.planes[0].bytesused,
            SEEK_SET);
          write(file_fd, (const void *)frames_pointer_s[bufInUseS.index].addr[1],
            frames_pointer_s[bufInUseS.index].buffer.m.planes[1].bytesused);
        } else {
          CDBG("cam_frame_v4l2: snapshot buffer address = 0x%p\n",
            (const void *)frames_pointer_s[bufInUseS.index].addr[0]);
          write(file_fd, (const void *)frames_pointer_s[bufInUseS.index].addr[0],
            frames_pointer_s[bufInUseS.index].buffer.length);
        }
        CDBG("%s:%d: done writing snapshot dump image %d, %s\n",
            __func__, __LINE__, lengthS, buf);
        close(file_fd);
#endif
        snprintf(jpegfilename, BUFF_SIZE_256,
          "/data/snapshot_%d.jpg", frameCnt);
        frameCnt++;
        snapshot_cnt++;
      }/*if ((fds[1].revents & POLLIN) && (fds[1].revents & POLLRDNORM)) */
    } /*else if (retval) */
    CDBG("%s exit flag = %d\n", __func__, camframe_v4l2_exit);
#ifndef DISABLE_JPEG_ENCODING
    if(s_buf && t_buf && (snapshot_cnt == thumbnail_cnt)) {
        do_jpeg_encoder(t_fd, t_buf, lengthT,
                    s_fd, s_buf, lengthS,
                    dimension, jpegfilename, &cropT, &cropM);
    }
#endif /* DISABLE_JPEG_ENCODING */

  } while ((snapshot_cnt < numFramesS || thumbnail_cnt < numFramesT));
  CDBG("%s: end, thumbnail_cnt=%d, snapshot_cnt=%d\n", __func__, thumbnail_cnt,
    snapshot_cnt);

  if(t_buf) {
#ifdef USE_ION
    do_munmap_ion(main_ion_fd, &t_fd_data, (void *) t_buf, lengthT);
#else
    do_munmap(t_fd, (void *)t_buf, lengthT);
#endif
  }
  if(s_buf) {
#ifdef USE_ION
    do_munmap_ion(main_ion_fd, &s_fd_data, (void *) s_buf, lengthS);
#else
    do_munmap(s_fd, (void *)s_buf, lengthS);
#endif
  }
#ifdef USE_ION
  close(main_ion_fd);
#endif
  return 0;
err:
  CDBG("%s: error end, thumbnail_cnt=%d, snapshot_cnt=%d\n", __func__,
    thumbnail_cnt, snapshot_cnt);
  return -1;
}

int32_t camframe_eztune_reg_callback(
  int32_t (*func_callback)(const char *, uint32_t))
{
  int32_t rc = 0;
  CDBG("%s called\n", __func__);
  eztune_preview_callback = func_callback;
  return rc;
}

int cam_get_thumbnail_images(int fd, struct v4l2_frame_buffer *buffers, int buffer_num)
{
  struct pollfd fds;
  int retval, ioctlRetval;
  int timeoutms;
  int cnt = 0;
  struct v4l2_buffer buffer;
  char path[BUFF_SIZE_128];
  int lengthT = 0;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];

  fds.fd = fd;
  fds.events = POLLIN | POLLRDNORM;
  timeoutms = 6000;

  if (buffers[0].buffer.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
    lengthT = buffers[0].buffer.m.planes[0].length +
                buffers[0].buffer.m.planes[1].length;
  } else
    lengthT = buffers[0].buffer.length;

  do {
    retval = poll(&fds, 1, timeoutms);
    if (retval == 0) {
      usleep(1000 * 100);
      continue;
    } else if (retval < 0) {
      CDBG_ERROR("%s: poll error %s\n", __func__, strerror(errno));
      usleep(1000 * 100);
      continue;
    } else {
      if (fds.revents & POLLERR) {
        CDBG_ERROR("%s: Revents return POLLERR\n", __func__);
        usleep(1000 * 100);
        continue;
      }

      if ((fds.revents & POLLIN) && (fds.revents & POLLRDNORM)) {
        int dump_fd;
        ssize_t bytes_written;

        buffer.type = buffers[cnt].buffer.type;
        buffer.memory = buffers[cnt].buffer.memory;
        if (buffer.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
          buffer.m.planes = &planes[0];
          buffer.length = 2;
        }
        ioctlRetval = ioctl(fd, VIDIOC_DQBUF, &buffer);
        if (ioctlRetval < 0) {
          CDBG_ERROR("%s: ioctl VIDIOC_DQBUF failed, rc = %d\n",
            __func__, ioctlRetval);
          continue;
        }

        snprintf(path, BUFF_SIZE_128, "/data/inline_thumbnail_%d.yuv", cnt);
        dump_fd = open(path, O_RDWR | O_CREAT, 0777);
        if (dump_fd < 0) {
          CDBG_ERROR("%s: cannot open file %s: %s\n", __func__, path, strerror(errno));
          continue;
        }

        if (buffer.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
          write(dump_fd,
            (const void *)buffers[buffer.index].addr[0],
            buffers[buffer.index].buffer.m.planes[0].bytesused);
          write(dump_fd,
            (const void *)buffers[buffer.index].addr[1],
            buffers[buffer.index].buffer.m.planes[1].bytesused);
        } else {
          CDBG(" Copying the thumbnail data ");
          write(dump_fd,
            (const void *)buffers[buffer.index].addr[0],
            buffers[buffer.index].buffer.length);
        }

#if 0
        bytes_written = write(dump_fd, (const void *)buffers[buffer.index].addr,
          buffers[buffer.index].buffer.length);
        if (bytes_written != (ssize_t)buffers[buffer.index].buffer.length) {
          CDBG_ERROR("%s: bytes written to file %d is not equal to buffer size %d\n",
            __func__, (int)bytes_written, buffers[buffer.index].buffer.length);
        }
#endif

        close(dump_fd);
        cnt++;
      }
    }
  } while (cnt < buffer_num);
  return 0;
}
