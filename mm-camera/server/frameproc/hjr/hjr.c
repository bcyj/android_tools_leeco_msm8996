/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hjr.h"
#include "frameproc.h"

/*===========================================================================
    Internal global variables
===========================================================================*/
#define TWO_BIT_MASK  0x3
#define SHIFT19 (1<<19)
#define SHIFT7  (1<<7)
#define SHIFT20 (1<<20)
#define PASS_THRESHOLD  60000
#define HJR_MULTI_FRAME_SEARCH_FACTOR 0.025
typedef struct proj_t {
  uint32_t *h1;
  uint32_t *h2;
  uint32_t *h3;
  uint32_t *h4;
  uint32_t *v1;
  uint32_t *v2;
  uint32_t *v3;
  uint32_t *v4;
} proj_t;

typedef struct m_vector_t {
  int16_t vert[9];
  int16_t hor[9];
} m_vector_t;

/* This structure defines the various color types
*/
typedef enum {
  HJR_YCbCr = 0,        /* YCbCr pixel color format 4:2:2                  */
  HJR_YCbCr420_FRAME_PK,/* YCbCr 4:2:0 Frame Packed Format                 */
  HJR_YCbCr420_LINE_PK, /* YCbCr 4:2:0 Line Packed Format                  */
  HJR_YCbCr420_MB_PK,   /* YCbCr 4:2:0 Line Packed Format                  */
  HJR_RGB565,           /* RGB 565 color format                            */
  HJR_RGB888,           /* RGB 888 color format                            */
  HJR_BAYER_GBRG,       /* Mega Pixel GBRG format                          */
  HJR_BAYER_BGGR,       /* Mega Pixel BGGR format                          */
  HJR_BAYER_GRBG,       /* Mega Pixel GRBG format                          */
  HJR_BAYER_RGGB,       /* Mega Pixel RGGB format                          */
  HJR_RGB666,           /* RGB 666 format                                  */
  HJR_RGB444,           /* RGB 444 format                                  */
  HJR_YCbCr422_LINE_PK, /* YCbCr 4:2:2 Line Packed Format                  */
  HJR_YCbCr444_LINE_PK, /* YCbCr 4:4:4 Line Packed Format                  */
  HJR_YCrCb420_LINE_PK, /* YCrCb 4:2:0 Line Packed Format                  */
  HJR_YCrCb422_LINE_PK, /* YCrCb 4:2:2 Line Packed Format                  */
  HJR_YCrCb444_LINE_PK, /* YCrCb 4:4:4 Line Packed Format                  */
  HJR_YCbCr444,         /* YCbCr 4:4:4                                     */
  HJR_YCrCb420_MB_PK,   /* YCrCb 4:2:0 Macro Block                         */
  HJR_YCbCr422_MB_PK,   /* YCrCb 4:2:0 Macro Block                         */
  HJR_YCrCb422_MB_PK,   /* YCrCb 4:2:0 Macro Block                         */
  HJR_YCrCb420_FRAME_PK,/* YCrCb 4:2:0 Frame Packed Format                 */
  HJR_H1V1MCU_CbCr,     /* H1V1 MCU data, usually from JPEG decoder        */
  HJR_H1V2MCU_CbCr,     /* H1V2 MCU data, usually from JPEG decoder        */
  HJR_H2V1MCU_CbCr,     /* H2V1 MCU data, usually from JPEG decoder        */
  HJR_H2V2MCU_CbCr,     /* H1V2 MCU data, usually from JPEG decoder        */
  HJR_MCU_GRAY,         /* MCU data, but only y since gray scale           */
  HJR_YCbCr444_PAD,     /* 0YCbCr data (32 bit word, padded on high order) */
  HJR_RGB888_PAD,       /* 0RGB data (32 bit word, padded on high order)   */
  HJR_LUMA_ONLY,        /* Just Y (luma) data                              */
  HJR_ALPHA,            /* Just 8bit alpha channel                         */
  HJR_HSV,              /* Hue saturation value format                     */
  HJR_COL_MAX           /* Maximum Number of color formats                 */
} hjr_col_for_type;

/* This structure defines the format of an image
*/
typedef struct hjr_image_struct {
  uint32_t dx;             /* Number of pixels in the x dirctn or in a row*/
  uint32_t dy;             /* Number of pixels in the y dirctn or in a col*/
  hjr_col_for_type cFormat;  /* Color Format for image                    */
  unsigned char* imgPtr;     /* Pointer to the image data                 */
  uint8_t* clrPtr;           /* Pointer to the Color data                 */
} hjr_image_type;


static uint8_t flat_gain1 = 0;
static uint8_t flat_gain2 = 0;
static uint8_t texture_gain1 = 0;
static uint8_t texture_gain2 = 0;

static uint32_t pix_sqr[256] =
{ 0x0000, 0x0001, 0x0004, 0x0009, 0x0010, 0x0019, 0x0024, 0x0031,
  0x0040, 0x0051, 0x0064, 0x0079, 0x0090, 0x00A9, 0x00C4, 0x00E1,
  0x0100, 0x0121, 0x0144, 0x0169, 0x0190, 0x01B9, 0x01E4, 0x0211,
  0x0240, 0x0271, 0x02A4, 0x02D9, 0x0310, 0x0349, 0x0384, 0x03C1,
  0x0400, 0x0441, 0x0484, 0x04C9, 0x0510, 0x0559, 0x05A4, 0x05F1,
  0x0640, 0x0691, 0x06E4, 0x0739, 0x0790, 0x07E9, 0x0844, 0x08A1,
  0x0900, 0x0961, 0x09C4, 0x0A29, 0x0A90, 0x0AF9, 0x0B64, 0x0BD1,
  0x0C40, 0x0CB1, 0x0D24, 0x0D99, 0x0E10, 0x0E89, 0x0F04, 0x0F81,
  0x1000, 0x1081, 0x1104, 0x1189, 0x1210, 0x1299, 0x1324, 0x13B1,
  0x1440, 0x14D1, 0x1564, 0x15F9, 0x1690, 0x1729, 0x17C4, 0x1861,
  0x1900, 0x19A1, 0x1A44, 0x1AE9, 0x1B90, 0x1C39, 0x1CE4, 0x1D91,
  0x1E40, 0x1EF1, 0x1FA4, 0x2059, 0x2110, 0x21C9, 0x2284, 0x2341,
  0x2400, 0x24C1, 0x2584, 0x2649, 0x2710, 0x27D9, 0x28A4, 0x2971,
  0x2A40, 0x2B11, 0x2BE4, 0x2CB9, 0x2D90, 0x2E69, 0x2F44, 0x3021,
  0x3100, 0x31E1, 0x32C4, 0x33A9, 0x3490, 0x3579, 0x3664, 0x3751,
  0x3840, 0x3931, 0x3A24, 0x3B19, 0x3C10, 0x3D09, 0x3E04, 0x3F01,
  0x4000, 0x4101, 0x4204, 0x4309, 0x4410, 0x4519, 0x4624, 0x4731,
  0x4840, 0x4951, 0x4A64, 0x4B79, 0x4C90, 0x4DA9, 0x4EC4, 0x4FE1,
  0x5100, 0x5221, 0x5344, 0x5469, 0x5590, 0x56B9, 0x57E4, 0x5911,
  0x5A40, 0x5B71, 0x5CA4, 0x5DD9, 0x5F10, 0x6049, 0x6184, 0x62C1,
  0x6400, 0x6541, 0x6684, 0x67C9, 0x6910, 0x6A59, 0x6BA4, 0x6CF1,
  0x6E40, 0x6F91, 0x70E4, 0x7239, 0x7390, 0x74E9, 0x7644, 0x77A1,
  0x7900, 0x7A61, 0x7BC4, 0x7D29, 0x7E90, 0x7FF9, 0x8164, 0x82D1,
  0x8440, 0x85B1, 0x8724, 0x8899, 0x8A10, 0x8B89, 0x8D04, 0x8E81,
  0x9000, 0x9181, 0x9304, 0x9489, 0x9610, 0x9799, 0x9924, 0x9AB1,
  0x9C40, 0x9DD1, 0x9F64, 0xA0F9, 0xA290, 0xA429, 0xA5C4, 0xA761,
  0xA900, 0xAAA1, 0xAC44, 0xADE9, 0xAF90, 0xB139, 0xB2E4, 0xB491,
  0xB640, 0xB7F1, 0xB9A4, 0xBB59, 0xBD10, 0xBEC9, 0xC084, 0xC241,
  0xC400, 0xC5C1, 0xC784, 0xC949, 0xCB10, 0xCCD9, 0xCEA4, 0xD071,
  0xD240, 0xD411, 0xD5E4, 0xD7B9, 0xD990, 0xDB69, 0xDD44, 0xDF21,
  0xE100, 0xE2E1, 0xE4C4, 0xE6A9, 0xE890, 0xEA79, 0xEC64, 0xEE51,
  0xF040, 0xF231, 0xF424, 0xF619, 0xF810, 0xFA09, 0xFC04, 0xFE01};

