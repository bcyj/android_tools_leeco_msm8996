/* q3a_stats_hw.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __Q3A_STATS_HW_H__
#define __Q3A_STATS_HW_H__
#include "mtype.h"

#define MAX_BE_STATS_NUM  768   /*32x24, usually we only use 32x24*/
#define MAX_BG_STATS_NUM     3888  /* 72x54, usually we only use 64x48 */
#define MAX_BF_STATS_NUM      252  /* 18x14 */
#define MAX_YUV_STATS_NUM     256  /* 16x16, aec & awb */
#define MAX_YUV_AF_STATS_NUM   81  /* 9x9, AF */
#define MAX_HIST_STATS_NUM    256

#define MAX_CS_STATS_NUM     1344
#define MAX_RS_STATS_NUM     1024

#define MAX_BF_H_NUM           18
#define MAX_BF_V_NUM           14
#define MAX_BF_BLOCK_WIDTH    336
#define MAX_BF_BLOCK_HEIGHT   252

/* by HW the min block sixe is 6x2
 * but noise is too bad for stats */
#define MIN_BF_BLOCK_WIDTH     64
#define MIN_BF_BLOCK_HEIGHT    48

/** q3a_led_flash_state_t
 *
 *  This enumeration represents the state of the flash.
 **/
typedef enum {
  Q3A_LED_OFF,
  Q3A_LED_LOW,
  Q3A_LED_HIGH,
  Q3A_LED_INIT,
  Q3A_LED_RELEASE
} q3a_led_flash_state_t;

/** q3a_led_flash_mode_t
 *
 *  This enumeration represents different modes of the flash.
 **/
typedef enum {
  LED_MODE_OFF,
  LED_MODE_AUTO,
  LED_MODE_ON,
  LED_MODE_TORCH,
  /* New modes should be added above this line */
  LED_MODE_MAX
} q3a_led_flash_mode_t;

/** camera_bestshot_mode_type
 *
 *  This enumeration represents different bestshot (or scene) modes.
 **/
typedef enum {
  CAMERA_BESTSHOT_OFF = 0,
  CAMERA_BESTSHOT_AUTO = 1,
  CAMERA_BESTSHOT_LANDSCAPE = 2,
  CAMERA_BESTSHOT_SNOW,
  CAMERA_BESTSHOT_BEACH,
  CAMERA_BESTSHOT_SUNSET,
  CAMERA_BESTSHOT_NIGHT,
  CAMERA_BESTSHOT_PORTRAIT,
  CAMERA_BESTSHOT_BACKLIGHT,
  CAMERA_BESTSHOT_SPORTS,
  CAMERA_BESTSHOT_ANTISHAKE,
  CAMERA_BESTSHOT_FLOWERS,
  CAMERA_BESTSHOT_CANDLELIGHT,
  CAMERA_BESTSHOT_FIREWORKS,
  CAMERA_BESTSHOT_PARTY,
  CAMERA_BESTSHOT_NIGHT_PORTRAIT,
  CAMERA_BESTSHOT_THEATRE,
  CAMERA_BESTSHOT_ACTION,
  CAMERA_BESTSHOT_AR,
  CAMERA_BESTSHOT_MAX
} camera_bestshot_mode_type;

/** time_stamp_t
 *    @time_stamp_sec: The seconds component of the timestamp
 *    @time_stamp_us:  The microseconds component of the timestamp
 *
 *  This structure represents a standard OS timestamp, including two
 *  components - time in seconds and time in microseconds.
 **/
typedef struct {
  uint32_t time_stamp_sec; /*time stamp second*/
  uint32_t time_stamp_us;  /*time stamp microsecond*/
} time_stamp_t;

