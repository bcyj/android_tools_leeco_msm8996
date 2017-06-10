/*============================================================================

   Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved
   QUALCOMM Proprietary and Confidential.

============================================================================*/

#ifndef DMLRO_CORRECTION_H
#define DMLRO_CORRECTION_H

/*========================================================================

*//** @file dmlrocorrection.h

This file contains DLMRO correction functions including Bayer Grid
conversion and Dynamic Tint Reduction

@par EXTERNALIZED FUNCTIONS
(none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
(none)

*/ /*====================================================================== */


/*========================================================================
Edit History


when       who     what, where, why
--------   ---     -------------------------------------------------------
06/01/12   rd   Initial version
06/21/12   rd   Added new API functions for pre and post Bayer stats
========================================================================== */

/*============================================================================
INCLUDE FILES
============================================================================*/
#include <stdarg.h>

/*============================================================================
DEFINITIONS and CONSTANTS
============================================================================*/

#define DMLROC_API_VERSION 2
#define DMLROC_FUNC_VERSION 53

#define DMLROC_STRENGTH_DEFAULT 4
#define DMLROC_CAMIF_WIDTH_DEFAULT 1536
#define DMLROC_CAMIF_HEIGHT_DEFAULT 1224
#define DMLROC_STAT_ELEM_WIDTH_DEFAULT 24
#define DMLROC_STAT_ELEM_HEIGHT_DEFAULT 24
#define TINTLESS_MESH_ROLLOFF_SIZE (17 * 13)

/*============================================================================
Type DECLARATIONS
============================================================================*/

typedef enum {
  DMLROC_SUCCESS          =  0,
  DMLROC_NO_MEMORY        = -1,
  DMLROC_ERROR            = -2,
  DMLROC_INVALID_STATS    = -3,
  DMLROC_BAD_OUTPUT_TABLE = -4,
  DMLROC_LIB_NOT_LOADED   = -5
} dmlroc_return_t;

typedef enum {
  BGC_SUCCESS     =  0,
  BGC_NO_MEMORY   = -1,
  BGC_ERROR       = -2
} bgconversion_return_t;

typedef struct {
  uint32_t  camif_win_w;    // width of the camif window
  uint32_t  camif_win_h;    // heigth of the camif window
  uint32_t  stat_elem_w;    // width of one stat element
  uint32_t  stat_elem_h;    // heigth of one stat element
                            // from chromatix_color_tint_correction_type
  unsigned char tint_correction_strength;
} dmlroc_config_t;

typedef struct {
  uint16_t  api_version;
  uint16_t  func_version;  // represents the Matlab version of the algo
} dmlroc_version_t;

// Defines mesh based roll off correction tables
typedef struct
{
    uint16_t table_size;                          /**< Size of Mesh Tables */
    float  r_gain[TINTLESS_MESH_ROLLOFF_SIZE];  /**< Red Channel Mesh Table */
    float  gr_gain[TINTLESS_MESH_ROLLOFF_SIZE]; /**< GreenRed Channel Mesh Table */
    float  gb_gain[TINTLESS_MESH_ROLLOFF_SIZE]; /**< GreenBlue Channel Mesh Table */
    float  b_gain[TINTLESS_MESH_ROLLOFF_SIZE];  /**< Blue Channel Mesh Table */
} mesh_rolloff_array_t;

/*
* bayer_stats_info_t defines the Bayer stats information.
*/
typedef struct
{
    const uint32_t* channel_counts;
    const uint32_t* channel_sums;
    // Size of each array. All the array sizes are same.
    uint32_t array_length;
} bayer_grid_stats_info_t;

/*============================================================================
Function DECLARATIONS
============================================================================*/

/*===========================================================================

  Function           : dmlroc_entry

  Description        : API function that computes correction rolloff table
  with Post Bayer stats

  Input parameter(s) : red bayer stats
  green-red bayer stats
  green-blue bayer stats
  blue bayer stats
  current rolloff table
  3A rolloff table

  Output parameter(s): correction rolloff table. only R and B chans are valid

  Return Value       : dmlroc_return_t

  Side Effects       : None

=========================================================================== */
dmlroc_return_t dmlroc_entry(
   const bayer_grid_stats_info_t *const pbayer_r,     // 48x64
   const bayer_grid_stats_info_t *const pbayer_gr,
   const bayer_grid_stats_info_t *const pbayer_gb,
   const bayer_grid_stats_info_t *const pbayer_b,
   const mesh_rolloff_array_t *const ptable_current,       // 13x17
   mesh_rolloff_array_t *const ptable_3a,                  // 13x17
   mesh_rolloff_array_t *const ptable_correction);         // 13x17


/*===========================================================================

Function           : dmlroc_init

Description        : Initialize and configure dmlroc.
                     Call this function when there's a change to the bg stats
                     or if the fixed table has changed.

Input parameter(s) : cfg - bg stats config + tuning param

Return Value       : dmlroc_return_t

Side Effects       : None

=========================================================================== */
dmlroc_return_t dmlroc_init(const dmlroc_config_t *const cfg);


/*===========================================================================

Function           : dmlroc_deinit

Description        : De-allocated resources used by the dmlroc library.

Input parameter(s) : none

Return Value       : void

Side Effects       : camif_win_w=0 represents an uninitiated library.
                     Init must be called before running the main algo.

=========================================================================== */
void dmlroc_deinit(void);


/*===========================================================================

Function           : dmlroc_get_version

Description        : Returns the libaray version of dmlroc.

Input parameter(s) : Pointer to a dmlroc_version_t structure

Return Value       : void

Side Effects       : Input struct is updated with the version info.

=========================================================================== */
void dmlroc_get_version(dmlroc_version_t *const pVersion);

#endif /* DMLRO_CORRECTION_H */
