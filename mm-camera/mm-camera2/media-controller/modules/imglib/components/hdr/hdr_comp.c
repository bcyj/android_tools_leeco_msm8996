/**********************************************************************
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
 * Qualcomm Technologies Proprietary and Confidential.                 *
 **********************************************************************/

#include "hdr_comp.h"

/**
 * CONSTANTS and MACROS
 **/

#define hdr_validate_image(p_frame, i) ({ \
  int rc = 0; \
  if ((p_frame->info.width % 4 != 0) || \
    (p_frame->info.height % 4 != 0) || \
    (p_frame->frame[0].plane[IY].stride != \
      p_frame->frame[0].plane[IC].stride) || \
    (p_frame->info.ss != IMG_H2V2)) { \
    IDBG_ERROR("%s:%d]: Error index %d", __func__, __LINE__, i); \
    rc = IMG_ERR_GENERAL; \
  } \
  rc; \
})

typedef struct
{
  void *ptr;
  int (*hdrTwoFrameCore)(hdr_config_t *pIn, hdr_return_t *pReturnCode);
  int (*hdrSingleFrameCore)(hdr_config_t *pIn, hdr_return_t *pReturnCode);
} hdr_lib_info_t;

static hdr_lib_info_t g_hdr_lib;

static int hdr_calc_inverse_gamma(hdr_gamma_table_struct_t *gamma,
  uint32_t *p_inverse_gamma);
static void hdr_calc_new_gamma(hdr_gamma_table_struct_t *gamma,
  uint32_t * new_gamma_tbl);

/**
 * Function: hdr_calculate_gammatbl
 *
 * Description: Calculates the gamma tables
 *
 * Input parameters:
 *   p_comp - The pointer to the component object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *     IMG_ERR_NO_MEMORY
 *
 * Notes: none
 **/
