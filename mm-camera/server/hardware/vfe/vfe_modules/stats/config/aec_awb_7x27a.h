/*============================================================================
   Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __AEC_WB_STATS_H__
#define __AEC_WB_STATS_H__

#include "vfe_util_common.h"
#define NUM_WB_EXP_NEUTRAL_REGION_LINES          4
#define NUM_WB_EXP_STAT_OUTPUT_BUFFERS           3

typedef enum {
  VFE_DISABLE_WB_EXP_STATS,
  VFE_ENABLE_WB_EXP_STATS,
  VFE_LAST_WB_EXP_STATS_ENABLE_ENUM = VFE_ENABLE_WB_EXP_STATS,
} VFE_WB_EXPStatsEnableType;

typedef enum {
  VFE_8_X_8_EXPOSURE_STAT_REGIONS,
  VFE_16_X_16_EXPOSURE_STAT_REGIONS,
  VFE_LAST_EXPOSURE_STAT_REGIONS_ENUM = VFE_16_X_16_EXPOSURE_STAT_REGIONS,
} VFE_ExposureStatRegionsType;

typedef enum {
  VFE_8_X_8_EXPOSURE_STAT_SUB_REGIONS,
  VFE_4_X_4_EXPOSURE_STAT_SUB_REGIONS,
  VFE_LAST_EXPOSURE_STAT_SUB_REGIONS_ENUM =
    VFE_4_X_4_EXPOSURE_STAT_SUB_REGIONS,
} VFE_ExposureStatSubRegionsType;

typedef enum {
  VFE_FIXED_OUTPUT_STATISTICS_BUS_PRIORITY,
  VFE_HARDWARE_BUFFER_LEVEL_STATISTICS_BUS_PRIORITY,
  VFE_LAST_STATISTICS_BUS_PRIORITY_ENUM =
    VFE_HARDWARE_BUFFER_LEVEL_STATISTICS_BUS_PRIORITY,
} VFE_StatisticsBusPrioritySelectType;

typedef struct {
  /* White Balance/Exposure Statistic Selection */
  uint8_t wb_expstatsenable:1;
  uint8_t wb_expstatbuspriorityselection:1;
  uint32_t wb_expstatbuspriorityvalue:4;
  uint32_t/* reserved */ :26;

  /* White Balance/Exposure Statistic Configuration, Part 1 */
  uint8_t exposurestatregions:1;
  uint8_t exposurestatsubregions:1;
  uint32_t/* reserved */ :14;

  uint32_t whitebalanceminimumy:8;
  uint32_t whitebalancemaximumy:8;

  /* White Balance/Exposure Statistic Configuration, Part 2 */
  uint8_t wb_expstatslopeofneutralregionline[NUM_WB_EXP_NEUTRAL_REGION_LINES];
  /* White Balance/Exposure Statistic Configuration, Part 3 */
  uint32_t wb_expstatcrinterceptofneutralregionline2:12;
  uint32_t/* reserved */ :4;
  uint32_t wb_expstatcbinterceptofneutralregionline1:12;
  uint32_t/* reserved */ :4;

  /* White Balance/Exposure Statistic Configuration, Part 4 */
  uint32_t wb_expstatcrinterceptofneutralregionline4:12;
  uint32_t/* reserved */ :4;
  uint32_t wb_expstatcbinterceptofneutralregionline3:12;
  uint32_t/* reserved */ :4;

  /* White Balance/Exposure Statistic Output Buffer Header */
  uint32_t wb_expmetricheaderpattern:8;
  uint32_t/* reserved */ :24;

  /* White Balance/Exposure Statistic Output Buffers-MUST
   * BE 64 bit ALIGNED */
  void *wb_expstatoutputbuffer[NUM_WB_EXP_STAT_OUTPUT_BUFFERS];
} __attribute__ ((packed, aligned(4))) vfe_cmd_stats_aecawb_t;

/* Stats WB EXP Update Command */
typedef struct {
  /* Part 1 */
  unsigned int  exposureRegions                                      :  1;
  unsigned int  exposureSubRegions                                   :  1;
  unsigned int  /* reserved */                                       : 14;
  unsigned int  whiteBalanceMinimumY                                 :  8;
  unsigned int  whiteBalanceMaximumY                                 :  8;
  /* Part 2 */
  uint8_t WB_EXPStatSlopeOfNeutralRegionLine[NUM_WB_EXP_NEUTRAL_REGION_LINES];
  /* Part 3 */
  unsigned int  WB_EXPStatCrInterceptOfNeutralRegionLine2            : 12;
  unsigned int  /* reserved */                                       :  4;
  unsigned int  WB_EXPStatCbInterceptOfNeutralRegionLine1            : 12;
  unsigned int  /* reserved */                                       :  4;
  /* Part 4 */
  unsigned int  WB_EXPStatCrInterceptOfNeutralRegionLine4            : 12;
  unsigned int  /* reserved */                                       :  4;
  unsigned int  WB_EXPStatCbInterceptOfNeutralRegionLine3            : 12;
  unsigned int  /* reserved */                                       :  4;
} __attribute__((packed, aligned(4)))vfe_statswb_updatecmdtype_t;

typedef struct {
  vfe_cmd_stats_aecawb_t cmd;
  vfe_statswb_updatecmdtype_t wb_cmd;
  uint32_t rgn_width;
  uint32_t rgn_height;
  int8_t enable;
  int8_t update;
}aecawb_stats_t;

vfe_status_t vfe_aecawb_stats_init(aecawb_stats_t *stats, vfe_params_t *vParams);
vfe_status_t vfe_aecawb_stats_enable(aecawb_stats_t *stats, vfe_params_t *params,
  int enable, int8_t hw_write);
vfe_status_t vfe_aecawb_stats_config(aecawb_stats_t *stats,
  vfe_params_t *vParams);
vfe_status_t vfe_aecawb_stats_update(aecawb_stats_t *mod,
  vfe_params_t *params);
vfe_status_t vfe_aecawb_stats_trigger_update(aecawb_stats_t* mod,
  vfe_params_t *params);
#endif /* __AEC_WB_STATS_H__ */
