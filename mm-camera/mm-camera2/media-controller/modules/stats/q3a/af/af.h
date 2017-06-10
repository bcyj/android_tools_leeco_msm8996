/* af.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AF_H__
#define __AF_H__

#include "modules.h"
#include "q3a_stats.h"
#include "chromatix_common.h"
#include "mct_event_stats.h"
#include "af_algo_tuning.h"

/* TBD: Need to include proper header for this */
#undef  CDBG_ERROR
#define CDBG_ERROR ALOGE

#define AF_COLLECTION_POINTS             50

/* TBD: Number of grids currently supported. For bayer max is 14x18.
   YUV it's 9x9.*/
#define USE_MULTI_WINDOW                  1
#define NUM_AUTOFOCUS_HORIZONTAL_GRID     5
#define NUM_AUTOFOCUS_VERTICAL_GRID       5
#define NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS  20


#ifdef AF_2X13_FILTER_SUPPORT
#ifndef MAX_HPF_COEFF
#define MAX_HPF_COEFF              26
#endif
#else
#ifndef MAX_HPF_COEFF
#define MAX_HPF_COEFF              10
#endif
#endif


#define MAX_SWAF_COEFFA_NUM                6
#define MAX_SWAF_COEFFB_NUM                6
#define MAX_SWAF_COEFFFIR_NUM              11


/* Additional Tuning Param */

// some customer wants to enable it.
#define AF_CAF_TRIGGER_AFTER_TAF 0


/** af_run_mode_type:
 *
 * Enum to distinguish if it's camcorder or camera mode.
 *
 **/
typedef enum {
  AF_RUN_MODE_INIT,
  AF_RUN_MODE_CAMERA,
  AF_RUN_MODE_VIDEO,
  AF_RUN_MODE_SNAPSHOT
} af_run_mode_type;

/** af_move_direction_type:
 *
 * Direction to move
 *
 **/
typedef enum {
  AF_MOVE_NEAR, /* Move towards MACRO position */
  AF_MOVE_FAR,  /* Move towards INFY position */
} af_move_direction_type;

/** af_metering_type:
 *
 * List of AF metering types supported.
 *
 **/
typedef enum {
  AF_METER_AUTO = 0x0,
  AF_METER_SPOT,
  AF_METER_CTR_WEIGHTED,
  AF_METER_AVERAGE,
} af_metering_type;

/** focus_distance_index_type:
 *
 * Index of near/optimal/far focus distance in the focus_distance array.
 *
 **/
typedef enum {
  FOCUS_DISTANCE_NEAR_INDEX,
  FOCUS_DISTANCE_OPTIMAL_INDEX,
  FOCUS_DISTANCE_FAR_INDEX,
  FOCUS_DISTANCE_MAX_INDEX
} af_fd_index_type;

/** _focus_distances:
 *    @focus_distance: array of focus distances
 *
 * near/optimal/far focus distances
 **/
typedef struct _focus_distances {
  float focus_distance[FOCUS_DISTANCE_MAX_INDEX];
} af_focus_distances_t;


/** af_done_type:
 *
 * Type to indicate AF done event to send.
 *
 **/
typedef enum {
  AF_FAILED  = 0, /* AF is failed or rejected */
  AF_SUCCESS = 1, /* AF is sucessful */
  AF_ABORT   = 2, /* AF is aborted */
} af_done_type;

/** _af_done:
 *    @focus_done: boolean indicating if focusing is complete
 *    @status:     result of auto-focus - success/failure/aborted
 *
 * Result of focus done.
 **/
typedef struct _af_done {
  boolean      focus_done;
  af_done_type status;
} af_done_t;

/** af_status_type:
 *
 * Autofocus Status
 *
 **/
typedef enum {
  AF_STATUS_INVALID = -1,
  AF_STATUS_INIT = 0,
  AF_STATUS_FOCUSED,
  AF_STATUS_UNKNOWN,
  AF_STATUS_FOCUSING,
} af_status_type;

/** _af_status:
 *    @focus_done: boolean indicating if focusing is complete
 *    @status:     result of auto-focus - focused/unknown/focusing
 *    @f_distance: calculated focus distance
 *
 * AF status
 **/
typedef struct _af_status {
  boolean              focus_done;
  af_status_type       status;
  af_focus_distances_t f_distance;
  int32_t              focus_pos;
} af_status_t;

