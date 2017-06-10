/*============================================================================

   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

#include "cam_mmap.h"
#include "common_cam.h"
#include "jpeg_encoder.h"
#include "camera.h"
#include "camera_dbg.h"
#include "camaf_ctrl.h"
#include "liveshot.h"
#include "mm_camera_interface.h"


int test_app_jpeg_fopen(const char *file_name);

extern camera_mode_t current_mode;
extern pthread_cond_t  af_cond;
extern pthread_mutex_t af_mutex_for_cond;
void *camaf_ctrl(void *data);
extern int8_t jps_format;

extern int32_t position;
/*===========================================================================
 * FUNCTION    - native_set_dimension -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_dimension(int camfd, void *pDim)
{
  int8_t rc = TRUE;
  int ret = -1;
  struct msm_ctrl_cmd ctrlCmd;

  cam_ctrl_dimension_t *pDimension = (cam_ctrl_dimension_t *) pDim;

  ctrlCmd.type = CAMERA_SET_PARM_DIMENSION;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.length = sizeof(cam_ctrl_dimension_t);
  ctrlCmd.value = pDimension;
  ctrlCmd.resp_fd = camfd;
  CDBG("Inside native_set_dimension: ioctl... rc  value is %d \n",
    ret);
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_dimension: ioctl failed... ioctl return value is %d \n",
      ret);
    rc = FALSE;
  }
  CDBG("coming out native_set_dimension: ioctl... rc  value is %d \n",
    ret);

  rc = (ctrlCmd.status == CAM_CTRL_SUCCESS);

  return rc;
}

/*===========================================================================
 * FUNCTION    - reg_unreg_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
void reg_unreg_buf(int camfd, int width, int height,
  uint32_t offset, int y_off, int pmempreviewfd, uint8_t * prev_buf,
  int type, int8_t unregister, int8_t active)
{
  struct msm_pmem_info pmemBuf;
  uint32_t ioctl_cmd;
  int ret;

  pmemBuf.type = type;
  pmemBuf.fd = pmempreviewfd;
  pmemBuf.vaddr = prev_buf;
  pmemBuf.planar0_off = y_off;
  pmemBuf.active = active;
  pmemBuf.offset = offset;

  /* If we are taking raw image, Y channel isn't used,
   * because the output is interleaved and packed. Instead,
   * we set y_offset to 0 to fill image all together.
   */
  if (type == MSM_PMEM_RAW_MAINIMG) {
    pmemBuf.planar1_off = 0;
    pmemBuf.len    = width * height;
  } else if (type == MSM_PMEM_MAINIMG) {
    jpeg_encoder_get_buffer_offset(width, height, &(pmemBuf.planar0_off), &(pmemBuf.planar1_off), &(pmemBuf.len) );
    printf("pmemBuf.cbcr_off %d \n", pmemBuf.planar1_off);
  } else {
    uint32_t y_size;
    CDBG("%s: current_mode %d \n", __func__, current_mode);
    if (CAMERA_MODE_3D == current_mode) {
      y_size = PAD_TO_2K(width * height);
      pmemBuf.planar1_off = PAD_TO_WORD(y_size);
      pmemBuf.len = PAD_TO_2K(width * height) * 3/2;
    } else {
      y_size = width * height;
      pmemBuf.planar1_off = PAD_TO_WORD(y_size);
      pmemBuf.len    = width * height * 3/2;
    }
  }

  if (type == MSM_PMEM_RAW_MAINIMG)
    CDBG("MSM_PMEM_RAW_MAINIMG unregister = %d call\n", unregister);
  else if (type == MSM_PMEM_THUMBNAIL)
    CDBG("MSM_PMEM_THUMBNAIL unregister = %d call\n", unregister);
  else if (type == MSM_PMEM_MAINIMG)
    CDBG("MSM_PMEM_MAINIMG unregister = %d call\n", unregister);
  else if (type == MSM_PMEM_PREVIEW)
    CDBG("MSM_PMEM_PREVIEW unregister = %d call\n", unregister);
  else
    CDBG("memory type = %d unregister = %d call\n", type, unregister);

  if (unregister)
    ioctl_cmd = MSM_CAM_IOCTL_UNREGISTER_PMEM;
  else
    ioctl_cmd = MSM_CAM_IOCTL_REGISTER_PMEM;

  if ((ret = ioctl(camfd, ioctl_cmd, &pmemBuf)) < 0) {
    CDBG
      ("reg_unreg_buf: MSM_CAM_IOCTL_(UN)REGISTER_PMEM ioctl failed... ioctl return value is %d \n",
      ret);
    if (type == MSM_PMEM_RAW_MAINIMG)
      CDBG("MSM_PMEM_RAW_MAINIMG unregister = %d failed\n", unregister);
    else if (type == MSM_PMEM_THUMBNAIL)
      CDBG("MSM_PMEM_THUMBNAIL unregister = %d failed\n", unregister);
    else if (type == MSM_PMEM_MAINIMG)
      CDBG("MSM_PMEM_MAINIMG unregister = %d failed\n", unregister);
    else if (type == MSM_PMEM_PREVIEW)
      CDBG("MSM_PMEM_PREVIEW unregister = %d failed\n", unregister);
    else
      CDBG("memory type = %d unregister = %d failed\n", type, unregister);
  } else {
    if (type == MSM_PMEM_RAW_MAINIMG)
      CDBG("MSM_PMEM_RAW_MAINIMG unregister = %d passed\n", unregister);
    else if (type == MSM_PMEM_THUMBNAIL)
      CDBG("MSM_PMEM_THUMBNAIL unregister = %d passed\n", unregister);
    else if (type == MSM_PMEM_MAINIMG)
      CDBG("MSM_PMEM_MAINIMG unregister = %d passed\n", unregister);
    else if (type == MSM_PMEM_PREVIEW)
      CDBG("MSM_PMEM_PREVIEW unregister = %d passed\n", unregister);
    else
      CDBG("memory type = %d unregister = %d passed\n", type, unregister);
  }
}

