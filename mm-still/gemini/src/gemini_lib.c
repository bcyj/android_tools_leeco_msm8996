
/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <sys/ioctl.h>
#include <stdint.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <pthread.h>

#include <media/msm_gemini.h>
#include <linux/msm_ion.h>
#include "gemini_lib.h"
#include "gemini_lib_hw.h"

#include "gemini_dbg.h"

#define MSM_GEMINI_NAME "/dev/gemini0"

typedef struct
{
  int gmnfd;
  int (*gemini_lib_event_handler) (gmn_obj_t,
    struct gemini_evt *, int);
  int (*gemini_lib_input_handler) (gmn_obj_t, struct gemini_buf *);
  int (*gemini_lib_output_handler) (gmn_obj_t, struct gemini_buf *);

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

  gemini_cmd_operation_cfg op_cfg;
  void *p_userdata;
} __gmn_obj_t;

void gemini_lib_wait_thread_ready (__gmn_obj_t * gmn_obj_p,
  pthread_t* thread_id)
{
  GMN_DBG ("%s:%d], thread_id %d\n", __func__, __LINE__, (int) *thread_id);
  if (*thread_id == gmn_obj_p->event_thread_id) {
    pthread_mutex_lock (&gmn_obj_p->event_thread_ready_mutex);
    GMN_DBG ("%s:%d], event thread ready %d\n", __func__, __LINE__,
      gmn_obj_p->event_thread_is_ready);
    if (!gmn_obj_p->event_thread_is_ready){
      pthread_cond_wait (&gmn_obj_p->event_thread_ready_cond,
       &gmn_obj_p->event_thread_ready_mutex);
    }
    gmn_obj_p->event_thread_is_ready = 0;
    pthread_mutex_unlock (&gmn_obj_p->event_thread_ready_mutex);
  } else if (*thread_id == gmn_obj_p->input_thread_id) {
    pthread_mutex_lock (&gmn_obj_p->input_thread_ready_mutex);
    GMN_DBG ("%s:%d], ready %d\n", __func__, __LINE__,
      gmn_obj_p->input_thread_is_ready);
    if (!gmn_obj_p->input_thread_is_ready)
      pthread_cond_wait (&gmn_obj_p->input_thread_ready_cond,
        &gmn_obj_p->input_thread_ready_mutex);
    gmn_obj_p->input_thread_is_ready = 0;
    pthread_mutex_unlock (&gmn_obj_p->input_thread_ready_mutex);
  } else if (*thread_id == gmn_obj_p->output_thread_id) {
    pthread_mutex_lock (&gmn_obj_p->output_thread_ready_mutex);
    GMN_DBG ("%s:%d], ready %d\n", __func__, __LINE__,
      gmn_obj_p->output_thread_is_ready);
    if (!gmn_obj_p->output_thread_is_ready)
      pthread_cond_wait (&gmn_obj_p->output_thread_ready_cond,
        &gmn_obj_p->output_thread_ready_mutex);
    gmn_obj_p->output_thread_is_ready = 0;
    pthread_mutex_unlock (&gmn_obj_p->output_thread_ready_mutex);
  }
  GMN_DBG ("%s:%d] thread_id %d done\n", __func__, __LINE__,
    (int) *thread_id);
}

void gemini_lib_send_thread_ready (__gmn_obj_t * gmn_obj_p,
  pthread_t* thread_id)
{
  GMN_DBG ("%s:%d], thread_id %d\n", __func__, __LINE__, (int) *thread_id);
  if (*thread_id == gmn_obj_p->event_thread_id) {
    pthread_mutex_lock (&gmn_obj_p->event_thread_ready_mutex);
    gmn_obj_p->event_thread_is_ready = 1;
    pthread_cond_signal (&gmn_obj_p->event_thread_ready_cond);
    pthread_mutex_unlock (&gmn_obj_p->event_thread_ready_mutex);
  } else if (*thread_id == gmn_obj_p->input_thread_id) {
    pthread_mutex_lock (&gmn_obj_p->input_thread_ready_mutex);
    gmn_obj_p->input_thread_is_ready = 1;
    pthread_cond_signal (&gmn_obj_p->input_thread_ready_cond);
    pthread_mutex_unlock (&gmn_obj_p->input_thread_ready_mutex);
  } else if (*thread_id == gmn_obj_p->output_thread_id) {
    pthread_mutex_lock (&gmn_obj_p->output_thread_ready_mutex);
    gmn_obj_p->output_thread_is_ready = 1;
    pthread_cond_signal (&gmn_obj_p->output_thread_ready_cond);
    pthread_mutex_unlock (&gmn_obj_p->output_thread_ready_mutex);
  }
  GMN_DBG ("%s:%d], thread_id %d done\n", __func__, __LINE__,
    (int) *thread_id);
}

