/* modules.h
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULES_H__
#define __MODULES_H__

#include <media/msm_cam_sensor.h>
#include <linux/v4l2-mediabus.h>
#include "mct_module.h"

mct_module_t *module_iface_init(const char *name);
void  module_iface_deinit(mct_module_t *mod);
mct_module_t *module_isp_init(const char *name);
void  module_isp_deinit(mct_module_t *mod);

#define SENSOR_CID_CH_MAX 8
#define MAX_META          5

#define PPROC_DIVERT_PROCESSED    (1 << 1)
#define PPROC_DIVERT_UNPROCESSED  (1 << 2)
#define PPROC_PROCESS             (1 << 3)
#define PPROC_INVALID_IDENTITY    0x00000000

#if 0
typedef enum _modulesEventID {
  GET_CHROMATIX_PTR = 1,
  /* MCT events start */
  MCT_ADD_STREAM,
  MCT_REMOVE_STREAM,
  MCT_STREAM_ON,
  MCT_STREAM_OFF,
  /* MCT events end */
  /* sensor events start */
  MODULE_SENSOR_STREAM_CONFIG,
  MODULE_SENSOR_SET_CHROMATIX_PTR,
  /* sensor events end */
  /* IFACE events start */
  MODULE_IFACE_STREAM_CONFIG,
  /* IFACE event end */
  /* VFE events start */
  MODULE_VFE_SOF,
  /* VFE events end */
  /* 3A events start */
  MODULE_3A_AEC_UPDATE,
  MODULE_3A_AF_UPDATE,
  MODULE_3A_GET_CUR_FPS,
  /* 3A events end */
} modulesEventID_t;
#endif

/** af_sw_filter_type: AF SW stats filter type
 **/
typedef enum {
  AFS_OFF = 0,
  AFS_ON_IIR,
  AFS_ON_FIR,
} af_sw_filter_type;

typedef struct _modulesChromatix {
  void *chromatixPtr;
  void *chromatixComPtr;
} modulesChromatix_t;

typedef struct _modules_liveshot_Chromatix {
  void *liveshot_chromatix_ptr;
} modules_liveshot_Chromatix_t;

typedef struct _af_rolloff_info_t {
  void *rolloff_tables_macro;
  uint16_t af_macro;
  uint16_t af_infinity;
} af_rolloff_info_t;

typedef struct _sensor_dim_output {
  uint32_t width;
  uint32_t height;
} sensor_dim_output_t;

typedef struct _sensor_request_crop {
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
} sensor_request_crop_t;

typedef struct {
  int32_t af_supported;
  float   focal_length;
  float   pix_size;
  float   f_number;
  float   total_f_dist;
  float   hor_view_angle;
  float   ver_view_angle;
  float   um_per_dac;
  int     dac_offset;
} af_lens_info_t;

typedef struct {
  boolean               is_valid;
  enum sensor_stats_type   stats_type;
  sensor_dim_output_t   dim;
  sensor_request_crop_t request_crop;
  cam_format_t          fmt;
  boolean dump_to_fs;
} sensor_meta_t;

typedef struct {
  uint32_t                  num_meta;
  sensor_meta_t             sensor_meta_info[MAX_META];
} sensor_meta_data_t;

typedef struct {
  boolean enable;         /* if false feature disabled */
  uint32_t full_width;    /* full-resolution width */
  uint32_t full_height;   /* full-resolution height */
  uint32_t output_width;  /* output width */
  uint32_t output_height; /* output height */
  uint32_t offset_x;      /* x-index of the top-left corner of output
                           * image on the full-resolution sensor */
  uint32_t offset_y;      /* y-index of the top-left corner of output image
                           * on the full-resolution sensor */
  uint32_t scale_factor;  /* the sensor scaling factor
                           * (=binning_factor * digal_sampling_factor) */
} sensor_rolloff_config_t;

typedef struct {
  enum                      camerab_mode_t mode;
  sensor_dim_output_t       dim_output;
  sensor_request_crop_t     request_crop;
  cam_format_t              fmt;
  uint32_t                  op_pixel_clk;
  uint8_t                   num_frames_skip;
  float                     max_gain;
  uint32_t                  max_linecount;
  uint32_t                  vt_pixel_clk;
  uint32_t                  ll_pck;
  uint32_t                  fl_lines;
  uint16_t                  pixel_sum_factor;
  float                     max_fps;
  af_lens_info_t            af_lens_info;
  sensor_meta_data_t        meta_cfg;
  boolean                   is_retry;
  uint32_t                  sensor_mount_angle;
  enum                      camb_position_t position;
  sensor_rolloff_config_t   sensor_rolloff_config;
  boolean                   prep_flash_on;
  /* Sensor frame delay to be exposed to other modules in pipeline*/
  uint32_t                  sensor_max_pipeline_frame_delay;
  uint8_t                   csi_clk_scale_enable;
} sensor_out_info_t;

