/******************************************************************************
  @file    qmi.h
  @brief   Common, external QMI definitions

  DESCRIPTION
  This file contains common, external header file definitions for QMI
  interface library.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_init() must be called at client process initialization time.
  qmi_connection_init() must be called for each QMI control channel that
  you want to send messages on.
  qmi_release() must be called prior to client process exiting

  $Header: //source/qcom/qct/modem/datacommon/qmimsglib/dev/work/inc/qmi.h#2 $
  $DateTime: 2009/07/15 10:38:12 $
  ---------------------------------------------------------------------------
  Copyright (c) 2007 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef QMI_H
#define QMI_H

#include "qmi_platform_config.h"
#include "qmi_port_defs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef EXTERN
#define EXTERN extern
#endif

/* Typedef for client handles */
typedef int qmi_client_handle_type;

/* qmi-framework also defines these variables in one of its header files
 * If we include both these header files we will get compiler warnings for
 * macro redefinitions. To fix this we will undefine and redefine these
 * values */
#undef QMI_INTERNAL_ERR
#undef QMI_SERVICE_ERR
#undef QMI_TIMEOUT_ERR
#undef QMI_EXTENDED_ERR
#undef QMI_PORT_NOT_OPEN_ERR
#undef QMI_MEMCOPY_ERROR
#undef QMI_INVALID_TXN
#undef QMI_CLIENT_ALLOC_FAILURE

/* Error codes returned from QMI interface functions */
#define QMI_NO_ERR                0       /* No Error */
#define QMI_INTERNAL_ERR               -1      /* Linux/System Error */
#define QMI_SERVICE_ERR               -2      /* QMI error, check QMI error code */
#define QMI_TIMEOUT_ERR           -3      /* Transaction timeout error code */
#define QMI_EXTENDED_ERR           -4      /* SubSystem specific error codes(example: ds_profile) */
#define QMI_PORT_NOT_OPEN_ERR      -5     /* QMI control port is not open */


/* Encode/Decode library specific error codes */
#define QMI_IDL_BUFFER_TOO_SMALL                -6
#define QMI_IDL_ARRAY_TOO_BIG                   -7
#define QMI_IDL_MESSAGE_ID_NOT_FOUND            -8
#define QMI_IDL_TLV_DUPLICATED                  -9
#define QMI_IDL_LENGTH_INCONSISTENCY            -10
#define QMI_IDL_MISSING_TLV                     -11
#define QMI_IDL_PARAMETER_ERROR                 -12
#define QMI_IDL_UNRECOGNIZED_SERVICE_VERSION    -13

/* Additional error codes */
#define QMI_MEMCOPY_ERROR                       -14
#define QMI_INVALID_TXN                         -15
#define QMI_CLIENT_ALLOC_FAILURE                -16
#define QMI_IDL_UNKNOWN_MANDATORY_TLV           -17
#define QMI_SERVICE_NOT_PRESENT_ON_MODEM        -18
#define QMI_IDL_TLV_RESP_ERROR                  -19



