/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_log.h"


#ifdef ENABLE_CV_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

extern isp_ops_t *chroma_enhan40_open(uint32_t version);
extern isp_ops_t *chroma_enhan32_open(uint32_t version);

/** ISP_MOD_COLOR_CONV_open:
 *
 *    @version:
 *
 **/
isp_ops_t *ISP_MOD_COLOR_CONV_open(uint32_t version)
{
  switch(version) {
  case ISP_VERSION_40:
    return chroma_enhan40_open(version);

  case ISP_VERSION_32:
    return chroma_enhan32_open(version);

  default:
    return NULL;
  }

  return NULL;
}