/*===========================================================================
 * FUNCTION    - native_set_default_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t native_set_default_params(int camfd)
{
  struct msm_ctrl_cmd ctrlCmd;
  uint32_t value;
  int ret;

  /*sending one parameter */
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.length = sizeof(uint32_t);
  ctrlCmd.value = malloc(ctrlCmd.length);
  ctrlCmd.resp_fd = camfd;
  if (!ctrlCmd.value) {
    CDBG("set_default_parameters: ctrlCmd.value malloc failed!\n");
    close(camfd);
    return FALSE;
  }

  /* Preview Mode */
  ctrlCmd.type = (uint16_t) CAMERA_SET_PARM_PREVIEW_MODE;
  value = (uint32_t) CAMERA_PREVIEW_MODE_SNAPSHOT;
  memcpy(ctrlCmd.value, &value, ctrlCmd.length);
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG
      ("set_default_parameters: CAMERA_SET_PARM_PREVIEW_MODE ioctl failed. ioctl return value is %d \n",
      ret);
  }

  /* Default Rotation - none */
  ctrlCmd.type = CAMERA_SET_PARM_ENCODE_ROTATION;
  value = 0;
  memcpy(ctrlCmd.value, &value, ctrlCmd.length);
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG
      ("set_default_parameters: CAMERA_SET_PARM_ENCODE_ROTATION ioctl failed. ioctl return value is %d \n",
      ret);
  }

  /* Default White Balance */
  ctrlCmd.type = CAMERA_SET_PARM_WB;
  value = 1;
  memcpy(ctrlCmd.value, &value, ctrlCmd.length);
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG
      ("set_default_parameters: CAMERA_SET_PARM_WB ioctl failed. ioctl return value is %d \n",
      ret);
  }
  /* Default Effect: hue, saturation */
  ctrlCmd.type = CAMERA_SET_PARM_EFFECT;
  value = 1;
  memcpy(ctrlCmd.value, &value, ctrlCmd.length);

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG
      ("set_default_parameters: CAMERA_SET_PARM_EFFECT ioctl failed. ioctl return value is %d \n",
      ret);
  }

  /* Default NightShot mode: off */
  ctrlCmd.type = CAMERA_SET_PARM_NIGHTSHOT_MODE;
  value = 0;
  memcpy(ctrlCmd.value, &value, ctrlCmd.length);
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG
      ("set_default_parameters: CAMERA_SET_PARM_NIGHTSHOT_MODE ioctl failed. ioctl return value is %d \n",
      ret);
  }

  /* Default LUMA Adaptation: off */
  ctrlCmd.type = CAMERA_SET_PARM_LUMA_ADAPTATION;
  value = 0;
  memcpy(ctrlCmd.value, &value, ctrlCmd.length);
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG
      ("set_default_parameters: CAMERA_SET_PARM_LUMA_ADAPTATION ioctl failed. ioctl return value is %d \n",
      ret);
  }

  ctrlCmd.type = CAMERA_SET_SCE_FACTOR;
  value = CAMERA_DEF_SCE_FACTOR;
  memcpy(ctrlCmd.value, &value, ctrlCmd.length);
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG
      ("set_default_parameters: CAMERA_SET_PARM_LUMA_ADAPTATION ioctl failed. ioctl return value is %d \n",
      ret);
  }

  /* Default Antibanding: off */
  ctrlCmd.type = CAMERA_SET_PARM_ANTIBANDING;
  value = 0;
  memcpy(ctrlCmd.value, &value, ctrlCmd.length);
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG
      ("set_default_parameters: CAMERA_SET_PARM_ANTIBANDING ioctl failed. ioctl return value is %d \n",
      ret);
  }

  free(ctrlCmd.value);

  /* sending two parameters */
  ctrlCmd.length = 2 * sizeof(uint32_t);
  ctrlCmd.value = malloc(ctrlCmd.length);
  if (!ctrlCmd.value) {
    CDBG("set_default_parameters: ctrlCmd.value malloc failed!\n");
    close(camfd);
    return FALSE;
  }
  /* Default Auto FPS: 30 */
  ctrlCmd.type = CAMERA_SET_PARM_PREVIEW_FPS;
  uint32_t previewFPSParams[2] = { 65551000, 65551000};
  memcpy(ctrlCmd.value, previewFPSParams, ctrlCmd.length);
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG
      ("set_default_parameters: CAMERA_SET_PARM_PREVIEW_FPS ioctl failed. ioctl return value is %d \n",
      ret);
  }

  free(ctrlCmd.value);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_register_preview_bufs -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_register_preview_bufs(int camfd, void *pDim, uint32_t offset,
  struct msm_frame * frame, int8_t active)
{
  cam_ctrl_dimension_t *dimension = (cam_ctrl_dimension_t *) pDim;
  CDBG("dimension->display_width = %d, display_height = %d\n",
    dimension->display_width, dimension->display_height);