/* QMI Error codes.  The value in the qmi error code should be set
** to one these when QMI_SERVICE_ERR is returned
*/
#define QMI_SERVICE_ERR_NONE                          0
#define QMI_SERVICE_ERR_MALFORMED_MSG                 1
#define QMI_SERVICE_ERR_NO_MEMORY                     2
#define QMI_SERVICE_ERR_INTERNAL                      3
#define QMI_SERVICE_ERR_ABORTED                       4
#define QMI_SERVICE_ERR_CLIENT_IDS_EXHAUSTED          5
#define QMI_SERVICE_ERR_UNABORTABLE_TRANSACTION       6
#define QMI_SERVICE_ERR_INVALID_CLIENT_ID             7
#define QMI_SERVICE_ERR_NO_THRESHOLDS                 8
#define QMI_SERVICE_ERR_INVALID_HANDLE                9
#define QMI_SERVICE_ERR_INVALID_PROFILE               10
#define QMI_SERVICE_ERR_INVALID_PINID                 11
#define QMI_SERVICE_ERR_INCORRECT_PIN                 12
#define QMI_SERVICE_ERR_NO_NETWORK_FOUND              13
#define QMI_SERVICE_ERR_CALL_FAILED                   14
#define QMI_SERVICE_ERR_OUT_OF_CALL                   15
#define QMI_SERVICE_ERR_NOT_PROVISIONED               16
#define QMI_SERVICE_ERR_MISSING_ARG                   17
#define QMI_SERVICE_ERR_ARG_TOO_LONG                  19
#define QMI_SERVICE_ERR_INVALID_TX_ID                 22
#define QMI_SERVICE_ERR_DEVICE_IN_USE                 23
#define QMI_SERVICE_ERR_OP_NETWORK_UNSUPPORTED        24
#define QMI_SERVICE_ERR_OP_DEVICE_UNSUPPORTED         25
#define QMI_SERVICE_ERR_NO_EFFECT                     26
#define QMI_SERVICE_ERR_NO_FREE_PROFILE               27
#define QMI_SERVICE_ERR_INVALID_PDP_TYPE              28
#define QMI_SERVICE_ERR_INVALID_TECH_PREF             29
#define QMI_SERVICE_ERR_INVALID_PROFILE_TYPE          30
#define QMI_SERVICE_ERR_INVALID_SERVICE_TYPE          31
#define QMI_SERVICE_ERR_INVALID_REGISTER_ACTION       32
#define QMI_SERVICE_ERR_INVALID_PS_ATTACH_ACTION      33
#define QMI_SERVICE_ERR_AUTHENTICATION_FAILED         34
#define QMI_SERVICE_ERR_PIN_BLOCKED                   35
#define QMI_SERVICE_ERR_PIN_PERM_BLOCKED              36
#define QMI_SERVICE_ERR_SIM_NOT_INITIALIZED           37
#define QMI_SERVICE_ERR_MAX_QOS_REQUESTS_IN_USE       38
#define QMI_SERVICE_ERR_INCORRECT_FLOW_FILTER         39
#define QMI_SERVICE_ERR_NETWORK_QOS_UNAWARE           40
#define QMI_SERVICE_ERR_INVALID_ID                    41
#define QMI_SERVICE_ERR_INVALID_QOS_ID                QMI_SERVICE_ERR_INVALID_ID
#define QMI_SERVICE_ERR_REQUESTED_NUM_UNSUPPORTED     42
#define QMI_SERVICE_ERR_INTERFACE_NOT_FOUND           43
#define QMI_SERVICE_ERR_FLOW_SUSPENDED                44
#define QMI_SERVICE_ERR_INVALID_DATA_FORMAT           45
#define QMI_SERVICE_ERR_GENERAL                       46
#define QMI_SERVICE_ERR_UNKNOWN                       47
#define QMI_SERVICE_ERR_INVALID_ARG                   48
#define QMI_SERVICE_ERR_INVALID_INDEX                 49
#define QMI_SERVICE_ERR_NO_ENTRY                      50
#define QMI_SERVICE_ERR_DEVICE_STORAGE_FULL           51
#define QMI_SERVICE_ERR_DEVICE_NOT_READY              52
#define QMI_SERVICE_ERR_NETWORK_NOT_READY             53
#define QMI_SERVICE_ERR_CAUSE_CODE                    54
#define QMI_SERVICE_ERR_MESSAGE_NOT_SENT              55
#define QMI_SERVICE_ERR_MESSAGE_DELIVERY_FAILURE      56
#define QMI_SERVICE_ERR_INVALID_MESSAGE_ID            57
#define QMI_SERVICE_ERR_ENCODING                      58
#define QMI_SERVICE_ERR_AUTHENTICATION_LOCK           59
#define QMI_SERVICE_ERR_INVALID_TRANSITION            60
#define QMI_SERVICE_ERR_NOT_A_MCAST_IFACE             61
#define QMI_SERVICE_ERR_MAX_MCAST_REQUESTS_IN_USE     62
#define QMI_SERVICE_ERR_INVALID_MCAST_HANDLE          63
#define QMI_SERVICE_ERR_INVALID_IP_FAMILY_PREF        64
#define QMI_SERVICE_ERR_SESSION_INACTIVE              65
#define QMI_SERVICE_ERR_SESSION_INVALID               66
#define QMI_SERVICE_ERR_SESSION_OWNERSHIP             67
#define QMI_SERVICE_ERR_INSUFFICIENT_RESOURCES        68
#define QMI_SERVICE_ERR_DISABLED                      69
#define QMI_SERVICE_ERR_INVALID_OPERATION             70
#define QMI_SERVICE_ERR_INVALID_QMI_CMD               71
#define QMI_SERVICE_ERR_TPDU_TYPE                     72
#define QMI_SERVICE_ERR_SMSC_ADDR                     73
#define QMI_SERVICE_ERR_INFO_UNAVAILABLE              74
#define QMI_SERVICE_ERR_SEGMENT_TOO_LONG              75
#define QMI_SERVICE_ERR_SEGMENT_ORDER                 76
#define QMI_SERVICE_ERR_BUNDLING_NOT_SUPPORTED        77
#define QMI_SERVICE_ERR_OP_PARTIAL_FAILURE            78
#define QMI_SERVICE_ERR_POLICY_MISMATCH               79
#define QMI_SERVICE_ERR_SIM_FILE_NOT_FOUND            80
#define QMI_SERVICE_ERR_EXTENDED_INTERNAL             81
#define QMI_SERVICE_ERR_ACCESS_DENIED                 82
#define QMI_SERVICE_ERR_HARDWARE_RESTRICTED           83
#define QMI_SERVICE_ERR_ACK_NOT_SENT                  84
#define QMI_SERVICE_ERR_INJECT_TIMEOUT                85
// Sups Errors
#define QMI_SERVICE_ERR_INCOMPATIBLE_STATE            90
#define QMI_SERVICE_ERR_FDN_RESTRICT                  91
#define QMI_SERVICE_ERR_SUPS_FAILURE_CAUSE            92
#define QMI_SERVICE_ERR_NO_RADIO                      93
#define QMI_SERVICE_ERR_NOT_SUPPORTED                 94
#define QMI_SERVICE_ERR_NO_SUBSCRIPTION               95
#define QMI_SERVICE_ERR_CARD_CALL_CONTROL_FAILED      96
#define QMI_SERVICE_ERR_NETWORK_ABORTED               97
#define QMI_SERVICE_ERR_MSG_BLOCKED                   98
// Error 99 is open
/* QMI PBM Errors */
#define QMI_SERVICE_ERR_INVALID_SESSION_TYPE          100
#define QMI_SERVICE_ERR_INVALID_PB_TYPE               101
#define QMI_SERVICE_ERR_NO_SIM                        102
#define QMI_SERVICE_ERR_PB_NOT_READY                  103
#define QMI_SERVICE_ERR_PIN_RESTRICTION               104
#define QMI_SERVICE_ERR_PIN2_RESTRICTION              105
#define QMI_SERVICE_ERR_PUK_RESTRICTION               106
#define QMI_SERVICE_ERR_PUK2_RESTRICTION              107
#define QMI_SERVICE_ERR_PB_ACCESS_RESTRICTED          108
#define QMI_SERVICE_ERR_PB_DELETE_IN_PROG             109
#define QMI_SERVICE_ERR_MAX                           110


