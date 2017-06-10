/*=============================================================================

  A D A P T I V E   B A C K L I G H T   C O N T R O L   H E A D E R   F I L E

DESCRIPTION:
  This file contains interface specifications of the adaptive backlight control
  feature for display enhancements of QUALCOMM MSM display drivers. It reduces
  display power consumption by applying only the minimum required backlight for
  any given full video frame. The change in backlight levels is coupled with
  corresponding gamma correction to compensate for changing brightness levels.

  Copyright (c) 2011-2012, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
#ifndef DISP_ABL_DRIVER_H
#define DISP_ABL_DRIVER_H


#ifdef __cplusplus
extern "C" {
#endif

#include "abl_oem.h"
#include "abl_core_api.h"
#include "lib-postproc.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/msm_mdp.h>
#include <linux/fb.h>

/* function declaration */
/*===========================================================================*/
int32_t abl_init(int32_t fb, struct fb_cmap *cmap, bl_oem_api *api_para,
                    struct hw_info_hist *hw_para, int32_t init_data_quality,
                                                    int32_t init_data_level);
int32_t abl_calc_lut(int32_t *bl_lvl, int32_t fb, struct fb_cmap *cmap);
void abl_revert_calc();
void abl_update_history();
void abl_reset_history();
int32_t abl_set_lut(int32_t fb, struct fb_cmap *cmap);
void abl_exit(int32_t fb, struct fb_cmap *cmap);
void rest_map(int32_t fb, struct fb_cmap *cmap);
void free_cmap(struct fb_cmap *cmap);
int32_t abl_change_orig_bl_level(bl_oem_api *api_para);
void abl_change_PG(bl_oem_api *api_para, int32_t i);
int32_t abl_change_quality_level(bl_oem_api *api_para, uint32_t level);
int32_t cabl_get_version(char *version);
/*===========================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* ABL_driver_H */
