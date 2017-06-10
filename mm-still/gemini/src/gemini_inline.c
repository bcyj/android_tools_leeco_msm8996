/* Copyright (c) 2011-2012, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "../src/gemini_dbg.h"
#include "gemini_lib.h"
#include "gemini_app_util_mmap.h"
#include "gemini_app_calc_param.h"
#include "gemini_inline.h"
#include "gemini_lib_common.h"

int total_frames;


#define IMG_WIDTH            1280
#define IMG_HEIGHT           960
#define NUM_OF_MCU_ROWS      1
#define CAPTURE_WIDTH          1280
#define IMEM_Y_SIZE            CAPTURE_WIDTH*16
#define IMEM_CBCR_SIZE         CAPTURE_WIDTH*8
#define IMEM_Y_PING_OFFSET     0x2E000000
#define IMEM_CBCR_PING_OFFSET  IMEM_Y_PING_OFFSET + IMEM_Y_SIZE
#define IMEM_Y_PONG_OFFSET     IMEM_CBCR_PING_OFFSET + IMEM_CBCR_SIZE
#define IMEM_CBCR_PONG_OFFSET  IMEM_Y_PONG_OFFSET + IMEM_Y_SIZE
#define FILE_NAME_MAX_SIZE 255

gmn_obj_t gmn_obj = 0;
gemini_params_t gemini_params = {
	IMG_WIDTH, // int yuv_w;
	IMG_HEIGHT, // int yuv_h;
	"/data/test.yuv", // char *input_yuv_filename;
	"/data/Inline", // char *output_bs_filename;
	NULL, // char *output_jpeg_filename;

	0, // int input_byte_ordering;
	0, // int output_byte_ordering;
	1, // int cbcr_ordering;
	0, // int zero_y;
	0, // int zero_cbcr;
	85, // int output_quality;

	0, // gmn_obj_t gmn_obj;
	-1, // int gmnfd;
	GEMINI_MODE_OFFLINE_ENCODE
};

int gemini_app_event_handler (gmn_obj_t gmn_obj,
  struct gemini_evt *pGmnCtrlCmd,
  int event)
{
  total_frames++;
  GMN_DBG ("%s:%d] handler called: event %d total frames %d\n", __func__,
    __LINE__, event, total_frames);
  return 0;
}

/* output buf processing */
static struct gemini_buf output_buf[] = {
  /*
   * type; fd; vaddr; y_off; y_len; framedone_len; cbcr_off; cbcr_len; NumOfMCURows
   */
  {0, 0, (void *) 0x0, 0, 2880000, 0, 0, 0, 0},
/* 	{0, 0, (void *) 0x0, 0, 4096, 0, 0, 0, 0}, */
/* 	{0, 0, (void *) 0x0, 0, 4096, 0, 0, 0, 0}, */
};

void gemini_app_output_buf_enq (gmn_obj_t gmn_obj)
{
    unsigned int i;

    for (i = 0; i < sizeof (output_buf) / sizeof (output_buf[0]); i++) {
	output_buf[i].alloc_ion.len = output_buf[i].y_len;
	output_buf[i].alloc_ion.flags = ION_FLAG_CACHED;
	output_buf[i].alloc_ion.heap_mask = 0x1 << ION_IOMMU_HEAP_ID;
	output_buf[i].alloc_ion.align = 4096;
	output_buf[i].vaddr =
		do_mmap (output_buf[i].y_len, &output_buf[i].fd,
		  GEMINI_PMEM_ADSP, &output_buf[i].ion_fd_main, &output_buf[i].alloc_ion,
				 &output_buf[i].fd_ion_map);

	if (!output_buf[i].vaddr) {
		GMN_DBG ("%s:%d] fail\n", __func__, __LINE__);
		break;
	}
	GMN_DBG ("gemini_app_output_buf_enq: gemini_lib_output_buf_enq: output_buf[%d] \n",i);
	gemini_lib_output_buf_enq (gmn_obj, &output_buf[i]);
	}
}

void gemini_app_free_output_buf (void)
{
  unsigned int i;

  for (i = 0; i < sizeof (output_buf) / sizeof (output_buf[0]); i++) {
    do_munmap (output_buf[i].fd, output_buf[i].vaddr,
      output_buf[i].y_len, output_buf[i].ion_fd_main,
      &output_buf[i].fd_ion_map);
  }
}

