/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MM_QCAMERA_APP_UNIT_TEST_H__
#define __MM_QCAMERA_APP_UNIT_TEST_H__

#include "camera.h"
#include "mm_qcamera_main_menu.h"
#include "mm_camera_interface2.h"
#include "mm_qcamera_app.h"

typedef int (*mm_app_test_t) (mm_camera_app_t *cam_apps, int cam_id, int query_cam_info);
typedef struct {
	mm_app_test_t f;
	int r;
} mm_app_tc_t;

#endif 

/* __MM_QCAMERA_APP_UNIT_TEST_H__ */
