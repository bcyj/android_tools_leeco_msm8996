#ifndef CHROMATIX_H
#define CHROMATIX_H
/*============================================================================
C H R O M A T I X    D E F I N I T I O N

DESCRIPTION
Shared structure definition file between the Chromatix
sensor tuning tool and the Camera/Camif DMSS code layer.

Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
Export of this technology or software is regulated by the U.S. Government.
Diversion contrary to U.S. law prohibited.
============================================================================*/

/*============================================================================

EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: $ $DateTime: $ $Author: $

============================================================================*/

/*============================================================================
NOTES
============================================================================*/

/*
* 1] Chromatix tool design policy is to make the tool very generic and
* independed of MSM target. It is mandatory that ARM does the applicable
* Q factor conversions on a specific MSM target.
*/

/*============================================================================
*                         CONSTANTS
============================================================================*/

#define CHROMATIX_DMSS7500_VERSION 0x0207

#define HJR_K_TABLE_SIZE 20

#define ASF_FILTER_SIZE 3

#define SMOOTHING_FILTER_SIZE (ASF_FILTER_SIZE * ASF_FILTER_SIZE)
#define LAPLACIAN_FILTER_SIZE SMOOTHING_FILTER_SIZE
#define ADAPTIVE_BAYER_FILTER_TABLE_SIZE 20
#define MAX_EXPOSURE_TABLE_SIZE    500

/*============================================================================
DATA TYPES
============================================================================*/

typedef unsigned short chromatix_version_type;

typedef unsigned short chromatix_number_elements_type;

/// Tuning Control Flag. 0 is Lux Index based, 1 is Gain Based.
typedef unsigned char tuning_control_type;

/// This type holds both luma index and gain based trigger points to be
/// used in configuration of VFE blocks.
typedef struct
{
    float gain_start;      ///< Gain based trigger point.
    float gain_end;        ///< Gain based end point.
    long lux_index_start; ///< Lux index based trigger point.
    long lux_index_end;   ///< Lux index based end point.
} trigger_point_type;

/******************************************************************************
Bad pixel correction data types
******************************************************************************/
typedef enum
{
    BPC_NORMAL_LIGHT =  0,
    BPC_LOW_LIGHT,
    BPC_MAX_LIGHT
} bpc_light_type;

typedef struct
{
    unsigned short  bpc_diff_threshold_r;
    unsigned short  bpc_diff_threshold_g;
    unsigned short  bpc_diff_threshold_b;
} bpc_diff_threshold_type;

/******************************************************************************
exposure table data types
******************************************************************************/

// typedef unsigned short exposure_index_type;

typedef struct
{
    unsigned short even_columns;                // EvenCols
    unsigned short odd_columns;                 // OddCols
} chromatix_black_level_offset_type;


typedef struct
{
    unsigned short gain;                        // Gain
    unsigned long line_count;                  // LineCt
} exposure_entry_type;


typedef struct
{
    unsigned char exp_table_flag;               // ExpTableFlag, 205 (removed), keep it for SW backward compatibility , marked as unused.
    unsigned short valid_entries;               // ValidEntries
    exposure_entry_type exposure_entries[MAX_EXPOSURE_TABLE_SIZE]; // ExposureEntries
} chromatix_exposure_table_type;


/******************************************************************************
AWB data types
******************************************************************************/

typedef enum
{
    AGW_AWB_OUTDOOR_SUNLIGHT =  0,  // D65
    AGW_AWB_OUTDOOR_CLOUDY,         // D75
    AGW_AWB_INDOOR_INCANDESCENT,    // A
    AGW_AWB_INDOOR_WARM_FLO,        // TL84
    AGW_AWB_INDOOR_COLD_FLO,        // CW
    AGW_AWB_HORIZON,                // H
    AGW_AWB_OUTDOOR_SUNLIGHT1,      // D50
    AGW_AWB_INDOOR_CUSTOM_FLO,      // CustFlo
    AGW_AWB_OUTDOOR_NOON,           // Noon
    AGW_AWB_HYBRID,                 // Daylight
    AGW_AWB_MAX_LIGHT,
    AGW_AWB_INVALID_LIGHT = AGW_AWB_MAX_LIGHT
} chromatix_awb_light_type;


typedef struct
{
    /* converted to Q7 in the code */
    float green_even;                // GreenEvenRow
    float green_odd;                 // GreenOddRow
    float red;                       // Red
    float blue;                      // Blue
} chromatix_channel_balance_gains_type;

typedef enum
{
    AWB_STATS_LOW_LIGHT = 0,
    AWB_STATS_NORMAL_LIGHT,
    AWB_STATS_OUTDOOR,
    AWB_STATS_MAX_LIGHT
} chromatix_awb_stats_light_type;



typedef struct
{
    /* 8-bit, Q0, unsigned */
    unsigned char y_min;                      // LumaMin

    /* 8-bit, Q0, unsigned */
    unsigned char y_max;                      // LumaMax

    /* Slope of neutral region line 1 */
    /* 8-bit, Q4, signed */
    char m1;                          // Slope1

    /* Slope of neutral region line 2 */
    /* 8-bit, Q4, signed */
    char m2;                          // Slope2

    /* Slope of neutral region line 3 */
    /* 8-bit, Q4, signed */
    char m3;                          // Slope3

    /* Slope of neutral region line 4 */
    /* 8-bit, Q4, signed */
    char m4;                          // Slope4

    /* Cb intercept of neutral region line 1 */
    /* 12-bit, Q0, signed */
    short c1;                         // CbOffset1

    /* Cb intercept of neutral region line 2 */
    /* 12-bit, Q0, signed */
    short c2;                         // CrOffset2

    /* Cb intercept of neutral region line 3 */
    /* 12-bit, Q0, signed */
    short c3;                         // CbOffset3

    /* Cb intercept of neutral region line 4 */
    /* 12-bit, Q0, signed */
    short c4;                         // CrOffset4
} chromatix_wb_exp_stats_type;

/* AGW AWB weight vector */
typedef struct
{
    int indoor_weight;                // Indoor
    int outdoor_weight;               // Outdoor
    int inoutdoor_weight;             // InOut
} chromatix_awb_weight_vector_type;

/* AWB sample influence */
typedef struct
{
    float outdoor_influence;          // Outdoor
    float indoor_influence;           // Indoor
} chromatix_awb_sample_influence_type;

/* Auto white balance reference points */
typedef struct
{
    float red_gain;                   // RedG
    float blue_gain;                  // BlueG
    float red_gain_adj;               // RedAdj
    float blue_gain_adj;              // BlueAdj
} chromatix_awb_reference_type;

typedef struct
{
    //AWB
    int AWB_purple_prevent_enable; //0x207, enable the feature
    tuning_control_type     control_purple_prevent;//default 0 for lux_index
    trigger_point_type      purple_prevent_trigger;
    float purple_sky_prevention_bg_threshold ;//range from 0 to 2.0, default is the BG ratio of D50
    unsigned char AWB_Ymin_low_threshold;//   (range: 0- 100, int, default =60)
    unsigned char AWB_Ymin_high_threshold;// (range: 0-100, int, default = 98).
}AWB_purple_prevent_type;

//0x207
/************************************************************************
AWB motion sensor type
************************************************************************/
typedef struct{
    float awb_gyro_trigger ;// input above trigger to detect change
    float awb_accelerometer_trigger; // input above trigger to detect change
    float awb_magnetometer_trigger; // input above trigger to detect change
    float awb_DIS_motion_vector_trigger; // input above trigger to detect change
}AWB_motion_sensor_type;

//this struct is updated from 0x207 to 0x208
typedef struct{
    int awb2_bright_green_percentage;// at outdoor index
    int awb2_dark_green_percentage; // at indoor index
    int awb2_dark_r_threshold; // R stat ave below this value is rejected (8-bit domain)
    int awb2_dark_g_threshold; // G stat ave below this value is rejected (8-bit domain)
    int awb2_dark_b_threshold; // B stat ave below this value is rejected (8-bit domain)
    int awb2_white_stat_y_threshold_high;
    float awb2_ref_b_bg_tl84;
    float awb2_ref_r_rg_a;
    float awb2_extreme_range_offset_b;
    float awb2_extreme_range_offset_r;
    float awb2_threshold_extreme_b_percent;
    float awb2_threshold_extreme_r_percent;
    int awb2_threshold_compact_cluster;
    int awb2_compact_to_grey_dis;
    int awb2_threshold_compact_cluster_valid;
    int awb2_dominant_cluster_threshold;
    unsigned char awb2_dist_weight_table[64];
    int awb2_outdoor_adjustment;
    int awb2_exposure_adjustment;
    float awb2_outlier_valid_ymax;
    float awb2_cluster_high_pec;
    float awb2_cluster_mid_pec;
    float awb2_cluster_low_pec;
    int awb2_weight_vector[AGW_AWB_MAX_LIGHT+4][4];
}AWB2_parameters_type;

/*****************************************/
//0x207  trigger pt for CCT
/*****************************************/
typedef struct{
    unsigned long CCT_start;
    unsigned long CCT_end;
}chromatix_CCT_trigger_type;

/*****************************************/
//0x207  enums for lowlight AWB_gain_adj
/*****************************************/

typedef enum
{
    AWB_LOWLIGHT_A=0,
    AWB_LOWLIGHT_TL84,
    AWB_LOWLIGHT_D65,
    AWB_LOWLIGHT_LED,
    AWB_LOWLIGHT_STROBE,
    AWB_MAX_LOWLIGHT
}AWB_LOWLIGHT_type;

/***********************************************************************
AFD (8K) row sum / column sum stat collection
************************************************************************/
//205 mods
typedef struct
{
    int  row_sum_enable;   // 0=disable, 1=enable, default=1
    float   row_sum_hor_Loffset_ratio;  // default=0;
    float   row_sum_ver_Toffset_ratio;  //default=0;
    float   row_sum_hor_Roffset_ratio;  // default=0;
    float   row_sum_ver_Boffset_ratio;  //default=0;
    unsigned char   row_sum_V_subsample_ratio; //1 to 4, int


    int  col_sum_enable;   // 0=disable, 1=enable, default=0
    float   col_sum_hor_Loffset_ratio;  //default=0
    float   col_sum_ver_Toffset_ratio;  // default=0;
    float   col_sum_hor_Roffset_ratio;  //default=0
    float   col_sum_ver_Boffset_ratio;  // default=0;
    unsigned char   col_sum_H_subsample_ratio; //2 to 4, int


} chromatix_rscs_stat_config_type;



/******************************************************************************
roll-off data types
******************************************************************************/
typedef enum
{
    ROLLOFF_TL84_LIGHT, /* Flourescent */
    ROLLOFF_A_LIGHT,    /* Incandescent */
    ROLLOFF_D65_LIGHT,  /* Day Light */
    ROLLOFF_LOW_LIGHT,  /* Low Light */
    ROLLOFF_PREVIEW,    //204, added for preview
    ROLLOFF_MAX_LIGHT,
    ROLLOFF_INVALID_LIGHT = ROLLOFF_MAX_LIGHT
} chromatix_rolloff_light_type;

