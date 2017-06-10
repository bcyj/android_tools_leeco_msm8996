#ifndef QCRIL_UIM_H
#define QCRIL_UIM_H
/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of QUALCOMM Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/sandbox/users/micheleb/ril/qcril_uim.h#4 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/23/15   vr      SIMLOCK Temperory unlock status indication
12/01/14   hh      Support for get MCC and MNC
06/17/14   tl      Added logic to better determine FCI value from AID
06/10/14   tl      Removed array structures for slot specific parameters
04/08/14   yt      Add macro for size of encrypted PIN1 info array
01/21/14   at      Added support for getSelectResponse()
01/09/14   yt      Perform silent PIN verification on SAP disconnect
12/11/13   at      Switch to new QCCI framework
09/11/13   yt      Add slot to encrypted PIN info structure
08/12/13   at      Added support for Long APDU indication
03/15/13   yt      Report SIM_STATUS only after finishing silent PIN verify
01/29/13   yt      Support for third SIM card slot
12/26/12   at      Move over to qcril_uim_srvc.h
05/15/12   at      Support for card status validity TLV
04/13/12   at      Added a new field for checking first card status indication
09/30/11   yt      Added support for ISIM Refresh
06/07/11   yt      Added fdn_status to qcril_uim_original_request_type
04/11/11   yt      Support for modem restart and silent PIN1 verification
01/28/11   at      Refresh registration for Phonebook files by reading PBR
10/06/10   at      Support for handling instance_id passed in requests
09/28/10   at      New struct for original pin request type
09/21/10   at      Added new macro QCRIL_UIM_UPIN_STATE_REPLACES_PIN1
08/19/10   at      Updated qcril_uim_indication_params_type with payload
07/13/10   at      Added support for service type and enum for orig_cmd_type
05/13/10   at      Clean up for merging with mainline
04/13/10   mib     Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "qcril_uim_srvc.h"
#include "qcril_qmi_client.h"

/*===========================================================================

                           DEFINES

===========================================================================*/
#define QCRIL_UIM_MAX_AID_SIZE                  20
#define QCRIL_UIM_MAX_PATH_SIZE                 10
#define QCRIL_UIM_MAX_SESSION_PRIORITY_TYPES     3
#define QCRIL_UIM_UPIN_STATE_REPLACES_PIN1       1
#define QCRIL_UIM_ICCID_LEN                     10
#define QCRIL_UIM_MAX_SELECT_RESP_COUNT          5
#define QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO         2

#define QCRIL_UIM_ADD_ENTRY_TO_REQUEST_LIST(params_ptr)                        \
          {                                                                    \
            qcril_reqlist_public_type   reqlist_entry;                         \
            qcril_reqlist_default_entry( params_ptr->t,                        \
                                         params_ptr->event_id,                 \
                                         params_ptr->modem_id,                 \
                                         QCRIL_REQ_AWAITING_CALLBACK,          \
                                         QCRIL_EVT_NONE,                       \
                                         NULL,                                 \
                                         &reqlist_entry );                     \
            if ( qcril_reqlist_new( params_ptr->instance_id,                   \
                                    &reqlist_entry ) != E_SUCCESS )            \
            {                                                                  \
              /* Fail to add to ReqList */                                     \
              return;                                                          \
            }                                                                  \
          }


/*===========================================================================

                           TYPES

===========================================================================*/

/* -----------------------------------------------------------------------------
   ENUM:      QCRIL_UIM_ORIG_CMD_ENUM_TYPE

   DESCRIPTION:
     Used as a command type in qcril_uim_pin2_original_request_type
-------------------------------------------------------------------------------*/
typedef enum
{
  QCRIL_UIM_ORIG_SIM_UNKNOWN            = 0,
  QCRIL_UIM_ORIG_SIM_IO_READ_BINARY,
  QCRIL_UIM_ORIG_SIM_IO_READ_RECORD,
  QCRIL_UIM_ORIG_SIM_IO_UPDATE_BINARY,
  QCRIL_UIM_ORIG_SIM_IO_UPDATE_RECORD,
  QCRIL_UIM_ORIG_SET_SERVICE_STATUS_FDN
} qcril_uim_orig_cmd_enum_type;


/* -----------------------------------------------------------------------------
   ENUM:      QCRIL_UIM_PROV_SESSION_STATE_TYPE

   DESCRIPTION:
     Used to describe what session state a provisioning app can be in.
-------------------------------------------------------------------------------*/
typedef enum
{
  QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED            = 0,
  QCRIL_UIM_PROV_SESSION_ACTIVATION_IN_PROGRESS,
  QCRIL_UIM_PROV_SESSION_ACTIVATED,
  QCRIL_UIM_PROV_SESSION_DEACTIVATION_IN_PROGESS
} qcril_uim_prov_session_state_type;


