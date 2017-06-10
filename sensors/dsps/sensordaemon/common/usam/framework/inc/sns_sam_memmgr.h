#ifndef SNS_SAM_MEMMGR_H
#define SNS_SAM_MEMMGR_H

/*============================================================================
  @file sns_sam_memmgr.h

  @brief
  Contains SAM's, macros and wrapper functions around the sensors's memroy
  manager's interface.

  @note
  File line length should generally be limited to <= 80 columns.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

#include "sns_memmgr.h"
#include "sns_common.h"


/*==============================================================================
  MACROS
==============================================================================*/

#define SNS_SAM_MEM_FREE(block)           SNS_OS_ANY_FREE((void *)(block))
#define SNS_SAM_MEM_ALLOC(block, size)    SNS_OS_ANY_MALLOC((uint8_t)(block), (uint32_t)(size))

#endif /* SNS_SAM_MEMMGR_H */
