
/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <string.h>
#include <stdio.h>
#include <media/msm_gemini.h>
#include "gemini_lib_hw.h"
#include "gemini_hw_reg.h"
#include "gemini_hw_reg_ctrl.h"

#include "gemini_dbg.h"

/*  first dimension is FE_BURST_MASK -> CBCR_WRITE_MASK_3 */
/*  second dimension is burst length (BL=4 or BL=8) */
static const uint32_t GEMINI_FE_RL_BURSTMASK[9][2] = { {0x00000f0f, 0x0000ffff},	/*  0 BURST_MASK: BL=4 & BL=8 ref 2.2.10 SWI */
{0xff0000ff, 0xff0000ff},	/*  1 Y_WRITE_MASK_0 */
{0xff0000ff, 0xff0000ff},	/*  2 Y_WRITE_MASK_1 */
{0x00000000, 0xff0000ff},	/*  3 Y_WRITE_MASK_2 */
{0x00000000, 0xff0000ff},	/*  4 Y_WRITE_MASK_3 */
{0xf0f00f0f, 0xf0f00f0f},	/*  5 CBCR_WRITE_MASK_0 */
{0xf0f00f0f, 0xf0f00f0f},	/*  6 CBCR_WRITE_MASK_1 */
{0x00000000, 0xf0f00f0f},	/*  7 CBCR_WRITE_MASK_2 */
{0x00000000, 0xf0f00f0f}
};				/*  8 CBCR_WRITE_MASK_3 */

/*  first dimension is FE_BURST_MASK -> CBCR_WRITE_MASK_3 */
/*  second dimension is input format H2V1, H2V2 and H1V2 */
/*  for 64-bit bus */
static const uint32_t GEMINI_FE_OL_BURSTMASK_64BB[9][3] = { {0x00000303, 0x00000303, 0x00000301},	/*  0 BURST_MASK: ref 2.2.10 SWI */
{0x00f0000f, 0x00f0000f, 0x00f0000f},	/*  1 Y_WRITE_MASK_0 */
{0xf0000f00, 0xf0000f00, 0x00000000},	/*  2 Y_WRITE_MASK_1 */
{0x00000000, 0x00000000, 0x00000000},	/*  3 Y_WRITE_MASK_2 */
{0x00000000, 0x00000000, 0x00000000},	/*  4 Y_WRITE_MASK_3 */
{0x0c0c0303, 0x0c0c0303, 0x0c0c0303},	/*  5 CBCR_WRITE_MASK_0 */
{0xc0c03030, 0xc0c03030, 0xc0c03030},	/*  6 CBCR_WRITE_MASK_1 */
{0x00000000, 0x00000000, 0x00000000},	/*  7 CBCR_WRITE_MASK_2 */
{0x00000000, 0x00000000, 0x00000000}
};				/*  8 CBCR_WRITE_MASK_3 */

static struct msm_gemini_hw_cmds *gemini_lib_hw_cmd_malloc_and_copy (uint16_t
								     size,
								     struct
								     msm_gemini_hw_cmd
								     *p_hw_cmd)
{
	struct msm_gemini_hw_cmds *p_hw_cmds;

	p_hw_cmds =
		malloc (sizeof (struct msm_gemini_hw_cmds) -
			sizeof (struct msm_gemini_hw_cmd) + size);
	if (p_hw_cmds) {
		p_hw_cmds->m = size / sizeof (struct msm_gemini_hw_cmd);
		if (p_hw_cmd)
			memcpy (p_hw_cmds->hw_cmd, p_hw_cmd, size);
	}
	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_get_version = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	MSM_GEMINI_HW_CMD_TYPE_READ, 1, HWIO_JPEG_HW_VERSION_ADDR,
		HWIO_JPEG_HW_VERSION_RMSK, {0},
};

void gemini_lib_hw_get_version (struct msm_gemini_hw_cmd *p_hw_cmd)
{
	memcpy (p_hw_cmd, &hw_cmd_get_version, sizeof (hw_cmd_get_version));
	return;
}

struct msm_gemini_hw_cmd hw_cmd_stop_realtime[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_REALTIME_CMD_ADDR,
	 HWIO_JPEG_REALTIME_CMD_RMSK, {JPEG_REALTIME_CMD_STOP_IM}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_BUS_CMD_ADDR,
	 HWIO_JPEG_BUS_CMD_RMSK, {JPEG_BUS_CMD_HALT_REQ}},
	{MSM_GEMINI_HW_CMD_TYPE_UWAIT, 0xFFF,
	 HWIO_JPEG_BUS_STATUS_ADDR,
	 HWIO_JPEG_BUS_STATUS_BUS_IDLE_BMSK |
	 HWIO_JPEG_BUS_STATUS_HALT_ACK_BMSK,
	 {HWIO_JPEG_BUS_STATUS_BUS_IDLE_BMSK |
	  HWIO_JPEG_BUS_STATUS_HALT_ACK_BMSK}},
};

struct msm_gemini_hw_cmds *gemini_lib_hw_stop_realtime (int nicely)
{
	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof
						   (hw_cmd_stop_realtime),
						   hw_cmd_stop_realtime);
	if (!p_hw_cmds) {
		return NULL;
	}
	p_hw_cmd = &p_hw_cmds->hw_cmd[0];

	if (nicely) {
		p_hw_cmd->data = JPEG_REALTIME_CMD_STOP_FB;
	} else {
		/*
		 * by default it is set as immediately stop.
		 */
	}
	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_stop_offline[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_BUS_CMD_ADDR,
	 HWIO_JPEG_BUS_CMD_RMSK, {JPEG_BUS_CMD_HALT_REQ}},
	{MSM_GEMINI_HW_CMD_TYPE_UWAIT, 0xFFF,
	 HWIO_JPEG_BUS_STATUS_ADDR,
	 HWIO_JPEG_BUS_STATUS_BUS_IDLE_BMSK |
	 HWIO_JPEG_BUS_STATUS_HALT_ACK_BMSK,
	 {HWIO_JPEG_BUS_STATUS_BUS_IDLE_BMSK |
	  HWIO_JPEG_BUS_STATUS_HALT_ACK_BMSK}},
};

struct msm_gemini_hw_cmds *gemini_lib_hw_stop_offline (void)
{
	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_stop_offline),
						   hw_cmd_stop_offline);
	if (!p_hw_cmds) {
		return NULL;
	}

	return p_hw_cmds;
}

struct msm_gemini_hw_cmds *gemini_lib_hw_stop (gemini_cmd_operation_cfg *
					       p_op_cfg, int nicely)
{
	struct msm_gemini_hw_cmds *p_hw_cmds;

	if (p_op_cfg->useMode == MSM_GEMINI_MODE_REALTIME_ENCODE) {
		p_hw_cmds = gemini_lib_hw_stop_realtime (nicely);
	} else {
		p_hw_cmds = gemini_lib_hw_stop_offline ();
	}
	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_pipeline_cfg[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_CFG_ADDR,
	 HWIO_JPEG_CFG_RMSK, {0}},
};

