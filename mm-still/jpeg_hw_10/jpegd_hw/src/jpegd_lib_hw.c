/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <media/msm_jpeg.h>
#include <errno.h>
#include "jpegd_dbg.h"
#include "jpegd_lib_hw.h"
#include "jpegd_hw_reg_ctrl.h"

/*============================================================================
   MACROS
============================================================================*/
#define USE_STD_DHT_TBL 0
#define Y_DC 0
#define C_DC 1
#define Y_AC 4
#define C_AC 5

//#define PRINT_DHT
#define DOWNSCALE_DEN 8

#define DIV_CEILING(x,y) (((x)+(y)-1)/(y))

/*============================================================================
   STATIC CONSTANTS
============================================================================*/
/* this value should match the jpegd_scale_type_t enums*/
static const int g_step_sz[] = { 8, 1, 2, 3, 4, 5, 6, 7 };

static const int g_hstep_sz_v2[] =
{ 128, 128, 128, 128, 128, 128, 128, 128 };


static const float g_hstep_factor_v1[][JPEGD_FORMAT_MAX] = {
     // H2V2 H2V1  H1V2  H1V1   MON
    {  2,    2,    1,    1,     1, },     // plane 0
    {  2,    2,    2,    2,     2, },     // plane 1
    {  1,    1,    1,    1,     1, }      // plane 2
};

static const float g_hstep_factor_v2[][JPEGD_FORMAT_MAX] = {
    // H2V2  H2V1  H1V2  H1V1   MON
    {  1,    2,    1,    1,     2,   },     // plane 0
    {  1,    2,    2,    2,     2,   },     // plane 1
    {  0.5,  0.5,  0.5,  0.5,   0.5, }      // plane 2
};

static const huff_table_t g_StandardLumaDCHuffTable = {

  /* bits (0-based) */

  { 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },

  /* values */

  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },

};

static const huff_table_t g_StandardChromaDCHuffTable = {

  /* bits (0-based) */

  { 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },

  /* values */

  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },

};

static const huff_table_t g_StandardLumaACHuffTable = {

  /* bits (0-based) */

  { 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d },

  /* values */

  { 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,

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

    0xf9, 0xfa },

};


static const huff_table_t g_StandardChromaACHuffTable = {

  /* bits (0-based) */

  { 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 },

  /* values */

  { 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,

    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,

    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,

    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,

    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,

    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,

    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,

    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,

    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,

    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,

    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,

    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,

    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,

    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,

    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,

    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,

    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,

    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,

    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,

    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,

    0xf9, 0xfa },

};

