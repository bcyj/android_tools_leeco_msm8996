/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "jpege_lib_common.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef struct
{
  uint32_t buffer_size;  /*  in Bytes */
  uint32_t quality;
  uint32_t image_width;
  uint32_t image_height;
  uint32_t h_sampling;
  uint32_t v_sampling;

} jpege_app_img_src;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
int jpege_app_calc_param (jpege_cmd_jpeg_encode_cfg * p_cfg, jpege_app_img_src src);