void *gemini_lib_event_thread (void *context)
{
  __gmn_obj_t *gmn_obj_p = (__gmn_obj_t *) context;
  int gmnfd = gmn_obj_p->gmnfd;

  struct msm_gemini_ctrl_cmd gmnCtrlCmd;
  struct gemini_evt gmnEvt;
  int result = 0;

  GMN_DBG ("%s:%d] Enter threadid %ld\n", __func__, __LINE__,
    gmn_obj_p->event_thread_id);
  gemini_lib_send_thread_ready (gmn_obj_p, &(gmn_obj_p->event_thread_id));

  do {
    result = ioctl (gmnfd, MSM_GMN_IOCTL_EVT_GET,
      &gmnCtrlCmd);
    GMN_DBG ("%s:%d] MSM_GMN_IOCTL_EVT_GET rc = %d\n",
      __func__, __LINE__, result);
    if (result) {
      if (!gmn_obj_p->event_thread_exit) {
        GMN_DBG ("%s:%d] fail\n", __func__, __LINE__);
      }
    } else {
      gmnEvt.type = gmnCtrlCmd.type;
      gmnEvt.len  = gmnCtrlCmd.len;
      gmnEvt.value = gmnCtrlCmd.value;
      gmn_obj_p->gemini_lib_event_handler (gmn_obj_p->p_userdata,
        &gmnEvt,
        gmnEvt.type);
    }
    gemini_lib_send_thread_ready (gmn_obj_p,
      &(gmn_obj_p->event_thread_id));
  } while (!gmn_obj_p->event_thread_exit);

  GMN_DBG ("%s:%d] Exit\n", __func__, __LINE__);
  return NULL;
}