struct msm_gemini_hw_cmds
	*gemini_lib_hw_pipeline_cfg (gemini_encode_pipeline_cfg_type * pIn)
{
	uint32_t nRegVal = JPEG_CFG_DEFAULT;
	GEMINI_OPERATION_MODE nUseMode;

	if (pIn->nUseMode == MSM_GEMINI_MODE_REALTIME_ENCODE) {
		nUseMode = GEMINI_REALTIME_ENCODE;
	} else {
		nUseMode = GEMINI_OFFLINE_ENCODE;
	}

	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_pipeline_cfg),
						   hw_cmd_pipeline_cfg);
	if (!p_hw_cmds) {
		return NULL;
	}
	p_hw_cmd = &p_hw_cmds->hw_cmd[0];

	GMN_DBG ("pIn->nInputFormat = %d, HWIO_JPEG_CFG_JPEG_FORMAT_SHFT = %d, HWIO_JPEG_CFG_JPEG_FORMAT_BMSK = %d\n", pIn->nInputFormat, HWIO_JPEG_CFG_JPEG_FORMAT_SHFT, HWIO_JPEG_CFG_JPEG_FORMAT_BMSK);
	nRegVal |=
		(((pIn->
		   nVFEEnable << HWIO_JPEG_CFG_VFE_INTERFACE_ENABLE_SHFT) &
		  HWIO_JPEG_CFG_VFE_INTERFACE_ENABLE_BMSK) | ((nUseMode <<
							       HWIO_JPEG_CFG_MODE_SHFT)
							      &
							      HWIO_JPEG_CFG_MODE_BMSK)
		 | ((pIn->nInputFormat << HWIO_JPEG_CFG_JPEG_FORMAT_SHFT) &
		    HWIO_JPEG_CFG_JPEG_FORMAT_BMSK) | ((pIn->
							nWEInputSel <<
							HWIO_JPEG_CFG_WE_INPUT_SEL_SHFT)
						       &
						       HWIO_JPEG_CFG_WE_INPUT_SEL_BMSK)
		 | ((pIn->nJPEGInputSel << HWIO_JPEG_CFG_JPEG_INPUT_SEL_SHFT) &
		    HWIO_JPEG_CFG_JPEG_INPUT_SEL_BMSK) | ((pIn->
							   nFEInputSel <<
							   HWIO_JPEG_CFG_FE_INPUT_SEL_SHFT)
							  &
							  HWIO_JPEG_CFG_FE_INPUT_SEL_BMSK)
		 | ((pIn->nFSCEnable << HWIO_JPEG_CFG_FSC_ENABLE_SHFT) &
		    HWIO_JPEG_CFG_FSC_ENABLE_BMSK)
		 | ((pIn->nImemFifoModeDisable <<
		   HWIO_JPEG_CFG_IMEM_MODE_DISABLE_SHFT) &
		   HWIO_JPEG_CFG_IMEM_MODE_BMSK));

        GMN_DBG("%s: nImemFifoModeDisable=%d nRegVal=%X\n", __func__, pIn->nImemFifoModeDisable, nRegVal);

	if (GEMINI_OFFLINE_ENCODE == nUseMode) {
          nRegVal = (nRegVal & 0xFFFFFBFF);
	}
	p_hw_cmd->data = nRegVal;
	ALOGE("%s:%d] Gemini Cfg 0x%x", __func__, __LINE__, p_hw_cmd->data);

	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_start[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_IRQ_MASK_ADDR,
	 HWIO_JPEG_IRQ_MASK_RMSK, {JPEG_IRQ_ALLSOURCES_ENABLE}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_WE_CMD_ADDR,
	 HWIO_JPEG_WE_CMD_RMSK, {JPEG_WE_CMD_BUFFERRELOAD}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_CMD_ADDR,
	 HWIO_JPEG_REALTIME_CMD_RMSK, {JPEG_OFFLINE_CMD_START}},
};

struct msm_gemini_hw_cmds *gemini_lib_hw_start (gemini_cmd_operation_cfg *
						p_op_cfg)
{
	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_start),
						   hw_cmd_start);
	if (!p_hw_cmds) {
		return NULL;
	}
	p_hw_cmd = &p_hw_cmds->hw_cmd[2];

	if (p_op_cfg->useMode == MSM_GEMINI_MODE_REALTIME_ENCODE) {
		p_hw_cmd->offset = HWIO_JPEG_REALTIME_CMD_ADDR;
		p_hw_cmd->data = JPEG_REALTIME_CMD_START;
	} else {
		/*
		 * preset as default mode
		 */
	}

	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_fe_cfg[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_CFG_ADDR,
	 HWIO_JPEG_FE_CFG_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_FRAME_CFG_ADDR,
	 HWIO_JPEG_FE_FRAME_CFG_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_OUTPUT_CFG_ADDR,
	 HWIO_JPEG_FE_OUTPUT_CFG_RMSK, {0}},

};

struct msm_gemini_hw_cmds *gemini_lib_hw_fe_cfg (gemini_cmd_input_cfg *
						 p_input_cfg)
{
	uint32_t reg_val = 0;
	uint32_t n_period = 0;
	uint32_t n_block_pattern = 0;

	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_fe_cfg),
						   hw_cmd_fe_cfg);
	if (!p_hw_cmds) {
		return NULL;
	}
	p_hw_cmd = &p_hw_cmds->hw_cmd[0];

	/*
	 * configure JPEG_FE_CFG with CbCr order, burst length and byte ordering
	 */
	/*
	 * note: no need to do read-modify-write 'cause other fields not used for Halcyon
	 */
	reg_val =
		(((p_input_cfg->
		   input_cbcr_order << HWIO_JPEG_FE_CFG_CBCR_ORDER_SHFT) &
		  HWIO_JPEG_FE_CFG_CBCR_ORDER_BMSK) | ((p_input_cfg->
							fe_burst_length <<
							HWIO_JPEG_FE_CFG_FE_BURST_LENGTH_SHFT)
						       &
						       HWIO_JPEG_FE_CFG_FE_BURST_LENGTH_BMSK)
		 |
		 ((p_input_cfg->
		   byte_ordering << HWIO_JPEG_FE_CFG_BYTE_ORDERING_SHFT) &
		  HWIO_JPEG_FE_CFG_BYTE_ORDERING_BMSK));
	p_hw_cmd->data = reg_val;
	p_hw_cmd++;

	/*
	 * configure JPEG_FE_FRAME_CFG with frame height/width MCUs
	 */
	GMN_DBG ("p_input_cfg->frame_height_mcus %d, p_input_cfg->frame_width_mcus %d\n", p_input_cfg->frame_height_mcus, p_input_cfg->frame_width_mcus);
	reg_val =
		((((p_input_cfg->frame_height_mcus -
		    1) << HWIO_JPEG_FE_FRAME_CFG_FRAME_HEIGHT_MCUS_SHFT) &
		  HWIO_JPEG_FE_FRAME_CFG_FRAME_HEIGHT_MCUS_BMSK) |
		 (((p_input_cfg->frame_width_mcus -
		    1) << HWIO_JPEG_FE_FRAME_CFG_FRAME_WIDTH_MCUS_SHFT) &
		  HWIO_JPEG_FE_FRAME_CFG_FRAME_WIDTH_MCUS_BMSK));
	p_hw_cmd->data = reg_val;
	p_hw_cmd++;

	/*
	 * configure JPEG_FE_OUTPUT_CFG
	 */
	if ((p_input_cfg->inputFormat == GEMINI_INPUT_H2V1) ||
	    (p_input_cfg->inputFormat == GEMINI_INPUT_H1V2)) {
		n_period = 2;
		n_block_pattern = 1;
	} else {
		/*
		 * inputFormat GEMINI_INPUT_H2V2
		 */
		n_period = 3;
		n_block_pattern = 3;
	}
	reg_val =
		((((n_period -
		    1) << HWIO_JPEG_FE_OUTPUT_CFG_PERIOD_SHFT) &
		  HWIO_JPEG_FE_OUTPUT_CFG_PERIOD_BMSK) | ((n_block_pattern <<
							   HWIO_JPEG_FE_OUTPUT_CFG_BLOCK_PATTERN_SHFT)
							  &
							  HWIO_JPEG_FE_OUTPUT_CFG_BLOCK_PATTERN_BMSK));
	p_hw_cmd->data = reg_val;

	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_op_cfg[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_FRAME_ROTATION_CFG_ADDR,
	 HWIO_JPEG_FE_FRAME_ROTATION_CFG_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1,
	 HWIO_JPEG_FE_Y_FRAME_ROTATION_START_ADDR,
	 HWIO_JPEG_FE_Y_FRAME_ROTATION_START_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1,
	 HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_ADDR,
	 HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_ADDR,
	 HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1,
	 HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_ADDR,
	 HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_BLOCK_ROTATION_CFG_ADDR,
	 HWIO_JPEG_FE_BLOCK_ROTATION_CFG_RMSK, {0x108}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_BURST_MASK_ADDR,
	 HWIO_JPEG_FE_BURST_MASK_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_Y_WRITE_MASK_0_ADDR,
	 HWIO_JPEG_FE_Y_WRITE_MASK_0_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_Y_WRITE_MASK_1_ADDR,
	 HWIO_JPEG_FE_Y_WRITE_MASK_1_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_Y_WRITE_MASK_2_ADDR,
	 HWIO_JPEG_FE_Y_WRITE_MASK_2_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_Y_WRITE_MASK_3_ADDR,
	 HWIO_JPEG_FE_Y_WRITE_MASK_3_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_CBCR_WRITE_MASK_0_ADDR,
	 HWIO_JPEG_FE_CBCR_WRITE_MASK_0_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_CBCR_WRITE_MASK_1_ADDR,
	 HWIO_JPEG_FE_CBCR_WRITE_MASK_1_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_CBCR_WRITE_MASK_2_ADDR,
	 HWIO_JPEG_FE_CBCR_WRITE_MASK_2_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_FE_CBCR_WRITE_MASK_3_ADDR,
	 HWIO_JPEG_FE_CBCR_WRITE_MASK_3_RMSK, {0}},
};

