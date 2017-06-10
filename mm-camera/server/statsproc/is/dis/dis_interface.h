/*==============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
 *============================================================================*/
#ifndef _DIS_H_
#define _DIS_H_

#include "stats_proc_interface.h"

#include "camera_dbg.h"

/*------------------------------------------------------------------------------
 * DIS error codes
 *----------------------------------------------------------------------------*/
#define DIS_SUCCESS             0
#define DIS_ERR_BASE            0x00
#define DIS_ERR_FAILED          (DIS_ERR_BASE + 1)
#define DIS_ERR_NO_MEMORY       (DIS_ERR_BASE + 2)
#define DIS_ERR_BAD_HANDLE      (DIS_ERR_BASE + 3)
#define DIS_ERR_BAD_POINTER     (DIS_ERR_BASE + 4)
#define DIS_ERR_BAD_PARAM       (DIS_ERR_BASE + 5)

/*------------------------------------------------------------------------------
 * Constant / Define Declarations
 *----------------------------------------------------------------------------*/
#define MAX_ROWS                64
#define MAX_HIST_SIZE           20
#define MAX_RELIABILITY_INDEX   10
#define MAX_HIGHEST_HISTPICKS   4
#define MAX_SUB_REGION_W        4
#define MAX_SUB_REGION_H        4
#define MAX_SUB_REGION_SIZE     (MAX_SUB_REGION_W * MAX_SUB_REGION_H)

#define DIS_MAX_ROI                       (1)
#define DIS_MAX_SEARCH_RANGE_X            (128)
#define DIS_MAX_SEARCH_RANGE_Y            (128)

//#define IS_ME_CENTER_AROUND_MVP
#define DIS_VFE_METHOD_FLAG                 (0x01 << 0)

#define DIS_DEFAULT_METHOD                  DIS_VFE_METHOD_FLAG
#define DIS_DEFAULT_ROI_TYPE                1
#define DIS_DEFAULT_GMV_THRESHOLD           3.9

#define DIS_SIGNATURE                       0xABBA
#define DIS_PARAM_VERSION_0                 0x0100
#define DIS_PARAM_VERSION_ROI_0             0x0200

#ifndef SIGN
#define SIGN(x) (((x) >= 0) ? 1 : -1)
#endif

#define ROW_COL_SUM_DIFF

#define MOTION_THRESHOLD 0.00005
/*------------------------------------------------------------------------------
 * Type Declarations
 *----------------------------------------------------------------------------*/
typedef struct {
  int32_t x;
  int32_t y;
  uint32_t frame_id;
} dis_position_type;

typedef struct {
  // input frame width, height
  uint16_t input_frame_width;
  uint16_t input_frame_height;

  // margin in x, y direction to allow image stabilization
  uint16_t margin_x;
  uint16_t margin_y;

  // stabilization search window in x, y direction (pixels)
  uint16_t search_x;
  uint16_t search_y;

  // video frame rate
  uint16_t frame_rate;

  uint32_t num_row_sum;
  uint32_t num_col_sum;
} dis_init_type;

typedef struct {
  int32_t offsetX;
  int32_t offsetY;
  int32_t width;
  int32_t height;
  int32_t weight;
  double gmv_threshold;
} dis_roi_info_type;

typedef struct {
  uint32_t row_sum[1024];
  uint32_t col_sum[1344];
} dis_1d_proj_type;

typedef struct {
  uint32_t use_hardcoded_roi;
  int32_t num_roi;
  uint32_t num_row_sum;
  uint32_t num_col_sum;
  dis_roi_info_type roi[DIS_MAX_ROI];

  dis_1d_proj_type *curr_frame_1d_proj;
  dis_1d_proj_type *prev_frame_1d_proj;

  dis_1d_proj_type curr_frame_1d_proj_data;
  dis_1d_proj_type prev_frame_1d_proj_data;
} dis_vfe_ctxt_type;

typedef struct {
  uint32_t dis_method;

  int32_t encode_frame_width;
  int32_t encode_frame_height;

  int32_t input_frame_width;
  int32_t input_frame_height;

  int32_t margin_x;
  int32_t margin_y;

  int32_t search_x;
  int32_t search_y;

  int32_t num_roi;
  dis_roi_info_type roi[DIS_MAX_ROI];

  int32_t hist_size;
  dis_position_type *shake_record;
  dis_position_type prev_shake;
  dis_position_type inst_shake;
  dis_position_type avg_shake;

  int32_t frameNo;

  dis_vfe_ctxt_type *vfe;
  dis_vfe_ctxt_type vfe_context;

#ifdef ROW_COL_SUM_DIFF
  //Difference of row and column sum statistics for current and previous frame
  int32_t *curr_frame_row_sum_diff;
  int32_t *curr_frame_col_sum_diff;
  int32_t *prev_frame_row_sum_diff;
  int32_t *prev_frame_col_sum_diff;
#endif /* ROW_COL_SUM_DIFF */

  dis_init_type init_data;
  stats_proc_set_dis_data_t input_data;
  int32_t gyro_motion_x;
  int32_t gyro_motion_y;
} dis_context_type;

typedef struct {
  uint16_t signature;
  uint16_t version;
  uint32_t size;
} dis_signature_type;

/*------------------------------------------------------------------------------
 * Function Declarations
 *----------------------------------------------------------------------------*/
int dis_init(dis_init_type *init_param, dis_context_type* p_dis_context);
int dis_exit(dis_context_type*   dis_context);
int dis_stabilize_frame_with_1d_proj(dis_context_type  *dis_context,
  dis_position_type *offset);

#endif //_DIS_H_
