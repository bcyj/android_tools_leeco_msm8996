/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/v3/jpege/wp8/dev/8612/src/jpege_engine_hybrid.h#3

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/19/13   revathys Created file.

========================================================================== */

#ifndef _JPEGE_HYBRID_ENGLIST_H
#define _JPEGE_HYBRID_ENGLIST_H

#include "jpege_engine_hybrid.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** -----------------------------------------------------------------------*/
// the maximum number of engines that can be used for multithreading
#define MAX_HYBRID_ENGINE 5
//Piece Height is multiple of 48 = 3 * 16 for proper JPEG encoding on each Piece.
//Image is divided into slices whose height is atleast MIN_PIECE_HEIGHT.
//Image is divided into slices of height DEFAULT_PIECE_HEIGHT.Each slice is treated as one job and passed to q6 or sw engine for encoding
#define MIN_PIECE_HEIGHT 48
#ifdef HYBRID_QDSP6_ENCODER_8x10
extern jpege_engine_hybrid_profile_t jpege_engine_q6_profile;
#endif

extern jpege_engine_hybrid_profile_t jpege_engine_sw_profile;

#ifndef JPEGHW_SIMULATOR
#ifdef HYBRID_QDSP6_ENCODER_8x10
static jpege_engine_hybrid_profile_t* hybrid_list[MAX_HYBRID_ENGINE + 1] = {&jpege_engine_q6_profile, &jpege_engine_sw_profile, &jpege_engine_sw_profile, &jpege_engine_sw_profile, 0, 0 };
#else
static jpege_engine_hybrid_profile_t* hybrid_list[MAX_HYBRID_ENGINE + 1] = {&jpege_engine_sw_profile, &jpege_engine_sw_profile, &jpege_engine_sw_profile, &jpege_engine_sw_profile, 0, 0 };
#endif
#else
static jpege_engine_hybrid_profile_t* hybrid_list[MAX_HYBRID_ENGINE + 1] = {&jpege_engine_sw_profile, &jpege_engine_sw_profile, &jpege_engine_sw_profile, &jpege_engine_sw_profile, &jpege_engine_sw_profile, 0 };
#endif /*JPEGHW_SIMULATOR*/
#endif/*_JPEGE_HYBRID_ENGLIST_H*/