typedef struct _sensor_fps_update {
  float max_fps;
} sensor_fps_update_t;

typedef enum {
  ISP_STRIPE_LEFT = 0,
  ISP_STRIPE_RIGHT,
  ISP_STRIPE_MAX,
} isp_stripe_id_t;

typedef struct {
  boolean                   is_split;
  uint32_t                  overlap;
  uint32_t                  right_stripe_offset;
} ispif_out_info_t;

/* This is the description of what the ISP outputs when
2 ISPs are working co-operatively on a single frame. */
typedef struct {
  isp_stripe_id_t     stripe_id;
  uint32_t            left_output_width;
  uint32_t            right_output_width;
  uint32_t            right_stripe_offset;
} isp_out_info_t;

typedef struct {
  int luma_target;
  int current_luma;
  float gain;
  float digital_gain;
  uint32_t linecount;
  uint32_t fps;
} sensor_set_aec_data_t;

typedef struct {
  uint8_t cid;
  uint8_t csid;
  uint8_t is_bayer_sensor;
  uint32_t csid_version;
  cam_format_t fmt;
} sensor_src_port_cap_entry_t;

typedef struct {
  uint8_t session_id;
  uint8_t num_cid_ch;     /* num CIDs from sensor */
  sensor_src_port_cap_entry_t sensor_cid_ch[SENSOR_CID_CH_MAX];
  uint8_t num_meta_ch;    /* num of meta channels */
  sensor_src_port_cap_entry_t meta_ch[MAX_META];
} sensor_src_port_cap_t;

typedef enum {
  ISP_INPUT_ISPIF,
  ISP_INPUT_FETCH_ENG,
  ISP_INPUT_TEST_GEN
} isp_input_type_t;

typedef struct {
  isp_input_type_t input_type;
  uint8_t use_pix;
  sensor_src_port_cap_t sensor_cap;
} ispif_src_port_caps_t;

typedef struct {
  sensor_out_info_t sensor_cfg;
  uint32_t meta_use_output_mask;
  uint32_t vfe_output_mask; /* hi 16 bits - VFE1, lo 16 bits VFE0 */
  uint32_t vfe_mask;        /* which vfe associated */
  uint32_t session_id;      /* session id */
  uint32_t stream_id;       /* stream id */
} ispif_src_port_stream_cfg_t;

typedef struct {
  boolean native_buf;            /* TRUE VFE allocated buf */
  void *vaddr;                   /* NULL if not native buf */
  int fd;                        /* not used if not native buf */
  struct v4l2_buffer buffer;     /* v4l2 buffer */
  boolean is_locked;             /* this flag is only used for
                                  * downstream module for sharing the
                                  * same struct. ISP does not read it. */
  boolean ack_flag;              /* as discussed with MCT and IMAGE_LIB team,
                                  * ack_flag is added for downstream module
                                  * to piggy back the ack. */
  boolean is_buf_dirty;          /* when ack_flag is TRUE is_buf_dirty
                                  * is checked by ISP to determine
                                  * if buf_done is needed */
  boolean is_uv_subsampled;      /* indicates that the buffer is additionally
                                  * chroma subsampled on top of the subsampling format */
  boolean is_cpp_processed;      /* indicates whether the buffer is touched by CPP */
  unsigned int identity;         /* identity 0x0000 0000
                                  * (session/stream index) */
  int channel_id;                /* channel id */
  boolean pass_through;          /* When pass_through is set to 1 some modules
                                    can simply by pass this event */
  void *meta_data;               /* This holds the pointer for meta data
                                    associated with this buffer */
  boolean is_skip_pproc;         /* indicates if PProc skip */
  uint32_t handle;
  uint32_t output_format;
  enum msm_vfe_input_src input_intf;
} isp_buf_divert_t;
typedef struct {
  int buf_idx;
  boolean is_buf_dirty;
  unsigned int identity;
  int channel_id;
  uint32_t frame_id;
  struct timeval timestamp;
  void *meta_data;               /* This holds the pointer for meta data
                                    associated with this buffer */
  boolean is_skip_pproc;         /* indicates if PProc skip */
  uint32_t handle;
  uint32_t output_format;
  enum msm_vfe_input_src input_intf;
} isp_buf_divert_ack_t;

typedef struct {
  uint32_t session_id;
  uint32_t stream_id;
  uint32_t width;
  uint32_t height;
  cam_streaming_mode_t streaming_mode;
  uint32_t col_num;
  uint32_t row_num;
} isp_dis_config_info_t;

