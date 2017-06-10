/**
 * Copyright (C) 2011 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
01/10/11   staceyw Created file.

========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include "jpeg_buffer.h"
#include "mpoe.h"
#include "os_thread.h"
#include "os_timer.h"

#if (defined WIN32 || defined WINCE)
#include "wingetopt.h"
#else
#include <getopt.h>
#endif

#define CMP_SIZE 1024

static const char color_formats[][30] =
{
    "YCRCBLP_H2V2",         // 0
    "YCBCRLP_H2V2",         // 1
    "YCRCBLP_H2V1",         // 2
    "YCBCRLP_H2V1",         // 3
    "YCRCBLP_H1V2",         // 4
    "YCBCRLP_H1V2",         // 5
    "YCRCBLP_H1V1",         // 6
    "YCBCRLP_H1V1",         // 7
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
static const char event_to_string[][30] =
{
    "EVENT_DONE",
    "EVENT_WARNING",
    "EVENT_ERROR",
    "EVENT_ABORTED",
    "EVENT_THUMBNAIL_DROPPED",
};
static const char *preference_str[] =
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

} input_image_args_t;

typedef struct
{
    input_image_args_t   main[2];
    input_image_args_t   thumbnail;
    char *               output_file;
    int16_t              rotation;
    uint32_t             preference;
    uint8_t              encode_thumbnail;
    uint16_t             back_to_back_count;
    uint32_t             output_buf_size;
    uint32_t             abort_time;
    const char*          reference_file;
    uint32_t             restart_interval;
    uint8_t              output_nowrite;
    uint8_t              use_pmem;
    mpoe_scale_cfg_t     main_scale_cfg;
    mpoe_scale_cfg_t     tn_scale_cfg;

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
    mpoe_obj_t            encoder;
    uint8_t               encoding;
    output_handler_args_t output_handler_args;
    test_args_t          *p_args;

} thread_ctrl_blk_t;

static int  read_yuv_from_file(const char *file, int width, int height,
                               jpeg_buffer_t *p_luma_buf,
                               jpeg_buffer_t *p_chroma_buf, uint8_t use_pmem);
static int  read_bs_from_file(const char *file,
                              jpeg_buffer_t *p_bitstream_buf,
                              uint8_t use_pmem);

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
            encoder_test(OS_THREAD_FUNC_ARG_T arg);
static void encoder_event_handler(void         *p_user_data,
                                  jpeg_event_t  event,
                                  void         *p_arg);
static int  encoder_output_handler(void           *p_user_data,
                                   void           *p_arg,
                                   jpeg_buffer_t   buffer,
                                   uint8_t         last_buf_flag);

extern int  jpeg_header_read(const uint8_t *ptr, uint32_t size, uint32_t *p_offset,
                             uint32_t *p_width, uint32_t *p_height, jpeg_subsampling_t *p_subsample,
                             uint8_t qtable_selectors[], uint16_t qtables[][64],
                             uint32_t *p_qtable_present_flag,
                             uint8_t  htable_dc_selectors[], uint8_t htable_ac_selectors[],
                             jpeg_huff_table_t htables[], uint32_t *p_htable_present_flag,
                             uint32_t *p_restart);

// Global variables
static thread_ctrl_blk_t *thread_ctrl_blks = NULL;
static int                concurrent_cnt = 1;
static uint8_t            jpeg_qtable_selectors[4];
static uint16_t           jpeg_qtables[4][64];
static uint32_t           jpeg_qtable_present_flag;
static uint8_t            jpeg_htable_dc_selectors[4];
static uint8_t            jpeg_htable_ac_selectors[4];
static jpeg_huff_table_t  jpeg_htables[8];
static uint32_t           jpeg_htable_present_flag;

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
    fprintf(stderr, "\t\t\t\tBITSTREAM_H2V2 (12), BITSTREAM_H2V1 (14), BITSTREAM_H1V2 (16), BITSTREAM_H1V1 (18)\n");
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
    test_args.main[0].format = 8;
    test_args.thumbnail.format = 8;
    test_args.back_to_back_count = 1;

    fprintf(stderr, "=============================================================\n");
    fprintf(stderr, "Encoder test\n");
    fprintf(stderr, "=============================================================\n");
    opterr = 1;
    while ((c = getopt(argc, argv, "I:O:W:H:F:Q:r:ti:w:h:f:q:p:c:um:n:x:y:M:N:Uj:k:X:Y:J:K:S:B:a:g:e:R:PZ")) != -1)
    {
        switch (c)
        {
        case 'O':
            test_args.output_file = optarg;
            fprintf(stderr, "%-25s%s\n", "Output image path", test_args.output_file);
            break;
        case 'I':
            test_args.main[0].file_name = optarg;
            fprintf(stderr, "%-25s%s\n", "Input image path", test_args.main[0].file_name);
            test_args.main[1].file_name = optarg; //"Data/input/7a.yuv"; //"Data/input/cherry_2M_1600x1200_full.ycrcb";
            fprintf(stderr, "%-25s%s\n", "Input image path", test_args.main[1].file_name);
            break;
        case 'i':
            test_args.thumbnail.file_name = optarg;
            fprintf(stderr, "%-25s%s\n", "Thumbnail image path", test_args.thumbnail.file_name);
            break;
        case 'Q':
            test_args.main[0].quality = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Main image quality", test_args.main[0].quality);
            break;
        case 'q':
            test_args.thumbnail.quality = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Thumbnail quality", test_args.thumbnail.quality);
            break;
        case 'W':
            test_args.main[0].width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Input width", test_args.main[0].width);
            break;
        case 'w':
            test_args.thumbnail.width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Thumbnail width", test_args.thumbnail.width);
            break;
        case 'H':
            test_args.main[0].height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Input height", test_args.main[0].height);
            break;
        case 'h':
            test_args.thumbnail.height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Thumbnail height", test_args.thumbnail.height);
            break;
        case 'r':
            test_args.rotation = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Rotation", test_args.rotation);
            break;
        case 't':
            test_args.encode_thumbnail = true;
            fprintf(stderr, "%-25s%s\n", "Encode thumbnail", "true");
            break;
        case 'F':
            test_args.main[0].format = atoi(optarg);
            fprintf(stderr, "%-25s%s\n", "Input format", color_formats[(uint8_t)test_args.main[0].format]);
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
            fprintf(stderr, "%-25s%s\n", "Main Scale enabled", "true");
            break;
        case 'm':
            test_args.main_scale_cfg.input_width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Main Scale input width", test_args.main_scale_cfg.input_width);
            break;
        case 'n':
            test_args.main_scale_cfg.input_height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Main Scale input height", test_args.main_scale_cfg.input_height);
            break;
        case 'M':
            test_args.main_scale_cfg.output_width = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Main Scale output width", test_args.main_scale_cfg.output_width);
            break;
        case 'N':
            test_args.main_scale_cfg.output_height = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Main Scale output height", test_args.main_scale_cfg.output_height);
            break;
        case 'x':
            test_args.main_scale_cfg.h_offset = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Main Scale h offset", test_args.main_scale_cfg.h_offset);
            break;
        case 'y':
            test_args.main_scale_cfg.v_offset = atoi(optarg);
            fprintf(stderr, "%-25s%d\n", "Main Scale v offset", test_args.main_scale_cfg.v_offset);
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
    if (!test_args.main[0].file_name || !test_args.output_file ||
        !test_args.main[0].width ||
        !test_args.main[0].height ||
        test_args.main[0].format == 8)
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
            rc = 1;
        }
    }

    if (!rc)
        fprintf(stderr, "encoder_test finished successfully\n");

    fprintf(stderr, "exit value: %d\n", rc);
    return rc;
}

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER encoder_test(OS_THREAD_FUNC_ARG_T arg)
{
    char *output_filename;
    thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)arg;
    int rc, i;
    mpoe_obj_t encoder;
    mpoe_src_t source[2];
    mpoe_dst_t dest;
    mpoe_cfg_t config[2];
    mpoe_img_cfg_t tn_cfg, main_cfg[2];
    mpoe_img_data_t img_info[2], tn_img_info;
    jpeg_buffer_t main_luma_buf[2], main_chroma_buf[2];
    jpeg_buffer_t tn_luma_buf, tn_chroma_buf;
    test_args_t *p_args = p_thread_arg->p_args;
    uint8_t use_pmem = true;
    os_timer_t os_timer;
    jpeg_buffer_t buffers[2];
    exif_info_obj_t exif_info[2];
    exif_tag_entry_t exif_data;
    mpo_info_obj_t mpo_info;
    mpo_index_obj_t mpo_index;
    mpo_attribute_obj_t mpo_attribute[2];
    exif_tag_entry_t mpo_data;

    FILE  *fout;
    uint32_t buf_size;
    uint8_t *buf_ptr;
    mpoe_app2_data_t  app2_data[2];
    uint8_t  default_value = 0;

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

        if (!output_filename) {
            fprintf(stderr, "encoder_test: output fail malloc failed\n");
            return OS_THREAD_FUNC_RET_FAILED;
        }
        snprintf(output_filename, sizeof(output_filename), "%s", p_args->output_file);
        if (s)
        {
            snprintf(output_filename + (s - p_args->output_file), sizeof(output_filename), "_%.2d%s", p_thread_arg->tid, s);
        }
        else
        {
            snprintf(output_filename, sizeof(output_filename), "%s_%.2d", output_filename, p_thread_arg->tid);
        }
    }

    // Determine whether pmem should be used (useful for pc environment testing where
    // pmem is not available)
    if ((mpoe_preference_t)p_args->preference == MPO_ENCODER_PREF_SOFTWARE_PREFERRED ||
        (mpoe_preference_t)p_args->preference == MPO_ENCODER_PREF_SOFTWARE_ONLY)
    {
        use_pmem = p_args->use_pmem;
    }

    // Set default configuration
    rc = mpoe_get_default_config(&config[0]);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "encoder_test: mpoe_set_default_config failed\n");
        goto fail;
    }
    tn_cfg = config[0].thumbnail_cfg;
    main_cfg[0] = config[0].main_cfg;

    rc = mpoe_get_default_config(&config[1]);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "encoder_test: mpoe_set_default_config failed\n");
        goto fail;
    }
    tn_cfg = config[1].thumbnail_cfg;
    main_cfg[1] = config[1].main_cfg;

    // Initialize source buffers
    if (jpeg_buffer_init(&main_luma_buf[0]) ||
        jpeg_buffer_init(&main_chroma_buf[0]) ||
        jpeg_buffer_init(&main_luma_buf[1]) ||
        jpeg_buffer_init(&main_chroma_buf[1]) ||
        jpeg_buffer_init(&tn_luma_buf) ||
        jpeg_buffer_init(&tn_chroma_buf))
    {
        goto fail;
    }

    // Open input file(s) and read the contents
    if (p_args->main[0].format <= YCBCRLP_H1V1)
    {
        if (read_yuv_from_file(p_args->main[0].file_name, p_args->main[0].width,
                               p_args->main[0].height, &main_luma_buf[0], &main_chroma_buf[0],
                               use_pmem))
            goto fail;
    }
    else if  ((p_args->main[0].format >= JPEG_BITSTREAM_H2V2) &&
              (p_args->main[0].format < JPEG_COLOR_FORMAT_MAX))
    {
        if (read_bs_from_file(p_args->main[0].file_name,
                                &main_luma_buf[0],
                                use_pmem))
            goto fail;
    }
    else
    {
        fprintf(stderr, "encoder_test: main inage color format not supported\n");
        goto fail;
    }

    if (p_args->encode_thumbnail)
    {
        if (p_args->thumbnail.format <= YCBCRLP_H1V1)
        {
            if (read_yuv_from_file(p_args->thumbnail.file_name, p_args->thumbnail.width,
                                   p_args->thumbnail.height, &tn_luma_buf, &tn_chroma_buf,
                                   use_pmem))
                goto fail;
        }
        else if ((p_args->thumbnail.format >= JPEG_BITSTREAM_H2V2) &&
                 (p_args->main[0].format < JPEG_COLOR_FORMAT_MAX))
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
    rc = mpoe_init(&encoder, &encoder_event_handler, (void *)p_thread_arg);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "encoder_test: mpoe_init failed\n");
        goto fail;
    }
    p_thread_arg->encoder = encoder;

    // Set source information (main)
    img_info[0].color_format           = p_args->main[0].format;
    img_info[0].width                  = p_args->main[0].width;
    img_info[0].height                 = p_args->main[0].height;
    img_info[0].fragment_cnt           = 1;
    img_info[0].p_fragments[0].width   = p_args->main[0].width;
    img_info[0].p_fragments[0].height  = p_args->main[0].height;
    if (img_info[0].color_format <= YCBCRLP_H1V1)
    {
        img_info[0].p_fragments[0].color.yuv.luma_buf   = main_luma_buf[0];
        img_info[0].p_fragments[0].color.yuv.chroma_buf = main_chroma_buf[0];
    }
    else if ((img_info[0].color_format >= JPEG_BITSTREAM_H2V2) &&
             (img_info[0].color_format < JPEG_COLOR_FORMAT_MAX))
    {
        img_info[0].p_fragments[0].color.bitstream.bitstream_buf = main_luma_buf[0];
    }

    source[0].p_main = &img_info[0];
    source[0].p_thumbnail = NULL;
    source[0].image_attribute.type = MULTI_VIEW_DISPARITY;

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

        tn_img_info.p_fragments[1].width   = p_args->thumbnail.width;
        tn_img_info.p_fragments[1].height  = p_args->thumbnail.height;
        if (tn_img_info.color_format <= YCBCRLP_H1V1)
        {
            tn_img_info.p_fragments[1].color.yuv.luma_buf    = tn_luma_buf;
            tn_img_info.p_fragments[1].color.yuv.chroma_buf  = tn_chroma_buf;
        }
        else if ((tn_img_info.color_format >= JPEG_BITSTREAM_H2V2) &&
                 (tn_img_info.color_format <= YCBCRLP_H1V1))
        {
            tn_img_info.p_fragments[1].color.bitstream.bitstream_buf = tn_luma_buf;
        }
        source[0].p_thumbnail = &tn_img_info;
    }


    img_info[1].color_format           = p_args->main[0].format;
    img_info[1].width                  = p_args->main[0].width;
    img_info[1].height                 = p_args->main[0].height;
    img_info[1].fragment_cnt           = 1;
    img_info[1].p_fragments[0].width   = p_args->main[0].width;
    img_info[1].p_fragments[0].height  = p_args->main[0].height;
    if (read_yuv_from_file(p_args->main[1].file_name, p_args->main[0].width,
                               p_args->main[0].height, &main_luma_buf[1], &main_chroma_buf[1],
                               use_pmem))
        goto fail;

    if (img_info[1].color_format <= YCBCRLP_H1V1)
    {
        img_info[1].p_fragments[0].color.yuv.luma_buf   = main_luma_buf[1];
        img_info[1].p_fragments[0].color.yuv.chroma_buf = main_chroma_buf[1];
    }
    else if ((img_info[1].color_format >= JPEG_BITSTREAM_H2V2) &&
             (img_info[1].color_format < JPEG_COLOR_FORMAT_MAX))
    {
        img_info[1].p_fragments[0].color.bitstream.bitstream_buf = main_luma_buf[1];
    }
    source[1].p_main = &img_info[1];
    source[1].p_thumbnail = NULL;
    source[1].image_attribute.type = MULTI_VIEW_DISPARITY;


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

        tn_img_info.p_fragments[1].width   = p_args->thumbnail.width;
        tn_img_info.p_fragments[1].height  = p_args->thumbnail.height;
        if (tn_img_info.color_format <= YCBCRLP_H1V1)
        {
            tn_img_info.p_fragments[1].color.yuv.luma_buf    = tn_luma_buf;
            tn_img_info.p_fragments[1].color.yuv.chroma_buf  = tn_chroma_buf;
        }
        else if ((tn_img_info.color_format >= JPEG_BITSTREAM_H2V2) &&
                 (tn_img_info.color_format <= YCBCRLP_H1V1))
        {
            tn_img_info.p_fragments[1].color.bitstream.bitstream_buf = tn_luma_buf;
        }
        source[1].p_thumbnail = &tn_img_info;
    }

    rc = mpoe_set_source(encoder, &source, 2);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "encoder_test: mpoe_set_source failed\n");
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
        if ((p_thread_arg->output_handler_args.fout = fopen(output_filename, "wb")) ==0 )
        {
            fprintf(stderr, "encoder_test: failed to open output file: %s\n", output_filename);
            goto fail;
        }
        p_thread_arg->output_handler_args.size = 0;
        p_thread_arg->output_handler_args.nowrite = p_args->output_nowrite;


        // Set default output buf size equal to 8K
        // if it is not provided by user
        if (!p_thread_arg->p_args->output_buf_size)
        {
            p_thread_arg->p_args->output_buf_size = 16000; //2 * p_args->main[0].width * p_args->main[0].height;
        }



        // Set destination information
        dest.p_output_handler = encoder_output_handler;
        dest.p_arg = (void*)&p_thread_arg->output_handler_args;
        dest.buffer_cnt = 1;
        if (JPEG_FAILED(jpeg_buffer_init(&buffers[0])) ||
            JPEG_FAILED(jpeg_buffer_allocate(buffers[0], p_thread_arg->p_args->output_buf_size, use_pmem)))
        {
            fprintf(stderr, "encoder_test: failed to allocate destination buffers\n");
            jpeg_buffer_destroy(&buffers[0]);
            jpeg_buffer_destroy(&buffers[1]);
            dest.buffer_cnt = 0;
        }

        dest.p_buffer = &buffers[0];
        rc = mpoe_set_destination(encoder, &dest);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "encoder_test: mpoe_set_destination failed\n");
            goto fail;
        }

        fprintf(stderr, "before qualirt is %d %d\n", main_cfg[0].quality, main_cfg[1].quality);

        // Set custom configuration
        config[0].thumbnail_cfg = tn_cfg;
        config[0].main_cfg = main_cfg[0];
        config[0].main_cfg.rotation_degree_clk = p_args->rotation;
        if (p_args->main[0].quality)
            config[0].main_cfg.quality = p_args->main[0].quality;



        config[0].thumbnail_cfg.rotation_degree_clk = p_args->rotation;
        if (p_args->thumbnail.quality)
            config[0].thumbnail_cfg.quality = p_args->thumbnail.quality;
        config[0].thumbnail_present = p_args->encode_thumbnail;
        if (p_args->preference < MPO_ENCODER_PREF_MAX)
        {
            config[0].preference = (mpoe_preference_t)p_args->preference;
        }



        // Set scale cfg
        config[0].main_cfg.scale_cfg = p_args->main_scale_cfg;
        config[0].thumbnail_cfg.scale_cfg = p_args->tn_scale_cfg;

        // Specify restart interval
        config[0].main_cfg.restart_interval = p_args->restart_interval;

        // Set scale cfg
        config[1].main_cfg.rotation_degree_clk = 0;
        config[1].main_cfg.scale_cfg.enable = true;
        config[1].main_cfg.scale_cfg.h_offset = 320;
        config[1].main_cfg.scale_cfg.v_offset = 0;
        config[1].main_cfg.scale_cfg.input_width = 320;
        config[1].main_cfg.scale_cfg.input_height = 480;
        config[1].main_cfg.scale_cfg.output_width = 320;
        config[1].main_cfg.scale_cfg.output_height = 480;


        // set orientation flag
        rc = exif_init(&exif_info[0]);
        exif_data.type = EXIF_RATIONAL;
        exif_data.count = 1;
        //exif_data.data._short = 10;
        exif_data.data._rat.denom = 7;
        exif_data.data._rat.num = 1;
        rc = exif_set_tag(exif_info[0], EXIFTAGID_EXIF_COMPRESSED_BITS_PER_PIXEL, &exif_data);
        // Test delete tags
        rc = exif_delete_tag(exif_info[0], EXIFTAGID_EXIF_COMPRESSED_BITS_PER_PIXEL);


    /************************
    * test mpo header
    ***********************/
    // Set mpo info
    rc = mpo_info_init(&(mpo_info));
    mpo_data.type = EXIF_LONG;
    mpo_data.count = 1;
    mpo_data.data._long = 2;
    rc = mpo_set_index_tag(mpo_info, MPOTAGID_NUMBER_OF_IMAGES, &mpo_data);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "encoder_test: mpo_set_tag failed\n");
            mpo_info_destroy(&(mpo_info));
            goto fail;
        }

        mpo_data.type = EXIF_UNDEFINED;
        mpo_data.count = 16 * 2;
        mpo_data.data._undefined = &default_value;
        rc = mpo_set_index_tag(mpo_info, MPOTAGID_MP_ENTRY, &mpo_data);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "encoder_test: mpo_set_tag failed\n");
            mpo_info_destroy(&(mpo_info));
            goto fail;
        }

        mpo_data.type = EXIF_LONG;
        mpo_data.count = 1;
        mpo_data.data._long = 1;
        rc = mpo_set_attribute_tag(mpo_info, MPOTAGID_MP_INDIVIDUAL_NUM, &mpo_data, 0);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "encoder_test: mpo_set_tag failed\n");
            mpo_info_destroy(&(mpo_info));
            goto fail;
        }

        mpo_data.type = EXIF_LONG;
        mpo_data.count = 1;
        mpo_data.data._long = 2;
        rc = mpo_set_attribute_tag(mpo_info, MPOTAGID_MP_INDIVIDUAL_NUM, &mpo_data, 1);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "encoder_test: mpo_set_tag failed\n");
            mpo_info_destroy(&(mpo_info));
            goto fail;
        }

        // Start encoding
        p_thread_arg->encoding = true;
        if (os_timer_start(&os_timer) < 0)
        {
            fprintf(stderr, "encoder_test: failed to get start time\n");
        }
        rc = mpoe_start(encoder, &config, &exif_info, &mpo_info, 2);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "encoder_test: mpoe_start failed\n");
            goto fail;
        }
        fprintf(stderr, "encoder_test: mpoe_start succeeded\n");



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
                    rc = mpoe_abort(encoder);
                    if (rc)
                    {
                        fprintf(stderr, "encoder_test: mpoe_abort failed: %d\n", rc);
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
        exif_destroy(&exif_info[0]);
        rc = mpoe_get_actual_config(encoder, &config, 0);

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
                    fprintf(stderr, "encoder_test: mpo size: %ld bytes\n",
                            (long)p_thread_arg->output_handler_args.size);
                }
            }
        }

        // Clean up allocated dest buffers
        jpeg_buffer_destroy(&buffers[0]);
        jpeg_buffer_destroy(&buffers[1]);

        fclose(p_thread_arg->output_handler_args.fout);
    }

    // Open output file and update

    rc = (fout = fopen( output_filename, "r+")) == 0;
    if (!rc)
        goto fail;
    jpeg_buffer_init(&app2_data[0].data_buf);
    jpeg_buffer_init(&app2_data[1].data_buf);
    for (i = 0; i < 2; i++)
    {
        if (JPEG_SUCCEEDED(rc))
        {
           rc = jpeg_buffer_allocate(app2_data[i].data_buf,
                                     1000, false);
        }
    }

    // Get App2 header
    mpoe_get_app2_header(encoder, &app2_data, 2);

    // Update all App2 header
    for (i = 0; i < 2; i++)
    {
        jpeg_buffer_get_addr(app2_data[i].data_buf, &buf_ptr);
        jpeg_buffer_get_actual_size(app2_data[i].data_buf, &buf_size);

        // Find App2 header start
        fseek(fout, app2_data[i].start_offset, SEEK_SET);

        // Write and update a App2 header into file
        fwrite(buf_ptr, 1, buf_size, fout);
    }

    // Close output file
    rc = fclose(fout);

    // Clean up allocated data buffers
    jpeg_buffer_destroy(&app2_data[0].data_buf);
    jpeg_buffer_destroy(&app2_data[1].data_buf);

    // Delete tags
    rc = mpo_delete_index_tag(mpo_info, MPOTAGID_NUMBER_OF_IMAGES);
    rc = mpo_delete_index_tag(mpo_info, MPOTAGID_MP_F_VERSION);
    rc = mpo_delete_attribute_tag(mpo_info, MPOTAGID_MP_INDIVIDUAL_NUM, 0);
    rc = mpo_delete_attribute_tag(mpo_info, MPOTAGID_MP_INDIVIDUAL_NUM, 1);

    // Destroy mpo info
    mpo_info_destroy(&(mpo_info));

    // Clean up allocated source buffers
    jpeg_buffer_destroy(&main_luma_buf[0]);
    jpeg_buffer_destroy(&main_chroma_buf[0]);
    jpeg_buffer_destroy(&main_luma_buf[1]);
    jpeg_buffer_destroy(&main_chroma_buf[1]);
    jpeg_buffer_destroy(&tn_luma_buf);
    jpeg_buffer_destroy(&tn_chroma_buf);

    // Clean up encoder
    mpoe_destroy(&encoder);

    // Do bitwise comparison if requested
    if (p_args->reference_file)
    {
        FILE *fout;
        char *buf1;
        char *buf2;
        FILE *fref;
        int bytes_read1, bytes_read2;
        buf1 = malloc(CMP_SIZE);
        if (!buf1)
        {
            fprintf(stderr, "encoder_test: failed to malloc for buffer for file comparison\n");
            goto fail;
        }
        buf2 = malloc(CMP_SIZE);
        if (!buf2)
        {
            free(buf1);
            fprintf(stderr, "encoder_test: failed to malloc for buffer for file comparison\n");
            goto fail;
        }
        fprintf(stderr, "encoder_test: comparing %s with %s...\n",
                output_filename, p_args->reference_file);
        if ((fref = fopen(p_args->reference_file, "rb")) ==0 )
        {
            fprintf(stderr, "encoder_test: error opening reference file: %s\n", p_args->reference_file);
            free(buf1);
            free(buf2);
            goto fail;
        }
        fout = fopen(output_filename, "rb");
        for (;;)
        {
            bytes_read1 = (int)fread(buf1, 1, CMP_SIZE, fref);
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
        fwrite(buf_ptr, 1, buf_size, p_output_args->fout);
    }
    p_output_args->size += buf_size;
    os_mutex_unlock(&p_output_args->mutex);

    if (last_buf_flag && (!p_output_args->nowrite))
    {
        fprintf(stderr, "encoder_output_handler: received last output buffer 0x%p (%d bytes)\n",
                buf_ptr, buf_size);
    }

    // Set the output buffer offset to zero
    if (JPEG_FAILED(jpeg_buffer_set_actual_size(buffer, 0)))
        fprintf(stderr, "encoder_output_handler: jpeg_buffer_set_actual_size failed\n");

    // Enqueue back the output buffer to queue
    rc = mpoe_enqueue_output_buffer((p_thread_arg->encoder),
                                      &buffer, 1);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "encoder_output_handler: mpoe_enqueue_output_buffer failed\n");
        return 1;
    }

    return 0;
}

