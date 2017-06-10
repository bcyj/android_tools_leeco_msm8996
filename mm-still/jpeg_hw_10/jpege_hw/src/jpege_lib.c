/*******************************************************************************
*                                                                         .
* Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential                      .
*                                                                         .
*******************************************************************************/

#include <sys/ioctl.h>
#include <stdint.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <pthread.h>

#include <linux/msm_ion.h>
#include "jpege_lib.h"
#include "jpege_lib_hw.h"
#include "jpeg_lib_hw_reg.h"

#include "jpeg_hw_dbg.h"
#define HW_INPUT_SIZE 607;
#define MSM_JPEGE_NAME "/dev/jpeg0"

#define JPEG_CLK_RATE_NORMAL (266670000)
#define JPEG_CLK_RATE_HIGH (320000000)

static const char* jpeg_devs[] = {"/dev/jpeg0", "/dev/jpeg1", NULL};

typedef struct
{
  int jpegefd;
  int (*jpege_hw_lib_event_handler) (void *,
    struct jpege_hw_evt *, int);
  int (*jpege_hw_lib_input_handler) (void *, struct jpege_hw_buf *);
  int (*jpege_hw_lib_output_handler) (void *, struct jpege_hw_buf *);

  pthread_t event_thread_id;
  unsigned char event_thread_exit;
  pthread_mutex_t event_thread_ready_mutex;
  pthread_cond_t event_thread_ready_cond;
  uint8_t event_thread_is_ready;

  pthread_t input_thread_id;
  unsigned char input_thread_exit;
  pthread_mutex_t input_thread_ready_mutex;
  pthread_cond_t input_thread_ready_cond;
  uint8_t input_thread_is_ready;

  pthread_t output_thread_id;
  unsigned char output_thread_exit;
  pthread_mutex_t output_thread_ready_mutex;
  pthread_cond_t output_thread_ready_cond;
  uint8_t output_thread_is_ready;

  void *p_userdata;
  uint32_t version;

} __jpege_hw_obj_t;

void jpege_hw_lib_wait_thread_ready (__jpege_hw_obj_t * jpege_hw_obj_p,
  pthread_t* thread_id)
{
  JPEG_HW_DBG("%s:%d], thread_id %d", __func__, __LINE__, (int) *thread_id);
  if (*thread_id == jpege_hw_obj_p->event_thread_id) {
    pthread_mutex_lock (&jpege_hw_obj_p->event_thread_ready_mutex);
    JPEG_HW_DBG("%s:%d], event thread ready %d", __func__, __LINE__,
      jpege_hw_obj_p->event_thread_is_ready);
    if (!jpege_hw_obj_p->event_thread_is_ready){
      pthread_cond_wait (&jpege_hw_obj_p->event_thread_ready_cond,
       &jpege_hw_obj_p->event_thread_ready_mutex);
    }
    jpege_hw_obj_p->event_thread_is_ready = 0;
    pthread_mutex_unlock (&jpege_hw_obj_p->event_thread_ready_mutex);
  } else if (*thread_id == jpege_hw_obj_p->input_thread_id) {
    pthread_mutex_lock (&jpege_hw_obj_p->input_thread_ready_mutex);
    JPEG_HW_DBG("%s:%d], ready %d", __func__, __LINE__,
      jpege_hw_obj_p->input_thread_is_ready);
    if (!jpege_hw_obj_p->input_thread_is_ready)
      pthread_cond_wait (&jpege_hw_obj_p->input_thread_ready_cond,
        &jpege_hw_obj_p->input_thread_ready_mutex);
    jpege_hw_obj_p->input_thread_is_ready = 0;
    pthread_mutex_unlock (&jpege_hw_obj_p->input_thread_ready_mutex);
  } else if (*thread_id == jpege_hw_obj_p->output_thread_id) {
    pthread_mutex_lock (&jpege_hw_obj_p->output_thread_ready_mutex);
    JPEG_HW_DBG("%s:%d], ready %d", __func__, __LINE__,
      jpege_hw_obj_p->output_thread_is_ready);
    if (!jpege_hw_obj_p->output_thread_is_ready)
      pthread_cond_wait (&jpege_hw_obj_p->output_thread_ready_cond,
        &jpege_hw_obj_p->output_thread_ready_mutex);
    jpege_hw_obj_p->output_thread_is_ready = 0;
    pthread_mutex_unlock (&jpege_hw_obj_p->output_thread_ready_mutex);
  }
  JPEG_HW_DBG("%s:%d] thread_id %d done", __func__, __LINE__,
    (int) *thread_id);
}