/* Enum for services.  */
typedef enum
{
  QMI_FIRST_SERVICE                       = 0x01,
  QMI_WDS_SERVICE                         = QMI_FIRST_SERVICE,
  QMI_DMS_SERVICE                         = 0x02,
  QMI_NAS_SERVICE                         = 0x03,
  QMI_QOS_SERVICE                         = 0x04,
  QMI_WMS_SERVICE                         = 0x05,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_1       = 0x06,
  QMI_EAP_SERVICE                         = 0x07,
  QMI_ATCOP_SERVICE                       = 0x08,
  QMI_VOICE_SERVICE                       = 0x09,
  QMI_CAT_SERVICE                         = 0x0A,
  QMI_UIM_SERVICE                         = 0x0B,
  QMI_PBM_SERVICE                         = 0x0C,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_2       = 0x0D,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_3       = 0x0E,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_4       = 0x0F,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_5       = 0x10,
  QMI_SAR_SERVICE                         = 0x11,
  QMI_IMS_VIDEO_SERVICE                   = 0x12,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_6       = 0x13,
  QMI_CSD_SERVICE                         = 0x14,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_7       = 0x15,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_8       = 0x16,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_9       = 0x17,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_10      = 0x18,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_11      = 0x19,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_12      = 0x1A,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_13      = 0x1B,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_14      = 0x1C,
  QMI_CSVT_SERVICE                        = 0x1D,
  QMI_IMS_VT_SERVICE,
  QMI_IMS_PRESENCE_SERVICE                = 0x1F,
  QMI_RFPE_SERVICE                        = 0x29,
  QMI_DSD_SERVICE                         = 0x2A,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_15      = 0x2B,
  QMI_SERVICE_NOT_YET_IMPLEMENTED_16      = 0x2C,
  QMI_FDS_SERVICE                         = 0x2D,
  QMI_ATP_SERVICE                         = 0x2E,
  QMI_DFS_SERVICE                         = 0x30,
  QMI_FIRST_VS_SERVICE,
  QMI_RF_SAR_SERVICE                      = QMI_FIRST_VS_SERVICE,
  QMI_MAX_SERVICES
} qmi_service_id_type;


