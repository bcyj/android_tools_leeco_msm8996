/**********************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/
#ifndef __CHROMATIX_METADATA_H__
#define __CHROMATIX_METADATA_H__

#define RNR_LUT_SIZE 164
#define SKINR_LUT_SIZE 255

/* Metadata dump structures */

#define PPROC_META_DATA_CPP_IDX 0
#define PPROC_META_DATA_FE_IDX (PPROC_META_DATA_CPP_IDX + 1)
#define PPROC_META_DATA_FD_IDX (PPROC_META_DATA_FE_IDX + 1)
#define PPROC_META_DATA_LDS_IDX (PPROC_META_DATA_FD_IDX + 1)
#define PPROC_META_DATA_CAC3_IDX (PPROC_META_DATA_LDS_IDX + 1)
#define PPROC_META_DATA_RNR_IDX (PPROC_META_DATA_CAC3_IDX + 1)
#define PPROC_META_DATA_MAX_IDX (PPROC_META_DATA_RNR_IDX + 1)

/** pproc_meta_dump_header_t
 *    @version:               metadata parser version
 *    @tuning_cpp_data_size:  size of cpp meta data
 *    @tuning_FE_data_size:   size of FE meta data
 *    @tuning_fd_data_size:   size of FD meta data
 *    @tuning_lds_data_size:  size of LDS meta data
 *    @tuning_cac3_data_size: size of CAC3 meta data
 *    @tuning_RNR2_data_size: size of RNR2 meta data
 *
 **/
typedef struct {
  uint32_t version;
  uint32_t tuning_cpp_data_size;
  uint32_t tuning_FE_data_size;
  uint32_t tuning_fd_data_size;
  uint32_t tuning_lds_data_size;
  uint32_t tuning_cac3_data_size;
  uint32_t tuning_RNR2_data_size;
} pproc_meta_header_t;

typedef enum {
  PPROC_META_DATA_INVALID = 0,
  PPROC_META_DATA_CPP,
  PPROC_META_DATA_FE,
  PPROC_META_DATA_FD,
  PPROC_META_DATA_LDS,
  PPROC_META_DATA_CAC3,
  PPROC_META_DATA_RNR,
} pproc_meta_type_t;

/** pproc_meta_entry_t
 *    @dump_type:       metadata parser version
 *    @len:             length of this meta data entry
 *    @start_addr:      start address of this meta data entry
 *    @lux_idx:         lux index
 *    @gain:            gain
 *    @pproc_meta_dump: pproc sub-module meta dump
 *
 **/
typedef struct {
  pproc_meta_type_t dump_type;
  uint32_t          len;
  uint32_t          start_addr;
  float             lux_idx;
  float             gain;
  uint32_t          component_revision_no;
  void             *pproc_meta_dump;
} pproc_meta_entry_t;

/** cpp_info_t
 *    @cpp_frame_msg: cpp frame message
 *
 **/
typedef struct {
  uint32_t  size;
  uint32_t *cpp_frame_msg;
} cpp_info_t;

typedef struct {
  uint32_t luma_dx;
  uint32_t luma_dy;
  uint32_t luma_x;
  uint32_t luma_y;
  uint32_t chroma_dx;
  uint32_t chroma_dy;
  uint32_t chroma_x;
  uint32_t chroma_y;
  uint32_t reserved[32];
} fe_config_t;

typedef struct {
  uint32_t face_detected;
  uint32_t reserve_data[32];
} fd_info_t;

typedef struct {
  uint32_t enable;
  float    LDS_DS_ratio;
  uint32_t reserve_data[8];
} lds_info_t;

typedef struct {
  uint32_t enable;
  uint32_t Detection_TH1;
  uint32_t Detection_TH2;
  uint32_t Verification_TH1;
  uint32_t Verification_TH2;
  uint32_t reserve_data[16];
} cac3_info_t;

typedef struct {
  uint32_t  enable;
  uint32_t  y_width;
  uint32_t  y_height;
  uint32_t  y_stride;
  uint32_t  cbcr_width;
  uint32_t  cbcr_height;
  uint32_t  cbcr_stride;
  uint32_t  hyst_sample_factor;
  uint32_t  sigma_lut_size ;
  uint32_t  sigma_lut[RNR_LUT_SIZE];
  uint32_t  skin_lut_size ;
  uint32_t  skin_lut[SKINR_LUT_SIZE];
  uint32_t  sampling_factor; // Def 2,2,4,4,8,8
  float     center_noise_sigma;
  float     center_noise_weight;
  float     weight_order;
  float     scale_factor;
  float     skin_strength;
  float     skin_scaler_cr;
  float     skin_sigma_scale;
  float     skin_threshold;
  uint32_t  skin_cb;
  uint32_t  skin_cr;
  uint32_t  skin_ymin;
  uint32_t  skin_ymax;
  int32_t   skin_chroma_ds_factor;
  uint32_t  ds_us_skin_detection;
  uint32_t  reserve_data[16];
} rnr_info_t;

typedef struct {
  pproc_meta_header_t header;
  pproc_meta_entry_t entry[PPROC_META_DATA_MAX_IDX];
} pproc_meta_data_t;

#endif
