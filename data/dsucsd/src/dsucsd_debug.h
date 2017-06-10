/******************************************************************************

                   D S U C S D _ D E B U G . H

******************************************************************************/

/******************************************************************************

  @file    dsucsd_debug.h
  @brief   DSUCSD Debug Definitions Header File

  DESCRIPTION
  Header file for DSUCSD debug definitions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved

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
11/11/11   sg         Initial version

******************************************************************************/

#ifndef __DSUCSD_DEBUG__
#define __DSUCSD_DEBUG__

#include "ds_util.h"

/*--------------------------------------------------------------------------- 
   Logging macros
---------------------------------------------------------------------------*/
#undef  DS_LOG_TAG
#define DS_LOG_TAG "QC-DSUCSD"

#define DSUCSD_TRACE

extern void ds_log_init2 (int threshold, int mode);

#define  ds_ucsd_log_err      ds_log_err
#define  ds_ucsd_log_high     ds_log_high
#define  ds_ucsd_log_med      ds_log_med
#define  ds_ucsd_log_low      ds_log_low
#define  ds_ucsd_log          ds_log
#define  ds_ucsd_log_dflt     ds_log_dflt
#define  ds_ucsd_log_sys_err  ds_log_sys_err
#define  ds_ucsd_log_init     ds_log_init2

#ifdef DSUCSD_TRACE
  #define DSUCSD_LOG_TRACE_ENTRY        ds_ucsd_log_low("%s: enter", __FUNCTION__)
  #define DSUCSD_LOG_TRACE_EXIT         ds_ucsd_log_low("%s: exit", __FUNCTION__)
  #define DSUCSD_LOG_TRACE_RETURN(ret)  ds_ucsd_log_low("%s: exit %s=%d", __FUNCTION__, #ret, ret)
#else
  #define DSUCSD_LOG_TRACE_ENTRY
  #define DSUCSD_LOG_TRACE_EXIT
  #define DSUCSD_LOG_TRACE_RETURN(ret)
#endif /*DSUCSD_TRACE*/

#endif /* __DSUCSD_DEBUG__ */