/* -----------------------------------------------------------------------------
   ENUM:      QCRIL_UIM_PHONEBOOK_FILE_TAG_TYPE

   DESCRIPTION:
     Used to denote various tag types that can be available in an EF PBR.
-------------------------------------------------------------------------------*/
typedef enum
{
  QCRIL_UIM_FILE_TAG_NONE                     = 0,
  QCRIL_UIM_FILE_TAG_CONST_TYPE_1             = 0xA8,
  QCRIL_UIM_FILE_TAG_CONST_TYPE_2             = 0xA9,
  QCRIL_UIM_FILE_TAG_CONST_TYPE_3             = 0xAA,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_ADN         = 0xC0,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_IAP         = 0xC1,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_EXT1        = 0xC2,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_SNE         = 0xC3,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_ANR         = 0xC4,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_PBC         = 0xC5,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_GRP         = 0xC6,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_AAS         = 0xC7,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_GAS         = 0xC8,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_UID         = 0xC9,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_EMAIL       = 0xCA,
  QCRIL_UIM_FILE_TAG_PRIMITIVE_EF_CCP1        = 0xCB
} qcril_uim_phonebook_file_tag_type;


/* -----------------------------------------------------------------------------
   ENUM:      QCRIL_UIM_FCI_VALUE_TYPE

   DESCRIPTION:
     Indicates the values for the template requested from the card in the SELECT
     command when the application is selected
-------------------------------------------------------------------------------*/
typedef enum
{
  QCRIL_UIM_FCI_VALUE_NO_DATA                   = 0,
  QCRIL_UIM_FCI_VALUE_FCP                       = 1,
  QCRIL_UIM_FCI_VALUE_FCI                       = 2,
  QCRIL_UIM_FCI_VALUE_FCI_WITH_INTERFACES       = 3,
  QCRIL_UIM_FCI_VALUE_FMD                       = 4,
  QCRIL_UIM_FCI_VALUE_FCI_FALLBACK_FCP          = 5
} qcril_uim_fci_value_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_PHONEBOOK_FILE_INFO_TYPE

   DESCRIPTION:
     Structure that holds data about files known as a result of EF PBR read.
     Note: file_tag is value defined per Spec 31.102, Section 4.4.2.1
-------------------------------------------------------------------------------*/
typedef struct
{
  qcril_uim_phonebook_file_tag_type     file_tag;
  qmi_uim_file_id_type                  file_id_info;
} qcril_uim_phonebook_file_info_type;

/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_REFRESH_INFO_TYPE

   DESCRIPTION:
     Structure that holds data about refresh information
-------------------------------------------------------------------------------*/
typedef struct
{
  uint32                                registration_mask;
  struct
  {
    uint16                                num_files;
    qcril_uim_phonebook_file_info_type  * files_ptr;
  }                                       files_info[QMI_UIM_MAX_CARD_COUNT];
} qcril_uim_refresh_info_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_PROV_SESSION_INFO_TYPE

   DESCRIPTION:
     Structure that holds data for all the provisioning session types
-------------------------------------------------------------------------------*/
typedef struct
{
  qcril_uim_prov_session_state_type      session_state_gw_indexes[QCRIL_UIM_MAX_SESSION_PRIORITY_TYPES];
  qcril_uim_prov_session_state_type      session_state_1x_indexes[QCRIL_UIM_MAX_SESSION_PRIORITY_TYPES];
} qcril_uim_prov_session_info_type;

/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_OPEN_CHANNEL_INFO_TYPE

   DESCRIPTION:
     Structure contains the information required to recreate an open channel
     request from the response in case a fallback is required
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char              aid_buffer[QMI_UIM_MAX_AID_LEN];
  unsigned short             aid_size;
  qcril_uim_fci_value_type   fci_value;
  qmi_uim_slot_type          slot;
} qcril_uim_open_channel_info_type;

/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_GET_MCC_MNC_REQ_TYPE

   DESCRIPTION:
     Structure contains the information to retrieve MCC and MNC
-------------------------------------------------------------------------------*/
typedef struct
{
  char    aid_buffer[QMI_UIM_MAX_AID_LEN+1];
  uint8   num_mnc_digits;
  uint16  file_id;
} qcril_uim_get_mcc_mnc_req_type;

/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_ORIGINAL_REQUEST_TYPE

   DESCRIPTION:
     Structure used to copy relevant details in original request that is needed
     by qmi_uim_callback. Currently supports data only for pin operation APIs
     and change subscription API.
-------------------------------------------------------------------------------*/
typedef struct
{
  qcril_instance_id_e_type             instance_id;
  qcril_modem_id_e_type                modem_id;
  RIL_Token                            token;
  int                                  request_id;
  qmi_uim_session_type                 session_type;
  union
  {
    /* Change subscription */
    qcril_evt_e_type                   qcril_evt;
    /* Set FDN status */
    qmi_uim_service_status_type        fdn_status;
    /* Open logical channel */
    qcril_uim_open_channel_info_type   channel_info;
    /* Get MCC and MNC info */
    qcril_uim_get_mcc_mnc_req_type     mcc_mnc_req;
  }                                    data;
} qcril_uim_original_request_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_PIN1_INFO_TYPE

   DESCRIPTION:
     Structure that holds encrypted PIN1 value along with AID for the
     corresponding application and the card's ICCID
