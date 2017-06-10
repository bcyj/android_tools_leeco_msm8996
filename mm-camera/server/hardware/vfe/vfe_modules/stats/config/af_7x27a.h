/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __AF_STATS_H__
#define __AF_STATS_H__


#define NUM_AF_MULTI_WINDOW_GRIDS 16
#define NUM_AF_STAT_OUTPUT_BUFFERS 3

typedef enum {
  VFE_DISABLE_AUTOFOCUS_STATISTICS,
  VFE_ENABLE_AUTOFOCUS_STATISTICS,
  /* Used for count purposes only */
  VFE_LAST_AUTOFOCUS_STATISTICS_ENUM =
    VFE_ENABLE_AUTOFOCUS_STATISTICS,
} VFE_AutofocusStatsEnableType;

typedef enum {
  VFE_SINGLE_AUTOFOCUS_WINDOW_MODE,
  VFE_MULTIPLE_AUTOFOCUS_WINDOWS_MODE,
  VFE_LAST_AUTOFOCUS_WINDOW_MODE_ENUM =
    VFE_MULTIPLE_AUTOFOCUS_WINDOWS_MODE,
} VFE_AutofocusWindowModeType;

typedef enum {
  VFE_CALCULATE_SUM_OF_AUTOFOCUS_METRICS_IN_ROW,
  VFE_CALCULATE_MAX_OF_AUTOFOCUS_METRICS_IN_ROW,
  VFE_LAST_AUTOFOCUS_METRIC_SELECT_ENUM =
    VFE_CALCULATE_MAX_OF_AUTOFOCUS_METRICS_IN_ROW,
} VFE_AutofocusMetricSelectionType;

typedef struct {
  uint8_t af_enable:1;
  uint8_t af_busprioritysel:1;
  uint32_t af_buspriorityval:4;
  uint32_t /* reserved */ :26;

  /* Autofocus Statistic Configuration, Part 1 */
  uint32_t af_singlewinvoffset:12;
  uint32_t /* reserved */ :4;
  uint32_t af_singlewinhoffset:12;
  uint32_t /* reserved */ :3;
  uint8_t af_winmode:1;

  /* Autofocus Statistic Configuration, Part 2 */
  uint32_t af_singglewinvh:11;
  uint32_t /* reserved */ :5;
  uint32_t af_singlewinhw:11;
  uint32_t /* reserved */ :5;

  /* Autofocus Statistic Configuration, Parts 3-6 */
  uint8_t af_multiwingrid[NUM_AF_MULTI_WINDOW_GRIDS];

  /* Autofocus Statistic Configuration, Part 7 */
  int32_t af_metrichpfcoefa00:5;
  int32_t af_metrichpfcoefa04:5;
  uint32_t af_metricmaxval:11;
  uint8_t af_metricsel:1;
  uint32_t /* reserved */ :10;

  /* Autofocus Statistic Configuration, Part 8 */
  int32_t af_metrichpfcoefa20:5;
  int32_t af_metrichpfcoefa21:5;
  int32_t af_metrichpfcoefa22:5;
  int32_t af_metrichpfcoefa23:5;
  int32_t af_metrichpfcoefa24:5;
  uint32_t /* reserved */ :7;

  /* Autofocus Statistic Output Buffer Header */
  uint32_t af_metrichp:8;
  uint32_t /* reserved */ :24;

  /* Autofocus Statistic Output Buffers - MUST BE 64 bit ALIGNED!!! */
  void *af_outbuf[NUM_AF_STAT_OUTPUT_BUFFERS];
} __attribute__ ((packed, aligned(4))) vfe_cmd_stats_af_t;

typedef struct {
  vfe_cmd_stats_af_t cmd;
  int8_t enable;
  int8_t af_update;
  int8_t use_hal_buf;
}af_stats_t;

vfe_status_t vfe_af_stats_init(af_stats_t *stats, vfe_params_t *vParams);
vfe_status_t vfe_af_stats_enable(af_stats_t *stats, vfe_params_t *params,
  int enable, int8_t hw_write);
vfe_status_t vfe_af_stats_config(void *ctrl, vfe_stats_af_params_t *mctl_af);
vfe_status_t vfe_af_stats_update(af_stats_t *mod, vfe_params_t* parms);
vfe_status_t vfe_af_stats_stop(af_stats_t *stats, vfe_params_t *params);

#endif /* __AF_STATS_H__ */
