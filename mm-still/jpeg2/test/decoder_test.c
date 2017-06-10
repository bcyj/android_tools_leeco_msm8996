/**
 * Copyright (C) 2008-11 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/18/10   mingy   Added getting JPS configuration.
11/18/09   ming    Added tiling output support.
11/03/09   mingy   Added CbCr output support.
09/21/09   mingy   Added region based decoding option.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
                   Client of the JPEG library should not be using
                   this layer normally (some header files are not public).
                   This is just for convenience to build an os_agnostic test
                   app.
06/23/09   zhiminl Replaced #ifdef _ANDROID_ with #ifdef ANDROID.
04/20/09   mingy   Added stride and rotation features.
03/05/09   leim    Added support for H1V1, H1V2, and aRGB format.
02/05/09   vma     Fixed transparency value in dumping images to fb.
10/09/08   vma     Added tests for back-to-back decoding and concurrency.
10/08/08   vma     Fixed abort deadlock.
09/16/08   vma     Added reference file comparison.
09/15/08   vma     Added Exif reading and aborting.
09/12/08   vma     Added upsampler support.
07/07/08   vma     Created file.

========================================================================== */

#define STRSAFE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include "jpeg_buffer.h"
#include "jpeg_common.h"
#include "jpege.h"
#include "jpegd.h"
#include "os_thread.h"
#include "os_timer.h"
#include "jpsd.h"

#if (defined WIN32  || defined WINCE)
#include "wingetopt.h"
#else
#include <getopt.h>
#endif

#ifdef SCREEN_DUMP_SUPPORTED
#include "linux/msm_mdp.h"
#include <linux/fb.h>

#include <memory.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>

#define OUTPUT_IS_SCREEN(output) (!strncmp(output, "screen", 7))
#else
#define OUTPUT_IS_SCREEN(...) 0
#endif

#define CMP_SIZE 1024

#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))

#define CLAMP(x,min,max) MIN(MAX((x),(min)),(max))

#define MAX_CONCURRENT_CNT (20)

const char color_formats[21][13] =
{
  "YCRCBLP_H2V2",
  "YCBCRLP_H2V2",
  "YCRCBLP_H2V1",
  "YCBCRLP_H2V1",
  "YCRCBLP_H1V2",
  "YCBCRLP_H1V2",
  "YCRCBLP_H1V1",
  "YCBCRLP_H1V1",
  "MONOCHROME",
  "RGB565",
  "RGB888",
  "RGBa",
  "IYUV_H2V2",
  "YUV2_H2V2",
  "IYUV_H2V1",
  "YUV2_H2V1",
  "IYUV_H1V2",
  "YUV2_H1V2",
  "IYUV_H1V1",
  "YUV2_H1V1",
};

typedef enum {
  TEST_COL_FMT_MIN = RGBa + 1,
  IYUV_H2V2 = TEST_COL_FMT_MIN,
  YUV2_H2V2,
  IYUV_H2V1,
  YUV2_H2V1,
  IYUV_H1V2,
  YUV2_H1V2,
  IYUV_H1V1,
  YUV2_H1V1,
  TEST_COL_FMT_MAX,
} test_color_fmt;
const char event_to_string[4][14] =
{
    "EVENT_DONE",
    "EVENT_WARNING",
    "EVENT_ERROR",
};
const char *preference_str[] =
{
    "Hardware accelerated preferred",
    "Hardware accelerated only",
    "Software based preferred",
    "Software based only",
};

static const float g_scale_factor[] =
  {1.0, 1.0/8.0, 2.0/8.0, 3.0/8.0, 4.0/8.0, 5.0/8.0, 6.0/8.0, 7.0/8.0};

typedef struct
{
    char      *input_file;
    char      *output_file;
    char      *thumbnail_file;
    char      *exif_dump_file;
    uint32_t   width;
    uint32_t   height;
    uint32_t   format;
    uint32_t   preference;
    uint32_t   abort_time;
    uint16_t   back_to_back_count;
    char      *reference_file;
    uint8_t    thumbnail_flag;
    uint32_t   stride;
    int32_t    rotation;
    jpeg_rectangle_t region;
    uint8_t    tiling_enabled;
    char       skip_writing_output_file;
    jpegd_scale_type_t scale_factor;
    uint8_t    num_planes;
    uint32_t   hw_rotation;
} test_args_t;

typedef struct
{
    int                   tid;
    os_thread_t           thread;
    jpegd_obj_t           decoder;
    FILE*                 fin;
    uint8_t               decoding;
    uint8_t               decode_success;
    os_mutex_t            mutex;
    os_cond_t             cond;
    test_args_t          *p_args;
    uint32_t              stride;
    jpegd_output_buf_t   *p_whole_output_buf;

} thread_ctrl_blk_t;

OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER decoder_test(OS_THREAD_FUNC_ARG_T p_thread_args);
void decoder_dump_output(char               *output_file,
                         jpegd_dst_t        *p_dst,
                         jpegd_output_buf_t *p_output_buffer,
                         float scale_ratio);
void decoder_event_handler(void        *p_user_data,
                           jpeg_event_t event,
                           void        *p_arg);
int decoder_output_handler(void               *p_user_data,
                           jpegd_output_buf_t *p_output_buffer,
                           uint32_t            first_row_id,
                           uint8_t             is_last_buffer);
int decoder_test_dump_exif(FILE*           dump_file,
                           exif_info_obj_t exif);
uint32_t decoder_input_req_handler(void           *p_user_data,
                                   jpeg_buffer_t   buffer,
                                   uint32_t        start_offset,
                                   uint32_t        length);

// Global variables
int concurrent_cnt = 1;
thread_ctrl_blk_t *thread_ctrl_blks = NULL;