void *gemini_lib_output_thread (void *context)
{
  __gmn_obj_t *gmn_obj_p = (__gmn_obj_t *) context;
  int gmnfd = gmn_obj_p->gmnfd;

  int result = 0;
  struct msm_gemini_buf msm_buf;
  struct gemini_buf buf;

  GMN_DBG ("%s:%d] Enter threadid %ld\n", __func__, __LINE__,
    gmn_obj_p->output_thread_id);
  gemini_lib_send_thread_ready (gmn_obj_p, &(gmn_obj_p->output_thread_id));

  do {
    result = ioctl (gmnfd, MSM_GMN_IOCTL_OUTPUT_GET, &msm_buf);
    GMN_DBG ("%s:%d] MSM_GMN_IOCTL_OUTPUT_GET rc = %d\n", __func__,
      __LINE__, result);
    if (result) {
      if (!gmn_obj_p->output_thread_exit) {
        GMN_DBG ("%s:%d] fail\n", __func__, __LINE__);
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
      gmn_obj_p->gemini_lib_output_handler (gmn_obj_p->p_userdata, &buf);
    }
    gemini_lib_send_thread_ready (gmn_obj_p,
      &(gmn_obj_p->output_thread_id));
  } while (!gmn_obj_p->output_thread_exit);

  GMN_DBG ("%s:%d] Exit\n", __func__, __LINE__);
  return NULL;
}

void *gemini_lib_input_thread (void *context)
{
  __gmn_obj_t *gmn_obj_p = (__gmn_obj_t *) context;
  int gmnfd = gmn_obj_p->gmnfd;

  int result = 0;
  struct msm_gemini_buf msm_buf;
  struct gemini_buf buf;

  GMN_DBG ("%s:%d] Enter threadid %ld\n", __func__, __LINE__,
    gmn_obj_p->input_thread_id);
  gemini_lib_send_thread_ready (gmn_obj_p, &(gmn_obj_p->input_thread_id));

  do {
    result = ioctl (gmnfd, MSM_GMN_IOCTL_INPUT_GET, &msm_buf);
    GMN_DBG ("%s:%d] MSM_GMN_IOCTL_INPUT_GET rc = %d\n", __func__,
      __LINE__, result);
    if (result) {
      if (!gmn_obj_p->input_thread_exit) {
        GMN_DBG ("%s:%d] fail\n", __func__, __LINE__);
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

      gmn_obj_p->gemini_lib_input_handler (gmn_obj_p->p_userdata,
        &buf);
    }
    gemini_lib_send_thread_ready (gmn_obj_p,
      &(gmn_obj_p->input_thread_id));
  } while (!gmn_obj_p->input_thread_exit);

  GMN_DBG ("%s:%d] Exit\n", __func__, __LINE__);
  return NULL;
}

int gemini_lib_init (gmn_obj_t * gmn_obj_p_p, void *p_userdata,
  int (*event_handler) (gmn_obj_t,
  struct gemini_evt *,
  int event),
  int (*output_handler) (void *, struct gemini_buf *),
  int (*input_handler) (void *, struct gemini_buf *))
{
  __gmn_obj_t *gmn_obj_p;
  int gmnfd;

  int result;
  struct msm_gemini_ctrl_cmd gmnCtrlCmd;

  gmn_obj_p = malloc (sizeof (__gmn_obj_t));
  if (!gmn_obj_p) {
    GMN_PR_ERR ("%s:%d] no mem\n", __func__, __LINE__);
    return -1;
  }
  memset (gmn_obj_p, 0, sizeof (__gmn_obj_t));

  gmnfd = open (MSM_GEMINI_NAME, O_RDWR);
  GMN_DBG ("open %s: fd = %d\n", MSM_GEMINI_NAME, gmnfd);
  if (gmnfd < 0) {
    GMN_PR_ERR ("Cannot open %s\n", MSM_GEMINI_NAME);
    goto gemini_init_err;
  }

  gmn_obj_p->gemini_lib_event_handler = event_handler;
  gmn_obj_p->gemini_lib_input_handler = input_handler;
  gmn_obj_p->gemini_lib_output_handler = output_handler;
  gmn_obj_p->gmnfd = gmnfd;
  gmn_obj_p->p_userdata = p_userdata;

  pthread_mutex_init (&gmn_obj_p->event_thread_ready_mutex, NULL);
  pthread_cond_init (&gmn_obj_p->event_thread_ready_cond, NULL);
  gmn_obj_p->event_thread_is_ready = 0;

  pthread_mutex_init (&gmn_obj_p->input_thread_ready_mutex, NULL);
  pthread_cond_init (&gmn_obj_p->input_thread_ready_cond, NULL);
  gmn_obj_p->input_thread_is_ready = 0;

  pthread_mutex_init (&gmn_obj_p->output_thread_ready_mutex, NULL);
  pthread_cond_init (&gmn_obj_p->output_thread_ready_cond, NULL);
  gmn_obj_p->output_thread_is_ready = 0;

  if (event_handler) {
    pthread_mutex_lock(&gmn_obj_p->event_thread_ready_mutex);
      result = pthread_create (&gmn_obj_p->event_thread_id, NULL,
        gemini_lib_event_thread, gmn_obj_p);
    if (result < 0) {
      GMN_PR_ERR ("%s event thread creation failed\n", __func__);
      pthread_mutex_unlock(&gmn_obj_p->event_thread_ready_mutex);
      goto gemini_init_err;
    }
    pthread_mutex_unlock(&gmn_obj_p->event_thread_ready_mutex);
  }
  if (input_handler) {
    pthread_mutex_lock(&gmn_obj_p->input_thread_ready_mutex);
    result = pthread_create (&gmn_obj_p->input_thread_id, NULL,
      gemini_lib_input_thread, gmn_obj_p);
    if (result < 0) {
      GMN_PR_ERR ("%s input thread creation failed\n", __func__);
      pthread_mutex_unlock(&gmn_obj_p->input_thread_ready_mutex);
      goto gemini_init_err;
    }
    pthread_mutex_unlock(&gmn_obj_p->input_thread_ready_mutex);
  }
  if (output_handler) {
    pthread_mutex_lock(&gmn_obj_p->output_thread_ready_mutex);
    result = pthread_create (&gmn_obj_p->output_thread_id, NULL,
    gemini_lib_output_thread, gmn_obj_p);
    if (result < 0) {
      GMN_PR_ERR ("%s output thread creation failed\n", __func__);
      pthread_mutex_unlock(&gmn_obj_p->output_thread_ready_mutex);
      goto gemini_init_err;
    }
    pthread_mutex_unlock(&gmn_obj_p->output_thread_ready_mutex);
  }

  GMN_DBG ("gemini create all threads success\n");
  gemini_lib_wait_done(gmn_obj_p);
  GMN_DBG ("gemini after starting all threads\n");
  *gmn_obj_p_p = gmn_obj_p;
  return gmnfd;
gemini_init_err:
  if (gmn_obj_p) {
    free (gmn_obj_p);
  }
  return -1;
}

int gemini_lib_release(gmn_obj_t gmn_obj)
{
  int result;
  if (!gmn_obj) {
    GMN_PR_ERR("%s:%d]Gemini object is NULL in release\n",
      __func__, __LINE__);
    return -EINVALID;
  }
  __gmn_obj_t *gmn_obj_p = (__gmn_obj_t *) gmn_obj;
  int gmnfd = gmn_obj_p->gmnfd;

  gmn_obj_p->event_thread_exit = 1;
  gmn_obj_p->input_thread_exit = 1;
  gmn_obj_p->output_thread_exit = 1;

  if (gmn_obj_p->gemini_lib_event_handler) {
    ioctl (gmnfd, MSM_GMN_IOCTL_EVT_GET_UNBLOCK);
    GMN_DBG ("%s:%d] pthread_join: event_thread\n", __func__,
      __LINE__);
    if (pthread_join (gmn_obj_p->event_thread_id, NULL) != 0) {
      GMN_DBG ("%s: failed %d\n", __func__, __LINE__);
    }
  }

  if (gmn_obj_p->gemini_lib_input_handler) {
    ioctl (gmnfd, MSM_GMN_IOCTL_INPUT_GET_UNBLOCK);
    GMN_DBG ("%s:%d] pthread_join: input_thread\n", __func__,
      __LINE__);
    if (pthread_join (gmn_obj_p->input_thread_id, NULL) != 0) {
      GMN_DBG ("%s: failed %d\n", __func__, __LINE__);
    }
  }

  if (gmn_obj_p->gemini_lib_output_handler) {
    ioctl (gmnfd, MSM_GMN_IOCTL_OUTPUT_GET_UNBLOCK);
    GMN_DBG ("%s:%d] pthread_join: output_thread\n", __func__,
      __LINE__);
    if (pthread_join (gmn_obj_p->output_thread_id, NULL) != 0) {
      GMN_DBG ("%s: failed %d\n", __func__, __LINE__);
    }
  }

  result = close (gmnfd);

  pthread_mutex_destroy (&gmn_obj_p->event_thread_ready_mutex);
  pthread_cond_destroy (&gmn_obj_p->event_thread_ready_cond);

  pthread_mutex_destroy (&gmn_obj_p->input_thread_ready_mutex);
  pthread_cond_destroy (&gmn_obj_p->input_thread_ready_cond);

  pthread_mutex_destroy (&gmn_obj_p->output_thread_ready_mutex);
  pthread_cond_destroy (&gmn_obj_p->output_thread_ready_cond);

  if(NULL!= gmn_obj_p){
    free(gmn_obj_p);
  }

  GMN_DBG ("%s:%d] closed %s\n", __func__, __LINE__, MSM_GEMINI_NAME);
  return result;
}

int gemini_lib_hw_config (gmn_obj_t gmn_obj,
  gemini_cmd_input_cfg *p_input_cfg,
  gemini_cmd_output_cfg *p_output_cfg,
  gemini_cmd_jpeg_encode_cfg *p_encode_cfg,
  gemini_cmd_operation_cfg *p_op_cfg)
{
  int result;
  struct msm_gemini_ctrl_cmd gmnCtrlCmd;
  struct msm_gemini_hw_cmd hw_cmd;
  struct msm_gemini_hw_cmds *p_hw_cmds = NULL;
  __gmn_obj_t *gmn_obj_p = (__gmn_obj_t *) gmn_obj;
  int gmnfd = gmn_obj_p->gmnfd;

  gmnCtrlCmd.type = p_op_cfg->useMode;
  result = ioctl (gmnfd, MSM_GMN_IOCTL_RESET, &gmnCtrlCmd);
  GMN_DBG ("ioctl MSM_GMN_IOCTL_RESET: rc = %d\n", result);
  if (result)
    goto fail;

  gemini_lib_hw_get_version (&hw_cmd);
  result = ioctl (gmnfd, MSM_GMN_IOCTL_GET_HW_VERSION, &hw_cmd);
  GMN_DBG ("ioctl %s: rc = %d, version: %d\n", MSM_GEMINI_NAME, result,
   hw_cmd.data);
  if (result)
    goto fail;

  if (MSM_GMN_OUTMODE_SINGLE == p_op_cfg->outputMode) {
    result = ioctl (gmnfd, MSM_GMN_IOCTL_SET_MODE, &p_op_cfg->outputMode);
    GMN_PR_ERR("%s:%d] rc = %d, outMode: %d", __func__, __LINE__, result,
      p_op_cfg->outputMode);
    if (result)
      goto fail;
  }
  p_hw_cmds = gemini_lib_hw_fe_cfg (p_input_cfg);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  GMN_DBG ("ioctl gemini_lib_hw_fe_cfg: rc = %d\n", result);
  if (result)
    goto fail;

  /*
   * @todo imem cfg for inline?
   */

  gemini_fe_operation_cfg_type hw_op_cfg_type;
  hw_op_cfg_type.eInputFormat = p_input_cfg->inputFormat;
  hw_op_cfg_type.nReadBurstLength = p_input_cfg->fe_burst_length;
  hw_op_cfg_type.nFrameWidthMCUs = p_input_cfg->frame_width_mcus;
  hw_op_cfg_type.nFrameHeightMCUs = p_input_cfg->frame_height_mcus;

  p_hw_cmds = gemini_lib_hw_op_cfg (p_op_cfg, &hw_op_cfg_type);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  GMN_DBG ("ioctl gemini_lib_hw_op_cfg: rc = %d\n", result);
  if (result)
    goto fail;

  p_hw_cmds = gemini_lib_hw_we_cfg (p_output_cfg);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  GMN_DBG ("ioctl gemini_lib_hw_we_cfg: rc = %d\n", result);
  if (result)
    goto fail;

  gemini_encode_pipeline_cfg_type hw_pipeline_cfg_type;
  hw_pipeline_cfg_type.nInputFormat = p_input_cfg->inputFormat;
  hw_pipeline_cfg_type.nUseMode = p_op_cfg->useMode;  /*  OFFLINE_ENCODE */
  hw_pipeline_cfg_type.nWEInputSel = 0; /*  OFFLINE_ENCODE || REALTIME_ENCODE */
  hw_pipeline_cfg_type.nVFEEnable =
    ((GEMINI_MODE_OFFLINE_ENCODE == p_op_cfg->useMode) ? 0 : 1); /*  OFFLINE_ENCODE */
  hw_pipeline_cfg_type.nFSCEnable = p_encode_cfg->bFSCEnable;
  hw_pipeline_cfg_type.nJPEGInputSel = 0; /*  always from Fetch Engine */
  hw_pipeline_cfg_type.nFEInputSel = 0; /*  choose 64 bit bus width */
  //hw_pipeline_cfg_type.nImemFifoModeDisable = 0; //FIFO mode
  hw_pipeline_cfg_type.nImemFifoModeDisable = 1; //Ping pong mode
  p_hw_cmds = gemini_lib_hw_pipeline_cfg (&hw_pipeline_cfg_type);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  GMN_DBG ("ioctl gemini_lib_hw_pipeline_cfg: rc = %d\n", result);
  if (result)
    goto fail;

  p_hw_cmds =
    gemini_lib_hw_restart_marker_set (p_encode_cfg->restartInterval);
  if (!p_hw_cmds)
    goto fail;
  result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMDS, p_hw_cmds);
  free (p_hw_cmds);
  GMN_DBG ("ioctl restart marker: rc = %d\n", result);
  if (result)
    goto fail;

  if (p_encode_cfg->bCustomHuffmanTbl) {
    gemini_huff_lookup_table_type
    sLumaDCHuffmanLUT[GEMINI_NUM_HUFFMANDC_ENTRIES];
    gemini_huff_lookup_table_type
    sChromaDCHuffmanLUT[GEMINI_NUM_HUFFMANDC_ENTRIES];
    gemini_huff_lookup_table_type sLumaACHuffmanLUT[256];
    gemini_huff_lookup_table_type sChromaACHuffmanLUT[256];

    gemini_lib_hw_create_huffman_tables (p_encode_cfg,
      &sLumaDCHuffmanLUT[0],
      &sChromaDCHuffmanLUT[0],
      &sLumaACHuffmanLUT[0],
      &sChromaACHuffmanLUT[0]);
    p_hw_cmds =
    gemini_lib_hw_set_huffman_tables (&sLumaDCHuffmanLUT[0],
      &sChromaDCHuffmanLUT
      [0],
      &sLumaACHuffmanLUT[0],
      &sChromaACHuffmanLUT
      [0]);
    if (!p_hw_cmds)
      goto fail;
    result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMDS, p_hw_cmds);
    free (p_hw_cmds);
    GMN_DBG ("ioctl huffman: rc = %d\n", result);
    if (result)
      goto fail;
  }

  if (p_encode_cfg->quantTblY && p_encode_cfg->quantTblChroma) {
    p_hw_cmds =
      gemini_lib_hw_set_quant_tables (p_encode_cfg->quantTblY,
        p_encode_cfg->
        quantTblChroma);
    if (!p_hw_cmds)
      goto fail;
    result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMDS, p_hw_cmds);
    free (p_hw_cmds);
    GMN_DBG ("ioctl gemini_lib_hw_set_quant_tables: rc = %d\n",
      result);
    if (result)
      goto fail;
    p_hw_cmds = gemini_lib_hw_read_quant_tables ();
    if (!p_hw_cmds)
      goto fail;
    result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMDS, p_hw_cmds);
    free (p_hw_cmds);
  }

  if (p_encode_cfg->bFSCEnable) {
    p_hw_cmds =
      gemini_lib_hw_set_filesize_ctrl (&p_encode_cfg->
        sFileSizeControl);
    if (!p_hw_cmds)
      goto fail;
    result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMDS, p_hw_cmds);
    free (p_hw_cmds);
    GMN_DBG ("ioctl gemini_lib_hw_set_filesize_ctrl: rc = %d\n",
      result);
    if (result)
      goto fail;
  }

  gmn_obj_p->op_cfg = *p_op_cfg;
  GMN_DBG ("%s:%d] success\n", __func__, __LINE__);
  return result;

