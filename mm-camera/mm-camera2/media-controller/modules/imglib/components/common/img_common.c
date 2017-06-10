/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#include "img_common.h"
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "img_meta.h"
#include <cutils/properties.h>
#include <stdlib.h>

#define Q12 4096
#define MN_DIVISION_Q_BITS 10

/** IMG_SCALE_RECT:
 *  @p_in_rect: input region
 *  @p_out_rect: output region
 *  @factor scale factor
 *
 *  Scale the region based on the scale factor
 **/
#define IMG_SCALE_RECT(p_in_rect, p_out_rect, factor) ({\
  p_out_rect->pos.x = p_in_rect->pos.x * factor; \
  p_out_rect->pos.y = p_in_rect->pos.y * factor; \
  p_out_rect->size.width = p_in_rect->size.width * factor; \
  p_out_rect->size.height = p_in_rect->size.height * factor; \
})

/** IMG_GET_ZOOM_IDX:
 *  @tbl: zoom table
 *  @size: size of the table
 *  @val: value in Q12 format
 *
 *  Find the zoom index on the zoom ratio table
 **/
#define IMG_GET_ZOOM_IDX(tbl, size, val) ({ \
  uint32_t i = 0; \
  for (i = 0; i < size; i++) { \
    if (tbl[i] >= val) \
      break; \
  } \
  if (i >= size) \
    i = size - 1; \
  i; \
})

/**sigma_lut_in
  * Default sigma table for RNR under nornal lighting conditions
**/
 float sigma_lut_in[RNR_LUT_SIZE] = {
  1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f,
  1.0000f, 1.0000f, 1.0000f, 1.0000f,
  1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0015f,
  1.0030f, 1.0045f, 1.0059f, 1.0074f,
  1.0089f, 1.0104f, 1.0119f, 1.0134f, 1.0149f, 1.0164f, 1.0178f, 1.0193f,
  1.0208f, 1.0223f, 1.0238f, 1.0253f,
  1.0268f, 1.0283f, 1.0297f, 1.0312f, 1.0327f, 1.0342f, 1.0357f, 1.0372f,
  1.0387f, 1.0402f, 1.0416f, 1.0430f,
  1.0444f, 1.0459f, 1.0473f, 1.0487f, 1.0502f, 1.0516f, 1.0530f, 1.0545f,
  1.0559f, 1.0573f, 1.0588f, 1.0602f,
  1.0616f, 1.0630f, 1.0645f, 1.0659f, 1.0673f, 1.0688f, 1.0702f, 1.0716f,
  1.0731f, 1.0745f, 1.0759f, 1.0774f,
  1.0788f, 1.0802f, 1.0816f, 1.0831f, 1.0845f, 1.0859f, 1.0874f, 1.0888f,
  1.0902f, 1.1036f, 1.1170f, 1.1304f,
  1.1438f, 1.1572f, 1.1706f, 1.1840f, 1.1974f, 1.2108f, 1.2242f, 1.2376f,
  1.2510f, 1.2644f, 1.2778f, 1.2870f,
  1.2962f, 1.3054f, 1.3145f, 1.3237f, 1.3329f, 1.3421f, 1.3513f, 1.3604f,
  1.3696f, 1.3788f, 1.3880f, 1.3971f,
  1.4063f, 1.4155f, 1.4247f, 1.4339f, 1.4430f, 1.4522f, 1.4614f, 1.4706f,
  1.4798f, 1.4889f, 1.4981f, 1.5073f,
  1.5165f, 1.5256f, 1.5348f, 1.5440f, 1.5532f, 1.5637f, 1.5743f, 1.5848f,
  1.5954f, 1.6060f, 1.6165f, 1.6271f,
  1.6376f, 1.6482f, 1.6587f, 1.6693f, 1.6798f, 1.6904f, 1.7009f, 1.7115f,
  1.7220f, 1.7326f, 1.7432f, 1.7537f,
  1.7662f, 1.7786f, 1.7911f, 1.8035f, 1.8160f, 1.8285f, 1.8409f, 1.8534f,
  1.8658f, 1.8783f, 1.8907f, 1.9032f,
  1.9157f, 1.9281f, 1.9406f, 1.9530f, 1.9655f, 1.9780f, 1.9780f, 1.9780f
};

/** img_perf_handle_t
 *   @instance: performance lib instance
 *   @perf_lock_acq: performance lib acquire function
 *   @perf_lock_rel: performance lib release function
 *
 *   Performance Lib Handle
 **/
typedef struct {
  void* instance;
  int32_t (*perf_lock_acq)(unsigned long handle, int32_t duration,
    int32_t list[], int32_t numArgs);
  int32_t (*perf_lock_rel)(unsigned long handle);
} img_perf_handle_t;

/** img_perf_lock_handle_t
 *   @instance: performance lock instance
 *
 *   Performance Lock Handle
 **/
typedef struct {
  int32_t instance;
} img_perf_lock_handle_t;