void print_usage(void)
{
    fprintf(stderr, "Usage: program_name [options]\n");
    fprintf(stderr, "Mandatory options:\n");
    fprintf(stderr, "  -i FILE\t\tPath to the input file.\n");
    fprintf(stderr, "  -o FILE\t\tPath for the output file, use \"screen\" to indicate the decoded image is to be displayed on screen (linux targets only).\n");
    fprintf(stderr, "Optional:\n");
    fprintf(stderr, "  -t FILE\t\tPath for the thumbnail file\n");
    fprintf(stderr, "  -w WIDTH\t\tOutput width. (If unset, it will be automatically chosen based on the original dimension)\n");
    fprintf(stderr, "  -h HEIGHT\t\tOutput height. (If unset, it will be automatically chosen based on the original dimension)\n");
    fprintf(stderr, "  -f FORMAT\t\tOutput format: (YCRCBLP_H2V1 is not supported if the input is decoded using H2V2 subsampling.)\n");
    fprintf(stderr, "\t\t\t\tYCRCBLP_H2V2 (%d - Default), YCBCRLP_H2V2 (%d),"
                    " YCRCBLP_H2V1 (%d), YCBCRLP_H2V1 (%d),"
                    " YCRCBLP_H1V2 (%d), YCBCRLP_H1V2 (%d),"
                    " YCRCBLP_H1V1 (%d), YCBCRLP_H1V1 (%d),"
                    " MONOCHROME(%d), RGB565 (%d),"
                    " RGB888 (%d), RGBA (%d),"
                    " IYUV_H2V2 (%d), YUV2_H2V2 (%d),"
                    " IYUV_H2V1 (%d), YUV2_H2V1 (%d),"
                    " IYUV_H1V2 (%d), YUV2_H1V2 (%d),"
                    " IYUV_H1V1 (%d), YUV2_H1V1 (%d)\n",
                    YCRCBLP_H2V2, YCBCRLP_H2V2, YCRCBLP_H2V1, YCBCRLP_H2V1,
                    YCRCBLP_H1V2, YCBCRLP_H1V2, YCRCBLP_H1V1, YCBCRLP_H1V1,
                    MONOCHROME, RGB565, RGB888, RGBa,
                    IYUV_H2V2, YUV2_H2V2, IYUV_H2V1, YUV2_H2V1,
                    IYUV_H1V2, YUV2_H1V2, IYUV_H1V1, YUV2_H1V1);
    fprintf(stderr, "  -p PREFERENCE\t\tPreference on which decoder to use (Software-based or Hardware-accelerated).\n");
    fprintf(stderr, "               \t\t\tHW preferred (0), HW only (1), SW preferred (2), SW only (3)\n");
    fprintf(stderr, "  -x FILE\t\tPath to the output text file which contains a dump of the EXIF tags found in the file.\n");
    fprintf(stderr, "  -a ABORT_TIME\t\tThe duration before an abort is issued (in milliseconds).\n");
    fprintf(stderr, "  -c COUNT\t\tRe-decode back-to-back 'COUNT' times. Default is 1.\n");
    fprintf(stderr, "  -e CONCURRENT_COUNT\tRun COUNT tests in concurrent threads (test for multiple instances of encoder within same process).\n");
    fprintf(stderr, "  -g GOLDEN_REFERENCE\tThe reference file to be matched against the output.\n");
    fprintf(stderr, "  -s STRIDE\t\tSpecify the stride for the output.\n");
    fprintf(stderr, "  -r ROTATION\t\tSpecify the rotation degrees requested (90, 180, 270).\n");
    fprintf(stderr, "  -L REGION LEFT\t\tSpecify the region left coordinate (must be a even #).\n");
    fprintf(stderr, "  -T REGION TOP\t\tSpecify the region top coordinate (must be a even #).\n");
    fprintf(stderr, "  -R REGION RIGHT\t\tSpecify the region right coordinate.\n");
    fprintf(stderr, "  -B REGION BOTTOM\t\tSpecify the region bottom coordinate.\n");
    fprintf(stderr, "  -z SKIP OUTPUT FILE\t\tSpecify whether writing to output file can be skipped.");
    fprintf(stderr, "  -l TILING ENABLED\t\tSpecify if the tiling is enabled. 1 to enalble, 0 to disable(default) )\n");
    fprintf(stderr, "  -S SCALE\t\tSpecify the scale factor 0. No scale(default) 1.1/8 2.2/8 3.3/8 4.4/8 5.5/8 6.6/8 7.7/8\n");
    fprintf(stderr, "\n");
}

int main(int argc, char* argv[])
{
    int rc, c, i;
    test_args_t  test_args;
    memset(&test_args, 0, sizeof(test_args_t));
    test_args.preference = JPEG_DECODER_PREF_HW_ACCELERATED_PREFERRED;
    test_args.back_to_back_count = 1;

    fprintf(stderr, "=============================================================\n");
    fprintf(stderr, "Decoder test\n");
    fprintf(stderr, "=============================================================\n");
    opterr = 1;
    while ((c = getopt(argc, argv, "i:o:t:w:h:f:p:x:a:g:c:e:s:r:L:T:R:B:S:l"))
      != -1)
    {
        switch (c)
        {
        case 'i':
            test_args.input_file = optarg;
            fprintf(stderr, "%-25s%s\n", "Input file path", test_args.input_file);
            break;
        case 'o':
            test_args.output_file = optarg;
            fprintf(stderr, "%-25s%s\n", "Output file path", test_args.output_file);
            break;
        case 't':
            test_args.thumbnail_file = optarg;
            test_args.thumbnail_flag = 1;
            fprintf(stderr, "%-25s%s\n", "Thumbnail file path", test_args.thumbnail_file);
            break;
        case 'w':
            test_args.width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Output width", test_args.width);
            break;
        case 'h':
            test_args.height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Output height", test_args.height);
            break;
        case 'f': {
            uint32_t test_format;
            test_args.format = atoi(optarg);
            test_format = test_args.format;
            if (test_args.format == MONOCHROME) {
              test_args.num_planes = 1;
            } else if ((test_args.format >= TEST_COL_FMT_MIN) &&
              (test_args.format < TEST_COL_FMT_MAX)) {
              test_args.num_planes = 3;
              test_args.format -= TEST_COL_FMT_MIN;
            } else {
              test_args.num_planes = 2;
            }
            if (test_args.format != YCRCBLP_H2V2 && test_args.format != YCBCRLP_H2V2 &&
                test_args.format != YCRCBLP_H2V1 && test_args.format != YCBCRLP_H2V1 &&
                test_args.format != YCRCBLP_H1V2 && test_args.format != YCBCRLP_H1V2 &&
                test_args.format != YCRCBLP_H1V1 && test_args.format != YCBCRLP_H1V1 &&
                test_args.format != RGB565 &&
                test_args.format != RGB888 &&
                test_args.format != RGBa &&
                test_args.format != MONOCHROME)
            {
                fprintf(stderr, "Invalid output format.\n");
                return 1;
            }
            fprintf(stderr, "%-25s%s\n", "Output format", color_formats[test_format]);
            break;
        }
        case 'p':
            test_args.preference = atoi(optarg);
            if (test_args.preference >= JPEG_DECODER_PREF_MAX)
            {
                fprintf(stderr, "Invalid decoder preference\n");
                return 1;
            }
            fprintf(stderr, "%-25s%s\n", "Decoder preference", preference_str[test_args.preference]);
            break;
        case 'x':
            test_args.exif_dump_file = optarg;
            fprintf(stderr, "%-25s%s\n", "Exif dump file", optarg);
            break;
        case 'a':
            test_args.abort_time = atoi(optarg);
            fprintf(stderr, "%-25s%d ms\n", "Abort time", test_args.abort_time);
            break;
        case 'c':
            test_args.back_to_back_count = atoi(optarg);
            if (test_args.back_to_back_count == 0)
            {
                fprintf(stderr, "Invalid count: -c %d\n", test_args.back_to_back_count);
                return 1;
            }
            break;
        case 'e':
            concurrent_cnt = atoi(optarg);
            if ((concurrent_cnt < 0) && (concurrent_cnt > MAX_CONCURRENT_CNT))
            {
                fprintf(stderr, "Invalid concurrent count: 0\n");
                return 1;
            }
            fprintf(stderr, "%-25s%d\n", "Concurrent count", concurrent_cnt);
            break;
        case 'g':
            test_args.reference_file = optarg;
            fprintf(stderr, "%-25s%s\n", "Reference file", test_args.reference_file);
            break;
        case 's':
            test_args.stride = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Stride", test_args.stride);
            break;
        case 'r':
            test_args.rotation = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Rotation", test_args.rotation);
            break;
        case 'L':
            test_args.region.left = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Region Left", test_args.region.left);
            break;
        case 'T':
            test_args.region.top = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Region Top", test_args.region.top);
            break;
        case 'R':
            test_args.region.right = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Region Right", test_args.region.right);
            break;
        case 'B':
            test_args.region.bottom = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Region Bottom", test_args.region.bottom);
            break;
        case 'S':
            test_args.scale_factor = atoi(optarg);
            test_args.scale_factor = CLAMP(test_args.scale_factor, 0, 7);
            fprintf(stderr, "%-25s%d/%d\n", "Scale", test_args.scale_factor, 8);
            break;
        case 'z':

        case 'l':
            test_args.tiling_enabled = atoi(optarg);
            if (test_args.tiling_enabled)
            {
                fprintf(stderr, "%-25s%s\n", "Tiling Enalbed?", "Yes");
            }
            else
            {
                fprintf(stderr, "%-25s%s\n", "Tiling Enalbed?", "No");
            }
            break;
        }
    }

    // Double check all the required arguments are set
    if (!test_args.input_file || !test_args.output_file)
    {
        fprintf(stderr, "Missing required arguments.\n");
        print_usage();
        return 1;
    }

    // check the formats
    if (((test_args.format == YCRCBLP_H1V2) || (test_args.format == YCBCRLP_H1V2) ||
      (test_args.format == YCRCBLP_H1V1) || (test_args.format == YCBCRLP_H1V1)) &&
      !(test_args.preference == JPEG_DECODER_PREF_HW_ACCELERATED_ONLY)) {
        fprintf(stderr, "These formats are not supported by SW format %d\n", test_args.format);
        return 1;
    }

    // Create thread control blocks
    thread_ctrl_blks = (thread_ctrl_blk_t *)malloc(concurrent_cnt * sizeof(thread_ctrl_blk_t));
    if (!thread_ctrl_blks)
    {
        fprintf(stderr, "decoder_test failed: insufficient memory in creating thread control blocks\n");
        return 1;
    }
    memset(thread_ctrl_blks, 0, concurrent_cnt * sizeof(thread_ctrl_blk_t));
    // Initialize the blocks and kick off the threads
    for (i = 0; i < concurrent_cnt; i++)
    {
        thread_ctrl_blks[i].tid = i;
        thread_ctrl_blks[i].p_args = &test_args;
        os_mutex_init(&thread_ctrl_blks[i].mutex);
        os_cond_init(&thread_ctrl_blks[i].cond);
        if (os_thread_create(&thread_ctrl_blks[i].thread, decoder_test, &thread_ctrl_blks[i]))
        {
            fprintf(stderr, "decoder_test: os_create failed\n");
            return 1;
        }
    }

    rc = 0;
    // Join the threads
    for (i = 0; i < concurrent_cnt; i++)
    {
        OS_THREAD_FUNC_RET_T ret;
        os_thread_join(&thread_ctrl_blks[i].thread, &ret);
        if (ret)
        {
            fprintf(stderr, "decoder_test: thread %d failed\n", i);
            rc = (int)OS_THREAD_FUNC_RET_FAILED;
        }
    }

    if (!rc)
        fprintf(stderr, "decoder_test finished successfully\n");

    fprintf(stderr, "exit value: %d\n", rc);
    return rc;
}

OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER decoder_test(OS_THREAD_FUNC_ARG_T arg)
{
    char *output_filename;
    int del_out_file_name = 0;
    int rc, i;
    jpegd_obj_t decoder;
    jpegd_src_t source;
    jpegd_dst_t dest;
    jpegd_output_buf_t  p_output_buffers;
    uint32_t            output_buffers_count = 1; // currently only 1 buffer a time is supported
    jpegd_dst_t         dest2;
    jpegd_output_buf_t  p_output_buffers2;
    jpegd_cfg_t config;
    jpeg_hdr_t header;
    unsigned long file_size;
    FILE* fin;
    uint8_t use_pmem = true;
    os_timer_t os_timer;
    thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)arg;
    test_args_t *p_args = p_thread_arg->p_args;
    uint32_t stride;
    uint32_t            output_width;
    uint32_t            output_height;
    uint8_t             tile_enable_copy = false;
    jps_cfg_3d_t        jps_config;
    uint32_t total_time = 0;
    int outfn_on_heap = false;

    // Define a final image buffer for the tiling enabled cases.
    // This buffer holds the whole image copied line by line from the tiling buffer
    jpegd_output_buf_t  p_final_image_buf;


    // Determine whether pmem should be used (useful for pc environment testing where
    // pmem is not available)
    if ((jpegd_preference_t)p_args->preference == JPEG_DECODER_PREF_SOFTWARE_PREFERRED ||
        (jpegd_preference_t)p_args->preference == JPEG_DECODER_PREF_SOFTWARE_ONLY) {
        use_pmem = false;
    }

    if (((jpegd_preference_t)p_args->preference !=
      JPEG_DECODER_PREF_HW_ACCELERATED_ONLY) &&
      ((jpegd_preference_t)p_args->scale_factor > 0)) {
        fprintf(stderr, "Setting scale factor to 1x");
    }
    // Append the output file name with a number to avoid multiple writing to the same file
    output_filename = p_args->output_file;
    if (p_thread_arg->tid && !OUTPUT_IS_SCREEN(p_args->output_file)) {
        // Look for the last occurence of '/' and then last occurence of '.'
        char *s = strrchr(p_args->output_file, '/');
        if (s) {
            s = strrchr(s, '.');
        } else {
            s = strrchr(p_args->output_file, '.');
        }
        output_filename = (char *)malloc(5 + strlen(p_args->output_file));
        if (output_filename) {
            outfn_on_heap = true;
            snprintf(output_filename, sizeof(output_filename), "%s", p_args->output_file);
            if (s)
            {
                snprintf(output_filename + (s - p_args->output_file), sizeof(output_filename), "_%.2d%s",
                         p_thread_arg->tid, s);
            }
            else
            {
                snprintf(output_filename, sizeof(output_filename), "%s_%.2d", output_filename,
                         p_thread_arg->tid);
            }
        }
    }

    // Open input file
    fin = fopen(p_args->input_file, "rb");
    if (!fin) {
        goto fail;
    }
    // Find out input file length
    fseek(fin, 0, SEEK_END);
    file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    if (file_size > 0xffffffff) {
        fprintf(stderr, "decoder_test: input file too large for decoder\n");
        goto fail;
    }
    // Initialize decoder
    rc = jpegd_init(&decoder,
                    &decoder_event_handler,
                    &decoder_output_handler,
                    p_thread_arg);

    if (JPEG_FAILED(rc)) {
        fprintf(stderr, "decoder_test: jpegd_init failed\n");
        goto fail;
    }
    p_thread_arg->decoder = decoder;

    p_thread_arg->fin = fin;

    // Set source information
    source.p_input_req_handler = &decoder_input_req_handler;
    source.total_length        = file_size & 0xffffffff;
    rc = jpeg_buffer_init(&source.buffers[0]);
    if (JPEG_SUCCEEDED(rc)) {
        rc = jpeg_buffer_init(&source.buffers[1]);
    }
    if (JPEG_SUCCEEDED(rc)) {
        rc = jpeg_buffer_allocate(source.buffers[0], 0xA000, use_pmem);
    }
    if (JPEG_SUCCEEDED(rc)) {
        rc = jpeg_buffer_allocate(source.buffers[1], 0xA000, use_pmem);
    }
    if (JPEG_FAILED(rc)) {
        jpeg_buffer_destroy(&source.buffers[0]);
        jpeg_buffer_destroy(&source.buffers[1]);
        goto fail;
    }

   fprintf(stderr, "--(%d)%s() *** Starting back-to-back decoding of %d frame(s)***\n",
                 __LINE__, __func__, p_args->back_to_back_count);

	 // Loop to perform n back-to-back decoding (to the same output file)
    for(i = 0; i < p_args->back_to_back_count; i++) {
        if(os_timer_start(&os_timer) < 0) {
            fprintf(stderr, "decoder_test: failed to get start time\n");
        }

        rc = jpegd_set_source(decoder, &source);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "decoder_test: jpegd_set_source failed\n");
            goto fail;
        }

        rc = jpegd_read_header(decoder, &header);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "decoder_test: jpegd_read_header failed\n");
            goto fail;
        }
        fprintf(stderr, "main dimension: (%dx%d) subsampling: (%d)\n",
                header.main.width, header.main.height, (int)header.main.subsampling);

    // Dump exif tag information if requested
