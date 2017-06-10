/*
  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIBMM_DISP_APIS
#define LIBMM_DISP_APIS

#include <stdlib.h>
#include <sys/types.h>
#include <cutils/log.h>
#include "common_log.h"

#define SUCCESS 0

/*
The following macros are to be used as bit mask for global adjustment settings
 - PA_GLOBAL_HUE For enabling global hue adjustment
 - PA_GLOBAL_SAT For enabling global saturation adjustment
 - PA_GLOBAL_VAL For enabling global value adjustment
 - PA_GLOBAL_CON For enabling global contrast adjustment
 - PA_GLOBAL_SAT_THRESH For setting the saturation threshold for global saturation
                        adjustment. Below this threshold no saturation adjument
                        would be applied
 - PA_GLOBAL_DESAT Setting this would remove all saturation globally. However
                   if a pixel is being adjusted in saturation by memory color
                   adjustment then that pixel would still retain that saturation
 - PA_GLOBAL_DISABLE For disabling all global adjustment.
*/
#define PA_GLOBAL_HUE         0x001
#define PA_GLOBAL_SAT         0x002
#define PA_GLOBAL_VAL         0x004
#define PA_GLOBAL_CON         0x008
#define PA_GLOBAL_SAT_THRESH  0x010
#define PA_GLOBAL_DESAT       0x020
#define PA_GLOBAL_DISABLE     0x040

/*
This enum includes all the APIs supported.
    DISP_COLOR_BALANCE_API -- set/get color balance
    DISP_COLOR_MODE_SEL_API -- set/get currectly active color mode,
                               get number of color modes on a given display ID,
                               get the list of all the color modes on a given display ID
    DISP_COLOR_MODE_MANAGEMENT_API -- create/update, save and delete color modes
    DISP_ADAPTIVE_BACKLIGHT_API -- start/stop/alter powersaving levels/read
                                   backlight scale for adaptive backlight feature
    DISP_GLOBAL_PICT_ADJ_API -- set/get the global hue, saturation, value and
                                contrast adjustment values
    DISP_MEM_COLOR_ADJ_API -- set/get the memory color adjustment values of SKIN,
                              SKY, FOLIAGE regions
    DISP_SUNLIGHT_VISIBILITY_API -- start/stop/alter the strength of the sunlight
                                    visibility feature
*/
typedef enum {
    DISP_COLOR_BALANCE_API,
    DISP_COLOR_MODE_SEL_API,
    DISP_COLOR_MODE_MANAGEMENT_API,
    DISP_ADAPTIVE_BACKLIGHT_API,
    DISP_GLOBAL_PICT_ADJ_API,
    DISP_MEM_COLOR_ADJ_API,
    DISP_SUNLIGHT_VISIBILITY_API,
} display_api_type;

typedef enum {
    DISP_API_INIT,
    DISP_API_DEINIT
} disp_api_init_type;

typedef enum {
    DISP_PRIMARY,
    DISP_EXTERNAL,
    DISP_WIFI
} disp_id_type;

typedef enum {
    DISP_MODE_OEM,
    DISP_MODE_USER,
    DISP_MODE_ALL
} disp_mode_type;

typedef enum {
    DISP_ADAPTIVE_BACKLIGHT,
    DISP_SUNLIGHT_VISIBILITY
} disp_active_feature_type;

typedef enum {
    DISP_FEATURE_ON,
    DISP_FEATURE_OFF
} disp_ctrl_req_type;

typedef enum {
    MEM_COL_SKIN,
    MEM_COL_SKY,
    MEM_COL_FOLIAGE
} disp_mem_col_type;

typedef enum {
   ADAPT_BL_QUAL_LOW,
   ADAPT_BL_QUAL_MEDIUM,
   ADAPT_BL_QUAL_HIGH,
   ADAPT_BL_QUAL_AUTO
} disp_adapt_bl_ql_level;

/*
struct disp_range -- Generic structure for storing range
*/
struct disp_range {
    int max;
    int min;
};

/*
struct disp_color_config -- structure for display color adjustment parameters
    flags -- flag to indicate which features are enabled;
        bit 0 -- Enable/Disable color balance adjustment; 0 - disable, 1 - enable
        bit 1~31 -- Reserved
    color_balance -- color balance adjustment, the larger the value, the warmer the color;
        range: -100 ~ 100 (%)
*/
struct disp_color_config {
    uint32_t flags;
    int color_balance;
};