/*===========================================================================
 * FUNCTION    - hjr_unpack_10_bit_bayer_data -
 *
 * DESCRIPTION: UNPACKS THE 10 BIT PACKED BAYER DATA.
 *==========================================================================*/
void hjr_unpack_10_bit_bayer_data(uint8_t *input_data_ptr,
  uint32_t input_data_size)
{
  int32_t i,j;
  uint8_t* bottom_input_data_ptr, *bottom_output_data_ptr;
  uint8_t  partial_10_bit_byte;
  uint8_t  two_bit_data;
  uint16_t ten_bit_data;
  uint16_t* sixteen_bit_output_data_ptr;
  uint32_t bottom_output_index;

  if (input_data_ptr == NULL)
    return;

  bottom_input_data_ptr = input_data_ptr + input_data_size - 1;
  /* Find out the number of bytes required to convert packed 10
   * bit values to unpacked 10 bit values of 2 bytes each */
  bottom_output_index = (input_data_size * 6) / 4 - 1;
  bottom_output_data_ptr = input_data_ptr + bottom_output_index;
  bottom_output_data_ptr--;
  sixteen_bit_output_data_ptr = (uint16_t*) bottom_output_data_ptr;

  for (i = (int32_t)input_data_size; i > 0; i -= 4) {
    partial_10_bit_byte = (uint8_t) (* bottom_input_data_ptr);
    bottom_input_data_ptr--;
    for (j = 2; j >= 0; j--) {
      two_bit_data = (partial_10_bit_byte & (TWO_BIT_MASK <<
        (j * 2)) ) >> (j * 2) ;
      ten_bit_data = ( (uint8_t) (* bottom_input_data_ptr) |
        (two_bit_data << 8 ) );
      *sixteen_bit_output_data_ptr = ten_bit_data;
      bottom_input_data_ptr--;
      sixteen_bit_output_data_ptr--;
    }
  }
} /* hjr_unpack_10_bit_bayer_data */

/*===========================================================================
 * FUNCTION    - hjr_pack_10_bit_bayer_data -
 *
 * DESCRIPTION: PACKS THE 10 BIT DATA TO PACKED 10-BIT BAYER FORMAT.
 *==========================================================================*/
void hjr_pack_10_bit_bayer_data(uint8_t *input_data_ptr,
  uint32_t input_data_size)
{
  int32_t i, j;
  uint8_t  partial_10_bit_byte;
  uint8_t  two_bit_data;
  uint8_t* eight_bit_output_data_ptr;

  if (input_data_ptr == NULL)
    return;

  eight_bit_output_data_ptr = input_data_ptr;
  for (i = 0; i < (int32_t)input_data_size; i += 4) {
    partial_10_bit_byte = 0;
    for (j = 0; j < 3; j++) {
      *eight_bit_output_data_ptr = *input_data_ptr;
      eight_bit_output_data_ptr++;
      input_data_ptr++;
      two_bit_data = *input_data_ptr;
      /* Check for overflow of 10 bit data. If more, round it to
       * highest 10 bit value */
      if (two_bit_data > 3) {
        two_bit_data = 3;
        *(eight_bit_output_data_ptr - 1 ) = 0xFF;
      }
      input_data_ptr++;
      two_bit_data &= TWO_BIT_MASK;
      partial_10_bit_byte = (uint8_t)(partial_10_bit_byte | (two_bit_data <<
        (j * 2)));
    }

    *eight_bit_output_data_ptr = partial_10_bit_byte;
    eight_bit_output_data_ptr++;
  }

} /* hjr_pack_10_bit_bayer_data */

/*===========================================================================
 * FUNCTION    - hjr_free_projection -
 *
 * DESCRIPTION: Checks to see if any projects have been allocated and frees
 *              them if nessasary.
 *==========================================================================*/
void hjr_free_projection(proj_t *proj)
{
  if (proj->h1 != NULL)
    free(proj->h1);
  if (proj->h2 != NULL)
    free(proj->h2);
  if (proj->h3 != NULL)
    free(proj->h3);
  if (proj->h4 != NULL)
    free(proj->h4);
  if (proj->v1 != NULL)
    free(proj->v1);
  if (proj->v2 != NULL)
    free(proj->v2);
  if (proj->v3 != NULL)
    free(proj->v3);
  if (proj->v4 != NULL)
    free(proj->v4);
  proj->h1 = proj->h2 = proj->h3 = proj->h4 = proj->v1 = proj->v2 =
    proj->v3 = proj->v4 = NULL;
} /* hjr_free_projection */

/*===========================================================================
 * FUNCTION    - hjr_allocate_projection -
 *
 * DESCRIPTION: Allocates all of the memory needed for the projects of
 *              one frame. If any one allocation fails, all memory if
 *              freed and IP_NO_MEMORY is returned
 *==========================================================================*/
int hjr_allocate_projection(proj_t *proj, uint16_t width,
  uint16_t height)
{
  proj->h1 = proj->h2 = proj->h3 = proj->h4 = proj->v1 = proj->v2 =
    proj->v3 = proj->v4 = NULL;

  proj->h1 = (uint32_t*) malloc(width * sizeof(uint32_t));
  proj->h2 = (uint32_t*) malloc(width * sizeof(uint32_t));
  proj->h3 = (uint32_t*) malloc(width * sizeof(uint32_t));
  proj->h4 = (uint32_t*) malloc(width * sizeof(uint32_t));
  proj->v1 = (uint32_t*) malloc(height * sizeof(uint32_t));
  proj->v2 = (uint32_t*) malloc(height * sizeof(uint32_t));
  proj->v3 = (uint32_t*) malloc(height * sizeof(uint32_t));
  proj->v4 = (uint32_t*) malloc(height * sizeof(uint32_t));

  if ((proj->h1 == NULL) || (proj->h2 == NULL) || (proj->h3 == NULL) ||
    (proj->h4 == NULL) || (proj->v1 == NULL) || (proj->v2 == NULL) ||
    (proj->v3 == NULL) || (proj->v4 == NULL)) {
    /* call free projections because checks which allocations failed */
    hjr_free_projection(proj);
    return FALSE; /* No Memory */
  }
  return TRUE;
} /* hjr_allocate_projection */