#ifdef DEC_DUMP_EXIF
        if(p_args->exif_dump_file && p_thread_arg->tid == 0) {
            FILE *exif_dump = fopen(p_args->exif_dump_file, "w");
            if(!exif_dump) {
                fprintf(stderr, "decoder_test: failed to open file for dumping exif tags: %s\n",
                        p_args->exif_dump_file);
            }
            else {
                decoder_test_dump_exif(exif_dump, header.exif_info);
                fclose(exif_dump);
            }
        }
#endif
        rc = jpsd_get_config(decoder, &jps_config);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "decoder_test: jpsd_get_config failed\n");
            goto fail;
        }
        fprintf(stderr, "JPS sturcture: \nlayout = %d\nheight_flag = %d  width_flag = %d\nfield order = %d\nSeparation =  %d\n",
                jps_config.layout, jps_config.height_flag, jps_config.width_flag, jps_config.field_order, jps_config.separation);

        //Disable tiling temporarily for thumbnail decoding
        if (1 == p_args->tiling_enabled && 1 == p_args->thumbnail_flag)
        {
            p_args->tiling_enabled = 0;
            tile_enable_copy = true;
        }

 #ifdef DEC_THUMB
        // if thumbnail file is specified, decode thumbnail first.
        if(1 == p_args->thumbnail_flag) {
            // check if there is an thumbnail
            if(header.thumbnail.height != 0 && header.thumbnail.width != 0) {
                // thumbnail output configuration
                dest2.width         = header.thumbnail.width;
                dest2.height        = header.thumbnail.height;
                dest2.output_format = RGB888;
                dest2.stride        = 0;

                dest2.region.left   = 0;
                dest2.region.top    = 0;
                dest2.region.right  = 0;
                dest2.region.bottom = 0;

                jpeg_buffer_init(&p_output_buffers2.data.rgb.rgb_buf);

                // Assign 0 to tile width and height
                // to indicate that no tiling is requested for thumbnail.
                p_output_buffers2.tile_width  = 0;
                p_output_buffers2.tile_height = 0;

                stride = dest2.width * 3;

                // output buffer allocation
                use_pmem = OUTPUT_IS_SCREEN(p_args->thumbnail_file);
                jpeg_buffer_allocate(p_output_buffers2.data.rgb.rgb_buf,
                        stride * dest2.height, use_pmem);

                // config to decoder to decode thumbnail.
                memset(&config, 0, sizeof(jpegd_cfg_t));
                config.preference = p_args->preference;
                config.decode_from = JPEGD_DECODE_FROM_THUMB;
                config.rotation = 0;

                // start decoding
                p_thread_arg->decoding = true;
                if(os_timer_start(&os_timer) < 0) {
                    fprintf(stderr, "decoder_test thumbnail: failed to get start time\n");
                }

                rc = jpegd_start(decoder, &config, &dest2, &p_output_buffers2, 1);

                if(JPEG_FAILED(rc)) {
                    fprintf(stderr, "decoder_test thumnail: jpegd_start failed\n");
                    goto fail;
                }
                fprintf(stderr, "decoder_test thumbnail: jpegd_start succeeded\n");

                // Wait until decoding is done or stopped due to error
                os_mutex_lock(&p_thread_arg->mutex);
                while(p_thread_arg->decoding) {
                    os_cond_wait(&p_thread_arg->cond, &p_thread_arg->mutex);
                }
                os_mutex_unlock(&p_thread_arg->mutex);

                // check thumbnail decoding results
                {
                    int diff;
                    if(os_timer_get_elapsed(&os_timer, &diff, 0) < 0) {
                        fprintf(stderr, "decoder_test: failed to get elapsed time\n");
                    }
                    else {
                        if(p_thread_arg->decode_success) {
                            fprintf(stderr, "decoder_test1: decode time: %d ms\n", diff);
                        }
                        else {
                            fprintf(stderr, "decoder_test: decode failed\n");
                        }

                        // Dump output under either of the conditions:
                        // 1. Output is to the screen AND it is the first thread
                        // 2. Output is to files
                        if(!OUTPUT_IS_SCREEN(p_args->thumbnail_file) || p_thread_arg->tid == 0) {
                            decoder_dump_output(p_args->thumbnail_file,
                                    &dest2, &p_output_buffers2, 1.0);
                        }
                    }
                }
                // clean up buffers
                jpeg_buffer_destroy(&p_output_buffers2.data.rgb.rgb_buf);

                if(!p_thread_arg->decode_success) {
                    fprintf(stderr, "decoder_test: thumbnail decode failed\n");
                    goto fail;
                }
            }
            else {
                fprintf(stderr, "decoder_test: Jpeg file does not contain thumbnail \n");
            }
        }
