/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __TGTCOMMON_H__
#define __TGTCOMMON_H__

/*===========================================================================
 *                         INCLUDE FILES
 *===========================================================================*/
#include <media/msm_camera.h>
#include <media/msm_isp.h>
#include "camera.h"
#include "vfe_stats_def.h"

/*===========================================================================
 *                         DATA DEFINITIONS
 *===========================================================================*/
#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

#define Q_TO_FLOAT(exp, q) \
  ((double)((double)(q-(q/(1<<(exp)))*(1<<(exp)))/(1<<(exp))+(q/(1<<(exp)))))

/* Return v1 * ratio + v2 * ( 1.0 - ratio ) */
#define LINEAR_INTERPOLATION(v1, v2, ratio) \
  ((v2) + ((ratio) * ((v1) - (v2))))

#define DEGREE_TO_RADIAN(d) \
  (0.0174532925 * d)

#define F_EQUAL(a, b) \
  ( fabs(a-b) < 1e-4 )

#define NUM_POLY_SCALER_COEFFS      17
#define NUM_VERT_POLY_SCALER_BANKS  16
#define NUM_HORIZ_POLY_SCALER_BANKS 16
#define AWB_NUMBER_OF_LIGHTING_CONDITION  \
  (ISP3A_AWB_HYBRID + 1)

#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define CEILING8(X)  (((X) + 0x0007) & 0xFFF8)
#define CEILING4(X)  (((X) + 0x0003) & 0xFFFC)
#define CEILING2(X)  (((X) + 0x0001) & 0xFFFE)
#define FLOOR16(X) ((X) & 0xFFF0)
#define FLOOR8(X)  ((X) & 0xFFF8)
#define FLOOR4(X)  ((X) & 0xFFFC)
#define FLOOR2(X)  ((X) & 0xFFFE)

/* Return the smallest integral value not less than x/y */
#define CAMCEIL(x,y)  ((x+(y-1))/y)
#define OFFSETOF(s,m) ((uint32_t)&(((s *)0)->m))

#define Q14   0x00004000
#define Q13   0x00002000
#define Q12   0x00001000
#define Q12_2 0x00002000
#define Q12_4 0x00004000
#define Q11   0x00000800
#define Q10   0x00000400
#define Q8    0x00000100
#define Q7    0x00000080
#define Q6    0x00000040
#define Q4    0x00000010

typedef enum {
  MARGIN_P_4 = 4,
  MARGIN_P_5 = 5,
  MARGIN_P_6 = 6,
  MARGIN_P_8 = 8,
  MARGIN_P_10 = 10,
  MARGIN_N_10,
  MARGIN_INVALID
} frame_margin_t;

typedef struct {
  int enable_dis;
  int sensor_has_margin;
  frame_margin_t dis_margin_p;
} dis_ctrl_info_t;

typedef struct {
  float c00;
  float c01;
  float c10;
  float c11;
} matrix_2x2_t;

typedef struct {
  float c00;
  float c01;
  float c02;
  float c10;
  float c11;
  float c12;
  float c20;
  float c21;
  float c22;
} matrix_3x3_t;

/*Data structures exposed by sensor to VFE*/
typedef enum {
  CAMIF_BAYER_G_B,
  CAMIF_BAYER_B_G,
  CAMIF_BAYER_G_R,
  CAMIF_BAYER_R_G,
  CAMIF_YCBCR_Y_CB_Y_CR,
  CAMIF_YCBCR_Y_CR_Y_CB,
  CAMIF_YCBCR_CB_Y_CR_Y,
  CAMIF_YCBCR_CR_Y_CB_Y,
  CAMIF_YCBCR_4_2_2_LINEPACKED, /* Only valid for Offline Snapshot */
  CAMIF_YCBCR_4_2_0_LINEPACKED, /* Only valid for Offline Snapshot */
  CAMIF_NUMBEROF_INPUTFORMATTYPE  /* Used for count purposes only */
} sensor_camif_inputformat_t;

