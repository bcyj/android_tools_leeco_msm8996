/* mct_pipeline.h
 *
 * Copyright (c) 2012, 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_PIPELINE_H__
#define __MCT_PIPELINE_H__

#include "mct_object.h"
#include "mct_stream.h"

/* Maximum number of resolution for non HFR and HFR mode */
#define SENSOR_MAX_RESOLUTION 10
#define TOTAL_RAM_SIZE_512MB 536870912
#define PICTURE_SIZE_5MP_WIDTH 2592
#define PICTURE_SIZE_5MP_HEIGHT 1944

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  cam_dimension_t  dim;
  cam_fps_range_t fps;
  int32_t mode;
} mct_pipeline_sensor_res_table_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/

typedef enum {
 FORMAT_BAYER,
 FORMAT_YCBCR,
} mct_sensor_format_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  int                             modes_supported;
  mct_sensor_format_t             sensor_format;
  cam_position_t                  position;
  uint32_t                        sensor_mount_angle;
  float                           focal_length;
  float                           hor_view_angle;
  float                           ver_view_angle;
  int32_t                         af_supported;
  int32_t                         ae_lock_supported;
  int32_t                         wb_lock_supported;
  int32_t                         scene_mode_supported;
  uint8_t                         dim_fps_table_count;
  mct_pipeline_sensor_res_table_t dim_fps_table[SENSOR_MAX_RESOLUTION];
  uint8_t                         supported_effects_cnt;
  cam_effect_mode_type            supported_effects[CAM_EFFECT_MODE_MAX];
  uint32_t                        is_flash_supported;
  uint8_t                         supported_flash_modes_cnt;
  cam_flash_mode_t                supported_flash_modes[CAM_FLASH_MODE_MAX];
  uint8_t                         supported_focus_modes_cnt;
  cam_focus_mode_type             supported_focus_modes[CAM_FOCUS_MODE_MAX];
  uint32_t                        supported_raw_fmts_cnt;
  cam_format_t                    supported_raw_fmts[CAM_FORMAT_MAX];
  uint32_t                        feature_mask;
  uint32_t                        max_pipeline_frame_delay;
  uint32_t                        max_frame_delay;
  float                           min_focus_distance;
  float                           hyper_focal_distance;
  float                           focal_lengths[CAM_FOCAL_LENGTHS_MAX];
  uint8_t                         focal_lengths_count;
  float                           apertures[CAM_APERTURES_MAX];
  uint8_t                         apertures_count;
  float                           lens_position[3];
  int64_t                         max_frame_duration;
  float                           sensor_physical_size[2];
  cam_dimension_t                 pixel_array_size;
  int64_t                         flash_charge_duration;
  cam_dimension_t                 scale_picture_sizes[MAX_SCALE_SIZES_CNT];
  uint8_t                         scale_picture_sizes_cnt;
  uint32_t                        sensor_supported_scene_modes;
  uint32_t                        sensor_supported_effect_modes;
  uint64_t                        min_exposure_time;
  uint64_t                        max_exposure_time;
  uint32_t                        min_iso;
  uint32_t                        max_iso;
  uint32_t                        near_end_distance;
} mct_pipeline_sensor_cap_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  uint8_t                supported_effects_cnt;
  cam_effect_mode_type   supported_effects[CAM_EFFECT_MODE_MAX];
  uint8_t                supported_scene_modes_cnt;
  cam_scene_mode_type    supported_scene_modes[CAM_SCENE_MODE_MAX];
  uint8_t                zoom_ratio_tbl_cnt;
  int                    zoom_ratio_tbl[MAX_ZOOMS_CNT];
  uint32_t               feature_mask;
  uint32_t               max_frame_delay;
  int32_t                histogram_size;
  int32_t                max_histogram_count;
  int32_t                max_sharpness_map_value;
  uint8_t                low_power_mode_supported;
  boolean                use_pix_for_SOC;
} mct_pipeline_isp_cap_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  uint8_t                     supported_white_balances_cnt;
  cam_wb_mode_type            supported_white_balances[CAM_WB_MODE_MAX];
  uint8_t                     supported_antibandings_cnt;
  cam_antibanding_mode_type   supported_antibandings[CAM_ANTIBANDING_MODE_MAX];
  uint8_t                     supported_scene_modes_cnt;
  cam_scene_mode_type         supported_scene_modes[CAM_SCENE_MODE_MAX];
  uint8_t                     supported_flash_modes_cnt;
  cam_flash_mode_t            supported_flash_modes[CAM_FLASH_MODE_MAX];
  uint8_t                     supported_focus_modes_cnt;
  cam_focus_mode_type         supported_focus_modes[CAM_FOCUS_MODE_MAX];
  uint8_t supported_focus_algos_cnt;
  cam_focus_algorithm_type supported_focus_algos[CAM_FOCUS_ALGO_MAX];
  uint8_t                     supported_iso_modes_cnt;
  cam_iso_mode_type           supported_iso_modes[CAM_ISO_MODE_MAX];
  uint8_t                     supported_aec_modes_cnt;
  cam_auto_exposure_mode_type supported_aec_modes[CAM_AEC_MODE_MAX];
  int                         exposure_compensation_min;
  int                         exposure_compensation_max;
  int                         exposure_compensation_step;
  uint8_t                     auto_wb_lock_supported;
  uint8_t                     auto_exposure_lock_supported;
  uint8_t                     video_stablization_supported;
  uint8_t                     max_num_focus_areas;
  uint8_t                     max_num_metering_areas;
  uint32_t                    feature_mask;
  uint32_t                    max_frame_delay;
  int64_t                     exposure_time_range[2];
} mct_pipeline_stats_cap_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  uint8_t                supported_effects_cnt;
  cam_effect_mode_type   supported_effects[CAM_EFFECT_MODE_MAX];
  cam_pad_format_t       width_padding;
  cam_pad_format_t       height_padding;
  cam_pad_format_t       plane_padding;
  int8_t                 min_num_pp_bufs;
  uint32_t               feature_mask;
  uint32_t               max_frame_delay;
  boolean                is_sw_wnr;
} mct_pipeline_pp_cap_t;