#endif

        //Re-enable tiling if it was disabled above for thumbnail decoding
        if (tile_enable_copy)
        {
            p_args->tiling_enabled = 1;
        }

        // main image decoding:
        // Set destination information
        dest.width = (p_args->width) ? (p_args->width) : header.main.width;
        dest.height = (p_args->height) ? (p_args->height) : header.main.height;
        dest.output_format = p_args->format;
        dest.stride = p_args->stride;
        dest.region = p_args->region;

        // if region is defined, re-assign the output width/height
        output_width  = dest.width;
        output_height = dest.height;

        if (p_args->region.right || p_args->region.bottom)
        {
            if (0 == p_args->rotation || 180 == p_args->rotation)
            {
                output_width  = MIN((dest.width),
                        (uint32_t)(dest.region.right  - dest.region.left + 1));
                output_height = MIN((dest.height),
                        (uint32_t)(dest.region.bottom - dest.region.top  + 1));
            }
            // Swap output width/height for 90/270 rotation cases
            else if (90 == p_args->rotation || 270 == p_args->rotation)
            {
                output_height  = MIN((dest.height),
                        (uint32_t)(dest.region.right  - dest.region.left + 1));
                output_width   = MIN((dest.width),
                        (uint32_t)(dest.region.bottom - dest.region.top  + 1));
            }
            // Unsupported rotation cases
            else
            {
                goto fail;
            }
        }

        if (dest.output_format == YCRCBLP_H2V2 || dest.output_format == YCBCRLP_H2V2 ||
            dest.output_format == YCRCBLP_H2V1 || dest.output_format == YCBCRLP_H2V1 ||
            dest.output_format == YCRCBLP_H1V2 || dest.output_format == YCBCRLP_H1V2 ||
            dest.output_format == YCRCBLP_H1V1 || dest.output_format == YCBCRLP_H1V1) {
            jpeg_buffer_init(&p_output_buffers.data.yuv.luma_buf);
            jpeg_buffer_init(&p_output_buffers.data.yuv.chroma_buf);
        } else if (dest.output_format == MONOCHROME) {
            jpeg_buffer_init(&p_output_buffers.data.yuv.luma_buf);
        } else {
            jpeg_buffer_init(&p_output_buffers.data.rgb.rgb_buf);

            // Init the final image buffer only when tiling is enabled
            if (p_args->tiling_enabled)
            {
                jpeg_buffer_init(&p_final_image_buf.data.rgb.rgb_buf);
            }
        }
        if(output_filename)
            use_pmem = OUTPUT_IS_SCREEN(output_filename);

        float buf_factor = 1.5;
        if (dest.stride == 0) {
            switch (dest.output_format)
            {
            case YCRCBLP_H2V2:
            case YCBCRLP_H2V2:
                stride = output_width;
                buf_factor = 1.5;
                break;

            case YCRCBLP_H2V1:
            case YCBCRLP_H2V1:
                stride = output_width;
                buf_factor = 2.0;
                break;

            case YCRCBLP_H1V2:
            case YCBCRLP_H1V2:
                stride = output_width;
                buf_factor = 2.0;
                break;

            case YCRCBLP_H1V1:
            case YCBCRLP_H1V1:
                stride = output_width;
                buf_factor = 3.0;
                break;

            case MONOCHROME:
                stride = output_width;
                break;

            case RGB565:
                stride = output_width * 2;
                break;

            case RGB888:
                stride = output_width * 3;
                break;

            case RGBa:
                stride = output_width * 4;
                break;

            default:
                goto fail;
            }
        } else {
            stride = dest.stride;
        }

        // assign the stride for the decoder_output_handler call back
        p_thread_arg->stride = stride;

        if (p_args->tiling_enabled)
        {
            fprintf(stderr, "decoder_test: tile enabled \n");
            p_output_buffers.tile_width  = stride;
            p_output_buffers.tile_height = 1;
        } else {
            // Assign 0 to tile width and height
            // to indicate that no tiling is requested.
            p_output_buffers.tile_width  = 0;
            p_output_buffers.tile_height = 0;
        }
        p_output_buffers.is_in_q = 0;

        switch (dest.output_format)
        {
        case YCRCBLP_H2V2:
        case YCBCRLP_H2V2:
        case YCRCBLP_H2V1:
        case YCBCRLP_H2V1:
        case YCRCBLP_H1V2:
        case YCBCRLP_H1V2:
        case YCRCBLP_H1V1:
        case YCBCRLP_H1V1:
            jpeg_buffer_allocate(p_output_buffers.data.yuv.luma_buf, stride * dest.height * buf_factor, use_pmem);
            jpeg_buffer_attach_existing(p_output_buffers.data.yuv.chroma_buf,
               p_output_buffers.data.yuv.luma_buf, stride * output_height);
            break;
        case MONOCHROME:
            jpeg_buffer_allocate(p_output_buffers.data.yuv.luma_buf, stride * dest.height, use_pmem);
            break;
        case RGB565:
        case RGB888:
        case RGBa:
            if (p_args->tiling_enabled)
            {
                // in case tiling is enabled, allocated the output buffer as the tiling buffer(holds 1 rgb line)
                // also allocate the final image buffer to hold the whole image to be dumped
                jpeg_buffer_allocate(p_output_buffers.data.rgb.rgb_buf, stride * p_output_buffers.tile_height, use_pmem);
                jpeg_buffer_allocate(p_final_image_buf.data.rgb.rgb_buf, stride * output_height, use_pmem);
            }
            else
            {
                // if no tiling is requested, allocate the output buffer to hold the whole image to be dumped
                jpeg_buffer_allocate(p_output_buffers.data.rgb.rgb_buf, stride * output_height, use_pmem);
            }
            break;

        default:
            fprintf(stderr, "decoder_test: unsupported output format\n");
            goto fail;
        }

        // Set up configuration
        memset(&config, 0, sizeof(jpegd_cfg_t));
        config.preference = p_args->preference;
        config.decode_from = JPEGD_DECODE_FROM_AUTO;
        config.rotation = p_args->rotation;
        config.scale_factor = p_args->scale_factor;
        config.hw_rotation = p_args->hw_rotation;
        config.num_planes = p_args->num_planes;
        dest.back_to_back_count = p_args->back_to_back_count;

        if(p_args->back_to_back_count > 1) {
            fprintf(stderr, "Decoding (%d-th time)\n", i+1);
        }

        // in case tiling is enabled, assign the final image buffer
        // to the thread arg for decoder_output_handler to call back
        if (p_args->tiling_enabled)
        {
            p_thread_arg->p_whole_output_buf = &p_final_image_buf;
        }

        // Start decoding
        p_thread_arg->decoding = true;

        rc = jpegd_start(decoder, &config, &dest, &p_output_buffers, output_buffers_count);
        dest.back_to_back_count--;

        if(JPEG_FAILED(rc)) {
            fprintf(stderr, "--(%d)%s()  decoder_test: jpegd_start failed (rc=%d)\n",
                    __LINE__, __func__, rc);
            goto fail;
        }

        fprintf(stderr, "decoder_test: jpegd_start succeeded\n");

        // Do abort
        if (p_args->abort_time) {
            os_mutex_lock(&p_thread_arg->mutex);
            while (p_thread_arg->decoding)
            {
                rc = os_cond_timedwait(&p_thread_arg->cond, &p_thread_arg->mutex, p_args->abort_time);
                if (rc == JPEGERR_ETIMEDOUT)
                {
                    // Do abort
                    os_mutex_unlock(&p_thread_arg->mutex);
                    rc = jpegd_abort(decoder);
                    if (rc)
                    {
                        fprintf(stderr, "decoder_test: jpegd_abort failed: %d\n", rc);
                        goto fail;
                    }
                    break;
                }
            }
            if (p_thread_arg->decoding)
                os_mutex_unlock(&p_thread_arg->mutex);
        } else {
            // Wait until decoding is done or stopped due to error
            os_mutex_lock(&p_thread_arg->mutex);
            while (p_thread_arg->decoding)
            {
                os_cond_wait(&p_thread_arg->cond, &p_thread_arg->mutex);
            }
            os_mutex_unlock(&p_thread_arg->mutex);
        }

        int diff;
        // Display the time elapsed
        if (os_timer_get_elapsed(&os_timer, &diff, 0) < 0) {
            fprintf(stderr, "decoder_test: failed to get elapsed time\n");
        } else {
            if(p_args->abort_time) {
                if(p_thread_arg->decoding) {
                    fprintf(stderr, "decoder_test: decoding aborted successfully after %d ms\n", diff);
                    goto buffer_clean_up;
                }
                else
                {
                    fprintf(stderr, "decoder_test: decoding stopped before abort is issued. "
                                    "decode time: %d ms\n", diff);
                }
            }
            else {
                if(p_thread_arg->decode_success) {
                    total_time += diff;
                    fprintf(stderr, "decode time: %d ms (%d frame(s), total=%dms, avg=%dms/frame)\n\n",
                            diff, i+1, total_time, total_time/(i+1));
                }
                else
                {
                    fprintf(stderr, "decoder_test: decode failed\n");
                }
            }
            // Dump output under either of the conditions:
            // 1. Output is to the screen AND it is the first thread
            // 2. Output is to files
            if(output_filename) {
                    if(!OUTPUT_IS_SCREEN(output_filename) || p_thread_arg->tid == 0) {
                        if(!p_args->skip_writing_output_file) {
                            if(p_args->tiling_enabled) {
                                decoder_dump_output(output_filename, &dest, &p_final_image_buf, 1.0);
                            }
                            else
                            {
                                decoder_dump_output(output_filename, &dest, &p_output_buffers,
                                  g_scale_factor[p_args->scale_factor]);
                            }
                        }
                    }
            }
        }
    }

    if(p_thread_arg->decode_success) {
        fprintf(stderr, "Frame(s) = %d, Total Time = %dms, Avg. decode time = %dms/frame)\n",
                 p_args->back_to_back_count, total_time, total_time/p_args->back_to_back_count);
    }

