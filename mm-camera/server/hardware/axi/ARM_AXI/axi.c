/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>
#include <media/msm_isp.h>
#include "camera_dbg.h"
#include "axi.h"

#ifdef ENABLE_AXI_LOGGING
#undef CDBG
#define CDBG LOGE
#endif

static axi_obj_t *axi_get_obj(axi_comp_root_t *axi_root, axi_client_t *axi_client)
{
  axi_obj_t *axi_obj = NULL;
  /* now we have not implemented the use case of using 2 AXI obj */
  if(axi_client->obj_idx_mask == 1)
    axi_obj= &axi_root->axi_obj[0];
  else if(axi_client->obj_idx_mask == 2)
    axi_obj= &axi_root->axi_obj[1];
  return axi_obj;
}

/*===========================================================================
 * FUNCTION    - find_write_master -
 *
 * DESCRIPTION: Return available write master in round robbin
 *==========================================================================*/
static int find_write_master(axi_obj_t *axi_obj)
{
  int i = 0, index = -1, free_wm = -1;

  for (i = 0; i < AXI_MAX_WM_NUM; i++) {
    if (axi_obj->axi_wm_table[i].is_reserved == 0) {
      if (free_wm == -1)
        free_wm = i;
      if (axi_obj->axi_wm_table[i].use_count == 0) {
        index = i;
        break;
      }
    }
  }
  if (free_wm == -1)
    return -1;

  if (index == -1) {
    for (i = 0; i < AXI_MAX_WM_NUM; i++) {
      if (axi_obj->axi_wm_table[i].is_reserved == 0)
        axi_obj->axi_wm_table[i].use_count = 0;
    }
  } else
    free_wm = index;
  axi_obj->axi_wm_table[free_wm].is_reserved = 1;
  axi_obj->axi_wm_table[free_wm].use_count = 1;
  return free_wm;

}

/*===========================================================================
 * FUNCTION    - release_write_master -
 *
 * DESCRIPTION: Return first available write master
 *==========================================================================*/
static void release_write_master(axi_obj_t *axi_obj, int wm_id)
{
  axi_obj->axi_wm_table[wm_id].is_reserved = 0;
  return;
}

/*===========================================================================
 * FUNCTION    - axi_config_wm_table_entry -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void axi_config_wm_table_entry(axi_wm_table_entry_t *wm_table, int index,
  int interface, axi_wm_state_t state, vfe_ports_used_t port)
{
  assert(wm_table);
  assert(index >= 0);
  assert(index < AXI_MAX_WM_NUM);

  wm_table[index].interface = interface;
  wm_table[index].state = state;
  wm_table[index].port = port;
}

/*===========================================================================
 * FUNCTION    - axi_cmd_print -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void axi_write_master_cmd_print(vfe_wm_config *wm)
{
  CDBG("AXI Output busUbOffset = %d\n", wm->busUbOffset);
  CDBG("AXI Output busUbDepth = %d\n", wm->busUbDepth);
  CDBG("AXI Output buslinesPerImage = %d\n", wm->buslinesPerImage);
  CDBG("AXI Output busdwordsPerLine = %d\n", wm->busdwordsPerLine);
  CDBG("AXI Output busburstLength = %d\n", wm->busburstLength);
  CDBG("AXI Output busbufferNumRows = %d\n", wm->busbufferNumRows);
  CDBG("AXI Output busrowIncrement = %d\n", wm->busrowIncrement);
} /* axi_cmd_print */

/*===========================================================================
 * FUNCTION    - axi_cfg_cmd_print -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void axi_cfg_cmd_print(VFE_AXIOutputConfigCmdType *cmd)
{
  CDBG("AXI Output busCmd = %d\n", cmd->busCmd);
  CDBG("AXI Output busCfg = %d\n", cmd->busCfg);
  CDBG("AXI Output xbarCfg0 = %d\n", cmd->xbarCfg0);
  CDBG("AXI Output xbarCfg1 = %d\n", cmd->xbarCfg1);
  CDBG("AXI Output busWrSkipCfg = %d\n", cmd->busWrSkipCfg);
} /* axi_cfg_cmd_print */

/*===========================================================================
 * FUNCTION    - axi_get_data_ub_size -
 *
 * DESCRIPTION:
 *==========================================================================*/
uint32_t axi_get_data_ub_size(axi_client_t *client)
{
  if(client->stats_version == 4)
    return TOTAL_SIZE_UB_BUFFER_FOR_IMAGE_MASTERS_BAYER;
  else
    return TOTAL_SIZE_UB_BUFFER_FOR_IMAGE_MASTERS_DEMOSAIC;
}

/*===========================================================================
 * FUNCTION    - axi_fill_sizes -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void axi_calculate_sizes(axi_client_t *axi_client,
  current_output_info_t *output_info, uint8_t minUbSize,
  uint32_t *totalSize, int64_t *totalUbSize)
{
  uint32_t prim_size = 0, sec_size = 0;
  uint8_t writeMastersInUse = 0;

  assert(totalSize);
  assert(totalUbSize);
  assert(output_info);

  *totalSize = 0;
  *totalUbSize = 0;

  writeMastersInUse += 2;
  if (output_info->output[PRIMARY].format == CAMERA_YUV_422_NV61 ||
      output_info->output[PRIMARY].format == CAMERA_YUV_422_NV16)
    prim_size = ((output_info->output[PRIMARY].image_width +
                  output_info->output[PRIMARY].extra_pad_width) *
                 (output_info->output[PRIMARY].image_height +
                  output_info->output[PRIMARY].extra_pad_height))
                * 2;
  else
    prim_size = ((output_info->output[PRIMARY].image_width +
                  output_info->output[PRIMARY].extra_pad_width) *
                 (output_info->output[PRIMARY].image_height +
                  output_info->output[PRIMARY].extra_pad_height))
                * 1.5;
  if (output_info->output[PRIMARY].format == CAMERA_YUV_420_YV12)
	writeMastersInUse++;

  if (output_info->vfe_ports_used & VFE_OUTPUT_SECONDARY) {
    writeMastersInUse += 2;
    if (output_info->output[SECONDARY].format == CAMERA_YUV_422_NV61 ||
	output_info->output[SECONDARY].format == CAMERA_YUV_422_NV16)
      sec_size = ((output_info->output[SECONDARY].image_width +
                   output_info->output[SECONDARY].extra_pad_width) *
                  (output_info->output[SECONDARY].image_height +
                   output_info->output[SECONDARY].extra_pad_height))
                  * 2;
    else
      sec_size = ((output_info->output[SECONDARY].image_width +
                   output_info->output[SECONDARY].extra_pad_width) *
                  (output_info->output[SECONDARY].image_height +
                   output_info->output[SECONDARY].extra_pad_height))
                  * 1.5;

    if (output_info->output[SECONDARY].format == CAMERA_YUV_420_YV12)
      writeMastersInUse++;
  }

  *totalSize = prim_size + sec_size;
  *totalUbSize = axi_get_data_ub_size(axi_client);
  *totalUbSize -= writeMastersInUse * minUbSize;
  CDBG("%s Total UB size available for image masters %d", __func__,
    axi_get_data_ub_size(axi_client));
  CDBG("%s Primary size %d, Secondary size %d, Num WMs = %d", __func__,
    prim_size, sec_size, writeMastersInUse);
  CDBG("%s Filled sizes Total size: %d, Total UB Size = %lld ", __func__,
    *totalSize, *totalUbSize);
}

static void axi_update_UB_data(axi_obj_t *axi_obj,
  axi_client_t *axi_client, current_output_info_t *output_info)
{
  int last_prim_wm = -1, i;
  uint8_t minUbSize = 64;
  int64_t totalUbSize = 0;
  uint32_t totalSize = 0;
  uint32_t imageWidth = 0, imageHeight = 0;

  assert(output_info);

  axi_calculate_sizes(axi_client, output_info,
    minUbSize, &totalSize, &totalUbSize);

  /* Reset all WM data */
  for (i = 0; i < AXI_MAX_WM_NUM; i++) {
    axi_obj->axi_wm_table[i].ub_info.ub_offset = 0;
    axi_obj->axi_wm_table[i].ub_info.ub_size = 0;
  }

  imageWidth = output_info->output[PRIMARY].image_width +
               output_info->output[PRIMARY].extra_pad_width;
  imageHeight = output_info->output[PRIMARY].image_height +
                output_info->output[PRIMARY].extra_pad_height;

  axi_obj->axi_wm_table[0].ub_info.ub_offset = 0;
  axi_obj->axi_wm_table[0].ub_info.ub_size = minUbSize + MAX(1,
    (int32_t)((imageHeight * imageWidth * totalUbSize)/totalSize - 1));

  axi_obj->axi_wm_table[1].ub_info.ub_offset =
    axi_obj->axi_wm_table[0].ub_info.ub_offset +
    axi_obj->axi_wm_table[0].ub_info.ub_size + 1;

  /* Update the last primary write master in each case. This will be
   * used to configure the UB offset of the first secondary write master
   * if secondary is configured */
  if (output_info->output[PRIMARY].format == CAMERA_YUV_420_YV12) {
    /* Two chroma planes size: W * H * 1/4 each */
    axi_obj->axi_wm_table[1].ub_info.ub_size = minUbSize + MAX(1,
      (int32_t)((imageHeight/2 * imageWidth/2 * totalUbSize)/totalSize - 1));

    axi_obj->axi_wm_table[2].ub_info.ub_offset =
      axi_obj->axi_wm_table[1].ub_info.ub_offset + 1
      + axi_obj->axi_wm_table[1].ub_info.ub_size;
    axi_obj->axi_wm_table[2].ub_info.ub_size = minUbSize + MAX(1,
      (int32_t)((imageHeight/2 * imageWidth/2 * totalUbSize)/totalSize - 1));
    last_prim_wm = 2;
  } else if (output_info->output[PRIMARY].format == CAMERA_YUV_422_NV61 ||
             output_info->output[PRIMARY].format == CAMERA_YUV_422_NV16) {
    /* Single chroma plane size: W * H * 2 */
    axi_obj->axi_wm_table[1].ub_info.ub_size = minUbSize + MAX(1,
      (int32_t)((imageHeight * imageWidth * totalUbSize)/totalSize - 1));
    last_prim_wm = 1;
  } else {
    /* Single chroma plane size: W * H * 1.5 */
    axi_obj->axi_wm_table[1].ub_info.ub_size = minUbSize + MAX(1,
      (int32_t)((imageHeight * imageWidth/2 * totalUbSize)/totalSize - 1));
    last_prim_wm = 1;
  }

  if (output_info->vfe_ports_used & VFE_OUTPUT_SECONDARY) {
    imageWidth = output_info->output[SECONDARY].image_width +
                 output_info->output[SECONDARY].extra_pad_width;
    imageHeight = output_info->output[SECONDARY].image_height +
                  output_info->output[SECONDARY].extra_pad_height;

    axi_obj->axi_wm_table[4].ub_info.ub_offset =
      axi_obj->axi_wm_table[last_prim_wm].ub_info.ub_offset + 1
      + axi_obj->axi_wm_table[last_prim_wm].ub_info.ub_size;

    axi_obj->axi_wm_table[4].ub_info.ub_size = minUbSize + MAX(1,
      (int32_t)((imageHeight * imageWidth * totalUbSize)/totalSize - 1));

    axi_obj->axi_wm_table[5].ub_info.ub_offset =
      axi_obj->axi_wm_table[4].ub_info.ub_offset + 1
      + axi_obj->axi_wm_table[4].ub_info.ub_size;

    if (output_info->output[SECONDARY].format == CAMERA_YUV_420_YV12) {
      assert(sec_wm_chroma >= 0);
      /* Two chroma planes size: W * H * 1/4 each*/
      axi_obj->axi_wm_table[5].ub_info.ub_size = minUbSize + MAX(1,
	  (int32_t)((imageHeight/2 * imageWidth/2 * totalUbSize)/totalSize - 1));

      axi_obj->axi_wm_table[6].ub_info.ub_offset =
	axi_obj->axi_wm_table[5].ub_info.ub_offset + 1
	+ axi_obj->axi_wm_table[5].ub_info.ub_size;
      axi_obj->axi_wm_table[6].ub_info.ub_size = minUbSize + MAX(1,
	  (int32_t)((imageHeight/2 * imageWidth/2 * totalUbSize)/totalSize - 1));
    } else if (output_info->output[SECONDARY].format == CAMERA_YUV_422_NV61 ||
               output_info->output[SECONDARY].format == CAMERA_YUV_422_NV16) {
      /* Single chroma plane size: W * H * 2 */
      axi_obj->axi_wm_table[5].ub_info.ub_size = minUbSize + MAX(1,
	  (int32_t)((imageHeight * imageWidth * totalUbSize)/totalSize - 1));
    } else {
      /* Single chroma plane size: W * H * 1.5 */
      axi_obj->axi_wm_table[5].ub_info.ub_size = minUbSize + MAX(1,
	  (int32_t)((imageHeight * imageWidth/2 * totalUbSize)/totalSize - 1));
    }
  }

  CDBG_HIGH("%s Operation mode = %d ", __func__,
    output_info->vfe_operation_mode);
  for (i = 0; i < AXI_MAX_WM_NUM; i++)
    CDBG_HIGH("%s Image master %d UB Offset %d UB Size %d\n", __func__, i,
      axi_obj->axi_wm_table[i].ub_info.ub_offset,
      axi_obj->axi_wm_table[i].ub_info.ub_size);
}

