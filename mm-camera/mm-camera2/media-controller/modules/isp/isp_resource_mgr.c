/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/media.h>
#include <media/msmb_isp.h>

#include "camera_dbg.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_resource_mgr.h"
#include "isp_hw.h"
#include "isp_log.h"
#include"isp.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#if 0
#undef CDBG
#define CDBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#undef CDBG_HIGH
#define CDBG_HIGH ALOGE

#define RM_RDI_POLICY_ONE_CID_ONE_RDI (1)
#define RM_RDI_POLICY_MIMIC_PIX       (2)
#define RM_RDI_POLICY_CURRENT         RM_RDI_POLICY_MIMIC_PIX

static isp_resources_t isp_res_mgr;

/** get_dual_vfe_session_id
 *
 * DESCRIPTION:
 *
 **/
boolean get_dual_vfe_session_id(int *session_idx)
{
  int i;

  *session_idx = 0;

  pthread_mutex_lock(&isp_res_mgr.mutex);

  if (isp_res_mgr.num_isps < 2) {
    ISP_DBG(ISP_MOD_COM,"%s: no dual VFE support\n", __func__);
    pthread_mutex_unlock(&isp_res_mgr.mutex);
    return FALSE;
  }

  if (isp_res_mgr.used_mask[0].camif_mask &&
    (isp_res_mgr.used_mask[0].camif_mask ==
    isp_res_mgr.used_mask[1].camif_mask)) {
    ISP_DBG(ISP_MOD_COM,"%s: camif_mask[0] = 0x%x, camif_mask[1] = 0x%x\n", __func__,
      isp_res_mgr.used_mask[0].camif_mask, isp_res_mgr.used_mask[1].camif_mask);

    for (i = 0; i < ISP_MAX_SESSIONS; i++) {
      if ((int)isp_res_mgr.used_mask[0].camif_mask == (1 << i)) {
        *session_idx = i;
        pthread_mutex_unlock(&isp_res_mgr.mutex);
        return TRUE;
      }
    }
  }

  pthread_mutex_unlock(&isp_res_mgr.mutex);

  return FALSE;
}

/** has_isp_pix_interface
 *
 * DESCRIPTION:
 *
 **/
boolean has_isp_pix_interface(void)
{
  int i;

  pthread_mutex_lock(&isp_res_mgr.mutex);
  for (i = 0; i < isp_res_mgr.num_isps; i++) {
    if (isp_res_mgr.used_mask[i].camif_mask == 0) {
      ISP_DBG(ISP_MOD_COM,"%s: ISP %d has free camif interface\n", __func__, i);
      pthread_mutex_unlock(&isp_res_mgr.mutex);
      return TRUE;
    }
  }
  pthread_mutex_unlock(&isp_res_mgr.mutex);

  return FALSE;
}

/** isp_get_info:
 *
 *  @info: isp_info_t pointer to array with size of at least VFE_MAX
 *
 * Provide information about the currently available VFEs on the chipset.
 *
 *  Return: the number of presented VFEs in the system
 *
 **/
int isp_get_info(isp_info_t *info)
{
  memcpy(info, isp_res_mgr.vfe_info, sizeof(isp_res_mgr.vfe_info));

  return isp_res_mgr.num_isps;
}

/** isp_set_info:
 *
 *  @num_isps: number of isps available
 *  @info: isp_info_t pointer to array with VFEs info
 *
 *  Save information about the currently available VFEs on the chipset.
 *
 *  Return: void
 *
 **/
void isp_set_info(int num_isps, isp_info_t *info)
{
  isp_res_mgr.num_isps = num_isps;
  memcpy(isp_res_mgr.vfe_info, info, sizeof(isp_res_mgr.vfe_info));
}

/** isp_is_camif_raw
 *
 * DESCRIPTION:
 *
 **/
