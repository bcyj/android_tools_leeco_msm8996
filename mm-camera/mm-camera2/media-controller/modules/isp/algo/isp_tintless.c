/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <pthread.h>
#include <semaphore.h>
#include "camera_dbg.h"
#include "isp_tintless.h"
#include "isp_log.h"
#include <sys/syscall.h>
#include <sys/prctl.h>

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ENABLE_TINTLESS_LOGGING
  #undef CDBG
  #define CDBG ALOGE
#endif

#define MESH_PRINT_SIZE  10
#define TINTLESS_ALGO_SKIP_COUNT 1

void isp_tintless_open_version(uint32_t version);
tintless_return_t isp_tintless_open(void ** const res, uint32_t * updates_needed);
tintless_return_t isp_tintless_stat_config(void * const res, tintless_stats_config_t *cfg);
tintless_return_t isp_tintless_mesh_config(void * const res, tintless_mesh_config_t *cfg);
tintless_return_t isp_tintless_update_chromatix_params(void * const res, chromatix_color_tint_correction_type *p);
tintless_return_t isp_tintless_get_version(void * const res, tintless_version_t *version);
tintless_return_t isp_tintless_algo(void * const res,
                                    tintless_stats_t * be_stats,
                                    tintless_mesh_rolloff_array_t *ptable_3a,
                                    tintless_mesh_rolloff_array_t *ptable_cur,
                                    tintless_mesh_rolloff_array_t *ptable_correction);
tintless_return_t isp_tintless_close(void ** const res);

/** isp_tintless_debug_BE:
 *
 *    @data:
 *
 *  print BE stats in pipeline.
 *
 **/
static void isp_tintless_debug_BE(q3a_be_stats_t *be_stats, uint32_t frame_id)
{
  int i;
  uint16_t print_count = MAX_BE_STATS_NUM;
  char buf[32];
  FILE *fptr;

  if (!be_stats) {
    CDBG_ERROR("%s: null pointer be_stats = %p\n", __func__, be_stats);
    return;
  }

  ISP_DBG(ISP_MOD_COM,"%s: BE stats: h_num %d v_num %d\n", __func__,
    be_stats->be_region_h_num, be_stats->be_region_v_num);


  if (be_stats != NULL) {
     snprintf(buf, sizeof(buf), "/data/misc/camera/BE_%d.txt", frame_id);
     fptr = fopen(buf, "w+");
     if (!fptr) {
       CDBG_ERROR("%s: error open files! fptr = %p\n", __func__, fptr);
       return;
     }

     fprintf(fptr, "==========BE STATS===frame id %d========\n", frame_id);
     for (i = 0;i < print_count; i++) {
       fprintf(fptr, " BE stats: [%d] num: r %5d, b %5d, gr %5d, gb %5d "
         "sum: r %5d, b %5d, gr %5d, gb %5d\n", i,
         be_stats->be_r_num[i], be_stats->be_b_num[i],
         be_stats->be_gr_num[i], be_stats->be_gb_num[i],
         be_stats->be_r_sum[i], be_stats->be_b_sum[i],
         be_stats->be_gr_sum[i], be_stats->be_gb_sum[i]);
    }
    fclose(fptr);
  }

 ISP_DBG(ISP_MOD_COM,"%s: X\n", __func__);

 return;
} /* isp_tintless_debug_BE */

/** isp_tintless_table_debug:
 *
 *    @data:
 *
 *  Print mesh in floats, 13x10.
 *
 **/