#define ROLLOFF_RADIUS_SQUARE_TABLE_SIZE 32
/* 32-bit, Q0, unsigned */
typedef unsigned long rolloff_radius_sqaure_table_type [ROLLOFF_RADIUS_SQUARE_TABLE_SIZE];

#define ROLLOFF_CORRECTION_FACTOR_SIZE  ROLLOFF_RADIUS_SQUARE_TABLE_SIZE
/* 13-bit, Q10, unsigned */
typedef double rolloff_correction_factor_table_type [ROLLOFF_CORRECTION_FACTOR_SIZE];

#define ROLLOFF_SLOPE_VALUE_SIZE  ROLLOFF_RADIUS_SQUARE_TABLE_SIZE
/* 19-bit, Q10, signed */
typedef double rolloff_slope_value_table_type [ROLLOFF_SLOPE_VALUE_SIZE];

typedef unsigned short rolloff_image_center_type;
typedef unsigned short rolloff_image_dimension_type;
typedef unsigned char  rolloff_intervals_type;


/* Rolloff Table */
typedef struct
{
    rolloff_image_center_type             cx;         // CenterX
    rolloff_image_center_type             cy;         // CenterY
    rolloff_image_dimension_type          width;      // Width
    rolloff_image_dimension_type          height;     // Height
    rolloff_intervals_type                N;          // Intervals
    rolloff_radius_sqaure_table_type      radius_square_table;  // RSqTable
    rolloff_correction_factor_table_type  red_cf_table;    // RedTable
    rolloff_correction_factor_table_type  green_cf_table;  // GreenTable
    rolloff_correction_factor_table_type  blue_cf_table;   // BlueTable
} chromatix_rolloff_table_type;


#define MESH_ROLLOFF_SIZE    (17 * 13)

typedef struct

{
    unsigned short                 mesh_rolloff_table_size;     // TableSize

    float                  r_gain[MESH_ROLLOFF_SIZE];   // RGain

    float                  gr_gain[MESH_ROLLOFF_SIZE];  // GRGain

    float                  gb_gain[MESH_ROLLOFF_SIZE];  // GBGain

    float                  b_gain[MESH_ROLLOFF_SIZE];   // BGain

} mesh_rolloff_array_type;


/******************************************************************************
3x3 ASF data types
******************************************************************************/

#define ASF_FILTER_SIZE 3
/* 8-bit, Q0-Q8, signed */
typedef float asf_filter_type [ASF_FILTER_SIZE][ASF_FILTER_SIZE];

/* range 0-8 */
typedef unsigned char asf_q_factor_type;

/* range 0-1 */
typedef unsigned char asf_edge_detection_flag_type;

/* 10-bit, Q0, unsigned */
typedef unsigned short asf_threshold_type;

typedef struct
{
    asf_filter_type              edge_filter;           // Edge
    asf_filter_type              noise_filter;          // Noise
    asf_threshold_type           noise_threshold;       // EdgeThresh
    asf_threshold_type           edge_threshold;        // NoiseThresh
    asf_q_factor_type            edge_filter_q_factor;  // EdgeQ
    asf_q_factor_type            noise_filter_q_factor; // NoiseQ
    asf_edge_detection_flag_type edge_detection_flag;   // EdgeFlag
} chromatix_adaptive_spatial_filter_type;


/******************************************************************************
5x5 ASF data types
******************************************************************************/
/* 5 x 5 Adaptive Spatial Filter.
* There are two components in this filter
* High Pass Filter (HPF): 5 x 5
* Low Pass Filter (LPF): 3 x 3
* HPF can use upto two 5 x 5 filters. The sensor driver chooses to use
* both or single filter.
* LPF can be implemented in H/W Luma Filter module or in IPL.
* LPF is a function of Smoothing and Laplacian filters of size 3 x 3.
*/
typedef struct
{
    short  a11;
    short  a12;
    short  a13;
    short  a14;
    short  a15;

    short  a21;
    short  a22;
    short  a23;
    short  a24;
    short  a25;

    short  a31;
    short  a32;
    short  a33;
    short  a34;
    short  a35;

    short  a41;
    short  a42;
    short  a43;
    short  a44;
    short  a45;

    short  a51;
    short  a52;
    short  a53;
    short  a54;
    short  a55;
} matrix_5_5_type;

typedef enum
{
    ASF_LOW_LIGHT = 0,
    ASF_NORMAL_LIGHT,
    ASF_BRIGHT_LIGHT,
    ASF_MAX_LIGHT,
    ASF_INVALID_LIGHT = ASF_MAX_LIGHT
} chromatix_asf_brightness_level_type;

typedef enum
{
    ASF_MODE_DISABLE = 0,
    ASF_MODE_SINGLE,
    ASF_MODE_DUAL,
    ASF_MODE_SMART,
    ASF_MODE_MAX,
    ASF_MODE_INVALID = ASF_MODE_MAX
} chromatix_asf_mode_type;

typedef char   smoothing_filter_type [SMOOTHING_FILTER_SIZE];
typedef char   laplacian_filter_type [LAPLACIAN_FILTER_SIZE];
typedef float  normalize_factor_type;
typedef char   filter_threshold_type;
typedef float  filter_sharpen_degree_type;
typedef unsigned char  filter_smoothing_degree_type;

typedef struct
{
    /* noise level to filter out */
    filter_threshold_type        lower_threshold;     // LowThresh
    /* max sharpening limit */
    filter_threshold_type        upper_threshold;     // UpThresh
    /* negative limit */
    filter_threshold_type        negative_threshold;  // NegThresh
    /* max sharpening limit f2 */
    filter_threshold_type        upper_threshold_f2;   // UpThreshF2
    /* negative limit f2 */
    filter_threshold_type        negative_threshold_f2;// NegThreshF2
    /* filter1 sharpening degree */
    filter_sharpen_degree_type   sharpen_degree_f1;   // SharpAmtF1
    /* filter2 sharpening degree */
    filter_sharpen_degree_type   sharpen_degree_f2;   // SharpAmtF2
    /* smoothing degree */
    filter_smoothing_degree_type smoothing_percent;   // SmoothPct (3x3)
    filter_smoothing_degree_type smoothing_percent_5x5;  //[205], range= 0-100, default=90
} asf_setting_type;

typedef struct
{
    /* Filter mode, 0 or 1 */
    unsigned long              filter_mode;                  // FilterMode
    /* LPF: 3x3 smoothing filter used to calculate luma filter */
    smoothing_filter_type smoothing_filter;           // SmoothFltr
    /* LPF: 3x3 laplacian filter used to calculate luma filter */
    laplacian_filter_type laplacian_filter;           // LapFltr
    /* normalize factor to filter 1 */
    normalize_factor_type normalize_factor1;          // Normal1
    /* normalize factor to filter 2 */
    normalize_factor_type normalize_factor2;          // Normal2
    /* HPF: 5x5 filter 1 coefficients */
    matrix_5_5_type filter1;                          // Filter1
    /* HPF: 5x5 filter 2 coefficients */
    matrix_5_5_type filter2;                          // Filter2
    /* extraction factor */
    unsigned char  extraction_factor;                         // ExtrFactor
    /* asf settings based on lighting condition */
    asf_setting_type setting[ASF_MAX_LIGHT];          // Setting
} chromatix_asf_5_5_type;




/*
Color Correction

|R|   |C0 C1 C2||R|   |K0|
|G| = |C3 C4 C5||G| + |K1|
|B|   |C6 C7 C8||B|   |K2|
*/
typedef struct
{
    /* 12-bit, {Q7,Q8,Q9,Q10}, signed */
    /* R */
    float  c0;                                // RtoR
    float  c1;                               // GtoR
    float  c2;                               // BtoR

    /* G */
    float  c3;                               // RtoG
    float  c4;                               // GtoG
    float  c5;                               // BtoG

    /* B */
    float  c6;                               // RtoB
    float  c7;                               // GtoB
    float  c8;                               // BtoB

    /* 11-bit, Q0, signed */
    short  k0;                               // ROffset

    /* 11-bit, Q0, signed */
    short  k1;                               // GOffset

    /* 11-bit, Q0, signed */
    short  k2;                               // BOffset

    /* range 0x0-0x3 maps to Q7-Q10 */
    unsigned char  q_factor;                          // QFactor
} chromatix_color_correction_type;

/* Manual white balance gains */
typedef struct
{
    /* 9-bit, Q7, unsigned */
    float r_gain;                           // RGain

    /* 9-bit, Q7, unsigned */
    float g_gain;                           // GGain

    /* 9-bit, Q7, unsigned */
    float b_gain;                           // BGain
} chromatix_manual_white_balance_type;

/* Advanced gray world white balance gains */
/* For now Advanced gray world white balance gains is defined
* same as manual white balance data type. Need to revisit once
* Matt confirms how it is going to be implemented on 6550 and it is
* going to be the same for 7500.
*/
typedef chromatix_manual_white_balance_type chromatix_agw_white_balance_type;


/******************************************************************************
Gamma
******************************************************************************/

#define GAMMA_TABLE_SIZE           1024
typedef struct {
    /* 8-bit, Q0, unsigned */
    unsigned char gamma[GAMMA_TABLE_SIZE];    // Gamma

    /* 1-bit, Q0, unsigned */
    unsigned char mode;                       // Mode
} chromatix_gamma_table_type;


/******************************************************************************
Chroma enhancement
******************************************************************************/

/* Chroma enhancement

|Cb|   |a  0| |1  b||B - G|   | KCb |
|  | = |    | |    ||     | + |     |
|Cr|   |0  c| |d  1||R - G|   | KCr |

*/
typedef struct
{
    /* 11-bit, Q8, signed*/
    float a_m;                  // CbScalingNegative_am

    /* 11-bit, Q8, signed*/
    float a_p;                  // CbScalingPositive_ap

    /* 11-bit, Q8, signed*/
    float b_m;                  // DiffRGtoCbShearingNegative_bm

    /* 11-bit, Q8, signed*/
    float b_p;                  // DiffRGtoCbShearingPositive_bp

    /* 11-bit, Q8, signed*/
    float c_m;                  // CrScalingNegative_cm

    /* 11-bit, Q8, signed*/
    float c_p;                  // CrScalingPositive_cp

    /* 11-bit, Q8, signed*/
    float d_m;                  // DiffBGtoCrShearingNegative_dm

    /* 11-bit, Q8, signed*/
    float d_p;                  // DiffBGtoCrShearingPositive_dp

    /* 11-bit, Q0, signed */
    short k_cb;                 // CBOffset_k_cb

    /* 11-bit, Q0, signed */
    short k_cr;                 // CROffset_k_cr
} chroma_enhancement_type;