typedef enum effects_t {
  EFFECTS_DEFAULT,
  EFFECTS_SEPIA,
  EFFECTS_MONO,
  EFFECTS_NEGATIVE,
  EFFECTS_AQUA,
  EFFECTS_POSTERIZE,
  EFFECTS_BLACKBOARD,
  EFFECTS_WHITEBOARD,
  EFFECTS_SOLARIZE,
  EFFECTS_EMBOSS,
  EFFECTS_SKETCH,
  EFFECTS_NEON,
  EFFECTS_FADED,
  EFFECTS_VINTAGE_COOL,
  EFFECTS_VINTAGE_WARM,
  EFFECTS_ACCENT_BLUE,
  EFFECTS_ACCENT_GREEN,
  EFFECTS_ACCENT_ORANGE,
} effects_t;

typedef enum {
  GAMMA_TABLE_DEFAULT_GEN = 0,
  GAMMA_TABLE_OUTDOOR_GEN,
  GAMMA_TABLE_LOWLIGHT_GEN,
  GAMMA_TABLE_BACKLIGHT_GEN,
  GAMMA_TABLE_SOLARIZE_GEN,
  GAMMA_TABLE_POSTERIZE_GEN,
  GAMMA_TABLE_NOT_VALID
} gamma_table_t;

typedef struct {
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
}pixel_crop_info_t;

typedef struct {
  uint32_t pixel_skip;
  uint32_t line_skip;
}pixel_skip_info_t;

typedef struct {
  uint32_t x; /* left */
  uint32_t y; /* top */
  uint32_t crop_out_x; /* width */
  uint32_t crop_out_y; /* height */
}crop_window_info_t;

typedef enum {
  STATS_PROC_ANTIBANDING_OFF,
  STATS_PROC_ANTIBANDING_60HZ,
  STATS_PROC_ANTIBANDING_50HZ,
  STATS_PROC_ANTIBANDING_AUTO,
  STATS_PROC_MAX_ANTIBANDING,
} stats_proc_antibanding_type;

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t dx;
  uint16_t dy;
} stats_proc_roi_t;

typedef enum {
  ROI_TYPE_GENERAL,
  ROI_TYPE_FACE,
} stats_proc_roi_type_t;

typedef struct {
  int width;
  int height;
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
} camera_size_t;

typedef struct {
  uint32_t width;
  uint32_t height;
  int format;
  uint32_t inst_handle;
} camera_resolution_t;

typedef enum {
  CAM_OP_MODE_INVALID,
  CAM_OP_MODE_PREVIEW,
  CAM_OP_MODE_SNAPSHOT,
  CAM_OP_MODE_RAW_SNAPSHOT,
  CAM_OP_MODE_VIDEO,
  CAM_OP_MODE_ZSL,
  CAM_OP_MODE_JPEG_SNAPSHOT,
  CAM_OP_MODE_MAX,
} camera_op_mode_t;

#define CAMERA_STATUS_SUCCESS                         CAMERA_SUCCESS
#define CAMERA_STATUS_ERROR_GENERAL               CAMERA_ERR_GENERAL
#define CAMERA_STATUS_NO_MEMORY                 CAMERA_ERR_NO_MEMORY
#define CAMERA_STATUS_INVALID_OPERATION CAMERA_ERR_INVALID_OPERATION
#define CAMERA_STATUS_INVALID_INPUT         CAMERA_ERR_INVALID_INPUT

typedef struct {
  int32_t id;
  uint16_t length;
  void *value;
} vfe_message_t;

typedef enum {
  PP_HW_TYPE_VPE, /*VPE PP HW*/
  PP_HW_TYPE_C2D, /*C2D PP HW INTF*/
  PP_HW_TYPE_S3D, /*S3D PP HW INTF*/
  PP_HW_TYPE_MAX_NUM
} tgt_pp_hw_t;

typedef struct {
  int32_t  dis_frame_width; /* Original */
  int32_t  dis_frame_height; /* Original */
  int32_t  vfe_output_width; /* Padded */
  int32_t  vfe_output_height; /* Padded */
  uint16_t frame_fps;      /* image fps */
} mctl_dis_frame_cfg_type_t;

