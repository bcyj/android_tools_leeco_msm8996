/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "jpege_engine_hw_10.h"
#include "jpege.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_buffer_private.h"
#include "jpeg_common_private.h"

#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#include <jpege_app_util_mmap.h>
#include <jpege_app_calc_param.h>

#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define MCU_SIZE 4
#define PAD_TO_WORD(a)  (((a)+3)&~3)
#define CEIL_PAD_TO_16(a)  (((a)+15)&~15)
#define FLOOR_PAD_TO_16(a)  ((a>>4)<<4)
#define MIN(x,y) (((x)<(y)) ? (x) : (y))

#ifdef SMIPOOL_AVAILABLE
static int smipool_available = true;
#else
static int smipool_available = false;
#endif

#define MAX_NUM_OUTPUT_BUF 10
#define MAX_JPEGE_WIDTH 8192
#define MAX_JPEGE_HEIGHT 8192

#define MAX_JPEGE_OUT_SIZE  FLOOR_PAD_TO_16(0x4000000)

jpege_engine_hw_t  *p_engine_local;
static uint32_t max_jpeg_op_size;
static uint32_t actual_op_size;

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* Static constants */
/*  default Luma Qtable */
const uint8_t DEFAULT_QTABLE_0[64] = {
  16, 11, 10, 16, 24, 40, 51, 61,
  12, 12, 14, 19, 26, 58, 60, 55,
  14, 13, 16, 24, 40, 57, 69, 56,
  14, 17, 22, 29, 51, 87, 80, 62,
  18, 22, 37, 56, 68, 109, 103, 77,
  24, 35, 55, 64, 81, 104, 113, 92,
  49, 64, 78, 87, 103, 121, 120, 101,
  72, 92, 95, 98, 112, 100, 103, 99
};

/*  default Chroma Qtable */
const uint8_t DEFAULT_QTABLE_1[64] = {
  17, 18, 24, 47, 99, 99, 99, 99,
  18, 21, 26, 66, 99, 99, 99, 99,
  24, 26, 56, 99, 99, 99, 99, 99,
  47, 66, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99
};

/*  default Luma Qtable */
const uint8_t DEFAULT_QTABLE_0_1[64] = {
  5, 3, 4, 4, 4, 3, 5, 4,
  4, 4, 5, 5, 5, 6, 7, 12,
  8, 7, 7, 7, 7, 15, 11, 11,
  9, 12, 17, 15, 18, 18, 17, 15,
  17, 17, 19, 22, 28, 23, 19, 20,
  26, 21, 17, 17, 24, 33, 24, 26,
  29, 29, 31, 31, 31, 19, 23, 34,
  36, 34, 30, 36, 28, 30, 31, 30,
};

/*  default Chroma Qtable */
const uint8_t DEFAULT_QTABLE_1_1[64] = {
  5, 5, 5, 7, 6, 7, 14, 8,
  8, 14, 30, 20, 17, 20, 30, 30,
  30, 30, 30, 30, 30, 30, 30, 30,
  30, 30, 30, 30, 30, 30, 30, 30,
  30, 30, 30, 30, 30, 30, 30, 30,
  30, 30, 30, 30, 30, 30, 30, 30,
  30, 30, 30, 30, 30, 30, 30, 30,
  30, 30, 30, 30, 30, 30, 30, 30,
};

