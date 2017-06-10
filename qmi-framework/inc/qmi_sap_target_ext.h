#ifndef QMI_SAP_TARGET_EXT_H
#define QMI_SAP_TARGET_EXT_H
/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QSAP_MALLOC(size)      malloc(size)
#define QSAP_CALLOC(num, size) calloc(num, size)
#define QSAP_FREE(ptr)         free(ptr)
#define QSAP_REALLOC(ptr,size) realloc(ptr, size)

#ifdef __cplusplus
}
#endif
#endif /* QMI_SAP_TARGET_EXT_H */