fail:
  GMN_DBG ("%s:%d] fail\n", __func__, __LINE__);
  return result;
}

int gemini_lib_input_buf_enq(gmn_obj_t gmn_obj, struct gemini_buf *buf)
{
  __gmn_obj_t *gmn_obj_p = (__gmn_obj_t *) gmn_obj;
  int gmnfd = gmn_obj_p->gmnfd;

  int result;
  struct msm_gemini_buf msm_buf;

  msm_buf.type = buf->type;
  msm_buf.fd   = buf->fd;

  msm_buf.vaddr = buf->vaddr;

  msm_buf.y_off         = buf->y_off;
  msm_buf.y_len         = buf->y_len;
  msm_buf.framedone_len = buf->framedone_len;

  msm_buf.cbcr_off = buf->cbcr_off;
  msm_buf.cbcr_len = buf->cbcr_len;

  msm_buf.num_of_mcu_rows = buf->num_of_mcu_rows;
  msm_buf.offset = buf->offset;
  GMN_DBG("%s:%d] input_buf: 0x%p enqueue %d, offset %d \n", __func__,
    __LINE__, buf->vaddr, buf->y_len, buf->offset);

  GMN_DBG ("%s:%d] y_off=0x%x cbcr_off=0x%x num_of_mcu_rows=%d\n", __func__,
    __LINE__, buf->y_off, buf->cbcr_off, buf->num_of_mcu_rows);

  result = ioctl (gmnfd, MSM_GMN_IOCTL_INPUT_BUF_ENQUEUE, &msm_buf);

  return result;
}