static void configure_xbar(VFE_AXIOutputConfigCmdType *cmd, int16_t ch_wm,
                           uint16_t config)
{

#ifdef VFE_40
  if (ch_wm >= 0 && ch_wm <= 1) {
    cmd->xbarCfg0 = (cmd->xbarCfg0) &
                    ~(0x0000FFFF << (16 * (ch_wm % 2)));
    cmd->xbarCfg0 = (cmd->xbarCfg0) |
                    (config << (16 * (ch_wm % 2)));
  } else if (ch_wm >= 2 && ch_wm <= 3) {
    cmd->xbarCfg1 = (cmd->xbarCfg1) &
                    ~(0x0000FFFF << (16 * (ch_wm % 2)));
    cmd->xbarCfg1 = (cmd->xbarCfg1) |
                    (config << (16 * (ch_wm % 2)));
  } else if (ch_wm >= 4 && ch_wm <= 5) {
    cmd->xbarCfg2 = (cmd->xbarCfg2) &
                    ~(0x0000FFFF << (16 * (ch_wm % 2)));
    cmd->xbarCfg2 = (cmd->xbarCfg2) |
                    (config << (16 * (ch_wm % 2)));
  } else {
    cmd->xbarCfg3 = (cmd->xbarCfg3) &
                    ~(0x0000FFFF << (16 * (ch_wm % 2)));
    cmd->xbarCfg3 = (cmd->xbarCfg3) |
                    (config << (16 * (ch_wm % 2)));
  }
#else
  if (ch_wm >= 0 && ch_wm <= 3) {
    cmd->xbarCfg0 = (cmd->xbarCfg0) &
                    ~(0x000000FF << (8 * (ch_wm % 4)));
    cmd->xbarCfg0 = (cmd->xbarCfg0) |
                    (config << (8 * (ch_wm % 4)));
  } else {
    cmd->xbarCfg1 = (cmd->xbarCfg1) &
                    ~(0x000000FF << (8 * (ch_wm % 4)));
    cmd->xbarCfg1 = (cmd->xbarCfg1) |
                    (config << (8 * (ch_wm % 4)));
  }
#endif

  return;
}

static void configure_wm_registers(VFE_AXIOutputConfigCmdType *cmd,
                                   axi_obj_t *axi_obj, int imageWidth,
                                   int imageHeight, plane_info_t *plane_info,
                                   int16_t ch0_wm, int16_t ch1_wm,
                                   int16_t ch2_wm, int axi_ctrl_index,
                                   int port_index, int port_used,
                                   int32_t axiOutputPpw,
                                   uint32_t burstLength, int num_WMs)
{
  cmd->wm[ch0_wm].busdwordsPerLine = (imageWidth+(axiOutputPpw*2-1))/(axiOutputPpw*2)-1;
#ifndef VFE_40
  cmd->wm[ch0_wm].busrowIncrement = (imageWidth+(axiOutputPpw-1))/(axiOutputPpw);
#else
  cmd->wm[ch0_wm].busrowIncrement = (plane_info[0].stride+(axiOutputPpw-1))/(axiOutputPpw);
#endif
  cmd->wm[ch0_wm].buslinesPerImage = imageHeight - 1;
  cmd->wm[ch0_wm].busbufferNumRows = imageHeight - 1;
  cmd->wm[ch0_wm].busburstLength = burstLength;
  cmd->wm[ch0_wm].busUbOffset =
    axi_obj->axi_wm_table[ch0_wm].ub_info.ub_offset;
  cmd->wm[ch0_wm].busUbDepth =
    axi_obj->axi_wm_table[ch0_wm].ub_info.ub_size;

#ifdef VFE_40
  cmd->wm[ch0_wm].mal_en = 0;
  cmd->wm[ch0_wm].ocmem_en = 0;
  cmd->wm[ch0_wm].framedrop_period = 0;
  cmd->wm[ch0_wm].irq_sub_period = 0;
  cmd->wm[ch0_wm].frame_increment = 0;
  cmd->wm[ch0_wm].framedrop_pattern = 0xFFFFFFFF;
  cmd->wm[ch0_wm].irq_sub_pattern = 0xFFFFFFFF;
#endif
  axi_config_wm_table_entry(axi_obj->axi_wm_table, ch0_wm,
    axi_ctrl_index, AXI_WM_IDLE, port_used);
  CDBG("%s: axi channel write master %d config\n", __func__, ch0_wm);
    axi_write_master_cmd_print(&(cmd->wm[ch0_wm]));

  if (num_WMs <= 1) {
    CDBG("%s: RDI WM is being configured\n", __func__);
    return;
  }


  if (axi_obj->axi_ctrl[axi_ctrl_index].pix.output_info.output[port_index].format
      == CAMERA_YUV_420_YV12) {
    /* Chroma Channel 1*/
    cmd->wm[ch1_wm].busdwordsPerLine = (imageWidth/2 + (axiOutputPpw*2 - 1))/
                              (axiOutputPpw*2) - 1;
    cmd->wm[ch1_wm].busrowIncrement = ((imageWidth/2 + (axiOutputPpw *2- 1))/
                              (axiOutputPpw * 2))*2;
    cmd->wm[ch1_wm].buslinesPerImage = imageHeight/2 - 1;
    cmd->wm[ch1_wm].busbufferNumRows = imageHeight/2 - 1;
    cmd->wm[ch1_wm].busburstLength = burstLength;
    cmd->wm[ch1_wm].busUbOffset =
      axi_obj->axi_wm_table[ch1_wm].ub_info.ub_offset;
    cmd->wm[ch1_wm].busUbDepth =
      axi_obj->axi_wm_table[ch1_wm].ub_info.ub_size;
#ifdef VFE_40
    cmd->wm[ch1_wm].mal_en = 0;
    cmd->wm[ch1_wm].ocmem_en = 0;
    cmd->wm[ch1_wm].framedrop_period = 0;
    cmd->wm[ch1_wm].irq_sub_period = 0;
    cmd->wm[ch1_wm].frame_increment = 0;
    cmd->wm[ch1_wm].framedrop_pattern = 0xFFFFFFFF;
    cmd->wm[ch1_wm].irq_sub_pattern = 0xFFFFFFFF;
#endif

    /* Chroma Channel 2*/
    cmd->wm[ch2_wm].busdwordsPerLine = (imageWidth/2 + (axiOutputPpw*2 - 1))/
                                (axiOutputPpw*2) - 1;
    cmd->wm[ch2_wm].busrowIncrement = ((imageWidth/2 + (axiOutputPpw *2- 1))/
                               (axiOutputPpw*2))*2;
    cmd->wm[ch2_wm].buslinesPerImage = imageHeight/2 - 1;
    cmd->wm[ch2_wm].busbufferNumRows = imageHeight/2 - 1;
    cmd->wm[ch2_wm].busburstLength = burstLength;
    cmd->wm[ch2_wm].busUbOffset =
      axi_obj->axi_wm_table[ch2_wm].ub_info.ub_offset;
    cmd->wm[ch2_wm].busUbDepth =
      axi_obj->axi_wm_table[ch2_wm].ub_info.ub_size;
#ifdef VFE_40
    cmd->wm[ch1_wm].mal_en = 0;
    cmd->wm[ch1_wm].ocmem_en = 0;
    cmd->wm[ch1_wm].framedrop_period = 0;
    cmd->wm[ch1_wm].irq_sub_period = 0;
    cmd->wm[ch1_wm].frame_increment = 0;
    cmd->wm[ch1_wm].framedrop_pattern = 0xFFFFFFFF;
    cmd->wm[ch1_wm].irq_sub_pattern = 0xFFFFFFFF;
#endif
    axi_config_wm_table_entry(axi_obj->axi_wm_table, ch1_wm,
      axi_ctrl_index, AXI_WM_IDLE, port_used);
    axi_config_wm_table_entry(axi_obj->axi_wm_table, ch2_wm,
      axi_ctrl_index, AXI_WM_IDLE, port_used);

    CDBG("%s: axi channel write master %d config\n", __func__, ch1_wm);
    axi_write_master_cmd_print(&(cmd->wm[ch1_wm]));
    CDBG("%s: axi channel write master %d config\n", __func__, ch2_wm);
    axi_write_master_cmd_print(&(cmd->wm[ch2_wm]));
  } else {
    /* Chroma Channel*/
    cmd->wm[ch1_wm].busdwordsPerLine = (imageWidth+(axiOutputPpw*2-1))/(axiOutputPpw*2)-1;
#ifndef VFE_40
    cmd->wm[ch1_wm].busrowIncrement = (imageWidth+(axiOutputPpw-1))/(axiOutputPpw);
#else
    cmd->wm[ch1_wm].busrowIncrement = (plane_info[1].stride+(axiOutputPpw-1))/(axiOutputPpw);
#endif
    if(axi_obj->axi_ctrl[axi_ctrl_index].pix.output_info.output[port_index].format
      == CAMERA_YUV_422_NV61 ||
      axi_obj->axi_ctrl[axi_ctrl_index].pix.output_info.output[port_index].format
      == CAMERA_YUV_422_NV16) {
      cmd->wm[ch1_wm].buslinesPerImage = imageHeight - 1;
      cmd->wm[ch1_wm].busbufferNumRows = imageHeight - 1;
    } else {
      cmd->wm[ch1_wm].buslinesPerImage = imageHeight/2 - 1;
      cmd->wm[ch1_wm].busbufferNumRows = imageHeight/2 - 1;
    }
    cmd->wm[ch1_wm].busburstLength   = burstLength;
    cmd->wm[ch1_wm].busUbOffset =
      axi_obj->axi_wm_table[ch1_wm].ub_info.ub_offset;
    cmd->wm[ch1_wm].busUbDepth =
      axi_obj->axi_wm_table[ch1_wm].ub_info.ub_size;
#ifdef VFE_40
    cmd->wm[ch1_wm].mal_en = 0;
    cmd->wm[ch1_wm].ocmem_en = 0;
    cmd->wm[ch1_wm].framedrop_period = 0;
    cmd->wm[ch1_wm].irq_sub_period = 0;
    cmd->wm[ch1_wm].frame_increment = 0;
    cmd->wm[ch1_wm].framedrop_pattern = 0xFFFFFFFF;
    cmd->wm[ch1_wm].irq_sub_pattern = 0xFFFFFFFF;
#endif
    axi_config_wm_table_entry(axi_obj->axi_wm_table, ch1_wm,
      axi_ctrl_index, AXI_WM_IDLE, port_used);

    CDBG("%s: axi channel write master %d config\n", __func__, ch1_wm);
    axi_write_master_cmd_print(&(cmd->wm[ch1_wm]));
  }
  return;
}