static uint8_t getburstmask_table_index(GEMINI_INPUT_FORMAT eInputFormat)
{
  switch(eInputFormat) {
  case GEMINI_INPUT_H2V1:
    return 0;
  case GEMINI_INPUT_H1V2:
    return 2;
  case GEMINI_INPUT_H2V2:
  default:
    return 1;
  }
  return 1;
}
struct msm_gemini_hw_cmds *gemini_lib_hw_op_cfg (gemini_cmd_operation_cfg *
						 p_op_cfg,
						 gemini_fe_operation_cfg_type *
						 p_op_cfg_type)
{
	/*
	 * HAL_gemini_fe_operation_cfg();
	 */
	uint32_t nRegVal = 0;
	uint32_t nYFrameRotStart = 0;
	uint32_t nCbcrFrameRotStart = 0;
	uint32_t nYFrameJumpOffset = 0;
	uint32_t nCbcrFrameJumpOffset = 0;
	uint32_t nYHorTmp = 0;
	uint32_t nYVerTmp = 0;
	uint32_t nCbcrHorTmp = 2;	/*  ref page 16 of Gemini SWI */
	uint32_t nCbcrVerTmp = 1;	/*  ref page 16 of Gemini SWI */
	uint32_t nBlockRotation = 0;
	uint32_t nPixelStartOffset = 0;
	uint32_t nPixelHIncrement = 0;
	uint32_t nPixelVIncrement = 0;
	uint32_t nBurstMask = 0;
	uint32_t nYWriteMask0 = 0;
	uint32_t nYWriteMask1 = 0;
	uint32_t nYWriteMask2 = 0;
	uint32_t nYWriteMask3 = 0;
	uint32_t nCbcrWriteMask0 = 0;
	uint32_t nCbcrWriteMask1 = 0;
	uint32_t nCbcrWriteMask2 = 0;
	uint32_t nCbcrWriteMask3 = 0;
	uint8_t nBurstIndex = 0;
	uint8_t nIndex = 0;

	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_op_cfg),
						   hw_cmd_op_cfg);
	if (!p_hw_cmds) {
		return NULL;
	}
	p_hw_cmd = &p_hw_cmds->hw_cmd[0];

	/*
	 * config JPEG_FE_FRAME_ROTATION_CFG with rotation degree which
	 */
	/*
	 * specifies how order at which the fetch engine will read data
	 */
	/*
	 * from memory
	 */
	nRegVal =
		(((uint32_t) p_op_cfg->
		  rotationDegree <<
		  HWIO_JPEG_FE_FRAME_ROTATION_CFG_FE_FRAME_ROTATION_SHFT) &
		 HWIO_JPEG_FE_FRAME_ROTATION_CFG_FE_FRAME_ROTATION_BMSK);
	p_hw_cmd->data = nRegVal;
	p_hw_cmd++;

	if (MSM_GEMINI_MODE_REALTIME_ENCODE != p_op_cfg->useMode) {
		if ((GEMINI_INPUT_H2V1 == p_op_cfg_type->eInputFormat) ||
		    (GEMINI_INPUT_H2V2 == p_op_cfg_type->eInputFormat)) {
			nYHorTmp = 2;	/*  ref page 16 of Gemini SWI */
		} else {
			nYHorTmp = 1;	/*  reg page 16 of Gemini SWI */
		}

		if ((GEMINI_INPUT_H2V2 == p_op_cfg_type->eInputFormat) ||
		    (GEMINI_INPUT_H1V2 == p_op_cfg_type->eInputFormat)) {
			nYVerTmp = 2;	/*  ref page 16 of Gemini SWI */
		} else {
			nYVerTmp = 1;	/*  ref page 16  of Gemini SWI */
		}

		if (GEMINI_ROTATE_90 == p_op_cfg->rotationDegree) {
			nYFrameRotStart =
				(p_op_cfg_type->nFrameWidthMCUs -
				 1) * nYHorTmp * 8;
			nCbcrFrameRotStart =
				(p_op_cfg_type->nFrameWidthMCUs -
				 1) * nCbcrHorTmp * 8;
			nYFrameJumpOffset =
				8 * nYHorTmp *
				((p_op_cfg_type->nFrameHeightMCUs -
				  1) * 8 * nYVerTmp *
				 (p_op_cfg_type->nFrameWidthMCUs) + 1);
			nCbcrFrameJumpOffset =
				8 * nCbcrHorTmp *
				((p_op_cfg_type->nFrameHeightMCUs -
				  1) * 8 * nCbcrVerTmp *
				 (p_op_cfg_type->nFrameWidthMCUs) + 1);
			nBlockRotation = 1;	/*  ref page 18 of Gemini SWI */
			nPixelStartOffset = 7;	/*  ref page 18 of Gemini SWI */
			nPixelHIncrement = 8;	/*  ref page 18 of Gemini SWI */
			nPixelVIncrement = 0x1f;	/*  two complement presentation of -1 */
		} else if (GEMINI_ROTATE_180 == p_op_cfg->rotationDegree) {
			nYFrameRotStart =
				((p_op_cfg_type->nFrameWidthMCUs -
				  1) * nYHorTmp * 8) +
				(((p_op_cfg_type->nFrameHeightMCUs -
				   1) * nYVerTmp * 8) *
				 p_op_cfg_type->nFrameWidthMCUs * nYHorTmp * 8);
			nCbcrFrameRotStart =
				((p_op_cfg_type->nFrameWidthMCUs -
				  1) * nCbcrHorTmp * 8) +
				(((p_op_cfg_type->nFrameHeightMCUs -
				   1) * nCbcrVerTmp * 8) *
				 p_op_cfg_type->nFrameWidthMCUs * nCbcrHorTmp *
				 8);
			nYFrameJumpOffset =
				8 * nYHorTmp * (p_op_cfg_type->nFrameWidthMCUs *
						(8 * nYVerTmp - 1) + 1);
			nCbcrFrameJumpOffset =
				8 * nCbcrHorTmp *
				(p_op_cfg_type->nFrameWidthMCUs *
				 (8 * nCbcrVerTmp - 1) + 1);
			nBlockRotation = 2;	/*  ref page 18 of Gemini SWI */
			nPixelStartOffset = 63;	/*  ref page 18 of Gemini SWI */
			nPixelHIncrement = 0x1f;	/*  two complement presentation of -1 */
			nPixelVIncrement = 0x18;	/*  two complement presentation of -8 */
		} else if (GEMINI_ROTATE_270 == p_op_cfg->rotationDegree) {
			nYFrameRotStart =
				((p_op_cfg_type->nFrameHeightMCUs -
				  1) * nYVerTmp * 8) *
				p_op_cfg_type->nFrameWidthMCUs * nYHorTmp * 8;
			nCbcrFrameRotStart =
				((p_op_cfg_type->nFrameHeightMCUs -
				  1) * nCbcrVerTmp * 8) *
				p_op_cfg_type->nFrameWidthMCUs * nCbcrHorTmp *
				8;
			nYFrameJumpOffset =
				8 * nYHorTmp *
				((p_op_cfg_type->nFrameHeightMCUs -
				  1) * 8 * nYVerTmp *
				 p_op_cfg_type->nFrameWidthMCUs + 1);
			nCbcrFrameJumpOffset =
				8 * nCbcrHorTmp *
				((p_op_cfg_type->nFrameHeightMCUs -
				  1) * 8 * nCbcrVerTmp *
				 p_op_cfg_type->nFrameWidthMCUs + 1);
			nBlockRotation = 3;	/*  ref page 18 of Gemini SWI */
			nPixelStartOffset = 56;	/*  ref page 18 of Gemini SWI */
			nPixelHIncrement = 0x18;	/*  two complement presentation of -8 */
			nPixelVIncrement = 1;	/*  ref page 18 of Gemini SWI */
		}
		/*
		 * no rotation
		 */
		else {
			nYFrameRotStart = 0;
			nCbcrFrameRotStart = 0;
			nYFrameJumpOffset =
				8 * nYHorTmp * (p_op_cfg_type->nFrameWidthMCUs *
						(8 * nYVerTmp - 1) + 1);
			nCbcrFrameJumpOffset =
				8 * nCbcrHorTmp *
				(p_op_cfg_type->nFrameWidthMCUs *
				 (8 * nCbcrVerTmp - 1) + 1);
			nBlockRotation = 0;	/*  ref page 18 of Gemini SWI */
			nPixelStartOffset = 0;	/*  ref page 18 of Gemini SWI */
			nPixelHIncrement = 1;	/*  ref page 18 of Gemini SWI */
			nPixelVIncrement = 8;	/*  ref page 18 of Gemini SWI */
		}

		nRegVal =
			((nYFrameRotStart <<
			  HWIO_JPEG_FE_Y_FRAME_ROTATION_START_FE_START_OFFSET_SHFT)
			 &
			 HWIO_JPEG_FE_Y_FRAME_ROTATION_START_FE_START_OFFSET_BMSK);
		p_hw_cmd->data = nRegVal;
		p_hw_cmd++;

		nRegVal =
			((nCbcrFrameRotStart <<
			  HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_FE_START_OFFSET_SHFT)
			 &
			 HWIO_JPEG_FE_CBCR_FRAME_ROTATION_START_FE_START_OFFSET_BMSK);
		p_hw_cmd->data = nRegVal;
		p_hw_cmd++;

		nRegVal =
			((nYFrameJumpOffset <<
			  HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_Y_ROW_JUMP_OFFSET_SHFT)
			 &
			 HWIO_JPEG_FE_Y_FRAME_JUMP_OFFSET_Y_ROW_JUMP_OFFSET_BMSK);
		p_hw_cmd->data = nRegVal;
		p_hw_cmd++;

		nRegVal =
			((nCbcrFrameJumpOffset <<
			  HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_CBCR_ROW_JUMP_OFFSET_SHFT)
			 &
			 HWIO_JPEG_FE_CBCR_FRAME_JUMP_OFFSET_CBCR_ROW_JUMP_OFFSET_BMSK);
		p_hw_cmd->data = nRegVal;
		p_hw_cmd++;

		nRegVal =
			(((nBlockRotation <<
			   HWIO_JPEG_FE_BLOCK_ROTATION_CFG_BLOCK_ROTATION_SHFT)
			  & HWIO_JPEG_FE_BLOCK_ROTATION_CFG_BLOCK_ROTATION_BMSK)
			 |
			 ((nPixelStartOffset <<
			   HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_START_OFFSET_SHFT)
			  &
			  HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_START_OFFSET_BMSK)
			 |
			 ((nPixelHIncrement <<
			   HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_H_INCREMENT_SHFT)
			  &
			  HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_H_INCREMENT_BMSK)
			 |
			 ((nPixelVIncrement <<
			   HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_V_INCREMENT_SHFT)
			  &
			  HWIO_JPEG_FE_BLOCK_ROTATION_CFG_PIXEL_V_INCREMENT_BMSK));
		p_hw_cmd->data = nRegVal;
		p_hw_cmd++;
	} else {
		/*
		 * zeroes out FE rotation related registers if no rotation is specified
		 */
		p_hw_cmd += 5;
	}

	/*
	 * config FE burst mask and write mask registers for real-time
	 */
	if (MSM_GEMINI_MODE_REALTIME_ENCODE == p_op_cfg->useMode) {
		if (IGEMINI_FE_BURST4 == (p_op_cfg_type->nReadBurstLength - 1)) {
			nBurstIndex = IGEMINI_FE_BURST4;
		} else {
			nBurstIndex = IGEMINI_FE_BURST8;
		}

		nBurstMask =
			GEMINI_FE_RL_BURSTMASK[IGEMINI_FE_BURSTMASK]
			[nBurstIndex];
		nYWriteMask0 =
			GEMINI_FE_RL_BURSTMASK[IGEMINI_FE_YWRITEMASK0]
			[nBurstIndex];
		nYWriteMask1 =
			GEMINI_FE_RL_BURSTMASK[IGEMINI_FE_YWRITEMASK1]
			[nBurstIndex];
		nYWriteMask2 =
			GEMINI_FE_RL_BURSTMASK[IGEMINI_FE_YWRITEMASK2]
			[nBurstIndex];
		nYWriteMask3 =
			GEMINI_FE_RL_BURSTMASK[IGEMINI_FE_YWRITEMASK3]
			[nBurstIndex];
		nCbcrWriteMask0 =
			GEMINI_FE_RL_BURSTMASK[IGEMINI_FE_CBCRWRITEMASK0]
			[nBurstIndex];
		nCbcrWriteMask1 =
			GEMINI_FE_RL_BURSTMASK[IGEMINI_FE_CBCRWRITEMASK1]
			[nBurstIndex];
		nCbcrWriteMask2 =
			GEMINI_FE_RL_BURSTMASK[IGEMINI_FE_CBCRWRITEMASK2]
			[nBurstIndex];
		nCbcrWriteMask3 =
			GEMINI_FE_RL_BURSTMASK[IGEMINI_FE_CBCRWRITEMASK3]
			[nBurstIndex];
	}
	/*
	 * config FE burst mask and write mask registers for offline
	 */
	else {
		nIndex = getburstmask_table_index(p_op_cfg_type->eInputFormat);
		nBurstMask =
			GEMINI_FE_OL_BURSTMASK_64BB[IGEMINI_FE_BURSTMASK]
			[nIndex];
		nYWriteMask0 =
			GEMINI_FE_OL_BURSTMASK_64BB[IGEMINI_FE_YWRITEMASK0]
			[nIndex];
		nYWriteMask1 =
			GEMINI_FE_OL_BURSTMASK_64BB[IGEMINI_FE_YWRITEMASK1]
			[nIndex];
		nYWriteMask2 =
			GEMINI_FE_OL_BURSTMASK_64BB[IGEMINI_FE_YWRITEMASK2]
			[nIndex];
		nYWriteMask3 =
			GEMINI_FE_OL_BURSTMASK_64BB[IGEMINI_FE_YWRITEMASK3]
			[nIndex];
		nCbcrWriteMask0 =
			GEMINI_FE_OL_BURSTMASK_64BB[IGEMINI_FE_CBCRWRITEMASK0]
			[nIndex];
		nCbcrWriteMask1 =
			GEMINI_FE_OL_BURSTMASK_64BB[IGEMINI_FE_CBCRWRITEMASK1]
			[nIndex];
		nCbcrWriteMask2 =
			GEMINI_FE_OL_BURSTMASK_64BB[IGEMINI_FE_CBCRWRITEMASK2]
			[nIndex];
		nCbcrWriteMask3 =
			GEMINI_FE_OL_BURSTMASK_64BB[IGEMINI_FE_CBCRWRITEMASK3]
			[nIndex];
	}

	p_hw_cmd->data = nBurstMask;
	p_hw_cmd++;
	p_hw_cmd->data = nYWriteMask0;
	p_hw_cmd++;
	p_hw_cmd->data = nYWriteMask1;
	p_hw_cmd++;
	p_hw_cmd->data = nYWriteMask2;
	p_hw_cmd++;
	p_hw_cmd->data = nYWriteMask3;
	p_hw_cmd++;
	p_hw_cmd->data = nCbcrWriteMask0;
	p_hw_cmd++;
	p_hw_cmd->data = nCbcrWriteMask1;
	p_hw_cmd++;
	p_hw_cmd->data = nCbcrWriteMask2;
	p_hw_cmd++;
	p_hw_cmd->data = nCbcrWriteMask3;

	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_we_cfg[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_WE_CFG_ADDR,
	 HWIO_JPEG_WE_CFG_RMSK, {0}},
};

