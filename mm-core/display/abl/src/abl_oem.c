
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
/*============================================================================
  INCLUDE FILES FOR MODULE
  ============================================================================*/
#include "abl_oem.h"

void set_interpolated_bl_min_level(bl_oem_api *api_para) {
    if (api_para->bl_min_level == 0) {
        api_para->bl_min_level = interpolate(api_para->bl_level_len[1],
        api_para->pY_lvl, api_para->pbl_lvl, api_para->bl_level_threshold) *
        api_para->bl_max_level / 1024;
    }
}

/*=============================================================================
  FUNCTION interpolate()  - Interpolate function
  =============================================================================*/
static uint32_t interpolate(uint32_t len, uint32_t x[], uint32_t y[], uint32_t target)
{
    uint32_t result = 0;
    uint32_t i, j, ui_m, ui_c;
    double dm, dc;
    if ((x == NULL) || (y == NULL))
        return result;
    if( !x || !y)
        DISP_OSAL_LOG3("%s: begining interpolation: X = %u Y = %u", __func__, x, y);

    if (target >= x[len-1]) {
        result = y[len-1];
    } else if (target == x[0]) {
        result = y[0];
    } else {
        for(i = 1; target > x[i]; i++);

        j = i - 1;
        if (target == x[j]) {
            result  = y[j];
        } else {
            ui_m = ((y[i] - y[j])<<14) / (x[i] - x[j]);
            ui_c = y[i] - ((ui_m * x[i] + 8192)>>14);
            result = ((ui_m * target + 8192)>>14)+ ui_c;
        }
    }
    return result;
}

void pp_oem_message_handler(const char *cmd, const int32_t len, const int32_t fd) {
    /* =============================================
     *    OEMs can put the handler code here
     * =============================================*/
}