/** q3a_stream_crop_t
 *  vfe_map_x:      Left coordinate of the map
 *  vfe_map_y:      Top coordinate of the map
 *  vfe_map_width:  The width of the map
 *  vfe_map_height: The height of the map
 *  pp_x:           Left coordinate of the crop
 *  pp_y:           Top coordinate of the crop
 *  pp_crop_out_x:  Width of the crop window
 *  pp_crop_out_y:  Height of the crop window
 *  vfe_out_width:  The width on which the remapping is calculated
 *  vfe_out_height: The height on which the remapping is calculated
 *
 *  This structure is used to store the scale and crop information
 *  that is given by the ISP and CPP. The info is then used by the library
 *  to calculate the real ROI in terms of sensor resolution dimensions.
 **/
 typedef struct {
  uint32_t vfe_map_x; /* left */
  uint32_t vfe_map_y; /* top */
  uint32_t vfe_map_width; /* width */
  uint32_t vfe_map_height; /* height */
  uint32_t pp_x; /* left */
  uint32_t pp_y; /* top */
  uint32_t pp_crop_out_x; /* width */
  uint32_t pp_crop_out_y; /* height */
  uint32_t vfe_out_width;
  uint32_t vfe_out_height;
} q3a_stream_crop_t;

/** stats_roi_t
 *    @x:  The x coordinate of the ROI
 *    @y:  The y coordinate of the ROI
 *    @dx: The width of the ROI
 *    @dy: The height of the ROI
 *
 *  This structure is used to pass the ROI position and size to the ISP.
 *  ISP will use this parameters to calculate the stats for the specified
 *  region of interest.
 **/
typedef struct {
  int32_t x;
  int32_t y;
  int32_t dx;
  int32_t dy;
} stats_roi_t;

/** stats_extra_awb_stats_t
 *    @GLB_Y:     Y sum of pixels with Ymin < Y <Ymax
 *    @GLB_Cb:    Cb sum of pixels with Ymin < Y < Ymax
 *    @GLB_Cr:    Cr sum of pixels with Ymin < Y < Ymax
 *    @GLB_N:     Num of pixels in the global AWB statistics
 *    @Green_r:   R sum of green zone pixels with Ymin < Y < Ymax
 *    @Green_g:   G sum of green zone pixels with Ymin < Y < Ymax
 *    @Green_b:   B sum of green zone pixels with Ymin < Y < Ymax
 *    @Green_N:   Number of green zone pixels
 *    @ExtBlue_r: R sum of blue zone pixels with Ymin < Y < Ymax
 *    @ExtBlue_g: G sum of blue zone pixels with Ymin < Y < Ymax
 *    @ExtBlue_b: B sum of blue zone pixels with Ymin < Y < Ymax
 *    @ExtBlue_N: Number of blue zone pixels
 *    @ExtRed_r:  R sum of red zone pixels with Ymin < Y < Ymax
 *    @ExtRed_g:  G sum of red zone pixels with Ymin < Y < Ymax
 *    @ExtRed_b:  B sum of red zone pixels with Ymin < Y < Ymax
 *    @ExtRed_N:  Number of red zone pixels
 *
 *  This structure is used for extra AWB statistics
 **/
typedef struct {
  unsigned long GLB_Y;     /* Y sum of pixels with Ymin < Y < Ymax */
  unsigned long GLB_Cb;    /* Cb sum of pixels with Ymin < Y < Ymax */
  unsigned long GLB_Cr;    /* Cr sum of pixels with Ymin < Y < Ymax */
  unsigned long GLB_N;     /* Number of pixels in the global AWB statistics */

  unsigned long Green_r;   /* R sum of green zone pixels with Ymin < Y < Ymax */
  unsigned long Green_g;   /* G sum of green zone pixels with Ymin < Y<  Ymax */
  unsigned long Green_b;   /* B sum of green zone pixels with Ymin < Y < Ymax */
  unsigned long Green_N;   /* Number of green zone pixels */

  unsigned long ExtBlue_r; /* R sum of blue zone pixels with Ymin < Y < Ymax */
  unsigned long ExtBlue_g; /* G sum of blue zone pixels with Ymin < Y < Ymax */
  unsigned long ExtBlue_b; /* B sum of blue zone pixels with Ymin < Y < Ymax */
  unsigned long ExtBlue_N; /* Number of blue zone pixels */

  unsigned long ExtRed_r;  /* R sum of red zone pixels with Ymin < Y < Ymax */
  unsigned long ExtRed_g;  /* G sum of red zone pixels with Ymin < Y < Ymax */
  unsigned long ExtRed_b;  /* B sum of red zone pixels with Ymin < Y < Ymax */
  unsigned long ExtRed_N;  /* Number of red zone pixels*/
} stats_extra_awb_stats_t;

