/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/


#ifndef __SW_WNR_CHROMATIX_H__
#define __SW_WNR_CHROMATIX_H__

#include "wavelet_denoise_lib.h"

/* Defining max number of affinity tables available for tuning */
#define SW_WNR_MAX_AFFINNITY_TABLES 10
/* Defining max number of segment dividers available for tuning */
#define SW_WNR_MAX_SEGMENT_DIVIDERS 5

/* Affinity table ranges  */
#define SW_WNR_NUM_SEGMENTS MAX_SEGMENTS
#define SW_WNR_NUM_SAMPLES  MAX_SAMPLES

/** sw_wnr_tunning_selection_t
 *   valid: This selection is valid or not.
 *   modes: Mask of supported modes for this selection
 *   uv_subsampling: Mask of supported subsampling factors for this selection
 *   min_size: Min size for this selection
 *   max_size: Max size for this selection
 *   sw_wnr_tunning_selection_t
 **/
typedef struct {
  uint32_t valid;
  uint32_t modes;
  uint32_t uv_subsampling;
  uint64_t min_size;
  uint64_t max_size;
} sw_wnr_tunning_selection_t;

/** sw_wnr_affinity_tables_t
 *   selection: Selection per this affinity table will be valid.
 *   core_type_t: Affinity table
 *
 *   sw_wnr_affinity_tables_t
 **/
typedef struct {
  sw_wnr_tunning_selection_t selection;
  core_type_t table[SW_WNR_NUM_SAMPLES][SW_WNR_NUM_SEGMENTS];
} sw_wnr_affinity_tables_t;

/** sw_wnr_affinity_tables_t
 *   selection: Selection per this affinity table will be valid.
 *   divider: Segment divider
 *
 *   sw_wnr_segment_dividers_t
 **/
typedef struct {
  sw_wnr_tunning_selection_t selection;
  uint32_t divider;
} sw_wnr_segment_dividers_t;

/** sw_wnr_segment_dividers_t
 *  Tuning parameters specific to sw wnr implementation
 *  All the parameters have selection and actual parameter.
 *  Tuning will be selected based on selection fields
 *
 *   afinity_table: Affinity tables tunning parameters.
 *   sw_wnr_segment_dividers_t: Segment divider tunning parameters
 *
 *   sw_wnr_segment_dividers_t
 **/
typedef struct {
  sw_wnr_affinity_tables_t  afinity_table[SW_WNR_MAX_AFFINNITY_TABLES];
  sw_wnr_segment_dividers_t segment_divider[SW_WNR_MAX_SEGMENT_DIVIDERS];
} sw_wnr_chromatix_t;

#endif //__SW_WNR_CHROMATIX_H__
