/*!
  @file
  dsi_netctrl_platform.c

  @brief
  This file implements the platform specific functions.

*/

/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/19/10   js      Created
===========================================================================*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "dsi_netctrl_platform.h"

/*===========================================================================
  FUNCTION:  dsi_format_log_msg
===========================================================================*/
/*!
    @brief
    formats log messages

    @return
    none
*/
/*=========================================================================*/
void dsi_format_log_msg
(
  char * buf, 
  int buf_size,
  char * fmt,
  ...
)
{
  va_list ap;

  va_start(ap, fmt);

  vsnprintf(buf, (size_t)buf_size, fmt, ap);

  va_end(ap);
}

void * dsi_malloc(size_t memsize)
{
  void * ptr = malloc(memsize);
  DSI_LOG_VERBOSE("dsi_mem_debug: malloc ptr [%p] memsize [%d]", ptr, memsize);
  return ptr;
}

void dsi_free(void * ptr)
{
  if (NULL==ptr)
  {
    DSI_LOG_DEBUG("%s","dsi_mem_debug: attempt to free NULL ptr");
  }
  else
  {
    DSI_LOG_VERBOSE("dsi_mem_debug free ptr [%p]", ptr);
    free(ptr);
  }
}