/**
 * Function: img_get_subsampling_factor
 *
 * Description: gets the height and width subsampling factors
 *
 * Input parameters:
 *   ss_type - subsampling type
 *   p_w_factor - width factor
 *   p_h_factor - height factor
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_get_subsampling_factor(img_subsampling_t ss_type, float *p_w_factor,
  float *p_h_factor)
{
  switch (ss_type) {
  case IMG_H4V4:
    *p_w_factor = .25;
    *p_h_factor = .25;
    break;
  case IMG_H4V2:
    *p_w_factor = .25;
    *p_h_factor = .5;
    break;
  case IMG_H2V2:
    *p_w_factor = .5;
    *p_h_factor = .5;
    break;
  case IMG_H2V1:
    *p_w_factor = .5;
    *p_h_factor = 1.0;
    break;
  case IMG_H1V2:
    *p_w_factor = 1.0;
    *p_h_factor = .5;
    break;
  case IMG_H1V1:
    *p_w_factor = 1.0;
    *p_h_factor = 1.0;
    break;
  default:
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: img_wait_for_completion
 *
 * Description: Static function to wait until the timer has expired.
 *
 * Input parameters:
 *   p_cond - pointer to the pthread condition
 *   p_mutex - pointer to the pthread mutex
 *   ms - time in milli seconds
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_TIMEOUT
 *
 * Notes: none
 **/
int img_wait_for_completion(pthread_cond_t *p_cond, pthread_mutex_t *p_mutex,
  uint32_t ms)
{
  struct timespec ts;
  int rc = clock_gettime(CLOCK_REALTIME, &ts);
  if (rc < 0)
    return IMG_ERR_GENERAL;

  if (ms >= 1000) {
    ts.tv_sec += (ms / 1000);
    ts.tv_nsec += ((ms % 1000) * 1000000);
  } else {
    ts.tv_nsec += (ms * 1000000);
  }

  rc = pthread_cond_timedwait(p_cond, p_mutex, &ts);
  if (rc == ETIMEDOUT) {
    rc = IMG_ERR_TIMEOUT;
  }
  return rc;
}

/** img_image_copy:
 *  @out_buff: output buffer handler
 *  @in_buff: input buffer handler
 *
 * Function to copy image data from source to destination buffer
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_image_copy(img_frame_t *out_buff, img_frame_t *in_buff)
{
  int ret_val = IMG_ERR_INVALID_INPUT;
  uint8_t *in_ptr;
  uint8_t *out_ptr;
  int32_t i,j;

  if (out_buff->info.width == in_buff->info.width
    && out_buff->info.height == in_buff->info.height
    && out_buff->frame[0].plane_cnt ==
    in_buff->frame[0].plane_cnt) {

    ret_val = IMG_SUCCESS;
    for (i = 0; i < in_buff->frame[0].plane_cnt; i++) {
      if (out_buff->frame[0].plane[i].height !=
          in_buff->frame[0].plane[i].height
        || out_buff->frame[0].plane[i].width !=
          in_buff->frame[0].plane[i].width
        || out_buff->frame[0].plane[i].plane_type
          != in_buff->frame[0].plane[i].plane_type) {
        ret_val = IMG_ERR_INVALID_INPUT;
        break;
      }
    }
  }

  if (IMG_SUCCESS == ret_val) {
    for (i = 0; i < in_buff->frame[0].plane_cnt; i++) {
      out_ptr = out_buff->frame[0].plane[i].addr
        + out_buff->frame[0].plane[i].offset;
      in_ptr = in_buff->frame[0].plane[i].addr
        + in_buff->frame[0].plane[i].offset;
      for (j=0; j<in_buff->frame[0].plane[i].height; j++) {
        memcpy(out_ptr, in_ptr, out_buff->frame[0].plane[i].width);
        out_ptr += out_buff->frame[0].plane[i].stride;
        in_ptr += in_buff->frame[0].plane[i].stride;
      }
    }
  } else
    IDBG_ERROR("%s:%d failed: Output and input buffers are not compatible",
      __func__, __LINE__);

  return ret_val;
}

/**
 * Function: img_translate_cordinates
 *
 * Description: Translate the region from one window
 *             dimension to another
 *
 * Input parameters:
 *   dim1 - dimension of 1st window
 *   dim2 - dimension of 2nd window
 *   p_in_region - pointer to the input region
 *   p_out_region - pointer to the output region
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes:  none
 **/
