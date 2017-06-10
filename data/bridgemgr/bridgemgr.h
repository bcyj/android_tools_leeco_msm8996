/******************************************************************************

                        B R I D G E M G R . H

******************************************************************************/

/******************************************************************************

  @file    bridgemgr.h
  @brief   Bridge Manager Main Functions Header File

  DESCRIPTION
  Header file for BridgeMgr main functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

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

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/25/11   sg         Initial version

******************************************************************************/

#ifndef __BRIDGEMGR_H__
#define __BRIDGEMGR_H__

#ifdef FEATURE_DS_LINUX_ANDROID
#include <android/log.h>
#include <utils/Log.h>
#endif
#include "comdef.h"
#include "ds_util.h"
#include "ds_cmdq.h"
#include "bridgemgr_common.h"
#include "bridgemgr_cmdq.h"


/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define BRIDGEMGR_SUCCESS  (0)
#define BRIDGEMGR_FAILURE  (-1)

/* Define for dummy client file descriptor */
#define BRIDGEMGR_FD_NONE     (-1)

/* Define for an invalid file descriptor */
#define BRIDGEMGR_FD_INVALID  (-2)

#define BRIDGEMGR_FD_CLOSE(fd)    do                               \
                                  {                                \
                                    if (BRIDGEMGR_IS_VALID_FD(fd)) \
                                    {                              \
                                      close(fd);                   \
                                    }                              \
                                    fd = BRIDGEMGR_FD_INVALID;     \
                                  } while(0)

#define BRIDGEMGR_IS_VALID_FD(fd) ((fd) != BRIDGEMGR_FD_NONE && \
                                   (fd) != BRIDGEMGR_FD_INVALID)

#define BRIDGEMGR_IS_VALID_SYS_TYPE(sys) (((sys) >= BRIDGEMGR_SYS_FIRST) && \
                                          ((sys) < BRIDGEMGR_SYS_MAX))

#define BRIDGEMGR_MAX_RETRY_COUNT  30      /* Max number of retries */
#define BRIDGEMGR_RETRY_DELAY      500000  /* Retry delay in usec */

#define BRIDGEMGR_MAX_STR_LEN      100

#define BRIDGEMGR_TO_STR(x)        (#x)

#define BRIDGEMGR_RET_TO_STR(x)    ((BRIDGEMGR_SUCCESS == x) ? "SUCCESS" : "FAILURE")


/* Extern variables */
extern boolean  bridgemgr_init_complete; /* Flag indicating power-up init completion */

#ifdef FEATURE_DATA_INTERNAL_LOG_TO_FILE
extern FILE     *bm_fp;
#endif


/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_cmd_dispatcher
===========================================================================*/
/*!
@brief
  This function calls the processing function of the subsystem module for
  which the command is destined

@param
  cmd_data - information about the command data

@return
  BRIDGEMGR_SUCCESS - if command data was successfully processed
  BRIDGEMGR_FAILURE - otherwise

@note
  This function gets called in the context of cmdq thread
*/
/*=========================================================================*/
int bridgemgr_cmd_dispatcher
(
  bridgemgr_cmdq_cmd_data_type *data
);


/*===========================================================================
  FUNCTION  bridgemgr_forward_data
===========================================================================*/
/*!
@brief
  This function is used to forward data to the given subsystem module

@param
  sys  - The module to send the data to
  data - Pointer to the data to be sent
  size - Size of the data

@return
  BRIDGEMGR_SUCCESS - if data was successfully sent
  BRIDGEMGR_FAILURE - otherwise

*/
/*=========================================================================*/
int bridgemgr_forward_data
(
  bridgemgr_sys_type sys,
  const void *data,
  int size
);

#endif /* __BRIDGEMGR_H__ */

