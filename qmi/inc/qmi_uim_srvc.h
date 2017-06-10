#ifndef QMI_UIM_SRVC_H
#define QMI_UIM_SRVC_H

/******************************************************************************
  @file    qmi_uim_srvc.h
  @brief   QMI message library UIM service definitions

  $Id$

  DESCRIPTION
  This file contains common, external header file definitions for QMI
  interface library.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_uim_srvc_init_client() must be called to create one or more clients
  qmi_uim_srvc_release_client() must be called to delete each client when
  finished.

  ---------------------------------------------------------------------------
  Copyright (c) 2010-2012 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
  Defines
---------------------------------------------------------------------------*/
#define QMI_UIM_MAX_CARD_COUNT                      2
#define QMI_UIM_MAX_APP_PER_CARD_COUNT              8
#define QMI_UIM_MAX_PIN_LEN                         16
#define QMI_UIM_MIN_PIN_LEN                         4
#define QMI_UIM_MAX_CK_LEN                          16
#define QMI_UIM_MAX_AID_LEN                         32
#define QMI_UIM_MAX_LABEL_LEN                       32
#define QMI_UIM_MAX_FILE_PATH                       6

#define QMI_UIM_SECURITY_ATTR_MASK_PIN1             0x0001
#define QMI_UIM_SECURITY_ATTR_MASK_PIN2             0x0002
#define QMI_UIM_SECURITY_ATTR_MASK_UPIN             0x0004
#define QMI_UIM_SECURITY_ATTR_MASK_ADM              0x0008

#define QMI_UIM_SERVICE_STATUS_NOT_AVAILABLE        0
#define QMI_UIM_SERVICE_STATUS_AVAILABLE_DISABLED   1
#define QMI_UIM_SERVICE_STATUS_AVAILABLE_ENABLED    2

/*---------------------------------------------------------------------------
  Basic types
---------------------------------------------------------------------------*/

typedef enum
{
  QMI_UIM_FALSE     = 0x00,
  QMI_UIM_TRUE      = 0x01
}qmi_uim_bool_type;

/*---------------------------------------------------------------------------
  ENUMS
---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_SLOT_TYPE

   DESCRIPTION:
     Slot value
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SLOT_1    = 1,
  QMI_UIM_SLOT_2    = 2
} qmi_uim_slot_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_SESSION_TYPE

   DESCRIPTION:
     Session type
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SESSION_TYPE_PRI_GW_PROV          = 0,
  QMI_UIM_SESSION_TYPE_PRI_1X_PROV          = 1,
  QMI_UIM_SESSION_TYPE_SEC_GW_PROV          = 2,
  QMI_UIM_SESSION_TYPE_SEC_1X_PROV          = 3,
  QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_1      = 4,
  QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_2      = 5,
  QMI_UIM_SESSION_TYPE_CARD_SLOT_1          = 6,
  QMI_UIM_SESSION_TYPE_CARD_SLOT_2          = 7
} qmi_uim_session_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_PIN_ID_TYPE

   DESCRIPTION:
     Pin ID
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_PIN_ID_PIN1                       = 1,
  QMI_UIM_PIN_ID_PIN2                       = 2,
  QMI_UIM_PIN_ID_UPIN                       = 3
} qmi_uim_pin_id_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_PIN_OP_TYPE

   DESCRIPTION:
     Set PIN operation
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_PIN_OP_DISABLE                    = 0,
  QMI_UIM_PIN_OP_ENABLE                     = 1
} qmi_uim_pin_op_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_PERSO_OP_TYPE

   DESCRIPTION:
     Perso operation
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_PERSO_OP_DEACTIVATE               = 0,
  QMI_UIM_PERSO_OP_UNBLOCK                  = 1
} qmi_uim_perso_op_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_FILE_TYPE

   DESCRIPTION:
     File type
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_FILE_TYPE_TRANSPARENT             = 0,
  QMI_UIM_FILE_TYPE_CYCLIC                  = 1,
  QMI_UIM_FILE_TYPE_LINEAR_FIXED            = 2,
  QMI_UIM_FILE_TYPE_DEDICATED_FILE          = 3,
  QMI_UIM_FILE_TYPE_MASTER_FILE             = 4
} qmi_uim_file_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_SECURITY_VALUE_TYPE

   DESCRIPTION:
     Security attribute value
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SECURITY_ATTR_ALWAYS              = 0,
  QMI_UIM_SECURITY_ATTR_NEVER               = 1,
  QMI_UIM_SECURITY_ATTR_AND                 = 2,
  QMI_UIM_SECURITY_ATTR_OR                  = 3,
  QMI_UIM_SECURITY_ATTR_SINGLE              = 4
} qmi_uim_security_value_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_CARD_STATE_TYPE

   DESCRIPTION:
     Card status
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_CARD_STATE_ABSENT                 = 0,
  QMI_UIM_CARD_STATE_PRESENT                = 1,
  QMI_UIM_CARD_STATE_ERROR                  = 2,
  QMI_UIM_CARD_STATE_UNKNOWN                = 3
} qmi_uim_card_state_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_PIN_STATUS_TYPE

   DESCRIPTION:
     PIN status
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_PIN_STATE_UNKNOWN                 = 0,
  QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED    = 1,
  QMI_UIM_PIN_STATE_ENABLED_VERIFIED        = 2,
  QMI_UIM_PIN_STATE_DISABLED                = 3,
  QMI_UIM_PIN_STATE_BLOCKED                 = 4,
  QMI_UIM_PIN_STATE_PERM_BLOCKED            = 5
} qmi_uim_pin_status_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_CARD_ERROR_TYPE

   DESCRIPTION:
     Card error
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_CARD_ERROR_UNKNOWN                = 0,
  QMI_UIM_CARD_ERROR_POWER_DOWN             = 1,
  QMI_UIM_CARD_ERROR_POLL_ERROR             = 2,
  QMI_UIM_CARD_ERROR_NO_ATR_RECEIVED        = 3,
  QMI_UIM_CARD_ERROR_VOLT_MISMATCH          = 4,
  QMI_UIM_CARD_ERROR_PARITY_ERROR           = 5,
  QMI_UIM_CARD_ERROR_UNKNOWN_REMOVED        = 6
} qmi_uim_card_error_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_APP_TYPE

   DESCRIPTION:
     Card application type
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_APP_UNKNOWN                       = 0,
  QMI_UIM_APP_SIM                           = 1,
  QMI_UIM_APP_USIM                          = 2,
  QMI_UIM_APP_RUIM                          = 3,
  QMI_UIM_APP_CSIM                          = 4,
  QMI_UIM_APP_ISIM                          = 5
} qmi_uim_app_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_APP_STATE_TYPE

   DESCRIPTION:
     Card application status type
-------------------------------------------------------------------------------*/
typedef enum {
  QMI_UIM_APP_STATE_UNKNOWN         = 0,
  QMI_UIM_APP_STATE_DETECTED        = 1,
  QMI_UIM_APP_STATE_PIN_REQUIRED    = 2,
  QMI_UIM_APP_STATE_PUK1_REQUIRED   = 3,
  QMI_UIM_APP_STATE_PERSO           = 4,
  QMI_UIM_APP_STATE_BLOCKED         = 5,
  QMI_UIM_APP_STATE_ILLEGAL         = 6,
  QMI_UIM_APP_STATE_READY           = 7
} qmi_uim_app_state_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_PERSO_STATE_TYPE

   DESCRIPTION:
     Perso status type
