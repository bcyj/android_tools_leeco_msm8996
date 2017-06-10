/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef _OV9726_U_H_
#define _OV9726_U_H_

#include "sensor.h"

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t ov9726_process_start(void *ctrl);

struct ov9726_dimension_t {
    uint16_t width;
    uint16_t height;
    uint16_t dummy_pixels;
    uint16_t dummy_lines;
    float    fps;
    float    nightshot_fps;
};

enum ov9726_resolution_t {
    OV9726_2D_QTR_SIZE,
    OV9726_2D_FULL_SIZE,
    OV9726_3D_QTR_SIZE,
    OV9726_3D_FULL_SIZE
};

#endif /* _OV9726_U_H_ */
