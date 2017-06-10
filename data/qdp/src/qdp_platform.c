/*!
  @file
  qdp_netctrl_platform.c

  @brief
  This file implements the platform specific functions.

*/

/*===========================================================================

  Copyright (c) 2010,2014 Qualcomm Technologies, Inc. All Rights Reserved

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
08/02/10   js      Created
===========================================================================*/
#include <stdarg.h>
#include <stdio.h>

#include "qdp_platform.h"
#include "qdp.h" /* qdp_deinit */

qdp_sig_handler_t qdp_sig_handler_tbl[] =
{ 
  {SIGILL, NULL} ,
  {SIGSEGV, NULL},
  {SIGBUS, NULL},
  {SIGIOT, NULL},
  {SIGTRAP, NULL},
  {SIGSYS, NULL},
  {SIGTERM, NULL},
  {SIGINT, NULL},
  {SIGQUIT, NULL},
  {SIGHUP, NULL}
};

/*===========================================================================
  FUNCTION:  qdp_format_log_msg
===========================================================================*/
/*!
    @brief
    formats log messages

    @return
    none
*/
/*=========================================================================*/
void qdp_format_log_msg
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

/*===========================================================================
  FUNCTION:  qdp_signal_handler
===========================================================================*/
/*!
    @brief
    processes signals sent to this process. see full list of signals
    in qdp_init

    @return
    void
*/
/*=========================================================================*/
void qdp_signal_handler
(
  int sig
)
{
  unsigned int i=0;

  /* clean up module */
  qdp_deinit();
  
  for(i=0; i<sizeof(qdp_sig_handler_tbl)/sizeof(qdp_sig_handler_t); i++)
  {
    if (qdp_sig_handler_tbl[i].sig == sig &&
        qdp_sig_handler_tbl[i].handler != NULL)
    {
      /* call  default handler */
      QDP_LOG_DEBUG("call default handler [%p] for sig [%d]",
                    qdp_sig_handler_tbl[i].handler,
                    qdp_sig_handler_tbl[i].sig);
      (qdp_sig_handler_tbl[i].handler)(sig);
      break;
    }
  }
}

/*===========================================================================
  FUNCTION:  qdp_platform_init
===========================================================================*/
/*!
    @brief
    platform specific init

    @return
    void
*/
/*=========================================================================*/
void qdp_platform_init(void)
{
  QDP_LOG_DEBUG("%s","qdp_platform_init: ENTRY");

#if 0
  unsigned int i=0;
  sighandler_t temp;

  /* qdp_deinit() from qdp_signal_handler() does not seem to work
     well (hangs) when running from within RIL daemon. disable for now.
     At power up, we clean up previously created profiles */
  /* register for all possible signals that may terminate
   * this process */
  for(i=0; i<sizeof(qdp_sig_handler_tbl)/sizeof(qdp_sig_handler_t); i++)
  {
    temp = qdp_sig_handler_tbl[i].handler;
    qdp_sig_handler_tbl[i].handler = signal(qdp_sig_handler_tbl[i].sig,
                                            qdp_signal_handler);
    /* swap previous handler back if signal() was unsuccessful */
    if (SIG_ERR == qdp_sig_handler_tbl[i].handler)
    {
      qdp_sig_handler_tbl[i].handler = temp;
    }
    QDP_LOG_DEBUG("sig_handler for sig [%d] was [%p]",
                  qdp_sig_handler_tbl[i].sig,
                  qdp_sig_handler_tbl[i].handler);
  }
#endif
  QDP_LOG_DEBUG("%s","qdp_platform_init: EXIT");
}