struct msm_gemini_hw_cmds *gemini_lib_hw_we_cfg (gemini_cmd_output_cfg * pIn)
{
	uint32_t nRegVal = 0;

	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_we_cfg),
						   hw_cmd_we_cfg);
	if (!p_hw_cmds) {
		return NULL;
	}
	p_hw_cmd = &p_hw_cmds->hw_cmd[0];

	nRegVal =
		(((pIn->
		   we_burst_length << HWIO_JPEG_WE_CFG_WE_BURST_LENGTH_SHFT) &
		  HWIO_JPEG_WE_CFG_WE_BURST_LENGTH_BMSK) | ((pIn->
							     byte_ordering <<
							     HWIO_JPEG_WE_CFG_BYTE_ORDERING_SHFT)
							    &
							    HWIO_JPEG_WE_CFG_BYTE_ORDERING_BMSK));
	p_hw_cmd->data = nRegVal;

	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_restart_marker_set[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE_OR, 1, HWIO_JPEG_ENCODE_CFG_ADDR,
	 HWIO_JPEG_ENCODE_CFG_RST_MARKER_PERIOD_BMSK, {0}},
};

struct msm_gemini_hw_cmds *gemini_lib_hw_restart_marker_set (uint32_t
							     restartInterval)
{
	uint32_t nEncodeCfg = 0;

	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof
						   (hw_cmd_restart_marker_set),
						   hw_cmd_restart_marker_set);
	if (!p_hw_cmds) {
		return NULL;
	}
	p_hw_cmd = &p_hw_cmds->hw_cmd[0];

	/*
	 * write updated restart interval value
	 */
	nEncodeCfg |=
		((restartInterval & HWIO_JPEG_ENCODE_CFG_RST_MARKER_PERIOD_BMSK)
		 >> HWIO_JPEG_ENCODE_CFG_RST_MARKER_PERIOD_SHFT);

	p_hw_cmd->data = nEncodeCfg;

	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_set_huffman_tables[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE_OR, 1, HWIO_JPEG_ENCODE_CFG_ADDR,
	 HWIO_JPEG_ENCODE_CFG_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_DMI_CFG_ADDR,
	 HWIO_JPEG_DMI_CFG_RMSK, {JPEG_DMI_CFG_HUFFMAN}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_DMI_ADDR_ADDR,
	 HWIO_JPEG_DMI_ADDR_RMSK, {JPEG_DMI_ADDR_START}},
	/*
	 * LUMA DC codewords  : GEMINI_NUM_HUFFMANDC_ENTRIES = 12  * 2
	 */
	/*
	 * CHROMA DC codewords: GEMINI_NUM_HUFFMANDC_ENTRIES = 12  * 2
	 */
	/*
	 * LUMA AC codewords  : GEMINI_NUM_HUFFMANAC_ENTRIES = 176 * 2
	 */
	/*
	 * CHROMA AC codewords: GEMINI_NUM_HUFFMANAC_ENTRIES = 176 * 2
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_DMI_CFG_ADDR,
	 HWIO_JPEG_DMI_CFG_RMSK, {JPEG_DMI_CFG_DISABLE}},
};

void gemini_lib_hw_create_huffman_table (uint8_t * pHuffBits,
					 uint8_t * pHuffVal,
					 gemini_huff_lookup_table_type *
					 pHuffLookUpTable,
					 uint8_t nComponentIndex)
{
	uint32_t maxNumValues, i, k;
	uint16_t codeLength, codeWord, symbolValue, lookUpTableSize;
	gemini_huff_lookup_table_type tempHuffLookUpTable[256];

	/*
	 * Set the maximum number of values for this component
	 */
	if (GEMINI_DC_COMPONENT_INDEX == nComponentIndex) {
		maxNumValues = 12;
		lookUpTableSize = 12;
	} else {		/*  GEMINI_AC_COMPONENT_INDEX == nComponentIndex */

		maxNumValues = 162;
		lookUpTableSize = 256;
	}

	/*
	 * Next populate the code lengths and code words in the local table
	 */
	codeLength = 1;
	codeWord = 0;
	k = 0;

	for (i = 0; i < 16; i++) {
		uint8_t bits = pHuffBits[i];
		while (bits--) {
			tempHuffLookUpTable[k].codeLength = codeLength;
			tempHuffLookUpTable[k].codeWord = codeWord;

			codeWord = codeWord + 1;
			k = k + 1;
		}
		codeLength = codeLength + 1;
		codeWord = codeWord << 1;
	}

	/*
	 * Clear the entries in the given look-up table
	 */
	for (i = 0; i < lookUpTableSize; i++) {
		(pHuffLookUpTable + i)->codeLength = 0;
		(pHuffLookUpTable + i)->codeWord = 0;
	}

	/*
	 * Now copy the entries from the local look-up table to the given look-up
	 */
	/*
	 * table using symbol values as index
	 */
	for (i = 0; i < maxNumValues; i++) {
		if (GEMINI_DC_COMPONENT_INDEX == nComponentIndex) {
			symbolValue = pHuffVal[i];
		} else {
			symbolValue =
				((pHuffVal[i] & 0xf) << 4) | (pHuffVal[i] >> 4);
		}

		(pHuffLookUpTable + symbolValue)->codeLength =
			tempHuffLookUpTable[i].codeLength;
		(pHuffLookUpTable + symbolValue)->codeWord =
			tempHuffLookUpTable[i].codeWord;
	}
	return;
}