buffer_clean_up:
    // Clean up decoder and allocate buffers
    jpeg_buffer_destroy(&source.buffers[0]);
    jpeg_buffer_destroy(&source.buffers[1]);
    switch (dest.output_format)
    {
    case YCRCBLP_H2V2:
    case YCBCRLP_H2V2:
    case YCRCBLP_H2V1:
    case YCBCRLP_H2V1:
    case YCRCBLP_H1V2:
    case YCBCRLP_H1V2:
    case YCRCBLP_H1V1:
    case YCBCRLP_H1V1:
        jpeg_buffer_destroy(&p_output_buffers.data.yuv.luma_buf);
        jpeg_buffer_destroy(&p_output_buffers.data.yuv.chroma_buf);
        break;
    case MONOCHROME:
        jpeg_buffer_destroy(&p_output_buffers.data.yuv.luma_buf);
        break;
    case RGB565:
    case RGB888:
    case RGBa:
        jpeg_buffer_destroy(&p_output_buffers.data.rgb.rgb_buf);
        if (p_args->tiling_enabled)
        {
            jpeg_buffer_destroy(&p_final_image_buf.data.rgb.rgb_buf);
        }
        break;
    default:
        break;
    }
    jpegd_destroy(&decoder);

    // Do bitwise comparison if requested (Output is not to the screen)
    if (p_args->reference_file && !OUTPUT_IS_SCREEN(p_args->output_file))
    {
        char *buf1;
        char *buf2;
        FILE *fref = fopen(p_args->reference_file, "rb");
        FILE *fout = fopen(p_args->output_file, "rb");
        int bytes_read1, bytes_read2;

        fprintf(stderr, "decoder_test: comparing %s with %s...\n",
                p_args->output_file, p_args->reference_file);
        buf1 = malloc(CMP_SIZE);
        if (!buf1)
        {
            fprintf(stderr, "decoder_test: failed to malloc for buffer for file comparison\n");
            goto fail;
        }
        buf2 = malloc(CMP_SIZE);
        if (!buf2)
        {
            fprintf(stderr, "decoder_test: failed to malloc for buffer for file comparison\n");
            free(buf1);
            goto fail;
        }

        if (!fref)
        {
            fprintf(stderr, "decoder_test: error opening reference file: %s\n", p_args->reference_file);
            free(buf1);
            free(buf2);
            goto fail;
        }

        if (!fout)
        {
          if (fref)
            fclose(fref);

          fprintf(stderr, "decoder_test: error opening reference file: %s\n",
              p_args->reference_file);
          free(buf1);
          free(buf2);
          goto fail;
        }

        for (;;)
        {
            bytes_read1 = (int)fread(buf1, 1, CMP_SIZE, fref);
            bytes_read2 = (int)fread(buf2, 1, CMP_SIZE, fout);
            if (bytes_read1 != bytes_read2 || memcmp(buf1, buf2, bytes_read1))
            {
                fprintf(stderr, "decoder_test: the two files differ.\n");
                free(buf1);
                free(buf2);
                goto fail;
            }
            if (feof(fref) && feof(fout))
            {
                fprintf(stderr, "decoder_test: output matches the reference file.\n");
                break;
            }
            if (feof(fref) || feof(fout) || ferror(fref) || ferror(fout))
            {
                fprintf(stderr, "decoder_test: the two files differ.\n");
                free(buf1);
                free(buf2);
                goto fail;
            }
        }
        free(buf1);
        free(buf2);
        fclose(fref);
        fclose(fout);
    }
    fclose(fin);


    if (!p_thread_arg->decode_success)
        goto fail;
    if(outfn_on_heap && output_filename)
        free(output_filename);
    return OS_THREAD_FUNC_RET_SUCCEEDED;
fail:
    if(outfn_on_heap && output_filename)
        free(output_filename);
    return OS_THREAD_FUNC_RET_FAILED;
}

void decoder_event_handler(void        *p_user_data,
                           jpeg_event_t event,
                           void        *p_arg)
{
    thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)p_user_data;

    fprintf(stderr, "decoder_event_handler: %s\n", event_to_string[event]);
    if (event == JPEG_EVENT_DONE)
    {
        p_thread_arg->decode_success = true;
    }
    // If it is not a warning event, decoder has stopped; Signal
    // main thread to clean up
    if (event != JPEG_EVENT_WARNING)
    {
        os_mutex_lock(&p_thread_arg->mutex);
        p_thread_arg->decoding = false;
        os_cond_signal(&p_thread_arg->cond);
        os_mutex_unlock(&p_thread_arg->mutex);
    }
}

void decoder_output_buf_handler_t(void *p_user_data, uint8_t *pl_addr[3], uint32_t pl_len[3])
{
    uint8_t* whole_output_buf_ptr, *tiling_buf_ptr;

    thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)p_user_data;
    // copy one line from the tiling buffer to the final output buffer
    memcpy(whole_output_buf_ptr, tiling_buf_ptr, p_thread_arg->stride);

}