-------------------------------------------------------------------------------*/
typedef enum {
  QMI_UIM_PERSO_STATE_UNKNOWN       = 0,
  QMI_UIM_PERSO_STATE_IN_PROGRESS   = 1,
  QMI_UIM_PERSO_STATE_READY         = 2,
  QMI_UIM_PERSO_STATE_CODE_REQUIRED = 3,
  QMI_UIM_PERSO_STATE_PUK_REQUIRED  = 4,
  QMI_UIM_PERSO_STATE_PERM_BLOCKED  = 5
} qmi_uim_perso_state_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_PERSO_FEATURE_ID_TYPE

   DESCRIPTION:
     Perso feature
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_PERSO_FEATURE_GW_NW       = 0,
  QMI_UIM_PERSO_FEATURE_GW_NS       = 1,
  QMI_UIM_PERSO_FEATURE_GW_SP       = 2,
  QMI_UIM_PERSO_FEATURE_GW_CP       = 3,
  QMI_UIM_PERSO_FEATURE_GW_SIM      = 4,
  QMI_UIM_PERSO_FEATURE_1X_NW1      = 5,
  QMI_UIM_PERSO_FEATURE_1X_NW2      = 6,
  QMI_UIM_PERSO_FEATURE_1X_HRPD     = 7,
  QMI_UIM_PERSO_FEATURE_1X_SP       = 8,
  QMI_UIM_PERSO_FEATURE_1X_CP       = 9,
  QMI_UIM_PERSO_FEATURE_1X_RUIM     = 10,
  QMI_UIM_PERSO_FEATURE_UNKNOWN     = 11
} qmi_uim_perso_feature_id_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_EVENT_MASK_TYPE

   DESCRIPTION:
     Event mask
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_EVENT_MASK_CARD_STATUS    = 0x00000001,
  QMI_UIM_EVENT_MASK_SAP_CONNECTION = 0x00000002,
} qmi_uim_event_mask_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_REFRESH_STAGE

   DESCRIPTION:
     Refresh stages
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_REFRESH_STAGE_WAIT_FOR_OK_TO_INIT             = 0,
  QMI_UIM_REFRESH_STAGE_START                           = 1,
  QMI_UIM_REFRESH_STAGE_END_SUCCESS                     = 2,
  QMI_UIM_REFRESH_STAGE_END_FAILURE                     = 3
} qmi_uim_refresh_stage;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_REFRESH_MODE

   DESCRIPTION:
     Refresh modes
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_REFRESH_MODE_RESET              = 0,
  QMI_UIM_REFRESH_MODE_INIT               = 1,
  QMI_UIM_REFRESH_MODE_INIT_FCN           = 2,
  QMI_UIM_REFRESH_MODE_FCN                = 3,
  QMI_UIM_REFRESH_MODE_INIT_FULL_FCN      = 4,
  QMI_UIM_REFRESH_MODE_APP_RESET          = 5,
  QMI_UIM_REFRESH_MODE_3G_SESSION_RESET   = 6
} qmi_uim_refresh_mode;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_CAPABILITIES_MASK_TYPE

   DESCRIPTION:
     Mask for service capabilities on the card
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_CAPS_MASK_SERVICE_FDN           = 0x00000001
} qmi_uim_capabilities_mask_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_SERVICE_STATUS_TYPE

   DESCRIPTION:
     Indicates the types of service status on the card
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SERVICE_DISABLE               = 0,
  QMI_UIM_SERVICE_ENABLE                = 1
} qmi_uim_service_status_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_ACTIVATION_TYPE

   DESCRIPTION:
     Indicates the operation to be performed for the provisioning session
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SESSION_DEACTIVATE            = 0,
  QMI_UIM_SESSION_ACTIVATE              = 1
} qmi_uim_activation_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_SAP_CONNECTION_OP_TYPE

-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SAP_CONNECTION_DISCONNECT      = 0,
  QMI_UIM_SAP_CONNECTION_CONNECT         = 1,
  QMI_UIM_SAP_CONNECTION_CHECK_STATUS    = 2
} qmi_uim_sap_connection_op_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_SAP_DISCONNECT_MODE_TYPE

-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SAP_DISCONNECT_MODE_IMMEDIATE  = 0,
  QMI_UIM_SAP_DISCONNECT_MODE_GRACEFUL   = 1
} qmi_uim_sap_disconnect_mode_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_SAP_REQUEST_OP_TYPE

-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SAP_REQUEST_OP_GET_ATR        = 0,
  QMI_UIM_SAP_REQUEST_OP_SEND_APDU      = 1,
  QMI_UIM_SAP_REQUEST_OP_POWER_SIM_OFF  = 2,
  QMI_UIM_SAP_REQUEST_OP_POWER_SIM_ON   = 3,
  QMI_UIM_SAP_REQUEST_OP_RESET_SIM      = 4,
  QMI_UIM_SAP_REQUEST_OP_READER_STATUS  = 5
} qmi_uim_sap_request_op_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_SAP_CONNECTION_STATE_TYPE

