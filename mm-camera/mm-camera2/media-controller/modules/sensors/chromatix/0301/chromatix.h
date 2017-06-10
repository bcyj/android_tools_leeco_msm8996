/*============================================================================

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CHROMATIX_H
#define CHROMATIX_H
/*============================================================================
                        CONSTANTS
============================================================================*/

#define CHROMATIX_VERSION 0x301

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

/*****************************************/
//0x207  trigger pt for CCT
/*****************************************/
typedef struct{
    unsigned long CCT_start;
    unsigned long CCT_end;
} chromatix_CCT_trigger_type;

/******************************************************************************
    VFE basic struct
    ******************************************************************************/

/******************************************************************************
Bad pixel correction data types
******************************************************************************/
typedef enum
{
    LOW_LIGHT = 0,
    NORMAL_LIGHT,
    LOWLIGHT_MAX_LIGHT
} lowlight_type;

typedef enum
{
    BPC_NORMAL_LIGHT =  0,
    BPC_LOW_LIGHT,
    BPC_MAX_LIGHT
} bpc_light_type;

typedef struct
{
    unsigned short even_columns;                // EvenCols
    unsigned short odd_columns;                 // OddCols
} chromatix_black_level_offset_type;

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
5x5 ASF data types
******************************************************************************/
/* 5 x 5 Adaptive Spatial Filter.
* There are two components in this filter
* High Pass Filter (HPF): 5 x 5
* Low Pass Filter (LPF): 3 x 3
* HPF can use upto two 5 x 5 filters. The sensor driver chooses to use
* both or single filter.
* LPF can be implemented in H/W Luma Filter module .
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

//remove from 0x300
//typedef char   smoothing_filter_type [SMOOTHING_FILTER_SIZE];
//typedef char   laplacian_filter_type [LAPLACIAN_FILTER_SIZE];
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
 //   smoothing_filter_type smoothing_filter;           // SmoothFltr
    /* LPF: 3x3 laplacian filter used to calculate luma filter */
 //   laplacian_filter_type laplacian_filter;           // LapFltr
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

typedef enum
{
    ASF_7x7_LOW_LIGHT = 0,
    ASF_7x7_NORMAL_LIGHT,
    ASF_7x7_BRIGHT_LIGHT,
    ASF_7x7_MAX_LIGHT
} ASF_7x7_light_type;

typedef struct
{
    // ASF on/off flag
    unsigned char asf_en;

    //*** VFE4 ***
    // 3x3 cross-type median filter
    int sp[ASF_7x7_MAX_LIGHT];           // 3x3 smooth percentage, [0, 16], Q4

    // Special effects
    unsigned char en_sp_eff;
    unsigned char  neg_abs_y1;
    int nz[3];  //[0] for F1, [1] for F2, [2] for F3 and F4.       // Order: nz[7]nz[6]nz[5]nz[4]nz[3]nz[2]nz[1]nz[0]

    // 7x7 filters
    // The last line is not median filtered to save 1 line buffer.
    // Alternated kernel symmatry by (negation & zero)'s according to zones:
    // nz[0]          |nz[1]|          nz[2]
    //        0  1  2 |   3 |  2  1  0
    //        4  5  6 |   7 |  6  5  4
    //        8  9 10 |  11 | 10  9  8
    // ---------------+-----+---------------
    // nz[7] 12 13 14 |  15 | 14 13 12 nz[3]
    // ---------------+-----+---------------
    //        8  9 10 |  11 | 10  9  8
    //        4  5  6 |   7 |  6  5  4
    //        0  1  2 |   3 |  2  1  0
    // nz[6]          |nz[5]|          nz[4]
    //
    float  f1[ASF_7x7_MAX_LIGHT][16];         // 7x7 H Sobel filter, 10sQ10 in quadrants, 11sQ10 in axes, 12sQ10 at center
    float  f2[ASF_7x7_MAX_LIGHT][16];         // 7x7 V Sobel filter, 10sQ10 in quadrants, 11sQ10 in axes, 12sQ10 at center
    float  f3[ASF_7x7_MAX_LIGHT][16];         // 7x7 H Wiener HPF, 10sQ10 in quadrants, 11sQ10 in axes, 12sQ10 at center
    float  f4[ASF_7x7_MAX_LIGHT][16];         // 7x7 V Wiener HPF, 10sQ10 in quadrants, 11sQ10 in axes, 12sQ10 at center
    float  f5[16];         // 7x7 Gaussian LPF, 10sQ10 in quadrants, 11sQ10 in axes, 12sQ10 at center

    // H & V sharpening LUTs, 24 entries
    float  lut1[ASF_7x7_MAX_LIGHT][24]; // Linear interpolated 16s Q6/Q5 entries
    float  lut2[ASF_7x7_MAX_LIGHT][24]; // Linear interpolated 16s Q6/Q5 entries

    // Dynamic clamping
    int en_dyna_clamp[ASF_7x7_MAX_LIGHT];
    float smax[ASF_7x7_MAX_LIGHT];         // Dynamic clamping positive level scaling, 7uQ4
    int omax[ASF_7x7_MAX_LIGHT];         // Dynamic clamping positive offset level, 7u
    float smin[ASF_7x7_MAX_LIGHT];         // Dynamic clamping negative level scaling, 7uQ4
    int omin[ASF_7x7_MAX_LIGHT];         // Dynamic clamping negative offset level, 7u
    int  reg_hh[ASF_7x7_MAX_LIGHT]; // Manual fixed H positive clamping level, 9s
    int  reg_hl[ASF_7x7_MAX_LIGHT]; // Manual fixed H negative clamping level, 9s
    int  reg_vh[ASF_7x7_MAX_LIGHT]; // Manual fixed V positive clamping level, 9s
    int  reg_vl[ASF_7x7_MAX_LIGHT]; // Manual fixed V negative clamping level, 9s

    // LP & HP output combination LUT, 12 entries
    float  lut3[ASF_7x7_MAX_LIGHT][12]; // Linear interpolated 15s Q8/Q6 entries

} chromatix_asf_7_7_type;

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



/******************************************************************************
Gamma
******************************************************************************/