int img_translate_cordinates(img_size_t dim1, img_size_t dim2,
  img_rect_t *p_in_region, img_rect_t *p_out_region)
{
  double min_scale;
  if (!dim1.width || !dim1.height || !dim2.width || !dim2.height) {
    IDBG_ERROR("%s:%d] Error invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((dim1.width < dim2.width) || (dim1.height < dim2.height)) {
    IDBG_MED("%s:%d] input greater than output", __func__, __LINE__);
    return IMG_ERR_NOT_SUPPORTED;
  }

  min_scale = MIN((double)dim1.width/(double)dim2.width,
    (double)dim1.height/(double)dim2.height);
  img_pixel_t temp_coodinate;
  img_size_t temp_dim;
  temp_dim.width = (double)dim1.width/min_scale;
  temp_dim.height = (double)dim1.height/min_scale;
  temp_coodinate.x = (temp_dim.width - dim2.width)/2;
  temp_coodinate.y = (temp_dim.height - dim2.height)/2;
  IDBG_MED("%s:%d] int dim %dx%d pos (%d %d)",
    __func__, __LINE__,
    temp_dim.width,
    temp_dim.height,
    temp_coodinate.x,
    temp_coodinate.y);
  p_out_region->pos.x = (p_in_region->pos.x + temp_coodinate.x) * min_scale;
  p_out_region->pos.y = (p_in_region->pos.y + temp_coodinate.y) * min_scale;
  p_out_region->size.width = p_in_region->size.width * min_scale;
  p_out_region->size.height = p_in_region->size.height * min_scale;

  IDBG_MED("%s:%d] dim1 %dx%d dim2 %dx%d in_reg (%d %d %d %d)"
    "out_reg (%d %d %d %d)",
    __func__, __LINE__,
    dim1.width, dim1.height, dim2.width, dim2.height,
    p_in_region->pos.x, p_in_region->pos.y,
    p_in_region->size.width, p_in_region->size.height,
    p_out_region->pos.x, p_out_region->pos.y,
    p_out_region->size.width, p_out_region->size.height);
  return IMG_SUCCESS;
}

/**
 * Function: img_translate_cordinates
 *
 * Description: Translate the cordinates from one window
 *             dimension to another
 *
 * Input parameters:
 *   dim1 - dimension of 1st window
 *   dim2 - dimension of 2nd window
 *   p_in_region - pointer to the input region
 *   p_out_region - pointer to the output region
 *   zoom_factor - zoom factor
 *   p_zoom_tbl - zoom table
 *   num_entries - number of zoom table entries
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_translate_cordinates_zoom(img_size_t dim1, img_size_t dim2,
  img_rect_t *p_in_region, img_rect_t *p_out_region,
  double zoom_factor, const uint32_t *p_zoom_tbl,
  uint32_t num_entries)
{
  int status = IMG_SUCCESS;
  img_rect_t zoom_reg;
  img_size_t zoom_fov;
  img_rect_t *p_zoom_reg = &zoom_reg;
  if (!dim1.width || !dim1.height || !dim2.width || !dim2.height
    || (zoom_factor < 1.0) || (zoom_factor > 8.0)) {
    IDBG_ERROR("%s:%d] Error invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  zoom_fov.width = (double)dim1.width/zoom_factor;
  zoom_fov.height = (double)dim1.height/zoom_factor;
  IDBG_MED("%s:%d] Zoom FOV %dx%d", __func__, __LINE__,
    zoom_fov.width, zoom_fov.height);

  if (!zoom_fov.width || !zoom_fov.height) {
    IDBG_ERROR("%s:%d] Error zoom value", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  status = img_translate_cordinates(zoom_fov, dim2, p_in_region, p_zoom_reg);

  if (zoom_factor == 1.0) {
    /* no zoom */
    *p_out_region = zoom_reg;
  } else if (IMG_SUCCESS == status) {
    IMG_SCALE_RECT(p_zoom_reg, p_out_region, zoom_factor);
  } else if (IMG_ERR_NOT_SUPPORTED == status) {
    /* update a zoom dimension */
    uint32_t scale_x = dim2.width * Q12 / zoom_fov.width;
    uint32_t scale_y = dim2.height * Q12 / zoom_fov.height;
    uint32_t scale = MAX(scale_x, scale_y);
    double ratio;
    int idx = 0;
    IDBG_MED("%s:%d] scale (%f %f) %u",
      __func__, __LINE__,
      (double)scale_x/(double)Q12, (double)scale_y/(double)Q12,
      scale);
    idx = IMG_GET_ZOOM_IDX(p_zoom_tbl, num_entries, scale);
    ratio = (double)p_zoom_tbl[idx] / (double)p_zoom_tbl[0];
    zoom_fov.width = ratio * zoom_fov.width;
    zoom_fov.height = ratio * zoom_fov.height;
    IDBG_MED("%s:%d] id %d val %u scale %f New Zoom FOV %dx%d",
      __func__, __LINE__, idx, p_zoom_tbl[idx], ratio,
      zoom_fov.width, zoom_fov.height);
    status = img_translate_cordinates(zoom_fov, dim2, p_in_region, p_zoom_reg);
    if (IMG_SUCCEEDED(status)) {
      zoom_factor = (double)dim1.width/(double)zoom_fov.width;
      IMG_SCALE_RECT(p_zoom_reg, p_out_region, zoom_factor);
    }
  }
  IDBG_MED("%s:%d] dim1 %dx%d dim2 %dx%d in_reg (%d %d %d %d)"
    "out_reg (%d %d %d %d)",
    __func__, __LINE__,
    dim1.width, dim1.height, dim2.width, dim2.height,
    p_in_region->pos.x, p_in_region->pos.y,
    p_in_region->size.width, p_in_region->size.height,
    p_out_region->pos.x, p_out_region->pos.y,
    p_out_region->size.width, p_out_region->size.height);

  return status;
}

/**
 * Function: img_sw_scale_init_mn
 *
 * Description: init downscaling
 *
 * Input parameters:
 *   vInfo - contains width/height info for scaling
 *   pSrc - pointer to original img buffer
 *   srcWidth - original image width
 *   srcHeight - original image height
 *   srcStride - original image stride
 *   pDst - pointer to scaled image buffer
 *   dstWidth - desired width of schaled image
 *   dstHeight - desired height of scaled image
 *   dstStride - scaled image stride
 *
 * Return values: none
 *
 * Notes:  none
 **/