void gemini_lib_hw_create_huffman_tables (gemini_cmd_jpeg_encode_cfg *
					  p_encode_cfg,
					  gemini_huff_lookup_table_type *
					  sLumaDCHuffmanLUT,
					  gemini_huff_lookup_table_type *
					  sChromaDCHuffmanLUT,
					  gemini_huff_lookup_table_type *
					  sLumaACHuffmanLUT,
					  gemini_huff_lookup_table_type *
					  sChromaACHuffmanLUT)
{
	gemini_huffmanDcTable *pHuffLumaDCTable;
	gemini_huffmanDcTable *pHuffChromaDCTable;
	gemini_huffmanAcTable *pHuffLumaACTable;
	gemini_huffmanAcTable *pHuffChromaACTable;

	/*
	 * derive Huffman Luma DC LUT code length and code word from bits and vals
	 */
	pHuffLumaDCTable = p_encode_cfg->huffmanTblYDcPtr;
	gemini_lib_hw_create_huffman_table (&pHuffLumaDCTable->huffcount[0],
					    &pHuffLumaDCTable->huffval[0],
					    &sLumaDCHuffmanLUT[0],
					    GEMINI_DC_COMPONENT_INDEX);

	/*
	 * derive Huffman Chroma DC LUT code length and code word from bits and vals
	 */
	pHuffChromaDCTable = p_encode_cfg->huffmanTblCbcrDcPtr;
	gemini_lib_hw_create_huffman_table (&pHuffChromaDCTable->huffcount[0],
					    &pHuffChromaDCTable->huffval[0],
					    &sChromaDCHuffmanLUT[0],
					    GEMINI_DC_COMPONENT_INDEX);

	/*
	 * derive Huffman Luma AC LUT code length and code word from bits and vals
	 */
	pHuffLumaACTable = p_encode_cfg->huffmanTblYAcPtr;
	gemini_lib_hw_create_huffman_table (&pHuffLumaACTable->huffcount[0],
					    &pHuffLumaACTable->huffval[0],
					    &sLumaACHuffmanLUT[0],
					    GEMINI_AC_COMPONENT_INDEX);

	/*
	 * derive Huffman Chroma AC LUT code length and code word from bits and vals
	 */
	pHuffChromaACTable = p_encode_cfg->huffmanTblCbcrAcPtr;
	gemini_lib_hw_create_huffman_table (&pHuffChromaACTable->huffcount[0],
					    &pHuffChromaACTable->huffval[0],
					    &sChromaACHuffmanLUT[0],
					    GEMINI_AC_COMPONENT_INDEX);
	return;
}