/*===========================================================================
 * FUNCTION    - get_scaler -
 *
 * DESCRIPTION: calculate the normalization scalers for the given resolution
 *==========================================================================*/
uint16_t get_scaler(uint32_t first, uint32_t second)
{
  uint16_t scale_out;
  uint8_t  scale_second;

  if (second < 256)
    scale_second = 8;
  else if (second < 512)
    scale_second = 9;
  else if (second < 1024)
    scale_second = 10;
  else if (second < 2048)
    scale_second = 11;
  else
    scale_second = 12;

  scale_out = (1 << (scale_second + 17)) / first / first;
  scale_out = scale_out * (1 << 10) / second;
  return scale_out;
} /* get_scaler */

/*===========================================================================
 * FUNCTION    - hjr_get_projections -
 *
 * DESCRIPTION: Creates the projects from an input frame.
 *==========================================================================*/
int hjr_get_projections(uint8_t *fr, uint32_t width ,
  uint32_t height, proj_t *proj)
{
  uint32_t i, j, h4, w4;
  uint32_t *temp1, *temp2, *temp3, *temp4, *temph;
  uint32_t wstop;
  uint8_t *pixel, *lineofdata;

  if (fr == NULL)
    return FALSE;

  lineofdata = (uint8_t *)malloc(width);
  if (lineofdata == NULL)
    return FALSE; /* No Memory */

  /* caculate the quadrents */
  h4 = height / 4;
  w4 = width / 4;
  /* Initialize the projections to zero */
  memset(proj->h1, 0, width * sizeof(proj->h1));
  memset(proj->h2, 0, width * sizeof(proj->h2));
  memset(proj->h3, 0, width * sizeof(proj->h3));
  memset(proj->h4, 0, width * sizeof(proj->h4));
  memset(proj->v1, 0, height * sizeof(proj->v1));
  memset(proj->v2, 0, height * sizeof(proj->v2));
  memset(proj->v3, 0, height * sizeof(proj->v3));
  memset(proj->v4, 0, height * sizeof(proj->v4));

  /* Temp pointers to the vertical projects */
  temp1 = proj->v1;
  temp2 = proj->v2;
  temp3 = proj->v3;
  temp4 = proj->v4;
  /* go through the first quadrent */
  for (i = 0;i < h4; i++) {
    temph = proj->h1;
    memcpy(lineofdata, fr, width);
    fr += width;
    pixel = lineofdata;
    /* calulate for the first fourth incrementing ponters */
    for (j=0;j<w4;j++) {
      *temph++ += *pixel;
      *temp1 += *pixel++;
    }
    wstop = w4 + w4;
    for (; j < wstop; j++) {
      *temph++ += *pixel;
      *temp2 += *pixel++;
    }
    wstop += w4;
    for (; j < wstop; j++) {
      *temph++ += *pixel;
      *temp3 += *pixel++;
    }
    for (; j < width; j++) {
      *temph++ += *pixel;
      *temp4 += *pixel++;
    }
    /* increment the temp vertical pointers */
    temp1++; temp2++; temp3++; temp4++;
  }
  for (i = h4; i < 2 * h4; i++) {
    temph = proj->h2;
    memcpy(     lineofdata,      fr,      width);
    fr += width;
    pixel = lineofdata;
    for (j = 0; j < w4; j++) {
      *temph++ += *pixel;
      *temp1 += *pixel++;
    }
    wstop = w4 + w4;
    for (; j < wstop; j++) {
      *temph++ += *pixel;
      *temp2 += *pixel++;
    }
    wstop += w4;
    for (; j < wstop; j++) {
      *temph++ += *pixel;
      *temp3 += *pixel++;
    }
    for (; j < width; j++) {
      *temph++ += *pixel;
      *temp4 += *pixel++;
    }
    /* increment the temp vertical pointers */
    temp1++; temp2++; temp3++; temp4++;
  }
  for (i = 2 * h4; i < 3 * h4; i++) {
    temph = proj->h3;
    memcpy(lineofdata, fr, width);
    fr += width;
    pixel = lineofdata;
    for (j = 0; j < w4; j++) {
      *temph++ += *pixel;
      *temp1 += *pixel++;
    }
    wstop = w4 + w4;
    for (; j < wstop; j++) {
      *temph++ += *pixel;
      *temp2 += *pixel++;
    }
    wstop += w4;
    for (; j < wstop; j++) {
      *temph++ += *pixel;
      *temp3 += *pixel++;
    }
    for (; j < width; j++) {
      *temph++ += *pixel;
      *temp4 += *pixel++;
    }
    /* increment the temp vertical pointers */
    temp1++; temp2++; temp3++; temp4++;
  }
  for (i = 3 * h4; i < height; i++) {
    temph = proj->h4;
    memcpy(lineofdata, fr, width);
    fr += width;
    pixel = lineofdata;
    for (j = 0; j < w4; j++) {
      *temph++ += *pixel;
      *temp1 += *pixel++;
    }
    wstop = w4 + w4;
    for (; j < wstop; j++) {
      *temph++ += *pixel;
      *temp2 += *pixel++;
    }
    wstop += w4;
    for (; j < wstop; j++) {
      *temph++ += *pixel;
      *temp3 += *pixel++;
    }
    for (; j < width; j++) {
      *temph++ += *pixel;
      *temp4 += *pixel++;
    }
    /* increment the temp vertical pointers */
    temp1++; temp2++; temp3++; temp4++;
  }
  free(lineofdata);

  temp1 = proj->h1;
  temp2 = proj->h2;
  temp3 = proj->h3;
  temp4 = proj->h4;

  for (i=0;i<width;i++) {
    *temp1++ += *temp2;
    *temp2++ += *temp3;
    *temp3++ += *temp4;
  }
  temp1 = proj->v1;
  temp2 = proj->v2;
  temp3 = proj->v3;
  temp4 = proj->v4;
  for (i = 0; i < height; i++) {
    *temp1++ +=  *temp2;
    *temp2++ += *temp3;
    *temp3++ += *temp4;
  }
  return TRUE;
} /* hjr_get_projections */

/*===========================================================================
 * FUNCTION    - hjr_corr1D -
 *
 * DESCRIPTION: Creates the correlation between two projections for a
 *              specific dimension
 *==========================================================================*/