/** _af_move_lens:
 *    @move_lens:    flag to check if we should move lens
 *    @direction:    which direction to move
 *    @num_of_steps: number of steps to move
 *
 * Media Controller process return type
 **/
typedef struct _af_move_lens {
  boolean move_lens;
  int     direction;
  int     num_of_steps;
  boolean use_dac_value;
  unsigned short pos[MAX_NUMBER_OF_STEPS];
  unsigned short delay[MAX_NUMBER_OF_STEPS];
  unsigned short num_of_interval;
} af_move_lens_t;

/** _af_reset_lens:
 *    @reset_lens: flag to check if we should reset lens
 *    @reset_pos:  position to reset the lens to
 *    @cur_pos:    current lens position - for reference
 *
 * Reset lens to default position
 **/
typedef struct _af_reset_lens {
  boolean reset_lens;
  int     reset_pos;
  int     cur_pos;
} af_reset_lens_t;


/** af_mode_type:
 *
 * list of focus modes supported
 *
 **/
typedef enum {
  AF_MODE_NORMAL,        /* Normal AF mode - Infy to boundary */
  AF_MODE_MACRO,         /* Macro mode - search range Macro to Infy */
  AF_MODE_AUTO,          /* Currently same as Macro mode */
  AF_MODE_CAF,           /* Continuous AF mode - default */
  AF_MODE_CAF_NORMAL,    /* CAF with Normal mode - Infy to boundary */
  AF_MODE_CAF_MACRO,     /* CAF with Macro - Macro to Infy search range */
  AF_MODE_INFINITY,      /* Lens is fixed at Infinity */
  AF_MODE_MANUAL,
  AF_MODE_NOT_SUPPORTED, /* The mode is not supported */
  AF_MODE_MAX            /* The size of the enum */
} af_mode_type;

/** _af_mode_info:
 *    @mode:        current focus mode.
 *    @near_end:    nearest end of the AF search range
 *    @far_end:     farthest end of the AF search range
 *    @pan_pos:     pan position
 *    @infy_pos:    lens position with object at infinity
 *    @default_pos: default lens reset position
 *    @hyp_pos:     the hyperfocal position of the lens
 *
 * AF parameters particular to ceratain focus
 **/
typedef struct _af_mode_info {
  af_mode_type mode;
  af_mode_type prev_mode;
  int          near_end;
  int          far_end;
  int          default_pos;
  int          hyp_pos;
} af_mode_info_t;

/** _af_preview_size:
 *    @width:  width of the frame
 *    @height: height of the frame
 *
 * Preview size received from upper layer.
 **/
typedef struct _af_preview_size {
  int width;
  int height;
} af_preview_size_t;

/** af_roi_type:
 *
 * Enum to indicate what type of ROI information we have received.
 *
 **/
typedef enum {
  AF_ROI_TYPE_GENERAL = 0x0, /* Default */
  AF_ROI_TYPE_FACE,          /* Face priority AF */
  AF_ROI_TYPE_TOUCH,         /* Touch-AF */
} af_roi_type;

/** _af_roi:
 *    @x:   horizon tal offset of the region
 *    @y:   vertical offset of the region
 *    @dx:  width of the RoI region
 *    @dy:  height of the region
 *    @roi: structure holding dimensions of each ROI
 *
 * RoI information in terms of frame
 **/
typedef struct _af_roi {
  uint16_t x;
  uint16_t y;
  uint16_t dx;
  uint16_t dy;
} af_roi_t;


/** _af_roi_info:
 *    @roi_updated: check if ROI information has been updated
 *    @type:        ROI type - General/Touch/Face
 *    @frm_id:      frame ID
 *    @num_roi:     Number of ROIs detected
 *    @roi:         structure holding dimensions of each ROI
 *    @weight:      array holding the weights
 *
 * ROI selected for AF stats.
 **/
typedef struct _af_roi_info {
  boolean     roi_updated;
  af_roi_type type;
  uint32_t    frm_id;
  uint32_t    num_roi;
  af_roi_t    roi[MAX_STATS_ROI_NUM];
  uint32_t    weight[MAX_STATS_ROI_NUM];
} af_roi_info_t;