typedef struct {
  enum msm_vfe_frame_skip_pattern skip_pattern;
} isp_stream_skip_pattern_t;

/** mct_face_data_t
 *   @region: face boundary
 *   @score: score for determining the prominence
 *
 *   Face data
 **/
typedef struct {
  cam_rect_t roi;
  int8_t score;
  int face_id;
} mct_face_data_t;

/** mct_face_info_t
 *   @face_count: number of faces detected
 *   @faces: data for each face detected
 *   @frame_id: sequence number of the frame
 *
 *   Face event information
 **/
typedef struct {
  uint8_t face_count;
  mct_face_data_t faces[MAX_ROI];
  mct_face_data_t orig_faces[MAX_ROI];
  uint32_t frame_id;
} mct_face_info_t;

/** pproc_divert_info_t
 *   @num_pass: number of pass
 *   @divert_flags: divert flag bitfield (0:processed,
 *                  1:unprocessed)
 *   @unprocess_identity: identity for unprocess divert
 *   @processed_identity[2]: identity for processed divert
 *   @proc_identity[2]: identities that are processed(# of pass)
 *
 *   pproc divert information
 **/
typedef struct {
  int32_t      num_passes;
  uint32_t     divert_flags;
  uint32_t     div_unproc_identity;
  uint32_t     div_proc_identity[2];
  uint32_t     proc_identity[2];
} pproc_divert_info_t;

typedef enum {
  PPROC_CFG_UPDATE_SINGLE,
  PPROC_CFG_UPDATE_DUAL,
} pproc_cfg_update_t;

/** pproc_divert_config_t
 *   @name: submodule name
 *   @update_mode: update mode (single/dual)
 *   @divert_info: divert info like #pass, relevant identities
 *
 *   divert config event information
 **/
typedef struct {
  char               *name;
  pproc_cfg_update_t  update_mode;
  pproc_divert_info_t divert_info;
} pproc_divert_config_t;

/** mct_isp_table_t
 *   @gamma_table - pointer to gamma table
 *   @gamma_num_entries: number of gamma table entries
 *   @la_enable - field to check if LA is enabled
 *   @luma_table - luma adaptation table
 *   @luma_num_entries - number of entries in LA table
 *
 *   ISP tables
 **/
typedef struct {
  uint16_t *gamma_table;
  uint32_t gamma_num_entries;
  int8_t la_enable;
  uint8_t *luma_table;
  uint32_t luma_num_entries;
} mct_isp_table_t;

/** meta_data_buf_divert_request_t
 *   @mata_idx_mask - bit mask of meta channel index mask
 *
 **/
typedef struct {
  uint32_t meta_idx_mask;
} meta_channel_buf_divert_request_t;

typedef struct {
  uint8_t enable;
} mct_fast_aec_mode_t;

/** mct_imglib_af_config_t
 *   @enable: Flag to enable/disable stats collection
 *   @frame_id: current frame id
 *   @roi: ROI of the stats collection
 *   @coeffa: filterA coefficients
 *   @coeffb: filterB coefficients
 *   @coeff_len: length of coefficient table
 *   @sw_filter_type: SW filter type
 *
 *   Imglib preview assisted AF coefficients
 **/
typedef struct {
  int8_t enable;
  int frame_id;
  cam_rect_t roi;
  double coeffa[6];
  double coeffb[6];
  int coeff_fir[11];
  uint32_t coeff_len;
  af_sw_filter_type filter_type;
  double FV_min;
} mct_imglib_af_config_t;

/** mct_imglib_af_output_t
 *   @frame_id: Frame ID out the AF output
 *   @fV: Focus value
 *
 *   Imglib preview assisted AF output
 **/
typedef struct {
  int frame_id;
  double fV;
} mct_imglib_af_output_t;

/** mct_bracket_ctrl_state_t
 *   @MCT_BRACKET_CTRL_OFF: bracketting is Off
 *   @MCT_BRACKET_CTRL_STORE_3A: ctrl to store current 3A info
 *   @MCT_BRACKET_CTRL_RESTORE__LOCK_3A: ctrl to restore the
 *                                     stored info and reapply
 *                                     3a
 *
 **/
typedef enum {
  MCT_BRACKET_CTRL_OFF,
  MCT_BRACKET_CTRL_STORE_3A,
  MCT_BRACKET_CTRL_RESTORE_LOCK_3A,
} mct_bracket_ctrl_state_t;

/** mct_bracket_ctrl_t
 *   @state: bracketing ctrl state
 *
 *   Ctrl info during bracketting (flash/focus)
 *   Note that this data structure will be expanded.
 **/
typedef struct {
  mct_bracket_ctrl_state_t state;
} mct_bracket_ctrl_t;

#endif /* __MODULES_H__ */
