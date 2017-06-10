/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __CAMIF_H__
#define __CAMIF_H__

#include <inttypes.h>
#include "camif_interface.h"

/* CAMIF registers */
typedef struct VFE_CAMIFConfigCmdType {
  /* CAMIF Config */
  uint32_t  /* reserved */     : 1;
  uint32_t  vSyncEdge          : 1;
  uint32_t  hSyncEdge          : 1;
  uint32_t  syncMode           : 2;
  uint32_t  vfeSubSampleEnable : 1;
  uint32_t  /* reserved */     : 1;
  uint32_t  busSubSampleEnable : 1;
  uint32_t  mipi_sel           : 1;
  uint32_t  /* reserved */     : 2;
  uint32_t  irqSubSampleEnable : 1;
  uint32_t  /* reserved */     : 20;

  /* EFS_Config */
  uint32_t efsEndOfLine        : 8;
  uint32_t efsStartOfLine      : 8;
  uint32_t efsEndOfFrame       : 8;
  uint32_t efsStartOfFrame     : 8;

  /* Frame Config */
  uint32_t pixelsPerLine       : 14;
  uint32_t linesPerFrame       : 14;
  uint32_t /* reserved */      : 4;

  /* Window Width Config */
  uint32_t lastPixel           : 14;
  uint32_t firstPixel          : 14;
  uint32_t /* reserved */      : 4;

  /* Window Height Config */
  uint32_t lastLine            : 14;
  uint32_t firstLine           : 14;
  uint32_t /* reserved */      : 4;

  /* Subsample 1 Config */
  uint32_t pixelSkipMask       : 16;
  uint32_t lineSkipMask        : 16;

  /* Subsample 2 Config */
  uint32_t frameSkip           : 4;
  uint32_t pixelSkipWrap       : 1;
  uint32_t /* reserved */      : 27;

  /* Epoch Interrupt */
  uint32_t epoch1Line          : 14;
  uint32_t epoch2Line          : 14;
  uint32_t /* reserved */      : 2;
  uint32_t epoch1IntEna        : 1;
  uint32_t epoch2IntEna        : 1;
}__attribute__((packed, aligned(4))) VFE_CAMIFConfigCmdType;

/* ===  CAMIF Config Command === */
typedef enum VFE_CAMIF_SYNC_EDGE {
  /* 0x0 = Active high. */
  VFE_CAMIF_SYNC_EDGE_ActiveHigh,
  /* 0x1 = Active low.  */
  VFE_CAMIF_SYNC_EDGE_ActiveLow
} VFE_CAMIF_SYNC_EDGE;

typedef enum VFE_CAMIF_SYNC_MODE {
  /* 0x00 = APS (active physical synchronization) */
  VFE_CAMIF_SYNC_MODE_APS,
  /* 0x01 = EFS (embedded frame synchronization) */
  VFE_CAMIF_SYNC_MODE_EFS,
  /* 0x10 = ELS (embedded line synchronization)  */
  VFE_CAMIF_SYNC_MODE_ELS,
  /* 0x11 = Illegal                              */
  VFE_CAMIF_SYNC_MODE_ILLEGAL
} VFE_CAMIF_SYNC_MODE;

typedef enum CAMIF_SUBSAMPLE_FRAME_SKIP {
  CAMIF_SUBSAMPLE_FRAME_SKIP_0,
  CAMIF_SUBSAMPLE_FRAME_SKIP_AllFrames,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_2Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_3Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_4Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_5Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_6Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_7Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_8Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_9Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_10Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_11Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_12Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_13Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_14Frame,
  CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_15Frame
} CAMIF_SUBSAMPLE_FRAME_SKIP;

typedef enum VFE_CAMIF_To_VFE_SubsampleEnableType {
  VFE_DISABLE_CAMIF_TO_VFE_SUBSAMPLE,
  VFE_ENABLE_CAMIF_TO_VFE_SUBSAMPLE,
  VFE_LAST_CAMIF_TO_VFE_SUBSAMPLE_ENABLE_ENUM =
    VFE_ENABLE_CAMIF_TO_VFE_SUBSAMPLE, /* Used for count purposes only */
} VFE_CAMIF_To_VFE_SubsampleEnableType;

