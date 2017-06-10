/******************************************************************************NS

  @file    qmi_uim_handle.h
  @brief

  DESCRIPTION

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
#ifndef __QMI_UI_HANDLE_H__
#define __QMI_UI_HANDLE_H__

#include "qmi.h"
#include "qmi_client.h"
#include "qmi_uim_srvc.h"
#include "user_identity_module_v01.h"
#include "common_v01.h"
#include "sap_constants.h"

#include "comdef.h"

#define SAP_QMI_TIMEOUT (2000)
/* Used for card status events info at index
 * zero gives the status of the SIM at slot-1
 */
#define SIM_CARD_SLOT_1 (0)

extern uim_sap_request_req_msg_v01 sap_req;

void clear_request_packet(void *buf, size_t size);
int init_qmi_uim();
void cleanup_qmi_uim();
int qmi_get_card_status();
int qmi_sap_connect(sap_status_change *status);
int qmi_sap_disconnect(uim_sap_disconnect_mode_enum_v01 a_mode, boolean isSocketValid);
int qmi_sap_transfer_apdu();
int qmi_sap_transfer_atr();
int qmi_sap_power_on_sim();
int qmi_sap_power_off_sim();
int qmi_sap_get_card_reader_stat();
int qmi_sap_reset_sim();



#endif /*__QMI_UI_HANDLE_H__*/