/** _af_crop_info: RoI information about scalar crop
*
*    @x: horizontal offset of the region
*
*    @y: vertical offset of the region
*
*    @dx: width of the crop region
*
*    @dy: height of the region
*
**/
typedef struct _af_crop_info{
  uint16_t x;
  uint16_t y;
  uint16_t dx;
  uint16_t dy;
  float zoom_ratio;
} af_crop_info_t;

/** _af_input_from_aec:
 *    @aec_settled:       flag to check if aec is settled now.
 *    @exp_index:         exposure index
 *    @pixels_per_region: TODO
 *    @comp_luma:         TODO
 *    @cur_luma:          current luma value
 *    @cur_real_gain:     TODO
 *    @lux_idx:           TODO
 *    @num_regions:       TODO
 *    @exp_tbl_val:       TODO
 *    @luma_settled_cnt:  TODO
 *    @SY:                TODO
 *
 * Data needed from AEC module for AF operation.
 **/
typedef struct _af_input_from_aec {
  int           aec_settled;
  int           exp_index;
  int           pixels_per_region;
  float         comp_luma;
  float         cur_luma;
  float         cur_real_gain;
  float         lux_idx;
  float         exp_time;
  int           preview_fps;
  int           preview_linesPerFrame;
  int           linecnt;
  float         target_luma;
  unsigned int  num_regions;
  unsigned int  exp_tbl_val;
  unsigned int  luma_settled_cnt;
  unsigned long SY[MAX_YUV_STATS_NUM];
  /*TDB : Apex information*/
  float Av_af;
  float Tv_af;
  float Sv_af;
  float Bv_af;
  float Ev_af;
} af_input_from_aec_t;

typedef enum {
  AF_MANUAL_FOCUS_MODE_INDEX,
  AF_MANUAL_FOCUS_MODE_DAC_CODE,
  AF_MANUAL_FOCUS_MODE_POS_RATIO,
  AF_MANUAL_FOCUS_MODE_DIOPTER,
} af_manual_focus_mode_type;


/** _af_input_manual_focus:
 *    @flag:      flag to check the mode of manual focus.

 *    @af_manual_lens_position_index:     Range (0-1000)
          User can provide index position directly if supported.

 *    @af_manual_lens_position_dac:     Range (0-1000)
           User can provide dac value of actuator directly if supported.

 *    @af_manual_lens_position_ratio:......range(0-100)
            currently supported mode where user input is mapped b/w
            near-end and far-end.

 *    @af_manual_diopter:       range(0-10)
            Diopter is 1/distance in meters ; distance to index mapping is done.

 *Manual lens movement information
 **/

typedef struct _af_input_manual_focus {
  af_manual_focus_mode_type flag;
  union{
    int af_manual_lens_position_index;
    int af_manual_lens_position_dac;
    int af_manual_lens_position_ratio;
    float af_manual_diopter;
  };

} af_input_manual_focus_t;

/** _af_input_from_isp:
 *    @width:  TODO
 *    @height: TODO
 *
 * Data needed from isp module for AF operation.
 **/
typedef struct _af_input_from_isp {
  unsigned int width;
  unsigned int height;
} af_input_from_isp_t;

/** _af_actuator_info:
 *    @focal_length:    focal length of lens (mm)
 *    @af_f_num:        numeric aperture of lens (mm)
 *    @af_f_pix:        pixel size (microns)
 *    @af_total_f_dist: Lens distance (microns)
 *    @hor_view_angle:  Sensor's horizontal view angle
 *    @ver_view_angle:  Sensor's vertical view angle
 *
 * information from actuator required for AF.
 **/
typedef struct _af_actuator_info{
  float focal_length;
  float af_f_num;
  float af_f_pix;
  float af_total_f_dist;
  float hor_view_angle;
  float ver_view_angle;
  float um_per_dac;
  int   dac_offset;
} af_actuator_info_t;

/** _af_input_from_sensor:
 *    @af_not_supported: Flag indicating if the sensor supports focus or not
 *    @preview_fps:      current preview fps
 *    @max_preview_fps:  maximum preview fps
 *    @actuator_info:    AF specific data from actuator
 *
 * Data needed from sensor module for AF operation.
 **/