-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SAP_CONNECTION_STATE_NOT_ENABLED     = 0,
  QMI_UIM_SAP_CONNECTION_STATE_CONNECTING      = 1,
  QMI_UIM_SAP_CONNECTION_STATE_CONNECTED       = 2,
  QMI_UIM_SAP_CONNECTION_STATE_CONNECTION_ERR  = 3,
  QMI_UIM_SAP_CONNECTION_STATE_DISCONNECTING   = 4,
  QMI_UIM_SAP_CONNECTION_STATE_DSICONNECTED    = 5
} qmi_uim_sap_connection_state_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_LOGICAL_CHANNEL_OP_TYPE

   DESCRIPTION:
     Indicates the operation to be performed on the logical channel
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_LOGICAL_CHANNEL_OPEN          = 0,
  QMI_UIM_LOGICAL_CHANNEL_CLOSE         = 1
} qmi_uim_logical_channel_op_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_AUTH_CONTEXT_TYPE

   DESCRIPTION:
     Indicates the possible values for the authentication context
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_AUTH_CONTEXT_RUN_GSM_ALGO            = 0,
  QMI_UIM_AUTH_CONTEXT_RUN_CAVE_ALGO           = 1,
  QMI_UIM_AUTH_CONTEXT_GSM_SECURITY            = 2,
  QMI_UIM_AUTH_CONTEXT_3G_SECURITY             = 3,
  QMI_UIM_AUTH_CONTEXT_VGCS_VBS_SECURITY       = 4,
  QMI_UIM_AUTH_CONTEXT_GBA_BOOTSTRAPPING       = 5,
  QMI_UIM_AUTH_CONTEXT_GBA_NAF_DERIVATION      = 6,
  QMI_UIM_AUTH_CONTEXT_MBMS_MSK_UPDATE         = 7,
  QMI_UIM_AUTH_CONTEXT_MBMS_MTK_GEN            = 8,
  QMI_UIM_AUTH_CONTEXT_MBMS_MSK_DELETION       = 9,
  QMI_UIM_AUTH_CONTEXT_MBMS_MUK_DELETION       = 10,
  QMI_UIM_AUTH_CONTEXT_IMS_AKA_SECURITY        = 11,
  QMI_UIM_AUTH_CONTEXT_HTTP_DIGEST_SECURITY    = 12,
  QMI_UIM_AUTH_CONTEXT_COMPUTE_IP_CHAP         = 13,
  QMI_UIM_AUTH_CONTEXT_COMPUTE_IP_MNHA         = 14,
  QMI_UIM_AUTH_CONTEXT_COMPUTE_IP_MIP_RRQ      = 15,
  QMI_UIM_AUTH_CONTEXT_COMPUTE_IP_MN_AAA       = 16,
  QMI_UIM_AUTH_CONTEXT_COMPUTE_IP_HRPD_ACCESS  = 17
} qmi_uim_auth_context_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_FCI_VALUE_TYPE

   DESCRIPTION:
     Indicates the values for the template requested from the card in the SELECT
     command when the application is selected
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_FCI_VALUE_NO_DATA                   = 0,
  QMI_UIM_FCI_VALUE_FCP                       = 1,
  QMI_UIM_FCI_VALUE_FCI                       = 2,
  QMI_UIM_FCI_VALUE_FCI_WITH_INTERFACES       = 3,
  QMI_UIM_FCI_VALUE_FMD                       = 4
} qmi_uim_fci_value_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_INDICATION_ID_TYPE

   DESCRIPTION:
     Indicates the type of QMI INDICATION
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SRVC_INVALID_IND_MSG,
  QMI_UIM_SRVC_STATUS_CHANGE_IND_MSG,
  QMI_UIM_SRVC_REFRESH_IND_MSG
} qmi_uim_indication_id_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_UIM_RSP_ID_TYPE

   DESCRIPTION:
     Indicates the type of the asynchronous callback
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_UIM_SRVC_NONE_RSP_MSG,
  QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG,
  QMI_UIM_SRVC_READ_RECORD_RSP_MSG,
  QMI_UIM_SRVC_WRITE_TRANSPARENT_RSP_MSG,
  QMI_UIM_SRVC_WRITE_RECORD_RSP_MSG,
  QMI_UIM_SRVC_GET_FILE_ATTRIBUTES_RSP_MSG,
  QMI_UIM_SRVC_REFRESH_REGISTER_RSP_MSG,
  QMI_UIM_SRVC_REFRESH_GET_LAST_EVENT_RSP_MSG,
  QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG,
  QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG,
  QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG,
  QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG,
  QMI_UIM_SRVC_DEPERSONALIZATION_RSP_MSG,
  QMI_UIM_SRVC_POWER_DOWN_RSP_MSG,
  QMI_UIM_SRVC_POWER_UP_RSP_MSG,
  QMI_UIM_SRVC_GET_CARD_STATUS_RSP_MSG,
  QMI_UIM_SRVC_EVENT_REG_RSP_MSG,
  QMI_UIM_SRVC_AUTHENTICATE_RSP_MSG,
  QMI_UIM_SRVC_CLOSE_SESSION_RSP_MSG,
  QMI_UIM_SRVC_GET_SERVICE_STATUS_RSP_MSG,
  QMI_UIM_SRVC_SET_SERVICE_STATUS_RSP_MSG,
  QMI_UIM_SRVC_CHANGE_PROV_SESSION_RSP_MSG,
  QMI_UIM_SRVC_GET_LABEL_RSP_MSG,
  QMI_UIM_SRVC_SEND_APDU_RSP_MSG,
  QMI_UIM_SRVC_SAP_CONNECTION_RSP_MSG,
  QMI_UIM_SRVC_SAP_REQUEST_RSP_MSG,
  QMI_UIM_SRVC_LOGICAL_CHANNEL_RSP_MSG,
  QMI_UIM_SRVC_GET_ATR_RSP_MSG
} qmi_uim_rsp_id_type;