static void isp_tintless_table_dump(tintless_mesh_rolloff_array_t *data,
  char* fname, uint32_t frame_id)
{
  char buf[32];
  FILE *fptr;
  int i;
  ALOGE("%s: start dump tintless\n", __func__);

  if (data != NULL) {
    snprintf(buf, sizeof(buf), "/data/misc/camera/%s_%d.txt", fname, frame_id);
    fptr = fopen(buf, "w+");
    if (!fptr) {
       CDBG_ERROR("%s: error open files! fptr = %p\n", __func__, fptr);
       return;
    }

    fprintf(fptr, "==========R===========\n");
    for (i = 0; i< 10 ; i ++) {
      fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f\n",
        data->r_gain[(i*13) + 0], data->r_gain[(i*13) + 1], data->r_gain[(i*13) + 2],
        data->r_gain[(i*13) + 3], data->r_gain[(i*13) + 4], data->r_gain[(i*13) + 5],
        data->r_gain[(i*13) + 6], data->r_gain[(i*13) + 7], data->r_gain[(i*13) + 8],
        data->r_gain[(i*13) + 9], data->r_gain[(i*13) + 10], data->r_gain[(i*13) + 11],
        data->r_gain[(i*13) + 12]);
    }
    fprintf(fptr, "==========GR=========== \n");
    for (i = 0; i< 10 ; i ++) {
      fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f\n",
        data->gr_gain[(i*13) + 0], data->gr_gain[(i*13) + 1], data->gr_gain[(i*13) + 2],
        data->gr_gain[(i*13) + 3], data->gr_gain[(i*13) + 4], data->gr_gain[(i*13) + 5],
        data->gr_gain[(i*13) + 6], data->gr_gain[(i*13) + 7], data->gr_gain[(i*13) + 8],
        data->gr_gain[(i*13) + 9], data->gr_gain[(i*13) + 10], data->gr_gain[(i*13) + 11],
        data->gr_gain[(i*13) + 12]);
     }
    fprintf(fptr, "==========GB=========== \n");
    for (i = 0; i< 10 ; i ++) {
      fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f\n",
        data->gb_gain[(i*13) + 0], data->gr_gain[(i*13) + 1], data->gb_gain[(i*13) + 2],
        data->gb_gain[(i*13) + 3], data->gb_gain[(i*13) + 4], data->gb_gain[(i*13) + 5],
        data->gb_gain[(i*13) + 6], data->gb_gain[(i*13) + 7], data->gb_gain[(i*13) + 8],
        data->gb_gain[(i*13) + 9], data->gb_gain[(i*13) + 10], data->gb_gain[(i*13) + 11],
        data->gb_gain[(i*13) + 12]);
     }
    fprintf(fptr, "==========B=========== \n");
     for (i = 0; i< 10 ; i ++) {
      fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f\n",
        data->b_gain[(i*13) + 0], data->b_gain[(i*13) + 1], data->b_gain[(i*13) + 2],
        data->b_gain[(i*13) + 3], data->b_gain[(i*13) + 4], data->b_gain[(i*13) + 5],
        data->b_gain[(i*13) + 6], data->b_gain[(i*13) + 7], data->b_gain[(i*13) + 8],
        data->b_gain[(i*13) + 9], data->b_gain[(i*13) + 10], data->b_gain[(i*13) + 11],
        data->b_gain[(i*13) + 12]);
    }
    fclose(fptr);
    CDBG_HIGH("%s: dumped : %s", __func__, fname);
  }
  return;
} /* isp_tintless_table_debug */


/** isp_tintless_main_loop:
 *
 *    @data:
 *
 *  tintless main loop.
 *
 **/
