/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <linux/types.h>
#include <media/msm_jpeg.h>
#include "jpegd_lib_hw.h"
#include "jpegd_dbg.h"

/*============================================================================
   MACROS
============================================================================*/
//#define DUMP_OUT_YUV

/*============================================================================
   DATA STRUCTURES
============================================================================*/
typedef struct {
  int fd;
  pthread_t event_thread_id;
  unsigned char event_thread_exit;
  pthread_mutex_t event_thread_ready_mutex;
  pthread_cond_t event_thread_ready_cond;
  uint8_t event_thread_is_ready;
  jpegd_evt_handler p_evt_handler;
  void *p_user;
} jpegd_lib_t;

/*===========================================================================
 * FUNCTION    - get_format  -
 *
 * DESCRIPTION:
 *==========================================================================*/
jpegd_subsampling_t get_format(jpeg_subsampling_t ss_type)
{
  switch (ss_type) {
  case JPEG_H2V1:
    return JPEGD_H2V1;
  case JPEG_H1V2:
    return JPEGD_H1V2;
  case JPEG_H1V1:
    return JPEGD_H1V1;
  case JPEG_GRAYSCALE:
    return JPEGD_GRAYSCALE;
  default:
  case JPEG_H2V2:
    return JPEGD_H2V2;
  }
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_configure_baseline  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_lib_configure_baseline(
  void *handle,
  jpegd_cmd_quant_cfg_t *p_dqt_cfg,
  jpegd_cmd_huff_cfg_t *p_huff_cfg,
  jpegd_base_config_t *p_base_cfg)
{
  int rc;
  jpegd_lib_t *p_jpg = (jpegd_lib_t *)handle;
  uint32_t rotation = 0;
  int scale_enable = (p_base_cfg->scale_factor != SCALE_NONE);

  if ((p_base_cfg->rotation != 90) &&
    (p_base_cfg->rotation != 180) &&
    (p_base_cfg->rotation != 270)) {
    p_base_cfg->rotation = 0;
  }
  switch (p_base_cfg->rotation) {
  case 90:
    rotation = 1;
    break;
  case 180:
    rotation = 2;
    break;
  case 270:
    rotation = 3;
    break;
  default:
  case 0:
    rotation = 0;
    break;
  }
  JDDBG_HIGH("%s:%d] width=%d height=%d rotation %d format %d scale %d \
    crcb_order %d restart_int %d",
    __func__, __LINE__,
    p_base_cfg->width,
    p_base_cfg->height,
    p_base_cfg->rotation,
    p_base_cfg->format,
    p_base_cfg->scale_factor,
    p_base_cfg->crcb_order,
    p_base_cfg->restart_interval);

  /* Reset command */
  rc = jpegd_hw_reset(p_jpg->fd);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, rc);
    return JPEGDERR_EFAILED;
  }

  /* Core config regs */
  rc = jpegd_hw_core_cfg(p_jpg->fd, scale_enable);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, rc);
    return JPEGDERR_EFAILED;
  }

  /* Fetch Engine regs */
  jpegd_cmd_fe_cfg fe_cfg = {
    0, //flip
    0, //rotation
    1, //mal_en
    2, //mal_boundary
    1, //plane0_enable
    0, //plane1_enable
    0, //plane2_enable
    1, //read_enable
    0, //swc_fetch_enable
    1, //bottom_vpad_enable
    0, //cbcr_order
    3, //memory_format
    2, //burst_length_max8
    0, //byte_ordering
  };
  rc = jpegd_hw_fe_cfg(p_jpg->fd, &fe_cfg);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, rc);
    return JPEGDERR_EFAILED;
  }

  /* Write Engine regs */
  jpegd_cmd_we_cfg we_cfg = {
    0, //flip
    0, //rotation
    0, //pop_buff_on_eos
    1, //mal_enable
    2, //mal_boundary
    0, //pln2_enable
    1, //pln1_enable
    1, //pln0_enable
    0, //cbcr_order
    1, //memory_format
    5, //burst_length_max
    0, //byte_ordering
    0, //jpegdFormat
  };
  we_cfg.rotation = rotation;
  we_cfg.cbcr_order = p_base_cfg->crcb_order;
  we_cfg.jpegdFormat = get_format(p_base_cfg->format);
  switch (p_base_cfg->num_planes) {
  case 3:
    we_cfg.memory_format = 0x0;
    we_cfg.pln2_enable = 1;
    break;
  case 1:
    we_cfg.memory_format = 0x2;
    we_cfg.pln1_enable = 0;
    we_cfg.pln2_enable = 0;
    break;
  default:
  case 2:
    we_cfg.memory_format = 0x1;
    break;
  }

  jpegd_image_info_t img_info = {
    p_base_cfg->width, //image_width
    p_base_cfg->height, //image_height
    p_base_cfg->actual_width, //actual image_width
    p_base_cfg->actual_height, //actual image_height
  };
  rc = jpegd_hw_we_cfg(p_jpg->fd, &we_cfg, &img_info,
    p_base_cfg->scale_factor);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, rc);
    return JPEGDERR_EFAILED;
  }

  if (scale_enable) {
    rc = jpegd_hw_scale_core_cfg(p_jpg->fd);
    if (rc) {
      JDDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, rc);
      return JPEGDERR_EFAILED;
    }
  }

  /* Decode regs */
  rc = jpegd_hw_decode_cfg(p_jpg->fd, 0, p_base_cfg->height,
    p_base_cfg->width, p_base_cfg->format, p_base_cfg->restart_interval);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, rc);
    return JPEGDERR_EFAILED;
  }

  /* Scale regs */
  rc = jpegd_hw_scaling_config(p_jpg->fd, p_base_cfg->scale_factor);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, rc);
    return JPEGDERR_EFAILED;
  }

  /* DMI regs */
  /* DQT table */
  rc = jpegd_hw_jpeg_dqt(p_jpg->fd, p_dqt_cfg);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error Configuring DQT", __func__, __LINE__);
    return JPEGDERR_EFAILED;
  }

  /* Huffman table */
  rc = jpegd_hw_dht_code_config(p_jpg->fd, p_huff_cfg);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, rc);
    return JPEGDERR_EFAILED;
  }

  rc = jpegd_hw_huff_config(p_jpg->fd, p_huff_cfg);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, rc);
    return JPEGDERR_EFAILED;
  }
  return JPEGDERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_output_buf_cfg  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_lib_output_buf_cfg(void *handle, jpegd_buf *buf)
{
  int rc;
  jpegd_lib_t *p_jpg = (jpegd_lib_t *)handle;
  struct msm_jpeg_buf msm_buf;
  memset(&msm_buf, 0x0, sizeof(struct msm_jpeg_buf));

  msm_buf.type = buf->type;
  msm_buf.vaddr = buf->vaddr;
  msm_buf.fd = buf->fd;

  msm_buf.y_off = buf->y_off;
  msm_buf.y_len = buf->y_len;

  msm_buf.cbcr_off = buf->cbcr_off;
  msm_buf.cbcr_len = buf->cbcr_len;

  msm_buf.pln2_off = buf->pln2_off;
  msm_buf.pln2_len = buf->pln2_len;

  JDDBG_HIGH("%s:%d]:vaddr=0x%0x, y_off=0x%0x, y_len=0x%0x \
    cbcr_off=0x%0x, cbcr_len=0x%0x pln2_off=0x%0x, pln2_len=0x%0x",
    __func__, __LINE__,
    (uint32_t)msm_buf.vaddr, msm_buf.y_off, msm_buf.y_len,
    msm_buf.cbcr_off, msm_buf.cbcr_len, msm_buf.pln2_off,
    msm_buf.pln2_len);

  rc = ioctl(p_jpg->fd, MSM_JPEG_IOCTL_OUTPUT_BUF_ENQUEUE, &msm_buf);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] error %d", __func__, __LINE__, errno);
    return JPEGDERR_EFAILED;
  }

  return JPEGDERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_input_buf_cfg  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_lib_input_buf_cfg(void *handle, jpegd_buf *buf)
{
  int rc;
  jpegd_lib_t *p_jpg = (jpegd_lib_t *)handle;
  struct msm_jpeg_buf msm_buf;
  memset(&msm_buf, 0x0, sizeof(struct msm_jpeg_buf));

  msm_buf.type = buf->type;
  msm_buf.vaddr = buf->vaddr;
  msm_buf.fd = buf->fd;

  msm_buf.y_off = buf->y_off;
  msm_buf.y_len = buf->y_len;

  msm_buf.cbcr_len = 0;
  msm_buf.offset = buf->offset;

  JDDBG_MED("%s:%d] :E", __func__, __LINE__);

  rc = jpegd_hw_input_len_cfg(p_jpg->fd, buf->y_len);
  if (rc) {
    JDDBG_MED("%s:%d] rc %d", __func__, __LINE__, rc);
    return JPEGDERR_EFAILED;
  }

  JDDBG_HIGH("%s:%d] vaddr=0x%0x, y_ff=0x%0x, y_len=0x%0x", __func__,
    __LINE__, (uint32_t)msm_buf.vaddr, (uint32_t)msm_buf.y_off,
    (uint32_t)msm_buf.y_len);

  rc = ioctl(p_jpg->fd, MSM_JPEG_IOCTL_INPUT_BUF_ENQUEUE, &msm_buf);
  if (rc < 0) {
    JDDBG_ERROR("%s:%d] error %d", __func__, __LINE__, errno);
    return JPEGDERR_EFAILED;
  }
  JDDBG_MED("%s:%d] :X", __func__, __LINE__);

  return JPEGDERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_output_buf_get  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_lib_output_buf_get(void *handle, jpegd_buf *buf)
{
  int rc;
  jpegd_lib_t *p_jpg = (jpegd_lib_t *)handle;
  struct msm_jpeg_buf msm_buf;
  memset(&msm_buf, 0x0, sizeof(struct msm_jpeg_buf));


  rc = ioctl(p_jpg->fd, MSM_JPEG_IOCTL_OUTPUT_GET, &msm_buf);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] error %d", __func__, __LINE__, errno);
    return JPEGDERR_EFAILED;
  }

