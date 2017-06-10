/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#ifndef __FRAMEPROC_INTERFACE_H__
#define __FRAMEPROC_INTERFACE_H__

#include "chromatix.h"
#include "intf_comm_data.h"
#include "tgtcommon.h"

/* Histogram size */
#define   FD_PIXEL_BIT_WIDTH      8
#define   FD_HIST_SIZE            (1<<FD_PIXEL_BIT_WIDTH)
#define   MAX_HDR_NUM_FRAMES      3
/* TODO: need to remove this from here or change name */
#define   HJR_MAX_FRAMES_SUPPORTED 2
#if !defined MAX
  #define  MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif
#define  MAX_FRAMEPROC_FRAME_NUM  MAX(HJR_MAX_FRAMES_SUPPORTED,MAX_HDR_NUM_FRAMES)

typedef enum {
  FRAME_PROC_H2V2 = 0, // YUV420
  FRAME_PROC_H2V1 = 1, //YUV422
  FRAME_PROC_H1V2 = 2,
  FRAME_PROC_H1V1 = 3,
} frame_proc_format_t;

typedef struct {
  uint32_t luma_target;
  uint32_t line_count;
  uint32_t redeye_led_on_skip_frm;
  uint32_t redeye_led_off_skip_frm;
  uint32_t auto_mode;
  /* exif tag data */
  uint32_t exp_mode;
  uint32_t exp_program;
  float    real_gain;
  float    exp_time;
} aec_snapshot_t;

typedef struct {
  int             aec_settled;
  int             max_line_cnt;
  int             hjr_snap_frame_count;
  uint32_t        cur_line_cnt;
  float           band_50hz_gap;
  aec_snapshot_t  snap;
  float             lux_idx;
} stats_proc_aec_info_t;

typedef struct {
  chromatix_manual_white_balance_type snapshot_wb;
} stats_proc_awb_info_t;

typedef struct {
  int is_af_active;
} stats_proc_af_info_t;

typedef struct {
  stats_proc_aec_info_t  aec_d;
  stats_proc_awb_info_t  awb_d;
  stats_proc_af_info_t   af_d;
} frame_proc_statsproc_info_t;
/***************************************/

typedef enum {
  FRAME_PROC_PREVIEW = 0,
  FRAME_PROC_SNAPSHOT,
  FRAME_PROC_OPT_MAX
} frame_proc_opt_mode_t;

typedef enum {
  FRAME_PROC_FACE_DETECT,
  FRAME_PROC_AFD,
  FRAME_PROC_WAVELET_DENOISE,
  FRAME_PROC_HJR,
  FRAME_PROC_HDR,
  FRAME_PROC_SHARE,
  FRAME_PROC_ADD_OBJ,
} frame_proc_type_t;

typedef enum {
  AFD_INIT = 0,
  AFD_ACTIVE,
  AFD_INACTIVE,
  AFD_INVALID_STATE,
  AFD_STATE_OFF,
  AFD_STATE_ON,
  AFD_STATE_BUSY,
  AFD_STATE_DONE,
} afd_state_type;

 typedef struct {
  afd_state_type               afd_state;
  int                          afd_enable;
//MCTL to call set antibanding
  int             afd_status;
  stats_proc_antibanding_type  afd_antibanding_type;
} frame_proc_afd_data_t;

typedef struct {
  uint32_t                bin[FD_HIST_SIZE];
  uint32_t                num_samples;
} fd_hist_t;
typedef struct {
  int32_t                 x;
  int32_t                 y;
} fd_pixel_t;

typedef struct {
  /* TO DO : change hard coded value */
  fd_pixel_t facePt[12];
  int Confidence[12];
  int faceDirectionUpDown; // -180 to 179
  int faceDirectionLeftRight;// -180 to 179
  int faceDirectionRoll; //-180 to 179
  } face_part_detect;

 typedef struct {
 int smile_degree;  //0 - 100
 int confidence;  // 0 -1000
 } fd_smile_detect;

 typedef struct {
  fd_pixel_t contour_pt[45];
 } contour_detect;

typedef struct {
  struct fd_rect_t       face_boundary;
  face_part_detect       fp;
  contour_detect         ct;
  fd_smile_detect        sm;
  int                    is_face_recognised;  /* 1 or 0 */
  fd_hist_t              histogram;
  int                    blink_detected; /* 1 or 0 */
  int                    left_blink; // 0 - 1000
  int                    right_blink; // 0 - 1000

  /* left gaze gives +ve value; right gaze gives -ve value */
  int                    left_right_gaze;
  /*top gaze gives -ve, bottom gaze give +ve value */
  int                    top_bottom_gaze;
  int                    unique_id;
  int                    gaze_angle;// -90, -45, 0 , 45 ,90
  int                    fd_confidence;
} frame_proc_fd_roi_t;

typedef struct {
  fd_mode_t               fd_mode;
  int                     fd_enable;
  int                     output_updated;
  frame_proc_fd_roi_t     roi[MAX_ROI];
  int                     fd_skip_cnt;
  stats_proc_roi_type_t  type;
  int                    frame_id;
  int                    config_num_fd;
  uint32_t               num_faces_detected;
} frame_proc_fd_data_t;

typedef struct {
  int denoise_enable;
  wd_process_plane_t process_mode;
} frame_proc_wd_data_t;

typedef struct {
  int hjr_enable;
} frame_proc_hjr_data_t;