void jpege_hw_lib_send_thread_ready (__jpege_hw_obj_t * jpege_hw_obj_p,
  pthread_t* thread_id)
{
  JPEG_HW_DBG("%s:%d], thread_id %d", __func__, __LINE__, (int) *thread_id);
  if (*thread_id == jpege_hw_obj_p->event_thread_id) {
    pthread_mutex_lock (&jpege_hw_obj_p->event_thread_ready_mutex);
    jpege_hw_obj_p->event_thread_is_ready = 1;
    pthread_cond_signal (&jpege_hw_obj_p->event_thread_ready_cond);
    pthread_mutex_unlock (&jpege_hw_obj_p->event_thread_ready_mutex);
  } else if (*thread_id == jpege_hw_obj_p->input_thread_id) {
    pthread_mutex_lock (&jpege_hw_obj_p->input_thread_ready_mutex);
    jpege_hw_obj_p->input_thread_is_ready = 1;
    pthread_cond_signal (&jpege_hw_obj_p->input_thread_ready_cond);
    pthread_mutex_unlock (&jpege_hw_obj_p->input_thread_ready_mutex);
  } else if (*thread_id == jpege_hw_obj_p->output_thread_id) {
    pthread_mutex_lock (&jpege_hw_obj_p->output_thread_ready_mutex);
    jpege_hw_obj_p->output_thread_is_ready = 1;
    pthread_cond_signal (&jpege_hw_obj_p->output_thread_ready_cond);
    pthread_mutex_unlock (&jpege_hw_obj_p->output_thread_ready_mutex);
  }
  JPEG_HW_DBG("%s:%d], thread_id %d done", __func__, __LINE__,
    (int) *thread_id);
}

int jpege_lib_get_event(jpege_hw_obj_t jpege_hw_obj,
  struct jpege_hw_evt *p_event)
{
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *)jpege_hw_obj;
  int jpegefd = jpege_hw_obj_p->jpegefd;
  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;
  int result = 0;

  result = ioctl (jpegefd, MSM_JPEG_IOCTL_EVT_GET,
    &jpegeCtrlCmd);
  JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_EVT_GET rc = %d",
    __func__, __LINE__, result);
  if (result) {
    JPEG_HW_DBG("%s:%d] cannot get event", __func__, __LINE__);
  } else {
    p_event->type = jpegeCtrlCmd.type;
    p_event->len  = jpegeCtrlCmd.len;
    p_event->value = jpegeCtrlCmd.value;
  }
  return 0;
}

int jpege_lib_get_input(jpege_hw_obj_t jpege_hw_obj, struct jpege_hw_buf *p_buf)
{
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *)jpege_hw_obj;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  int result = 0;
  struct msm_jpeg_buf msm_buf;

  result = ioctl (jpegefd, MSM_JPEG_IOCTL_INPUT_GET, &msm_buf);
  JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_INPUT_GET rc = %d", __func__,
    __LINE__, result);
  if (result) {
    JPEG_HW_DBG("%s:%d] cannot get input", __func__, __LINE__);
  } else {
    p_buf->type = msm_buf.type;
    p_buf->fd   = msm_buf.fd;
    p_buf->vaddr = msm_buf.vaddr;
    p_buf->y_off         = msm_buf.y_off;
    p_buf->y_len         = msm_buf.y_len;
    p_buf->framedone_len = msm_buf.framedone_len;
    p_buf->cbcr_off = msm_buf.cbcr_off;
    p_buf->cbcr_len = msm_buf.cbcr_len;
    p_buf->num_of_mcu_rows = msm_buf.num_of_mcu_rows;
  }
  return 0;
}

int jpege_lib_get_output(jpege_hw_obj_t jpege_hw_obj, struct jpege_hw_buf *p_buf)
{
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *)jpege_hw_obj;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  int result = 0;
  struct msm_jpeg_buf msm_buf;

  result = ioctl (jpegefd, MSM_JPEG_IOCTL_OUTPUT_GET, &msm_buf);
  JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_OUTPUT_GET rc = %d", __func__,
    __LINE__, result);
  if (result) {
    JPEG_HW_DBG("%s:%d] cannot get output", __func__, __LINE__);
  } else {
    p_buf->type = msm_buf.type;
    p_buf->fd   = msm_buf.fd;
    p_buf->vaddr = msm_buf.vaddr;
    p_buf->y_off         = msm_buf.y_off;
    p_buf->y_len         = msm_buf.y_len;
    p_buf->framedone_len = msm_buf.framedone_len;
    p_buf->cbcr_off = msm_buf.cbcr_off;
    p_buf->cbcr_len = msm_buf.cbcr_len;
    p_buf->num_of_mcu_rows = msm_buf.num_of_mcu_rows;
  }
  return 0;
}

