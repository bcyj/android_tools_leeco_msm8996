/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <media/msm_jpeg.h>
#include "jpeg_buffer_private.h"
#include "jpeg_common.h"
#include "jpegd_engine_hw_10.h"
#include "jpegd.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "os_timer.h"
#include "jpege_app_util_mmap.h"

/*============================================================================
   FUNCTIONS
============================================================================*/
/* Function prototypes of jpegd_engine_obj_t interface functions */
static void jpegd_engine_hw_create(jpegd_engine_obj_t *p_obj,
  jpegd_obj_t decoder);
static int jpegd_engine_hw_init(jpegd_engine_obj_t *,
  jpegd_engine_event_handler_t,
  jpegd_engine_input_req_handler_t);
static int jpegd_engine_hw_start(jpegd_engine_obj_t *);
static int jpegd_engine_hw_abort(jpegd_engine_obj_t *);
static void jpegd_engine_hw_destroy(jpegd_engine_obj_t *);
static int jpegd_engine_hw_configure(jpegd_engine_obj_t *,
  jpegd_engine_src_t *,
  jpegd_engine_dst_t *,
  jpegd_dst_t *,
  uint32_t *,
  uint32_t *);
/* Function prototypes of helper functions */
static int  jpegd_engine_hw_configure_baseline(jpegd_engine_hw_t *);
static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
jpegd_engine_hw_decode_thread(OS_THREAD_FUNC_ARG_T arg);
static jpeg_event_t jpegd_engine_hw_decode_baseline(jpegd_engine_hw_t *);

/* Extern functions */
extern int jpegd_engine_hw_check_qtable(jpegd_engine_hw_t *);
extern int jpegd_engine_hw_check_htable(jpegd_engine_hw_t *);
extern int jpegd_engine_hw_init_bit_buffer(jpegd_engine_hw_t *);
extern int jpegd_engine_hw_get_buf_in_use(jpegd_engine_hw_t *,
  jpeg_buf_t *);

/*============================================================================
   MACROS
============================================================================*/

#define INTERNAL_BUFFER_SIZE   8192 /* in bytes */
#define MAX_MCU_SIZE           384  /* in bytes */
#define BYTES_PER_FETCH   0x2000
#define MAX_NUM_OUTPUT_BUF 2

#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define CEILING8(X)  (((X) + 0x0007) & 0xFFF8)

/*============================================================================
   GLOBAL VARIABLES
============================================================================*/
static const char jpegd_engine_hw_name[] = "Jpeg Hardware Decode Engine";
jpegd_engine_profile_t jpegd_engine_hw_profile = {jpegd_engine_hw_create,
  jpegd_engine_hw_name,
  1};

static jpegd_engine_hw_t *p_engine_local;

static const float g_scale_factor[] =
  {1.0, 1.0/8.0, 2.0/8.0, 3.0/8.0, 4.0/8.0, 5.0/8.0, 6.0/8.0, 7.0/8.0};

/*===========================================================================
 * FUNCTION    - jpegd_engine_hw_create  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void jpegd_engine_hw_create(jpegd_engine_obj_t *p_obj,
  jpegd_obj_t p_decoder) {

  JPEG_DBG_LOW("%s:%d]", __func__, __LINE__);

  if (p_obj) {
    /* Destroy previous engine if it exists*/
    if (p_obj->destroy) {
       p_obj->destroy(p_obj);
    }
    p_obj->create        = &jpegd_engine_hw_create;
    p_obj->init          = &jpegd_engine_hw_init;
    p_obj->configure     = &jpegd_engine_hw_configure;
    p_obj->start         = &jpegd_engine_hw_start;
    p_obj->abort         = &jpegd_engine_hw_abort;
    p_obj->destroy       = &jpegd_engine_hw_destroy;
    p_obj->p_engine      = NULL;
    p_obj->decoder       = p_decoder;
    p_obj->is_intialized = false;
    p_obj->skip_pp       = true;
  }
}

/*===========================================================================
 * FUNCTION    - jpegd_engine_hw_event_handler  -
 *
 * DESCRIPTION:
 *==========================================================================*/
int jpegd_engine_hw_event_handler(void* p_user, jpegd_event_t *p_evt)
{
  jpegd_engine_hw_t *p_engine = (jpegd_engine_hw_t *)p_user;
  JPEG_DBG_MED("%s:%d] type %d", __func__, __LINE__, p_evt->type);

  os_mutex_lock(&p_engine->frame_done_mutex);
  if (JPEGD_EVT_FRAMEDONE == p_evt->type) {
    p_engine->output_done = true;
  } else if (JPEGD_EVT_ERROR == p_evt->type) {
    p_engine->error_flag = true;
  }
  os_cond_signal(&p_engine->frame_done_cond);
  os_mutex_unlock(&p_engine->frame_done_mutex);
  return 0;
}