/** q3a_aec_stats_t
 *    @ae_region_h_num: Horizontal number of regions
 *    @ae_region_v_num: Vertical number of regions
 *    @SY:              Array of calculated luma sums for each region
 *
 *  This structure is used to pass the ROI position and size to the ISP.
 *  ISP will use this parameters to calculate the stats for the specified
 *  region of interest. The coordinates and dimension are in terms of
 *  Sensor resolution.
 **/
typedef struct {
  uint32_t  ae_region_h_num;            /* max 16 */
  uint32_t  ae_region_v_num;            /* max 16 */
  unsigned long SY[MAX_YUV_STATS_NUM];  /* 24 bits, sum of luminance in each
                                         * of the X regions */
} q3a_aec_stats_t;

/** q3a_af_stats_t
 *    @af_region_h_num: Horizontal number of regions
 *    @af_region_v_num: Vertical number of regions
 *    @focus_val:       Array of calculated focus values for each region
 *    @Focus:           Focus vale - only 27 bits used
 *    @NFocus:          Count of number of rows having - 9 bits used
 *
 *  This structure is used to pass the calculated AF stats from the ISP.
 **/
typedef struct {
  uint32_t af_region_h_num; /* max 9 */
  uint32_t af_region_v_num; /* max 9 */
  uint32_t focus_val[MAX_YUV_AF_STATS_NUM];
  unsigned long Focus;      /* 27 bits, focus value */
  unsigned long NFocus;     /* 9 bits, count of number of rows having */
}  q3a_af_stats_t;

/** q3a_awb_stats_t
 *    @wb_region_h_num: Horizontal number of regions
 *    @wb_region_v_num: Vertical number of regions
 *    @SCb:             24 bits, sum of Cb in each of the X regions for the
 *                      pixels that meet the inequalities
 *    @SCr:             24 bits, sum of Cr in each of the X regions for the
 *                      pixels that meet the inequalities
 *    @SY1:             24 bits, sum of Y in each of the X regions for the
 *                      pixels that meet the inequalities
 *    @NSCb:            16 bits, number of pixels included in each of the
 *                      X regions for the pixels that meet the inequalities
 *    @awb_extra_stats: The extra AWB stats for the frame
 *
 *  This structure is used to pass the calculated AWB stats from the ISP.
 **/
typedef struct {
  uint32_t wb_region_h_num;             /* max 16 */
  uint32_t wb_region_v_num;             /* max 16 */
  unsigned long SCb[MAX_YUV_STATS_NUM]; /* 24 bits, sum of Cb in each of the
                                         * X regions for the pixels that meet
                                         * the inequalities */
  unsigned long SCr[MAX_YUV_STATS_NUM]; /* 24 bits, sum of Cr in each of the
                                         * X regions for the pixels that meet
                                         * the inequalities */
  unsigned long SY1[MAX_YUV_STATS_NUM]; /* 24 bits, sum of Y in each of the
                                         * X regions for the pixels that meet
                                         * the inequalities */
  unsigned long NSCb[MAX_YUV_STATS_NUM]; /* 16 bits, number of pixels included
                                          * inSCb in each of the X regions for
                                          * the pixels that meet the
                                          * inequalities */
  stats_extra_awb_stats_t awb_extra_stats;
} q3a_awb_stats_t;

/** q3a_cs_stats_t
 *    @num_col_sum: The number of columns with sums
 *    @col_sum:     Array containing the column sums
 *
 *  This structure is used to pass the column stats from the ISP.
 **/
