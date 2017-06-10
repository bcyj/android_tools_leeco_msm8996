/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef _CAC2_H
#define _CAC2_H

#include <stdint.h>

#define CAC2_SUCCESS (0)
#define CAC2_ERROR   (1)

#define CAC2_TRUE            ((uint32_t)(1))
#define CAC2_FALSE           ((uint32_t)(0))
typedef struct
{
    uint32_t  cac2_enable_flag; // 0 - no cac, 1 - cac
    uint32_t  rnr_enable_flag;

    uint8_t * p_y;              // y component
    uint8_t * p_crcb;           // cbcr component
    int       fd;               // fd to y
    int       ion_heap_id;      // ION heap id
    uint32_t  image_format;     // 0 - cbcr 1 - crcb, both in h2v2
    uint32_t  image_width;      // image width
    uint32_t  image_height;     // image height
    uint32_t  y_stride;         // image y stride
    uint32_t  cbcr_stride;      // image cbcr stride

    // parameters for CAC tunning
    int32_t   detection_th1;
    int32_t   detection_th2;
    int32_t   detection_th3;
    int32_t   verification_th1;
    int32_t   correction_strength;

    //RNR arguments
    uint8_t  sampling_factor;// RNR downsample/upsample factor
    float *sigma_lut;     // Pointer to RNR sigma (threshold) lookup table, 162 length
    int lut_size;       // Size of the sigma_lut
    float scale_factor;   // Ratio of maximum image resolution radius to maximum lut size
    float center_noise_sigma;  // center ratio
    float center_noise_weight; // default 1.0
    float weight_order;        // 2.0f if sampling factor=2, 1.5f if sampling factor=4, 1.0f if sampling factor=8
}cac2_args_t;

uint32_t cac2_init   (int ion_heap_id);

uint32_t cac2_process(cac2_args_t * args);

uint32_t cac2_deinit();

#endif