static void *isp_tintless_main_loop(void *data)
{
  isp_tintless_session_t *session = (isp_tintless_session_t *)data;
  isp_tintless_t *tintless = ( isp_tintless_t *)session->tintless;
  isp_tintless_thread_data_t *thread_data = &session->thread_data;
  tintless_mesh_rolloff_array_t *in_table = &session->mesh_fixed;
  tintless_mesh_rolloff_array_t *cur_table = &session->curr_rolloff_hw;
  tintless_stats_t *mod_stats = &thread_data->mod_stats;
  tintless_mesh_rolloff_array_t out_table;
  int rc;
  int32_t dump_to_file;
  char value[PROPERTY_VALUE_MAX];
  uint32_t ready_frame_id;

  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "isp_tintless", 0, 0, 0);
  while(!(thread_data->thread_exit)) {
    sem_wait(&thread_data->wait_sem);

    pthread_mutex_lock(&thread_data->lock_mutex);
    if(thread_data->algo_requested) {
      pthread_mutex_unlock(&thread_data->lock_mutex);

    ISP_DBG(ISP_MOD_COM,"%s: run tintless algo, stats type = %d\n",
      __func__, tintless->tintless_data.stats_support_type);
      /* Tintless core algorithm*/
    if(tintless->tintless_data.stats_support_type == ISP_TINTLESS_STATS_TYPE_BG){
      rc = isp_tintless_algo(session->tinstance, &thread_data->mod_stats,
             in_table, cur_table, &out_table);
      if (rc != TINTLESS_SUCCESS) {
        CDBG_ERROR("%s: algo failed, not update, rc : %d\n", __func__, rc);
        thread_data->algo_requested = 0;
        continue;
      }
    }
    else {
      rc = isp_tintless_algo(session->tinstance, &thread_data->mod_stats,
            in_table, NULL, &out_table);
      if(rc != TINTLESS_SUCCESS) {
        CDBG_ERROR("%s: algo failed, not update, rc : %d\n", __func__, rc);
        thread_data->algo_requested = 0;
        continue;
      }
    }

      pthread_mutex_lock(&thread_data->lock_mutex);
      thread_data->algo_requested = 0;
      session->mesh_hw = out_table;
      session->ready_frame_id = thread_data->in_frame_id;
      ready_frame_id = session->ready_frame_id;
      pthread_mutex_unlock(&thread_data->lock_mutex);

      property_get("persist.camera.tintless.dump", value, "0");
      dump_to_file = atoi(value);
      ISP_DBG(ISP_MOD_COM,"%s: frame_id: %d\n", __func__, ready_frame_id);
      if (dump_to_file) {
        isp_tintless_table_dump(in_table, "algo_in", ready_frame_id);
        isp_tintless_table_dump(&out_table, "algo_out", ready_frame_id);
      }
    } else {
      pthread_mutex_unlock(&thread_data->lock_mutex);
    }
  }

  return NULL;
}

/** isp_tintless_create:
 *
 *    @isp_version:
 *
 *  Return pointer to tintless data structure.
 *
 **/
isp_tintless_t *isp_tintless_create(uint32_t isp_version)
{
  isp_tintless_t *tintless = malloc(sizeof(isp_tintless_t));

  if (!tintless) {
    CDBG_ERROR("%s: cannot malloc for tintless struct\n", __func__);
    return NULL;
  }

  memset(tintless, 0, sizeof(isp_tintless_t));
  tintless->tintless_data.isp_version = isp_version;
  tintless->tintless_data.is_supported = FALSE;
  tintless->tintless_data.is_enabled = TRUE;

  if ((GET_ISP_MAIN_VERSION(isp_version) == ISP_VERSION_40)) {
     CDBG_ERROR("%s: using BE stats for tintless!, isp main version = %x\n",
       __func__, ISP_VERSION_40);
     tintless->tintless_data.stats_support_type = ISP_TINTLESS_STATS_TYPE_BE;
  } else if ((GET_ISP_MAIN_VERSION(isp_version) == ISP_VERSION_32)) {
     CDBG_ERROR("%s: using BG stats for tintless!, isp version = %x\n",
       __func__, ISP_VERSION_32);
     tintless->tintless_data.stats_support_type = ISP_TINTLESS_STATS_TYPE_BG;
  }

  isp_tintless_open_version(isp_version);
  return tintless;
}

/** isp_tintless_destroy:
 *
 *    @tintless: pointer to tintless data structure
 *
 **/
void isp_tintless_destroy(isp_tintless_t *tintless)
{
  if (tintless) {
    free(tintless);
    tintless = NULL;
  }
}

/** isp_tintless_start_task:
 *
 *    @session:
 *
 **/
static int isp_tintless_start_task(isp_tintless_session_t *session)
{
  int rc = 0;
  isp_tintless_thread_data_t *thread_data = &session->thread_data;

  sem_init(&thread_data->wait_sem, 0, 0);
  pthread_mutex_init(&thread_data->lock_mutex, NULL);

  rc = pthread_create(&thread_data->pid, NULL,
    isp_tintless_main_loop, (void *)session);
  pthread_setname_np(thread_data->pid, "CAM_isp_tintless");
  if(rc) {
    CDBG_ERROR("%s: pthread_create error = %d\n",
      __func__, rc);
  }

  return rc;
}

/** isp_tintless_open_session:
 *
 *    @tintless:
 *    @session_id:
 *
 **/