typedef struct _af_input_from_sensor {
  boolean            af_not_supported;
  int                preview_fps;
  int                max_preview_fps;
  af_actuator_info_t actuator_info;
  uint32_t           sensor_res_height;
  uint32_t           sensor_res_width;
} af_input_from_sensor_t;

/** _af_input_from_gyro:
 *    @float_ready: float data is available
 *    @flt:         gyro metrics in float
 *    @q16_ready:   gyro metrics in q16 ready to use
 *    @q16:         gyro metrics in q16
 *
 * Gyro data required to assist AF.
**/
typedef struct _af_input_from_gyro {
  int   float_ready;
  float flt[3];
  int   q16_ready;
  long  q16[3];
} af_input_from_gyro_t;

typedef struct _af_output_eztune_data {
  int          peak_location_index;
  boolean      ez_running;
  boolean      enable;
  uint16_t     roi_left;
  uint16_t     roi_top;
  uint16_t     roi_width;
  uint16_t     roi_height;
  int          grid_info_h_num;
  int          grid_info_v_num;
  uint32_t     r_fv_min;
  uint32_t     gr_fv_min;
  uint32_t     gb_fv_min;
  uint32_t     b_fv_min;
  int          hpf[10];
  int          mode;
  int          status;
  int          far_end;
  int          near_end;
  int          hyp_pos;
  int          state;
  int          stats_index;
  int          stats_pos;
  int          stats_fv[50];
  int          stats_max_fv;
  int          stats_min_fv;
  int          frame_delay;
  int          enable_multiwindow;
  int          Mwin[14];
  int          num_downhill;
  int          caf_state;
  uint32_t     cur_luma;
  int          exp_index;
  unsigned int luma_settled_cnt;
  int          ave_fv;
  int          caf_panning_unstable_cnt;
  int          caf_panning_stable_cnt;
  int          caf_panning_stable;
  int          caf_sad_change;
  int          caf_exposure_change;
  int          caf_luma_chg_during_srch;
  int          caf_trig_refocus;
  int          caf_gyro_assisted_panning;
} af_output_eztune_data_t;

/** _af_eztune:
 *    @enable:        eztune is enabled/disabled.
 *    @peakpos_index: index to max FV position
 *    @tracing_index: current index in the array
 *    @tracing_stats: array of subsequent focus values
 *    @tracing_pos:   corresponding lens positions
 *    @eztune_data:   eztune variables
 *
 * some AF parameters for EZ Tune
**/
typedef struct _af_eztune {
  boolean                 enable;
  boolean                 running;
  int                     peakpos_index;
  int                     tracing_index;
  int                     tracing_stats[AF_COLLECTION_POINTS];
  int                     tracing_pos[AF_COLLECTION_POINTS];
  af_output_eztune_data_t eztune_data;
} af_eztune_t;

/** _af_stats_kernel_size: Enum to indicate AF kernel
 *  coefficient sizes.
 **/
typedef enum _af_stats_kernel_size {
  AF_STATS_HPF_LEGACY,
  AF_STATS_HPF_2x5,
  AF_STATS_HPF_2x13,
} af_stats_kernel_size_type;

/** _af_config_data: information required to configure VFE for
*   AF stats
*    @region: dimensions of each ROIs
*
*    @af_multi_nfocus: nfocus values for each region for
*                    MULTIPLE mode
*
*    @af_multi_roi_window: regions to consider for MULTIPLE mode
*
**/
typedef struct _af_config_data{
  af_roi_t     region;
  int          rgn_h_num;
  int          rgn_v_num;
  unsigned int r_min;
  unsigned int b_min;
  unsigned int gr_min;
  unsigned int gb_min;
  int          hpf[MAX_HPF_COEFF];
  af_roi_type  roi_type;
} af_config_data_t;

/** _af_mobicat:
 *    @enable:        mobicat is enabled/disabled.
 *    @peakpos_index: index to max FV position
 *    @tracing_index: current index in the array
 *    @tracing_stats: array of subsequent focus values
 *    @tracing_pos:   corresponding lens positions
 *    @eztune_data:   mobicat variables
 *
 * some AF parameters for mobicat
**/
typedef struct _af_mobicat {
  boolean                 enable;
  boolean                 running;
  int                     peakpos_index;
  int                     tracing_index;
  int                     tracing_stats[AF_COLLECTION_POINTS];
  int                     tracing_pos[AF_COLLECTION_POINTS];
  af_output_mobicat_data_t mobicat_data;
} af_mobicat_t;