/*
struct disp_mode -- structure for display color mode
  id -- integer mode ID, zero based
  name -- name of the mode, UTF-8 encoding
  name_len -- the length of the mode name
  type -- enum indicates whether this is an OEM or user defined color mode
*/
struct disp_mode {
    int id;
    char *name;
    uint32_t name_len;
    disp_mode_type type;
};

/*
struct disp_pa_config_data -- structure for storing global PA data
  hue        -- int value denoting degrees of hue
  saturation -- int value denoting percentage of saturation
  value      -- int value denoting percentage of value
  contrast   -- int value denoting percentage of contrast
  sat_thresh -- int value denoting percentage of saturation threshold
*/
struct disp_pa_config_data {
    int hue;
    int saturation;
    int value;
    int contrast;
    int sat_thresh;
};

/*
struct disp_pa_config -- structure for storing global PA config
  ops        -- indicates which of the 4 values has to be set and also if
                there needs to be exlusivity between global and mem color
                The following values can be 'OR'ed with ops for changing
                the corresponding setting
      PA_GLOBAL_HUE  For altering the global hue
      PA_GLOBAL_SAT  For altering the global saturation
      PA_GLOBAL_VAL  For altering the global value
      PA_GLOBAL_CON  For altering the global contrast
      PA_GLOBAL_SAT_THRESH  For setting a global saturation threshold below
                            which no adjustment happens
      PA_GLOBAL_DESAT  For applying complete desaturation. Any pixel with memory
                       color adjustment would still show effect of saturation
      PA_GLOBAL_DISABLE For disabling entire global adjustment applied
  data       -- structure to store the PA data
*/
struct disp_pa_config {
    int ops;
    struct disp_pa_config_data data;
};

/*
struct disp_pa_range -- structure for getting the ranges of global PA data
  max        -- structure to store the max values of global PA data
  min        -- structure to store the min values of global PA data
*/
struct disp_pa_range {
    struct disp_pa_config_data max;
    struct disp_pa_config_data min;
};

/*
struct disp_mem_color_config_data -- structure for storing memory color data
  hue         -- int value denoting degrees of hue
  saturation  -- int value denoting percentage of saturation
  value       -- int value denoting percentage of value
*/
struct disp_mem_color_config_data {
    int hue;
    int saturation;
    int value;
};

/*
struct disp_mem_color_config -- structure for storing memory color config
  enable      -- indicates whether the particular memory color is
                 - to be set or unset in HW while using set API
                 - is set or not in HW while using get API
  col         -- indicates whether memory color is SKIN or SKY or FOLIAGE
  data        -- structure to store the memory color data
*/
struct disp_mem_color_config {
    int enable;
    disp_mem_col_type col;
    struct disp_mem_color_config_data data;
};

/*
struct disp_pa_range -- structure for getting the ranges of global PA data
  col        -- indicates whether memory color is SKIN or SKY or FOLIAGE
  max        -- structure to store the max values of memory color data
  min        -- structure to store the min values of memory color data
*/
struct disp_mem_color_range {
    disp_mem_col_type col;
    struct disp_mem_color_config_data max;
    struct disp_mem_color_config_data min;
};

/*=========================================================================
FUNCTION
  disp_api_init()

DESCRIPTION
  This API initializes/de-initializes the display API library.
  This function must be called before calling any display APIs.
  init_type -- DISP_API_INIT or DISP_API_DEINIT

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_init(disp_api_init_type init_type);

/*=========================================================================
FUNCTION
  disp_api_supported()

DESCRIPTION
  This API checks if the input display API type is supported or not.
  disp_id -- display ID type: primary, external, or WIFI display
  api_type -- color balance adjustment, mode_selection, mode_management and etc.

RETURN VALUE
  TRUE - supported.
  FALSE - not supported
=========================================================================*/
int disp_api_supported(disp_id_type disp_id, display_api_type api_type);
struct API_SUPPORTED_IN {
    disp_id_type id;
    display_api_type type;
};

