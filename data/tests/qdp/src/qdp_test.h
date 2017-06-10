/*!
  @file
  qdp_test.h

  @brief
  common definitions for qdp tests

*/

/*===========================================================================

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/15/10   js      created file

===========================================================================*/
#include <stdlib.h>
#include <stdio.h>

#define QDP_TEST_APN_NAME "qdp-test-apn"
#define QDP_TEST_NAI "qdp_test_nai"

#define QDP_TEST_LOG(...) \
  QDP_LOG_DEBUG(__VA_ARGS__); \
  printf(__VA_ARGS__); \
  printf("\n")