static boolean isp_is_camif_raw(mct_stream_info_t *stream_info)
{
  switch (stream_info->fmt) {
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
    return TRUE;

  default:
    return FALSE;
  }

  return FALSE;
}

/** decide_isp_nums_camif
 *
 * DESCRIPTION:
 *
 **/
static int decide_isp_nums_camif(mct_stream_info_t *stream_info,
  sensor_dim_output_t *dim_output, uint32_t op_pix_clk)
{
  int num_isps = 0;
  int32_t enabled = 0;
  char value[PROPERTY_VALUE_MAX];

  if (isp_res_mgr.num_isps == 1)
    num_isps = 1;
  else if (isp_res_mgr.num_isps == 2) {
    /* (1) in multi camera use case we try to use turbo mode instead
     *     of dual vfe for one session.
     * (2) In single camera use case we try to use dual vfe to avoid
     *     turbo mode if isp_res_mgr.avoid_turbo is TRUE
     * (3) Use single ISP for camif/ideal raw dump */
    if (isp_res_mgr.avoid_turbo == FALSE || isp_res_mgr.isp_session_cnt > 1) {
      if (isp_res_mgr.vfe_info[0].max_pix_clk < op_pix_clk) {
        num_isps = 2;
      } else
        num_isps = 1;
    } else {
      /* we use dual VFE if we hitting the max isp clock to avoid turbo mode */
      if (isp_res_mgr.vfe_info[0].max_pix_clk <= op_pix_clk) {
        num_isps = 2;
      } else
        num_isps = 1;
    }

    if (isp_is_camif_raw(stream_info))
      num_isps = 1;

    /*set property to enable dual vfe*/
    property_get("persist.camera.isp.dualisp", value, "0");
    enabled = atoi(value);
    if (enabled == 1) {
      num_isps = 2;
      CDBG_ERROR("%s: Enforce enable dual vfe!!\n", __func__);
    }
  } else
    CDBG_ERROR("%s: error, num_isps = 0\n", __func__);

  CDBG_ERROR("%s: num_isp = %d\n", __func__, num_isps);
  return num_isps;
}

/** is_isp_used_by_session
 *
 * DESCRIPTION:
 *
 **/
static boolean is_isp_used_by_session(int isp_id, uint32_t session_bit)
{
  isp_interface_session_mask_t *isp_mask = &isp_res_mgr.used_mask[isp_id];
  int i;
  if (isp_mask->camif_mask & session_bit)
    return TRUE;
  else {
    for (i = 0; i < ISP_RM_NUM_RDI; i++)
      if (isp_mask->rdi_mask[i] & session_bit)
        return TRUE;
  }
  return FALSE;
}

/** get_camif_resource
 *
 * DESCRIPTION:
 *
 **/
static int get_camif_resource(int isp_id, uint32_t session_bit,
  uint32_t *isp_interface_mask, uint32_t *isp_id_mask, uint32_t cid)
{
  isp_interface_session_mask_t *isp_mask = &isp_res_mgr.used_mask[isp_id];

  if (isp_mask->camif_mask & session_bit) {
    /* reuse the camif */
    *isp_interface_mask = (1 << (16 * isp_id + ISP_INTF_PIX));
    *isp_id_mask = (1 << isp_id);
    return 0;
  } else if (isp_mask->camif_mask == 0) {
    isp_mask->camif_mask = session_bit;
    isp_mask->camif_cid = cid;
    *isp_interface_mask = (1 << (16 * isp_id + ISP_INTF_PIX));
    *isp_id_mask = (1 << isp_id);
    return 0;
  } else
    return -1;
}

/** reserve_camif_resource
 *
 * DESCRIPTION:
 *
 **/
