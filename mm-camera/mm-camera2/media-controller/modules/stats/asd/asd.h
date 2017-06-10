/* asd.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ASD_H__
#define __ASD_H__

#include "q3a_stats.h"

#define ASD_AEC_STATS_NUM_MAX 256
#define ASD_UI_UPDATE_FRAME_INTERVAL 10

typedef q3a_operation_mode_t asd_operation_mode_t;

/** asd_get_parameter_type: type of asd get parameters
**/
typedef enum {
  ASD_GET_PARAMS,
  ASD_GET_MAX
} asd_get_parameter_type;

/** asd_set_parameter_type:
*   type of parameters that can be set for ASD.
**/
typedef enum {
  ASD_SET_PARAM_INIT_CHROMATIX   = 1,
  ASD_SET_ENABLE,
  ASD_SET_BESTSHOT,
  ASD_SET_UI_FRAME_DIM,
  ASD_SET_STATS_DEBUG_MASK,
  ASD_SET_PARAM_OP_MODE,
  ASD_SET_PARAM_MAX
} asd_set_parameter_type;

typedef enum {
  ASD_UPDATE = 1,
  ASD_SEND_EVENT,
} asd_output_type_t;

/** _asd_get_params:
*
*    @landscape_severity: degree of landscape content in the
*                       scene
*
*    @backlight_detected: true if backlight detected

*    @backlight_scene_severity: degree by which backlight
*                             adjustment needs to be done
*
*    @portrait_severity: degree of portrait content in the scene
*
*    @asd_soft_focus_dgr: degree of softness to be applied for
*                       portrait.
*
**/
typedef struct _asd_get_params {
  unsigned int landscape_severity;
  unsigned int backlight_detected;
  unsigned int backlight_scene_severity;
  unsigned int portrait_severity;
  float asd_soft_focus_dgr;
} asd_get_params_t;

/** _asd_get_parameter: ASD get parameters request structure

*    @type: parameter type as listed by asd_get_parameter_type
*
*    @asd_params: asd data specific to get parameter type.
**/
typedef struct _asd_get_parameter {
  asd_get_parameter_type type;

  union {
    asd_get_params_t asd_params;
  } u;
} asd_get_parameter_t;


/** asd_set_parameter_init_t: Initial parameter that needs to be
*   passed to ASD.

*    @chromatix: pointer to chromatix
*    @preview_width/height - used to check portrait severity
**/
typedef struct {
  void *chromatix;
  void *comm_chromatix;
  uint32_t preview_width;
  uint32_t preview_height;
  asd_operation_mode_t op_mode;
} asd_set_parameter_init_t;

/** _asd_set_parameter: Used for setting ASD parameters

*    @type: parameter type as listed by asd_set_parameter_type
*
*    @init_param: ASD initialization parameters

*    @asd_enable: enable/disable ASD.
*
*    @bestshot_mode: bestshot mode currently selected.
**/
typedef struct _asd_set_parameter {
  asd_set_parameter_type type;

  union {
    asd_set_parameter_init_t init_param;
    boolean                  asd_enable;
    uint32_t                 stats_debug_mask;
  } u;
} asd_set_parameter_t;

/** _asd_output_data: Output of ASD processing
*
*    @backlight_luma_target_offset: target luma offset for
*                                 backlight
*
*    @snow_or_cloudy_luma_target_offset: target luma offset for
*                                      snow or cloudy detection

*    @backlight_detected: true if backlight is detected
*
*    @backlight_scene_severity: degree of backlight detection
*
*    @snow_or_cloudy_scene_detected: snow or cloudy scene
*                                  detected
*
*    @histo_backlight_detected: backlight detected using
*                             histogram
*
*    @landscape_severity: degree of landscape detection
*
*    @portrait_severity: degree of portrait detected in the
*                      scene
*
*    @soft_focus_dgr: degree of softness for portrait
*
*    @mixed_light: true if it's a mixed light
*
*    @scene_t - enum for all scenes
*    @severity[] - array of severity of each scene type
**/
typedef struct _asd_output_data {
  asd_output_type_t type;
  uint32_t backlight_luma_target_offset;
  uint32_t snow_or_cloudy_luma_target_offset;
  boolean  backlight_detected;
  uint32_t backlight_scene_severity;
  boolean  snow_or_cloudy_scene_detected;
  boolean  histo_backlight_detected;
  uint32_t landscape_severity;
  uint32_t portrait_severity;
  float    soft_focus_dgr;
  boolean  mixed_light;
  cam_auto_scene_t scene;
  uint32_t severity[S_MAX];
  uint32_t is_hdr_scene;
  float    hdr_confidence;
  boolean  asd_enable;
  char     asd_debug_data_array[ASD_DEBUG_DATA_SIZE];
  uint32_t asd_debug_data_size;
} asd_output_data_t;

/** asd_roi_type: type of ROI received
**/
typedef enum {
  ASD_ROI_TYPE_GENERAL,
  ASD_ROI_TYPE_FACE,
} asd_roi_type_t;