#ifdef DUMP_OUT_YUV
  char *output_filename = "/data/dec1.yuv";
  FILE *fout = fopen(output_filename, "wb");
  if(!fout){
     JDDBG_HIGH("%s:%d] failed to open output file: %s",
       __func__, __LINE__, output_filename);
  } else {
    fwrite(msm_buf.vaddr, 1, msm_buf.y_len + msm_buf.cbcr_len
      msm_buf.pln2_len, fout);
    JDDBG_HIGH("%s:%d] -- Image dumped ", __func__, __LINE__);
    fclose(fout);
  }
#endif
  JDDBG_MED("%s:%d] :E", __func__, __LINE__);
  return JPEGDERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_input_buf_get  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_lib_input_buf_get(void *handle, jpegd_buf *buf)
{
  int rc;
  jpegd_lib_t *p_jpg = (jpegd_lib_t *)handle;
  struct msm_jpeg_buf msm_buf;

  JDDBG_MED("%s:%d] ", __func__, __LINE__);

  rc = ioctl(p_jpg->fd, MSM_JPEG_IOCTL_INPUT_GET, &msm_buf);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] error %d", __func__, __LINE__, errno);
    return JPEGDERR_EFAILED;
  }
  JDDBG_MED("%s:%d] ", __func__, __LINE__);
  return JPEGDERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_event_get_unblock  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_lib_event_get_unblock(jpegd_lib_t *p_jpg)
{
  struct msm_jpeg_hw_cmd hw_cmd;
  int rc;

  JDDBG_MED("%s:%d] ", __func__, __LINE__);
  rc = ioctl(p_jpg->fd, MSM_JPEG_IOCTL_EVT_GET_UNBLOCK, &hw_cmd);
  if (rc) {
    JDDBG_ERROR("%s:%d] error %d", __func__, __LINE__, errno);
    return JPEGDERR_EFAILED;
  }
  JDDBG_MED("%s:%d] ", __func__, __LINE__);
  return JPEGDERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_decode  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_lib_decode(void *handle)
{
  int rc = JPEGDERR_SUCCESS;
  jpegd_lib_t *p_jpg = (jpegd_lib_t *)handle;

  JDDBG_MED("%s:%d] ", __func__, __LINE__);

  rc = jpegd_hw_decode(p_jpg->fd);
  return rc;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_send_thread_ready  -
 *
 * DESCRIPTION:
 *==========================================================================*/
void jpegd_lib_wait_thread_ready(jpegd_lib_t *p_jpg, pthread_t* thread_id)
{
  if (*thread_id == p_jpg->event_thread_id) {
    pthread_mutex_lock(&p_jpg->event_thread_ready_mutex);
    JDDBG_MED("%s:%d], event thread ready %d", __func__, __LINE__,
      p_jpg->event_thread_is_ready);
    if (!p_jpg->event_thread_is_ready) {
      pthread_cond_wait(&p_jpg->event_thread_ready_cond,
         &p_jpg->event_thread_ready_mutex);
    }
    JDDBG_MED("%s:%d], event thread ready wait done %d", __func__,
      __LINE__, p_jpg->event_thread_is_ready);
    p_jpg->event_thread_is_ready = 0;
    pthread_mutex_unlock(&p_jpg->event_thread_ready_mutex);
  }
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_send_thread_ready  -
 *
 * DESCRIPTION:
 *==========================================================================*/
void jpegd_lib_send_thread_ready (jpegd_lib_t *p_jpg, pthread_t* thread_id)
{
  if (*thread_id == p_jpg->event_thread_id) {
    pthread_mutex_lock (&p_jpg->event_thread_ready_mutex);
    p_jpg->event_thread_is_ready = 1;
    pthread_cond_signal (&p_jpg->event_thread_ready_cond);
    pthread_mutex_unlock (&p_jpg->event_thread_ready_mutex);
  }
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_wait_done  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_lib_wait_done(void* handle)
{
  jpegd_lib_t *p_jpg = (jpegd_lib_t *)handle;

  if (!pthread_equal(p_jpg->event_thread_id, pthread_self())) {
    JDDBG_MED ("%s:%d] event_thread_id %d",
      __func__, __LINE__, (int)p_jpg->event_thread_id);
    jpegd_lib_wait_thread_ready(p_jpg, &(p_jpg->event_thread_id));
  }
  JDDBG_MED ("%s:%d] ", __func__, __LINE__);
  return JPEGDERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_event_thread  -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *jpegd_lib_event_thread (void *data)
{
  jpegd_lib_t *p_jpg = (jpegd_lib_t *)data;
  struct msm_jpeg_ctrl_cmd ctrlCmd;
  int rc = JPEGDERR_SUCCESS;
  jpegd_event_t event;

  JDDBG_MED("%s:%d] Enter threadid %ld", __func__, __LINE__,
    p_jpg->event_thread_id);
  jpegd_lib_send_thread_ready (p_jpg, &(p_jpg->event_thread_id));

  p_jpg->event_thread_exit = 0;

  do {
    JDDBG_MED("%s:%d] Event loop", __func__, __LINE__);

    rc = ioctl(p_jpg->fd, MSM_JPEG_IOCTL_EVT_GET, &ctrlCmd);
    JDDBG_MED("%s:%d] type %d rc %d", __func__, __LINE__, ctrlCmd.type, rc);

    if (rc < 0) {
      JDDBG_MED("%s:%d] ioctl unblock %d", __func__, __LINE__, rc);
    } else if (ctrlCmd.type == MSM_JPEG_EVT_SESSION_DONE) {
      JDDBG_MED("%s:%d] MSM_JPEG_EVT_SESSION_DONE", __func__, __LINE__);
      if (p_jpg->p_evt_handler) {
        event.type = JPEGD_EVT_FRAMEDONE;
        p_jpg->p_evt_handler(p_jpg->p_user, &event);
      }
    } else if (ctrlCmd.type == MSM_JPEG_EVT_ERR) {
      JDDBG_MED("%s:%d] MSM_JPEG_EVT_ERR", __func__, __LINE__);
      if (p_jpg->p_evt_handler) {
        event.type = JPEGD_EVT_ERROR;
        p_jpg->p_evt_handler(p_jpg->p_user, &event);
      }
    } else if (ctrlCmd.type == MSM_JPEG_EVT_RESET) {
      JDDBG_MED("%s:%d] MSM_JPEG_EVT_RESET", __func__, __LINE__);
    }

    jpegd_lib_send_thread_ready (p_jpg, &(p_jpg->event_thread_id));
  } while (!p_jpg->event_thread_exit);
  JDDBG_MED("%s:%d] Exit", __func__, __LINE__);
  return NULL;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_init  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_lib_init(void **handle, void *p_user, jpegd_evt_handler evt_handler)
{
  jpegd_lib_t *p_jpg = NULL;
  int rc = 0;

  p_jpg = (jpegd_lib_t *)malloc(sizeof(jpegd_lib_t));
  if (!p_jpg) {
    JDDBG_ERROR("%s:%d] no mem", __func__, __LINE__);
    return -ENOMEM;
  }
  memset(p_jpg, 0, sizeof(jpegd_lib_t));

  p_jpg->fd = open(MSM_JPEGD_NAME, O_RDWR);
  if (p_jpg->fd < 0) {
    JDDBG_ERROR ("%s:%d] Error opening device handle %s",
      __func__, __LINE__, MSM_JPEGD_NAME);
    goto error;
  }

  p_jpg->p_evt_handler = evt_handler;

  pthread_mutex_init(&p_jpg->event_thread_ready_mutex, NULL);
  pthread_cond_init(&p_jpg->event_thread_ready_cond, NULL);
  p_jpg->event_thread_is_ready = 0;

  JDDBG_MED("%s:%d] wait ", __func__, __LINE__);
  pthread_mutex_lock(&p_jpg->event_thread_ready_mutex);
  rc = pthread_create(&p_jpg->event_thread_id, NULL,
    jpegd_lib_event_thread, p_jpg);
  if (rc < 0) {
    JDDBG_ERROR ("%s:%d] event thread creation failed", __func__, __LINE__);
    pthread_mutex_unlock(&p_jpg->event_thread_ready_mutex);
    goto error;
  }
  pthread_mutex_unlock(&p_jpg->event_thread_ready_mutex);

  JDDBG_MED ("%s:%d] jpegd create all threads success", __func__, __LINE__);
  jpegd_lib_wait_done(p_jpg);
  JDDBG_MED ("%s:%d] jpegd after starting all threads", __func__, __LINE__);
  p_jpg->p_user = p_user;
  *handle = p_jpg;
  return JPEGDERR_SUCCESS;

error:
  if (p_jpg) {
    free(p_jpg);
  }
  return JPEGDERR_EFAILED;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_release  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_lib_release(void* handle)
{
  jpegd_lib_t *p_jpg = (jpegd_lib_t *)handle;
  int rc = JPEGDERR_SUCCESS;

  JDDBG_MED("%s:%d] ", __func__, __LINE__);

  p_jpg->event_thread_exit = 1;

  rc = ioctl(p_jpg->fd, MSM_JPEG_IOCTL_EVT_GET_UNBLOCK);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, errno);
    return JPEGDERR_EFAILED;
  }
  JDDBG_MED("%s:%d] threadid %d", __func__, __LINE__,
    (int)p_jpg->event_thread_id);

  if(!pthread_equal(p_jpg->event_thread_id, pthread_self())) {
    pthread_join (p_jpg->event_thread_id, NULL);
    JDDBG_MED("%s:%d] ", __func__, __LINE__);
  } else {
    JDDBG_ERROR("%s:%d] Error deadlock", __func__, __LINE__);
    return JPEGDERR_EFAILED;
  }

  rc = close(p_jpg->fd);
  p_jpg->fd = -1;

  pthread_mutex_destroy(&p_jpg->event_thread_ready_mutex);
  pthread_cond_destroy(&p_jpg->event_thread_ready_cond);

  JDDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return rc;
}