isp_tintless_session_t *isp_tintless_open_session(isp_tintless_t *tintless,
  uint32_t session_id)
{
  int i;
  int rc = TINTLESS_SUCCESS;
  uint32_t update;

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (tintless->sessions[i].tinstance == NULL) {
      isp_tintless_session_t *session = &tintless->sessions[i];
      memset(session, 0, sizeof(*session));

      /* open library for tintless correction */
      rc = isp_tintless_open(&(session->tinstance), &update);
      if (rc != TINTLESS_SUCCESS) {
        CDBG_ERROR("%s: tintless library failed. rc = %d\n", __func__, rc);
        goto open_failed;
      }

      rc = isp_tintless_get_version(session->tinstance,
             &tintless->tintless_data.version);
      if (rc != TINTLESS_SUCCESS) {
        CDBG_ERROR("%s: Lib mismatch. get version fail rc : %d\n", __func__,
          rc);
        goto ver_failed;
      }

      ISP_DBG(ISP_MOD_COM,"%s: versionX %d.%d\n", __func__,
        tintless->tintless_data.version.api_version,
        tintless->tintless_data.version.minor_version);

      session->update_stats =
        ((update & TINTLESS_UPDATE_STATS) != 0);
      session->update_chromatix =
        ((update & TINTLESS_UPDATE_CHROMATIX_PARAMS) != 0);
      session->update_mesh =
        ((update & TINTLESS_UPDATE_MESH) != 0);

      session->session_id = session_id;
      session->tintless = tintless;

      tintless->tintless_data.is_supported = TRUE;

      /* start tintless task */
      isp_tintless_start_task(session);

      return session;
    }
  }

  CDBG_ERROR("%s: no tintless session available, error\n", __func__);
  return NULL;

ver_failed:
  rc = isp_tintless_close(&(tintless->sessions[i].tinstance));
  if (rc != TINTLESS_SUCCESS)
    CDBG_ERROR("%s: Close tintless lib failed : %d\n", __func__, rc);

open_failed:
  return NULL;
}

/** isp_tintless_stop_task:
 *
 *    @session:
 *
 **/
static void isp_tintless_stop_task(isp_tintless_session_t *session)
{
  isp_tintless_thread_data_t *thread_data = &session->thread_data;

  thread_data->thread_exit = 1;
  thread_data->algo_requested = 0;
  sem_post(&thread_data->wait_sem);
  pthread_join(thread_data->pid, NULL);

  sem_destroy(&thread_data->wait_sem);
  pthread_mutex_destroy(&thread_data->lock_mutex);
}

/** isp_tintless_close_session:
 *
 *    @session:
 *
 **/
void isp_tintless_close_session(isp_tintless_session_t *session)
{
  int rc;

  if (session == NULL) {
    CDBG_ERROR("%s: invalid session\n", __func__);
    return;
  }

  if (session->tinstance == NULL) {
    ISP_DBG(ISP_MOD_COM,"%s: session already closed\n", __func__);
    return;
  }

  /* stop tintles task */
  isp_tintless_stop_task(session);

  rc = isp_tintless_close(&(session->tinstance));
  if (rc != TINTLESS_SUCCESS)
    CDBG_ERROR("%s: Close tintless lib failed : %d\n", __func__, rc);

  /* zero out old tintless session data */
  memset(session, 0, sizeof(*session));
}

/** isp_tintless_set_param:
 *
 *    @session:
 *    @enabled:
 *
 *  Return 0 on success, -1 on error
 **/
void isp_tintless_set_param(isp_tintless_session_t *session, uint8_t *enabled)
{
  isp_tintless_t * tintless;
  tintless = session->tintless;
  if (tintless == NULL) {
    CDBG_ERROR("%s: invalid tintless module ptr\n", __func__);
    return;
  }
  if(enabled == NULL) {
   tintless->tintless_data.is_enabled = 0;
   return;
  }
  tintless->tintless_data.is_enabled = *enabled;
}

/** isp_tintless_be_config:
 *
 *    @session:
 *
 *  Return 0 on success, -1 on error
 **/