/*===========================================================================
 * FUNCTION    - axi_raw_snapshot_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int axi_raw_snapshot_config(axi_comp_root_t *axi_root, axi_client_t *axi_client,
                                            VFE_AXIOutputConfigCmdType *cmd)
{
  uint32_t axiOutputPpw = 8;
  uint32_t imageWidth = 0, imageHeight = 0;
  plane_info_t *plane_info = NULL;
  uint8_t burstLength = 2;
  int status = 0;
  axi_obj_t *axi_obj = NULL;
  int wm_config;

  CDBG("%s: begin", __func__);
  if(axi_client->obj_idx_mask)
    axi_obj = axi_get_obj(axi_root, axi_client);
  if(!axi_obj) {
    CDBG_ERROR("%s: no axi obj associated with teh client",  __func__);
    return -1;
  }

  cmd->busCmd = 0x0; /*this has no effect*/
  cmd->busWrSkipCfg =  0;

  if (axi_client->current_target == TARGET_MSM8930)
    burstLength = 0x1; // Use burst length of 4 for 8930.

  switch (axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.sensor_data.sensor_raw_depth) {
    case SENSOR_8_BIT_DIRECT:
      axiOutputPpw = 8;
      cmd->busCfg = 0x2aaa771;
#ifndef VFE_31
      cmd->busioFormat = 0x0;
#endif
      break;
    case SENSOR_10_BIT_DIRECT:
      axiOutputPpw = 6;
      cmd->busCfg = 0x2aaa775;
#ifndef VFE_31
      cmd->busioFormat = 0x1000;
#endif
      break;
    case SENSOR_12_BIT_DIRECT:
      axiOutputPpw = 5;
      cmd->busCfg = 0x2aaa779;
#ifndef VFE_31
      cmd->busioFormat = 0x2000;
#endif
      break;
    default:
      CDBG_ERROR("%s raw depth %d not supported yet.\n", __func__,
        axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.sensor_data.sensor_raw_depth);
      status = -EINVAL;
      break;
  }
  if(status < 0)
    return status;

  if (axi_client->current_target != TARGET_MSM8930) {
    cmd->outpath.out0.ch0 = find_write_master(axi_obj);
  } else {
    /* Single plane, Assign write master 0 */
    cmd->outpath.out0.ch0 = 0;
  }
  if (cmd->outpath.out0.ch0 < 0) {
    CDBG_ERROR("%s: Couldn't find write master\n", __func__);
    return -EINVAL;
  }
  cmd->outpath.out0.inst_handle = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].
    pix.output_info.output[PRIMARY].inst_handle;
  wm_config = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
              (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
              (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
              (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
              (RAW_CAMIF_STREAM << STREAM_SEL_SHIFT);
  configure_xbar(cmd, cmd->outpath.out0.ch0, wm_config);

  CDBG("%s: AXI Config\n", __func__);
  axi_cfg_cmd_print(cmd);

  imageWidth = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_width;
  imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_height;
  plane_info = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].plane;

  if (!imageWidth) {
    CDBG_ERROR("%s: image width is not valid\n", __func__);
    return -EINVAL;
  }

  configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                         cmd->outpath.out0.ch0, -1,
                         -1, AXI_INTF_PIXEL_0, PRIMARY,
                         VFE_OUTPUT_PRIMARY, axiOutputPpw, burstLength,
                         1);

  return status;
} /* axi_raw_snapshot_config */

/*===========================================================================
 * FUNCTION    - axi_vfe_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int axi_vfe_config(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  VFE_AXIOutputConfigCmdType *cmd, vfe_ports_used_t vfe_port_used, int *cfg_cmd)
{
  uint32_t axiOutputPpw;
  uint32_t imageWidth = 0, imageHeight = 0;
  plane_info_t *plane_info = NULL;
  uint8_t burstLength = 2;
  uint8_t writeMastersInUsePrimary = 0;
  uint8_t writeMastersInUseSecondary = 0;
  uint8_t writeMastersInUseTertiary1 = 0;
  uint8_t writeMastersInUseTertiary2 = 0;
  uint16_t wm_config[3];
  int i;
  int status = 0;
  axi_obj_t *axi_obj = NULL;

  CDBG("%s: begin", __func__);

  for (i = 0; i < 3; i++) {
    wm_config[i] = -1;
  }

  axiOutputPpw = 8; /* hard code for now.*/
  if(axi_client->obj_idx_mask)
    axi_obj = axi_get_obj(axi_root, axi_client);
  if(!axi_obj) {
    CDBG_ERROR("%s: no axi obj associated with teh client",  __func__);
    return -1;
  }

  cmd->busCmd = 0x0; /*this has no effect*/
  cmd->busCfg = 0x2aaa771;

  if (axi_client->current_target == TARGET_MSM8930)
    burstLength = 0x1; // Use burst length of 4 for 8930.

#ifndef VFE_31
  cmd->busioFormat = 0;