typedef struct {
  uint32_t num_col_sum;
  uint32_t col_sum[MAX_CS_STATS_NUM * 4];
} q3a_cs_stats_t;

/** q3a_rs_stats_t
 *    @num_row_sum: The number of rows with sums
 *    @row_sum:     Array containing the row sums
 *
 *  This structure is used to pass the row stats from the ISP.
 **/
typedef struct {
  uint32_t num_row_sum;
  uint32_t row_sum[MAX_RS_STATS_NUM * 8];
} q3a_rs_stats_t;

/** q3a_ihist_stats_t
 *    @histogram: Array containing the histogram values
 *
 *  This structure is used to pass the histogram stats from the ISP.
 **/
typedef struct {
  uint32_t histogram[MAX_HIST_STATS_NUM];
} q3a_ihist_stats_t;

/** q3a_be_stats_t
 *    @be_region_h_num: Horizontal number of regions for the BE
 *    @be_region_v_num: Vertical number of regions for the BE
 *    @be_r_sum:        Array with the sums for the red pixels
 *    @be_b_sum:        Array with the sums for the blue pixels
 *    @be_gr_sum:       Array with the sums for the green-red pixels
 *    @be_gb_sum:       Array with the sums for the green-blue pixels
 *    @be_r_num:
 *    @be_b_num:
 *    @be_gr_num:
 *    @be_gb_num:
 *
 *  This structure is used to pass the calculated BAYER exposure statistics
 *  from the ISP.
 **/
typedef struct {
  uint32_t be_region_h_num;
  uint32_t be_region_v_num;
  uint32_t be_r_sum[MAX_BE_STATS_NUM];
  uint32_t be_b_sum[MAX_BE_STATS_NUM];
  uint32_t be_gr_sum[MAX_BE_STATS_NUM];
  uint32_t be_gb_sum[MAX_BE_STATS_NUM];
  uint32_t be_r_num[MAX_BE_STATS_NUM];
  uint32_t be_b_num[MAX_BE_STATS_NUM];
  uint32_t be_gr_num[MAX_BE_STATS_NUM];
  uint32_t be_gb_num[MAX_BE_STATS_NUM];
} q3a_be_stats_t;


/** q3a_bg_stats_t
 *    @bg_region_h_num:  Horizontal number of regions for the BG
 *    @bg_region_v_num:  Vertical number of regions for the BG
 *    @region_pixel_cnt: The count of the region pixels
 *    @bg_r_sum:         Array with the sums for the red pixels
 *    @bg_b_sum:         Array with the sums for the blue pixels
 *    @bg_gr_sum:        Array with the sums for the green-red pixels
 *    @bg_gb_sum:        Array with the sums for the green-blue pixels
 *    @bg_r_num:
 *    @bg_b_num:
 *    @bg_gr_num:
 *    @bg_gb_num:
 *
 *
 *  This structure is used to pass the calculated BAYER grid statistics
 *  from the ISP, used by the AWB library.
 **/
typedef struct {
  uint32_t bg_region_h_num; /* 64, max 72 */
  uint32_t bg_region_v_num; /* 48, max 54 */
  uint32_t region_pixel_cnt;
  uint32_t bg_region_height;
  uint32_t bg_region_width;
  uint16_t rMax, bMax, grMax, gbMax;
  uint32_t bg_r_sum[MAX_BG_STATS_NUM];
  uint32_t bg_b_sum[MAX_BG_STATS_NUM];
  uint32_t bg_gr_sum[MAX_BG_STATS_NUM];
  uint32_t bg_gb_sum[MAX_BG_STATS_NUM];
  uint32_t bg_r_num[MAX_BG_STATS_NUM];
  uint32_t bg_b_num[MAX_BG_STATS_NUM];
  uint32_t bg_gr_num[MAX_BG_STATS_NUM];
  uint32_t bg_gb_num[MAX_BG_STATS_NUM];
} q3a_bg_stats_t;

