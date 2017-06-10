/*============================================================================

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CHROMATIX_VFE_COMMON_H
#define CHROMATIX_VFE_COMMON_H
#include "chromatix.h"
/*============================================================================
                        CONSTANTS
============================================================================*/

#define CHROMATIX_VFE_COMMON_VERSION 0x301 //does this need to be same as other header?

/******************************************************************************
    VFE basic struct
    ******************************************************************************/

/******************************************************************************
Linearization data types
******************************************************************************/

typedef struct
{
    unsigned short r_lut_p[8]; // 12uQ0
    unsigned short r_lut_base[9]; // 12uQ0
    float r_lut_delta[9]; // 18uQ9
    // GR channel knee points & LUT (2 banks)
    unsigned short gr_lut_p[8]; // 12uQ0
    unsigned short gr_lut_base[9]; // 12uQ0
    float gr_lut_delta[9]; // 18uQ9
    // GB channel knee points & LUT (2 banks)
    unsigned short gb_lut_p[8]; // 12uQ0
    unsigned short gb_lut_base[9]; // 12uQ0
    float gb_lut_delta[9]; // 18uQ9
    // B channel knee points & LUT (2 banks)
    unsigned short b_lut_p[8]; // 12uQ0
    unsigned short b_lut_base[9]; // 12uQ0
    float b_lut_delta[9]; // 18uQ9
} chromatix_linearization_type;

/******************************************************************************
roll-off data types
******************************************************************************/
typedef enum
{
    ROLLOFF_TL84_LIGHT, /* Flourescent */
    ROLLOFF_A_LIGHT,    /* Incandescent */
    ROLLOFF_D65_LIGHT,  /* Day Light */
 //   ROLLOFF_LOW_LIGHT,  /* Low Light */ removed for 0x300
 //   ROLLOFF_PREVIEW,    //204, added for preview, removed for 0x300
    ROLLOFF_MAX_LIGHT,
    ROLLOFF_INVALID_LIGHT = ROLLOFF_MAX_LIGHT
} chromatix_rolloff_light_type;

#define MESH_ROLLOFF_SIZE    (17 * 13)

typedef struct
{
    unsigned short                 mesh_rolloff_table_size;     // TableSize

    float                  r_gain[MESH_ROLLOFF_SIZE];   // RGain

    float                  gr_gain[MESH_ROLLOFF_SIZE];  // GRGain

    float                  gb_gain[MESH_ROLLOFF_SIZE];  // GBGain

    float                  b_gain[MESH_ROLLOFF_SIZE];   // BGain
} mesh_rolloff_array_type;

typedef struct
{
    tuning_control_type control_linearization;
    // trigger_point_type linearization_bright_trigger;
    trigger_point_type linearization_lowlight_trigger; //changed to lowlight trigger in 0x300

    chromatix_CCT_trigger_type linear_A_trigger;
    chromatix_CCT_trigger_type linear_D65_trigger;

    chromatix_linearization_type linear_table_A_lowlight;//changed to lowlightin 0x300
    chromatix_linearization_type linear_table_A_normal;
    chromatix_linearization_type linear_table_TL84_lowlight;//changed to lowlightin 0x300
    chromatix_linearization_type linear_table_TL84_normal;
    chromatix_linearization_type linear_table_Day_lowlight;//changed to lowlightin 0x300
    chromatix_linearization_type linear_table_Day_normal;
} chromatix_L_type;

typedef struct
{
    tuning_control_type         control_rolloff;//default 0 for lux_index
    trigger_point_type      rolloff_lowlight_trigger; //0x208
    chromatix_CCT_trigger_type rolloff_A_trigger;
    chromatix_CCT_trigger_type rolloff_D65_trigger;

    // 0x206, use led influence params from AEC, sensitivity_low, senstivity_high, sensitivity_off
    float rolloff_LED_start;
    float rolloff_LED_end ;
    float rolloff_Strobe_start;
    float rolloff_Strobe_end;

    mesh_rolloff_array_type                chromatix_mesh_rolloff_table[ROLLOFF_MAX_LIGHT]; // Mesh
    mesh_rolloff_array_type                chromatix_mesh_rolloff_table_lowlight[ROLLOFF_MAX_LIGHT]; // NEW
    mesh_rolloff_array_type                chromatix_mesh_rolloff_table_golden_module[ROLLOFF_MAX_LIGHT]; // NEW
    mesh_rolloff_array_type                chromatix_mesh_rolloff_table_LED;
    mesh_rolloff_array_type                chromatix_mesh_rolloff_table_Strobe;
} chromatix_rolloff_type;

typedef struct
{

    float LA_LUT_backlit[64];
    float LA_LUT_solarize[64];
    float LA_LUT_posterize[64];
    float LA_LUT_blackboard[64];
    float LA_LUT_whiteboard[64];
} chromatix_LA_special_effects_type;

/******************************************************************************
******************************************************************************
******************************************************************************
******************************************************************************
******************************************************************************
******************************************************************************
    CHROMATIX COMMON VFE HEADER definition
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/

typedef struct
{
//common structs

chromatix_version_type                 chromatix_version;
    unsigned char is_compressed;

unsigned short                         revision_number;

/******************************************************************************
    Linearization
    changed from brightlight to lowlight in 0x300
    ******************************************************************************/
    chromatix_L_type chromatix_L;

    /******************************************************************************
    Lens Rolloff   (Rolloff)
    removed radial based supports
    added lowlight rolloff for different illuminants
    ******************************************************************************/
    chromatix_rolloff_type chromatix_rolloff;

    /******************************************************************************
    Luma adaptation
    These are parameters for special effect, manual BSM
    ******************************************************************************/
    chromatix_LA_special_effects_type chromatix_LA_special_effects;
} chromatix_VFE_common_type;

#endif