int16_t hjr_corr1D(uint32_t *sum1, uint32_t *sum2, uint32_t len,
  uint32_t max_lag, uint32_t *work_buffer, uint32_t *pass)
{
  uint8_t scale;
  uint32_t N,i,j, ind1, ind2;
  int32_t max_ind;
  int32_t tmp32;
  uint32_t *ptr1;
  uint32_t *ptr2;
  uint32_t *corr;
  uint32_t tmp;

  corr = work_buffer;
  if (len < 256)
    scale = 8;
  else if (len < 512)
    scale = 9;
  else if (len < 1024)
    scale = 10;
  else if (len < 2048)
    scale = 11;
  else
    scale = 12;

  N = len-max_lag;
  ind1 = 0;
  ind2 = max_lag;

  for (i = 0; i < 2 * max_lag + 1; i++) {
    ptr1 = sum1 + ind1;
    ptr2 = sum2 + ind2;
    tmp = 0;
    for (j = 0; j < N; j++) {
      tmp32 = (int32_t)(*ptr1++) - (int32_t)(*ptr2++);
      tmp32 = tmp32 >> 4;
      tmp += (tmp32 * tmp32) >> scale;
    }
    corr[i] = tmp;
    if (ind1 < ind2)
      ind2 -= 1;
    else
      ind1 += 1;
  }
  tmp = corr[0];
  max_ind = 0;

  for (i = 1; i < 2 * max_lag + 1; i++) {
    if (tmp > corr[i]) {
      tmp = corr[i];
      max_ind = i;
    }
  }
  *pass = tmp;
  return(int16_t) (max_lag - max_ind);
} /*hjr_corr1D */

/*===========================================================================
 * FUNCTION    - hjr_interpolate_vector -
 *
 * DESCRIPTION: Interpolates the motion vectors for each line of the image.
 *==========================================================================*/
void hjr_interpolate_vector(int32_t motion1, int32_t motion2,
  uint32_t size, int32_t *vector, uint8_t inv)
{
  uint32_t i;
  register int32_t diff, tmp;

  /* caculate the difference between two rows of the frame */
  diff = motion2 - motion1;
  if (diff > 0)
    diff = 2 * diff / size;
  else
    diff =- ((-2 * diff) / (int32_t)size);

  /* loop through caculating the motion for each row */
  tmp = motion1;
  if (!inv) {
    for (i = 0; i < size; i++) {
      *vector++ = tmp;
      tmp += diff;
    }
  } else {
    vector = vector + size - 1;
    for (i = 0; i < size; i++) {
      *vector-- = tmp;
      tmp += diff;
    }
  }
} /* hjr_interpolate_vector */

/*===========================================================================
 * FUNCTION    - hjr_interpolate_vectorLine -
 *
 * DESCRIPTION: Calulates the inital offset and slope for a line given the
 *              motion vec
 *==========================================================================*/
void hjr_interpolate_vectorLine(int32_t motion0, int32_t motion1,
  int32_t motion2, uint32_t size, int32_t *InitalOffset, int32_t* Slope)
{
  *InitalOffset = ((4 * motion0 + motion1 - 2 * motion2)) / 3;
  *Slope = ((motion2 - motion0) * 2) / (int32_t)size;
} /* hjr_interpolate_vectorLine */

/*===========================================================================
 * FUNCTION    - hjr_smart_combine_frames -
 *
 * DESCRIPTION: Takes the registration data between two frames and combines
 *              the two frames.
 *              The first frame is overwritten by the new combined frames.
 *==========================================================================*/
