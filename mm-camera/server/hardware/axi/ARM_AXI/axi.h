/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __AXI_H__
#define __AXI_H__

#include "camera.h"
#include "axi_interface.h"

/* burst length = 8 for all line and frame write masters.*/
#define TOTAL_SIZE_UB_BUFFER_FOR_IMAGE_MASTERS  920
#define TOTAL_SIZE_UB_BUFFER_FOR_IMAGE_MASTERS_DEMOSAIC  920

/*1024 double words sliced equally for 7 Write Masters*/
#ifdef VFE_40
#define TOTAL_SIZE_UB_BUFFER_FOR_IMAGE_MASTERS_BAYER     760
#define EQUALLY_SLICED_UB_SIZE 108
#else
#define TOTAL_SIZE_UB_BUFFER_FOR_IMAGE_MASTERS_BAYER     848
#define EQUALLY_SLICED_UB_SIZE 131
#endif

/*XBAR CONFIG MACROS*/
/* sel_shift_bit*/
#ifdef VFE_40
#define STREAM_SOURCE_SHIFT 0
#define STREAM_TYPE_SHIFT 1
#define PIXEL_ALIGNMENT_SHIFT 2
#define SWAP_CTRL_SHIFT 4
#define STREAM_SEL_SHIFT 8
#else
#define STREAM_SOURCE_SHIFT 0
#define STREAM_TYPE_SHIFT 1
#define PIXEL_ALIGNMENT_SHIFT 2
#define SWAP_CTRL_SHIFT 3
#define STREAM_SEL_SHIFT 5
#endif

/* Single stream sel*/
#define Y_SINGLE_STREAM 0
#define CB_SINGLE_STREAM 1
#define CR_SINGLE_STREAM 2
#define RAW_CAMIF_STREAM 3
#define IDEAL_RAW_STREAM 4
#define RDI0_STREAM 5
#define RDI1_STREAM 6
#define RDI2_STREAM 7
#define DONT_CARE_STREAM 0
/*Pair stream swap ctrl*/
#define NO_SWAP 0
#define INTRA_SWAP_ONLY 1
#define INTER_SWAP_ONLY 2
#define INTRA_AND_INTER_SWAP 3
#define DONT_CARE_SWAP 0
/*PIXEL ALIGNMENT*/
#define Y_CBCR_UNALIGNED 0
#define Y_CBCR_ALIGNED 1
/*Single or paired stream */
#define SINGLE_STREAM 0
#define PAIRED_STREAM 1
/*Viewfinder or Encoder stream*/
#define ENCODER_STREAM 0
#define VIEWFINDER_STREAM 1

#define AXI_MAX_CLIENT_NUM    2

#define AXI_MAX_WM_NUM	      7

struct output_ch {
  int32_t   ch0             : 16;
  int32_t   ch1             : 16;
  int32_t   ch2             : 16;
  int32_t   /* reserved */  : 16;
  int32_t   inst_handle     : 32;
}__attribute__((packed, aligned(4)));

typedef struct VFE_PixelIfCfg {
  uint32_t inputSelect          : 2;
  uint32_t rdiEnable            : 1;
  uint32_t /* reserved */       : 1;
  uint32_t rdiM0Select          : 4;
  uint32_t rdiM1Select          : 4;
  uint32_t rdiM2Select          : 4;
  uint32_t rdiM0FrameBasedEnable: 1;
  uint32_t rdiM1FrameBasedEnable: 1;
  uint32_t rdiM2FrameBasedEnable: 1;
  uint32_t /* reserved */       : 1;
  uint32_t rdiFrameSkip         : 4;
  uint32_t rdiFrameSkipEnable   : 1;
  uint32_t /* reserved */       : 3;
  uint32_t rdiStreamSelect      : 4;
} __attribute__((packed, aligned(4))) VFE_PixelIfCfg;

typedef struct VFE_RdiCfg0 {
  uint32_t /*reserved*/         : 2;
  uint32_t rdiEnable            : 1;
  uint32_t /* reserved */       : 1;
  uint32_t rdiM3Select          : 4;
  uint32_t rdiM4Select          : 4;
  uint32_t rdiM5Select          : 4;
  uint32_t /* reserved */       : 4;
  uint32_t rdiFrameSkip         : 4;
  uint32_t rdiFrameSkipEnable   : 1;
  uint32_t /*reserved*/         : 3;
  uint32_t rdiStreamSelect1     : 4;
} __attribute__((packed, aligned(4))) VFE_RdiCfg0;

typedef struct VFE_RdiCfg1 {
  uint32_t /*reserved*/         : 2;
  uint32_t rdiEnable            : 1;
  uint32_t /* reserved */       : 1;
  uint32_t rdiM6Select          : 4;
  uint32_t rdiM7Select          : 4;
  uint32_t rdiM8Select          : 4;
  uint32_t /* reserved */       : 4;
  uint32_t rdiFrameSkip         : 4;
  uint32_t rdiFrameSkipEnable   : 1;
  uint32_t /*reserved*/         : 3;
  uint32_t rdiStreamSelect2     : 4;
} __attribute__((packed, aligned(4))) VFE_RdiCfg1;


typedef struct output_path {
  struct output_ch out0;
  struct output_ch out1;
  struct output_ch out2;
  struct output_ch out3;
}__attribute__((packed, aligned(4)))output_path;