#endif
  cmd->busWrSkipCfg =  0;

  *cfg_cmd = 0;

  if (vfe_port_used & VFE_OUTPUT_PRIMARY) {

    if (axi_client->current_target != TARGET_MSM8930)
      cmd->outpath.out0.ch0 = find_write_master(axi_obj);
    else
      cmd->outpath.out0.ch0 = 0;

    if (cmd->outpath.out0.ch0 < 0) {
      CDBG_ERROR("%s: Out 0 Channel 0 write master not found\n", __func__);
      status = -EINVAL;
      goto channel_00_wm_fail;
    }

    if (axi_client->current_target != TARGET_MSM8930)
      cmd->outpath.out0.ch1 = find_write_master(axi_obj);
    else
      cmd->outpath.out0.ch1 = 1;

    if (cmd->outpath.out0.ch1 < 0) {
      CDBG_ERROR("%s: Out 0 Channel 1 write master not found\n", __func__);
      status = -EINVAL;
      goto channel_01_wm_fail;
    }
    cmd->outpath.out0.inst_handle = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].
      pix.output_info.output[PRIMARY].inst_handle;
    writeMastersInUsePrimary = 2;
    *cfg_cmd = CMD_AXI_CFG_PRIM;

    switch (axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
      output[PRIMARY].format) {
      case CAMERA_YUV_420_NV21:
      case CAMERA_YUV_420_NV21_ADRENO:
      case CAMERA_YUV_422_NV61:
        /* Y, CrCb */
        wm_config[0] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                       (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                       (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
        wm_config[1] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                       (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (NO_SWAP << SWAP_CTRL_SHIFT) |
                       (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
        //cmd->xbarCfg0 = 0x0200;
        break;
      case CAMERA_YUV_420_NV12:
      case CAMERA_YUV_422_NV16:
        /* Y, CbCr */
        wm_config[0] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                       (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                       (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
        wm_config[1] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                       (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (INTRA_AND_INTER_SWAP << SWAP_CTRL_SHIFT) |
                       (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
        //cmd->xbarCfg0 = 0x1A00;
        break;
      case CAMERA_YUV_420_YV12:
        if (axi_client->current_target != TARGET_MSM8930)
          cmd->outpath.out0.ch2 = find_write_master(axi_obj);
        else
          cmd->outpath.out0.ch2 = 2;

        if (cmd->outpath.out0.ch2 < 0) {
          CDBG_ERROR("%s: Out 0 Channel 2 write master not found\n", __func__);
          status = -EINVAL;
          goto channel_02_wm_fail;
        }
        /* Y, Cr, Cb*/
        wm_config[0] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                       (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                       (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
        wm_config[1] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                       (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                       (CR_SINGLE_STREAM << STREAM_SEL_SHIFT);
        wm_config[2] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                       (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                       (CB_SINGLE_STREAM << STREAM_SEL_SHIFT);
        //cmd->xbarCfg0 = 0x204000;
        /* One additional write master used*/
        writeMastersInUsePrimary++;
        *cfg_cmd = CMD_AXI_CFG_PRIM_ALL_CHNLS;
        break;
      default:
        CDBG_ERROR("%s: Primary image format %d not supported\n",
                __func__, axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].format);
        status = -EINVAL;
        break;
    }
    if (status < 0)
      return status;

    configure_xbar(cmd, cmd->outpath.out0.ch0, wm_config[0]);
    configure_xbar(cmd, cmd->outpath.out0.ch1, wm_config[1]);
    if (writeMastersInUsePrimary > 2)
      configure_xbar(cmd, cmd->outpath.out0.ch2, wm_config[2]);
  }

  if (vfe_port_used & VFE_OUTPUT_SECONDARY) {
    if (axi_client->current_target != TARGET_MSM8930)
      cmd->outpath.out1.ch0 = find_write_master(axi_obj);
    else
      cmd->outpath.out1.ch0 = 4;
    if (cmd->outpath.out1.ch0 < 0) {
      CDBG_ERROR("%s: Out 1 Channel 0 write master not found\n", __func__);
      status = -EINVAL;
      goto channel_10_wm_fail;
    }

    if (axi_client->current_target != TARGET_MSM8930)
      cmd->outpath.out1.ch1 = find_write_master(axi_obj);
    else
      cmd->outpath.out1.ch1 = 5;
    if (cmd->outpath.out1.ch1 < 0) {
      CDBG_ERROR("%s: Out 1 Channel 1 write master not found\n", __func__);
      status = -EINVAL;
      goto channel_11_wm_fail;
    }
    cmd->outpath.out1.inst_handle = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].
      pix.output_info.output[SECONDARY].inst_handle;

    /*_info.output[SECONDARY] output needed. 2 more write masters*/
    writeMastersInUseSecondary += 2;

    switch(axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].format) {
      case CAMERA_YUV_420_NV21:
      case CAMERA_YUV_422_NV61:
        wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                       (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                       (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
        wm_config[1] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                       (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (NO_SWAP << SWAP_CTRL_SHIFT) |
                       (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
        //cmd->xbarCfg1 = 0x0301;
        *cfg_cmd |= CMD_AXI_CFG_SEC;
        break;
      case CAMERA_YUV_420_NV12:
      case CAMERA_YUV_422_NV16:
        wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                       (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                       (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
        wm_config[1] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                       (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (INTRA_AND_INTER_SWAP << SWAP_CTRL_SHIFT) |
                       (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
        //cmd->xbarCfg1 = 0x1B01;
        *cfg_cmd |= CMD_AXI_CFG_SEC;
        break;
      case CAMERA_YUV_420_YV12:
        if (axi_client->current_target != TARGET_MSM8930)
          cmd->outpath.out1.ch2 = find_write_master(axi_obj);
        else
           cmd->outpath.out1.ch2 = 6;
        if (cmd->outpath.out1.ch2 < 0) {
          CDBG_ERROR("%s: Out 1 Channel 2 write master not found\n", __func__);
          status = -EINVAL;
          goto channel_12_wm_fail;
        }
        /* Y, Cr, Cb*/
        wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                       (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                       (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
        wm_config[1] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                       (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                       (CR_SINGLE_STREAM << STREAM_SEL_SHIFT);
        wm_config[2] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                       (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                       (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                       (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                       (CB_SINGLE_STREAM << STREAM_SEL_SHIFT);
        /* Y, Cr, Cb*/
        //cmd->xbarCfg1 = 0x214101;
        /* One additional write master used*/
        writeMastersInUseSecondary++;
        *cfg_cmd |= CMD_AXI_CFG_SEC_ALL_CHNLS;
        break;
      default:
        CDBG_ERROR("%s: Secondary Image format %d not supported\n",
                  __func__, axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].format);
        status = -EINVAL;
        break;
    }

    if(status < 0)
      return status;
    configure_xbar(cmd, cmd->outpath.out1.ch0, wm_config[0]);
    configure_xbar(cmd, cmd->outpath.out1.ch1, wm_config[1]);
    if (writeMastersInUseSecondary > 2)
      configure_xbar(cmd, cmd->outpath.out1.ch2, wm_config[2]);
  }

  if (vfe_port_used & VFE_OUTPUT_TERTIARY1) {
    cmd->outpath.out2.ch0 = find_write_master(axi_obj);
    if (cmd->outpath.out2.ch0 < 0) {
      CDBG_ERROR("%s: Out 2 Channel 0 write master not found\n", __func__);
      status = -EINVAL;
      goto tertiary1_fail;
    }
    cmd->outpath.out2.inst_handle = axi_obj->axi_ctrl[AXI_INTF_RDI_0].
      rdi.output_info.output[TERTIARY1].inst_handle;

    writeMastersInUseTertiary1++;

    if ((axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.output[TERTIARY1].
        format == CAMERA_BAYER_SBGGR10) || (axi_obj->axi_ctrl[AXI_INTF_RDI_0].
        rdi.output_info.output[TERTIARY1].format == CAMERA_YUV_422_YUYV)) {
      wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (RDI0_STREAM << STREAM_SEL_SHIFT);
      *cfg_cmd |= CMD_AXI_CFG_TERT1;
    } else {
      CDBG_ERROR("%s: Tert1 Image format %d not supported\n",
        __func__, axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.output[TERTIARY1].format);
      return -EINVAL;
    }
    cmd->pixelIfCfg.rdiEnable = 0x1;
    configure_xbar(cmd, cmd->outpath.out2.ch0, wm_config[0]);
  }

  if (vfe_port_used & VFE_OUTPUT_TERTIARY2) {
    cmd->outpath.out3.ch0 = find_write_master(axi_obj);
    if (cmd->outpath.out3.ch0 < 0) {
      CDBG_ERROR("%s: Out 3 Channel 0 write master not found\n", __func__);
      status = -EINVAL;
      goto tertiary2_fail;
    }
    writeMastersInUseTertiary2++;
    cmd->outpath.out3.inst_handle = axi_obj->axi_ctrl[AXI_INTF_RDI_1].
      rdi.output_info.output[TERTIARY2].inst_handle;

    if ((axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.output[TERTIARY2].
        format == CAMERA_BAYER_SBGGR10) || (axi_obj->axi_ctrl[AXI_INTF_RDI_1].
        rdi.output_info.output[TERTIARY2].format == CAMERA_YUV_422_YUYV)) {
      wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (RDI1_STREAM << STREAM_SEL_SHIFT);
      *cfg_cmd |= CMD_AXI_CFG_TERT2;
    } else {
      CDBG_ERROR("%s: Tert1 Image format %d not supported\n",
      __func__, axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.output[TERTIARY2].format);
      return -EINVAL;
    }
    cmd->rdiCfg0.rdiEnable = 0x1;
    cmd->rdiCfg0.rdiStreamSelect1 = 0x3;
    configure_xbar(cmd, cmd->outpath.out3.ch0, wm_config[0]);
    for (i = 0; i < 3; i++) {
      wm_config[i] = -1;
    }
  }

  CDBG("%s: AXI Config\n", __func__);
  axi_cfg_cmd_print(cmd);

  if (vfe_port_used & VFE_OUTPUT_PRIMARY) {
    /* Primary output configuration */
    imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
      output[PRIMARY].image_width +
      axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
      output[PRIMARY].extra_pad_width;
    imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.
      output_info.output[PRIMARY].image_height +
      axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
      output[PRIMARY].extra_pad_height;
    plane_info = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
      output[PRIMARY].plane;

    configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                           cmd->outpath.out0.ch0, cmd->outpath.out0.ch1,
                           cmd->outpath.out0.ch2, AXI_INTF_PIXEL_0, PRIMARY,
                           VFE_OUTPUT_PRIMARY, axiOutputPpw, burstLength,
                           writeMastersInUsePrimary);
  }

  if (vfe_port_used & VFE_OUTPUT_SECONDARY) {
    /* Secondary output configuration */
    imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
      output[SECONDARY].image_width +
      axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
      output[SECONDARY].extra_pad_width;
    imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.
      output_info.output[SECONDARY].image_height +
      axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
      output[SECONDARY].extra_pad_height;
    plane_info = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
      output[SECONDARY].plane;

    configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                           cmd->outpath.out1.ch0, cmd->outpath.out1.ch1,
                           cmd->outpath.out1.ch2, AXI_INTF_PIXEL_0, SECONDARY,
                           VFE_OUTPUT_SECONDARY, axiOutputPpw, burstLength,
                           writeMastersInUseSecondary);
  }

  if (vfe_port_used & VFE_OUTPUT_TERTIARY1) {
    /* Tert1 output configuration */
    imageWidth  = axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
      output[TERTIARY1].image_width +
      axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
      output[TERTIARY1].extra_pad_width;
    imageHeight = axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
      output[TERTIARY1].image_height +
      axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
      output[TERTIARY1].extra_pad_height;
    plane_info  = axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
      output[TERTIARY1].plane;
    configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                           cmd->outpath.out2.ch0, -1,
                           -1, AXI_INTF_RDI_0, TERTIARY1,
                           VFE_OUTPUT_TERTIARY1, axiOutputPpw, burstLength,
                           writeMastersInUseTertiary1);
  }

  if (vfe_port_used & VFE_OUTPUT_TERTIARY2) {
    /* Tert2 output configuration */
    imageWidth  = axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
      output[TERTIARY2].image_width +
      axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
      output[TERTIARY2].extra_pad_width;
    imageHeight = axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
      output[TERTIARY2].image_height +
      axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
      output[TERTIARY2].extra_pad_height;
    plane_info  = axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
      output[TERTIARY1].plane;

    configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                           cmd->outpath.out3.ch0, -1,
                           -1, AXI_INTF_RDI_1, TERTIARY2,
                           VFE_OUTPUT_TERTIARY2, axiOutputPpw, burstLength,
                           writeMastersInUseTertiary2);
  }

  return status;

tertiary2_fail:
  release_write_master(axi_obj, cmd->outpath.out2.ch0);
tertiary1_fail:
  if (writeMastersInUseSecondary > 2)
    release_write_master(axi_obj, cmd->outpath.out1.ch2);
channel_12_wm_fail:
  release_write_master(axi_obj, cmd->outpath.out1.ch1);
channel_11_wm_fail:
  release_write_master(axi_obj, cmd->outpath.out1.ch0);
channel_10_wm_fail:
  if (writeMastersInUsePrimary > 2)
    release_write_master(axi_obj, cmd->outpath.out0.ch2);
channel_02_wm_fail:
  release_write_master(axi_obj, cmd->outpath.out0.ch1);
channel_01_wm_fail:
  release_write_master(axi_obj, cmd->outpath.out0.ch0);
channel_00_wm_fail:
  return status;
} /* axi_vfe_config */

/*===========================================================================
 * FUNCTION    - axi_vfe_config_snapshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int axi_vfe_config_snapshot(axi_comp_root_t *axi_root, axi_client_t *axi_client,
                      VFE_AXIOutputConfigCmdType *cmd,
                      vfe_ports_used_t vfe_port_used,
                      int *cfg_cmd)
{
  uint32_t axiOutputPpw;
  uint32_t imageWidth = 0, imageHeight = 0;
  plane_info_t *plane_info = NULL;
  uint8_t burstLength = 2;
  uint8_t writeMastersInUsePrimary = 0;
  uint8_t writeMastersInUseSecondary = 0;
  uint8_t writeMastersInUseTertiary1 = 0;
  uint8_t writeMastersInUseTertiary2 = 0;
  int status = 0;
  uint16_t wm_config[2];
  int i;
  axi_obj_t *axi_obj = NULL;

  CDBG("%s begin ", __func__);
  axiOutputPpw = 8; /* hard code for now.*/
  if(axi_client->obj_idx_mask)
    axi_obj = axi_get_obj(axi_root, axi_client);
  if(!axi_obj) {
    CDBG_ERROR("%s: no axi obj associated with teh client",  __func__);
    return -1;
  }

  cmd->busCmd = 0x0; /*this has no effect*/
  cmd->busCfg = 0x2aaa771;

  if (axi_client->current_target == TARGET_MSM8930)
   burstLength = 0x1; // Use burst length of 4 for 8930.

#ifndef VFE_31
  cmd->busioFormat = 0;
#endif
  cmd->busWrSkipCfg =  0;

  *cfg_cmd = 0;

  /*PRIMARY*/
  if (vfe_port_used & VFE_OUTPUT_PRIMARY) {

  if (axi_client->current_target != TARGET_MSM8930)
    cmd->outpath.out0.ch0 = find_write_master(axi_obj);
  else
    cmd->outpath.out0.ch0 = 0;

  if (cmd->outpath.out0.ch0 < 0) {
    CDBG_ERROR("%s: Out 0 Channel 0 write master not found\n", __func__);
    status = -EINVAL;
    goto channel_00_wm_fail;
  }

  if (axi_client->current_target != TARGET_MSM8930)
    cmd->outpath.out0.ch1 = find_write_master(axi_obj);
  else
    cmd->outpath.out0.ch1 = 1;

  if (cmd->outpath.out0.ch1 < 0) {
    CDBG_ERROR("%s: Out 0 Channel 1 write master not found\n", __func__);
    status = -EINVAL;
    goto channel_01_wm_fail;
  }
  cmd->outpath.out0.inst_handle = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].
    pix.output_info.output[PRIMARY].inst_handle;
  writeMastersInUsePrimary = 2;
  *cfg_cmd |= CMD_AXI_CFG_PRIM;

  switch(axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].format) {
    case CAMERA_YUV_420_NV21:
    case CAMERA_YUV_420_NV21_ADRENO:
    case CAMERA_YUV_422_NV61:
      /* Y, CrCb */
      wm_config[0] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
      wm_config[1] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                     (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (NO_SWAP << SWAP_CTRL_SHIFT) |
                     (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
      //cmd->xbarCfg0 = 0x0200;
      break;
    case CAMERA_YUV_420_NV12:
    case CAMERA_YUV_422_NV16:
      /* Y, CbCr */
      wm_config[0] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
      wm_config[1] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                     (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (INTRA_AND_INTER_SWAP << SWAP_CTRL_SHIFT) |
                     (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
      //cmd->xbarCfg0 = 0x1A00;
      break;
    default:
      CDBG_ERROR("%s: Primary image format %d not supported\n",
                __func__, axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].format);
      status = -EINVAL;
      break;
  }
  if(status < 0)
    return status;
  configure_xbar(cmd, cmd->outpath.out0.ch0, wm_config[0]);
  configure_xbar(cmd, cmd->outpath.out0.ch1, wm_config[1]);
  }

  if (vfe_port_used & VFE_OUTPUT_SECONDARY) {
  /*SECONDARY*/
  if (axi_client->current_target != TARGET_MSM8930)
    cmd->outpath.out1.ch0 = find_write_master(axi_obj);
  else
    cmd->outpath.out1.ch0 = 4;

  if (cmd->outpath.out1.ch0 < 0) {
    CDBG_ERROR("%s: Out 1 Channel 0 write master not found\n", __func__);
    status = -EINVAL;
    goto channel_10_wm_fail;
  }
  if (axi_client->current_target != TARGET_MSM8930)
    cmd->outpath.out1.ch1 = find_write_master(axi_obj);
  else
    cmd->outpath.out1.ch1 = 5;

  if (cmd->outpath.out1.ch1 < 0) {
    CDBG_ERROR("%s: Out 1 Channel 1 write master not found\n", __func__);
    status = -EINVAL;
    goto channel_11_wm_fail;
  }
  cmd->outpath.out1.inst_handle = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].
    pix.output_info.output[SECONDARY].inst_handle;
  /*_info.output[SECONDARY] output needed. 2 more write masters*/
  writeMastersInUseSecondary += 2;
  *cfg_cmd |= CMD_AXI_CFG_SEC;
  switch(axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].format) {
    case CAMERA_YUV_420_NV21:
    case CAMERA_YUV_422_NV61:
      wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
      wm_config[1] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (NO_SWAP << SWAP_CTRL_SHIFT) |
                     (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
      //cmd->xbarCfg1 = 0x0301;
      break;
    case CAMERA_YUV_420_NV12:
    case CAMERA_YUV_422_NV16:
      wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
      wm_config[1] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (INTRA_AND_INTER_SWAP << SWAP_CTRL_SHIFT) |
                     (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
      //cmd->xbarCfg1 = 0x1B01;
      break;
    default:
      CDBG_ERROR("%s: Secondary Image format %d not supported\n",
                __func__, axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].format);
      status = -EINVAL;
      break;
  }

    if(status < 0)
      return status;
  configure_xbar(cmd, cmd->outpath.out1.ch0, wm_config[0]);
  configure_xbar(cmd, cmd->outpath.out1.ch1, wm_config[1]);

  CDBG("%s: AXI Config\n", __func__);
  axi_cfg_cmd_print(cmd);
 }

  if (vfe_port_used & VFE_OUTPUT_TERTIARY1) {
    cmd->outpath.out2.ch0 = find_write_master(axi_obj);
    if (cmd->outpath.out2.ch0 < 0) {
      CDBG_ERROR("%s: Out 2 Channel 0 write master not found\n", __func__);
      status = -EINVAL;
      goto tertiary1_fail;
    }
    cmd->outpath.out2.inst_handle = axi_obj->axi_ctrl[AXI_INTF_RDI_0].
      rdi.output_info.output[TERTIARY1].inst_handle;

    writeMastersInUseTertiary1++;
    *cfg_cmd |= CMD_AXI_CFG_TERT1;

    if ((axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.output[TERTIARY1].
        format == CAMERA_BAYER_SBGGR10) || (axi_obj->axi_ctrl[AXI_INTF_RDI_0].
        rdi.output_info.output[TERTIARY1].format == CAMERA_YUV_422_YUYV)) {
      wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (RDI0_STREAM << STREAM_SEL_SHIFT);
      *cfg_cmd |= CMD_AXI_CFG_TERT1;
    } else {
      CDBG_ERROR("%s: Tert1 Image format %d not supported\n",
        __func__, axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.output[TERTIARY1].format);
      return -EINVAL;
    }
    cmd->pixelIfCfg.rdiEnable = 0x1;
    configure_xbar(cmd, cmd->outpath.out2.ch0, wm_config[0]);
  }

  if (vfe_port_used & VFE_OUTPUT_TERTIARY2) {
    cmd->outpath.out3.ch0 = find_write_master(axi_obj);
    if (cmd->outpath.out3.ch0 < 0) {
      CDBG_ERROR("%s: Out 3 Channel 0 write master not found\n", __func__);
      status = -EINVAL;
      goto tertiary2_fail;
    }
    writeMastersInUseTertiary2++;
    cmd->outpath.out3.inst_handle = axi_obj->axi_ctrl[AXI_INTF_RDI_1].
      rdi.output_info.output[TERTIARY2].inst_handle;

    if((axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.output[TERTIARY2].
        format == CAMERA_BAYER_SBGGR10) || (axi_obj->axi_ctrl[AXI_INTF_RDI_1].
        rdi.output_info.output[TERTIARY2].format == CAMERA_YUV_422_YUYV)) {
      wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (RDI1_STREAM << STREAM_SEL_SHIFT);
      *cfg_cmd |= CMD_AXI_CFG_TERT2;
    } else {
      CDBG_ERROR("%s: Tert1 Image format %d not supported\n",
      __func__, axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.output[TERTIARY2].format);
      return -EINVAL;
    }
    cmd->rdiCfg0.rdiEnable = 0x1;
    configure_xbar(cmd, cmd->outpath.out3.ch0, wm_config[0]);
    for (i = 0; i < 2; i++) {
      wm_config[i] = -1;
    }
  }

  /*Primary WM configuration*/
  if (vfe_port_used & VFE_OUTPUT_PRIMARY) {
  imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_width;
  imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_height;
  plane_info  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].plane;

  configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                         cmd->outpath.out0.ch0, cmd->outpath.out0.ch1,
                         -1, AXI_INTF_PIXEL_0, PRIMARY,
                         VFE_OUTPUT_PRIMARY, axiOutputPpw, burstLength,
                         writeMastersInUsePrimary);
  }
  /* Secondary WM configuration */
  if (vfe_port_used & VFE_OUTPUT_SECONDARY) {
  imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].image_width;
  imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].image_height;
  plane_info  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].plane;

  configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                         cmd->outpath.out1.ch0, cmd->outpath.out1.ch1,
                         -1, AXI_INTF_PIXEL_0, SECONDARY,
                         VFE_OUTPUT_SECONDARY, axiOutputPpw, burstLength,
                         writeMastersInUseSecondary);
  }
  if (vfe_port_used & VFE_OUTPUT_TERTIARY1) {
    /* Tert1 output configuration */
    imageWidth  = axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
      output[TERTIARY1].image_width +
      axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
      output[TERTIARY1].extra_pad_width;
    imageHeight = axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
      output[TERTIARY1].image_height +
      axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
      output[TERTIARY1].extra_pad_height;
    plane_info = axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
      output[TERTIARY1].plane;
    configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                           cmd->outpath.out2.ch0, -1,
                           -1, AXI_INTF_RDI_0, TERTIARY1,
                           VFE_OUTPUT_TERTIARY1, axiOutputPpw, burstLength,
                           writeMastersInUseTertiary1);
  }

  if (vfe_port_used & VFE_OUTPUT_TERTIARY2) {
    /* Tert2 output configuration */
    imageWidth  = axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
      output[TERTIARY2].image_width +
      axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
      output[TERTIARY2].extra_pad_width;
    imageHeight = axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
      output[TERTIARY2].image_height +
      axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
      output[TERTIARY2].extra_pad_height;
    plane_info  = axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
      output[TERTIARY2].plane;

    configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                           cmd->outpath.out3.ch0, -1,
                           -1, AXI_INTF_RDI_1, TERTIARY2,
                           VFE_OUTPUT_TERTIARY2, axiOutputPpw, burstLength,
                           writeMastersInUseTertiary2);
  }

  return status;

tertiary2_fail:
  release_write_master(axi_obj, cmd->outpath.out2.ch0);
tertiary1_fail:
  if (writeMastersInUseSecondary > 2)
    release_write_master(axi_obj, cmd->outpath.out1.ch2);
channel_11_wm_fail:
  release_write_master(axi_obj, cmd->outpath.out1.ch0);
channel_10_wm_fail:
  release_write_master(axi_obj, cmd->outpath.out0.ch1);
channel_01_wm_fail:
  release_write_master(axi_obj, cmd->outpath.out0.ch0);
channel_00_wm_fail:
  return status;
} /* axi_vfe_config_snapshot */

/*===========================================================================
 * FUNCTION    - axi_jpeg_snapshot_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int axi_jpeg_snapshot_config(axi_comp_root_t *axi_root, axi_client_t *axi_client,
                                    VFE_AXIOutputConfigCmdType *cmd,
                                    vfe_ports_used_t vfe_port_used)
{
  uint32_t axiOutputPpw;
  uint32_t imageWidth = 0, imageHeight = 0;
  plane_info_t *plane_info = NULL;
  uint8_t burstLength = 2;
  uint8_t minUbSize = 64;
  uint8_t writeMastersInUsePrimary = 0;
  uint8_t writeMastersInUseSecondary = 0;
  int status = 0;
  uint16_t wm_config[2];
  axi_obj_t *axi_obj = NULL;

  axiOutputPpw = 8; /* hard code for now.*/
  if(axi_client->obj_idx_mask)
    axi_obj = axi_get_obj(axi_root, axi_client);
  if(!axi_obj) {
    CDBG_ERROR("%s: no axi obj associated with teh client",  __func__);
    return -1;
  }

  cmd->busCmd = 0x0; /*this has no effect*/
  //cmd->busCfg = 0x2aaa511; // For FIFO mode
  cmd->busCfg = 0x82aaa001;  // For Ping-pong mode

  if (axi_client->current_target == TARGET_MSM8930)
    burstLength = 0x1; // Use burst length of 4 for 8930.

#ifndef VFE_31
  cmd->busioFormat = 0;
#endif
  cmd->busWrSkipCfg =  0;

  /*PRIMARY*/
  if (axi_client->current_target != TARGET_MSM8930)
    cmd->outpath.out0.ch0 = find_write_master(axi_obj);
  else
    cmd->outpath.out0.ch0 = 0;

  if (cmd->outpath.out0.ch0 < 0) {
    CDBG_ERROR("%s: Out 0 Channel 0 write master not found\n", __func__);
    status = -EINVAL;
    goto channel_00_wm_fail;
  }
  if (axi_client->current_target != TARGET_MSM8930)
    cmd->outpath.out0.ch1 = find_write_master(axi_obj);
  else
    cmd->outpath.out0.ch1 = 1;

  if (cmd->outpath.out0.ch1 < 0) {
    CDBG_ERROR("%s: Out 0 Channel 1 write master not found\n", __func__);
    status = -EINVAL;
    goto channel_01_wm_fail;
  }
  cmd->busCfg = cmd->busCfg |
    (cmd->outpath.out0.ch0 << 4) | (cmd->outpath.out0.ch1 << 8);
  cmd->outpath.out0.inst_handle = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].
    pix.output_info.output[PRIMARY].inst_handle;
  writeMastersInUsePrimary = 2;

  switch(axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].format) {
    case CAMERA_YUV_420_NV21:
    case CAMERA_YUV_420_NV21_ADRENO:
    case CAMERA_YUV_422_NV61:
      /* Y, CrCb */
      wm_config[0] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
      wm_config[1] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                     (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (NO_SWAP << SWAP_CTRL_SHIFT) |
                     (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
      //cmd->xbarCfg0 = 0x0200;
      break;
    case CAMERA_YUV_420_NV12:
    case CAMERA_YUV_422_NV16:
      /* Y, CbCr */
      wm_config[0] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
      wm_config[1] = (ENCODER_STREAM << STREAM_SOURCE_SHIFT) |
                     (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (INTRA_AND_INTER_SWAP << SWAP_CTRL_SHIFT) |
                     (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
      //cmd->xbarCfg0 = 0x1A00;
      break;
    default:
      CDBG_ERROR("%s: Primary image format %d not supported\n",
                __func__, axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].format);
      status = -EINVAL;
      break;
  }
  if(status < 0)
    return status;
  configure_xbar(cmd, cmd->outpath.out0.ch0, wm_config[0]);
  configure_xbar(cmd, cmd->outpath.out0.ch1, wm_config[1]);

  /*SECONDARY*/
  if (axi_client->current_target != TARGET_MSM8930)
    cmd->outpath.out1.ch0 = find_write_master(axi_obj);
  else
    cmd->outpath.out1.ch0 = 4;

  if (cmd->outpath.out1.ch0 < 0) {
    CDBG_ERROR("%s: Out 1 Channel 0 write master not found\n", __func__);
    status = -EINVAL;
    goto channel_10_wm_fail;
  }
  if (axi_client->current_target != TARGET_MSM8930)
    cmd->outpath.out1.ch1 = find_write_master(axi_obj);
  else
    cmd->outpath.out1.ch1 = 5;

  if (cmd->outpath.out1.ch1 < 0) {
    CDBG_ERROR("%s: Out 1 Channel 1 write master not found\n", __func__);
    status = -EINVAL;
    goto channel_11_wm_fail;
  }
  cmd->outpath.out1.inst_handle = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].
    pix.output_info.output[SECONDARY].inst_handle;
  /*_info.output[SECONDARY] output needed. 2 more write masters*/
  writeMastersInUseSecondary += 2;

  switch(axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].format) {
    case CAMERA_YUV_420_NV21:
    case CAMERA_YUV_422_NV61:
      wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
      wm_config[1] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (NO_SWAP << SWAP_CTRL_SHIFT) |
                     (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
      //cmd->xbarCfg1 = 0x0301;
      break;
    case CAMERA_YUV_420_NV12:
    case CAMERA_YUV_422_NV16:
      wm_config[0] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (SINGLE_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (DONT_CARE_SWAP << SWAP_CTRL_SHIFT) |
                     (Y_SINGLE_STREAM << STREAM_SEL_SHIFT);
      wm_config[1] = (VIEWFINDER_STREAM << STREAM_SOURCE_SHIFT) |
                     (PAIRED_STREAM << STREAM_TYPE_SHIFT) |
                     (Y_CBCR_UNALIGNED << PIXEL_ALIGNMENT_SHIFT) |
                     (INTRA_AND_INTER_SWAP << SWAP_CTRL_SHIFT) |
                     (DONT_CARE_STREAM << STREAM_SEL_SHIFT);
      //cmd->xbarCfg1 = 0x1B01;
      break;
    default:
      CDBG_ERROR("%s: Secondary Image format %d not supported\n",
                __func__, axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].format);
      status = -EINVAL;
      break;
  }

  if(status < 0)
    return status;
  configure_xbar(cmd, cmd->outpath.out1.ch0, wm_config[0]);
  configure_xbar(cmd, cmd->outpath.out1.ch1, wm_config[1]);

  CDBG("%s: AXI Config\n", __func__);
  axi_cfg_cmd_print(cmd);

  /*Primary output configuration*/
  imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_width;
  imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_height;
  plane_info  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].plane;

  configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                         cmd->outpath.out0.ch0, cmd->outpath.out0.ch1,
                         -1, AXI_INTF_PIXEL_0, PRIMARY,
                         VFE_OUTPUT_PRIMARY, axiOutputPpw, burstLength,
                         writeMastersInUsePrimary);

  /* Secondary output configuration */
  imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].image_width;
  imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].image_height;
  plane_info  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].plane;

  configure_wm_registers(cmd, axi_obj, imageWidth, imageHeight, plane_info,
                         cmd->outpath.out1.ch0, cmd->outpath.out1.ch1,
                         -1, AXI_INTF_PIXEL_0, SECONDARY,
                         VFE_OUTPUT_SECONDARY, axiOutputPpw, burstLength,
                         writeMastersInUseSecondary);

  CDBG("%s:sensor_output_format=%d PRIMARY w=%d h=%d", __func__,
    axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.sensor_data.sensor_output_format,
    axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_width,
    axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_height);

  CDBG("%s:sensor_output_format=%d PRIMARY w=%d h=%d", __func__,
    axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.sensor_data.sensor_output_format,
    axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].image_width,
    axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].image_height);

  return status;