int hjr_smart_combine_frames(hjr_image_type *frame1, hjr_image_type *frame2,
  m_vector_t *vec, int32_t width, int32_t height,
  uint8_t scaleQ8, hjr_col_for_type cs)
{
  uint16_t scaleQ8_inv;
  uint16_t tmp; /* used in the caculation of the new pixel data */
  uint8_t *chr1; /* Croma for the first frame */
  uint8_t *chr2; /* Croma for the second frame */
  uint8_t *fr1; /* Y data for the first frame */
  uint8_t *fr2; /* Y data for the second frame */
  int32_t i, j;  /* Counters for looping through the data */
  int32_t I, J;  /* Current offset for the vertical and horizontal directions */
  int32_t *p1_h, *p2_h, *p3_h, *p1_v, *p2_v, *p3_v;
  /* Temp pointers to the pixel data currently being processed */
  uint8_t *TempFr1,*TempFr2;
  int32_t maxshiftI,maxshiftJ,nextbreak, stopwidth, inextbreak, jnextbreak;
  int32_t hnoff,vnoff;
  /* For caculating Croma, we need the orginal offset for the line */
  int32_t iOrgImage,iCromaFactor;
  int32_t HorInitalOffset, HorSlope, VerInitalOffset, VerSlope;
  int32_t OrgWidth = width;
  int32_t linelength;
  /* used as temp holders of image data so we can operate on cashed data */
  uint8_t *line_one, *line_two;
  uint8_t *cur_frame_line;

  scaleQ8_inv = (1 << 8) - scaleQ8;
  fr1 = frame1->imgPtr;
  fr2 = frame2->imgPtr;

  /* Allocate interperlated vectors */
  p1_h = (int32_t*) malloc(height * sizeof(uint32_t));
  p2_h = (int32_t*) malloc(height * sizeof(uint32_t));
  p3_h = (int32_t*) malloc(height * sizeof(uint32_t));
  p1_v = (int32_t*) malloc(height * sizeof(uint32_t));
  p2_v = (int32_t*) malloc(height * sizeof(uint32_t));
  p3_v = (int32_t*) malloc(height * sizeof(uint32_t));

  TempFr1 = line_one = (uint8_t *)malloc(width);
  TempFr2 = line_two = (uint8_t *)malloc(width);

  /* Make sure we allocated the buffers */
  if ((p1_h == NULL) || (p2_h == NULL) || (p3_h == NULL) || (p1_v == NULL) ||
    (p2_v == NULL) || (p3_v == NULL) ||(line_one == NULL) ||
    (line_two == NULL)) { /* allocation failed, free sucessful allocations */
    if (p1_h != NULL)
      free(p1_h);
    if (p2_h != NULL)
      free(p2_h);
    if (p3_h != NULL)
      free(p3_h);
    if (p1_v != NULL)
      free(p1_v);
    if (p2_v != NULL)
      free(p2_v);
    if (p3_v != NULL)
      free(p3_v);
    if (line_one != NULL)
      free(line_one);
    if (line_two != NULL)
      free(line_two);
    return FALSE;  /* No Memory */
  }
  /* Interpolate from the correlation vectors for three columns of the image */
  hjr_interpolate_vector(vec->hor[3] << 20, vec->hor[0] << 20, height / 2,
    p1_h, 1);
  hjr_interpolate_vector(vec->hor[3] << 20, vec->hor[6] << 20, height / 2,
    p1_h + height / 2, 0);
  hjr_interpolate_vector(vec->vert[3] << 20, vec->vert[0] << 20, height / 2,
    p1_v, 1);
  hjr_interpolate_vector(vec->vert[3] << 20, vec->vert[6] << 20, height / 2,
    p1_v + height / 2, 0);
  hjr_interpolate_vector(vec->hor[4] << 20, vec->hor[1] << 20,
    height / 2, p2_h, 1);
  hjr_interpolate_vector(vec->hor[4] << 20, vec->hor[7] << 20,
    height / 2, p2_h + height / 2, 0);
  hjr_interpolate_vector(vec->vert[4] << 20, vec->vert[1] << 20,
    height / 2, p2_v, 1);
  hjr_interpolate_vector(vec->vert[4] << 20, vec->vert[7] << 20,
    height / 2, p2_v + height / 2, 0);

  hjr_interpolate_vector(vec->hor[5] << 20, vec->hor[2] << 20,
    height / 2, p3_h, 1);
  hjr_interpolate_vector(vec->hor[5] << 20, vec->hor[8] << 20,
    height / 2, p3_h + height / 2, 0);
  hjr_interpolate_vector(vec->vert[5] << 20, vec->vert[2] << 20,
    height / 2, p3_v, 1);
  hjr_interpolate_vector(vec->vert[5] << 20, vec->vert[8] << 20,
    height / 2, p3_v + height / 2, 0);

  for (i = 0; i < height; i++) {
    /* calculate the Initial offset and slope for the horizontal and
     * vertical shifts between the images */
    hjr_interpolate_vectorLine(p1_v[i], p2_v[i], p3_v[i],width,
      &VerInitalOffset, &VerSlope);
    hjr_interpolate_vectorLine(p1_h[i], p2_h[i], p3_h[i],width,
      &HorInitalOffset, &HorSlope);
    I = (VerInitalOffset + SHIFT19) >> 20; /* initial offset */
    J = (HorInitalOffset + SHIFT19) >> 20; /* initial offset */
    /* if the slope is zero, then the caculated values will
     * not change on this row */
    if (VerSlope == 0) {
      inextbreak = width;
      vnoff = width;
      maxshiftI = I;
    } else {
      /* The length of a segment that the vertical offset will not change */
      inextbreak = SHIFT20/VerSlope;
      if (inextbreak < 0)  inextbreak = -inextbreak; /* Get the abs value */
      /* Caculate the inital segment this value holds true for */
      vnoff = (VerInitalOffset%SHIFT20)/VerSlope;
      if (vnoff < 0) vnoff = -vnoff;  /* Get the absolute value */
      /* Calculate the maximum vertical offset for this row */
      maxshiftI = I + ((VerSlope*width) >> 20);
    }
    /* same caculations as above, but for the horizontal offsets */
    if (HorSlope == 0) {
      jnextbreak = width;
      hnoff = width;
      maxshiftJ = J;
    } else {
      jnextbreak = SHIFT20 / HorSlope;
      if (jnextbreak < 0)  jnextbreak = -jnextbreak;
      hnoff = (HorInitalOffset % SHIFT20) / HorSlope;
      if (hnoff < 0) hnoff = -hnoff;
      maxshiftJ = J + ((HorSlope * width) >> 20);
    }
    /* first let see if this line, will ever overlap */
    if ((((i + I) >= 0) || ((i + I + maxshiftI) >= 0)) &&
      (((i + I) < height) || ((i + I + maxshiftI) < height))) {
      /* Initialze the values */
      if (J < 0) {
        /* If the inital hor shift is negitave, then we can start
         * that number of pixels into the row */
        j = -J;
        /* caculate how many pixels neede for horizontal shifts */
        stopwidth = width - maxshiftJ + J + 1;
      } else {
        j = 0;
        stopwidth = width - maxshiftJ;
      }
      /* Get the overall effect lenght of the row */
      stopwidth = width < stopwidth ? width : stopwidth;
      /* Get the location of the pixel in frame 1 */
      cur_frame_line = &fr1[i * width + j];
      linelength = stopwidth - j; /* we can get the entire needed line */
      /* copy the data from the first frame */
      memcpy(line_one,cur_frame_line, linelength);
      TempFr1 = line_one; /* Set the temp variable to the cashed line */
      while (j < stopwidth) {
        nextbreak = vnoff < hnoff ? vnoff : hnoff; /* min change of pixel */
        /* catch a special case that the start of the
         * line is after the first break */
        if (j < nextbreak) {
          if ((I + i >= 0) && (I + i < height)) {
            nextbreak = nextbreak < stopwidth ? nextbreak : stopwidth;
            memcpy(line_two, &fr2[(I + i) * width + j + J], nextbreak-j);
            /* copy the second frame to cashed data */
            TempFr2 = line_two;
            if (scaleQ8 == 128) {
              /* This is evenly merge the images, so use the faster shifts */
              for (;j < nextbreak;j++) {
                tmp = (uint16_t)((*TempFr1) + (uint16_t)(*TempFr2++));
                *TempFr1++ = (uint8_t)(tmp >> 1);
              }
            } else {
              /* Generic case, need to multiply by the scaling factor */
              for (;j < nextbreak;j++) {
                tmp = (uint16_t)((*TempFr1 * scaleQ8_inv) +
                  (uint16_t)(*TempFr2++ * scaleQ8));
                *TempFr1++ = (uint8_t)((tmp + SHIFT7) >> 8);
              }
            }
          } else {
            /* Move the frames forward so the loop will continue */
            TempFr1 += nextbreak - j;
            TempFr2 += nextbreak - j;
            j = nextbreak;
          }
        }
        /* caculate the break */
        /* If both cases are true we will update vertical then loop back around
         * effectively dowing nothing and update the horizontal variables */
        if (nextbreak == vnoff) {
          vnoff += inextbreak;
          if (VerSlope > 0)
            I++;
          else
            I--;
        } else { /* we are at the horizontal break in the offset */
          hnoff += jnextbreak;
          if (HorSlope > 0)
            J++;
          else
            J--;
        }
        if ((i + I < 0) || (i + I > height))
          break;
      }
      /* copy the data back now we are done processing*/
      memcpy(cur_frame_line,line_one,linelength);
    }
  }
  CDBG_HJR("Done hjr_interpolate_vector\n");
  chr1 = frame1->clrPtr;
  chr2 = frame2->clrPtr;
  iCromaFactor = 0;

  if (cs == HJR_YCrCb420_LINE_PK) {
    height = height / 2;
    iCromaFactor = 1;
    width = width / 2;
  } else if (cs == HJR_YCrCb422_LINE_PK) {
    width = width / 2;
  }
  /* Chroma Registration */
  for (i = 0; i < height; i++) {
    iOrgImage = i << iCromaFactor;
    hjr_interpolate_vectorLine(p1_v[iOrgImage], p2_v[iOrgImage],
      p3_v[iOrgImage], OrgWidth, &VerInitalOffset, &VerSlope);
    hjr_interpolate_vectorLine(p1_h[iOrgImage], p2_h[iOrgImage],
      p3_h[iOrgImage], OrgWidth, &HorInitalOffset, &HorSlope);
    /* initial offset */
    I = ((VerInitalOffset >> iCromaFactor) + SHIFT19) >> 20;
    J = ((HorInitalOffset >> 1) + SHIFT19) >> 20;

    if (VerSlope == 0) {
      inextbreak = width;
      vnoff = width;
      maxshiftI = I;
    } else {
      inextbreak = (SHIFT20 / (VerSlope << iCromaFactor));
      if (inextbreak < 0)  inextbreak = -inextbreak;
      vnoff = ((VerInitalOffset >> 1) % SHIFT20) / (VerSlope << 1);
      if (vnoff < 0) vnoff = -vnoff;
      maxshiftI = I + (((VerSlope << iCromaFactor) * width) >> 20);
    }
    if (HorSlope == 0) {
      jnextbreak = width;
      hnoff = width;
      maxshiftJ = J;
    } else {
      jnextbreak = SHIFT20 / (HorSlope << iCromaFactor);
      if (jnextbreak < 0)  jnextbreak = -jnextbreak;
      hnoff = ((HorInitalOffset >> iCromaFactor) % SHIFT20) /
        (HorSlope << iCromaFactor);
      if (hnoff < 0) hnoff = -hnoff;
      maxshiftJ = J + (((HorSlope << iCromaFactor) * width) >> 20);
    }

    /* first let see if this line, will ever overlap */
    if ((((i + I) >= 0) || ((i + I + maxshiftI) >= 0)) &&
      (((i + I) < height) || ((i + I + maxshiftI) < height))) {
      if (J < 0) {
        j = -J;
        stopwidth = width - maxshiftJ + J + 1;
      } else {
        j = 0;
        stopwidth = width - maxshiftJ;
      }
      stopwidth = width < stopwidth ? width : stopwidth;
      cur_frame_line = &chr1[(i * width + j) << 1];
      linelength = (stopwidth - j) << 1;
      /* copy the data from the first frame */
      memcpy(line_one, cur_frame_line, linelength);
      TempFr1 = line_one;

      while (j < stopwidth) {
        nextbreak = vnoff < hnoff ? vnoff : hnoff; /* min change of pixel */

        if (j < nextbreak) {
          /* catch a special case that the start of the line
           * is after the first break */
          if ((i + I >= 0) && (i + I < height)) {
            nextbreak = nextbreak < stopwidth ? nextbreak : stopwidth;

            /* catch a special case that the start of the
             * line is after the first break */
            if (j < nextbreak) {
              memcpy(line_two,&chr2[((I + i) * width + j + J) <<1 ],
                (nextbreak - j) <<1); /* copy second frame to cashed data */

              TempFr2 = line_two;
              if (scaleQ8 == 128) {
                for (;j < nextbreak;j++) {
                  tmp = (uint16_t)((*TempFr1) + (uint16_t)(*TempFr2++));
                  *TempFr1++ = (uint8_t)(tmp >> 1);
                  tmp = (uint16_t)((*TempFr1) + (uint16_t)(*TempFr2++));
                  *TempFr1++ = (uint8_t)(tmp >> 1);
                }
              } else {
                for (; j < nextbreak; j++) {
                  tmp = (uint16_t)((*TempFr1 * scaleQ8_inv) +
                    (uint16_t)(*TempFr2++ * scaleQ8));
                  *TempFr1++ = (uint8_t)((tmp + SHIFT7) >> 8);
                  tmp = (uint16_t)((*TempFr1 * scaleQ8_inv) +
                    (uint16_t)(*TempFr2++ * scaleQ8));
                  *TempFr1++ = (uint8_t)((tmp + SHIFT7) >> 8);
                }
              }
            } else {
              TempFr1 += (nextbreak - j) << 1;
              TempFr2 += (nextbreak - j) << 1;
              j = nextbreak;
            }

          }
        }
        /* caculate the break */
        if (nextbreak == vnoff) {
          vnoff += inextbreak;
          if (VerSlope > 0)
            I++;
          else
            I--;
        } else {
          hnoff += jnextbreak;
          if (HorSlope > 0)
            J++;
          else
            J--;
        }
        if ((i + I < 0) || (i + I > height))
          break;
      }
      memcpy(cur_frame_line,line_one,linelength); /* copy the data from the first frame */
    }
  }
  CDBG_HJR("Done Chroma Registration\n");
  /* Just free the temp vectors.  They have already been checked for NULL */
  free(p1_h);
  free(p2_h);
  free(p3_h);
  free(p1_v);
  free(p2_v);
  free(p3_v);
  free(line_one);
  free(line_two);
  CDBG_HJR("Done hjr_smart_combine_frames\n");
  return TRUE;
} /* hjr_smart_combine_frames */

