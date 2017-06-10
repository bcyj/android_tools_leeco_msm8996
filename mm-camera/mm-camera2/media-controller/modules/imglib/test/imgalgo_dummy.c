/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#include "img_meta.h"
#include "img_common.h"

/**
 * Function: img_algo_process
 *
 * Description: Main algo process function
 *
 * Arguments:
 *   @p_in_frame: array of input frames
 *   @in_frame_cnt: input frame count
 *   @p_out_frame: array of output frames
 *   @in_frame_cnt: output frame count
 *   @p_out_frame: array of meta frames
 *   @in_frame_cnt: meta frame count
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This symbol is Mandatory
 **/
int img_algo_process(void *p_context,
  img_frame_t *p_in_frame[],
  int in_frame_cnt,
  img_frame_t *p_out_frame[],
  int out_frame_cnt,
  img_meta_t *p_meta[],
  int meta_cnt)
{
  IDBG_ERROR("%s:%d] ", __func__, __LINE__);
  int i;

  for (i = 0; i < in_frame_cnt; i++) {
    img_dump_frame(p_in_frame[i], "test_algo_in", i);
  }
  for (i = 0; i < out_frame_cnt; i++) {
    img_dump_frame(p_out_frame[i], "test_algo_out", i);
  }
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_init
 *
 * Description: Create and initialize the algorithm wrapper
 *
 * Arguments:
 *   @pp_context: return context [Output parameter]
 *   @p_params: init params
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This symbol is Mandatory
 **/
int img_algo_init(void **pp_context, img_init_params_t *p_params)
{
  IDBG_ERROR("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_deinit
 *
 * Description: deinitialize and destroy the algorithm wrapper
 *
 * Arguments:
 *   @p_context: context of the algorithm
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This symbol is Mandatory
 **/
int img_algo_deinit(void *p_context)
{
  IDBG_ERROR("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_frame_ind
 *
 * Description: Function to indicate when an input frame is
 *             available
 *
 * Arguments:
 *   @p_context: context of the algorithm
 *   @p_frame: input frame
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This symbol is Optional
 **/
int img_algo_frame_ind(void *p_context, img_frame_t *p_frame)
{
  IDBG_ERROR("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_meta_ind
 *
 * Description: Function to indicate when an meta frame is
 *             available
 *
 * Arguments:
 *   @p_context: context of the algorithm
 *   @p_meta: meta frame
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This symbol is Optional
 **/
int img_algo_meta_ind(void *p_context, img_meta_t *p_meta)
{
  IDBG_ERROR("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}
