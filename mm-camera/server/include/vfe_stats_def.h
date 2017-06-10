/**********************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __VFE_STATS_DEF_H__
#define __VFE_STATS_DEF_H__

typedef struct {
  /* stats version number */
  uint32_t stats_version;
  /* map to enum msm_stats_enum_type in msm_camera.h */
  uint32_t stats_type;
  /* the window width used to collect the stats */
  uint32_t width;
  /* the window height used to collect the stats */
  uint32_t height;
  /* timestamp in microsecond */
  struct timeval tv;
  /* unique frame id of that stats */
  uint32_t frame_id;
} vfe_stats_common_data_t;

#define VFE_STATS_AEC_NUM_MAX 256

typedef struct {
  uint32_t num_of_regions;
  //TODO: need to remove if not used
  uint32_t shift_bits;
  /* AEC STATS DATA */
  /* 24 bit, Y sum in each of X rgs meeting inequalities */
  uint32_t SY[VFE_STATS_AEC_NUM_MAX];
} vfe_stats_ae_data_t;

typedef struct {
  vfe_stats_common_data_t cm_data;
  union {
    vfe_stats_ae_data_t ae;
  };
} vfe_stats_ae_struct_t;

#define VFE_STATS_AWB_NUM_MAX 256
typedef struct {
  /* 24 bits, sum of luminance in each of the X regions */
  uint32_t SY1[VFE_STATS_AWB_NUM_MAX];
  /* 24 bits Cb sum in each X regions meeting inequalities*/
  uint32_t SCb[VFE_STATS_AWB_NUM_MAX];
  /* 24 bits Cr sum in each X regions meeting inequalities*/
  uint32_t SCr[VFE_STATS_AWB_NUM_MAX];
  /* 16 bit, # of pix in SCb in each X rgns meeting	the inequalities */
  uint32_t NSCb[VFE_STATS_AWB_NUM_MAX];
  /*Y sum of pixels with Ymin<Y<Ymax */
  uint32_t GLB_Y;
  /*Cb sum of pixels with Ymin<Y<Ymax*/
  uint32_t GLB_Cb;
  /*Cr sum of pixels with Ymin<Y<Ymax*/
  uint32_t GLB_Cr;
  /*Num of pixels in the global AWB statistics*/
  uint32_t GLB_N;

  /*R sum of green zone pixels with Ymin<Y<Ymax */
  uint32_t Green_R;
  /*G sum of green zone pixels with Ymin<Y<Ymax */
  uint32_t Green_G;
  /*B sum of green zone pixels with Ymin<Y<Ymax */
  uint32_t Green_B;
  /*Number of green zone pixels*/
  uint32_t Green_N;

  /*R sum of blue zone pixels with Ymin<Y<Ymax */
  uint32_t ExtBlue_R;
  /*G sum of blue zone pixels with Ymin<Y<Ymax */
  uint32_t ExtBlue_G;
  /*B sum of blue zone pixels with Ymin<Y<Ymax */
  uint32_t ExtBlue_B;
  /*Number of blue zone pixels*/
  uint32_t ExtBlue_N;

  /*R sum of red zone pixels with Ymin<Y<Ymax */
  uint32_t ExtRed_R;
  /*G sum of red zone pixels with Ymin<Y<Ymax */
  uint32_t ExtRed_G;
  /*B sum of red zone pixels with Ymin<Y<Ymax */
  uint32_t ExtRed_B;
  /*Number of red zone pixels*/
  uint32_t ExtRed_N;
} vfe_stats_awb_data_t;
typedef struct {
  vfe_stats_common_data_t cm_data;
  union {
    vfe_stats_awb_data_t awb;
  };
} vfe_stats_awb_struct_t;

typedef struct {
  /* AF STATS DATA */
  /* 27 bit, focus value */
  uint32_t Focus;
  /* 9 bit, cnt of # of rows meeting the condition */
  uint32_t NFocus;
} vfe_stats_af_data_t;
typedef struct {
  vfe_stats_common_data_t cm_data;
  union {
    vfe_stats_af_data_t af;
  };
} vfe_stats_af_struct_t;

#define VFE_STATS_IHIST_NUM_MAX 256
typedef struct {
  uint32_t num_ihist;
  uint16_t buffer[VFE_STATS_IHIST_NUM_MAX];
  uint32_t *histogram;
} vfe_stats_ihist_data_t;
typedef struct {
  vfe_stats_common_data_t cm_data;
  union {
    vfe_stats_ihist_data_t ihist;
  };
} vfe_stats_ihist_struct_t;

#define VFE_STATS_CS_NUM_MAX 1344
typedef struct {
  uint32_t num_col_sum;
  uint32_t frame_id;
  uint32_t col_sum[VFE_STATS_CS_NUM_MAX];
} vfe_stats_cs_data_t;
typedef struct {
  vfe_stats_common_data_t cm_data;
  union {
    vfe_stats_cs_data_t cs;
  };
} vfe_stats_cs_struct_t;

#define VFE_STATS_RS_NUM_MAX 1024
typedef struct {
  uint32_t num_row_sum;
  uint32_t row_sum[VFE_STATS_RS_NUM_MAX];
} vfe_stats_rs_data_t;
typedef struct {
  vfe_stats_common_data_t cm_data;
  union {
    vfe_stats_rs_data_t rs;
  };
} vfe_stats_rs_struct_t;

#endif /* __VFE_STATS_DEF_H__ */