static int reserve_camif_resource(boolean is_ispif,
  sensor_src_port_cap_t *sensor_cap, mct_stream_info_t *stream_info,
  sensor_dim_output_t *dim_output, uint32_t cid, uint32_t op_pix_clk,
  uint32_t session_idx, uint32_t *isp_interface_mask, uint32_t *isp_id_mask)
{
  int rc = -1;
  uint8_t isp_fetch = 0;
  uint32_t session_bit = 0;
  int num_isps = 0;
  int isp_id = 0;
  boolean sess_uses_isp1 = FALSE;

  /*only fetch engine will turn on isp_fetch*/
  if (!is_ispif)
    isp_fetch = 1;

  session_bit = ISP_SESSION_BIT(isp_fetch, session_idx);
  num_isps = decide_isp_nums_camif(stream_info, dim_output, op_pix_clk);
  ISP_DBG(ISP_MOD_COM,"%s: is_ispif = %d, sess_idx = %d, num_isps = %d\n",
       __func__, is_ispif, session_idx, num_isps);

  if (num_isps == 1) {
    isp_id = 0;
    if (is_isp_used_by_session(isp_id, session_bit)) {
      /* isp0 is used by the session. get camif from isp 0 */
      rc = get_camif_resource(isp_id, session_bit,
        isp_interface_mask, isp_id_mask, cid);
    } else if ((is_isp_used_by_session(1, session_bit))){
      /* session is associated with isp 1 */
      isp_id = 1;
      rc = get_camif_resource(isp_id, session_bit,
        isp_interface_mask, isp_id_mask, cid);
    } else {
      if (isp_res_mgr.used_mask[0].camif_mask == 0) {
        isp_id = 0;
        rc = get_camif_resource(isp_id, session_bit,
          isp_interface_mask, isp_id_mask, cid);
      } else if (isp_res_mgr.used_mask[1].camif_mask == 0) {
        isp_id = 1;
        rc = get_camif_resource(isp_id, session_bit,
          isp_interface_mask, isp_id_mask, cid);
      }
    }
  } else {
    if ((isp_res_mgr.used_mask[0].camif_mask & session_bit ||
      isp_res_mgr.used_mask[0].camif_mask == 0) &&
      (isp_res_mgr.used_mask[1].camif_mask & session_bit ||
      isp_res_mgr.used_mask[1].camif_mask == 0)) {
      isp_res_mgr.used_mask[0].camif_mask = session_bit;
      isp_res_mgr.used_mask[1].camif_mask = session_bit;

      *isp_interface_mask =
        ((1 << (16 + ISP_INTF_PIX)) | (1 << ISP_INTF_PIX));
      *isp_id_mask = 0x3;
      rc = 0;
    }
  }

  return rc;
}

/** get_rdi_resource
 *
 * DESCRIPTION:
 *
 **/
static int get_rdi_resource(int isp_id, uint32_t session_bit,
  uint32_t *isp_interface_mask, uint32_t *isp_id_mask, uint32_t cid,
  cam_stream_type_t stream_type)
{
  int i;
  isp_interface_session_mask_t *isp_mask = &isp_res_mgr.used_mask[isp_id];

#if (RM_RDI_POLICY_CURRENT == RM_RDI_POLICY_ONE_CID_ONE_RDI)
  for (i = 0; i < ISP_RM_NUM_RDI; i++) {
    if ((isp_mask->rdi_mask[i] != 0) && (isp_mask->rdi_cid[i] == (1u << cid)))
    {
      /* if stream is connected with same cid - reuse RDI */
      *isp_interface_mask = (1 << (16 * isp_id + (ISP_INTF_RDI0 + i)));
      *isp_id_mask = (1 << isp_id);
      isp_mask->rdi_streams[i] |= 1u << stream_type;
      return 0;
    }
  }
#elif (RM_RDI_POLICY_CURRENT == RM_RDI_POLICY_MIMIC_PIX)
  if((stream_type == CAM_STREAM_TYPE_PREVIEW) ||
      (stream_type == CAM_STREAM_TYPE_VIDEO)) {
    for (i = 0; i < ISP_RM_NUM_RDI; i++) {
      if ((isp_mask->rdi_mask[i] != 0) &&
          ((isp_mask->rdi_streams[i] & (1u << CAM_STREAM_TYPE_PREVIEW)) ||
            (isp_mask->rdi_streams[i] & (1u << CAM_STREAM_TYPE_VIDEO))))
      {
        /* if stream is connected with same cid - reuse RDI */
        *isp_interface_mask = (1 << (16 * isp_id + (ISP_INTF_RDI0 + i)));
        *isp_id_mask = (1 << isp_id);
        isp_mask->rdi_streams[i] |= 1u << stream_type;
        return 0;
      }
    }
  }
#endif

  for (i = 0; i < ISP_RM_NUM_RDI; i++) {
    if (isp_mask->rdi_mask[i] == 0) {
      isp_mask->rdi_mask[i] = session_bit;
      isp_mask->rdi_cid[i] = (1u << cid);
      isp_mask->rdi_streams[i] = (1u << stream_type);
      *isp_interface_mask = (1 << (16 * isp_id + (ISP_INTF_RDI0 + i)));
      *isp_id_mask = (1 << isp_id);
      return 0;
    }
  }
  return -1;
}

