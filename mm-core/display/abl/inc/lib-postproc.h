/*
 *Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 *Qualcomm Technologies Proprietary and Confidential.
 */
#ifdef __cplusplus
 extern "C" {  // only need to export C interface if
               // used by C++ source code
#endif

#ifndef DISP_PP_CSC_H
#define DISP_PP_CSC_H

#include <sys/types.h>
#include <linux/msm_mdp.h>

#define NUM_ARGC_STAGES     16
#define MAX_IGC_LUT_ENTRIES 256
#define MAX_HIST_LUT_ENTRIES 256
#define PCC_COEFF_PER_COLOR 11

#define NUM_ROW_CC_MATRIX	3
#define NUM_COL_CC_MATRIX	3

#define VERS_MAX_LEN 80

#define PP_OP_HSIC      0x0001
#define PP_OP_QSEED     0x0002
#define PP_OP_PCC       0x0004
#define PP_OP_ARGC      0x0008
#define PP_OP_IGC       0x0010
#define PP_OP_HIST_LUT  0x0020
#define PP_OP_PA        0x0040
#define PP_OP_SHARP      0x0080
#define PP_OP_PA_V2     0x0100

#define DEFAULT_MAX_MIXER_WIDTH 2048
#define MDP_HW_REV_UNKNOWN 0
#define MDP_HW_REV_V3_0 300
#define MDP_HW_REV_V4_2 420
#define MDP_HW_REV_V5_0 500

typedef enum{
    interface_rec601,
    interface_rec709,
    interface_custom,
}interface_type;

struct display_hw_info {
    uint32_t nPriDisplayHistBlock;
    uint32_t nPriDisplayHistBins;
    uint32_t nPriDisplayHistComp;
    int32_t isMDP5;
    int32_t hwRevision;
    uint32_t maxMixerWidth;
};

typedef enum{
    hsic_order_hsc_i,
    hsic_order_i_hsc,
}hsic_order_type;

struct display_hsic_cust_params {
    interface_type intf;
    float cust_rgb2yuv[3][3], cust_yuv2rgb[3][3];
};

/* Following structures are needed for PA features in mode manager */
typedef struct {
    int32_t hue;
    int32_t sat;
    int32_t val;
    int32_t con;
    int32_t sat_thresh;
} display_native_global_adj_data;

typedef struct {
    int32_t hue;
    int32_t sat;
    int32_t val;
} display_native_mem_col_adj_data;

/*
struct display_pp_conv_cfg - Structure for Hue, Saturation, Intensity,
    Contrast (HSIC) & Color Correction parameters to post processing library

    ops - Options for Convert block of MDP
        bit 0 - enable/disable convert block, 0 - disable, 1 - enable
        bit 1 - input format, 0 - RGB, 1 - YUV
        bit 2 - output format, 0 - RGB, 1 - YUV
        bit 3-31 - Reserved

        hue             - Hue, valid from -180 to 180 degrees
        intensity       - Intensity, valid from -255 to 255
        sat             - Saturation, valid from -1.0 to 1.0 (percentage)
        contrast        - Contrast, valid from -1.0 to 1.0 (percentage)
        cc_matrix[4][4] - Color Correction matrix, 4*4 matrix (including
                          black offset)
*/

struct display_pp_conv_cfg
{
    uint32_t ops;
    int32_t hue;
    int32_t intensity;
    float sat;
    float contrast;
    float cc_matrix[4][4];
};

/*
struct display_pcc_coeff - Structure for passing Polynomial Color Correction
    coefficients to post processing library.

     pcc_coeff[0] - Constant Coefficient (c)

     pcc_coeff[1] - First Order Coefficient (r)
     pcc_coeff[2] - First Order Coefficient (g)
     pcc_coeff[3] - First Order Coefficient (b)

     pcc_coeff[4] - Second Order Coefficient (rr)
     pcc_coeff[5] - Second Order Coefficient (gg)
     pcc_coeff[6] - Second Order Coefficient (bb)
     pcc_coeff[7] - Second Order Coefficient (rg)
     pcc_coeff[8] - Second Order Coefficient (gb)
     pcc_coeff[9] - Second Order Coefficient (rb)

     pcc_coeff[10] - Third Order Coefficient (rgb)
*/
struct display_pcc_coeff{
     double pcc_coeff[PCC_COEFF_PER_COLOR];
};

/*
struct display_pp_pcc_cfg - Structure for Polynomial Color Correction
    coefficients & PCC block options

    ops - Options for PCC block of MDP
        bit 0 - Enable/disable PCC, 0 - disable, 1 - enable
        bit 1,2 - Read/write PCC coefficients
                  00 - Do not read/write
                  01 - Read PCC coefficients from regs
                  10 - Write PCC coefficients to regs
                  11 - Reserved
        bit 3 - Coefficient register set, 0 - set one, 1 - set two
        bit 4 - Enable register set, 0 - no change, 1 - enable register
                set (see bit 3)
        bit 5-31 - Reserved

    r, g, b - PCC coeffcients for R, G, and B channel
*/

struct display_pp_pcc_cfg{
    uint32_t ops;
    struct display_pcc_coeff r, g, b;
};

/*
struct display_pp_argc_stage_data - Structure for AR-GC stage data

        is_stage_enabled - Is stage enabled, 0 - disabled, 1 - enabled
        x_start - X start value for stage
        slope - Slope of line for stage
        offset - Y offset for stage
*/

struct display_pp_argc_stage_data{
    uint8_t is_stage_enabled;
    uint32_t x_start;
    double slope;
    double offset;
};

/*
struct display_pp_argc_lut_data - Structure for AR-GC stages
    ops - Options for AR-GC LUT
        bit 0 - Enable/disable AR-GC LUT, 0 - disable, 1 - enable
        bit 1,2 - Read/write AR-GC LUT coefficients
                  00 - Do not read/write
                  01 - Read LUT data from regs
                  10 - Write LUT data to regs
                  11 - Reserved
        bit 3-31 - Reserved

    r - Stages data for R channel
    g - Stages data for G channel
    b - Stages data for B channel
*/
struct display_pp_argc_lut_data{
    uint32_t ops;
    struct display_pp_argc_stage_data r[NUM_ARGC_STAGES];
    struct display_pp_argc_stage_data g[NUM_ARGC_STAGES];
    struct display_pp_argc_stage_data b[NUM_ARGC_STAGES];
};

/*
   struct display_pp_igc_lut_data - Structure for IGC data
   ops - Options for IGC LUT
        bit 0 - Enable/disable IGC LUT, 0 - disable, 1 - enable
        bit 1,2 - Read/write IGC LUT coefficients
                  00 - Do not read/write
                  01 - Read LUT data from regs
                  10 - Write LUT data to regs
                  11 - Reserved
        bit 3-31 - Reserved
   c0   - IGC LUT data for c0
   c1   - IGC LUT data for c1
   c2   - IGC LUT data for c2
 */

struct display_pp_igc_lut_data{
    uint32_t ops;
    uint16_t c0[MAX_IGC_LUT_ENTRIES];
    uint16_t c1[MAX_IGC_LUT_ENTRIES];
    uint16_t c2[MAX_IGC_LUT_ENTRIES];
};

/*
struct display_pp_hist_lut_data - Structure for histogram look up table data
    ops - Options for histogram LUT,
        bit 0 - Enable/disable LUT, 0 - disable, 1 - enable
        bit 1,2 - Read/write LUT coefficients
                  00 - Do not read/write
                  01 - Read LUT data from regs
                  10 - Write LUT data to regs
                  11 - Reserved
        bit 3 - LUT register set, 0 - set one, 1 - set two
        bit 4 - Enable register set, 0 - no change, 1 - enable register
                set (see bit 3)
        bit 5-31 - Reserved

    c0   - histogram LUT data for c0
    c1   - histogram LUT data for c1
    c2   - histogram LUT data for c2
 */

struct display_pp_hist_lut_data{
    uint32_t ops;
    uint16_t c0[MAX_HIST_LUT_ENTRIES];
    uint16_t c1[MAX_HIST_LUT_ENTRIES];
    uint16_t c2[MAX_HIST_LUT_ENTRIES];
};

struct display_pp_conv_input_params{
    hsic_order_type order;
    interface_type interface;
    uint32_t ops;
    int32_t hue;
    int32_t intensity;
    float sat;
    float contrast;
    float cc_matrix[NUM_ROW_CC_MATRIX][NUM_COL_CC_MATRIX];
    float black_offsets[3];
    float rgb2yuv[3][3], yuv2rgb[3][3];
};

struct display_pp_qseed_input_params {
    int strength;
};

/*
struct display_pp_pa_cfg - Structure for Picture Adjustmemt data
    ops - Options for HSIC data & choosing between SPA and PA
        bit 0 - Enable/disable HSIC data, 0 - disable, 1 - enable
        bit 1,2 - Read/write HSIC data
                  00 - Do not read/write
                  01 - Read HSIC data from regs
                  10 - Write HSIC data to regs
                  11 - Reserved
        bit 3-30 - Reserved
        bit 31 - choose between SPA and PA
                  0 - PA
                  1 - SPA

    hue             - Hue, valid from -180.0 to 180.0 degrees
    intensity       - Intensity, valid from -255 to 255
    sat_thresh      - threshold for saturation, valid from 0 - 255
    sat             - Saturation, valid from -1.0 to 1.0 (percentage)
    contrast        - Contrast, valid from -1.0 to 1.0 (percentage)
*/

struct display_pp_pa_cfg
{
    uint32_t ops;
    float hue;
    int32_t intensity;
    int32_t sat_thresh;
    float sat;
    float contrast;
};

/*
struct display_pp_mem_col_cfg - Structure for Memory Color adjustment data
    offset - offset from minimum hue value to top corner of trapezoid (0.0 - 360.0)
    hue_slope - floating point slope of trapezoid side for hue adjust, starting from hue min (-inf - inf)
    sat_slope - floating point slope of trapezoid side for sat adjust, starting from hue min (-inf - inf)
    val_slope - floating point slope of trapezoid side for val adjust, starting from hue min (-inf - inf)
    hue_min - Min hue for memory color adjustments (0.0 - 360.0)
    hue_max - Max hue for memory color adjustments (0.0 - 360.0)
    sat_min - Min sat for memory color adjustments (0 - 1.0)
    sat_max - Max sat for memory color adjustments (0 - 1.0)
    val_min - Min val for memory color adjustments (0 - 255)
    val_max - Max val for memory color adjustments (0 - 255)
*/
struct display_pp_mem_col_cfg
{
    float offset;
    float hue_slope;
    float sat_slope;
    float val_slope;
    float hue_min;
    float hue_max;
    float sat_min;
    float sat_max;
    int32_t val_min;
    int32_t val_max;
};

/*
struct display_pp_six_zone_cfg - Structure for Six Zone adjustment data
    hue - 384 Hue values specify the six zone hue adjust curve (-180.0 - 180.0)
    sat - 384 sat values specify the six zone sat adjust curve (-1.0 - 1.0)
    val - 384 val values specify the six zone val adjust curve (-1.0 - 1.0)
    sat_min - threshold for six zone sat adjustment (0 - 1.0)
    val_min - min val for six zone val adjustment (0 - 255)
    val_max - max val for six zone val adjustment (0 - 255)
*/
struct display_pp_six_zone_cfg
{
    float hue[MDP_SIX_ZONE_LUT_SIZE];
    float sat[MDP_SIX_ZONE_LUT_SIZE];
    float val[MDP_SIX_ZONE_LUT_SIZE];
    float sat_min;
    int32_t val_min;
    int32_t val_max;
};

/*
struct display_pp_pa_v2_cfg - Structure for Picture Adjustmemt V2 data
    ops - Options for HSVC, memory color, and six zone color adjustment
        bit 0 - Enable/disable Picture Adjustment, 0 - disable, 1 - enable
        bit 1,2 - Read/write PA data
                  00 - Do not read/write
                  01 - Read HSIC data from regs
                  10 - Write HSIC data to regs
                  11 - Reserved
        - bits 4-11 specify enable/disable of the registers being read from or
                written to for that specific feature.
        bit 4 - Global hue adjust read/write enable, 0 -disable, 1 - enable
        bit 5 - Global saturation adjust read/write enable, 0-disable, 1-enable
        bit 6 - Global value adjust read/write enable, 0-disable, 1-enable
        bit 7 - Global contrast adjust read/write enable, 0-disable, 1-enable
        bit 8 - Six zone adjust read/write enable, 0-disable, 1-enable
        bit 9 - Memory color skin adjust read/write enable, 0-disable, 1-enable
        bit 10 - Memory color sky adjust read/write enable, 0-disable, 1-enable
        bit 11 - Mem col foliage adjust read/write enable, 0-disable, 1-enable

        - bits 12-21 specify enable/disable of the feature via mask bit even if
                the registers have been programmed
        bit 12 - Global hue adjust mask, 0-disable, 1-enable
        bit 13 - Global saturation adjust mask, 0-disable, 1-enable
        bit 14 - Global value adjust mask, 0-disable, 1-enable
        bit 15 - Global contrast adjust mask, 0-disable, 1-enable
        bit 16 - Six zone hue adjust mask, 0-disable, 1-enable
        bit 17 - Six zone saturation adjust mask, 0-disable, 1-enable
        bit 18 - Six zone value adjust mask, 0-disable, 1-enable
        bit 19 - Memory color skin adjust mask, 0-disable, 1-enable
        bit 20 - Memory color sky adjust mask, 0-disable, 1-enable
        bit 21 - Memory color foliage adjust mask, 0-disable, 1-enable

        bit 22 - Memory color adjustment protection, 0 - global adjustment
                happens independently of memory color, 1 - global adjustment
                allowed only if pixel hasn't been processed by memory color
        bit 23 - Saturation zero exponent enable, 0 - if global sat adjust < -32
                the sat adjust is clamped to 0, 1 - if global sat adjust < -64
                the sat adjust is clamped to 0.
    global_adj - elements specified below
        global_hue          - Hue, valid from -180.0 to 180.0 degrees
        global_saturation   - Saturation, valid from -1.0 to 1.0 (percentage)
        sat_thresh          - Saturation adjustment threshold, valid 0 to 1.0
        global_value        - Value, valid from -255 to 255
        global_contrast     - Contrast, valid from -1.0 to 1.0 (percentage)
    six_zone_cfg - structure containing six zone adjustment data
    skin/sky/fol_cfg - structure containing memory color adjustment data
*/
struct display_pp_pa_v2_cfg
{
    uint32_t ops;
    float global_hue;
    float global_saturation;
    float sat_thresh;
    int32_t global_value;
    float global_contrast;
    struct display_pp_mem_col_cfg skin_cfg;
    struct display_pp_mem_col_cfg sky_cfg;
    struct display_pp_mem_col_cfg fol_cfg;
    struct display_pp_six_zone_cfg six_zone_cfg;
};

/*
struct display_pp_sharp_cfg - Structure for SEED2 sharpening
    ops - Options for QSEED2 sharpening
        bit 0 - Enable/disable HSIC data, 0 - disable, 1 - enable
        bit 1,2 - Read/write sharpening data
                  00 - Do not read/write
                  01 - Read data from regs
                  10 - Write data to regs
                  11 - Reserved
        bit 3-31 - Reserved

    edge_thr     - Unsigned Threshold to select Edge/Normal Filter
                   for Content Adaptive Filter
    smooth_thr   - Unsigned Threshold to select Normal/Smooth Filter
                   for Content Adaptive Filter
    noise_thr    - Unsigned Threshold to select Normal/Smooth Filter
                   for Content Adaptive Filter

    RECOMMENDED VALUES FOR THRESHOLDS:
    if (strength > 0) (i.e., sharpening)
                     {edge_thr, smooth_thr,noise_thr} = {8, 8, 2}; 
    if (strength < 0) (i.e., smooth)
                     {edge_thr, smooth_thr,noise_thr} = {0, 0, 0}; 
*/
struct display_pp_sharp_cfg {
    uint32_t ops;
    uint32_t edge_thr;
    uint32_t smooth_thr;
    uint32_t noise_thr;
    int strength;
};

struct compute_operation_params {
    struct display_pp_conv_input_params conv_params;
    struct display_pp_qseed_input_params qseed_params;
    struct display_pp_pcc_cfg pcc_params;
    struct display_pp_argc_lut_data argc_params;
    struct display_pp_igc_lut_data igc_lut_params;
    struct display_pp_hist_lut_data hist_lut_params;
    struct display_pp_pa_cfg pa_params;
    struct display_pp_pa_v2_cfg pa_v2_params;
    struct display_pp_sharp_cfg sharp_params;
};

struct compute_params {
    uint32_t operation;
    struct compute_operation_params params;
};


/*
 display_pp_init(): Function to initialize the post processing library.
    This function must be called prior to calling any other functions in post
     processing library.

    Return Value: 0 if successful, otherwise appropriate error code
 */
int display_pp_init();

/*
 display_pp_exit(): Function to release all resources initialized by
     display_pp_init(). This function must be called the end of of post
     processing operations, to make sure all allocated resources are freed.

    Return Value: 0 if successful, otherwise appropriate error code
 */
int display_pp_exit();

/*
display_pp_conv_init(): Function to initialize post processing library Convert
    block on specified MDP block. This function must be called before calling
    display_pp_conv_set_cfg()

    block - MDP block ID, see linux/msm_mdp.h for definitions
    cust_param - customization values to select specific conversion settings,
    see display_hsic_cust_params definition for elements. If cust_param is NULL
    by default REC 601 coefficients are used for color space conversion.

    Return Value : 0 if successful, -1 if failed
*/
int display_pp_conv_init(uint32_t block,
                                struct display_hsic_cust_params *cust_param);

/*
display_pp_conv_set_cfg(): Function to set HSIC and color correction matrix
    of Convert block on specified MDP block. This function must be called
    after display_pp_conv_init()

    block - MDP block ID, see linux/msm_mdp.h for definition

    conv_cfg - pointer to display_pp_conv_cfg structure, containing HSIC
           and color correction matrix

    Return Value : 0 if successful, -1 if failed
*/

int display_pp_conv_set_cfg(uint32_t block, struct display_pp_conv_cfg *conv_cfg);

/*
display_pp_pcc_set_cfg(): Function to set PCC coefficients of PCC block on
    specified MDP block.

    block - MDP block ID, see linux/msm_mdp.h for definition

    pcc_cfg - pointer to display_pp_pcc_cfg structure, containing PCC block
            configuration

    Return Value : 0 if successful, -1 if failed

*/
int display_pp_pcc_set_cfg(uint32_t block, struct display_pp_pcc_cfg *pcc_cfg);

/*
display_pp_argc_set_lut(): Function to set Area Reduced Gamma Correction
    parameters of AR-GC block on specified MDP block.

    block - MDP block ID, see linux/msm_mdp.h for definition

    argc_lut_data - pointer to display_pp_argc_lut_data structure, containing
    AR-GC block configuration

    Return Value : 0 if successful, -1 if failed

*/

int display_pp_argc_set_lut(uint32_t block, struct display_pp_argc_lut_data *argc_lut_data);

/*
display_pp_igc_set_lut(): Function to set Inverse Gamma Correction Lookup table
    parameters of IGC block on specified MDP block.

    block - MDP block ID, see linux/msm_mdp.h for definition

    igc_lut_data - pointer to display_pp_igc_lut_data structure, containing
    IGC block configuration

    Return Value : 0 if successful, -1 if failed

*/

int display_pp_igc_set_lut(uint32_t block, struct display_pp_igc_lut_data *igc_lut_data);

/*
display_pp_hist_set_lut(): Function to set histogram look up table parameters
    of histogram LUT block on specified MDP block.

    block - MDP block ID, see linux/msm_mdp.h for definition

    hist_lut_data - Pointer to display_pp_hist_lut_data structure containing
    histogram LUT block configuration

    Return Value : 0 if successful, -1 if failed
*/

int display_pp_hist_set_lut(uint32_t block, struct display_pp_hist_lut_data *hist_lut_data);

/*
display_pp_compute_params(): Function to compute register values for
specified operation based on given parameters.

    op_params - Operations parameters, required for computation

    mdp_pp_cfg - Computed register values, based on op_params

    Return Value : 0 if successful, -1 if failed
*/

int display_pp_compute_params(const struct compute_params *op_params,
                                            struct mdp_overlay_pp_params *cfg);


/*
 * display_pp_ad_supported(): Function to check if Assertive Display feature
 * is supported by the target.
 * Return Value: 0 if not supported, 1 if supported
 */
int display_pp_ad_supported(void);
int display_pp_cabl_supported(void);
int display_pp_svi_supported(void);
int display_pp_has_right_split();
int display_pp_get_max_mixer_width(uint32_t *width);
int display_pp_get_hw_revision(int32_t *version);

#define PA_V2_DESAT                    -500

/* Function to check if PA V2 is supported */
int display_pp_pa_v2_supported(void);

/* Function which needs to be called to get all cached kernel params for
 * Global and memory color adjustment, passed through data
 */
int display_pp_pa_v2_get_cached_cfg(struct mdp_pa_v2_data *pa_v2_cfg,
    uint32_t *data, uint32_t len, uint32_t flag);

/* Function to convert native Global PA API values to kernel parameters */
int display_pp_pa_glb_native_to_ker_params(uint32_t *data,
    display_native_global_adj_data *adj, display_native_global_adj_data *ref,
    int32_t *ops);

/* Function to convert native Global PA V2 API values to kernel parameters */
int display_pp_pa_v2_glb_native_to_ker_params(uint32_t *data,
    display_native_global_adj_data *adj, display_native_global_adj_data *ref,
    int ops);

/* Function to convert native memory PA V2 API values to kernel parameters */
int display_pp_pa_mem_native_to_ker_params(uint32_t *data,
                         display_native_mem_col_adj_data *adj, int32_t type);

/* Function to convert kernel global PA values to native API values */
int display_pp_pa_glb_ker_to_native_params(uint32_t *data,
    display_native_global_adj_data *adj);

/* Function to convert kernel global PA V2 values to native API values */
int display_pp_pa_v2_glb_ker_to_native_params(uint32_t *data,
    display_native_global_adj_data *adj);

/* Function to convert kernel memory PA V2 values to native API values */
int display_pp_pa_mem_ker_to_native_params(uint32_t *data, uint32_t idx,
    display_native_mem_col_adj_data *adj);

/* Following functions are used to get ranges for user using native APIs */
int display_pp_get_global_native_range(display_native_global_adj_data *max,
                                    display_native_global_adj_data *min);

int display_pp_get_mem_native_range(display_native_mem_col_adj_data *max,
                                    display_native_mem_col_adj_data *min);

#endif //DISP_PP_CSC_H

#ifdef __cplusplus
 }
#endif