#define GAMMA_TABLE_SIZE           64
typedef struct {
    /* 8-bit, Q0, unsigned */
    unsigned char gamma[GAMMA_TABLE_SIZE];    // Gamma
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

typedef struct
{
    unsigned short threshold[3];  // 12-bit pixels
    float scale_factor[2];

    float table_pos[16];
    float table_neg[8];
} chromatix_luma_filter_type;


/******************************************************************************
Auto Flicker Detection data types
******************************************************************************/
typedef struct
{
	int AFD_continuous_enable; //0x301
    float  std_threshold;                             // StdThresh
    unsigned char  percent_threshold;                         // PctThresh
    unsigned long diff_threshold;                            // DiffThresh
    unsigned long frame_ct_threshold;                        // FrameCtThresh
    unsigned char  num_frames;                                //204,  default 6
    unsigned char  frame_skip;                                //204 , default 1
    unsigned long num_rows;                                  //204 , default 480
    unsigned char  num_frames_settle;							//205, default 3
// 0x206 AFD
    unsigned char num_peaks_threshold; //default 6
    float INTL_adj_factor; //0 to 0.45, default 0.25,how much INTL has to be away from band gap.

} chromatix_auto_flicker_detection_data_type;



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
    Bayer histogram config
 ******************************************************************************/

typedef struct
{

    float       horizontalOffsetRatio;
    float       verticalOffsetRatio;
    float       horizontalWindowRatio;
    float       verticalWindowRatio;

} bayer_histogram_config_type;

 /******************************************************************************
    Bayer grid config
 ******************************************************************************/
typedef struct
{
    unsigned char saturation_thresh_R;
    unsigned char saturation_thresh_Gr;
    unsigned char saturation_thresh_Gb;
    unsigned char saturation_thresh_B;
    float       horizontalOffsetRatio;
    float       verticalOffsetRatio;
    float       horizontalWindowRatio;
    float       verticalWindowRatio;
    unsigned short num_H_regions;
    unsigned short num_V_regions;

} bayer_grid_config_type;

    /******************************************************************************
    LA Struct definition
 ******************************************************************************/
typedef struct
{
    /**
       float offset; //iHist offset added to iHist.  Suggested from 0.0 to
        //16.0.  Reference value: 0.0.  Note: Increasing offset reduces LA strength.
    float lad_cfg.backlight; // iHist enhancement on the dark end.  Suggested from 0.0 to 10.0.  Reference value: 0.0.
        //Note: Increasing backlight boosts up low light.
    float frontlight;// iHist enhancement on the bright end.  Suggested from 0.0 to 1.0.  Reference value: 0.0.
        //CAUTION: frontlight > 0  may cause contour/banding artifacts!!
    float cap_adjust; // Gaussian cap curve width adjustment,  default = 3.6.
    float cap_orig; // Gaussian cap curve maximum above 1.0 in low light,  default: 1.5.
    unsigned char CDF_50_threshold; // CDF[50] threshold to adjust cap curve  maximum for low light, normal: 100, backlight: 70.
    float cap_ratio; //Rate of increase of cap curve maximum if CDF[50] > CDF_50_threshold, normal: 0.05, backlight: 0.25.
    float cap_max; // Upper bound of cap curve maximum,  normal: 3, backlight: 12.
***/

    unsigned char shadow_range; // 0 to 127, default 70
    unsigned char shadow_boost_allowance; //0 to 100 , default 10
    unsigned char LA_reduction_fine_tune;  //0 to 100, default 0
    unsigned char highlight_suppress_fine_tune;// 0 to 100 , default 0
    unsigned char shadow_boost_fine_tune; //0 to 100, default 0
} LA_args_type;


//0x207
typedef enum
{
    FACE_PRIORITY_CENTER=0,  // default
    FACE_PRIORITY_BIG,       // big face
    FACE_MAX_PRIORITY,
    FACE_INVALID_PRIORITY=FACE_MAX_PRIORITY
} chromatix_face_priority_type;
//0x207

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

//wavelet denoise, updated in 0x301
#define NUM_NOISE_PROFILE 24
#define WAVELET_LEVEL 4
typedef struct
{
    float trigger_value;
    float referenceNoiseProfileData[NUM_NOISE_PROFILE];// default0
    float denoise_scale_y[WAVELET_LEVEL];
    float denoise_scale_chroma[WAVELET_LEVEL];
    float denoise_edge_softness_y[WAVELET_LEVEL];
    float denoise_edge_softness_chroma[WAVELET_LEVEL];
    float denoise_weight_y[WAVELET_LEVEL];
    float denoise_weight_chroma[WAVELET_LEVEL];
    int sw_denoise_edge_threshold_y;
    int sw_denoise_edge_threshold_chroma;
} ReferenceNoiseProfile_type;


#define NUM_GRAY_PATCHES 6
typedef struct
{
    int wavelet_enable_index;
    tuning_control_type control_denoise;
    ReferenceNoiseProfile_type noise_profile[NUM_GRAY_PATCHES];
} wavelet_denoise_type;

//tempral denoise 0x301

typedef struct
{
	float y;
	float cb;
	float cr;
}temporal_noise_profile_type;

typedef struct
{
    float trigger_value;
    temporal_noise_profile_type referenceNoiseProfileData;  // default 0
    float temporal_denoise_scale_y;
    float temporal_denoise_scale_chroma;
} reference_temporal_noise_profile_type;

typedef struct
{
    //3A 2.0
    //scene change detection I-frame trigger

    int enable;// 0: scene change detection feature is disabled; 1: scene change detection feature is enabled.
    float aec_para_mean; // this parameter is used to control the contribution to the dynamic threshold from the mean value of the latest scd_aec_dy_window frames? SADs (sum of the absolute difference). The default value is: 2. The range of this parameter is [1, 4].
    float aec_para_std; // this parameter is used to control the contribution to the dynamic threshold from the standard deviation of the latest scd_aec_dy_window frames? SADs (sum of the absolute difference). The default value is: 2.76. The range of this parameter is [1, 10].
    unsigned short aec_dy_window;// this value is used to control the window size (how many frames) for computing the mean and the standard deviation. The default value is: 15 frames. The range of this value is [1, maximum frame rate that can be achieved]
    unsigned short threshold_aec_lux; // this parameter is a predefined threshold in AEC scene change detection lux index method. The default value is: 10. The range of this threshold is [10, 100].
    float af_range_threshold;// used to control the threshold of scene change detection in auto focus. Range of this parameter is [0, 1]. Default value is 0.5. The higher the parameter, the higher the thresholds for scene change detection in auto focus.
} scene_change_detect_type;

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
} snow_scene_detect_type;

typedef struct
{
    //Backlit Scene Detection

    int backlight_detection_enable; //TRUE enables the feature from AEC perspective, FALSE disables the feature.
    unsigned long low_luma_threshold; // Histogram samples which luma is below this threshold are added to low luma count.
    unsigned long high_luma_threshold; // Histogram samples which luma is above this threshold are added to high luma count.
    float low_luma_count_percent_threshold ; // If count of low luma samples exceed this percentage of total samples, we consider potential backlight case.
    float high_luma_count_percent_threshold ;// If count of high luma samples exceed this percentage of total samples, we consider potential backlight case.
    LA_args_type backlight_la_8k_config; // Luma Adaptation configuration when backlight scene is detected, compensation portion.
    int backlight_max_la_luma_target_offset; // Maximum luma target adjustment when backlight is detected.   We expect to increase luma target.
    float backlit_aggressiveness; //added in 0x207
    float	max_percent_threshold; //added in 0x208, defines interpolation range of backlit severity in 		//histogram detector.
    unsigned char ui_backlit_display_th;//added in 0x208
} backlit_scene_detect_type;


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
} red_eye_reduction_type;

//0x208, mod in 0x301
/******************************************************************************
// feature name: Landscape Scene Detection and Compensation

// ****************************************************************************/
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
  float asd_ext_blue_th; //ADDED 0X301  //extreme blue region detection
  float asd_ext_green_th_r; //ADDED 0X301 //extreme green region detection
  float asd_ext_green_th_b;  //ADDED 0X301 //extreme green region detection
  float aggressiveness;  //response of temporal filter to severity change
  long lux_idx_indoor;  //lux_idx above this threshold disabled landscape detection
  long lux_idx_outdoor; //lux idx below this threshold has full application of landscape scene compensation.  Between indoor and outdoor, we interpolate severity.
  unsigned char ui_landscape_display_th;
} landscape_scene_detect_type;


typedef struct
{
  int portrait_detection_enable;
  float skin_color_boost_factor;
  float min_face_content_threshold;
  float max_face_content_threshold;
  filter_sharpen_degree_type  soft_focus_degree_7_7;
  filter_sharpen_degree_type  soft_focus_degree_5_5;
  float aggressiveness;
  unsigned char ui_portrait_display_th;
} portrait_scene_detect_type;

/******************************************************************************
    AWB basic struct
    ******************************************************************************/