// consumes the output buffer.
int decoder_output_handler(void *p_user_data,
                           jpegd_output_buf_t *p_output_buffer,
                           uint32_t first_row_id,
                           uint8_t is_last_buffer)
{
    uint8_t* whole_output_buf_ptr, *tiling_buf_ptr;

    thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)p_user_data;

    jpeg_buffer_get_addr(p_thread_arg->p_whole_output_buf->data.rgb.rgb_buf, &whole_output_buf_ptr);
    jpeg_buffer_get_addr(p_output_buffer->data.rgb.rgb_buf, &tiling_buf_ptr);

    // copy one line from the tiling buffer to the final output buffer
    memcpy(whole_output_buf_ptr + first_row_id * p_thread_arg->stride,
           tiling_buf_ptr, p_thread_arg->stride);

    if (p_output_buffer->tile_height != 1)
        return JPEGERR_EUNSUPPORTED;

    // testing purpose only
    // This is to simulate that the user needs to bail out when error happens
    // in the middle of decoding
    //if (first_row_id == 162)
     //   return JPEGERR_EFAILED;

    // do not enqueue any buffer if it reaches the last buffer
    if (!is_last_buffer)
    {
        jpegd_enqueue_output_buf(p_thread_arg->decoder, p_output_buffer, 1);
    }

    return JPEGERR_SUCCESS;
}

//      p_reader->p_input_req_handler(p_reader->decoder,
//                                    p_reader->p_input_buf,
//                                    p_reader->next_byte_offset,
//                                    MAX_BYTES_TO_FETCH);

uint32_t decoder_input_req_handler(void           *p_user_data,
                                   jpeg_buffer_t   buffer,
                                   uint32_t        start_offset,
                                   uint32_t        length)
{
    uint32_t buf_size;
    uint8_t *buf_ptr;
    int bytes_to_read, bytes_read, rc;
    thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)p_user_data;
    FILE *fd = (FILE *)p_thread_arg->fin;

    jpeg_buffer_get_max_size(buffer, &buf_size);
    jpeg_buffer_get_addr(buffer, &buf_ptr);
    bytes_to_read = (length < buf_size) ? length : buf_size;
    bytes_read = 0;
    rc = fseek(fd, start_offset, SEEK_SET);

    if (rc < 0)
    {
        fprintf(stderr, "decoder_input_req_handler: fseek failed\n");
    }
    else if (bytes_to_read)
    {
        bytes_read = (int)fread(buf_ptr, 1, bytes_to_read, fd);
    }

//  printf("\n---(%d)%s() Rd_buf=0x%08X, start_offset=0x%04X, length=%d, bytes_read=%d\n",
//         __LINE__, __func__, (int)buf_ptr, start_offset, length, bytes_read);

    return bytes_read;
}

void decoder_dump_output(char *output_file, jpegd_dst_t *p_dst,
  jpegd_output_buf_t *p_output_buffer, float scale_ratio)
{
    uint8_t* buf_ptr;
    uint32_t stride;

    if (p_dst->stride == 0)
    {
        switch (p_dst->output_format)
        {
        case YCRCBLP_H2V2:
        case YCBCRLP_H2V2:
        case YCRCBLP_H2V1:
        case YCBCRLP_H2V1:
        case YCRCBLP_H1V2:
        case YCBCRLP_H1V2:
        case YCRCBLP_H1V1:
        case YCBCRLP_H1V1:
            stride = p_dst->width;
            break;

        case MONOCHROME:
            stride = p_dst->width;
            break;

        case RGB565:
            stride = p_dst->width * 2;
            break;

        case RGB888:
            stride = p_dst->width * 3;
            break;

        case RGBa:
            stride = p_dst->width * 4;
            break;

        default:
            return;
        }
    }
    else
    {
        stride = p_dst->stride;
    }

    // Determine if the destination is frame buffer or file
    if (OUTPUT_IS_SCREEN(output_file))
    {
#ifdef SCREEN_DUMP_SUPPORTED
        int rc;
        // Draw decoded frame to screen
        struct mdp_blit_req* e;
        unsigned char *fb_mmap;
        int fb_fd, fb_size;
        struct fb_var_screeninfo vinfo;
        union {
            char dummy[sizeof(struct mdp_blit_req_list) + sizeof(struct mdp_blit_req)*1];
            struct mdp_blit_req_list list;
        } req;

        // Open frame buffer driver
#ifdef ANDROID
        fb_fd = open("/dev/graphics/fb0", O_RDWR );
#else
        fb_fd = open("/dev/fb0", O_RDWR);
#endif
        if (fb_fd < 0)
        {
            fprintf(stderr, "decoder_dump_output: failed to open frame buffer driver\n");
            return;
        }

        // Get screen info
        if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
        {
            fprintf(stderr, "decoder_dump_output: failed to retrieve vscreen info\n");
            close(fb_fd);
        }

        vinfo.activate = FB_ACTIVATE_VBL;
        fb_size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

        fb_mmap = mmap (NULL, fb_size, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);

        if (fb_mmap == MAP_FAILED)
        {
            fprintf(stderr, "decoder_dump_output: failed to mmap\n");
            close(fb_fd);
        }

        req.list.count = 1;

        e = &req.list.req[0];

        // Source info
        e->src.width  = stride;
        e->src.height = p_dst->height;
        if (p_dst->output_format == YCRCBLP_H2V2)
        {
            e->src.format = MDP_Y_CBCR_H2V2;
            e->src.memory_id = jpeg_buffer_get_pmem_fd(p_output_buffer->data.yuv.luma_buf);
        }
        else if (p_dst->output_format == YCRCBLP_H2V1)
        {
            e->src.format = MDP_Y_CBCR_H2V1;
            e->src.memory_id = jpeg_buffer_get_pmem_fd(p_output_buffer->data.yuv.luma_buf);
        }
        else if (p_dst->output_format == RGB565)
        {
            e->src.format = MDP_RGB_565;
            e->src.memory_id = jpeg_buffer_get_pmem_fd(p_output_buffer->data.rgb.rgb_buf);
        }
        else if (p_dst->output_format == RGB888)
        {
            e->src.format = MDP_RGB_888;
            e->src.memory_id = jpeg_buffer_get_pmem_fd(p_output_buffer->data.rgb.rgb_buf);
        }
        else // if RGBa
        {
            e->src.format = MDP_RGBA_8888;
            e->src.memory_id = jpeg_buffer_get_pmem_fd(p_output_buffer->data.rgb.rgb_buf);
        }
        e->src.offset = 0;
        if (e->src.memory_id <= 0)
        {
            fprintf(stderr, "decoder_dump_output: source buffer not mmap-ed\n");

            close(fb_fd);
            return;
        }

        // Destination info
        e->dst.width  = vinfo.xres;
        e->dst.height = vinfo.yres;
        e->dst.format = MDP_RGB_565;
        e->dst.offset = 0;
        e->dst.memory_id = fb_fd;

        e->transp_mask = 0xffffffff;
        e->flags = 0;
        e->alpha  = 0xff;

        // Destination Rect
        e->dst_rect.x = 0;
        e->dst_rect.y = 0;
        if (vinfo.xres >= stride && vinfo.yres >= p_dst->height)
        {
            e->dst_rect.w = stride;
            e->dst_rect.h = p_dst->height;
        }
        else
        {
            e->dst_rect.w = vinfo.xres;
            e->dst_rect.h = vinfo.yres;
        }

        // Source Rect
        e->src_rect.x = 0;
        e->src_rect.y = 0;
        e->src_rect.w = stride;
        e->src_rect.h = p_dst->height;

        rc = ioctl(fb_fd, MSMFB_BLIT, &req.list);
        if (!rc)
        {
            rc = ioctl(fb_fd, FBIOPAN_DISPLAY, &vinfo);
        }
        fprintf(stderr, "decoder_dump_output: %s\n", rc ? "failed" : "succeeded");
        close(fb_fd);
#endif // #ifdef SCREEN_DUMP_SUPPORTED
    }
    else
    {
        FILE *fout = fopen(output_file, "wb");
        fprintf(stderr, "decoder_dump_output: dumping decoded image to file %s\n", output_file);
        fprintf(stderr, "decoder_dump_output: scale ratio %f\n", scale_ratio);

        if (!fout)
        {
            fprintf(stderr, "decoder_dump_output: failed to open output file\n");
            return;
        }
        switch (p_dst->output_format)
        {
        case YCRCBLP_H2V2:
        case YCBCRLP_H2V2:
            jpeg_buffer_get_addr(p_output_buffer->data.yuv.luma_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height * SQUARE(scale_ratio), fout);
            jpeg_buffer_get_addr(p_output_buffer->data.yuv.chroma_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height / 2 * SQUARE(scale_ratio), fout);
            break;
        case YCRCBLP_H1V2:
        case YCBCRLP_H1V2:
        case YCRCBLP_H2V1:
        case YCBCRLP_H2V1:
            jpeg_buffer_get_addr(p_output_buffer->data.yuv.luma_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height * SQUARE(scale_ratio), fout);
            jpeg_buffer_get_addr(p_output_buffer->data.yuv.chroma_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height * SQUARE(scale_ratio), fout);
            break;
        case YCRCBLP_H1V1:
        case YCBCRLP_H1V1:
            jpeg_buffer_get_addr(p_output_buffer->data.yuv.luma_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height * SQUARE(scale_ratio), fout);
            jpeg_buffer_get_addr(p_output_buffer->data.yuv.chroma_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height * 2 * SQUARE(scale_ratio), fout);
            break;
        case MONOCHROME:
            jpeg_buffer_get_addr(p_output_buffer->data.yuv.luma_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height * SQUARE(scale_ratio), fout);
            break;
        case RGB565:
        case RGB888:
        case RGBa:
            jpeg_buffer_get_addr(p_output_buffer->data.rgb.rgb_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height, fout);
            break;
        default:
            break;
        }
        fclose(fout);
    }
}