int isp_tintless_be_config(isp_tintless_session_t *session,
  isp_tintless_notify_data_t *tintless_data)
{
  int rc = 0;
  tintless_stats_config_t *stats_cfg = NULL;

  if (session == NULL) {
    CDBG_ERROR("%s: invalid session\n", __func__);
    rc = -1;
    goto error;
  }

  if (session->tinstance == NULL) {
    CDBG_ERROR("%s: session closed\n", __func__);
    rc = -1;
    goto error;
  }

  if (tintless_data == NULL) {
    CDBG_ERROR("%s: invalid data\n", __func__);
    rc = -1;
    goto error;
  }

  if (tintless_data->notify_data_size != sizeof(tintless_stats_config_t)) {
    CDBG_ERROR("%s: Type mismatch\n", __func__);
    rc = -1;
    goto error;
  }
  stats_cfg = tintless_data->notify_data;

  ISP_DBG(ISP_MOD_COM,"%s: tintless be_stats_cfg: camif_w = %d, camif_h = %d,"
    "elem_w = %d, elem_h = %d, num_stat_elem_rows = %d, num_stat_elem_cols ="
    "%d\n",__func__, stats_cfg->camif_win_w, stats_cfg->camif_win_h,
    stats_cfg->stat_elem_w, stats_cfg->stat_elem_h,
    stats_cfg->num_stat_elem_rows, stats_cfg->num_stat_elem_cols);

  /* config BE stats params to library */
  if (session->update_stats) {
    rc = isp_tintless_stat_config(session->tinstance, stats_cfg);
    if (rc != TINTLESS_SUCCESS)
      CDBG_ERROR("%s: error: stats - returned %d", __func__, rc);
  }

error:
  return rc;
}

/** isp_tintless_bg_config:
 *
 *    @session:
 *
 *  Return 0 on success, -1 on error
 **/
int isp_tintless_bg_config(isp_tintless_session_t *session,
  isp_tintless_notify_data_t *tintless_data)
{
  int rc = 0;
  tintless_stats_config_t *stats_cfg;

  if (session == NULL) {
    CDBG_ERROR("%s: invalid session\n", __func__);
    rc = -1;
    goto error;
  }

  if (session->tinstance == NULL) {
    CDBG_ERROR("%s: session closed\n", __func__);
    rc = -1;
    goto error;
  }

  if (tintless_data == NULL) {
    CDBG_ERROR("%s: invalid data\n", __func__);
    rc = -1;
    goto error;
  }

  if (tintless_data->notify_data_size != sizeof(tintless_stats_config_t)) {
    CDBG_ERROR("%s: Type mismatch\n", __func__);
    rc = -1;
    goto error;
  }

  stats_cfg = tintless_data->notify_data;

  /* config BE stats params to library */
  if (session->update_stats) {
    rc = isp_tintless_stat_config(session->tinstance, stats_cfg);
    if (rc != TINTLESS_SUCCESS)
      CDBG_ERROR("%s: error: stats - returned %d", __func__, rc);
  }

error:
  return rc;
}
/** isp_tintless_chroma_config:
 *
 *    @session:
 *
 *  Return 0 on success, -1 on error
 **/
int isp_tintless_chroma_config(isp_tintless_session_t *session,
  void *chromatix_ptr)
{
  int rc = 0;
  chromatix_parms_type *p_chromatix = chromatix_ptr;

  if (session == NULL) {
    CDBG_ERROR("%s: invalid session\n", __func__);
    rc = -1;
    goto error;
  }

  if (session->tinstance == NULL) {
    CDBG_ERROR("%s: session closed\n", __func__);
    rc = -1;
    goto error;
  }

  if (p_chromatix == NULL) {
    CDBG_ERROR("%s: chromatix not initialized\n", __func__);
    rc = -1;
    goto error;
  }

  /* config Chromatix params to library*/
  if (session->update_chromatix) {
    chromatix_color_tint_correction_type *tintless_strength =
      &(p_chromatix->chromatix_post_processing.chromatix_color_tint_correction);

    rc = isp_tintless_update_chromatix_params(session->tinstance, tintless_strength);
    if (rc != TINTLESS_SUCCESS)
      CDBG_ERROR("%s:  error: chromatix rc: %d", __func__, rc);
  }

error:
  return rc;
}

/** isp_tintless_rolloff_config:
 *
 *    @session:
 *
 *  Return 0 on success, -1 on error
 **/
