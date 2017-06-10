/***********************************************************************
 * tftp_comdef.h
 *
 * TFTP common definitions which are os agnostic.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * TFTP common definitions which are os agnostic.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-12-30   dks   Fixes to config and log modules.
2014-06-04   rp    Switch to IPCRouter sockets.
2013-12-05   nr    Modify the server code to compile across all HLOS.
2013-12-02   dks   Include stdint needed for compilation of common files.
2013-11-21   nr    Create

===========================================================================*/

#ifndef __TFTP_COMDEF_H__
#define __TFTP_COMDEF_H__

#include "tftp_config_i.h"

#if defined (TFTP_NHLOS_BUILD)
  #include "tftp_comdef_modem.h"
#elif defined (TFTP_LA_BUILD)
  #include "tftp_comdef_la.h"
#elif defined (TFTP_WINDOWS_BUILD)
  #include "tftp_comdef_win.h"
#else
  #error "Comdef : Unknown config"
#endif

#endif /* __TFTP_COMDEF_H_ */
