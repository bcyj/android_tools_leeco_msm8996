
/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef GEMINI_LIB_HW_H
#define GEMINI_LIB_HW_H

#include <unistd.h>
#include <media/msm_gemini.h>
#include "gemini_hw_core.h"

struct msm_gemini_hw_buf
{
	int len;
	int framedone_len;
	/*
	 * void * vaddr;
	 */
	void *paddr;
};

struct msm_gemini_hw_fake_pingpong
{
	struct msm_gemini_hw_buf pingpong_buf[2];
	int pingpong_buf_status[2];
	int pingpong_buf_active_index;
};

int gemini_lib_hw_fake_pingpong_enqueue (struct msm_gemini_hw_fake_pingpong
					 *fake_hw,
					 struct msm_gemini_hw_buf *buf);
void *gemini_lib_hw_fake_pingpong_irq (struct msm_gemini_hw_fake_pingpong
				       *fake_hw);
void *gemini_lib_hw_fake_pingpong_active_buffer (struct
						 msm_gemini_hw_fake_pingpong
						 *fake_hw);

int gemini_lib_hw_get_irq_status (void);
void gemini_lib_hw_set_irq_clear (void);
void gemini_lib_hw_set_irq_mask (int enable);
void gemini_lib_hw_parse_irq_status (void);
void gemini_lib_hw_framedone_irq_no_fragment (void);
void gemini_lib_hw_wait_bus_idle (void);
void gemini_lib_hw_wait_bus_idle_ack (void);
void gemini_lib_hw_fe_pingpong_update (void);
void gemini_lib_hw_we_pingpong_update (void);

void gemini_lib_hw_get_version (struct msm_gemini_hw_cmd *p_hw_cmd);

struct msm_gemini_hw_cmds *gemini_lib_hw_stop (gemini_cmd_operation_cfg *
					       p_op_cfg, int nicely);
struct msm_gemini_hw_cmds
	*gemini_lib_hw_pipeline_cfg (gemini_encode_pipeline_cfg_type * pIn);
void gemini_lib_hw_we_buffer_reload (void);

struct msm_gemini_hw_cmds *gemini_lib_hw_start (gemini_cmd_operation_cfg *
						p_op_cfg);

struct msm_gemini_hw_cmds *gemini_lib_hw_fe_cfg (gemini_cmd_input_cfg *
						 p_input_cfg);
struct msm_gemini_hw_cmds *gemini_lib_hw_op_cfg (gemini_cmd_operation_cfg *
						 p_op_cfg,
						 gemini_fe_operation_cfg_type *
						 p_op_cfg_type);
void gemini_lib_hw_fe_buffer_cfg (void);
void gemini_lib_hw_fe_imem_cfg (void);
struct msm_gemini_hw_cmds *gemini_lib_hw_we_cfg (gemini_cmd_output_cfg * pIn);
void gemini_lib_hw_we_buffer_cfg (void);

struct msm_gemini_hw_cmds *gemini_lib_hw_restart_marker_set (uint32_t
							     restartInterval);

void gemini_lib_hw_create_huffman_tables (gemini_cmd_jpeg_encode_cfg *
					  p_encode_cfg,
					  gemini_huff_lookup_table_type *
					  sLumaDCHuffmanLUT,
					  gemini_huff_lookup_table_type *
					  sChromaDCHuffmanLUT,
					  gemini_huff_lookup_table_type *
					  sLumaACHuffmanLUT,
					  gemini_huff_lookup_table_type *
					  sChromaACHuffmanLUT);
struct msm_gemini_hw_cmds
	*gemini_lib_hw_set_huffman_tables (gemini_huff_lookup_table_type *
					   pLumaDC,
					   gemini_huff_lookup_table_type *
					   pChromaDC,
					   gemini_huff_lookup_table_type *
					   pLumaAC,
					   gemini_huff_lookup_table_type *
					   pChromaAC);
struct msm_gemini_hw_cmds *gemini_lib_hw_read_quant_tables (void);
struct msm_gemini_hw_cmds *gemini_lib_hw_set_quant_tables (gemini_quantTable *
							   pY,
							   gemini_quantTable *
							   pChroma);
struct msm_gemini_hw_cmds
	*gemini_lib_hw_set_filesize_ctrl (gemini_cmds_jpeg_fsc_cfg *
					  sFileSizeControl);

void gemini_lib_hw_clk_cfg (void);

void gemini_lib_hw_irq_cfg (void);

#endif /* GEMINI_LIB_HW_H */