/** swaf_config_t
 *   @enable:    Flag to enable/disable stats collection
 *   @frame_id:  current frame id
 *   @roi:       ROI of the stats collection
 *   @coeffa:    filterA coefficients
 *   @coeffb:    filterB coefficients
 *   @coeff_len: length of coefficient table
 *   @sw_filter_type: filter type
 *
 *   Imglib preview assisted AF coefficients
 **/
typedef struct {
  int8_t     enable;
  int        frame_id;
  af_roi_t   roi;
  double     coeffa[MAX_SWAF_COEFFA_NUM];
  double     coeffb[MAX_SWAF_COEFFB_NUM];
  uint32_t   coeff_len;
  int        coeff_fir[MAX_SWAF_COEFFFIR_NUM];
  double     fv_min;
  af_sw_filter_type sw_filter_type;
} swaf_config_data_t;

/** af_imglib_output_t
 *   @frame_id: Frame ID out the AF output
 *   @fV: Focus value
 *   @pending: Flag to indicate the fv value has been query but
 *           missing.
 *
 *   Imglib preview assisted AF output
 **/
typedef struct {
  int     frame_id;
  double  fV;
  uint8_t pending;
} af_sw_stats_t;

/** af_bestshot_mode_type:
 *  List of scene modes supported.
 **/
typedef enum {
  AF_SCENE_MODE_OFF,
  AF_SCENE_MODE_AUTO,
  AF_SCENE_MODE_LANDSCAPE,
  AF_SCENE_MODE_SNOW,
  AF_SCENE_MODE_BEACH,
  AF_SCENE_MODE_SUNSET,
  AF_SCENE_MODE_NIGHT,
  AF_SCENE_MODE_PORTRAIT,
  AF_SCENE_MODE_BACKLIGHT,
  AF_SCENE_MODE_SPORTS,
  AF_SCENE_MODE_ANTISHAKE,
  AF_SCENE_MODE_FLOWERS,
  AF_SCENE_MODE_CANDLELIGHT,
  AF_SCENE_MODE_FIREWORKS,
  AF_SCENE_MODE_PARTY,
  AF_SCENE_MODE_NIGHT_PORTRAIT,
  AF_SCENE_MODE_THEATRE,
  AF_SCENE_MODE_ACTION,
  AF_SCENE_MODE_AR,
  AF_SCENE_MODE_MAX
} af_bestshot_mode_type;

/** _af_bestshot_data:
 *    @enable:    TODO
 *    @srch_mode: focus mode to be used
 *    @curr_mode: bestshot mode currently selected
 *
 * AF related informated for bestshot mode selected.
**/
typedef struct _af_bestshot_data {
  boolean               enable;
  uint32_t              srch_mode;
  af_bestshot_mode_type curr_mode;
} af_bestshot_data_t;

/** _af_stream_crop:
 *    @vfe_map_x:      left
 *    @vfe_map_y:      top
 *    @vfe_map_width:  width
 *    @vfe_map_height: height
 *    @pp_x:           left
 *    @pp_y:           top
 *    @pp_crop_out_x:  width
 *    @pp_crop_out_y:  height
 *
 * TODO
 **/
typedef struct _af_stream_crop {
  uint32_t vfe_map_x;
  uint32_t vfe_map_y;
  uint32_t vfe_map_width;
  uint32_t vfe_map_height;
  uint32_t pp_x;
  uint32_t pp_y;
  uint32_t pp_crop_out_x;
  uint32_t pp_crop_out_y;
} af_stream_crop_t;

/** _af_move_lens_cb:
 *    @object_id:    pointer to the AF port
 *    @move_lens_cb: function pointer to the callback function in AF port
 *
 * This structure is used to pass the move lens callback function information
 * to the library.
 **/
typedef struct _af_move_lens_cb {
  int32_t object_id;
  boolean (*move_lens_cb)(void *output, int32_t object_id);
} af_move_lens_cb_t;

typedef q3a_operation_mode_t af_operation_mode_t;

/** _af_tunable_params:
 *    @af_caf_trigger_after_taf:  tar to caf trigger behaviour
**/
typedef struct _af_tunable_params {
  uint32_t                af_caf_trigger_after_taf;
} af_tuning_params_t;

