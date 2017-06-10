/* Copyright (c) 2012, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef MERCURY_LIB_HW_H
#define MERCURY_LIB_HW_H

#include <unistd.h>
#include <media/msm_mercury.h>
#include "mercury_lib.h"
#include "mercury_lib_hw_reg.h"
#include "mercury_lib_hw_reg_ctrl.h"
#include "mercury_dbg.h"

int mercury_lib_hw_jpegd_core_reset(void);
int mercury_lib_hw_jpeg_dqt(mercury_cmd_quant_cfg_t dqt_cfg);
int mercury_lib_hw_jpeg_sof(mercury_cmd_sof_cfg_t sof_cfg);
int mercury_lib_hw_jpeg_dht(mercury_cmd_huff_cfg_t huff_cfg);
int mercury_lib_hw_jpeg_sos(mercury_cmd_sos_cfg_t sos_cfg);
int mercury_lib_hw_bus_control_config(mercury_cmd_control_cfg_t  ctrl_cfg);
int mercury_lib_hw_bus_write_config(mercury_cmd_writebus_cfg_t write_config);
int mercury_lib_hw_bus_read_config(mercury_cmd_readbus_cfg_t bus_read_cfg);

int mercury_lib_jpegd_rtdma_rd_status_enable(uint8_t sof_val, uint8_t eof_val);
uint32_t mercury_lib_jpegd_rtdma_wr_status_clear(int mcrfd);
int mercury_lib_jpegd_rtdma_wr_status_enable(uint8_t err_en_val,uint8_t eof_en_val,
    uint8_t sof_en_val);
int mercury_lib_jpegd_rtdma_rd_status_clear(int mcrfd);

int mercury_lib_jpegd_wr_op_cfg(uint8_t align, uint8_t flip, uint8_t mirror);
int mercury_lib_hw_update_bus_write_config(uint8_t *y_buf, uint8_t *u_buf, uint8_t *v_buf);
int mercury_lib_hw_decode(void);

uint32_t mercury_lib_hw_check_rd_ack_irq(void);
uint32_t mercury_lib_hw_check_wr_ack_irq(void);
uint32_t mercury_lib_hw_check_jpeg_status(void);
int mercury_lib_hw_check_hw_status(void);
int mercury_lib_hw_check_buf_configs(void);

void mercury_lib_hw_setfd(int mcrfd);

#endif /* MERCURY_LIB_HW_H */
