/*============================================================================

  Copyright (c) 2013, 2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __PCA_ROLLOFF32_H__
#define __PCA_ROLLOFF32_H__

#include "pca_rolloff32_reg.h"
#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "isp_pipeline32.h"
#include "isp_pipeline_util.h"
#include "chromatix.h"
#include "chromatix_common.h"


typedef struct {
  mesh_rolloff_array_type left[ISP_ROLLOFF_MAX_LIGHT];
  mesh_rolloff_array_type right[ISP_ROLLOFF_MAX_LIGHT];
}isp_rolloff_info_t;

/*===========================================================================
 * PCA Roll-off data structures
 *==========================================================================*/
#define PCA_ROLLOFF_NUMBER_BASE 8
#define PCA_ROLLOFF_NUMBER_ROWS 13
#define PCA_ROLLOFF_NUMBER_COLS 17
#define PCA_ROLLOFF_CHANNELS 4
#define PCA_ROLLOFF_BASIS_TABLE_SIZE (1 * PCA_ROLLOFF_NUMBER_COLS)
#define PCA_ROLLOFF_COEFF_TABLE_SIZE (PCA_ROLLOFF_CHANNELS * PCA_ROLLOFF_NUMBER_ROWS)

/* Start: Data structures to hold tuned tables */
typedef struct PCA_RolloffStruct {
  /* [13][8],for 4 channels: 0->R, 1->Gr, 2->Gb, 3->B
   * coeff Components: 12uQ8, 9sQ8, 9sQ8, 8sQ8, 8sQ8, 8sQ9, 8sQ9, 8sQ10
   * basis Components: 11uQ11, 8sQ7, 8sQ7, 8sQ7, 8sQ7, 8sQ7, 8sQ7, 8sQ7 */
  float coeff_table_R[PCA_ROLLOFF_NUMBER_ROWS][PCA_ROLLOFF_NUMBER_BASE];
  float coeff_table_Gr[PCA_ROLLOFF_NUMBER_ROWS][PCA_ROLLOFF_NUMBER_BASE];
  float coeff_table_Gb[PCA_ROLLOFF_NUMBER_ROWS][PCA_ROLLOFF_NUMBER_BASE];
  float coeff_table_B[PCA_ROLLOFF_NUMBER_ROWS][PCA_ROLLOFF_NUMBER_BASE];
  float PCA_basis_table[PCA_ROLLOFF_NUMBER_BASE][PCA_ROLLOFF_NUMBER_COLS];
}PCA_RolloffStruct;
/* End: Data structures to hold tunned tables */

/* Start: Data structures to hold re-arranged tables before writing them
 *        into HW specific format */
typedef struct PCA_RollOffBasisData {
  uint16_t v0[PCA_ROLLOFF_BASIS_TABLE_SIZE];
  int v1[PCA_ROLLOFF_BASIS_TABLE_SIZE];
  int v2[PCA_ROLLOFF_BASIS_TABLE_SIZE];
  int v3[PCA_ROLLOFF_BASIS_TABLE_SIZE];
  int v4[PCA_ROLLOFF_BASIS_TABLE_SIZE];
  int v5[PCA_ROLLOFF_BASIS_TABLE_SIZE];
  int v6[PCA_ROLLOFF_BASIS_TABLE_SIZE];
  int v7[PCA_ROLLOFF_BASIS_TABLE_SIZE];
}PCA_RollOffBasisData;

typedef struct PCA_RollOffCoeffData {
  uint16_t a0[PCA_ROLLOFF_CHANNELS][PCA_ROLLOFF_NUMBER_ROWS];
  int a1[PCA_ROLLOFF_CHANNELS][PCA_ROLLOFF_NUMBER_ROWS];
  int a2[PCA_ROLLOFF_CHANNELS][PCA_ROLLOFF_NUMBER_ROWS];
  int a3[PCA_ROLLOFF_CHANNELS][PCA_ROLLOFF_NUMBER_ROWS];
  int a4[PCA_ROLLOFF_CHANNELS][PCA_ROLLOFF_NUMBER_ROWS];
  int a5[PCA_ROLLOFF_CHANNELS][PCA_ROLLOFF_NUMBER_ROWS];
  int a6[PCA_ROLLOFF_CHANNELS][PCA_ROLLOFF_NUMBER_ROWS];
  int a7[PCA_ROLLOFF_CHANNELS][PCA_ROLLOFF_NUMBER_ROWS];
}PCA_RollOffCoeffData;

typedef struct PCA_RollOffTable {
  PCA_RollOffBasisData basisData;
  PCA_RollOffCoeffData coeffData;
}PCA_RollOffTable;

/* End: Data structures to hold re-arranged tables before writing them
 *      into HW specific format */

typedef struct PCA_RollOffRamTable {
  uint64_t basisTable[PCA_ROLLOFF_BASIS_TABLE_SIZE];
  uint64_t coeffTable[PCA_ROLLOFF_COEFF_TABLE_SIZE];
}PCA_RollOffRamTable;

typedef struct PCA_RollOffConfigCmdType {
  uint32_t dmi_set0[2];
  PCA_RollOffRamTable ram0;
  uint32_t dmi_set1[2];
  PCA_RollOffRamTable ram1;
  uint32_t dmi_reset[2];
  PCA_RollOffConfigParams CfgParams;
}PCA_RollOffConfigCmdType;
/* End: Data structures to hold HW specific table formats and params */

typedef struct {
  PCA_RolloffStruct left_table[ISP_ROLLOFF_MAX_LIGHT];
  PCA_RolloffStruct right_table[ISP_ROLLOFF_MAX_LIGHT];
}pca_rolloff_tables_t;

typedef struct {
  PCA_RolloffStruct left_input_table;
  PCA_RolloffStruct right_input_table;
}pca_rolloff_params_t;

typedef struct {
  int fd;
   uint32_t session_id;
  uint32_t pca_rolloff_enable;
  uint32_t pca_rolloff_update;
 /* Driven only by EzTune */
  uint32_t pca_rolloff_trigger_enable;
  /* Drivern only by EzTune */
  uint32_t pca_rolloff_reload_params;
  PCA_RollOffConfigCmdType pca_rolloff_cmd;
  pca_rolloff_params_t pca_rolloff_param;
  PCA_RolloffStruct last_non_flash_tbl;
  pca_rolloff_tables_t pca_tbls;
  isp_rolloff_info_t rolloff_calibration_table;
  isp_rolloff_info_t rolloff_tbls;
  uint8_t hw_update_pending;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  tintless_mesh_rolloff_array_t mesh_hw;
  uint32_t tintless_configured;
  float tintless_lowlight_adjust[TINTLESS_ROLLOFF_TABLE_SIZE];
  float tintless_current_adjust[TINTLESS_ROLLOFF_TABLE_SIZE];
  uint32_t tintless_low_light_mode;
}isp_pca_rolloff_mod_t;

#endif /* __PCA_ROLLOFF32_H__ */
