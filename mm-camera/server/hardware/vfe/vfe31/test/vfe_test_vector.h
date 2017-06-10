/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __VFE_TEST_VECTOR_H__
#define __VFE_TEST_VECTOR_H__

#include "tgtcommon.h"
#include "vfe_util_common.h"
#include <stdio.h>
#include <string.h>

#define MAX_FILENAME_LEN 256

#define VFE_TEST_VEC_PCA 1
#define VFE_TEST_VEC_MESH 0

#ifdef FEATURE_VFE_TEST_VECTOR
  #define CDBG_TV LOGE
#else
  #define CDBG_TV do {}while(0);
#endif

typedef struct {
  chromatix_manual_white_balance_type awb_gains;
  uint32_t color_temp;
  vfe_flash_type flash_mode;
  uint32_t sensitivity_led_off;
  uint32_t sensitivity_led_low;
  uint32_t sensitivity_led_hi;
  float lux_idx;
  float cur_real_gain;
  float digital_gain;
} vfe_test_params_t;

typedef struct {
  uint32_t *table;
  uint32_t size;
  uint32_t mask;
} vfe_test_table_t;

typedef struct {
  uint64_t *table;
  uint32_t size;
  uint64_t mask;
} vfe_test_table_64_t;

typedef struct {
  vfe_test_table_64_t ram[2];
} vfe_test_pca_table_t;

typedef struct {
  uint32_t *reg_dump;
  uint32_t *reg_mask;
  uint32_t reg_size;
  vfe_test_table_t gamma;
  vfe_test_table_t la;
  vfe_test_table_t linearization;
  vfe_test_pca_table_t pca_rolloff;
}vfe_test_module_input_t;

typedef struct {
  /* VFE register specific */
  uint32_t register_total;
  uint32_t reg_dump_enable;
  uint32_t *reg_dump_data;
  uint32_t reg_dump_len;
  /* VFE module's table specific */
  uint32_t table_dump_enable;
  uint32_t *linearization_table;
  uint32_t linearization_table_len;
  uint32_t *rolloff_table;
  uint32_t rolloff_table_len;
  uint32_t *gamma_table;
  uint32_t gamma_table_len;
}vfe_test_module_output_t;

typedef struct {
  int enable;
  int index;
  char filename[MAX_FILENAME_LEN];
  FILE *fp;
  camera_size_t camif_size;
  camera_size_t output_size;
  camera_size_t full_size;
  char *input_data;
  int input_size;
  int current;
  void* vfe_obj;
  int parse_mode;
  vfe_test_params_t params;
  int snapshot_mode;
  int update;
  vfe_test_module_input_t mod_input;
  int32_t current_index;
  int rolloff_type;
  int params_updated;
  vfe_test_module_output_t mod_output;
  int validate;
} vfe_test_vector_t;

vfe_status_t vfe_test_vector_init(vfe_test_vector_t *mod, void* vfe_obj);
vfe_status_t vfe_test_vector_deinit(vfe_test_vector_t *mod);
vfe_status_t vfe_test_vector_update_params(vfe_test_vector_t *mod);
int vfe_test_vector_enabled(vfe_test_vector_t *mod);
vfe_status_t vfe_test_vector_get_output(vfe_test_vector_t *mod);
vfe_status_t vfe_test_vector_validate(vfe_test_vector_t *mod);
#endif //__VFE_TEST_VECTOR_H__


