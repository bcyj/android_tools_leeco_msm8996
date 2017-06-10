/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef _OV2720_U_H_
#define _OV2720_U_H_

#include "sensor.h"

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t ov2720_process_start(void *ctrl);

struct ov2720_dimension_t {
    uint16_t width;
    uint16_t height;
    uint16_t dummy_pixels;
    uint16_t dummy_lines;
    float    fps;
    float    nightshot_fps;
};

enum ov2720_resolution_t {
    OV2720_2D_QTR_SIZE,
    OV2720_2D_FULL_SIZE,
    OV2720_3D_QTR_SIZE,
    OV2720_3D_FULL_SIZE
};

#endif /* _OV2720_U_H_ */
