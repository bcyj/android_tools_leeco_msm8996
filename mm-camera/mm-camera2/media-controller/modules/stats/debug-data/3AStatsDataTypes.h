#ifndef __3A_STATS_DATA_TYPES_H__
#define __3A_STATS_DATA_TYPES_H__

/*===========================================================================

         AWBDebugData D a t a  S t r u c t u r e  D e c l a r a t i o n

*//** @file 3AStatsDataTypes.h
  This header file contains the format of the 3A stats data.


* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/* ==========================================================================

                             Edit History






when       who     what, where, why
--------   ---     -------------------------------------------------------
03/27/14   vb      Initial Revision

========================================================================== */

/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "3ADebugDataTypes.h"

/* ==========================================================================
                       Preprocessor Definitions and Constants
========================================================================== */
/// Max AWB stats size
#define BAYER_GRID_NUM_REGIONS (3072)

/// Number of bayer histogram bins.
#define BAYER_HISTOGRAM_NUM_BINS (256)
/* ==========================================================================
                       Static Declarations
========================================================================== */


/* ==========================================================================
                       Type Declarations
========================================================================== */
#include "3ADebugDataPackStart.h"

/// This struct is used to define the Bayer Grid Stats
typedef struct PACKED
{
    /// Number of horizontal regions for BG stats.
    uint16                          bgStatsNumHorizontalRegions;

    /// Number of vertical regions for BG stats.
    uint16                          bgStatsNumVerticalRegions;

    /// Red channel sum.
    uint32                          redChannelSum[BAYER_GRID_NUM_REGIONS];

    /// Red channel counts per region.
    uint16                          redChannelCount[BAYER_GRID_NUM_REGIONS];

    /// Gr channel sum.
    uint32                          grChannelSum[BAYER_GRID_NUM_REGIONS];

    /// Gr channel counts per region.
    uint16                          grChannelCount[BAYER_GRID_NUM_REGIONS];

    /// Gb channel sum.
    uint32                          gbChannelSum[BAYER_GRID_NUM_REGIONS];

    /// Gb channel count.
    uint16                          gbChannelCount[BAYER_GRID_NUM_REGIONS];

    /// Blue channel sum.
    uint32                          blueChannelSum[BAYER_GRID_NUM_REGIONS];

    /// Blue channel sum.
    uint16                          blueChannelCount[BAYER_GRID_NUM_REGIONS];

} BayerGridStatsType;


/// This struct is used to define the Bayer Histogram stats data.
typedef struct PACKED
{
    /// Saturation percentage of all stats.
    uint32 grChannel[BAYER_HISTOGRAM_NUM_BINS];


} BayerHistogramStatsType;


#include "3ADebugDataPackEnd.h"

#endif // __3A_STATS_DATA_TYPES_H__