int hdr_calculate_gammatbl(hdr_comp_t *p_comp)
{
  int rc = 0;
  //Program  gamma tables accordingly
  if ((p_comp->gamma.gamma_t == GAMMA_TBL_ALL)) {
    memcpy(p_comp->g_hdr_gamma.red_gamma_table,
      p_comp->gamma.gamma_tbl,
      GAMMA_TABLE_SIZE_HDR*sizeof(uint16_t));
    memcpy(p_comp->g_hdr_gamma.green_gamma_table,
      p_comp->gamma.gamma_tbl,
      GAMMA_TABLE_SIZE_HDR*sizeof(uint16_t));
    memcpy(p_comp->g_hdr_gamma.blue_gamma_table,
      p_comp->gamma.gamma_tbl,
      GAMMA_TABLE_SIZE_HDR*sizeof(uint16_t));
    p_comp->g_hdr_gamma.hdr_gamma_R.gamma_tbl =
      p_comp->g_hdr_gamma.red_gamma_table;
    p_comp->g_hdr_gamma.hdr_gamma_G.gamma_tbl =
      p_comp->g_hdr_gamma.green_gamma_table;
    p_comp->g_hdr_gamma.hdr_gamma_B.gamma_tbl =
      p_comp->g_hdr_gamma.blue_gamma_table;
    //allocate memory for inverse gamma and new gamma correction tables
    if (!p_comp->param.mpRedInverseGammatable)
      p_comp->param.mpRedInverseGammatable = (uint32_t *)malloc(
        256 * sizeof(uint32_t));
    if (!p_comp->param.mpGreenInverseGammatable)
      p_comp->param.mpGreenInverseGammatable = (uint32_t *)malloc(
        256 * sizeof(uint32_t));
    if (!p_comp->param.mpBlueInverseGammatable)
      p_comp->param.mpBlueInverseGammatable = (uint32_t *)malloc(
        256 * sizeof(uint32_t));
    if (!p_comp->param.mpRedNewGammatable)
      p_comp->param.mpRedNewGammatable = (uint32_t *)malloc(
        MAX_GAMMA_INTERPOLATED * sizeof(uint32_t));
    if (!p_comp->param.mpGreenNewGammatable)
      p_comp->param.mpGreenNewGammatable = (uint32_t *)malloc(
        MAX_GAMMA_INTERPOLATED * sizeof(uint32_t));
    if (!p_comp->param.mpBlueNewGammatable)
      p_comp->param.mpBlueNewGammatable = (uint32_t *)malloc(
        MAX_GAMMA_INTERPOLATED * sizeof(uint32_t));

    if (p_comp->param.mpRedInverseGammatable == NULL
      || p_comp->param.mpGreenInverseGammatable == NULL
      || p_comp->param.mpBlueInverseGammatable == NULL
      || p_comp->param.mpRedNewGammatable == NULL
      || p_comp->param.mpGreenNewGammatable == NULL
      || p_comp->param.mpBlueNewGammatable == NULL) {

      if (p_comp->param.mpRedInverseGammatable) {
        free(p_comp->param.mpRedInverseGammatable);
        p_comp->param.mpRedInverseGammatable = NULL;
      }
      if (p_comp->param.mpGreenInverseGammatable) {
        free(p_comp->param.mpGreenInverseGammatable);
        p_comp->param.mpGreenInverseGammatable = NULL;
      }
      if (p_comp->param.mpBlueInverseGammatable) {
        free(p_comp->param.mpBlueInverseGammatable);
        p_comp->param.mpBlueInverseGammatable = NULL;
      }
      if (p_comp->param.mpRedNewGammatable) {
        free(p_comp->param.mpRedNewGammatable);
        p_comp->param.mpRedNewGammatable = NULL;
      }
      if (p_comp->param.mpGreenNewGammatable) {
        free(p_comp->param.mpGreenNewGammatable);
        p_comp->param.mpGreenNewGammatable = NULL;
      }
      if (p_comp->param.mpBlueNewGammatable) {
        free(p_comp->param.mpBlueNewGammatable);
        p_comp->param.mpBlueNewGammatable = NULL;
      }
      IDBG_ERROR("%s:%d] Gamma table malloc failed for GAMMA_TABLE_ALL",
        __func__, __LINE__);
      return IMG_ERR_NO_MEMORY;
    }

    //For R
    rc = hdr_calc_inverse_gamma(&(p_comp->gamma),
      p_comp->param.mpRedInverseGammatable);
    if (rc != IMG_SUCCESS) {
      if (p_comp->param.mpRedInverseGammatable) {
        free(p_comp->param.mpRedInverseGammatable);
        p_comp->param.mpRedInverseGammatable = NULL;
      }
      if (p_comp->param.mpGreenInverseGammatable) {
        free(p_comp->param.mpGreenInverseGammatable);
        p_comp->param.mpGreenInverseGammatable = NULL;
      }
      if (p_comp->param.mpBlueInverseGammatable) {
        free(p_comp->param.mpBlueInverseGammatable);
        p_comp->param.mpBlueInverseGammatable = NULL;
      }
      if (p_comp->param.mpRedNewGammatable) {
        free(p_comp->param.mpRedNewGammatable);
        p_comp->param.mpRedNewGammatable = NULL;
      }
      if (p_comp->param.mpGreenNewGammatable) {
        free(p_comp->param.mpGreenNewGammatable);
        p_comp->param.mpGreenNewGammatable = NULL;
      }
      if (p_comp->param.mpBlueNewGammatable) {
        free(p_comp->param.mpBlueNewGammatable);
        p_comp->param.mpBlueNewGammatable = NULL;
      }
      IDBG_ERROR("%s:%d] calculate inverse red failed", __func__, __LINE__);
      return IMG_ERR_GENERAL;
    }
    hdr_calc_new_gamma(&(p_comp->gamma), p_comp->param.mpRedNewGammatable);
    //For G
    memcpy(p_comp->param.mpGreenInverseGammatable,
      p_comp->param.mpRedInverseGammatable, 256 * sizeof(uint32_t));
    memcpy(p_comp->param.mpGreenNewGammatable, p_comp->param.mpRedNewGammatable,
      MAX_GAMMA_INTERPOLATED * sizeof(uint32_t));
    //For B
    memcpy(p_comp->param.mpBlueInverseGammatable,
      p_comp->param.mpRedInverseGammatable, 256 * sizeof(uint32_t));
    memcpy(p_comp->param.mpBlueNewGammatable, p_comp->param.mpRedNewGammatable,
      MAX_GAMMA_INTERPOLATED * sizeof(uint32_t));

  } else {
    if (p_comp->gamma.gamma_t == GAMMA_TBL_R) {
      //For R
      if (!p_comp->param.mpRedInverseGammatable)
        p_comp->param.mpRedInverseGammatable = (uint32_t *)malloc(
          256 * sizeof(uint32_t));
      if (!p_comp->param.mpRedNewGammatable)
        p_comp->param.mpRedNewGammatable = (uint32_t *)malloc(
          MAX_GAMMA_INTERPOLATED * sizeof(uint32_t));
      if (p_comp->param.mpRedInverseGammatable == NULL
        || p_comp->param.mpRedNewGammatable == NULL) {
        if (p_comp->param.mpRedInverseGammatable) {
          free(p_comp->param.mpRedInverseGammatable);
          p_comp->param.mpRedInverseGammatable = NULL;
        }
        if (p_comp->param.mpRedNewGammatable) {
          free(p_comp->param.mpRedNewGammatable);
          p_comp->param.mpRedNewGammatable = NULL;
        }
        return IMG_ERR_NO_MEMORY;
      }
      memcpy(p_comp->g_hdr_gamma.red_gamma_table,
        p_comp->gamma.gamma_tbl,
        GAMMA_TABLE_SIZE_HDR*sizeof(uint16_t));

      p_comp->g_hdr_gamma.hdr_gamma_R.gamma_tbl =
        p_comp->g_hdr_gamma.red_gamma_table;

      rc = hdr_calc_inverse_gamma(&(p_comp->gamma),
        p_comp->param.mpRedInverseGammatable);
      if (rc != IMG_SUCCESS) {
        if (p_comp->param.mpRedInverseGammatable) {
          free(p_comp->param.mpRedInverseGammatable);
          p_comp->param.mpRedInverseGammatable = NULL;
        }
        if (p_comp->param.mpRedNewGammatable) {
          free(p_comp->param.mpRedNewGammatable);
          p_comp->param.mpRedNewGammatable = NULL;
        }
        IDBG_ERROR("%s calculate inverse red 1 failed", __func__);
        return IMG_ERR_GENERAL;
      }
      hdr_calc_new_gamma(&(p_comp->gamma), p_comp->param.mpRedNewGammatable);
    } else if (p_comp->gamma.gamma_t == GAMMA_TBL_G) {
      //For G
      if (!p_comp->param.mpGreenInverseGammatable)
        p_comp->param.mpGreenInverseGammatable = (uint32_t *)malloc(
          256 * sizeof(uint32_t));
      if (!p_comp->param.mpGreenNewGammatable)
        p_comp->param.mpGreenNewGammatable = (uint32_t *)malloc(
          MAX_GAMMA_INTERPOLATED * sizeof(uint32_t));
      if (p_comp->param.mpGreenInverseGammatable == NULL
        || p_comp->param.mpGreenNewGammatable == NULL) {
        if (p_comp->param.mpGreenInverseGammatable) {
          free(p_comp->param.mpGreenInverseGammatable);
          p_comp->param.mpGreenInverseGammatable = NULL;
        }
        if (p_comp->param.mpGreenNewGammatable) {
          free(p_comp->param.mpGreenNewGammatable);
          p_comp->param.mpGreenNewGammatable = NULL;
        }
        return IMG_ERR_NO_MEMORY;
      }
      memcpy(p_comp->g_hdr_gamma.green_gamma_table,
        p_comp->gamma.gamma_tbl,
        GAMMA_TABLE_SIZE_HDR*sizeof(uint16_t));

      p_comp->g_hdr_gamma.hdr_gamma_G.gamma_tbl =
        p_comp->g_hdr_gamma.green_gamma_table;
      rc = hdr_calc_inverse_gamma(&(p_comp->gamma),
        p_comp->param.mpGreenInverseGammatable);
      if (rc != IMG_SUCCESS) {
        if (p_comp->param.mpGreenInverseGammatable) {
          free(p_comp->param.mpGreenInverseGammatable);
          p_comp->param.mpGreenInverseGammatable = NULL;
        }
        if (p_comp->param.mpGreenNewGammatable) {
          free(p_comp->param.mpGreenNewGammatable);
          p_comp->param.mpGreenNewGammatable = NULL;
        }
        IDBG_ERROR("%s:%d] calculate inverse green failed", __func__, __LINE__);
        return IMG_ERR_GENERAL;
      }
      hdr_calc_new_gamma(&(p_comp->gamma), p_comp->param.mpGreenNewGammatable);
    } else if (p_comp->gamma.gamma_t == GAMMA_TBL_B) {
      //For B
      if (!p_comp->param.mpBlueInverseGammatable)
        p_comp->param.mpBlueInverseGammatable = (uint32_t *)malloc(
          256 * sizeof(uint32_t));
      if (!p_comp->param.mpBlueNewGammatable)
        p_comp->param.mpBlueNewGammatable = (uint32_t *)malloc(
          MAX_GAMMA_INTERPOLATED * sizeof(uint32_t));
      if (p_comp->param.mpBlueInverseGammatable == NULL
        || p_comp->param.mpBlueNewGammatable == NULL) {
        if (p_comp->param.mpBlueInverseGammatable) {
          free(p_comp->param.mpBlueInverseGammatable);
          p_comp->param.mpBlueInverseGammatable = NULL;
        }
        if (p_comp->param.mpBlueNewGammatable) {
          free(p_comp->param.mpBlueNewGammatable);
          p_comp->param.mpBlueNewGammatable = NULL;
        }
        return IMG_ERR_NO_MEMORY;
      }
      memcpy(p_comp->g_hdr_gamma.blue_gamma_table,
        p_comp->gamma.gamma_tbl,
        GAMMA_TABLE_SIZE_HDR*sizeof(uint16_t));

      p_comp->g_hdr_gamma.hdr_gamma_B.gamma_tbl =
        p_comp->g_hdr_gamma.blue_gamma_table;
      rc = hdr_calc_inverse_gamma(&(p_comp->gamma),
        p_comp->param.mpBlueInverseGammatable);
      if (rc != IMG_SUCCESS) {
        if (p_comp->param.mpBlueInverseGammatable) {
          free(p_comp->param.mpBlueInverseGammatable);
          p_comp->param.mpBlueInverseGammatable = NULL;
        }
        if (p_comp->param.mpBlueNewGammatable) {
          free(p_comp->param.mpBlueNewGammatable);
          p_comp->param.mpBlueNewGammatable = NULL;
        }
        IDBG_ERROR("%s:%d] calculate inverse red failed", __func__, __LINE__);
        return IMG_ERR_GENERAL;
      }
      hdr_calc_new_gamma(&(p_comp->gamma), p_comp->param.mpBlueNewGammatable);

    }
  }
  return IMG_SUCCESS;
}