static void img_sw_scale_init_mn(img_scale_mn_v_info_t*  vInfo,
  uint8_t* pSrc,
  uint32_t srcWidth,
  uint32_t srcHeight,
  uint32_t srcStride,
  uint8_t* pDst,
  uint32_t dstWidth,
  uint32_t dstHeight,
  uint32_t dstStride)
{
  vInfo->count = 0;
  vInfo->step  = 0;
  vInfo->height = srcHeight;
  vInfo->output_height = dstHeight;
  vInfo->p_v_accum_line = (uint16_t *)malloc( dstStride * sizeof(uint16_t) );

  if ( ! (vInfo->p_v_accum_line) ) {
    return; //error
  }
  memset(vInfo->p_v_accum_line,
    0,
    dstStride * sizeof(uint16_t)); // init to zero //TBD
}

/**
 * Function: img_sw_scale_mn_vscale_byte
 *
 * Description: init Vertical M/N scaling on an input lines,
 * which is one byte per pixel
 *
 * Input parameters:
 *   p_v_info - contains width/height info for scaling
 *   p_output_line
 *   output_width
 *   p_input_line
 *
 * Return values:
 *   0 - accumulating
 *   1 - outputting 1 line
 *
 * Notes:  none
 **/
int img_sw_scale_mn_vscale_byte (img_scale_mn_v_info_t *p_v_info,
  uint8_t *p_output_line,
  uint32_t output_width,
  uint8_t *p_input_line)
{
  uint32_t output_width_copy = output_width;
  uint16_t *p_v_accum_line = p_v_info->p_v_accum_line;
  uint32_t input_height = p_v_info->height;
  uint32_t output_height = p_v_info->output_height;
  uint32_t step = p_v_info->step;
  uint32_t count = p_v_info->count;

  // Accumulate one line
  while (output_width_copy--) {
    *p_v_accum_line++ += *p_input_line++;
  }
  step++;
  count += output_height;           // M
  if (count >= input_height) {        // N
    output_width_copy = output_width;
    p_v_accum_line    = p_v_info->p_v_accum_line;
    while (output_width_copy--) {
      // Scaled pixel is average of either ceiling(N/M) or flooring(N/M)
      // original pixels
      *p_output_line++ =
      (uint8_t)(((*p_v_accum_line++) * mn_division_table[step]) >> MN_DIVISION_Q_BITS);
    }
    // Update count and step
    p_v_info->count = (count - input_height);
    p_v_info->step  = 0;
    return 1;
  }
  // Update count and step
  p_v_info->count = count;
  p_v_info->step  = step;
  return 0;
}

/**
 * Function: face_proc_scale_mn_hscale_byte
 *
 * Description: init horizontal scaling
 *
 * Input parameters:
 *   p_output_line
 *   output_width - M value
 *   p_input_line
 *   input_width - N value
 *
 * Return values: None
 *
 * Notes:  none
 **/
void img_sw_scale_mn_hscale_byte (uint8_t *p_output_line,
  uint32_t output_width,
  uint8_t *p_input_line,
  uint32_t input_width)
{
  uint32_t input_width_copy = input_width;
  uint32_t step, count, accum;

  // Validate input pointer
  if (!p_output_line || !p_input_line)
    return;

  count = 0;
  step  = 0;
  accum = 0;
  while (input_width_copy--) {
    accum += *p_input_line++;
    count += output_width;
    step++;
    if (count >= input_width) {
      // Scaled pixel is average of either ceiling(N/M) or flooring(N/M)
      // original pixels
      *p_output_line++ = (uint8_t)((accum * mn_division_table[step]) >> MN_DIVISION_Q_BITS);
      count -= input_width;
      accum = 0;
      step = 0;
    }
  }
}

/**
 * Function: scalingInitMN
 *
 * Description: Image downscaling using MN method
 *
 * Input parameters:
 *   pSrc - pointer to original img buffer
 *   srcWidth - original image width
 *   srcHeight - original image height
 *   pDst - pointer to scaled image buffer
 *   dstWidth - desired width of schaled image
 *   dstHeight - desired height of scaled image
 *
 * Return values: none
 *
 * Notes:  none
 **/
void img_sw_downscale(  uint8_t *src,uint32_t srcWidth,uint32_t srcHeight,
  uint8_t *dst, uint32_t dstWidth,uint32_t dstHeight)
{
  uint32_t srcStride, dstStride;
  img_scale_mn_v_info_t  vInfo;

  uint32_t linesOutput;
  uint8_t  *pSourceLine, *pHScaledLine,  *pScaledLine;

  srcStride   = srcWidth;
  dstStride   = dstWidth;

  //init phase/filter coef/...
  img_sw_scale_init_mn(&vInfo, (uint8_t *)src, srcWidth, srcHeight, srcStride,
    dst, dstWidth, dstHeight, dstStride );

  linesOutput = 0;
  pSourceLine = src;
  pHScaledLine = (uint8_t *)malloc((dstWidth) << 1);
  if (!(pHScaledLine)) {
    free(vInfo.p_v_accum_line);
    return; //error
  }
  pScaledLine  = dst;

  while ( linesOutput < dstHeight ) {
    img_sw_scale_mn_hscale_byte( pHScaledLine, dstWidth,
      pSourceLine, srcWidth);

    pSourceLine += srcStride;

    if (img_sw_scale_mn_vscale_byte(&vInfo, pScaledLine,
      dstWidth, pHScaledLine)) {
      // Clear accumulation line
      (void)memset(vInfo.p_v_accum_line, 0,
        ((dstStride) * sizeof(uint16_t)));
      // Move to next destination line
      linesOutput++;
      pScaledLine += dstStride;
    }
  }
  free(pHScaledLine);
  free(vInfo.p_v_accum_line);
}