  reg_unreg_buf(camfd,
    dimension->display_width,
    dimension->display_height, offset, 0,
    frame->fd, (uint8_t *) frame->buffer, MSM_PMEM_PREVIEW, FALSE, active);

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_unregister_preview_bufs -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_unregister_preview_bufs(int camfd,
  void *pDim, uint32_t offset, int pmempreviewfd, uint8_t * prev_buf)
{
  cam_ctrl_dimension_t *dimension = (cam_ctrl_dimension_t *) pDim;

  reg_unreg_buf(camfd,
    dimension->display_width,
    dimension->display_height, offset, 0,
    pmempreviewfd, prev_buf, MSM_PMEM_PREVIEW, TRUE, TRUE);

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_register_video_bufs -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_register_video_bufs(int camfd, void *pDim, uint32_t offset,
  struct msm_frame * frame, int8_t active, int8_t vpe_en)
{
  cam_ctrl_dimension_t *dimension = (cam_ctrl_dimension_t *) pDim;
  CDBG("dimension->display_width = %d, display_height = %d\n",
    dimension->display_width, dimension->display_height);

  if (!vpe_en)
    reg_unreg_buf(camfd,
      dimension->video_width,
      dimension->video_height, offset, 0,
      frame->fd, (uint8_t *) frame->buffer, MSM_PMEM_VIDEO, FALSE, active);
  else
    reg_unreg_buf(camfd,
      dimension->video_width,
      dimension->video_height, offset, 0,
      frame->fd, (uint8_t *) frame->buffer, MSM_PMEM_VIDEO_VPE, FALSE, active);

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_unregister_video_bufs -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_unregister_video_bufs(int camfd,
  void *pDim, uint32_t offset, int pmemvideofd, uint8_t * video_buf, int8_t vpe_en)
{
  cam_ctrl_dimension_t *dimension = (cam_ctrl_dimension_t *) pDim;
  if (!vpe_en)
    reg_unreg_buf(camfd,
      dimension->video_width,
      dimension->video_height, offset, 0,
      pmemvideofd, video_buf, MSM_PMEM_VIDEO, TRUE, TRUE);
  else
    reg_unreg_buf(camfd,
      dimension->video_width,
      dimension->video_height, offset, 0,
      pmemvideofd, video_buf, MSM_PMEM_VIDEO_VPE, TRUE, TRUE);

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_start_preview -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_start_preview(int camfd)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_START_PREVIEW;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ret < 0) {
    CDBG("native_start_preview: MSM_CAM_IOCTL_CTRL_COMMAND failed. ret=%d \n",
      ret);
    ret = ctrlCmd.status;
    return FALSE;
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_register_raw_snapshot_bufs -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_register_raw_snapshot_bufs(int camfd,
  void *pDim, int pmemsnapshotfd, unsigned char * main_img_buf)
{
  cam_ctrl_dimension_t *dimension = (cam_ctrl_dimension_t *) pDim;

  int y_off = 0;

  /* For original snapshot */
  CDBG("Inside native_register_raw_snapshot_bufs \n");
  reg_unreg_buf(camfd, dimension->raw_picture_width,
    dimension->raw_picture_height, 0, y_off,
    pmemsnapshotfd, main_img_buf, MSM_PMEM_RAW_MAINIMG, FALSE, TRUE);

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_register_snapshot_bufs -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_register_snapshot_bufs(int camfd, int width,
  int height, int buf_offset, int y_off, int buf_fd, uint8_t *buf,
  int pmem_type)
{
  if ((buf != NULL) && (buf_fd != 0)) {
    reg_unreg_buf(camfd, width, height, buf_offset, y_off,
      buf_fd, buf, pmem_type, FALSE, TRUE);
  }

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_unregister_raw_snapshot_bufs -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_unregister_raw_snapshot_bufs(int camfd,
  void *pDim, int pmemSnapshotfd, unsigned char * main_img_buf)
{
  cam_ctrl_dimension_t *dimension = (cam_ctrl_dimension_t *) pDim;

  int y_off = 0;
  /* For original snapshot */
  reg_unreg_buf(camfd, dimension->raw_picture_width,
    dimension->raw_picture_height, 0,
    y_off,
    pmemSnapshotfd, main_img_buf, MSM_PMEM_RAW_MAINIMG, TRUE, TRUE);

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_unregister_snapshot_bufs -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_unregister_snapshot_bufs(int camfd, int width,
  int height, int buf_offset, int y_off, int buf_fd, uint8_t *buf,
  int pmem_type)
{
  if ((buf != NULL) && (buf_fd != 0)) {
    reg_unreg_buf(camfd, width, height, buf_offset, y_off, buf_fd, buf,
      pmem_type, TRUE, TRUE);
  }

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_get_picture -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_get_picture(int camfd, struct crop_info *cropInfo)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  struct crop_info *pCrop = (struct crop_info *)cropInfo;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.length     = pCrop->len;
  ctrlCmd.value      = pCrop->info;
  ctrlCmd.resp_fd    = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_GET_PICTURE, &ctrlCmd)) < 0) {
    CDBG
      ("native_get_picture: MSM_CAM_IOCTL_GET_PICTURE failed... ioctl return value is %d \n",
      ret);
    ret = ctrlCmd.status;
    return FALSE;
  }

  return TRUE;
}


/*===========================================================================
 * FUNCTION    - native_set_jpeg_rotation -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_jpeg_rotation(int rotation)
{
  CDBG("native_set_jpeg_rotation %d",rotation);
  if (!jpeg_encoder_setRotation(rotation)) {
    CDBG("native_set_jpeg_rotation: FAILED ");
    return FALSE;
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_set_strobe_flash_mode -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_strobe_flash_mode(int camfd, strobe_flash_mode_t mode)
{
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_STROBE_FLASH_MODE;
  ctrlCmd.length = sizeof(strobe_flash_mode_t);
  ctrlCmd.value =(void *) &(mode);
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_flash_mode: ioctl failed. ioctl return: %d \n", ret);
    ret = ctrlCmd.status;
    return FALSE;
  }
  CDBG("native_set_flash_mode: ioctl good. ioctl return: %d \n",ret);

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_jpeg_encode -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_jpeg_encode(const char *path, void *pDim,
  int pmemThumbnailfd, int pmemSnapshotfd,
  uint8_t *thumbnail_buf, uint8_t *main_img_buf, void *pCrop,
  camera_encoding_rotate_t rotate)
{
  CDBG("%s:", __func__);
  return FALSE;
}

/*===========================================================================
 * FUNCTION    - native_stop_preview -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_stop_preview(int camfd)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_STOP_PREVIEW;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_stop_preview: ioctl failed. ioctl return value is %d \n", ret);
    ret = ctrlCmd.status;
    return FALSE;
  }

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_start_raw_snapshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_start_raw_snapshot(int camfd)
{
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_START_RAW_SNAPSHOT;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_start_snapshot: ioctl failed. ioctl return value is %d \n",
      ret);
    ret = ctrlCmd.status;
    return FALSE;
  }

  return TRUE;
}
/*===========================================================================
 * FUNCTION    - native_start_recording -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_start_recording(int camfd)
{
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_START_RECORDING;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_start_video: ioctl failed. ioctl return value is %d \n",
      ret);
    ret = ctrlCmd.status;
    return FALSE;
  }
  CDBG("native_start_video: ioctl good. ioctl return value is %d \n",ret);

  /* TODO: Check status of postprocessing if there is any,
   *       PP status should be in  ctrlCmd */

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_stop_recording -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_stop_recording(int camfd)
{
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_STOP_RECORDING;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_stop_video: ioctl failed. ioctl return value is %d \n",
      ret);
    return FALSE;
  }

  return TRUE;
}
/*===========================================================================
 * FUNCTION    - native_start_video -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_start_video(int camfd)
{
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_START_VIDEO;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_start_video: ioctl failed. ioctl return value is %d \n",
      ret);
    ret = ctrlCmd.status;
    return FALSE;
  }

  /* TODO: Check status of postprocessing if there is any,
   *       PP status should be in  ctrlCmd */

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_stop_video -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_stop_video(int camfd)
{
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_STOP_VIDEO;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_stop_video: ioctl failed. ioctl return value is %d \n",
      ret);
    return FALSE;
  }

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_start_snapshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_start_snapshot(int camfd)
{
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_START_SNAPSHOT;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_start_snapshot: ioctl failed. ioctl return value is %d \n",
      ret);
    ret = ctrlCmd.status;
    return FALSE;
  }

  /* TODO: Check status of postprocessing if there is any,
   *       PP status should be in  ctrlCmd */

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_stop_snapshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_stop_snapshot(int camfd)
{
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_STOP_SNAPSHOT;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_stop_snapshot: ioctl failed. ioctl return value is %d \n",
      ret);
    return FALSE;
  }

  return TRUE;
}