/** _af_set_parameter_init:
 *    @chromatix:      pointer to chromatix
 *    @comm_chromatix: pointer to common chromatix parameters
 *    @tuning_info:    af tuning header pointer
 *    @op_mode:        camera operation mode - video/camera/snapshot
 *                     it can be derived from stream info
 *    @preview_size:   current preview size
 *    @af_tuning_params: additional tuning param
 * Parameters that need to be set during initialization.
 **/
typedef struct _af_set_parameter_init{
  void              *chromatix;
  void              *comm_chromatix;
  void              *tuning_info;
  af_run_mode_type  op_mode;
  af_preview_size_t preview_size;
  af_tuning_params_t  af_tuning_params;
} af_set_parameter_init_t;


/** af_set_parameter_type:
 * List of AF parameters that can be used by other componenets to set.
 **/
typedef enum {
  AF_SET_PARAM_INIT_CHROMATIX_PTR = 1, /* Initialize chromatix */
  AF_SET_PARAM_UPDATE_TUNING_HDR,      /* Update AF tuning header */
  AF_SET_PARAM_INIT,                   /* Init AF data */
  AF_SET_PARAM_RESET_LENS,             /* Reset lens */
  AF_SET_PARAM_METERING_MODE,          /* Metering mode */
                                       /* - Auto/Spot/Average/Center Weighted */
  AF_SET_PARAM_START,                  /* Start AF */
  AF_SET_PARAM_MOVE_LENS,              /* Move lens */
  AF_SET_PARAM_LENS_MOVE_DONE,         /* Lens move request completed */
  AF_SET_PARAM_FOCUS_MODE,             /* Change focus mode */
  AF_SET_PARAM_FOCUS_MANUAL_POSITION,
  AF_SET_PARAM_CANCEL_FOCUS,           /* Cancel auto focus */
  AF_SET_PARAM_STOP_FOCUS,             /* Stop auto-focus */
  AF_SET_PARAM_BESTSHOT,               /* Set required AF parameters */
                                       /* different for bestshot modes */
  AF_SET_PARAM_ROI,                    /* ROI information */
                                       /* - in case of Touch/Face */
  AF_SET_PARAM_LOCK_CAF,               /* Request to lock CAF till unlocked */
  AF_SET_PARAM_EZ_ENABLE,              /* Enable/disable eztune */
  AF_SET_PARAM_UPDATE_AEC_INFO,        /* Update aec info */
  AF_SET_PARAM_UPDATE_SENSOR_INFO,     /* Update sensor info */
  AF_SET_PARAM_UPDATE_GYRO_INFO,       /* Update gyro data */
  AF_SET_PARAM_UPDATE_ISP_INFO,        /* Update isp info */
  AF_SET_PARAM_STREAM_CROP_INFO,       /* Update stream crop info */
  AF_SET_PARAM_RUN_MODE,               /* Camera run mode
                                        * - Video/Camera/Snapshot */
  AF_SET_PARAM_MOVE_LENS_CB,           /* Set move lens callback function */
  AF_SET_PARAM_STATS_DEBUG_MASK,       /* Set the stats debug mask to algorithm lib*/
  AF_SET_PARAM_EZ_TUNE_RUNNING,        /* Set the state of the eztune */
  AF_SET_PARAM_SUPER_EVT,             /* Super evt id */
  AF_SET_PARAM_SOF,                   /* Set sof id */
  AF_SET_PARAM_META_MODE,
  AF_SET_PARAM_WAIT_FOR_AEC_EST,      /* Wait for AEC to complete estimation when LED is ON */
  AF_SET_PARAM_RESET_CAF,             /* Reset CAF to make it start from the beginning */
  AF_SET_PARAM_CROP_REGION,           /* Update Scalar crop info */
  AF_SET_PARAM_IMGLIB_OUTPUT,         /* SW stats output*/
  AF_SET_PARAM_HFR_MODE,              /* HFR mode */
  AF_SET_PARAM_SW_STATS_POINTER,      /* SW stats backup pointer*/
  AF_SET_PARAM_UPDATE_KERNEL_SIZE,    /* HPF size currently supported */
  AF_SET_PARAM_RECONFIG_ISP,          /* Request AF algo to reconfigure ISP */
  AF_SET_PARAM_MAX = 0xFF              /* TODO */
} af_set_parameter_type;