typedef struct {
  uint32_t        cond_posted;
  pthread_cond_t  cond;
  pthread_mutex_t mutex;
} mct_sync_data_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct{
  pthread_t pid;
  mct_module_t *module;
  unsigned int session_id;
  pthread_cond_t cond_v;
  mct_sync_data_t sync;
  pthread_mutex_t mutex;
  unsigned int started_num;
  unsigned int started_num_success;
  unsigned int modules_num;
}mct_pipeline_thread_data_t;


/** mct_pipeline_imaging_cap_t:
 *
 *  Arguments/Fields:
 *    @max_num_roi: Face detection number of ROI
 *    @feature_mask: imglib featuremask
 *    @max_frame_delay: maximum frame delay
 *    @max_face_detection_count: face detection count
 *    @hdr_bracketing_setting: AE bracketting config
 *    @ubifocus_af_bracketing_need: AF bracketting config
 *    @refocus_af_bracketing_need: AF bracketting config
 *    @opti_zoom_settings: optizoom configuration
 *    @true_portrait_settings: trueportrait configuration
 *
 *  Structure to define the imaging capabilities
 *
 **/
typedef struct {
  uint8_t max_num_roi;
  uint32_t feature_mask;
  uint32_t max_frame_delay;
  uint32_t max_face_detection_count;
  cam_hdr_bracketing_info_t hdr_bracketing_setting;
  cam_af_bracketing_t ubifocus_af_bracketing_need;
  cam_af_bracketing_t refocus_af_bracketing_need;
  cam_af_bracketing_t mtf_af_bracketing;
  cam_opti_zoom_t opti_zoom_settings;
  cam_true_portrait_t true_portrait_settings;
  cam_fssr_t fssr_settings;
} mct_pipeline_imaging_cap_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  mct_pipeline_sensor_cap_t  sensor_cap;
  mct_pipeline_isp_cap_t     isp_cap;
  mct_pipeline_stats_cap_t   stats_cap;
  mct_pipeline_pp_cap_t      pp_cap;
  mct_pipeline_imaging_cap_t imaging_cap;
} mct_pipeline_cap_t;

typedef boolean (* mct_pipeline_add_stream_func)
  (mct_pipeline_t *pipeline, uint32_t stream_id);

typedef boolean (* mct_pipeline_remove_stream_func)
  (mct_pipeline_t *pipeline, mct_stream_t *stream);

typedef boolean (* mct_pipeline_event_func)
  (mct_pipeline_t *pipeline, uint32_t stream_id, mct_event_t *event);

typedef boolean (* mct_pipeline_set_bus_func)
  (mct_pipeline_t *pipeline, mct_bus_t *bus);

typedef mct_bus_t* (* mct_pipeline_get_bus_func)
  (mct_pipeline_t *pipeline);

typedef boolean (* mct_pipeline_map_buf_func)
  (void *msg, mct_pipeline_t *pipeline);

typedef boolean (* mct_pipeline_unmap_buf_func)
  (void *msg, mct_pipeline_t *pipeline);

typedef boolean (* mct_pipeline_proc_msg_func)
  (void *msg, mct_pipeline_t *pipeline);

