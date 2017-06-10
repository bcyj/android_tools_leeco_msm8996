/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef QTIBLUR_BLUR_H
#define QTIBLUR_BLUR_H

#include <stdlib.h>

namespace qtiblur {

typedef void* BLUR_TOKEN;

BLUR_TOKEN initBlurToken();
void releaseBlurToken(BLUR_TOKEN token);
bool blur(BLUR_TOKEN token,
        int blurness,
        uint32_t inputTexName,
        size_t inputTexWidth,
        size_t inputTexHeight,
        uint32_t outputTexName,
        size_t* pOutputTexWidth,
        size_t* pOutputTexHeight);


}

#endif