int gemini_app_output_handler (gmn_obj_t gmn_obj,
      struct gemini_buf *buf)
{
  static FILE *jpeg_bs_fout = NULL;
  static FILE *jpeg_fout = NULL;
  static int output_total_segments = 0;
  long size;

  if (!jpeg_bs_fout) {
    char open_mode;
    if (gemini_params.output_jpeg_filename) {
      jpeg_fout = fopen (gemini_params.output_jpeg_filename, "a");
      if (!jpeg_fout) {
      GMN_DBG ("failed to open %s\n",
       gemini_params.output_jpeg_filename);
      return -1;
    }
  }

  char filename[FILE_NAME_MAX_SIZE] = "\0";
  static count = 0;
  snprintf(filename, FILE_NAME_MAX_SIZE, "%s_%d.bs", gemini_params.output_bs_filename, count);
  count++;
  jpeg_bs_fout = fopen (filename, "wb");
  if (!jpeg_bs_fout) {
  GMN_DBG ("failed to open %s\n", gemini_params.output_bs_filename);
  return -1;
}
GMN_DBG ("%s:%d] handler called: file open %s\n", __func__,
__LINE__, gemini_params.output_bs_filename);
  }

  GMN_PR_ERR ("%s:%d] buf->vaddr = %p, buf->y_len = %d, framedone_len %d\n",
    __func__, __LINE__, buf->vaddr, buf->y_len,
    buf->framedone_len);

  if (buf->type == GEMINI_EVT_FRAMEDONE)
    size = buf->framedone_len;
  else
    size = buf->y_len;

  fwrite (buf->vaddr, 1, size, jpeg_bs_fout);
  if (jpeg_fout)
    fwrite (buf->vaddr, 1, size, jpeg_fout);

  output_total_segments++;
  GMN_DBG ("%s:%d] buf->type: %d total segments: %d\n", __func__,
    __LINE__, buf->type, output_total_segments);

  if (buf->type == GEMINI_EVT_FRAMEDONE) {
   fclose (jpeg_bs_fout);
   GMN_DBG ("%s:%d] framedone: file close %s, total segments %d\n",
      __func__, __LINE__, gemini_params.output_bs_filename,
      output_total_segments);
   jpeg_bs_fout = NULL;
  }

  GMN_DBG ("gemini_app_output_handler: gemini_lib_output_buf_enq: buf %p \n", buf);
  gemini_lib_output_buf_enq (gmn_obj, buf);

  return 0;
}

/* input buf processing */
struct gemini_buf input_buf[] = {
  /*
   * type; fd; vaddr; y_off; y_len; framedone_len; cbcr_off; cbcr_len; NumOfMCURows;
   */
   {0, 0, (void *) 0x0, IMEM_Y_PING_OFFSET, IMEM_Y_SIZE, 0, IMEM_CBCR_PING_OFFSET, IMEM_CBCR_SIZE, NUM_OF_MCU_ROWS},
   {0, 0, (void *) 0x0, IMEM_Y_PONG_OFFSET, IMEM_Y_SIZE, 0, IMEM_CBCR_PING_OFFSET, IMEM_CBCR_SIZE, NUM_OF_MCU_ROWS},
/* {0, 0, (void *) 0x0, 0, 8192, 0, 0x0, 4096, 30}, */
/* {0, 0, (void *) 0x0, 0, 8192, 0, 0x0, 4096, 30}, */
/* {0, 0, (void *) 0x0, 0, 8192, 0, 0x0, 4096, 30}, */
/* {0, 0, (void *) 0x0, 0, 8192, 0, 0x0, 4096, 30}, */
/* {0, 0, (void *) 0x0, 0, 8192, 0, 0x0, 4096, 30}, */
/* {0, 0, (void *) 0x0, 0, 8192, 0, 0x0, 4096, 30}, */
/* {0, 0, (void *) 0x0, 0, 0,    0, 0x0,    0,  0}, */
};

