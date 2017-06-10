/**
 * Copyright (C) 2008-2011,2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/18/10   staceyw Enable and added downscale tests for thumbnail and main iamge.
05/20/10   staceyw Added tests on bitstream input.
11/02/09   staceyw Added file size control tests.
10/13/09   vma     Adhered to the interface change on encoder where user
                   data is passed back by encoder in handlers.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
                   Client of the JPEG library should not be using
                   this layer normally (some header files are not public).
                   This is just for convenience to build an os_agnostic test
                   app.
06/15/09   zhiminl Added jpeg size in output_handler_args_t;
04/23/09   vma     Added reporting of the event where thumbnail is
                   too big and is dropped.
10/08/08   vma     Fixed abort deadlock.
09/29/08   vma     Added concurrency testing.
09/16/08   vma     Added reference file comparison.
09/15/08   vma     Added abort testing.
09/12/08   vma     Added upsampler support.
07/07/08   vma     Created file.

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
#include "remote.h"

#define OS_Q6_PAGE  4096
  struct mmap_info_ion mapion_input0, mapion_input1;
  uint32_t ion_heapid;

#pragma weak  remote_register_buf

static void register_buf(void* buf, int size, int fd) {
  if(remote_register_buf) {
      remote_register_buf(buf, size, fd);
   }
}


#if (defined WIN32 || defined WINCE)
#include "wingetopt.h"
#else
#include <getopt.h>
#endif

#define CMP_SIZE 1024
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)

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
const char memory_formats[][5] =
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
const char *preference_str[] =
{
    "Hardware accelerated preferred",
    "Hardware accelerated only",
    "Software based preferred",
    "Software based only",
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

} test_args_t;

typedef struct
{
    FILE*           fout;
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

int read_bytes_from_file(const char* file, int width, int height,
                         jpeg_buffer_t *p_luma_buf,
                         jpeg_buffer_t *p_chroma_buf,
                         uint8_t use_pmem, test_args_t *p_args,
                         uint8_t is_thumbnail);
int read_bytes_from_file_3planes(const char* file, int width, int height,
                         jpeg_buffer_t *p_luma_buf,
                         jpeg_buffer_t *p_chroma_buf,
                         jpeg_buffer_t *p_cr_buf,
                         uint8_t use_pmem, test_args_t *p_args,
                         uint8_t is_thumbnail);

int read_bs_from_file(const char* file,
                      jpeg_buffer_t *p_luma_buf,
                      uint8_t use_pmem);

OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER encoder_test(OS_THREAD_FUNC_ARG_T arg);

void encoder_event_handler(void         *p_user_data,
                           jpeg_event_t  event,
                           void         *p_arg);

int  encoder_output_handler(void           *p_user_data,
                            void           *p_arg,
                            jpeg_buffer_t   buffer,
                            uint8_t         last_buf_flag);

// Global variables
int concurrent_cnt = 1;
thread_ctrl_blk_t *thread_ctrl_blks = NULL;
static int fwrite_time = 0;

void print_usage(void)
{
    fprintf(stderr, "Usage: program_name [options] [-I <input file>] [-O <output file>] [-W <width>] [-H <height>] [-F <format>]\n");
    fprintf(stderr, "Mandatory options:\n");
    fprintf(stderr, "  -I FILE\t\tPath to the input file.\n");
    fprintf(stderr, "  -O FILE\t\tPath for the output file.\n");
    fprintf(stderr, "  -W WIDTH\t\tInput image width.\n");
    fprintf(stderr, "  -H HEIGHT\t\tInput image height.\n");
    fprintf(stderr, "  -F FORMAT\t\tInput image format:\n");
    fprintf(stderr, "\t\t\t\tYCRCBLP_H2V2 (0), YCBCRLP_H2V2 (1), YCRCBLP_H2V1 (2), YCBCRLP_H2V1 (3)\n");
    fprintf(stderr, "\t\t\t\tYCRCBLP_H1V2 (4), YCBCRLP_H1V2 (5), YCRCBLP_H1V1 (6), YCBCRLP_H1V1 (7)\n");
    fprintf(stderr, "\t\t\t\t MONOCHROME  (8)\n");
    fprintf(stderr, "\t\t\t\tBITSTREAM_H2V2 (12), BITSTREAM_H2V1 (14), BITSTREAM_H1V2 (16), BITSTREAM_H1V1 (18)\n");
    fprintf(stderr, "  -C Memory Format\t\tInput memory format:\n");
    fprintf(stderr, "\t\t\t\tNV12 (0), NV21 (1), IYUV (2), YU12 (3)\n");
    fprintf(stderr, "Optional:\n");
    fprintf(stderr, "  -r ROTATION\t\tRotation (clockwise) in degrees\n");
    fprintf(stderr, "  -t\t\t\tEncode thumbnail image as well. If turned on, the four arguments below should be supplied.\n");
    fprintf(stderr, "  -i FILE\t\tPath to the input file for thumbnail.\n");
    fprintf(stderr, "  -w WIDTH\t\tThumbnail image width.\n");
    fprintf(stderr, "  -h HEIGHT\t\tThumbnail image height.\n");
    fprintf(stderr, "  -f FORMAT\t\tThumbnail image format. (Permissible values are listed above)\n");
    fprintf(stderr, "  -Q QUALITY\t\tQuality factor for main image (1-100).\n");
    fprintf(stderr, "  -q QUALITY\t\tQuality factor for thumbnail (1-100).\n");
    fprintf(stderr, "  -p PREFERENCE\t\tPreference on which encoder to use (Software-based or Hardware-accelerated).\n");
    fprintf(stderr, "               \t\t\tHW preferred (0-default), HW only (1), SW preferred (2), SW only (3)\n");
    fprintf(stderr, "  -P PMEM\t\tEnable use of PMEM.\n");
    fprintf(stderr, "  -c COUNT\t\tRe-encode back-to-back 'COUNT' times. Default is 1.\n");
    fprintf(stderr, "  -u\t\t\tEnable scale for Main Image. Make sure the scale parameters are supplied.\n");
    fprintf(stderr, "  -m SCALE_I_WIDTH\tMain input width.\n");
    fprintf(stderr, "  -n SCALE_I_HEIGHT\tMain input height.\n");
    fprintf(stderr, "  -x SCALE_H_OFFSET\tMain horizontal offset.\n");
    fprintf(stderr, "  -y SCALE_V_OFFSET\tMain vertical offset.\n");
    fprintf(stderr, "  -M SCALE_O_WIDTH\tMain output width.\n");
    fprintf(stderr, "  -N SCALE_O_HEIGHT\tMain output height.\n");
    fprintf(stderr, "  -U\t\t\tEnable scale for Thumbnail. Make sure the scale parameters are supplied.\n");
    fprintf(stderr, "  -j SCALE_I_WIDTH\tThumbnail input width.\n");
    fprintf(stderr, "  -k SCALE_I_HEIGHT\tThumbnail input height.\n");
    fprintf(stderr, "  -X SCALE_H_OFFSET\tThumbnail horizontal offset.\n");
    fprintf(stderr, "  -Y SCALE_V_OFFSET\tThumbnail vertical offset.\n");
    fprintf(stderr, "  -J SCALE_O_WIDTH\tThumbnail output width.\n");
    fprintf(stderr, "  -K SCALE_O_HEIGHT\tThumbnail output height.\n");
    fprintf(stderr, "  -S FILE_SIZE_CONTROL\t\tTarget file size in Bytes.\n");
    fprintf(stderr, "  -B OUTPUT_BUFFER_SIZE\t\tOutput buffer size in KBytes.\n");
    fprintf(stderr, "  -e CONCURRENT_COUNT\tRun COUNT tests in concurrent threads (test for multiple instances of encoder within same process).\n");
    fprintf(stderr, "  -a ABORT_TIME\t\tThe duration before an abort is issued (in milliseconds).\n");
    fprintf(stderr, "  -g GOLDEN_REFERENCE\tThe reference file to be matched against the output.\n");
    fprintf(stderr, "  -R RESTART_INTERVAL\tRestart Interval in number of MCUs\n");
    fprintf(stderr, "  -Z NOWRITE_TO_OUTPUT\tDisable write to the output file.\n");

    fprintf(stderr, "\n");
}

#ifdef WINCE
char **tchar2char(int c, _TCHAR*v[])
{
    int i, j;
    char **ret = (char **)malloc(c * sizeof(char *));
    for (i = 0; i < c; i++)
    {
        j = 0;
        ret[i] = (char *)malloc(50);
        while ((ret[i][j] = (char)v[i][j]) != '\0') j++;
    }
    return ret;
}
int _tmain(int argc, _TCHAR* _argv[])
{
    char **argv = tchar2char(argc, _argv);
#else
int main(int argc, char* argv[])
{
#endif
    int rc, c, i;
    test_args_t test_args;

    memset(&test_args, 0, sizeof(test_args));
    test_args.main.format = 8;
    test_args.thumbnail.format = 8;
    test_args.back_to_back_count = 1;
    test_args.main.memory_format =0;

    fprintf(stderr, "=============================================================\n");
    fprintf(stderr, "Encoder test\n");
    fprintf(stderr, "=============================================================\n");
    opterr = 1;
    while ((c = getopt(argc, argv, "I:O:W:H:F:Q:C:r:ti:w:h:f:q:p:c:um:n:x:y:M:N:Uj:k:X:Y:J:K:S:B:a:g:e:R:PZ")) != -1)
    {
        switch (c)
        {
        case 'O':
            test_args.output_file = optarg;
            fprintf(stderr, "%-25s%s\n", "Output image path", test_args.output_file);
            break;
        case 'I':
            test_args.main.file_name = optarg;
            fprintf(stderr, "%-25s%s\n", "Input image path", test_args.main.file_name);
            break;
        case 'i':
            test_args.thumbnail.file_name = optarg;
            fprintf(stderr, "%-25s%s\n", "Thumbnail image path", test_args.thumbnail.file_name);
            break;
        case 'Q':
            test_args.main.quality = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Main image quality", test_args.main.quality);
            break;
        case 'q':
            test_args.thumbnail.quality = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Thumbnail quality", test_args.thumbnail.quality);
            break;
        case 'W':
            test_args.main.width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Input width", test_args.main.width);
            break;
        case 'w':
            test_args.thumbnail.width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Thumbnail width", test_args.thumbnail.width);
            break;
        case 'H':
            test_args.main.height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Input height", test_args.main.height);
            break;
        case 'h':
            test_args.thumbnail.height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Thumbnail height", test_args.thumbnail.height);
            break;
        case 'r':
            test_args.rotation = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Rotation", test_args.rotation);
            break;
        case 'C':
            test_args.main.memory_format = atoi(optarg);
            fprintf(stderr, "%-25s%s\n", "Memory format", memory_formats[(uint8_t)test_args.main.memory_format]);
            break;
        case 't':
            test_args.encode_thumbnail = true;
            fprintf(stderr, "%-25s%s\n", "Encode thumbnail", "true");
            break;
        case 'F':
            test_args.main.format = atoi(optarg);
            fprintf(stderr, "%-25s%s\n", "Input format", color_formats[(uint8_t)test_args.main.format]);
            break;
        case 'f':
            test_args.thumbnail.format = atoi(optarg);
            fprintf(stderr, "%-25s%s\n", "Thumbnail format", color_formats[(uint8_t)test_args.thumbnail.format]);
            break;
        case 'p':
            test_args.preference = atoi(optarg);
            fprintf(stderr, "%-25s%s\n", "Encoder preference", preference_str[test_args.preference]);
            break;
        case 'c':
            test_args.back_to_back_count = atoi(optarg);
            if (test_args.back_to_back_count == 0)
            {
                fprintf(stderr, "Invalid count: -c %d\n", test_args.back_to_back_count);
                return 1;
            }
            break;
        case 'u':
            test_args.main_scale_cfg.enable = true;
            fprintf(stderr, "%-25s%s\n", "scale enabled for main image", "true");
            break;
        case 'm':
            test_args.main_scale_cfg.input_width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "main image scale input width", test_args.main_scale_cfg.input_width);
            break;
        case 'n':
            test_args.main_scale_cfg.input_height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "main image scale input height", test_args.main_scale_cfg.input_height);
            break;
        case 'M':
            test_args.main_scale_cfg.output_width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "main image scale output width", test_args.main_scale_cfg.output_width);
            break;
        case 'N':
            test_args.main_scale_cfg.output_height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "main image scale output height", test_args.main_scale_cfg.output_height);
            break;
        case 'x':
            test_args.main_scale_cfg.h_offset = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "main image scale h offset", test_args.main_scale_cfg.h_offset);
            break;
        case 'y':
            test_args.main_scale_cfg.v_offset = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "main image scale v offset", test_args.main_scale_cfg.v_offset);
            break;
        case 'U':
            test_args.tn_scale_cfg.enable = true;
            fprintf(stderr, "%-25s%s\n", "scale enabled for thumbnail", "true");
            break;
        case 'j':
            test_args.tn_scale_cfg.input_width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "thumbnail scale input width", test_args.tn_scale_cfg.input_width);
            break;
        case 'k':
            test_args.tn_scale_cfg.input_height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "thumbnail scale input height", test_args.tn_scale_cfg.input_height);
            break;
        case 'J':
            test_args.tn_scale_cfg.output_width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "thumbnail scale output width", test_args.tn_scale_cfg.output_width);
            break;
        case 'K':
            test_args.tn_scale_cfg.output_height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "thumbnail scale output height", test_args.tn_scale_cfg.output_height);
            break;
        case 'X':
            test_args.tn_scale_cfg.h_offset = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "thumbnail scale h offset", test_args.tn_scale_cfg.h_offset);
            break;
        case 'Y':
            test_args.tn_scale_cfg.v_offset = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "thumbnail scale v offset", test_args.tn_scale_cfg.v_offset);
            break;
        case 'S':
            test_args.target_filesize = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Target file size", test_args.target_filesize);
            break;
        case 'B':
            test_args.output_buf_size = atoi(optarg) * 1024;
            fprintf(stderr, "%-25s%d\n", "Output bufer size", test_args.output_buf_size);
            break;
        case 'a':
            test_args.abort_time = atoi(optarg);
            fprintf(stderr, "%-25s%d ms\n", "Abort time", test_args.abort_time);
            break;
        case 'g':
            test_args.reference_file = optarg;
            fprintf(stderr, "%-25s%s\n", "Reference file", test_args.reference_file);
            break;
        case 'e':
            concurrent_cnt = atoi(optarg);
            if (!concurrent_cnt)
            {
                fprintf(stderr, "Invalid concurrent count: 0\n");
                return 1;
            }
            fprintf(stderr, "%-25s%d\n", "Concurrent count", concurrent_cnt);
            break;
        case 'R':
            test_args.restart_interval = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Restart interval", test_args.restart_interval);
            break;
        case 'P':
            test_args.use_pmem = true;
            fprintf(stderr, "%-25s%s\n", "Use PMEM", "true");
            break;
        case 'Z':
            test_args.output_nowrite = true;
            fprintf(stderr, "%-25s%s\n", "Output write disabled", "true");
            break;
        }
    }
    // Double check all the required arguments are set
    if (!test_args.main.file_name || !test_args.output_file ||
        !test_args.main.width ||
        !test_args.main.height ||
        test_args.main.format == 9)
    {
        fprintf(stderr, "Missing required arguments.\n");
        print_usage();
        return 1;
    }
    if (test_args.encode_thumbnail &&
        (!test_args.thumbnail.file_name ||
         !test_args.thumbnail.width ||
         !test_args.thumbnail.height ||
         test_args.thumbnail.format == 8))
    {
        fprintf(stderr, "Missing thumbnail arguments.\n");
        print_usage();
        return 1;
    }

    // Create thread control blocks
    thread_ctrl_blks = (thread_ctrl_blk_t *)malloc(concurrent_cnt * sizeof(thread_ctrl_blk_t));
    if (!thread_ctrl_blks)
    {
        fprintf(stderr, "encoder_test failed: insufficient memory in creating thread control blocks\n");
        return 1;
    }
    memset(thread_ctrl_blks, 0, concurrent_cnt * sizeof(thread_ctrl_blk_t));
    // Initialize the blocks and kick off the threads
    for (i = 0; i < concurrent_cnt; i++)
    {
        thread_ctrl_blks[i].tid = i;
        thread_ctrl_blks[i].p_args = &test_args;
        os_mutex_init(&thread_ctrl_blks[i].output_handler_args.mutex);
        os_cond_init(&thread_ctrl_blks[i].output_handler_args.cond);
        if (os_thread_create(&thread_ctrl_blks[i].thread, encoder_test, &thread_ctrl_blks[i]))
        {
            fprintf(stderr, "encoder_test: os_create failed\n");
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
            fprintf(stderr, "encoder_test: thread %d failed\n", i);
            rc = (int)OS_THREAD_FUNC_RET_FAILED;
        }
    }

    if (!rc)
        fprintf(stderr, "encoder_test finished successfully\n");

    fprintf(stderr, "exit value: %d\n", rc);
    return rc;
}

OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER encoder_test(OS_THREAD_FUNC_ARG_T arg)
{
    char *output_filename;
    thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)arg;
    int rc, i;
    jpege_obj_t encoder;
    jpege_src_t source;
    jpege_dst_t dest;
    jpege_cfg_t config;
    jpege_img_data_t img_info, tn_img_info;
    jpeg_buffer_t main_luma_buf, main_chroma_buf, main_cr_buf;
    jpeg_buffer_t tn_luma_buf, tn_chroma_buf;
    test_args_t *p_args = p_thread_arg->p_args;
    uint8_t use_pmem = false;
    os_timer_t os_timer;
    jpeg_buffer_t buffers[2];
    exif_info_obj_t exif_info;
    exif_tag_entry_t exif_data;

    output_filename = p_args->output_file;
    // Append the output file name with a number to avoid multiple writing to the same file
    if (p_thread_arg->tid)
    {
        // Look for the last occurence of '/' and then last occurence of '.'
        char *s = strrchr(p_args->output_file, '/');
        if (s)
        {
            s = strrchr(s, '.');
        }
        else
        {
            s = strrchr(p_args->output_file, '.');
        }
        output_filename = (char *)malloc(5 + strlen(p_args->output_file));
        if(output_filename) {
            snprintf(output_filename, sizeof(output_filename), "%s", p_args->output_file);
            if (s)
            {
                snprintf(output_filename + (s - p_args->output_file), sizeof(output_filename),"_%.2d%s", p_thread_arg->tid, s);
            }
            else
            {
                snprintf(output_filename, sizeof(output_filename),"%s_%.2d", output_filename, p_thread_arg->tid);
            }
        }
    }

    // Determine whether pmem should be used (useful for pc environment testing where
    // pmem is not available)
    if (!((jpege_preference_t)p_args->preference == JPEG_ENCODER_PREF_SOFTWARE_PREFERRED ||
        (jpege_preference_t)p_args->preference == JPEG_ENCODER_PREF_SOFTWARE_ONLY))
    {
        use_pmem = p_args->use_pmem;
    }

    // Initialize source buffers
    if (jpeg_buffer_init(&main_luma_buf) ||
        jpeg_buffer_init(&main_chroma_buf) ||
        jpeg_buffer_init(&main_cr_buf) ||
        jpeg_buffer_init(&tn_luma_buf) ||
        jpeg_buffer_init(&tn_chroma_buf))
    {
        goto fail;
    }

    // Open input file(s) and read the contents
    if ((p_args->main.memory_format == NV12) ||
        (p_args->main.memory_format == NV21))
    {
        if (p_args->main.format == MONOCHROME)
            p_args->main.num_of_planes = 1;
        else
            p_args->main.num_of_planes = 2;

        if (p_args->main.format <= MONOCHROME)
        {
            if (read_bytes_from_file(p_args->main.file_name,
                        p_args->main.width,
                        p_args->main.height, &main_luma_buf,
                        &main_chroma_buf,
                        use_pmem, p_args, false))
        goto fail;
        }
        else if  ((p_args->main.format >= JPEG_BITSTREAM_H2V2) &&
                 (p_args->main.format < JPEG_COLOR_FORMAT_MAX))
        {

        fprintf(stderr, "encoder_test: BIT STREAM: "
                "read_bs_from_file\n");
        if (read_bs_from_file(p_args->main.file_name, &main_luma_buf,
                              use_pmem))
            goto fail;
         }
         else
        {
         fprintf(stderr, "encoder_test: main image color format not "
                 "supported\n"); goto fail;
         }
     }
    else if ((p_args->main.memory_format == IYUV) ||
            (p_args->main.memory_format == YUV12))
    {
         if (p_args->main.format == MONOCHROME)
            p_args->main.num_of_planes = 1;
        else
            p_args->main.num_of_planes = 3;

        if (p_args->main.format <= MONOCHROME)
        {
            if (read_bytes_from_file_3planes(p_args->main.file_name,
                      p_args->main.width, p_args->main.height,
                      &main_luma_buf, &main_chroma_buf, &main_cr_buf,
                      use_pmem, p_args, false))
            goto fail;
        }
    }
    else{
        fprintf(stderr, "Encoder_test:memory format not supported\n");
        }

    if (p_args->encode_thumbnail)
    {
        if (p_args->thumbnail.format <= YCBCRLP_H1V1)
        {
            if (read_bytes_from_file(p_args->thumbnail.file_name, p_args->thumbnail.width,
                                     p_args->thumbnail.height, &tn_luma_buf, &tn_chroma_buf,
                                     use_pmem, p_args, true))
                goto fail;
        }
        else if  ((p_args->thumbnail.format >= JPEG_BITSTREAM_H2V2) &&
                  (p_args->main.format < JPEG_COLOR_FORMAT_MAX))
        {
            if (read_bs_from_file(p_args->thumbnail.file_name,
                                 &tn_luma_buf,
                                  use_pmem))
                goto fail;
        }
        else
        {
            fprintf(stderr, "encoder_test: thumbnail color format not supported\n");
            goto fail;
        }
    }

    // Initialize encoder
    rc = jpege_init(&encoder, &encoder_event_handler, (void *)p_thread_arg);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "encoder_test: jpege_init failed\n");
        goto fail;
    }
    p_thread_arg->encoder = encoder;

    // Set source information (main)
    img_info.color_format           = p_args->main.format;
    img_info.width                  = p_args->main.width;
    img_info.height                 = p_args->main.height;
    img_info.fragment_cnt           = 1;
    img_info.p_fragments[0].width   = p_args->main.width;
    img_info.p_fragments[0].height  = p_args->main.height;
    img_info.p_fragments[0].num_of_planes = p_args->main.num_of_planes;
    if (img_info.color_format <= MONOCHROME)
    {
        img_info.p_fragments[0].color.yuv.luma_buf   = main_luma_buf;
        img_info.p_fragments[0].color.yuv.chroma_buf = main_chroma_buf;
        img_info.p_fragments[0].color.yuv.cr_buf = main_cr_buf;
    }
    else if ((img_info.color_format >= JPEG_BITSTREAM_H2V2) &&
             (img_info.color_format < JPEG_COLOR_FORMAT_MAX))
    {
        img_info.p_fragments[0].color.bitstream.bitstream_buf = main_luma_buf;
        fprintf(stderr, "encoder_test:bit stream\n");
    }

    source.p_main = &img_info;
    source.p_thumbnail = NULL;

    // Set source information (thumbnail)
    if (p_args->encode_thumbnail)
    {
        tn_img_info.color_format           = p_args->thumbnail.format;
        tn_img_info.width                  = p_args->thumbnail.width;
        tn_img_info.height                 = p_args->thumbnail.height;
        tn_img_info.fragment_cnt           = 1;
        tn_img_info.p_fragments[0].width   = p_args->thumbnail.width;
        tn_img_info.p_fragments[0].height  = p_args->thumbnail.height;
        if (tn_img_info.color_format <= YCBCRLP_H1V1)
        {
            tn_img_info.p_fragments[0].color.yuv.luma_buf    = tn_luma_buf;
            tn_img_info.p_fragments[0].color.yuv.chroma_buf  = tn_chroma_buf;
        }
        else if ((tn_img_info.color_format >= JPEG_BITSTREAM_H2V2) &&
                 (tn_img_info.color_format <= YCBCRLP_H1V1))
        {
            tn_img_info.p_fragments[0].color.bitstream.bitstream_buf = tn_luma_buf;
        }
        source.p_thumbnail = &tn_img_info;
    }

    rc = jpege_set_source(encoder, &source);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "encoder_test: jpege_set_source failed\n");
        goto fail;
    }

    // Loop to perform n back-to-back encoding (to the same output file)
    for (i = 0; i < p_args->back_to_back_count; i++)
    {
        if (p_args->back_to_back_count > 1)
        {
            fprintf(stderr, "Encoding (%d-th time)\n", i+1);
        }

        // Open output file
        if(output_filename) {
            p_thread_arg->output_handler_args.fout = fopen(output_filename, "wb");
            if (!p_thread_arg->output_handler_args.fout) {
                fprintf(stderr, "encoder_test: failed to open output file: %s\n", output_filename);
                goto fail;
            }
        }
        p_thread_arg->output_handler_args.size = 0;
        p_thread_arg->output_handler_args.nowrite = p_args->output_nowrite;


        // Set default output buf size equal to 8K
        // if it is not provided by user
        if (!p_thread_arg->p_args->output_buf_size)
        {
            p_thread_arg->p_args->output_buf_size = 8192;
        }

        // Set destination information
        dest.p_output_handler = encoder_output_handler;
        dest.p_arg = (void*)&p_thread_arg->output_handler_args;
        dest.buffer_cnt = 2;
        if (JPEG_FAILED(jpeg_buffer_init(&buffers[0])) ||
            JPEG_FAILED(jpeg_buffer_init(&buffers[1])) ||
            JPEG_FAILED(jpeg_buffer_allocate(buffers[0], p_thread_arg->p_args->output_buf_size, use_pmem)) ||
            JPEG_FAILED(jpeg_buffer_allocate(buffers[1], p_thread_arg->p_args->output_buf_size, use_pmem)))
        {
            fprintf(stderr, "encoder_test: failed to allocate destination buffers\n");
            jpeg_buffer_destroy(&buffers[0]);
            jpeg_buffer_destroy(&buffers[1]);
            dest.buffer_cnt = 0;
        }

        dest.p_buffer = &buffers[0];
        rc = jpege_set_destination(encoder, &dest);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "encoder_test: jpege_set_destination failed\n");
            goto fail;
        }

        // Set default configuration
        rc = jpege_get_default_config(&config);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "encoder_test: jpege_set_default_config failed\n");
            goto fail;
        }
        // Set custom configuration
        config.main_cfg.rotation_degree_clk = p_args->rotation;
        if (p_args->main.quality)
            config.main_cfg.quality = p_args->main.quality;
        config.thumbnail_cfg.rotation_degree_clk = p_args->rotation;
        if (p_args->thumbnail.quality)
            config.thumbnail_cfg.quality = p_args->thumbnail.quality;
        config.thumbnail_present = p_args->encode_thumbnail;
        if (p_args->preference < JPEG_ENCODER_PREF_MAX)
        {
            config.preference = (jpege_preference_t)p_args->preference;
        }

        // Set scale cfg
        config.main_cfg.scale_cfg = p_args->main_scale_cfg;
        config.thumbnail_cfg.scale_cfg = p_args->tn_scale_cfg;

        // Set target file size
        config.target_filesize =  p_args->target_filesize;

        // Specify restart interval
        config.main_cfg.restart_interval = p_args->restart_interval;

        // set orientation flag
        rc = exif_init(&exif_info);
        exif_data.type = EXIF_RATIONAL;
        exif_data.count = 1;
        //exif_data.data._short = 10;
        exif_data.data._rat.denom = 7;
        exif_data.data._rat.num = 1;
        rc = exif_set_tag(exif_info, EXIFTAGID_EXIF_COMPRESSED_BITS_PER_PIXEL, &exif_data);
        // Test delete tags
        rc = exif_delete_tag(exif_info, EXIFTAGID_EXIF_COMPRESSED_BITS_PER_PIXEL);
        // Start encoding
        p_thread_arg->encoding = true;
        if (os_timer_start(&os_timer) < 0)
        {
            fprintf(stderr, "encoder_test: failed to get start time\n");
        }
        rc = jpege_start(encoder, &config, &exif_info);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "encoder_test: jpege_start failed\n");
            goto fail;
        }
        fprintf(stderr, "encoder_test: jpege_start succeeded\n");

        // Abort
        if (p_args->abort_time)
        {
            os_mutex_lock(&p_thread_arg->output_handler_args.mutex);
            while (p_thread_arg->encoding)
            {
                rc = os_cond_timedwait(&p_thread_arg->output_handler_args.cond,
                                            &p_thread_arg->output_handler_args.mutex,
                                            p_args->abort_time);
                if (rc == JPEGERR_ETIMEDOUT)
                {
                    // Do abort
                    fprintf(stderr, "encoder_test: abort now...\n");
                    os_mutex_unlock(&p_thread_arg->output_handler_args.mutex);
                    rc = jpege_abort(encoder);
                    if (rc)
                    {
                        fprintf(stderr, "encoder_test: jpege_abort failed: %d\n", rc);
                        goto fail;
                    }
                    break;
                }
            }
            if (p_thread_arg->encoding)
                os_mutex_unlock(&p_thread_arg->output_handler_args.mutex);
        }
        else
        {
            // Wait until encoding is done or stopped due to error
            os_mutex_lock(&p_thread_arg->output_handler_args.mutex);
            while (p_thread_arg->encoding)
            {
                os_cond_wait(&p_thread_arg->output_handler_args.cond,
                                  &p_thread_arg->output_handler_args.mutex);
            }
            os_mutex_unlock(&p_thread_arg->output_handler_args.mutex);
        }
        // delete exif tags
        exif_destroy(&exif_info);
        rc = jpege_get_actual_config(encoder, &config);

        // Latency measurement
        {
            int diff;
            // Display the time elapsed
            if (os_timer_get_elapsed(&os_timer, &diff, 0) < 0)
            {
                fprintf(stderr, "encoder_test: failed to get elapsed time\n");
            }
            else
            {
                if (p_args->abort_time)
                {
                    if (p_thread_arg->encoding)
                    {
                        fprintf(stderr, "encoder_test: encoding aborted successfully after %d ms\n", diff);
                    }
                    else
                    {
                        fprintf(stderr, "encoder_test: encoding is done before abort is issued. "
                                "encode time: %d ms\n", diff);
                    }
                }
                else
                {
                    fprintf(stderr, "encoder_test: encode time: %d ms\n", diff);
                    fprintf(stderr, "encoder_test: jpeg size: %ld bytes\n",
                            (long)p_thread_arg->output_handler_args.size);
                }
            }
        }

        // Clean up allocated dest buffers
        jpeg_buffer_destroy(&buffers[0]);
        jpeg_buffer_destroy(&buffers[1]);

        fclose(p_thread_arg->output_handler_args.fout);
    }

    // Clean up allocated source buffers
    apr_pmem_free_ion(mapion_input0);
    apr_pmem_free_ion(mapion_input1);
    jpeg_buffer_destroy(&tn_luma_buf);
    jpeg_buffer_destroy(&tn_chroma_buf);

    // Clean up encoder
    jpege_destroy(&encoder);
    fprintf(stderr, "encoder_test: jpege_destroy done\n");

    // Do bitwise comparison if requested
    if (p_args->reference_file)
    {
        FILE *fout = NULL;
        char *buf1;
        char *buf2;
        FILE *fref = fopen(p_args->reference_file, "rb");
        int bytes_read1 =0, bytes_read2 =0;
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
        if(output_filename) {
            fprintf(stderr, "encoder_test: comparing %s with %s...\n",
                output_filename, p_args->reference_file);
        }
        if (!fref)
        {
            fprintf(stderr, "encoder_test: error opening reference file: %s\n", p_args->reference_file);
            free(buf1);
            free(buf2);
            goto fail;
        }
        if (!buf1 || !buf2)
        {
            fprintf(stderr, "encoder_test: failed to malloc for buffer for file comparison\n");
            free(buf1);
            free(buf2);
            goto fail;
        }
        if(output_filename)
            fout = fopen(output_filename, "rb");

        if(!fout)
        {
            fprintf(stderr, "encoder_test: error opening output file: %s\n",
                output_filename);
            free(buf1);
            free(buf2);

            if (fref)
            {
              fclose(fref);
              fref = NULL;
            }

            goto fail;
        }


        for (;;)
        {
            if(!fref)
                bytes_read1 = (int)fread(buf1, 1, CMP_SIZE, fref);
            if(!fout)
                bytes_read2 = (int)fread(buf2, 1, CMP_SIZE, fout);
            if (bytes_read1 != bytes_read2 || memcmp(buf1, buf2, bytes_read1))
            {
                fprintf(stderr, "encoder_test: the two files differ.\n");
                free(buf1);
                free(buf2);
                goto fail;
            }
            if (feof(fref) && feof(fout))
            {
                fprintf(stderr, "encoder_test: output matches the reference file.\n");
                break;
            }
            if (feof(fref) || feof(fout) || ferror(fref) || ferror(fout))
            {
                fprintf(stderr, "encoder_test: the two files differ.\n");
                free(buf1);
                free(buf2);
                goto fail;
            }
        }
        free(buf1);
        free(buf2);
    }
    return OS_THREAD_FUNC_RET_SUCCEEDED;
fail:
    if(output_filename)
        free(output_filename);
    return OS_THREAD_FUNC_RET_FAILED;
}

void encoder_event_handler(void *p_user_data,
                           jpeg_event_t event,
                           void        *p_arg)
{
    thread_ctrl_blk_t* p_thread_arg = (thread_ctrl_blk_t *)p_user_data;

    fprintf(stderr, "encoder_event_handler: %s\n", event_to_string[event]);
    // If it is not a warning event, encoder has stopped; Signal
    // main thread to clean up
    if (event == JPEG_EVENT_DONE || event == JPEG_EVENT_ERROR)
    {
        os_mutex_lock(&p_thread_arg->output_handler_args.mutex);
        p_thread_arg->encoding = false;
        os_cond_signal(&p_thread_arg->output_handler_args.cond);
        os_mutex_unlock(&p_thread_arg->output_handler_args.mutex);
    }
}

int  encoder_output_handler(void           *p_user_data,
                            void           *p_arg,
                            jpeg_buffer_t   buffer,
                            uint8_t         last_buf_flag)
{
    output_handler_args_t *p_output_args = (output_handler_args_t*)p_arg;
    uint32_t buf_size;
    uint8_t *buf_ptr;
    uint32_t rc;
    thread_ctrl_blk_t* p_thread_arg = (thread_ctrl_blk_t *)p_user_data;
    os_timer_t os_timer;
    int diff = 0;

    os_mutex_lock(&p_output_args->mutex);
    if (!p_output_args->fout)
    {
        fprintf(stderr, "encoder_output_handler: invalid p_arg\n");
        os_mutex_unlock(&p_output_args->mutex);
        return 1;
    }

    if (JPEG_FAILED(jpeg_buffer_get_actual_size(buffer, &buf_size)))
        fprintf(stderr, "encoder_output_handler: jpeg_buffer_get_actual_size failed\n");
    if (JPEG_FAILED(jpeg_buffer_get_addr(buffer, &buf_ptr)))
        fprintf(stderr, "encoder_output_handler: jpeg_buffer_get_addr failed\n");

    //fprintf(stderr, "encoder_output_handler: writing 0x%p (%d bytes)\n", buf_ptr, buf_size);
    if (!p_output_args->nowrite)
    {
        os_timer_start(&os_timer);
        fwrite(buf_ptr, 1, buf_size, p_output_args->fout);
        os_timer_get_elapsed(&os_timer, &diff, 0);
        fwrite_time += diff;
    }

    p_output_args->size += buf_size;
    os_mutex_unlock(&p_output_args->mutex);

    if (last_buf_flag)
    {
        fprintf(stderr, "encoder_output_handler:  received last output buffer\n");
    }

    // Set the output buffer offset to zero
    if (JPEG_FAILED(jpeg_buffer_set_actual_size(buffer, 0)))
        fprintf(stderr, "encoder_output_handler: jpeg_buffer_set_actual_size failed\n");

    // Enqueue back the output buffer to queue
    rc = jpege_enqueue_output_buffer((p_thread_arg->encoder),
                                      &buffer, 1);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "encoder_output_handler: jpege_enqueue_output_buffer failed\n");
        return 1;
    }
    return 0;
}

int read_bs_from_file(const char* file,
                      jpeg_buffer_t *p_luma_buf,
                      uint8_t use_pmem)
{
    FILE *f = fopen(file, "rb");
    long file_size;
    uint8_t *buf_ptr;

    if (!f)
    {
        fprintf(stderr, "read_bs_from_file: failed to open file %s\n", file);
        return 1;
    }

    // Find out input file size
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    fprintf(stderr, "%s: file_size=%ld\n", __func__, file_size);
    // Allocate buffer to hold the entire file
    if (jpeg_buffer_allocate(*p_luma_buf, file_size, use_pmem))
    {
        return 1;
    }

    // Read the content
    jpeg_buffer_get_addr(*p_luma_buf, &buf_ptr);
    fread(buf_ptr, 1, file_size, f);
    fclose(f);
    return 0;
}

static int use_padded_buffer(test_args_t *p_args)
{
    int use_padding = false;
    if ( ((p_args->main.format == YCRCBLP_H2V2)
        || (p_args->main.format == YCBCRLP_H2V2))
        && (((jpege_preference_t)p_args->preference == JPEG_ENCODER_PREF_HW_ACCELERATED_ONLY)
        || ((jpege_preference_t)p_args->preference == JPEG_ENCODER_PREF_HW_ACCELERATED_PREFERRED))
         ) {
        uint32_t actual_size = p_args->main.height*p_args->main.width;
        uint32_t padded_size = CEILING16(p_args->main.width) * CEILING16(p_args->main.height);
        if (actual_size != padded_size) {
            use_padding = true; // For gemini
        }
    }
    return use_padding;
}


int read_bytes_from_file_ion(const char* file, int width, int height,
  jpeg_buffer_t *p_luma_buf,
  jpeg_buffer_t *p_chroma_buf,
  uint8_t use_pmem,
  test_args_t *p_args,
  uint8_t is_thumbnail)
{
  FILE *f = fopen(file, "rb");
  long file_size;
  uint8_t *buf_ptr;
  uint32_t size = width*height;
  int ret = 0;
  int start_offset = 0;
  int use_offset;
  int cbcr_size = 0;
  os_timer_t os_timer;
  int diff = 0;

  if (!p_args) {
    fprintf(stderr, "read_bytes_from_file: p_args is NULL\n");
    return 1;
  }

  if (!f) {
    fprintf(stderr, "read_bytes_from_file: failed to open file %s\n", file);
    return 1;
  }

  // Find out input file size
  fseek(f, 0, SEEK_END);
  file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

    jpeg_buffer_allocate(*p_luma_buf, file_size, use_pmem);

    jpeg_buffer_attach_existing(*p_chroma_buf,
        *p_luma_buf,
        size);

    mapion_input0.bufsize = size + OS_Q6_PAGE;
    ion_heapid = (1<<ION_IOMMU_HEAP_ID);
    (void) apr_pmem_alloc_ion_cached(&mapion_input0,ion_heapid);
    ((jpeg_buf_t *)(*p_luma_buf))->ptr = mapion_input0.pVirtualAddr;
    ((jpeg_buf_t *)(*p_luma_buf))->size = size;


    mapion_input1.bufsize = file_size - size + OS_Q6_PAGE;
    ion_heapid = (1<<ION_IOMMU_HEAP_ID);
    (void) apr_pmem_alloc_ion_cached(&mapion_input1,ion_heapid);
    ((jpeg_buf_t *)(*p_chroma_buf))->ptr = mapion_input1.pVirtualAddr;

    ((jpeg_buf_t *)(*p_chroma_buf))->size = file_size - size;


    fread(mapion_input0.pVirtualAddr, 1, size, f);

    cbcr_size = file_size - size;

    fread(mapion_input1.pVirtualAddr, 1, cbcr_size, f);


  return 0;
}



int read_bytes_from_file(const char* file, int width, int height,
  jpeg_buffer_t *p_luma_buf,
  jpeg_buffer_t *p_chroma_buf,
  uint8_t use_pmem,
  test_args_t *p_args,
  uint8_t is_thumbnail)
{
  FILE *f = fopen(file, "rb");
  long file_size;
  uint8_t *buf_ptr;
  uint32_t size = width*height;
  int ret = 0;
  int start_offset = 0;
  int use_offset;
  int cbcr_size = 0;
  os_timer_t os_timer;
  int diff = 0;
  //fprintf(stderr,"read_bytes_from_file \n");
  use_pmem = 1;

  if (use_pmem)
  {
      read_bytes_from_file_ion(file,
                        width,
                        height, p_luma_buf,
                        p_chroma_buf,
                        use_pmem, p_args, false);
      return 0;
  }


  if (!p_args) {
    fprintf(stderr, "read_bytes_from_file: p_args is NULL\n");
    return 1;
  }
  use_offset = (p_args->rotation == 90) || (p_args->rotation == 180);

  int use_padding = is_thumbnail ? false : use_padded_buffer(p_args);
  fprintf(stderr, "read_bytes_from_file: use_padding %d use_pmem %d\n",
    use_padding, use_pmem);

  if (!f) {
    fprintf(stderr, "read_bytes_from_file: failed to open file %s\n", file);
    return 1;
  }
  os_timer_start(&os_timer);

  // Find out input file size
  fseek(f, 0, SEEK_END);
  file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  uint32_t actual_size = width*height;
  uint32_t padded_size = CEILING16(width) * CEILING16(height);
  fprintf(stderr, "read_bytes_from_file: file_size %ld padded_size %d\n",
    file_size, padded_size);
  if(use_padding){
    size = padded_size;
    if (jpeg_buffer_allocate(*p_luma_buf, padded_size*1.5, use_pmem) ) {
      fclose(f);
      return 1;
    }
  } else {
    if (jpeg_buffer_allocate(*p_luma_buf, file_size, use_pmem) ) {
      fclose(f);
      return 1;
    }
  }

  if (p_args->main.num_of_planes == 2)
  {
      jpeg_buffer_attach_existing(*p_chroma_buf,
        *p_luma_buf,
        size);
  }

  if(use_padding){
    size = actual_size;
  }

  // Read the content
  if (JPEG_FAILED(jpeg_buffer_get_addr(*p_luma_buf, &buf_ptr))) {
    fclose(f);
    return 1;
  }
  if (use_padding && use_offset)
    start_offset = padded_size-actual_size;
  fread(buf_ptr + start_offset, 1, size, f);

  if (p_args->main.num_of_planes == 2)
  {
      if (JPEG_FAILED (jpeg_buffer_get_addr(*p_chroma_buf, &buf_ptr))) {
        fclose(f);
        return 1;
      }
      cbcr_size = file_size - size;
      if (use_padding) {
        cbcr_size = size >> 1;
        if (use_offset) {
          start_offset = (padded_size-actual_size)>>1;
        }
      }

      fread(buf_ptr + start_offset, 1, cbcr_size, f);
  }
  fclose(f);

  if (use_padding){
    size = padded_size;
  }

  jpeg_buffer_set_actual_size(*p_luma_buf, size);

  fprintf(stderr," ptr, file size, fd 0x%x %d %d \n", ((jpeg_buf_t *)(*p_luma_buf))->ptr, file_size, ((jpeg_buf_t *)(*p_luma_buf))->pmem_fd);

  register_buf(((jpeg_buf_t *)(*p_luma_buf))->ptr, file_size, ((jpeg_buf_t *)(*p_luma_buf))->pmem_fd);

  if(use_padding) {
    if (p_args->main.num_of_planes == 2)
    {
      jpeg_buffer_set_actual_size(*p_chroma_buf, padded_size>>1);
    }
    if (use_offset) {
      jpeg_buffer_set_start_offset(*p_luma_buf, (padded_size-actual_size));
      if (p_args->main.num_of_planes == 2){
          jpeg_buffer_set_start_offset(*p_chroma_buf,
            ((padded_size-actual_size) >> 1));
      }
    }
  } else {
    if (p_args->main.num_of_planes == 2){
        jpeg_buffer_set_actual_size(*p_chroma_buf, file_size - size);
    }
  }

  return 0;
}


int read_bytes_from_file_3planes(const char* file, int width, int
height, jpeg_buffer_t *p_luma_buf,
  jpeg_buffer_t *p_cb_buf,
  jpeg_buffer_t *p_cr_buf,
  uint8_t use_pmem,
  test_args_t *p_args,
  uint8_t is_thumbnail)
{
  FILE *f = fopen(file, "rb");
  long file_size;
  uint8_t *buf_ptr;
  uint32_t size = width*height;
  int ret = 0;
  int start_offset = 0;
  int use_offset;
  int cb_size = 0, cr_size = 0;
  os_timer_t os_timer;
  int diff = 0;

  if (!p_args) {
    fprintf(stderr, "read_bytes_from_file: p_args is NULL\n");
    return 1;
  }
  os_timer_get_elapsed(&os_timer, &diff, 0);
  fprintf(stderr, "read_bytes_from_file: read complete time %d\n",
    diff);

  int use_padding = is_thumbnail ? false : use_padded_buffer(p_args);
  fprintf(stderr, "read_bytes_from_file: use_padding %d\n",use_padding);

  if (!f) {
    fprintf(stderr, "read_bytes_from_file: failed to open file %s\n", file);
    return 1;
  }

  // Find out input file size
  fseek(f, 0, SEEK_END);
  file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  uint32_t actual_size = width*height;
  uint32_t padded_size = CEILING16(width) * CEILING16(height);
  fprintf(stderr, "read_bytes_from_file: file_size %ld padded_size %d\n",
    file_size, padded_size);
  if(use_padding){
    size = padded_size;
    if (jpeg_buffer_allocate(*p_luma_buf, padded_size*1.5, use_pmem) ) {
      fclose(f);
      return 1;
    }
  } else {
    if (jpeg_buffer_allocate(*p_luma_buf, file_size, use_pmem) ) {
      fclose(f);
      return 1;
    }
  }

  if(p_args->main.num_of_planes ==3){
  jpeg_buffer_attach_existing(*p_cb_buf,
        *p_luma_buf,
        size);
  jpeg_buffer_attach_existing(*p_cr_buf,
        *p_luma_buf,
        (size+ (file_size - size)/2));
  }
  if(use_padding){
    size = actual_size;
  }

  // Read the content
  if (JPEG_FAILED(jpeg_buffer_get_addr(*p_luma_buf, &buf_ptr))) {
    fclose(f);
    return 1;
  }
  if (use_padding && use_offset)
    start_offset = padded_size-actual_size;
    fread(buf_ptr + start_offset, 1, size, f);

  if(p_args->main.num_of_planes ==3){
    if (JPEG_FAILED (jpeg_buffer_get_addr(*p_cb_buf, &buf_ptr))) {
        fclose(f);
        return 1;
    }
    cb_size = floor(file_size - size)/2;
    if (use_padding) {
      cb_size = size >> 2;
      if (use_offset) {
         start_offset = (padded_size-actual_size)>>1;
      }
    }
    fread(buf_ptr + start_offset, 1, cb_size, f);

    if (JPEG_FAILED (jpeg_buffer_get_addr(*p_cr_buf, &buf_ptr))) {
        fclose(f);
        return 1;
    }
    cr_size = (file_size - size)/2;
    if (use_padding) {
        cr_size = size >> 2;
        if (use_offset) {
          start_offset = 0;
        }
    }
    fread(buf_ptr + start_offset, 1, cr_size, f);
  }
  fclose(f);

  if (use_padding){
    size = padded_size;
  }

  jpeg_buffer_set_actual_size(*p_luma_buf, size);
  if (use_offset)
    jpeg_buffer_set_start_offset(*p_luma_buf, (padded_size-actual_size));

  if(p_args->main.num_of_planes ==3){
    if(use_padding) {
      jpeg_buffer_set_actual_size(*p_cb_buf, padded_size>>2);
      jpeg_buffer_set_actual_size(*p_cr_buf, padded_size>>2);

      if (use_offset) {
        jpeg_buffer_set_start_offset(*p_cb_buf,
            ((padded_size-actual_size) >> 1));
        jpeg_buffer_set_start_offset(*p_cr_buf,
            0);
      }
   }
   else {
      jpeg_buffer_set_actual_size(*p_cb_buf, (file_size - size)/2);
      jpeg_buffer_set_actual_size(*p_cr_buf, (file_size - size)/2);
   }
  }
  return 0;
}
