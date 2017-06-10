
/* Copyright (c) 2010, 2013, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef GEMINI_LIB_H
#define GEMINI_LIB_H
#include <linux/msm_ion.h>
#include "gemini_lib_common.h"

#define EINVALID 1
enum GEMINI_MODE
{
	GEMINI_MODE_REALTIME_ENCODE    = MSM_GEMINI_MODE_REALTIME_ENCODE,
	GEMINI_MODE_OFFLINE_ENCODE     = MSM_GEMINI_MODE_OFFLINE_ENCODE,
	GEMINI_MODE_REALTIME_ROTATION  = MSM_GEMINI_MODE_REALTIME_ROTATION,
	GEMINI_MODE_OFFLINE_ROTATION   = MSM_GEMINI_MODE_OFFLINE_ROTATION,
};

typedef void *gmn_obj_t;

struct gemini_buf {
        uint32_t type;
        int      fd;

        void     *vaddr;

        uint32_t y_off;
        uint32_t y_len;
        uint32_t framedone_len;

        uint32_t cbcr_off;
        uint32_t cbcr_len;

        uint32_t num_of_mcu_rows;
        uint32_t offset;
        int ion_fd_main;
        struct ion_allocation_data alloc_ion;
        struct ion_fd_data fd_ion_map;
};

struct gemini_evt {
        uint32_t type;
        uint32_t len;
        void     *value;
};

int gemini_lib_init (gmn_obj_t * gmn_obj_p_p, void *p_userdata,
		     int (*event_handler) (void * p_userdata,
					   struct gemini_evt *,
					   int event),
		     int (*output_handler) (void *, struct gemini_buf *),
		     int (*input_handler) (void *, struct gemini_buf *));

int gemini_lib_release(gmn_obj_t gmn_obj);

int gemini_lib_hw_config (gmn_obj_t gmn_obj,
			  gemini_cmd_input_cfg * p_input_cfg,
			  gemini_cmd_output_cfg * p_output_cfg,
			  gemini_cmd_jpeg_encode_cfg * p_encode_cfg,
			  gemini_cmd_operation_cfg * p_op_cfg);
int gemini_lib_input_buf_enq (gmn_obj_t gmn_obj, struct gemini_buf *);
int gemini_lib_output_buf_enq (gmn_obj_t gmn_obj, struct gemini_buf *);
int gemini_lib_encode (gmn_obj_t gmn_obj);

int gemini_lib_wait_done (gmn_obj_t gmn_obj);
int gemini_lib_stop (gmn_obj_t gmn_obj, int nicely);

/* @todo remove me */
#if 0
int gemini_lib_unblock (gmn_obj_t gmn_obj, int input_or_output_or_event);
#endif

#endif /* GEMINI_LIB_H */