/*===========================================================================
 * FUNCTION    - hjr_smart_register_frames -
 *
 * DESCRIPTION: Takes two or three frames and automatically registers
 *              the frames to create one frame with reduced hand jitter
 *==========================================================================*/
int hjr_smart_register_frames(frame_proc_t *frameCtrl, hjr_image_type **in_image,
  uint32_t max_corr_Xlag, uint32_t max_corr_Ylag)
{
  int rc = TRUE;
  hjr_image_type *frame1 = NULL, *frame2 = NULL, *frame3 = NULL;
  uint32_t numberFrames;
  proj_t proj1, proj2;
  m_vector_t m_vec12, m_vec13;
  uint32_t max_corr_factor;
  uint32_t *corr_buffer;
  uint32_t height, width;
  hjr_col_for_type cs;
  uint32_t  i = 0;
  uint32_t passH[9], passV[9], pass = 0, max_pass = 0;
  uint32_t scaler_h, scaler_v;

  /* Check for valid input values */
  /* Check for valid input values */
  if (in_image == NULL)
    return FALSE;

  frame1 = in_image[0];
  frame2 = in_image[1];
  frame3 = NULL;

  if ((frame1 == NULL) || (frame2 == NULL))
    return FALSE;

  /* In the two frame case, frame three is set to null
   * If more frames are given, they are ignored */
  if (frame3 == NULL)
    numberFrames = 2;
  else
    numberFrames = 3;

  height = frame1->dy;
  width = frame1->dx;
  cs = frame1->cFormat;
  /* check to make sure the dimisions and format are the same */
  if ((height != frame2->dy) || (width != frame2->dx) ||
    (cs != frame2->cFormat))
    return FALSE;

  if ((numberFrames == 3)&& ((height != frame3->dy) ||
    (width != frame3->dx) || (cs != frame3->cFormat)))
    return FALSE;

  if ((cs != HJR_YCrCb420_LINE_PK) && (cs != HJR_YCrCb422_LINE_PK))
    return FALSE;

  /* allocate the work buffer for corrolating the motion vecotors
   * This is done to eliminate multiple malloc/frees */
  max_corr_factor = max_corr_Xlag > max_corr_Ylag ?
    max_corr_Xlag : max_corr_Ylag;
  corr_buffer = (uint32_t*) malloc((2 * max_corr_factor + 1) *
    sizeof(uint32_t));
  if (corr_buffer == NULL)
    return FALSE; /* No Memory */

  /* Allocate memory for the projections */
  if (!hjr_allocate_projection(&proj1,width,height)) {
    free(corr_buffer);
    return FALSE; /* No Memory */
  }
  if (!hjr_allocate_projection(&proj2,width,height)) {
    free(corr_buffer);
    hjr_free_projection(&proj1);
    return FALSE; /* No Memory */
  }
  /* caculate the projections for frame 1 and 2 */
  if ((hjr_get_projections(frame1->imgPtr, width , height,
    &proj1)) &&
    (hjr_get_projections(frame2->imgPtr, width , height,
    &proj2))) {
    /* Calculate the motion vectors from the projections */
    m_vec12.hor[0]  = hjr_corr1D(proj1.h1, proj2.h1, width / 2,
      max_corr_Xlag, corr_buffer, &passH[0]);
    m_vec12.hor[1]  = hjr_corr1D(proj1.h1 + width / 4, proj2.h1 + width / 4,
      width / 2, max_corr_Xlag, corr_buffer, &passH[1]);
    m_vec12.hor[2]  = hjr_corr1D(proj1.h1 + width / 2, proj2.h1 + width / 2,
      width / 2, max_corr_Xlag, corr_buffer, &passH[2]);
    m_vec12.hor[3]  = hjr_corr1D(proj1.h2, proj2.h2, width / 2,
      max_corr_Xlag, corr_buffer, &passH[3]);
    m_vec12.hor[4]  = hjr_corr1D(proj1.h2 + width / 4, proj2.h2 + width / 4,
      width / 2, max_corr_Xlag, corr_buffer, &passH[4]);
    m_vec12.hor[5]  = hjr_corr1D(proj1.h2 + width / 2, proj2.h2 + width / 2,
      width / 2, max_corr_Xlag, corr_buffer, &passH[5]);
    m_vec12.hor[6]  = hjr_corr1D(proj1.h3, proj2.h3, width / 2, max_corr_Xlag,
      corr_buffer, &passH[6]);
    m_vec12.hor[7]  = hjr_corr1D(proj1.h3 + width / 4, proj2.h3 + width / 4,
      width / 2, max_corr_Xlag, corr_buffer, &passH[7]);
    m_vec12.hor[8]  = hjr_corr1D(proj1.h3 + width / 2, proj2.h3 + width / 2,
      width / 2, max_corr_Xlag, corr_buffer, &passH[8]);
    m_vec12.vert[0] = hjr_corr1D(proj1.v1, proj2.v1, height / 2, max_corr_Ylag,
      corr_buffer, &passV[0]);
    m_vec12.vert[3] = hjr_corr1D(proj1.v1 + height / 4, proj2.v1 + height / 4,
      height / 2, max_corr_Ylag, corr_buffer, &passV[3]);
    m_vec12.vert[6] = hjr_corr1D(proj1.v1 + height / 2, proj2.v1 + height / 2,
      height / 2, max_corr_Ylag, corr_buffer, &passV[6]);
    m_vec12.vert[1] = hjr_corr1D(proj1.v2, proj2.v2, height / 2, max_corr_Ylag,
      corr_buffer, &passV[1]);
    m_vec12.vert[4] = hjr_corr1D(proj1.v2 + height / 4, proj2.v2 + height / 4,
      height / 2, max_corr_Ylag, corr_buffer, &passV[4]);
    m_vec12.vert[7] = hjr_corr1D(proj1.v2 + height / 2, proj2.v2 + height / 2,
      height / 2, max_corr_Ylag, corr_buffer, &passV[7]);
    m_vec12.vert[2] = hjr_corr1D(proj1.v3, proj2.v3, height / 2, max_corr_Ylag,
      corr_buffer, &passV[2]);
    m_vec12.vert[5] = hjr_corr1D(proj1.v3 + height / 4, proj2.v3 + height / 4,
      height / 2, max_corr_Ylag, corr_buffer, &passV[5]);
    m_vec12.vert[8] = hjr_corr1D(proj1.v3 + height / 2, proj2.v3 + height / 2,
      height / 2, max_corr_Ylag, corr_buffer, &passV[8]);
    scaler_h = get_scaler(height / 2, width / 2);
    scaler_v = get_scaler( width / 2, height / 2);
    CDBG_HJR("Scaler H %d, V %d\n", scaler_h, scaler_v);

    for (i = 0; i < 9; i++) {
      pass = (passH[i] >> 8) * scaler_h + (passV[i] >> 8) * scaler_v;

      if ((passH[i] < 4096) && (passV[i] < 4096))
        pass = (passH[i] * scaler_h + passV[i] * scaler_v) >> 8;

      CDBG_HJR("pass H %u, V %u\n", passH[i], passV[i]);
      if (pass > max_pass)
        max_pass = pass;
      /* camsensor_static_params_ptr->chromatix_parms->m_vector12_vert[i] =
       * m_vec12.vert[i]; camsensor_static_params_ptr->chromatix_parms->
       * m_vector12_hor[i] = m_vec12.hor[i]; */
    }
    CDBG_HJR("HJR max_pass %u, THRESHOLD %d\n", max_pass, PASS_THRESHOLD);
    if (max_pass > PASS_THRESHOLD) {
      /* camsensor_static_params_ptr->chromatix_parms->
       * pass_registration12 = 0; */
      free(corr_buffer);
      hjr_free_projection(&proj1);
      hjr_free_projection(&proj2);
      return FALSE;
    }
    /* camsensor_static_params_ptr->chromatix_parms->
     * pass_registration12 = 1; */
    /* combine frames 1 and 2 */
    if ((rc = hjr_smart_combine_frames(frame1, frame2,
      &m_vec12,width, height, 128, cs))) {
      if (numberFrames > 2) {
        /* In the three frame case, combine frame 1 and 3 */
        /* reuse the memory allocated for frame 2 projections for frame 3 */
        /* Frame 1 projections would not have changed */
        hjr_get_projections(frame3->imgPtr, width , height, &proj2);
        /* caculate the motion vecotors between frame 1 and 3 */
        m_vec13.hor[2]  = hjr_corr1D(proj1.h1, proj2.h1, width / 2,
          max_corr_Xlag, corr_buffer, &passH[2]);
        m_vec13.hor[1]  = hjr_corr1D(proj1.h1 + width / 4, proj2.h1 +
          width / 4, width / 2, max_corr_Xlag, corr_buffer, &passH[1]);
        m_vec13.hor[0]  = hjr_corr1D(proj1.h1 + width / 2, proj2.h1 +
          width / 2, width / 2, max_corr_Xlag, corr_buffer, &passH[0]);
        m_vec13.hor[5]  = hjr_corr1D(proj1.h2, proj2.h2, width / 2,
          max_corr_Xlag, corr_buffer, &passH[5]);
        m_vec13.hor[4]  = hjr_corr1D(proj1.h2 + width / 4, proj2.h2 +
          width / 4, width / 2, max_corr_Xlag, corr_buffer, &passH[4]);
        m_vec13.hor[3]  = hjr_corr1D(proj1.h2 + width / 2, proj2.h2 +
          width / 2, width / 2, max_corr_Xlag, corr_buffer, &passH[3]);
        m_vec13.hor[8]  = hjr_corr1D(proj1.h3, proj2.h3, width / 2,
          max_corr_Xlag, corr_buffer, &passH[8]);
        m_vec13.hor[7]  = hjr_corr1D(proj1.h3 + width / 4, proj2.h3 +
          width / 4, width / 2, max_corr_Xlag, corr_buffer, &passH[7]);
        m_vec13.hor[6]  = hjr_corr1D(proj1.h3 + width / 2, proj2.h3 +
          width / 2, width / 2, max_corr_Xlag, corr_buffer, &passH[6]);
        m_vec13.vert[2] = hjr_corr1D(proj1.v1, proj2.v1, height / 2,
          max_corr_Ylag, corr_buffer, &passV[2]);
        m_vec13.vert[5] = hjr_corr1D(proj1.v1 + height / 4, proj2.v1 +
          height / 4, height / 2, max_corr_Ylag, corr_buffer, &passV[5]);
        m_vec13.vert[8] = hjr_corr1D(proj1.v1 + height / 2, proj2.v1 +
          height / 2, height / 2, max_corr_Ylag, corr_buffer, &passV[8]);
        m_vec13.vert[1] = hjr_corr1D(proj1.v2, proj2.v2, height / 2,
          max_corr_Ylag, corr_buffer, &passV[1]);
        m_vec13.vert[4] = hjr_corr1D(proj1.v2 + height / 4, proj2.v2 +
          height / 4, height / 2, max_corr_Ylag, corr_buffer, &passV[4]);
        m_vec13.vert[7] = hjr_corr1D(proj1.v2 + height / 2, proj2.v2 +
          height / 2, height / 2, max_corr_Ylag, corr_buffer, &passV[7]);
        m_vec13.vert[0] = hjr_corr1D(proj1.v3, proj2.v3, height / 2,
          max_corr_Ylag, corr_buffer, &passV[0]);
        m_vec13.vert[3] = hjr_corr1D(proj1.v3 + height / 4, proj2.v3 +
          height / 4, height / 2, max_corr_Ylag, corr_buffer, &passV[3]);
        m_vec13.vert[6] = hjr_corr1D(proj1.v3 + height / 2, proj2.v3 +
          height / 2, height / 2, max_corr_Ylag, corr_buffer, &passV[6]);
        scaler_h = get_scaler(height / 2,  width / 2);
        scaler_v = get_scaler( width / 2, height / 2);
        CDBG_HJR("Scaler h %d, v %d\n", scaler_h, scaler_v);

        for (i = 0, pass = 0, max_pass = 0; i < 9; i++) {
          pass = (passH[i]>>8)*scaler_h + (passV[i]>>8)*scaler_v;

          if ((passH[i]<4096)&&(passV[i]<4096))
            pass = (passH[i]* scaler_h + passV[i]*scaler_v)>>8;

          CDBG_HJR("pass h %u, v %u\n", passH[i], passV[i]);
          if (pass > max_pass)
            max_pass = pass;
          /* camsensor_static_params_ptr->chromatix_parms->
           * m_vector13_vert[i] = m_vec13.vert[i];
           * camsensor_static_params_ptr->chromatix_parms->
           * m_vector13_hor[i] = m_vec13.hor[i]; */
        }
        CDBG_HJR("HJR 2 max_pass %u, THRESHOLD %d\n", max_pass, PASS_THRESHOLD);
        if (max_pass > PASS_THRESHOLD) {
          /* camsensor_static_params_ptr->chromatix_parms->
          pass_registration13 = 0; */
          free(corr_buffer);
          hjr_free_projection(&proj1);
          hjr_free_projection(&proj2);
          return FALSE;
        }
        /* camsensor_static_params_ptr->chromatix_parms->
         *pass_registration13 = 1; */
        /* combine the frames betwee frame 1 and 3.
         * Frame 3's is using a smaller factor */
        if (!hjr_smart_combine_frames(frame1, frame3,
          &m_vec13,width, height, 85, cs))
          rc = FALSE; /* No Memory */
      }
    } else {
      rc = FALSE; /* No Memory */
    }
  } else {
    rc = FALSE; /* No Memory */
  }

  /* Free the allocated memory */
  free(corr_buffer);
  hjr_free_projection(&proj1);
  hjr_free_projection(&proj2);
  return rc;
} /* hjr_smart_register_frames */