/**
 * Function: hdr_calc_inverse_gamma
 *
 * Description: Calculates the inverse gamma table
 *
 * Input parameters:
 *   gamma - The input gamma table
 *   p_inverse_gamma - The pointer to the inverse table which is created
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int hdr_calc_inverse_gamma(hdr_gamma_table_struct_t *gamma,
  uint32_t *p_inverse_gamma)
{
  uint16_t *pGammaTableValues;
  uint16_t gammavalue, gammadelta;
  uint32_t tempnew, tempold, i, j, k, index;
  uint32_t numberOfGammaMatches, totalCount;

  pGammaTableValues = (uint16_t *)(gamma->gamma_tbl);
  numberOfGammaMatches = 0;
  totalCount = 0;
  k = 0;
  tempold = 0;
  tempnew = 0;
  //Interpolate gamma table
  for (i = 0; i < 64; i++) {
    gammavalue = (*pGammaTableValues) & 0xff;
    gammadelta = (*pGammaTableValues++) >> 8;

    for (j = 0; j < 16; j++) {
      index = i * 16 + j;
      tempnew = gammavalue + ((gammadelta * j + 8) >> 4);
      tempnew = MIN(tempnew, 255);
      if (index != 0) {
        if (tempnew == tempold) {
          numberOfGammaMatches += index;
          totalCount = totalCount + 1;
        } else {
          if (k < 256) {
            p_inverse_gamma[k] = (numberOfGammaMatches + (totalCount >> 1))
              / totalCount;
            k = k + 1;
            numberOfGammaMatches = 0;
            totalCount = 0;
            numberOfGammaMatches += index;
            totalCount = totalCount + 1;
          } else {
            IDBG_ERROR("%s:%d] Gamma table inverse calculation failed \n",
              __func__, __LINE__);
            return IMG_ERR_GENERAL;
          }
        }
        tempold = tempnew;
      } else {
        tempold = tempnew;
        numberOfGammaMatches += index;
        totalCount = totalCount + 1;
      }

    }
  }
  p_inverse_gamma[0] = 0;
  for (i = k; i < 256; i++) {
    p_inverse_gamma[i] = (numberOfGammaMatches + (totalCount >> 1))
      / totalCount;
    if (p_inverse_gamma[i] > 1023) {
      IDBG_ERROR("p_inverse_gamma[%d] %d", i, p_inverse_gamma[i]);
      return IMG_ERR_GENERAL;
    }
  }
  p_inverse_gamma[255] = 1023;
  return IMG_SUCCESS;
}

/**
 * Function: hdr_calc_new_gamma
 *
 * Description: Calculates the new gamma table
 *
 * Input parameters:
 *   gamma - The input gamma table
 *   new_gamma_tbl - The pointer to the new gamma table which is created
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void hdr_calc_new_gamma(hdr_gamma_table_struct_t *gamma,
  uint32_t * new_gamma_tbl)
{
  uint16_t *pGammaTableValues;
  uint16_t gammavalue, gammadelta = 0;
  uint32_t i, j, index;

#ifdef HDR_LIB_GHOSTBUSTER
  int32_t origrefvalue,difference;

  pGammaTableValues = (uint16_t *)(gamma->gamma_tbl);

  //Interpolate gamma table 0-4095 values
  for (i = 0; i < 64; i++) {
    gammavalue = (*pGammaTableValues) & 0xff;
    gammadelta = (*pGammaTableValues++) >> 8;
    IDBG_ERROR("gamma value %d %d", gammavalue,gammadelta);

    for (j = 0; j < 64; j++) {
      index = i * 64 + j;
      new_gamma_tbl[index] = (gammavalue << 2) + ((gammadelta * j + 8) >> 4);
      new_gamma_tbl[index] = MIN(new_gamma_tbl[index], 1023);
    }
  }
  origrefvalue = new_gamma_tbl[2047];
#endif

  pGammaTableValues = (uint16_t *)(gamma->gamma_tbl);

  //Interpolate gamma table 0-2047 values
  for (i = 0; i < 64; i++) {
    gammavalue = (*pGammaTableValues) & 0xff;
    gammadelta = (*pGammaTableValues++) >> 8;
    for (j = 0; j < 32; j++) {
      index = i * 32 + j;
      new_gamma_tbl[index] = (gammavalue << 2) + ((gammadelta * j + 4) >> 3);
      new_gamma_tbl[index] = MIN(new_gamma_tbl[index], 1023);
    }
  }

#ifdef HDR_LIB_GHOSTBUSTER
  //Calculate difference between the newGammatable[2047] and original gamma table, and add this difference
  //to each value from 2048-4095
  difference = new_gamma_tbl[2047] - origrefvalue;
  //interpolate 2048-4095 values
  for (i = 2048; i < 4096; i++) {
    new_gamma_tbl[i] = new_gamma_tbl[i] + difference;
  }
#else
  // we need to find out gamma value of 2048 index
  // considering gamma table interpolated to 4096
  pGammaTableValues = (uint16_t*)(gamma->gamma_tbl);
  pGammaTableValues = pGammaTableValues + 32;
  gammadelta = (*pGammaTableValues) >> 8;

  index = 2048;
  //interpolate 2048-4095 values
  for (i = 0; i < 2048; i++) {
    new_gamma_tbl[index] = new_gamma_tbl[2047] + ((gammadelta * i + 8) >> 4);
    index++;
  }
#endif

  //Normalize gamma table
  for (i = 0; i < 4096; i++) {
    new_gamma_tbl[i] = new_gamma_tbl[i] * 1023 / new_gamma_tbl[4095];
    if (new_gamma_tbl[i] > 1023) {
      IDBG_ERROR("new_gamma_tbl[%d] %d is more than 1023",
        i, new_gamma_tbl[i]);
    }
  }
}

/**
 * Function: hdr_comp_init
 *
 * Description: Initializes the Qualcomm HDR component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_userdata - the handle which is passed by the client
 *   p_data - The pointer to the parameter which is required during the
 *            init phase
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int hdr_comp_init(void *handle, void* p_userdata, void *p_data)
{
  hdr_comp_t *p_comp = (hdr_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] p_comp %p", __func__, __LINE__, p_comp);
  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (status < 0)
    return status;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: hdr_comp_deinit
 *
 * Description: Un-initializes the HDR component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int hdr_comp_deinit(void *handle)
{
  hdr_comp_t *p_comp = (hdr_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    return IMG_ERR_GENERAL;
  }
  status = p_comp->b.ops.deinit(&p_comp->b);
  if (status < 0)
    return status;

  if (p_comp->param.mpRedInverseGammatable) {
    free(p_comp->param.mpRedInverseGammatable);
    p_comp->param.mpRedInverseGammatable = NULL;
  }
  if (p_comp->param.mpGreenInverseGammatable) {
    free(p_comp->param.mpGreenInverseGammatable);
    p_comp->param.mpGreenInverseGammatable = NULL;
  }
  if (p_comp->param.mpBlueInverseGammatable) {
    free(p_comp->param.mpBlueInverseGammatable);
    p_comp->param.mpBlueInverseGammatable = NULL;
  }
  if (p_comp->param.mpRedNewGammatable) {
    free(p_comp->param.mpRedNewGammatable);
    p_comp->param.mpRedNewGammatable = NULL;
  }
  if (p_comp->param.mpGreenNewGammatable) {
    free(p_comp->param.mpGreenNewGammatable);
    p_comp->param.mpGreenNewGammatable = NULL;
  }
  if (p_comp->param.mpBlueNewGammatable) {
    free(p_comp->param.mpBlueNewGammatable);
    p_comp->param.mpBlueNewGammatable = NULL;
  }
  free(p_comp);
  p_comp = NULL;
  return IMG_SUCCESS;
}

/**
 * Function: hdr_fill_config
 *
 * Description: Fills the HDR library structure with the calculated values
 *
 * Input parameters:
 *   p_comp - The pointer to the component object
 *   p_config - The pointer to the HDL lib config
 *   p_frame - Array of frame pointers which needs to be filled
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void hdr_fill_config(hdr_comp_t *p_comp, hdr_config_t *p_config,
  img_frame_t *p_frame[MAX_HDR_FRAMES])
{
  int i = 0;
  p_config->imageWidth = p_frame[0]->frame->plane[IY].stride;
  p_config->imageHeight = p_frame[0]->frame->plane[IY].height;
  p_config->subSampleFormat = HDR_H2V2;
  p_config->chromaOrder = YCRCB;
  p_config->pHdrBuffer1Y = p_frame[0]->frame[0].plane[IY].addr
    + p_frame[0]->frame[0].plane[IY].offset;
  p_config->pHdrBuffer1C = p_frame[0]->frame[0].plane[IC].addr
    + p_frame[0]->frame[0].plane[IC].offset;

  /*TODO: Check if there is a need to restrict processing to active region of frame
   * If yes, should be able to specify start/end of processing of row and column in this frame
   */

  if (p_comp->analyse_image) {
    p_config->thumbMainIndicator = 1;
    p_config->calculatedExposureRatioG = 0;
  } else {
    p_config->thumbMainIndicator = 0;
    p_config->calculatedExposureRatioG =
      p_comp->param.mCalculatedExposureRatioG;
  }
  if (p_comp->hdr_chromatix.enable) {
    p_config->contrastControl =
      p_comp->hdr_chromatix.hdr_contrast_control;
    p_config->chromaSat_wgt     = (float)((p_comp->hdr_chromatix.hdr_chromaSat_wgt))/10.0f;
    p_config->chromaSat_clamp   =
      p_comp->hdr_chromatix.hdr_chromaSat_clamp;
    p_config->chromaSat_shift   =
      p_comp->hdr_chromatix.hdr_chromaSat_shift;
  }  else {
      /* By Default, Color saturation control is set to mild level of desaturation */
      p_config->chromaSat_wgt   = 0.5f;
      p_config->chromaSat_clamp = 90;
      p_config->chromaSat_shift = 5;
  }
  if (p_comp->mode == MULTI_FRAME) {
    p_config->maxLag = 200;
    p_config->pRedGammaInvTable = p_comp->param.mpRedInverseGammatable;
    p_config->pGreenGammaInvTable = p_comp->param.mpGreenInverseGammatable;
    p_config->pBlueGammaInvTable = p_comp->param.mpBlueInverseGammatable;
    p_config->pRedGammaTable = p_comp->param.mpRedNewGammatable;
    p_config->pGreenGammaTable = p_comp->param.mpGreenNewGammatable;
    p_config->pBlueGammaTable = p_comp->param.mpBlueNewGammatable;
    p_config->pGammaStruct = &p_comp->g_hdr_gamma.hdr_gamma_R;
    p_config->pGammaStructG = &p_comp->g_hdr_gamma.hdr_gamma_G;
    p_config->pGammaStructB = &p_comp->g_hdr_gamma.hdr_gamma_B;
    p_config->pHdrBuffer2Y = p_frame[1]->frame[0].plane[IY].addr
      + p_frame[1]->frame[0].plane[IY].offset;
    p_config->pHdrBuffer2C = p_frame[1]->frame[0].plane[IC].addr
      + p_frame[1]->frame[0].plane[IC].offset;
#ifdef HDR_LIB_GHOSTBUSTER
    p_config->pHdrBuffer3Y = p_frame[2]->frame[0].plane[IY].addr
      + p_frame[2]->frame[0].plane[IY].offset;
    p_config->pHdrBuffer3C = p_frame[2]->frame[0].plane[IC].addr
      + p_frame[2]->frame[0].plane[IC].offset;
#endif

    /*TODO: Check if there is a need to restrict processing to active region of frame
     * If yes, should be able to specify start/end of processing of row and column in this frame
     */
  }
}