struct gemini_buf *input_p;
void gemini_app_input_buf_enq (gmn_obj_t gmn_obj)
{
  unsigned int i;
  long n;

  for (i = 0; i < sizeof (input_buf) / sizeof (input_buf[0]); i++) {
          GMN_DBG ("%s:gemini_inline: y_off=%x n_mcus=%d cbcr_off=%x\n",
            __func__, input_buf[i].y_off, input_buf[i].num_of_mcu_rows,
            input_buf[i].cbcr_off);
        gemini_lib_input_buf_enq (gmn_obj, &input_buf[i]);
  }
  input_p = &input_buf[i + 1];
  return;
}

void gemini_app_free_input_buf (void)
{
  unsigned int i;

  for (i = 0; i < sizeof (input_buf) / sizeof (input_buf[0]); i++) {
    if (input_buf[i].vaddr)
      do_munmap (input_buf[i].fd, input_buf[i].vaddr,
        input_buf[i].y_len + input_buf[i].cbcr_len,
        input_buf[i].ion_fd_main,
        &input_buf[i].fd_ion_map);
  }
}

int gemini_app_input_handler (gmn_obj_t gmn_obj,
  struct gemini_buf *buf)
{
  static int input_total_segments = 0;
  long n;

  if (input_p != NULL && input_p->y_len != 0) {
    input_p->alloc_ion.len = input_p->y_len + input_p->cbcr_len;
    input_p->alloc_ion.flags = ION_FLAG_CACHED;
    input_p->alloc_ion.heap_mask = 0x1 << ION_IOMMU_HEAP_ID;
    input_p->alloc_ion.align = 4096;
    input_p->vaddr =
    do_mmap (input_p->y_len + input_p->cbcr_len,
      &input_p->fd, GEMINI_PMEM_ADSP,
      &input_p->ion_fd_main, &input_p->alloc_ion,
      &input_p->fd_ion_map);
    if (!input_p->vaddr) {
      GMN_DBG ("%s:%d] fail\n", __func__, __LINE__);
      return 0;
  }
      gemini_lib_input_buf_enq (gmn_obj, input_p);
      input_p++;
      input_total_segments++;
  }
  GMN_DBG ("%s:%d] %p total segments %d\n", __func__, __LINE__, buf,
      input_total_segments);
  return 0;
}

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

