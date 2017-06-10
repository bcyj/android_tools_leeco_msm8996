/**********************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __INTF_COMM_DATA_H__
#define __INTF_COMM_DATA_H__

#include "camera.h"
/*===========================================================================
 *                         INCLUDE FILES
 *===========================================================================*/
#include "tgtcommon.h"
#include "chromatix.h"
#include <media/msm_camera.h>

#define MM_COMP_AXI_STR         "AXI"
#define MM_COMP_CAMIF_STR       "CAMIF"
#define MM_COMP_VFE_STR         "VFE"
#define MM_COMP_STATSPROC_STR   "STATSPROC"
#define MM_COMP_FRAMEPROC_STR   "FRAMEPROC"
#define MM_COMP_SENSOR_STR      "SENSOR"
#define MM_COMP_FLASHLED_STR    "FLASHLED"
#define MM_COMP_FLASHSTROBE_STR "FLASHSTROBE"
#define MM_COMP_ACTUATOR_STR    "ACTUATOR"
#define MM_COMP_EEPROM_STR      "EEPROM"
#define MM_COMP_ISPIF_STR       "ISPIF"
#define MM_COMP_C2D_STR         "C2D"
#define MM_COMP_VPE_STR         "VPE"
#define MM_COMP_CPP_STR         "CPP"
#define MM_COMP_IRQROUTER_STR   "IRQROUTER"
#define MM_COMP_CCI_STR         "CCI"
#define MM_COMP_CSI_STR         "CSI"

typedef enum {
  MCTL_COMPID_AXI  = 0,
  MCTL_COMPID_CAMIF,
  MCTL_COMPID_VFE,
  MCTL_COMPID_STATSPROC,
  MCTL_COMPID_FRAMEPROC,
  MCTL_COMPID_SENSOR,
  MCTL_COMPID_FLASHLED,
  MCTL_COMPID_FLASHSTROBE,
  MCTL_COMPID_ACTUATOR,
  MCTL_COMPID_EEPROM,
  MCTL_COMPID_ISPIF,
  MCTL_COMPID_C2D,
  MCTL_COMPID_VPE,
  MCTL_COMPID_CPP,
  MCTL_COMPID_IRQROUTER,
  MCTL_COMPID_CCI,
  MCTL_COMPID_CSI,
  MCTL_COMPID_MAX
}comp_id_t;

/* MCTL common data Begin */
typedef enum {
  MCTL_DATA_SENSOR_MODE,
  MCTL_DATA_MAX
}mctl_data_t;
/* MCTL common data End */

/* Interface common defines Start*/
typedef enum {
  CAMERA_FLASH_NONE,
  CAMERA_FLASH_LED,
  CAMERA_FLASH_STROBE,
}camera_flash_type;

/* Interface common defines End*/

/* Individual interface params */
/* ACTUATOR Interface params Begin */
/* ACTUATOR Interface params End */

/* AXI Interface params Begin */
/* AXI Interface params End */

/* CAMIF Interface params Begin */
typedef enum {
  CAMIF_PARAMS_SENSOR_DIMENSION,
  CAMIF_PARAMS_SENSOR_CROP_WINDOW,
  CAMIF_PARAMS_MODE,
  CAMIF_PARAMS_CAMIF_DIMENSION,
  CAMIF_PARAMS_SENSOR_FORMAT,
  CAMIF_PARAMS_STROBE_INFO,
  CAMIF_PARAMS_CONNECTION_MODE,
  CAMIF_PARAMS_HW_VERSION,
  CAMIF_PARAMS_ADD_OBJ_ID,
  CAMIF_PARAMS_MAX_NUM
}camif_params_type_t;

typedef struct {
  union {
    camera_size_t camif_window;
  }d;
}camif_output_t;
/* CAMIF Interface params End */

/* Flash LED Interface params Begin */
/* Flash LED Interface params End */

/* Flash Strobe Interface params Begin */
/* Flash Strobe Interface params End */

/* Frameproc Interface params Begin */
/* Frameproc Interface params End */