/**
 * Function: hdr_comp_execute
 *
 * Description: Executes the HDR algorithm
 *
 * Input parameters:
 *   p_comp - The pointer to the component object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int hdr_comp_execute(hdr_comp_t *p_comp)
{
  hdr_return_t rc = HDR_SUCESS;
  hdr_config_t *p_config = &(p_comp->structHdrConfig);
  int i = 0;

  memset(p_config, 0, sizeof(hdr_config_t));
  if (p_comp->mode == SINGLE_FRAME) {
    if (p_comp->analyse_image) {
      hdr_fill_config(p_comp, p_config, p_comp->p_analysis_frame);
      g_hdr_lib.hdrSingleFrameCore(p_config, &rc);

      if (HDR_SUCESS != rc) {
        IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
        return IMG_ERR_INVALID_INPUT;
      }
    }
    hdr_fill_config(p_comp, p_config, p_comp->p_main_frame);
    g_hdr_lib.hdrSingleFrameCore(p_config, &rc);

    if (HDR_SUCESS != rc) {
      IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
      return IMG_ERR_INVALID_INPUT;
    }
  } else { /* MULTIFRAME */
    /* Apply MULTIFRAME frame HDR */
    if (p_comp->analyse_image) {
      hdr_fill_config(p_comp, p_config, p_comp->p_analysis_frame);
      g_hdr_lib.hdrTwoFrameCore(p_config, &rc);

      if (HDR_SUCESS != rc) {
        IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
        return IMG_ERR_INVALID_INPUT;
      }
    }
    p_comp->param.mCalculatedExposureRatioG =
      p_config->calculatedExposureRatioG;
    hdr_fill_config(p_comp, p_config, p_comp->p_main_frame);
    g_hdr_lib.hdrTwoFrameCore(p_config, &rc);

    if (HDR_SUCESS != rc) {
      IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
#ifdef HDR_LIB_GHOSTBUSTER
      p_config->cropdimension.dx = 0;
      p_config->cropdimension.dy = 0;
      p_config->cropdimension.x = 0;
      p_config->cropdimension.y = 0;

      if (IMG_SUCCESS != img_image_copy(p_comp->p_main_frame[0],
        p_comp->p_main_frame[2])) {
          IDBG_ERROR("%s:%d] HDR library image copy error",
            __func__, __LINE__);
      }
#endif
    }

