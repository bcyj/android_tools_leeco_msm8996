/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*


            W I R E L E S S   M E S S A G I N G   S E R V I C E S
            -- Translation Services for CDMA SMS

GENERAL DESCRIPTION
  Provides an interface to allow CDMA SMS Messages to be encoded/decoded
  using RIL structures.

Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
              by Qualcomm Technologies, Inc.
  All Rights Reserved.

Export of this technology or software is regulated by the U.S. Government.
Diversion contrary to U.S. law prohibited.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


/* ^L<EJECT> */
/*===========================================================================

$Header: 

===========================================================================*/


/*===========================================================================
========================  INCLUDE FILES =====================================
===========================================================================*/


#include "comdef.h"

#include "wmsts.h"
#include "ril_cdma_sms.h"
#include <string.h>

#define LOG_TAG "WMSTS_RIL"
#ifdef FEATURE_UNIT_TEST
#include "Log.h"
#else
#include <utils/Log.h>
#include "common_log.h"
#endif /* FEATURE_UNIT_TEST */

static wms_raw_ts_data_s_type encoded_sms_data; 

/*=========================================================================
FUNCTION
  ril_cdma_encode_sms

DESCRIPTION
  Encode a CDMA SMS Message.

DEPENDENCIES
  None

RETURN VALUE
  Status of decoding

SIDE EFFECTS
  None

=========================================================================*/
RIL_Errno ril_cdma_encode_sms
(
  RIL_CDMA_SMS_ClientBd * client_bd,     /* Input */
  RIL_CDMA_Encoded_SMS *  encoded_sms    /* Output */
)
{
  wms_client_ts_data_s_type client_ts_data;
  wms_status_e_type status;

  if ((client_bd == NULL) || (encoded_sms == NULL))
  {
    LOGE("NULL pointer passed into ril_cdma_encode_sms.\n");
    return RIL_E_GENERIC_FAILURE;
  }

  memset(&client_ts_data, 0, sizeof(wms_client_ts_data_s_type));

  client_ts_data.format = WMS_FORMAT_CDMA;

  /* Copy each individual field.  We cannot just do a memcpy on the entire bearer data
     structure, because a few fields have been omitted from the RIL interface. */
  client_ts_data.u.cdma.mask = 
  client_bd->mask;

  memcpy(&client_ts_data.u.cdma.message_id,
         &client_bd->message_id,
         sizeof(wms_message_id_s_type));

  memcpy(&client_ts_data.u.cdma.user_data,
         &client_bd->user_data,
         sizeof(wms_cdma_user_data_s_type));

  client_ts_data.u.cdma.user_response = client_bd->user_response;

  memcpy(&client_ts_data.u.cdma.mc_time,
         &client_bd->mc_time,
         sizeof(wms_timestamp_s_type));

  memcpy(&client_ts_data.u.cdma.validity_absolute,
         &client_bd->validity_absolute,
         sizeof(wms_timestamp_s_type));

  memcpy(&client_ts_data.u.cdma.validity_relative,
         &client_bd->validity_relative,
         sizeof(wms_timestamp_s_type));

  memcpy(&client_ts_data.u.cdma.deferred_absolute,
         &client_bd->deferred_absolute,
         sizeof(wms_timestamp_s_type));

  memcpy(&client_ts_data.u.cdma.deferred_relative,
         &client_bd->deferred_relative,
         sizeof(wms_timestamp_s_type));

  client_ts_data.u.cdma.priority = client_bd->priority;
  client_ts_data.u.cdma.privacy = client_bd->privacy;

  memcpy(&client_ts_data.u.cdma.reply_option,
         &client_bd->reply_option,
         sizeof(wms_reply_option_s_type));

  client_ts_data.u.cdma.num_messages = client_bd->num_messages;
  client_ts_data.u.cdma.alert_mode = client_bd->alert_mode;
  client_ts_data.u.cdma.language = client_bd->language;

  memcpy(&client_ts_data.u.cdma.callback,
         &client_bd->callback,
         sizeof(wms_address_s_type));

  client_ts_data.u.cdma.display_mode = client_bd->display_mode;
  client_ts_data.u.cdma.download_mode = WMS_DOWNLOAD_MODE_NONE;

  memcpy(&client_ts_data.u.cdma.delivery_status,
         &client_bd->delivery_status,
         sizeof(wms_delivery_status_s_type));

  client_ts_data.u.cdma.deposit_index = client_bd->deposit_index;

  memcpy(&client_ts_data.u.cdma.other,
         &client_bd->other,
         sizeof(wms_other_parm_s_type));

  /* Encode the message */
  status = wms_ts_encode(&client_ts_data,
                         &encoded_sms_data);

  if (status != WMS_OK_S)
  {
    LOGE("Failed to encode CDMA SMS message.\n");
    return RIL_E_GENERIC_FAILURE;
  }

  encoded_sms->length = (uint8) encoded_sms_data.len;
  encoded_sms->data = encoded_sms_data.data;

  return RIL_E_SUCCESS;

} /* ril_cdma_encode_sms */

