/*============================================================================
   Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include <unistd.h>
#include <math.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdint.h>
#include "dmlrocorrection.h"
#include "tintless_interface.h"
#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_tintless_interface.h"
#include "isp_log.h"

static tintless_rolloff_ops_t tintless_ops;
void tintless_interface_open(tintless_rolloff_ops_t *ops);
void tintless_bg_pca_interface_open(tintless_rolloff_ops_t *ops);

void isp_tintless_open_version(uint32_t version)
{

  CDBG_HIGH("isp_tintless_open_version: E: %d\n",
    GET_ISP_MAIN_VERSION(version));
  switch(GET_ISP_MAIN_VERSION(version)) {
  case ISP_VERSION_40: {
    return tintless_interface_open(&tintless_ops);
  }

  case ISP_VERSION_32: {
    return tintless_bg_pca_interface_open(&tintless_ops);
  }
 }
  CDBG_HIGH("isp_tintless_open_version: X: %d\n",
    GET_ISP_MAIN_VERSION(version));
}

tintless_return_t isp_tintless_open(void ** const res, uint32_t * updates_needed)
{
  CDBG_HIGH("isp_tintless_open: E: %x\n",
    (unsigned int)tintless_ops.isp_tintless_open);
    if (tintless_ops.isp_tintless_open)
      return tintless_ops.isp_tintless_open(res, updates_needed);

    return TINTLESS_SUCCESS;
} /* isp_tintless_open */

/*===========================================================================
FUNCTION      isp_tintless_get_version

DESCRIPTION    Checks the version of the tintless wrapper. As a side effect,
               the wrapper will check the version of the algorithm. If there
               is a mismatch, it will show up in the return code.
===========================================================================*/
tintless_return_t isp_tintless_get_version(void * const res, tintless_version_t * version)
{
    return tintless_ops.isp_tintless_get_version(res, version);
} /* isp_tintless_get_version */

/*===========================================================================
FUNCTION      isp_tintless_stat_config

DESCRIPTION   Update the stat params for the tintless algo.
              Should be called after the BG stat config has
              been called.
===========================================================================*/
tintless_return_t isp_tintless_stat_config(void * const res, tintless_stats_config_t * cfg)
{
    return tintless_ops.isp_tintless_stat_config(res, cfg);
} /* isp_tintless_stat_config */

/*===========================================================================
FUNCTION      isp_tintless_mesh_config

DESCRIPTION   Update the stat params for the tintless algo.
              Should be called after the BG stat config has
              been called.
===========================================================================*/
tintless_return_t isp_tintless_mesh_config(void * const res, tintless_mesh_config_t * cfg)
{
    return tintless_ops.isp_tintless_mesh_config(res, cfg);
} /* isp_tintless_mesh_config */

/*===========================================================================
FUNCTION      isp_tintless_update_chromatix_params

DESCRIPTION   Update the tintless related chromatix tuning parameters.
===========================================================================*/
tintless_return_t isp_tintless_update_chromatix_params(
   void * const res, chromatix_color_tint_correction_type * p)
{
    return tintless_ops.isp_tintless_update_chromatix_params(res, p);
} /* isp_tintless_update_chromatix_params */

/*===========================================================================
FUNCTION      isp_tintless_algo

DESCRIPTION   calling the main algorithm for tintless processing
===========================================================================*/
tintless_return_t isp_tintless_algo(void * const res,
                                    tintless_stats_t * be_stats,
                                    tintless_mesh_rolloff_array_t * ptable_3a,
                                    tintless_mesh_rolloff_array_t * ptable_cur,
                                    tintless_mesh_rolloff_array_t * const ptable_correction)
{
  return tintless_ops.isp_tintless_algo(res, be_stats, ptable_3a, ptable_cur, ptable_correction);
} /* isp_tintless_algo */

/*===========================================================================
FUNCTION      isp_tintless_close

DESCRIPTION   close the library and free all allocated memory
===========================================================================*/
tintless_return_t isp_tintless_close(void ** const res)
{
    return tintless_ops.isp_tintless_close(res);
} /* isp_tintless_close */