/** reserve_rdi_resource
 *
 * DESCRIPTION:
 *
 **/
static int reserve_rdi_resource(boolean is_ispif, uint32_t session_idx,
 uint32_t *isp_interface_mask, uint32_t *isp_id_mask, uint32_t cid,
  cam_stream_type_t stream_type)
{
  int rc = 0;
  uint8_t isp_fetch = 0;
  uint32_t session_bit = 0;
  int num_isps = 0;
  ISP_DBG(ISP_MOD_COM,"%s: E\n",  __func__);

  /*only when we use fetch engine, the isp_fetch =1*/
  if (!is_ispif)
    isp_fetch = 1;
  session_bit = ISP_SESSION_BIT(isp_fetch, session_idx);

  if (is_isp_used_by_session(VFE0, session_bit)) {
    /* isp0 is used by the session. get RDI from isp 0 */
    rc = get_rdi_resource(VFE0, session_bit, isp_interface_mask, isp_id_mask,
      cid, stream_type);
  } else if (is_isp_used_by_session(VFE1, session_bit)) {
    rc = get_rdi_resource(VFE1, session_bit, isp_interface_mask, isp_id_mask,
      cid, stream_type);
  } else {
    /* One VFE can support 1 bayer camera and 1 RDI camera (RDI based PIP).
     * But RDI based PIP has not been implemented. Now, for pure
     * RDI camera we force to use VFE1 as a short time solution. This policy
     * logic needs to be revisited when implementing RDI PIP. */
    if (isp_res_mgr.num_isps > 1)
      rc = get_rdi_resource(VFE1, session_bit, isp_interface_mask, isp_id_mask,
        cid, stream_type);
    else
      rc = get_rdi_resource(VFE0, session_bit, isp_interface_mask, isp_id_mask,
        cid, stream_type);
  }
  return rc;
}

/** dump_isp_res_mask
 *
 * DESCRIPTION:
 *
 **/
static void dump_isp_res_mask()
{
  ISP_DBG(ISP_MOD_COM,"%s: isp0: camif = 0x%x, rdi0 = 0x%x, rdi1 = 0x%x, rdi2 = 0x%x",
     __func__, isp_res_mgr.used_mask[0].camif_mask,
       isp_res_mgr.used_mask[0].rdi_mask[0],
       isp_res_mgr.used_mask[0].rdi_mask[1],
       isp_res_mgr.used_mask[0].rdi_mask[2]);
  ISP_DBG(ISP_MOD_COM,"%s: isp1: camif = 0x%x, rdi0 = 0x%x, rdi1 = 0x%x, rdi2 = 0x%x",
       __func__, isp_res_mgr.used_mask[1].camif_mask,
       isp_res_mgr.used_mask[1].rdi_mask[0],
       isp_res_mgr.used_mask[1].rdi_mask[1],
       isp_res_mgr.used_mask[1].rdi_mask[2]);
}

