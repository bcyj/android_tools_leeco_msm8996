/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef MM_COLOR_CONVERT_H_
#define MM_COLOR_CONVERT_H_

#include "sys/types.h"

//------------------------------------------
enum ColorConvertFormat {
    RGB565 = 1,
    YCbCr420Tile,
    YCbCr420SP128M,
    YCbCr420SP,
    YCbCr420P,
    YCrCb420P,
    YCrCb420YT,
};

/* 64 bit flag variable, reserving bits as needed */
enum ColorConvertFlags {
    COLOR_CONVERT_ALIGN_NONE = 1,
    COLOR_CONVERT_CENTER_OUTPUT = 1<<1,
    COLOR_CONVERT_ALIGN_16 =   1<<4,
    COLOR_CONVERT_ALIGN_2048 = 1<<11,
    COLOR_CONVERT_ALIGN_8192 = 1<<13,
};

struct ColorConvertParams {
    size_t width;
    size_t height;

    size_t cropWidth;
    size_t cropHeight;

    size_t cropLeft;
    size_t cropRight;
    size_t cropTop;
    size_t cropBottom;

    ColorConvertFormat colorFormat;
    const void * data;
    int fd;

    uint64_t flags;
};

typedef int (* convertFn)(ColorConvertParams src,
                          ColorConvertParams dst, uint8_t *adjustedClip);

extern "C" int convert(ColorConvertParams src,
                       ColorConvertParams dst, uint8_t *adjustedClip);

#endif  // MM_COLOR_CONVERT_H_