/** _asd_roi_info: ROI information received from AEC.

*    @type: type of ROI
*
*    @roi_updated: if ROI has been updated, set to TRUE

*    @frm_width: preview frame width
*
*    @frm_height: preview frame height
*
*    @num_roi: number of ROIs
*
*    @roi: array of detected ROIs
**/
typedef struct _asd_roi_info {
  asd_roi_type_t type;
  boolean        roi_updated;
  unsigned int   frm_width;
  unsigned int   frm_height;
  unsigned int   num_roi;
  stats_roi_t    roi[MAX_STATS_ROI_NUM];
} asd_roi_info_t;

/** _asd_data_from_aec: AEC updates required by ASD.
*
*    @SY: luma sum
*
*    @num_of_regions: number of retions
*
*    @pixels_per_region: pixels in each region
*
*    @target_luma: target luma value
*
*    @comp_luma: compensated luma value
*
*    @exp_index: exposure index
*
*    @exp_tbl_val: maximum exposure index table size
*
*    @roi: ROIs detected
*
*    @extreme_green_cnt: extreme green count
*
*    @extreme_blue_cnt: extreme blue count
*
*    @extreme_tot_regions: total number of regions
*
*    @lux_idx: current lux index
**/
/* Below should come from AEC update */
typedef struct _asd_data_from_aec {
  unsigned int SY[ASD_AEC_STATS_NUM_MAX];
  unsigned int num_of_regions;
  int pixels_per_region;
  unsigned int target_luma;
  int comp_luma;
  int exp_index;
  unsigned int exp_tbl_val;
  asd_roi_info_t roi_info;
  int extreme_green_cnt;
  int extreme_blue_cnt;
  int extreme_tot_regions;
  float lux_idx;
} asd_data_from_aec_t;

/** _asd_data_from_awb: Data required by ASD from AWB

*    @awb_asd_sync_flag:
*
*    @stat_index_mapping:

*    @stat_sample_decision:
*
*    @grey_world_stats:
**/
typedef struct _asd_data_from_awb {
  int      stat_sample_decision[64];
  uint32_t grey_world_stats;
} asd_data_from_awb_t;

/*
  This struct is a mirror copy of mct_face_data_t.
  Please keep it updated.
  Unfortunately, the latter structure is not accessible
  from this header file..
*/
typedef struct {
  cam_rect_t roi;
  int8_t score;
} asd_face_data_t;

typedef struct _asd_data_face_info_t {
  //use_roi is a confidence flag
  //thats meant to be set when a pre-defined number
  //of face info messages have been received
  //consecutively... this is a means to imply
  //the face detection process has settled.
  boolean use_roi;

  //most recent roi
  uint8_t face_count;
  asd_face_data_t faces[MAX_ROI];
  //ignore frame_id for now
} asd_data_face_info_t;


/** asd_process_asd_t: enum to define current ASD process state
*
* ASD_NO_ACTION - no action
*
* ASD_AEC_UPDATED - AEC data has been updated
*
* ASD_AWB_UPDATED - AWB data has been updated
*
* @ASD_ASD_PROCESS - both AEC/AWB has been updated.ASC can be
*                  processed now.
**/
typedef enum {
  ASD_NO_ACTION  = 0,
  ASD_AEC_UPDATED = (1 << 1),
  ASD_AWB_UPDATED = (1 << 2),
  ASD_FACE_INFO_UPDATED = (1 << 3), //optional for process
  ASD_ASD_PROCESS = (ASD_AEC_UPDATED | ASD_AWB_UPDATED),
  ASD_INVALID
} asd_process_asd_type;

/** _asd_process_data: Data required to processed ASD

*    @aec_data: data from AEC
*
*    @awb_data: input from AWB
**/
typedef struct _asd_process_data {
  stats_t stats;
  asd_data_from_aec_t aec_data;
  asd_data_from_awb_t awb_data;
  asd_data_face_info_t face_data;
  uint64_t frame_count;
  uint64_t ui_update_count;
} asd_process_data_t;


typedef boolean(*asd_set_parameters_func)(asd_set_parameter_t *param,
  void *asd_obj);

typedef boolean(*asd_get_parameters_func)(asd_get_parameter_t *param,
  void *asd_obj);

typedef void    (*asd_process_func)(asd_process_data_t *stats, void *asd_obj,
  asd_output_data_t *output);

typedef void (*asd_callback_func)(asd_output_data_t *output,
  void *port);

typedef struct {
  asd_set_parameters_func set_parameters;
  asd_get_parameters_func get_parameters;
  asd_process_func        process;
}asd_ops_t;


void* asd_init(asd_ops_t *asd_ops);
boolean asd_set_parameters(asd_set_parameter_t *param,
  void *asd);
boolean asd_get_parameters(asd_get_parameter_t *param, void *asd);
void asd_process(asd_process_data_t *data, void *asd, asd_output_data_t *output);
void asd_destroy(void *asd);

#endif /* __ASD_H__ */