struct msm_gemini_hw_cmds
	*gemini_lib_hw_set_huffman_tables (gemini_huff_lookup_table_type *
					   pLumaDC,
					   gemini_huff_lookup_table_type *
					   pChromaDC,
					   gemini_huff_lookup_table_type *
					   pLumaAC,
					   gemini_huff_lookup_table_type *
					   pChromaAC)
{
	uint8_t status = 0;
	uint32_t nEncodeCfg = 0;
	uint32_t nHuffmanSel = 1;
	uint32_t nAddress = 0;
	uint32_t nValue = 0;
	uint8_t nIndex = 0;

	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof
						   (hw_cmd_set_huffman_tables) +
						   sizeof (struct
							   msm_gemini_hw_cmd) *
						   (GEMINI_NUM_HUFFMANDC_ENTRIES
						    +
						    GEMINI_NUM_HUFFMANAC_ENTRIES)
						   * 4, NULL);
	if (!p_hw_cmds) {
		return NULL;
	}
	memcpy (&p_hw_cmds->hw_cmd[0], hw_cmd_set_huffman_tables,
		sizeof (hw_cmd_set_huffman_tables));
	p_hw_cmd = &p_hw_cmds->hw_cmd[0];

	/*
	 * protect null pointer accesses for Huffman LUT
	 */
	/*
	 * since these pointers generated from DAL layer
	 */
	if ((NULL == pLumaDC) || (NULL == pChromaDC) || (NULL == pLumaAC) ||
	    (NULL == pChromaAC)) {
		status = -1;	/*  CAMERA_EMEMPTR; */
	} else {
		gemini_huff_lookup_table_type *pTmp = NULL;
		uint16_t nTmpCodeLength = 0;
		uint16_t nTmpCodeWord = 0;

		/*
		 * read, modify and write HUFFMAN_SEL field of JPEG_ENCODE_CFG
		 */
		nEncodeCfg = 0;
		nEncodeCfg |=
			((nHuffmanSel << HWIO_JPEG_ENCODE_CFG_HUFFMAN_SEL_SHFT)
			 & HWIO_JPEG_ENCODE_CFG_HUFFMAN_SEL_BMSK);
		p_hw_cmd->data = nEncodeCfg;
		p_hw_cmd++;

		/*
		 * select huffman LUT RAM and auto-increment JPEG_DMI_ADDR
		 */
		p_hw_cmd++;

		/*
		 * ensure first RAM address accessed is location 0
		 */
		p_hw_cmd++;

		/*
		 * load LUMA DC codewords
		 */
		pTmp = pLumaDC;
		for (nIndex = 0; nIndex < GEMINI_NUM_HUFFMANDC_ENTRIES;
		     nIndex++) {
			/*
			 * form address/data ref Gemini SWI section 2.2.7
			 */
			/*
			 * code length = Huffman code length + residual length - 1
			 */
			/*
			 * code word to be stored starting from the most significant bit
			 */
			nTmpCodeLength = pTmp->codeLength + nIndex - 1;
			nTmpCodeWord =
				pTmp->codeWord << (16 - pTmp->codeLength);

			nAddress = (nIndex * 64) + 2;
			nValue = nTmpCodeWord + (65536 * nTmpCodeLength);

			p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_WRITE;
			p_hw_cmd->n = 1;
			p_hw_cmd->offset = HWIO_JPEG_DMI_ADDR_ADDR;
			p_hw_cmd->mask = HWIO_JPEG_DMI_ADDR_RMSK;
			p_hw_cmd->data = nAddress;
			p_hw_cmd++;

			p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_WRITE;
			p_hw_cmd->n = 1;
			p_hw_cmd->offset = HWIO_JPEG_DMI_DATA_LO_ADDR;
			p_hw_cmd->mask = HWIO_JPEG_DMI_DATA_LO_RMSK;
			p_hw_cmd->data = nValue;
			p_hw_cmd++;

			/*
			 * increment pointer to the next index within LUT
			 */
			pTmp++;
		}

		/*
		 * load CHROMA DC codewords
		 */
		pTmp = pChromaDC;
		for (nIndex = 0; nIndex < GEMINI_NUM_HUFFMANDC_ENTRIES;
		     nIndex++) {
			/*
			 * form address/data ref Gemini SWI section 2.2.7
			 */
			/*
			 * code length = Huffman code length + residual length - 1
			 */
			/*
			 * code word to be stored starting from the most significant bit
			 */
			nTmpCodeLength = pTmp->codeLength + nIndex - 1;
			nTmpCodeWord =
				pTmp->codeWord << (16 - pTmp->codeLength);

			nAddress = (nIndex * 64) + 3;
			nValue = nTmpCodeWord + (65536 * nTmpCodeLength);

			p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_WRITE;
			p_hw_cmd->n = 1;
			p_hw_cmd->offset = HWIO_JPEG_DMI_ADDR_ADDR;
			p_hw_cmd->mask = HWIO_JPEG_DMI_ADDR_RMSK;
			p_hw_cmd->data = nAddress;
			p_hw_cmd++;

			p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_WRITE;
			p_hw_cmd->n = 1;
			p_hw_cmd->offset = HWIO_JPEG_DMI_DATA_LO_ADDR;
			p_hw_cmd->mask = HWIO_JPEG_DMI_DATA_LO_RMSK;
			p_hw_cmd->data = nValue;
			p_hw_cmd++;

			/*
			 * increment pointer to the next index within LUT
			 */
			pTmp++;
		}

		/*
		 * load LUMA AC codewords
		 */
		pTmp = pLumaAC;
		for (nIndex = 0; nIndex < GEMINI_NUM_HUFFMANAC_ENTRIES;
		     nIndex++) {
			/*
			 * form address/data ref Gemini SWI section 2.2.7
			 */
			/*
			 * code length = Huffman code length + residual length - 1
			 */
			/*
			 * code word to be stored starting from the most significant bit
			 */
			/*
			 * notes: nIndex >> 4 'cause category is the upper 4  bits
			 */
			nTmpCodeLength =
				(pTmp->codeLength + ((nIndex >> 4) & 0xf) -
				 1) & 0x1f;
			nTmpCodeWord =
				pTmp->codeWord << (16 - pTmp->codeLength);

			nAddress = nIndex * 4;
			nValue = nTmpCodeWord + (65536 * nTmpCodeLength);

			p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_WRITE;
			p_hw_cmd->n = 1;
			p_hw_cmd->offset = HWIO_JPEG_DMI_ADDR_ADDR;
			p_hw_cmd->mask = HWIO_JPEG_DMI_ADDR_RMSK;
			p_hw_cmd->data = nAddress;
			p_hw_cmd++;

			p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_WRITE;
			p_hw_cmd->n = 1;
			p_hw_cmd->offset = HWIO_JPEG_DMI_DATA_LO_ADDR;
			p_hw_cmd->mask = HWIO_JPEG_DMI_DATA_LO_RMSK;
			p_hw_cmd->data = nValue;
			p_hw_cmd++;

			/*
			 * increment pointer to the next index within LUT
			 */
			pTmp++;
		}

		/*
		 * load CHROMA AC codewords
		 */
		pTmp = pChromaAC;
		for (nIndex = 0; nIndex < GEMINI_NUM_HUFFMANAC_ENTRIES;
		     nIndex++) {
			/*
			 * form address/data ref Gemini SWI section 2.2.7
			 */
			/*
			 * code length = Huffman code length + residual length - 1
			 */
			/*
			 * code word to be stored starting from the most significant bit
			 */
			/*
			 * notes: nIndex >> 4 'cause category is the upper 4  bits
			 */
			nTmpCodeLength =
				(pTmp->codeLength + ((nIndex >> 4) & 0xf) -
				 1) & 0x1f;
			nTmpCodeWord =
				pTmp->codeWord << (16 - pTmp->codeLength);

			nAddress = (nIndex * 4) + 1;
			nValue = nTmpCodeWord + (65536 * nTmpCodeLength);

			p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_WRITE;
			p_hw_cmd->n = 1;
			p_hw_cmd->offset = HWIO_JPEG_DMI_ADDR_ADDR;
			p_hw_cmd->mask = HWIO_JPEG_DMI_ADDR_RMSK;
			p_hw_cmd->data = nAddress;
			p_hw_cmd++;

			p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_WRITE;
			p_hw_cmd->n = 1;
			p_hw_cmd->offset = HWIO_JPEG_DMI_DATA_LO_ADDR;
			p_hw_cmd->mask = HWIO_JPEG_DMI_DATA_LO_RMSK;
			p_hw_cmd->data = nValue;
			p_hw_cmd++;

			/*
			 * increment pointer to the next index within LUT
			 */
			pTmp++;
		}

		/*
		 * disable DMI capabilities
		 */
		p_hw_cmd->type = hw_cmd_set_huffman_tables[3].type;
		p_hw_cmd->n = hw_cmd_set_huffman_tables[3].n;
		p_hw_cmd->offset = hw_cmd_set_huffman_tables[3].offset;
		p_hw_cmd->mask = hw_cmd_set_huffman_tables[3].mask;
		p_hw_cmd->data = hw_cmd_set_huffman_tables[3].data;
	}

	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_read_quant_tables[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_DMI_ADDR_ADDR,
	 HWIO_JPEG_DMI_ADDR_RMSK, {JPEG_DMI_ADDR_START}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_DMI_CFG_ADDR,
	 HWIO_JPEG_DMI_CFG_RMSK, {JPEG_DMI_CFG_QUANTIZATION}},
	/*
	 * Y quantization     : GEMINI_NUM_QUANTIZATION_ENTRIES = 64
	 */
	/*
	 * Chroma quantization: GEMINI_NUM_QUANTIZATION_ENTRIES = 64
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_DMI_CFG_ADDR,
	 HWIO_JPEG_DMI_CFG_RMSK, {JPEG_DMI_CFG_DISABLE}},
};

struct msm_gemini_hw_cmds *gemini_lib_hw_read_quant_tables (void)
{
	uint8_t status = 0;
	uint8_t nIndex = 0;

	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof
						   (hw_cmd_read_quant_tables) +
						   sizeof (struct
							   msm_gemini_hw_cmd) *
						   GEMINI_NUM_QUANTIZATION_ENTRIES
						   * 2, NULL);
	if (!p_hw_cmds) {
		return NULL;
	}
	memcpy (&p_hw_cmds->hw_cmd[0], hw_cmd_read_quant_tables,
		sizeof (hw_cmd_read_quant_tables));
	p_hw_cmd = &p_hw_cmds->hw_cmd[2];

	/*
	 * load Y quantization values to lower portion of quantization LUT
	 */
	for (nIndex = 0; nIndex < GEMINI_NUM_QUANTIZATION_ENTRIES; nIndex++) {
		p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_READ;
		p_hw_cmd->n = 1;
		p_hw_cmd->offset = HWIO_JPEG_DMI_DATA_LO_ADDR;
		p_hw_cmd->mask = HWIO_JPEG_DMI_DATA_LO_RMSK;
		p_hw_cmd++;

	}

	/*
	 * load Chroma quantization values to upper portion of quantization LUT
	 */
	for (nIndex = 0; nIndex < GEMINI_NUM_QUANTIZATION_ENTRIES; nIndex++) {
		p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_READ;
		p_hw_cmd->n = 1;
		p_hw_cmd->offset = HWIO_JPEG_DMI_DATA_LO_ADDR;
		p_hw_cmd->mask = HWIO_JPEG_DMI_DATA_LO_RMSK;
		p_hw_cmd++;

	}

	/*
	 * disable DMI capabilities
	 */
	p_hw_cmd->type = hw_cmd_read_quant_tables[2].type;
	p_hw_cmd->n = hw_cmd_read_quant_tables[2].n;
	p_hw_cmd->offset = hw_cmd_read_quant_tables[2].offset;
	p_hw_cmd->mask = hw_cmd_read_quant_tables[2].mask;
	p_hw_cmd->data = hw_cmd_read_quant_tables[2].data;

	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_set_quant_tables[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_DMI_ADDR_ADDR,
	 HWIO_JPEG_DMI_ADDR_RMSK, {JPEG_DMI_ADDR_START}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_DMI_CFG_ADDR,
	 HWIO_JPEG_DMI_CFG_RMSK, {JPEG_DMI_CFG_QUANTIZATION}},
	/*
	 * Y quantization     : GEMINI_NUM_QUANTIZATION_ENTRIES = 64
	 */
	/*
	 * Chroma quantization: GEMINI_NUM_QUANTIZATION_ENTRIES = 64
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_DMI_CFG_ADDR,
	 HWIO_JPEG_DMI_CFG_RMSK, {JPEG_DMI_CFG_DISABLE}},
};

struct msm_gemini_hw_cmds *gemini_lib_hw_set_quant_tables (gemini_quantTable *
							   pY,
							   gemini_quantTable *
							   pChroma)
{
	uint8_t status = 0;
	uint8_t nTmpValue = 0;
	uint8_t nIndex = 0;
	uint32_t nInversedQ = 0;

	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof
						   (hw_cmd_set_quant_tables) +
						   sizeof (struct
							   msm_gemini_hw_cmd) *
						   GEMINI_NUM_QUANTIZATION_ENTRIES
						   * 2, NULL);
	if (!p_hw_cmds) {
		return NULL;
	}
	memcpy (&p_hw_cmds->hw_cmd[0], hw_cmd_set_quant_tables,
		sizeof (hw_cmd_set_quant_tables));
	p_hw_cmd = &p_hw_cmds->hw_cmd[2];

	/*
	 * load Y quantization values to lower portion of quantization LUT
	 */
	for (nIndex = 0; nIndex < GEMINI_NUM_QUANTIZATION_ENTRIES; nIndex++) {
		nTmpValue = pY->qtbl[nIndex];

		/*
		 * since 1/Q value is an unsigned Q16 value in hardware
		 */
		/*
		 * set 1/Q to max for the case of zero (ref SWI section 2.2.7)
		 */
		if (0 == nTmpValue || 1 == nTmpValue) {
			nInversedQ = JPEG_DMI_DATA_MAX;
		} else {
			/*
			 * notes: no floating point can be used
			 */
			/*
			 * notes: this equation is from hw team for 1/Q
			 */
			/*
			 * notes: nInversedQ = (uint32_t) ((1 << 16) * (1.0/nTmpValue));
			 */
			nInversedQ = (uint32_t) (0x10000 / nTmpValue);
		}
		p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_WRITE;
		p_hw_cmd->n = 1;
		p_hw_cmd->offset = HWIO_JPEG_DMI_DATA_LO_ADDR;
		p_hw_cmd->mask = HWIO_JPEG_DMI_DATA_LO_RMSK;
		p_hw_cmd->data = nInversedQ;
		p_hw_cmd++;

	}

	/*
	 * load Chroma quantization values to upper portion of quantization LUT
	 */
	for (nIndex = 0; nIndex < GEMINI_NUM_QUANTIZATION_ENTRIES; nIndex++) {
		nTmpValue = pChroma->qtbl[nIndex];

		/*
		 * since 1/Q value is an unsigned Q16 value in hardware
		 */
		/*
		 * set 1/Q to max for the case of zero (ref SWI section 2.2.7)
		 */
		if (0 == nTmpValue || 1 == nTmpValue) {
			nInversedQ = JPEG_DMI_DATA_MAX;
		} else {
			/*
			 * notes: no floating point can be used
			 */
			/*
			 * notes: this equation is from hw team for 1/Q
			 */
			/*
			 * notes: nInversedQ = (uint32_t) ((1 << 16) * (1.0/nTmpValue));
			 */
			nInversedQ = (uint32_t) (0x10000 / nTmpValue);
		}
		p_hw_cmd->type = MSM_GEMINI_HW_CMD_TYPE_WRITE;
		p_hw_cmd->n = 1;
		p_hw_cmd->offset = HWIO_JPEG_DMI_DATA_LO_ADDR;
		p_hw_cmd->mask = HWIO_JPEG_DMI_DATA_LO_RMSK;
		p_hw_cmd->data = nInversedQ;
		p_hw_cmd++;

	}

	/*
	 * disable DMI capabilities
	 */
	p_hw_cmd->type = hw_cmd_set_quant_tables[2].type;
	p_hw_cmd->n = hw_cmd_set_quant_tables[2].n;
	p_hw_cmd->offset = hw_cmd_set_quant_tables[2].offset;
	p_hw_cmd->mask = hw_cmd_set_quant_tables[2].mask;
	p_hw_cmd->data = hw_cmd_set_quant_tables[2].data;

	return p_hw_cmds;
}