static int read_bs_from_file(const char    *file,
                             jpeg_buffer_t *p_bitstream_buf,
                             uint8_t        use_pmem)
{
    FILE           *f;
    uint32_t        file_size;
    uint8_t        *buf_ptr;

    if ( ( f= fopen(file, "rb")) == 0)
    {
        fprintf(stderr, "read_bs_from_file: failed to open file %s\n", file);
        return 1;
    }

    // Find out input file size
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Allocate buffer to hold the entire file
    if (jpeg_buffer_allocate(*p_bitstream_buf, file_size, use_pmem))
    {
        return 1;
    }

    // Read the content
    jpeg_buffer_get_addr(*p_bitstream_buf, &buf_ptr);
    fread(buf_ptr, 1, file_size, f);
    fclose(f);

    // Set bitstream size
    jpeg_buffer_set_actual_size(*p_bitstream_buf, file_size);

    return 0;
}

static int read_yuv_from_file(const char* file, int width, int height,
                              jpeg_buffer_t *p_luma_buf,
                              jpeg_buffer_t *p_chroma_buf,
                              uint8_t use_pmem)
{
    FILE *f;
    long file_size;
    uint8_t *buf_ptr;

    if ((f =fopen(file, "rb")) == 0)
    {
        fprintf(stderr, "read_yuv_from_file: failed to open file %s\n", file);
        return 1;
    }

    // Find out input file size
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Allocate buffer to hold the entire file
    if (jpeg_buffer_allocate(*p_luma_buf, width * height, use_pmem) ||
        jpeg_buffer_allocate(*p_chroma_buf, file_size - width * height, use_pmem))
    {
        return 1;
    }

    // Read the content
    jpeg_buffer_get_addr(*p_luma_buf, &buf_ptr);
    fread(buf_ptr, 1, width * height, f);
    jpeg_buffer_get_addr(*p_chroma_buf, &buf_ptr);
    fread(buf_ptr, 1, file_size - width * height, f);
    fclose(f);
    return 0;
}