/* RGB-to-Y conversion.

|R|
Y = |VO V1 V2| |G| + K
|B|

*/
typedef struct
{
    /* 12-bit, Q8, signed*/
    float v0;

    /* 12-bit, Q8, signed*/
    float v1;

    /* 12-bit, Q8, signed*/
    float v2;

    /* 12-bit, Q0, unsigned*/
    unsigned short k;
} luma_calculation_type;

/* Color Conversion */
typedef struct
{
    chroma_enhancement_type chroma;           // Chroma
    luma_calculation_type   luma;             // Luma
} chromatix_color_conversion_type;


/* For VFE Color Conversion */
typedef struct
{
    /* R or Cr */
    float  a11;
    float  a12;
    float  a13;
    /* G or Y  */
    float  a21;
    float  a22;
    float  a23;
    /* B or Cb */
    float  a31;
    float  a32;
    float  a33;
    short  bias_y;
    short  bias_Cb;
    short  bias_Cr;
} chromatix_color_conversion_matrix_type;  // for 6K



/******************************************************************************
Adaptive Bayer Filter data types
******************************************************************************/
typedef struct   // old ABF
{
    unsigned short  abf_lp_threshold_table[ADAPTIVE_BAYER_FILTER_TABLE_SIZE]; // LPThr
    unsigned char   abf_shift;                                                // Shift
    float   abf_ratio_table[ADAPTIVE_BAYER_FILTER_TABLE_SIZE];        // Ratio
    unsigned short  abf_min_table[ADAPTIVE_BAYER_FILTER_TABLE_SIZE];          // Min
    unsigned short  abf_max_table[ADAPTIVE_BAYER_FILTER_TABLE_SIZE];          // Max
    unsigned char   abf_shift_lowlight;										//205 mods
} chromatix_adaptive_bayer_filter_data_type;  //7K & old 8K

//205  for new ABF
typedef struct
{
    // red channel parameters
    unsigned short threshold_red[3];  // 12-bit pixels
    float scale_factor_red[2];

    // green channel parameters
    unsigned short threshold_green[3];   //12-bit  pixels
    float scale_factor_green[2];
    float a[2];

    // blue channel parameters
    unsigned short threshold_blue[3];   //12-bit  pixels
    float scale_factor_blue[2];

    // tables for red/green/blue channels
    float table_pos[16];
    float table_neg[8];
    // note: mult_factor is derived from threshold
} chromatix_adaptive_bayer_filter_data_type2;  // new 8K


/******************************************************************************
Auto Flicker Detection data types
******************************************************************************/
typedef struct
{
    float  afd_std_threshold;                             // StdThresh
    unsigned char  afd_percent_threshold;                         // PctThresh
    unsigned long afd_diff_threshold;                            // DiffThresh
    unsigned long afd_frame_ct_threshold;                        // FrameCtThresh
    unsigned char  afd_num_frames;                                //204,  default 6
    unsigned char  afd_frame_skip;                                //204 , default 1
    unsigned long afd_num_rows;                                  //204 , default 480
    unsigned char  afd_num_frames_settle;							//205, default 3
} chromatix_auto_flicker_detection_data_type;

/******************************************************************************
4 Channel Black Level data types
******************************************************************************/

typedef struct
{
    unsigned short                       black_even_row_even_col;                 // BlackEvenRowEvenCol
    unsigned short                       black_even_row_odd_col;                  // BlackEvenRowOddCol
    unsigned short                       black_odd_row_even_col;                  // BlackOddRowEvenCol
    unsigned short                       black_odd_row_odd_col;                   // BlackOddRowOddCol
} chromatix_4_channel_black_level;

/*******************************************************************
1 channel black level data type
********************************************************************/

typedef unsigned short  chromatix_1_channel_black_level_type;  // for 6K


/******************************************************************************
Chroma Suppression data types
******************************************************************************/
typedef struct
{
    unsigned char                                  cs_luma_threshold1;    // LumaThresh1
    unsigned char                                  cs_luma_threshold2;    // LumaThresh2
    unsigned char                                  cs_luma_threshold3;    // LumaThresh3
    unsigned char                                  cs_luma_threshold4;    // LumaThresh4
    unsigned char                                  cs_chroma_threshold1;  // ChromaThresh1
    unsigned char                                  cs_chroma_threshold2;  // ChromaThresh2
} cs_luma_threshold_type;

/******************************************************************************
AF data types
******************************************************************************/
typedef struct
{
    unsigned short      minimumY;
    unsigned short      maximumY;
    float       horizontalOffsetRatio;
    float       verticalOffsetRatio;
    float       horizontalClipRatio;
    float       verticalClipRatio;

} af_vfe_config_type;

typedef struct
{
    char      a00;
    char      a02; // Coefficient a02 is used for 8k only.
    char      a04;
    char      a20;
    char      a21;
    char      a22;
    char      a23;
    char      a24;
} af_vfe_hpf_type;

//204,
//0x207, removed
//typedef enum
//{
//  AF_SINGLE_WINDOW=0,  // single window, default
//  AF_MULTI_WINDOW,    // multiple window,
//  AF_FACE_PIROITY,    // face priority
//  AF_CONTINUOUS,  // continuous AF
//  AF_MAX_TYPE,
//  AF_INVALID_TYPE = AF_MAX_TYPE
//} chromatix_af_type;

//204
//0x207, removed
//typedef enum
//{
//	AF_WINDOW_SINGLE=0,   //default, single window
//	AF_WINDOW_MULTI_CENTRAL,  // pre-defined pattern
//	AF_WINDOW_MULTI_AVERAGE,  // pre-defined pattern
//	AF_WINDOW_MULTI_SPOT,     // pre-defined pattern
//	AF_WINDOW_MULTI_USER1,    // reserved for OEM defined pattern 1
//	AF_WINDOW_MULTI_USER2,    // reserved for OEM defined pattern 2
//	AF_WINDOW_MAX_TYPE,
//	AF_WINDOW_INVALID_TYPE=AF_WINDOW_MAX_TYPE
//} chromatix_af_window_type;

//204
//0x207, removed
//typedef enum
//{
//	AF_NEAR_OBJECT_PRIORITY,
//	AF_FAR_OBJECT_PRIORITY,
//	AF_MAX_PRORITY,
//	AF_INVALID_PRIORITY=AF_MAX_PRORITY
//} chromatix_af_priority_type;

//0x207 remove af_face_priority_type and replace with chromatix_face_priority_type
//typedef enum
//{
//	AF_FACE_PRIORITY_CENTER=0,  // default
//	AF_FACE_PRIORITY_BIG,       // big face
//	AF_FACE_MAX_PRIORITY,
//	AF_FACE_INVALID_PRIORITY=AF_FACE_MAX_PRIORITY
//} chromatix_af_face_priority_type;

typedef struct
{
    //3A  1.8 shake resistant AF
    int AF_shake_resistant_enable; //0x207
	float AF_max_gain ;//was unsigned char
    unsigned char AF_min_frame_luma ;
    float AF_tradeoff_ratio;
    unsigned char AF_shake_resistante_toggle_frame_skip;  //0x207
}AF_shake_resistant_type;

//0x207
typedef enum
{
    FACE_PRIORITY_CENTER=0,  // default
    FACE_PRIORITY_BIG,       // big face
    FACE_MAX_PRIORITY,
    FACE_INVALID_PRIORITY=FACE_MAX_PRIORITY
} chromatix_face_priority_type;
//0x207
typedef enum
{
    FACE_PRIORITY_AF_DOMINANT_FACE=0,
    FACE_PRIORITY_AF_WEIGHTED_AVERAGE,
    FACE_PRIORITY_AF_MAX_TYPE
}chromatix_face_priority_AF_weight_type;


//0x207
typedef enum
{
    MACRO = 0,
    MACRO_LIMITED,
    SUPER_MACRO,
    AF_MACRO_MAX_TYPE
}chromatix_af_macro_type;
//0x207
typedef enum
{
    HILL_CLIMBING = 0,
    FAST_EXHAUSTIVE,
    SLOW_EXHAUSTIVE,
    FAST_HILL_CLIMBING,
    AF_SNAPSHOT_ALGORITHM_MAX_TYPE
}chromatix_af_snapshot_algorithm_type;
//0x207
typedef struct
{
	int CAF_enable; //added on 11-23-10
    unsigned char  af_scene_change_detection_ratio ;
    float     af_panning_stable_fv_change_trigger  ;
    float     af_panning_stable_fvavg_to_fv_change_trigger ;
    unsigned short  af_panning_unstable_trigger_cnt ;

    unsigned short   af_scene_change_trigger_cnt;
    unsigned long    af_downhill_allowance  ;

    unsigned short  af_cont_base_frame_delay;   // default=2, how many frames to wait after lens move
    unsigned short  af_cont_lux_index_change_threshold;  //default=5, refocusing is needed when exp change > threshold
    float   af_cont_threshold_in_noise;  // default =0.05 (FV1-FV0)/FV1, FV1>FV0, determine whether it's just noise

	unsigned short af_cont_search_step_size; //new in 0x208

}AF_CAF_type;

/******************************************************************************
AF motion sensor data types
******************************************************************************/
typedef struct{
    float af_gyro_trigger;  //input above trigger to detect change
    float af_accelerometer_trigger;  //input above trigger to detect change
    float af_magnetometer_trigger ; //input above trigger to detect change
    float af_DIS_motion_vector_trigger ;// input above trigger to detect change
}AF_motion_sensor_type;
/******************************************************************************
AEC data types
******************************************************************************/
typedef struct
{
    int                                is_supported;
    float                                  reduction;
    unsigned long                                 threshold_LO;
    unsigned long								  lux_index_LO;  //204
    unsigned long                                 threshold_HI;
    unsigned long                                 lux_index_HI;  //204
    float                                  discard_ratio;
} aec_outdoor_bright_region_type;

typedef struct
{
    int                                is_supported;
    float                                  threshold_LO;
    float                                  threshold_HI;
    float                                  discard_ratio;
} aec_dark_region_type;

//204
typedef struct
{
    int motion_iso_enable;         // 1=enable, 0=disable, default=1
    float motion_iso_aggressiveness;   // 0=none (disabled), 1=very aggressive
    float motion_iso_threshold;        // default =4.0, motion metric has to > this number to
    // perform gain/exp trade-off
    float motion_iso_max_gain;         // Maximum total gain that can be used for motion iso feature
    // default = 2x higher than max gain in AEC table
} aec_motion_iso_type;

//****************************************************
// for AEC snapshot exposure stretching
//*****************************************************
typedef struct
{
    unsigned short 	lux_index;
    float	gain_trade_off; //less than 1.0 unless wanting to use higher gain in snapshot than preview.
    float	max_exp_time; //max exposure time cap @ this lux index.
} snapshot_trade_off_table_type ;