const uint32_t nZigzagTable[64] = { 0, 1, 8, 16, 9, 2, 3, 10,
  17, 24, 32, 25, 18, 11, 4, 5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13, 6, 7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

void gemini_app_quant_table_helper (gemini_cmd_jpeg_encode_cfg * p_encode_cfg)
{
  uint32_t nIndex = 0;

  /*
  * Q Tables
  */
  for (nIndex = 0; nIndex < 64; nIndex++) {
    if (!gemini_params.output_jpeg_filename) {
      p_encode_cfg->quantTblY->qtbl[nIndex] =
        DEFAULT_QTABLE_0[nIndex];
      p_encode_cfg->quantTblChroma->qtbl[nIndex] =
        DEFAULT_QTABLE_1[nIndex];
    } else {
      p_encode_cfg->quantTblY->qtbl[nIndex] =
        DEFAULT_QTABLE_0_1[nIndex];
      p_encode_cfg->quantTblChroma->qtbl[nIndex] =
        DEFAULT_QTABLE_1_1[nIndex];
    }
  }
}

void gemini_app_quant_zigzag_table_helper (gemini_cmd_jpeg_encode_cfg * p_encode_cfg)
{
  uint32_t i, j;

  for (i = 0; i < 64; i++) {
    j = nZigzagTable[i];
    if (!gemini_params.output_jpeg_filename) {
      p_encode_cfg->quantTblY->qtbl[j] = DEFAULT_QTABLE_0[i];
      p_encode_cfg->quantTblChroma->qtbl[j] =
        DEFAULT_QTABLE_1[i];
    } else {
    p_encode_cfg->quantTblY->qtbl[j] =
      DEFAULT_QTABLE_0_1[i];
    p_encode_cfg->quantTblChroma->qtbl[j] =
      DEFAULT_QTABLE_1_1[i];
    }
  }
}

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

void gemini_app_huffman_table_helper (gemini_cmd_jpeg_encode_cfg * p_encode_cfg)
{
  uint32_t nIndex = 0;

  gemini_huffmanDcTable sHuffmanTblYDcPtr;
  gemini_huffmanAcTable sHuffmanTblYAcPtr;
  gemini_huffmanDcTable sHuffmanTblCbcrDcPtr;
  gemini_huffmanAcTable sHuffmanTblCbcrAcPtr;
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

void gemini_app_send_hw_cmds(int gmnfd)
{
  int result;
  int n = 0;
  struct msm_gemini_hw_cmd *hw_cmd_p;
  struct msm_gemini_hw_cmd hw_cmds[] = {
    /*
     * type, n (max 0xFFF/4095), offset, mask, data/pdata
     */
    {MSM_GEMINI_HW_CMD_TYPE_READ, 1, 0x4, 0xFF00FF00, {0x0}},	/* 30 */
    {MSM_GEMINI_HW_CMD_TYPE_READ, 1, 0x4, 0xFFFFFFFF, {0x0}},	/* 31 */
    {MSM_GEMINI_HW_CMD_TYPE_READ, 4, 0x14C, 0xFF00FF00, {0x0}},	/* 32 */
    {MSM_GEMINI_HW_CMD_TYPE_READ, 4, 0x14C, 0xFFFFFFFF, {0x0}},	/* 33 */
    {MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, 0x4, 0xFF00FF00, {0xFFFFFFFF}},	/* 34 */
    {MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, 0x4, 0xFFFFFFFF, {0xFFFFFFFF}},	/* 35 */
    {MSM_GEMINI_HW_CMD_TYPE_WRITE, 4, 0x14C, 0xFF00FF00, {0xFFFFFFFF}},	/* 36 */
    {MSM_GEMINI_HW_CMD_TYPE_WRITE, 4, 0x14C, 0xFFFFFFFF, {0xFFFFFFFF}},	/* 37 */
    {MSM_GEMINI_HW_CMD_TYPE_UWAIT, 4095, 0x4, 0xFF00FF00, {0x0}},	/* 38 */
    {MSM_GEMINI_HW_CMD_TYPE_UWAIT, 4095, 0x4, 0xFFFFFFFF, {0x0}},	/* 39 */
    {MSM_GEMINI_HW_CMD_TYPE_MWAIT, 4095, 0x14C, 0xFF00FF00, {0xFFFFFFFF}},	/* 40 */
    {MSM_GEMINI_HW_CMD_TYPE_MWAIT, 4095, 0x14C, 0xFFFFFFFF, {0xFFFFFFFF}},	/* 41 */
    {MSM_GEMINI_HW_CMD_TYPE_UDELAY, 4095, 0x0, 0x0, {0x0}},	/* 42 */
    {MSM_GEMINI_HW_CMD_TYPE_MDELAY, 4095, 0x0, 0x0, {0x0}},	/* 43 */
    {MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, 0x24, 0xFFFFFFFF, {0x1}},	/* 37 */
    {MSM_GEMINI_HW_CMD_TYPE_READ, 1, 0x28, 0xFFFFFFFF, {0x0}},	/* 37 */
  };

  hw_cmd_p = &hw_cmds[n];
  result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMD, hw_cmd_p);
  GMN_DBG ("type:%d, n:%d, offset:0x%08x, mask:0x%08x, data/pdata:0x%08x\n",
   hw_cmd_p->type, hw_cmd_p->n, hw_cmd_p->offset, hw_cmd_p->mask, hw_cmd_p->data);
  GMN_DBG ("ioctl MSM_GMN_IOCTL_HW_CMD: %d\n", result);
}

void* gemini_config (int8_t encode_type)
{
	int gmnfd = -1;

	GMN_DBG("\nGMNFD is: %d, GMNOBJ is %p \n" , gmnfd, gmn_obj);
	gmnfd = gemini_lib_init (&gmn_obj, NULL, gemini_app_event_handler,
				 gemini_app_output_handler,
				 gemini_app_input_handler);

	if (gmnfd < 0 || !gmn_obj) {
		GMN_PR_ERR ("%s:%d] gmnfd = %d, gmn_obj = %p\n", __func__,
			    __LINE__, gmnfd, gmn_obj);
		return NULL;
	}

	gemini_params.gmn_obj = gmn_obj;
	gemini_params.gmnfd = gmnfd;
	gemini_params.encode_type = encode_type;

	GMN_DBG ("%s:%d] init done\n", __func__, __LINE__);
        GMN_PR_ERR ("%s: encode_type=%d\n", __func__, gemini_params.encode_type);
	int frame_height_mcus = gemini_params.yuv_h / 16;
	int frame_width_mcus = gemini_params.yuv_w / 16;
	GMN_DBG ("%s:%d] %d %d %d\n", __func__, __LINE__, frame_height_mcus, frame_width_mcus, GEMINI_BURST_LENGTH4);
	gemini_cmd_input_cfg input_cfg = { GEMINI_INPUT_H2V2,	/*  inputFormat */
		gemini_params.cbcr_ordering,	/*  input_cbcr_ordering */
		GEMINI_BURST_LENGTH4,	/*  fe_burst_length; */
		gemini_params.input_byte_ordering,	/*  byte_ordering */
		frame_height_mcus,
		frame_width_mcus,
	};
	gemini_cmd_output_cfg output_cfg = { GEMINI_BURST_LENGTH8,	/*  we_burst_length; */
		gemini_params.output_byte_ordering,	/*  byte_ordering */
	};
	gemini_huffmanDcTable sHuffmanTblYDcPtr;
	gemini_huffmanAcTable sHuffmanTblYAcPtr;
	gemini_huffmanDcTable sHuffmanTblCbcrDcPtr;
	gemini_huffmanAcTable sHuffmanTblCbcrAcPtr;

	gemini_quantTable sQuantTblY;
	gemini_quantTable sQuantTblChroma;

	gemini_cmd_jpeg_encode_cfg encode_cfg = { 0,	/*  0x64, restartInterval */
		0,		/*  1, bCustomHuffmanTbl */
		NULL,		/*  &sHuffmanTblYDcPtr, huffmanTblYDcPtr */
		NULL,		/*  &sHuffmanTblYAcPtr, huffmanTblYAcPtr */
		NULL,		/*  &sHuffmanTblCbcrDcPtr, huffmanTblCbcrDcPtr */
		NULL,		/*  &sHuffmanTblCbcrAcPtr, huffmanTblCbcrAcPtr */
		&sQuantTblY,	/*  quantTblY */
		&sQuantTblChroma,	/*  quantTblChroma */
		0,		/*  1, bFSCEnable */
		{		/*  sFileSizeControl */
		 1,		/* regionSize */
		 29,		/* region0Budget */
		 29,		/* region1Budget */
		 29,		/* region2Budget */
		 29,		/* region3Budget */
		 29,		/* region4Budget */
		 29,		/* region5Budget */
		 29,		/* region6Budget */
		 29,		/* region7Budget */
		 29,		/* region8Budget */
		 29,		/* region9Budget */
		 29,		/* region10Budget */
		 29,		/* region11Budget */
		 29,		/* region12Budget */
		 29,		/* region13Budget */
		 29,		/* region14Budget */
		 29,		/* region15Budget */
		 },
	};
	gemini_cmd_operation_cfg op_cfg = { encode_type,	/*  useMode */
		GEMINI_ROTATE_NONE,	/*  rotationDegree */
		MSM_GMN_OUTMODE_FRAGMENTED
	};
	if (gemini_params.output_jpeg_filename) {
		if (encode_cfg.bCustomHuffmanTbl)
			gemini_app_huffman_table_helper (&encode_cfg);
		if (encode_cfg.quantTblY && encode_cfg.quantTblChroma) {
			/*
			 * if zigzag is not enabled
			 */
			gemini_app_quant_table_helper (&encode_cfg);

			/*
			 * if zigzag is enabled
			 * gemini_app_quant_zigzag_table_helper(&encode_cfg);
			 */
		}
	} else {
		gemini_app_img_src src = { 0,	/*  buffer_size; */
			gemini_params.output_quality,	/*  quality; */
			1600,	/*  image_width; */
			1200,	/*  image_height; */
			2,	/*  h_sampling; */
			2,	/*  v_sampling; */
		};
		gemini_app_calc_param (&encode_cfg, src);
		encode_cfg.restartInterval = 0;	/*  0x64; */

		int k;
		GMN_DBG ("gemini_app_calc_param output: %d\n",gemini_params.output_quality);
		for (k = 0; k < 64; k += 8) {
			GMN_DBG ("%d %d %d %d    %d %d %d %d\n",
				 sQuantTblY.qtbl[k],
				 sQuantTblY.qtbl[k + 1],
				 sQuantTblY.qtbl[k + 2],
				 sQuantTblY.qtbl[k + 3],
				 sQuantTblY.qtbl[k + 4],
				 sQuantTblY.qtbl[k + 5],
				 sQuantTblY.qtbl[k + 6],
				 sQuantTblY.qtbl[k + 7]);
			if (gemini_params.output_quality == 100) {
				GMN_DBG ("change to all 1s\n");
				sQuantTblY.qtbl[k] = 1;
				sQuantTblY.qtbl[k + 1] = 1;
				sQuantTblY.qtbl[k + 2] = 1;
				sQuantTblY.qtbl[k + 3] = 1;
				sQuantTblY.qtbl[k + 4] = 1;
				sQuantTblY.qtbl[k + 5] = 1;
				sQuantTblY.qtbl[k + 6] = 1;
				sQuantTblY.qtbl[k + 7] = 1;
			}
		}

		GMN_DBG ("gemini_app_calc_param output %d\n",gemini_params.output_quality);
		for (k = 0; k < 64; k += 8) {
			GMN_DBG ("%d %d %d %d    %d %d %d %d\n",
				 sQuantTblChroma.qtbl[k],
				 sQuantTblChroma.qtbl[k + 1],
				 sQuantTblChroma.qtbl[k + 2],
				 sQuantTblChroma.qtbl[k + 3],
				 sQuantTblChroma.qtbl[k + 4],
				 sQuantTblChroma.qtbl[k + 5],
				 sQuantTblChroma.qtbl[k + 6],
				 sQuantTblChroma.qtbl[k + 7]);
			if (gemini_params.output_quality == 100) {
				GMN_DBG ("change to all 1s\n");
				sQuantTblChroma.qtbl[k] = 1;
				sQuantTblChroma.qtbl[k + 1] = 1;
				sQuantTblChroma.qtbl[k + 2] = 1;
				sQuantTblChroma.qtbl[k + 3] = 1;
				sQuantTblChroma.qtbl[k + 4] = 1;
				sQuantTblChroma.qtbl[k + 5] = 1;
				sQuantTblChroma.qtbl[k + 6] = 1;
				sQuantTblChroma.qtbl[k + 7] = 1;
			}
		}
	}

	gemini_lib_hw_config (gmn_obj, &input_cfg, &output_cfg, &encode_cfg,
			      &op_cfg);

	gemini_app_input_buf_enq (gmn_obj);
	gemini_app_output_buf_enq (gmn_obj);

        gemini_app_send_hw_cmds(gemini_params.gmnfd);

	total_frames = 0;
	GMN_DBG ("%s:%d] waiting for all done...\n", __func__, __LINE__);

	gemini_lib_encode (gmn_obj);
	ioctl (gmnfd, MSM_GMN_IOCTL_TEST_DUMP_REGION, 0x0150);
#if 0  //Moved to gemini_lib_inline_wait_done() function to stop gemini driver.
	gemini_lib_wait_done (gmn_obj);

	GMN_DBG ("%s:%d] frame done! total_frames = %d\n", __func__, __LINE__,
		 total_frames);
	ioctl (gmnfd, MSM_GMN_IOCTL_TEST_DUMP_REGION, 0x0150);

	gemini_app_free_output_buf ();
	gemini_app_free_input_buf ();

	gemini_lib_stop (gmn_obj, 0);
	GMN_DBG ("%s:%d] stop done\n", __func__, __LINE__);

	gemini_lib_release (gmn_obj);

	GMN_DBG ("%s:%d] release done\n", __func__, __LINE__);

#endif
	return 0;
}

void gemini_lib_inline_wait_done(void)
{
  gemini_lib_wait_done (gmn_obj);
  GMN_DBG ("%s:%d] frame done! total_frames = %d\n", __func__, __LINE__,
  	 total_frames);
  ioctl (gemini_params.gmnfd, MSM_GMN_IOCTL_TEST_DUMP_REGION, 0x0150);
  gemini_app_free_output_buf ();
  gemini_app_free_input_buf ();
  gemini_lib_stop (gmn_obj, 0);
  GMN_DBG ("%s:%d] stop done\n", __func__, __LINE__);
  gemini_lib_release (gmn_obj);
  GMN_DBG ("%s:%d] release done\n", __func__, __LINE__);
}
