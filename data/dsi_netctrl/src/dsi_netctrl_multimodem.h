/*!
  @file
  dsi_netctrl_multimodem.h

  @brief
  This file declares multi modem specific routines used by
  dsi_netctrli.c

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

$Header: //depot/restricted/linux/android/ril/qcril_dsi.c $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/07/10   js      created file

===========================================================================*/

#include "dsi_netctrl.h"

#ifndef _DSI_NETCTRL_MM_
#define _DSI_NETCTRL_MM_

extern int dsi_mm_get_pref_modem();

extern int dsi_mm_iface_look_up
(
  dsi_store_t * st_hndl,
  int *piface,
  int *pmodem,
  int *ptech
);

#endif /* _DSI_NETCTRL_MM_ */