/******************************************************************************
AWB data types
******************************************************************************/
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
} AWB_LOWLIGHT_type;

typedef enum
{
    AGW_AWB_D65 =  0,  // D65
    AGW_AWB_D75,         // D75
    AGW_AWB_A,    // A
    AGW_AWB_WARM_FLO,        // TL84
    AGW_AWB_COLD_FLO,        // CW
    AGW_AWB_HORIZON,                // H
    AGW_AWB_D50,      // D50
    AGW_AWB_CUSTOM_FLO,      // CustFlo
    AGW_AWB_NOON,           // Noon
    AGW_AWB_CUSTOM_DAYLIGHT,
    AGW_AWB_CUSTOM_A,
    AGW_AWB_U30,
    AGW_AWB_MAX_LIGHT,
    AGW_AWB_INVALID_LIGHT = AGW_AWB_MAX_LIGHT,
    DAY_LINE_1 = AGW_AWB_MAX_LIGHT,
    DAY_LINE_2,
    FLINE,
    A_LINE_1,
    A_LINE_2,
    AGW_AWB_HYBRID,                 // Daylight, only used for algorithm, not data
    AGW_AWB_MAX_ALL_LIGHT = AGW_AWB_HYBRID, // Don't count the hybrid
    AGW_AWB_INVALID_ALL_LIGHT = AGW_AWB_MAX_ALL_LIGHT
} chromatix_awb_all_light_type;

typedef struct
{
    /* converted to Q7 in the code */
    float green_even;                // GreenEvenRow
    float green_odd;                 // GreenOddRow
    float red;                       // Red
    float blue;                      // Blue
} chromatix_channel_balance_gains_type;

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


typedef struct
{
    float RG_ratio;                   // RedG
    float BG_ratio;                  // BlueG
} chromatix_awb_reference_type;

typedef struct
{
    float red_gain_adj;               // RedAdj
    float blue_gain_adj;              // BlueAdj
} chromatix_awb_gain_adj_type;

typedef struct
{
    //AWB
    int AWB_purple_prevent_enable; //0x207, enable the feature
    tuning_control_type     control_purple_prevent;//default 0 for lux_index
    trigger_point_type      purple_prevent_trigger;
    float purple_sky_prevention_bg_threshold ;//range from 0 to 2.0, default is the BG ratio of D50
} AWB_purple_prevent_type;

//0x207
/************************************************************************
AWB motion sensor type
************************************************************************/
typedef struct
{
    float gyro_trigger ;// input above trigger to detect change
    float accelerometer_trigger; // input above trigger to detect change
    float magnetometer_trigger; // input above trigger to detect change
    float DIS_motion_vector_trigger; // input above trigger to detect change
} AWB_motion_sensor_type;

#define AGW_NUMBER_GRID_POINT             241

typedef struct
{
    int lux_index;
    float green_rg_offset_adj;
    float green_bg_offset_adj;
    float outlier_dist_adj;
} low_light_adj_type;

#define MAX_LOW_LIGHT_AWB_LUT_SIZE 6
typedef struct
{
    unsigned char enable;
    low_light_adj_type  lut_entry[MAX_LOW_LIGHT_AWB_LUT_SIZE];
} awb_lowlight_adj_lut_type;


/******************************************************************************/
// Feature name : Match LED Level
// Applicale chipset: MSM8x30 and later.
// Applicable operation mode:  For video and snapshot.
//
// This is the feature to match dual LED flash with the ambient lighting
//
// Desription of variables in chromatix_match_LED_lighting_table
// structure.
//
// Variable names: table_size, CCT_control[table_size-1]
// The three values for the table entries
// The color temperature
// The LED1 current configuration for LED controller IC
// The LED1 current configuration for LED controller IC
/******************************************************************************/
#define MAX_LED_MIX_LEVEL 16

typedef struct
{
    unsigned short CCT;
    unsigned short LED1_setting; //the controller IC LED1 current config
    unsigned short LED2_setting; // the controller IC LED2 config
} LED_mix_type;

typedef struct
{
    // table size
    unsigned short table_size;  //depends on the dual LED controller IC

    LED_mix_type CCT_control[MAX_LED_MIX_LEVEL];

} chromatix_match_LED_lighting_table_type;

/******************************************************************************
    AEC basic structures
******************************************************************************/
/******************************************************************************
exposure table data types
******************************************************************************/

typedef struct
{
    unsigned short gain;                        // Gain
    unsigned long line_count;                  // LineCt
} exposure_entry_type;


typedef struct
{
    unsigned char exp_table_flag;               // ExpTableFlag, 205 (removed), keep it for SW backward compatibility , marked as unused.
    unsigned short valid_entries;               // ValidEntries
    int aec_enable_digital_gain_for_EV_lowlight;    // 0x301 if this is one, the extended table will be used – default 1
	unsigned short exp_index_for_digital_gain_for_EV_lowlight;  // 0x301, where in the table

    exposure_entry_type exposure_entries[MAX_EXPOSURE_TABLE_SIZE]; // ExposureEntries
} chromatix_exposure_table_type;

 typedef struct
{
    int                                     is_supported;
    float                                   reduction;
    unsigned long                           threshold_LO;
    unsigned long						    lux_index_LO;  //204
    unsigned long                           threshold_HI;
    unsigned long                           lux_index_HI;  //204
    float                                   discard_ratio;
} aec_outdoor_bright_region_type;

typedef struct
{
    int                                    is_supported;
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
} snapshot_exposure_type;

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

} AEC_strobe_flash_type;

typedef struct
{
    //AEC
    //Touch AEC  3A 2.2
    int touch_roi_enable; //TRUE enables the feature from AEC perspective, FALSE disables the feature.
    float touch_roi_weight; //Combines the ?touched area luma? and the overall AEC metered luma.

} AEC_touch_type;

typedef struct
{
    //Face priority AEC, 3A 2.2

    int aec_face_enable; //TRUE enables the feature from AEC perspective, FALSE disables the feature.
    float aec_face_weight; //0 to 1, determines how much face area contributes for frame luma calculation.  1 means all influence comes from face area.
    float dark_skin_ratio; //skin luma vs. white point luma ratio if close to this value then we set AEC as dark skin.
    float light_skin_ratio; //skin luma vs. white point luma ratio if close to this value then we set AEC as light skin.
    long dark_skin_luma_target; //if dark skin detected, this is luma target is used for AEC (post UI EV update)
    long light_skin_luma_target; //if light skin detected, this is luma target is used for AEC (post UI EV update)

} AEC_face_priority_type;

/******************************************************************************
    AEC motion sensor data types
******************************************************************************/
/*
    typedef struct{
        float aec_gyro_trigger;  //input above trigger to detect change
        float aec_accelerometer_trigger;  //input above trigger to detect change
        float aec_magnetometer_trigger ; //input above trigger to detect change
        float aec_DIS_motion_vector_trigger ;// input above trigger to detect change
    }AEC_motion_sensor_type;
*/

typedef struct
{
    int pixel_value;
    float weight;
} hist_weight_type;

//modified in 0x301

/******************************************************************************
    feature name: Bayer Grid Stat Histogram Flat Detector

 ******************************************************************************/
typedef struct
{
    //histogram stat use for flatness indicator
    int hist_flat_detector_enable; //ADDED 0X301
    int start_level;  //pixel level to start checking
    int end_level;//pixel level when to stop checking
    int range; //pixel delta to check
    float delta_th; //if pixel delta over range is larger than this threshold, we call the image has enough flat area.
    float bright_flat_det_th; //ADDED 0X301
    float dark_flat_det_th; //ADDED 0X301
    float bright_flat_tail_det; //ADDED 0X301
    float dark_flat_tail_det; //ADDED 0X301
} AE_hist_flat_detector_type;


