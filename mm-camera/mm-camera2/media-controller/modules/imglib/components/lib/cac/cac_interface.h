/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/
#ifndef CAC_INTERFACE_H
#define CAC_INTERFACE_H

#include "cac_int.h"

#ifdef ANDROID_BUILD

#include <stdint.h>

#endif
typedef struct
{
    uint8_t                *pInY;     //Y component
    int32_t                fdInY;     //Y component ION buffer file descriptor
    uint8_t                *pInC;     //CbCr component
    uint8_t                *pTmpY;    //a copy of Y
    uint8_t                *pTmpC;    //a copy of Cbcr

    uint32_t               imageWidth;
    uint32_t               imageHeight;
    uint32_t               Y_stride;  // Y stride
    uint32_t               C_stride;  // C stride

    uint32_t               numThread;    //number of thread

    uint32_t               chromaOrder; //0: CbCr, 1: CrCb

    uint32_t               awbGR;     //awb gain
    uint32_t               awbGB;     //awb gain

    uint16_t *              RGammaTable_16;// gamma table, 64 entry, 16 bit,
    uint16_t *              GGammaTable_16;
    uint16_t *              BGammaTable_16;

    int16_t edgeTH;            // edge detection threshold
    uint8_t saturatedTH;       // Y component saturation threshold
    int32_t chrom0LowTH;       // R/G hue low threshold
    int32_t chrom0HighTH;      // R/G hue high threshold
    int32_t chrom1LowTH;       // B/G hue low threshold
    int32_t chrom1HighTH;      // B/G hue low threshold
    int32_t chrom0LowDiffTH;   // R/G hue difference low threshold
    int32_t chorm0HighDiffTH;  // R/G hue difference high threshold
    int32_t chrom1LowDiffTH;   // B/G hue difference low threshold
    int32_t chorm1HighDiffTH;  // B/G hue difference high threshold

#ifdef CAC_DEBUG
    uint32_t num0;
    uint32_t num1;
#endif


} cac_arg_t;


uint32_t cac_module( cac_arg_t * p_args);

uint32_t cac_module_init();

uint32_t cac_module_deinit();
#endif
