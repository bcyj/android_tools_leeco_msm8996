/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#ifndef __FRAMEPROC_COMP_H__
#define __FRAMEPROC_COMP_H__

#include "img_comp_priv.h"
#include "chromatix.h"

/** frameproc_comp_t
 *   @ptr: library handle
 *   @img_algo_process: Main algo process function pointer
 *   @img_algo_init: Function pointer to create and initialize
 *                 the algorithm wrapper
 *   @img_algo_deinit: Function pointer to deinitialize and
 *                   destroy the algorithm wrapper
 *   @img_algo_frame_ind: Function pointer to indicate when an
 *             input frame is available
 *   @img_algo_meta_ind: Function pointer to indicate when an
 *             meta frame is available
 *   @img_algo_set_frame_ops: Function pointer to set frame ops
 *
 *   Frameproc library function pointers
 **/
typedef struct {
  void *ptr;
  int (*img_algo_process)(void *p_context,
    img_frame_t *p_in_frame[],
    int in_frame_cnt,
    img_frame_t *p_out_frame[],
    int out_frame_cnt,
    img_meta_t *p_meta[],
    int meta_cnt);
  int (*img_algo_init)(void **pp_context, img_init_params_t *p_params);
  int (*img_algo_deinit)(void *p_context);
  int (*img_algo_frame_ind)(void *p_context, img_frame_t *p_frame);
  int (*img_algo_meta_ind)(void *p_context, img_meta_t *p_meta);
  int (*img_algo_set_frame_ops)(void *p_context, img_frame_ops_t *p_ops);
} frameproc_lib_info_t;

/** frameproc_comp_t
 *   @b: base component
 *   @p_lib: library info
 *   @p_algocontext: algorithm context
 *   @abort: Flag to indicate whether the algo is aborted
 *   @msgQ: Message queue
 *
 *   Frameproc component
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  frameproc_lib_info_t *p_lib;
  void *p_algocontext;
  int8_t abort;
  img_queue_t msgQ;
} frameproc_comp_t;

#endif //__FRAMEPROC_COMP_H__