/** _af_set_parameter: Used for setting AF parameters
 *    @type:              parameter type as listed by af_set_parameter_type
 *    @current_frame_id:  SOF id wrt to set param
 *    @af_set_data:       TODO
 *    @af_init_param:     init AF
 *    @af_lens_move_done: TODO
 *    @af_run_mode:       TODO
 *    @af_metering_mode:  Metering mode -Auto/Spot/Average/CenterWeighted
 *    @af_steps_to_move:  number of steps to move lens
 *    @af_mode:           focus mode - Auto/Macro/Normal
 *    @af_cont_focus:     enable/disable continuous auto-focus
 *    @af_lock_caf:       enable/disable CAF lock. Till CAF is locked
 *                        scene-change wouldn't trigger new search.
 *    @af_ez_disable:     enable/disable eztune logging for CAF
 *    @af_roi_info:       contents hold AF ROI information
 *    @aec_info:          TODO
 *    @sensor_info:       TODO
 *    @gyro_info:         TODO
 *    @isp_info:          TODO
 *    @af_bestshot_mode:  TODO
 *    @stream_crop:       TODO
 *    @move_lens_cb_info: TODO
 *
 * Used for setting AF parameters
 **/
typedef struct _af_set_parameter {
  af_set_parameter_type type;
  unsigned int          current_frame_id;
  union {
    boolean                   af_set_data;
    af_set_parameter_init_t   af_init_param;
    boolean                   af_lens_move_done;
    af_run_mode_type          af_run_mode;
    int                       af_metering_mode;
    int                       af_steps_to_move;
    int                       af_mode;
    int                       af_cont_focus;
    boolean                   af_lock_caf;
    int                       af_ez_enable;
    af_roi_info_t             af_roi_info;
    af_input_manual_focus_t   af_manual_focus_info;
    af_input_from_aec_t       aec_info;
    af_input_from_sensor_t    sensor_info;
    af_input_from_gyro_t      gyro_info;
    af_input_from_isp_t       isp_info;
    af_sw_stats_t             sw_stats;
    af_sw_stats_t             *p_sw_stats;
    camera_bestshot_mode_type af_bestshot_mode;
    af_stream_crop_t          stream_crop;
    af_move_lens_cb_t         move_lens_cb_info;
    uint32_t                  stats_debug_mask;
    boolean                   ez_running;
    int                       af_trigger_id;
    uint32_t                  af_set_sof_id;
    uint32_t                  af_set_parm_id;
    uint8_t                   af_set_meta_mode;
    int                       af_wait_for_aec_est;
    af_crop_info_t            crop_info;
    int32_t                   hfr_mode;
    int                       kernel_size;
  } u;
} af_set_parameter_t;

/** af_get_parameter_type:
 * List of AF parameters that can be
 * requested by other components.
 **/
typedef enum {
  AF_GET_PARAM_FOCUS_DISTANCES,       /* focus distances - near/far/optimal */
  AF_GET_PARAM_CUR_LENS_POSITION,     /* current lens position */
  AF_GET_PARAM_DEFAULT_LENS_POSITION, /* default reset position */
  AF_GET_PARAM_STATUS,                /* AF status - Focused/Unknown/Focusing */
  AF_GET_PARAM_STATS_CONFIG_INFO,     /* Stats configuration data */
  AF_GET_PARAM_FOCUS_MODE,            /* Autofocus mode */
  AF_GET_PARAM_MOBICAT_INFO,          /* Mobicat info */
  AF_GET_PARAM_SW_STATS_FILTER_TYPE,  /* SW Stats filter type */
} af_get_parameter_type;

/** _af_get_parameter:
 *    @type:              parameter type as listed by af_get_parameter_type
 *    @af_cur_lens_pos:   current lens position
 *    @af_def_lens_pos:   default lens reset position
 *    @af_status:         AF status - Focused/Unknown/Focusing
 *    @af_mode:           focus mode - Auto/Macro/Normal
 *    @af_focus_distance: focus distances
 *    @af_stats_config:   Stats configuration data
 *
 * Interface to get access to AF parameters from outside.
 **/