/* Sensor Interface params Begin */
typedef enum {
  SENSOR_MODE_SNAPSHOT,
  SENSOR_MODE_RAW_SNAPSHOT,
  SENSOR_MODE_PREVIEW,
  SENSOR_MODE_VIDEO,
  SENSOR_MODE_VIDEO_HD,
  SENSOR_MODE_HFR_60FPS,
  SENSOR_MODE_HFR_90FPS,
  SENSOR_MODE_HFR_120FPS,
  SENSOR_MODE_HFR_150FPS,
  SENSOR_MODE_ZSL,
  SENSOR_MODE_INVALID,
} sensor_mode_t;

typedef enum {
  SENSOR_GET_DIM_INFO,
  SENSOR_GET_CHROMATIX_PTR,
  SENSOR_GET_CAMIF_CFG,
  SENSOR_GET_OUTPUT_CFG,
  SENSOR_GET_SENSOR_MODE_AEC_INFO,
  SENSOR_GET_DIGITAL_GAIN,
  SENSOR_GET_SENSOR_MAX_AEC_INFO,
  SENSOR_GET_PREVIEW_FPS_RANGE,
  SENSOR_GET_PENDING_FPS,
  SENSOR_GET_CHROMATIX_TYPE,
  SENSOR_GET_MAX_SUPPORTED_HFR_MODE,
  SENSOR_GET_CUR_FPS,
  SENSOR_GET_LENS_INFO,
  SENSOR_GET_CSI_PARAMS,
  /* mount angle, position, YUV/Bayer, AF, etc */
  SENSOR_GET_SENSOR_INFO,
  SENSOR_GET_CUR_RES,
} sensor_get_type_t;

typedef enum {
  SENSOR_BAYER,
  SENSOR_YCBCR
} sensor_output_format_t;

typedef enum {
  SENSOR_PARALLEL,
  SENSOR_MIPI_CSI,
  SENSOR_MIPI_CSI_1,
  SENSOR_MIPI_CSI_2,
} sensor_connection_mode_t;

typedef enum {
  SENSOR_8_BIT_DIRECT,
  SENSOR_10_BIT_DIRECT,
  SENSOR_12_BIT_DIRECT
} sensor_raw_output_t;

typedef enum {
  SENSOR_LOAD_CHROMATIX_PREVIEW,
  SENSOR_LOAD_CHROMATIX_VIDEO_DEFAULT,
  SENSOR_LOAD_CHROMATIX_VIDEO_HD,
  SENSOR_LOAD_CHROMATIX_VIDEO_HFR_60FPS,
  SENSOR_LOAD_CHROMATIX_VIDEO_HFR_90FPS,
  SENSOR_LOAD_CHROMATIX_VIDEO_HFR_120FPS,
  SENSOR_LOAD_CHROMATIX_VIDEO_HFR_150FPS,
  SENSOR_LOAD_CHROMATIX_AR,
  SENSOR_LOAD_CHROMATIX_ZSL,
  SENSOR_LOAD_CHROMATIX_MAX,
} sensor_load_chromatix_t;

typedef struct {
  sensor_output_format_t output_format;
  sensor_connection_mode_t connection_mode;
  sensor_raw_output_t raw_output;
} sensor_output_t;

typedef struct {
  float max_fps;
  float min_fps;
} sensor_fps_range_t;

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
  uint32_t cropped_width;
  uint32_t cropped_height;
  sensor_camif_inputformat_t format;
} sensor_camif_setting_t;

typedef struct {
  sensor_mode_t op_mode;
  uint32_t pixels_per_line;
  uint32_t lines_per_frame;
  uint32_t pclk;
  uint32_t max_fps;

  float digital_gain;
  float stored_digital_gain;
  float max_gain;
  uint32_t max_linecount;
} sensor_aec_data_t;

typedef struct {
  sensor_mode_t op_mode;
  uint32_t width;
  uint32_t height;
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
} sensor_dim_info_t;

typedef struct {
  float focal_length;
  float pix_size;
  float f_number;
  float total_f_dist;
  float hor_view_angle;
  float ver_view_angle;
} sensor_lens_info_t;

