/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __PCA_ROLLOFF_H__
#define __PCA_ROLLOFF_H__

#include "vfe_util_common.h"
#include "vfe_test_vector.h"
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

/* Start: Data structures to hold HW specific table formats and params */
typedef struct PCA_RollOffConfigParams {
  /* VFE_ROLLOFF_CONFIG */
  uint32_t                     pixelOffset             : 9;
  uint32_t                     /* reserved */          : 7;
  uint32_t                     pcaLutBankSel           : 1;
  uint32_t                     /* reserved */          : 15;
  /* VFE_ROLLOFF_GRID_CFG_0 */
  uint32_t                     xDelta                  : 17;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     yDelta                  : 10;
  uint32_t                     /* reserved */          : 2;
  /* VFE_ROLLOFF_GRID_CFG_1 */
  uint32_t                     gridWidth               : 9;
  uint32_t                     gridHeight              : 9;
  uint32_t                     /* reserved */          : 14;
  /* VFE_ROLLOFF_RIGHT_GRID_CFG_0 */
  uint32_t                     xDeltaRight             : 17;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     yDeltaRight             : 10;
  uint32_t                     /* reserved */          : 2;
  /* VFE_ROLLOFF_RIGHT_GRID_CFG_1 */
  uint32_t                     gridWidthRight          : 9;
  uint32_t                     gridHeightRight         : 9;
  uint32_t                     /* reserved */          : 14;
  /* VFE_ROLLOFF_STRIPE_CFG_0 */
  uint32_t                     gridXIndex              : 4;
  uint32_t                     gridYIndex              : 4;
  uint32_t                     gridPixelXIndex         : 9;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     gridPixelYIndex         : 9;
  uint32_t                     /* reserved */          : 3;
  /* VFE_ROLLOFF_STRIPE_CFG_1 */
  uint32_t                     yDeltaAccum             : 13;
  uint32_t                     /* reserved */          : 19;
}__attribute__((packed, aligned(4))) PCA_RollOffConfigParams;

typedef struct PCA_RollOffRamTable {
  uint64_t basisTable[PCA_ROLLOFF_BASIS_TABLE_SIZE];
  uint64_t coeffTable[PCA_ROLLOFF_COEFF_TABLE_SIZE];
}PCA_RollOffRamTable;

typedef struct PCA_RollOffConfigTable {
  PCA_RollOffRamTable ram0;
  PCA_RollOffRamTable ram1;
}PCA_RollOffConfigTable;

typedef struct PCA_RollOffConfigCmdType {
  uint32_t tableOffset;
  PCA_RollOffConfigParams CfgParams;
  PCA_RollOffConfigTable Table;
}PCA_RollOffConfigCmdType;
/* End: Data structures to hold HW specific table formats and params */

typedef struct {
  PCA_RolloffStruct left_table[VFE_ROLLOFF_MAX_LIGHT];
  PCA_RolloffStruct right_table[VFE_ROLLOFF_MAX_LIGHT];
}pca_rolloff_tables_t;

typedef struct {
  PCA_RolloffStruct left_input_table;
  PCA_RolloffStruct right_input_table;
}pca_rolloff_params_t;

typedef struct {
  uint32_t pca_rolloff_enable;
  uint32_t pca_rolloff_update;
 /* Driven only by EzTune */
  uint32_t pca_rolloff_trigger_enable;
  /* Drivern only by EzTune */
  uint32_t pca_rolloff_reload_params;
  PCA_RollOffConfigCmdType pca_rolloff_prev_cmd;
  PCA_RollOffConfigCmdType pca_rolloff_snap_cmd;
  pca_rolloff_params_t pca_rolloff_prev_param;
  pca_rolloff_params_t pca_rolloff_snap_param;
  pca_rolloff_tables_t pca_tbls;
}pca_rolloff_mod_t;

vfe_status_t pca_rolloff_init(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_rolloff_info_t *mesh_tbls, vfe_params_t* vfe_params);
vfe_status_t pca_rolloff_config(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_params_t* vfe_params);
vfe_status_t pca_rolloff_update(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_params_t* vfe_params);
vfe_status_t pca_rolloff_trigger_update(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_params_t* vfe_params);
vfe_status_t pca_rolloff_trigger_enable(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_params_t* vfe_params, int8_t enable);
vfe_status_t pca_rolloff_reload_params(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_params_t* vfe_params, vfe_rolloff_info_t *mesh_tbls);
vfe_status_t vfe_PCA_Roll_off_test_vector_validation(
  void *mod_in, void *mod_op);
#endif /* __PCA_ROLLOFF_H__ */
