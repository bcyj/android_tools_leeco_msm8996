/**
 * Copyright (C) 2011 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
03/15/11   mingy   Updated included header file list.
03/10/11   mingy   Created file.

========================================================================== */

#define STRSAFE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include "jpeg_buffer.h"
#include "jpeg_common.h"
#include "jpegd.h"
#include "os_thread.h"
#include "os_timer.h"
#include "mpod.h"

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
#define TILE_HEIGHT 16

#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

const char color_formats[11][13] =
{
    "YCRCBLP_H2V2",
    "YCBCRLP_H2V2",
    "YCRCBLP_H2V1",
    "YCBCRLP_H2V1",
    "YCRCBLP_H1V2",
    "YCBCRLP_H1V2",
    "YCRCBLP_H1V1",
    "YCBCRLP_H1V1",
    "RGB565",
    "RGB888",
    "RGBa",
};
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
typedef struct
{
    char                 *input_file;
    char                 *output_file;
    char                 *thumbnail_file;
    char                 *exif_dump_file;
    char                 *mpo_dump_file;
    uint32_t              width;
    uint32_t              height;
    uint32_t              format;
    uint32_t              preference;
    uint32_t              abort_time;
    uint32_t              back_to_back_count;
    char                 *reference_file;
    uint8_t               thumbnail_flag;
    uint32_t              stride;
    int32_t               rotation;
    jpeg_rectangle_t      region;
    uint8_t               tiling_enabled;
    uint8_t               output_nowrite;

} test_args_t;

typedef struct
{
    int                   tid;
    os_thread_t           thread;
    mpod_obj_t            mpo_decoder;
    FILE*                 fin;
    uint8_t               decoding;
    uint8_t               decode_success;
    os_mutex_t            mutex;
    os_cond_t             cond;
    test_args_t          *p_args;
    jpeg_buffer_t         p_entire_input_buffer;  // buffer to hold entrie input Jpeg file
    mpod_output_buf_t    *p_whole_output_buf;     // buffer to hold entrie output decoded file

} thread_ctrl_blk_t;

OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER decoder_test(OS_THREAD_FUNC_ARG_T p_thread_args);
void decoder_dump_output(char               *output_file,
                         mpod_dst_t         *p_dst,
                         mpod_output_buf_t  *p_output_buffer);
void decoder_event_handler(void        *p_user_data,
                           jpeg_event_t event,
                           void        *p_arg);
int decoder_output_handler(void               *p_user_data,
                           mpod_output_buf_t  *p_output_buffer,
                           uint32_t            first_row_id,
                           uint8_t             is_last_buffer);
int decoder_test_dump_exif(FILE*           dump_file,
                           exif_info_obj_t exif);
uint32_t decoder_input_req_handler(void           *p_user_data,
                                   jpeg_buffer_t   buffer,
                                   uint32_t        start_offset,
                                   uint32_t        length);

int mpod_test_dump_index(FILE* dump_file, mpo_index_obj_t mpo_index);
int mpod_test_dump_attribute(FILE* dump_file, mpo_attribute_obj_t mpo_attribute);
void append_filename(char *original_name, char **appended_name, int append_id);


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
    fprintf(stderr, "\t\t\t\tYCRCBLP_H2V2 (%d - Default), YCBCRLP_H2V2 (%d), YCRCBLP_H2V1 (%d), YCBCRLP_H2V1 (%d), RGB565 (%d), RGB888 (%d), RGBA (%d)\n",
                    YCRCBLP_H2V2, YCBCRLP_H2V2, YCRCBLP_H2V1, YCBCRLP_H2V1, RGB565, RGB888, RGBa);
    fprintf(stderr, "  -p PREFERENCE\t\tPreference on which decoder to use (Software-based or Hardware-accelerated).\n");
    fprintf(stderr, "               \t\t\tHW preferred (0), HW only (1), SW preferred (2), SW only (3)\n");
    fprintf(stderr, "  -x FILE\t\tPath to the output text file which contains a dump of the EXIF tags found in the file.\n");
    fprintf(stderr, "  -m FILE\t\tPath to the output text file which contains a dump of the MPO tags found in the file.\n");
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
    fprintf(stderr, "  -l TILING ENABLED\t\tSpecify if the tiling is enabled. (1 to enalble, 0 to disable(default) )\tTiling is only supported when output format is rgb\n");
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
    test_args_t  test_args;
    memset(&test_args, 0, sizeof(test_args_t));
    test_args.preference = JPEG_DECODER_PREF_HW_ACCELERATED_PREFERRED;
    test_args.back_to_back_count = 1;

    fprintf(stderr, "=============================================================\n");
    fprintf(stderr, "Decoder test\n");
    fprintf(stderr, "=============================================================\n");
    opterr = 1;
    // Notice the below colons are not dividers.
    // A colon is attached to its left letter meaning a specific paramter is required.
    // If no parameter is required, no colon is needed, eg: "Z" w/o colon attached.
    while ((c = getopt(argc, argv, "i:o:t:w:h:f:p:x:m:a:g:c:e:s:r:L:T:R:B:l:Z")) != -1)
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
        case 'f':
            test_args.format = atoi(optarg);
            if (test_args.format != YCRCBLP_H2V2 && test_args.format != YCBCRLP_H2V2 &&
                test_args.format != YCRCBLP_H2V1 && test_args.format != YCBCRLP_H2V1 &&
                test_args.format != RGB565 &&
                test_args.format != RGB888 &&
                test_args.format != RGBa)
            {
                fprintf(stderr, "Invalid output format.\n");
                return 1;
            }
            fprintf(stderr, "%-25s%s\n", "Output format", color_formats[test_args.format]);
            break;
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
        case 'm':
            test_args.mpo_dump_file = optarg;
            fprintf(stderr, "%-25s%s\n", "MPO dump file", optarg);
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
            if (!concurrent_cnt)
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
        case 'l':
            test_args.tiling_enabled = (uint8_t)atoi(optarg);
            if (test_args.tiling_enabled)
            {
                fprintf(stderr, "%-25s%s\n", "Tiling Enalbed?", "Yes");
            }
            else
            {
                fprintf(stderr, "%-25s%s\n", "Tiling Enalbed?", "No");
            }
            break;
        case 'Z':
            test_args.output_nowrite = true;
            fprintf(stderr, "%-25s%s\n", "Output write disabled", "true");
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
            rc = OS_THREAD_FUNC_RET_FAILED;
        }
    }

    if (!rc)
        fprintf(stderr, "decoder_test finished successfully\n");

    fprintf(stderr, "exit value: %d\n", rc);
    return rc;
}

OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER decoder_test(OS_THREAD_FUNC_ARG_T arg)
{
    char               *output_filename;
    int                 rc;
    uint32_t            i;
    mpod_src_t          source;
    mpod_dst_t          dest_thumb, dest;
    mpod_output_buf_t   p_output_buffers_thumb, p_output_buffers;
    uint32_t            output_buffers_count = 1; // currently only 1 buffer a time is supported
    mpod_cfg_t          config;
    long                file_size;
    FILE               *fin;
    uint8_t             use_pmem = true;
    os_timer_t          os_timer;
    thread_ctrl_blk_t  *p_thread_arg = (thread_ctrl_blk_t *)arg;
    test_args_t        *p_args = p_thread_arg->p_args;
    uint32_t            stride;
    uint32_t            output_width;
    uint32_t            output_height;
    uint8_t             tiling_flag = false;
    uint8_t            *buf_ptr;
    mpod_obj_t          mpo_decoder;
    mpo_hdr_t           header;
    mp_entry_t          mp_entry[8];
    uint32_t            num_images;
    uint32_t            image_index;

    // Define a final image buffer for the tiling enabled cases.
    // This buffer holds the whole image copied tile by tile from the tiling buffer
    mpod_output_buf_t   p_final_image_buf;

    // Determine whether pmem should be used (useful for pc environment testing where
    // pmem is not available)
    if ((jpegd_preference_t)p_args->preference == JPEG_DECODER_PREF_SOFTWARE_PREFERRED ||
        (jpegd_preference_t)p_args->preference == JPEG_DECODER_PREF_SOFTWARE_ONLY)
    {
        use_pmem = false;
    }

    output_filename = p_args->output_file;

    // Append the output file name with a number to avoid multiple writing to the same file
    if (p_thread_arg->tid && !OUTPUT_IS_SCREEN(p_args->output_file))
    {
        append_filename(p_args->output_file, &output_filename, p_thread_arg->tid);
    }

    /**************************************************************************
    *   Input preparation...
    **************************************************************************/

    // Open input file
    fin = fopen(p_args->input_file, "rb");
    if (!fin)
        goto fail;

    // Find out input file length
    fseek(fin, 0, SEEK_END);
    file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    if (file_size > 0xffffffff)
    {
        fprintf(stderr, "decoder_test: input file too large for decoder\n");
        goto fail;
    }

    // Initialize and allocate buffer to hold the entire input Jpeg file
    rc = jpeg_buffer_init(&p_thread_arg->p_entire_input_buffer);
    if (JPEG_SUCCEEDED(rc))
    {
        rc = jpeg_buffer_allocate(p_thread_arg->p_entire_input_buffer, file_size, use_pmem);
    }
    if (JPEG_FAILED(rc))
    {
        jpeg_buffer_destroy(&p_thread_arg->p_entire_input_buffer);
        goto fail;
    }

    // Read the entrie Jpeg content
    jpeg_buffer_get_addr(p_thread_arg->p_entire_input_buffer, &buf_ptr);
    fread(buf_ptr, 1, file_size, fin);

     // Initialize MPO decoder
    rc = mpod_init(&mpo_decoder,
                   &decoder_event_handler,
                   &decoder_output_handler,
                   p_thread_arg);

    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "mpod_test: mpod_init failed\n");
        goto fail;
    }

    p_thread_arg->mpo_decoder = mpo_decoder;

    p_thread_arg->fin = fin;

    // Set source information
    source.p_input_req_handler = &decoder_input_req_handler;
    source.total_length        = file_size & 0xffffffff;
    rc = jpeg_buffer_init(&source.buffers[0]);
    if (JPEG_SUCCEEDED(rc))
    {
        rc = jpeg_buffer_init(&source.buffers[1]);
    }
    if (JPEG_SUCCEEDED(rc))
    {
        rc = jpeg_buffer_allocate(source.buffers[0], 0xA000, use_pmem);
    }
    if (JPEG_SUCCEEDED(rc))
    {
        rc = jpeg_buffer_allocate(source.buffers[1], 0xA000, use_pmem);
    }
    if (JPEG_FAILED(rc))
    {
        jpeg_buffer_destroy(&source.buffers[0]);
        jpeg_buffer_destroy(&source.buffers[1]);
        goto fail;
    }

    rc = mpod_set_source(mpo_decoder, &source);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "mpod_test: mpod_set_source failed\n");
        goto fail;
    }


    /**************************************************************************
    *
    *  Decode the FIRST individual images in MPO...
    *
    **************************************************************************/

    // Parse header from the first individual image
    rc = mpod_read_first_header(mpo_decoder, &header);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "mpod_test: mpod_read_header_first failed\n");
        goto fail;
    }

    // Print out the frame information
    fprintf(stderr, "main dimension: (%dx%d) subsampling: (%d)\n",
            header.main.width, header.main.height, (int)header.main.subsampling);

    // Get the number of images in MPO
    rc = mpod_get_num_image(mpo_decoder, &num_images);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "mpod_test: mpod_get_num_image failed\n");
        goto fail;
    }

    // Extract MP_ENTRY values
    rc = mpod_get_mp_entry_value(mpo_decoder, mp_entry, num_images);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "mpod_test: mpod_get_mp_entry_value failed\n");
        goto fail;
    }

    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "mpo_decoder_test: mpod_get_mp_entry_value failed\n");
        goto fail;
    }

    // Dump exif tag information if requested
    if (p_args->exif_dump_file && p_thread_arg->tid == 0)
    {
        FILE *exif_dump;
        exif_dump = fopen(p_args->exif_dump_file, "w");
        if (!exif_dump)
        {
            fprintf(stderr, "decoder_test: failed to open file for dumping exif tags: %s\n",
                    p_args->exif_dump_file);
        }
        else
        {
            decoder_test_dump_exif(exif_dump, header.exif_info);
            fclose(exif_dump);
        }
    }

    // Dump MPO tag information if requested
    if (p_args->mpo_dump_file && p_thread_arg->tid == 0)
    {
        FILE *mpo_dump = fopen(p_args->mpo_dump_file, "w");

        if (!mpo_dump)
        {
            fprintf(stderr, "mpo_decoder_test: failed to open file for dumping mpo tags: %s\n",
                    p_args->mpo_dump_file);
        }
        else
        {
            fprintf(mpo_dump, "**********Index IFD:**********\n");
            mpod_test_dump_index(mpo_dump, header.mpo_index_obj);

            fprintf(mpo_dump, "\n******Attribute IFD from the 1st individual image:*********\n");
            mpod_test_dump_attribute(mpo_dump, header.mpo_attribute_obj);

            fclose(mpo_dump);
        }
    }

    // if thumbnail file is specified, decode thumbnail first.
    if (1 == p_args->thumbnail_flag)
    {
        // Temporary disable tiling when decoding thumbnail
        tiling_flag = p_args->tiling_enabled;
        p_args->tiling_enabled = 0;

        // check if there is an thumbnail
        if (header.thumbnail.height != 0 && header.thumbnail.width != 0)
        {
            // thumbnail output configuration
            dest_thumb.width         = header.thumbnail.width;
            dest_thumb.height        = header.thumbnail.height;
            dest_thumb.output_format = RGB888;
            dest_thumb.stride        = 0;

            dest_thumb.region.left   = 0;
            dest_thumb.region.top    = 0;
            dest_thumb.region.right  = 0;
            dest_thumb.region.bottom = 0;

            jpeg_buffer_init(&p_output_buffers_thumb.data.rgb.rgb_buf);

            // Assign 0 to tile width and height
            // to indicate that no tiling is requested for thumbnail.
            p_output_buffers_thumb.tile_width  = 0;
            p_output_buffers_thumb.tile_height = 0;

            stride = dest_thumb.width * 3;

            // output buffer allocation
            use_pmem = OUTPUT_IS_SCREEN(p_args->thumbnail_file);
            jpeg_buffer_allocate(p_output_buffers_thumb.data.rgb.rgb_buf, stride * dest_thumb.height, use_pmem);

            // config decoder to decode thumbnail.
            memset(&config, 0, sizeof(mpod_cfg_t));
            config.preference = p_args->preference;
            config.decode_from = JPEGD_DECODE_FROM_THUMB;
            config.rotation = 0;

            // start decoding
            p_thread_arg->decoding = true;
            if (os_timer_start(&os_timer) < 0)
            {
                fprintf(stderr, "decoder_test thumbnail: failed to get start time\n");
            }
            rc = mpod_start(mpo_decoder,
                            &config,
                            &dest_thumb,
                            &p_output_buffers_thumb,
                            1);

            if (JPEG_FAILED(rc))
            {
                fprintf(stderr, "decoder_test thumnail: jpegd_start failed\n");
                goto fail;
            }
            fprintf(stderr, "decoder_test thumbnail: jpegd_start succeeded\n");

            // Wait until decoding is done or stopped due to error
            os_mutex_lock(&p_thread_arg->mutex);
            while (p_thread_arg->decoding)
            {
                os_cond_wait(&p_thread_arg->cond, &p_thread_arg->mutex);
            }
            os_mutex_unlock(&p_thread_arg->mutex);

            // check thumbnail decoding results
            {
                int diff;
                if (os_timer_get_elapsed(&os_timer, &diff, 0) < 0)
                {
                    fprintf(stderr, "decoder_test: failed to get elapsed time\n");
                }
                else
                {
                    if (p_thread_arg->decode_success)
                    {
                        fprintf(stderr, "decoder_test: decode time: %d ms\n", diff);
                    }
                    else
                    {
                        fprintf(stderr, "decoder_test: decode failed\n");
                    }

                    // Dump output under either of the conditions:
                    // 1. Output is to the screen AND it is the first thread
                    // 2. Output is to files
                    if (!OUTPUT_IS_SCREEN(p_args->thumbnail_file) || p_thread_arg->tid == 0)
                    {
                        decoder_dump_output(p_args->thumbnail_file, &dest_thumb, &p_output_buffers_thumb);
                    }
                }
            }
            // clean up buffers
            jpeg_buffer_destroy(&p_output_buffers_thumb.data.rgb.rgb_buf);

            if (!p_thread_arg->decode_success)
            {
                fprintf(stderr, "decoder_test: thumbnail decode failed\n");
                return OS_THREAD_FUNC_RET_FAILED;
            }
        }
        else
        {
            fprintf(stderr, "decoder_test: Jpeg file does not contain thumbnail \n");
        }

        // Restore the tiling flag after thumbnail has been decoded
        p_args->tiling_enabled = tiling_flag;
    }

    // main image decoding:
    // Set destination information
    dest.width         = p_args->width  ? p_args->width  : header.main.width;
    dest.height        = p_args->height ? p_args->height : header.main.height;
    dest.output_format = p_args->format;
    dest.stride        = p_args->stride;
    dest.region        = p_args->region;

    // if region is defined, re-assign the output width/height
    output_width  = dest.width;
    output_height = dest.height;

    if (p_args->region.right || p_args->region.bottom)
    {
        if (0 == p_args->rotation || 180 == p_args->rotation)
        {
            output_width  = MIN(dest.width,  (uint32_t)(dest.region.right  - dest.region.left + 1));
            output_height = MIN(dest.height, (uint32_t)(dest.region.bottom - dest.region.top  + 1));
        }
        // Swap output width/height for 90/270 rotation cases
        else if (90 == p_args->rotation || 270 == p_args->rotation)
        {
            output_height  = MIN(dest.height, (uint32_t)(dest.region.right  - dest.region.left + 1));
            output_width   = MIN(dest.width,  (uint32_t)(dest.region.bottom - dest.region.top  + 1));
        }
        // Unsupported rotation cases
        else
        {
           goto fail;
        }
    }

    if (dest.output_format == YCRCBLP_H2V2 || dest.output_format == YCBCRLP_H2V2 ||
        dest.output_format == YCRCBLP_H2V1 || dest.output_format == YCBCRLP_H2V1)
    {
        jpeg_buffer_init(&p_output_buffers.data.yuv.luma_buf);
        jpeg_buffer_init(&p_output_buffers.data.yuv.chroma_buf);
    }
    else
    {
        jpeg_buffer_init(&p_output_buffers.data.rgb.rgb_buf);

        // Init the final image buffer only when tiling is enabled
        if (p_args->tiling_enabled)
        {
            jpeg_buffer_init(&p_final_image_buf.data.rgb.rgb_buf);
        }
    }

    use_pmem = OUTPUT_IS_SCREEN(output_filename);

    if (dest.stride == 0)
    {
        switch (dest.output_format)
        {
        case YCRCBLP_H2V2:
        case YCBCRLP_H2V2:
        case YCRCBLP_H2V1:
        case YCBCRLP_H2V1:
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
            return OS_THREAD_FUNC_RET_FAILED;
        }
    }
    else
    {
        stride = dest.stride;
    }

    // assign the stride for the decoder_output_handler call back
    p_thread_arg->p_args->stride = stride;

    if (p_args->tiling_enabled)
    {
        p_output_buffers.tile_width  = stride;
        p_output_buffers.tile_height = TILE_HEIGHT;
    }
    else
    {
        // Assign 0 to tile width and height
        // to indicate that no tiling is requested.
        p_output_buffers.tile_width  = 0;
        p_output_buffers.tile_height = 0;
    }

    switch (dest.output_format)
    {
    case YCRCBLP_H2V2:
    case YCBCRLP_H2V2:
        jpeg_buffer_allocate(p_output_buffers.data.yuv.luma_buf, stride * dest.height * 3 / 2, use_pmem);
        jpeg_buffer_attach_existing(p_output_buffers.data.yuv.chroma_buf,
                                    p_output_buffers.data.yuv.luma_buf, stride * output_height);
        break;
    case YCRCBLP_H2V1:
    case YCBCRLP_H2V1:
        jpeg_buffer_allocate(p_output_buffers.data.yuv.luma_buf, stride * dest.height * 2, use_pmem);
        jpeg_buffer_attach_existing(p_output_buffers.data.yuv.chroma_buf,
                                    p_output_buffers.data.yuv.luma_buf, stride * output_height);
        break;
    case RGB565:
    case RGB888:
    case RGBa:
        if (p_args->tiling_enabled)
        {
            // in case tiling is enabled, allocated the output buffer as the tiling buffer(holds 16 rgb line)
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
    memset(&config, 0, sizeof(mpod_cfg_t));
    config.preference  = p_args->preference;
    config.decode_from = JPEGD_DECODE_FROM_AUTO;
    config.rotation    = p_args->rotation;

    // Loop to perform n back-to-back decoding (to the same output file)
    for (i = 0; i < p_args->back_to_back_count; i++)
    {
        if (p_args->back_to_back_count > 1)
        {
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
        if (os_timer_start(&os_timer) < 0)
        {
            fprintf(stderr, "decoder_test: failed to get start time\n");
        }
        rc = mpod_start(mpo_decoder,
                        &config,
                        &dest,
                        &p_output_buffers,
                        output_buffers_count);

        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "decoder_test: jpegd_start failed\n");
            goto fail;
        }
        fprintf(stderr, "decoder_test: jpegd_start succeeded\n");

        // Do abort
        if (p_args->abort_time)
        {
            os_mutex_lock(&p_thread_arg->mutex);
            while (p_thread_arg->decoding)
            {
                rc = os_cond_timedwait(&p_thread_arg->cond, &p_thread_arg->mutex, p_args->abort_time);
                if (rc == JPEGERR_ETIMEDOUT)
                {
                    // Do abort
                    os_mutex_unlock(&p_thread_arg->mutex);
                    rc = mpod_abort(mpo_decoder);
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
        }
        else
        {
            // Wait until decoding is done or stopped due to error
            os_mutex_lock(&p_thread_arg->mutex);
            while (p_thread_arg->decoding)
            {
                os_cond_wait(&p_thread_arg->cond, &p_thread_arg->mutex);
            }
            os_mutex_unlock(&p_thread_arg->mutex);
        }

        {
            int diff;
            // Display the time elapsed
            if (os_timer_get_elapsed(&os_timer, &diff, 0) < 0)
            {
                fprintf(stderr, "decoder_test: failed to get elapsed time\n");
            }
            else
            {
                if (p_args->abort_time)
                {
                    if (p_thread_arg->decoding)
                    {
                        fprintf(stderr, "decoder_test: decoding aborted successfully after %d ms\n", diff);
                    }
                    else
                    {
                        fprintf(stderr, "decoder_test: decoding stopped before abort is issued. "
                                        "decode time: %d ms\n", diff);
                    }
                }
                else
                {
                    if (p_thread_arg->decode_success)
                    {
                        fprintf(stderr, "decoder_test: decode time: %d ms\n", diff);
                    }
                    else
                    {
                        fprintf(stderr, "decoder_test: decode failed\n");
                    }
                }
                // Dump output under either of the conditions:
                // 1. Output is to the screen AND it is the first thread
                // 2. Output is to files
                if ((!OUTPUT_IS_SCREEN(output_filename) || p_thread_arg->tid == 0) && !p_args->output_nowrite)
                {
                    if (p_args->tiling_enabled)
                    {
                        decoder_dump_output(output_filename, &dest, &p_final_image_buf);
                    }
                    else
                    {
                        decoder_dump_output(output_filename, &dest, &p_output_buffers);
                    }
                }
            }
        }

    }

    // Clean the output buffers each time a JPEG image has been decoded.
    switch (dest.output_format)
    {
    case YCRCBLP_H2V2:
    case YCBCRLP_H2V2:
    case YCRCBLP_H2V1:
    case YCBCRLP_H2V1:
        jpeg_buffer_destroy(&p_output_buffers.data.yuv.luma_buf);
        jpeg_buffer_destroy(&p_output_buffers.data.yuv.chroma_buf);
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


    /**************************************************************************
    *
    *  Decoding rest individual images in MPO...
    *
    **************************************************************************/
    for (image_index = 1; image_index < num_images; image_index++)
    {
        // Derive the output file name by appending the name with a incremental number
        append_filename(p_args->output_file, &output_filename, image_index + 1);

        // Move the starting pointer in the input buffer to the beginning of the new image
        rc = mpod_set_input_offset(mpo_decoder, mp_entry[image_index].data_offset);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "mpo_decoder_test: mpod_reset_input_offset failed\n");
            goto fail;
        }

        rc = mpod_read_header(mpo_decoder, &header);
        if (JPEG_FAILED(rc))
        {
            fprintf(stderr, "mpo_decoder_test: mpo_read_header failed. rc =%d \n", rc);
            goto fail;
        }

        // Print out dimension and subsampling information
        fprintf(stderr, "main dimension: (%dx%d) subsampling: (%d)\n",
                header.main.width, header.main.height, (int)header.main.subsampling);

        // Dump exif tag information if requested
        if (p_args->exif_dump_file && p_thread_arg->tid == 0)
        {
            FILE *exif_dump;
            char *exif_dump_name = p_args->exif_dump_file;

            // Derive the output file name by appending the name with a incremental number
            append_filename(p_args->exif_dump_file, &exif_dump_name, image_index + 1);


            exif_dump = fopen(exif_dump_name, "w");
            if(!exif_dump)
            {
                fprintf(stderr, "decoder_test: failed to open file for dumping exif tags: %s\n",
                        p_args->exif_dump_file);
            }
            else
            {
                decoder_test_dump_exif(exif_dump, header.exif_info);
                fclose(exif_dump);
            }

            free(exif_dump_name);
        }

        //Dump MPO tag information if requested
        if (p_args->mpo_dump_file && p_thread_arg->tid == 0)
        {
            FILE *mpo_dump;
            char *mpo_dump_name = p_args->mpo_dump_file;

            // Derive the output file name by appending the name with a incremental number
            append_filename(p_args->mpo_dump_file, &mpo_dump_name, image_index + 1);

            mpo_dump = fopen(mpo_dump_name, "w");
            if (!mpo_dump)
            {
                fprintf(stderr, "mpd_test: failed to open file for dumping mpo tags: %s\n",
                        p_args->mpo_dump_file);
            }
            else
            {
                fprintf(mpo_dump, "********Attribute IFD from the %dth individual image:*******:\n", image_index + 1);
                mpod_test_dump_attribute(mpo_dump, header.mpo_attribute_obj);
                fclose(mpo_dump);
            }

            free (mpo_dump_name);
        }

        // if thumbnail file is specified, decode thumbnail first.
        if (1 == p_args->thumbnail_flag)
        {
            // Temporary disable tiling when decoding thumbnail
            tiling_flag = p_args->tiling_enabled;
            p_args->tiling_enabled = 0;

            // check if there is an thumbnail
            if (header.thumbnail.height != 0 && header.thumbnail.width != 0)
            {
                // thumbnail output configuration
                dest_thumb.width         = header.thumbnail.width;
                dest_thumb.height        = header.thumbnail.height;
                dest_thumb.output_format = RGB888;
                dest_thumb.stride        = 0;

                dest_thumb.region.left   = 0;
                dest_thumb.region.top    = 0;
                dest_thumb.region.right  = 0;
                dest_thumb.region.bottom = 0;

                jpeg_buffer_init(&p_output_buffers_thumb.data.rgb.rgb_buf);

                // Assign 0 to tile width and height
                // to indicate that no tiling is requested for thumbnail.
                p_output_buffers_thumb.tile_width  = 0;
                p_output_buffers_thumb.tile_height = 0;

                stride = dest_thumb.width * 3;

                // output buffer allocation
                use_pmem = OUTPUT_IS_SCREEN(p_args->thumbnail_file);
                jpeg_buffer_allocate(p_output_buffers_thumb.data.rgb.rgb_buf, stride * dest_thumb.height, use_pmem);

                // config decoder to decode thumbnail.
                memset(&config, 0, sizeof(mpod_cfg_t));
                config.preference = p_args->preference;
                config.decode_from = JPEGD_DECODE_FROM_THUMB;
                config.rotation = 0;

                // start decoding
                p_thread_arg->decoding = true;
                if (os_timer_start(&os_timer) < 0)
                {
                    fprintf(stderr, "decoder_test thumbnail: failed to get start time\n");
                }
                rc = mpod_start(mpo_decoder,
                                &config,
                                &dest_thumb,
                                &p_output_buffers_thumb,
                                1);

                if (JPEG_FAILED(rc))
                {
                    fprintf(stderr, "decoder_test thumnail: jpegd_start failed\n");
                    goto fail;
                }
                fprintf(stderr, "decoder_test thumbnail: jpegd_start succeeded\n");

                // Wait until decoding is done or stopped due to error
                os_mutex_lock(&p_thread_arg->mutex);
                while (p_thread_arg->decoding)
                {
                    os_cond_wait(&p_thread_arg->cond, &p_thread_arg->mutex);
                }
                os_mutex_unlock(&p_thread_arg->mutex);

                // check thumbnail decoding results
                {
                    int diff;
                    if (os_timer_get_elapsed(&os_timer, &diff, 0) < 0)
                    {
                        fprintf(stderr, "decoder_test: failed to get elapsed time\n");
                    }
                    else
                    {
                        if (p_thread_arg->decode_success)
                        {
                            fprintf(stderr, "decoder_test: decode time: %d ms\n", diff);
                        }
                        else
                        {
                            fprintf(stderr, "decoder_test: decode failed\n");
                        }

                        // Dump output under either of the conditions:
                        // 1. Output is to the screen AND it is the first thread
                        // 2. Output is to files
                        if (!OUTPUT_IS_SCREEN(p_args->thumbnail_file) || p_thread_arg->tid == 0)
                        {
                            char *output_tn_name = p_args->thumbnail_file;

                            append_filename(p_args->thumbnail_file, &output_tn_name, image_index + 1);

                            decoder_dump_output(output_tn_name, &dest_thumb, &p_output_buffers_thumb);

                            free(output_tn_name);
                        }
                    }
                }
                // clean up buffers
                jpeg_buffer_destroy(&p_output_buffers_thumb.data.rgb.rgb_buf);

                if (!p_thread_arg->decode_success)
                {
                    fprintf(stderr, "decoder_test: thumbnail decode failed\n");
                    goto fail;
                }
            }
            else
            {
                fprintf(stderr, "decoder_test: Jpeg file does not contain thumbnail \n");
            }

            // Restore the tiling flag after thumbnail has been decoded
            p_args->tiling_enabled = tiling_flag;
        }

        // main image decoding:
        // Set destination information
        dest.width         = p_args->width  ? p_args->width  : header.main.width;
        dest.height        = p_args->height ? p_args->height : header.main.height;
        dest.output_format = p_args->format;
        dest.stride        = p_args->stride;
        dest.region        = p_args->region;

        // if region is defined, re-assign the output width/height
        output_width  = dest.width;
        output_height = dest.height;

        if (p_args->region.right || p_args->region.bottom)
        {
            if (0 == p_args->rotation || 180 == p_args->rotation)
            {
                output_width  = MIN(dest.width,  (uint32_t)(dest.region.right  - dest.region.left + 1));
                output_height = MIN(dest.height, (uint32_t)(dest.region.bottom - dest.region.top  + 1));
            }
            // Swap output width/height for 90/270 rotation cases
            else if (90 == p_args->rotation || 270 == p_args->rotation)
            {
                output_height  = MIN(dest.height, (uint32_t)(dest.region.right  - dest.region.left + 1));
                output_width   = MIN(dest.width,  (uint32_t)(dest.region.bottom - dest.region.top  + 1));
            }
            // Unsupported rotation cases
            else
            {
                goto fail;
            }
        }

        if (dest.output_format == YCRCBLP_H2V2 || dest.output_format == YCBCRLP_H2V2 ||
            dest.output_format == YCRCBLP_H2V1 || dest.output_format == YCBCRLP_H2V1)
        {
            jpeg_buffer_init(&p_output_buffers.data.yuv.luma_buf);
            jpeg_buffer_init(&p_output_buffers.data.yuv.chroma_buf);
        }
        else
        {
            jpeg_buffer_init(&p_output_buffers.data.rgb.rgb_buf);

            // Init the final image buffer only when tiling is enabled
            if (p_args->tiling_enabled)
            {
                jpeg_buffer_init(&p_final_image_buf.data.rgb.rgb_buf);
            }
        }

        use_pmem = OUTPUT_IS_SCREEN(output_filename);

        if (dest.stride == 0)
        {
            switch (dest.output_format)
            {
            case YCRCBLP_H2V2:
            case YCBCRLP_H2V2:
            case YCRCBLP_H2V1:
            case YCBCRLP_H2V1:
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
                return OS_THREAD_FUNC_RET_FAILED;
            }
        }
        else
        {
            stride = dest.stride;
        }

        // assign the stride for the decoder_output_handler call back
        p_thread_arg->p_args->stride = stride;

        if (p_args->tiling_enabled)
        {
            p_output_buffers.tile_width  = stride;
            p_output_buffers.tile_height = TILE_HEIGHT;
        }
        else
        {
            // Assign 0 to tile width and height
            // to indicate that no tiling is requested.
            p_output_buffers.tile_width  = 0;
            p_output_buffers.tile_height = 0;
        }

        switch (dest.output_format)
        {
        case YCRCBLP_H2V2:
        case YCBCRLP_H2V2:
            jpeg_buffer_allocate(p_output_buffers.data.yuv.luma_buf, stride * dest.height * 3 / 2, use_pmem);
            jpeg_buffer_attach_existing(p_output_buffers.data.yuv.chroma_buf,
                                        p_output_buffers.data.yuv.luma_buf, stride * output_height);
            break;
        case YCRCBLP_H2V1:
        case YCBCRLP_H2V1:
            jpeg_buffer_allocate(p_output_buffers.data.yuv.luma_buf, stride * dest.height * 2, use_pmem);
            jpeg_buffer_attach_existing(p_output_buffers.data.yuv.chroma_buf,
                                        p_output_buffers.data.yuv.luma_buf, stride * output_height);
            break;
        case RGB565:
        case RGB888:
        case RGBa:
            if (p_args->tiling_enabled)
            {
                // in case tiling is enabled, allocated the output buffer as the tiling buffer(holds 16 rgb line)
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
        memset(&config, 0, sizeof(mpod_cfg_t));
        config.preference  = p_args->preference;
        config.decode_from = JPEGD_DECODE_FROM_AUTO;
        config.rotation    = p_args->rotation;

        // Loop to perform n back-to-back decoding (to the same output file)
        for (i = 0; i < p_args->back_to_back_count; i++)
        {
            if (p_args->back_to_back_count > 1)
            {
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
            if (os_timer_start(&os_timer) < 0)
            {
                fprintf(stderr, "decoder_test: failed to get start time\n");
            }
            rc = mpod_start(mpo_decoder,
                            &config,
                            &dest,
                            &p_output_buffers,
                            output_buffers_count);

            if (JPEG_FAILED(rc))
            {
                fprintf(stderr, "decoder_test: jpegd_start failed\n");
                goto fail;
            }
            fprintf(stderr, "decoder_test: jpegd_start succeeded\n");

            // Do abort
            if (p_args->abort_time)
            {
                os_mutex_lock(&p_thread_arg->mutex);
                while (p_thread_arg->decoding)
                {
                    rc = os_cond_timedwait(&p_thread_arg->cond, &p_thread_arg->mutex, p_args->abort_time);
                    if (rc == JPEGERR_ETIMEDOUT)
                    {
                        // Do abort
                        os_mutex_unlock(&p_thread_arg->mutex);
                        rc = mpod_abort(mpo_decoder);
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
            }
            else
            {
                // Wait until decoding is done or stopped due to error
                os_mutex_lock(&p_thread_arg->mutex);
                while (p_thread_arg->decoding)
                {
                    os_cond_wait(&p_thread_arg->cond, &p_thread_arg->mutex);
                }
                os_mutex_unlock(&p_thread_arg->mutex);
            }

            {
                int diff;
                // Display the time elapsed
                if (os_timer_get_elapsed(&os_timer, &diff, 0) < 0)
                {
                    fprintf(stderr, "decoder_test: failed to get elapsed time\n");
                }
                else
                {
                    if (p_args->abort_time)
                    {
                        if (p_thread_arg->decoding)
                        {
                            fprintf(stderr, "decoder_test: decoding aborted successfully after %d ms\n", diff);
                        }
                        else
                        {
                            fprintf(stderr, "decoder_test: decoding stopped before abort is issued. "
                                            "decode time: %d ms\n", diff);
                        }
                    }
                    else
                    {
                        if (p_thread_arg->decode_success)
                        {
                            fprintf(stderr, "decoder_test: decode time: %d ms\n", diff);
                        }
                        else
                        {
                            fprintf(stderr, "decoder_test: decode failed\n");
                        }
                    }
                    // Dump output under either of the conditions:
                    // 1. Output is to the screen AND it is the first thread
                    // 2. Output is to files
                    if ((!OUTPUT_IS_SCREEN(output_filename) || p_thread_arg->tid == 0) && !p_args->output_nowrite)
                    {
                        if (p_args->tiling_enabled)
                        {
                            decoder_dump_output(output_filename, &dest, &p_final_image_buf);
                        }
                        else
                        {
                            decoder_dump_output(output_filename, &dest, &p_output_buffers);
                        }
                    }
                }
            }
        }

        // Clean the output buffers each time a JPEG image has been decoded.
        switch (dest.output_format)
        {
        case YCRCBLP_H2V2:
        case YCBCRLP_H2V2:
        case YCRCBLP_H2V1:
        case YCBCRLP_H2V1:
            jpeg_buffer_destroy(&p_output_buffers.data.yuv.luma_buf);
            jpeg_buffer_destroy(&p_output_buffers.data.yuv.chroma_buf);
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


        free(output_filename);

    } // end for loop for processing non-first individual images.

    /**************************************************************************
    *
    *  Done with decoding all individual images in MPO, start cleaning up...
    *
    **************************************************************************/
    // Clean up decoder and allocate buffers
    jpeg_buffer_destroy(&p_thread_arg->p_entire_input_buffer);
    jpeg_buffer_destroy(&source.buffers[0]);
    jpeg_buffer_destroy(&source.buffers[1]);

    mpod_destroy(&mpo_decoder);

    // Do bitwise comparison if requested (Output is not to the screen)
    if (p_args->reference_file && !OUTPUT_IS_SCREEN(p_args->output_file))
    {
        char *buf1 = malloc(CMP_SIZE);
        char *buf2 = malloc(CMP_SIZE);
        FILE *fref;
        FILE *fout;
        int bytes_read1, bytes_read2;

        fprintf(stderr, "decoder_test: comparing %s with %s...\n",
                p_args->output_file, p_args->reference_file);
        fref = fopen(p_args->reference_file, "rb");
        if (!fref)
        {
            fprintf(stderr, "decoder_test: error opening reference file: %s\n", p_args->reference_file);
            goto fail;
        }
        if (!buf1 || !buf2)
        {
            fprintf(stderr, "decoder_test: failed to malloc for buffer for file comparison\n");
            goto fail;
        }
        fout = fopen(p_args->output_file, "rb");
        for (;;)
        {
            bytes_read1 = (int)fread(buf1, 1, CMP_SIZE, fref);
            bytes_read2 = (int)fread(buf2, 1, CMP_SIZE, fout);
            if (bytes_read1 != bytes_read2 || memcmp(buf1, buf2, bytes_read1))
            {
                fprintf(stderr, "decoder_test: the two files differ.\n");
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
                goto fail;
            }
        }
    }
    if (!p_thread_arg->decode_success)
        goto fail;

    return OS_THREAD_FUNC_RET_SUCCEEDED;
fail:
    free(output_filename);
    return OS_THREAD_FUNC_RET_FAILED;
}

void decoder_event_handler(void         *p_user_data,
                           jpeg_event_t  event,
                           void         *p_arg)
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
    if (event == JPEG_EVENT_ERROR && p_arg)
    {
        fprintf(stderr, "decoder_event_handler: ERROR: %s\n", (char *)p_arg);
    }
}

// consumes the output buffer.
int decoder_output_handler(void               *p_user_data,
                           mpod_output_buf_t  *p_output_buffer,
                           uint32_t            first_row_id,
                           uint8_t             is_last_buffer)
{
    uint8_t *whole_output_buf_ptr;
    uint8_t *tiling_buf_ptr;
    uint32_t tiling_buf_actual_size;

    thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)p_user_data;

    jpeg_buffer_get_addr(p_thread_arg->p_whole_output_buf->data.rgb.rgb_buf, &whole_output_buf_ptr);
    jpeg_buffer_get_addr(p_output_buffer->data.rgb.rgb_buf, &tiling_buf_ptr);
    jpeg_buffer_get_actual_size(p_output_buffer->data.rgb.rgb_buf, &tiling_buf_actual_size);

    // copy data from the tiling buffer to the final output buffer
    if (!(p_thread_arg->p_args->output_nowrite))
    {
        memcpy(whole_output_buf_ptr + first_row_id * p_thread_arg->p_args->stride,
               tiling_buf_ptr, tiling_buf_actual_size);
    }

    if (p_output_buffer->tile_height != TILE_HEIGHT)
        return JPEGERR_EUNSUPPORTED;

    // testing purpose only
    // This is to simulate that the user needs to bail out when error happens
    // in the middle of decoding
    //if (first_row_id == 162)
    //   return JPEGERR_EFAILED;

    // do not enqueue any buffer if it reaches the last buffer
    if (!is_last_buffer)
    {
        mpod_enqueue_output_buf(p_thread_arg->mpo_decoder, p_output_buffer, 1);
    }

    return JPEGERR_SUCCESS;

}

/******************************************************************************
* This function assumes the entire Jpeg file has been pre-fetched into a big
* buffer. It then copies Jpeg data from the big buffer to one of the local
* ping-pong fashioned buffer when requested.
******************************************************************************/
uint32_t decoder_input_req_handler(void           *p_user_data,
                                   jpeg_buffer_t   buffer,
                                   uint32_t        start_offset,
                                   uint32_t        length)
{
    uint32_t entire_buf_size, buf_size;
    uint8_t *entire_buf_ptr, *buf_ptr;
    uint32_t bytes_to_read;
    thread_ctrl_blk_t *p_thread_arg = (thread_ctrl_blk_t *)p_user_data;

    jpeg_buffer_get_max_size(p_thread_arg->p_entire_input_buffer, &entire_buf_size);
    jpeg_buffer_get_addr(p_thread_arg->p_entire_input_buffer, &entire_buf_ptr);

    jpeg_buffer_get_max_size(buffer, &buf_size);
    jpeg_buffer_get_addr(buffer, &buf_ptr);

    bytes_to_read = MIN(length, buf_size);

    if(entire_buf_size < start_offset)
    {
        fprintf(stderr, "decoder_input_req_handler: - start offset is too high, start_offset = 0x%x & entire_buf_size = 0x%x\n",start_offset,entire_buf_size);
        return 0;
    }

    bytes_to_read = MIN(bytes_to_read, (entire_buf_size - start_offset));

    if (bytes_to_read > 0)
    {
        memcpy(buf_ptr, entire_buf_ptr + start_offset, bytes_to_read);
        return bytes_to_read;
    }
    else
    {
        // The caller needs this returned 0 to know that the entire Jpeg data
        // has been consumed.
        return 0;
    }
}

void decoder_dump_output(char *output_file, mpod_dst_t *p_dst, mpod_output_buf_t *p_output_buffer)
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
        FILE *fout;
        fprintf(stderr, "decoder_dump_output: dumping decoded image to file %s\n", output_file);

        fout = fopen(output_file, "wb");
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
            fwrite(buf_ptr, 1, stride * p_dst->height, fout);
            jpeg_buffer_get_addr(p_output_buffer->data.yuv.chroma_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height / 2, fout);
            break;
        case YCRCBLP_H2V1:
        case YCBCRLP_H2V1:
            jpeg_buffer_get_addr(p_output_buffer->data.yuv.luma_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height, fout);
            jpeg_buffer_get_addr(p_output_buffer->data.yuv.chroma_buf, &buf_ptr);
            fwrite(buf_ptr, 1, stride * p_dst->height, fout);
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


int mpod_test_dump_index(FILE* dump_file, mpo_index_obj_t mpo_index)
{
    exif_tag_id_t *p_tag_ids = NULL;
    uint32_t num_tag, i;

    // First get the number of tags present
    if (mpo_list_index_tagid(mpo_index, p_tag_ids, 0, &num_tag))
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
    if (mpo_list_index_tagid(mpo_index, p_tag_ids, num_tag, NULL))
    {
        fprintf(stderr, "decoder_test_dump_mpo: mpo_list_tagid failed\n");
        free(p_tag_ids);
        return -1;
    }

    for (i = 0; i < num_tag; i++)
    {
        uint32_t j;
        exif_tag_entry_t entry;
        if (mpo_get_index_tag(mpo_index, p_tag_ids[i], &entry))
        {
            fprintf(dump_file, "decoder_test_dump_mpo: failed to retrieve tag 0x%x\n", p_tag_ids[i] >> 16);
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


int mpod_test_dump_attribute(FILE* dump_file, mpo_attribute_obj_t mpo_attribute)
{
    exif_tag_id_t *p_tag_ids = NULL;
    uint32_t num_tag, i;

    // First get the number of tags present
    if (mpo_list_attribute_tagid(mpo_attribute, p_tag_ids, 0, &num_tag))
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
    if (mpo_list_attribute_tagid(mpo_attribute, p_tag_ids, num_tag, NULL))
    {
        fprintf(stderr, "decoder_test_dump_mpo: mpo_list_tagid failed\n");
        free(p_tag_ids);
        return -1;
    }

    for (i = 0; i < num_tag; i++)
    {
        uint32_t j;
        exif_tag_entry_t entry;
        if (mpo_get_attribute_tag(mpo_attribute, p_tag_ids[i], &entry))
        {
            fprintf(dump_file, "decoder_test_dump_mpo: failed to retrieve tag 0x%x\n", p_tag_ids[i] >> 16);
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

// Append the output file name with a number to avoid multiple writing to the same file
void append_filename(char *original_name, char **appended_name, int append_id)
{
    // Look for the last occurence of '/' and then last occurence of '.'
    char *s = strrchr(original_name, '/');
    if (s)
    {
        s = strrchr(s, '.');
    }
    else
    {
        s = strrchr(original_name, '.');
    }

    // Allocate space for appended file name, reserve 5 bytes for additional appending id
    *appended_name = (char *)malloc(5 + strlen(original_name));
    if (!(*appended_name)) {
        return;
    }
    // Copy original name to appended name
    snprintf(*appended_name, 5 + strlen(original_name), "%s", original_name);
    if (s)
    {
        // If the original file_name contains an file extension, then insert
        // the append_id between the main file name and the file extension
        // eg: test.yuv ==> test_02.yuv
        snprintf(*appended_name + (s - original_name), 5 + strlen(original_name) - (s - original_name), "_%.2d%s", append_id, s);
    }
    else
    {
        // eg: test ==> test_02
        snprintf(*appended_name, 5 + strlen(original_name) - (s - original_name), "%s_%.2d", *appended_name, append_id);
    }
}