typedef struct {
  struct msm_camera_csi_params **csic_params;
  struct msm_camera_csi2_params **csi2_params;
  struct csi_lane_params_t csi_lane_params;
} sensor_csi_params_t;

typedef struct {
  chromatix_parms_type *chromatix_ptr;
  sensor_load_chromatix_t chromatix_type;
  sensor_dim_info_t sensor_dim;
  sensor_output_t sensor_output;
  sensor_camif_setting_t camif_setting;
  sensor_aec_data_t aec_info;
  uint32_t fps;
  sensor_csi_params_t *sensor_csi_params;
  enum msm_sensor_resolution_t cur_res;
  uint32_t get_pending_fps;
  sensor_mode_t op_mode;
  camera_hfr_mode_t max_supported_hfr_mode;
  sensor_fps_range_t fps_range;
  sensor_lens_info_t lens_info;
  uint32_t pxlcode;
} sensor_data_t;

 typedef struct {
  sensor_get_type_t type;
  union {
    sensor_data_t data;
	struct msm_camsensor_info sinfo;
  };
} sensor_get_t;

/* Sensor Interface params End */

/* Statsproc Interface params Begin */
typedef enum {
  STATS_PROC_AEC_TYPE,
  STATS_PROC_AWB_TYPE,
  STATS_PROC_AF_TYPE,
  STATS_PROC_ASD_TYPE,
  STATS_PROC_AFD_TYPE,
  STATS_PROC_DIS_TYPE,
  STATS_PROC_EZ_TUNE_TYPE,
  STATS_PROC_CHROMATIX_RELOAD_TYPE,
  STATS_PROC_MOBICAT_TYPE,
  STATS_PROC_COMMON_TYPE,
} stats_proc_type_t;

typedef enum {
  AEC_FPS_MODE_AUTO,
  AEC_FPS_MODE_FIXED,
} stats_proc_fps_mode_type;

/********************************
     STATS_PROC GET Interface Data
*********************************/
/* AEC GET DATA */
typedef enum {
  AEC_LED_SETTLE_CNT,
  AEC_OVER_EXP_STATE,
  AEC_LED_STROBE,
  AEC_QUERY_FLASH_FOR_SNAPSHOT,
  AEC_FPS_MODE_G,
  AEC_AFR_PREVIEW_FPS,
  AEC_PREVIEW_EXP_TIME,
  AEC_LOCK_STATE,
  AEC_FORCED_EXPOSURE,
  AEC_PARMS,
  AEC_FLASH_DATA,
  AEC_EXPOSURE_PARAMS,
} aec_get_t;

typedef enum {
  DISTANCE_NEAR_INDEX,  /* 0 */
  DISTANCE_OPTIMAL_INDEX,
  DISTANCE_FAR_INDEX,
  DISTANCE_MAX_INDEX
} focus_distance_index;

typedef struct {
  float focus_distance[DISTANCE_MAX_INDEX];
} focus_distances_info;

typedef struct {
  uint32_t target_luma;
  uint32_t cur_luma;
  int32_t exp_index;
  int32_t exp_tbl_val;
  float lux_idx;
  float cur_real_gain;
  float snapshot_real_gain;
}stats_proc_aec_parms_t;

typedef struct {
  int luma_target;
  int current_luma;
  float gain;
  uint32_t linecount;
  int is_snapshot;
}stats_proc_aec_exp_parms_t;

typedef struct {
  camera_flash_type flash_mode;
  uint32_t sensitivity_led_off;
  uint32_t sensitivity_led_low;
  uint32_t sensitivity_led_hi;
  uint32_t strobe_duration;
}stats_proc_flash_parms_t;

typedef struct {
  uint32_t t1;
  uint32_t t2;
  uint32_t t3;
  uint32_t t6;
  uint32_t t4;
  uint32_t mg;
  uint32_t t5;
}stats_proc_awb_exterme_col_param_t;