#define MCTL_AXI_COMID                            0x01FFFFFF
#define MCTL_CAMIF_COMID                          0x02FFFFFF
#define MCTL_VFE_COMID                            0x03FFFFFF
#define MCTL_STATS_PROC_COMID                     0x04FFFFFF
#define MCTL_SENSOR_COMID                         0x05FFFFFF

#define MCTL_FETCH_PARM_TYPE_NOT_USED             0

#define MCTL_FETCH_PARM_TYPE_CAMIF_OUT_WIN_SIZE  (MCTL_CAMIF_COMID + 1)
#define MCTL_FETCH_PARM_TYPE_VFE_CS_RS_CFG_INFO  (MCTL_VFE_COMID + 1)

typedef enum {
  AXI_INTF_PIXEL_0,
  AXI_INTF_RDI_0,
  AXI_INTF_PIXEL_1,
  AXI_INTF_RDI_1,
  AXI_INTF_PIXEL_2,
  AXI_INTF_RDI_2,
  AXI_INTF_MAX_NUM
} axi_intf_type_t;

typedef struct
{
    int  row_sum_enable;   // 0=disable, 1=enable, default=1
    float   row_sum_hor_Loffset_ratio;  // default=0;
    float   row_sum_ver_Toffset_ratio;  //default=0;
    float   row_sum_hor_Roffset_ratio;  // default=0;
    float   row_sum_ver_Boffset_ratio;  //default=0;
    unsigned char   row_sum_V_subsample_ratio; //1 to 4, int
    unsigned char   row_sum_H_subsample_ratio;


    int  col_sum_enable;   // 0=disable, 1=enable, default=0
    float   col_sum_hor_Loffset_ratio;  //default=0
    float   col_sum_ver_Toffset_ratio;  // default=0;
    float   col_sum_hor_Roffset_ratio;  //default=0
    float   col_sum_ver_Boffset_ratio;  // default=0;
    unsigned char   col_sum_H_subsample_ratio; //2 to 4, int
    unsigned char   col_sum_V_subsample_ratio; //2 to 4, int

} rscs_stat_config_type;

typedef struct {
  rscs_stat_config_type config;
  uint32_t rs_max_rgns;
  uint32_t rs_max_h_rgns;
  uint32_t rs_num_rgns; /* TODO: vertical num of regions*/
  uint32_t rs_num_h_rgns;
  uint32_t rs_shift_bits;
  uint32_t cs_max_rgns;
  uint32_t cs_max_v_rgns;
  uint32_t cs_num_rgns; /*TODO: horiz num of regions*/
  uint32_t cs_num_v_rgns;
  uint32_t cs_shift_bits;
}vfe_stats_rs_cs_params;

typedef struct {
  int fd;
  void *local_vaddr;
} mctl_pp_local_buf_info_t;

typedef struct {
  void *cookie;
  uint32_t frame_id;
  cam_format_t src_format;
  cam_format_t dest_format;
  int action_flag;
  struct msm_cam_evt_divert_frame *src;
  struct msm_cam_evt_divert_frame *dest;
  struct msm_pp_crop crop;
  mctl_pp_local_buf_info_t src_buf_data;
  mctl_pp_local_buf_info_t dst_buf_data;
  int path;
  /* TBD: 3D related */
  /* return flag by VPE/CPP/C2D if no pp needed */
  int not_pp_flag;
  int status;
} pp_frame_data_t;

#define TGT_OK                 0
#define TGT_FAIL              -1
#define TGT_ERR_NO_MEM        -2
#define TGT_ERR_NOT_SUPPORT   -3
#define TGT_ERR_NOT_FOUND     -4
#define TGT_ERR_BUSY          -5

typedef struct {
  uint32_t buf_size;
  uint8_t  *buf;
  int32_t  fd;
  struct ion_allocation_data ion_alloc;
  struct ion_fd_data fd_data;
} mem_buffer_struct_t;