/*===========================================================================
 * FUNCTION    - hjr_handle_multi_frames_for_handjitter -
 *
 * DESCRIPTION: Entry point for the registering of two frames for HJR.
 *==========================================================================*/
int hjr_handle_multi_frames_for_handjitter(void *Ctrl)
{
  static hjr_image_type
    hjr_snapshot_frames_main[HJR_MAX_FRAMES_SUPPORTED],
    hjr_snapshot_frames_thmb[HJR_MAX_FRAMES_SUPPORTED];
  hjr_image_type *in_img_ptr[HJR_MAX_FRAMES_SUPPORTED + 1];
  uint32_t max_corr_Xlag = 0;
  uint32_t max_corr_Ylag = 0;
  int cnt;
  frame_proc_t *frameCtrl = (frame_proc_t *) Ctrl;
  int frame_cnt = frameCtrl->input.statsproc_info.aec_d.hjr_snap_frame_count;
  CDBG_HJR("In hjr_handle_multi_frames_for_handjitter\n");
  /*Thumbnail processing*/
  for (cnt = 0; cnt < HJR_MAX_FRAMES_SUPPORTED + 1; cnt++)
    in_img_ptr[cnt] = NULL;
  for (cnt = 0; cnt < frame_cnt; cnt++) {
    hjr_snapshot_frames_thmb[cnt].imgPtr =
      (unsigned char*) frameCtrl->input.mctl_info.thumb_img_frame[cnt].mp[0].vaddr;
    hjr_snapshot_frames_thmb[cnt].clrPtr =
      (uint8_t*) frameCtrl->input.mctl_info.thumb_img_frame[cnt].mp[1].vaddr;
    /*todo thumbnail dimensions are never set by media controller */
    hjr_snapshot_frames_thmb[cnt].dx =
      frameCtrl->input.mctl_info.thumbnail_dim.width;
    hjr_snapshot_frames_thmb[cnt].dy =
      frameCtrl->input.mctl_info.thumbnail_dim.height;
    hjr_snapshot_frames_thmb[cnt].cFormat = HJR_YCrCb420_LINE_PK;
  }
  for (cnt = 0; cnt < frame_cnt; cnt++)
    in_img_ptr[cnt]= &hjr_snapshot_frames_thmb[cnt];

  max_corr_Xlag = frameCtrl->input.mctl_info.thumbnail_dim.width * HJR_MULTI_FRAME_SEARCH_FACTOR;
  max_corr_Ylag = frameCtrl->input.mctl_info.thumbnail_dim.height * HJR_MULTI_FRAME_SEARCH_FACTOR;
  if (!hjr_smart_register_frames(frameCtrl, in_img_ptr, max_corr_Xlag,
    max_corr_Ylag))
    CDBG_ERROR("HJR Thumbnail Processing FAIL\n");
  else
    CDBG_HIGH("Done HJR Thumbnail Processing\n");

  /* Main Image Processing */
  for (cnt = 0; cnt < HJR_MAX_FRAMES_SUPPORTED + 1; cnt++)
    in_img_ptr[cnt] = NULL;
  for (cnt = 0; cnt < frame_cnt; cnt++) {
    hjr_snapshot_frames_main[cnt].imgPtr =
      (unsigned char*) frameCtrl->input.mctl_info.main_img_frame[cnt].mp[0].vaddr;
    hjr_snapshot_frames_main[cnt].clrPtr =
      (uint8_t*) frameCtrl->input.mctl_info.main_img_frame[cnt].mp[1].vaddr;
    hjr_snapshot_frames_main[cnt].dx =
      frameCtrl->input.mctl_info.picture_dim.width;
    hjr_snapshot_frames_main[cnt].dy =
      frameCtrl->input.mctl_info.picture_dim.height;
    hjr_snapshot_frames_main[cnt].cFormat = HJR_YCrCb420_LINE_PK;
  }
  for (cnt = 0; cnt < frame_cnt; cnt++)
    in_img_ptr[cnt]= &hjr_snapshot_frames_main[cnt];

  max_corr_Xlag = frameCtrl->input.mctl_info.picture_dim.width *
    HJR_MULTI_FRAME_SEARCH_FACTOR;
  max_corr_Ylag = frameCtrl->input.mctl_info.picture_dim.height *
    HJR_MULTI_FRAME_SEARCH_FACTOR;
  if (!hjr_smart_register_frames(frameCtrl, in_img_ptr, max_corr_Xlag,
    max_corr_Ylag))
    return FALSE;
  CDBG_HIGH("Done HJR Main Img Processing\n");
  return TRUE;
} /* hjr_process_multi_frames */