typedef struct {
  uint32_t regionW;
  uint32_t regionH;
  uint32_t regionHNum;
  uint32_t regionVNum;
  uint32_t regionHOffset;
  uint32_t regionVOffset;
  uint32_t shiftBits;
}stats_proc_awb_region_info_t;

typedef struct {
  chromatix_manual_white_balance_type gain;
  uint32_t color_temp;
  chromatix_wb_exp_stats_type bounding_box;
  stats_proc_awb_region_info_t region_info;
  stats_proc_awb_exterme_col_param_t exterme_col_param;
}stats_proc_awb_params_t;

typedef struct {
  chromatix_manual_white_balance_type curr_gains;
}stats_proc_awb_gains_t;

/* AWB GET DATA */
typedef enum {
  AWB_PARMS,
  AWB_GAINS
} awb_get_t;

typedef struct {
  awb_get_t type;
  union {
    stats_proc_awb_params_t awb_params;
    stats_proc_awb_gains_t  awb_gains;
  } d;
} stats_proc_get_awb_data_t;

/* AF GET DATA */
#define NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS  9
#define NUM_AUTOFOCUS_HORIZONTAL_GRID 9
#define NUM_AUTOFOCUS_VERTICAL_GRID 9
#define AUTOFOCUS_STATS_BUFFER_MAX_ENTRIES 1056

typedef enum {
    AF_STATS_CONFIG_MODE_DEFAULT,
    AF_STATS_CONFIG_MODE_SINGLE,
    AF_STATS_CONFIG_MODE_MULTIPLE,
} stats_proc_af_stats_config_mode_t;

typedef enum {
  AF_SHARPNESS,
  AF_FOCUS_DISTANCES,
  AF_CUR_LENS_POSITION,
  AF_STATUS,
  AF_STATS_CONFIG_MODE,
  AF_FOCUS_MODE,
  AF_MOBICAT_INFO,
} af_get_t;

typedef enum {
  CAF_NOT_FOCUSED,
  CAF_FOCUSED, /* Image is sharp */
  CAF_FOCUSING, /* Focusing is in progress */
  CAF_UNKNOWN /* Unknown - may be sharp or not */
}caf_status_t;

typedef struct {
    int mode;
    roi_t region[MAX_ROI];
    unsigned int af_multi_nfocus;
    uint8_t *af_multi_roi_window;
} stats_proc_af_config_data;

typedef struct {
  af_get_t type;
  union {
    int af_sharpness;
    /*todo need to revisit */
    focus_distances_info af_focus_distance;
    int af_cur_lens_pos;
    int af_status;
    int af_mode;
    stats_proc_af_config_data af_stats_config;
  } d;
} stats_proc_get_af_data_t;

/* ASD GET DATA */
typedef enum {
  ASD_PARMS,
} asd_get_t;

typedef struct {
  uint32_t landscape_severity;
  uint32_t backlight_detected;
  uint32_t backlight_scene_severity;
  uint32_t portrait_severity;
  float asd_soft_focus_dgr;
}stats_proc_asd_params_t;

typedef struct {
  asd_get_t type;
  union {
    stats_proc_asd_params_t asd_params;
  } d;
} stats_proc_get_asd_data_t;

/* DIS GET DATA */
typedef enum {
  DIS_TODO,
} dis_get_t;

typedef struct {
  dis_get_t type;
  union {
    int todo;
  } d;
} stats_proc_get_dis_data_t;

/* AFD GET DATA */
typedef enum {
  AFD_TODO,
} afd_get_t;

typedef struct {
  afd_get_t type;
  union {
    int todo;
  } d;
} stats_proc_get_afd_data_t;

typedef struct {
  aec_get_t type;
  union {
    int                         use_strobe;
    int                         aec_over_exposure;
    int                         aec_led_settle_cnt;
    int                         aec_force_exposure;
    int                         query_flash_for_snap;
    float                       aec_preview_expotime;
    float                       afr_preview_fps;
    stats_proc_fps_mode_type    fps_mode;
    stats_proc_aec_parms_t      aec_parms;
    stats_proc_flash_parms_t    flash_parms;
    stats_proc_aec_exp_parms_t  exp_params;
  } d;
} stats_proc_get_aec_data_t;

