/*
* Copyright (c) 2011-2012 Qualcomm Atheros Inc. All Rights Reserved.
* Qualcomm Atheros Proprietary and Confidential.
*/

#ifndef  _PSAT_COMMON_H_
#define  _PSAT_COMMON_H_

#include <stdint.h>
#include "testcmd.h"

#define DEVIDE_COEFF 10000

// CMAC to power lookup table, for platform such as embedded without sophisticated math function
typedef struct {
    uint32_t cmac;
    int32_t  pwr_t10;
} _CMAP_PWR_MAPPING;

extern _CMAP_PWR_MAPPING CmacPwrLkupTbl[];
#define CMAC_PWR_LOOKUP_MAX (sizeof(CmacPwrLkupTbl) / sizeof(_CMAP_PWR_MAPPING))

extern PSAT_SWEEP_TABLE psatSweepTbl[];
extern uint16_t NumEntriesPSTSweepTable;

int32_t interpolate_round(int32_t target, int32_t srcLeft, int32_t srcRight,
	    int32_t targetLeft, int32_t targetRight, int32_t roundUp);
int16_t cmac2Pwr_t10(uint32_t cmac);

#endif //#ifndef _PSAT_COMMON_H_