#define MAX_SNAPSHOT_LUT_SIZE 10
typedef struct
{
    int enable;  // enables snapshot gain and exposure trade-off. default=True
    int exposure_stretch_enable; // Allows extra exposure stretch to meet luma target whenever luma target is not reached , default=true
    unsigned char valid_entries;
    snapshot_trade_off_table_type snapshot_ae_table[MAX_SNAPSHOT_LUT_SIZE];  					//to value of 10 for number of entries.
}snapshot_exposure_type;


/******************************************************************************
AFR data types
******************************************************************************/
// 0x207 removed
//#define AFR_TABLE_SIZE 8
//typedef struct
//{
//   float      fps;
//   int    use_in_auto_frame_rate;
//   float      faster_fps_gain_trigger;
//   float      slower_fps_gain_trigger;
//   long      faster_fps_exp_table_index_mod;
//   long      slower_fps_exp_table_index_mod;
//} afr_type;

typedef struct
{
    //AEC

    //strobe flash estimation --> Improvements for AWB.
    int strobe_enable;
    unsigned long strobe_min_time; //T_min: minimum strobe trigger time (microseconds scale)
    float intersect_time_gain;  //@ T_int: flux gain point where flux gain compresses with respect to trigger time.
    float post_intersect_strobe_flux_rate_increase; // from T_int to T_max: after compression point, what is the trigger time increment for each 1x gain increment.
    float max_strobe_gain; // gain @ T_max:  max flux gain relative to flux at strobe_min_time.
	int strobe_flash_lux_idx_trigger; //added in 0x208

}AEC_strobe_flash_type;

typedef struct
{
    //AEC
    //Touch AEC  3A 2.2
    int touch_roi_enable; //TRUE enables the feature from AEC perspective, FALSE disables the feature.
    float touch_roi_weight; //Combines the ?touched area luma? and the overall AEC metered luma.

}AEC_touch_type;

typedef struct
{
    //Face priority AEC, 3A 2.2

    int aec_face_enable; //TRUE enables the feature from AEC perspective, FALSE disables the feature.
    float aec_face_weight; //0 to 1, determines how much face area contributes for frame luma calculation.  1 means all influence comes from face area.
    float dark_skin_ratio; //skin luma vs. white point luma ratio if close to this value then we set AEC as dark skin.
    float light_skin_ratio; //skin luma vs. white point luma ratio if close to this value then we set AEC as light skin.
    long dark_skin_luma_target; //if dark skin detected, this is luma target is used for AEC (post UI EV update)
    long light_skin_luma_target; //if light skin detected, this is luma target is used for AEC (post UI EV update)

}AEC_face_priority_type;


//0x207
/******************************************************************************
AEC motion sensor data types
******************************************************************************/
typedef struct{
    float aec_gyro_trigger;  //input above trigger to detect change
    float aec_accelerometer_trigger;  //input above trigger to detect change
    float aec_magnetometer_trigger ; //input above trigger to detect change
    float aec_DIS_motion_vector_trigger ;// input above trigger to detect change
}AEC_motion_sensor_type;


/******************************************************************************
LA (8k) data types
******************************************************************************/
typedef struct
{
    float offset;  // default=3.3, range =0.0 to 8.0
    float low_beam;  // default=0.9, range =0.0 to 1.0
    float high_beam;  // default=0.1 , range =0.0 to 1.0
    float histogram_cap;  // default=5.0, range 2.0 to 8.0
    float cap_high;   // default=2.0, range=1.0 to 4.0
    float cap_low;   // default=0.75, range=0.0 to 2.0
} la_8k_type;


/******************************************************************************
MCE (Memory Color Enhancement) data types
******************************************************************************/
typedef struct
{
    unsigned char green_y[4];  // default = {20, 40, 200, 235}
    float green_boost_factor;  // default=1.4;
    char  green_cr_boundary;   // default=0, range=[-127,127]
    char  green_cb_boundary;   // default=-12, range=[-127.127]
    unsigned char green_cr_transition_width; // default=20, range=[4,31]
    unsigned char green_cb_transition_width; // default=20, range=[4,31]
    unsigned short green_bright_index;   // default = AEC outdoor index, (<this, use full boost factor)
    unsigned short green_dark_index;   // default = AEC (indoor+outdoor)/2 index  (>this, boost factor=1.0)

    unsigned char blue_y[4];  // default = {80, 150, 235, 255}
    float blue_boost_factor;  // default=1.3;
    char  blue_cr_boundary;   // default=-12, range=[-127,127]
    char  blue_cb_boundary;   // default=25, range=[-127.127]
    unsigned char blue_cr_transition_width; // default=25, range=[4,31]
    unsigned char blue_cb_transition_width; // default=25, range=[4,31]
    unsigned short blue_bright_index;   // default = AEC outdoor index (<this, use full boost factor)
    unsigned short blue_dark_index;   // default = AEC (indoor+outdoor)/2 index (>this, boost factor=1.0)

    unsigned char red_y[4];  // default = {10, 30, 200, 235}
    float red_boost_factor;  // default=1.2;
    char  red_cr_boundary;   // default=45, range=[-127,127]
    char  red_cb_boundary;   // default=127, range=[-127.127]
    unsigned char red_cr_transition_width; // default=25, range=[4,31]
    unsigned char red_cb_transition_width; // default=25, range=[4,31]
    unsigned short red_bright_index;   // default = AEC outdoor index (<this, use full boost factor)
    unsigned short red_dark_index;   // default = AEC (indoor+outdoor)/2 index (>this, boost factor=1.0)

} mce_type;


//****************************************************
// for skin color enhancement
//*****************************************************
typedef struct
{
    int cr;
    int cb;
} cr_cb_point;

typedef struct
{
    cr_cb_point point1;
    cr_cb_point point2;
    cr_cb_point point3;
} cr_cb_triangle;

typedef struct
{
    cr_cb_triangle triangle1;
    cr_cb_triangle triangle2;
    cr_cb_triangle triangle3;
    cr_cb_triangle triangle4;
    cr_cb_triangle triangle5;
} sce_cr_cb_triangle_set;

typedef struct
{
    float a,b,c,d,e,f;  // default: a=1, b=0, c=0, d=0,e=1, f=0.
    //  cr' = [ a   b    c] cr
    //  cb' = [ d   e    f] cb
    //   1  = [ 0   0    1] 1
} sce_affine_transform_2d;

//begin 0x206

#define NUM_AEC_STATS 16

//removed in 0x207 , replaced with new type
//typedef struct
//{
//  unsigned short  bpc_offset_r_hi;
//  unsigned short  bpc_offset_r_lo;

//  unsigned short  bpc_offset_g_hi;
//  unsigned short  bpc_offset_g_lo;

//  unsigned short  bpc_offset_b_hi;
//  unsigned short  bpc_offset_b_lo;
//} bpc_offset_type;

//BPC new offset struct including 4 channels (7x30,8660 has 3 channels)
//0x207
typedef struct
{
    unsigned short  bpc_4_offset_r_hi;
    unsigned short  bpc_4_offset_r_lo;

    unsigned short  bpc_4_offset_gr_hi;
    unsigned short  bpc_4_offset_gr_lo;

    unsigned short  bpc_4_offset_gb_hi;
    unsigned short  bpc_4_offset_gb_lo;

    unsigned short  bpc_4_offset_b_hi;
    unsigned short  bpc_4_offset_b_lo;
} bpc_4_offset_type;

//0x207 for bad cluster

//#define ABCC_LUT_SIZE 512
//  typedef struct {

// long pixel_index[ABCC_LUT_SIZE];   // ABCC index table
//short kernel_index[ABCC_LUT_SIZE];  // 25 bits: defect address, 3 bits: kernel index
//short skip0_index[ABCC_LUT_SIZE];   // 3 bits: skip0_table, 3 bits: skip1_table
//short skip1_index[ABCC_LUT_SIZE];

//}abcc_table_type;

//0x207 demosaic3

typedef struct
{
    float wk[18];
    int bk[18];
    int lk[18];
    int Tk[18];
} demosaic3_LUT_type;

//207 chroma filter
typedef struct
{
    float h[4];
    float v[2];
} Chroma_filter_type;


//wavelet denoise
#define NUM_NOISE_PROFILE 24
typedef struct
{
    float referencePatchAverageValue; // default 0
    float referenceNoiseProfileData [NUM_NOISE_PROFILE];  // default 0
} ReferenceNoiseProfile_type;

typedef struct
{
    unsigned short r_lut_p_l[8]; // 12uQ0
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
}chromatix_linearization_type;


typedef struct
{
    //3A 2.0
    //scene change detection I-frame trigger

    int scd_3a_enable;// 0: scene change detection feature is disabled; 1: scene change detection feature is enabled.
    float scd_aec_para_mean; // this parameter is used to control the contribution to the dynamic threshold from the mean value of the latest scd_aec_dy_window frames? SADs (sum of the absolute difference). The default value is: 2. The range of this parameter is [1, 4].
    float scd_aec_para_std; // this parameter is used to control the contribution to the dynamic threshold from the standard deviation of the latest scd_aec_dy_window frames? SADs (sum of the absolute difference). The default value is: 2.76. The range of this parameter is [1, 10].
    unsigned short scd_aec_dy_window;// this value is used to control the window size (how many frames) for computing the mean and the standard deviation. The default value is: 15 frames. The range of this value is [1, maximum frame rate that can be achieved]
    unsigned short scd_threshold_aec_lux; // this parameter is a predefined threshold in AEC scene change detection lux index method. The default value is: 10. The range of this threshold is [10, 100].
    float scd_af_range_threshold;// used to control the threshold of scene change detection in auto focus. Range of this parameter is [0, 1]. Default value is 0.5. The higher the parameter, the higher the thresholds for scene change detection in auto focus.

}scene_change_detect_type;

typedef struct
{
    //Snow/cloudy Scene Detection

    int snow_scene_detection_enable; // TRUE enables the feature from AEC perspective, FALSE disables the feature.
    unsigned long y_cloudy_snow_threshold; // AE region?s min luma to be considered a potential snow/cloudy region.
    unsigned long awb_y_max_in_grey; // Any AE region above this threshold is considered potential snow/cloudy region.  These regions will not have WB data because WB considered too bright and above YMAX WB configuration.
    unsigned long min_snow_cloudy_sample_th; // If count of snow/cloudy regions detected above this threshold, we consider the scene as snow/cloudy.
    unsigned long extreme_snow_cloudy_sample_th; // if count of detected snow regions above this threshold, scene is considered extreme snow/cloudy scene.  Luma offset is maxed.
    float extreme_luma_target_offset;// Maximum luma offset that can be applied when scnow scene is detected.  This happens when extreme_snow_cloudy_sample_th is reached or exceeded.  Luma offset is gradually reduced for darker scenes until eventually made 0 for indoor cases, this is based on exp_index. (cannot use lux_idx due to rapid change based on frame luma, will cause luma offset to be unstable).
    unsigned long severe_snow_scene_cap; //? At what point will ?extreme_luma_target_offset? be applied, value is 0 to 255 for severity.  255 means snow scene detection must report 255 severity for extreme_luma_target_offset to be applied.
    float snowscene_aggressiveness; //added in 0x207
	unsigned char ui_snow_cloudy_display_th; //added in 0x208
}snow_scene_detect_type;