typedef struct {
  Q3AInfo_t stats_proc_info;
} stats_proc_get_mobicat_data_t;

typedef struct {
  stats_proc_type_t type;
  union {
    stats_proc_get_aec_data_t    get_aec;
    stats_proc_get_awb_data_t    get_awb;
    stats_proc_get_af_data_t     get_af;
    stats_proc_get_asd_data_t    get_asd;
    stats_proc_get_dis_data_t    get_dis;
    stats_proc_get_afd_data_t    get_afd;
    stats_proc_get_mobicat_data_t get_mobicat;
  } d;
} stats_proc_get_t;
/* Statsproc Interface params End */

/* VFE Interface params Begin */
/* VFE Interface params End */

/* CSI Interface params Begin */
typedef enum {
  CSI_GET_CSID_VERSION,
} csi_get_params_type_t;

typedef struct {
  uint32_t csid_version;
} csi_get_t;
/* CSI Interface params End */

/* resource manager */

typedef struct {
  uint32_t mctl_id;       /* which mctl requested */
  uint32_t client_handle;     /* client's handle */
  uint8_t  idx;               /* resource idx, could be interface idx */
  uint32_t used_fps;          /* used fps if appliable */
  uint8_t  real_time_client;  /* the reservation is for realtime usage */
} comp_res_client_t;

typedef struct {
  uint8_t max_num_rdis;              /* num RDI interfaces */
  uint8_t max_num_pixs;              /* max = 1 now */
  uint8_t num_used_rdi;              /* num RDI reserved */
  uint8_t num_used_pix;              /* num pix interfaces reserved */
} comp_res_axi_entry_t;

typedef struct {
  uint8_t max_interfaces;            /* max pixel interfaces */
  uint8_t num_used;                  /* num of used pixel interfaces */
} comp_res_vfe_entry_t;

typedef struct {
  uint8_t max_interfaces;            /* max interfaces */
  uint8_t num_used;                  /* num used interfaces */
} comp_res_camif_entry_t;

typedef struct {
  uint8_t max_clients;               /* max clients */
} comp_res_c2d_entry_t;

typedef struct {
  uint8_t max_clients;               /* max clients */
} comp_res_vpe_entry_t;

typedef struct {
  uint8_t max_clients;               /* max clients */
} comp_res_cpp_entry_t;

typedef struct {
  uint8_t max_clients;               /* max clients */
} comp_res_irqrouter_entry_t;

typedef struct {
  uint8_t max_clients;               /* max clients */
} comp_res_cci_entry_t;

typedef struct {
  comp_id_t comp_id;                /* comp id of the res */
  uint32_t  fps_cap;                /* fps cap if > 0 */
  uint32_t  fps_used;               /* fps used */
  uint8_t   num_reservations;       /* num reservation called */
  uint8_t   max_reservations;       /* allow unlimited reservation */
  uint32_t subdev_revision;         /*revision/subdev node number*/
  union {
    comp_res_axi_entry_t axi;             /* AXI HW */
    comp_res_irqrouter_entry_t irqrouter; /* IRQ router HW */
    comp_res_vfe_entry_t vfe;             /* VFE */
    comp_res_camif_entry_t camif;         /* CAMIF */
    comp_res_c2d_entry_t c2d;             /* C2D */
    comp_res_vpe_entry_t vpe;             /* VPE */
    comp_res_cpp_entry_t cpp;             /* CPP */
    comp_res_cci_entry_t cci;             /* CCI */
  }u;
  comp_res_client_t    def;             /* generic entry */
} comp_res_entries_t;