/*=========================================================================
FUNCTION
  disp_api_get_color_balance_range()

DESCRIPTION
  This API returns the color balance range
  disp_id -- display ID type: primary, external, or WIFI display
  range -- pointer to struct containing the color balance range

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_color_balance_range(disp_id_type disp_id, struct disp_range *range);

/*=========================================================================
FUNCTION
  disp_api_set_color_balance()

DESCRIPTION
  This API adjusts the color balance between cold and warm.
  disp_id -- display ID type: primary, external, or WIFI display
  warmness -- integer value for color balance, the larger the value, the warmer the color;

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_set_color_balance(disp_id_type disp_id, int warmness);
struct SET_CB_IN {
    disp_id_type id;
    int warmness;
};

/*=========================================================================
FUNCTION
  disp_api_get_color_balance()

DESCRIPTION
  This API returns the color balance applied on target.
  disp_id -- display ID type: primary, external, or WIFI display
  warmness -- integer value for color balance, the larger the value, the warmer the color;

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_color_balance(disp_id_type disp_id, int *warmness);

/*=========================================================================
FUNCTION
  disp_api_get_num_display_modes()

DESCRIPTION
  This API can return the number of display color modes available based on a given display ID.
  disp_id -- display ID type: primary, external, or WIFI display
  mode_type - system defined mode type or user defined mode type
  mode_cnt - number of display color modes

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_num_display_modes(disp_id_type disp_id, disp_mode_type mode_type, int *mode_cnt);
struct GET_NUM_MODE_IN {
    disp_id_type id;
    disp_mode_type type;
};

/*=========================================================================
FUNCTION
  disp_api_get_display_modes()

DESCRIPTION
  This API can return the list of display color modes available for a given display ID.
  disp_id -- display ID type: primary, external, or WIFI display
  mode_type - OEM defined mode type or user defined mode type
  modes - list of display color modes, including ID, name, and etc.
  mode_cnt - number of display color modes; this variable indicates
    that the caller of this API allocated memory for that many (mode_cnt)
    disp_modes structures.

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_display_modes(disp_id_type disp_id, disp_mode_type mode_type, struct disp_mode *modes, int mode_cnt);
struct GET_DISPLAY_MODES_IN {
    disp_id_type id;
    disp_mode_type type;
    int cnt;
};

/*=========================================================================
FUNCTION
  disp_api_get_active_display_mode()

DESCRIPTION
  This API returns the ID of the currently active display color mode on different
    display ID based on input.
  disp_id -- display ID type: primary, external, or WIFI display
  mode_id -- the ID of the currently active mode
  mask -- bit mask that indicates which features are modified on top of the last applied mode
        bit 0 -- Enable/Disable color balance adjustment; 0 - not modified, 1 - modified
        bit 1~31 -- Reserved

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_active_display_mode(disp_id_type disp_id, int *mode_id, uint32_t *mask);
struct GET_ACTIVE_MODE_OUT {
    int id;
    int mask;
};

/*=========================================================================
FUNCTION
  disp_api_set_active_display_mode()

DESCRIPTION
  This API applies a display color mode to target according to the
  input mode ID.
  disp_id -- display ID type: primary, external, or WIFI display
  mode_id -- display color mode ID, if this ID input is not given, reset
                to default display color mode.

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_set_active_display_mode(disp_id_type disp_id, int mode_id);
struct SET_MODE_PROP_IN {
    disp_id_type id;
    int mode_id;
};

/*=========================================================================
FUNCTION
  disp_api_save_display_mode()

DESCRIPTION
  This API saves the current display color balance setting as a display mode.
  disp_id -- display ID type: primary, external, or WIFI display
  config -- data structure for all the display color features supported
  mode_name -- the name of the saved mode
  mode_name_len -- the length of the mode name
  mode_id -- the id of the saved mode;
        if the caller of this API passes a negative ID, then this API will assign
        an positive integer as the ID of the saved mode, and return to caller;
        if the caller of this API passes a positive ID or zero, then this API will
        check whether this ID already exists or not. If it is, then update this
        mode; otherwise, create a new mode with this ID.

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_save_display_mode(disp_id_type disp_id,
                struct disp_color_config *config, char *mode_name,
                                       uint32_t mode_name_len, int *mode_id);

/*=========================================================================
FUNCTION
  disp_api_save_display_mode_v2()

DESCRIPTION
  This API saves the current active settings as a display mode.
  disp_id -- display ID type: primary, external, or WIFI display
  mode_name -- the name of the saved mode
  mode_name_len -- the length of the mode name
  mode_id -- the id of the saved mode;
        if the caller of this API passes a negative ID, then this API will assign
        an positive integer as the ID of the saved mode, and return to caller;
        if the caller of this API passes a positive ID or zero, then this API will
        check whether this ID already exists or not. If it is, then update this
        mode; otherwise, create a new mode with this ID.

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_save_display_mode_v2(disp_id_type disp_id, char *mode_name, uint32_t mode_name_len, int *mode_id);
struct SAVE_DISPLAY_MODE_V2_IN {
    disp_id_type id;
    const char *name;
    uint32_t name_len;
    int mode_id;
};

/*=========================================================================
FUNCTION
  disp_api_delete_display_mode()

DESCRIPTION
  This API deletes a display mode based on input mode ID. Note this API
  cannot delete OEM modes and the currently active mode.
  mode_id -- display color mode ID

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_delete_display_mode(int mode_id);

/*=========================================================================
FUNCTION
  disp_api_set_default_display_mode()

DESCRIPTION
  This API sets a display mode as the default mode for a given display type based on input mode ID.
  disp_id -- display ID type: primary, external, or WIFI display
  mode_id -- display color mode ID

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_set_default_display_mode(disp_id_type disp_id, int mode_id);

/*=========================================================================
FUNCTION
  disp_api_get_default_display_mode()

DESCRIPTION
  This API returns the default display mode ID on a given display type.
  disp_id -- display ID type: primary, external, or WIFI display
  mode_id -- display color mode ID

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_default_display_mode(disp_id_type disp_id, int *mode_id);

/*=========================================================================
FUNCTION
  disp_api_is_active_feature_on()

DESCRIPTION
  This API checks if a particular active display feature is running or not.
  disp_id -- display ID type: primary, external, or WIFI display
  feature_id -- display active feature, either adaptive backlight or sunlight
  visibility

RETURN VALUE
  1 - running.
  0 - not running
  NEGATIVE - API failed
=========================================================================*/
int disp_api_is_active_feature_on(disp_id_type disp_id, disp_active_feature_type feature_id);
struct IS_ACTIVE_FEATURE_ON_IN {
    disp_id_type id;
    disp_active_feature_type feature_id;
};