/*===========================================================================
FUNCTION      native_get_maxzoom

DESCRIPTION
===========================================================================*/
int8_t native_get_maxzoom(
  int camfd,
  void *pZm)
{
  int8_t rc = TRUE;
  int ioctlRetVal;
  struct msm_ctrl_cmd ctrlCmd;

  int32_t *pZoom = (int32_t *)pZm;

  ctrlCmd.type       = CAMERA_GET_PARM_MAXZOOM;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.length     = sizeof(int32_t);
  ctrlCmd.value      = pZoom;
  ctrlCmd.resp_fd = camfd;

  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_zoom: ioctl failed... ioctl return value is %d \n", ioctlRetVal);
    rc = FALSE;
  }

  memcpy(pZoom,
    (int32_t *)ctrlCmd.value,
    sizeof(int32_t));

  rc = ctrlCmd.status;

  return rc;

}

/*===========================================================================
FUNCTION      native_get_zoomratios

DESCRIPTION
===========================================================================*/
int8_t native_get_zoomratio(
  int camfd, int maxZoom,
  void *pZr)
{
  CDBG("native_get_zoomratios E");

  struct msm_ctrl_cmd ctrlCmd;
  int16_t *pZoomRatios = (int16_t *)pZr;

  ctrlCmd.type       = CAMERA_GET_PARM_ZOOMRATIOS;
  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.length     = sizeof(int16_t) * maxZoom;
  ctrlCmd.value      = pZoomRatios;
  ctrlCmd.resp_fd    = camfd;

  if (ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0) {
    CDBG("native_get_zoomratios: ioctl fd %d error %s",
      camfd,
      strerror(errno));
    return false;
  }

  CDBG("native_get_zoomratios X");
  return true;
}

/*===========================================================================
 * FUNCTION    - native_set_zoom -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_zoom(int camfd, void *pZm)
{
  int8_t rc = TRUE;
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  int32_t *pZoom = (int32_t *) pZm;

  ctrlCmd.type = CAMERA_SET_PARM_ZOOM;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.length = sizeof(int32_t);
  ctrlCmd.value = pZoom;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_zoom: ioctl failed... ioctl return value is %d \n", ret);
    rc = FALSE;
  }

  memcpy(pZoom, (int32_t *) ctrlCmd.value, sizeof(int32_t));

  rc = ctrlCmd.status;

  return rc;
}

/*===========================================================================
 * FUNCTION    - native_set_special_effect -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_special_effect(int camfd, int effect)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  CDBG("native_set_special_effect\n");
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_EFFECT;
  ctrlCmd.length = sizeof(uint32_t);
  ctrlCmd.value = (void *) &effect;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_special_effect: ioctl failed. ioctl return value is %d \n",
      ret);
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_brightness -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_brightness(int camfd, int brightness)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_BRIGHTNESS;
  ctrlCmd.length = sizeof(int);
  ctrlCmd.value = (void *) &brightness;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_brightness: ioctl failed. ioctl return value is %d \n",
      ret);
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_contrast -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_contrast(int camfd, int contrast)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_CONTRAST;
  ctrlCmd.length = sizeof(int);
  ctrlCmd.value = (void *) &contrast;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_brightness: ioctl failed. ioctl return value is %d \n",
      ret);
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_SCE_factor -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_SCE_factor(int camfd, int sce_adj)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_SCE_FACTOR;
  ctrlCmd.length = sizeof(int);
  ctrlCmd.value = (void *) &sce_adj;
  ctrlCmd.resp_fd = camfd;
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_SCE_factor: ioctl failed. ioctl return value is %d \n",
      ret);
  }
  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_saturation -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_saturation(int camfd, int saturation)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_SATURATION;
  ctrlCmd.length = sizeof(int);
  ctrlCmd.value = (void *) &saturation;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_brightness: ioctl failed. ioctl return value is %d \n",
      ret);
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_ev -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_ev(int camfd, int ev)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_EXPOSURE_COMPENSATION;
  ctrlCmd.length = sizeof(int32_t);
  ctrlCmd.value = (void *) &ev;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_ev: ioctl failed. ioctl return value is %d \n", ret);
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_AntiBanding -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_AntiBanding(int camfd, int32_t antiBanding)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_ANTIBANDING;
  ctrlCmd.length = sizeof(int32_t);
  ctrlCmd.value = (void *) &antiBanding;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_AndiBanding: ioctl failed. ioctl return value is %d \n",
      ret);
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_WhiteBalance -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_WhiteBalance(int camfd, int32_t wb)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_WB;
  ctrlCmd.length = sizeof(int32_t);
  ctrlCmd.value = (void *) &wb;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_WhiteBalance: ioctl failed. ioctl return value is %d \n",
      ret);
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_aecmode -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_aecmode(int camfd, camera_auto_exposure_mode_type aec_mode)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_EXPOSURE;
  ctrlCmd.length = sizeof(int);
  ctrlCmd.value = (void *) &aec_mode;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_brightness: ioctl failed. ioctl return value is %d \n",
      ret);
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_aecroi -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_aecroi(int camfd, cam_set_aec_roi_t cam_set_aec_roi)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_AEC_ROI;
  ctrlCmd.length = sizeof(cam_set_aec_roi_t);
  ctrlCmd.value = (void *) &cam_set_aec_roi;
  ctrlCmd.resp_fd = camfd;

  CDBG("%s: cmd->length =%d, value =%p\n", __func__, ctrlCmd.length,
    ctrlCmd.value);

#if 1
  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_aecroi: ioctl failed. ioctl return value is %d \n",
      ret);
  }
#endif

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_afmode -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_afmode(int camfd, isp3a_af_mode_t af_mode, cam_af_ctrl_t *pAfCtrl)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  pAfCtrl->ctrlCmd.timeout_ms = 5000;
  pAfCtrl->ctrlCmd.status = 0;
  pAfCtrl->ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  CDBG("%s: Spawning new thread for AF \n", __func__);
  if (launch_camafctrl_thread(pAfCtrl)) {
    CDBG("main: launch_camframe_fb_thread failed!\n");
    ret = FALSE;
    return ret;
  }

  CDBG("%s: AF thread is started.\n", __func__);

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_AUTO_FOCUS;
  ctrlCmd.length = sizeof(int);
  ctrlCmd.value = (void *) &af_mode;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d ret = %d with error: \n",
      __func__, ret, errno);
    ret = errno;
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_iso -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_iso(int camfd, camera_iso_mode_type iso)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_ISO;
  ctrlCmd.length = sizeof(int);
  ctrlCmd.value = (void *) &iso;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_iso: ioctl failed. ioctl return value is %d \n", ret);
  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - native_set_sharpness -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_set_sharpness(int camfd, int sharpness)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_SHARPNESS;
  ctrlCmd.length = sizeof(int);
  ctrlCmd.value = (void *) &sharpness;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_sharpness: ioctl failed. ioctl return value is %d \n",
      ret);
  }

  return ret;
}

/*===========================================================================
FUNCTION      native_enable_HJR

DESCRIPTION
===========================================================================*/
int8_t native_set_hjr (int camfd, int hjr_status)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_PARM_HJR;
  ctrlCmd.length     = sizeof(int);
  ctrlCmd.value      = &hjr_status;
  ctrlCmd.resp_fd = camfd;
  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_sharpness: ioctl failed. ioctl return value is %d \n", ioctlRetVal);
  }

  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_set_lens_shading