/*---------------------------------------------------------------------------
  COMMON STRUCTURES
---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_DATA_TYPE

   DESCRIPTION:   The generic data structure
     data_len:    Length of data
     data_ptr:    Data
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned short     data_len;
  unsigned char    * data_ptr;
} qmi_uim_data_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SESSION_INFO_TYPE

   DESCRIPTION:   Session information
     session_type:   Type of the session
     aid:            AID value (for non provisioning sessions)
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_type              session_type;
  qmi_uim_data_type                 aid;
} qmi_uim_session_info_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_APP_INFO_TYPE

   DESCRIPTION:   Application information
     session_type:   Type of the session
     aid:            AID value
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_slot_type                 slot;
  qmi_uim_data_type                 aid;
} qmi_uim_app_info_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_FILE_ID_TYPE

   DESCRIPTION:   File information
     file_id:     File ID on the card
     path:        Path of the file
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned short                    file_id;
  qmi_uim_data_type                 path;
} qmi_uim_file_id_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_REFRESH_FILE_ID_TYPE

   DESCRIPTION:   File information that is used only in refresh indication or
                   refresh get last event response
     file_id:     File ID on the card
     path_len:    Length of the valid path of the file
     path_value:  Path of the file
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned short                    file_id;
  unsigned char                     path_len;
  char                              path_value[QMI_UIM_MAX_FILE_PATH];
} qmi_uim_refresh_file_id_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SECURITY_TYPE

   DESCRIPTION:   Security information
     security_value:     Type of the security value
     pin1:
     pin2:
     upin:
     adm:
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_security_value_type       security_value;
  qmi_uim_bool_type                 pin1;
  qmi_uim_bool_type                 pin2;
  qmi_uim_bool_type                 upin;
  qmi_uim_bool_type                 adm;
} qmi_uim_security_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_FILE_CONTROL_INFO_TYPE

   DESCRIPTION:   File control information
     is_valid:     If the value in fci_value is valid
     fci_value:    File control info requested
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_bool_type                    is_valid;
  qmi_uim_fci_value_type               fci_value;
} qmi_uim_file_control_info_type;


/*---------------------------------------------------------------------------
  STRUCTURES USED FOR QMI REQUESTS
---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_READ_TRANSPARENT_PARAMS_TYPE

   DESCRIPTION:   Structure used for read transparent command
     session_info:   Session information
     file_id:        File ID
     offset:         Offset
     length:         Length
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_file_id_type             file_id;
  unsigned short                   offset;
  unsigned short                   length;
} qmi_uim_read_transparent_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_READ_RECORD_PARAMS_TYPE

   DESCRIPTION:   Structure used for read record command
     session_info:   Session information
     file_id:        File ID
     record:         Record
     length:         Length
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_file_id_type             file_id;
  unsigned short                   record;
  unsigned short                   length;
} qmi_uim_read_record_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_WRITE_TRANSPARENT_PARAMS_TYPE

   DESCRIPTION:   Structure used for write transparent command
     session_info:   Session information
     file_id:        File ID
     offset:         Offset
     data:           Data to write
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_file_id_type             file_id;
  unsigned short                   offset;
  qmi_uim_data_type                data;
} qmi_uim_write_transparent_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_WRITE_RECORD_PARAMS_TYPE

   DESCRIPTION:   Structure used for write record command
     session_info:   Session information
     file_id:        File ID
     record:         Record
     data:           Data to write
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_file_id_type             file_id;
  unsigned short                   record;
  qmi_uim_data_type                data;
} qmi_uim_write_record_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_GET_FILE_ATTRIBUTES_PARAMS_TYPE

   DESCRIPTION:   Structure used for get file attributes command
     session_info:   Session information
     file_id:        File ID
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_file_id_type             file_id;
} qmi_uim_get_file_attributes_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_REFRESH_REGISTER_PARAMS_TYPE

   DESCRIPTION:   Structure used for register for refresh command
     session_info:    Session information
     reg_for_refresh: Specifies if the client is registering or deregistering
     vote_for_init:   Specifies if the client wants to vote for init
     num_files:       Number of files
     files_ptr:       List of files
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_bool_type                reg_for_refresh;
  qmi_uim_bool_type                vote_for_init;
  unsigned short                   num_files;
  qmi_uim_file_id_type           * files_ptr;
} qmi_uim_refresh_register_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_REFRESH_OK_PARAMS_TYPE

   DESCRIPTION:   Structure used for ok to refresh command
     session_info:    Session information
     ok_to_refresh:   Specifies if it's ok to refresh or not
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_bool_type                ok_to_refresh;
} qmi_uim_refresh_ok_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_REFRESH_COMPLETE_PARAMS_TYPE

   DESCRIPTION:   Structure used for refresh complete command
     session_info:     Session information
     refresh_success:  Specifies if refresh was completed successfully
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_bool_type                refresh_success;
} qmi_uim_refresh_complete_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_REFRESH_GET_LAST_EVENT_PARAMS_TYPE

   DESCRIPTION:   Structure used for get last refresh event command
     session_info:     Session information
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
} qmi_uim_refresh_get_last_event_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SET_PIN_PROTECTION_PARAMS_TYPE

   DESCRIPTION:   Structure used for set pin protection command
     session_info:   Session information
     pin_operation:  PIN operations
     pin_id:         PIN id
     pin_data:       PIN value
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_pin_op_type              pin_operation;
  qmi_uim_pin_id_type              pin_id;
  qmi_uim_data_type                pin_data;
} qmi_uim_set_pin_protection_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_VERIFY_PIN_PARAMS_TYPE

   DESCRIPTION:   Structure used for PIN verify command
     session_info:   Session information
     pin_id:         PIN id
     pin_data:       PIN value
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_pin_id_type              pin_id;
  qmi_uim_data_type                pin_data;
  qmi_uim_bool_type                is_pin1_encrypted;
} qmi_uim_verify_pin_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_UNBLOCK_PIN_PARAMS_TYPE

   DESCRIPTION:   Structure used for PIN unblock command
     session_info:   Session information
     pin_id:         PIN id
     puk_data:       PUK value
     new_pin_data:   New PIN value
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_pin_id_type              pin_id;
  qmi_uim_data_type                puk_data;
  qmi_uim_data_type                new_pin_data;
} qmi_uim_unblock_pin_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_CHANGE_PIN_PARAMS_TYPE

   DESCRIPTION:   Structure used for PIN change command
     session_info:   Session information
     pin_id:         PIN id
     old_pin_data:   Old PIN value
     new_pin_data:   New PIN value
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type        session_info;
  qmi_uim_pin_id_type              pin_id;
  qmi_uim_data_type                old_pin_data;
  qmi_uim_data_type                new_pin_data;
} qmi_uim_change_pin_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_DEPERSONALIZATION_PARAMS_TYPE

   DESCRIPTION:   Structure used for depersonalization command
     perso_feature:    Perso feature
     perso_operation:  Perso operation
     ck_data:          Control key value
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_perso_feature_id_type    perso_feature;
  qmi_uim_perso_op_type            perso_operation;
  qmi_uim_data_type                ck_data;
} qmi_uim_depersonalization_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_POWER_DOWN_PARAMS_TYPE

   DESCRIPTION:   Structure used for power down command
     slot:    Slot value
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_slot_type                slot;
} qmi_uim_power_down_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_POWER_UP_PARAMS_TYPE

   DESCRIPTION:   Structure used for power up command
     slot:    Slot value
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_slot_type                slot;
} qmi_uim_power_up_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_EVENT_REG_PARAMS_TYPE

   DESCRIPTION:   Structure used for event registration command
     card_status: Indicates if client wants to receive card status indications
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_bool_type                 card_status;
  qmi_uim_bool_type                 sap_connection;
} qmi_uim_event_reg_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_AUTHENTICATE_PARAMS_TYPE

   DESCRIPTION:   Structure used for authenticate command
     session_info: Session information
     auth_context: Authentication context for the data
     auth_data:    Authenticate data
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type         session_info;
  qmi_uim_auth_context_type         auth_context;
  qmi_uim_data_type                 auth_data;
} qmi_uim_authenticate_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_GET_SERVICE_STATUS_PARAMS_TYPE

   DESCRIPTION:   Structure used for get service status command
     session_info: Session information
     mask: Indicates the type of the service for which info is needed
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type         session_info;
  qmi_uim_capabilities_mask_type    mask;
} qmi_uim_get_service_status_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SET_SERVICE_STATUS_PARAMS_TYPE

   DESCRIPTION:   Structure used for set service status command
     session_info: Session information
     mask: Indicates the type of the service for which info is needed
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type         session_info;
  qmi_uim_service_status_type       fdn_status;
} qmi_uim_set_service_status_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_CHANGE_PROV_SESSION_PARAMS_TYPE

   DESCRIPTION:   Structure used for change provisioning session
     app_info: Application information
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_type              session_type;
  qmi_uim_activation_type           activation_type;
  qmi_uim_app_info_type             app_info;
} qmi_uim_change_prov_session_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_GET_LABEL_PARAMS_TYPE

   DESCRIPTION:   Structure used for get label command
     app_info: Application information
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_app_info_type             app_info;
} qmi_uim_get_label_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_CLOSE_SESSION_PARAMS_TYPE

   DESCRIPTION:   Structure used for close session command
     session_info: Session information
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_session_info_type         session_info;
} qmi_uim_close_session_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SEND_APDU_PARAMS_TYPE

   DESCRIPTION:   Structure used for send apdu command
     slot: Slot on with the operation is requested
     apdu: Raw APDU data to be sent
     channel_id_present: Flag that indicates if channel_id is present
     channel_id: Channel id on which operation is requested
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_slot_type                 slot;
  qmi_uim_data_type                 apdu;
  qmi_uim_bool_type                 channel_id_present;
  unsigned char                     channel_id;
} qmi_uim_send_apdu_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SAP_CONNECTION_PARAMS_TYPE

   DESCRIPTION:   Structure used for SAP connection command
     operation_type: Requested operation type
     slot: Slot on with the operation is requested
     disconnect_mode: Disconnect mode type
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_sap_connection_op_type    operation_type;
  qmi_uim_slot_type                 slot;
  qmi_uim_sap_disconnect_mode_type  disconnect_mode;
} qmi_uim_sap_connection_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SAP_REQUEST_PARAMS_TYPE

   DESCRIPTION:   Structure used for SAP request command
     request_type: Request type
     slot: Slot on with the operation is requested
     apdu: APDU data
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_sap_request_op_type       request_type;
  qmi_uim_slot_type                 slot;
  qmi_uim_data_type                 apdu;
} qmi_uim_sap_request_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_LOGICAL_CHANNEL_PARAMS_TYPE

   DESCRIPTION:   Structure used for logical channel command
     operation_type: Type of operation on the logical channel
     slot: Slot on with the operation is requested
     aid: Application id on which the open operation is requested
     channel_id: Channel id on which close opertation is requested
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_logical_channel_op_type   operation_type;
  qmi_uim_slot_type                 slot;
  qmi_uim_file_control_info_type    file_control_information;
  union
  {
    qmi_uim_data_type               aid;
    unsigned char                   channel_id;
  }                                 channel_data;
} qmi_uim_logical_channel_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_GET_ATR_PARAMS_TYPE

   DESCRIPTION:   Structure used for get ATR command
     slot:    Slot value
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_slot_type                slot;
} qmi_uim_get_atr_params_type;

/*---------------------------------------------------------------------------
  DATA TYPES FOR QMI INDICATIONS AND RESPONSES
---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_CARD_STATUS_TYPE

   DESCRIPTION:   Structure used to indicate the card status
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned short                      index_gw_pri_prov;
  unsigned short                      index_1x_pri_prov;
  unsigned short                      index_gw_sec_prov;
  unsigned short                      index_1x_sec_prov;
  unsigned char                       num_slots;
  struct
  {
    qmi_uim_card_state_type           card_state;
    qmi_uim_pin_status_type           upin_state;
    unsigned char                     upin_num_retries;
    unsigned char                     upuk_num_retries;
    qmi_uim_card_error_type           card_error;
    unsigned char                     num_app;
    struct
    {
      qmi_uim_app_type                app_type;
      qmi_uim_app_state_type          app_state;
      qmi_uim_perso_state_type        perso_state;
      qmi_uim_perso_feature_id_type   perso_feature;
      unsigned char                   perso_retries;
      unsigned char                   perso_unblock_retries;
      unsigned char                   aid_len;
      char                            aid_value[QMI_UIM_MAX_AID_LEN];
      unsigned char                   univ_pin;
      qmi_uim_pin_status_type         pin1_state;
      unsigned char                   pin1_num_retries;
      unsigned char                   puk1_num_retries;
      qmi_uim_pin_status_type         pin2_state;
      unsigned char                   pin2_num_retries;
      unsigned char                   puk2_num_retries;
    } application[QMI_UIM_MAX_APP_PER_CARD_COUNT];
  } card[QMI_UIM_MAX_CARD_COUNT];
} qmi_uim_card_status_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_CARD_STATUS_VALIDITY_TYPE

   DESCRIPTION:   Structure used to indicate the validity of the card status

-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_bool_type              card_status_invalid[QMI_UIM_MAX_CARD_COUNT];
} qmi_uim_card_status_validity_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_REFRESH_EVENT_TYPE

   DESCRIPTION:   Structure used to indicate the parameters of a refresh event
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_refresh_stage             refresh_stage;
  qmi_uim_refresh_mode              refresh_mode;
  qmi_uim_session_type              session_type;
  unsigned char                     aid_len;
  char                              aid_value[QMI_UIM_MAX_AID_LEN];
  unsigned short                    num_files;
  qmi_uim_refresh_file_id_type    * files_ptr;
} qmi_uim_refresh_event_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_READ_TRANSPARENT_RSP_TYPE

   DESCRIPTION:   Result of read transparent command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          sw1;
  unsigned char          sw2;
  qmi_uim_data_type      content;
} qmi_uim_read_transparent_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_READ_RECORD_RSP_TYPE

   DESCRIPTION:   Result of read record command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          sw1;
  unsigned char          sw2;
  qmi_uim_data_type      content;
} qmi_uim_read_record_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_WRITE_TRANSPARENT_RSP_TYPE

   DESCRIPTION:   Result of write transparent command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          sw1;
  unsigned char          sw2;
} qmi_uim_write_transparent_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_WRITE_RECORD_RSP_TYPE

   DESCRIPTION:   Result of write record command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          sw1;
  unsigned char          sw2;
} qmi_uim_write_record_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_GET_FILE_ATTRIBUTES_RSP_TYPE

   DESCRIPTION:   Result of get file attributes command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          sw1;
  unsigned char          sw2;
  unsigned short         file_size;
  unsigned short         file_id;
  qmi_uim_file_type      file_type;
  unsigned short         record_size;
  unsigned short         record_count;
  qmi_uim_security_type  read_security;
  qmi_uim_security_type  write_security;
  qmi_uim_security_type  increase_security;
  qmi_uim_security_type  deactivate_security;
  qmi_uim_security_type  activate_security;
  qmi_uim_data_type      raw_value;
} qmi_uim_get_file_attributes_rsp_type;


/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_REFRESH_GET_LAST_EVENT_RSP_TYPE

   DESCRIPTION:   Structure used for refresh indications
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_refresh_event_type     refresh_event;
} qmi_uim_refresh_get_last_evt_rsp_type;


/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SET_PIN_PROTECTION_RSP_TYPE

   DESCRIPTION:   Result of set PIN protection command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          num_retries;
  unsigned char          num_unblock_retries;
  qmi_uim_data_type      encr_pin_data;
} qmi_uim_set_pin_protection_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_VERIFY_PIN_RSP_TYPE

   DESCRIPTION:   Result of verify PIN command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          num_retries;
  unsigned char          num_unblock_retries;
  qmi_uim_data_type      encr_pin_data;
} qmi_uim_verify_pin_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_UNBLOCK_PIN_RSP_TYPE

   DESCRIPTION:   Result of unblock PIN command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          num_retries;
  unsigned char          num_unblock_retries;
  qmi_uim_data_type      encr_pin_data;
} qmi_uim_unblock_pin_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_CHANGE_PIN_RSP_TYPE

   DESCRIPTION:   Result of change PIN command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          num_retries;
  unsigned char          num_unblock_retries;
  qmi_uim_data_type      encr_pin_data;
} qmi_uim_change_pin_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_DEPERSONALIZATION_RSP_TYPE

   DESCRIPTION:   Result of depersonalization command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          num_retries;
  unsigned char          num_unblock_retries;
} qmi_uim_depersonalization_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_EVENT_REG_RSP_TYPE

   DESCRIPTION:   Result of event registration command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned long int      event_mask;
} qmi_uim_event_reg_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_GET_CARD_STATUS_RSP_TYPE

   DESCRIPTION:   Result of get card status command.
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_card_status_type            card_status;
  qmi_uim_card_status_validity_type   card_status_validity;
} qmi_uim_get_card_status_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_AUTHENTICATE_RSP_TYPE

   DESCRIPTION:   Result of authenticate command.
      sw1:        SW1 received from the card
      sw2:        SW2 received from the card
      content:    Authenticate result from the card
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char          sw1;
  unsigned char          sw2;
  qmi_uim_data_type      auth_response;
} qmi_uim_authenticate_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_GET_SERVICE_STATUS_RSP_TYPE

   DESCRIPTION:   Result of get service status command.
      fdn_status_valid:         Indicates if FDN status in response is valid
      fdn_status:               Fixed dialing number status
      hidden_key_status_valid:  Indicates if Hidden Key status in response is 
                                valid
      hidden_key_status:        Status of the hidden key PIN
      index_valid:              Indicates if index in response is valid
      index:                    Index of the application in the EF_DIR file, 
                                starting from 1
      esn_status_valid:         Indicates if ESN status in response is valid
      esn_status:               ESN status
      acl_status_valid:         Indicates if ACL status in response is valid
      acl_status:               Access point name control list service status
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_bool_type          fdn_status_valid;
  unsigned char              fdn_status;
  qmi_uim_bool_type          hidden_key_status_valid;
  unsigned char              hidden_key_status;
  qmi_uim_bool_type          index_valid;
  unsigned char              index;
  qmi_uim_bool_type          esn_status_valid;
  unsigned char              esn_status;
  qmi_uim_bool_type          acl_status_valid;
  unsigned char              acl_status;
} qmi_uim_get_service_status_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_GET_LABEL_RSP_TYPE

   DESCRIPTION:   Result of get label command.
       label_len: Length of the label_value
       label_value: Retrieved label value
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char                   label_len;
  char                            label_value[QMI_UIM_MAX_LABEL_LEN];
} qmi_uim_get_label_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SEND_APDU_RSP_TYPE

   DESCRIPTION:   Result of send apdu command.
       apdu_response: Raw APDu response from the card
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_data_type               apdu_response;
} qmi_uim_send_apdu_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SAP_CONNECTION_RSP_TYPE

   DESCRIPTION:   Result of SAP connection command.
       connection_status: Connection status for the SAP connection
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_sap_connection_state_type    connection_status;
} qmi_uim_sap_connection_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_SAP_REQUEST_RSP_TYPE

   DESCRIPTION:   Result of SAP request command.
       sap_response: SAP response
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_data_type               sap_response;
} qmi_uim_sap_request_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_LOGICAL_CHANNEL_RSP_TYPE

   DESCRIPTION:   Result of logical channel command.
       channel_id: Channel id
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char                   channel_id;
} qmi_uim_logical_channel_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_GET_ATR_RSP_TYPE

   DESCRIPTION:   Result of get ATR command.
       atr_response: ATR response from the card
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_data_type               atr_response;
} qmi_uim_get_atr_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_UIM_REFRESH_IND_TYPE

   DESCRIPTION:   Structure used for refresh indications
                  Additional details for each field needs to be added here
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_uim_refresh_event_type     refresh_event;
} qmi_uim_refresh_ind_type;


/*---------------------------------------------------------------------------
  QMI INDICATIONS
---------------------------------------------------------------------------*/
typedef union
{
  qmi_uim_card_status_type     status_change_ind;
  qmi_uim_refresh_ind_type     refresh_ind;
} qmi_uim_indication_data_type;