#define COMP_AXI_MAX_RDI_IF     3         /* max RDI interfaces */
#define COMP_AXI_MAX_PIX_IF     1         /* max pixel interface */
#define COMP_CAMIF_MAX_HWS      2         /* max camifs */
#define COMP_VFE_MAX_HWS        2         /* max vfes */
#define COMP_AXI_MAX_HWS        2         /* max axis */
#define COMP_C2D_MAX_HWS        2         /* max c2ds */
#define COMP_VPE_MAX_HWS        1         /* max vpes */
#define COMP_CPP_MAX_HWS        1         /* max cpps */
#define COMP_IRQROUTER_MAX_HWS  1         /* max irq routers */
#define COMP_CCI_MAX_HWS        1         /* max ccis */
#define COMP_MAX_HWS            16        /*sum of all above*/

typedef struct {
  uint8_t res_idx;                        /* resource index,
                                           * resource could be hw */
  uint8_t num_rdis;                       /* num RDI interfaces */
  uint8_t num_pixs;                       /* max = 1 now */
  uint8_t rdi_idx[COMP_AXI_MAX_RDI_IF];   /* returned rdi indexes */
  uint8_t pix_idx[COMP_AXI_MAX_PIX_IF];   /* returned  pix interfaces */
} comp_res_axi_if_req_t;

typedef struct {
  int num_res;                                  /* num resources */
  comp_res_axi_if_req_t axis[COMP_AXI_MAX_HWS]; /* axi req entry */
} comp_res_axi_req_t;

typedef struct {
  uint8_t num_res;                       /* num camif hws */
  uint8_t idx[COMP_CAMIF_MAX_HWS];       /* returned indexes */
} comp_res_camif_req_t;
typedef struct {
  uint8_t num_res;                       /* num vfe hws */
  uint8_t idx[COMP_VFE_MAX_HWS];         /* returned indexes */
} comp_res_vfe_req_t;
typedef struct {
  uint8_t num_res;                       /* num c2ds */
  uint8_t real_time_client;              /* is realtime client? */
  uint8_t idx[COMP_C2D_MAX_HWS];         /* returned indexes */
} comp_res_c2d_req_t;
typedef struct {
  uint8_t num_res;                       /* num vpes */
  uint8_t real_time_client;
  uint8_t idx[COMP_VPE_MAX_HWS];         /* returned indexes */
} comp_res_vpe_req_t;
typedef struct {
  uint8_t num_res;                       /* num cpps */
  uint8_t real_time_client;
  uint8_t idx[COMP_CPP_MAX_HWS];         /* returned indexes */
} comp_res_cpp_req_t;

typedef struct {
  uint8_t num_res;                       /* num irq routers */
  uint8_t idx[COMP_IRQROUTER_MAX_HWS];   /* returned indexes */
} comp_res_irqrouter_req_t;

typedef struct {
  uint8_t num_res;                       /* num ccis */
  uint8_t idx[COMP_CCI_MAX_HWS];         /* returned indexes */
} comp_res_cci_req_t;

typedef struct {
  comp_id_t comp_id;                     /* resource's compid */
  uint32_t  requested_fps;               /* 1080p based fps information */
  uint32_t  sdev_revision;               /* selected subdev number*/
  union {
    comp_res_cci_req_t       cci;        /* CCI */
    comp_res_axi_req_t       axi;        /* AXI */
    comp_res_camif_req_t     camif;      /* CAMIF */
    comp_res_vfe_req_t       vfe;        /* VFE */
    comp_res_c2d_req_t       c2d;        /* C2D */
    comp_res_vpe_req_t       vpe;        /* VPE */
    comp_res_cpp_req_t       cpp;        /* CPP */
    comp_res_irqrouter_req_t irqrouter;  /* IRQ ROUTER */
  }u;
} comp_res_req_info_t;


typedef struct {
  int fd;
} comp_ext_client_info_t;

/* reserve resources. One compid at a time */
extern int qcamsvr_reserve_res(uint32_t mctl_id,
  comp_res_req_info_t *res_req_info, comp_ext_client_info_t* p_ext);
/* release resource. One compid at a time. */
extern void qcamsvr_release_res(uint32_t mctl_id,
  comp_id_t res_comp_id,  comp_ext_client_info_t* p_ext);
extern int get_num_res(uint32_t mctl_id, comp_id_t comp_id, uint8_t *num_res);

#endif /* __INTF_COMM_DATA_H__ */