channel_11_wm_fail:
  release_write_master(axi_obj, cmd->outpath.out1.ch0);
channel_10_wm_fail:
  release_write_master(axi_obj, cmd->outpath.out0.ch1);
channel_01_wm_fail:
  release_write_master(axi_obj, cmd->outpath.out0.ch0);
channel_00_wm_fail:
  return status;
} /* axi_jpeg_snapshot_config */

/*===========================================================================
 * FUNCTION    - axi_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
int axi_config(axi_comp_root_t *axi_root, axi_client_t *axi_client,
               axi_config_t *cfg)
{
  int status = 0;
  struct msm_vfe_cfg_cmd cfgCmd;
  struct msm_isp_cmd ispcmd;
  axi_obj_t *axi_obj = NULL;

  assert(axi_client->obj_idx_mask - 1 >= 0);
  assert(axi_client->obj_idx_mask < AXI_MAX_OBJS);
  axi_obj = &axi_root->axi_obj[axi_client->obj_idx_mask-1];

  CDBG("%s: begin", __func__);
  ispcmd.id = VFE_CMD_AXI_OUT_CFG;
  ispcmd.length = sizeof(axi_obj->VFE_AXIOutputConfigCmd);
  ispcmd.value = (void *)&axi_obj->VFE_AXIOutputConfigCmd;

  axi_obj->VFE_AXIOutputConfigCmd.pixelIfCfg.inputSelect = VFE_PIXEL_IF_MIPI;

  switch(cfg->mode) {
    case CAM_OP_MODE_VIDEO:
    case CAM_OP_MODE_ZSL:
      status = axi_vfe_config(axi_root, axi_client,
                 &axi_obj->VFE_AXIOutputConfigCmd, cfg->vfe_port, &cfgCmd.cmd_type);
      cfgCmd.length = sizeof(struct msm_isp_cmd);
      cfgCmd.value = &ispcmd;
      break;
    case CAM_OP_MODE_SNAPSHOT:
      status = axi_vfe_config_snapshot(axi_root, axi_client,
                 &axi_obj->VFE_AXIOutputConfigCmd, cfg->vfe_port, &cfgCmd.cmd_type);
      cfgCmd.length = sizeof(struct msm_isp_cmd);
      cfgCmd.value = &ispcmd;
      break;
    case CAM_OP_MODE_RAW_SNAPSHOT:
      status = axi_raw_snapshot_config(axi_root, axi_client,
                 &axi_obj->VFE_AXIOutputConfigCmd);
      cfgCmd.cmd_type = CMD_AXI_CFG_PRIM;
      cfgCmd.length = sizeof(struct msm_isp_cmd);
      cfgCmd.value = &ispcmd;
      break;
  case CAM_OP_MODE_JPEG_SNAPSHOT:
      CDBG("%s CAM_OP_MODE_JPEG_SNAPSHOT", __func__);
      status = axi_jpeg_snapshot_config(axi_root, axi_client,
                 &axi_obj->VFE_AXIOutputConfigCmd, cfg->vfe_port);
      cfgCmd.cmd_type = CMD_AXI_CFG_PRIM|CMD_AXI_CFG_SEC;
      cfgCmd.length = sizeof(struct msm_isp_cmd);
      cfgCmd.value = &ispcmd;
      break;
  default:
      status = -EINVAL;
      break;
  }
  if (status < 0) {
    CDBG_HIGH("%s: axi_process failed for %d \n", __func__, cfg->mode);
    return status;
  }

  status = ioctl(axi_client->ops->fd, MSM_CAM_IOCTL_AXI_CONFIG, &cfgCmd);
  if(status < 0) {
      CDBG_ERROR("%s: MSM_CAM_IOCTL_AXI_CONFIG failed!\n", __func__);
      return status;
  }
  CDBG("%s AXI Configuration success ", __func__);
  return status;
} /* axi_config */