typedef void (*qmi_uim_indication_hdlr_type)
(
  int                            user_handle,
  qmi_service_id_type            service_id,
  void                         * user_data,
  qmi_uim_indication_id_type     ind_id,
  qmi_uim_indication_data_type * ind_data_ptr
);

/*---------------------------------------------------------------------------
  QMI asynchronous responses
---------------------------------------------------------------------------*/
typedef struct
{
  int                                       sys_err_code;
  int                                       qmi_err_code;
  qmi_uim_rsp_id_type                       rsp_id;
  union
  {
    qmi_uim_read_transparent_rsp_type       read_transparent_rsp;
    qmi_uim_read_record_rsp_type            read_record_rsp;
    qmi_uim_write_transparent_rsp_type      write_transparent_rsp;
    qmi_uim_write_record_rsp_type           write_record_rsp;
    qmi_uim_get_file_attributes_rsp_type    get_file_attributes_rsp;
    qmi_uim_refresh_get_last_evt_rsp_type   refresh_get_last_event_rsp;
    qmi_uim_set_pin_protection_rsp_type     set_pin_protection_rsp;
    qmi_uim_verify_pin_rsp_type             verify_pin_rsp;
    qmi_uim_unblock_pin_rsp_type            unblock_pin_rsp;
    qmi_uim_change_pin_rsp_type             change_pin_rsp;
    qmi_uim_depersonalization_rsp_type      depersonalization_rsp;
    qmi_uim_get_card_status_rsp_type        get_card_status_rsp;
    qmi_uim_event_reg_rsp_type              event_reg_rsp;
    qmi_uim_authenticate_rsp_type           authenticate_rsp;
    qmi_uim_get_service_status_rsp_type     get_service_status_rsp;
    qmi_uim_get_label_rsp_type              get_label_rsp;
    qmi_uim_send_apdu_rsp_type              send_apdu_rsp;
    qmi_uim_sap_connection_rsp_type         sap_connection_rsp;
    qmi_uim_sap_request_rsp_type            sap_response_rsp;
    qmi_uim_logical_channel_rsp_type        logical_channel_rsp;
    qmi_uim_get_atr_rsp_type                get_atr_rsp;
  }                                         rsp_data;
} qmi_uim_rsp_data_type;


