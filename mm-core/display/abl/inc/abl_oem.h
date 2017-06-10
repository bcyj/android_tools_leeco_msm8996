/*=============================================================================

  A D A P T I V E   B A C K L I G H T   C O N T R O L   H E A D E R   F I L E

  DESCRIPTION
  This file contains interface specifications of the adaptive backlight control
  feature for display enhancements on QUALCOMM MSM display drivers. It reduces
  display power consumption by applying only the minimum required backlight for
  any given full video frame. The change in backlight levels is coupled with
  correponding gamma correction to compensate for changing brightness levels.

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/
#ifndef __ABL_OEM_H
#define __ABL_OEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <fcntl.h>
#include "aba_cabl.h"
#include "aba_svi.h"

#define CABL_LVL_LOW "Low"
#define CABL_LVL_MEDIUM "Medium"
#define CABL_LVL_HIGH "High"
#define CABL_LVL_AUTO "Auto"

#define MAX_BACKLIGHT_LEN 12
#define SYS_MAX_BRIGHTNESS  "/sys/class/leds/lcd-backlight/max_brightness"
    struct hw_info_hist {
        uint32_t hist_block;
        uint32_t hist_bins;
        uint32_t hist_components;
        uint32_t halfBin;
        uint32_t factor;
    };

    struct cabl_per_ql_params {
        uint32_t bl_min_ratio;
        uint32_t bl_max_ratio;
        uint32_t bl_filter_stepsize; // refer to the thresholds of temporal filtering for BL and LUT
        uint32_t pixel_distortion;   // allowed percentage of saturated pixel

        // Reserved parameters for CABL process
        double   reserved_param_SS;  // SS
        int32_t  reserved_param_LT;  // LT
        uint32_t reserved_param_WST;  // WST
        uint32_t reserved_param_FCT;  // FCT
        uint32_t reserved_param_BDF;  // BDF
        uint32_t reserved_param_BSTHC;  // BSTHC
        uint32_t reserved_param_SCT;  // SCT
        uint32_t reserved_param_SCD;  // SCD
    };

    typedef struct
    {
        int32_t bl_debug;               /*ABL level, read from UI */
        uint32_t SetLevel;              /*ABL level, read from UI */
        uint32_t orig_level;
        // bl_level_len[0] corresponds to be the total number of sample points of gamma response curve
        // bl_level_len[1] corresponds to be the total number of sample points of backlight brightness curve
        uint32_t bl_level_len[2];

        // minimum backlight level to be used regardless of content when ABL is enabled
        // range: 0-1024
        uint32_t bl_level_threshold;

        // minimum backlight level converted from bl_level_threshold
        // range: 0-255
        uint32_t bl_min_level;
        uint32_t bl_max_level;

        //Quality level specific params
        struct cabl_per_ql_params cabl_quality_params[ABL_QUALITY_MAX];

        // Y_gray and gray_shade LUTs
        uint32_t *pY_gamma;
        uint32_t *pY_shade;

        // Y_gray_BL, BL_lvl LUTs
        uint32_t *pY_lvl;
        uint32_t *pbl_lvl;

        //CABL quality level mode
        uint32_t default_ql_mode;
        //default UI and video quality levels
        int32_t ui_quality_lvl;
        int32_t video_quality_lvl;

    } bl_oem_api;

void set_interpolated_bl_min_level(bl_oem_api *api_para);

void pp_oem_message_handler(const char *cmd, const int32_t len, const int32_t fd);

static uint32_t interpolate(uint32_t len, uint32_t x[], uint32_t y[], uint32_t target);

#ifdef __cplusplus
}
#endif

#endif /* ABL_OEM_H */