/* HDR Data */
typedef struct {
  int max_num_hdr_frames;
  float exp_values[MAX_HDR_NUM_FRAMES];
  /* to be filled by mctl */
  int hdr_enable;
  int hdr_output_buf_num; // which buffer contains hdr frame
}frame_proc_hdr_data_t;

typedef struct {
  int32_t    afd_zoom_xScale;
  int32_t    afd_zoom_yScale;
  int lumaAdaptationEnable;
  int VFE_GAMMA_NUM_ENTRIES;
  int VFE_LA_TABLE_LENGTH;
  int16_t *RGB_gamma_table;
  int16_t *LA_gamma_table;
} frame_proc_isp_info_t;

typedef struct {
  sensor_camif_inputformat_t format;
  int max_preview_fps;
  int preview_fps;
} frame_proc_sensor_info_t;

typedef struct {
  int width;
  int height;
} frame_proc_dimension_t;

typedef struct {
  uint32_t              crop_factor;
  frame_proc_opt_mode_t opt_mode;
  int num_main_img;
  int num_thumb_img;
  struct msm_pp_frame  frame;
  struct msm_pp_frame main_img_frame[MAX_FRAMEPROC_FRAME_NUM];
  struct msm_pp_frame thumb_img_frame[MAX_FRAMEPROC_FRAME_NUM];

  frame_proc_dimension_t display_dim; /* copy to local framectrl->frame_width */
  frame_proc_dimension_t picture_dim; /* calibrate copy to p_denoise */
  frame_proc_dimension_t thumbnail_dim;
  frame_proc_format_t main_img_format;
  frame_proc_format_t thumb_img_format;
} frame_proc_mctl_info_t;


typedef struct {
  frame_proc_mctl_info_t      mctl_info;     /* mctl provided information */
  frame_proc_isp_info_t       isp_info;
  frame_proc_sensor_info_t    sensor_info;
  frame_proc_statsproc_info_t statsproc_info;
  chromatix_parms_type        *chromatix; /* chromatix header pointer */
} frame_proc_interface_input_t;

typedef struct {
  int divert_preview;
  int divert_snapshot;
  uint32_t stream_type;
} frame_proc_share_data_t;

typedef struct {
  frame_proc_share_data_t share_d;
  frame_proc_fd_data_t    fd_d;
  frame_proc_afd_data_t   afd_d;
  frame_proc_wd_data_t    wd_d;
  frame_proc_hjr_data_t   hjr_d;
  frame_proc_hdr_data_t   hdr_d;
} frame_proc_interface_output_t;
typedef struct {
  frame_proc_interface_output_t output;
  frame_proc_interface_input_t input;
} frame_proc_interface_t;

typedef enum {
  FRAME_PROC_AFD_RESET,
  FRAME_PROC_AFD_ENABLE,
} afd_type_t;

typedef struct {
  afd_type_t type;
  int afd_enable;
  int afd_mode;
} frame_proc_set_afd_data_t;

typedef enum {
  FACE_DETECT_ENABLE,
  FACE_CLEAR_ALBUM,
  FACE_ADJUST_ZOOM,
} fd_type_t;

typedef struct {
  fd_type_t type;
  fd_mode_t mode;
  int fd_enable;
  int num_fd;
} frame_proc_set_fd_data_t;

typedef enum {
  WAVELET_DENOISE_ENABLE,
  WAVELET_DENOISE_CALIBRATE,
} wd_type_t;


typedef struct {
  uint32_t              crop_factor;
  wd_type_t type;
  int denoise_enable; // set
  wd_process_plane_t process_planes;
} frame_proc_set_wd_data_t;

typedef struct {
  int hjr_enable;
} frame_proc_set_hjr_data_t;

typedef enum {
  FRAME_PROC_HDR_ENABLE,
  FRAME_PROC_HDR_HW_INFO,
} hdr_type_t;
typedef struct {
  int hdr_enable;
  int num_hal_buf;
} hdr_init_info_t;
typedef struct {
  hdr_type_t type;
  hdr_init_info_t hdr_init_info;
} frame_proc_set_hdr_data_t;
typedef enum {
  FRAME_PROC_ADJUST_ZOOM,
  FRAME_PROC_SET_STREAM,
}  frame_proc_share_type_t;

typedef struct{
  frame_proc_share_type_t type;
  union{
    uint32_t stream_type;
    struct fd_rect_t zoom_crp;
  } d;
}frame_proc_set_share_data_t;

typedef struct {
  frame_proc_type_t type;
  union {
    frame_proc_set_fd_data_t   set_fd;
    frame_proc_set_afd_data_t  set_afd;
    frame_proc_set_wd_data_t   set_wd;
    frame_proc_set_hjr_data_t  set_hjr;
    frame_proc_set_hdr_data_t  set_hdr;
	frame_proc_set_share_data_t set_share;
  } d;
} frame_proc_set_t;

/*******************************
 Post Processing Exported APIs
*******************************/
/*uint32_t frame_proc_interface_create(void);
int frame_proc_init(uint32_t handle, frame_proc_interface_input_t *init_data);
int frame_proc_destroy(uint32_t handle);
int frame_proc_set_params(uint32_t handle, frame_proc_set_t *frame_proc_set,
  frame_proc_interface_t *frame_proc_intf);
int frame_proc_process(uint32_t handle, frame_proc_interface_t *frame_proc_intf);
void frame_proc_abort(void); */
int frame_proc_comp_create();
uint32_t frame_proc_client_open(module_ops_t *ops);
int frame_proc_comp_destroy();
#endif /* __FRAMEPROC_INTERFACE_H__ */