typedef struct {
  int fd;
  void *parent;
  int (*buf_done) (void *userdata, pp_frame_data_t *frame);
  int (*fetch_params) (int my_comp_id, void *userdata, uint32_t type,
    void *params, int params_len);
  int (*get_mem_buffer) (void *userdata, mem_buffer_struct_t *buf,
    uint32_t size);
  int (*put_mem_buffer) (void *userdata, mem_buffer_struct_t *buf);
  int (*cache_invalidate) (void *userdata, mem_buffer_struct_t *buf);
  void *(*get_stats_op_buffer) (void *userdata, void *div_frame,
    enum msm_stats_enum_type type);
  int (*put_stats_op_buffer) (void *userdata, void *div_frame,
    enum msm_stats_enum_type type);
} mctl_ops_t;

typedef struct {
  uint32_t output1w;
  uint32_t output1h;
  uint32_t output2w;
  uint32_t output2h;
}vfe_axi_output_dim_t;

#define AWB_AEC_STATS_BUFNUM 3
#define AF_STATS_BUFNUM      3
#define AEC_STATS_BUFNUM     3
#define AWB_STATS_BUFNUM     4
#define IHIST_STATS_BUFNUM   4
#define CS_STATS_BUFNUM      5
#define RS_STATS_BUFNUM      5

#define BE_STATS_BUFNUM      4
#define BG_STATS_BUFNUM      4
#define BF_STATS_BUFNUM      4
#define BHIST_STATS_BUFNUM   4

#define MCTL_AEC_NUM_REGIONS      256

typedef enum {
  STATS_TYPE_NOTUSED,
  STATS_TYPE_YUV,
  STATS_TYPE_BAYER
} stats_type_t;

typedef struct {
  uint8_t cur_be_idx;
  mem_buffer_struct_t BeBuf[BE_STATS_BUFNUM];
  uint8_t cur_bg_idx;
  mem_buffer_struct_t BgBuf[BG_STATS_BUFNUM];
  uint8_t cur_bf_idx;
  mem_buffer_struct_t BfBuf[BF_STATS_BUFNUM];
  uint8_t cur_bhist_idx;
  mem_buffer_struct_t BhistBuf[BHIST_STATS_BUFNUM];
} bayer_stats_buffer_t;

typedef struct {
  uint8_t cur_AwbAecBuf;
  mem_buffer_struct_t AwbAecBuf[AWB_AEC_STATS_BUFNUM];
  uint8_t cur_AecBuf;
  mem_buffer_struct_t AecBuf[AEC_STATS_BUFNUM];
  uint8_t cur_AFBuf;
  mem_buffer_struct_t AfBuf[AF_STATS_BUFNUM];
}legacy_stats_buffer_t;

typedef struct {
  uint8_t cur_IhistBuf;
  mem_buffer_struct_t IhistBuf[IHIST_STATS_BUFNUM];
  uint8_t cur_CSBuf;
  mem_buffer_struct_t CSBuf[CS_STATS_BUFNUM];
  uint8_t cur_RSBuf;
  mem_buffer_struct_t RSBuf[RS_STATS_BUFNUM];
  uint8_t cur_AwbBuf;
  mem_buffer_struct_t AwbBuf[AWB_STATS_BUFNUM];
}common_stats_buffer_t;

typedef struct {
  stats_type_t type;
  union {
    bayer_stats_buffer_t bayer_stats_buf;
    legacy_stats_buffer_t legacy_stats_buf;
  }s;
  common_stats_buffer_t common_stats_buf;
} stats_buffers_type_t;

typedef enum {
  STATS_PARSER_STATE_NOTUSED,
  STATS_PARSER_STATE_INITED
} stats_parser_state_type_t;

typedef struct {
  stats_buffers_type_t stats_bufs;
  stats_parser_state_type_t state;
  uint32_t stats_marker;
} stats_parser_mod_t;

typedef struct {
  uint32_t handle;
  int pipe_fd[2];
  int (*init)(uint32_t handle, mctl_ops_t *ops, void *init_data);
  int (*set_params)(uint32_t handle, int type, void *parm_in, void *parm_out);
  int (*get_params)(uint32_t handle, int type, void *parm, int parm_len);
  int (*process)(uint32_t handle, int event, void *data);
  int (*parse_stats)(uint32_t handle, int isp_started, stats_type_t stats_type,
    void *stats, void *stats_output);
  void (*abort)(uint32_t handle);
  int (*destroy)(uint32_t handle);
} module_ops_t;