#define HIST_WEIGHT_SIZE 20 //MODIFIED FOR 0X301

/******************************************************************************
   feature name: Bayer Stat AEC

*******************************************************************************/
typedef struct
{
    //spatial BG metering
    int bright_region_influence_adj_enable; //ADDED 0X301 //enables and disables bright region luma adjustments
    int bright_weight_lux_idx_trigger;// = 300; //if lux_idx > 300, temp_bright_weight = 1.0, i.e. not effect.
    float bright_weight_def;// = 0.25; //weight for brightness saturated regions add to chromatix bayer AEC
    int bright_level ;//= 220; //add to chormatix, regions above this level are considered bright.

    int color_based_metering_enable;  //ADDED 0X301 //enables and disables all color options from AEC metering
    float col_sat_weight;// = 1.0; // weight for color saturated regions. add to chromatix bayer AEC
    float color_luma_decrease_ratio;// = 2.0; //add to chromatix bayer AEC
    //extreme color detector threshold
    //ratios after WB, therefore remove WB gain for raw bayer stat comp
    float rg_ratio_lth; // = 0.85/(r_wb_gain/g_wb_gain);//for green  //add to chromatix bayer AEC
    float rg_ratio_hth; // = 2.2/(r_wb_gain/g_wb_gain); //for extreme red add to chromatix bayer AEC
    float bg_ratio_lth; // = 0.6/(b_wb_gain/g_wb_gain); //add to chromatix bayer AEC
    float bg_ratio_hth; // = 1.8/(b_wb_gain/g_wb_gain); //for extreme blue add to chromatix bayer AE

    //large near grey flat area handling
    float flat_white_grey_vs_nongrey_th;//to determine if flat area is brighter than colored regions: if (near_gr_luma > 1.1 * non_gr_luma) make brighter
    float flat_dark_grey_vs_nongrey_th;  //to determine if flat area is darker than colored regions: if (near_gr_luma < 0.5*non_gr_luma) make darker

    //hist_weight_type hist_weight_table_dark[HIST_WEIGHT_SIZE]; //REMOVED 0X301 //pixel bin weighting for dark pixels
    //hist_weight_type hist_weight_table_bright[HIST_WEIGHT_SIZE]; //REMOVED 0X301 //pixel bin weighting for bright pixels
    //MODIFIED FOR 0X301 ==> change from two table to single table.
    int hist_ent_enable; //ADDED 0X301  //enables and disabled histogram entropy LT offset
    hist_weight_type hist_weight_table[HIST_WEIGHT_SIZE]; //ADDED 0X301  //pixel bin weighting for histogram

    //int luma_target_reset_th; //REMOVED 0X301 //exp_index delta that will zero out entropy luma target adjustment
    float luma_target_reset_ratio; //ADDED 0X301 //this value is still used to reset entropy LT offset but exp_index tolerance is now dynamically calculated.
    float hist_luma_target_adjusment_cap; //factor, 2 is LT can be doubled, and LT can be halved (EV-1 to EV+1).  4 is EV-2 to EV+2   0 disables the feature.

    //near grey check used for light and dark backgrounds, example white board or black board.
    float near_grey_tolerance; //0.05;  //0.95 to 1.05 = 5% tol. Regions with R/G and B/G within 5% of grey are considered grey regions.
    AE_hist_flat_detector_type hist_flat_detect;
    float bright_flat_compensation_ratio;//ADDED 0X301 //  from tf = tf*0.9+0.6*0.1;
    float dark_flat_compensation_ratio;  //ADDED 0X301 //from tf = tf*0.9+1.8*0.1;

} AEC_bayer_stats_algo_params_type;


typedef struct
{
    float 	exposure_time;
    float	  no_shake_threshold;
    float	  moderate_shake_threshold;
} shake_table_type ;

#define MAX_SHAKE_LUT_SIZE 10
typedef struct
{
    int enable;
    unsigned char valid_entries;
    shake_table_type shake_table[MAX_SHAKE_LUT_SIZE];
} shake_detection_type;

/******************************************************************************
    ASD struct
******************************************************************************/
typedef struct
{
    int ASD_Software_Type; // 0 means hybrid, 1 means using bayer stats, for UA code only
    scene_change_detect_type scene_change_detect;
    snow_scene_detect_type snow_scene_detect; //added aggressiveness in 0x207
    backlit_scene_detect_type backlit_scene_detect; //added aggressiveness in 0x207, added max_percent_threshold in 0x208

    //0x208
    landscape_scene_detect_type landscape_scene_detect; //0x208
    portrait_scene_detect_type portrait_scene_detect;

    // moved to ASD from VFE section
    chromatix_color_conversion_type        sunset_color_conversion;                   // Sunset
    chromatix_color_conversion_type        skintone_color_conversion;                 // SkinTL84
    chromatix_color_conversion_type        skintone_color_conversion_d65;             // SkinD65
    chromatix_color_conversion_type        skintone_color_conversion_a;               // SkinA
} ASD_struct_type;



/******************************************************************************
    AEC algo struct
******************************************************************************/
typedef struct
{
     int AEC_Software_Type; // 0 means hybrid, 1 means using bayer stats, for UA code only
     float                                  color_correction_global_gain;              // CCGlobalGain

    chromatix_exposure_table_type          chromatix_exposure_table;                  // Table
    snapshot_exposure_type					snapshot_exposure_table;

    unsigned char                                  default_luma_target;                       // Default
    unsigned char                                  outdoor_luma_target;                       // Outdoor
    //removed in 0x300   unsigned char                                  lowlight_luma_target;                      // LowLight
    //removed from 0x207
    //unsigned char                                  backlight_luma_target;             // BacklightLuma

    long                                  luma_tolerance;                    // LumaTolerance
    float                                  exposure_index_adj_step;           // ExpIndexAdjStep
    float                                  ISO100_gain;                       // ISO100Gain

    long                                  aec_indoor_index;                 // IndoorIndex
    long                                  aec_outdoor_index;                // OutdoorIndex

    float                                  max_snapshot_exposure_time_allowed;
    float                                  aggressiveness_values;
    unsigned short                                 fix_fps_aec_table_index;
    //removed in 0x300 int                                linear_afr_support;
    unsigned long                                 high_luma_region_threshold;

    aec_outdoor_bright_region_type         bright_region;
    aec_dark_region_type                   dark_region;
    //0x203
    unsigned short                                 wled_trigger_idx;
    // float                                  aec_led_preview_flux;
    float aec_led_pre_flux; //changed name in 0x300
    float                                  aec_led_flux_hi; //replaces aec_led_snapshot_flux, 0x205 mods
    float                                  aec_led_flux_med; //0x205 mods
    float                                  aec_led_flux_low;  //0x205 mods

    //204
    aec_motion_iso_type					 aec_motion_iso;
    //205

    int									aec_conv_holding_time_adj;  //205

    /******** AEC parameters below not used in 8k/OpenMM. 7k only. *************/
    float                                  max_video_fps;  //Android only

    //0x206 AEC
    unsigned short max_sensitivity_lux_index ;
    float AEC_weight_center_weighted[NUM_AEC_STATS][NUM_AEC_STATS];
    float AEC_weight_spot_meter[NUM_AEC_STATS][NUM_AEC_STATS];

    AEC_strobe_flash_type AEC_strobe_flash; // added strobe_flash_lux_idx_trigger in 0x208
    AEC_touch_type AEC_touch;

    AEC_face_priority_type AEC_face_priority;
    //0x207
   //removed in 0x300 AEC_motion_sensor_type AEC_motion_sensor_data;
    //0x208
    unsigned long  aec_fast_convergence_skip;  //determines how often fast convergence //updates exposure index
    unsigned long  aec_slow_convergence_skip;  //determines how often slow convergence //updates exposure index
    //0x300
    AEC_bayer_stats_algo_params_type AEC_bayer_stats_algo_params;
    shake_detection_type  shake_detection;
    red_eye_reduction_type red_eye_reduction; // added in 0x0207, added more enable params in 0x208

} AEC_algo_struct_type;