typedef struct
{
    //Backlit Scene Detection

    int backlight_detection_enable; //TRUE enables the feature from AEC perspective, FALSE disables the feature.
    unsigned long low_luma_threshold; // Histogram samples which luma is below this threshold are added to low luma count.
    unsigned long high_luma_threshold; // Histogram samples which luma is above this threshold are added to high luma count.
    float low_luma_count_percent_threshold ; // If count of low luma samples exceed this percentage of total samples, we consider potential backlight case.
    float high_luma_count_percent_threshold ;// If count of high luma samples exceed this percentage of total samples, we consider potential backlight case.
    la_8k_type backlight_la_8k_config; // Luma Adaptation configuration when backlight scene is detected, compensation portion.
    float backlight_max_la_luma_target_adj; // Maximum luma target adjustment when backlight is detected.   We expect to increase luma target.
    float backlit_aggressiveness; //added in 0x207
	float	max_percent_threshold; //added in 0x208, defines interpolation range of backlit severity in 		//histogram detector.
	unsigned char ui_backlit_display_th;//added in 0x208
}backlit_scene_detect_type;


//added 207
/******************************************************************************
//red eye reduction
******************************************************************************/

typedef struct
{
   // int enable; // enable the feature //in 0x208 replaced this with the following 2 params
	int red_eye_reduction_xenon_strobe_enable; //0x208
   int red_eye_reduction_led_flash_enable; //0x208
    int number_of_preflash_cycles; // default =3
    int preflash_interval_between_pulese; // default = 45, unit = msec
    int preflash_Xenon_pulese_duration;  // default = 20 , unit  = usec
    int preflash_LED_pulse_duration;  // default = 30, unit = msec
    int preflash_LED_current;   // default = 600, unit = mA.
}red_eye_reduction_type;


//0x208
typedef struct
{
  int landscape_detection_enable; //TRUE enables the feature ,FALSE disables the feature.
  float landscape_red_boost_factor;
  float landscape_green_boost_factor;
  float landscape_blue_boost_factor;
  float min_blue_green_content_detection_threshold; //min detection
  float max_blue_green_content_detecton_threshold; //stop interpolation detection range
  int   green_offset_rg; //for extreme green zone boundary config
  int   green_offset_bg; //for extreme green zone boundary config
  float aggressiveness;  //response of temporal filter to severity change
  long lux_idx_indoor;  //lux_idx above this threshold disabled landscape detection
  long lux_idx_outdoor; //lux idx below this threshold has full application of landscape scene compensation.  Between indoor and outdoor, we interpolate severity.
  unsigned char ui_landscape_display_th;
}landscape_scene_detect_type;

/******************************************************************************
//Slope-predictive Auto-focus parameters
******************************************************************************/
typedef struct
{
  int portrait_detection_enable;
  float skin_color_boost_factor;
  float min_face_content_threshold;
  float max_face_content_threshold;
  filter_sharpen_degree_type  soft_focus_degree;
  float aggressiveness;
  unsigned char ui_portrait_display_th;
}portrait_scene_detect_type;

typedef struct
{
    float af_fv_curve_flat_threshold;  // def: 0.9 - threshold to determine if
                                       //FV curve is flat
    float af_slope_threshold1;   // default: 0.9
    float af_slope_threshold2;  // default: 1.1
    float af_slope_threshold3;  // default: 0.5
    float af_slope_threshold4;  // default: 3
    unsigned int af_lens_pos_0;  // Lens poisiton when the object is at 3m
    unsigned int af_lens_pos_1;  // Lens poisiton when the object is at 70cm
    unsigned int af_lens_pos_2;  // Lens poisiton when the object is at 30cm
    unsigned int af_lens_pos_3;  // Lens poisiton when the object is at 20cm
    unsigned int af_lens_pos_4;  // Lens poisiton when the object is at 10cm
    unsigned int af_lens_pos_5;  // Lens poisiton Macro
    unsigned int af_sp_frame_delay;  // default: 1
    int af_downhill_allowance;  // max number of consecutive downhill in the
                                //first 4 or 6 samples
    int af_downhill_allowance_1;  // max number of consecutive downhill in 2 or
                                  // 3 round
}AF_slope_predictive_type;

typedef struct {
  int lux_index;
  float green_rg_offset_adj;
  float green_bg_offset_adj;
  float outlier_dist_adj;
} low_light_adj_type;

typedef struct {
  unsigned char enable;
  low_light_adj_type lut_entry[6];
} awb_lowlight_adj_lut_type;

/******************************************************************************
******************************************************************************
CHROMATIX GENERATED HEADER FORMAT
******************************************************************************
******************************************************************************/