int axi_set_params(axi_comp_root_t *axi_root, axi_client_t *axi_client,
                   int type, axi_set_t *axi_set_parm)
{
  int rc = 0, i;
  axi_obj_t *axi_obj = NULL;

  CDBG("%s: begin, axi_obj_idx = %d", __func__, axi_set_parm->data.axi_obj_idx);
  axi_obj= &axi_root->axi_obj[axi_set_parm->data.axi_obj_idx];
  switch(type) {
  case AXI_PARM_ADD_OBJ_ID:
    if(axi_client->obj_idx_mask) {
      CDBG_ERROR("%s: client has associated with axi obj", __func__);
      rc = -1;
    } else {
      axi_client->obj_idx_mask = (1 << axi_set_parm->data.axi_obj_idx);
      axi_obj = &axi_root->axi_obj[axi_set_parm->data.axi_obj_idx];
      if (axi_obj->ref_count == 0) {
        for (i = 0; i < AXI_MAX_WM_NUM; i++) {
          axi_obj->axi_wm_table[i].is_reserved = 0;
          axi_obj->axi_wm_table[i].use_count = 0;
          axi_obj->axi_wm_table[i].ub_info.ub_offset =
            i * EQUALLY_SLICED_UB_SIZE;
          axi_obj->axi_wm_table[i].ub_info.ub_size =
            EQUALLY_SLICED_UB_SIZE - 1;
        }
      }
      axi_obj->ref_count++;
    }
    break;
  case AXI_PARM_HW_VERSION:
    axi_client->vfe_version = axi_set_parm->data.vfe_version;
    axi_client->current_target = axi_set_parm->data.current_target;
    break;
  case AXI_PARM_RESERVE_INTF:{
    uint8_t intf_mask = axi_set_parm->data.interface_mask;
    int index = 0x1;
    uint8_t intf_type;
    while (intf_mask != 0) {
      switch (axi_set_parm->data.interface_mask & index) {
      case PIX_0:
        intf_type = AXI_INTF_PIXEL_0;
        break;
      case RDI_0:
        intf_type = AXI_INTF_RDI_0;
        break;
      case RDI_1:
        intf_type = AXI_INTF_RDI_1;
        break;
      case RDI_2:
        intf_type = AXI_INTF_RDI_2;
        break;
      default:
        intf_type = AXI_INTF_MAX_NUM;
        break;
      }
      if (intf_type != AXI_INTF_MAX_NUM) {
        axi_obj->axi_ctrl[intf_type].client_idx =
            axi_client->client_idx;
        axi_obj->axi_ctrl[intf_type].used = 1;
      }
      index <<= 1;
      intf_mask >>= 1;
    }
    break;
  }
  case AXI_PARM_SENSOR_DATA:{
    uint8_t intf_mask = axi_set_parm->data.interface_mask;
    int index = 0x1;
    uint8_t intf_type;
    while (intf_mask != 0) {
      switch (axi_set_parm->data.interface_mask & index) {
      case PIX_0:
        intf_type = AXI_INTF_PIXEL_0;
        axi_obj->axi_ctrl[intf_type].pix.sensor_data =
          axi_set_parm->data.sensor_data;
        break;
      case RDI_0:
        intf_type = AXI_INTF_RDI_0;
        axi_obj->axi_ctrl[intf_type].rdi.sensor_data =
          axi_set_parm->data.sensor_data;
        break;
      case RDI_1:
        intf_type = AXI_INTF_RDI_1;
        axi_obj->axi_ctrl[intf_type].rdi.sensor_data =
          axi_set_parm->data.sensor_data;
        break;
      case RDI_2:
        intf_type = AXI_INTF_RDI_2;
        axi_obj->axi_ctrl[intf_type].rdi.sensor_data =
          axi_set_parm->data.sensor_data;
        break;
      default:
        intf_type = AXI_INTF_MAX_NUM;
        break;
      }
      index <<= 1;
      intf_mask >>= 1;
    }
    break;
  }
  case AXI_PARM_OUTPUT_INFO: {
    uint8_t port_info = axi_set_parm->data.output_info.vfe_ports_used;
    int index = 0x1;
    while (port_info != 0) {
      switch (axi_set_parm->data.output_info.vfe_ports_used & index) {
      case VFE_OUTPUT_PRIMARY:
      case VFE_OUTPUT_SECONDARY:
        axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info =
        axi_set_parm->data.output_info;
        break;
      case VFE_OUTPUT_TERTIARY1:
        axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info =
        axi_set_parm->data.output_info;
        break;
      case VFE_OUTPUT_TERTIARY2:
        axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info =
        axi_set_parm->data.output_info;
        break;
      default:
        break;
      }
      index <<= 1;
      port_info >>= 1;
    }
    if (axi_client->current_target == TARGET_MSM8930)
      axi_update_UB_data(axi_obj, axi_client, &axi_set_parm->data.output_info);
    break;
  }
  case AXI_PARM_UPDATE_CONFIG:
    switch (axi_set_parm->data.mod_params.port_info) {
    case VFE_OUTPUT_PRIMARY:
      axi_obj->VFE_AXIOutputConfigCmd.outpath.out0.inst_handle =
        axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
          output[PRIMARY].inst_handle =
        axi_set_parm->data.mod_params.inst_handle;
      break;
    case VFE_OUTPUT_SECONDARY:
      axi_obj->VFE_AXIOutputConfigCmd.outpath.out1.inst_handle =
        axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.
          output[SECONDARY].inst_handle =
        axi_set_parm->data.mod_params.inst_handle;
      break;
    case VFE_OUTPUT_TERTIARY1:
      axi_obj->VFE_AXIOutputConfigCmd.outpath.out2.inst_handle =
        axi_obj->axi_ctrl[AXI_INTF_RDI_0].rdi.output_info.
          output[TERTIARY1].inst_handle =
        axi_set_parm->data.mod_params.inst_handle;
      break;
    case VFE_OUTPUT_TERTIARY2:
      axi_obj->VFE_AXIOutputConfigCmd.outpath.out3.inst_handle =
        axi_obj->axi_ctrl[AXI_INTF_RDI_1].rdi.output_info.
          output[TERTIARY2].inst_handle =
        axi_set_parm->data.mod_params.inst_handle;
      break;
    default:
      break;
    }
    break;

  case AXI_PARM_PREVIEW_FORMAT:
    axi_obj->axi_ctrl[axi_set_parm->data.intf_type].pix.prev_format =
      axi_set_parm->data.prev_format;
    break;
  case AXI_PARM_SNAPSHOT_FORMAT:
    axi_obj->axi_ctrl[axi_set_parm->data.intf_type].pix.snap_format =
      axi_set_parm->data.snap_format;
    break;
  case AXI_PARM_RECORDING_FORMAT:
    axi_obj->axi_ctrl[axi_set_parm->data.intf_type].pix.rec_format =
      axi_set_parm->data.rec_format;
    break;
  case AXI_PARM_THUMBNAIL_FORMAT:
    axi_obj->axi_ctrl[axi_set_parm->data.intf_type].pix.thumb_format =
      axi_set_parm->data.thumb_format;
    break;
  case AXI_PARM_STATS_VERSION:
    axi_client->stats_version = axi_set_parm->data.stats_version;
  break;
  default:
    CDBG_ERROR("%s Unsupported set parm type ", __func__);
    rc = -EINVAL;
    break;
  }
  return rc;
}