typedef struct {
  void *ptr;
  int (*comp_create)(void);
  uint32_t (*client_open)(module_ops_t *ops);
  int (*comp_destroy)();
}mctl_comp_lib_t;

typedef struct {
  uint16_t stride;
} plane_info_t;

typedef struct {
  cam_format_t format;
  uint16_t image_width; /* Always use (image_width + extra_pad_width) */
  uint16_t image_height; /* Always use (image_height + extra_pad_height) */
  uint16_t extra_pad_width;
  uint16_t extra_pad_height;
  uint32_t stream_type;
  plane_info_t plane[MAX_PLANES];
  int path;
  uint32_t inst_handle;
}vfe_output_info_t;

#define  PRIMARY      0
#define  SECONDARY    1
#define  TERTIARY1     2
#define  TERTIARY2     3
#define  TERTIARY3     4


typedef enum {
  VFE_OUTPUT_PRIMARY = 0x01,
  VFE_OUTPUT_SECONDARY = 0x02,
  VFE_OUTPUT_TERTIARY1 = 0x04,
  VFE_OUTPUT_TERTIARY2 = 0x08,
  VFE_OUTPUT_TERTIARY3 = 0x10,
  VFE_OUTPUT_INVALID = 0x00,
}vfe_ports_used_t;

typedef struct {
  uint8_t num_output;
  uint8_t  vfe_ports_used;
  uint8_t active_ports;
  uint8_t pending_ports; /* Keep track of ports for START_ACK/STOP_ACK */
  vfe_output_info_t output[MAX_OUTPUT_SUPPORTED];
  uint32_t vfe_operation_mode;
}current_output_info_t;

typedef struct {
  //preview or thumbnail
  uint32_t output1period;
  uint32_t output1pattern;
  //vidoe or main image
  uint32_t output2period;
  uint32_t output2pattern;
}vfe_frame_skip;

typedef struct {
  uint32_t output1w;
  uint32_t output1h;
  uint32_t output2w;
  uint32_t output2h;
  crop_window_info_t crop_win;
} vfe_zoom_crop_info_t;

typedef struct {
  int x;
  int y;
} pixel_t;

typedef struct {
  pixel_t *pix;
  int count;
} pixels_array_t;

typedef enum {
  MOD_CMD_RESET,
  MOD_CMD_START,
  MOD_CMD_STOP,
  MOD_CMD_ABORT,
}mod_cmd_ops_t;

typedef struct {
 mod_cmd_ops_t mod_cmd_ops;
 void * cmd_data;
 uint16_t length;
}mod_cmd_t;

/* vfe/isp provided information */
typedef struct {
  vfe_stats_ae_data_t aec_op;
  vfe_stats_ae_data_t be_op;
  vfe_stats_awb_data_t awb_op;
  vfe_stats_af_data_t af_op;
  vfe_stats_rs_data_t rs_op;
  vfe_stats_cs_data_t cs_op;
  vfe_stats_ihist_data_t ihist_op;
  /* Bayer Grid Stats */
  uint32_t bayer_r_sum[3888];
  uint32_t bayer_b_sum[3888];
  uint32_t bayer_gr_sum[3888];
  uint32_t bayer_gb_sum[3888];
  uint32_t bayer_r_num[3888];
  uint32_t bayer_b_num[3888];
  uint32_t bayer_gr_num[3888];
  uint32_t bayer_gb_num[3888];

  /* Bayer Focus Stats */
  uint32_t bf_stats[2521];
  uint32_t bf_r_sum[255];
  uint32_t bf_b_sum[255];
  uint32_t bf_gr_sum[255];
  uint32_t bf_gb_sum[255];
  uint32_t bf_r_sharp[255];
  uint32_t bf_b_sharp[255];
  uint32_t bf_gr_sharp[255];
  uint32_t bf_gb_sharp[255];
  uint32_t bf_r_num[255];
  uint32_t bf_b_num[255];
  uint32_t bf_gr_num[255];
  uint32_t bf_gb_num[255];

  /* Bayer Histogram Stats */
  uint32_t bayer_r_hist[255];
  uint32_t bayer_b_hist[255];
  uint32_t bayer_gr_hist[255];
  uint32_t bayer_gb_hist[255];
} isp_stats_t;