int isp_tintless_rolloff_config(isp_tintless_session_t *session,
  isp_tintless_notify_data_t *tintless_data)
{
  int rc = 0;
  isp_tintless_mesh_config_t *isp_mesh_cfg;

  if (session == NULL) {
    CDBG_ERROR("%s: invalid session\n", __func__);
    rc = -1;
    goto error;
  }

  if (session->tinstance == NULL) {
    CDBG_ERROR("%s: session closed\n", __func__);
    rc = -1;
    goto error;
  }

  if (tintless_data == NULL) {
    CDBG_ERROR("%s: invalid data\n", __func__);
    rc = -1;
    goto error;
  }

  if (tintless_data->notify_data_size != sizeof(isp_tintless_mesh_config_t)) {
    CDBG_ERROR("%s: Type mismatch\n", __func__);
    rc = -1;
    goto error;
  }

  isp_mesh_cfg = tintless_data->notify_data;

  /* Check table size and set mesh_fixed only once */
  if (session->mesh_fixed.table_size == 0) {
    session->mesh_fixed = isp_mesh_cfg->mesh_fixed;
    session->mesh_hw = isp_mesh_cfg->mesh_hw;
    session->curr_rolloff_hw = isp_mesh_cfg->mesh_fixed;
  }

  ISP_DBG(ISP_MOD_COM,"%s: tintless_mesh_cfg: num_rows = %d, num_cols = %d,"
    "offset_Hori =%d, offset_Vert = %d, subgrid_Width = %d,subgrid_Height ="
    "%d\n", __func__, isp_mesh_cfg->mesh_cfg.num_mesh_elem_rows,
    isp_mesh_cfg->mesh_cfg.num_mesh_elem_cols,
    isp_mesh_cfg->mesh_cfg.offset_horizontal,
    isp_mesh_cfg->mesh_cfg.offset_vertical,
    isp_mesh_cfg->mesh_cfg.subgrid_width,
    isp_mesh_cfg->mesh_cfg.subgrid_height);

  /* config ROLLOFF params to library */
  if (session->update_mesh) {
    rc = isp_tintless_mesh_config(session->tinstance, &(isp_mesh_cfg->mesh_cfg));
    if (rc != TINTLESS_SUCCESS)
      CDBG_ERROR("%s: error: stats - returned %d", __func__, rc);
  }

error:
  return rc;
}

/** isp_tintless_parse_stats:
 *
 *    @be_stats: in
 *    @mod_stats: out
 *
 **/
static void isp_tintless_parse_stats(void *stats,
  tintless_stats_t *mod_stats, int type, uint32_t frame_id)
{
  uint32_t array_length;
  q3a_be_stats_t *be_stats;
  q3a_bg_stats_t *bg_stats;
  int32_t dump_to_file;
  char value[PROPERTY_VALUE_MAX];

  if (type == MSM_ISP_STATS_BE) {
    /* unpack stats to tintless format */
    be_stats = stats;

    property_get("persist.camera.tintless.dump", value, "0");
    dump_to_file = atoi(value);
    if (dump_to_file)
      isp_tintless_debug_BE(be_stats, frame_id);

    array_length = be_stats->be_region_h_num * be_stats->be_region_v_num;
    mod_stats->r.array_length    = array_length;
    mod_stats->gr.array_length   = array_length;
    mod_stats->gb.array_length   = array_length;
    mod_stats->b.array_length    = array_length;
    mod_stats->r.channel_sums    = be_stats->be_r_sum;
    mod_stats->b.channel_sums    = be_stats->be_b_sum;
    mod_stats->gr.channel_sums   = be_stats->be_gr_sum;
    mod_stats->gb.channel_sums   = be_stats->be_gb_sum;
    mod_stats->r.channel_counts  = be_stats->be_r_num;
    mod_stats->b.channel_counts  = be_stats->be_b_num;
    mod_stats->gr.channel_counts = be_stats->be_gr_num;
    mod_stats->gb.channel_counts = be_stats->be_gb_num;
  } else if (type == MSM_ISP_STATS_BG) {
    bg_stats = stats;
    array_length = 3072;
    mod_stats->r.array_length    = array_length;
    mod_stats->gr.array_length   = array_length;
    mod_stats->gb.array_length   = array_length;
    mod_stats->b.array_length    = array_length;
    mod_stats->r.channel_sums    = bg_stats->bg_r_sum;
    mod_stats->b.channel_sums    = bg_stats->bg_b_sum;
    mod_stats->gr.channel_sums   = bg_stats->bg_gr_sum;
    mod_stats->gb.channel_sums   = bg_stats->bg_gb_sum;
    mod_stats->r.channel_counts  = bg_stats->bg_r_num;
    mod_stats->b.channel_counts  = bg_stats->bg_b_num;
    mod_stats->gr.channel_counts = bg_stats->bg_gr_num;
    mod_stats->gb.channel_counts = bg_stats->bg_gb_num;
  }
}