/*===========================================================================
 * FUNCTION    - jpegd_hw_cmd_malloc_and_copy  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static struct msm_jpeg_hw_cmds* jpegd_hw_cmd_malloc_and_copy(
  uint16_t size,
  struct msm_jpeg_hw_cmd* p_hw_cmd)
{
  struct msm_jpeg_hw_cmds* p_hw_cmds;

  p_hw_cmds = malloc(sizeof(struct msm_jpeg_hw_cmds) -
    sizeof(struct msm_jpeg_hw_cmd) + size);

  if (p_hw_cmds) {
    p_hw_cmds->m = size / sizeof(struct msm_jpeg_hw_cmd);
    if (p_hw_cmd) {
      memcpy(p_hw_cmds->hw_cmd, p_hw_cmd, size);
    }
  }
  return p_hw_cmds;
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_reset  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_reset(int fd)
{
  struct msm_jpeg_hw_cmd hw_cmd;
  int rc;
  uint32_t val = 0;

  JDDBG_MED("%s:%d]", __func__, __LINE__);

  MEM_OUTF2(&val, RESET_CMD, CORE_RESET, WE_RESET, 1, 1);

  jpegd_write(RESET_CMD, val);
  rc = ioctl(fd, MSM_JPEG_IOCTL_RESET, &hw_cmd);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    return rc;
  }
  JDDBG_MED("%s:%d] X", __func__, __LINE__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_reset  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_core_cfg(int fd, int scale_enable)
{
  struct msm_jpeg_hw_cmd hw_cmd;
  int rc;
  uint32_t val = 0;

  JDDBG_HIGH("%s:%d]: scale_enable %d", __func__, __LINE__,
    scale_enable);

  MEM_OUTF3(&val, CORE_CFG, MODE, INLINE_AUTO_RESYNC_EN, TESTBUS_ENABLE,
    JPEG_CORE_CFG__MODE__JPEG_DECODE, 0, 0);
  MEM_OUTF3(&val, CORE_CFG, BLOCK_FORMATTER_OUTPUT, SCALE_INPUT_SEL,
    SCALE_ENABLE, 0, 1, scale_enable);
  /* Always enable the block formatter*/
  MEM_OUTF3(&val, CORE_CFG, BLOCK_FORMATTER_ENABLE, JPEG_DECODE_ENABLE,
    JPEG_ENCODE_ENABLE, 1, 1, 0);
  MEM_OUTF3(&val, CORE_CFG, BRIDGE_ENABLE, INLINE_INTERFACE_ENABLE,
    WE_ENABLE,
    JPEG_CORE_CFG__BRIDGE_ENABLE__BRIDGE_INTERFACE_ENABLED, 0,
    JPEG_CORE_CFG__WE_ENABLE__WRITE_ENGINE_ENABLED);
  MEM_OUTF(&val, CORE_CFG, FE_ENABLE,
    JPEG_CORE_CFG__FE_ENABLE__FETCH_ENGINE_ENABLED);
  jpegd_write(CORE_CFG, val);
  rc = ioctl(fd, MSM_JPEG_IOCTL_HW_CMD, &hw_cmd);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    return rc;
  }
  JDDBG_MED("%s:%d]:X", __func__, __LINE__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_fe_cfg  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_fe_cfg(int fd, jpegd_cmd_fe_cfg *fe_cfg)
{
  struct msm_jpeg_hw_cmd hw_cmd;
  int rc = 0;
  uint32_t val = 0;
  JDDBG_MED("%s:%d]", __func__, __LINE__);

  MEM_OUTF3(&val, FE_CFG, H_FLIP, ROTATION, MAL_EN, fe_cfg->flip,
    fe_cfg->rotation, fe_cfg->mal_en);
  MEM_OUTF3(&val, FE_CFG, MAL_BOUNDARY, PLN2_EN, PLN1_EN,
    fe_cfg->mal_boundary,
    fe_cfg->plane2_enable, fe_cfg->plane1_enable);
  MEM_OUTF3(&val, FE_CFG, PLN0_EN, RD_SRC, SWC_FETCH_EN,
    fe_cfg->plane0_enable,
    fe_cfg->read_enable, fe_cfg->swc_fetch_enable);

  MEM_OUTF3(&val, FE_CFG, BOTTOM_VPAD_EN, CBCR_ORDER, MEMORY_FORMAT,
    fe_cfg->bottom_vpad_enable, 1,
    fe_cfg->memory_format);
  MEM_OUTF2(&val, FE_CFG, BURST_LENGTH_MAX, BYTE_ORDERING,
    fe_cfg->burst_length_max, fe_cfg->byte_ordering);
  jpegd_write(FE_CFG, val);

  rc = ioctl(fd, MSM_JPEG_IOCTL_HW_CMD, &hw_cmd);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    return rc;
  }

  rc = jpegd_hw_fe_qos_cfg(fd);
  if (rc) {
    JDDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  JDDBG_MED("%s:%d] X", __func__, __LINE__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_input_len_cfg  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_input_len_cfg(int fd, int input_len)
{
  struct msm_jpeg_hw_cmd hw_cmd;
  int rc;
  uint32_t val = 0;

  jpegd_write(PLN0_RD_BUFFER_SIZE, input_len);
  rc = ioctl(fd, MSM_JPEG_IOCTL_HW_CMD, &hw_cmd);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    return rc;
  }
  JDDBG_MED("%s:%d] X", __func__, __LINE__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_fe_qos_cfg  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_fe_qos_cfg(int fd)
{
  struct msm_jpeg_hw_cmds* p_hw_cmds;
  struct msm_jpeg_hw_cmd* p_hw_cmd;
  int rc = 0;

  JDDBG_MED("%s:%d]", __func__, __LINE__);

  p_hw_cmds =
    jpegd_hw_cmd_malloc_and_copy(sizeof(struct msm_jpeg_hw_cmd) *
      4,
      NULL);
  if (!p_hw_cmds) {
    JDDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return -ENOMEM;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  p_jpegd_writes(FE_QOS_CFG, 0x5508);
  p_jpegd_writes(FE_QOS_LAT_HIGH, 0x0);
  p_jpegd_writes(FE_QOS_LAT_MID, 0x0);
  p_jpegd_writes(FE_QOS_LAT_LOW, 0x0);

  rc = ioctl(fd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    free(p_hw_cmds);
    return rc;
  }

  free(p_hw_cmds);
  JDDBG_MED("%s:%d] X", __func__, __LINE__);
  return 0;
}


/*===========================================================================
 * FUNCTION    - jpegd_hw_we_cfg  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_we_cfg(
  int fd,
  jpegd_cmd_we_cfg *we_cfg,
  jpegd_image_info_t *write_config,
  jpegd_scale_type_t scale_type)
{
  struct msm_jpeg_hw_cmds* p_hw_cmds;
  struct msm_jpeg_hw_cmd* p_hw_cmd;
  uint32_t plane0_hstep, plane1_hstep, plane2_hstep;
  uint32_t plane0_vstep, plane1_vstep, plane2_vstep;
  int rc = 0;
  uint32_t val;
  uint16_t image_width = write_config->image_width;
  uint16_t image_height = write_config->image_height;
  uint16_t actual_width = write_config->actual_width;
  float pl_factor_w[3] = {1.0, 1.0, 1.0};
  float pl_factor_h[3] = {1.0, 1.0, 1.0};
  int use_pln2 = 0;
  const int *l_hstep_sz;
  const float (*l_step_fact)[5];
  struct msm_jpeg_hw_cmd hw_cmd;
  uint32_t version;
  uint32_t num_blocks_height, num_blocks_width;
  uint32_t stride;

  if (we_cfg->memory_format == 0x0)
    use_pln2 = 1;

  JDDBG_MED("%s:%d] scale_type %d use_pln2 %d", __func__, __LINE__,
    scale_type, use_pln2);

  if (scale_type > SCALE_MAX) {
    JDDBG_ERROR("%s:%d] Invalid scale_type %d", __func__, __LINE__, scale_type);
    return -EINVAL;
  }

  JDDBG_MED("%s:%d] orig dim %dx%d", __func__, __LINE__,
    image_width, image_height);
  image_width = (image_width * g_step_sz[scale_type] / DOWNSCALE_DEN);
  image_height = (image_height * g_step_sz[scale_type] / DOWNSCALE_DEN);
  JDDBG_HIGH("%s:%d] new dim %dx%d", __func__, __LINE__,
    image_width, image_height);

  p_hw_cmds =
    jpegd_hw_cmd_malloc_and_copy(sizeof(struct msm_jpeg_hw_cmd) *
    24,
    NULL);
  if (!p_hw_cmds) {
    JDDBG_ERROR("%s:%d] Error no memory", __func__, __LINE__);
    return -ENOMEM;
  }

  p_hw_cmd = &p_hw_cmds->hw_cmd[0];
  val = 0;
  MEM_OUTF3(&val, WE_CFG, H_FLIP, ROTATION, POP_BUFF_ON_EOS, we_cfg->flip,
    we_cfg->rotation, we_cfg->pop_buff_on_eos);
  MEM_OUTF3(&val, WE_CFG, MAL_EN, MAL_BOUNDARY, PLN2_EN, we_cfg->mal_enable,
    we_cfg->mal_boundary, we_cfg->pln2_enable);
  MEM_OUTF3(&val, WE_CFG, PLN1_EN, PLN0_EN, CBCR_ORDER, we_cfg->pln1_enable,
    we_cfg->pln0_enable, we_cfg->cbcr_order);
  MEM_OUTF3(&val, WE_CFG, MEMORY_FORMAT, CBCR_ORDER, BURST_LENGTH_MAX,
    we_cfg->memory_format, we_cfg->cbcr_order,
    we_cfg->burst_length_max);
  MEM_OUTF(&val, WE_CFG, BYTE_ORDERING, we_cfg->byte_ordering);
  MEM_OUTF(&val, WE_CFG, ROTATION, we_cfg->rotation);
  MEM_OUTF(&val, WE_CFG, PLN2_EN, we_cfg->pln2_enable);

  p_jpegd_writes(WE_CFG, val);

  p_jpegd_writes(WE_PATH_CFG, 0x24);
  p_jpegd_writes(WE_QOS_CFG, 0x5555);

  jpegd_lib_hw_get_version(&hw_cmd);
  rc = ioctl (fd, MSM_JPEG_IOCTL_GET_HW_VERSION, &hw_cmd);
  JDDBG_MED("%s:%d] result %d", __func__, __LINE__, rc);
  if (rc)
    return rc;

  version = hw_cmd.data;

  if (IS_8974_V2(version)) {
    l_hstep_sz = g_hstep_sz_v2;
    l_step_fact = &g_hstep_factor_v2[0];
  } else {
    l_hstep_sz = g_step_sz;
    l_step_fact = &g_hstep_factor_v1[0];
  }

  plane0_hstep = l_hstep_sz[scale_type] *
      l_step_fact[JPEGD_PLN_0][we_cfg->jpegdFormat];
  plane1_hstep = l_hstep_sz[scale_type] *
      l_step_fact[JPEGD_PLN_1][we_cfg->jpegdFormat];
  plane2_hstep = l_hstep_sz[scale_type] *
      l_step_fact[JPEGD_PLN_2][we_cfg->jpegdFormat];

  switch (we_cfg->jpegdFormat) {
    case JPEGD_H1V1:
      plane0_vstep = g_step_sz[scale_type];
      plane1_vstep = g_step_sz[scale_type];
      plane2_vstep = g_step_sz[scale_type];
      pl_factor_w[1] = 2.0;
      stride = CEILING8(image_width);
      break;
    case JPEGD_H2V1:
      plane0_vstep = g_step_sz[scale_type];
      plane1_vstep = g_step_sz[scale_type];
      plane2_vstep = g_step_sz[scale_type];
      stride = CEILING16(image_width);
      break;
    case JPEGD_H1V2:
      plane0_vstep = g_step_sz[scale_type] * 2;
      plane1_vstep = g_step_sz[scale_type];
      plane2_vstep = g_step_sz[scale_type];
      pl_factor_w[1] = 2.0;
      pl_factor_h[1] = 0.5;
      stride = CEILING8(image_width);
      break;
    case JPEGD_GRAYSCALE:
      plane0_vstep = g_step_sz[scale_type];
      plane1_vstep = g_step_sz[scale_type];
      plane2_vstep = g_step_sz[scale_type];
      pl_factor_w[1] = 0.0;
      pl_factor_h[1] = 0.0;
      pl_factor_w[2] = 0.0;
      pl_factor_h[2] = 0.0;
      stride = CEILING8(image_width);
      break;
    default:
    case JPEGD_H2V2:
      plane0_vstep = g_step_sz[scale_type] * 2;
      plane1_vstep = g_step_sz[scale_type];
      plane2_vstep = g_step_sz[scale_type];
      pl_factor_h[1] = 0.5;
      stride = CEILING16(image_width);
      break;
  }

  if (use_pln2) {
    plane1_hstep >>= 1;
    pl_factor_w[1] /= 2;
    pl_factor_w[2] = pl_factor_w[1];
    pl_factor_h[2] = pl_factor_h[1];
  }


  val = 0;
  MEM_OUTF2(&val, PLN0_WR_BUFFER_SIZE, WIDTH, HEIGHT,
    image_width,
    image_height);

  p_jpegd_writes(PLN0_WR_BUFFER_SIZE, val);
  val = 0;
  MEM_OUTF2(&val, PLN1_WR_BUFFER_SIZE, WIDTH, HEIGHT,
    (image_width * pl_factor_w[1]),
    (image_height * pl_factor_h[1]));

  p_jpegd_writes(PLN1_WR_BUFFER_SIZE, val);

  val = 0;
  MEM_OUTF2(&val, PLN2_WR_BUFFER_SIZE, WIDTH, HEIGHT,
    (image_width * pl_factor_w[2]),
    (image_height * pl_factor_h[2]));
  p_jpegd_writes(PLN2_WR_BUFFER_SIZE, val);

  p_jpegd_writes(PLN0_WR_STRIDE, stride);
  p_jpegd_writes(PLN1_WR_STRIDE, (stride * pl_factor_w[1]));
  p_jpegd_writes(PLN2_WR_STRIDE, (stride * pl_factor_w[2]));

  p_jpegd_writes(PLN1_WR_HINIT, 0);
  p_jpegd_writes(PLN2_WR_HINIT, 0);
  p_jpegd_writes(PLN0_WR_HINIT, 0);
  p_jpegd_writes(PLN0_WR_VINIT, 0);
  p_jpegd_writes(PLN1_WR_VINIT, 0);
  p_jpegd_writes(PLN2_WR_VINIT, 0);

  p_jpegd_writes(PLN0_WR_HSTEP, plane0_hstep);
  p_jpegd_writes(PLN0_WR_VSTEP, plane0_vstep);
  p_jpegd_writes(PLN1_WR_HSTEP, plane1_hstep);
  p_jpegd_writes(PLN1_WR_VSTEP, plane1_vstep);
  p_jpegd_writes(PLN2_WR_HSTEP, plane2_hstep);
  p_jpegd_writes(PLN2_WR_VSTEP, plane2_vstep);

  num_blocks_height = DIV_CEILING(image_height, plane0_vstep);
  num_blocks_width = DIV_CEILING(image_width, plane0_hstep);

  val = 0;
  MEM_OUTF2(&val, PLN0_WR_BLOCK_CFG, BLOCKS_PER_COL, BLOCKS_PER_ROW,
    (num_blocks_height - 1),
    (num_blocks_width  - 1));

   JDDBG_MED("%s,%d, val =0x%x",__func__, __LINE__,val);
   p_jpegd_writes(PLN0_WR_BLOCK_CFG, val);

   num_blocks_height = (uint32_t) (image_height * pl_factor_h[1]);
   num_blocks_height = DIV_CEILING(num_blocks_height, plane1_vstep);

   num_blocks_width = (uint32_t) (image_width * pl_factor_w[1]);
   num_blocks_width = DIV_CEILING(num_blocks_width, plane1_hstep);

  val = 0;
  MEM_OUTF2(&val, PLN1_WR_BLOCK_CFG, BLOCKS_PER_COL, BLOCKS_PER_ROW,
    (num_blocks_height - 1),
    (num_blocks_width  - 1));

  p_jpegd_writes(PLN1_WR_BLOCK_CFG, val);

  num_blocks_height = (uint32_t) (image_height * pl_factor_h[2]);
  num_blocks_height = DIV_CEILING(num_blocks_height, plane2_vstep);

  num_blocks_width = (uint32_t) (image_width * pl_factor_w[2]);
  num_blocks_width = DIV_CEILING(num_blocks_width, plane2_hstep);

  val = 0;
  MEM_OUTF2(&val, PLN2_WR_BLOCK_CFG, BLOCKS_PER_COL, BLOCKS_PER_ROW,
    (num_blocks_height - 1),
    (num_blocks_width  - 1));

  p_jpegd_writes(PLN2_WR_BLOCK_CFG, val);

  rc = ioctl(fd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    free(p_hw_cmds);
    return rc;
  }

  free(p_hw_cmds);
  JDDBG_MED("%s:%d] :X", __func__, __LINE__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_decode_cfg  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_decode_cfg(
  int fd,
  uint32_t decode_mode,
  uint32_t height,
  uint32_t width,
  jpegd_subsampling_t sampling_factor,
  uint32_t restart_interval)
{
  struct msm_jpeg_hw_cmds* p_hw_cmds;
  struct msm_jpeg_hw_cmd* p_hw_cmd;

  int rc = 0;
  uint32_t val = 0;
  uint32_t hBlockSize;
  uint32_t vBlockSize;
  uint32_t image_w_blocks;
  uint32_t image_h_blocks;
  uint32_t imageformat = 0;

  JDDBG_MED("%s:%d]", __func__, __LINE__);
  p_hw_cmds =
    jpegd_hw_cmd_malloc_and_copy(sizeof(struct msm_jpeg_hw_cmd) *
      5,
      NULL);
  if (!p_hw_cmds) {
    return 1;
  }

  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  switch (sampling_factor) {
    case JPEGD_H2V1: /*h2v1*/
      hBlockSize = 16;
      vBlockSize = 8;
      imageformat = 0x2;
      break;
    case JPEGD_H1V2: /*h1v2*/
      hBlockSize = 8;
      vBlockSize = 16;
      imageformat = 0x1;
      break;
    case JPEGD_H1V1:
      hBlockSize = 8;
      vBlockSize = 8;
      imageformat = 0x0;
      break;
    case JPEGD_GRAYSCALE: /*mono*/
      hBlockSize = 8;
      vBlockSize = 8;
      imageformat = 0x4;
      break;
    default:
    case JPEGD_H2V2: /*h2v2*/
      hBlockSize = 16;
      vBlockSize = 16;
      imageformat = 0x3;
      break;
  }

  MEM_OUTF2(&val, DECODE_CFG, IMAGE_FORMAT, DECODE_MODE, imageformat,
    decode_mode);
  /*Setting restart marker insertion is deleted by setting to 0*/
  MEM_OUTF(&val, DECODE_CFG, RST_MARKER_PERIOD, restart_interval);

  p_jpegd_writes(DECODE_CFG, val);

  image_w_blocks = DIV_CEILING(width, hBlockSize) - 1;
  image_h_blocks = DIV_CEILING(height, vBlockSize) - 1;
  val = 0;
  MEM_OUTF2(&val, DECODE_IMAGE_SIZE, DECODE_IMAGE_HEIGHT,
    DECODE_IMAGE_WIDTH,
    image_h_blocks, image_w_blocks);
  p_jpegd_writes(DECODE_IMAGE_SIZE, val);

  val = 0;
  /*Defaulting the crop to 0 for now.*/
  MEM_OUTF2(&val, DECODE_CROP_START, DECODE_CROP_START_V,
    DECODE_CROP_START_H,
    0, 0);
  p_jpegd_writes(DECODE_CROP_START, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_CROP_STOP, DECODE_CROP_STOP_V,
    DECODE_CROP_STOP_H,
    image_h_blocks, image_w_blocks);
  p_jpegd_writes(DECODE_CROP_STOP, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_STOP, DECODE_STOP_V, DECODE_STOP_H,
    image_h_blocks,
    image_w_blocks);
  p_jpegd_writes(DECODE_STOP, val);

  rc = ioctl(fd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    free(p_hw_cmds);
    return rc;
  }

  free(p_hw_cmds);
  JDDBG_MED("%s:%d] X", __func__, __LINE__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_decode_cfg  -
 *
 * DESCRIPTION:
 *==========================================================================*/
void jpegd_hw_printhuffman_helper(huff_table_t *tbl)
{
  int j = 0, i = 0;
  JDDBG_LOW("huff_cfg.p_htables.bits = {");
  for (i=0; i<16; i+=8) {
    JDDBG_LOW("%d %d %d %d %d %d %d %d",
      tbl->bits[i],
      tbl->bits[i+1],
      tbl->bits[i+2],
      tbl->bits[i+3],
      tbl->bits[i+4],
      tbl->bits[i+5],
      tbl->bits[i+6],
      tbl->bits[i+7]);
  }
  JDDBG_LOW(" %d", tbl->bits[16]);
  JDDBG_LOW("}");
  JDDBG_LOW("huff_cfg.p_htables.values = {");
  for (i=0; i<256; i+=8) {
    JDDBG_LOW("%d %d %d %d %d %d %d %d",
      tbl->values[i],
      tbl->values[i+1],
      tbl->values[i+2],
      tbl->values[i+3],
      tbl->values[i+4],
      tbl->values[i+5],
      tbl->values[i+6],
      tbl->values[i+7]);
  }
  JDDBG_LOW("}");
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_dht_code_config  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_dht_code_config(
  int fd,
  jpegd_cmd_huff_cfg_t *huff_cfg)
{
  int codeWord = 0;
  int i, j;
  int* huffBits, *huffVal;
  int ccc[17];
  int mincode[17];
  uint8_t bytevalue;
  uint16_t ac_cnt, dc_cnt;
  struct msm_jpeg_hw_cmds* p_hw_cmds;
  struct msm_jpeg_hw_cmd* p_hw_cmd;
  uint32_t val = 0;
  int rc = 0;
  JDDBG_MED("%s:%d]", __func__, __LINE__);

  p_hw_cmds =
    jpegd_hw_cmd_malloc_and_copy(sizeof(struct msm_jpeg_hw_cmd) *
      48,
      NULL);
  if (!p_hw_cmds) {
    JDDBG_ERROR("%s:%d] Error no mem", __func__, __LINE__);
    return -ENOMEM;
  }

  /* DC Y */
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];
  bytevalue = huff_cfg->htable_present_flag;
  i = (bytevalue >> 4);
  j = (bytevalue & 0xF);

  for (ac_cnt = 0; i > 0; i >>= 1) {
    ac_cnt++;
  }

  for (dc_cnt = 0; j > 0; j >>= 1) {
    dc_cnt++;
  }

#ifdef PRINT_DHT
  jpegd_hw_printhuffman_helper(&g_StandardLumaDCHuffTable);
  jpegd_hw_printhuffman_helper(&g_StandardChromaDCHuffTable);
  jpegd_hw_printhuffman_helper(&g_StandardLumaACHuffTable);
  jpegd_hw_printhuffman_helper(&g_StandardChromaACHuffTable);
  jpegd_hw_printhuffman_helper(&huff_cfg->p_htables[Y_DC]);
  jpegd_hw_printhuffman_helper(&huff_cfg->p_htables[C_DC]);
  jpegd_hw_printhuffman_helper(&huff_cfg->p_htables[Y_AC]);
  jpegd_hw_printhuffman_helper(&huff_cfg->p_htables[C_AC]);
#endif

#if USE_STD_DHT_TBL
  huffBits = g_StandardLumaDCHuffTable.bits;
  huffVal = g_StandardLumaDCHuffTable.values;
#else
  huffBits = huff_cfg->p_htables[Y_DC].bits;
  huffVal  = huff_cfg->p_htables[Y_DC].values;
#endif
  ccc[0] = 0;
  codeWord = 0;
  mincode[16] = 0xffff;
  for (i = 0; i < 16; i++) {
    ccc[i + 1] = ccc[i] + huffBits[i];
    if (huffBits[i]) {
      mincode[i] = codeWord << (15 - i);
    }
    codeWord += huffBits[i];
    codeWord <<= 1;
  }

  for (i = 15; i >= 0; i--) {
    if (huffBits[i] == 0) {
      mincode[i] = mincode[i + 1];
    }
  }

  for (i = 1; i <= 16; i++) {
    JDDBG_LOW("%s:%d] DC_Y: ccc[%d]=0x%08X", __func__, __LINE__, i,
      ccc[i]);
  }

  val = 0;
  MEM_OUTF3(&val, DECODE_DC_Y_CUMULATIVE_CODE_COUNT_0, CCC_LENGTH0,
    CCC_LENGTH1,
    CCC_LENGTH2, ccc[1], ccc[2], ccc[3]);

  MEM_OUTF(&val, DECODE_DC_Y_CUMULATIVE_CODE_COUNT_0,  CCC_LENGTH3,
    ccc[4]);

  p_jpegd_writes(DECODE_DC_Y_CUMULATIVE_CODE_COUNT_0, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_DC_Y_CUMULATIVE_CODE_COUNT_1, CCC_LENGTH4,
    CCC_LENGTH5,
    CCC_LENGTH6, ccc[5], ccc[6], ccc[7]);
  MEM_OUTF(&val, DECODE_DC_Y_CUMULATIVE_CODE_COUNT_1,  CCC_LENGTH7,
    ccc[8]);
  p_jpegd_writes(DECODE_DC_Y_CUMULATIVE_CODE_COUNT_1, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_DC_Y_CUMULATIVE_CODE_COUNT_2, CCC_LENGTH8,
    CCC_LENGTH9,
    CCC_LENGTH10, ccc[9], ccc[10], ccc[11]);
  MEM_OUTF(&val, DECODE_DC_Y_CUMULATIVE_CODE_COUNT_2,  CCC_LENGTH11,
    ccc[12]);
  p_jpegd_writes(DECODE_DC_Y_CUMULATIVE_CODE_COUNT_2, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_DC_Y_CUMULATIVE_CODE_COUNT_3, CCC_LENGTH12,
    CCC_LENGTH13, CCC_LENGTH14, ccc[13], ccc[14], ccc[15]);
  MEM_OUTF(&val, DECODE_DC_Y_CUMULATIVE_CODE_COUNT_3,  CCC_LENGTH15,
    ccc[16]);
  p_jpegd_writes(DECODE_DC_Y_CUMULATIVE_CODE_COUNT_3, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_Y_MIN_CODE_WORD_0, MIN_CODE_WORD_LENGTH0,
    MIN_CODE_WORD_LENGTH1, mincode[0], mincode[1]);
  p_jpegd_writes(DECODE_DC_Y_MIN_CODE_WORD_0, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_Y_MIN_CODE_WORD_1, MIN_CODE_WORD_LENGTH2,
    MIN_CODE_WORD_LENGTH3, mincode[2], mincode[3]);
  p_jpegd_writes(DECODE_DC_Y_MIN_CODE_WORD_1, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_Y_MIN_CODE_WORD_2, MIN_CODE_WORD_LENGTH4,
    MIN_CODE_WORD_LENGTH5, mincode[4], mincode[5]);
  p_jpegd_writes(DECODE_DC_Y_MIN_CODE_WORD_2, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_Y_MIN_CODE_WORD_3, MIN_CODE_WORD_LENGTH6,
    MIN_CODE_WORD_LENGTH7, mincode[6], mincode[7]);
  p_jpegd_writes(DECODE_DC_Y_MIN_CODE_WORD_3, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_Y_MIN_CODE_WORD_4, MIN_CODE_WORD_LENGTH8,
    MIN_CODE_WORD_LENGTH9, mincode[8], mincode[9]);
  p_jpegd_writes(DECODE_DC_Y_MIN_CODE_WORD_4, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_DC_Y_MIN_CODE_WORD_5, MIN_CODE_WORD_LENGTH10,
    MIN_CODE_WORD_LENGTH11, mincode[10], mincode[11]);
  p_jpegd_writes(DECODE_DC_Y_MIN_CODE_WORD_5, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_Y_MIN_CODE_WORD_6, MIN_CODE_WORD_LENGTH12,
    MIN_CODE_WORD_LENGTH13, mincode[12], mincode[13]);
  p_jpegd_writes(DECODE_DC_Y_MIN_CODE_WORD_6, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_Y_MIN_CODE_WORD_7, MIN_CODE_WORD_LENGTH14,
    MIN_CODE_WORD_LENGTH15, mincode[14], mincode[15]);
  p_jpegd_writes(DECODE_DC_Y_MIN_CODE_WORD_7, val);

  /* DC CbCr */
#if USE_STD_DHT_TBL
  huffBits = g_StandardChromaDCHuffTable.bits;
  huffVal = g_StandardChromaDCHuffTable.values;
#else
  huffBits = huff_cfg->p_htables[C_DC].bits;
  huffVal = huff_cfg->p_htables[C_DC].values;
#endif
  ccc[0] = 0;
  codeWord = 0;
  mincode[16] = 0xffff;
  for (i = 0; i < 16; i++) {
    ccc[i + 1] = ccc[i] + huffBits[i];
    if (huffBits[i]) {
      mincode[i] = codeWord << (15 - i);
    }
    codeWord += huffBits[i];
    codeWord <<= 1;
  }

  for (i = 15; i >= 0; i--) {
    if (huffBits[i] == 0) {
      mincode[i] = mincode[i + 1];
    }
  }

  val = 0;
  MEM_OUTF3(&val, DECODE_DC_C_CUMULATIVE_CODE_COUNT_0, CCC_LENGTH0,
    CCC_LENGTH1, CCC_LENGTH2, ccc[1], ccc[2], ccc[3]);
  MEM_OUTF(&val, DECODE_DC_C_CUMULATIVE_CODE_COUNT_0,  CCC_LENGTH3,
    ccc[4]);
  p_jpegd_writes(DECODE_DC_C_CUMULATIVE_CODE_COUNT_0, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_DC_C_CUMULATIVE_CODE_COUNT_1, CCC_LENGTH4,
    CCC_LENGTH5, CCC_LENGTH6, ccc[5], ccc[6], ccc[7]);
  MEM_OUTF(&val, DECODE_DC_C_CUMULATIVE_CODE_COUNT_1,  CCC_LENGTH7,
    ccc[8]);
  p_jpegd_writes(DECODE_DC_C_CUMULATIVE_CODE_COUNT_1, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_DC_C_CUMULATIVE_CODE_COUNT_2, CCC_LENGTH8,
    CCC_LENGTH9, CCC_LENGTH10, ccc[9], ccc[10], ccc[11]);
  MEM_OUTF(&val, DECODE_DC_C_CUMULATIVE_CODE_COUNT_2,  CCC_LENGTH11,
    ccc[12]);
  p_jpegd_writes(DECODE_DC_C_CUMULATIVE_CODE_COUNT_2, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_DC_C_CUMULATIVE_CODE_COUNT_3, CCC_LENGTH12,
    CCC_LENGTH13, CCC_LENGTH14, ccc[13], ccc[14], ccc[15]);
  MEM_OUTF(&val, DECODE_DC_C_CUMULATIVE_CODE_COUNT_3,  CCC_LENGTH15,
    ccc[16]);
  p_jpegd_writes(DECODE_DC_C_CUMULATIVE_CODE_COUNT_3, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_C_MIN_CODE_WORD_0, MIN_CODE_WORD_LENGTH0,
    MIN_CODE_WORD_LENGTH1, mincode[0], mincode[1]);
  p_jpegd_writes(DECODE_DC_C_MIN_CODE_WORD_0, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_C_MIN_CODE_WORD_1, MIN_CODE_WORD_LENGTH2,
    MIN_CODE_WORD_LENGTH3, mincode[2], mincode[3]);
  p_jpegd_writes(DECODE_DC_C_MIN_CODE_WORD_1, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_C_MIN_CODE_WORD_2, MIN_CODE_WORD_LENGTH4,
    MIN_CODE_WORD_LENGTH5, mincode[4], mincode[5]);
  p_jpegd_writes(DECODE_DC_C_MIN_CODE_WORD_2, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_C_MIN_CODE_WORD_3, MIN_CODE_WORD_LENGTH6,
    MIN_CODE_WORD_LENGTH7, mincode[6], mincode[7]);
  p_jpegd_writes(DECODE_DC_C_MIN_CODE_WORD_3, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_DC_C_MIN_CODE_WORD_4, MIN_CODE_WORD_LENGTH8,
    MIN_CODE_WORD_LENGTH9, mincode[8], mincode[9]);
  p_jpegd_writes(DECODE_DC_C_MIN_CODE_WORD_4, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_DC_C_MIN_CODE_WORD_5, MIN_CODE_WORD_LENGTH10,
    MIN_CODE_WORD_LENGTH11, mincode[10], mincode[11]);
  p_jpegd_writes(DECODE_DC_C_MIN_CODE_WORD_5, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_DC_C_MIN_CODE_WORD_6, MIN_CODE_WORD_LENGTH12,
    MIN_CODE_WORD_LENGTH13, mincode[12], mincode[13]);
  p_jpegd_writes(DECODE_DC_C_MIN_CODE_WORD_6, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_DC_C_MIN_CODE_WORD_7, MIN_CODE_WORD_LENGTH14,
    MIN_CODE_WORD_LENGTH15, mincode[14], mincode[15]);
  p_jpegd_writes(DECODE_DC_C_MIN_CODE_WORD_7, val);

  /* AC Y */
#if USE_STD_DHT_TBL
  huffBits = g_StandardLumaACHuffTable.bits;
  huffVal = g_StandardLumaACHuffTable.values;
#else

  huffBits = huff_cfg->p_htables[Y_AC].bits;
  huffVal = huff_cfg->p_htables[Y_AC].values;
#endif
  ccc[0] = 0;
  codeWord = 0;
  mincode[16] = 0xffff;
  for (i = 0; i < 16; i++) {
    ccc[i + 1] = ccc[i] + huffBits[i];
    if (huffBits[i]) {
      mincode[i] = codeWord << (15 - i);
    }
    codeWord += huffBits[i];
    codeWord <<= 1;
  }

  for (i = 15; i >= 0; i--) {
    if (huffBits[i] == 0) {
      mincode[i] = mincode[i + 1];
    }
  }

  val = 0;
  MEM_OUTF3(&val, DECODE_AC_Y_CUMULATIVE_CODE_COUNT_0, CCC_LENGTH0,
    CCC_LENGTH1, CCC_LENGTH2, ccc[1], ccc[2], ccc[3]);
  MEM_OUTF(&val, DECODE_AC_Y_CUMULATIVE_CODE_COUNT_0,  CCC_LENGTH3,
    ccc[4]);
  p_jpegd_writes(DECODE_AC_Y_CUMULATIVE_CODE_COUNT_0, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_AC_Y_CUMULATIVE_CODE_COUNT_1, CCC_LENGTH4,
    CCC_LENGTH5, CCC_LENGTH6, ccc[5], ccc[6], ccc[7]);
  MEM_OUTF(&val, DECODE_AC_Y_CUMULATIVE_CODE_COUNT_1,  CCC_LENGTH7,
    ccc[8]);
  p_jpegd_writes(DECODE_AC_Y_CUMULATIVE_CODE_COUNT_1, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_AC_Y_CUMULATIVE_CODE_COUNT_2, CCC_LENGTH8,
    CCC_LENGTH9, CCC_LENGTH10, ccc[9], ccc[10], ccc[11]);
  MEM_OUTF(&val, DECODE_AC_Y_CUMULATIVE_CODE_COUNT_2,  CCC_LENGTH11,
    ccc[12]);
  p_jpegd_writes(DECODE_AC_Y_CUMULATIVE_CODE_COUNT_2, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_AC_Y_CUMULATIVE_CODE_COUNT_3, CCC_LENGTH12,
    CCC_LENGTH13, CCC_LENGTH14, ccc[13], ccc[14], ccc[15]);
  MEM_OUTF(&val, DECODE_AC_Y_CUMULATIVE_CODE_COUNT_3,  CCC_LENGTH15,
    ccc[16]);
  p_jpegd_writes(DECODE_AC_Y_CUMULATIVE_CODE_COUNT_3, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_AC_Y_MIN_CODE_WORD_0, MIN_CODE_WORD_LENGTH0,
    MIN_CODE_WORD_LENGTH1, mincode[0], mincode[1]);
  p_jpegd_writes(DECODE_AC_Y_MIN_CODE_WORD_0, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_AC_Y_MIN_CODE_WORD_1, MIN_CODE_WORD_LENGTH2,
    MIN_CODE_WORD_LENGTH3, mincode[2], mincode[3]);
  p_jpegd_writes(DECODE_AC_Y_MIN_CODE_WORD_1, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_AC_Y_MIN_CODE_WORD_2, MIN_CODE_WORD_LENGTH4,
    MIN_CODE_WORD_LENGTH5, mincode[4], mincode[5]);
  p_jpegd_writes(DECODE_AC_Y_MIN_CODE_WORD_2, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_AC_Y_MIN_CODE_WORD_3, MIN_CODE_WORD_LENGTH6,
    MIN_CODE_WORD_LENGTH7, mincode[6], mincode[7]);
  p_jpegd_writes(DECODE_AC_Y_MIN_CODE_WORD_3, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_AC_Y_MIN_CODE_WORD_4, MIN_CODE_WORD_LENGTH8,
    MIN_CODE_WORD_LENGTH9, mincode[8], mincode[9]);
  p_jpegd_writes(DECODE_AC_Y_MIN_CODE_WORD_4, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_AC_Y_MIN_CODE_WORD_5, MIN_CODE_WORD_LENGTH10,
    MIN_CODE_WORD_LENGTH11, mincode[10], mincode[11]);
  p_jpegd_writes(DECODE_AC_Y_MIN_CODE_WORD_5, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_AC_Y_MIN_CODE_WORD_6, MIN_CODE_WORD_LENGTH12,
    MIN_CODE_WORD_LENGTH13, mincode[12], mincode[13]);
  p_jpegd_writes(DECODE_AC_Y_MIN_CODE_WORD_6, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_AC_Y_MIN_CODE_WORD_7, MIN_CODE_WORD_LENGTH14,
    MIN_CODE_WORD_LENGTH15, mincode[14], mincode[15]);
  p_jpegd_writes(DECODE_AC_Y_MIN_CODE_WORD_7, val);

  /* AC CbCr */
#if USE_STD_DHT_TBL
  huffBits = g_StandardChromaACHuffTable.bits;
  huffVal = g_StandardChromaACHuffTable.values;
#else
  huffBits = huff_cfg->p_htables[C_AC].bits;
  huffVal = huff_cfg->p_htables[C_AC].values;
#endif
  ccc[0] = 0;
  codeWord = 0;
  mincode[16] = 0xffff;
  for (i = 0; i < 16; i++) {
    ccc[i + 1] = ccc[i] + huffBits[i];
    if (huffBits[i]) {
      mincode[i] = codeWord << (15 - i);
    }
    codeWord += huffBits[i];
    codeWord <<= 1;
  }

  for (i = 15; i >= 0; i--) {
    if (huffBits[i] == 0) {
      mincode[i] = mincode[i + 1];
    }
  }

  val = 0;
  MEM_OUTF3(&val, DECODE_AC_C_CUMULATIVE_CODE_COUNT_0, CCC_LENGTH0,
    CCC_LENGTH1, CCC_LENGTH2, ccc[1], ccc[2], ccc[3]);
  MEM_OUTF(&val, DECODE_AC_C_CUMULATIVE_CODE_COUNT_0,  CCC_LENGTH3,
    ccc[4]);
  p_jpegd_writes(DECODE_AC_C_CUMULATIVE_CODE_COUNT_0, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_AC_C_CUMULATIVE_CODE_COUNT_1, CCC_LENGTH4,
    CCC_LENGTH5, CCC_LENGTH6, ccc[5], ccc[6], ccc[7]);
  MEM_OUTF(&val, DECODE_AC_C_CUMULATIVE_CODE_COUNT_1,  CCC_LENGTH7,
    ccc[8]);
  p_jpegd_writes(DECODE_AC_C_CUMULATIVE_CODE_COUNT_1, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_AC_C_CUMULATIVE_CODE_COUNT_2, CCC_LENGTH8,
    CCC_LENGTH9, CCC_LENGTH10, ccc[9], ccc[10], ccc[11]);
  MEM_OUTF(&val, DECODE_AC_C_CUMULATIVE_CODE_COUNT_2,  CCC_LENGTH11,
    ccc[12]);
  p_jpegd_writes(DECODE_AC_C_CUMULATIVE_CODE_COUNT_2, val);

  val = 0;
  MEM_OUTF3(&val, DECODE_AC_C_CUMULATIVE_CODE_COUNT_3, CCC_LENGTH12,
    CCC_LENGTH13, CCC_LENGTH14, ccc[13], ccc[14], ccc[15]);
  MEM_OUTF(&val, DECODE_AC_C_CUMULATIVE_CODE_COUNT_3,  CCC_LENGTH15,
    ccc[16]);
  p_jpegd_writes(DECODE_AC_C_CUMULATIVE_CODE_COUNT_3, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_AC_C_MIN_CODE_WORD_0, MIN_CODE_WORD_LENGTH0,
    MIN_CODE_WORD_LENGTH1, mincode[0], mincode[1]);
  p_jpegd_writes(DECODE_AC_C_MIN_CODE_WORD_0, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_AC_C_MIN_CODE_WORD_1, MIN_CODE_WORD_LENGTH2,
    MIN_CODE_WORD_LENGTH3, mincode[2], mincode[3]);
  p_jpegd_writes(DECODE_AC_C_MIN_CODE_WORD_1, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_AC_C_MIN_CODE_WORD_2, MIN_CODE_WORD_LENGTH4,
    MIN_CODE_WORD_LENGTH5, mincode[4], mincode[5]);
  p_jpegd_writes(DECODE_AC_C_MIN_CODE_WORD_2, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_AC_C_MIN_CODE_WORD_3, MIN_CODE_WORD_LENGTH6,
    MIN_CODE_WORD_LENGTH7, mincode[6], mincode[7]);
  p_jpegd_writes(DECODE_AC_C_MIN_CODE_WORD_3, val);

  val = 0;
  MEM_OUTF2(&val, DECODE_AC_C_MIN_CODE_WORD_4, MIN_CODE_WORD_LENGTH8,
    MIN_CODE_WORD_LENGTH9, mincode[8], mincode[9]);
  p_jpegd_writes(DECODE_AC_C_MIN_CODE_WORD_4, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_AC_C_MIN_CODE_WORD_5, MIN_CODE_WORD_LENGTH10,
    MIN_CODE_WORD_LENGTH11, mincode[10], mincode[11]);
  p_jpegd_writes(DECODE_AC_C_MIN_CODE_WORD_5, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_AC_C_MIN_CODE_WORD_6, MIN_CODE_WORD_LENGTH12,
    MIN_CODE_WORD_LENGTH13, mincode[12], mincode[13]);
  p_jpegd_writes(DECODE_AC_C_MIN_CODE_WORD_6, val);


  val = 0;
  MEM_OUTF2(&val, DECODE_AC_C_MIN_CODE_WORD_7, MIN_CODE_WORD_LENGTH14,
    MIN_CODE_WORD_LENGTH15, mincode[14], mincode[15]);
  p_jpegd_writes(DECODE_AC_C_MIN_CODE_WORD_7, val);


  rc = ioctl(fd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    free(p_hw_cmds);
    return rc;
  }

  free(p_hw_cmds);
  JDDBG_MED("%s:%d]", __func__, __LINE__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_huff_config  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_huff_config(int fd, jpegd_cmd_huff_cfg_t *huff_cfg)
{
  uint32_t val = 0;
  struct msm_jpeg_hw_cmds* p_hw_cmds;
  struct msm_jpeg_hw_cmd* p_hw_cmd;
  int i;
  int rc = 0;
  const int huff_tbl_size = 162;

  JDDBG_MED("%s:%d]", __func__, __LINE__);
  p_hw_cmds =
    jpegd_hw_cmd_malloc_and_copy(sizeof(struct msm_jpeg_hw_cmd) *
    (1 + 1 + 12 + 12 + huff_tbl_size + huff_tbl_size + 1),
     NULL);
  if (!p_hw_cmds) {
    JDDBG_ERROR("%s:%d] Error nomem", __func__, __LINE__);
    return -ENOMEM;
  }

  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  /*set the DMI cfg to auto increment and select no memory*/
  val = 0;
  MEM_OUTF2(&val, DMI_CFG, AUTO_INC_EN, DMI_MEM_SEL, 1,
    JPEG_DMI_CFG__DMI_MEM_SEL__HUFFMAN_LUT);
  p_jpegd_writes(DMI_CFG, val);
  val = 0;
  p_jpegd_writes(DMI_ADDR, 0);


  /* Y DC  - 0 to 11 ie. Total = 12-1 */
  for (i = 0; i <= 11; i++) {
#if USE_STD_DHT_TBL
    p_jpegd_writes(DMI_DATA, g_StandardLumaDCHuffTable.values[i]);
#else
    p_jpegd_writes(DMI_DATA, huff_cfg->p_htables[Y_DC].values[i]);
#endif
  }

  /* CBCR DC - 12 to 23 ie. Total: 12-1 */
  for (i = 0; i <= 11; i++) {
#if USE_STD_DHT_TBL
    p_jpegd_writes(DMI_DATA, g_StandardChromaDCHuffTable.values[i]);
#else
    p_jpegd_writes(DMI_DATA, huff_cfg->p_htables[C_DC].values[i]);
#endif
  }

  /* Y AC - 24 to 12+12+162-1 ie. Total = 162-1  */
  for (i = 0; i < huff_tbl_size; i++) {
#if USE_STD_DHT_TBL
    p_jpegd_writes(DMI_DATA, g_StandardLumaACHuffTable.values[i]);
#else
    p_jpegd_writes(DMI_DATA, huff_cfg->p_htables[Y_AC].values[i]);
#endif
  }

  /* CBCR AC - 12+12+162 to 12+12+162+162 ie. Total = 162 -1 */
  for (i = 0; i < huff_tbl_size; i++) {
#if USE_STD_DHT_TBL
    p_jpegd_writes(DMI_DATA, g_StandardChromaACHuffTable.values[i]);
#else
    p_jpegd_writes(DMI_DATA, huff_cfg->p_htables[C_AC].values[i]);
#endif
  }

  val = 0;
  p_jpegd_writes(DMI_CFG, val);

  rc = ioctl(fd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    free(p_hw_cmds);
    return rc;
  }

  free(p_hw_cmds);
  JDDBG_MED("%s:%d] ", __func__, __LINE__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_jpeg_dqt  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_jpeg_dqt(int fd, jpegd_cmd_quant_cfg_t *dqt_cfg)
{
  int i;
  int cnt;
  int rc;
  uint16_t jpeg_dqt_qk;
  uint8_t jpeg_dqt_tq;
  uint32_t val;
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;
  uint8_t qtable_present_flag =  dqt_cfg->qtable_present_flag;

  JDDBG_MED("%s:%d", __func__,__LINE__);

  for (cnt=0; qtable_present_flag>0; qtable_present_flag >>= 1) {
    cnt++;
  }
  JDDBG_MED("%s:%d] Number of Quantization Tables = %d", __func__, __LINE__,
     cnt);

  p_hw_cmds =
    jpegd_hw_cmd_malloc_and_copy (sizeof (struct msm_jpeg_hw_cmd) *
      (1+1+(64*cnt) + 1), NULL);

  if (!p_hw_cmds) {
    JDDBG_ERROR("%s:%d] Error nomem", __func__, __LINE__);
    return -ENOMEM;
  }

  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  JDDBG_MED("%s:%d, dqt_cfg.qtable_present_flag 0x%x", __func__,
      __LINE__,dqt_cfg->qtable_present_flag);

  /*set the DMI cfg to auto increment and select no memory*/
  val = 0;
  MEM_OUTF2(&val, DMI_CFG, AUTO_INC_EN, DMI_MEM_SEL, 1,
    JPEG_DMI_CFG__DMI_MEM_SEL__QUANTIZATION_LUT);
  p_jpegd_writes(DMI_CFG, val);
  val = 0;
  p_jpegd_writes(DMI_ADDR, 0);

  /* Y Quant - 64 bytes */
  for (jpeg_dqt_tq = 0; jpeg_dqt_tq < 4; jpeg_dqt_tq++) {

    if (!(dqt_cfg->qtable_present_flag & 1<<jpeg_dqt_tq))
      continue;

    for (i = 0; i <64; i++) {
      JDDBG_LOW("%s:%d]: dqt_cfg.p_qtables[%d] = %d",
        __func__, __LINE__, i, dqt_cfg->p_qtables[jpeg_dqt_tq][i]);

       p_jpegd_writes(DMI_DATA, dqt_cfg->p_qtables[jpeg_dqt_tq][i]);

    }
    JDDBG_LOW("%s:%d: jpeg_dqt_tq=%d",__func__, __LINE__, jpeg_dqt_tq);
  }

  val = 0;
  p_jpegd_writes(DMI_CFG, val);

  rc = ioctl (fd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    free(p_hw_cmds);
    return rc;
  }
  JDDBG_MED("%s:%d] X",__func__,__LINE__);
  free(p_hw_cmds);

  return 0;
}

/*===========================================================================
 * FUNCTION    - get_scale_params  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void get_scale_params(int scale_type, uint32_t *p_hstep_int,
  uint32_t *p_hstep_frac, uint32_t *p_output_cfg)
{
  switch (scale_type) {
     case SCALE_1_8:
       *p_hstep_int = 0x8;
       *p_hstep_frac = 0;
       *p_output_cfg = 0;
       break;
     case SCALE_2_8:
       *p_hstep_int = 0x4;
       *p_hstep_frac = 0;
       *p_output_cfg = 0x1;
       break;
     case SCALE_3_8:
       *p_hstep_int = 0x2;
       *p_hstep_frac = 0x155556;
       *p_output_cfg = 0x2;
       break;
     case SCALE_4_8:
       *p_hstep_int = 0x2;
       *p_hstep_frac = 0;
       *p_output_cfg = 0x3;
       break;
     case SCALE_5_8:
       *p_hstep_int = 0x1;
       *p_hstep_frac = 0x133333;
       *p_output_cfg = 0x4;
       break;
     case SCALE_6_8:
       *p_hstep_int = 0x1;
       *p_hstep_frac = 0xaaaaa;
       *p_output_cfg = 0x5;
       break;
     case SCALE_7_8:
       *p_hstep_int = 0x1;
       *p_hstep_frac = 0x49249;
       *p_output_cfg = 0x6;
       break;
     default:
     case SCALE_NONE:
       *p_hstep_int = 0x1;
       *p_hstep_frac = 0;
       *p_output_cfg = 0x7;
       break;
  }
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_scale_core_cfg  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_scale_core_cfg(int fd)
{
  struct msm_jpeg_hw_cmd hw_cmd;
  int rc;
  uint32_t val = 0;

  JDDBG_MED("%s:%d] E", __func__, __LINE__);

  MEM_OUTF2(&val, SCALE_CFG, VSCALE_ENABLE, HSCALE_ENABLE, 1, 1);
  jpegd_write(SCALE_CFG, val);
  rc = ioctl(fd, MSM_JPEG_IOCTL_HW_CMD, &hw_cmd);
  if (rc) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    return rc;
  }
  JDDBG_MED("%s:%d] X", __func__, __LINE__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_lib_scaling_config  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_scaling_config(int fd, int scale_type)
{
  uint32_t val = 0;
  struct msm_jpeg_hw_cmds* p_hw_cmds;
  struct msm_jpeg_hw_cmd* p_hw_cmd;
  int i;
  int rc = 0;
  uint32_t hstep_int = 0;
  uint32_t hstep_frac = 0;
  uint32_t block_size = 0;
  const int int_shift = 21;

  JDDBG_MED("%s:%d] ", __func__, __LINE__);
  p_hw_cmds =
      jpegd_hw_cmd_malloc_and_copy(
      sizeof(struct msm_jpeg_hw_cmd) * (9),
      NULL);
  if (!p_hw_cmds) {
    JDDBG_ERROR("%s:%d] Error no mem", __func__, __LINE__);
    return -ENOMEM;
  }

  get_scale_params(scale_type, &hstep_int, &hstep_frac,
    &block_size);
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  val = 0;
  val = (hstep_int << int_shift) | hstep_frac;
  p_jpegd_writes(SCALE_PLN0_HSTEP, val);
  p_jpegd_writes(SCALE_PLN1_HSTEP, val);
  p_jpegd_writes(SCALE_PLN2_HSTEP, val);
  p_jpegd_writes(SCALE_PLN0_VSTEP, val);
  p_jpegd_writes(SCALE_PLN1_VSTEP, val);
  p_jpegd_writes(SCALE_PLN2_VSTEP, val);
  val = block_size<<16 | block_size;
  p_jpegd_writes(SCALE_PLN0_OUTPUT_CFG, val);
  p_jpegd_writes(SCALE_PLN1_OUTPUT_CFG, val);
  p_jpegd_writes(SCALE_PLN2_OUTPUT_CFG, val);

  rc = ioctl(fd, MSM_JPEG_IOCTL_HW_CMDS, p_hw_cmds);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    free(p_hw_cmds);
    return rc;
  }

  free(p_hw_cmds);
  JDDBG_MED("%s:%d] X", __func__, __LINE__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_hw_decode  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_hw_decode(int fd)
{
  struct msm_jpeg_hw_cmd hw_cmd;
  int rc;
  uint32_t val = 0;
  struct msm_jpeg_hw_cmds* p_hw_cmds;
  struct msm_jpeg_hw_cmd* p_hw_cmd;

  JDDBG_MED("%s:%d] ", __func__, __LINE__);

  p_hw_cmds =
    jpegd_hw_cmd_malloc_and_copy(
    sizeof(struct msm_jpeg_hw_cmd) * (1 + 1),
    NULL);
  if (!p_hw_cmds) {
    JDDBG_ERROR("%s:%d] Error no mem", __func__, __LINE__);
    return -ENOMEM;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  p_jpegd_writes(IRQ_MASK, JPEG_IRQ_MASK_BMSK);
  p_jpegd_writes(CMD, JPEG_CMD__HW_START_BMSK);

  rc = ioctl(fd, MSM_JPEG_IOCTL_START, p_hw_cmds);
  if (rc  < 0) {
    JDDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, errno);
    free(p_hw_cmds);
    return rc;
  }
  free(p_hw_cmds);
  JDDBG_MED("%s:%d] X", __func__, __LINE__);
  return 0;
}

static struct msm_jpeg_hw_cmd hw_cmd_get_version = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
  MSM_JPEG_HW_CMD_TYPE_READ, 1, JPEG_HW_VERSION_ADDR,
    JPEG_HW_VERSION_BMSK, {0},
};

void jpegd_lib_hw_get_version (struct msm_jpeg_hw_cmd *p_hw_cmd)
{
  memcpy (p_hw_cmd, &hw_cmd_get_version, sizeof (hw_cmd_get_version));
  return;
}