int gemini_lib_output_buf_enq(gmn_obj_t gmn_obj, struct gemini_buf *buf)
{
  __gmn_obj_t *gmn_obj_p = (__gmn_obj_t *) gmn_obj;
  int gmnfd = gmn_obj_p->gmnfd;

  int result;

  struct msm_gemini_buf msm_buf;

  msm_buf.type = buf->type;
  msm_buf.fd   = buf->fd;

  msm_buf.vaddr = buf->vaddr;

  msm_buf.y_off         = buf->y_off;
  msm_buf.y_len         = buf->y_len;
  msm_buf.framedone_len = buf->framedone_len;

  msm_buf.cbcr_off = buf->cbcr_off;
  msm_buf.cbcr_len = buf->cbcr_len;

  msm_buf.num_of_mcu_rows = buf->num_of_mcu_rows;

  result = ioctl (gmnfd, MSM_GMN_IOCTL_OUTPUT_BUF_ENQUEUE, &msm_buf);
  GMN_DBG ("%s:%d] output_buf: 0x%p enqueue %d, result %d\n", __func__,
    __LINE__, buf->vaddr, buf->y_len, result);
  return result;
}

int gemini_lib_encode (gmn_obj_t gmn_obj)
{
  __gmn_obj_t *gmn_obj_p = (__gmn_obj_t *) gmn_obj;
  int gmnfd = gmn_obj_p->gmnfd;

  int result = -1;
  struct msm_gemini_ctrl_cmd gmnCtrlCmd;

  struct msm_gemini_hw_cmd hw_cmd;
  struct msm_gemini_hw_cmds *p_hw_cmds = NULL;

  p_hw_cmds = gemini_lib_hw_start (&gmn_obj_p->op_cfg);
  if (p_hw_cmds) {
    result = ioctl (gmnfd, MSM_GMN_IOCTL_START, p_hw_cmds);
    GMN_DBG ("ioctl %s: rc = %d\n", MSM_GEMINI_NAME, result);
    free (p_hw_cmds);
  }

  return result;
}

