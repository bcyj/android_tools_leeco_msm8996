/*==========================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "eeprom_interface.h"
#include "tgtcommon.h"
#include "af_tuning.h"
#define EEPROM_MAX_CLIENT_NUM 4

typedef struct {
  float r_over_g;
  float b_over_g;
  float gr_over_gb;
} wbcalib_data;

typedef struct {
  uint16_t worst_macro;
  uint16_t worst_inf;
  uint16_t start_current;
} afcalib_data;

typedef struct {
  mesh_rolloff_array_type lsc_calib;
} lsccalib_data;

typedef struct {
  pixel_t snapshot_cord[MESH_ROLLOFF_SIZE];
  pixel_t preview_cord[MESH_ROLLOFF_SIZE];
  pixel_t video_cord[MESH_ROLLOFF_SIZE];
} dpccalib_data;

typedef struct {
  wbcalib_data wbc;
  afcalib_data afc;
  lsccalib_data lsc;
  dpccalib_data dpc;
} calibrated_2d_data;

typedef struct {
  struct msm_calib_wb wbc;
  struct msm_calib_af afc;
  struct msm_calib_lsc lsc;
  struct msm_calib_dpc dpc;
  struct msm_calib_raw raw;
} eeprom_calib_2d_data;

typedef struct {
  void (*dpc_calibration_info) (void *, int, void *);
  void (*do_calibration) (void *);
  int32_t (*eeprom_get_raw_data) (void *, void *);
} eeprom_function_table_t;

typedef struct {
  uint32_t fd;
  eeprom_function_table_t fn_table;
  chromatix_parms_type *chromatixPtr;
  af_tune_parms_t *aftune_ptr;
  uint8_t is_eeprom_supported;
  pixels_array_t dpc_data;
  void *cali_data;
  calibrated_2d_data calibrated_data;
} eeprom_ctrl_t;

typedef struct {
  uint8_t client_idx;
  uint32_t handle;
  mctl_ops_t *ops;
  eeprom_ctrl_t *eepromCtrl;
} eeprom_client_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t handle_cnt;
  eeprom_client_t client[EEPROM_MAX_CLIENT_NUM];
} eeprom_comp_root_t;

void eeprom_init(eeprom_ctrl_t *ectrl);
void eeprom_destroy(eeprom_ctrl_t *ectrl);

#endif /* __EEPROM_H__ */
