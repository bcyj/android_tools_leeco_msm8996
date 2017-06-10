/*============================================================================
   Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __TINTLESS_INTERFACE_H__
#define __TINTLESS_INTERFACE_H__

#include "chromatix.h"
#include "chromatix_common.h"

#define TINTLESS_UPDATE_STATS            (1 << 0)
#define TINTLESS_UPDATE_AEC              (1 << 1)
#define TINTLESS_UPDATE_AWB              (1 << 2)
#define TINTLESS_UPDATE_AF               (1 << 3)
#define TINTLESS_UPDATE_CAMERA_MODE      (1 << 4)
#define TINTLESS_UPDATE_FLASH_MODE       (1 << 5)
#define TINTLESS_UPDATE_CHROMATIX_PARAMS (1 << 6)
#define TINTLESS_UPDATE_MESH             (1 << 7)

#ifndef VFE_32
#define TINTLESS_ROLLOFF_TABLE_SIZE  (13 * 10)
#else
#define TINTLESS_ROLLOFF_TABLE_SIZE  (17 * 13)
#endif

typedef enum {
  TINTLESS_SUCCESS               =  0,
  TINTLESS_NO_MEMORY             = -1,
  TINTLESS_ERROR                 = -2,
  TINTLESS_INVALID_STATS         = -3,
  TINTLESS_BAD_OUTPUT_TABLE      = -4,
  TINTLESS_LIB_NOT_LOADED        = -5,
  TINTLESS_LIB_MISMATCH          = -6,
  TINTLESS_UPDATES_NOT_SUPPORTED = -7,
  TINTLESS_UNKNOWN_ERROR         = -8
} tintless_return_t;

typedef enum {
  STATS_TYPE_BE,
  STATS_TYPE_BG
} tintless_stats_type;

typedef enum {
    SUBGRID1_1  = 1,
    SUBGRID2_2  = 2,
    SUBGRID4_4  = 4,
    SUBGRID8_8  = 8
} tintless_subgrid_t;

typedef struct {
  uint16_t  api_version;
  uint16_t  minor_version;
} tintless_version_t;

typedef struct {
  uint32_t  camif_win_w;    // width of the camif window
  uint32_t  camif_win_h;    // heigth of the camif window
  uint32_t  stat_elem_w;    // width of one stat element
  uint32_t  stat_elem_h;    // heigth of one stat element
#ifndef VFE32
  uint32_t  num_stat_elem_rows;    // number of stat element rows
  uint32_t  num_stat_elem_cols;    // number of stat element columns
#endif
  tintless_stats_type  stats_type; // post or pre stats
} tintless_stats_config_t;

typedef struct {
    uint32_t            num_mesh_elem_rows;    // number of stat element rows
    uint32_t            num_mesh_elem_cols;    // number of stat element columns
    uint32_t            offset_horizontal;
    uint32_t            offset_vertical;
    uint32_t            subgrid_height;
    uint32_t            subgrid_width;
    tintless_subgrid_t  subgrids;    // any interpolation inside the mesh
} tintless_mesh_config_t;

typedef struct {
    const uint32_t* channel_counts; // number of pixels counted
    const uint32_t* channel_sums;   // sum of all pixel values counted
    uint32_t array_length;          // length of the array
} tintless_bayer_stats_t;

typedef struct {
    tintless_bayer_stats_t r;
    tintless_bayer_stats_t gr;
    tintless_bayer_stats_t gb;
    tintless_bayer_stats_t b;
} tintless_stats_t;

typedef struct {
    unsigned short table_size;                   // number of elements in each color channel
    float r_gain[TINTLESS_ROLLOFF_TABLE_SIZE];   // RGain array in mesh table [13 x 10]
    float gr_gain[TINTLESS_ROLLOFF_TABLE_SIZE];  // GRGain array in mesh table
    float gb_gain[TINTLESS_ROLLOFF_TABLE_SIZE];  // GBGain array in mesh table
    float b_gain[TINTLESS_ROLLOFF_TABLE_SIZE];   // BGain array in mesh table
} tintless_mesh_rolloff_array_t;

#if 0
tintless_return_t isp_tintless_open(void ** const res, uint32_t * updates_needed);
tintless_return_t isp_tintless_stat_config(void * const res, tintless_stats_config_t *cfg);
tintless_return_t isp_tintless_mesh_config(void * const res, tintless_mesh_config_t *cfg);
tintless_return_t isp_tintless_update_chromatix_params(void * const res, chromatix_color_tint_correction_type *p);
tintless_return_t isp_tintless_get_version(void * const res, tintless_version_t *version);
tintless_return_t isp_tintless_algo(void * const res,
                                    tintless_stats_t * be_stats,
                                    tintless_mesh_rolloff_array_t *ptable_3a,
                                    tintless_mesh_rolloff_array_t *ptable_correction);
tintless_return_t isp_tintless_close(void ** const res);
#endif
#endif //__TINTLESS_INTERFACE_H