/* #defines for numbers of connections */
/* Max number of PDP connections */
#define QMI_MAX_PDP_CONNECTIONS (3)

/* Max number of connections including PDP and broadcast */
#define QMI_MAX_CONNECTIONS (QMI_MAX_CONN_IDS)


/* Definitions for set_data_format command */
typedef unsigned short qmi_link_layer_protocol_type;
#define QMI_DATA_FORMAT_LINK_PROTOCOL_UNSPECIFIED  0x0000
#define QMI_DATA_FORMAT_LINK_PROTOCOL_ETHERNET     0x0001
#define QMI_DATA_FORMAT_LINK_PROTOCOL_IP           0x0002
#define QMI_DATA_FORMAT_LINK_PROTOCOL_ALL          0x0003 /* Update to all valid
                                                          ** bits when adding protocols
                                                          */

#define QMI_MAX_PWR_INDICATIONS 10  /* Must be at least 3 */
#define QMI_MAX_SERVICE_VERSIONS (QMI_MAX_SERVICES)
#define QMI_MAX_ADDENDUM_LABEL 255

/* used to initialize qmi handle
 * before calling the qmi_init() API
 */
#define QMI_INVALID_CLIENT_HANDLE   -1

typedef enum
{
  QMI_PWR_EVENT_REPORT_OFF = 0,
  QMI_PWR_EVENT_REPORT_ON  = 1
} qmi_pwr_report_type;


typedef enum
{
  QMI_DATA_FORMAT_WITHOUT_QOS_HDR = 0,
  QMI_DATA_FORMAT_WITH_QOS_HDR = 1
} qmi_data_format_qos_hdr_state_type;


typedef enum
{
  QMI_IP_FAMILY_PREF_ANY = 0,
  QMI_IP_FAMILY_PREF_IPV4 = 4,
  QMI_IP_FAMILY_PREF_IPV6 = 6,
  QMI_IP_FAMILY_PREF_UNSPECIFIED = 8
} qmi_ip_family_pref_type;

/* System event types */
typedef enum
{
  QMI_SYS_EVENT_INVALID = -1,
  QMI_SYS_EVENT_PWR_REPORT,
  QMI_SYS_EVENT_SYNC_IND,
  QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND,
  QMI_SYS_EVENT_MODEM_IN_SERVICE_IND,
  QMI_SYS_EVENT_MODEM_NEW_SRVC_IND,
  QMI_SYS_EVENT_PORT_WRITE_FAIL_IND
} qmi_sys_event_type;

/* Data associated with system events (to be filled in as required) */
typedef union
{
  struct
  {
    int curr_pwr_state_hndl;
    int prev_pwr_state_hndl;
  } qmi_sys_event_pwr_report;

  struct
  {
    int conn_id;
  } qmi_sync_ind;

  struct
  {
    int conn_id;
    const char *dev_id;
  } qmi_modem_service_ind;

  struct
  {
    qmi_connection_id_type conn_id;
    int write_err_code;
  } qmi_sys_port_write_failed_ind;

} qmi_sys_event_info_type;

/* Event callback handler prototype */
typedef void (*qmi_sys_event_rx_hdlr)
(
  qmi_sys_event_type        event_id,
  const qmi_sys_event_info_type   *event_info,
  void                      *user_data
);

/* Service version tuple */
typedef struct {
    unsigned short  major_ver;  /*  Major version number */
    unsigned short  minor_ver;  /*  Minor version number */
}qmi_service_version_info;