typedef enum VFE_BusSubsampleEnableType {
  VFE_DISABLE_BUS_SUBSAMPLE,
  VFE_ENABLE_BUS_SUBSAMPLE,
  VFE_LAST_BUS_SUBSAMPLE_ENABLE_ENUM =
    VFE_ENABLE_BUS_SUBSAMPLE,  /* Used for count purposes only */
} VFE_BusSubsampleEnableType;

typedef enum VFE_IRQ_SubsampleEnableType {
  VFE_DISABLE_IRQ_SUBSAMPLE,
  VFE_ENABLE_IRQ_SUBSAMPLE,
  VFE_LAST_IRQ_SUBSAMPLE_ENABLE_ENUM =
    VFE_ENABLE_IRQ_SUBSAMPLE,  /* Used for count purposes only */
} VFE_IRQ_SubsampleEnableType;

typedef enum VFE_CAMIFPixelSkipWrapType {
  VFE_USE_ALL_16_BITS_OF_PIXEL_SKIP_PATTERN,
  VFE_USE_12_MSB_BITS_OF_PIXEL_SKIP_PATTERN,
  VFE_LAST_PIXEL_SKIP_WRAP_ENUM = VFE_USE_12_MSB_BITS_OF_PIXEL_SKIP_PATTERN
} VFE_CAMIFPixelSkipWrapType;

typedef enum VFE_Epoch1InterruptEnableType {
  VFE_DISABLE_EPOCH_1_INTERRUPT,
  VFE_ENABLE_EPOCH_1_INTERRUPT,
  VFE_LAST_EPOCH_1_INTERRUPT_ENABLE_ENUM = VFE_ENABLE_EPOCH_1_INTERRUPT,  /* Used for count purposes only */
} VFE_Epoch1InterruptEnableType;

typedef enum VFE_Epoch2InterruptEnableType {
  VFE_DISABLE_EPOCH_2_INTERRUPT,
  VFE_ENABLE_EPOCH_2_INTERRUPT,
  VFE_LAST_EPOCH_2_INTERRUPT_ENABLE_ENUM = VFE_ENABLE_EPOCH_2_INTERRUPT,  /* Used for count purposes only */
} VFE_Epoch2InterruptEnableType;

typedef struct {
  uint32_t operation;  /* 1 Start 0 Stop*/
  uint32_t repeatCount;
  uint32_t whichSyncTimer; /* 0 = Timer0, 1 = Timer1 2 = Timer2 */
  uint32_t hsyncCount;
  uint32_t pclkCount;
  uint32_t outputDuration; /* in micro sec */
  uint32_t polarity; /* 0 = Active High, 1 = Active Low*/
}VFE_SyncTimerCmdType;

typedef struct {
  pixel_crop_info_t sensor_crop_info;
  camera_size_t sensor_dim;
  camera_op_mode_t mode;
  camera_size_t camif_window;
  sensor_output_format_t format;
  camif_strobe_info_t strobe_info;
  vfe_axi_output_dim_t dimension;
  uint32_t connection_mode;
  int fd;
} camif_params_t;

typedef struct {
  int ref_count;
  VFE_CAMIFConfigCmdType camif_cmd;
  VFE_SyncTimerCmdType sync_timer_cmd;
  camif_params_t params;
}camif_obj_t;

#define AXI_MAX_CLIENT_NUM    1
#define CAMIF_MAX_OBJS 1
#define CAMIF_MAX_CLIENT_NUM 1

typedef struct {
  uint32_t obj_idx_mask;
  uint8_t client_idx;
  uint32_t handle;
  uint8_t my_comp_id;
  uint32_t vfe_version;
  mctl_ops_t *ops;
} camif_client_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t camif_handle_cnt;
  camif_client_t client[CAMIF_MAX_CLIENT_NUM];
  camif_obj_t camif_obj[CAMIF_MAX_OBJS];
} camif_comp_root_t;

int camif_init(camif_obj_t *p_camif);
int camif_config(camif_obj_t *p_camif);
int camif_timer_config(camif_obj_t *p_camif);
int camif_command_ops(camif_obj_t *p_camif, void *data);
#endif //__CAMIF_H__