const uint32_t nZigzagTable[64] = {
  0, 1, 8, 16, 9, 2, 3, 10,
  17, 24, 32, 25, 18, 11, 4, 5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13, 6, 7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

/*  default Huffman Tables */
uint8_t DEFAULT_HTABLE_DC_BITS_0[16] = {
  0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t DEFAULT_HTABLE_DC_HUFFVAL_0[12] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b
};

uint8_t DEFAULT_HTABLE_AC_BITS_0[16] = {
  0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
  0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d
};

uint8_t DEFAULT_HTABLE_AC_HUFFVAL_0[162] = {
  0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
  0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
  0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
  0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
  0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
  0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
  0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
  0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
  0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
  0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
  0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
  0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
  0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
  0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
  0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
  0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
  0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
  0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
  0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
  0xf9, 0xfa
};

uint32_t jpege_num_pixels[] = {
  3145728,  /*3Mp   2048*1536*/
  5059584,  /*5Mp   2592x1952*/
  7990272,  /*8Mp   3264*2448*/
  9980928,  /*10Mp  3648*2736*/
  12000000, /*12Mp  4000*3000*/
  13996800, /*14Mp  4320*3240*/
  16822272, /*16Mp  4736*3552*/
};

/* File size requirement for quality values 50-100*/
#define JPEGE_MAX_FS_TABLE_SIZE 11
uint32_t jpege_max_fs_table[7][JPEGE_MAX_FS_TABLE_SIZE] = {
  {  4718592,  2996614,  2072161, 1641348, 1400955, 1232319,
     1118978, 1024444,  946373,  882238,  828973 }, /*3MP*/
  {  7589376,  4819749,  3332860, 2639941, 2253293, 1982060,
     1799762, 1647715, 1522145, 1418990, 1333319 }, /*5Mp*/
  { 11520000,  7611516,  5263369, 4169087, 3558479, 3130138,
     2842247, 2602128, 2403824, 2240919, 2105623 }, /*8Mp*/
  { 14971392,  9507811,  6574658, 5207752, 4445021, 3909964,
     3550350, 3250409, 3002701, 2799210, 2630207 }, /*10Mp*/
  { 18000000, 11431174,  7904666, 6261244, 5344217, 4700923,
     4268561, 3907944, 3610126, 3365470, 3162280 }, /*12Mp*/
  { 20995200, 13333322,  9220002, 7303115, 6233495, 5483156,
     4978849, 4558226, 4210851, 3925485, 3688483 }, /*14Mp*/
  { 25233408, 16024860, 11081203, 8777362, 7491823, 6590016,
     5983908, 5478375, 5060876, 4717905, 4433060 }, /*16Mp*/
};

/* output buf processing */
static struct jpege_hw_buf output_buf[MAX_NUM_OUTPUT_BUF];

/* input buf processing */
struct jpege_hw_buf input_buf[] = {
  /*
   * type; fd; vaddr; y_off; y_len; framedone_len;
   * cbcr_off; cbcr_len; NumOfMCURows;
   */
  {0, 0, (void *) 0x0, 0, 1920000, 0, 0x0, 960000, 75, 0},
};

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* Function prototypes of jpege_engine_obj_t interface functions */
static void jpege_engine_hw_create  (jpege_engine_obj_t *p_obj,
  jpege_hw_obj_t encoder);
static int  jpege_engine_hw_init    (jpege_engine_obj_t*,
  jpege_engine_event_handler_t);
static int  jpege_engine_hw_start   (jpege_engine_obj_t*, jpege_img_cfg_t*,
  jpege_img_data_t*,
  jpege_engine_dst_t*);
static int  jpege_engine_hw_abort   (jpege_engine_obj_t*);
static void jpege_engine_hw_destroy (jpege_engine_obj_t*);
static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
  jpege_engine_hw_encode  (OS_THREAD_FUNC_ARG_T);

static int  jpege_engine_hw_check_start_param (jpege_img_cfg_t*,
  jpege_img_data_t*,
  jpege_engine_dst_t*);
static jpeg_buf_t *jpege_engine_hw_locate_dst_buf(jpege_engine_hw_t *p_engine,
  void *ptr);

/* =======================================================================
**                      Global Object Definitions
** ======================================================================= */
static const char jpege_engine_hw_name[] = "Jpeg HARDWARE Encode Engine";
jpege_engine_profile_t jpege_engine_hw_profile = {jpege_engine_hw_create,
  jpege_engine_hw_name};

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
/******************************************************************************
 * Helper functions below
 *****************************************************************************/
/* event processing */
int jpege_engine_hw_event_handler (jpege_hw_obj_t gmn_obj,
  struct jpege_hw_evt *pGmnCtrlCmd,
  int event)
{
  //to do: once this event adds engine handler
  //total_frames++;

  //printf ("%s:%d] handler called: event %d total frames %d\n",
  // __func__, __LINE__, event, total_frames);
  // to do:  check gemini status?

  return 0;
}

int jpege_engine_allocate_output_buf (jpege_hw_obj_t gmn_obj,
  jpege_engine_hw_t *p_engine,
  int* p_index,
  uint32_t *p_allocated_size,
  uint32_t requested_size,
  int level)
{
  uint32_t buf_size = requested_size;
  int rc = 0;
  int i = *p_index;
  JPEG_DBG_MED("%s: buf_size %u index %d level %d", __func__, buf_size,
    *p_index, level);
  if (0 == buf_size)
    return JPEGERR_SUCCESS;

  if ((*p_index >= MAX_NUM_OUTPUT_BUF)
    || (2 == level)) {
    return JPEGERR_EFAILED;
  }
  output_buf[i].y_len = (buf_size > MAX_JPEGE_OUT_SIZE) ?
    MAX_JPEGE_OUT_SIZE : buf_size;
  output_buf[i].alloc_ion.len = output_buf[i].y_len;
  output_buf[i].alloc_ion.flags = ION_FLAG_CACHED;
  output_buf[i].alloc_ion.heap_mask = 0x1 << ION_IOMMU_HEAP_ID;
  output_buf[i].alloc_ion.align = 4096;
  output_buf[i].vaddr = do_mmap (output_buf[i].y_len, &output_buf[i].fd,
    JPEG_PMEM_ADSP, &output_buf[i].ion_fd_main, &output_buf[i].alloc_ion,
    &output_buf[i].fd_ion_map);
  if (!output_buf[i].vaddr) {
    /* do_mmap is half sucess. it was not complete. so we do not need to
       call do_munmap().
    do_munmap (output_buf[i].fd, output_buf[i].vaddr, output_buf[i].y_len,
              output_buf[i].ion_fd_main,
               &output_buf[i].fd_ion_map);
    */
    output_buf[i].fd = -1;
    output_buf[i].y_len = 0;
    JPEG_DBG_ERROR ("%s:%d] fail to allocate PMEM for jpege output\n",
      __func__, __LINE__);
    rc = jpege_engine_allocate_output_buf(gmn_obj, p_engine,
      p_index, p_allocated_size, requested_size/2, level+1);
    if (rc) {
      return JPEGERR_EFAILED;
    }
    rc = jpege_engine_allocate_output_buf(gmn_obj, p_engine,
      p_index, p_allocated_size, requested_size/2, level+1);
    if (rc) {
      return JPEGERR_EFAILED;
    }
    JPEG_DBG_MED("%s:%d] allocated size %d \n", __func__, __LINE__,
       *p_allocated_size);
    return JPEGERR_SUCCESS;
  }

  JPEG_DBG_MED("%s:%d] allocation suceeded y_len %d \n", __func__, __LINE__,
    output_buf[i].y_len);
  jpege_lib_output_buf_enq (gmn_obj, &output_buf[i]);
  buf_size = PAD_TO_4K(output_buf[i].y_len);
  p_engine->num_output_buffers++;
  *p_index = *p_index+1;
  *p_allocated_size += buf_size;
  JPEG_DBG_MED("%s:%d] allocated size %d index %d\n", __func__, __LINE__,
    *p_allocated_size, *p_index);
  return JPEGERR_SUCCESS;
}

/* output buf processing */
int jpege_engine_hw_output_buf_enq (jpege_hw_obj_t gmn_obj,
  jpege_engine_hw_t *p_engine)
{
  int i, rc = 0;
  int fs_table_index = 0;
  uint32_t num_pixels;
  int fs_table_size = sizeof(jpege_num_pixels)/sizeof(jpege_num_pixels[0]);
  uint32_t quality_table_size = JPEGE_MAX_FS_TABLE_SIZE;
  uint32_t quality_index = 10;
  uint32_t buf_size = 0;
  if (p_engine->quality >= 50) {
    quality_index = 10 - (p_engine->quality - 50 + 4) / 5;
  }
  if(p_engine->scale_cfg.enable)
      num_pixels = p_engine->scale_cfg.output_height *
                   p_engine->scale_cfg.output_width;
  else
      num_pixels = p_engine->luma_width * p_engine->luma_height;

  JPEG_DBG_MED(" quality %d ", p_engine->quality);
  JPEG_DBG_MED("%s: num_pixels =%d\n",__func__,num_pixels);

  for (fs_table_index = 0; fs_table_index < fs_table_size; fs_table_index++ ) {
    if (num_pixels <= jpege_num_pixels[fs_table_index]) {
      break;
    }
  }
  if (fs_table_index >= fs_table_size) {
    fs_table_index = fs_table_size-1;
  }

  if ((uint32_t)quality_index >= quality_table_size) {
    quality_index = quality_table_size-1;
  }

  reencode_type_t type =
    p_engine_local->reencode_info.type[p_engine_local->reencode_info.current];
  switch(type) {
    case REENCODE_TYPE_NEXT_MAXSIZE:
     if (quality_index > 0)
       quality_index--;
     break;
    case REENCODE_TYPE_MAXSIZE:
     quality_index = 0;
     break;
    default:;
  }
  JPEG_DBG_MED("%s:%d] type %d current %d actual_op_size %u\n", __func__,
    __LINE__, type, p_engine_local->reencode_info.current,
    actual_op_size);
  if ((REENCODE_TYPE_PREV_ESTIMATE == type)
    && (p_engine_local->reencode_info.current > 0)
    && (actual_op_size > 0)) {
    max_jpeg_op_size = actual_op_size;
  } else
    max_jpeg_op_size =
     CEIL_PAD_TO_16(jpege_max_fs_table[fs_table_index][quality_index]);
  buf_size = max_jpeg_op_size;
  JPEG_DBG_MED("%s:%d] quality_index %d fs_table_index %d size %u\n",
  __func__,__LINE__, quality_index, fs_table_index, max_jpeg_op_size);


  p_engine->num_output_buffers = 0;

  i = 0;
  uint32_t allocated_size = 0;
  uint32_t requested_size = buf_size;
  while(i < MAX_NUM_OUTPUT_BUF) {
    rc = jpege_engine_allocate_output_buf(gmn_obj, p_engine,
    &i, &allocated_size, requested_size, 0);
    if (rc) {
      return rc;
    }
    if (requested_size <= allocated_size)
      break;
    requested_size -= allocated_size;
  }
  if (i == MAX_NUM_OUTPUT_BUF) {
    return JPEGERR_EFAILED;
  }
  p_engine->total_size_received = 0;
  p_engine->output_buf_rcvd_cnt = 0;
  JPEG_DBG_MED("%s:%d] num_output_buffers %d\n", __func__, __LINE__,
    p_engine->num_output_buffers);

  return JPEGERR_SUCCESS;
}

int jpege_engine_hw_free_output_buf (void)
{
  unsigned int i;

  JPEG_DBG_MED("%s: num buffers %d", __func__,
   p_engine_local->num_output_buffers);
  for (i = 0; i < p_engine_local->num_output_buffers; i++)
  {
    JPEG_DBG_MED("%s: fd %d", __func__, output_buf[i].fd);
    if (output_buf[i].fd <= 0) {
      continue;
    }
    int rc = do_munmap (output_buf[i].fd, output_buf[i].vaddr,
      output_buf[i].y_len, output_buf[i].ion_fd_main,
                            &output_buf[i].fd_ion_map);
    if (rc) {
      JPEG_DBG_ERROR ("%s:%d] fail to free PMEM for jpeg output\n",
        __func__, __LINE__);
      return JPEGERR_EFAILED;
    }
    output_buf[i].y_len = 0;
    JPEG_DBG_MED ("%s: free PMEM for jpeg output %d\n", __func__,
      output_buf[i].y_len);
  }
  return JPEGERR_SUCCESS;
}

/* input buf processing */
int jpege_engine_hw_input_buf_enq (jpege_hw_obj_t gmn_obj,
  jpege_engine_hw_t *p_engine)
{
  unsigned int i;
  long n;
  int lumaHeight;
  int lumaWidth;
  int chromaHeight;
  int chromaWidth;

  if(p_engine->color_format == JPEGE_INPUT_H2V2){
      lumaHeight =  CEILING16(p_engine->luma_height);
      lumaWidth = CEILING16(p_engine->luma_width);
  }
  else{
      lumaHeight =  p_engine->luma_height;
      lumaWidth = p_engine->luma_width;
  }
  chromaHeight = p_engine->chroma_height;
  chromaWidth = p_engine->chroma_width;

  JPEG_DBG_MED("%s:%d jpeg input pmem %d offset %d \n", __func__, __LINE__,
    p_engine->input_pmem_flag, p_engine->p_input_luma->phy_offset);
  if (p_engine->input_pmem_flag) {
    input_buf[0].y_len = lumaWidth * lumaHeight;
    if (p_engine->num_of_input_planes == 2) {
      input_buf[0].cbcr_len = chromaWidth * chromaHeight;
      input_buf[0].cr_len = 0;
    }
    else if (p_engine->num_of_input_planes == 3) {
      input_buf[0].cbcr_len = floor(chromaWidth * chromaHeight)/2;
      input_buf[0].cr_len = floor(chromaWidth * chromaHeight)/2;
    }
    else{
      input_buf[0].cbcr_len = 0;
      input_buf[0].cr_len = 0;
    }
    input_buf[0].cr_offset = 0;
    input_buf[0].vaddr = p_engine->p_input_luma->ptr;
    input_buf[0].fd = p_engine->p_input_luma->pmem_fd;
    input_buf[0].num_of_mcu_rows = lumaHeight >> MCU_SIZE;
    input_buf[0].offset = p_engine->p_input_luma->phy_offset;
    JPEG_DBG_ERROR ("%s: lumaHeight =%d lumaWidth=%d chromaWidth =%d ylen = %d,"
      "chromaHeight=%d cbcr_length=%d offset=%d y_off = %d cbcr_off = %d\n",
      __func__, lumaHeight, lumaWidth,chromaWidth, input_buf[0].y_len,
      chromaHeight, input_buf[0].cbcr_len, input_buf[0].offset,
      input_buf[0].y_off, input_buf[0].cbcr_off);

    jpege_lib_input_buf_enq (gmn_obj, &input_buf[0]);
  }
  else {
    input_buf[0].y_len = lumaWidth * lumaHeight;
    if (p_engine->num_of_input_planes == 2) {
      input_buf[0].cbcr_len = chromaWidth * chromaHeight;
      input_buf[0].cr_len = 0;
    }
    else if (p_engine->num_of_input_planes == 3) {
      input_buf[0].cbcr_len = floor(chromaWidth * chromaHeight)/2;
      input_buf[0].cr_len = floor(chromaWidth * chromaHeight)/2;
    }
    else{
      input_buf[0].cbcr_len = 0;
      input_buf[0].cr_len = 0;
    }
    input_buf[0].cr_offset = 0;
    input_buf[0].num_of_mcu_rows = lumaHeight >> MCU_SIZE;
    input_buf[0].offset = 0;

    JPEG_DBG_ERROR ("%s: lumaHeight =%d lumaWidth=%d chromaWidth =%d"
      "ylen = %d,chromaHeight=%d cbcr_length=%d offset=%d y_off = %d"
      "cbcr_off = %d, cr_len =%d\n", __func__, lumaHeight, lumaWidth,
      chromaWidth, input_buf[0].y_len, chromaHeight,
      input_buf[0].cbcr_len, input_buf[0].offset,
      input_buf[0].y_off, input_buf[0].cbcr_off, input_buf[0].cr_len);

    for (i = 0; i < sizeof (input_buf) / sizeof (input_buf[0]); i++) {
      input_buf[i].alloc_ion.len = input_buf[i].y_len +
        input_buf[i].cbcr_len + input_buf[0].cr_len;
      input_buf[i].alloc_ion.flags = ION_FLAG_CACHED;
      input_buf[i].alloc_ion.heap_mask = 0x1 << ION_IOMMU_HEAP_ID;
      input_buf[i].alloc_ion.align = 4096;
      input_buf[i].vaddr =
        do_mmap (input_buf[i].y_len + input_buf[i].cbcr_len +
          input_buf[i].cr_len,
          &input_buf[i].fd,
          (smipool_available ? JPEG_PMEM_SMIPOOL : JPEG_PMEM_ADSP),
                 &input_buf[i].ion_fd_main, &input_buf[i].alloc_ion,
           &input_buf[i].fd_ion_map);
      if (!input_buf[i].vaddr) {
        JPEG_DBG_ERROR ("%s:%d fail to allocate PMEM for jpeg input\n",
          __func__, __LINE__);
        return JPEGERR_EFAILED;
      }

      (void)STD_MEMMOVE(input_buf[i].vaddr, p_engine->p_input_luma->ptr,
        input_buf[i].y_len);

      if (p_engine->num_of_input_planes == 2) {
        (void)STD_MEMMOVE((uint8_t *)input_buf[i].vaddr +
        input_buf[i].y_len, p_engine->p_input_chroma->ptr,
        input_buf[i].cbcr_len);
        }
      else if (p_engine->num_of_input_planes == 3) {
          (void)STD_MEMMOVE((uint8_t *)input_buf[i].vaddr +
          input_buf[i].y_len,p_engine->p_input_chroma->ptr,
          input_buf[i].cbcr_len);

          (void)STD_MEMMOVE((uint8_t *)input_buf[i].vaddr +
          input_buf[i].y_len + input_buf[i].cbcr_len,
          p_engine->p_input_cr->ptr, input_buf[i].cr_len);
      }

      jpege_lib_input_buf_enq (gmn_obj, &input_buf[i]);
    }
  }
  return JPEGERR_SUCCESS;
}

int jpege_engine_hw_free_input_buf (jpege_engine_hw_t *p_engine)
{
  unsigned int i;

  if (!p_engine->input_pmem_flag) {
    for (i = 0; i < sizeof (input_buf) / sizeof (input_buf[0]); i++) {
      if (input_buf[i].vaddr) {
        int rc = do_munmap (input_buf[i].fd, input_buf[i].vaddr,
          input_buf[i].y_len + input_buf[i].cbcr_len, input_buf[i].ion_fd_main,
           &input_buf[i].fd_ion_map);
        if (rc) {
          JPEG_DBG_ERROR ("%s:%d] fail to free PMEM for jpeg input\n",
          __func__, __LINE__);
          return JPEGERR_EFAILED;
        }
      }
    }
  }
  return JPEGERR_SUCCESS;
}

int jpege_engine_hw_input_handler (jpege_hw_obj_t gmn_obj,
  struct jpege_hw_buf *buf)
{
  //static int input_total_segments = 0;

  //printf ("%s:%d] %p total segments %d\n", __func__, __LINE__, buf, input_total_segments);

  //to do: piece by piece input? rotation not support!

  return 0;
}

static void jpege_engine_send_encoded_data (uint32_t bytes_remaining,
  struct jpege_hw_buf *buf)
{
  uint32_t bytes_written = 0, bytes_to_copy;
  long size = bytes_remaining;
  int rc;
  jpeg_buf_t *p_dest_buf = p_engine_local->p_dest_buffer;


  while (bytes_remaining)
  {
    // Get the next output buffer from buffer(s) queue if necessary
    if (!p_engine_local->p_dest_buffer) {
        rc = p_engine_local->dest.p_get_output_buffer_handler(
                p_engine_local->p_wrapper, &p_engine_local->p_dest_buffer);
        if (JPEG_FAILED(rc) || !p_engine_local->p_dest_buffer) {
            JPEG_DBG_ERROR("jpege_engine_hw_output_handler: failed to get next"
              "output buffer\n"); os_mutex_lock(&(p_engine_local->mutex));
            p_engine_local->error_flag = 1;
            os_mutex_unlock(&(p_engine_local->mutex));
            JPEG_DBG_LOW("jpege_engine_hw_output_handler: set error flag,"
            "return.\n");
            return;
        }
        p_dest_buf = p_engine_local->p_dest_buffer;
    }

    // Calculate how much can be written to the destination buffer
    bytes_to_copy = STD_MIN(bytes_remaining, (p_dest_buf->size
            - p_dest_buf->offset));

    // Copy the data from bitstream buffer to destination buffer starting from the offset
    STD_MEMMOVE(p_dest_buf->ptr + p_dest_buf->offset,
            (uint8_t *) buf->vaddr + bytes_written, bytes_to_copy);

    // Update destination buffer offset and other state variables
    p_dest_buf->offset += bytes_to_copy;
    bytes_remaining -= bytes_to_copy;
    bytes_written += bytes_to_copy;

    //TODO change add check  buf->type == GEMINI_EVT_FRAMEDONE &&
    p_engine_local->dest.p_output_handler(p_engine_local->p_wrapper,
            p_engine_local->p_dest_buffer, !(bytes_remaining));

    // Clean up the local output buffer ref
    p_engine_local->p_dest_buffer = NULL;
    p_dest_buf = NULL;
  }

  p_engine_local->total_size_received += size;
  p_engine_local->output_buf_rcvd_cnt++;
  JPEG_DBG_MED ("%s:%d] total_size_received %u output_buf_rcvd_cnt %u",
    __func__, __LINE__,
    p_engine_local->total_size_received,
    p_engine_local->output_buf_rcvd_cnt);
}

int jpege_engine_hw_output_handler (jpege_hw_obj_t gmn_obj,
  struct jpege_hw_buf *buf)
{
  uint32_t size;
  uint32_t i = 0;

  JPEG_DBG_MED ("%s:%d] buf->vaddr = %p, buf->y_len = %d, framedone_len %d\n",
    __func__, __LINE__, buf->vaddr, buf->y_len, buf->framedone_len);

  /* check if estimated size is enough */
  if ((buf->type == JPEG_EVT_SESSION_DONE) &&
       (buf->framedone_len > max_jpeg_op_size)) {
    JPEG_DBG_HIGH("%s:%d], Error Estimates size %u actual size %d",
      __func__, __LINE__, max_jpeg_op_size, buf->framedone_len);

    actual_op_size = buf->framedone_len;
    os_mutex_lock(&p_engine_local->frame_done_mutex);
    p_engine_local->output_done = true;
    p_engine_local->reencode_info.current++;
    JPEG_DBG_HIGH("%s:%d], current %u total %d",
      __func__, __LINE__,
      p_engine_local->reencode_info.current,
      p_engine_local->reencode_info.total_num_attempts);
    if (p_engine_local->reencode_info.current <
      p_engine_local->reencode_info.total_num_attempts) {
      p_engine_local->reencode_info.reencode = true;
    } else
      p_engine_local->reencode_info.reencode = false;
    p_engine_local->error_flag = true;
    os_cond_signal(&p_engine_local->frame_done_cond);
    os_mutex_unlock(&p_engine_local->frame_done_mutex);
    return 0;
  }

  if (buf->type == JPEG_EVT_SESSION_DONE) {
    size = buf->framedone_len - p_engine_local->total_size_received;
  } else
    size = buf->y_len;

  JPEG_DBG_MED ("%s:%d] size %d\n", __func__, __LINE__,
      size);

  if ((size <= buf->y_len) &&
    (p_engine_local->reencode_info.total_num_attempts <= 1)) {
    jpege_engine_send_encoded_data(size, buf);
  } else {
    /* This case happens when composite IRQ for WE and FD occurs*/
    uint32_t bytes_remaining;
    for (i = p_engine_local->output_buf_rcvd_cnt;
      (i < p_engine_local->num_output_buffers) &&
      (p_engine_local->total_size_received < buf->framedone_len); i++) {
      bytes_remaining = MIN(output_buf[i].y_len,
        (buf->framedone_len - p_engine_local->total_size_received));
      jpege_engine_send_encoded_data(bytes_remaining, &output_buf[i]);
    }
  }

  if (buf->type == JPEG_EVT_SESSION_DONE) {
    os_mutex_lock(&p_engine_local->frame_done_mutex);
    p_engine_local->output_done = true;
    p_engine_local->reencode_info.reencode = false;
    os_cond_signal(&p_engine_local->frame_done_cond);
    os_mutex_unlock(&p_engine_local->frame_done_mutex);
  }
  else {
    /* In current design we send all buffers to encoder at one shot
     * Hence we dont need to re enqueue the buffer again
     */
    /* jpege_lib_output_buf_enq (gmn_obj, buf); */
  }

  return 0;
}

void jpege_engine_hw_quant_table_helper (jpege_cmd_jpeg_encode_cfg
  *p_encode_cfg)
{
  uint32_t nIndex = 0;

  /*
   * Q Tables
   */
  for (nIndex = 0; nIndex < 64; nIndex++) {
    p_encode_cfg->quantTblY->qtbl[nIndex] =
      DEFAULT_QTABLE_0[nIndex];
    p_encode_cfg->quantTblChroma->qtbl[nIndex] =
      DEFAULT_QTABLE_1[nIndex];
  }
}

void jpege_engine_hw_quant_zigzag_table_helper (jpege_cmd_jpeg_encode_cfg
  *p_encode_cfg)
{
  uint32_t i, j;

  for (i = 0; i < 64; i++) {
    j = nZigzagTable[i];
    p_encode_cfg->quantTblY->qtbl[j] = DEFAULT_QTABLE_0[i];
    p_encode_cfg->quantTblChroma->qtbl[j] =
      DEFAULT_QTABLE_1[i];
  }
}

void jpege_engine_hw_huffman_table_helper (jpege_cmd_jpeg_encode_cfg
  *p_encode_cfg)
{
  uint32_t nIndex = 0;

  jpege_huffmanDcTable sHuffmanTblYDcPtr;
  jpege_huffmanAcTable sHuffmanTblYAcPtr;
  jpege_huffmanDcTable sHuffmanTblCbcrDcPtr;
  jpege_huffmanAcTable sHuffmanTblCbcrAcPtr;
  /*
   * Luma and Chroma DC bits
   */
  for (nIndex = 0; nIndex < 16; nIndex++) {
    sHuffmanTblYDcPtr.huffcount[nIndex] =
      DEFAULT_HTABLE_DC_BITS_0[nIndex];
    sHuffmanTblCbcrDcPtr.huffcount[nIndex] =
      DEFAULT_HTABLE_DC_BITS_0[nIndex];
  }

  /*
   * Luma and Chroma DC vals
   */
  for (nIndex = 0; nIndex < 12; nIndex++) {
    sHuffmanTblYDcPtr.huffval[nIndex] =
      DEFAULT_HTABLE_DC_HUFFVAL_0[nIndex];
    sHuffmanTblCbcrDcPtr.huffval[nIndex] =
      DEFAULT_HTABLE_DC_HUFFVAL_0[nIndex];
  }

  /*
   * Luma and Chroma AC bits
   */
  for (nIndex = 0; nIndex < 16; nIndex++) {
    sHuffmanTblYAcPtr.huffcount[nIndex] =
      DEFAULT_HTABLE_AC_BITS_0[nIndex];
    sHuffmanTblCbcrAcPtr.huffcount[nIndex] =
      DEFAULT_HTABLE_AC_BITS_0[nIndex];
  }

  /*
   * Luma and Chroma AC vals
   */
  for (nIndex = 0; nIndex < 162; nIndex++) {
    sHuffmanTblYAcPtr.huffval[nIndex] =
      DEFAULT_HTABLE_AC_HUFFVAL_0[nIndex];
    sHuffmanTblYAcPtr.huffval[nIndex] =
      DEFAULT_HTABLE_AC_HUFFVAL_0[nIndex];
  }
  return;
}

static void jpege_engine_hw_create(jpege_engine_obj_t *p_obj,
  jpege_hw_obj_t encoder)
{
  if (p_obj) {
    // Destroy previous engine if it exists
    if (p_obj->destroy) {
      p_obj->destroy(p_obj);
    }
    p_obj->create            = &jpege_engine_hw_create;
    p_obj->init              = &jpege_engine_hw_init;
    p_obj->check_start_param = &jpege_engine_hw_check_start_param;
    p_obj->start             = &jpege_engine_hw_start;
    p_obj->abort             = &jpege_engine_hw_abort;
    p_obj->destroy           = &jpege_engine_hw_destroy;
    p_obj->p_engine          = NULL;
    p_obj->encoder           = encoder;
    p_obj->is_initialized    = false;
  }
}

static int jpege_engine_hw_init(
  jpege_engine_obj_t           *p_obj,
  jpege_engine_event_handler_t  p_handler)
{
  int rc;
  jpege_engine_hw_t *p_engine;
  jpege_hw_obj_t gmn_obj = 0;
  int jpege_hw_fd = -1;

  // Validate input arguments
  if (!p_obj || !p_handler)
    return JPEGERR_ENULLPTR;

  // Allocate memory for the engine structure
  p_engine = (jpege_engine_hw_t *)JPEG_MALLOC(sizeof(jpege_engine_hw_t));
  if (!p_engine) {
    return JPEGERR_EMALLOC;
  }

  // Initialize the fields inside the engine structure below
  STD_MEMSET(p_engine, 0, sizeof(jpege_engine_hw_t));   // Zero out the entire structure
  p_engine->p_wrapper = p_obj;                          // Initialize the pointer to the wrapper
  p_engine->p_handler = p_handler;                      // Initialize the event handler
  os_mutex_init(&(p_engine->mutex));                    // Initialize the mutex
  os_cond_init(&(p_engine->cond));                      // Initialize the condition variable
  /* Initialize the mutex */
  os_mutex_init(&(p_engine->frame_done_mutex));
  /* Initialize the condition variable */
  os_cond_init(&(p_engine->frame_done_cond));

  STD_MEMSET(output_buf, 0, sizeof(struct jpege_hw_buf));

  // Initialize jpege driver library
  jpege_hw_fd = jpege_lib_init(&gmn_obj,
    jpege_engine_hw_event_handler,
    jpege_engine_hw_output_handler,
    jpege_engine_hw_input_handler);
  if (jpege_hw_fd < 0 || !gmn_obj) {
    JPEG_DBG_ERROR("jpege_engine_hw_init: jpege_lib_init failed\n");
    JPEG_FREE(p_engine);
    return JPEGERR_EFAILED;
  }

  p_engine->gmn_obj = gmn_obj;
  p_engine->jpege_hw_fd  = jpege_hw_fd;
  p_engine->num_output_buffers = 0;
  JPEG_DBG_MED ("jpege driver library init done\n");

  // Initialize the state
  p_engine->state = JPEGE_HW_IDLE;
  p_engine->reencode_info.current = 0;
  p_engine->reencode_info.total_num_attempts = 1;
  p_engine->reencode_info.type[0] = REENCODE_TYPE_NONE;
  p_engine->reencode_info.type[1] = REENCODE_TYPE_PREV_ESTIMATE;
  p_engine->reencode_info.reencode = false;

  // Assign allocated engine structure to p_obj
  p_obj->p_engine = (void *)p_engine;
  p_obj->is_initialized = true;
  return JPEGERR_SUCCESS;
}

static int jpege_engine_hw_check_start_param (
  jpege_img_cfg_t      *p_config,
  jpege_img_data_t     *p_source,
  jpege_engine_dst_t   *p_dest)
{
  uint32_t i;

  JPEG_DBG_HIGH("jpege_engine_hw_check_start_param : E\n");
  JPEG_DBG_HIGH("scaling enabled = %d\n",p_config->scale_cfg.enable);

  // Pointer validation
  if (!p_config || !p_source || !p_dest) {
    JPEG_DBG_HIGH("%s : Invalid input p_config=%p, p_source=%p, p_dest=%p\n",
                           __func__, p_config, p_source, p_dest);
    return JPEGERR_ENULLPTR;
  }

  // Support H2V2YCrCb only
  if (p_source->color_format > MONOCHROME) {
    JPEG_DBG_HIGH("jpege_engine_hw_check_start_param: only support"
      " YCRCB H2V2,YCBCR H2V2,H1V1, H1V2 and H2V1)\n");
    return JPEGERR_EUNSUPPORTED;
  }

  // Validate rotation
  if (!(p_config->rotation_degree_clk == 0 )){
    JPEG_DBG_HIGH("jpege_engine_hw_configure: rotation angle %d not supported\n",
      p_config->rotation_degree_clk);
    return JPEGERR_EBADPARM;
  }

  // Validate fragments
  if (!p_source->fragment_cnt) {
    JPEG_DBG_MED("jpege_engine_hw_check_start_param: no valid fragment\n");
    return JPEGERR_EBADPARM;
  }

  // Validate Scaling params
  if (p_config->scale_cfg.enable) {

    if (p_config->scale_cfg.enable &&
        (!p_config->scale_cfg.input_width ||
         !p_config->scale_cfg.input_height ||
         !p_config->scale_cfg.output_width ||
         !p_config->scale_cfg.output_height ||
         (p_config->scale_cfg.input_width  > MAX_JPEGE_WIDTH) ||
         (p_config->scale_cfg.input_height > MAX_JPEGE_HEIGHT) ||
         (p_config->scale_cfg.h_offset > 0x2000) ||
         (p_config->scale_cfg.v_offset > 0x2000) ||
         ((p_config->scale_cfg.h_offset + p_config->scale_cfg.input_width) >
           p_source->width)||
         ((p_config->scale_cfg.v_offset + p_config->scale_cfg.input_height) >
           p_source->height)))
    {
      JPEG_DBG_ERROR("jpege_engine_hw_check_start_param: invalid scale config:"
        "(h offset, v offset)(%d, %d)  (input width, input height)(%d, %d)"
        "(output width,output height)(%d, %d)(source width, source height)"
        "(%d, %d)\n",p_config->scale_cfg.h_offset, p_config->scale_cfg.v_offset,
          p_config->scale_cfg.input_width, p_config->scale_cfg.input_height,
          p_config->scale_cfg.output_width, p_config->scale_cfg.output_height,
          p_source->width, p_source->height);
      return JPEGERR_EBADPARM;
    }
    if (p_config->scale_cfg.output_width > p_config->scale_cfg.input_width ||
        p_config->scale_cfg.output_height > p_config->scale_cfg.input_height) {
          if ((p_config->scale_cfg.output_width >
             (p_config->scale_cfg.input_width << 8)) ||
             (p_config->scale_cfg.output_height >
             (p_config->scale_cfg.input_height << 8))) {
               JPEG_DBG_ERROR("%s: Upscaling > 8x not supported", __func__);
               return JPEGERR_EUNSUPPORTED;
          }
    }
    if((p_config->scale_cfg.input_width > p_config->scale_cfg.output_width)||
       (p_config->scale_cfg.input_height > p_config->scale_cfg.output_height)) {
         if((p_config->scale_cfg.output_width <
            (floor(p_config->scale_cfg.input_width >> 16)))||
            (p_config->scale_cfg.output_height <
            (floor(p_config->scale_cfg.input_height >> 16)))){
              JPEG_DBG_ERROR("%s: Downscaling < 1/16 not supported", __func__);
              return JPEGERR_EUNSUPPORTED;
      }
    }
  }

  if (((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->pmem_fd < 0
  || ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->pmem_fd < 0)
  {
    JPEG_DBG_MED("jpege_engine_hw_check_start_param: warning: input buffer not"
      "physically mapped; " "use internal PMEM\n");
    // flag is set to true if input is provided from PMEM
  }
  else if (((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->offset
    < p_source->width * p_source->height ||
    ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->offset
    < (p_source->width * p_source->height / 2) ) {
      JPEG_DBG_MED("jpege_engine_hw_check_start_param: Input PMEM is Not"
        "sufficient\n");
      return JPEGERR_EBADPARM;
  }

  if ( (p_source->width > MAX_JPEGE_WIDTH) ||
    (p_source->height > MAX_JPEGE_HEIGHT)){
    JPEG_DBG_MED("jpege_engine_hw_check_start_param: maximum resolution"
      " supported is %dx%d\n",MAX_JPEGE_WIDTH, MAX_JPEGE_HEIGHT);
    return JPEGERR_EBADPARM;
  }
  JPEG_DBG_LOW ("jpege_engine_hw_check_start_param: done\n");
  return JPEGERR_SUCCESS;
}

static int jpege_engine_hw_start(
  jpege_engine_obj_t   *p_obj,
  jpege_img_cfg_t      *p_config,
  jpege_img_data_t     *p_source,
  jpege_engine_dst_t   *p_dest)
{
  int i, rc, chroma_size =0, luma_size =0, offset=0, y_len=0;
  jpege_engine_hw_t     *p_engine;

  // Validate input arguments
  if (!p_obj || !p_obj->p_engine)
    return JPEGERR_ENULLPTR;

  // Cast p_obj->obj as jpege_engine_hw_t object
  p_engine = p_obj->p_engine;

  // Make sure engine is idle
  os_mutex_lock(&(p_engine->mutex));
  if (p_engine->is_active) {
    os_mutex_unlock(&(p_engine->mutex));
    JPEG_DBG_HIGH("jpege_engine_hw_start: engine not idle\n");
    return JPEGERR_EBADSTATE;
  }

  // Reset abort flag
  p_engine->abort_flag = false;
  // Reset error flag
  p_engine->error_flag = false;

  // Store p_dest
  p_engine->dest = *p_dest;
  p_engine->output_done = false;

  //Reset Global values
  input_buf[0].y_off = 0;
  input_buf[0].cbcr_off = 0;

  // to do: configuration ; this for test purpose
  /*------------------------------------------------------------
   Initialize the fields in the p_engine structure
  --------------------------------------------------------------*/
  p_engine->restart_interval = p_config->restart_interval;
  p_engine->rotation = p_config->rotation_degree_clk;
  p_engine->quality = p_config->quality;
  p_engine->luma_width  = p_source->width;
  p_engine->luma_height = p_source->height;

  //Calculate the addional padding other than the CEILING16(height) for H2V2.
  if(p_source->color_format == YCRCBLP_H2V2 ||
     p_source->color_format == YCBCRLP_H2V2){
        luma_size = ((jpeg_buf_t *)p_source->
                    p_fragments[0].color.yuv.luma_buf)->size;
        chroma_size = ((jpeg_buf_t *)p_source->
                       p_fragments[0].color.yuv.chroma_buf)->size;
      offset = luma_size - chroma_size;
      y_len = CEILING16(p_source->width)* CEILING16(p_source->height);
      JPEG_DBG_ERROR("%s luma_size = %d chroma_size = %d size = %d y_len =%d\n",
        __func__, luma_size,chroma_size,offset,y_len);
      p_engine->padding = offset - y_len;
  }
  else
     p_engine->padding = 0;

  JPEG_DBG_ERROR("%s: Additional paddding = %d\n",__func__,p_engine->padding);

  p_engine->input_format = ((uint8_t)p_source->color_format % 2) ?
    YCBCR : YCRCB;
  p_engine->subsampling =
    (jpeg_subsampling_t)((uint8_t)p_source->color_format / 2);

  switch(p_source->color_format) {
    case YCRCBLP_H2V2:
    case YCBCRLP_H2V2:
      p_engine->color_format = JPEGE_INPUT_H2V2;
      //Added to handle double padding
      input_buf[0].y_off = p_engine->padding;
      input_buf[0].cbcr_off = p_engine->padding/2;
      break;

    case YCRCBLP_H2V1:
    case YCBCRLP_H2V1:
      p_engine->color_format = JPEGE_INPUT_H2V1;
      break;

    case YCRCBLP_H1V2:
    case YCBCRLP_H1V2:
      p_engine->color_format = JPEGE_INPUT_H1V2;
      break;

    case YCRCBLP_H1V1:
    case YCBCRLP_H1V1:
      p_engine->color_format = JPEGE_INPUT_H1V1;
      break;

     case MONOCHROME:
       p_engine->color_format = JPEGE_INPUT_MONOCHROME;
       break;

    default:
      p_engine->color_format = JPEGE_INPUT_H2V2;
      break;
  }

     p_engine->num_of_input_planes = p_source->p_fragments[0].num_of_planes;
  /* Initialize input Y and CbCr/CrCb pointers */
      p_engine->p_input_luma   = ((jpeg_buf_t *)p_source->
                                p_fragments[0].color.yuv.luma_buf);
      p_engine->p_input_chroma = ((jpeg_buf_t *)p_source->
                                p_fragments[0].color.yuv.chroma_buf);
      p_engine->p_input_cr = ((jpeg_buf_t *)p_source->
                                p_fragments[0].color.yuv.cr_buf);

  //If scaling is enabled, pass the scaling parameters
   if(p_config->scale_cfg.enable) {
     p_engine->scale_cfg = p_config->scale_cfg;
   }
   else
     p_engine->scale_cfg.enable = false;

  // Start Jpeg Encoding as a new thread
  rc = os_thread_create(&p_engine->thread, jpege_engine_hw_encode,
    (void *)p_engine); // If there is a failure in creating the thread, clean up and exit
  if (rc) {
    os_mutex_unlock(&p_engine->mutex);
    JPEG_DBG_HIGH("jpege_engine_sw_start: os_thread_create() failed: %d\n", rc);
    return JPEGERR_EFAILED;
  }
  p_engine->is_active = true;
  os_mutex_unlock(&(p_engine->mutex));

  return JPEGERR_SUCCESS;
}

int configure_chroma_size(jpege_engine_hw_t *p_engine)
{
    switch (p_engine->color_format)
    {
      case JPEGE_INPUT_H2V2:
      {
          p_engine->chroma_width = CEILING16(p_engine->luma_width);
          p_engine->chroma_height = (CEILING16(p_engine->luma_height)) >> 1;
          break;
        }
      case JPEGE_INPUT_H2V1:
      {
         p_engine->chroma_width = p_engine->luma_width;
         p_engine->chroma_height = p_engine->luma_height;
         break;
      }
      case JPEGE_INPUT_H1V2:
        {
           p_engine->chroma_width = p_engine->luma_width*2;
           p_engine->chroma_height = p_engine->luma_height/2;
           break;
        }
      case JPEGE_INPUT_H1V1:
        {
           p_engine->chroma_width = p_engine->luma_width;
           p_engine->chroma_height = p_engine->luma_height *2;
           break;
        }
       case JPEGE_INPUT_MONOCHROME:
          p_engine->chroma_width = 0;
          p_engine->chroma_height = 0;
           break;
      default:
         JPEG_DBG_ERROR("Invalid JPEG Format = %d\n",p_engine->color_format);
          return JPEGERR_EBADPARM;
    }
   return JPEGERR_SUCCESS;
}

static void jpeg_engine_reencode(jpege_engine_hw_t *p_engine)
{
  jpege_hw_obj_t gmn_obj = 0;
  int jpege_hw_fd = -1;
  p_engine->error_flag = false;
  p_engine->abort_flag = false;// Initialize gemini driver library
  jpege_hw_fd = jpege_lib_init(&gmn_obj,
    jpege_engine_hw_event_handler,
    jpege_engine_hw_output_handler,
    jpege_engine_hw_input_handler);

  p_engine->gmn_obj = gmn_obj;
  p_engine->jpege_hw_fd  = jpege_hw_fd;
  p_engine->num_output_buffers = 0;
  p_engine->output_done = false;
  jpege_engine_hw_encode((void *)p_engine);
}

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
  jpege_engine_hw_encode(OS_THREAD_FUNC_ARG_T arg)
{
  // Cast input argument as the engine object
  jpege_engine_hw_t *p_engine = arg;

  int imageHeight;
  int imageWidth;
  int chromaHeight;
  int chromaWidth;

  jpege_hw_obj_t gmn_obj = 0;
  int jpege_hw_fd = -1;
  int rc;

  p_engine_local = p_engine;

  // Validate hw object:  to do
  gmn_obj = p_engine->gmn_obj;
  jpege_hw_fd = p_engine->jpege_hw_fd;
  if (jpege_hw_fd < 0 || !gmn_obj) {
    JPEG_DBG_HIGH("jpege_engine_hw_start: jpege init failed\n");
    p_engine->error_flag = true;
    goto err;
  }

  //Configure chroma size based on input format
  configure_chroma_size(p_engine);

  if(p_engine->color_format == JPEGE_INPUT_H2V2){
      imageHeight = CEILING16(p_engine->luma_height);
      imageWidth = p_engine->luma_width;
  }
  else{
      imageHeight = p_engine->luma_height;
      imageWidth = p_engine->luma_width;
  }
  chromaHeight = p_engine->chroma_height;
  chromaWidth = p_engine->chroma_width;

  jpege_cmd_input_cfg input_cfg = { p_engine->color_format,  /*  inputFormat */
    p_engine->input_format,   /*  input_cbcr_ordering */
    GEMINI_FE_BURST_LENGTH4,  /*  fe_burst_length; */
    0,   /*  byte_ordering */
    imageWidth,
    imageHeight,
    p_engine->num_of_input_planes,
  };

  jpege_cmd_output_cfg output_cfg = { GEMINI_WE_BURST_LENGTH8,  /*  we_burst_length; */
    0, // test_param.output_byte_ordering,  /*  byte_ordering */
  };
  jpege_huffmanDcTable sHuffmanTblYDcPtr;
  jpege_huffmanAcTable sHuffmanTblYAcPtr;
  jpege_huffmanDcTable sHuffmanTblCbcrDcPtr;
  jpege_huffmanAcTable sHuffmanTblCbcrAcPtr;

  jpege_quantTable sQuantTblY;
  jpege_quantTable sQuantTblChroma;

  BOOL fscEnable = (REENCODE_TYPE_FSC ==
    p_engine_local->reencode_info.type[p_engine_local->reencode_info.current]);

  jpege_cmd_jpeg_encode_cfg encode_cfg = { 0,  /*  0x64, restartInterval */
    0,        /*  1, bCustomHuffmanTbl */
    NULL,    /*  &sHuffmanTblYDcPtr, huffmanTblYDcPtr */
    NULL,    /*  &sHuffmanTblYAcPtr, huffmanTblYAcPtr */
    NULL,    /*  &sHuffmanTblCbcrDcPtr, huffmanTblCbcrDcPtr */
    NULL,    /*  &sHuffmanTblCbcrAcPtr, huffmanTblCbcrAcPtr */
    &sQuantTblY,  /*  quantTblY */
    &sQuantTblChroma,  /*  quantTblChroma */
  };

  //Pass scaling params if scaling is enabled
 jpege_cmd_scale_cfg scale_cfg;
  if(p_engine->scale_cfg.enable) {
      //Scaling and cropping are treated as separate operations
      //Check if scaling is really enabled
      if(p_engine->scale_cfg.input_width < p_engine->scale_cfg.output_width||
         p_engine->scale_cfg.input_width > p_engine->scale_cfg.output_width ||
         p_engine->scale_cfg.input_height < p_engine->scale_cfg.output_height ||
         p_engine->scale_cfg.input_height > p_engine->scale_cfg.output_height)
         {
             scale_cfg.scale_enable       = p_engine->scale_cfg.enable;
         }
     //Check if cropping is enabled
      if ((scale_cfg.h_offset != 0 || scale_cfg.v_offset != 0 ||
         ((scale_cfg.h_offset + scale_cfg.scale_input_width) <
           p_engine->luma_width) ||
         ((scale_cfg.v_offset + scale_cfg.scale_input_height) <
           p_engine->luma_height)) &&
         (scale_cfg.scale_input_width != p_engine->luma_width ||
          scale_cfg.scale_input_height != p_engine->luma_height))
         {
                scale_cfg.crop_enable = true;
         }

      scale_cfg.scale_input_width  = p_engine->scale_cfg.input_width;
      scale_cfg.scale_input_height = p_engine->scale_cfg.input_height;
      scale_cfg.h_offset = p_engine->scale_cfg.h_offset;
      scale_cfg.v_offset = p_engine->scale_cfg.v_offset;
      scale_cfg.output_width = p_engine->scale_cfg.output_width;
      scale_cfg.output_height = p_engine->scale_cfg.output_height;

  }
  else{
        scale_cfg.scale_enable       = p_engine->scale_cfg.enable;
        scale_cfg.scale_input_width  = imageWidth;
        scale_cfg.scale_input_height = imageHeight;
        scale_cfg.h_offset = 0;
        scale_cfg.v_offset = 0;
        scale_cfg.output_width = p_engine->luma_width;
        scale_cfg.output_height = p_engine->luma_height;
   }

   uint32_t buffer_size = (imageHeight * imageWidth)+(chromaHeight *
      chromaWidth);

  jpege_app_img_src src = { buffer_size  ,  /*  buffer_size; */
    p_engine->quality, // test_param.output_quality,  /*  quality; */
    imageWidth,  /*  image_width; */
    imageHeight,  /*  image_height; */
    2,  /*  h_sampling; */
    2,  /*  v_sampling; */
  };

  jpege_app_calc_param (&encode_cfg, src);
  encode_cfg.restartInterval = 0;  /*  0x64; */

  p_engine->input_pmem_flag = true;

  if (((jpeg_buf_t *)p_engine->p_input_luma)->pmem_fd < 0 ||
    ((jpeg_buf_t *)p_engine->p_input_chroma)->pmem_fd < 0) {
     JPEG_DBG_HIGH("jpege_engine_hw_start: warning: input buffer not"
       "physically mapped; " "use internal PMEM\n");
    // flag is set to true if input is provided from PMEM
     p_engine->input_pmem_flag = false;
  }

  rc = jpege_lib_hw_config (gmn_obj, &input_cfg, &output_cfg, &encode_cfg,
    &scale_cfg);
  if (rc) {
    JPEG_DBG_HIGH("jpege_engine_hw_start: jpege_lib_hw_config failed\n");
    p_engine->error_flag = true;
    goto err;
  }

  rc = jpege_engine_hw_input_buf_enq (gmn_obj, p_engine);
  if (rc) {
    JPEG_DBG_HIGH("jpege_engine_hw_start: jpege_engine_hw_input_buf_enq"
      "failed\n"); p_engine->error_flag = true;
    jpege_engine_hw_free_input_buf(p_engine);
    goto err;
  }

  rc = jpege_engine_hw_output_buf_enq (gmn_obj, p_engine);
  if (rc) {
    JPEG_DBG_ERROR("jpege_engine_hw_start: jpege_engine_hw_output_buf_enq"
      "failed\n");
    p_engine->error_flag = true;
    jpege_engine_hw_free_output_buf ();
    goto err;
  }

  rc = jpege_lib_encode (gmn_obj);
  if (rc) {
    JPEG_DBG_ERROR("jpege_engine_hw_start: jpege_lib_encode failed\n");
    p_engine->error_flag = true;
    goto err;
  }

  JPEG_DBG_MED ("%s:%d] waiting for frame done event\n", __func__, __LINE__);
  os_mutex_lock(&(p_engine->frame_done_mutex));
  if (!p_engine->output_done) {
    rc = os_cond_timedwait(&p_engine->frame_done_cond,
      &(p_engine->frame_done_mutex), 20000);
  }
  os_mutex_unlock(&(p_engine->frame_done_mutex));

  JPEG_DBG_MED ("%s:%d] waiting for all done... rc %d\n", __func__, __LINE__,
    rc);
  if (rc) {
    JPEG_DBG_ERROR("jpege_engine_hw_start: os_cond_timedwait failed\n");
    p_engine->error_flag = true;
    goto err;
  }

  rc = jpege_lib_wait_done (gmn_obj);
  if (rc) {
    JPEG_DBG_ERROR("jpege_engine_hw_start: jpege_lib_wait_done failed\n");
    p_engine->error_flag = true;
    goto err;
  }

  jpege_engine_hw_free_input_buf (p_engine);
  jpege_engine_hw_free_output_buf ();

  jpege_lib_stop (gmn_obj, 0);

  // Notify event done
err:
  jpege_lib_release (gmn_obj);

  if (p_engine->error_flag &&
    p_engine_local->reencode_info.reencode) {
    JPEG_DBG_HIGH("%s:%d] Reencode", __func__, __LINE__);
    jpeg_engine_reencode(p_engine);
    return OS_THREAD_FUNC_RET_SUCCEEDED;
  }

  os_mutex_lock(&(p_engine->mutex));
  p_engine->is_active = false;
  JPEG_DBG_MED ("%s:%d] error_val %d\n", __func__, __LINE__,
    p_engine->error_flag);
  if (p_engine->abort_flag) {
    p_engine->abort_flag = false;
    os_cond_signal(&p_engine->cond);
    os_mutex_unlock(&(p_engine->mutex));
  } else {
    if (p_engine->error_flag) {
      p_engine->error_flag = false;
      os_mutex_unlock(&(p_engine->mutex));
      // Throw encode error event
      p_engine->p_handler(p_engine->p_wrapper, JPEG_EVENT_ERROR, NULL);
    } else {
      os_mutex_unlock(&(p_engine->mutex));
      p_engine->p_handler(p_engine->p_wrapper, JPEG_EVENT_DONE, NULL);
    }
  }
  return OS_THREAD_FUNC_RET_SUCCEEDED;
}

static int jpege_engine_hw_abort(
  jpege_engine_obj_t *p_obj)
{
  jpege_engine_hw_t *p_engine;

  // Validate input arguments
  if (!p_obj || !p_obj->p_engine)
    return JPEGERR_ENULLPTR;

  // Cast p_obj->obj as jpege_engine_hw_t object
  p_engine = p_obj->p_engine;

  // Abort if engine is actively encoding
  os_mutex_lock(&(p_engine->mutex));
  if (p_engine->is_active) {
    p_engine->abort_flag = true;
    while (p_engine->abort_flag) {
      os_cond_wait(&(p_engine->cond), &(p_engine->mutex));
    }
  }
  os_mutex_unlock(&(p_engine->mutex));
  return JPEGERR_SUCCESS;
}

static void jpege_engine_hw_destroy(
  jpege_engine_obj_t *p_obj)
{
  if (p_obj) {
    jpege_engine_hw_t *p_engine = (jpege_engine_hw_t *)(p_obj->p_engine);
    // Abort and wait until engine is done with current encoding
    // todo
    (void)jpege_engine_hw_abort(p_obj);

    JPEG_DBG_LOW("jpege_engine_hw_destroy\n");
    // Release allocated memory
    if (p_engine) {
      // Join the engine thread
      JPEG_DBG_MED("jpege_engine_sw_destroy: join %d %d",
        p_engine->thread, os_thread_self());
      if (os_thread_self() != p_engine->thread) {
        os_thread_join(&p_engine->thread, NULL);
      }

      os_mutex_destroy(&(p_engine->mutex));
      os_cond_destroy(&(p_engine->cond));
      JPEG_FREE(p_obj->p_engine);
      p_obj->p_engine = NULL;
    }
  }
}