/*=========================================================================
FUNCTION
  disp_api_active_feature_control()

DESCRIPTION
  This API starts and stops display active features.
  disp_id -- display ID type: primary, external, or WIFI display
  feature_id -- display active feature, either adaptive backlight or sunlight
  visibility
  req -- request to turn the feature ON or OFF

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_active_feature_control(disp_id_type disp_id, disp_active_feature_type feature_id, disp_ctrl_req_type req);

/*=========================================================================
FUNCTION
  disp_api_get_adapt_bl_ql_level()

DESCRIPTION
  This API gets the quality level of adaptive backlight. This will fail if
  adaptive backlight feature is not running.
  disp_id -- display ID type: primary, external, or WIFI display
  level -- pointer to return the quality level of adaptive backlight

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_adapt_bl_ql_level(disp_id_type disp_id, disp_adapt_bl_ql_level *level);

/*=========================================================================
FUNCTION
  disp_api_set_adapt_bl_ql_level()

DESCRIPTION
  This API sets the quality level of adaptive backlight. This will fail if
  adaptive backlight feature is not running.
  disp_id -- display ID type: primary, external, or WIFI display
  level -- value to store as the quality level of adaptive backlight

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_set_adapt_bl_ql_level(disp_id_type disp_id, disp_adapt_bl_ql_level level);

/*=========================================================================
FUNCTION
  disp_api_get_adapt_bl_scale()

DESCRIPTION
  This API gets the BL scale as percentage. This function will fail if adaptive
  backlight is not running.
  disp_id -- display ID type: primary, external, or WIFI display
  scale -- pointer to return the percentage of backlight being applied.

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_adapt_bl_scale(disp_id_type disp_id, int *scale);

/*=========================================================================
FUNCTION
  disp_api_get_sunlight_visibility_strength_range()

DESCRIPTION
  This API gets the range of strength of sunlight visibility.
  disp_id -- display ID type: primary, external, or WIFI display
  range -- pointer to structure to store range of sunlight visibility

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_sunlight_visibility_strength_range(disp_id_type disp_id, struct disp_range *range);

/*=========================================================================
FUNCTION
  disp_api_get_sunlight_visibility_strength()

DESCRIPTION
  This API gets the current strength of value set for sunlight visibility. This
  API would fail if the sunlight visibility feature is not running.
  disp_id -- display ID type: primary, external, or WIFI display
  level -- pointer to return as the strength of sunlight visibility

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_sunlight_visibility_strength(disp_id_type disp_id, int *level);

/*=========================================================================
FUNCTION
  disp_api_set_sunlight_visibility_strength()

DESCRIPTION
  This API sets the sunlight visibility strength. This API would fail if the
  sunlight visibility feature is not running.
  disp_id -- display ID type: primary, external, or WIFI display
  level -- value to be set as the strength of sunlight visibility

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_set_sunlight_visibility_strength(disp_id_type disp_id, int level);

/*=========================================================================
FUNCTION
  disp_api_get_pa_range()

DESCRIPTION
  This API gets the global picture adjustment ranges for hue, saturation, value
  and contrast.
  disp_id -- display ID type: primary, external or WIFI display
  range -- pointer to the struct to return the ranges of global cfg

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_pa_range(disp_id_type disp_id, struct disp_pa_range *range);

/*=========================================================================
FUNCTION
  disp_api_get_pa_config()

DESCRIPTION
  This API gets the current set values for global adjustment. The returned
  ops value needs to be checked for checking if DESAT was set or if the
  adjustment was disabled.
  disp_id -- display ID type: primary, external or WIFI display
  cfg -- pointer to the struct to return the current global cfg

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_pa_config(disp_id_type disp_id, struct disp_pa_config *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_pa_config()

DESCRIPTION
  This API sets the values for global adjustment based on what is enabled
  through the bitmask in ops.
  disp_id -- display ID type: primary, external or WIFI display
  cfg -- pointer to structure to program the global adjustment values

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_set_pa_config(disp_id_type disp_id, struct disp_pa_config *cfg);
struct SET_PA_CONFIG_IN {
    disp_id_type id;
    struct disp_pa_config pa;
};

/*=========================================================================
FUNCTION
  disp_api_get_memory_color_range()

DESCRIPTION
  This API gets the range for memory color adjustment for the specific type
  asked in "col" member variable of "range".
  disp_id -- display ID type: primary, external or WIFI display
  range -- pointer to struct to return the current memory color range

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_memory_color_range(disp_id_type disp_id, struct disp_mem_color_range *range);

/*=========================================================================
FUNCTION
  disp_api_get_memory_color_config()

DESCRIPTION
  This API gets the current set values for memory color adjustment for the
  specific type asked in cfg. If the queried memory color is not enabled
  then the "enable" parameter would be set to 0, else it would be set to 1.
  disp_id -- display ID type: primary, external or WIFI display
  cfg -- pointer to struct to return the current memory color cfg in hardware

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_get_memory_color_config(disp_id_type disp_id, struct disp_mem_color_config *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_memory_color_config()

DESCRIPTION
  This API sets the values for memory color adjustment for the required set
  of registers based on id. For enabling the memory color the "enable" parameter
  needs to be set to 1. If "enable" is not set to 1 then the memory color
  adjustment for the queried type would be disabled.
  disp_id -- display ID type: primary, external or WIFI display
  cfg -- pointer to structure to program the memory color adjustment values for
         required component in hardware

RETURN VALUE
  ZERO - success.
  NON-ZERO - fail
=========================================================================*/
int disp_api_set_memory_color_config(disp_id_type disp_id, struct disp_mem_color_config *cfg);
struct SET_MEMCOLOR_CONFIG_IN {
    disp_id_type id;
    struct disp_mem_color_config cfg;
};

#endif //LIBMM_DISP_APIS
#ifdef __cplusplus
}
#endif
