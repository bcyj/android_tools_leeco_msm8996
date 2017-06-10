/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_HW_MODULE_DEF_H__
#define __ISP_HW_MODULE_DEF_H__

#include "isp_ops.h"


isp_ops_t *ISP_MOD_LINEARIZATION_open(uint32_t version);
isp_ops_t *ISP_MOD_ROLLOFF_open(uint32_t version);
isp_ops_t *ISP_MOD_DEMUX_open(uint32_t version);
isp_ops_t *ISP_MOD_DEMOSAIC_open(uint32_t version);
isp_ops_t *ISP_MOD_ABF_open(uint32_t version);
isp_ops_t *ISP_MOD_ASF_open(uint32_t version);
isp_ops_t *ISP_MOD_COLOR_CONV_open(uint32_t version);
isp_ops_t *ISP_MOD_COLOR_CORRECT_open(uint32_t version);
isp_ops_t *ISP_MOD_CHROMA_SS_open(uint32_t version);
isp_ops_t *ISP_MOD_CHROMA_SUPPRESS_open(uint32_t version);
isp_ops_t *ISP_MOD_LA_open(uint32_t version);
isp_ops_t *ISP_MOD_MCE_open(uint32_t version);
isp_ops_t *ISP_MOD_SCE_open(uint32_t version);
isp_ops_t *ISP_MOD_BPC_open(uint32_t version);
isp_ops_t *ISP_MOD_CLF_open(uint32_t version);
isp_ops_t *ISP_MOD_WB_open(uint32_t version);
isp_ops_t *ISP_MOD_GAMMA_open(uint32_t version);
isp_ops_t *ISP_MOD_FOV_open(uint32_t version);
isp_ops_t *ISP_MOD_SCALER_open(uint32_t version);
isp_ops_t *ISP_MOD_BCC_open(uint32_t version);
isp_ops_t *ISP_MOD_CLAMP_open(uint32_t version);
isp_ops_t *ISP_MOD_FRAME_SKIP_open(uint32_t version);
isp_ops_t *ISP_MOD_STATS_open(uint32_t version);
isp_ops_t *ISP_MOD_COLOR_XFORM_open(uint32_t version);

#endif /* __ISP_HW_MODULE_DEF_H__ */