typedef struct {
  uint32_t rgn_width;
  uint32_t rgn_height;
  uint32_t shift_bits;
}vfe_stats_aec_params;

typedef struct {
  uint32_t frame_width;
  uint32_t frame_height;
  uint32_t camif_width;
  uint32_t camif_height;
  uint32_t multi_roi_nfocus;
  roi_t region;
  uint8_t roi_type;
  uint8_t *af_mult_window;
}vfe_stats_af_params_t;

typedef struct {
  vfe_stats_aec_params aec_params;
  vfe_stats_rs_cs_params rs_cs_params;
  uint32_t ihist_shift_bits;
  uint32_t awb_shift_bits;
  float blk_inc_comp;
  int numRegions;
  int comp_done;
  int aec_bg_done;
  int awb_done;
  int af_bf_done;
  int rs_done;
  int cs_done;
  int be_done;
  int wbexp_done;
  int ihist_done;
  int bhist_done;
  int zero_hregions;
  int zero_vregions;
  isp_stats_t vfe_stats_struct;
  uint16_t ihist_stats_buffer[256];
  uint8_t ihist_index;
}vfe_stats_output_t;

/* definition of camera configuration state */
typedef enum {
  CAMERA_STATE_IDLE,
  CAMERA_STATE_SENT_RESET,
  CAMERA_STATE_RESET,
  CAMERA_STATE_SENT_STOP,
  CAMERA_STATE_SENT_START,
  CAMERA_STATE_STARTED,
  CAMERA_STATE_ERROR,

  CAMERA_STATE_INVALID = 0xFFFFFFFF,
} config_state_t;

typedef enum {
  VFE_STATS_REGBUF,
  VFE_STATS_UNREGBUF,
  VFE_STATS_INVALID = 0xFFFFFFFF,
}vfe_stats_buf_state_t;

typedef struct {
  uint32_t type;
  void *mod_ptr;
}vfe_plugin_module_params_t;

/*ISP 3A diagnostics*/
typedef struct {
  int32_t coef_rtor;
  int32_t coef_gtor;
  int32_t coef_btor;
  int32_t coef_rtog;
  int32_t coef_btog;
  int32_t coef_gtog;
  int32_t coef_rtob;
  int32_t coef_gtob;
  int32_t coef_btob;
  int32_t roffset;
  int32_t boffset;
  int32_t goffset;
  uint32_t coef_qfactor;
  int32_t enable;
}QColorCorrect_t;

typedef struct
{
  uint16_t rlut_pl[8];
  uint16_t grlut_pl[8];
  uint16_t gblut_pl[8];
  uint16_t blut_pl[8];
  uint16_t rlut_base[9];
  uint16_t grlut_base[9];
  uint16_t gblut_base[9];
  uint16_t blut_base[9];
  uint32_t rlut_delta[9];
  uint32_t grlut_delta[9];
  uint32_t gblut_delta[9];
  uint32_t blut_delta[9];
  int32_t enable;
}QLinearization_t;

typedef struct {
  uint32_t wk;
  uint32_t bk;
  uint32_t lk;
  uint32_t tk;
} QDemosaicClassifier_t;

typedef struct {
  QDemosaicClassifier_t lut[18];
  uint32_t aG;
  uint32_t bL;
  int32_t enable;
} QDemosaic_t;

typedef struct {
  float coefftable_R[13][8];
  float coefftable_Gr[13][8];
  float coefftable_Gb[13][8];
  float coefftable_B[13][8];
  float basistable[8][17];
  int enable;
}QLensCorrection_t;

typedef struct {
  uint8_t table[64];
  int enable;
}QGamma_t;

