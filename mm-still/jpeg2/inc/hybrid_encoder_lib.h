/*****************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/16/14   vgaur   Added multi-threaded fastCV flag.
12/09/13   csriniva Created File.
========================================================================== */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "jpege.h"
#include "jpegd.h"
#include "jpeg_buffer.h"
#include "os_thread.h"
#include "os_timer.h"
#include "jpeg_buffer_private.h"
#include "apr_pmem.h"


#if (defined WIN32 || defined WINCE)
#include "wingetopt.h"
#else
#include <getopt.h>
#endif

const char color_formats[][30] =
{
    "YCRCBLP_H2V2",         // 0
    "YCBCRLP_H2V2",         // 1
    "YCRCBLP_H2V1",         // 2
    "YCBCRLP_H2V1",         // 3
    "YCRCBLP_H1V2",         // 4
    "YCBCRLP_H1V2",         // 5
    "YCRCBLP_H1V1",         // 6
    "YCBCRLP_H1V1",         // 7
    "MONOCHROME",
    "RGB565",               // 8
    "RGB888",               // 9
    "RGBa",                 // 10
    "",                     // 11
    "JPEG_BITSTREAM_H2V2",  // 12
    "",                     // 13
    "JPEG_BITSTREAM_H2V1",  // 14
    "",                     // 15
    "JPEG_BITSTREAM_H1V2"   // 16
    "",                     // 17
    "JPEG_BITSTREAM_H1V1"   // 18
};
const char memory_formats[][6] =
{
    "NV12",
    "NV21",
    "IYUV",
    "YUV12"
};

const char event_to_string[4][30] =
{
    "EVENT_DONE",
    "EVENT_WARNING",
    "EVENT_ERROR",
    "EVENT_THUMBNAIL_DROPPED",
};

typedef struct
{
    char                *file_name;
    uint32_t             width;
    uint32_t             height;
    uint32_t             quality;
    jpeg_color_format_t  format;
    jpeg_memory_format_t memory_format;
    uint16_t             num_of_planes;

} input_image_args_t;

typedef struct
{
    input_image_args_t   main;
    input_image_args_t   thumbnail;
    char *               output_file;
    int16_t              rotation;
    uint32_t             preference;
    uint8_t              encode_thumbnail;
    uint16_t             back_to_back_count;
    uint32_t             target_filesize;
    uint32_t             output_buf_size;
    uint32_t             abort_time;
    const char*          reference_file;
    uint32_t             restart_interval;
    uint8_t              output_nowrite;
    uint8_t              use_pmem;
    jpege_scale_cfg_t    main_scale_cfg;
    jpege_scale_cfg_t    tn_scale_cfg;
    jpege_stride_cfg_t   main_stride_cfg;
    jpege_stride_cfg_t   tn_stride_cfg;
    uint8_t              fastCV_flag;
} test_args_t;

typedef struct
{
    void*           fout;
    os_mutex_t      mutex;
    os_cond_t       cond;
    uint32_t        size;
    uint8_t         nowrite;

} output_handler_args_t;

typedef struct
{
    int                   tid;
    os_thread_t           thread;
    jpege_obj_t           encoder;
    uint8_t               encoding;
    output_handler_args_t output_handler_args;
    test_args_t          *p_args;

} thread_ctrl_blk_t;

void encoder_event_handler(void         *p_user_data,
                           jpeg_event_t  event,
                           void         *p_arg);

int  encoder_output_handler(void           *p_user_data,
                            void           *p_arg,
                            jpeg_buffer_t   buffer,
                            uint8_t         last_buf_flag);