/*=========================================================================
FUNCTION
  ril_cdma_decode_sms

DESCRIPTION
  Decode a CDMA SMS Message.

DEPENDENCIES
  None

RETURN VALUE
  Status of decoding

SIDE EFFECTS
  None

=========================================================================*/
RIL_Errno ril_cdma_decode_sms
(
  RIL_CDMA_Encoded_SMS *  encoded_sms,   /* Input */
  RIL_CDMA_SMS_ClientBd * client_bd      /* Output */
)
{
  wms_status_e_type status;
  wms_raw_ts_data_s_type raw_ts_data;
  wms_client_ts_data_s_type client_ts_data;

  if ((client_bd == NULL) || (encoded_sms == NULL))
  {
    LOGE("NULL pointer passed into ril_cdma_decode_sms.\n");
    return RIL_E_GENERIC_FAILURE;
  }

  raw_ts_data.format = WMS_FORMAT_CDMA;
  raw_ts_data.len = encoded_sms->length;
  (void)memcpy(raw_ts_data.data,
               encoded_sms->data,
               encoded_sms->length);

  /* Decode the message */
  status = wms_ts_decode(&raw_ts_data,
                         &client_ts_data);

  if (status != WMS_OK_S)
  {
    LOGE("Failed to decode CDMA SMS message.\n");
    return RIL_E_GENERIC_FAILURE;
  }

  memset(client_bd, 0, sizeof(RIL_CDMA_SMS_ClientBd));

  /* Copy each individual field.  We cannot just do a memcpy on the entire bearer data
     structure, because a few fields have been omitted from the RIL interface. */
  client_bd->mask = client_ts_data.u.cdma.mask;

  memcpy(&client_bd->message_id,
         &client_ts_data.u.cdma.message_id,
         sizeof(wms_message_id_s_type));

  memcpy(&client_bd->user_data,
         &client_ts_data.u.cdma.user_data,
         sizeof(wms_cdma_user_data_s_type));

  client_bd->user_response = client_ts_data.u.cdma.user_response;

  memcpy(&client_bd->mc_time,
         &client_ts_data.u.cdma.mc_time,
         sizeof(wms_timestamp_s_type));

  memcpy(&client_bd->validity_absolute,
         &client_ts_data.u.cdma.validity_absolute,
         sizeof(wms_timestamp_s_type));

  memcpy(&client_bd->validity_relative,
         &client_ts_data.u.cdma.validity_relative,
         sizeof(wms_timestamp_s_type));

  memcpy(&client_bd->deferred_absolute,
         &client_ts_data.u.cdma.deferred_absolute,
         sizeof(wms_timestamp_s_type));

  memcpy(&client_bd->deferred_relative,
         &client_ts_data.u.cdma.deferred_relative,
         sizeof(wms_timestamp_s_type));

  client_bd->priority = client_ts_data.u.cdma.priority;
  client_bd->privacy = client_ts_data.u.cdma.privacy;

  memcpy(&client_bd->reply_option,
         &client_ts_data.u.cdma.reply_option,
         sizeof(wms_reply_option_s_type));

  client_bd->num_messages = client_ts_data.u.cdma.num_messages;
  client_bd->alert_mode = client_ts_data.u.cdma.alert_mode;
  client_bd->language = client_ts_data.u.cdma.language;

  memcpy(&client_bd->callback,
         &client_ts_data.u.cdma.callback,
         sizeof(wms_address_s_type));

  client_bd->display_mode = client_ts_data.u.cdma.display_mode;

  memcpy(&client_bd->delivery_status,
         &client_ts_data.u.cdma.delivery_status,
         sizeof(wms_delivery_status_s_type));

  client_bd->deposit_index = client_ts_data.u.cdma.deposit_index;

  memcpy(&client_bd->other,
         &client_ts_data.u.cdma.other,
         sizeof(wms_other_parm_s_type));

  return RIL_E_SUCCESS;

} /* ril_cdma_decode_sms */