/** q3a_bf_stats_t
 *    @bf_region_h_num: Horizontal number of regions for the BF
 *    @bf_region_v_num: Vertical number of regions for the BF
 *    @use_max_fv:
 *    @bf_r_sum:        Array with the sums for the red pixels
 *    @bf_b_sum:        Array with the sums for the blue pixels
 *    @bf_gr_sum:       Array with the sums for the green-red pixels
 *    @bf_gb_sum:       Array with the sums for the green-blue pixels
 *    @bf_r_sharp:      Array with the sums for the sharp red pixels
 *    @bf_b_sharp:      Array with the sums for the sharp blue pixels
 *    @bf_gr_sharp:     Array with the sums for the sharp green-red pixels
 *    @bf_gb_sharp:     Array with the sums for the sharp green-blue pixels
 *    @bf_r_num:
 *    @bf_b_num:
 *    @bf_gr_num:
 *    @bf_gb_num:
 *    @bf_r_max_fv:     Array with the max focus value for the red pixels
 *    @bf_b_max_fv:     Array with the max focus value for the blue pixels
 *    @bf_gr_max_fv:    Array with the max focus value for the green-red pixels
 *    @bf_gb_max_fv:    Array with the max focus value for the green-blue pixels
 *
 *  This structure is used to pass the calculated BAYER focus statistics
 *  from the ISP.
 **/
typedef struct {
  uint32_t bf_region_h_num;
  uint32_t bf_region_v_num;
  uint8_t  use_max_fv;
  uint32_t bf_r_sum[MAX_BF_STATS_NUM];
  uint32_t bf_b_sum[MAX_BF_STATS_NUM];
  uint32_t bf_gr_sum[MAX_BF_STATS_NUM];
  uint32_t bf_gb_sum[MAX_BF_STATS_NUM];
  uint32_t bf_r_sharp[MAX_BF_STATS_NUM];
  uint32_t bf_b_sharp[MAX_BF_STATS_NUM];
  uint32_t bf_gr_sharp[MAX_BF_STATS_NUM];
  uint32_t bf_gb_sharp[MAX_BF_STATS_NUM];
  uint32_t bf_r_num[MAX_BF_STATS_NUM];
  uint32_t bf_b_num[MAX_BF_STATS_NUM];
  uint32_t bf_gr_num[MAX_BF_STATS_NUM];
  uint32_t bf_gb_num[MAX_BF_STATS_NUM];
  uint32_t bf_r_max_fv[MAX_BF_STATS_NUM];
  uint32_t bf_b_max_fv[MAX_BF_STATS_NUM];
  uint32_t bf_gr_max_fv[MAX_BF_STATS_NUM];
  uint32_t bf_gb_max_fv[MAX_BF_STATS_NUM];
} q3a_bf_stats_t;

/** q3a_bhist_stats_t
 *    @bayer_r_hist:  Array containing the red histogram values
 *    @bayer_b_hist:  Array containing the blue histogram values
 *    @bayer_gr_hist: Array containing the green-red histogram values
 *    @bayer_gb_hist: Array containing the green-blue histogram values
 *
 *  This structure is used to pass the BAYER histogram statistics from the ISP.
 **/
typedef struct {
  uint32_t bayer_r_hist[MAX_HIST_STATS_NUM];
  uint32_t bayer_b_hist[MAX_HIST_STATS_NUM];
  uint32_t bayer_gr_hist[MAX_HIST_STATS_NUM];
  uint32_t bayer_gb_hist[MAX_HIST_STATS_NUM];
}q3a_bhist_stats_t;


/** q3a_strobe_confg_st_type
 *
 *  This enumeration represents the configuration type for the strobe.
 **/
typedef enum {
  STROBE_NOT_NEEDED,
  STROBE_TO_BE_CONFIGURED,
  STROBE_PRE_CONFIGURED,
  STROBE_PRE_FIRED,
  STROBE_SNAP_CONFIGURED,
} q3a_strobe_confg_st_type;

#endif /* __Q3A_STATS_HW_H__ */