/** img_image_stride_fill:
 *  @out_buff: output buffer handler
 *
 * Function to fill image stride with image data
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_image_stride_fill(img_frame_t *out_buff)
{
  int ret_val = IMG_ERR_INVALID_INPUT;
  uint8_t *in_ptr8;
  uint8_t *out_ptr8;
  uint16_t *in_ptr16;
  uint16_t *out_ptr16;
  uint32_t *in_ptr32;
  uint32_t *out_ptr32;
  int32_t i,j;
  register int32_t k,padding;
  int32_t step;

  if (out_buff->frame[0].plane_cnt) {
    ret_val = IMG_SUCCESS;
    for (i=0; i<out_buff->frame[0].plane_cnt; i++) {
      if (out_buff->frame[0].plane[i].height <= 0
        || out_buff->frame[0].plane[i].width <= 0
        || out_buff->frame[0].plane[i].stride <= 0
        || out_buff->frame[0].plane[i].width >
            out_buff->frame[0].plane[i].stride) {
        ret_val = IMG_ERR_INVALID_INPUT;
        break;
      }

      step = 1;
      if (PLANE_CB_CR == out_buff->frame[0].plane[i].plane_type)
        step = 2;
      else if (PLANE_Y_CB_CR == out_buff->frame[0].plane[i].plane_type)
        step = 4;

      // If stride and width are multiple by step,
      // then padding is also multiple by step
      if (out_buff->frame[0].plane[i].stride % step
        || out_buff->frame[0].plane[i].width % step) {
          ret_val = IMG_ERR_INVALID_INPUT;
          break;
      }
    }
  }

  if (IMG_SUCCESS == ret_val) {
    for (i=0; i<out_buff->frame[0].plane_cnt; i++) {

      padding = out_buff->frame[0].plane[i].stride
        - out_buff->frame[0].plane[i].width;

      if (!padding)
        continue;

      step = 1;
      out_ptr8 = out_buff->frame[0].plane[i].addr
        + out_buff->frame[0].plane[i].offset;

      if (PLANE_CB_CR == out_buff->frame[0].plane[i].plane_type) {
        step = 2;
        padding >>= 1;
        out_ptr16 = (uint16_t*)out_ptr8;
      } else if (PLANE_Y_CB_CR == out_buff->frame[0].plane[i].plane_type) {
        step = 4;
        padding >>= 2;
        out_ptr32 = (uint32_t*)out_ptr8;
      }

      if (1 == step) {
        for (j=out_buff->frame[0].plane[i].height; j; j--) {

          out_ptr8 += out_buff->frame[0].plane[i].width;
          in_ptr8 = out_ptr8 - 1;
          for (k=padding; k; k--) {
            *out_ptr8++ = *in_ptr8--;
          }

        }
      } else if (2 == step) {
        for (j=out_buff->frame[0].plane[i].height; j; j--) {

          out_ptr16 += (out_buff->frame[0].plane[i].width >> 1);
          in_ptr16 = out_ptr16 - 1;
          for (k=padding; k; k--) {
            *out_ptr16++ = *in_ptr16--;
          }

        }
      } else if (4 == step) {
        for (j=out_buff->frame[0].plane[i].height; j; j--) {

          out_ptr32 += (out_buff->frame[0].plane[i].width >> 2);
          in_ptr32 = out_ptr32 - 1;
          for (k=padding; k; k--) {
            *out_ptr32++ = *in_ptr32--;
          }

        }
      }
    }
  } else
    IDBG_ERROR("%s:%d failed: Output and input buffers are not compatible",
      __func__, __LINE__);

  return ret_val;
}

/** img_alloc_ion:
 *  @mapion_list: Ion structure list to memory blocks to be allocated
 *  @num: number of buffers to be allocated
 *  @ionheapid: ION heap ID
 *  @cached:
 *    TRUE: mappings of this buffer should be cached, ion will do cache
            maintenance when the buffer is mapped for dma
 *    FALSE: mappings of this buffer should not be cached
 *
 * Function to allocate a physically contiguous memory
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_alloc_ion(img_mmap_info_ion *mapion_list, int num, uint32_t ionheapid,
  int cached)
{
  int ret_val = IMG_ERR_NO_MEMORY;
  struct ion_handle_data handle_data;
  struct ion_allocation_data alloc;
  int rc;
  int ion_fd;
  unsigned int ret;
  int  p_pmem_fd;
  img_mmap_info_ion *mapion = mapion_list;
  int i;

  if (!num || !mapion_list) {
    IDBG_ERROR("%s:%d failed: Wrong input parameters",
      __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i=0, mapion = mapion_list; i<num; i++, mapion++) {
    if (mapion->virtual_addr
      || mapion->ion_info_fd.handle
      || mapion->ion_info_fd.fd
      || mapion->ion_fd
      || !mapion->bufsize) {
        IDBG_ERROR("%s:%d failed: Wrong input parameters",
          __func__, __LINE__);
        return IMG_ERR_INVALID_INPUT;
    }
  }

  ion_fd = open("/dev/ion", O_RDONLY);
  if (ion_fd < 0) {
    IDBG_ERROR("%s:%d Open ion device failed",
      __func__, __LINE__);
    return ret_val;
  }

  mapion_list->ion_fd = ion_fd;

  alloc.len = mapion_list->bufsize;
  alloc.align = 4096;
  alloc.heap_id_mask = ionheapid;
  alloc.flags = 0;
  if (ION_HEAP(ION_CP_MM_HEAP_ID) == ionheapid) {
    alloc.flags |= ION_SECURE;
  }
  if (TRUE == cached) {
    alloc.flags |= ION_FLAG_CACHED;
  }

  for (i=0, mapion = mapion_list; i<num; i++, mapion++) {
    rc = ioctl(ion_fd, ION_IOC_ALLOC, &alloc);

    if (rc < 0) {
      IDBG_ERROR("%s:%d ION alloc length %d %d failed",
        __func__, __LINE__, rc, alloc.len);
      break;
    } else {
      mapion->ion_info_fd.handle = alloc.handle;
      rc = ioctl(ion_fd, ION_IOC_SHARE, &(mapion->ion_info_fd));

      if (rc < 0) {
        IDBG_ERROR("%s:%d ION map call failed %d",
          __func__, __LINE__, rc);
        break;
      } else {
        p_pmem_fd = mapion->ion_info_fd.fd;
        ret = (uint32_t)mmap(NULL,
          alloc.len,
          PROT_READ  | PROT_WRITE,
          MAP_SHARED,
          p_pmem_fd,
          0);

        if (ret == (uint32_t)MAP_FAILED) {
          IDBG_ERROR("%s:%d mmap call failed %d",
            __func__, __LINE__, rc);
          break;
        } else {
          IDBG_HIGH("%s:%d Ion allocation success virtaddr : %u fd %u",
            __func__, __LINE__,
            (uint32_t)ret,
            (uint32_t)p_pmem_fd);
          mapion->virtual_addr = (void *)ret;
          mapion->ion_fd = ion_fd;
          ret_val = IMG_SUCCESS;
        }
      }
    }
  }

  if (IMG_SUCCESS != ret_val) {
    img_free_ion(mapion_list, num);
  }

  return ret_val;
}

/** img_free_ion:
 *  @mapion_list: Ion structure list to the allocated memory blocks
 *  @num: number of buffers to be freed
 *
 * Free ion memory
 *
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_free_ion(img_mmap_info_ion* mapion_list, int num)
{
  struct ion_handle_data handle_data;
  unsigned int bufsize;
  int i;
  img_mmap_info_ion* mapion;

  if (!num || !mapion_list) {
    IDBG_ERROR("%s:%d Wrong input parameters",
      __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i=0, mapion = mapion_list; i<num; i++, mapion++) {

    if (mapion->ion_info_fd.handle) {
      if (mapion->ion_info_fd.fd) {
        if (mapion->virtual_addr) {
          bufsize = (mapion->bufsize + 4095) & (~4095);
          munmap(mapion->virtual_addr, bufsize);
        }
        close(mapion->ion_info_fd.fd);
      }
      handle_data.handle = mapion->ion_info_fd.handle;
      ioctl(mapion->ion_fd, ION_IOC_FREE, &handle_data);
    }

  }

  if (mapion_list->ion_fd) {
    close(mapion_list->ion_fd);
  }

  return IMG_SUCCESS;
}

/** img_cache_ops_external:
 *  @p_buffer: vaddr of the buffer
 *  @size: Buffer size
 *  @offset: Buffer offset
 *  @fd: Fd of p_buffer
 *  @type: Type of cache operation- CACHE_INVALIDATE, CACHE_CLEAN
 *              or CACHE_CLEAN_INVALIDATE
 *  @ion_device_fd: Ion device FD
 *
 * Invalidate cache for memory allocated externally
 *
 *
 * Returns IMG_SUCCESS
 *         IMG_ERR_INVALID_INPUT
 *         IMG_ERR_GENERAL
 **/