void *jpege_hw_lib_event_thread (void *context)
{
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *) context;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;
  struct jpege_hw_evt gmnEvt;
  int result = 0;

  JPEG_HW_DBG("%s:%d] Enter threadid %ld", __func__, __LINE__,
    jpege_hw_obj_p->event_thread_id);
  jpege_hw_lib_send_thread_ready (jpege_hw_obj_p, &(jpege_hw_obj_p->event_thread_id));

  do {
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_EVT_GET,
      &jpegeCtrlCmd);
    JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_EVT_GET rc = %d",
      __func__, __LINE__, result);
    if (result) {
      if (!jpege_hw_obj_p->event_thread_exit) {
        JPEG_HW_DBG("%s:%d] fail", __func__, __LINE__);
      }
    } else {
      gmnEvt.type = jpegeCtrlCmd.type;
      gmnEvt.len  = jpegeCtrlCmd.len;
      gmnEvt.value = jpegeCtrlCmd.value;
      jpege_hw_obj_p->jpege_hw_lib_event_handler (jpege_hw_obj_p->p_userdata,
        &gmnEvt,
        gmnEvt.type);
    }
    jpege_hw_lib_send_thread_ready (jpege_hw_obj_p,
      &(jpege_hw_obj_p->event_thread_id));
  } while (!jpege_hw_obj_p->event_thread_exit);

  JPEG_HW_DBG("%s:%d] Exit", __func__, __LINE__);
  return NULL;
}

void *jpege_hw_lib_output_thread (void *context)
{
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *) context;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  int result = 0;
  struct msm_jpeg_buf msm_buf;
  struct jpege_hw_buf buf;

  JPEG_HW_DBG("%s:%d] Enter threadid %ld", __func__, __LINE__,
    jpege_hw_obj_p->output_thread_id);
  jpege_hw_lib_send_thread_ready (jpege_hw_obj_p, &(jpege_hw_obj_p->output_thread_id));

  do {
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_OUTPUT_GET, &msm_buf);
    JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_OUTPUT_GET rc = %d", __func__,
      __LINE__, result);
    if (result) {
      if (!jpege_hw_obj_p->output_thread_exit) {
        JPEG_HW_DBG("%s:%d] fail", __func__, __LINE__);
      }
    } else {
      buf.type = msm_buf.type;
      buf.fd   = msm_buf.fd;

      buf.vaddr = msm_buf.vaddr;

      buf.y_off         = msm_buf.y_off;
      buf.y_len         = msm_buf.y_len;
      buf.framedone_len = msm_buf.framedone_len;

      buf.cbcr_off = msm_buf.cbcr_off;
      buf.cbcr_len = msm_buf.cbcr_len;

      buf.num_of_mcu_rows = msm_buf.num_of_mcu_rows;
      JPEG_HW_DBG("%s:%d] framedone_len %d", __func__, __LINE__,
        buf.framedone_len);
      jpege_hw_obj_p->jpege_hw_lib_output_handler(jpege_hw_obj_p->p_userdata,
        &buf);
    }
    jpege_hw_lib_send_thread_ready (jpege_hw_obj_p,
      &(jpege_hw_obj_p->output_thread_id));
  } while (!jpege_hw_obj_p->output_thread_exit);

  JPEG_HW_DBG("%s:%d] Exit", __func__, __LINE__);
  return NULL;
}

void *jpege_lib_input_thread (void *context)
{
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *) context;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  int result = 0;
  struct msm_jpeg_buf msm_buf;
  struct jpege_hw_buf buf;

  JPEG_HW_DBG("%s:%d] Enter threadid %ld", __func__, __LINE__,
    jpege_hw_obj_p->input_thread_id);
  jpege_hw_lib_send_thread_ready (jpege_hw_obj_p, &(jpege_hw_obj_p->input_thread_id));

  do {
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_INPUT_GET, &msm_buf);
    JPEG_HW_DBG("%s:%d] MSM_JPEG_IOCTL_INPUT_GET rc = %d", __func__,
      __LINE__, result);
    if (result) {
      if (!jpege_hw_obj_p->input_thread_exit) {
        JPEG_HW_DBG("%s:%d] fail", __func__, __LINE__);
      }
    } else {
      buf.type = msm_buf.type;
      buf.fd   = msm_buf.fd;

      buf.vaddr = msm_buf.vaddr;

      buf.y_off         = msm_buf.y_off;
      buf.y_len         = msm_buf.y_len;
      buf.framedone_len = msm_buf.framedone_len;

      buf.cbcr_off = msm_buf.cbcr_off;
      buf.cbcr_len = msm_buf.cbcr_len;

      buf.num_of_mcu_rows = msm_buf.num_of_mcu_rows;

      jpege_hw_obj_p->jpege_hw_lib_input_handler (jpege_hw_obj_p->p_userdata,
        &buf);
    }
    jpege_hw_lib_send_thread_ready (jpege_hw_obj_p,
      &(jpege_hw_obj_p->input_thread_id));
  } while (!jpege_hw_obj_p->input_thread_exit);

  JPEG_HW_DBG("%s:%d] Exit", __func__, __LINE__);
  return NULL;
}