DESCRIPTION
===========================================================================*/
int8_t native_set_lens_shading (int camfd, int8_t
  lens_shading_status) {
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_PARM_ROLLOFF;
  ctrlCmd.length     = sizeof(int8_t);
  ctrlCmd.value      = &lens_shading_status;
  ctrlCmd.resp_fd = camfd;

  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_lens_shading: ioctl failed. ioctl return value is %d \n", ioctlRetVal);
  }

  return ioctlRetVal;
}

int8_t native_set_led_mode(int camfd, led_mode_t led_mode) {
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_PARM_LED_MODE;
  ctrlCmd.length     = sizeof(int32_t);
  ctrlCmd.value      = &led_mode;
  ctrlCmd.resp_fd = camfd;

  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_lens_shading: ioctl failed. ioctl return value is %d \n", ioctlRetVal);
  }

  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_get_AF_sharpness

DESCRIPTION
===========================================================================*/
int8_t native_get_AF_sharpness (int camfd, int32_t *sharpness)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_GET_PARM_AF_SHARPNESS;
  ctrlCmd.length     = sizeof(int32_t *);
  ctrlCmd.value      = (void *)sharpness;
  ctrlCmd.resp_fd = camfd;

  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_afmode: ioctl failed. ioctl return value is %d \n", ioctlRetVal);
  }

  CDBG("native_get_AF_sharpness:sharpness=%d\n", *sharpness);

  return ioctlRetVal;
}

/*===========================================================================
FUNCTION     native_set_motion_iso

DESCRIPTION
===========================================================================*/
int8_t native_set_motion_iso(int camfd, motion_iso_t motion_iso)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_SET_MOTION_ISO;
  ctrlCmd.length     = sizeof(motion_iso_t);
  ctrlCmd.value      = &motion_iso;
  ctrlCmd.resp_fd = camfd;

  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("native_set_motion_iso: ioctl failed. ioctl return value is %d\n",
      ioctlRetVal);
  }

  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_set_hue

DESCRIPTION   Set the hue value. Value range from 0 to 300, 60 incremental
===========================================================================*/
int8_t native_set_hue(int camfd, int32_t hue)
{
  int ioctlRetVal;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_SET_PARM_HUE;
  ctrlCmd.length     = sizeof(int32_t);
  ctrlCmd.value      = &hue;
  ctrlCmd.resp_fd = camfd;

  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("native_set_hue: ioctl failed. ioctl return value is %d\n",
      ioctlRetVal);
    return FALSE;
  } else
    return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_AF_cancel -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_AF_cancel(int camfd)
{
  int8_t rc;
  struct msm_ctrl_cmd ctrlCmd;

  CDBG("%s: IN\n", __func__);

  errno = 0;
  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_AUTO_FOCUS_CANCEL;
  ctrlCmd.length     = 0;
  ctrlCmd.value      = NULL;
  ctrlCmd.resp_fd = camfd;

  CDBG("%s: Calling CAMERA_AUTO_FOCUS_CANCEL\n", __func__);

  rc = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (rc < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d errno = %d\n",
      __func__, rc, errno);
  }
  CDBG("%s:rc = %d\n", __func__, rc);

  return rc;
}

/*===========================================================================
FUNCTION      native_get_Af_step

DESCRIPTION
===========================================================================*/
int8_t native_get_Af_step (int camfd, int32_t *afStep)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_GET_PARM_FOCUS_STEP;
  ctrlCmd.length     = sizeof(int32_t);
  ctrlCmd.value      = (void *)afStep;
  ctrlCmd.resp_fd = camfd;

  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_get_af_step: ioctl failed. ioctl return value is %d \n", ioctlRetVal);
  }

  CDBG("native_get_Af_step:afStep=%d\n", *afStep);

  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_set_Af_step

