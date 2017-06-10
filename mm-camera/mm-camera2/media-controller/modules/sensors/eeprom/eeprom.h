/* eeprom.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__
#include <linux/types.h>
#include "sensor_common.h"



void eeprom_get_calibration_items(void *e_ctrl);
void eeprom_format_calibration_data(void *e_ctrl);
boolean eeprom_autofocus_calibration(void *e_ctrl);
void eeprom_print_matrix(float *paramlist);
void eeprom_lensshading_calibration(void *e_ctrl);
void eeprom_whitebalance_calibration(void *e_ctrl);
void eeprom_defectpixcel_calibration(void *e_ctrl);
void eeprom_get_dpc_calibration_info(void *e_ctrl, int, void *data);
int32_t eeprom_get_raw_data(void *e_ctrl, void *data);
#endif
