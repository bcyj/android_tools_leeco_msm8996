/*!
  @file
  qcril_smsi.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_smsi.h#13 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/08/09   sb      Store message_ref per message, in the reqlist.
10/02/09   sb      Added support for getting and setting the SMSC address.
07/22/09   sb      Added support for latest ril.h under FEATURE_NEW_RIL_API.
06/04/09   sb      Drop CDMA SMS ack if WMS has already acked the message.
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
05/14/09   pg      Mainlined FEATURE_MULTIMODE_ANDROID.
04/05/09   fc      Clenaup log macros.
01/26/08   fc      Logged assertion info.
12/16/08   fc      Changes to support the release of AMSS WMS object for ONCRPC.
06/03/08   sb      First cut implementation.

===========================================================================*/

#ifndef QCRIL_SMSI_H
#define QCRIL_SMSI_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <pthread.h>
#include "comdef.h"
#include "wms.h"
#include "wmsts.h"
#include "ril.h"
#include "ril_cdma_sms.h"
#include "qcrili.h"
#include "qcril_log.h"

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

#define QCRIL_SMS_BC_MM_TABLE_SIZE  WMS_BC_MM_TABLE_MAX

/* The WMS enum for RP causes does not contain all possible values which can be
   sent by the network.  332 means network timeout (Standards 27.005 section 3.2.5) */
#define QCRIL_SMS_RP_CAUSE_NETWORK_TIMEOUT  332                  

/* Buffer to hold a received GW SMS Message or Status Report.  
   Used to translate a message from WMS format to ATCOP format.
   The first byte is for the address length; then comes the address, followed by the SMS PDU.  
   Multiply the whole thing by two since it will be translated to ASCII hex format.
   This buffer is also used for Submit Report Acks. */
#define QCRIL_SMS_BUF_MAX_SIZE ( 1 + CM_CALLED_PARTY_BCD_NO_LENGTH + WMS_MAX_LEN )* 2 

/* NV_SMS_UTC_I Persistent System Property */
#define QCRIL_NV_SMS_UTC                       "persist.radio.nv_sms_utc"

/*! @brief SMS command callback event parameters 
*/
typedef struct 
{
  wms_cmd_id_e_type        command;
  wms_cmd_err_e_type       error;
} qcril_sms_command_callback_params_type;

/*! @brief Typedef variables internal to module qcril_sms.c
*/
typedef struct
{
  boolean client_id_is_valid;     /*!< Indicates the validity of QCRIL's WMS client id */
  wms_client_id_type client_id;   /*!< QCRIL's WMS client id */
  boolean client_is_primary;      /*! Indicates whether this is the primary client of the modem */

} qcril_sms_client_info_type;

typedef struct 
{
  boolean gw_ack_pending;                       /* Indicates whether RIL needs to ack a GW SMS message */

  /* QCRIL SMS configures all non-class 2 SMS messages to be WMS_ROUTE_TRANSFER_ONLY,
     so normally the Android application is responsible for acking the message.
     However, in a few cases, WMS acks the message and overwrites the route to be
     WMS_ROUTE_TRANSFER_AND_ACK.  In that case, QCRIL SMS will drop the ack from
     the Android application. */
  boolean gw_ack_is_needed;

  wms_transaction_id_type gw_transaction_id;    /* Transaction id for the GW SMS message to be acked */

  /* QCRIL SMS configures all non-class 2 SMS messages to be WMS_ROUTE_TRANSFER_ONLY,
     so normally the Android application is responsible for acking the message.
     However, in a few cases, WMS acks the message and overwrites the route to be
     WMS_ROUTE_TRANSFER_AND_ACK.  In that case, QCRIL SMS will drop the ack from
     the Android application. */
  boolean cdma_ack_is_needed;
  boolean cdma_ack_pending;                     /* Indicates whether RIL needs to ack a CDMA SMS message */
  boolean cdma_ack_is_requested;                /* Indicates whether the client has requested a SMS ack */
  wms_transaction_id_type cdma_transaction_id;  /* Transaction id for the CDMA SMS message to be acked */
} qcril_sms_ack_info_type;

typedef struct
{
  qcril_sms_client_info_type client_info[ QCRIL_MAX_MODEM_ID ];               /* WMS client info */
  qcril_sms_ack_info_type sms_ack_info[ QCRIL_MAX_MODEM_ID ];                 /* Ack info */
  pthread_mutex_t sms_ack_info_mutex;                                         /* Mutex to protect access to SMS ACK info */

  #ifdef FEATURE_QCRIL_IMS
  int ims_client_msg_ref;                                                     /* Client message reference for SMS over IMS (3GPP2 format) */
  int ims_wms_msg_ref;                                                        /* WMS message reference for SMS over IMS (3GPP2 format) */
  wms_msg_transport_reg_info_s_type transport_reg_info[ QCRIL_MAX_MODEM_ID ]; /* Transport registration info */
  pthread_mutex_t transport_reg_info_mutex;                                   /* Mutex to protect access to transport reg info */
  #endif /* FEATURE_QCRIL_IMS */

  wms_address_s_type smsc_address;                                            /* Hold the SMSC address */

} qcril_sms_struct_type;

void qcril_sms_clearing_cdma_ack( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id );

#endif /* QCRIL_SMSI_H */