#ifdef HDR_LIB_GHOSTBUSTER
    p_comp->out_crop.height = p_config->cropdimension.dy;
    p_comp->out_crop.width = p_config->cropdimension.dx;
    p_comp->out_crop.start_x = p_config->cropdimension.x;
    p_comp->out_crop.start_y = p_config->cropdimension.y;

    if (p_comp->out_crop.width + p_comp->out_crop.start_x >
      (unsigned int)p_comp->p_main_frame[0]->info.width) {
        p_comp->out_crop.width =
          p_comp->p_main_frame[0]->info.width - 2 * p_comp->out_crop.start_x;
        if (p_comp->out_crop.width >
          (unsigned int)p_comp->p_main_frame[0]->info.width) {
            memset(&p_comp->out_crop, 0, sizeof(hdr_crop_t));
            rc = HDR_ERROR;
            IDBG_ERROR("%s:%d] HDR library crop output is wrong",
              __func__, __LINE__);
        }
    }
#endif

    if (HDR_SUCESS != rc) {
      IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
      return IMG_ERR_INVALID_INPUT;
    }
  }
  return IMG_SUCCESS;
}

/**
 * Function: hdr_thread_loop
 *
 * Description: Main algorithm thread loop
 *
 * Input parameters:
 *   data - The pointer to the component object
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
void *hdr_thread_loop(void *data)
{
  hdr_comp_t *p_comp = (hdr_comp_t *)data;
  img_component_t *p_base = (img_component_t *)&p_comp->b;
  int status = 0;
  int i = 0;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  status = hdr_comp_execute(p_comp);
  IDBG_HIGH("%s:%d] status %d analyse %d main_cnt %d", __func__, __LINE__,
    status, p_comp->analyse_image, p_comp->main_count);

  pthread_mutex_lock(&p_base->mutex);
  /* Don't send back buffers on abort */
  if (IMG_STATE_STOP_REQUESTED == p_base->state) {
    pthread_mutex_unlock(&p_base->mutex);
    p_base->state = IMG_STATE_STOPPED;
    return NULL;
  }
  pthread_mutex_unlock(&p_base->mutex);

  /* send all the buffers back */
  for (i = 0; i < p_comp->main_count; i++) {
    img_q_enqueue(&p_base->outputQ, p_comp->p_main_frame[i]);
    IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
    if (p_comp->analyse_image) {
      img_q_enqueue(&p_base->outputQ, p_comp->p_analysis_frame[i]);
      IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
    }
  }

  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_STOPPED;
  pthread_mutex_unlock(&p_base->mutex);
  if (status < 0)
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_ERROR, status, status);
  else
    IMG_SEND_EVENT(p_base, QIMG_EVT_DONE);
  return NULL;
}