DESCRIPTION
===========================================================================*/
int8_t native_set_Af_step (int camfd, int32_t afStep)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_SET_PARM_FOCUS_STEP;
  ctrlCmd.length     = sizeof(int32_t);
  ctrlCmd.value      = (void *)&afStep;
  ctrlCmd.resp_fd = camfd;

  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_af_step: ioctl failed. ioctl return value is %d \n", ioctlRetVal);
  }

  return ioctlRetVal;
}
/*===========================================================================
FUNCTION      native_enable_AFD

DESCRIPTION
===========================================================================*/
int8_t native_enable_AFD (int camfd)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_ENABLE_AFD;
  ctrlCmd.length     = 0;
  ctrlCmd.value      = NULL;
  ctrlCmd.resp_fd = camfd;
  CDBG("%s: IN\n", __func__);
  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  CDBG("%s: OUT\n", __func__);
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_prepare_snapshot

DESCRIPTION
===========================================================================*/
int8_t native_prepare_snapshot (int camfd)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_PREPARE_SNAPSHOT;
  ctrlCmd.length     = 0;
  ctrlCmd.value      = NULL;
  ctrlCmd.resp_fd = camfd;
  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  if(ioctlRetVal == 0)
    usleep(1000*1000);
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_set_fps_mode

DESCRIPTION
===========================================================================*/
int8_t native_set_fps_mode (int camfd, fps_mode_t fps_mode)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_FPS_MODE;
  ctrlCmd.length     = sizeof(fps_mode_t);
  ctrlCmd.value      = &fps_mode;
  ctrlCmd.resp_fd = camfd;
  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_set_fps

DESCRIPTION
===========================================================================*/
int8_t native_set_fps(int camfd, uint32_t fps)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_PARM_FPS;
  ctrlCmd.length     = sizeof(uint32_t);
  ctrlCmd.value      = &fps;
  ctrlCmd.resp_fd = camfd;
  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_set_af_focus_rect

DESCRIPTION
===========================================================================*/
int8_t native_set_af_focus_rect(int camfd, cam_af_focusrect_t af_focus_rect)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_PARM_FOCUS_RECT;
  ctrlCmd.length     = sizeof(uint16_t);
  ctrlCmd.value      = &af_focus_rect;
  ctrlCmd.resp_fd = camfd;
  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION     native_set_CAF

DESCRIPTION
===========================================================================*/
int8_t native_set_CAF(int camfd, caf_ctrl_t caf)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_SET_CAF;
  ctrlCmd.length     = sizeof(caf_ctrl_t);
  ctrlCmd.value      = &caf;
  ctrlCmd.resp_fd = camfd;

  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("native_set_CAF: ioctl failed. ioctl return value is %d\n",
      ioctlRetVal);
  }
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_enable_la

DESCRIPTION
===========================================================================*/
int8_t native_enable_la(int camfd, void *la_enable)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  int32_t *la_vlaue = (int32_t *)la_enable;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_PARM_LUMA_ADAPTATION;
  ctrlCmd.length     = sizeof(uint32_t);
  ctrlCmd.value      = la_vlaue;
  ctrlCmd.resp_fd = camfd;
  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_set_video_dis_parms

DESCRIPTION
===========================================================================*/
int8_t native_set_video_dis_parms(int camfd, void *disCtrl)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  video_dis_param_ctrl_t *disInfo = (video_dis_param_ctrl_t *)disCtrl;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_VIDEO_DIS_PARAMS;
  ctrlCmd.length     = sizeof(video_dis_param_ctrl_t);
  ctrlCmd.value      = disInfo;
  ctrlCmd.resp_fd    = camfd;

  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  CDBG("In %s: size =%d\n", __func__,ctrlCmd.length);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_set_video_rot_parms

DESCRIPTION
===========================================================================*/
int8_t native_set_video_rot_parms(int camfd, void *rotCtrl)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  video_rotation_param_ctrl_t *rotInfo = (video_rotation_param_ctrl_t *)rotCtrl;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_VIDEO_ROT_PARAMS;
  ctrlCmd.length     = sizeof(video_rotation_param_ctrl_t);
  ctrlCmd.value      = rotInfo;
  ctrlCmd.resp_fd    = camfd;

  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  CDBG("In %s: size =%d\n", __func__,ctrlCmd.length);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_enable_la

DESCRIPTION
===========================================================================*/
int8_t native_disable_la(int camfd, void *la_disable)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  int32_t *la_vlaue = (int32_t *)la_disable;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_PARM_LUMA_ADAPTATION;
  ctrlCmd.length     = sizeof(uint32_t);
  ctrlCmd.value      = la_vlaue;
  ctrlCmd.resp_fd = camfd;
  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_set_bl_detection

DESCRIPTION  set backlight detection enable
===========================================================================*/
int8_t native_set_bl_detection(int camfd, uint8_t bl_detection_enable)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_PARM_BL_DETECTION_ENABLE;
  ctrlCmd.length     = sizeof(uint8_t);
  ctrlCmd.value      = &bl_detection_enable;
  ctrlCmd.resp_fd = camfd;
  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  return ioctlRetVal;
}

/*===========================================================================
FUNCTION      native_set_snow_detection

DESCRIPTION  set snow-cloudy scene detection enable
===========================================================================*/
int8_t native_set_snow_detection(int camfd, uint8_t snow_detection_enable)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type       = CAMERA_SET_PARM_SNOW_DETECTION_ENABLE;
  ctrlCmd.length     = sizeof(uint8_t);
  ctrlCmd.value      = &snow_detection_enable;
  ctrlCmd.resp_fd = camfd;
  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n", __func__, ioctlRetVal);
  }
  return ioctlRetVal;
}
/*===========================================================================
    * FUNCTION    - native_set_BestShotMode -
    *
    * DESCRIPTION:
*==========================================================================*/
int8_t native_set_BestShotMode(int camfd, uint8_t bestshotmode)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;
  /* int32_t *bestshot_value = (int32_t *)bestshotmode ; */

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_BESTSHOT_MODE;
  ctrlCmd.length = sizeof(uint8_t);
  ctrlCmd.value = &bestshotmode ; //bestshot_value ;
  ctrlCmd.resp_fd = camfd;

  CDBG("bestshotmode is %d\n", bestshotmode);

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_set_BestShotMode: ioctl failed. ioctl return value is %d \n",ret);
  }
  return ret;
}