/** reserve_isp_resource
 *
 * DESCRIPTION:
 *
 **/
int reserve_isp_resource(boolean use_pix, boolean is_ispif,
  sensor_src_port_cap_t *sensor_cap, mct_stream_info_t *stream_info,
  sensor_dim_output_t *dim_output, uint32_t cid, uint32_t op_pix_clk,
  uint32_t session_idx, uint32_t *isp_interface_mask, uint32_t *isp_id_mask)
{
  int rc = 0;
  cam_stream_type_t stream_type;

  pthread_mutex_lock(&isp_res_mgr.mutex);
  if (use_pix) {
    rc = reserve_camif_resource(is_ispif,
      sensor_cap, stream_info, dim_output, cid,
      op_pix_clk, session_idx, isp_interface_mask, isp_id_mask);
  } else {
    if(stream_info)
      stream_type = stream_info->stream_type;
    else
      stream_type = CAM_STREAM_TYPE_METADATA;
    rc = reserve_rdi_resource(is_ispif, session_idx,
      isp_interface_mask, isp_id_mask, cid, stream_type);
  }
  if(rc < 0)
    CDBG_ERROR("%s: Error reserving %s resource\n",
      __func__, use_pix?"camif":"RDI");

  ISP_DBG(ISP_MOD_COM,"%s: session_idx = %d, isp_mask = 0x%x, isp_interface_mask = 0x%x",
       __func__, session_idx, *isp_id_mask, *isp_interface_mask);
  dump_isp_res_mask();
  pthread_mutex_unlock(&isp_res_mgr.mutex);

  return rc;
}

/** release_isp_resource
 *
 * DESCRIPTION:
 *
 **/