typedef struct {
  uint16_t threshold[3];
  uint16_t pos[16];
  int16_t neg[8];
} QABFData_t;

typedef struct {
  QABFData_t red;
  QABFData_t green;
  QABFData_t blue;
  int enable;
} QABF_t;

typedef struct {
  int32_t enable;
  uint32_t smoothfilterEnabled;
  uint32_t sharpMode;
  uint32_t lpfMode;
  uint32_t smoothcoefCenter;
  uint32_t smoothcoefSurr;
  uint32_t pipeflushCount;
  uint32_t pipeflushOvd;
  uint32_t flushhaltOvd;
  uint32_t cropEnable;
  uint32_t normalizeFactor;
  uint32_t sharpthreshE1;
  int32_t sharpthreshE2;
  int32_t sharpthreshE3;
  int32_t sharpthreshE4;
  int32_t sharpthreshE5;
  int32_t sharpK1;
  int32_t sharpK2;

  int32_t f1coef0;
  int32_t f1coef1;
  int32_t f1coef2;
  int32_t f1coef3;
  int32_t f1coef4;
  int32_t f1coef5;
  int32_t f1coef6;
  int32_t f1coef7;
  int32_t f1coef8;
  int32_t f2coef0;
  int32_t f2coef1;
  int32_t f2coef2;
  int32_t f2coef3;
  int32_t f2coef4;
  int32_t f2coef5;
  int32_t f2coef6;
  int32_t f2coef7;
  int32_t f2coef8;
  int32_t f3coef0;
  int32_t f3coef1;
  int32_t f3coef2;
  int32_t f3coef3;
  int32_t f3coef4;
  int32_t f3coef5;
  int32_t f3coef6;
  int32_t f3coef7;
  int32_t f3coef8;
} QASF_t;

typedef struct {
  uint32_t y1;
  uint32_t y2;
  uint32_t y3;
  uint32_t y4;
  uint32_t yM1;
  uint32_t yM3;
  uint32_t yS1;
  uint32_t yS3;
  uint32_t transWidth;
  uint32_t transTrunc;
  int32_t crZone;
  int32_t cbZone;
  int32_t translope;
  int32_t k;
} QMCEData_t;

typedef struct {
  int32_t enable;
  uint32_t qk;
  QMCEData_t red;
  QMCEData_t green;
  QMCEData_t blue;
} QMemoryColor_t;

typedef struct {
  QColorCorrect_t cc;
  QLinearization_t linearization;
  QDemosaic_t demosaic;
  QLensCorrection_t lc;
  QGamma_t gamma;
  QABF_t abf;
  QMemoryColor_t mce;
  QASF_t asf;
} QISPInfo_t;

typedef struct {
  float rGain;
  float gGain;
  float bGain;
} QWB_gain_t;

typedef struct {
  uint32_t colorTemp;
  QWB_gain_t wbGain;
} QAWBInfo_t;

typedef struct {
  int expIndex;
  int lightCond;
  int expMode;
  int expBias;
  float analogGain;
  float expTime;
} QAECInfo_t;

typedef struct {
  int focusMode;
  int cafStatus;
  int startLensPos;
  int finalLensPos;
  int numOfFocusArea;
  roi_t focusArea[MAX_ROI];
  int focusValue[50];
  int focusSteps[50];
  int index;  /* internal use only */
} QAFInfo_t;

typedef struct {
  QAWBInfo_t awb_info;
  QAECInfo_t aec_info;
  QAFInfo_t af_info;
} Q3AInfo_t;

typedef struct {
  QISPInfo_t isp_info;
  Q3AInfo_t stats_proc_info;
}QCameraInfo_t;

#define VFE_STATS_AEC     0x2000  /* bit 13 */
#define VFE_STATS_AF      0x4000  /* bit 14 */
#define VFE_STATS_AWB     0x8000  /* bit 15 */
#define VFE_STATS_RS      0x10000  /* bit 16 */
#define VFE_STATS_CS      0x20000  /* bit 17 */
#define VFE_STATS_IHIST   0x40000  /* bit 18 */

#endif /* __TGTCOMMON_H__ */