int gemini_lib_wait_done (gmn_obj_t gmn_obj)
{
  __gmn_obj_t *gmn_obj_p = (__gmn_obj_t *) gmn_obj;
  int gmnfd = gmn_obj_p->gmnfd;

  struct msm_gemini_ctrl_cmd gmnCtrlCmd;
  int result = 0;


  GMN_DBG ("%s:%d] gemini_lib_wait_thread_ready; event_handler %d\n", __func__,
    __LINE__, (int)gmn_obj_p->event_thread_id);

  if (gmn_obj_p->gemini_lib_event_handler) {
    gemini_lib_wait_thread_ready (gmn_obj_p,
      &(gmn_obj_p->event_thread_id));
  }

  GMN_DBG ("%s:%d] gemini_lib_wait_thread_ready: input_handler %d\n", __func__,
    __LINE__,(int)gmn_obj_p->input_thread_id);

  if (gmn_obj_p->gemini_lib_input_handler) {
    gemini_lib_wait_thread_ready (gmn_obj_p,
      &(gmn_obj_p->input_thread_id));
  }

  GMN_DBG ("%s:%d] gemini_lib_wait_thread_ready: output_handler\n", __func__,
    __LINE__);

  if (gmn_obj_p->gemini_lib_output_handler) {
    gemini_lib_wait_thread_ready (gmn_obj_p,
      &(gmn_obj_p->output_thread_id));
  }

  GMN_DBG ("%s:%d] gemini_lib_wait_done\n", __func__, __LINE__);
  return result;
}

