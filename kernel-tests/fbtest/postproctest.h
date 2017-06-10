/******************************************************************************
@file  postproctest.h
@brief This file contains test code to verify display post processing
functionalities of msm_fb for MDP.

DESCRIPTION
Mobile Display Processor (MDP) provides sophisticated image processing features
for display panel to enhance display picture quality. This file contains
implementation of display post processing tests.

-----------------------------------------------------------------------------
Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
-----------------------------------------------------------------------------

******************************************************************************/

#ifndef __POSTPROC_TEST_H__
#define __POSTPROC_TEST_H__

#include <stdlib.h>
#include <cutils/sockets.h>
#include "fbtest.h"
#include "fbtestUtils.h"
#ifdef MDP3_FLAG
#include "mdp3.h"
#endif
#ifdef MDP4_FLAG
#include "mdp4.h"
#endif
#include "lib-postproc.h"


#define DEFAULT_POSTPROC_FILE_PATH "./postproc_cfg/SanityCfg.cfg"

#define CSC_CALIB_DATA_MAX_FIELDS_MV 9
#define CSC_CALIB_DATA_MAX_FIELDS_BV 3
#define CSC_CALIB_DATA_MAX_FIELDS_LV 6
#define CSC_CALIB_DATA_MAX_LINES 5

#define GAMUT_T0_SIZE		125
#define GAMUT_T1_SIZE		100
#define GAMUT_T2_SIZE		80
#define GAMUT_T3_SIZE		100
#define GAMUT_T4_SIZE		100
#define GAMUT_T5_SIZE		80
#define GAMUT_T6_SIZE		64
#define GAMUT_T7_SIZE		80

#define PA_GLOBAL_ADJ_LINE	0
#define PA_SIX_ZONE_HUE_LINE	1
#define PA_SIX_ZONE_SAT_LINE	2
#define PA_SIX_ZONE_VAL_LINE	3
#define PA_SIX_ZONE_THRESH_LINE	4
#define PA_MEM_COL_SKIN_LINE	5
#define PA_MEM_COL_SKY_LINE	6
#define PA_MEM_COL_FOL_LINE	7

struct gamut_tbl_c {
	uint16_t	c0[GAMUT_T0_SIZE];
	uint16_t	c1[GAMUT_T1_SIZE];
	uint16_t	c2[GAMUT_T2_SIZE];
	uint16_t	c3[GAMUT_T3_SIZE];
	uint16_t	c4[GAMUT_T4_SIZE];
	uint16_t	c5[GAMUT_T5_SIZE];
	uint16_t	c6[GAMUT_T6_SIZE];
	uint16_t	c7[GAMUT_T7_SIZE];
};

struct gamut_tbl {
	struct gamut_tbl_c r;
	struct gamut_tbl_c g;
	struct gamut_tbl_c b;
};

struct display_pp_gm_lut_data{
	uint32_t ops;
	struct gamut_tbl gamut_lut_tbl;

};

extern int parsePPFrameworkFile(const char* cfg_file_path);
extern int display_pp_compute_pa_params(
                            struct display_pp_pa_cfg *pa_params,
                            struct mdp_overlay_pp_params *cfg);
int print_pa_cfg(struct mdp_pa_cfg *pa_cfg_ptr);

#endif //__POSTPROC_TEST_H__