-------------------------------------------------------------------------------*/
typedef struct
{
  uint8          aid_len;
  uint8          aid_value[QMI_UIM_MAX_AID_LEN];
  uint8          iccid_len;
  uint8          iccid_data[QCRIL_UIM_ICCID_LEN];
  uint8          encrypted_pin1_len;
  uint8          encrypted_pin1[QMI_UIM_MAX_PIN_LEN];
  uint8          slot;
  boolean        silent_verify_in_progress;
} qcril_uim_pin1_info_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_LONG_APDU_INFO_TYPE

   DESCRIPTION:
     Structure that holds the info that is required to handle the long APDU
     transaction scenario between the modem & QCRIL.
-------------------------------------------------------------------------------*/
typedef struct
{
  boolean                             in_use;
  uint16                              total_apdu_len;
  uint32                              token;
  uint16                              rx_len;
  uint8                             * apdu_ptr;
  qcril_uim_original_request_type   * original_request_ptr;
} qcril_uim_long_apdu_info_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_SELECT_RESPONSE_INFO_TYPE

   DESCRIPTION:
     Structure that holds the info that is required to handle the long APDU
     transaction scenario between the modem & QCRIL.
-------------------------------------------------------------------------------*/
typedef struct
{
  boolean                             in_use;
  uint8                               channel_id;
  uint8                               sw1;
  uint8                               sw2;
  uint16                              select_resp_len;
  uint8                             * select_resp_ptr;
} qcril_uim_select_response_info_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_STRUCT_TYPE

   DESCRIPTION:
     Global data with UIM data
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_client_type                       qmi_handle;
  qmi_uim_card_status_type              card_status;
  qcril_uim_prov_session_info_type      prov_session_info;
  qcril_uim_refresh_info_type           refresh_info;
  boolean                               silent_pin_verify_reqd;
  qcril_uim_pin1_info_type              pin1_info[QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO];
  qcril_uim_long_apdu_info_type         long_apdu_info;
  qcril_uim_select_response_info_type   select_response_info[QCRIL_UIM_MAX_SELECT_RESP_COUNT];
  qmi_uim_bool_type                     temp_unlock_status;
} qcril_uim_struct_type;

extern qcril_uim_struct_type   qcril_uim;

/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_PIN2_ORIGINAL_REQUEST_TYPE

   DESCRIPTION:
     Structure used to copy the original request when the PIN 2
     needs to be verified before executing the request
-------------------------------------------------------------------------------*/
typedef struct
{
  uint32                                     size;
  qcril_uim_orig_cmd_enum_type               cmd;
  RIL_Token                                  token;
  qcril_instance_id_e_type                   instance_id;
  qcril_modem_id_e_type                      modem_id;
  union
  {
    qmi_uim_read_transparent_params_type     read_transparent;
    qmi_uim_read_record_params_type          read_record;
    qmi_uim_write_transparent_params_type    write_transparent;
    qmi_uim_write_record_params_type         write_record;
    qmi_uim_set_service_status_params_type   set_service_status;
  }                                          data;
  int                                        pin2_result;
  uint8                                      pin2_num_retries;
} qcril_uim_pin2_original_request_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_INDICATION_PARAMS_TYPE

   DESCRIPTION:
     Structure used to copy indications received from the
     modem
-------------------------------------------------------------------------------*/
typedef struct
{
  qcril_instance_id_e_type         instance_id;
  qcril_modem_id_e_type            modem_id;
  qmi_uim_indication_id_type       ind_id;
  qmi_uim_indication_data_type     ind_data;
  void                           * user_data;
  uint8                            payload;
} qcril_uim_indication_params_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_CALLBACK_PARAMS_TYPE

   DESCRIPTION:
     Structure used to copy the parameters of a callback
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_rsp_data_type                      qmi_rsp_data;
  qcril_uim_original_request_type          * orig_req_data;
  uint32                                     payload;
} qcril_uim_callback_params_type;


/*=========================================================================

  FUNCTION:  qcril_uim_response

===========================================================================*/
/*!
    @brief
    Send a response to framework for a specific request.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_response
(
  qcril_instance_id_e_type  instance_id,
  RIL_Token                 token,
  RIL_Errno                 result,
  void*                     rsp_ptr,
  size_t                    rsp_len,
  boolean                   remove_entry,
  char*                     logstr
);


/*=========================================================================

  FUNCTION:  qmi_uim_callback

===========================================================================*/
/*!
    @brief
    Callback for QMI commands.

    @return
    None
*/
/*=========================================================================*/
void qmi_uim_callback
(
  qmi_uim_rsp_data_type        * rsp_data_ptr,
  void                         * user_data
);


/*=========================================================================

  FUNCTION:  qcril_uim_reset_state

===========================================================================*/
/*!
    @brief
    Reset state of QCRIL_UIM at power up and whenever modem resets.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_reset_state
(
  void
);


/*=========================================================================

  FUNCTION:  qcril_uim_init_state

===========================================================================*/
/*!
    @brief
    Initializes QMI_UIM service and state of QCRIL_UIM. Called during
    initial power up and whenever modem restarts.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_init_state
(
  void
);

#endif /* QCRIL_UIM_H */

