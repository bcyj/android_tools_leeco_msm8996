/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __AXI_H__
#define __AXI_H__

#include "camera.h"
#include "intf_comm_data.h"
#include "axi_interface.h"

/* burst length = 8 for all line and frame write masters.*/
#define TOTAL_SIZE_UB_BUFFER_FOR_IMAGE_MASTERS  920

typedef enum {
  VFE_8_BITS_PER_COLOR_COMP_8_BIT_BAYER,
  VFE_8_BITS_PER_COLOR_COMP_8_BIT_YCBCR422
    = VFE_8_BITS_PER_COLOR_COMP_8_BIT_BAYER,
  VFE_10_BITS_PER_COLOR_COMP_10_BIT_BAYER,
  VFE_12_BITS_PER_COLOR_COMP_12_BIT_BAYER,
  VFE_LAST_AXI_FORMAT_ENUM = VFE_12_BITS_PER_COLOR_COMP_12_BIT_BAYER
} vfe_axifmt_t;

typedef enum {
  VFE_BURST_LENGTH_2,
  VFE_BURST_LENGTH_4,
  VFE_BURST_LENGTH_8,
  VFE_BURST_LENGTH_16,
  VFE_LAST_AXI_BURST_LENGTH_ENUM = VFE_BURST_LENGTH_16    /* For count purposes */
} vfe_axiburst_len_t;

typedef enum {
  VFE_ONLY_OUTPUT_1_ENABLED,      /* Enable output1 only */
  VFE_ONLY_OUTPUT_2_ENABLED,      /* Enable output2 only */
  VFE_BOTH_OUTPUT_1_AND_2_ENABLED,        /* Enable output1 & output2 */
  /* Enable output from CAMIF to AXI only via output2 configuration */
  VFE_CAMIF_TO_AXI_VIA_OUTPUT_2_ENABLED,
  /* Enable output from CAMIF to AXI (via output2) and output1 */
  VFE_OUTPUT_1_AND_CAMIF_TO_AXI_VIA_OUTPUT_2_ENABLED,
  /* Enable output from CAMIF to AXI (via output1) and output2 */
  VFE_OUTPUT_2_AND_CAMIF_TO_AXI_VIA_OUTPUT_1_ENABLED,
  VFE_LAST_AXI_OUTPUT_MODE_ENUM =
    VFE_OUTPUT_2_AND_CAMIF_TO_AXI_VIA_OUTPUT_1_ENABLED      /* For count only */
} vfe_axiout_mode_t;