/******************************************************************************
    AWB algo struct
 ******************************************************************************/
typedef struct
{
    int AWB_Software_Type; //0 means hybrid, 1 means using bayer stats, for UA code only
    chromatix_awb_reference_type           reference[AGW_AWB_MAX_LIGHT] ;
    chromatix_awb_gain_adj_type             LED_gain_adj;
    chromatix_awb_gain_adj_type             strobe_gain_adj;
    chromatix_awb_gain_adj_type			    gain_adj[AGW_AWB_MAX_LIGHT];

    chromatix_awb_gain_adj_type				gain_adj_lowlight[AGW_AWB_MAX_LIGHT];

    long                                  indoor_index;                            // IndoorIndex
    long                                  outdoor_index;                           // OutdoorIndex

    float                                  snow_blue_gain_adj_ratio;                // SnowBlueGainAdj
    float                                  beach_blue_gain_adj_ratio;               // BeachBlueGainAdj

    int                                    outlier_distance;                        // OutlierDist
    int                                    green_offset_rg;                         // GreenOffRG
    int                                    green_offset_bg;                         // GreenOffBG
    unsigned char                                  skip_frames_after_changing_VFE;      // default value is 3

    float                                 extreme_RG_ratio_threshold;       // default=2.0, extreme color threshold in R/G direction
    float                                 extreme_BG_ratio_threshold;       // default=1.5, extreme color threshold in B/G direction

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

 //   float                                  color_correction_global_gain;              // CCGlobalGain
    chromatix_manual_white_balance_type    awb_min_gains;                             // MinGains
    chromatix_manual_white_balance_type    awb_max_gains;                             // MaxGains
    chromatix_awb_sample_influence_type    sample_influence;                          // SampleInfluence
    chromatix_awb_weight_vector_type       awb_weight_vector [AGW_AWB_MAX_ALL_LIGHT];     // WeightVector

    int                                    awb_white_world_y_min_ratio;               // WhiteWorldYMinRatio

    float                                 awb_aggressiveness;                        // Aggressiveness
    int                                awb_self_cal_enable;                       // SelfCalEnable
    //204
    float                                  awb_self_cal_adj_ratio_high;               // default=1.15
    float                                  awb_self_cal_adj_ratio_low;                // default = 0.9
    //205
    unsigned char								 awb_enable_lock_heuristics_1;				// default=1; changed from int type 0x300
    unsigned char								 awb_enable_lock_heuristics_2;				// default=1;changed from int type
    unsigned char								 awb_enable_lock_heuristics_3;				// default=1;changed from int type
    unsigned char	awb_indoor_daylight_lock_enable;  // to enable/disable the locking for indoor daylight condition.

    unsigned char								 awb_enable_white_world;					// default=1;changed from int type


    //0x206
    AWB_purple_prevent_type AWB_purple_prevent; //0x207 added enable variable
   unsigned char AWB_Ymin_low_threshold;//   (range: 0- 100, int, default =60)
    unsigned char AWB_Ymin_high_threshold;// (range: 0-100, int, default = 98).

    float AWB_golden_module_R_Gr_ratio[AGW_AWB_MAX_LIGHT];
    float AWB_golden_module_Gb_Gr_ratio[AGW_AWB_MAX_LIGHT];
    float AWB_golden_module_B_Gr_ratio[AGW_AWB_MAX_LIGHT];

    //0x207
    int enable_AWB_module_cal;// (???)
    AWB_motion_sensor_type AWB_motion_sensor_data;

    //new to 0x300
    float awb_led_strobe_adjustment_factor ;
    unsigned char lowlight_toggle_enable; //choose whether to enable stat bounding box toggling or not for lowlight. // default is FALSE.
    unsigned char awb_interpolate_gain_adj_enable;

    awb_lowlight_adj_lut_type AWB_lowlight_LUT;
    chromatix_match_LED_lighting_table_type mix_LED_table;

} AWB_algo_struct_type;

//this struct is updated from 0x207 to 0x208
typedef struct
{
	/********************************************
	added for parameters that overlapped with legacy awb
	**************************************************/
	chromatix_awb_reference_type           reference[AGW_AWB_MAX_LIGHT] ;
    chromatix_awb_gain_adj_type            LED_gain_adj;
    chromatix_awb_gain_adj_type            strobe_gain_adj;
    chromatix_awb_gain_adj_type			   gain_adj[AGW_AWB_MAX_LIGHT];

    chromatix_awb_gain_adj_type				gain_adj_lowlight[AGW_AWB_MAX_LIGHT];

    long                                  indoor_index;                            // IndoorIndex
    long                                  outdoor_index;                           // OutdoorIndex

    float                                  snow_blue_gain_adj_ratio;                // SnowBlueGainAdj
    float                                  beach_blue_gain_adj_ratio;               // BeachBlueGainAdj

    int                                    outlier_distance;                        // OutlierDist
    int                                    green_offset_rg;                         // GreenOffRG
    int                                    green_offset_bg;                         // GreenOffBG

	unsigned char                          skip_frames;      // default value is 1

	chromatix_manual_white_balance_type    awb_min_gains;                             // MinGains
    chromatix_manual_white_balance_type    awb_max_gains;                             // MaxGains

	float                                 awb_aggressiveness;                        // Aggressiveness
    int                                   awb_self_cal_enable;                       // SelfCalEnable
    //204
    float                                  awb_self_cal_adj_ratio_high;               // default=1.15
    float                                  awb_self_cal_adj_ratio_low;                // default = 0.9

	AWB_purple_prevent_type AWB_purple_prevent; //0x207 added enable variable

	//0x207
    int enable_AWB_module_cal;// (???)
    AWB_motion_sensor_type AWB_motion_sensor_data;

	//new to 0x300
    float awb_led_strobe_adjustment_factor ;
    unsigned char awb_interpolate_gain_adj_enable;

    awb_lowlight_adj_lut_type AWB_lowlight_LUT;
    chromatix_match_LED_lighting_table_type mix_LED_table;

	/**************************************************
	BAYER AWB Parameters
	***************************************************/

    int bright_green_percentage; //=6;  // at outdoor index
    int dark_green_percentage; //=12;    // at indoor index

    int dark_r_threshold; //=1;  // R stat ave below this value is rejected (8-bit domain)
    int dark_g_threshold; //=1;  // G stat ave below this value is rejected (8-bit domain)
    int dark_b_threshold; //=1;  // B stat ave below this value is rejected (8-bit domain)

    int white_stat_y_threshold_high; //=245;
    float threshold_extreme_b_percent; //=0.1;
    float threshold_extreme_r_percent; //=0.1;
    int threshold_compact_cluster; //=60;
    int compact_to_grey_dis; //=4;
    int threshold_compact_cluster_valid; //=4;
    int dominant_cluster_threshold; //=4;
    // distance weight table
    int distance_weight_table[AGW_NUMBER_GRID_POINT];
    int outdoor_adjustment; //=30;
    int exposure_adjustment; //=5;
    float outlier_valid_ymax_ratio; //=3.0;
    float cluster_high_pec;
    float cluster_mid_pec;
    float cluster_low_pec;
    int weight_vector[AGW_AWB_MAX_ALL_LIGHT][4];
    float ref_b_bg_tl84;
    float ref_r_rg_a;
    float extreme_range_perc_b; //=1.0;
    float extreme_range_perc_r; //=1.0;
    int threshold_compact_cluster_grey; // = 12;
    float blue_sky_pec;
    float blue_sky_pec_buffer;
    float slope_factor_m;
    float awb2_mcc_extreme_blue_threshold;
} Bayer_AWB_parameters_type;

