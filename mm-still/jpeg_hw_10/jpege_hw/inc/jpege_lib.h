/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef JPEGE_LIB_H
#define JPEGE_LIB_H
#include <linux/msm_ion.h>
#include "jpege_lib_common.h"

#define EINVALID 1

typedef void *jpege_hw_obj_t;

struct jpege_hw_buf {
  uint32_t type;
  int fd;
  void *vaddr;
  uint32_t y_off;
  uint32_t y_len;
  uint32_t framedone_len;
  uint32_t cbcr_off;
  uint32_t cbcr_len;
  uint32_t cr_len;
  uint32_t cr_offset;
  uint32_t num_of_mcu_rows;
  uint32_t offset;
  int ion_fd_main;
  struct ion_allocation_data alloc_ion;
  struct ion_fd_data fd_ion_map;
};

struct jpege_hw_evt {
  uint32_t type;
  uint32_t len;
  void *value;
};

int jpege_lib_init(jpege_hw_obj_t * jpege_hw_obj, void *p_userdata,
  int (*event_handler) (void * p_userdata,
  struct jpege_hw_evt *,
  int event),
  int (*output_handler) (void*, struct jpege_hw_buf *),
  int (*input_handler) (void*, struct jpege_hw_buf *));

int jpege_lib_release(jpege_hw_obj_t jpege_hw_obj);

int jpege_lib_hw_config(jpege_hw_obj_t jpege_hw_obj,
  jpege_cmd_input_cfg * p_input_cfg,
  jpege_cmd_jpeg_encode_cfg * p_encode_cfg,
  jpege_cmd_scale_cfg * p_scale_cfg);
int jpege_lib_input_buf_enq(jpege_hw_obj_t jpege_hw_obj,
  struct jpege_hw_buf *);
int jpege_lib_output_buf_enq(jpege_hw_obj_t jpege_hw_obj,
  struct jpege_hw_buf *);
int jpege_lib_encode(jpege_hw_obj_t jpege_hw_obj);

int jpege_lib_get_event(jpege_hw_obj_t jpege_hw_obj, struct jpege_hw_evt *);
int jpege_lib_get_input(jpege_hw_obj_t jpege_hw_obj, struct jpege_hw_buf *);
int jpege_lib_get_output(jpege_hw_obj_t jpege_hw_obj, struct jpege_hw_buf *);

int jpege_lib_wait_done(jpege_hw_obj_t jpege_hw_obj);
int jpege_lib_stop(jpege_hw_obj_t jpege_hw_obj);

#endif /* JPEGE_LIB_H */