struct msm_gemini_hw_cmd hw_cmd_set_filesize_control[] = {
	/*
	 * type, repeat n times, offset, mask, data or pdata
	 */
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_ENCODE_FSC_REGION_SIZE_ADDR,
	 HWIO_JPEG_ENCODE_FSC_REGION_SIZE_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_ENCODE_FSC_BUDGET_0_ADDR,
	 HWIO_JPEG_ENCODE_FSC_BUDGET_0_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_ENCODE_FSC_BUDGET_1_ADDR,
	 HWIO_JPEG_ENCODE_FSC_BUDGET_1_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_ENCODE_FSC_BUDGET_2_ADDR,
	 HWIO_JPEG_ENCODE_FSC_BUDGET_2_RMSK, {0}},
	{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, HWIO_JPEG_ENCODE_FSC_BUDGET_3_ADDR,
	 HWIO_JPEG_ENCODE_FSC_BUDGET_3_RMSK, {0}},
};

struct msm_gemini_hw_cmds
	*gemini_lib_hw_set_filesize_ctrl (gemini_cmds_jpeg_fsc_cfg *
					  sFileSizeControl)
{
	uint8_t status = 0;
	uint32_t nRegionSize = 0;
	uint32_t nBudget = 0;

	struct msm_gemini_hw_cmds *p_hw_cmds;
	struct msm_gemini_hw_cmd *p_hw_cmd;

	p_hw_cmds =
		gemini_lib_hw_cmd_malloc_and_copy (sizeof
						   (hw_cmd_set_filesize_control),
						   hw_cmd_set_filesize_control);
	if (!p_hw_cmds) {
		return NULL;
	}
	p_hw_cmd = &p_hw_cmds->hw_cmd[0];

	/*
	 * write region size to JPEG_ENCODE_FSC_REGION_SIZE
	 */
	nRegionSize |=
		(((sFileSizeControl->regionSize -
		   1) << HWIO_JPEG_ENCODE_FSC_REGION_SIZE_REGION_SIZE_SHFT) &
		 HWIO_JPEG_ENCODE_FSC_REGION_SIZE_REGION_SIZE_BMSK);
	p_hw_cmd->data = nRegionSize;
	p_hw_cmd++;

	/*
	 * set bit budget for regions 0 through 3
	 */
	nBudget =
		(((sFileSizeControl->
		   region0Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION0_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION0_BMSK) |
		 ((sFileSizeControl->
		   region1Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION1_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION1_BMSK) |
		 ((sFileSizeControl->
		   region2Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION2_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION2_BMSK) |
		 ((sFileSizeControl->
		   region3Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION3_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_0_BUDGET_REGION3_BMSK));
	p_hw_cmd->data = nBudget;
	p_hw_cmd++;

	/*
	 * set bit budget for regions 4 through 7
	 */
	nBudget =
		(((sFileSizeControl->
		   region4Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION4_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION4_BMSK) |
		 ((sFileSizeControl->
		   region5Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION5_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION5_BMSK) |
		 ((sFileSizeControl->
		   region6Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION6_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION6_BMSK) |
		 ((sFileSizeControl->
		   region7Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION7_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_1_BUDGET_REGION7_BMSK));
	p_hw_cmd->data = nBudget;
	p_hw_cmd++;

	/*
	 * set bit budget for regions 8 through 11
	 */
	nBudget =
		(((sFileSizeControl->
		   region8Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION8_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION8_BMSK) |
		 ((sFileSizeControl->
		   region9Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION9_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION9_BMSK) |
		 ((sFileSizeControl->
		   region10Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION10_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION10_BMSK) |
		 ((sFileSizeControl->
		   region11Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION11_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_2_BUDGET_REGION11_BMSK));
	p_hw_cmd->data = nBudget;
	p_hw_cmd++;

	/*
	 * set bit budget for regions 12 through 15
	 */
	nBudget =
		(((sFileSizeControl->
		   region12Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION12_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION12_BMSK) |
		 ((sFileSizeControl->
		   region13Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION13_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION13_BMSK) |
		 ((sFileSizeControl->
		   region14Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION14_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION14_BMSK) |
		 ((sFileSizeControl->
		   region15Budget <<
		   HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION15_SHFT) &
		  HWIO_JPEG_ENCODE_FSC_BUDGET_3_BUDGET_REGION15_BMSK));
	p_hw_cmd->data = nBudget;
	p_hw_cmd++;

	return p_hw_cmds;
}