typedef struct _af_get_parameter {
  af_get_parameter_type  type;
  union {
    int                  af_cur_lens_pos;
    int                  af_def_lens_pos;
    int                  af_status;
    int                  af_mode;
    af_focus_distances_t af_focus_distance;
    af_config_data_t     af_stats_config;
    af_sw_filter_type    af_sw_stats_filter_type;
  } u;
} af_get_parameter_t;

/** af_output_type
 * TODO
 **/
typedef enum {
  AF_OUTPUT_STOP_AF        = 1,
  AF_OUTPUT_CHECK_LED      = (1 << 1),
  AF_OUTPUT_STATUS         = (1 << 2),
  AF_OUTPUT_RESET_LENS     = (1 << 3),
  AF_OUTPUT_MOVE_LENS      = (1 << 4),
  AF_OUTPUT_FOCUS_MODE     = (1 << 5),
  AF_OUTPUT_ROI_INFO       = (1 << 6),
  AF_OUTPUT_EZTUNE         = (1 << 7),
  AF_OUTPUT_STATS_CONFIG   = (1 << 8),
  AF_OUTPUT_EZ_METADATA    = (1 << 9),
  AF_OUTPUT_MOBICAT_METADATA = (1 << 10),
  AF_OUTPUT_SWAF_CONFIG    = (1 << 11),
  AF_OUTPUT_RESET_AEC      = (1 << 12),
  AF_OUTPUT_UPDATE_EVENT   = (1 << 13),
  AF_OUTPUT_UPDATE_FOCUS_POS = (1 << 14),
} af_output_type;

/* structure to have current lens position in terms of scale and diopter */
typedef struct _af_curr_lens_pos{
  uint32_t scale;
  float diopter ;
} af_curr_lens_pos;


/** _af_output_data:
 *    @result:          TODO
 *    @type:            TODO
 *    @stop_af:         if true AF needs to be stopped.
 *    @check_led:       ask AEC to check if LED needs to be turned on
 *    @focus_mode_info: parameters specific to each focus mode
 *    @reset_lens:      reset the lens if required
 *    @move_lens:       move the lens by number of steps if requested
 *    @focus_status:    AF status that needs to be sent
 *    @roi_info:        TODO
 *    @eztune:          eztune data
 *    @af_stats_config: Stats configuration data
 *    @asf_config:      configure software AF in the imglib
 *
 * Output AF data for other components to consume once AF stat is processed.
 **/
typedef struct _af_output_data {
  uint32_t         frame_id;
  boolean          result;
  af_output_type   type;
  boolean          stop_af;
  boolean          check_led;
  af_mode_info_t   focus_mode_info;
  af_reset_lens_t  reset_lens;
  af_move_lens_t   move_lens;
  af_status_t      focus_status;
  af_roi_info_t    roi_info;
  af_eztune_t      eztune;
  af_mobicat_t     mobicat;
  af_config_data_t af_stats_config;
  uint32_t         sof_id;
  swaf_config_data_t swaf_config;
  char             af_debug_data_array[AF_DEBUG_DATA_SIZE];
  uint32_t         af_debug_data_size;
  af_curr_lens_pos cur_lens_pos;
} af_output_data_t;


typedef boolean (* af_set_parameters_func)(af_set_parameter_t *param,
  af_output_data_t *output, void *af_obj);
typedef boolean (* af_get_parameters_func)(af_get_parameter_t *param,
  void *af_obj);

typedef void (* af_process_func)(stats_af_t *stats, af_output_data_t *output,
  void *af);
typedef void (* af_callback_func)(af_output_data_t *output, void *port);

/** _af_ops:
 *    @set_parameters: function pointer to set parameters
 *    @get_parameters: function pointer to get parameters
 *    @process:        function pointer to process stats
 *
 * Operation table open to external component
 **/
typedef struct _af_ops {
  af_set_parameters_func set_parameters;
  af_get_parameters_func get_parameters;
  af_process_func        process;
}af_ops_t;

void *  af_init(af_ops_t *af_ops);
boolean af_set_parameters(af_set_parameter_t *param, af_output_data_t *out,
  void *af);
boolean af_get_parameters(af_get_parameter_t *param, void *af);
void    af_process(stats_af_t *stats, af_output_data_t *output, void *af_obj);
void    af_destroy(void *af_obj);

#endif /* __AF_H__ */