int jpege_lib_init (jpege_hw_obj_t *jpege_hw_obj,
  void *p_userdata,
  int (*event_handler) (jpege_hw_obj_t,
  struct jpege_hw_evt *,
  int event),
  int (*output_handler) (jpege_hw_obj_t, struct jpege_hw_buf *),
  int (*input_handler) (jpege_hw_obj_t, struct jpege_hw_buf *))
{
  __jpege_hw_obj_t *jpege_hw_obj_p;
  int jpegefd = -1;

  int result;
  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;
  char **p;

  jpege_hw_obj_p = malloc (sizeof (__jpege_hw_obj_t));
  if (!jpege_hw_obj_p) {
    JPEG_HW_PR_ERR ("%s:%d] no mem", __func__, __LINE__);
    return -1;
  }
  memset (jpege_hw_obj_p, 0, sizeof (__jpege_hw_obj_t));

  for (p = jpeg_devs; *p; p++ ) {
    jpegefd = open (*p, O_RDWR);
    JPEG_HW_PR_ERR("open %s: fd = %d", MSM_JPEGE_NAME, jpegefd);
    if (jpegefd < 0) {
      JPEG_HW_PR_ERR ("Cannot open %s", MSM_JPEGE_NAME);
    } else {
      break;
    }
  }

  if (!*p) {
    goto jpege_init_err;
  }

  jpege_hw_obj_p->jpege_hw_lib_event_handler = event_handler;
  jpege_hw_obj_p->jpege_hw_lib_input_handler = input_handler;
  jpege_hw_obj_p->jpege_hw_lib_output_handler = output_handler;
  jpege_hw_obj_p->jpegefd = jpegefd;
  jpege_hw_obj_p->p_userdata = p_userdata;

  pthread_mutex_init (&jpege_hw_obj_p->event_thread_ready_mutex, NULL);
  pthread_cond_init (&jpege_hw_obj_p->event_thread_ready_cond, NULL);
  jpege_hw_obj_p->event_thread_is_ready = 0;

  pthread_mutex_init (&jpege_hw_obj_p->input_thread_ready_mutex, NULL);
  pthread_cond_init (&jpege_hw_obj_p->input_thread_ready_cond, NULL);
  jpege_hw_obj_p->input_thread_is_ready = 0;

  pthread_mutex_init (&jpege_hw_obj_p->output_thread_ready_mutex, NULL);
  pthread_cond_init (&jpege_hw_obj_p->output_thread_ready_cond, NULL);
  jpege_hw_obj_p->output_thread_is_ready = 0;

  JPEG_HW_PR_ERR("%s:%d] handler %p %p %p", __func__, __LINE__,
    event_handler, input_handler, output_handler);
  if (event_handler) {
    pthread_mutex_lock(&jpege_hw_obj_p->event_thread_ready_mutex);
      result = pthread_create (&jpege_hw_obj_p->event_thread_id, NULL,
        jpege_hw_lib_event_thread, jpege_hw_obj_p);
    if (result < 0) {
      JPEG_HW_PR_ERR ("%s event thread creation failed", __func__);
      pthread_mutex_unlock(&jpege_hw_obj_p->event_thread_ready_mutex);
      goto jpege_init_err;
    }
    pthread_mutex_unlock(&jpege_hw_obj_p->event_thread_ready_mutex);
  }
  if (input_handler) {
    pthread_mutex_lock(&jpege_hw_obj_p->input_thread_ready_mutex);
    result = pthread_create (&jpege_hw_obj_p->input_thread_id, NULL,
      jpege_lib_input_thread, jpege_hw_obj_p);
    if (result < 0) {
      JPEG_HW_PR_ERR ("%s input thread creation failed", __func__);
      pthread_mutex_unlock(&jpege_hw_obj_p->input_thread_ready_mutex);
      goto jpege_init_err;
    }
    pthread_mutex_unlock(&jpege_hw_obj_p->input_thread_ready_mutex);
  }
  if (output_handler) {
    pthread_mutex_lock(&jpege_hw_obj_p->output_thread_ready_mutex);
    result = pthread_create (&jpege_hw_obj_p->output_thread_id, NULL,
    jpege_hw_lib_output_thread, jpege_hw_obj_p);
    if (result < 0) {
      JPEG_HW_PR_ERR ("%s output thread creation failed", __func__);
      pthread_mutex_unlock(&jpege_hw_obj_p->output_thread_ready_mutex);
      goto jpege_init_err;
    }
    pthread_mutex_unlock(&jpege_hw_obj_p->output_thread_ready_mutex);
  }

  if (event_handler || output_handler || input_handler) {
    JPEG_HW_DBG("jpeg create all threads success");
    jpege_lib_wait_done(jpege_hw_obj_p);
    JPEG_HW_DBG("jpeg after starting all threads");
  } else {
    JPEG_HW_DBG("%s:%d] Successful", __func__, __LINE__);
  }
  *jpege_hw_obj = jpege_hw_obj_p;
  return jpegefd;
jpege_init_err:
  if (jpege_hw_obj_p) {
    free (jpege_hw_obj_p);
  }
  return -1;
}

