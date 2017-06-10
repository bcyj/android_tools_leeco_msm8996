/*==========================================================================

                     FTM BT HCI PFAL Header File

Description
   PFAL API declarations of the ftm bt hci pfal component.

# Copyright (c) 2010 by Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                         Edit History


when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/18/10   rakeshk  Created a header file to hold the PFAL declarations for
                    HCI UART programming
===========================================================================*/
#include "ftm_bt_common.h"

#ifndef __FTM_BT_HCI_PFAL_H__
#define __FTM_BT_HCI_PFAL_H__

/*===========================================================================
FUNCTION   ftm_bt_hci_pfal_set_transport

DESCRIPTION
 sets the type of transport based on the msm type

DEPENDENCIES
  NIL

RETURN VALUE
returns the type of transport

SIDE EFFECTS
  None

===========================================================================*/
boolean ftm_bt_hci_pfal_set_transport(void);

/*===========================================================================
FUNCTION   ftm_bt_hci_pfal_deinit_transport

DESCRIPTION
  Platform specific routine to de-intialise the UART/SMD resource.

DEPENDENCIES
  NIL

RETURN VALUE
  RETURN VALUE
  STATUS_SUCCESS if SUCCESS, else other reasons

SIDE EFFECTS
  None

===========================================================================*/
request_status ftm_bt_hci_pfal_deinit_transport();

/*===========================================================================
FUNCTION   ftm_bt_hci_pfal_init_transport

DESCRIPTION
  Platform specific routine to intialise the UART/SMD resources.

DEPENDENCIES
  NIL

RETURN VALUE
  RETURN VALUE
  STATUS_SUCCESS if SUCCESS, else other reasons

SIDE EFFECTS
  None

===========================================================================*/
request_status ftm_bt_hci_pfal_init_transport ();

/*===========================================================================
FUNCTION   ftm_bt_hci_pfal_nwrite

DESCRIPTION
  Platform specific routine to write the data in the argument to the UART/SMD
  port intialised.

DEPENDENCIES
  NIL

RETURN VALUE
  RETURN VALUE
  STATUS_SUCCESS if SUCCESS, else other reasons

SIDE EFFECTS
  None

===========================================================================*/
request_status ftm_bt_hci_pfal_nwrite(uint8 *buf, int size);

/*===========================================================================
FUNCTION   ftm_bt_hci_pfal_nread

DESCRIPTION
  Platform specific routine to read data from the UART/SMD port intialised into
  the buffer passed in argument.

DEPENDENCIES
  NIL

RETURN VALUE
  RETURN VALUE
  STATUS_SUCCESS if SUCCESS, else other reasons

SIDE EFFECTS
  None

===========================================================================*/
request_status ftm_bt_hci_pfal_nread(uint8 *buf, int size);

/*===========================================================================
FUNCTION   ftm_bt_hci_pfal_changebaudrate

DESCRIPTION
  Platform specific routine to intiate a change in baud rate

DEPENDENCIES
  NIL

RETURN VALUE
  RETURN VALUE
  TRUE if SUCCESS, else FALSE

SIDE EFFECTS
  None

===========================================================================*/
boolean ftm_bt_hci_pfal_changebaudrate (uint32 new_baud);

#endif //__FTM_BT_HCI_PFAL_H__