/*===========================================================================
    * FUNCTION    - native_set_FD -
    *
    * DESCRIPTION:
*==========================================================================*/
int8_t native_set_FD(int camfd, uint8_t face_detection)
{
  int ret = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_SET_PARM_FD;
  ctrlCmd.length = sizeof(uint8_t);
  ctrlCmd.value = &face_detection ;
  ctrlCmd.resp_fd = camfd;

  CDBG("face detection is set to %d\n", face_detection);

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d \n",__func__, ret);
  }
  return ret;
}


/*===========================================================================
FUNCTION     native_set_AF_ROI

DESCRIPTION
===========================================================================*/
int8_t native_set_AF_ROI(int camfd, roi_info_t roi)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_SET_PARM_AF_ROI;
  ctrlCmd.length     = sizeof(roi_info_t);
  ctrlCmd.value      = &roi;
  ctrlCmd.resp_fd = camfd;

  ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
  if (ioctlRetVal < 0) {
    CDBG("%s: ioctl failed. ioctl return value is %d\n", __func__,
      ioctlRetVal);
  }
  return ioctlRetVal;
}


/* ========================================================================
 * FUNCTION    - native_start_liveshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_start_liveshot(int camfd, void* buffer, int buffer_len,
                             uint32_t width, uint32_t height)
{
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  ret = set_liveshot_params(width, height, NULL, 0, buffer, buffer_len);
  if (FALSE == ret) {
      CDBG("native_start_liveshot: set params failed %d \n",
      ret);
      return FALSE;
  }
  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type = CAMERA_START_LIVESHOT;
  ctrlCmd.length = 0;
  ctrlCmd.value = NULL;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("native_start_liveshot: ioctl failed. ioctl return value is %d \n",
      ret);
    return FALSE;
  }

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - native_set_strobe_flash_mode -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_query_flash_for_snapshot(int camfd, int *is_flash_needed)
{
  int ret;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 1000;
  ctrlCmd.type = CAMERA_QUERY_FLASH_FOR_SNAPSHOT;
  ctrlCmd.length = sizeof(int);
  ctrlCmd.value =(void *) is_flash_needed;
  ctrlCmd.resp_fd = camfd;

  if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG("%s: ioctl failed. ioctl return: %d \n", __func__, ret);
    ret = ctrlCmd.status;
    return FALSE;
  }
  CDBG("%s: ioctl return: %d, flash =%d\n", __func__, ret, *is_flash_needed);

  return TRUE;
} /* native_set_strobe_flash_mode */