int jpege_lib_release(jpege_hw_obj_t jpege_hw_obj)
{
  int result;
  if (!jpege_hw_obj) {
    JPEG_HW_PR_ERR("%s:%d]jpege object is NULL in release",
      __func__, __LINE__);
    return -EINVALID;
  }
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *) jpege_hw_obj;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  jpege_hw_obj_p->event_thread_exit = 1;
  jpege_hw_obj_p->input_thread_exit = 1;
  jpege_hw_obj_p->output_thread_exit = 1;

  if (jpege_hw_obj_p->jpege_hw_lib_event_handler) {
    ioctl (jpegefd, MSM_JPEG_IOCTL_EVT_GET_UNBLOCK);
    JPEG_HW_DBG("%s:%d] pthread_join: event_thread", __func__,
      __LINE__);
    if (pthread_join (jpege_hw_obj_p->event_thread_id, NULL) != 0) {
      JPEG_HW_DBG("%s: failed %d", __func__, __LINE__);
    }
  }

  if (jpege_hw_obj_p->jpege_hw_lib_input_handler) {
    ioctl (jpegefd, MSM_JPEG_IOCTL_INPUT_GET_UNBLOCK);
    JPEG_HW_DBG("%s:%d] pthread_join: input_thread", __func__,
      __LINE__);

    if (pthread_join (jpege_hw_obj_p->input_thread_id, NULL) != 0) {
      JPEG_HW_DBG("%s: failed %d", __func__, __LINE__);
    }

  }

  if (jpege_hw_obj_p->jpege_hw_lib_output_handler) {
    ioctl (jpegefd, MSM_JPEG_IOCTL_OUTPUT_GET_UNBLOCK);
    JPEG_HW_DBG("%s:%d] pthread_join: output_thread", __func__,
      __LINE__);
    if (pthread_join (jpege_hw_obj_p->output_thread_id, NULL) != 0) {
      JPEG_HW_DBG("%s: failed %d", __func__, __LINE__);
    }
  }

  result = close (jpegefd);

  pthread_mutex_destroy (&jpege_hw_obj_p->event_thread_ready_mutex);
  pthread_cond_destroy (&jpege_hw_obj_p->event_thread_ready_cond);

  pthread_mutex_destroy (&jpege_hw_obj_p->input_thread_ready_mutex);
  pthread_cond_destroy (&jpege_hw_obj_p->input_thread_ready_cond);

  pthread_mutex_destroy (&jpege_hw_obj_p->output_thread_ready_mutex);
  pthread_cond_destroy (&jpege_hw_obj_p->output_thread_ready_cond);

  JPEG_HW_PR_ERR("%s:%d] closed %s", __func__, __LINE__, MSM_JPEGE_NAME);

  if (jpege_hw_obj) {
    free(jpege_hw_obj);
  }
  return result;
}

/**
 * Function: jpege_get_mcus_per_block
 *
 * Description: This method is to get the MCUs per block
 *
 * Input parameters:
 *   version: hardware version
 *   p_scale_cfg: pointer to scale configuration
 *
 * Return values:
 *     MCUs per block
 *
 * Notes: none
 **/
