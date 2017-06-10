/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include <linux/media.h>
#include "mct_module.h"
#include "module_cac.h"
#include "mct_stream.h"
#include "chromatix.h"


/**
 * Function: module_cac1_update_offline_params
 *
 * Description: This function is to update the CAC parameters
 * for offline usecase for cac v1.
 *
 * Arguments:
 *   @p_client: cac client
 *   @pframe: frame pointer
 *   @native_buf: flag to indicate if its a native buffer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_cac1_config_update_offline_params(cac_client_t *p_client)
{
  int status = IMG_SUCCESS;
  cam_metadata_info_t *metadata_buff = NULL;
  mct_stream_session_metadata_info* session_meta;
  awb_update_t awb_update_val;
  awb_update_t *p_awb_update = &awb_update_val;
  chromatix_parms_type *chromatix;
  chromatix_gamma_type *chromatix_gamma;
  img_gamma_t *p_gamma;
  int i;
  boolean ret_val;
  int g_gain = 127;
  int16_t *p_gamma_in;

  IDBG_MED("%s:%d] ", __func__, __LINE__);


  if (p_client->stream_info->reprocess_config.pp_type ==
      CAM_ONLINE_REPROCESS_TYPE) {
    metadata_buff = mct_module_get_buffer_ptr(
      p_client->stream_info->parm_buf.reprocess.meta_buf_index,
      p_client->parent_mod,
      IMGLIB_SESSIONID(p_client->identity),
      p_client->stream_info->parm_buf.reprocess.meta_stream_handle);

  } else {
    metadata_buff = module_imglib_common_get_metadata(p_client->stream_info,
        p_client->stream_info->parm_buf.reprocess.meta_buf_index);
  }

  if (!metadata_buff) {
    IDBG_ERROR("%s:%d] Invalid metadata buffer", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  session_meta =
    (mct_stream_session_metadata_info *)&metadata_buff->private_metadata;

  if (!(session_meta->sensor_data.chromatix_ptr
    && session_meta->sensor_data.common_chromatix_ptr)) {
    IDBG_ERROR("%s:%d] Invalid chromatix pointer", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  chromatix = session_meta->sensor_data.chromatix_ptr;
  chromatix_gamma = &(chromatix->chromatix_VFE.chromatix_gamma);
  memcpy(p_awb_update, session_meta->isp_stats_awb_data.private_data,
    sizeof(awb_update_t));

  p_gamma = &p_client->cac_cfg_info.r_gamma;

  for (i = 0; i < GAMMA_TABLE_ENTRIES; i++) {
#ifdef USE_CHROMATIX
    p_gamma->table[i] = chromatix_gamma->default_gamma_table.gamma[i];
#else
    p_gamma_in = (int16_t *)&session_meta->isp_gamma_data.private_data[0];
    p_gamma->table[i] = p_gamma_in[i];
#endif
  }

#ifdef DEBUG_GAMMA_TBL
  for (i = 0; i < GAMMA_TABLE_ENTRIES; i++) {
    ALOGE("gamma[%d] 0x%x", i, p_client->cac_cfg_info.r_gamma.table[i]);
  }
#endif
  p_client->cac_cfg_info.b_gamma = p_client->cac_cfg_info.r_gamma;
  p_client->cac_cfg_info.g_gamma = p_client->cac_cfg_info.r_gamma;

  g_gain = FLOAT_TO_Q(7, p_awb_update->gain.g_gain);
  g_gain = MIN(MAX(g_gain, 127), 381);
  p_client->cac_cfg_info.cac_3a_data.awb_gb_gain =
    p_client->cac_cfg_info.cac_3a_data.awb_gr_gain = g_gain;

  IDBG_MED("%s:%d] g_gain %d", __func__, __LINE__, g_gain);
  return status;
}

/**
 * Function: module_cac1_config_get_gamma
 *
 * Description: Get Gamma Tables from ISP
 *
 * Arguments:
 *   @p_client: cac client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_cac1_config_get_gamma(cac_client_t *p_client)
{
  mct_event_t get_gtbl_event;
  mct_isp_table_t gammatbl;
  int rc = IMG_SUCCESS;

  memset(&get_gtbl_event, 0x0, sizeof(mct_event_t));
  memset(&gammatbl, 0x0, sizeof(mct_isp_table_t));

  gammatbl.gamma_num_entries = GAMMA_TABLE_ENTRIES;
  gammatbl.gamma_table = &p_client->cac_cfg_info.r_gamma.table[0];
  get_gtbl_event.type = MCT_EVENT_MODULE_EVENT;
  get_gtbl_event.identity = p_client->identity;
  get_gtbl_event.direction = MCT_EVENT_UPSTREAM;
  get_gtbl_event.u.module_event.type = MCT_EVENT_MODULE_GET_ISP_TABLES;
  get_gtbl_event.u.module_event.module_event_data = &gammatbl;

  rc =  mct_port_send_event_to_peer(p_client->p_sinkport, &get_gtbl_event);
  if (!rc) {
    IDBG_ERROR("%s:%d] MCT_EVENT_MODULE_GET_ISP_TABLES failed : rc = %d",
      __func__, __LINE__, rc);
    return rc;
  }
  IDBG_MED("%s:%d] MCT_EVENT_MODULE_GET_ISP_TABLES Successful", __func__,
    __LINE__);
  p_client->cac_cfg_info.b_gamma = p_client->cac_cfg_info.r_gamma;
  p_client->cac_cfg_info.g_gamma = p_client->cac_cfg_info.r_gamma;

  p_client->cac_cfg_info.cac_3a_data.awb_gb_gain = 127;
  p_client->cac_cfg_info.cac_3a_data.awb_gr_gain = 127;

#ifdef DEBUG_GAMMA_TBL
  int i = 0;
  for (i = 0; i < GAMMA_TABLE_ENTRIES; i++) {
    ALOGE("gamma[%d] %d", i, p_client->cac_cfg_info.g_gamma.table[i]);
  }
#endif
  return rc;
}

/**
 * Function: module_cac_v1_config_client
 *
 * Description: This function configures the cac v1 component
 *
 * Arguments:
 *   @p_client: cac client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_v1_config_client(cac_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;

  IDBG_MED("%s:%d] type %d", __func__, __LINE__,
    p_client->stream_info->stream_type);

  if (CAM_STREAM_TYPE_OFFLINE_PROC == p_client->stream_info->stream_type) {
    rc = module_cac1_config_update_offline_params(p_client);
  } else {
    rc = module_cac1_config_get_gamma(p_client);
  }

  //Fill in the CAC data
  p_client->cac_cfg_info.chroma_order = CAC_CHROMA_ORDER_CRCB;

 //Need to update with the params from chromatix header after chromatix 303 is integrated
  p_client->cac_cfg_info.chromatix_info.edgeTH = 20;
  p_client->cac_cfg_info.chromatix_info.saturatedTH = 120;
  p_client->cac_cfg_info.chromatix_info.chrom0LowTH = 8;
  p_client->cac_cfg_info.chromatix_info.chrom0HighTH = 448;
  p_client->cac_cfg_info.chromatix_info.chrom1LowTH = 8;
  p_client->cac_cfg_info.chromatix_info.chrom1HighTH = 448;
  p_client->cac_cfg_info.chromatix_info.chrom0LowDiffTH = 192;
  p_client->cac_cfg_info.chromatix_info.chorm0HighDiffTH = 320;
  p_client->cac_cfg_info.chromatix_info.chrom1LowDiffTH = 192;
  p_client->cac_cfg_info.chromatix_info.chorm1HighDiffTH = 320;

  //Set the component to be executed in syncromous mode
  p_client->mode = IMG_SYNC_MODE;

  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_CHROMATIX_INFO,
    (void *)&(p_client->cac_cfg_info.chromatix_info));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_CHROMA_ORDER,
    (void *)&(p_client->cac_cfg_info.chroma_order));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set Chroma Order rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_RGAMMA_TABLE,
    (void*)&(p_client->cac_cfg_info.r_gamma));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set rGamma tables rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_GGAMMA_TABLE,
    (void*)&(p_client->cac_cfg_info.g_gamma));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set gGamma tables rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_BGAMMA_TABLE,
    (void*)&(p_client->cac_cfg_info.b_gamma));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set bGamma tables rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_3A_INFO,
    (void*)&(p_client->cac_cfg_info.cac_3a_data));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_MODE,
    (void*)&(p_client->mode));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return rc;
}