/**************************************************************************************/
/**************************************************************************************/
/**************************************************************************************
     VFE structs
 **************************************************************************************/
/**************************************************************************************/
/**************************************************************************************/
typedef struct
{
    tuning_control_type     control_blk;//default 0 for lux_index
    trigger_point_type          blk_lowlight_trigger;
    unsigned short                max_blk_increase; //default 15*16 in 12bit
    chromatix_4_channel_black_level        normal_light_4_channel;             // NormalLight4Channel
} chromatix_black_level_type;

typedef struct
{
    tuning_control_type     control_abf2; //default 1
    unsigned short abf2_enable_index;
    trigger_point_type  abf2_low_light_trigger;
    trigger_point_type  abf2_bright_light_trigger;
    chromatix_adaptive_bayer_filter_data_type2  abf2_config_low_light;
    chromatix_adaptive_bayer_filter_data_type2  abf2_config_normal_light;
    chromatix_adaptive_bayer_filter_data_type2  abf2_config_bright_light;
} chromatix_ABF2_type;

typedef struct
{
    tuning_control_type     control_bpc; //default 0
    trigger_point_type        bpc_lowlight_trigger;

    //204
    unsigned char                              bpc_Fmin;
    unsigned char                              bpc_Fmax;
    unsigned char                              bpc_Fmin_lowlight;
    unsigned char                              bpc_Fmax_lowlight;

    bpc_4_offset_type     bpc_4_offset[BPC_MAX_LIGHT];

    // 0x207 bad pixel cluster params
    tuning_control_type     control_bcc; //default 0
    trigger_point_type          bcc_lowlight_trigger;

    unsigned char                              bcc_Fmin;
    unsigned char                              bcc_Fmax;
    unsigned char                              bcc_Fmin_lowlight;
    unsigned char                              bcc_Fmax_lowlight;

    bpc_4_offset_type     bcc_4_offset[BPC_MAX_LIGHT];
} chromatix_BPC_type;

typedef struct
{
//0x207 demosaic3
    tuning_control_type control_demosaic3;
    trigger_point_type demosaic3_trigger_lowlight;
    demosaic3_LUT_type demosaic3_LUT;

    float demosaic3_aG[LOWLIGHT_MAX_LIGHT]; //2 light condition,0 is lowlight, 1 is normal light
    float demosaic3_bL[LOWLIGHT_MAX_LIGHT]; //2 light condition
} chromatix_demosaic_type;

typedef struct
{
     //0x207 chroma filter
    tuning_control_type control_chroma_filter;
    trigger_point_type chroma_filter_trigger_lowlight;
    int chroma_filter_enable_index;
    Chroma_filter_type chroma_filter[LOWLIGHT_MAX_LIGHT]; //2 light condition

    //0x207 luma filter , 0x300 new type
    tuning_control_type control_LF;
    trigger_point_type  LF_low_light_trigger;
    trigger_point_type  LF_bright_light_trigger;
    int LF_enable_index;
    chromatix_luma_filter_type  LF_config_low_light;
    chromatix_luma_filter_type  LF_config_normal_light;
    chromatix_luma_filter_type  LF_config_bright_light;
} chromatix_CL_filter_type;

typedef struct
{
    tuning_control_type     control_cc;//default 0 for lux_index
    trigger_point_type          cc_trigger;
    chromatix_CCT_trigger_type CC_A_trigger;
    chromatix_CCT_trigger_type CC_Daylight_trigger;
    // chromatix_CCT_trigger_type CC_LED_trigger; removed in 0x300, use sensitivity trigger instead
    // chromatix_CCT_trigger_type CC_STROBE_trigger;
   //  use led influence params from AEC, sensitivity_low, senstivity_high, sensitivity_off
    float CC_LED_start;
    float CC_LED_end ;
    float CC_strobe_start;
    float CC_strobe_end ;

    //existing set is for TL84
    chromatix_color_correction_type        TL84_color_correction;        // default
    chromatix_color_correction_type         lowlight_color_correction;        // LowLight
    chromatix_color_correction_type        D65_color_correction;        // default
    chromatix_color_correction_type        A_color_correction;        // LowLight
    chromatix_color_correction_type	outdoor_color_correction; //NEW, use the same trigger as gamma outdoor trigger
    chromatix_color_correction_type        LED_color_correction;
    chromatix_color_correction_type        STROBE_color_correction;
} chromatix_CC_type;

typedef struct
{
    tuning_control_type     control_gamma;//default 0 for lux_index

    trigger_point_type          gamma_lowlight_trigger;
    trigger_point_type          gamma_outdoor_trigger;

    chromatix_gamma_table_type             default_gamma_table;         // Default
    chromatix_gamma_table_type             lowlight_gamma_table;         // LowLight
    chromatix_gamma_table_type             outdoor_gamma_table;         // Outdoor


} chromatix_gamma_type;

typedef struct
{
    tuning_control_type     control_cv;//default 0 for lux_index
    trigger_point_type          cv_trigger;
    chromatix_CCT_trigger_type CV_A_trigger;
    chromatix_CCT_trigger_type CV_Daylight_trigger;

    chromatix_color_conversion_type        TL84_color_conversion;           // TL84
    chromatix_color_conversion_type        A_color_conversion;   // A
    chromatix_color_conversion_type        daylight_color_conversion;       // D65
    chromatix_color_conversion_type        lowlight_color_conversion;        // LowLight
    chromatix_color_conversion_type        outdoor_color_conversion;        // Outdoor  (205)

    chromatix_color_conversion_type        mono_color_conversion;           // Mono
    chromatix_color_conversion_type        sepia_color_conversion;          // Sepia
    chromatix_color_conversion_type        negative_color_conversion;       // Negative
    chromatix_color_conversion_type        aqua_color_conversion;           // Aqua

 //we can remove BSM params because BSM can have separate header

    float                                  saturated_color_conversion_factor;         // SatFactor
} chromatix_CV_type;

typedef struct
{
    tuning_control_type control_asf_5X5; //DEFAULT 1 GAIN default value for VF and snapshot is same

    trigger_point_type          asf_5x5_lowlight_trigger;
    trigger_point_type          asf_5x5_outdoor_trigger;

    chromatix_asf_5_5_type                 asf_5_5;                             // SnapshotFilter
    /* soft_focus_degree */
    filter_sharpen_degree_type             soft_focus_degree_5_5;                   // SoftFocusDegree


    float asf_5_5_sharp_min_ds_factor; //default 0.5
    float asf_5_5_sharp_max_ds_factor; //default 4.0
    float asf_5_5_sharp_max_factor;   //default 1 for 6k, 8k, old 7k (before sROC). default 2.0 for sROC.
} chromatix_ASF_5x5_type;