/**
 * Function: hdr_comp_set_param
 *
 * Description: Set HDR parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in hdr.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int hdr_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  hdr_comp_t *p_comp = (hdr_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  switch (param) {
  case QHDR_GAMMA_TABLE: {
    img_gamma_t *p_gamma = (img_gamma_t *)p_data;

    if (NULL == p_gamma) {
      IDBG_ERROR("%s:%d] invalid gamma table", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->gamma.gamma_tbl = p_gamma->table;
    p_comp->gamma.entry = GAMMA_TABLE_ENTRIES;
    p_comp->gamma.gamma_t = GAMMA_TBL_ALL;

    status = hdr_calculate_gammatbl(p_comp);

    if (status < 0) {
      IDBG_ERROR("%s:%d] invalid gamma table while parse", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
  }
    break;
  case QHDR_ANALYZE_IMAGE: {
    int *p_info = (int *)p_data;

    if (NULL == p_info) {
      IDBG_ERROR("%s:%d] invalid info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->analyse_image = *p_info;
    IDBG_HIGH("%s:%d] analyse_image %d", __func__, __LINE__,
      p_comp->analyse_image);
  }
    break;
  case QHDR_MODE: {
    hdr_mode_t *p_mode = (hdr_mode_t *)p_data;

    if (NULL == p_mode) {
      IDBG_ERROR("%s:%d] invalid info", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->mode = *p_mode;
    IDBG_HIGH("%s:%d] mode %d", __func__, __LINE__, p_comp->mode);
  }
    break;
  case QHDR_OUT_INDEX: {
    int *p_info = (int *)p_data;

    if (NULL == p_info) {
      IDBG_ERROR("%s:%d] invalid out index", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->out_index = *p_info;
    IDBG_HIGH("%s:%d] out_index %d", __func__, __LINE__, p_comp->out_index);
  }
    break;
   case QHDR_HDR_CHROMATIX: {
    hdr_chromatix_t *p_chromatix = (hdr_chromatix_t *)p_data;

    if (NULL == p_chromatix) {
      IDBG_ERROR("%s:%d] invalid hdr chromatix", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->hdr_chromatix = *p_chromatix;
  }
    break;
  default:
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: hdr_comp_get_param
 *
 * Description: Gets HDR parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in hdr.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int hdr_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  hdr_comp_t *p_comp = (hdr_comp_t *)handle;
  int status = IMG_SUCCESS;
  hdr_crop_t *out_crop = p_data;

  if (NULL == p_data) {
    IDBG_ERROR("%s:%d] invalid user data", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  switch (param) {
  case QHDR_OUT_CROP: {
    *out_crop = p_comp->out_crop;
    break;
  }
  default:
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: hdr_comp_start
 *
 * Description: Start the execution of HDR
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in hdr.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int hdr_comp_start(void *handle, void *p_data)
{
  hdr_comp_t *p_comp = (hdr_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame;
  img_frame_t **pp_frame;
  int count = 0;
  int main_idx = 0, thumb_idx = 0;
  int i = 0;

  if (NULL == g_hdr_lib.ptr) {
    IDBG_ERROR("%s:%d] library not loaded", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  IDBG_MED("%s:%d] mode %d", __func__, __LINE__, p_comp->mode);
  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT) || (NULL == p_base->thread_loop)) {
    IDBG_ERROR("%s:%d] Error state %d", __func__, __LINE__, p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  p_comp->main_count = (p_comp->mode == SINGLE_FRAME) ? 1 : MAX_HDR_FRAMES;
  count = (p_comp->analyse_image) ? p_comp->main_count * 2 : p_comp->main_count;

  p_comp->count = img_q_count(&p_base->inputQ);
  IDBG_MED("%s:%d] num buffers %d count %d", __func__, __LINE__, p_comp->count,
    count);
  if (count != p_comp->count) {
    IDBG_ERROR("%s:%d] Error buffers not sufficient", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_GENERAL;
  }

  for (i = 0; i < p_comp->count; i++) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if ((main_idx >= MAX_HDR_FRAMES) || (thumb_idx >= MAX_HDR_FRAMES)
      || (NULL == p_frame)) {
      IDBG_ERROR("%s:%d] Error invalid buffers", __func__, __LINE__);
      pthread_mutex_unlock(&p_base->mutex);
      return IMG_ERR_GENERAL;
    }

    status = img_image_stride_fill(p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] image stride fill error %d",
        __func__, __LINE__, status);
      pthread_mutex_unlock(&p_base->mutex);
      return status;
    }

    IDBG_MED("%s:%d] frame %dx%d 0x%x", __func__, __LINE__, p_frame->info.width,
      p_frame->info.height, (uint32_t)p_frame);
    pp_frame =
        (p_frame->info.analysis) ?
          &p_comp->p_analysis_frame[thumb_idx++] :
          &p_comp->p_main_frame[main_idx++];
    *pp_frame = p_frame;
  }

  for (i = 0; i < p_comp->main_count; i++) {
    IDBG_MED("%s:%d] frame 0x%x", __func__, __LINE__,
      (uint32_t)p_comp->p_main_frame[i]);
    IDBG_MED("%s:%d] main image size %dx%d", __func__, __LINE__,
      p_comp->p_main_frame[i]->info.width,
      p_comp->p_main_frame[i]->info.height);
    if ((status = hdr_validate_image(p_comp->p_main_frame[i], i)) < 0) {
      pthread_mutex_unlock(&p_base->mutex);
      return status;
    }
  }

  if (p_comp->analyse_image) {
    for (i = 0; i < p_comp->main_count; i++) {
      IDBG_MED("%s:%d] Analysis image size %dx%d", __func__, __LINE__,
        p_comp->p_analysis_frame[i]->info.width,
        p_comp->p_analysis_frame[i]->info.height);
      if ((status = hdr_validate_image(p_comp->p_analysis_frame[i], i)) < 0) {
        pthread_mutex_unlock(&p_base->mutex);
        return status;
      }
    }
  }

  p_comp->out_crop.height = p_comp->p_main_frame[0]->info.height;
  p_comp->out_crop.width = p_comp->p_main_frame[0]->info.width;
  p_comp->out_crop.start_x = 0;
  p_comp->out_crop.start_y = 0;

  pthread_mutex_unlock(&p_base->mutex);

  status = p_comp->b.ops.start(&p_comp->b, p_data);

  return status;
}

/**
 * Function: hdr_comp_abort
 *
 * Description: Aborts the execution of HDR
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type is defined in hdr.h
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int hdr_comp_abort(void *handle, void *p_data)
{
  hdr_comp_t *p_comp = (hdr_comp_t *)handle;
  int status;

  status = p_comp->b.ops.abort(&p_comp->b, p_data);
  if (status < 0)
    return status;

  return 0;
}

/**
 * Function: hdr_comp_process
 *
 * Description: This function is used to send any specific commands for the
 *              HDR component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   cmd - The command type which needs to be processed
 *   p_data - The pointer to the command payload
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int hdr_comp_process(void *handle, img_cmd_type cmd, void *p_data)
{
  hdr_comp_t *p_comp = (hdr_comp_t *)handle;
  int status;

  status = p_comp->b.ops.process(&p_comp->b, cmd, p_data);
  if (status < 0)
    return status;

  return 0;
}

/**
 * Function: hdr_comp_create
 *
 * Description: This function is used to create Qualcomm HDR component
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_ops - The pointer to img_component_t object. This object
 *            contains the handle and the function pointers for
 *            communicating with the imaging component.
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int hdr_comp_create(void* handle, img_component_ops_t *p_ops)
{
  hdr_comp_t *p_comp;
  int status;

  if (NULL == p_ops) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  if (NULL == g_hdr_lib.ptr) {
    IDBG_ERROR("%s:%d] Error library not loaded", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  p_comp = (hdr_comp_t *)malloc(sizeof(hdr_comp_t));

  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  memset(p_comp, 0x0, sizeof(hdr_comp_t));
  status = img_comp_create(&p_comp->b);
  if (status < 0) {
    free(p_comp);
    return status;
  }

  /*set the main thread*/
  p_comp->b.thread_loop = hdr_thread_loop;
  p_comp->b.p_core = p_comp;

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init = hdr_comp_init;
  p_ops->deinit = hdr_comp_deinit;
  p_ops->set_parm = hdr_comp_set_param;
  p_ops->get_parm = hdr_comp_get_param;
  p_ops->start = hdr_comp_start;
  p_ops->abort = hdr_comp_abort;
  p_ops->process = hdr_comp_process;

  p_ops->handle = (void *)p_comp;
  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: hdr_comp_load
 *
 * Description: This function is used to load Qualcomm HDR component
 *
 * Input parameters:
 *   @name: library name
 *   @handle: library handle
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int hdr_comp_load(const char* name, void* handle)
{
  if (g_hdr_lib.ptr) {
    IDBG_ERROR("%s:%d] library already loaded", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  g_hdr_lib.ptr = dlopen(HDR_COMP_LIB_NAME, RTLD_NOW);
  if (!g_hdr_lib.ptr) {
    IDBG_ERROR("%s:%d] Error opening hdr library", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_hdr_lib.hdrSingleFrameCore) = dlsym(g_hdr_lib.ptr,
    "hdrSingleFrameCore");
  if (!g_hdr_lib.hdrSingleFrameCore) {
    IDBG_ERROR("%s:%d] Error linking hdrSingleFrameCore", __func__, __LINE__);
    goto error;
  }

  *(void **)&(g_hdr_lib.hdrTwoFrameCore) = dlsym(g_hdr_lib.ptr,
    "hdrTwoFrameCore");
  if (!g_hdr_lib.hdrTwoFrameCore) {
    IDBG_ERROR("%s:%d] Error linking hdrTwoFrameCore", __func__, __LINE__);
    goto error;
  }

  IDBG_HIGH("%s:%d] HDR library loaded successfully", __func__, __LINE__);
  return IMG_SUCCESS;

  error: if (g_hdr_lib.ptr)
    dlclose(g_hdr_lib.ptr);

  return IMG_ERR_NOT_FOUND;
}

/**
 * Function: hdr_comp_unload
 *
 * Description: This function is used to unload Qualcomm HDR component
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
void hdr_comp_unload(void* handle)
{
  IDBG_HIGH("%s:%d] ptr %p", __func__, __LINE__, g_hdr_lib.ptr);
  if (g_hdr_lib.ptr) {
    dlclose(g_hdr_lib.ptr);
    g_hdr_lib.ptr = NULL;
  }
}

