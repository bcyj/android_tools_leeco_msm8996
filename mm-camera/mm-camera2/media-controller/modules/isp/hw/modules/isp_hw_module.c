/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "camera_dbg.h"
#include "isp_ops.h"
#include "isp_hw.h"
#include "isp_hw_module_ops.h"
#include "isp_hw_module_def.h"
#include "isp_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ENABLE_ISP_HW_M_LOGGING
  #undef CDBG
  #define CDBG ALOGE
#endif

/* ============================================================
 * function name: isp_module_open
 * description: open VFE pixel interface modules
 *        return NULL means no such module
 *        for that version hardware
 * ============================================================*/
isp_ops_t *isp_hw_module_open(uint32_t isp_version,
  isp_hw_module_id_t module_id)
{
  uint32_t main_version = GET_ISP_MAIN_VERSION(isp_version);

  ISP_DBG(ISP_MOD_COM,"%s: open module %d\n", __func__, module_id);

  switch (module_id) {
  case ISP_MOD_LINEARIZATION:
    return ISP_MOD_LINEARIZATION_open(main_version);

  case ISP_MOD_ROLLOFF:
    return ISP_MOD_ROLLOFF_open(main_version);

  case ISP_MOD_DEMUX:
    return ISP_MOD_DEMUX_open(main_version);

  case ISP_MOD_DEMOSAIC:
    return ISP_MOD_DEMOSAIC_open(main_version);

  case ISP_MOD_BPC:
    return ISP_MOD_BPC_open(main_version);

  case ISP_MOD_ABF:
    return ISP_MOD_ABF_open(main_version);

  case ISP_MOD_ASF:
    return ISP_MOD_ASF_open(main_version);

  case ISP_MOD_COLOR_CONV:
    return ISP_MOD_COLOR_CONV_open(main_version);

  case ISP_MOD_COLOR_CORRECT:
    return ISP_MOD_COLOR_CORRECT_open(main_version);

  case ISP_MOD_CHROMA_SS:
    return ISP_MOD_CHROMA_SS_open(main_version);

  case ISP_MOD_CHROMA_SUPPRESS:
    return ISP_MOD_CHROMA_SUPPRESS_open(main_version);

  case ISP_MOD_LA:
    return ISP_MOD_LA_open(main_version);

  case ISP_MOD_MCE:
    return ISP_MOD_MCE_open(main_version);

  case ISP_MOD_SCE:
    return ISP_MOD_SCE_open(main_version);

  case ISP_MOD_CLF:
    return ISP_MOD_CLF_open(main_version);

  case ISP_MOD_WB:
    return ISP_MOD_WB_open(main_version);

  case ISP_MOD_GAMMA:
    return ISP_MOD_GAMMA_open(main_version);

  case ISP_MOD_FOV:
    return ISP_MOD_FOV_open(main_version);

  case ISP_MOD_SCALER:
    return ISP_MOD_SCALER_open(main_version);

  case ISP_MOD_BCC:
    return ISP_MOD_BCC_open(main_version);

  case ISP_MOD_CLAMP:
    return ISP_MOD_CLAMP_open(main_version);

  case ISP_MOD_FRAME_SKIP:
    return ISP_MOD_FRAME_SKIP_open(main_version);

  case ISP_MOD_COLOR_XFORM:
    return ISP_MOD_COLOR_XFORM_open(main_version);

  case ISP_MOD_STATS:
    return ISP_MOD_STATS_open(isp_version);

  default:
    return NULL;
  }

  return NULL;
}


