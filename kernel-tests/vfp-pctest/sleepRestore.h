/****************************************************************************
 *
 *       sleepRestore.h
 *
 *  Copyright (c) 2008-2009 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *
 ***************************************************************************/
#ifndef _SLEEPRESTORE_H
#define _SLEEPRESTORE_H

#ifdef __cplusplus
 #define EXTERN_C extern "C"
#else
 #define EXTERN_C extern
#endif

EXTERN_C void vfpv2_regLoad(const int* input);
EXTERN_C void vfpv2_regStore (int*  reg_dump);
EXTERN_C void vfpv2_instructions(int* input);

EXTERN_C void neon_regLoad(const int* input);
EXTERN_C void neon_regStore (int*  reg_dump);
EXTERN_C void neon_instructions(int* input);

#endif //_SLEEPRESTORE_H