typedef void (*qmi_uim_user_async_cb_type)
(
  int                            user_handle,
  qmi_service_id_type            service_id,
  qmi_uim_rsp_data_type        * rsp_data,
  void                         * user_data
);


/*===========================================================================
  FUNCTION  qmi_uim_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the UIM service.  This function
  must be called prior to calling any other UIM service functions.
  For the time being, the indication handler callback and user data
  should be set to NULL until this is implemented.  Also note that this
  function may be called multiple times to allow for multiple, independent
  clients.

@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
EXTERN qmi_client_handle_type qmi_uim_srvc_init_client
(
  const char                   * dev_id,
  qmi_uim_indication_hdlr_type   user_rx_ind_msg_hdlr,
  void                         * user_rx_ind_msg_hdlr_user_data,
  int                          * qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_uim_srvc_release_client
===========================================================================*/
/*!
@brief
  This function is called to release a client created by the
  qmi_uim_srvc_init_client() function.  This function should be called
  for any client created when terminating a client process, especially
  if the modem processor is not reset.  The modem side QMI server has
  a limited number of clients that it will allocate, and if they are not
  released, we will run out.

@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
EXTERN int qmi_uim_srvc_release_client
(
  int      user_handle,
  int    * qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_uim_reset
===========================================================================*/
/*!
@brief
  Resets UIM service.  If the user_cb function pointer is set to NULL,
  then this function will be invoked synchronously, otherwise it will
  be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value in rsp_data  will give you the QMI error reason.
  Otherwise, qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - Resets UIM service
*/
/*=========================================================================*/
EXTERN int qmi_uim_reset
(
  int                           user_handle,
  qmi_uim_user_async_cb_type    user_cb,
  void                        * user_data,
  int                         * qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_uim_read_transparent
===========================================================================*/
/*!
@brief
  Read a transparent file from the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_read_transparent
(
  int                                           client_handle,
  const qmi_uim_read_transparent_params_type  * params,
  qmi_uim_user_async_cb_type                    user_cb,
  void                                        * user_data,
  qmi_uim_rsp_data_type                       * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_read_record
===========================================================================*/
/*!
@brief
  Read a record from a linear/cyclic file from the card.  If the user_cb
  function pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_read_record
(
  int                                           client_handle,
  const qmi_uim_read_record_params_type       * params,
  qmi_uim_user_async_cb_type                    user_cb,
  void                                        * user_data,
  qmi_uim_rsp_data_type                       * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_write_transparent
===========================================================================*/
/*!
@brief
  Write a transparent file to the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_write_transparent
(
  int                                           client_handle,
  const qmi_uim_write_transparent_params_type * params,
  qmi_uim_user_async_cb_type                    user_cb,
  void                                        * user_data,
  qmi_uim_rsp_data_type                       * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_write_record
===========================================================================*/
/*!
@brief
  Writes a record to a linear/cyclic file from the card.  If the user_cb
  function pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_write_record
(
  int                                               client_handle,
  const qmi_uim_write_record_params_type          * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_get_file_attributes
===========================================================================*/
/*!
@brief
  Gets the file sttributes for a file on the card.  If the user_cb
  function pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_get_file_attributes
(
  int                                                 client_handle,
  const qmi_uim_get_file_attributes_params_type     * params,
  qmi_uim_user_async_cb_type                          user_cb,
  void                                              * user_data,
  qmi_uim_rsp_data_type                             * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_set_pin_protection
===========================================================================*/
/*!
@brief
  Enables or disables specified pin on the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_set_pin_protection
(
  int                                               client_handle,
  const qmi_uim_set_pin_protection_params_type    * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_verify_pin
===========================================================================*/
/*!
@brief
  Verifies the specified pin on the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_verify_pin
(
  int                                               client_handle,
  const qmi_uim_verify_pin_params_type            * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_unblock_pin
===========================================================================*/
/*!
@brief
  Unblocks the specified pin on the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_unblock_pin
(
  int                                                 client_handle,
  const qmi_uim_unblock_pin_params_type             * params,
  qmi_uim_user_async_cb_type                          user_cb,
  void                                              * user_data,
  qmi_uim_rsp_data_type                             * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_change_pin
===========================================================================*/
/*!
@brief
  Changes the specified pin on the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_change_pin
(
  int                                               client_handle,
  const qmi_uim_change_pin_params_type            * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_depersonalization
===========================================================================*/
/*!
@brief
  Deactivates or unblocks specified personalization on the phone.  If the
  user_cb function pointer is set to NULL, this function will be invoked
  synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_depersonalization
(
  int                                               client_handle,
  const qmi_uim_depersonalization_params_type     * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_power_down
===========================================================================*/
/*!
@brief
  Powers down the card.  If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_power_down
(
  int                                               client_handle,
  const qmi_uim_power_down_params_type            * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_power_up
===========================================================================*/
/*!
@brief
  Powers up the card.  If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_power_up
(
  int                                               client_handle,
  const qmi_uim_power_up_params_type              * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_get_card_status
===========================================================================*/
/*!
@brief
  Gets the card status.  If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_get_card_status
(
  int                                           client_handle,
  qmi_uim_user_async_cb_type                    user_cb,
  void                                        * user_data,
  qmi_uim_rsp_data_type                       * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_event_reg
===========================================================================*/
/*!
@brief
  Gets the card status.  If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_event_reg
(
  int                                               client_handle,
  const qmi_uim_event_reg_params_type             * params,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_refresh_register
===========================================================================*/
/*!
@brief
  Registers for file change notifications triggered by the card. If the
  user_cb function pointer is set to NULL, this function will be invoked
  synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_refresh_register
(
  int                                               client_handle,
  const qmi_uim_refresh_register_params_type      * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_refresh_ok
===========================================================================*/
/*!
@brief
  Enables the client to indicate if it is OK to start the refresh procedure.
  This function is supported only synchronously.

@return
  If return code < 0, the operation failed. In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give you
  the QMI error reason.  Otherwise, qmi_err_code will have meaningless data.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_refresh_ok
(
  int                                               client_handle,
  const qmi_uim_refresh_ok_params_type            * params,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_refresh_complete
===========================================================================*/
/*!
@brief
  Indicates to the modem that the client has finished the refresh procedure
  after re-reading all the cached files. This function is supported only
  synchronously.

@return
  If return code < 0, the operation failed. In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give you
  the QMI error reason.  Otherwise, qmi_err_code will have meaningless data.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_refresh_complete
(
  int                                               client_handle,
  const qmi_uim_refresh_complete_params_type      * params,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_refresh_get_last_event
===========================================================================*/
/*!
@brief
  Retreives the latest refresh event. This function is supported only
  synchronously.

@return
  If return code < 0, the operation failed. In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give you
  the QMI error reason.  Otherwise, qmi_err_code will have meaningless data.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_refresh_get_last_event
(
  int                                                 client_handle,
  const qmi_uim_refresh_get_last_event_params_type  * params,
  qmi_uim_rsp_data_type                             * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_authenticate
===========================================================================*/
/*!
@brief
  Issues the authenticate request on the card. If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_authenticate
(
  int                                               client_handle,
  const qmi_uim_authenticate_params_type          * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);

/*===========================================================================
  FUNCTION  qmi_uim_get_service_status
===========================================================================*/
/*!
@brief
  Issues the Service status command that queries for the status of particular
  services in the card e.g FDN, BDN etc. Note that currently only FDN query
  is supported. If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_get_service_status
(
  int                                               client_handle,
  const qmi_uim_get_service_status_params_type    * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);


/*===========================================================================
  FUNCTION  qmi_uim_set_service_status
===========================================================================*/
/*!
@brief
  Issues the Service status command that sets the status of particular
  services in the card e.g FDN, BDN etc. Note that currently only FDN
  service is supported. If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_set_service_status
(
  int                                               client_handle,
  const qmi_uim_set_service_status_params_type    * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);


/*===========================================================================
  FUNCTION  qmi_uim_change_provisioning_session
===========================================================================*/
/*!
@brief
  Issues the change provisioning session command that is used to activate,
  deactivate or switch the provisioning sessions. If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_change_provisioning_session
(
  int                                                     client_handle,
  const qmi_uim_change_prov_session_params_type         * params,
  qmi_uim_user_async_cb_type                              user_cb,
  void                                                  * user_data,
  qmi_uim_rsp_data_type                                 * rsp_data
);


/*===========================================================================
  FUNCTION  qmi_uim_get_label
===========================================================================*/
/*!
@brief
  Issues the get label command that retrieves the label of an application
  from EF-DIR on the UICC card. Note that it will return an error if used on
  an ICC card. If the user_cb function pointer is set to NULL, this function
  will be invoked synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_get_label
(
  int                                               client_handle,
  const qmi_uim_get_label_params_type             * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);


/*===========================================================================
  FUNCTION  qmi_uim_close_session
===========================================================================*/
/*!
@brief
  Issues the close session command for a non-provisioning session that may
  have been opened before by the client. This function is supported only
  synchronously.

@return
  If return code < 0, the operation failed. In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give you
  the QMI error reason.  Otherwise, qmi_err_code will have meaningless data.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_close_session
(
  int                                               client_handle,
  const qmi_uim_close_session_params_type         * params,
  qmi_uim_rsp_data_type                           * rsp_data
);


/*===========================================================================
  FUNCTION  qmi_uim_send_apdu
===========================================================================*/
/*!
@brief
  Issues the request for sending raw APDUs to the card. An optional channel
  id parameter can be used when a logical channel is already opened previously
  using the qmi_uim_logical_channel command. If the user_cb function pointer
  is set to NULL, this function will be invoked synchronously. Otherwise it
  will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_send_apdu
(
  int                                               client_handle,
  const qmi_uim_send_apdu_params_type             * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);


/*===========================================================================
  FUNCTION  qmi_uim_logical_channel
===========================================================================*/
/*!
@brief
  Issues the request for open or close logical channel on a particlar
  application on the UICC card. Note that it will return an error if used on
  an ICC card. If the user_cb function pointer is set to NULL, this function
  will be invoked synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_logical_channel
(
  int                                               client_handle,
  const qmi_uim_logical_channel_params_type       * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);


/*===========================================================================
  FUNCTION  qmi_uim_sap_connection
===========================================================================*/
/*!
@brief
  Issues the request for establishing or releasing a SAP connection to the
  UIM module on the modem. If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_sap_connection
(
  int                                               client_handle,
  const qmi_uim_sap_connection_params_type        * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);


/*===========================================================================
  FUNCTION  qmi_uim_sap_request
===========================================================================*/
/*!
@brief
  Issues the various types of SAP requests after a SAP connection is
  successfully established to the UIM module on the modem. If the user_cb
  function pointer is set to NULL, this function will be invoked
  synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_sap_request
(
  int                                               client_handle,
  const qmi_uim_sap_request_params_type           * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);


/*===========================================================================
  FUNCTION  qmi_uim_get_atr
===========================================================================*/
/*!
@brief
  Issues the request to retrieve the ATR of the specified card. If the
  user_cb function pointer is set to NULL, this function will be invoked
  synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_get_atr
(
  int                                               client_handle,
  const qmi_uim_get_atr_params_type               * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
);

#ifdef __cplusplus
}
#endif

#endif  /* QMI_UIM_SRVC_H */