uint8_t jpege_get_mcus_per_block(uint32_t version,
  jpege_cmd_scale_cfg *p_scale_cfg)
{
  float downscale_ratio = 1.0;
  uint8_t mcus_per_blk = 3;

  if (!IS_8974_V2(version))
    return 0;

  if (p_scale_cfg->scale_enable) {
    downscale_ratio = p_scale_cfg->scale_input_width /
      p_scale_cfg->output_width;

    if (downscale_ratio <= 2.0)
      mcus_per_blk = 3;
    else if (downscale_ratio <= 4.0)
      mcus_per_blk = 2;
    else if (downscale_ratio <= 8.0)
      mcus_per_blk = 1;
    else
      mcus_per_blk = 0;
  }
  JPEG_HW_PR_ERR("%s:%d] mcus_per_blk %d", __func__, __LINE__, mcus_per_blk);
  return mcus_per_blk;
}

int jpege_lib_hw_config (jpege_hw_obj_t jpege_hw_obj,
  jpege_cmd_input_cfg *p_input_cfg,
  jpege_cmd_jpeg_encode_cfg *p_encode_cfg,
  jpege_cmd_scale_cfg *p_scale_cfg)
{
  int result, i=0,j=0;
  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;
  struct msm_jpeg_hw_cmd hw_cmd;
  struct msm_jpeg_hw_cmd input_cmd;
  struct msm_jpeg_hw_cmds *p_hw_cmds = NULL;
  unsigned int jpeg_clk;

  uint32_t data;
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *) jpege_hw_obj;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  jpegeCtrlCmd.type = MSM_JPEG_MODE_OFFLINE_ENCODE;
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_RESET, &jpegeCtrlCmd);
  JPEG_HW_DBG("ioctl MSM_JPEG_IOCTL_RESET: rc = %d", result);
  if (result)
    goto fail;

  jpege_lib_hw_get_version (&hw_cmd);
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_GET_HW_VERSION, &hw_cmd);
  JPEG_HW_DBG("%s:%d] result %d", __func__, __LINE__, result);
  if (result)
    goto fail;
  jpege_hw_obj_p->version = hw_cmd.data;
  JPEG_HW_PR_ERR("%s:%d] Version %x", __func__, __LINE__,
    jpege_hw_obj_p->version);

  p_hw_cmds = jpege_cmd_core_cfg(p_scale_cfg);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpege_cmd_core_cfg: rc = %d", result);
  if (result)
    goto fail;

  //Configure Fetch Engine
  //Temporary workaround for V2 - pass always 0 as mcu count
  p_hw_cmds = jpege_lib_hw_fe_cfg(p_input_cfg, 0);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpege_lib_hw_fe_cfg: rc = %d", result);
  if (result)
    goto fail;

  //Configure Fetch Engine Buffers
  p_hw_cmds = jpege_lib_hw_fe_buffer_cfg (p_input_cfg, p_scale_cfg);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpege_lib_hw_fe_buffer_cfg: rc = %d", result);
  if (result)
    goto fail;

  //Configure Write Engine
  p_hw_cmds = jpege_lib_hw_we_cfg ();
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpege_lib_hw_we_cfg: rc = %d", result);
  if (result)
    goto fail;

  //Configure Write Engine Buffers
  p_hw_cmds = jpege_lib_hw_we_bffr_cfg(p_input_cfg, p_scale_cfg);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpege_lib_hw_we_buffer_cfg: rc = %d", result);
  if (result)
    goto fail;

  p_hw_cmds = jpege_lib_hw_encode_cfg(p_input_cfg, p_scale_cfg);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  JPEG_HW_DBG("ioctl jpege_lib_hw_encode_cfg: rc = %d", result);
  if (result)
    goto fail;

  //Configure cropping parameters if cropping is enabled
  if(p_scale_cfg->crop_enable){
     p_hw_cmds = jpege_lib_hw_crop_cfg(p_scale_cfg, p_input_cfg);
     if (!p_hw_cmds)
       goto fail;
     result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
     free (p_hw_cmds);
     JPEG_HW_DBG("ioctl jpege_lib_hw_crop_cfg: rc = %d", result);
     if (result)
      goto fail;
  }

  //if scaling not enabled, set default scaling params
  if(!p_scale_cfg->scale_enable) {
     p_hw_cmds = jpege_lib_hw_default_scale_cfg();
     if (!p_hw_cmds)
       goto fail;
     result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
     free (p_hw_cmds);
     JPEG_HW_DBG("ioctl jpege_lib_hw_default_scale_cfg: rc = %d", result);
     if (result)
      goto fail;
  }
  else{
     JPEG_HW_DBG("Scaling enabled.. Setting scaling params");

     p_hw_cmds = jpege_lib_hw_scale_cfg(p_scale_cfg, p_input_cfg);
     if (!p_hw_cmds)
       goto fail;
     result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
     free (p_hw_cmds);
     JPEG_HW_DBG("ioctl jpege_lib_hw_scale_cfg: rc = %d", result);
     if (result)
       goto fail;

     p_hw_cmds = jpege_lib_hw_scale_reg_cfg(p_scale_cfg, p_input_cfg);
     if (!p_hw_cmds)
       goto fail;
     result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
     free (p_hw_cmds);
     JPEG_HW_DBG("ioctl jpege_lib_hw_scale_reg_cfg: rc = %d", result);
     if (result)
       goto fail;
  }

     p_hw_cmds = jpege_lib_hw_encode_state();
     if (!p_hw_cmds)
       goto fail;
     result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
     free (p_hw_cmds);
     JPEG_HW_DBG("ioctl jpeg_lib_hw_encode_state: rc = %d", result);
     if (result)
       goto fail;


 if (p_encode_cfg->quantTblY && p_encode_cfg->quantTblChroma) {
    p_hw_cmds =
      jpege_lib_hw_set_quant_tables (p_encode_cfg->quantTblY,
        p_encode_cfg->
        quantTblChroma);
    if (!p_hw_cmds)
      goto fail;
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
    free (p_hw_cmds);
    JPEG_HW_DBG("ioctl jpege_lib_hw_read_quant_tables: rc = %d",
      result);
    if (result)
      goto fail;
    p_hw_cmds = jpege_lib_hw_read_quant_tables ();
    if (!p_hw_cmds)
      goto fail;
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
    free (p_hw_cmds);
  }

  jpeg_clk = JPEG_CLK_RATE_NORMAL;
  if (p_encode_cfg->speed_mode == JPEG_SPEED_HIGH) {
    jpeg_clk = JPEG_CLK_RATE_HIGH;

    JPEG_HW_DBG("%s:%d] set clk %d", __func__, __LINE__, jpeg_clk);
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_SET_CLK_RATE, &jpeg_clk);
    JPEG_HW_DBG("%s:%d] after set clk", __func__, __LINE__);
    if (result)
      goto fail;
  }

  JPEG_HW_DBG("%s:%d] success", __func__, __LINE__);
  return result;