typedef struct VFE_AXIOutputConfigCmdType{
  /* AXI Output Selection */
  vfe_axiout_mode_t outputMode:3;
  vfe_axifmt_t format:2;
  uint32_t /* reserved */ :27;

  /* AXI Output 1 Y Configuration, Part 1 */
  uint32_t out1YImageHeight:12;
  uint32_t /* reserved */ :4;
  uint32_t out1YImageWidthIn64BitWords:10;
  uint32_t /* reserved */ :6;

  /* AXI Output 1 Y Configuration, Part 2 */
  vfe_axiburst_len_t out1YBurstLen:2;
  uint32_t out1YNumRows:12;
  uint32_t out1YRowIncIn64bitIncs:12;
  uint32_t /* reserved */ :6;

  /* AXI Output 1 CbCr Configuration, Part 1 */
  uint32_t out1CbCrImageHeight:12;
  unsigned int /* reserved */ :4;
  uint32_t out1CbCrImageWidthIn64BitWords:10;
  unsigned int /* reserved */ :6;

  /* AXI Output 1 CbCr Configuration, Part 2 */
  vfe_axiburst_len_t out1CbCrBurstLen:2;
  uint32_t out1CbCrNumRows:12;
  uint32_t out1CbCrRowIncIn64bitIncs:12;
  uint32_t /* reserved */ :6;

  /* AXI Output 2 Y Configuration, Part 1 */
  uint32_t out2YImageHeight:12;
  uint32_t /* reserved */ :4;
  uint32_t out2YImageWidthIn64BitWords:10;
  unsigned int /* reserved */ :6;

  /* AXI Output 2 Y Configuration, Part 2 */
  vfe_axiburst_len_t out2YBurstLen:2;
  uint32_t out2YNumRows:12;
  uint32_t out2YRowIncIn64bitIncs:12;
  unsigned int /* reserved */ :6;

  /* AXI Output 2 CbCr Configuration, Part 1 */
  uint32_t out2CbCrImageHeight:12;
  unsigned int /* reserved */ :4;
  uint32_t out2CbCrImageWidthIn64BitWords:10;
  unsigned int /* reserved */ :6;

  /* AXI Output 2 CbCr Configuration, Part 2 */
  vfe_axiburst_len_t out2CbCrBurstLen:2;
  uint32_t out2CbCrNumRows:12;
  uint32_t out2CbCrRowIncIn64bitIncs:12;
  unsigned int /* reserved */ :6;

  uint8_t *output1buffer1YAddress;
  uint8_t *output1buffer1CbCrAddress;
  uint8_t *output1buffer2YAddress;
  uint8_t *output1buffer2CbCrAddress;
  uint8_t *output1buffer3YAddress;
  uint8_t *output1buffer3CbCrAddress;
  uint8_t *output1buffer4YAddress;
  uint8_t *output1buffer4CbCrAddress;
  uint8_t *output1buffer5YAddress;
  uint8_t *output1buffer5CbCrAddress;
  uint8_t *output1buffer6YAddress;
  uint8_t *output1buffer6CbCrAddress;
  uint8_t *output1buffer7YAddress;
  uint8_t *output1buffer7CbCrAddress;
  uint8_t *output1buffer8YAddress;
  uint8_t *output1buffer8CbCrAddress;
  uint8_t *output2buffer1YAddress;
  uint8_t *output2buffer1CbCrAddress;
  uint8_t *output2buffer2YAddress;
  uint8_t *output2buffer2CbCrAddress;
  uint8_t *output2buffer3YAddress;
  uint8_t *output2buffer3CbCrAddress;
  uint8_t *output2buffer4YAddress;
  uint8_t *output2buffer4CbCrAddress;
  uint8_t *output2buffer5YAddress;
  uint8_t *output2buffer5CbCrAddress;
  uint8_t *output2buffer6YAddress;
  uint8_t *output2buffer6CbCrAddress;
  uint8_t *output2buffer7YAddress;
  uint8_t *output2buffer7CbCrAddress;
  uint8_t *output2buffer8YAddress;
  uint8_t *output2buffer8CbCrAddress;
} __attribute__ ((packed, aligned(4))) VFE_AXIOutputConfigCmdType;


typedef struct {
  current_output_info_t output_info;
  vfe_axi_output_dim_t dimension;
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
  axi_pix_intf_ctrl_t pix;
} axi_ctrl_t;

typedef struct {
  int ref_count;
  uint8_t reset_done;
  axi_ctrl_t axi_ctrl[AXI_INTF_MAX_NUM];
} axi_obj_t;

typedef struct {
 uint32_t handle;
 axi_ctrl_t axi_ctrl;
} axi_intf_t;

typedef struct {
  uint32_t obj_idx_mask;
  uint8_t client_idx;
  uint32_t handle;
  uint8_t my_comp_id;
  uint32_t vfe_version;
  mctl_ops_t *ops;
  int sdev_fd;
} axi_client_t;

#define AXI_MAX_CLIENT_NUM 1
#define AXI_MAX_OBJS 1

typedef struct {
  pthread_mutex_t mutex;
  uint32_t axi_handle_cnt;
  axi_client_t client[AXI_MAX_CLIENT_NUM];
  axi_obj_t axi_obj[AXI_MAX_OBJS];
} axi_comp_root_t;


int axi_config(axi_comp_root_t *axi_root, axi_client_t *axi_client, axi_config_t *cfg);
int axi_set_params(axi_comp_root_t *axi_root, axi_client_t *axi_client,
				   int type, axi_set_t *axi_set_parm);
int axi_command_ops(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  void *parm);
int axi_proc_reg_update(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  void *parm);
int axi_proc_unregister_wms(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  void *parm);

//#define ENABLE_AXI_LOGGING
#endif