int release_isp_resource(boolean is_ispif, uint32_t session_idx,
  uint32_t isp_interface_mask, uint32_t isp_id_mask)
{
  isp_interface_session_mask_t *isp_mask = NULL;
  uint8_t isp_fetch = 0;
  uint32_t session_bit = 0;
  int num_isps = 0;
  int isp_id = VFE0;
  int intf_idx = 0;
  int rc = 0;

  pthread_mutex_lock(&isp_res_mgr.mutex);
  if (!is_ispif)
    isp_fetch = 1;

  session_bit = ISP_SESSION_BIT(isp_fetch, session_idx);
  if (isp_id_mask & (1 << VFE0)) {
    /* release isp0 rdi */
    isp_mask = &isp_res_mgr.used_mask[isp_id];
    intf_idx = isp_interface_mask_to_interface_num(isp_interface_mask,
      (1 << isp_id));
    if (intf_idx < 0 || intf_idx >= ISP_INTF_MAX) {
      CDBG_ERROR("%s:  invalid interface num %d\n", __func__, intf_idx);
      rc = -1;
      goto vfe1;
    }
    if (intf_idx == ISP_INTF_PIX) {
      isp_mask->camif_mask &= ~session_bit;
      ISP_DBG(ISP_MOD_COM,"%s: isp_id = %d, camif_session_bit = 0x%x\n",
        __func__, isp_id, isp_mask->camif_mask);
    } else if (intf_idx == ISP_INTF_RDI0) {
      isp_mask->rdi_mask[0] &= ~session_bit;
      isp_mask->rdi_cid[0] = 0;
      isp_mask->rdi_streams[0] = 0;
    } else if (intf_idx == ISP_INTF_RDI1) {
      isp_mask->rdi_mask[1] &= ~session_bit;
      isp_mask->rdi_cid[1] = 0;
      isp_mask->rdi_streams[1] = 0;
    } else if (intf_idx == ISP_INTF_RDI2) {
      isp_mask->rdi_mask[2] &= ~session_bit;
      isp_mask->rdi_cid[2] = 0;
      isp_mask->rdi_streams[2] = 0;
    } else
      rc = -1;
  }
vfe1:
  if (isp_id_mask & (1 << VFE1)) {
    /* release isp1 rdi */
    isp_id = VFE1;
    isp_mask = &isp_res_mgr.used_mask[isp_id];
    intf_idx = isp_interface_mask_to_interface_num(isp_interface_mask,
      (1 << isp_id));
    if (intf_idx < 0 || intf_idx >= ISP_INTF_MAX) {
      CDBG_ERROR("%s: invalid interface num %d\n", __func__, intf_idx);
      rc = -1;
      goto end;
    }
    if (intf_idx == ISP_INTF_PIX) {
      isp_mask->camif_mask &= ~session_bit;
      ISP_DBG(ISP_MOD_COM,"%s: isp_id = %d, camif_session_bit = 0x%x\n",
        __func__, isp_id, isp_mask->camif_mask);
    } else if (intf_idx == ISP_INTF_RDI0) {
      isp_mask->rdi_mask[0] &= ~session_bit;
      isp_mask->rdi_cid[0] = 0;
      ISP_DBG(ISP_MOD_COM,"%s: isp_id = %d, rdi0_session_bit = 0x%x\n",
        __func__, isp_id, isp_mask->rdi_mask[0]);
    } else if (intf_idx == ISP_INTF_RDI1) {
      isp_mask->rdi_mask[1] &= ~session_bit;
      isp_mask->rdi_cid[1] = 0;
      ISP_DBG(ISP_MOD_COM,"%s: isp_id = %d, rdi1_session_bit = 0x%x\n",
        __func__, isp_id, isp_mask->rdi_mask[1]);
    } else if (intf_idx == ISP_INTF_RDI2) {
      isp_mask->rdi_mask[2] &= ~session_bit;
      isp_mask->rdi_cid[2] = 0;
      ISP_DBG(ISP_MOD_COM,"%s: isp_id = %d, rdi2_session_bit = 0x%x\n",
        __func__, isp_id, isp_mask->rdi_mask[2]);
    } else
      rc = -1;
  }

end:
  ISP_DBG(ISP_MOD_COM,"%s: session_idx = %d, isp_mask = 0x%x, isp_interface_mask = 0x%x",
    __func__, session_idx, isp_id_mask, isp_interface_mask);
  dump_isp_res_mask();
  pthread_mutex_unlock(&isp_res_mgr.mutex);

  return rc;
}

/** isp_interface_mask_to_interface_num
 *
 * DESCRIPTION:
 *
 **/
int isp_interface_mask_to_interface_num(uint32_t isp_interface_mask,
  uint32_t isp_id_mask)
{
  if ((1 << VFE1) & isp_id_mask) {
    /* it's isp1 */
    isp_interface_mask = isp_interface_mask >> 16;
  }

  if (isp_interface_mask & (1 << ISP_INTF_PIX))
    return ISP_INTF_PIX;
  else if (isp_interface_mask & (1 << ISP_INTF_RDI0))
    return ISP_INTF_RDI0;
  else if (isp_interface_mask & (1 << ISP_INTF_RDI1))
    return ISP_INTF_RDI1;
  else if (isp_interface_mask & (1 << ISP_INTF_RDI2))
    return ISP_INTF_RDI2;
  else
    return -1;
}

/** choose_isp_interface:
 *
 *  Return: void
 *
 **/