typedef struct
{
    tuning_control_type control_asf_7x7; //DEFAULT 1 GAIN default value for VF and snapshot is same

    trigger_point_type          asf_7x7_lowlight_trigger;
    trigger_point_type          asf_7x7_brightlight_trigger;

    chromatix_asf_7_7_type                 asf_7_7;                             // SnapshotFilter
    /* soft_focus_degree */
    filter_sharpen_degree_type             soft_focus_degree_7_7;                   // SoftFocusDegree

    float asf_7_7_sharp_min_ds_factor; //default 0.5
    float asf_7_7_sharp_max_ds_factor; //default 4.0
    float asf_7_7_sharp_max_factor;   //
} chromatix_ASF_7x7_type;

typedef struct
{
    int LA_enable; //0x207 default false
    tuning_control_type					 control_la;// 205 default 0 for lux_index  //new205 mods
    trigger_point_type                     la_brightlight_trigger;

    LA_args_type   LA_config;  //this is for indoor
    LA_args_type   LA_config_outdoor; //new205 mods

    int la_luma_target_adj; //0x300 mods default -7. we reduce luma target when LA is on. This parameter is for 7k,6k,8k.
} chromatix_LA_type;

typedef struct
{
    tuning_control_type					 control_cs;//default 0 for lux_index
    trigger_point_type                     cs_lowlight_trigger;
    cs_luma_threshold_type                 cs_lowlight;
    cs_luma_threshold_type                 cs_normal;
    unsigned char                          chroma_thresh_BW;
    unsigned char                          chroma_thresh_color;
    mce_type                               mce_config;  //205
} chromatix_CS_MCE_type;

typedef struct
{
    wavelet_denoise_type wavelet_denoise_SW_420;
    wavelet_denoise_type wavelet_denoise_SW_422;
    wavelet_denoise_type wavelet_denoise_HW_420;
    wavelet_denoise_type wavelet_denoise_HW_422;
} chromatix_wavelet_type;

//0x301
#define NUM_GRAY_PATCHES 6
typedef struct
{
    int temporal_denoise_enable_index;
    tuning_control_type control_temporal_denoise;
    reference_temporal_noise_profile_type temporal_noise_profile[NUM_GRAY_PATCHES];
    int algorithm_select; // default 1, 0: power function, 1: Gaussian, 2: local linear minimum mean-squared error
} chromatix_temporal_denoise_type;


typedef struct
{
    float cr;
    float cb;
} sce_shift_vector;

typedef struct
{
    int                   sce_enable;
    //0x207
    tuning_control_type control_SCE;
    trigger_point_type SCE_trigger;   //under lowlight reduce SCE mapping to none
    chromatix_CCT_trigger_type SCE_A_trigger;
    chromatix_CCT_trigger_type SCE_D65_trigger;

    sce_cr_cb_triangle_set    origin_triangles_TL84; //existing set is for TL84
    sce_cr_cb_triangle_set    destination_triangles_TL84;
    sce_shift_vector          shift_vector_TL84;
    sce_affine_transform_2d   outside_region_mapping;
     //for other illuminants
    sce_cr_cb_triangle_set    origin_triangles_A;
    sce_cr_cb_triangle_set    destination_triangles_A;
    sce_shift_vector          shift_vector_A;
    sce_cr_cb_triangle_set    origin_triangles_D65;
    sce_cr_cb_triangle_set    destination_triangles_D65;
    sce_shift_vector          shift_vector_D65;
} chromatix_SCE_type;

typedef struct
{
    /* Stats bounding box with hw rolloff*/
    chromatix_wb_exp_stats_type            wb_exp_stats[AWB_STATS_MAX_LIGHT];  // StatConfig
    chromatix_rscs_stat_config_type			rscs_cnfig;
    bayer_histogram_config_type					bayer_histogram_config;
    bayer_grid_config_type						AWB_AEC_stats_window;
	bayer_grid_config_type						bayer_exp_window;  //0x301
} chromatix_grid_stats_type;

typedef struct
{
    float      skintone_Hmin;  //1.0 to 3.0, default 1.5
    float      skintone_Hmax;  //0.0 to 1.0, default 0.5
    float      skintone_Ymin;  //0.0 to 0.3, default 0.1
    float      skintone_Ymax;  //0.7 to 1.0, default 0.9
    float      skintone_S_HY_min;  //0.0 to 0.4, default 0.05
    float      skintone_S_HY_max;  //0.1 to 0.5, default 0.25
    float      skintone_S_LY_min;  //0.0 to 0.5, default 0.25
    float      skintone_S_LY_max;  //0.2 to 1.0, default 0.6
    unsigned char      skintone_percentage;  // threshold percentage (0 to 100), output=1 when skin pixels exceed this percentage
} chromatix_skin_detection_type;


typedef struct
{
    chromatix_manual_white_balance_type    MWB_tl84;            // Flourescent
    chromatix_manual_white_balance_type    MWB_d50;             // Sunny
    chromatix_manual_white_balance_type    MWB_A;    // Tungsten
    chromatix_manual_white_balance_type    MWB_d65;             // Cloudy/Shade
    chromatix_manual_white_balance_type    strobe_flash_white_balance;              // Strobe
    chromatix_manual_white_balance_type    led_flash_white_balance;                 // LED
} chromatix_MWB_type;


typedef struct
{
    unsigned short ZSL_default_shutter_lag;
    unsigned short ZSL_max_Q_factor_reduction;
} chromatix_ZSL_type;

//0x301

typedef struct
{
float tolerance;
float contrast_meter;
float normal_lowlight_boost_meter;
float brightlight_boost_meter;

}chromatix_HDR_type;

//face detection and recognition
//0x301

#define ANGLE_0     0x00001001  /* Top of the head facing from 345 degrees to 15  degrees */
#define ANGLE_1     0x00002002  /* Top of the head facing from 15  degrees to 45  degrees */
#define ANGLE_2     0x00004004  /* Top of the head facing from 45  degrees to 75  degrees */
#define ANGLE_3     0x00008008  /* Top of the head facing from 75  degrees to 105 degrees */
#define ANGLE_4     0x00010010  /* Top of the head facing from 105 degrees to 135 degrees */
#define ANGLE_5     0x00020020  /* Top of the head facing from 135 degrees to 165 degrees */
#define ANGLE_6     0x00040040  /* Top of the head facing from 165 degrees to 195 degrees */
#define ANGLE_7     0x00080080  /* Top of the head facing from 195 degrees to 225 degrees */
#define ANGLE_8     0x00100100  /* Top of the head facing from 225 degrees to 255 degrees */
#define ANGLE_9     0x00200200  /* Top of the head facing from 255 degrees to 285 degrees */
#define ANGLE_10    0x00400400  /* Top of the head facing from 285 degrees to 315 degrees */
#define ANGLE_11    0x00800800  /* Top of the head facing from 315 degrees to 345 degrees */

#define ANGLE_ALL   0x00ffffff  /* All angles are detected                                */
#define ANGLE_NONE  0x00000000  /* None of the angles will be detected                    */

typedef struct
{
    //  face detection tuning parameters
    int minimum_face_width;
    int maximum_face_width;
    int maximum_num_face_to_detect;
     int angles_to_detect_pose_front;
     int angles_to_detect_pose_half_profile;
     int angles_to_detect_pose_profile;
} chromatix_face_detection_type;


typedef struct
{
    // face recognition tuning parameters
    int max_num_users;     // maximum number of users in the database;
    int max_data_per_user; // maximum number of data per user in the 				//database;

    // the above two perimeters govern the upper limit of how big the database can grow to
    // the database size is approximately:
    // 32 (header) + 152 x (# people registered) + 148 x (# data/images registered overall) bytes
} chromatix_face_recognition_type;