/*===========================================================================
  FUNCTION  qmi_connection_init
===========================================================================*/
/*!
@brief
  This function is called to initialize the connection for a particular
  port.  Once the connection is brought up, the CTL service will also
  be initialized prior to this function returning.

@return
  0 if function is successful and connection is brought up,
   negative value if not.

@note
  - Side Effects
    Opens connection
*/
/*=========================================================================*/
EXTERN int
qmi_dev_connection_init
(
  const char              *dev_id,
  int                     *qmi_err_code
);

/* This is here temporarily to allow clients time to move to the new API
*/
#define qmi_connection_init(a,b) qmi_dev_connection_init(a,b)


/*===========================================================================
  FUNCTION  qmi_init
===========================================================================*/
/*!
@brief
  Function to initialize the QMI system.  This must be called by before
  any other QMI API's at startup time.  This function registers a system event
  callback and user data which will be call when/if a system event occurs.

@return
  Negative error code value if error.  Otherwise a positive value, opaque handle will
  be returned.  This handle should be passed to the qmi_release() function when
  client is exiting and no longer wants to use QMI.  Note that all individual
  service handles should be released prior to doing qmi_release().

@note
  - Side Effects
    Initializes QMI QMUX subsystem
*/
/*=========================================================================*/
EXTERN int
qmi_init
(
  qmi_sys_event_rx_hdlr   event_rx_hdlr,
  void                    *event_user_data
);


/*===========================================================================
  FUNCTION  qmi_release
===========================================================================*/
/*!
@brief
  Function to initialize release/cleanup QMI library prior to exit.  Handle
  passed in is the handle returned from the qmi_init() function call.  Note that
  client should release all individual service handles prior to calling this
  function.

@return
  0 if function is successful, negative value if not.

@note
  - Side Effects
    Cleans up client ID's and data structures IF this is the last client
    in the PD to release.
*/
/*=========================================================================*/

EXTERN int
qmi_release
(
  int init_handle
);


/*===========================================================================
  FUNCTION  qmi_set_port_data_format
===========================================================================*/
/*!
@brief
  This function is called to set the data format of a particular port
  to the user specified values.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
EXTERN int
qmi_set_port_data_format
(
  const char                            *dev_id,
  qmi_data_format_qos_hdr_state_type    qos_hdr_state,
  qmi_link_layer_protocol_type          *link_protocol,
  int                                   *qmi_err_code
);

/* Power related functions */
/*===========================================================================
  FUNCTION  qmi_reg_pwr_save_mode
===========================================================================*/
/*!
@brief
  This function is used to register/de-register for power state change
  events.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
EXTERN int
qmi_reg_pwr_save_mode
(
  qmi_pwr_report_type   report_state,
  int                   *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_config_pwr_save_settings
===========================================================================*/
/*!
@brief
  Configures the power state indication filter for each connection.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
EXTERN int
qmi_config_pwr_save_settings
(
  int                  pwr_state_hndl,
  qmi_service_id_type  service_id,
  int                  num_indication_ids,
  unsigned short       indication_ids[],
  int                  *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_set_pwr_state
===========================================================================*/
/*!
@brief
  Sets power state for each connection.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
    - Modem will not send filtered indications until later power state change.
*/
/*=========================================================================*/
EXTERN int
qmi_set_pwr_state
(
  unsigned long        pwr_state,
  int                  *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_get_pwr_state
===========================================================================*/
/*!
@brief
  Gets power state for specified connection.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
EXTERN int
qmi_get_pwr_state
(
  const char       *dev_id,
  unsigned long    *pwr_state,
  int              *qmi_err_code
);


/*=========================================================================
 FUNCTION qmi_service_get_version
=========================================================================*/
/*!
@brief
  Gets the major and minor version identifier for specified service.

@return
  QMI_NO_ERR if function is successful, error code otherwise. If
  return code is QMI_SERVICE_ERR, then the qmi_err_code will be valid
  and will indicate which QMI error occurred.

@note

  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
int
qmi_service_get_version
(
  const char                    *dev_id,
  qmi_service_id_type            service_id,
  qmi_service_version_info      *service_version_info,
  int                           *qmi_err_code
);



#ifdef __cplusplus
}
#endif


#endif /* QMI_H */