void choose_isp_interface(sensor_out_info_t *sensor_info,
  sensor_src_port_cap_t *sensor_cap, mct_stream_info_t *stream_info,
  uint8_t *use_pix)
{
  uint32_t primary_cid_idx = isp_hw_find_primary_cid(sensor_info, sensor_cap);
  if (SENSOR_CID_CH_MAX - 1 < primary_cid_idx) {
    CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
    return;
  }
  *use_pix = 0;
  if ((stream_info->fmt >= CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG &&
    stream_info->fmt <= CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR) ||
    stream_info->fmt == CAM_FORMAT_JPEG_RAW_8BIT ||
    stream_info->fmt == CAM_FORMAT_META_RAW_8BIT ||
    stream_info->fmt == CAM_FORMAT_META_RAW_10BIT) {
    /* For MIPI RAW format, use RDI interface to dump the data directly*/
    *use_pix = 0;
  } else if (stream_info->fmt !=
    sensor_cap->sensor_cid_ch[primary_cid_idx].fmt) {
    /* sensor output format does not match stream fmt. We need pix or PP.
     * By default ispif assign it to use vfe pix output.
     * Another rule is to push all non bayer camera to use RDI.
     * For specific OEM customozation is needed.
     * For Google stock solution we do not implement the case that different.
     * color plane using different CIDs. That can be achieved by using      .
     * sensor_cid_ch[1]. This is not implemented in this phase. */
    if ( isp_res_mgr.vfe_info[0].use_pix_for_SOC) {
      *use_pix = 1;
    } else if (sensor_cap->sensor_cid_ch[primary_cid_idx].fmt >=
      CAM_FORMAT_YUV_RAW_8BIT_YUYV &&
      sensor_cap->sensor_cid_ch[primary_cid_idx].fmt <=
      CAM_FORMAT_YUV_RAW_8BIT_VYUY) {
      *use_pix = 0;
    } else if (isp_res_mgr.vfe_info[0].use_pix_for_SOC){
      *use_pix = 1;
    } else {
      *use_pix = 1;
    }
  }
}

/** isp_resource_mgr_init
 *
 * DESCRIPTION:
 *
 **/
int isp_resource_mgr_init(uint32_t version, void *isp)
{
  memset(&isp_res_mgr, 0, sizeof(isp_res_mgr));
  pthread_mutex_init(&isp_res_mgr.mutex, NULL);
  isp_t * temp_isp = (isp_t *)isp;
  /* if enable avoid_turbo,
     we use dual vfe to avoid turbo mode in single camera. */
  if (ISP_VERSION_40 == GET_ISP_MAIN_VERSION(version)) {
    isp_res_mgr.num_isps = 2;
    isp_res_mgr.avoid_turbo = FALSE;
  } else {
    isp_res_mgr.avoid_turbo = FALSE; /* if FALSE disable the feature */
    isp_res_mgr.num_isps = 1;
  }
  isp_res_mgr.isp_ptr = isp;
 temp_isp->res_mgr = &isp_res_mgr;
  return 0;
}

/** isp_resouirce_mgr_destroy
 *
 * DESCRIPTION:
 *
 **/
void isp_resouirce_mgr_destroy()
{
  pthread_mutex_destroy(&isp_res_mgr.mutex);
  memset(&isp_res_mgr, 0, sizeof(isp_res_mgr));
}

/** increase_isp_session
 *
 * DESCRIPTION:
 *
 **/
void increase_isp_session()
{
  pthread_mutex_lock(&isp_res_mgr.mutex);
  isp_res_mgr.isp_session_cnt++;
  pthread_mutex_unlock(&isp_res_mgr.mutex);
}

/** isp_get_number_of_active_sessions
 *
 * DESCRIPTION
 *
 **/
int isp_get_number_of_active_sessions()
{
  int session_count = 0;
  pthread_mutex_lock(&isp_res_mgr.mutex);
  session_count = isp_res_mgr.isp_session_cnt;
  pthread_mutex_unlock(&isp_res_mgr.mutex);
  return session_count;
}

/** decrease_isp_session_cnt
 *
 * DESCRIPTION:
 *
 **/
void decrease_isp_session_cnt()
{
  pthread_mutex_lock(&isp_res_mgr.mutex);
  isp_res_mgr.isp_session_cnt--;
  pthread_mutex_unlock(&isp_res_mgr.mutex);
}



