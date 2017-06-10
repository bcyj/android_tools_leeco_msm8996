/*============================================================================
   Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "tintless_interface.h"

typedef struct {
  tintless_return_t (*isp_tintless_open)(void ** const res, uint32_t * updates_needed);
  tintless_return_t (*isp_tintless_stat_config)(void * const res, tintless_stats_config_t *cfg);
  tintless_return_t (*isp_tintless_mesh_config)(void * const res, tintless_mesh_config_t *cfg);
  tintless_return_t (*isp_tintless_update_chromatix_params)(void * const res, chromatix_color_tint_correction_type *p);
  tintless_return_t (*isp_tintless_get_version)(void * const res, tintless_version_t *version);
  tintless_return_t (*isp_tintless_algo)(void * const res,
                                    tintless_stats_t * be_stats,
                                    tintless_mesh_rolloff_array_t *ptable_3a,
                                    tintless_mesh_rolloff_array_t *ptable_cur,
                                    tintless_mesh_rolloff_array_t *ptable_correction);
  tintless_return_t (*isp_tintless_close)(void ** const res);
} tintless_rolloff_ops_t;