/** _mct_pipeline
 *
 *    @ module           : Pipeline itself can be a Module, s;
 *    @ bus              : The mct_bus associated with this session. This bus
 *                         is used for sending events up to the mctl.
 *    @ config_w         : HAL's configuration buffer - Write Only
 *    @ config_w_size    : Write only config buffer size.
 *    @ config_r         : HAL's configuration buffer - Read Only
 *    @ config_r_size    : Read only config buffer size
 *    @ query_buf        : Query caps buffer used during boot up time.
 *    @ query_buf_size   : Query cap buffer size
 *    @ modules          : global list of modules to choose from
 *    @ session          : session id assigned to this pipeline
 *    @ pipeline_private : private data
 *    @ add_stream       :
 *    @ remove_stream    :
 *    @ send_event       :
 *    @ set_bus          :
 *    @ get_bus          :
 *    @ map_buf          :
 *    @ unmap_buf        :
 *    @ process_serv_msg :
 *    @ process_bus_msg  :
 *
 *  Pipeline is top level object; it has no parent. Pipeline's
 *  children shall be streams.
 *
 **/
struct _mct_pipeline {
  mct_module_t             module;
  mct_bus_t               *bus;
  mct_pipeline_cap_t       query_data;

  void                    *config_parm;
  size_t                   config_parm_size;
  int                      config_parm_fd;

  void                    *query_buf;
  size_t                   query_buf_size;
  int                      query_buf_fd;

  mct_list_t              *modules;
  uint32_t                 session;

  parm_buffer_new_t        *pending_set_parm;
  parm_buffer_new_t        *pending_get_parm;
  void                     *pipeline_private;
  unsigned int              max_pipeline_frame_delay;
  cam_hal_version_t         hal_version;

  unsigned int              linked_streams;

  mct_pipeline_add_stream_func    add_stream;
  mct_pipeline_remove_stream_func remove_stream;
  mct_pipeline_event_func         send_event;
  mct_pipeline_set_bus_func       set_bus;
  mct_pipeline_get_bus_func       get_bus;
  mct_pipeline_map_buf_func       map_buf;
  mct_pipeline_unmap_buf_func     unmap_buf;
  mct_pipeline_proc_msg_func      process_serv_msg;
  mct_pipeline_proc_msg_func      process_bus_msg;
  mct_pipeline_thread_data_t      thread_data;
};

#define MCT_PIPELINE_CAST(mod)         ((mct_pipeline_t *)(mod))
#define MCT_PIPELINE_NUM_CHILDREN(mod) (MCT_OBJECT_NUM_CHILDREN(mod))
#define MCT_PIPELINE_CHILDREN(mod)     (MCT_OBJECT_CHILDREN(mod))
#define MCT_PIPELINE_BUS(mod)          (MCT_PIPELINE_CAST(mod)->bus)
#define MCT_PIPELINE_SESSION(mod)      (MCT_PIPELINE_CAST(mod)->session)
#define MCT_PIPELINE_MODULES(mod)      (MCT_PIPELINE_CAST(mod)->modules)

#if defined(__cplusplus)
extern "C" {
#endif

mct_pipeline_t* mct_pipeline_new(void);
void mct_pipeline_destroy(mct_pipeline_t *pipeline);

boolean mct_pipeline_start_session(mct_pipeline_t *pipeline);
void mct_pipeline_stop_session(mct_pipeline_t *pipeline);

boolean mct_pipeline_send_ctrl_events(mct_pipeline_t *pipeline,
  uint32_t stream_id, mct_event_control_type_t event_type);

mct_stream_t* mct_pipeline_find_stream (mct_module_t *module,
  unsigned int session_id);

void *mct_pipeline_get_buffer_ptr(mct_pipeline_t *pipeline, uint32_t buf_idx,
  uint32_t stream_id);

mct_stream_map_buf_t *mct_pipeline_get_buffer(mct_pipeline_t *pipeline,
  uint32_t buf_idx, uint32_t stream_id);

mct_stream_t* mct_pipeline_find_stream_from_stream_id
  (mct_pipeline_t *pipeline, uint32_t stream_id);

mct_event_t mct_pipeline_pack_event(mct_event_type type, unsigned int identity,
  mct_event_direction direction, void *payload);

void mct_pipeline_add_stream_to_linked_streams(mct_pipeline_t *pipeline,
  mct_stream_t *stream);

void mct_pipeline_remove_stream_from_linked_streams(mct_pipeline_t *pipeline,
  mct_stream_t *stream);

#if defined(__cplusplus)
}
#endif

#endif /* __MCT_PIPELINE_H__ */