/*===========================================================================
 * FUNCTION    - axi_send_cmd -
 *
 * DESCRIPTION:
 *==========================================================================*/
int axi_send_cmd(int fd, int type,
  void *pCmdData, unsigned int messageSize, int cmd_id)
{
  int rc;
  struct msm_vfe_cfg_cmd cfgCmd;
  struct msm_isp_cmd ispcmd;

  ispcmd.id = 0;
  ispcmd.length = messageSize;
  ispcmd.value = pCmdData;

  cfgCmd.cmd_type = cmd_id;
  cfgCmd.length = sizeof(struct msm_isp_cmd);
  cfgCmd.value = &ispcmd;

  rc = ioctl(fd, MSM_CAM_IOCTL_AXI_CONFIG, &cfgCmd);
  if (rc < 0)
    CDBG_ERROR("%s: MSM_CAM_IOCTL_AXI_CONFIG failed %d\n",
      __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION    - axi_command_ops -
 *
 * DESCRIPTION:
 *==========================================================================*/
int axi_command_ops(axi_comp_root_t *axi_root, axi_client_t *axi_client,
               void *parm)
{
  int rc = 0;
  int i;
  mod_cmd_t *cmd = (mod_cmd_t *)parm;
  struct msm_camera_vfe_params_t *mod_params = NULL;
  axi_obj_t *axi_obj = NULL;

  assert(cmd);
  assert(axi_client->obj_idx_mask - 1 >= 0);
  assert(axi_client->obj_idx_mask < AXI_MAX_OBJS);

  mod_params = (struct msm_camera_vfe_params_t *)cmd->cmd_data;
  axi_obj = &axi_root->axi_obj[axi_client->obj_idx_mask-1];

  CDBG("%s cmd_type = %d", __func__, cmd->mod_cmd_ops);
  switch(cmd->mod_cmd_ops) {
   case MOD_CMD_START:
     rc = axi_send_cmd(axi_client->ops->fd, 0, cmd->cmd_data, cmd->length, CMD_AXI_START);
     break;
   case MOD_CMD_STOP:
     rc = axi_send_cmd(axi_client->ops->fd, 0, cmd->cmd_data, cmd->length, CMD_AXI_STOP);
     break;
   case MOD_CMD_ABORT:
     if (axi_obj->ref_count <= 1)
       rc = axi_send_cmd(axi_client->ops->fd, 0, cmd->cmd_data, cmd->length, CMD_AXI_ABORT);
     else
       rc = 1;
     break;
   case MOD_CMD_RESET:
     if (axi_obj->ref_count <= 1 || !axi_obj->reset_done) {
       axi_obj->reset_done = true;
       memset(&axi_obj->VFE_AXIOutputConfigCmd, 0,
         sizeof(VFE_AXIOutputConfigCmdType));
     } else
       mod_params->skip_reset = true;
     rc = axi_send_cmd(axi_client->ops->fd, 0, cmd->cmd_data, cmd->length, CMD_AXI_RESET);
     break;
   default:
     CDBG_ERROR("cmd_type = %d not supported", cmd->mod_cmd_ops);
     rc = -EINVAL;
     break;
  }
  if (rc < 0) {
    CDBG_ERROR("Failed configuring cmd_type = %d", cmd->mod_cmd_ops);
    goto end;
  }

  if (cmd->mod_cmd_ops != MOD_CMD_START && cmd->mod_cmd_ops != MOD_CMD_STOP)
    goto end;

  for (i = 0; i < AXI_MAX_WM_NUM; i ++) {
    if (axi_obj->axi_wm_table[i].port & mod_params->port_info) {
      if (cmd->mod_cmd_ops == MOD_CMD_START && axi_obj->axi_wm_table[i].state == AXI_WM_IDLE) {
        axi_obj->axi_wm_table[i].state = AXI_WM_SENT_START;
      }
      if (cmd->mod_cmd_ops == MOD_CMD_STOP && axi_obj->axi_wm_table[i].state == AXI_WM_ACTIVE) {
        axi_obj->axi_wm_table[i].state = AXI_WM_SENT_STOP;
      }
    }
  }
end:
  return rc;
}

/*===========================================================================
 * FUNCTION    - axi_command_ops -
 *
 * DESCRIPTION:
 *==========================================================================*/
int axi_proc_reg_update(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  void *parm)
{
  int rc = 0;
  int interface = (int)parm;
  int i;
  axi_obj_t *axi_obj = NULL;

  assert(axi_client->obj_idx_mask - 1 >= 0);
  assert(axi_client->obj_idx_mask < AXI_MAX_OBJS);
  axi_obj = &axi_root->axi_obj[axi_client->obj_idx_mask-1];

  for (i = 0; i < AXI_MAX_WM_NUM; i++)
    if (axi_obj->axi_wm_table[i].interface == interface) {
      if (axi_obj->axi_wm_table[i].state == AXI_WM_SENT_START) {
        CDBG("%s: write master %d is active\n", __func__, i);
        axi_obj->axi_wm_table[i].state = AXI_WM_ACTIVE;
      } else if (axi_obj->axi_wm_table[i].state == AXI_WM_SENT_STOP) {
        axi_obj->axi_wm_table[i].state = AXI_WM_IDLE;
      }
    }

  return rc;
}

/*===========================================================================
 * FUNCTION    - axi_command_ops -
 *
 * DESCRIPTION:
 *==========================================================================*/
int axi_proc_unregister_wms(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  void *parm)
{
  int rc = 0;
  int interface = (int)parm;
  int i;
  axi_obj_t *axi_obj = NULL;

  assert(axi_client->obj_idx_mask - 1 >= 0);
  assert(axi_client->obj_idx_mask < AXI_MAX_OBJS);
  axi_obj = &axi_root->axi_obj[axi_client->obj_idx_mask-1];

  for (i = 0; i < AXI_MAX_WM_NUM; i++)
    if (axi_obj->axi_wm_table[i].interface == interface) {
//      if (axi_obj->axi_wm_table[i].state == AXI_WM_IDLE) {
        CDBG("%s: Releasing write master %d\n", __func__, i);
        release_write_master(axi_obj, i);
//      }
    }
  switch(interface) {
  case AXI_INTF_RDI_0:
    axi_obj->VFE_AXIOutputConfigCmd.pixelIfCfg.rdiEnable = 0x0;
    break;
  case AXI_INTF_RDI_1:
    axi_obj->VFE_AXIOutputConfigCmd.rdiCfg0.rdiEnable = 0x0;
    break;
  default:
    break;
  };

  return rc;
}