fail:
  JPEG_HW_DBG("%s:%d] fail", __func__, __LINE__);
  return result;
}

int jpege_lib_input_buf_enq(jpege_hw_obj_t jpege_hw_obj,
  struct jpege_hw_buf *buf)
{
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *) jpege_hw_obj;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  int result;
  struct msm_jpeg_buf msm_buf;

  memset(&msm_buf,0,sizeof( struct msm_jpeg_buf));
  msm_buf.type = buf->type;
  msm_buf.fd   = buf->fd;

  msm_buf.vaddr = buf->vaddr;

  msm_buf.y_off         = buf->y_off;
  msm_buf.y_len         = buf->y_len;
  msm_buf.framedone_len = buf->framedone_len;

  msm_buf.cbcr_off = buf->cbcr_off;
  msm_buf.cbcr_len = buf->cbcr_len;
  msm_buf.pln2_len = buf->cr_len;
  msm_buf.pln2_off = buf->cr_offset;

  msm_buf.num_of_mcu_rows = buf->num_of_mcu_rows;
  msm_buf.offset = buf->offset;
  JPEG_HW_DBG("%s:%d] input_buf: 0x%p enqueue %d, offset %d,"
  "fd %d ", __func__,__LINE__, buf->vaddr, buf->y_len, buf->offset,
   msm_buf.fd);

  JPEG_HW_DBG("%s:%d] y_off=0x%x cbcr_off=0x%x num_of_mcu_rows=%d,"
    "cr_len=%d, cb_len =%d",__func__,__LINE__, buf->y_off,
    buf->cbcr_off, buf->num_of_mcu_rows, msm_buf.pln2_len,
    buf->cbcr_len);

  result = ioctl (jpegefd, MSM_JPEG_IOCTL_INPUT_BUF_ENQUEUE, &msm_buf);

  return result;
}

