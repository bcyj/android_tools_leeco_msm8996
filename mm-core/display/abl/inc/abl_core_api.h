/*=============================================================================

  A D A P T I V E   B A C K L I G H T   C O N T R O L   H E A D E R   F I L E

  DESCRIPTION
  This file contains interface specifications of the adaptive backlight control
  feature for display enhancements on QUALCOMM MSM display drivers. It reduces
  display power consumption by applying only the minimum required backlight for
  any given full video frame. The change in backlight levels is coupled with
  correponding gamma correction to compensate for changing brightness levels.

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
#ifndef DISP_ABL_CORE_API_H
#define DISP_ABL_CORE_API_H

#include <sys/types.h>
#include "abl_oem.h"
#include "disp_osal.h"

#define BL_LUT_SIZE              256

#ifdef HW_ABL

#define BL_HIST_BLOCK            MDP_BLOCK_DMA_P
#ifdef HW_MAX_BIN_128
#define BL_HIST_BINS             128
#define BL_HIST_FACTOR           1
#else
#define BL_HIST_BINS             32
#define BL_HIST_FACTOR           3
#endif

#define BL_HALF_HISTBIN          (1<<(BL_HIST_FACTOR-1))
#else
#define BL_HIST_BINS             256
#define BL_HIST_FACTOR           0
#define BL_HALF_HISTBIN          0
#endif

#define BL_MIN_LUMA              BL_HALF_HISTBIN

#define M_MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define M_MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define M_ABS(a)   (((a) >= 0)  ? (a) : (-a))
#define M_SIGN(x)    (((x) > 0)   ?  1  : (((x) < 0) ? (-1):0))
#define CLIP(x, up, lw) (((x)>=(up)) ? up: ((x)<=(lw))? (lw):(x))

int32_t abl_process(int32_t);
void revert_last_abl_process();
void reset_history();
uint32_t *GetCurLUT(void);
uint32_t *GetCurHistBuf(void);
void FinishABLProc(void);
void FreeABLmem(void);
int32_t HwInfoInit(struct hw_info_hist * hw_para);
int orig_levelInit(bl_oem_api *api_para);
int32_t dataInfoInit(bl_oem_api *api_para);
int PG_Init(bl_oem_api *api_para, int32_t i);
int minBL_Init(bl_oem_api *api_para);
int32_t qualityLevel_Init(bl_oem_api *api_para);
int32_t minRatio_Init(bl_oem_api *, uint32_t , uint32_t);

int32_t hist_preprocess(uint32_t *hist, uint32_t *finalhist);
int32_t directmap_LUT(uint32_t intable[], uint32_t outtable[], int32_t len);
int32_t get_version(char *version);
#endif /* DISP_ABL_CORE_API_H */