int img_cache_ops_external (void *p_buffer, size_t size, int offset, int fd,
  img_cache_ops_type type, int ion_device_fd)
{
  int rc = IMG_SUCCESS;
  struct ion_custom_data custom_data;
  struct ion_flush_data cache_inv_data;
  struct ion_fd_data fd_data;
  struct ion_handle_data handle_data;
  uint32_t cmd = ION_IOC_CLEAN_INV_CACHES;

  memset(&custom_data, 0, sizeof(struct ion_custom_data));
  memset(&cache_inv_data, 0, sizeof(struct ion_flush_data));
  memset(&fd_data, 0, sizeof(struct ion_fd_data));
  memset(&handle_data, 0, sizeof(struct ion_handle_data));

  if (ion_device_fd < 0) {
    IDBG_ERROR("%s:%d: Invalid ION fd %d", __func__, __LINE__, ion_device_fd);
    return IMG_ERR_INVALID_INPUT;
  }

  if (NULL == p_buffer) {
    IDBG_ERROR("%s:%d: Buffer is null", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  fd_data.fd = fd;
  rc = ioctl (ion_device_fd, ION_IOC_IMPORT, &fd_data);
  if (rc) {
    IDBG_ERROR("%s:%d: ION_IOC_IMPORT failed", __func__, __LINE__);
    return rc;
  }

  switch (type) {
  case CACHE_INVALIDATE:
    cmd = ION_IOC_INV_CACHES;
    break;
  case CACHE_CLEAN:
    cmd = ION_IOC_CLEAN_CACHES;
    break;
  default:
  case CACHE_CLEAN_INVALIDATE:
    cmd = ION_IOC_CLEAN_INV_CACHES;
    break;
  }

  handle_data.handle = fd_data.handle;
  cache_inv_data.handle = fd_data.handle;
  cache_inv_data.vaddr = p_buffer;
  cache_inv_data.offset = offset;
  cache_inv_data.length = size;
  custom_data.cmd = cmd;
  custom_data.arg = (unsigned long)&cache_inv_data;

  rc = ioctl(ion_device_fd, ION_IOC_CUSTOM, &custom_data);
  if (rc ) {
    IDBG_ERROR("%s:%d: Cache Invalidation failed %d", __func__, __LINE__, rc);
    ioctl(ion_device_fd, ION_IOC_FREE, &handle_data);
    return IMG_ERR_GENERAL;
  }

  ioctl(ion_device_fd, ION_IOC_FREE, &handle_data);
  return IMG_SUCCESS;
}

/** img_dump_frame
 *    @img_frame: frame handler
 *    @number: number to be appended at the end of the file name
 *
 * Saves specified frame to folder /data/
 *
 * Returns TRUE in case of success
 **/
void img_dump_frame(img_frame_t *img_frame, char* file_name,
  uint32_t number)
{
  int32_t i;
  uint32_t size;
  uint32_t written_size;
  int32_t out_file_fd;
  char out_file_name[80];
  time_t rawtime;
  struct tm *currenttime = NULL;
  char value[PROPERTY_VALUE_MAX];

  property_get("persist.camera.imglib.dump", value, "0");
  if (0 == atoi(value)) {
    return;
  }

  if (img_frame && img_frame->frame) {
    char timestamp[30];
    timestamp[0] = '\0';
    rawtime = time(NULL);
    currenttime = localtime(&rawtime);
    if (currenttime) {
      snprintf(timestamp,
        25, "%04d%02d%02d%02d%02d%02d",
        currenttime->tm_year+1900, currenttime->tm_mon, currenttime->tm_mday,
        currenttime->tm_hour, currenttime->tm_min, currenttime->tm_sec);
    }

    snprintf(out_file_name, sizeof(out_file_name), "%s%s_%s_%dx%d_%d_%d.yuv",
      "/data/misc/camera/", file_name, timestamp,
      img_frame->frame->plane[0].stride,
      img_frame->frame->plane[0].scanline,
      img_frame->frame_id,
      number);
    out_file_fd = open(out_file_name, O_RDWR | O_CREAT, 0777);

    if (out_file_fd >= 0) {
      for (i = 0; i < img_frame->frame->plane_cnt; i++) {
        size = img_frame->frame->plane[i].stride
          * img_frame->frame->plane[i].scanline;
        written_size = write(out_file_fd,
          img_frame->frame->plane[i].addr + img_frame->frame->plane[i].offset,
          size);
        if (size != written_size)
          IDBG_ERROR("%s:%d failed: Cannot write data to file %s\n", __func__,
            __LINE__, out_file_name);
      }

      close(out_file_fd);

      IDBG_HIGH("%s:%d: dim %dx%d pad_dim %dx%d",
        __func__, __LINE__,
        img_frame->frame->plane[0].width,
        img_frame->frame->plane[0].height,
        img_frame->frame->plane[0].stride,
        img_frame->frame->plane[0].scanline);
    } else
      IDBG_ERROR("%s:%d failed: Cannot open file\n", __func__, __LINE__);
  } else
    IDBG_ERROR("%s:%d failed: Null pointer detected\n", __func__, __LINE__);
}

/**
 * Function: img_get_meta
 *
 * Description: This macro is to get the meta value from
 *               metadata if present
 *
 * Arguments:
 *   @p_meta : meta buffer
 *   @type: meta type
 *
 * Return values:
 *     meta buffer value pointer
 *
 * Notes: none
 **/
void *img_get_meta(img_meta_t *p_meta, img_meta_type_t type)
{
  void *val = NULL;
  if (p_meta && (type < IMG_META_MAX)) {
    switch (type) {
    case IMG_META_R_GAMMA:
      val = (void *)&p_meta->gamma_R; break;
    case IMG_META_G_GAMMA:
      val = (void *)&p_meta->gamma_G; break;
    case IMG_META_B_GAMMA:
      val = (void *)&p_meta->gamma_B; break;
    case IMG_META_AEC_INFO:
      val = (void *)&p_meta->aec_info; break;
    case IMG_META_AWB_INFO:
      val = (void *)&p_meta->awb_info; break;
    case IMG_META_AF_INFO:
      val = (void *)&p_meta->af_info; break;
    case IMG_META_OUTPUT_ROI:
      val = (void *)&p_meta->output_crop; break;
    case IMG_META_ZOOM_FACTOR:
      val = (void *)&p_meta->zoom_factor; break;
    case IMG_META_OUTPUT_DIM:
      val = (void *)&p_meta->output_dim; break;
    default:;
    }
  }
  return val;
}

/** img_perf_lock_handle_create
 *
 * Creates new performance handle
 *
 * Returns new performance handle
 **/
void* img_perf_handle_create()
{
  img_perf_handle_t *perf_handle;
  char qcopt_lib_path[PATH_MAX] = {0};

  perf_handle = calloc(1, sizeof(img_perf_handle_t));
  if (!perf_handle) {
    IDBG_ERROR("%s:%d Not enough memory\n", __func__, __LINE__);
    return NULL;
  }

  if (!property_get("ro.vendor.extension_library", qcopt_lib_path, NULL)) {
    IDBG_ERROR("%s:%d Cannot get performance lib name\n", __func__, __LINE__);
    free(perf_handle);
    return NULL;
  }

  dlerror();
  perf_handle->instance = dlopen(qcopt_lib_path, RTLD_NOW);
  if (!perf_handle->instance) {
    IDBG_ERROR("%s:%d Unable to open %s: %s\n", __func__, __LINE__,
      qcopt_lib_path, dlerror());
    free(perf_handle);
    return NULL;
  }

  perf_handle->perf_lock_acq = dlsym(perf_handle->instance, "perf_lock_acq");

  if (!perf_handle->perf_lock_acq) {
    IDBG_ERROR("%s:%d Unable to get perf_lock_acq function handle\n", __func__,
      __LINE__);
    if (dlclose(perf_handle->instance)) {
      IDBG_ERROR("%s:%d Error occurred while closing qc-opt library\n",
        __func__, __LINE__);
    }
    free(perf_handle);
    return NULL;
  }

  perf_handle->perf_lock_rel = dlsym(perf_handle->instance, "perf_lock_rel");

  if (!perf_handle->perf_lock_rel) {
    IDBG_ERROR("%s:%d Unable to get perf_lock_rel function handle\n", __func__,
      __LINE__);
    if (dlclose(perf_handle->instance)) {
      IDBG_ERROR("%s:%d Error occurred while closing qc-opt library\n",
        __func__, __LINE__);
    }
    free(perf_handle);
    return NULL;
  }

  return perf_handle;
}

/** img_perf_handle_destroy
 *    @p_perf: performance handle
 *
 * Destoyes performance handle
 *
 * Returns None.
 **/
void img_perf_handle_destroy(void* p_perf)
{
  img_perf_handle_t *perf_handle = (img_perf_handle_t*)p_perf;

  IMG_RETURN_IF_NULL(return, perf_handle)
  IMG_RETURN_IF_NULL(return, perf_handle->instance)

  if (dlclose(perf_handle->instance)) {
    IDBG_ERROR("%s:%d Error occurred while closing qc-opt library\n",
      __func__, __LINE__);
  }

  free(perf_handle);
}

/** img_perf_lock_start
 *    @p_perf: performance handle
 *    @p_perf_lock_params: performance lock parameters
 *    @perf_lock_params_size: size of performance lock parameters
 *    @duration: duration
 *
 * Locks performance with specified parameters
 *
 * Returns new performance lock handle
 **/
void* img_perf_lock_start(void* p_perf, int32_t* p_perf_lock_params,
  size_t perf_lock_params_size, int32_t duration)
{
  img_perf_handle_t *perf_handle = (img_perf_handle_t*)p_perf;
  img_perf_lock_handle_t *lock_handle;

  IMG_RETURN_IF_NULL(return NULL, perf_handle)
  IMG_RETURN_IF_NULL(return NULL, p_perf_lock_params)
  IMG_RETURN_IF_NULL(return NULL, perf_handle->perf_lock_acq)

  lock_handle = calloc(1, sizeof(img_perf_lock_handle_t));
  if (!lock_handle) {
    IDBG_ERROR("%s:%d Not enough memory\n", __func__, __LINE__);
    return NULL;
  }

  lock_handle->instance = perf_handle->perf_lock_acq(lock_handle->instance,
    duration, p_perf_lock_params, perf_lock_params_size);

  return lock_handle;
}

/** img_perf_lock_end
 *    @p_perf: performance handle
 *    @p_perf_lock: performance lock handle
 *
 * Locks performance with specified parameters
 *
 * Returns None.
 **/
void img_perf_lock_end(void* p_perf, void* p_perf_lock)
{
  img_perf_handle_t *perf_handle = (img_perf_handle_t*)p_perf;
  img_perf_lock_handle_t *lock_handle = (img_perf_lock_handle_t*)p_perf_lock;

  IMG_RETURN_IF_NULL(return, perf_handle)
  IMG_RETURN_IF_NULL(return, lock_handle)
  IMG_RETURN_IF_NULL(return, perf_handle->perf_lock_rel)

  perf_handle->perf_lock_rel(lock_handle->instance);

  free(lock_handle);
}