/* Chromatix generates the complete following data */
typedef struct
{

    /* Always equals the pound-define CHROMATIX_DMSS7500_VERSION
    * so that DMSS can flag an error if it's chromatix_dmss7500
    * version does not match this one
    */
    chromatix_version_type                 chromatix_version;

    /******************************************************************************
    Color Correction     (ColorCorr)
    ******************************************************************************/
    tuning_control_type     control_cc; //default 0 for lux_index

    trigger_point_type          cc_snapshot_trigger;
    //existing set is for TL84
    chromatix_color_correction_type        chromatix_TL84_color_correction_snapshot;        // default
    chromatix_color_correction_type        chromatix_yhi_ylo_color_correction_snapshot;        // LowLight
    //0x207
    chromatix_CCT_trigger_type CC_A_trigger_snapshot;
    chromatix_CCT_trigger_type CC_Daylight_trigger_snapshot;

    chromatix_color_correction_type        chromatix_D65_color_correction_snapshot;        // default
    chromatix_color_correction_type        chromatix_A_color_correction_snapshot;        // LowLight

    //0x207
    chromatix_CCT_trigger_type CC_LED_trigger_snapshot;

    chromatix_color_correction_type        chromatix_LED_color_correction_snapshot;
    //0x207

    chromatix_CCT_trigger_type CC_STROBE_trigger_snapshot;

    chromatix_color_correction_type        chromatix_STROBE_color_correction;

    //VF, 0x207
    trigger_point_type          cc_VF_trigger;
    chromatix_color_correction_type        chromatix_TL84_color_correction;        // default
    chromatix_color_correction_type        chromatix_yhi_ylo_color_correction;        // LowLight

    chromatix_CCT_trigger_type CC_LED_trigger_VF;
    chromatix_color_correction_type        chromatix_LED_color_correction_VF;

    //0x207
    chromatix_CCT_trigger_type CC_A_trigger_VF;
    chromatix_CCT_trigger_type CC_Daylight_trigger_VF;

    chromatix_color_correction_type        chromatix_D65_color_correction_VF;        // default
    chromatix_color_correction_type        chromatix_A_color_correction_VF;        // LowLight


    /******************************************************************************
    Color Conversion   (ColorConv)
    ******************************************************************************/
    //VF and snapshot use the same parameters
    tuning_control_type     control_cv;//default 0 for lux_index
    trigger_point_type          cv_trigger;

    chromatix_color_conversion_type        chromatix_tl84_color_conversion;           // TL84
    chromatix_color_conversion_type        chromatix_incandescent_color_conversion;   // A
    chromatix_color_conversion_type        chromatix_daylight_color_conversion;       // D65
    chromatix_color_conversion_type        chromatix_yhi_ylo_color_conversion;        // LowLight
    chromatix_color_conversion_type        chromatix_outdoor_color_conversion;        // Outdoor  (205)
    chromatix_color_conversion_type        chromatix_mono_color_conversion;           // Mono
    chromatix_color_conversion_type        chromatix_sepia_color_conversion;          // Sepia
    chromatix_color_conversion_type        chromatix_negative_color_conversion;       // Negative
    chromatix_color_conversion_type        chromatix_aqua_color_conversion;           // Aqua

    /* saturated color conversion */ //subgroup all prefixed with BSM
    float                                  saturated_color_conversion_factor;         // SatFactor
    chromatix_color_conversion_type        sunset_color_conversion;                   // Sunset
    chromatix_color_conversion_type        skintone_color_conversion;                 // SkinTL84
    chromatix_color_conversion_type        skintone_color_conversion_d65;             // SkinD65
    chromatix_color_conversion_type        skintone_color_conversion_a;               // SkinA

    //0x206, removed these in 0x207, use CC_LED, CC_Strobe instead
    //  float CV_LED_start;
    //  float CV_LED_end;
    //  chromatix_color_conversion_type        chromatix_color_conversion_LED;
    //  float CV_Strobe_start;
    //  float CV_Strobe_end  ;
    //  chromatix_color_conversion_type        chromatix_color_conversion_Strobe;

    //0x207

    chromatix_CCT_trigger_type CV_A_trigger;
    chromatix_CCT_trigger_type CV_Daylight_trigger;


    // for 6K (3x3 matrix)
    chromatix_color_conversion_matrix_type		chromatix_tl84_color_conversion_matrix;           // TL84
    chromatix_color_conversion_matrix_type        chromatix_incandescent_color_conversion_matrix;   // A
    chromatix_color_conversion_matrix_type        chromatix_daylight_color_conversion_matrix;       // D65
    chromatix_color_conversion_matrix_type        chromatix_yhi_ylo_color_conversion_matrix;        // LowLight
    chromatix_color_conversion_matrix_type        chromatix_outdoor_color_conversion_matrix;        // Outdoor  (205)
    chromatix_color_conversion_matrix_type        chromatix_mono_color_conversion_matrix;           // Mono
    chromatix_color_conversion_matrix_type        chromatix_sepia_color_conversion_matrix;          // Sepia
    chromatix_color_conversion_matrix_type        chromatix_negative_color_conversion_matrix;       // Negative
    chromatix_color_conversion_matrix_type        chromatix_aqua_color_conversion_matrix;           // Aqua
    // 6K BSM 3x3 matrix
    chromatix_color_conversion_matrix_type        sunset_color_conversion_matrix;                   // Sunset
    chromatix_color_conversion_matrix_type        skintone_color_conversion_matrix;                 // SkinTL84
    chromatix_color_conversion_matrix_type        skintone_color_conversion_d65_matrix;             // SkinD65
    chromatix_color_conversion_matrix_type        skintone_color_conversion_a_matrix;               // SkinA


    /******************************************************************************
    MWB    ** Manual white balance gains **
    ******************************************************************************/

    chromatix_manual_white_balance_type    chromatix_tl84_white_balance;            // Flourescent
    chromatix_manual_white_balance_type    chromatix_d50_white_balance;             // Sunny
    chromatix_manual_white_balance_type    chromatix_incandescent_white_balance;    // Tungsten
    chromatix_manual_white_balance_type    chromatix_d65_white_balance;             // Cloudy/Shade
    chromatix_manual_white_balance_type    strobe_flash_white_balance;              // Strobe
    chromatix_manual_white_balance_type    led_flash_white_balance;                 // LED

    /******************************************************************************
    AWB
    ******************************************************************************/
    /* channel balance */
    chromatix_channel_balance_gains_type   chromatix_channel_balance_gains;          // ChannelGain

    /* Auto white balance reference points, h/w lens rolloff present */
    chromatix_awb_reference_type           awb_reference_hw_rolloff [AGW_AWB_MAX_LIGHT-1]; // RefPoints
    /* Stats bounding box with hw rolloff*/
    chromatix_wb_exp_stats_type            wb_exp_stats_hw_rolloff [AWB_STATS_MAX_LIGHT];  // StatConfig

    long                                  awb_indoor_index;                            // IndoorIndex
    long                                  awb_outdoor_index;                           // OutdoorIndex

    float                                  snow_blue_gain_adj_ratio;                // SnowBlueGainAdj
    float                                  beach_blue_gain_adj_ratio;               // BeachBlueGainAdj

    int                                    outlier_distance;                        // OutlierDist
    int                                    green_offset_rg;                         // GreenOffRG
    int                                    green_offset_bg;                         // GreenOffBG
    unsigned char                                  awb_skip_frames_after_changing_VFE;      // default value is 3

    float                                 awb_extreme_RG_ratio_threshold;       // default=2.0, extreme color threshold in R/G direction
    float                                 awb_extreme_BG_ratio_threshold;       // default=1.5, extreme color threshold in B/G direction

    /******************************************************************************
    AWBExt  (extended AWB parameters)
    ******************************************************************************/

    unsigned short                                 compact_cluster_r2;                      // CClusterDistSq
    unsigned short                                 compact_cluster_to_ref_point_r2;         // CClusterToRefSq
    unsigned char                                  a_cluster_threshold;  /* pcercentage */  // AClusterThresh
    unsigned char                                  f_cluster_threshold;  /* pcercentage */  // FClusterThresh
    unsigned char                                  day_cluster_threshold;  /* pcercentage */ // DayClusterThresh
    unsigned char                                  outdoor_green_threshold;                  // OutdoorGThresh
    unsigned char                                  outdoor_green_threshold_bright_F;         // OutdoorGThrBrtF
    unsigned char                                  outdoor_green_threshold_dark_F;           // OutdoorGThrDarkF
    unsigned char                                  day_cluster_threshold_for_F;              // DayClusterThrF
    unsigned char                                  white_balance_allow_fline;                // WBAllowFLine
    unsigned char                                  outdoor_valid_sample_cnt_threshold;       // OutValidSamplesThr
    unsigned char                                  outdoor_green_upper_threshold;            // OutGreenUpThresh
    unsigned short                                 r2_threshold;  /* distance square */      // RadiusSquaredThr
    unsigned char                                  outdoor_green_threshold_bright_A;         // OutGThreshBrightA
    unsigned char                                  outdoor_green_threshold_dark_A;           // OutGThreshDarkA
    unsigned char                                  day_cluster_threshold_for_A;              // DayClusterThrA

    float                                  color_correction_global_gain;              // CCGlobalGain
    chromatix_manual_white_balance_type    awb_min_gains;                             // MinGains
    chromatix_manual_white_balance_type    awb_max_gains;                             // MaxGains
    chromatix_awb_sample_influence_type    sample_influence;                          // SampleInfluence
    chromatix_awb_weight_vector_type       awb_weight_vector [AGW_AWB_MAX_LIGHT];     // WeightVector

    int                                    awb_white_world_y_min_ratio;               // WhiteWorldYMinRatio

    unsigned char                                  awb_aggressiveness;                        // Aggressiveness
    int                                awb_self_cal_enable;                       // SelfCalEnable
    //204
    float                                  awb_self_cal_adj_ratio_high;               // default=1.15
    float                                  awb_self_cal_adj_ratio_low;                // default = 0.9
    //205
    int								 awb_enable_lock_heuristics_1;				// default=1;
    int								 awb_enable_lock_heuristics_2;				// default=1;
    int								 awb_enable_lock_heuristics_3;				// default=1;
    int								 awb_enable_white_world;					// default=1;


    //0x206
    AWB_purple_prevent_type AWB_purple_prevent; //0x207 added enable variable
    float AWB_R_adj_VF[AGW_AWB_MAX_LIGHT-1]; //fixed typo in 0x207
    float AWB_B_adj_VF[AGW_AWB_MAX_LIGHT-1];  //fixed typo in 0x207
    float AWB_R_adj_lowlight_VF[AWB_MAX_LOWLIGHT];
    float AWB_B_adj_lowlight_VF[AWB_MAX_LOWLIGHT];
    float AWB_R_adj_lowlight_snapshot[AWB_MAX_LOWLIGHT];
    float AWB_B_adj_lowlight_snapshot[AWB_MAX_LOWLIGHT];

    float AWB_golden_module_R_Gr_ratio;
    float AWB_golden_module_Gb_Gr_ratio;
    float AWB_golden_module_B_Gr_ratio;

    //0x207
    int enable_AWB_module_cal;// (???)
    AWB_motion_sensor_type AWB_motion_sensor_data;

    //Nokia STATs

    AWB2_parameters_type AWB2_params;




    /******************************************************************************
    AEC
    ******************************************************************************/
    chromatix_exposure_table_type          chromatix_exposure_table;                  // Table

    unsigned char                                  default_luma_target;                       // Default
    unsigned char                                  outdoor_luma_target;                       // Outdoor
    unsigned char                                  lowlight_luma_target;                      // LowLight
    //removed from 0x207
    //unsigned char                                  backlight_luma_target;             // BacklightLuma

    long                                  luma_tolerance;                    // LumaTolerance
    float                                  exposure_index_adj_step;           // ExpIndexAdjStep
    float                                  ISO100_gain;                       // ISO100Gain

    long                                  aec_indoor_index;                 // IndoorIndex
    long                                  aec_outdoor_index;                // OutdoorIndex
    //removed from 0x207
    //  float                                  max_preview_gain_allowed;
    //  float                                  min_preview_gain_allowed;
    //  float                                  max_snapshot_gain_allowed;
    //  float                                  min_snapshot_gain_allowed;
    float                                  max_snapshot_exposure_time_allowed;
    float                                  aggressiveness_values;
    unsigned short                                 fix_fps_aec_table_index;
    int                                linear_afr_support;
    unsigned long                                 high_luma_region_threshold;
    //removed in 0x207 int                                aec_digital_gain_is_supported;
    aec_outdoor_bright_region_type         bright_region;
    aec_dark_region_type                   dark_region;
    //0x203
    unsigned short                                 wled_trigger_idx;
    float                                  aec_led_preview_flux;
    //float                                  aec_led_snapshot_flux;  //changed in 0x205
    float                                  aec_led_snapshot_flux_hi; //replaces aec_led_snapshot_flux, 0x205 mods
    float                                  aec_led_snapshot_flux_med; //0x205 mods
    float                                  aec_led_snapshot_flux_low;  //0x205 mods



    //204
    aec_motion_iso_type					 aec_motion_iso_snapshot;
    aec_motion_iso_type					 aec_motion_iso_preview;
    aec_motion_iso_type					 aec_motion_iso_video;
    //205
    snapshot_exposure_type                 aec_exp_stretch;
    int									aec_conv_holding_time_adj;  //205

    /******** AEC parameters below not used in 8k/OpenMM. 7k only. *************/
    float                                  max_video_fps;  //Android only
    // remove in 0x207
    //float                                  video_fps;
    // float                                  max_preview_fps;
    // float                                  preview_fps;
    // float                                  nightshot_fps;
    //unsigned short                                 nightshot_table_index;

    //0x206 AEC
    unsigned short max_sensitivity_lux_index ;
    float AEC_weight_center_weighted[NUM_AEC_STATS][NUM_AEC_STATS];
    float AEC_weight_spot_meter[NUM_AEC_STATS][NUM_AEC_STATS];

    AEC_strobe_flash_type AEC_strobe_flash; // added strobe_flash_lux_idx_trigger in 0x208
    AEC_touch_type AEC_touch;

    AEC_face_priority_type AEC_face_priority;
    //0x207
    AEC_motion_sensor_type AEC_motion_sensor_data;
	//0x208
	unsigned long  aec_fast_convergence_skip;  //determines how often fast convergence //updates exposure index
	unsigned long  aec_slow_convergence_skip;  //determines how often slow convergence //updates exposure index


    /******************************************************************************
    AF  (Auto Focus)
    ******************************************************************************/
    //0x207 removed
    //unsigned short             total_steps;
    unsigned short             num_gross_steps_between_stat_points;
    unsigned short             num_fine_steps_between_stat_points;
    unsigned short             num_fine_search_points;
    unsigned short             af_process_type;
    unsigned short             position_near_end;
    unsigned short             position_default_in_macro;
    unsigned short             position_boundary;
    unsigned short             position_default_in_normal;
    unsigned short             position_far_end;
    //0x207 removed
    // float              steps_gain;
    // unsigned short             initial_current;
    // unsigned short             num_steps_near_to_far;
    int            undershoot_protect;
    unsigned short             undershoot_adjust;
    int            reset_lens_after_snap;
    af_vfe_config_type af_config;
    unsigned short             fv_metric; // 0 = Sum of FV, 1 = Max of FV. Default = 1
    af_vfe_hpf_type    af_vfe_hpf;


    // AF multi-window
    //
    /*******************************************************************************************************************************************
    // The features below are designed for continuous AF use on 7k and 8k only.
    // feature name: Continuous AF Control Parameters
    // variable name: all below
    // Brief description of the parameter: please check the comments for each variable.
    // applicale chipset(s): For 7k and 8k only.
    // default value:  please check the comments for detailed descriptions
    // data range: please check the comments
    // constraints: please check the comments
    // effect: Variables required to design:
    //         1. focus window type (multi window or single window)
    //         2. focus window size and location (check each data structure for details)
    //         3. continuous AF search parameters such as:
    //            - monitor interval (how often continous AF algorithm should run)
    //            - frame delay between each move of the sensor (required to let the lens settle down before reading FV)
    //            - threshold to determine when to start continuous AF when there is a change in FV
    //
    //
    ********************************************************************************************************************************************/

    //0x207 removed.
    // unsigned short      AFWindowType; // 0 = single window (Default), 1= multiple-central, 2= multiple-average, 3 = multiple-spot, 4 & 5 are user defined;
    // int     af_SkinAssisted; // 0 = no Skin-tone Priority (Default), 1 = with Skin-tone Priority.

    // removed in 0x206
    //unsigned short  af_cont_monitor_interval;   // in msec, how often to check FV, default=200ms
    // removed in 0x206
    // unsigned short  af_cont_search_step_size;   // defaul=2, twice as large as that for snapshot AF(=1), number of steps the lens need to move at a time
    //in 0x207 moved to CAF struct
    //unsigned short  af_cont_base_frame_delay;   // default=2, how many frames to wait after lens move
    //unsigned short  af_cont_lux_index_change_threshold;  //default=5, refocusing is needed when exp change > threshold
    //float   af_cont_threshold_in_noise;  // default =0.05 (FV1-FV0)/FV1, FV1>FV0, determine whether it's just noise

    // for face priority AF
    //204
    //removed on 0x207   unsigned short af_max_faces;  // default=0, max number of faces that can be detected.
    //removed in 0x207, replaced with generic face_priority
    // chromatix_af_face_priority_type af_face_priority;  // default=0 (center face), 7K VFE has limitation of 16 AF grids
    //removed on 0x207 int af_is_partial_face_ok; //default = 1 (yes)
    // 0x206
    AF_shake_resistant_type AF_shake_resistant; //0x207 added enable variable, and toggle frame skip
    unsigned char AF_scene_change_detection_ratio; //for continous AF
    float AF_peak_drop_down_factor; //AF algorithm
    //0x207

    AF_CAF_type af_CAF;
    int basedelay_snapshot_AF;
    chromatix_af_macro_type af_macro;
    chromatix_af_snapshot_algorithm_type af_snapshot_algorithm;
    chromatix_face_priority_AF_weight_type face_priority_AF_weight;
    chromatix_face_priority_type chromatix_face_priority;

    AF_motion_sensor_type AF_motion_sensor;
	//0x208 LED assisted AF
	int led_af_assist_enable;
	long led_af_assist_trigger_idx;


    /******************************************************************************
    Gamma
    ******************************************************************************/
    tuning_control_type     control_gamma;//default 0 for lux_index

    trigger_point_type          gamma_snapshot_lowlight_trigger;
    trigger_point_type          gamma_snapshot_outdoor_trigger;

    chromatix_gamma_table_type             chromatix_rgb_default_gamma_table_snapshot;         // Default
    chromatix_gamma_table_type             chromatix_rgb_yhi_ylo_gamma_table_snapshot;         // LowLight 8k
    chromatix_gamma_table_type             chromatix_rgb_outdoor_gamma_table_snapshot;         // Outdoor 8k

    trigger_point_type          gamma_lowlight_trigger;
    trigger_point_type          gamma_outdoor_trigger;

    chromatix_gamma_table_type             chromatix_rgb_default_gamma_table;         // Default
    chromatix_gamma_table_type             chromatix_rgb_yhi_ylo_gamma_table;         // LowLight
    chromatix_gamma_table_type             chromatix_rgb_outdoor_gamma_table;         // Outdoor
    chromatix_gamma_table_type             chromatix_rgb_backlight_gamma_table;       // Backlight
    unsigned char                                  chromatix_solarize_reflection_point;       // SolarizeReflection

    /******************************************************************************
    Black Level     (BlackLevel)
    ******************************************************************************/
    tuning_control_type     control_blk;//default 0 for lux_index

    trigger_point_type          blk_snapshot_lowlight_trigger;
    unsigned short                max_blk_increase_snapshot; //default 8*16

    chromatix_black_level_offset_type      normal_light_black_level_offset_snapshot;    // NormalLightBlackLevelOffset
    chromatix_4_channel_black_level        normal_light_4_channel_snapshot;             // NormalLight4Channel


    trigger_point_type          blk_lowlight_trigger;
    unsigned short                max_blk_increase; //default 15*16 in 12bit

    chromatix_black_level_offset_type      normal_light_black_level_offset;    // NormalLightBlackLevelOffset
    chromatix_4_channel_black_level        normal_light_4_channel;             // NormalLight4Channel

    chromatix_1_channel_black_level_type   normal_light_1_channel_black_level_offset_snapshot;  // 6K
    chromatix_1_channel_black_level_type   normal_light_1_channel_black_level_offset;  // 6K

    /******************************************************************************
    Lens Rolloff   (Rolloff)
    ******************************************************************************/
    tuning_control_type         control_rolloff;//default 0 for lux_index

    trigger_point_type      rolloff_lowlight_trigger; //0x208

    float                    gain_lowlight_rolloff;
    int                       lux_index_lowlight_rolloff;

    chromatix_rolloff_table_type           chromatix_rolloff_table[ROLLOFF_MAX_LIGHT];      //Table
    mesh_rolloff_array_type                chromatix_mesh_rolloff_table[ROLLOFF_MAX_LIGHT]; // Mesh

    // 0x206, use led influence params from AEC, sensitivity_low, senstivity_high, sensitivity_off
    float rolloff_LED_start;
    float rolloff_LED_end ;
    mesh_rolloff_array_type                chromatix_mesh_rolloff_table_LED;

    float rolloff_Strobe_start;
    float rolloff_Strobe_end  ;
    mesh_rolloff_array_type                chromatix_mesh_rolloff_table_Strobe;

    //0x207, for snapshot only
    chromatix_CCT_trigger_type rolloff_A_trigger_snapshot;
    chromatix_CCT_trigger_type rolloff_D65_trigger_snapshot;

	//0x208
	//float PCA_basis_table[PCA_ROLLOFF_NUMBER_BASE][PCA_ROLLOFF_NUMBER_COLS];    // [8][17]11uQ11 for the 1st basis, 8sQ7 for the 2nd to the 8th bases
    //int use_common_base; //indicate if we use the same base for all illuminant.feault is 1, SW do not need to update base table on the fly. if it is 0, SW needs to update base table
	//PCA_RolloffStruct  PCA_rolloff_FL;
	//PCA_RolloffStruct  PCA_rolloff_Day;
	//PCA_RolloffStruct  PCA_rolloff_A;
	//PCA_RolloffStruct  PCA_rolloff_lowlight;
	//PCA_RolloffStruct  PCA_rolloff_LED;
	//PCA_RolloffStruct  PCA_rolloff_strobe;



    /******************************************************************************
    3x3 H/W ASF   (3X3ASF)
    ******************************************************************************/
    tuning_control_type control_asf_3x3; //DEFAULT 1 GAIN

    trigger_point_type          asf_3x3_lowlight_trigger;
    trigger_point_type          asf_3x3_outdoor_trigger;

    float asf_3_3_sharp_min_ds_factor;  //default value 0.5
    float asf_3_3_sharp_max_ds_factor; //default 4
    float asf_3_3_sharp_max_factor;  //default 1 for 8k and 6k, 2.0 for 7k

    chromatix_adaptive_spatial_filter_type chromatix_adaptive_spatial_filter;   // Regular
    chromatix_adaptive_spatial_filter_type chromatix_low_light_asf;             // LowLight


    /******************************************************************************
    5x5 ASF      (5X5ASF)
    ******************************************************************************/
    tuning_control_type control_asf_5X5; //DEFAULT 1 GAIN default value for VF and snapshot is same

    trigger_point_type          asf_5x5_lowlight_trigger;
    trigger_point_type          asf_5x5_outdoor_trigger;

    chromatix_asf_5_5_type                 asf_5_5;                             // SnapshotFilter
    /* soft_focus_degree */
    filter_sharpen_degree_type             soft_focus_degree;                   // SoftFocusDegree

    /* For Viewfinder/preview */
    chromatix_asf_5_5_type                 asf_5_5_preview;                     // PreviewFilter

    float asf_5_5_sharp_min_ds_factor; //default 0.5
    float asf_5_5_sharp_max_ds_factor; //default 4.0
    float asf_5_5_sharp_max_factor;   //default 1 for 6k, 8k, old 7k (before sROC). default 2.0 for sROC.

    /******************************************************************************
    Luma Adaptation   (LumaAdapt)  (7K)
    ******************************************************************************/
    unsigned char                                  num_iteration;                       // NumIterations
    unsigned char                                  low_percentage;                      // LowPercentage
    unsigned char                                  low_range;                           // LowRange
    unsigned char                                  high_percentage;                     // HighPercentage
    unsigned char                                  high_range;                          // HighRange

    /******************************************************************************
    Luma Adaptation  (6K)
    ******************************************************************************/
    unsigned char  la_threshold;

    /******************************************************************************
    Luma Adaptation  (8K)
    ******************************************************************************/
    tuning_control_type					 control_la;// 205 default 0 for lux_index  //new205 mods
    trigger_point_type                     la_brightlight_trigger;
    int la_8k_enable; //0x207 default false
    la_8k_type   la_8k_config;  //this is for indoor
    la_8k_type   la_8k_config_outdoor; //new205 mods

    float la_luma_target_adj; //0x205 mods default 0.9. we reduce luma target when LA is on. This parameter is for 7k,6k,8k.

    //luma adaptation for viewfinder  0x206
    tuning_control_type					 control_la_VF; //0x206
    trigger_point_type                     la_brightlight_trigger_VF;

    la_8k_type   la_8k_config_indoor_VF;  //0x206
    la_8k_type   la_8k_config_outdoor_VF;
    float la_luma_target_adj_VF;

    /******************************************************************************
    Chroma Suppression  (ChromaSupp) & MCE (memory color enhancement)
    ******************************************************************************/

    tuning_control_type					 control_cs;//default 0 for lux_index

    trigger_point_type                     cs_snapshot_lowlight_trigger;

    cs_luma_threshold_type                 cs_luma_threshold_snapshot_lowlight;

    cs_luma_threshold_type                 cs_luma_threshold_snapshot;

    trigger_point_type                     cs_lowlight_trigger;

    cs_luma_threshold_type                 cs_luma_threshold_lowlight;

    cs_luma_threshold_type                 cs_luma_threshold;

    mce_type                               mce_config;  //205

    /******************************************************************************
    Hand Jittering Reduction  (HJR)
    ******************************************************************************/
    unsigned char                                  max_number_of_frames;                // MaxFrames
    // removed in 0x206
    //unsigned char                                  one_to_two_frame_hjr_offset;         // 1to2FrameOffset
    unsigned char                                  flat_area_noise_reduction_level;     // FlatAreaNoise
    unsigned char                                  texture_noise_reduction_level;       // TextureNoise
    unsigned char                                  texture_threshold;                   // TextureThresh
    unsigned char                                  hjr_k_table[HJR_K_TABLE_SIZE];       // KTable
    unsigned char                                  bayer_filter_enable_index;           // BayerFilterEnable


    /******************************************************************************
    Adaptive Bayer Filter     (ABF)  7K + old 8K
    ******************************************************************************/
    tuning_control_type             control_ABF; //default 1
    trigger_point_type			  abf_shift_lowlight_trigger_snapshot;  //205 mods
    chromatix_adaptive_bayer_filter_data_type    adaptive_bayer_filter_data;    // SnapshotFilterData
    chromatix_adaptive_bayer_filter_data_type    adaptive_bayer_filter_data_preview;    // PreviewFilterData
    trigger_point_type			  abf_shift_lowlight_trigger_preview;  //205 mods

    unsigned char   bayer_filter_enable_lux_index; //default 0
    unsigned char   bayer_filter_enable_lux_index_snapshot; //default 0
    trigger_point_type  index_ABF; // The start and end points are used for dividing 20 entries
    trigger_point_type  index_ABF_snapshot; // The start and end points are used for dividing 20 entries

    /******************************************************************************
    Adaptive Bayer Filter 2    (ABF2)  new 8K, 7x30, 8x55 and later chips
    ******************************************************************************/
    chromatix_adaptive_bayer_filter_data_type2  abf2_config_low_light_preview;
    chromatix_adaptive_bayer_filter_data_type2  abf2_config_normal_light_preview;
    chromatix_adaptive_bayer_filter_data_type2  abf2_config_bright_light_preview;
    trigger_point_type  abf2_low_light_trigger_preview;
    trigger_point_type  abf2_bright_light_trigger_preview;

    chromatix_adaptive_bayer_filter_data_type2  abf2_config_low_light_snapshot;
    chromatix_adaptive_bayer_filter_data_type2  abf2_config_normal_light_snapshot;
    chromatix_adaptive_bayer_filter_data_type2  abf2_config_bright_light_snapshot;
    trigger_point_type  abf2_low_light_trigger_snapshot;
    trigger_point_type  abf2_bright_light_trigger_snapshot;


    /******************************************************************************
    Auto Flickering Detection  (AFD)
    ******************************************************************************/
    chromatix_auto_flicker_detection_data_type   auto_flicker_detection_data;   // DetectionData
    chromatix_rscs_stat_config_type  auto_flicker_detection_config;  // new 8K only

    // 0x206 AFD
    unsigned char AFD_num_peaks_threshold; //default 6
    float AFD_INTL_adj_factor; //0 to 0.45, default 0.25,how much INTL has to be away from band gap.


    /******************************************************************************
    Bad Pixel Correction
    ******************************************************************************/
    //204
    unsigned char                              bpc_Fmin_preview;
    unsigned char                              bpc_Fmax_preview;
    unsigned char                              bpc_Fmin_preview_lowlight;
    unsigned char                              bpc_Fmax_preview_lowlight;
    unsigned char                              bpc_Fmin_snapshot;
    unsigned char                              bpc_Fmax_snapshot;
    unsigned char                              bpc_Fmin_snapshot_lowlight;
    unsigned char                              bpc_Fmax_snapshot_lowlight;
    tuning_control_type     control_bpc; //default 0

    trigger_point_type          bpc_snapshot_lowlight_trigger;
    bpc_diff_threshold_type     bpc_diff_threshold_snapshot[BPC_MAX_LIGHT]; //same as VF

    trigger_point_type          bpc_lowlight_trigger;
    bpc_diff_threshold_type     bpc_diff_threshold[BPC_MAX_LIGHT];

    //0x206 for 7x30 and 8x60
    //removed this for 0x207, replaced with 4 channel new type
    //  bpc_offset_type     bpc_offset_snapshot[BPC_MAX_LIGHT];
    //  bpc_offset_type     bpc_offset[BPC_MAX_LIGHT];

    bpc_4_offset_type     bpc_4_offset_snapshot[BPC_MAX_LIGHT];
    bpc_4_offset_type     bpc_4_offset[BPC_MAX_LIGHT];


    // 0x207 bad pixel cluster params
    tuning_control_type     control_bcc; //default 0
    trigger_point_type          bcc_snapshot_lowlight_trigger;
    trigger_point_type          bcc_lowlight_trigger;

    unsigned char                              bcc_Fmin_preview;
    unsigned char                              bcc_Fmax_preview;
    unsigned char                              bcc_Fmin_preview_lowlight;
    unsigned char                              bcc_Fmax_preview_lowlight;
    unsigned char                              bcc_Fmin_snapshot;
    unsigned char                              bcc_Fmax_snapshot;
    unsigned char                              bcc_Fmin_snapshot_lowlight;
    unsigned char                              bcc_Fmax_snapshot_lowlight;


    bpc_4_offset_type     bcc_4_offset_snapshot[BPC_MAX_LIGHT];
    bpc_4_offset_type     bcc_4_offset[BPC_MAX_LIGHT];

    //calibration table does not need to be in chromatix header, it handles by SW directly to read from EEPROM per factory on-module cal
    //  abcc_table_type  abcc_table_preview;
    //  abcc_table type  abcc_table_snapshot;
    //0x207 demosaic3
    tuning_control_type control_demosaic3_preview;
    trigger_point_type demosaic3_trigger_lowlight_preview;
    demosaic3_LUT_type demosaic3_LUT_preview;

    float demosaic3_aG_preview[2]; //2 light condition,0 is lowlight, 1 is normal light
    float demosaic3_bL_preview[2]; //2 light condition

    tuning_control_type control_demosaic3_snapshot;
    trigger_point_type demosaic3_trigger_lowlight_snapshot;
    demosaic3_LUT_type demosaic3_LUT_snapshot;
    float demosaic3_aG_snapshot[2]; //2 light condition,0 is lowlight, 1 is normal light
    float demosaic3_bL_snapshot[2]; //2 light condition

    //0x207 chroma filter
    tuning_control_type control_chroma_filter_preview;
    trigger_point_type chroma_filter_trigger_lowlight_preview;
    Chroma_filter_type chroma_filter_preview[2]; //2 light condition
    tuning_control_type control_chroma_filter_snapshot;
    trigger_point_type chroma_filter_trigger_lowlight_snapshot;
    Chroma_filter_type chroma_filter_snapshot[2]; //2 light condition, 0 is lowlight, 1 is normal light

    //0x207 luma filter , same struct as ABF
    chromatix_adaptive_bayer_filter_data_type2  LF_config_low_light_preview;
    chromatix_adaptive_bayer_filter_data_type2  LF_config_normal_light_preview;
    chromatix_adaptive_bayer_filter_data_type2  LF_config_bright_light_preview;
    trigger_point_type  LF_low_light_trigger_preview;
    trigger_point_type  LF_bright_light_trigger_preview;

    chromatix_adaptive_bayer_filter_data_type2  LF_config_low_light_snapshot;
    chromatix_adaptive_bayer_filter_data_type2  LF_config_normal_light_snapshot;
    chromatix_adaptive_bayer_filter_data_type2  LF_config_bright_light_snapshot;
    trigger_point_type  LF_low_light_trigger_snapshot;
    trigger_point_type  LF_bright_light_trigger_snapshot;

    //0x207 wavelet, for snapshot only
    ReferenceNoiseProfile_type noise_profile[6];
    float denoise_scale;
    float ref_gain; //gain used when calibration was conducted
	int wavelet_enable_index;

    //0x207 linearization, only 1 set, if different blk is needed for VF and snapshot, use the chromatix blk data and modify this table
    tuning_control_type control_linearization;
    trigger_point_type linearization_bright_trigger;

    chromatix_CCT_trigger_type linear_A_trigger;
    chromatix_CCT_trigger_type linear_D65_trigger;

    chromatix_linearization_type linear_table_A_bright;
    chromatix_linearization_type linear_table_A_normal;
    chromatix_linearization_type linear_table_TL84_bright;
    chromatix_linearization_type linear_table_TL84_normal;
    chromatix_linearization_type linear_table_Day_bright;
    chromatix_linearization_type linear_table_Day_normal;




    /******************************************************************************
    Skin Tone Detection (Skintone)
    ******************************************************************************/

    float      skintone_Hmin;  //1.0 to 3.0, default 1.5
    float      skintone_Hmax;  //0.0 to 1.0, default 0.5
    float      skintone_Ymin;  //0.0 to 0.3, default 0.1
    float      skintone_Ymax;  //0.7 to 1.0, default 0.9
    float      skintone_S_HY_min;  //0.0 to 0.4, default 0.05
    float      skintone_S_HY_max;  //0.1 to 0.5, default 0.25
    float      skintone_S_LY_min;  //0.0 to 0.5, default 0.25
    float      skintone_S_LY_max;  //0.2 to 1.0, default 0.6
    unsigned char      skintone_percentage;  // threshold percentage (0 to 100), output=1 when skin pixels exceed this percentage



    /******************************************************************************
    AFR
    ******************************************************************************/
    //0x207 removed
    //  afr_type afr[AFR_TABLE_SIZE];

    /******************************************************************************
    Demosaic     (Demosaic)
    ******************************************************************************/
    unsigned char demosaic_slope_shift; //default 0

    /******************************************************************************
    skin color enhancement (SCE)
    ******************************************************************************/
    int                   sce_enable;
    sce_cr_cb_triangle_set    origin_triangles_TL84; //existing set is for TL84
    sce_cr_cb_triangle_set    destination_triangles_TL84;
    sce_affine_transform_2d   outside_region_mapping;
    //0x207
    tuning_control_type control_SCE;
        trigger_point_type SCE_trigger_point;   //under lowlight reduce SCE mapping to none
    //for other illuminants
    sce_cr_cb_triangle_set    origin_triangles_A;
    sce_cr_cb_triangle_set    destination_triangles_A;
    sce_cr_cb_triangle_set    origin_triangles_D65;
    sce_cr_cb_triangle_set    destination_triangles_D65;

    chromatix_CCT_trigger_type SCE_A_trigger;
    chromatix_CCT_trigger_type SCE_D65_trigger;

    //0x206
    unsigned short min_snapshot_resolution_x; //8k WM
    unsigned short min_snapshot_resolution_y;  //8k WM

    scene_change_detect_type scene_change_detect;
    snow_scene_detect_type snow_scene_detect; //added aggressiveness in 0x207
    backlit_scene_detect_type backlit_scene_detect; //added aggressiveness in 0x207, added max_percent_threshold in 0x208
	red_eye_reduction_type red_eye_reduction; // added in 0x0207, added more enable params in 0x208

	//0x208
	landscape_scene_detect_type landscape_scene_detect; //0x208
	portrait_scene_detect_type portrait_scene_detect;

	//ZSL
	unsigned short default_shutter_lag;
	unsigned short max_Q_factor_reduction;

    /**************************************************************************
    Slope-predictive auto-focus algorithm parameters
    **************************************************************************/
    AF_slope_predictive_type AF_slope_predictive;

    /**************************************************************************
     * AWB Low Light Look Up Table
    **************************************************************************/
    awb_lowlight_adj_lut_type AWB_lowlight_LUT;

    mesh_rolloff_array_type                chromatix_rolloff_goldenmod;
} chromatix_parms_type;







#endif /* CHROMATIX_7500_H */
