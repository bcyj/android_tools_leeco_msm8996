/* q3a_stats.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __Q3A_STATS_H__
#define __Q3A_STATS_H__
#include "q3a_stats_hw.h"
#include "camera_dbg.h"

#define MAX_ZOOMS_CNT             79
#define MAX_EXP_BRACKETING_LENGTH 32
#define MAX_STATS_ROI_NUM          5

#define STATS_PROC_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define STATS_PROC_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define FLOAT_TO_Q(exp, f)   ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

#undef  MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/** stats_type_t
 *
 *  This enumeration defines the types of the stats coming from the ISP
 *  Entries beginning with STATS_B are for the bayer stats, the rest are for
 *  the legacy YUV stats.
 */
typedef enum {
  /* Bayer stats */
  STATS_BE     = (1<<0),
  STATS_BG     = (1<<1),
  STATS_BF     = (1<<2),
  STATS_BHISTO = (1<<3),

  /* YUV stats */
  STATS_AEC    = (1<<8),
  STATS_AWB    = (1<<9),
  STATS_AF     = (1<<10),
  STATS_CS     = (1<<11),
  STATS_RS     = (1<<12),
  STATS_IHISTO = (1<<13),
  STATS_HDR_VID = (1<<14),
} stats_type_t;

/** q3a_operation_mode_t
 *
 *  This enumeration defines the operation modes for the module.
 */
typedef enum {
  Q3A_OPERATION_MODE_NONE,
  Q3A_OPERATION_MODE_INIT,
  Q3A_OPERATION_MODE_PREVIEW,
  Q3A_OPERATION_MODE_SNAPSHOT,
  Q3A_OPERATION_MODE_CAMCORDER,
  Q3A_OPERATION_MODE_INVALID,
} q3a_operation_mode_t;


/** stats_yuv_stats_t
 *    @q3a_aec_stats:   AEC stats from the ISP
 *    @q3a_af_stats:    AF stats from the ISP
 *    @q3a_awb_stats:   AWB stats from the ISP
 *    @histogram:       histogram stats from the ISP
 *    @q3a_cs_stats:    CS stats from the ISP
 *    @q3a_rs_stats:    RS stats from the ISP
 *    @q3a_ihist_stats: IHIST stats from the ISP
 *
 *  This structure is used to pass YUV stats from the ISP to the libraries
 *  for processing.
 */
typedef struct {
  q3a_aec_stats_t   q3a_aec_stats;
  q3a_af_stats_t    q3a_af_stats;
  q3a_awb_stats_t   q3a_awb_stats;
  uint32_t          histogram[MAX_HIST_STATS_NUM];
  q3a_cs_stats_t    q3a_cs_stats;
  q3a_rs_stats_t    q3a_rs_stats;
  q3a_ihist_stats_t q3a_ihist_stats;
} stats_yuv_stats_t;

/** stats_bayer_stats_t
 *    @q3a_bg_stats:    Bayer Grid Stats from the ISP
 *    @q3a_bf_stats:    Bayer Focus Stats from the ISP
 *    @q3a_bhist_stats: Bayer Histogram Stats from the ISP
 *
 *  This structure is used to pass BAYER stats from the ISP to the libraries
 *  for processing.
 */
typedef struct {
  q3a_bg_stats_t    q3a_bg_stats;
  q3a_bf_stats_t    q3a_bf_stats;
  q3a_bhist_stats_t q3a_bhist_stats;
} stats_bayer_stats_t;

/** stats_t
 *    @stats_type_mask: This mask shows what types of stats are passed by ISP
 *    @frame_id:        The ID of the frame for which the stats are calculated
 *    @time_stamp:      The timestamp of the message.
 *    @yuv_stats:       The YUV stats from the ISP.
 *    @bayer_stats      The BAYER stats from the ISP.
 *
 *  This structure is used to pass the calculated stats from the ISP to
 *  the libraries for processing. It packs additional information to determine
 *  which frame are these stats for.
 */
typedef struct {
  stats_type_t        stats_type_mask;
  uint32_t            frame_id;
  time_stamp_t        time_stamp;

  stats_yuv_stats_t   yuv_stats;
  stats_bayer_stats_t bayer_stats;
} stats_t;

/** af_stats_t
 *    @stats_type_mask: What is the type of the stats. For this structure
 *                      they will be either STATS_BF or STATS_AF. This will
 *                      tell us what union member to access.
 *    @frame_id:        The ID of the frame for which these stats are
 *                      calculated.
 *    @time_stamp:      The timestamp of the message.
 *    @q3a_af_stats:    If the stats are of type STATS_AF, this member will
 *                      contain the calculated AF stats passed from the ISP.
 *    @q3a_bf_stats:    If the stats are of type STATS_BF, this member will
 *                      contain the calculated BF stats passed from the ISP.
 *
 *  This structure is used to pass the AF/BF stats from ISP to the AF library
 *  for processing.
 */
typedef struct {
  stats_type_t   stats_type_mask;
  uint32_t       frame_id;
  time_stamp_t   time_stamp;

  union {
    q3a_af_stats_t q3a_af_stats;
    q3a_bf_stats_t q3a_bf_stats;
  } u;
} stats_af_t;

/** q3q_flash_sensitivity_t
 *    @off:  No sensitivity for the flash
 *    @low:  Low sensitivity for the flash
 *    @high: High sensitivity for the flash
 *
 *  This structure is used to pass FLASH sensitivity information from AEC
 *  to AWB.
 */
typedef struct {
  float off;
  float low;
  float high;
} q3q_flash_sensitivity_t;

#endif /* __Q3A_STATS_H__ */