int decoder_test_dump_exif(FILE* dump_file, exif_info_obj_t exif)
{
    exif_tag_id_t *p_tag_ids = NULL;
    uint32_t num_tag, i;

    // First get the number of tags present
    if (exif_list_tagid(exif, p_tag_ids, 0, &num_tag))
        return -1;

    if (!num_tag)
    {
        fprintf(stderr, "No EXIF tag present\n");
        return 0;
    }
    // Allocate storage to hold the tag ids
    p_tag_ids = (exif_tag_id_t *)malloc(num_tag * sizeof(exif_tag_id_t));

    if (!p_tag_ids)
    {
        return -1;
    }
    // Get the list again (now with enough storage)
    if (exif_list_tagid(exif, p_tag_ids, num_tag, NULL))
    {
        fprintf(stderr, "decoder_test_dump_exif: exif_list_tagid failed\n");
        free(p_tag_ids);
        return -1;
    }

    for (i = 0; i < num_tag; i++)
    {
        uint32_t j;
        exif_tag_entry_t entry;
        if (exif_get_tag(exif, p_tag_ids[i], &entry))
        {
            fprintf(dump_file, "decoder_test_dump_exif: failed to retrieve tag 0x%x\n", p_tag_ids[i] >> 16);
            continue;
        }
        fprintf(dump_file, "===================================================\n");
        fprintf(dump_file, "Tag:\t\t0x%x\n", p_tag_ids[i] & 0xffff);
        fprintf(dump_file, "Count:\t%d\n", entry.count);
        switch (entry.type)
        {
        case EXIF_BYTE:
        case EXIF_UNDEFINED:
            if (entry.type == EXIF_BYTE)
                fprintf(dump_file, "Type:\t\tEXIF_BYTE\n");
            else
                fprintf(dump_file, "Type:\t\tEXIF_UNDEFINED\n");
            fprintf(dump_file, "Data:\t\t");
            if (entry.count > 1)
            {
                for (j = 0; j < entry.count; j++)
                {
                    fprintf(dump_file, "0x%.2x ", entry.data._bytes[j]);
                    if (j % 20 == 19)
                    {
                        fprintf(dump_file, "\n\t\t");
                    }
                }
                fprintf(dump_file, "\n");
            }
            else
            {
                fprintf(dump_file, "0x%.2x\n", entry.data._byte);
            }
            break;
        case EXIF_SHORT:
            fprintf(dump_file, "Type:\t\tEXIF_SHORT\n");
            fprintf(dump_file, "Data:\t\t");
            if (entry.count > 1)
            {
                for (j = 0; j < entry.count; j++)
                {
                    fprintf(dump_file, "0x%.4x ", entry.data._shorts[j]);
                    if (j % 20 == 19)
                    {
                        fprintf(dump_file, "\n\t\t");
                    }
                }
                fprintf(dump_file, "\n");
            }
            else
            {
                fprintf(dump_file, "0x%.4x\n", entry.data._short);
            }
            break;
        case EXIF_LONG:
        case EXIF_SLONG:
            if (entry.type == EXIF_LONG)
                fprintf(dump_file, "Type:\t\tEXIF_LONG\n");
            else
                fprintf(dump_file, "Type:\t\tEXIF_SLONG\n");
            fprintf(dump_file, "Data:\t\t");
            if (entry.count > 1)
            {
                for (j = 0; j < entry.count; j++)
                {
                    fprintf(dump_file, "0x%.8x ", entry.data._longs[j]);
                    if (j % 16 == 15)
                    {
                        fprintf(dump_file, "\n\t\t");
                    }
                }
                fprintf(dump_file, "\n");
            }
            else
            {
                fprintf(dump_file, "0x%.8x\n", entry.data._long);
            }
            break;
        case EXIF_RATIONAL:
        case EXIF_SRATIONAL:
            if (entry.type == EXIF_RATIONAL)
                fprintf(dump_file, "Type:\t\tEXIF_RATIONAL\n");
            else
                fprintf(dump_file, "Type:\t\tEXIF_SRATIONAL\n");
            fprintf(dump_file, "Data:\t\t");
            if (entry.count > 1)
            {
                for (j = 0; j < entry.count; j++)
                {
                    fprintf(dump_file, "(0x%.8x/0x%.8x) ", entry.data._rats[j].num, entry.data._rats[j].denom);
                    if (j % 16 == 15)
                    {
                        fprintf(dump_file, "\n\t\t");
                    }
                }
                fprintf(dump_file, "\n");
            }
            else
            {
                fprintf(dump_file, "(0x%.8x/0x%.8x)\n", entry.data._rat.num, entry.data._rat.denom);
            }
            break;
        case EXIF_ASCII:
            fprintf(dump_file, "Type:\t\tEXIF_ASCII\n");
            fprintf(dump_file, "Data:\t\t%s\n", entry.data._ascii);
            break;
        default:
            break;
        }
    }
    // clean up allocated array
    free(p_tag_ids);
    return 0;
}

