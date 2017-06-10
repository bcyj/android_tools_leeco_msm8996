/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __STATS_MEM_AND_PARSER_H__
#define __STATS_MEM_AND_PARSER_H__

#include "vfe_util_common.h"

#define VFE2X_AWB_AEC_BUF_SIZE    (8*16*16 + 8)
#define VFE2X_AF_BUF_SIZE         (4 + 1056)

#define VFE32_AEC_BUF_SIZE        128
#define VFE32_AF_BUF_SIZE         162
#define VFE32_AWB_BUF_SIZE        1040
#define VFE32_IHIST_BUF_SIZE      128
#define VFE32_CS_BUF_SIZE         1344
#define VFE32_RS_BUF_SIZE         1024

#define VFE31_AWB_BUF_SIZE        528

#define VFE_BAYER_GRID_BUF_SIZE   (72 * 54 * 6)
#define VFE_BAYER_EXPOSURE_BUF_SIZE   (32 * 24 * 6)
#define VFE_BAYER_FOCUS_BUF_SIZE  (18 * 14 * 10)
#define VFE_BAYER_HIST_BUF_SIZE   (256 * 4)

#define STATS_AEAW      (1<<0)
#define STATS_AEC       (1<<1)
#define STATS_AF        (1<<2)
#define STATS_AWB       (1<<3)
#define STATS_IHIST     (1<<4)
#define STATS_RS        (1<<5)
#define STATS_CS        (1<<6)
#define STATS_BG        (1<<7)
#define STATS_BF        (1<<8)
#define STATS_BHIST     (1<<9)
#define STATS_BE        (1<<10)

#define VFE_STATS_ALL   0xFFFF

typedef struct {
  uint32_t buf_size;
  uint8_t  *buf;
  int32_t  fd;
} stats_buffer_t;

vfe_status_t vfe_stats_buffer_init(void *ctrl);
vfe_status_t vfe_stats_buffer_free(void *ctrl);
vfe_status_t vfe_stats_buffer_release(int fd, int buf_type, void *parm);
vfe_status_t vfe_af_calc_roi(void *p_obj,
  vfe_stats_af_params_t *ip_af_params);
int vfe_stats_proc_MSG_ID_STATS_AE(void *p_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_AWB(void *p_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_IHIST(void *p_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_RS(void *p_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_CS(void *p_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_AF(void *p_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_COMPOSITE(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_WB_EXP(void *vfe_ctrl_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_BG(void *p_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_BF(void *p_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_BE(void *p_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);
int vfe_stats_proc_MSG_ID_STATS_BHIST(void *p_obj,
  int isp_started, stats_type_t type, void *stats,
  vfe_stats_output_t *stats_output);

#endif /* __STATS_MEM_AND_PARSER_H__ */