/** isp_tintless_trigger_update:
 *
 *    @session:
 *
 *  Return 0 on success, -1 on error
 **/
int isp_tintless_trigger_update(isp_tintless_session_t *session,
  void *stats, int type, int hfr_mode)
{
  int rc =0;
  int skip_count = TINTLESS_ALGO_SKIP_COUNT;

  if (session == NULL) {
    CDBG_ERROR("%s: invalid session\n", __func__);
    rc = -1;
    goto error;
  }

  if (session->tinstance == NULL) {
    CDBG_ERROR("%s: session closed\n", __func__);
    rc = -1;
    goto error;
  }

  /* Tintless needs to be applied at 30FPS in case of BG stats.
     This is especially applicable to VFE32 where stats are not
     dropped in hardware */
  if (type == MSM_ISP_STATS_BG) {
    switch (hfr_mode) {
      case CAM_HFR_MODE_60FPS:
        skip_count = 2;
        break;
      case CAM_HFR_MODE_90FPS:
        skip_count = 3;
        break;
      case CAM_HFR_MODE_120FPS:
        skip_count = 4;
        break;
      case CAM_HFR_MODE_150FPS:
        skip_count = 5;
        break;
      case CAM_HFR_MODE_OFF:
      default:
        skip_count = 1;
        break;
    }
  }

  /*configurable tintless algo running frequency
    value 1 means run every frame*/
  if ((session->frame_id % skip_count) != 0) {
     ISP_DBG(ISP_MOD_COM,"%s: skip algo for frame %d, skip_count = %d\n",
       __func__, session->frame_id, skip_count);
     return 0;
  }

  ISP_DBG(ISP_MOD_COM,"%s: frame_id: %d\n", __func__, session->frame_id);

  pthread_mutex_lock(&session->thread_data.lock_mutex);
  if(!session->thread_data.algo_requested) {
    /* prepare stats data for the algo */
    isp_tintless_parse_stats(stats, &session->thread_data.mod_stats, type, session->frame_id);

    session->thread_data.in_frame_id = session->frame_id;
    session->thread_data.algo_requested = 1;
    sem_post(&session->thread_data.wait_sem);
    pthread_mutex_unlock(&session->thread_data.lock_mutex);
  } else {
    pthread_mutex_unlock(&session->thread_data.lock_mutex);
  }

error:
  return rc;
}

/** isp_tintless_get_table:
 *
 *    @session:
 *
 *  Return 0 on success, -1 on error
 **/
int isp_tintless_get_table(isp_tintless_session_t *session,
  isp_tintless_notify_data_t *tintless_data_out)
{
  int rc =0;
  int i;
  uint32_t ready_frame_id;

  tintless_mesh_rolloff_array_t *mesh_hw_out;

  if (session == NULL) {
    CDBG_ERROR("%s: invalid session\n", __func__);
    rc = -1;
    goto error;
  }

  if (session->tinstance == NULL) {
    CDBG_ERROR("%s: session closed\n", __func__);
    rc = -1;
    goto error;
  }

  if (tintless_data_out == NULL) {
    CDBG_ERROR("%s: invalid data\n", __func__);
    rc = -1;
    goto error;
  }

  if (tintless_data_out->notify_data_size != sizeof(tintless_mesh_rolloff_array_t)) {
    CDBG_ERROR("%s: Type mismatch\n", __func__);
    rc = -1;
    goto error;
  }

  mesh_hw_out = tintless_data_out->notify_data;

  pthread_mutex_lock(&session->thread_data.lock_mutex);
  ISP_DBG(ISP_MOD_COM,"%s: fetch tintles table!\n", __func__);
  *mesh_hw_out = session->mesh_hw;
  ready_frame_id = session->ready_frame_id;
  pthread_mutex_unlock(&session->thread_data.lock_mutex);

  ISP_DBG(ISP_MOD_COM,"%s: frame_id: %d\n", __func__, ready_frame_id);

error:
  return rc;
}