int jpege_lib_output_buf_enq(jpege_hw_obj_t jpege_hw_obj,
  struct jpege_hw_buf *buf)
{
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *) jpege_hw_obj;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  int result;

  struct msm_jpeg_buf msm_buf;

  memset(&msm_buf,0,sizeof(struct msm_jpeg_buf));
  msm_buf.type = buf->type;
  msm_buf.fd   = buf->fd;

  msm_buf.vaddr = buf->vaddr;

  msm_buf.y_off         = buf->y_off;
  msm_buf.y_len         = buf->y_len;
  msm_buf.framedone_len = buf->framedone_len;

  msm_buf.cbcr_off = buf->cbcr_off;
  msm_buf.cbcr_len = buf->cbcr_len;

  msm_buf.num_of_mcu_rows = buf->num_of_mcu_rows;

  result = ioctl (jpegefd, MSM_JPEG_IOCTL_OUTPUT_BUF_ENQUEUE, &msm_buf);
  JPEG_HW_DBG("%s:%d] output_buf: 0x%p enqueue %d, fd %d, result %d", __func__,
    __LINE__, buf->vaddr, buf->y_len, msm_buf.fd, result);
  return result;
}

int jpege_lib_encode (jpege_hw_obj_t jpege_hw_obj)
{
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *) jpege_hw_obj;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  int result = -1;
  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;

  struct msm_jpeg_hw_cmd hw_cmd;
  struct msm_jpeg_hw_cmds *p_hw_cmds = NULL;

  p_hw_cmds = jpege_lib_hw_start ();
  if (p_hw_cmds) {
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_START, p_hw_cmds);
    JPEG_HW_DBG("ioctl %s: rc = %d", MSM_JPEGE_NAME, result);
    free (p_hw_cmds);
  }

  return result;
}

int jpege_lib_wait_done (jpege_hw_obj_t jpege_hw_obj)
{
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *) jpege_hw_obj;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  struct msm_jpeg_ctrl_cmd jpegeCtrlCmd;
  int result = 0;


  JPEG_HW_DBG("%s:%d] jpege_hw_lib_wait_thread_ready; event_handler %d",
    __func__, __LINE__, (int)jpege_hw_obj_p->event_thread_id);

  if (jpege_hw_obj_p->jpege_hw_lib_event_handler) {
    jpege_hw_lib_wait_thread_ready (jpege_hw_obj_p,
      &(jpege_hw_obj_p->event_thread_id));
  }

  JPEG_HW_DBG("%s:%d] jpege_hw_lib_wait_thread_ready: input_handler %d",
    __func__, __LINE__,(int)jpege_hw_obj_p->input_thread_id);

  if (jpege_hw_obj_p->jpege_hw_lib_input_handler) {
    jpege_hw_lib_wait_thread_ready (jpege_hw_obj_p,
      &(jpege_hw_obj_p->input_thread_id));
  }

  JPEG_HW_DBG("%s:%d] jpege_hw_lib_wait_thread_ready: output_handler",
    __func__, __LINE__);

  if (jpege_hw_obj_p->jpege_hw_lib_output_handler) {
    jpege_hw_lib_wait_thread_ready (jpege_hw_obj_p,
      &(jpege_hw_obj_p->output_thread_id));
  }

  JPEG_HW_DBG("%s:%d] jpege_lib_wait_done", __func__, __LINE__);
  return result;
}

int jpege_lib_stop (jpege_hw_obj_t jpege_hw_obj)
{
  int result = 0;
  __jpege_hw_obj_t *jpege_hw_obj_p = (__jpege_hw_obj_t *) jpege_hw_obj;
  int jpegefd = jpege_hw_obj_p->jpegefd;

  struct msm_jpeg_hw_cmd hw_cmd;
  struct msm_jpeg_hw_cmds *p_hw_cmds = NULL;

  p_hw_cmds = jpege_lib_hw_stop ();
  if (p_hw_cmds) {
    JPEG_HW_DBG("%s:%d] ioctl MSM_JPEG_IOCTL_STOP", __func__,
      __LINE__);
    result = ioctl (jpegefd, MSM_JPEG_IOCTL_STOP, p_hw_cmds);
    JPEG_HW_DBG("ioctl %s: rc = %d", MSM_JPEGE_NAME, result);

      ioctl (jpegefd, MSM_JPEG_IOCTL_EVT_GET_UNBLOCK);
      ioctl (jpegefd, MSM_JPEG_IOCTL_INPUT_GET_UNBLOCK);
      ioctl (jpegefd, MSM_JPEG_IOCTL_OUTPUT_GET_UNBLOCK);

    free(p_hw_cmds);
  }

  return result;
}