/*===========================================================================
 * FUNCTION    - native_getFocusDistances -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_getFocusDistances(int camfd,
  focus_distances_info_t *focus_distances_info)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_GET_PARM_FOCUS_DISTANCES;
  ctrlCmd.length     = sizeof(focus_distances_info_t);
  ctrlCmd.value      = (void *)focus_distances_info;
  ctrlCmd.resp_fd = camfd;

  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG_ERROR("%s: ioctl failed. ioctl return value is %d \n",
      __func__, ioctlRetVal);
  }

  CDBG("\n\n %s: Get Focus Distances = %f %f %f\n",
    __func__,
    focus_distances_info->focus_distance[FOCUS_DISTANCE_NEAR_INDEX],
    focus_distances_info->focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX],
    focus_distances_info->focus_distance[FOCUS_DISTANCE_FAR_INDEX]);

  return ioctlRetVal;

}

/*===========================================================================
 * FUNCTION    - native_getFocalLength -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_getFocalLength(int camfd,
  float *focalLength)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_GET_PARM_FOCAL_LENGTH;
  ctrlCmd.length     = sizeof(float);
  ctrlCmd.value      = (void *)focalLength;
  ctrlCmd.resp_fd = camfd;

  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG_ERROR("%s: ioctl failed. ioctl return value is %d \n",
      __func__, ioctlRetVal);
  }

  CDBG("\n\n %s: Get Focal Length = %f\n",
    __func__,
    *focalLength);

  return ioctlRetVal;
}
/*===========================================================================
 * FUNCTION    - native_getHorinzontalViewAngle -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_getHorizontalViewAngle(int camfd,
  float *viewAngle)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_GET_PARM_HORIZONTAL_VIEW_ANGLE;
  ctrlCmd.length     = sizeof(float);
  ctrlCmd.value      = (void *)viewAngle;
  ctrlCmd.resp_fd = camfd;

  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG_ERROR("%s: ioctl failed. ioctl return value is %d \n",
      __func__, ioctlRetVal);
  }

  CDBG("\n\n %s: Get View Angle Length = %f\n",
    __func__,
    *viewAngle);

  return ioctlRetVal;
}
/*===========================================================================
 * FUNCTION    - native_getVerticalViewAngle -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_getVerticalViewAngle(int camfd,
  float *viewAngle)
{
  int ioctlRetVal = TRUE;
  struct msm_ctrl_cmd ctrlCmd;

  ctrlCmd.timeout_ms = 5000;
  ctrlCmd.type       = CAMERA_GET_PARM_HORIZONTAL_VIEW_ANGLE;
  ctrlCmd.length     = sizeof(float);
  ctrlCmd.value      = (void *)viewAngle;
  ctrlCmd.resp_fd = camfd;

  if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG_ERROR("%s: ioctl failed. ioctl return value is %d \n",
      __func__, ioctlRetVal);
  }

  CDBG("\n\n %s: Get View Angle Length = %f\n",
    __func__,
    *viewAngle);

  return ioctlRetVal;
}

/*===========================================================================
 * FUNCTION    - native_resetLensToInfinity -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_resetLensToInfinity(int camfd){
    int ioctlRetVal = TRUE;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = CAMERA_SET_PARM_RESET_LENS_TO_INFINITY;
    ctrlCmd.length     = 0;
    ctrlCmd.value      = NULL;
    ctrlCmd.resp_fd    = camfd;

    if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
    CDBG_ERROR("%s: ioctl failed. ioctl return value is %d \n",
      __func__, ioctlRetVal);
    }
    return ioctlRetVal;
}

/*===========================================================================
 * FUNCTION    - native_getMetadata -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_getSnapshotdata(int camfd, snapshotData_info_t snapshotData){
    int ioctlRetVal = TRUE;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = CAMERA_GET_PARM_SNAPSHOTDATA;
    ctrlCmd.length     = 0;
    ctrlCmd.value      = NULL;
    ctrlCmd.resp_fd    = camfd;

    if ((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd))< 0) {
    CDBG_ERROR("%s: ioctl failed. ioctl return value is %d \n",
      __func__, ioctlRetVal);
    }
    return ioctlRetVal;
}

/*===========================================================================
 * FUNCTION    - native_interface_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t native_interface_init(interface_ctrl_t * intrfcCtrl, int *camfd)
{
  char device[MAX_DEV_NAME_LEN];
  snprintf(device, MAX_DEV_NAME_LEN, MSM_CAMERA_CONTROL, get_device_id());
  *camfd = open(device, O_RDWR);
  if (*camfd < 0) {
    CDBG("native_interface_init: msm_camera opened failed!\n");
    return FALSE;
  }
  CDBG("native_interface_init: camfd = %d\n",
    *camfd);

  intrfcCtrl->frameThread = cam_frame;
  intrfcCtrl->setDimension = native_set_dimension;
  intrfcCtrl->setDefaultParams = native_set_default_params;
  intrfcCtrl->registerPreviewBuf = native_register_preview_bufs;
  intrfcCtrl->unregisterPreviewBuf = native_unregister_preview_bufs;
  intrfcCtrl->registerVideoBuf = native_register_video_bufs;
  intrfcCtrl->unregisterVideoBuf = native_unregister_video_bufs;
  intrfcCtrl->startPreview = native_start_preview;
  intrfcCtrl->stopPreview = native_stop_preview;
  intrfcCtrl->startVideo = native_start_video;
  intrfcCtrl->stopVideo = native_stop_video;
  intrfcCtrl->startRecording = native_start_recording;
  intrfcCtrl->stopRecording = native_stop_recording;
  intrfcCtrl->startSnapshot = native_start_snapshot;
  intrfcCtrl->startRawSnapshot = native_start_raw_snapshot;
  intrfcCtrl->registerSnapshotBuf = native_register_snapshot_bufs;
  intrfcCtrl->getPicture = native_get_picture;
  intrfcCtrl->stopPreview = native_stop_preview;
  intrfcCtrl->stopSnapshot = native_stop_snapshot;
  intrfcCtrl->jpegEncode = native_jpeg_encode;
  intrfcCtrl->setJpegRotation = native_set_jpeg_rotation;
  intrfcCtrl->setStrobeFlashMode = native_set_strobe_flash_mode;
  intrfcCtrl->getMaxZoom = native_get_maxzoom;
  intrfcCtrl->getZoomRatio = native_get_zoomratio;
  intrfcCtrl->setZoom = native_set_zoom;
  intrfcCtrl->setSpecialEffect = native_set_special_effect;
  intrfcCtrl->setBrightness = native_set_brightness;
  intrfcCtrl->setContrast = native_set_contrast;
  intrfcCtrl->setSaturation = native_set_saturation;
  intrfcCtrl->setEV = native_set_ev;
  intrfcCtrl->setAecMode = native_set_aecmode;
  intrfcCtrl->setAecROI = native_set_aecroi;
  intrfcCtrl->setIso = native_set_iso;
  intrfcCtrl->setSharpness = native_set_sharpness;
  intrfcCtrl->setAutoFocus = native_set_afmode;
  intrfcCtrl->sethjr = native_set_hjr;
  intrfcCtrl->setLensShading = native_set_lens_shading;
  intrfcCtrl->setLedMode = native_set_led_mode;
  intrfcCtrl->setAntiBanding = native_set_AntiBanding;
  intrfcCtrl->setWhiteBalance = native_set_WhiteBalance;
  intrfcCtrl->registerRawSnapshotBuf = native_register_raw_snapshot_bufs;
  intrfcCtrl->unregisterSnapshotBuf = native_unregister_snapshot_bufs;
  intrfcCtrl->unregisterRawSnapshotBuf = native_unregister_raw_snapshot_bufs;
  intrfcCtrl->getSharpness_AF = native_get_AF_sharpness;
  intrfcCtrl->setMotionIso = native_set_motion_iso;
  intrfcCtrl->setHue = native_set_hue;
  intrfcCtrl->cancelAF = native_AF_cancel;
  intrfcCtrl->getAfStep = native_get_Af_step;
  intrfcCtrl->setAfStep = native_set_Af_step;
  intrfcCtrl->enableAFD = native_enable_AFD;
  intrfcCtrl->prepareSnapshot = native_prepare_snapshot;
  intrfcCtrl->setFpsMode = native_set_fps_mode;
  intrfcCtrl->setFps = native_set_fps;
  intrfcCtrl->setAFFocusRect = native_set_af_focus_rect;
  intrfcCtrl->setCAF = native_set_CAF;
  intrfcCtrl->enableLA = native_enable_la;
  intrfcCtrl->disableLA = native_disable_la;
  intrfcCtrl->setBacklightDetection = native_set_bl_detection;
  intrfcCtrl->setSnowSceneDetection = native_set_snow_detection;
  intrfcCtrl->setBestShotMode = native_set_BestShotMode;
  intrfcCtrl->setFaceDetection = native_set_FD;
  intrfcCtrl->setAF_ROI = native_set_AF_ROI;
  intrfcCtrl->video_rot_config = native_set_video_rot_parms;
  intrfcCtrl->video_dis_config = native_set_video_dis_parms;
  intrfcCtrl->startLiveShot = native_start_liveshot;
  intrfcCtrl->setSCEfactor = native_set_SCE_factor;
  intrfcCtrl->queryFlash4Snap = native_query_flash_for_snapshot;
  intrfcCtrl->getFocusDistances = native_getFocusDistances;
  intrfcCtrl->getFocalLength = native_getFocalLength;
  intrfcCtrl->getHorizontalViewAngle = native_getHorizontalViewAngle;
  intrfcCtrl->getVerticalViewAngle = native_getVerticalViewAngle;
  intrfcCtrl->resetLensToInfinity = native_resetLensToInfinity;
  intrfcCtrl->getSnapshotdata = native_getSnapshotdata;
  return TRUE;
}