typedef struct vfe_wm_config {
  uint32_t wmEnable          :  1;
  uint32_t /* reserved */    :  31;
  uint32_t busPingAddr       :  32;
  uint32_t busPongAddr       :  32;
#ifdef VFE_40
  uint32_t mal_en            :  1;
  uint32_t ocmem_en          :  1;
  uint32_t framedrop_period  :  5;
  uint32_t /* reserved */    :  1;
  uint32_t irq_sub_period    :  5;
  uint32_t /* reserved */    :  3;
  uint32_t frame_increment   :  13;
  uint32_t /* reserved */    :  3;
#endif
#ifdef VFE_40
  uint32_t busUbDepth        :  11;
  uint32_t /* reserved */    :  5;
  uint32_t busUbOffset       :  11;
  uint32_t /* reserved */    :  5;
#else
  uint32_t busUbDepth        :  10;
  uint32_t /* reserved */    :  6;
  uint32_t busUbOffset       :  10;
  uint32_t /* reserved */    :  6;
#endif
  uint32_t buslinesPerImage  :  12;
  uint32_t /* reserved */    :  4;
  uint32_t busdwordsPerLine  :  10;
  uint32_t /* reserved */    :  6;
  uint32_t busburstLength    :  2;
  uint32_t /* reserved */    :  2;
  uint32_t busbufferNumRows  :  12;
  uint32_t busrowIncrement   :  13;
  uint32_t /* reserved */    :  3;
#ifdef VFE_40
  uint32_t framedrop_pattern :  32;
  uint32_t irq_sub_pattern   :  32;
#endif
}__attribute__((packed, aligned(4))) vfe_wm_config;

typedef struct VFE_AXIOutputConfigCmdType {
#ifdef VFE_40
    uint32_t         busCmd        :  32;
    uint32_t         busCfg        :  32;
    uint32_t         busioFormat   :  32;
#else
#ifdef VFE_31
  uint32_t         /*reserved*/  :  32;
#else
  uint32_t         busioFormat   :  32;
#endif
  uint32_t         busCmd        :  32;
  /* busCfg's 31st bit config is as following
   * 7x30: OOO_WRITE_ENABLE  ---  Not Used
   * 8x60: Reserved          ---  Not Used
   * 8960: IMEM Mode disable (For Inline-JPEG only)
   */
  uint32_t         busCfg        :  32;
#endif
  uint32_t         xbarCfg0      :  32;
  uint32_t         xbarCfg1      :  32;
#ifdef VFE_40
  uint32_t         xbarCfg2      :  32;
  uint32_t         xbarCfg3      :  32;
#endif
  uint32_t         busWrSkipCfg  :  32;
  vfe_wm_config    wm[AXI_MAX_WM_NUM];
  output_path      outpath;
  VFE_PixelIfCfg   pixelIfCfg;
  VFE_RdiCfg0      rdiCfg0;
  VFE_RdiCfg1      rdiCfg1;
}__attribute__((packed, aligned(4))) VFE_AXIOutputConfigCmdType;

typedef struct VFE_XBarConfigCmdType {
  uint32_t         xbarCfg0      :  32;
  uint32_t         xbarCfg1      :  32;
}__attribute__((packed, aligned(4))) VFE_XBarConfigCmdType;

typedef struct {
  cam_format_t rdi_format;
  current_output_info_t output_info;
  axi_sensor_data_t sensor_data;
}axi_rdi_intf_ctrl_t;

typedef struct {
  current_output_info_t output_info;
  cam_format_t prev_format;
  cam_format_t rec_format;
  cam_format_t snap_format;
  cam_format_t thumb_format;
  axi_sensor_data_t sensor_data;
}axi_pix_intf_ctrl_t;

typedef struct {
  axi_intf_type_t intf_type;
  uint8_t used;
  uint8_t client_idx;
  union {
    axi_pix_intf_ctrl_t pix;
    axi_rdi_intf_ctrl_t rdi;
  };
}axi_ctrl_t;

typedef enum {
  AXI_WM_IDLE = 0,
  AXI_WM_SENT_START,
  AXI_WM_ACTIVE,
  AXI_WM_SENT_STOP,
}axi_wm_state_t;

typedef struct {
  int ub_offset;
  int ub_size;
}axi_ub_info_t;

typedef struct {
  int interface;
  axi_wm_state_t state;
  axi_ub_info_t ub_info;
  vfe_ports_used_t port;
  int is_reserved;
  uint8_t use_count;
}axi_wm_table_entry_t;

typedef struct {
  int ref_count;
  uint8_t reset_done;
  axi_ctrl_t axi_ctrl[AXI_INTF_MAX_NUM];
  axi_wm_table_entry_t axi_wm_table[AXI_MAX_WM_NUM];
  VFE_AXIOutputConfigCmdType VFE_AXIOutputConfigCmd;
} axi_obj_t;

typedef struct {
  uint32_t obj_idx_mask;
  uint8_t client_idx;
  uint8_t my_comp_id;
  uint32_t handle;
  uint32_t vfe_version;
  uint32_t stats_version;
  mctl_ops_t *ops;
  int sdev_fd;
  int current_target;
} axi_client_t;

#define AXI_MAX_OBJS 2   /* for now only need 2 AXI objects */

typedef struct {
  pthread_mutex_t mutex;
  uint32_t axi_handle_cnt;
  axi_client_t client[AXI_MAX_CLIENT_NUM];
  axi_obj_t axi_obj[AXI_MAX_OBJS];
} axi_comp_root_t;

int axi_config(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  axi_config_t *cfg);
int axi_command_ops(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  void *parm);
int axi_set_params(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  int type, axi_set_t *axi_set_parm);
int axi_proc_reg_update(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  void *parm);
int axi_proc_unregister_wms(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  void *parm);
//int8_t vfe_update_preview_format(void * , cam_format_t cam_format);

//#define ENABLE_AXI_LOGGING
#endif