int gemini_lib_stop (gmn_obj_t gmn_obj, int nicely)
{
  int result = 0;
  __gmn_obj_t *gmn_obj_p = (__gmn_obj_t *) gmn_obj;
  int gmnfd = gmn_obj_p->gmnfd;

  struct msm_gemini_hw_cmd hw_cmd;
  struct msm_gemini_hw_cmds *p_hw_cmds = NULL;

  p_hw_cmds = gemini_lib_hw_stop (&gmn_obj_p->op_cfg, nicely);
  if (p_hw_cmds) {
    GMN_DBG ("%s:%d] ioctl MSM_GMN_IOCTL_STOP\n", __func__,
      __LINE__);
    result = ioctl (gmnfd, MSM_GMN_IOCTL_STOP, p_hw_cmds);
    GMN_DBG ("ioctl %s: rc = %d\n", MSM_GEMINI_NAME, result);

    if (!nicely) {
      ioctl (gmnfd, MSM_GMN_IOCTL_EVT_GET_UNBLOCK);
      ioctl (gmnfd, MSM_GMN_IOCTL_INPUT_GET_UNBLOCK);
      ioctl (gmnfd, MSM_GMN_IOCTL_OUTPUT_GET_UNBLOCK);
    }
    free(p_hw_cmds);
  }

  return result;
}