//chroma aliasing coreection (CAC)  0x301
typedef struct
{
int correction_strength;
unsigned char brightness_threshold;
unsigned char _threshold_center;
unsigned char soft_threshold_span;
}chroma_aliasing_correction_type;


typedef struct
{

tuning_control_type     control_CAC;//default 0 for lux_index
trigger_point_type          CAC_lowlight_trigger;
chroma_aliasing_correction_type CAC_lowlight;
chroma_aliasing_correction_type CAC_normal;

}chromatix_chroma_aliasing_correction_type;

//color tint correction 0x301
typedef struct
{
int	tint_correction_mode;
unsigned char		tint_correction_strength;
unsigned char		tint_correction_accuracy_degree;
}chromatix_color_tint_correction_type;

//local tone mapping
typedef struct {
	float fCV_R;
	float fCV_G;
	float fCV_B;
	float fCV_Kc;
	float fCV_Cthr;
	int nInitCntFactor;
	float fSigmaXY;
	float fSigmaZ;
	int nNormalLoCut;
	int nNormalHiCut;
	int nStrongLoCut;
	int nStrongHiCut;
	float fBusyThr1;
	float fBusyThr2;
	unsigned long nLowTrig0;
	unsigned long nLowTrig1;
	unsigned long nLowTrig2;
	unsigned long nLowTrig3;
	unsigned long nHighTrig0;
	unsigned long nHighTrig1;
	unsigned long nHighTrig2;
	unsigned long nHighTrig3;
	float fManualGTMGain;
	float fManualGTMSigma;
	float fW1_LA;
	float fScaleConstraintLB;
	float fScaleSigmaMult;
	float fScaleWidthLB;
	float fMasterScaleAdj;
	float fShiftSigma;
	float fShiftMag;
	int nSoftThr0;
	int nSoftThr1;
	int nSoftThrSlope;
	float fShiftScaleAdj;
} chromatix_LTM_type;


/****************************************************************************************/
// post processing
/*************************************************************************************/
typedef struct
{
	chromatix_HDR_type chromatix_HDR ;
	chromatix_face_detection_type chromatix_face_detection;
	chromatix_face_recognition_type chromatix_face_recognition;
	chromatix_chroma_aliasing_correction_type chromatix_chroma_aliasing_correction;
	chromatix_color_tint_correction_type chromatix_color_tint_correction;
	chromatix_LTM_type chromatix_LTM_data;
}chromatix_post_processing_type;

/*********************************************************************************
**********************************************************************************
VFE overall
**********************************************************************************
**********************************************************************************
**********************************************************************************/


typedef struct
{
 /******************************************************************************
    Black Level     (BlackLevel)
    removed 6k and 7k blk supports
    ******************************************************************************/

    chromatix_black_level_type chromatix_black_level;

/******************************************************************************
 channel balance
******************************************************************************/

    chromatix_channel_balance_gains_type chromatix_channel_balance_gains;          // ChannelGain

/******************************************************************************
    Adaptive Bayer Filter 2    (ABF2)  new 8K, 7x30, 8x55 and later chips
    no change
    ******************************************************************************/

    chromatix_ABF2_type chromatix_ABF2;

 /******************************************************************************
    Bad Pixel Correction
    removed old structs, only supports 4 channel offsets type
    ABCC is handled by sensor driver
******************************************************************************/

    chromatix_BPC_type chromatix_BPC;

/******************************************************************************
    Demosaic3
    no change
    ******************************************************************************/

    chromatix_demosaic_type chromatix_demosaic;

/******************************************************************************
    LC filters
    create luma filter type
    ******************************************************************************/

    chromatix_CL_filter_type chromatix_CL_filter;

/******************************************************************************
    Color Correction     (ColorCorr)
    added outdoor CC
    changed name to remove chromatix_, changed yhi_ylo to lowlight
    removed LED , strobe trigger in 0x300, use sensitivity trigger instead
******************************************************************************/

    chromatix_CC_type chromatix_CC;

 /******************************************************************************
    Gamma
    change to 64 entries
    changed name to remove chromatix_rgb_, changed yhi_ylo to lowlight, removed backlit gamma, needs to use LA
    ******************************************************************************/

    chromatix_gamma_type chromatix_gamma;

    /******************************************************************************
    Color Conversion   (ColorConv)
    changed name to remove chromatix_, changed yhi_ylo to lowlight
    ******************************************************************************/

    chromatix_CV_type chromatix_CV;

    /******************************************************************************
    5x5 ASF      (5X5ASF)

    ******************************************************************************/

    chromatix_ASF_5x5_type chromatix_ASF_5x5;

 /******************************************************************************
    7x7 ASF      NEW

    ******************************************************************************/

    chromatix_ASF_7x7_type chromatix_ASF_7x7;

/******************************************************************************
    Luma Adaptation
    removed old (8K)struct, added new struct LA_type
    ******************************************************************************/

    chromatix_LA_type chromatix_LA;

 /******************************************************************************
    Chroma Suppression  (ChromaSupp) & MCE (memory color enhancement)
    no change
    ******************************************************************************/

    chromatix_CS_MCE_type chromatix_CS_MCE;

    /******************************************************************************
    wavelet
    same struct, but offer 420 and 422
    ******************************************************************************/

    chromatix_wavelet_type chromatix_wavelet;

	/******************************************************************************
   temporal denoise for GPU version
    ******************************************************************************/
	chromatix_temporal_denoise_type chromatix_temporal_denoise;  //0x301

 /******************************************************************************
    skin color enhancement (SCE)
    no change, move trigger to beginning
    ******************************************************************************/

   chromatix_SCE_type chromatix_SCE;

/******************************************************************************/
//STATS config
/******************************************************************************/

    chromatix_grid_stats_type chromatix_grid_stats;

/******************************************************************************
Skin Tone Detection (Skintone)
******************************************************************************/

    chromatix_skin_detection_type chromatix_skin_detection;

} chromatix_VFE_type;


/******************************************************************************
******************************************************************************
******************************************************************************
******************************************************************************
******************************************************************************
******************************************************************************
    CHROMATIX HEADER definition
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/

typedef struct
{
//common structs

    chromatix_version_type chromatix_version;
    unsigned char is_compressed;
    unsigned short revision_number;

/******************************************************************************
    VFE structs
    ******************************************************************************/

    chromatix_VFE_type chromatix_VFE;

 /******************************************************************************
    MWB    ** Manual white balance gains **
    no change
    ******************************************************************************/

    chromatix_MWB_type chromatix_MWB;

/******************************************************************************
    AWB
******************************************************************************/

    AWB_algo_struct_type AWB_algo_data; //this contains algorithm related params
	Bayer_AWB_parameters_type AWB_bayer_algo_data; //this contains Bayer algorithm related params

/******************************************************************************
    AEC
 ******************************************************************************/

    AEC_algo_struct_type AEC_algo_data;

/******************************************************************************
    AFD
******************************************************************************/

    chromatix_auto_flicker_detection_data_type  auto_flicker_detection_data;   // DetectionData
/******************************************************************************
    ASD
 ******************************************************************************/

    ASD_struct_type ASD_algo_data;

/******************************************************************************
 ZSL
******************************************************************************/

    chromatix_ZSL_type chromatix_ZSL;


/******************************************************************************
 post processing modules
******************************************************************************/


	chromatix_post_processing_type chromatix_post_processing;

} chromatix_parms_type;

#endif