/*===========================================================================
 * FUNCTION    - jpegd_engine_hw_init  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int jpegd_engine_hw_init(
  jpegd_engine_obj_t *p_obj,
  jpegd_engine_event_handler_t p_event_handler,
  jpegd_engine_input_req_handler_t p_input_req_handler)
{
  jpegd_engine_hw_t *p_engine;
  int rc = 0;

  JPEG_DBG_HIGH("%s:%d] ", __func__, __LINE__);

  /* Validate input arguments*/
  if (!p_obj || !p_event_handler || !p_input_req_handler) {
     return JPEGERR_ENULLPTR;
  }

  /* Allocate memory for the engine structure*/
  p_engine = (jpegd_engine_hw_t *)JPEG_MALLOC(sizeof(jpegd_engine_hw_t));
  if (!p_engine) {
     return JPEGERR_EMALLOC;
  }
  /* Initialize the fields inside the engine structure below*/
  STD_MEMSET(p_engine, 0, sizeof(jpegd_engine_hw_t));
  /* Initialize the pointer to the wrapper*/
  p_engine->p_wrapper               = p_obj;
  /* Initialize the pointer to the wrapper*/
  p_engine->input_fetcher.p_wrapper = p_obj;
  /* Initialize the event handler*/
  p_engine->p_event_handler         = p_event_handler;
  /* Initialize the input request handler*/
  p_engine->input_fetcher.p_input_req_handler = p_input_req_handler;
  /* Initialize source pointer of fetcher*/
  p_engine->input_fetcher.p_source  = &p_engine->source;
  os_mutex_init(&(p_engine->mutex));
  os_cond_init(&(p_engine->cond));

  /*buffers*/
  memset(&p_engine->output_buf, 0x0, sizeof(jpegd_buf));
  memset(&p_engine->in_buf, 0x0, sizeof(jpegd_buf));

  /* Initialize the mutex */
  os_mutex_init(&(p_engine->frame_done_mutex));
  /* Initialize the condition variable */
  os_cond_init(&(p_engine->frame_done_cond));

  JPEG_DBG_ERROR("%s: jpegd_lib_init", __func__);

  rc = jpegd_lib_init(&p_engine->p_handle,
    (void *)p_engine,
    jpegd_engine_hw_event_handler);
  if ((rc < 0) || (NULL == p_engine->p_handle)) {
    JPEG_DBG_ERROR("%s:%d] failed", __func__, __LINE__);
    JPEG_FREE(p_engine);
    return JPEGERR_EFAILED;
  }

  /* Assign allocated engine structure to p_obj*/
  p_obj->p_engine = (void *)p_engine;
  p_obj->is_intialized = true;
  return JPEGERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_engine_hw_start  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int jpegd_engine_hw_start(jpegd_engine_obj_t *p_obj)
{
  int rc;
  jpegd_engine_hw_t *p_engine;
  os_thread_t thread;

  JPEG_DBG_HIGH("%s:%d] ", __func__, __LINE__);

  /* Validate input arguments*/
  if (!p_obj || !p_obj->p_engine)
    return JPEGERR_ENULLPTR;

  /* Cast p_obj->obj as jpegd_engine_hw_t object*/
  p_engine = p_obj->p_engine;

  /* Make sure no threads are running*/
  os_mutex_lock(&(p_engine->mutex));
  if (p_engine->is_active || !p_engine->is_configured) {
    os_mutex_unlock(&(p_engine->mutex));
    JPEG_DBG_HIGH("%s:%d] bad state", __func__, __LINE__);
    return JPEGERR_EBADSTATE;
  }

  /* Reset abort flag*/
  p_engine->abort_flag = false;
  /* Set engine to be active*/
  p_engine->is_active = true;

  /* Join previously created decode thread if needed*/
  if (p_engine->is_dthread_need_join) {
    JPEG_DBG_MED("%s:%d] wait for thread join", __func__, __LINE__);
    (void)os_thread_join(&p_engine->decode_thread, NULL);
  }
  p_engine->is_dthread_need_join = true;

  /* Start Jpeg Decoding as a new thread*/
  rc = os_thread_create(&p_engine->decode_thread,
    jpegd_engine_hw_decode_thread, (void *)p_engine);

  JPEG_DBG_MED("%s:%d] rc = %d", __func__, __LINE__, rc);

  /* If there is a failure in creating the thread, clean up and exit*/
  if (rc) {
    /* Reset engine to be inactive*/
    p_engine->is_active = false;
    /* Reset thread needs join flag*/
    p_engine->is_dthread_need_join = false;

    os_mutex_unlock(&p_engine->mutex);
    JPEG_DBG_ERROR("%s:%d] os_thread_create() failed:%d",
      __func__, __LINE__, rc);
    return JPEGERR_EFAILED;
  }

  os_mutex_unlock(&(p_engine->mutex));
  return JPEGERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_engine_hw_abort  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int jpegd_engine_hw_abort(jpegd_engine_obj_t *p_obj)
{
  jpegd_engine_hw_t *p_engine;

  JPEG_DBG_HIGH("%s:%d] ", __func__, __LINE__);

  /* Validate input arguments*/
  if (!p_obj || !p_obj->p_engine)
    return JPEGERR_ENULLPTR;

  /* Cast p_obj->obj as jpegd_engine_hw_t object*/
  p_engine = p_obj->p_engine;

  /* Abort if engine is actively decoding*/
  os_mutex_lock(&(p_engine->mutex));
  if (p_engine->is_active) {
    p_engine->abort_flag = true;
    while (p_engine->abort_flag) {
      JPEG_DBG_MED("%s:%d] ", __func__, __LINE__);
      os_cond_wait(&(p_engine->cond), &(p_engine->mutex));
    }
  }
  os_mutex_unlock(&(p_engine->mutex));
  JPEG_DBG_MED("%s:%d] ", __func__, __LINE__);
  return JPEGERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_engine_hw_destroy  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void jpegd_engine_hw_destroy(jpegd_engine_obj_t *p_obj)
{
  JPEG_DBG_HIGH("%s:%d] ", __func__, __LINE__);

  if (p_obj) {
    uint32_t i;
    jpegd_engine_hw_t *p_engine = (jpegd_engine_hw_t *)(p_obj->p_engine);
    /* Abort and wait until engine is done with current decoding*/
    (void)jpegd_engine_hw_abort(p_obj);

    /* Join decode thread if necessary*/
    if (p_engine->is_dthread_need_join) {
      JPEG_DBG_MED("%s:%d] ", __func__, __LINE__);
      (void)os_thread_join(&p_engine->decode_thread, NULL);
    }

    for (i = 0; i < JPEGD_MAXHUFFTABLES; i++) {
       JPEG_FREE(p_engine->pDerivedHuffTable[i]);
    }
    /* Free pDeQuantTable*/
    for (i = 0; i < JPEGD_MAXQUANTTABLES; i++) {
       JPEG_FREE(p_engine->pDeQuantTable[i]);
    }
    /* Free pCompCoeffBuf*/
    for (i = 0; i < JPEGD_MAXCOMPONENTS; i++) {
       JPEG_FREE(p_engine->pCompCoeffBuf[i]);
    }
    /* Free quant tables */
    for (i = 0; i < 4; i++) {
      JPEG_FREE(p_engine->dqt_cfg.p_qtables[i]);
    }
    /* Release allocated memory*/
    if (p_engine) {
      os_mutex_destroy(&(p_engine->mutex));
      os_cond_destroy(&(p_engine->cond));
      JPEG_FREE(p_obj->p_engine);
      p_obj->p_engine = NULL;
    }
  }
}

/*===========================================================================
 * FUNCTION    - jpegd_engine_hw_configure_baseline  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int jpegd_engine_hw_configure_baseline(jpegd_engine_hw_t *p_engine)
{
  uint8_t j;
  uint32_t i, qt;
  int cnt = 0;
  uint8_t componentId;
  int32_t blockCount;
  uint32_t numOfBlocks;
  uint8_t hSample[JPEGD_MAXCOMPONENTS];
  uint8_t vSample[JPEGD_MAXCOMPONENTS];
  uint32_t numOfHBlocksInComp[JPEGD_MAXCOMPONENTS];
  uint32_t numOfVBlocksInComp[JPEGD_MAXCOMPONENTS];
  jpeg_rectangle_t region;
  jpeg_frame_info_t *p_frame_info = p_engine->source.p_frame_info;
  jpeg_scan_info_t  *p_scan_info  = p_frame_info->pp_scan_infos[0];
  uint8_t maxHSample  = 1;
  uint8_t maxVSample  = 1;
  uint32_t nRowsPerMCU = 0;
  uint32_t nColsPerMCU = 0;
  int rc, k;

  p_engine->dqt_cfg.qtable_present_flag = p_frame_info->qtable_present_flag;

  JPEG_DBG_HIGH("%s:%d, qtable_present_flag= 0x%x",
    __func__, __LINE__, p_frame_info->qtable_present_flag);

  /* Allocate qtable*/
  for (qt = 0; qt < 4; qt++) {
    if (!(p_frame_info->qtable_present_flag & (1<<qt))) {
      continue;
    }
    p_engine->dqt_cfg.p_qtables[qt] = (jpeg_quant_table_t)JPEG_MALLOC(64 *
      sizeof(uint16_t));
    if (!p_engine->dqt_cfg.p_qtables[qt])
    return JPEGERR_EMALLOC;
    STD_MEMSET(p_engine->dqt_cfg.p_qtables[qt], 0, 64 * sizeof(uint16_t));
    STD_MEMMOVE(p_engine->dqt_cfg.p_qtables[qt], p_frame_info->p_qtables[qt],
      64 * sizeof(uint16_t));
  }

  /*Huffman*/
  jpegd_cmd_huff_cfg_t huff_cfg;
  huff_cfg.htable_present_flag = p_frame_info->htable_present_flag;
  JPEG_DBG_HIGH("%s:%d, htable_present_flag = 0x%x",
    __func__, __LINE__, p_frame_info->htable_present_flag);

  int ht_cnt = 0;
  for (i = 0; i < 8; i++) {
    if (!(p_frame_info->htable_present_flag & (1 << i))) {
      ht_cnt++;
      continue;
    }
    for (k = 1; k < 17; k++) {
      huff_cfg.p_htables[ht_cnt].bits[k-1] =
        p_frame_info->p_htables[i].bits[k];
    }
    for (k = 0; k < 256; k++) {
      huff_cfg.p_htables[ht_cnt].values[k] =
        p_frame_info->p_htables[i].values[k];
    }
    ht_cnt++;
  }

  /**********************************************************************
  * First convert jpeg header related info in the p_engine->decodeSpec into
  * local variables
  * 1. image size
  * 2. restart interval
  * 3. components in frame and scan (for now only 1 scan is allowed)
  * 4. set components info (id, sampling, quant table) in sof marker
  * 5. set components info (huff table) in sos (scan 1 only)
  *    as well as spectrum selection.
  *********************************************************************/
  p_engine->nImageWidth               = p_frame_info->width;
  p_engine->nImageHeight              = p_frame_info->height;

  p_engine->nOutputWidth              = p_engine->dest.width;
  p_engine->nOutputHeight             = p_engine->dest.height;

  p_engine->nRestartInterval          = p_frame_info->restart_interval;
  p_engine->nNumberOfComponentsInScan = p_scan_info->num_selectors;

  /* component id, sampling, quant info*/
  for (j = 0; j < p_frame_info->num_comps; j++) {
    p_engine->compId[j] = p_frame_info->p_comp_infos[j].comp_id;
    hSample[j] = p_frame_info->p_comp_infos[j].sampling_h;
    vSample[j] = p_frame_info->p_comp_infos[j].sampling_v;
    p_engine->quantId[j] = p_frame_info->p_comp_infos[j].qtable_sel;
  }

  /* scan info, for baseline it is only 1 scan.*/
  p_engine->nNumberOfComponentsInScan = p_scan_info->num_selectors;
  p_engine->currentScanProgressiveInfo.ss = (uint32_t)(p_scan_info->spec_start);
  p_engine->currentScanProgressiveInfo.se = (uint32_t)(p_scan_info->spec_end);

  /* component huffman info*/
  for (j = 0; j < p_engine->nNumberOfComponentsInScan; j++) {
    /* get component id*/
    componentId = p_scan_info->p_selectors[j].comp_id;
    /* dc table index*/
    p_engine->dcHuffTableId[componentId] =
      p_scan_info->p_selectors[j].dc_selector;

    /* since only one set of huff table index is used,*/
    /* ac table is appened to dc table with index starting from*/
    /* JPEGD_MAXHUFFTABLES/2.*/
    p_engine->acHuffTableId[componentId] =
      p_scan_info->p_selectors[j].ac_selector +
      (uint8_t)(JPEGD_MAXHUFFTABLES >> 1);
    /* get components list in a MCU in one scan*/
    p_engine->compListInScan[j] = componentId;
  }

  /**********************************************************************
  *  If no region information is specified, then set the region to      *
  *  the whole image.                                                   *
  **********************************************************************/
  region = p_engine->dest.region;
  if (region.right == 0 && region.bottom == 0) {
    region.right  = p_engine->nImageWidth  - 1;
    region.bottom = p_engine->nImageHeight - 1;
  }

  /**********************************************************************
  * Validate the region:
  * 0 <= left < right  < Input Image Width
  * 0 <= top  < bottom < Input Image Height
  *********************************************************************/
  if (0 > region.left || region.left >= region.right  || region.right  >=
    (int32_t)(p_engine->nImageWidth) ||
    0 > region.top  || region.top  >= region.bottom || region.bottom >=
    (int32_t)(p_engine->nImageHeight)) {
    return JPEGERR_EFAILED;
  }

  JPEG_DBG_ERROR("%s:%d] num_comp %d hsample %d vsample %d",
    __func__, __LINE__, p_frame_info->num_comps,
    hSample[0], vSample[0]);
  /**********************************************************************
  * Here we check the color format. Also set up MCU related settings
  * 1. color format
  * 2. numbers of blocks in a MCU
  * 3. numbers of pixels in a row in one MCU
  * 4. numbers of pixels in a column in one MCU
  **********************************************************************/
  if (p_frame_info->num_comps == 1) {
    /********************************************************
    * grey scale, only 1 (Y) 8x8 block per MCU
    ********************************************************/
    p_engine->jpegdFormat   = JPEG_GRAYSCALE;
    p_engine->nBlocksPerMCU = 1;
    nRowsPerMCU     = 8;
    nColsPerMCU     = 8;

    /***********************************************************
    * Compute the # of MCUs outside of the Region.
    *
    * left_mcu_num   is the # of MCU before the Region MCU Row.
    * top_mcu_num    is the # of MCU above  the Region MCU Row.
    * right_mcu_num  is the # of MCU after  the Region MCU Row.
    * bottom_mcu_num is the # of MCU below  the Region MCU Row.
    *
    * Right shift by 3 is equivalent to integer divide by 8.
    *************************************************************/
    p_engine->left_mcu_num   = region.left >> 3;
    p_engine->top_mcu_num    = region.top  >> 3;
    p_engine->right_mcu_num  = (ROUND_TO_8(p_engine->nImageWidth) -
                                (uint32_t)(region.right) - 1) >> 3;
    p_engine->bottom_mcu_num = (ROUND_TO_8(p_engine->nImageHeight) -
                                (uint32_t)(region.bottom) - 1) >> 3;
  } else if (p_frame_info->num_comps == 3) {
    if (hSample[1] != 1 || vSample[1] != 1 ||
      hSample[2] != 1 || vSample[2] != 1) {
      return JPEGERR_EUNSUPPORTED;
    }
    if (hSample[0] == 1 && vSample[0] == 1) {
      /**************************************************************
      * H1V1, 3 (Y, Cb, Cr) blocks per MCU
      *************************************************************/
      p_engine->jpegdFormat   = JPEG_H1V1;
      p_engine->nBlocksPerMCU = 3;
      nRowsPerMCU     = 8;
      nColsPerMCU     = 8;

      /************************************************************************
      * Compute the # of MCUs outside of the Region.
      ************************************************************************/
      p_engine->left_mcu_num   = region.left >> 3;
      p_engine->top_mcu_num    = region.top  >> 3;
      p_engine->right_mcu_num  = (ROUND_TO_8(p_engine->nImageWidth) -
                                  (uint32_t)(region.right) - 1) >> 3;
      p_engine->bottom_mcu_num = (ROUND_TO_8(p_engine->nImageHeight) -
                                  (uint32_t)(region.bottom) - 1) >> 3;
  } else if (hSample[0] == 2 && vSample[0] == 1) {
    /**************************************************************
     * H2V1, 4 (Yx2, Cb, Cr) blocks per MCU
     *************************************************************/
     p_engine->jpegdFormat   = JPEG_H2V1;
     p_engine->nBlocksPerMCU = 4;
     nRowsPerMCU     = p_engine->nImageHeight;       /* 8*/
     nColsPerMCU     = 16;

     /*********************************************************
     * Compute the # of MCUs outside of the Region.
     ********************************************************/
     p_engine->left_mcu_num   = region.left >> 4;
     p_engine->top_mcu_num    = region.top  >> 3;
     p_engine->right_mcu_num  = (ROUND_TO_16(p_engine->nImageWidth) -
                                 (uint32_t)(region.right) - 1) >> 4;
     p_engine->bottom_mcu_num = (ROUND_TO_8(p_engine->nImageHeight) -
                                 (uint32_t)(region.bottom) - 1) >> 3;
    } else if (hSample[0] == 1 && vSample[0] == 2) {

        /*H1V2, 4 (Yx2, Cb, Cr) blocks per MCU*/

        p_engine->jpegdFormat   = JPEG_H1V2;
        p_engine->nBlocksPerMCU = 4;
        nRowsPerMCU     = p_engine->nImageHeight;
        /* 16*/
        nColsPerMCU     = 8;
        /* Compute the # of MCUs outside of the Region.*/

        p_engine->left_mcu_num   = region.left >> 3;
        p_engine->top_mcu_num    = region.top  >> 4;
        p_engine->right_mcu_num  = (ROUND_TO_8(p_engine->nImageWidth) -
                                    (uint32_t)(region.right) - 1) >> 3;
        p_engine->bottom_mcu_num = (ROUND_TO_16(p_engine->nImageHeight) -
                                    (uint32_t)(region.bottom) - 1) >> 4;
     } else if (hSample[0] == 2 && vSample[0] == 2) {
        /** H2V2, 6 (Yx4, Cb, Cr) blocks per
                        MCU*/
        p_engine->jpegdFormat   = JPEG_H2V2;
        p_engine->nBlocksPerMCU = 6;
        /* 16*/
        nRowsPerMCU     = p_engine->nImageHeight;
        nColsPerMCU     = 16;

        /* Compute the # of MCUs outside of the Region.*/

        p_engine->left_mcu_num   = region.left >> 4;
        p_engine->top_mcu_num    = region.top  >> 4;
        p_engine->right_mcu_num  = (ROUND_TO_16(p_engine->nImageWidth) -
                                    (uint32_t)(region.right) - 1) >> 4;
        p_engine->bottom_mcu_num = (ROUND_TO_16(p_engine->nImageHeight) -
                                    (uint32_t)(region.bottom) - 1) >> 4;
     } else {
       /**if none of these format, we can not decode this, never
        * should go here.*/

       return JPEGERR_EUNSUPPORTED;
    } /* end if ( hSample[0] == 1 &&  vSample[0] == 1 )*/
  } else {
    return JPEGERR_EUNSUPPORTED;
  } /* end if ( p_frame_info->num_comps == 1 )*/

  /**********************************************************************
  * here we find out how many lines and coloumns of MCUs in a component
  * 1. find out the maximum sampling factor for all components
  * 2. calculate number of blocks in verticle and horizontal direction
  *    for each component.
  **********************************************************************/
  /* first figure out the maximum sample for rols and cols*/
  for (j = 0; j < p_frame_info->num_comps; j++) {
    if (hSample[j] > maxHSample) {
       maxHSample = hSample[j];
    }
    if (vSample[j] > maxVSample) {
       maxVSample = vSample[j];
    }
  }
  /* then use the maximum sample to determine the largest block per line*/
  /* and block per col.*/
  for (j = 0; j < p_frame_info->num_comps; j++) {
    numOfHBlocksInComp[j] =
      (uint32_t)((7 + (p_engine->nImageWidth * maxHSample
      + maxHSample - 1) / maxHSample) / 8);
     numOfVBlocksInComp[j] =
       (uint32_t)((7 + (p_engine->nImageHeight * maxVSample
       + maxVSample - 1) / maxVSample) / 8);
  }
  /**********************************************************************
  * also need to calculate the number of MCUs per row and col
  * This is determind by the color format and number of blocks in image
  * For each mcu, the type of block need to be labeled also
  *********************************************************************/
  if (p_engine->nNumberOfComponentsInScan == 1) {
     /* grey image*/
     p_engine->nMCUPerRow = numOfHBlocksInComp[p_engine->compListInScan[0]];
     p_engine->nMCUPerCol = numOfVBlocksInComp[p_engine->compListInScan[0]];
     p_engine->MCUBlockCompList[0] = p_engine->compListInScan[0];
  } else {
     /*color image.*/
     p_engine->nMCUPerRow = ((p_engine->nImageWidth  + 7) / 8 +
                             maxHSample - 1) / maxHSample;
     p_engine->nMCUPerCol = ((p_engine->nImageHeight + 7) / 8 +
                             maxVSample - 1) / maxVSample;

     blockCount = 0;
     for (j = 0; j < p_engine->nNumberOfComponentsInScan; j++) {
        /* using color format information, we can get number of blocks*/
        /* per MCU. we can assume that blocks in MCU is Y first*/
        /* followed by cb and cr.*/
        componentId = p_engine->compListInScan[j];
        numOfBlocks = hSample[componentId] * vSample[componentId];

        while (numOfBlocks--) {
           p_engine->MCUBlockCompList[blockCount++] = componentId;
        }
     }
  }

  /**************************************************************************
  *  Calculate the Region Start MCU index and Region End MCU index.         *
  **************************************************************************/
  p_engine->region_start_mcu_id =
     p_engine->top_mcu_num * p_engine->nMCUPerRow +
     p_engine->left_mcu_num;

  p_engine->region_end_mcu_id   =
     (p_engine->nMCUPerCol - p_engine->bottom_mcu_num) *
     p_engine->nMCUPerRow -
     p_engine->right_mcu_num;

  /* Adjust the # of MCUs in a MCU Row for Region decoding*/
  p_engine->nMCUPerRow -= (p_engine->right_mcu_num + p_engine->left_mcu_num);

  /* Adjust the image width/height to be the Region dimensions.*/
  p_engine->nImageWidth  = region.right  - region.left + 1;
  p_engine->nImageHeight = region.bottom - region.top  + 1;

  /**********************************************************************
   * Calculate the Resize factor for IDCT according to input/output size
   * ratio
  *********************************************************************/
  p_engine->nResizeFactor = 3;
  while ((p_engine->nResizeFactor >= 0) &&
    ((p_engine->nOutputWidth > (p_engine->nImageWidth  >>
    p_engine->nResizeFactor)) ||
    (p_engine->nOutputHeight > (p_engine->nImageHeight >>
    p_engine->nResizeFactor)))) {
     p_engine->nResizeFactor--;
  }
  if (p_engine->nResizeFactor < 0) {
    return JPEGERR_EFAILED;
  }
  nRowsPerMCU >>= p_engine->nResizeFactor;
  nColsPerMCU >>= p_engine->nResizeFactor;

  if (p_engine->nRestartInterval != 0) {
    p_engine->nRestartLeft    = p_engine->nRestartInterval;
    p_engine->nNextRestartNum = 0xD0;
  }

  /**********************************************************************
  * check/make quantization table, huffman table
  *********************************************************************/
  if (JPEG_FAILED(jpegd_engine_hw_check_qtable(p_engine))) {
    return JPEGERR_EFAILED;
  }

  if (JPEG_FAILED(jpegd_engine_hw_check_htable(p_engine))) {
    return JPEGERR_EFAILED;
  }

  /**************************************************************************
  * Paddings are added to Jpeg image to round it up to multiple of blocks,
  * but actual image size be less than the round up.
  *
  * Calculate the Row Increment for the MCU Row buffer that holds the
  * rounded up lines.
  *
  * Also set which JpegdOutputOneMCU method to use according to format.
  *
  * The output size is changed proportionally to the input size, unlike
  * the output buffer size, which is rounded to multiple of cols per MCU.

  * This will be used for postprocess to unpad the image.
  *************************************************************************/
  if (p_engine->jpegdFormat == JPEG_GRAYSCALE) {
    /* gray-scale*/
    p_engine->nYLinesPerMCU      = nRowsPerMCU;
    p_engine->nCbCrLinesPerMCU   = 0;
    p_engine->nOutputRowIncrY    = nColsPerMCU * p_engine->nMCUPerRow;
    p_engine->nOutputRowIncrCbCr = 0;
  } else if (p_engine->jpegdFormat == JPEG_H1V1) {
    /* H1V1*/
    p_engine->nYLinesPerMCU      = nRowsPerMCU;
    p_engine->nCbCrLinesPerMCU   = nRowsPerMCU;
    p_engine->nOutputRowIncrY    = nColsPerMCU * p_engine->nMCUPerRow;
    p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY * 2;
  } else if (p_engine->jpegdFormat == JPEG_H2V1) {
    /* H2V1*/
    p_engine->nYLinesPerMCU      = nRowsPerMCU;
    p_engine->nCbCrLinesPerMCU   = nRowsPerMCU;
    p_engine->nOutputRowIncrY    = nColsPerMCU * p_engine->nMCUPerRow;
    p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY;
  } else if (p_engine->jpegdFormat == JPEG_H1V2) {
    /* H1V2*/
    p_engine->nYLinesPerMCU      = nRowsPerMCU;
    p_engine->nCbCrLinesPerMCU   = nRowsPerMCU >> 1;
    p_engine->nOutputRowIncrY    = nColsPerMCU * p_engine->nMCUPerRow;
    p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY * 2;
  } else if (p_engine->jpegdFormat == JPEG_H2V2) {
    /* H2V2*/
    p_engine->nYLinesPerMCU      = nRowsPerMCU;
    p_engine->nCbCrLinesPerMCU   = nRowsPerMCU >> 1;
    p_engine->nOutputRowIncrY    = nColsPerMCU * p_engine->nMCUPerRow;
    p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY;
  }

  JPEG_DBG_ERROR("%s:%d] jpegdFormat %d", __func__, __LINE__,
    p_engine->jpegdFormat);
  /* Set output dimension*/
  p_engine->nOutputWidth  = (uint32_t)(p_engine->nImageWidth >>
    p_engine->nResizeFactor);
  p_engine->nOutputHeight = (uint32_t)(p_engine->nImageHeight >>
    p_engine->nResizeFactor);

  p_engine->nOutputBufferSizeY = p_engine->nOutputRowIncrY *
     p_engine->nYLinesPerMCU;
  p_engine->nOutputBufferSizeCbCr = p_engine->nOutputRowIncrCbCr *
     p_engine->nCbCrLinesPerMCU;

  jpegd_base_config_t base_cfg;
  base_cfg.width = p_frame_info->width;
  base_cfg.height = p_frame_info->height;
  base_cfg.actual_width = p_engine->actual_width;
  base_cfg.actual_height = p_engine->actual_height;
  base_cfg.rotation = p_engine->source.hw_rotation;
  base_cfg.scale_factor = p_engine->source.hw_scale_factor;
  base_cfg.format = p_engine->jpegdFormat;
  base_cfg.crcb_order = p_engine->output_crcb_order;
  base_cfg.num_planes = p_engine->source.num_planes;
  base_cfg.restart_interval = p_engine->nRestartInterval;

  rc = jpegd_lib_configure_baseline(p_engine->p_handle,
    &p_engine->dqt_cfg,
    &huff_cfg,
    &base_cfg);
  if (rc) {
    JPEG_DBG_ERROR("%s:%d] failed %d", __func__, __LINE__, rc);
    return JPEGERR_EFAILED;
  }

  return JPEGERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpeg_update_output_buffers  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void jpeg_update_output_buffers(jpegd_engine_hw_t *p_engine)
{
  float sc_fac = g_scale_factor[p_engine->source.hw_scale_factor];
  uint8_t *plane_addr[JPEG_MAX_PLANES] = {NULL, NULL, NULL};
  uint32_t plane_size[JPEG_MAX_PLANES];
  uint32_t orig_pln_size[JPEG_MAX_PLANES];
  uint32_t i = 0, k = 0;
  int num_planes = p_engine->source.num_planes;
  uint32_t luma_size   = p_engine->out_luma_size;
  uint32_t chroma_size = p_engine->out_chroma_size;
  uint32_t chroma2_size = p_engine->out_chroma2_size;
  int use_line_copy = (p_engine->plane_stride[0] !=
    p_engine->orig_plane_stride[0]);
  uint8_t *scr_addr[JPEG_MAX_PLANES] = {
    (uint8_t *)p_engine->output_buf.vaddr,
    (uint8_t *)p_engine->output_buf.vaddr + luma_size,
    (uint8_t *)p_engine->output_buf.vaddr + luma_size + chroma_size,
  };

  for (i = 0; i < JPEG_MAX_PLANES; i++) {
    orig_pln_size[i] = p_engine->orig_pln_size[i];
    plane_size[i] = (orig_pln_size[i] * SQUARE(sc_fac));
  }

  struct jpeg_buf_t *p_jbuf =
    (struct jpeg_buf_t *)p_engine->dest.p_output_buf_array->data.
    yuv.luma_buf;
  plane_addr[0] = p_jbuf->ptr;

  if (num_planes > 1) {
    p_jbuf =
      (struct jpeg_buf_t *)p_engine->dest.p_output_buf_array->data.
      yuv.chroma_buf;
    plane_addr[1] = p_jbuf->ptr;
    if (num_planes > 2) {
      plane_addr[2] = plane_addr[1] + orig_pln_size[1];
    }
  } else {
    plane_addr[1] = NULL;
  }

  JPEG_DBG_HIGH("%s:%d] copy the buffers scalefac %f size %d %d %d pl_cnt %d",
    __func__, __LINE__, sc_fac, luma_size, chroma_size, chroma2_size,
    num_planes);
  JPEG_DBG_HIGH("%s:%d] use_line_copy %d orig size %d %d %d",
    __func__, __LINE__,
    use_line_copy,
    orig_pln_size[0],
    orig_pln_size[1],
    orig_pln_size[2]);
  JPEG_DBG_HIGH("%s:%d] height %d %d %d stride orig %d %d %d pad %d %d %d",
    __func__, __LINE__,
    p_engine->plane_height[0],
    p_engine->plane_height[1],
    p_engine->plane_height[2],
    p_engine->orig_plane_stride[0],
    p_engine->orig_plane_stride[1],
    p_engine->orig_plane_stride[2],
    p_engine->plane_stride[0],
    p_engine->plane_stride[1],
    p_engine->plane_stride[2]);

  for (k = 0; k < JPEG_MAX_PLANES; k++) {
    if (plane_addr[k] && plane_size[k]) {
      if (!use_line_copy) {
        memcpy(plane_addr[k], scr_addr[k], plane_size[k]);
      } else {
        uint8_t *p_dest_addr = plane_addr[k];
        uint8_t *p_src_addr = scr_addr[k];
        for (i = 0; i < p_engine->plane_height[0]; i++) {
          memcpy(p_dest_addr, p_src_addr, p_engine->orig_plane_stride[k]);
          p_src_addr += p_engine->plane_stride[k];
          p_dest_addr += p_engine->orig_plane_stride[k];
        }
      }
    }
  }
}

/*===========================================================================
 * FUNCTION    - jpegd_engine_hw_configure  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int jpegd_engine_hw_configure(jpegd_engine_obj_t *p_obj,
  jpegd_engine_src_t *p_source,
  jpegd_engine_dst_t *p_engine_dst,
  jpegd_dst_t        *p_dest,
  uint32_t           *p_output_chunk_width,
  uint32_t           *p_output_chunk_height)
{
  jpeg_frame_info_t *p_frame_info;
  jpegd_engine_hw_t *p_engine;
  int rc;

  JPEG_DBG_MED("%s:%d]", __func__, __LINE__);

  /* Input validation*/
  if (!p_obj || !p_obj->p_engine ||
    !p_source || !p_engine_dst || !p_source->p_frame_info ||
    !p_source->p_input_req_handler ||
    !p_engine_dst->p_luma_buffers[0] ||
    !p_engine_dst->p_chroma_buffers[0] ||
    !p_engine_dst->p_luma_buffers[1] ||
    !p_engine_dst->p_chroma_buffers[1] ||
    !p_engine_dst->width || !p_engine_dst->height) {
    JPEG_DBG_ERROR("%s:%d] bad param", __func__, __LINE__);
    return JPEGERR_EBADPARM;
  }

  /* Support baseline & progressive decoding*/
  p_frame_info = p_source->p_frame_info;
  if (p_frame_info->process != JPEG_PROCESS_BASELINE) {
    JPEG_DBG_ERROR("%s:%d] unsupported coding process %d", __func__, __LINE__,
    p_frame_info->process);
    return JPEGERR_EUNSUPPORTED;
  }

  /* Cast p_obj->p_engine as jpegd_engine_hw_t object*/
  p_engine = (jpegd_engine_hw_t *)p_obj->p_engine;

  /* Initialize engine input*/
  p_engine->source = *p_source;
  JPEG_DBG_MED("%s:%d] HW rotation %d Scale factor %d",
    __func__, __LINE__,
    p_engine->source.hw_rotation,
    p_engine->source.hw_scale_factor);
  jpeg_buffer_mark_empty(p_engine->source.p_buffers[0]);
  jpeg_buffer_mark_empty(p_engine->source.p_buffers[1]);
  p_engine->input_fetcher.buffer_in_use = 0;

  /* Initialize engine output dimensions*/
  p_engine->dest = *p_engine_dst;

  /* Initialize engine output MCU line buffer pointers*/
  p_engine->pMCULineBufPtrY    = NULL;
  p_engine->pMCULineBufPtrCbCr = NULL;

  /* Initialize engine internal data*/
  (void)STD_MEMSET(p_engine->MCUBlockCompList, 0,
    sizeof(p_engine->MCUBlockCompList));
  (void)STD_MEMSET(p_engine->compId, 0, sizeof(p_engine->compId));
  (void)STD_MEMSET(p_engine->quantId, 0, sizeof(p_engine->quantId));
  (void)STD_MEMSET(p_engine->compListInScan, 0,
    sizeof(p_engine->compListInScan));
  (void)STD_MEMSET(p_engine->dcHuffTableId, 0,
    sizeof(p_engine->dcHuffTableId));
  (void)STD_MEMSET(p_engine->acHuffTableId, 0,
    sizeof(p_engine->acHuffTableId));
  (void)STD_MEMSET(p_engine->hSample, 0, sizeof(p_engine->hSample));
  (void)STD_MEMSET(p_engine->vSample, 0, sizeof(p_engine->vSample));
  (void)STD_MEMSET(p_engine->compEntropySelectors, 0,
    sizeof(p_engine->compEntropySelectors));

  /* Initialize restart interval*/
  p_engine->nRestartInterval = 0;

  JPEG_DBG_HIGH("%s:%d]: format = %d planes %d",
    __func__, __LINE__, p_dest->output_format,
    p_engine->source.num_planes);

  int o_width, o_height;
  o_width = p_engine->actual_width = p_frame_info->width;
  o_height = p_engine->actual_height = p_frame_info->height;
  int width, height;

  switch (p_dest->output_format) {
  case YCBCRLP_H2V2:
  case YCRCBLP_H2V2:
    width = CEILING16(p_frame_info->width);
    height = CEILING16(p_frame_info->height);
    p_engine->output_crcb_order = (p_dest->output_format == YCRCBLP_H2V2)
      ? 1 : 0;
    p_engine->out_luma_size = (width * height);
    p_engine->orig_pln_size[0] = (o_width * o_height);
    p_engine->plane_stride[0] = width;
    p_engine->orig_plane_stride[0] = o_width;
    p_engine->plane_height[0] = o_height;
    if (p_engine->source.num_planes == 3) {
      p_engine->out_chroma2_size = p_engine->out_chroma_size =
        (width * height) >> 2;
      p_engine->orig_pln_size[1] = p_engine->orig_pln_size[2] =
        (o_width * o_height) >> 2;
      p_engine->plane_stride[2] = p_engine->plane_stride[1] = width >> 1;
      p_engine->orig_plane_stride[2] = p_engine->orig_plane_stride[1] =
        o_width >> 1;
      p_engine->plane_stride[2] = p_engine->plane_stride[1] = width >> 1;
      p_engine->orig_plane_stride[2] = p_engine->orig_plane_stride[1] =
        o_width >> 1;
      p_engine->plane_height[1] = p_engine->plane_height[2] = o_height >> 1;
    } else {
      p_engine->out_chroma_size = (width * height) >> 1;
      p_engine->orig_pln_size[1] = (o_width * o_height) >> 1;
      p_engine->out_chroma2_size = 0;
      p_engine->orig_pln_size[2] = 0;
      p_engine->plane_stride[1] = width;
      p_engine->plane_stride[2] = 0;
      p_engine->orig_plane_stride[1] = o_width;
      p_engine->orig_plane_stride[2] = 0;
      p_engine->plane_height[1] = o_height >> 1;
      p_engine->plane_height[2] = 0;
    }
    break;
  case YCBCRLP_H2V1:
  case YCRCBLP_H2V1:
    width = CEILING16(p_frame_info->width);
    height = CEILING8(p_frame_info->height);
    p_engine->output_crcb_order = (p_dest->output_format == YCRCBLP_H2V1)
      ? 1 : 0;
    p_engine->out_luma_size = (width * height);
    p_engine->orig_pln_size[0] = (o_width * o_height);
    p_engine->plane_stride[0] = width;
    p_engine->orig_plane_stride[0] = o_width;
    p_engine->plane_height[0] = o_height;
    if (p_engine->source.num_planes == 3) {
      p_engine->out_chroma2_size = p_engine->out_chroma_size =
        (width * height) >> 1;
      p_engine->orig_pln_size[1] = p_engine->orig_pln_size[2] =
        (o_width * o_height) >> 1;
      p_engine->plane_stride[2] = p_engine->plane_stride[1] = width >> 1;
      p_engine->orig_plane_stride[2] = p_engine->orig_plane_stride[1] =
        o_width >> 1;
      p_engine->plane_height[1] = p_engine->plane_height[2] = o_height;
    } else {
      p_engine->out_chroma_size = (width * height);
      p_engine->orig_pln_size[1] = (o_width * o_height);
      p_engine->out_chroma2_size = 0;
      p_engine->orig_pln_size[2] = 0;
      p_engine->plane_stride[1] = width;
      p_engine->plane_stride[2] = 0;
      p_engine->orig_plane_stride[1] = o_width;
      p_engine->orig_plane_stride[2] = 0;
      p_engine->plane_height[1] = o_height;
      p_engine->plane_height[2] = 0;
    }
    break;
  case YCBCRLP_H1V2:
  case YCRCBLP_H1V2:
    width = CEILING8(p_frame_info->width);
    height = CEILING16(p_frame_info->height);
    p_engine->output_crcb_order = (p_dest->output_format == YCRCBLP_H1V2)
      ? 1 : 0;
    p_engine->out_luma_size = (width * height);
    p_engine->orig_pln_size[0] = (o_width * o_height);
    p_engine->plane_stride[0] = width;
    p_engine->orig_plane_stride[0] = o_width;
    p_engine->plane_height[0] = o_height;
    if (p_engine->source.num_planes == 3) {
      p_engine->out_chroma2_size = p_engine->out_chroma_size =
        (width * height) >> 1;
      p_engine->orig_pln_size[1] = p_engine->orig_pln_size[2] =
        (o_width * o_height) >> 1;
      p_engine->plane_stride[2] = p_engine->plane_stride[1] = width;
      p_engine->orig_plane_stride[2] = p_engine->orig_plane_stride[1] =
        o_width;
      p_engine->plane_height[1] = p_engine->plane_height[2] = o_height >> 1;
    } else {
      p_engine->out_chroma_size = (width * height);
      p_engine->orig_pln_size[1] = (o_width * o_height);
      p_engine->out_chroma2_size = 0;
      p_engine->orig_pln_size[2] = 0;
      p_engine->plane_stride[1] = width << 1;
      p_engine->plane_stride[2] = 0;
      p_engine->orig_plane_stride[1] = o_width << 1;
      p_engine->orig_plane_stride[2] = 0;
      p_engine->plane_height[1] = o_height >> 1;
      p_engine->plane_height[2] = 0;
    }
    break;
  case YCBCRLP_H1V1:
  case YCRCBLP_H1V1:
    width = CEILING8(p_frame_info->width);
    height = CEILING8(p_frame_info->height);
    p_engine->output_crcb_order = (p_dest->output_format == YCRCBLP_H1V1)
      ? 1 : 0;
    p_engine->out_luma_size = (width * height);
    p_engine->orig_pln_size[0] = (o_width * o_height);
    p_engine->plane_stride[0] = width;
    p_engine->orig_plane_stride[0] = o_width;
    p_engine->plane_height[0] = o_height;
    if (p_engine->source.num_planes == 3) {
      p_engine->out_chroma2_size = p_engine->out_chroma_size =
        (width * height);
      p_engine->orig_pln_size[1] = p_engine->orig_pln_size[2] =
        (o_width * o_height);
      p_engine->plane_stride[2] = p_engine->plane_stride[1] = width;
      p_engine->orig_plane_stride[2] = p_engine->orig_plane_stride[1] =
        o_width;
      p_engine->plane_height[1] = p_engine->plane_height[2] = o_height;
    } else {
      p_engine->out_chroma_size = (width * height * 2);
      p_engine->orig_pln_size[1] = (o_width * o_height * 2);
      p_engine->out_chroma2_size = 0;
      p_engine->orig_pln_size[2] = 0;
      p_engine->plane_stride[1] = width << 1;
      p_engine->plane_stride[2] = 0;
      p_engine->orig_plane_stride[1] = o_width << 1;
      p_engine->orig_plane_stride[2] = 0;
      p_engine->plane_height[1] = o_height;
      p_engine->plane_height[2] = 0;
    }
    break;
  case MONOCHROME:
    width = CEILING8(p_frame_info->width);
    height = CEILING8(p_frame_info->height);
    p_engine->out_luma_size = (width * height);
    p_engine->orig_pln_size[0] = (o_width * o_height);
    p_engine->plane_stride[0] = width;
    p_engine->orig_plane_stride[0] = o_width;
    p_engine->plane_height[0] = o_height;
    p_engine->out_chroma2_size = p_engine->out_chroma_size = 0;
    p_engine->orig_pln_size[1] = p_engine->orig_pln_size[2] = 0;
    p_engine->orig_plane_stride[1] = p_engine->orig_plane_stride[2] = 0;
    p_engine->plane_stride[1] = p_engine->plane_stride[2] = 0;
    p_engine->plane_height[1] = p_engine->plane_height[2] = 0;
    break;
  default:
    JPEG_DBG_ERROR("%s:%d] Invalid format", __func__, __LINE__);
  }
  p_frame_info->width = width;
  p_frame_info->height = height;
  JPEG_DBG_HIGH("%s:%d]: actual %dx%d new %dx%d",
    __func__, __LINE__,
    p_engine->actual_width,
    p_engine->actual_height,
    p_frame_info->width,
    p_frame_info->height);
  p_engine->back_to_back_count = p_dest->back_to_back_count;

  if (p_frame_info->process == JPEG_PROCESS_BASELINE) {
     /* Initialized engine in baseline mode*/
     p_engine->is_progressive = false;
     rc = jpegd_engine_hw_configure_baseline(p_engine);
     if (JPEG_FAILED(rc)) return rc;
  } else {
     JPEG_DBG_ERROR("%s:%d] unsupported coding process %d",
       __func__, __LINE__, p_frame_info->process);
     return JPEGERR_EUNSUPPORTED;
  }

  /* Set engine output dimensions, may get changed by resize factor*/
  p_engine_dst->width       = p_engine->nOutputWidth;
  p_engine_dst->height      = p_engine->nOutputHeight;
  p_engine_dst->subsampling = p_engine->jpegdFormat;

  /* Set engine output MCU line buffer sizes*/
  *p_output_chunk_width  = p_engine->nOutputRowIncrY;
  *p_output_chunk_height = p_engine->nYLinesPerMCU;

  p_engine->is_configured = true;
  p_engine->fInputError = 0;
  return JPEGERR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - jpegd_engine_hw_decode_baseline  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static jpeg_event_t jpegd_engine_hw_decode_baseline(
  jpegd_engine_hw_t *p_engine)
{
  int rc;
  int use_pmem = 1;
  os_timer_t os_timer;
  int diff;
  uint32_t jpegd_hw_irq_status;
  uint32_t framedone_irq = 0;
  int i, j;
  int buffer_in_use;
  int bitstream_size;
  uint8_t scale_ratio = 0;
  uint8_t    *p_buf;
  uint8_t    *y_buf;
  uint8_t    *uv_buf;
  int y_offset, uv_offset;
  uint32_t nColsToProcess = p_engine->nOutputRowIncrY;
  uint32_t nRowsToProcess = p_engine->nYLinesPerMCU;
  uint32_t file_size              = p_engine->source.total_length;
  jpeg_frame_info_t *p_frame_info = p_engine->source.p_frame_info;
  uint32_t height      = p_frame_info->height;
  uint32_t width       = p_frame_info->width;
  uint32_t offset      = p_frame_info->pp_scan_infos[0]->offset;
  uint32_t luma_size   = p_engine->out_luma_size;
  uint32_t chroma_size = p_engine->out_chroma_size;
  uint32_t chroma2_size = p_engine->out_chroma2_size;
  uint32_t frame_done = 0, irq_status = 0;

  static jpeg_buf_t *p_out_y_buf;
  static jpeg_buf_t *p_out_uv_buf;
  static jpeg_buf_t *p_bitstream_buf;
  static jpeg_buf_t *saved_source_buf;
  static int buffers_initialized = false;

  /* bit_buf_length is actually at least two bytes less */
  bitstream_size = file_size - offset;
  JPEG_DBG_HIGH("%s:%d] size %d %d %d", __func__, __LINE__,
    bitstream_size, file_size, offset);

  if(!buffers_initialized) {
    JPEG_DBG_MED("%s:%d]Initializing decoder buffers", __func__, __LINE__);

    /* Initialize output buffers*/
    p_engine->in_buf.alloc_ion.len = bitstream_size;
    p_engine->in_buf.alloc_ion.flags = 0;
    p_engine->in_buf.alloc_ion.heap_mask = 0x1 << ION_IOMMU_HEAP_ID;
    p_engine->in_buf.alloc_ion.align = 4096;
    p_engine->in_buf.vaddr = do_mmap(bitstream_size, &p_engine->in_buf.fd,
      JPEG_PMEM_ADSP,
      &p_engine->in_buf.ion_fd_main,
      &p_engine->in_buf.alloc_ion,
      &p_engine->in_buf.fd_ion_map);
    if (!p_engine->in_buf.vaddr){
      JPEG_DBG_ERROR("(%d)%s()  Error mapping bitstream buffer!!!",
        __LINE__, __func__);
      return JPEG_EVENT_ERROR;
    }

    JPEG_DBG_LOW("%s:%d]:in_buf.vaddr=0x%08X", __func__, __LINE__,
      (uint32_t)p_engine->in_buf.vaddr);

    /* Initialize output buffers*/
    p_engine->output_buf.alloc_ion.len =
      luma_size + chroma_size + chroma2_size;
    p_engine->output_buf.alloc_ion.flags = 0;
    p_engine->output_buf.alloc_ion.heap_mask = 0x1 << ION_IOMMU_HEAP_ID;
    p_engine->output_buf.alloc_ion.align = 4096;
    p_engine->output_buf.vaddr = do_mmap(
      p_engine->output_buf.alloc_ion.len,
      &p_engine->output_buf.fd,
      JPEG_PMEM_ADSP,
      &p_engine->output_buf.ion_fd_main,
      &p_engine->output_buf.alloc_ion,
      &p_engine->output_buf.fd_ion_map);
    if (!p_engine->output_buf.vaddr) {
      JPEG_DBG_ERROR("%s:%d] Error mapping output buffer!!!",
        __func__, __LINE__);
      return JPEG_EVENT_ERROR;
    }
    JPEG_DBG_LOW("%s:%d] output_buf.vaddr=0x%08X", __func__, __LINE__,
      p_engine->output_buf.vaddr);

    /* Initialize output buffers*/
    if (jpeg_buffer_init(&p_bitstream_buf) ||
      jpeg_buffer_init(&p_out_y_buf) ||
      jpeg_buffer_init(&p_out_uv_buf)) {
      JPEG_DBG_ERROR("%s:%d] Error init. output buffer!!!",
        __func__, __LINE__);
       return JPEG_EVENT_ERROR;
    }
    buffers_initialized = true;
  }

  p_engine->in_buf.type = 0;
  p_engine->in_buf.offset = 0;
  p_engine->in_buf.y_off = 0;
  p_engine->in_buf.y_len = bitstream_size;

  p_engine->output_buf.type = 0;
  p_engine->output_buf.offset = 0;
  p_engine->output_buf.y_off = 0;
  p_engine->output_buf.y_len = luma_size;
  p_engine->output_buf.cbcr_off = luma_size;
  p_engine->output_buf.cbcr_len = chroma_size;

  /* updates for 3-planar ans single planar*/
  if (p_engine->source.num_planes == 3) {
    p_engine->output_buf.pln2_off = p_engine->output_buf.cbcr_off +
      p_engine->output_buf.cbcr_len;
    p_engine->output_buf.pln2_len = chroma2_size;
  } else if (p_engine->source.num_planes == 1) {
    p_engine->output_buf.cbcr_off = 0;
    p_engine->output_buf.cbcr_len = 0;
  }

  p_out_y_buf->size = nRowsToProcess * width;
  p_out_uv_buf->size = (nRowsToProcess * width)>>1;

  buffer_in_use = p_engine->input_fetcher.buffer_in_use;
  saved_source_buf =
    p_engine->input_fetcher.p_source->p_buffers[1-buffer_in_use];
  p_engine->source.p_buffers[1-buffer_in_use] = (jpeg_buf_t *)p_bitstream_buf;

  p_bitstream_buf->ptr = (uint8_t *)p_engine->in_buf.vaddr;
  p_bitstream_buf->size = bitstream_size;
  p_buf = p_bitstream_buf->ptr;

  /* first is to make sure input and output pointer are valid and ready*/
  p_engine->input_fetcher.next_byte_to_fetch_offset = offset;

  /**********************************************************************
  then load up the input buffer and bit buffer
  ********************************************************************/
  rc = jpegd_engine_hw_init_bit_buffer(p_engine);
  if(JPEG_FAILED(rc)) {
    JPEG_DBG_ERROR("%s:%d] Error jpegd_engine_hw_init_bit_buffer!!!",
    __func__, __LINE__);
    rc = JPEG_EVENT_ERROR;
    goto decode_error;
  }

  /* set output - MCU line buffers*/
  p_engine->pOutputBufferY = p_engine->dest.p_luma_buffers[0]->ptr;
  p_engine->pOutputBufferCbCr = p_engine->dest.p_chroma_buffers[0]->ptr;
  p_engine->curr_dest_buf_index = 0;

  p_engine->pMCULineBufPtrY    = p_engine->pOutputBufferY;
  p_engine->pMCULineBufPtrCbCr = p_engine->pOutputBufferCbCr;

  for(j = 0, i = bitstream_size-1; i; i--) {
    if ((p_buf[i-1] == 0xFF) && (p_buf[i]==0xD9)/*M_EOI*/) {
      bitstream_size = i-1;
      JPEG_DBG_HIGH("%s:%d] M_EOI detected.  Bitstream size = %d",
        __func__, __LINE__, bitstream_size);
      break;
    }
  }

  if (!i) {
    JPEG_DBG_ERROR("%s:%d] Error getting bitstream content!!!",
      __func__, __LINE__);
    rc = JPEG_EVENT_ERROR;
    goto decode_error;
  }

  rc = jpegd_lib_input_buf_cfg(p_engine->p_handle, &p_engine->in_buf);
  if (rc) {
    JPEG_DBG_ERROR("%s:%d]", __func__, __LINE__);
    rc = JPEG_EVENT_ERROR;
    goto decode_error;
  }

  rc = jpegd_lib_output_buf_cfg(p_engine->p_handle, &p_engine->output_buf);
  if (rc) {
    JPEG_DBG_ERROR("%s:%d]", __func__, __LINE__);
    rc = JPEG_EVENT_ERROR;
    goto decode_error;
  }

  rc = jpegd_lib_decode(p_engine->p_handle);
  if (rc) {
    JPEG_DBG_ERROR("%s:%d] Error Decoding ", __func__, __LINE__);
    rc = JPEG_EVENT_ERROR;
    goto decode_error;
  }

  JPEG_DBG_MED("%s:%d] waiting for frame done event ", __func__, __LINE__);

  os_mutex_lock(&(p_engine->frame_done_mutex));
  if (!p_engine->output_done && !p_engine->error_flag) {
    rc = os_cond_timedwait(&p_engine->frame_done_cond,
      &(p_engine->frame_done_mutex), 10000);
  }
  os_mutex_unlock(&(p_engine->frame_done_mutex));

  JPEG_DBG_MED("%s:%d] wait done %d", __func__, __LINE__, rc);

  if (p_engine->error_flag || (JPEGERR_ETIMEDOUT == rc)) {
    JPEG_DBG_ERROR("%s:%d] error event", __func__, __LINE__);
    jpegd_lib_input_buf_get(p_engine->p_handle, &p_engine->in_buf);
    jpegd_lib_output_buf_get(p_engine->p_handle, &p_engine->output_buf);
    p_engine->error_flag = true;
    rc = JPEG_EVENT_ERROR;
    p_engine->source.p_buffers[1-buffer_in_use] =
      (jpeg_buf_t *)saved_source_buf;
    goto decode_error;
  }

  jpegd_lib_input_buf_get(p_engine->p_handle, &p_engine->in_buf);
  jpegd_lib_output_buf_get(p_engine->p_handle, &p_engine->output_buf);

  float sc_fac = g_scale_factor[p_engine->source.hw_scale_factor];

#ifdef DUMP_SCALED_OUTPUT
  char *output_filename = "/data/dec.yuv";
  FILE *fout = fopen(output_filename, "wb");
  if(!fout){
    JPEG_DBG_ERROR("%s:%d] failed to open output file: %s",
      __func__, __LINE__, output_filename);
   } else {
     JPEG_DBG_HIGH("%s:%d] Writing to %s %dx%d s %f",
       __func__, __LINE__, output_filename, width, height, sc_fac);
     fwrite(p_engine->output_buf.vaddr, 1, width *height * SQUARE(sc_fac),
       fout);
     fwrite(p_engine->output_buf.vaddr + luma_size, 1, width *height * .5 *
       SQUARE(sc_fac), fout);
     JPEG_DBG_HIGH("$%s:%d] Image dumped ", __func__, __LINE__);
     fclose(fout);
   }
#endif

  /* send output to engine user*/
#if HW10_USE_PP

  for(i = 0; i <(height / nRowsToProcess); i++) {
    y_offset = (nRowsToProcess*width*i);
    uv_offset = (nRowsToProcess*width*i)>> 1;

    p_out_y_buf->ptr = (uint8_t *)(p_engine->output_buf.vaddr + y_offset);
    p_out_uv_buf->ptr = (uint8_t *)(p_engine->output_buf.vaddr +
      luma_size+uv_offset);


    rc = p_engine->dest.p_output_handler(p_engine->p_wrapper,
      p_out_y_buf, p_out_uv_buf);

    /* if something goes wrong when sending the output, bail out
       from the loop and return event error.*/
    if (JPEG_FAILED(rc)){
       rc = JPEG_EVENT_ERROR;
       goto decode_error;
    }

    /* Wait for the now current set of buffer to be ready*/
    jpeg_buffer_wait_until_empty(p_out_y_buf);
    jpeg_buffer_wait_until_empty(p_out_uv_buf);
  }
#else
  jpeg_update_output_buffers(p_engine);
#endif

  p_engine->source.p_buffers[1-buffer_in_use] = (jpeg_buf_t *)saved_source_buf;
  rc = JPEG_EVENT_DONE;

decode_error:
  if (p_engine->back_to_back_count == 1) {
     JPEG_DBG_LOW("%s:%d] Freeing output buffers", __func__, __LINE__);
     buffers_initialized = false;
     jpeg_buffer_destroy(&p_bitstream_buf);
     jpeg_buffer_destroy(&p_out_y_buf);
     jpeg_buffer_destroy(&p_out_uv_buf);

     if (do_munmap(p_engine->in_buf.fd,
       p_engine->in_buf.vaddr,
       p_engine->in_buf.y_len,
       p_engine->in_buf.ion_fd_main,
       &p_engine->in_buf.fd_ion_map)) {
       JPEG_DBG_ERROR("%s:%d] fail to free ION", __func__, __LINE__);
       return JPEG_EVENT_ERROR;
     }
     p_engine->in_buf.y_len = 0;

     if (do_munmap(p_engine->output_buf.fd,
       p_engine->output_buf.vaddr,
       p_engine->output_buf.y_len,
       p_engine->output_buf.ion_fd_main,
       &p_engine->output_buf.fd_ion_map)) {
       JPEG_DBG_ERROR("%s:%d] fail to free ION", __func__, __LINE__);
       return JPEG_EVENT_ERROR;
     }
     p_engine->output_buf.y_len = 0;
  }
  return rc;
}

/*===========================================================================
 * FUNCTION    - jpegd_engine_hw_decode_thread  -
 *
 * DESCRIPTION:
 *==========================================================================*/
static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
  jpegd_engine_hw_decode_thread(OS_THREAD_FUNC_ARG_T arg) {
  /* Cast input argument as the engine object*/
  jpegd_engine_hw_t *p_engine = (jpegd_engine_hw_t *)arg;
  jpeg_event_t event;
  int rc;

  p_engine_local = p_engine;

  JPEG_DBG_LOW("%s:%d] back_to_back_count = %d",
    __func__, __LINE__, p_engine->back_to_back_count);

  if (!p_engine->is_progressive) {
    JPEG_DBG_LOW("%s:%d] ", __func__, __LINE__);
    event = jpegd_engine_hw_decode_baseline(p_engine);

    if (JPEG_EVENT_ERROR == event) {
      JPEG_DBG_ERROR("%s:%d] Error", __func__, __LINE__);
      p_engine->error_flag = true;
      goto err;
    }
  } else {
    JPEG_DBG_HIGH("%s:%d] progressive decode not supported!!!", __func__,
      __LINE__);
    event = JPEG_EVENT_ERROR;
  }

  rc = jpegd_lib_wait_done(p_engine->p_handle);
  if (rc) {
    JPEG_DBG_ERROR("%s:%d] jpegd_lib_wait_done failed", __func__, __LINE__);
    p_engine->error_flag = true;
    goto err;
  }

err:
  rc = jpegd_lib_release(p_engine->p_handle);
  if (rc)
    JPEG_DBG_ERROR("%s:%d] release failed", __func__, __LINE__);

  /* Set the active flag to inactive*/
  os_mutex_lock(&(p_engine->mutex));
  p_engine->is_active = false;
  if (p_engine->abort_flag) {
    p_engine->abort_flag = false;
    os_cond_signal(&p_engine->cond);
    os_mutex_unlock(&(p_engine->mutex));
  } else {
    if (p_engine->error_flag) {
      p_engine->error_flag = false;
      os_mutex_unlock(&(p_engine->mutex));
      /* Notify event jpegd_engine_event_handler()*/
      p_engine->p_event_handler(p_engine->p_wrapper, JPEG_EVENT_ERROR, NULL);
    } else {
      os_mutex_unlock(&(p_engine->mutex));
      /* Notify event jpegd_engine_event_handler()*/
      p_engine->p_event_handler(p_engine->p_wrapper, JPEG_EVENT_DONE, NULL);
    }
  }

  JPEG_DBG_HIGH("%s:%d] Decode thread exit", __func__, __LINE__);
  return OS_THREAD_FUNC_RET_SUCCEEDED;
}
