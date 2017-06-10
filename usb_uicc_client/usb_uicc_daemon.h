#ifndef USB_UICC_DAEMON_H
#define USB_UICC_DAEMON_H
/*===========================================================================
  Copyright (c) 2013, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/30/13   xj      Initial version
03/27/14   xj      Remove usb uicc event definitions and power down mode
04/18/14   xj      Add usb bridge type
07/04/14   xj      Add response timeout interface to usb ccid driver
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include "comdef.h"

#include "usb_uicc_qmi.h"
#ifndef USB_UICC_UT
#include "ccid_bridge.h"
#endif

#ifndef USB_UICC_QMI_UT
#define LOG_TAG "USB_UICC"
#include <utils/Log.h>
#include "common_log.h"
#endif

/*===========================================================================

                           DEFINES

===========================================================================*/
#ifdef USB_UICC_QMI_UT
/* Logging macros */
#define LOGI(...) fprintf(stderr, "USB_UICC: INFO:  " __VA_ARGS__)
#define LOGE(...) fprintf(stderr, "USB_UICC: ERROR: " __VA_ARGS__)
#define LOGD(...) fprintf(stderr, "USB_UICC: " __VA_ARGS__)
#endif

/* Macros for critical sections */
#define USB_UICC_ENTER_CRITICAL_SECTION( mutex, log_str )        \
  {                                                              \
    pthread_mutex_lock( mutex );                                 \
  }

#define USB_UICC_LEAVE_CRITICAL_SECTION( mutex, log_str )        \
  {                                                              \
    pthread_mutex_unlock( mutex );                               \
  }

/* static memory buffer size */
#define STATIC_BUFFER_MAX_SIZE                    512

/* status field mask in usb_uicc_rsp_msg_header_type */
#define USB_UICC_ICC_STATUS_MASK                  0x03
#define USB_UICC_RFU_MASK                         0x3c
#define USB_UICC_COMMAND_STATUS_MASK              0xc0

#define USB_UICC_MAX_SLOTS                        3

/* request and response message header length */
#define USB_UICC_REQ_MSG_LEN                      10
#define USB_UICC_RSP_MSG_LEN                      10

#define USB_UICC_MAX_POLL                         50

#ifdef USB_UICC_QMI_UT
/* ioctl msg type, need to be determined later */
#define USB_CCID_GET_CLASS_DESC                   0
#define USB_CCID_GET_CLOCK_FREQUENCIES            1
#define USB_CCID_GET_DATA_RATES                   2
#define USB_CCID_ABORT                            3
#define USB_CCID_GET_EVENT                        4
#endif /* USB_UICC_QMI_UT */

/*===========================================================================

                           TYPES

===========================================================================*/
/* -----------------------------------------------------------------------------
   ENUM:      USB_UICC_CMD_ENUM_TYPE

   DESCRIPTION:
     Commands from remote uim.
-------------------------------------------------------------------------------*/
typedef enum
{
  USB_UICC_POWER_UP_CMD,
  USB_UICC_POWER_DOWN_CMD,
  USB_UICC_SEND_APDU_CMD,
  USB_UICC_RESET_CMD,
  USB_UICC_MAX_CMD
} usb_uicc_cmd_enum_type;

/* -----------------------------------------------------------------------------
   ENUM:      USB_UICC_STATE_ENUM_TYPE

   DESCRIPTION:
     Possible USB UICC card running States.
-------------------------------------------------------------------------------*/
typedef enum
{
  USB_UICC_NOT_INIT_ST,
  USB_UICC_ATTACH_ST,
  USB_UICC_POWER_OFF_ST,
  USB_UICC_GET_SLOT_STATUS_ST,
  USB_UICC_POWER_ON_ST,
  USB_UICC_INIT_DONE_ST,
  USB_UICC_APDU_ST,
  USB_UICC_POLL_ST,
  USB_UICC_CLOSE_ST,
  USB_UICC_DONE_ST,
  USB_UICC_MAX_ST
} usb_uicc_state_enum_type;

/* -----------------------------------------------------------------------------
   ENUM:      USB_UICC_RETURN_ENUM_TYPE

   DESCRIPTION:
     Return values of state machine.
-------------------------------------------------------------------------------*/
typedef enum
{
  USB_UICC_ERR_NONE,
  USB_UICC_ERR_FAIL,
  USB_UICC_ERR_WAIT,
  USB_UICC_ERR_POLL_MAX
} usb_uicc_return_enum_type;

/* -----------------------------------------------------------------------------
   ENUM:      UIM_USB_STATE_ENUM_TYPE

   DESCRIPTION:
     Possible USB states.
-------------------------------------------------------------------------------*/
typedef enum
{
  UIM_USB_STATE_UNKNOWN,
  UIM_USB_STATE_DISCONNECTED,
  UIM_USB_STATE_CONNECTED,
  UIM_USB_STATE_ERROR
} uim_usb_state_enum_type;

/* -----------------------------------------------------------------------------
   ENUM:      USB_UICC_CARD_STATE_ENUM_TYPE

   DESCRIPTION:
     Possible USB UICC card states.
-------------------------------------------------------------------------------*/
typedef enum
{
  USB_UICC_CARD_NOT_INIT,
  USB_UICC_CARD_PRESENT_ACTIVE,
  USB_UICC_CARD_PRESENT_INAVTIVE,
  USB_UICC_CARD_ERROR
} usb_uicc_card_state_enum_type;

/* -----------------------------------------------------------------------------
   ENUM:      USB_UICC_ERROR_ENUM_TYPE

   DESCRIPTION:
     Possible error types returned by USB UICC card.
-------------------------------------------------------------------------------*/
typedef enum
{
  USB_UICC_ERR_COMMAND_NOT_SUPPORT                = 0x00,
  /* index of not supported or incorrect message parameter */
  USB_UICC_ERR_INDEX_MESSAGE_ERROR_BEGIN          = 0x01,
  USB_UICC_ERR_INDEX_MESSAGE_ERROR_END            = 0x7f,
  /* user defined */
  USB_UICC_ERR_USER_DEFINED_BEGIN                 = 0x81,
  USB_UICC_ERR_USER_DEFINED_END                   = 0xc0,
  USB_UICC_ERR_CMD_SLOT_BUSY                      = 0xe0,
  USB_UICC_ERR_PIN_CANCELLED                      = 0xef,
  USB_UICC_ERR_PIN_TIMEOUT                        = 0xf0,
  USB_UICC_ERR_BUSY_WITH_AUTO_SEQUENCE            = 0xf2,
  USB_UICC_ERR_DEACTIVATED_PROTOCOL               = 0xf3,
  USB_UICC_ERR_PROCEDURE_BYTE_CONFLICT            = 0xf4,
  USB_UICC_ERR_ICC_CLASS_NOT_SUPPORTED            = 0xf5,
  USB_UICC_ERR_ICC_PROTOCOL_NOT_SUPPORTED         = 0xf6,
  USB_UICC_ERR_BAD_ATR_TCK                        = 0xf7,
  USB_UICC_ERR_BAD_ATR_TS                         = 0xf8,
  USB_UICC_ERR_HW_ERROR                           = 0xfb,
  USB_UICC_ERR_XFR_OVERRUN                        = 0xfc,
  USB_UICC_ERR_XFR_PARITY_ERROR                   = 0xfd,
  USB_UICC_ERR_ICC_MUTE                           = 0xfe,
  USB_UICC_ERR_CMD_ABORTED                        = 0xff
} usb_uicc_error_enum_type;

/* -----------------------------------------------------------------------------
   ENUM:      USB_UICC_CMD_STATUS_ENUM_TYPE

   DESCRIPTION:
     COMMAND STATUS
-------------------------------------------------------------------------------*/
typedef enum
{
  USB_UICC_CMD_SUCCESS                            = 0x00,
  USB_UICC_CMD_FAILED                             = 0x01,
  USB_UICC_CMD_TIME_EXTENSION                     = 0x02,
  USB_UICC_CMD_RFU                                = 0x03
} usb_uicc_cmd_status_enum_type;

/* -----------------------------------------------------------------------------
   ENUM:      USB_UICC_BRIDGE_ENUM_TYPE

   DESCRIPTION:
     BRIDGE TYPE
-------------------------------------------------------------------------------*/
typedef enum
{
  USB_UICC_INVALID_SOLN_TYPE                      = 0x00,
  USB_UICC_DIRECT_CONNECT_SOLN_TYPE               = 0x01,
  USB_UICC_SPI_BRIDGE_SOLN_TYPE                   = 0x02,
  USB_UICC_HSIC_HUB_SOLN_TYPE                     = 0x03
} usb_uicc_bridge_enum_type;

/* timer callback */
typedef void (* usb_uicc_timer_cb_type)(sigval_t sig_val);

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_TIMER_INFO_TYPE

   DESCRIPTION:
     Timer information
-------------------------------------------------------------------------------*/
typedef struct
{
  timer_t                                         timer_id;
  boolean                                         timer_started;
  usb_uicc_timer_cb_type                          timer_cb;
} usb_uicc_timer_info_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_CARD_INFO_TYPE

   DESCRIPTION:
     Details of card information
-------------------------------------------------------------------------------*/
typedef struct
{
  usb_uicc_card_state_enum_type                   card_state;
  usb_uicc_cmd_status_enum_type                   card_cmd_status;
  usb_uicc_error_enum_type                        card_error_code;
  uint8                                           atr[QMI_UIM_REMOTE_MAX_ATR_LEN_V01];
  uint32                                          atr_len;
} usb_uicc_card_info_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_CMD_TYPE

   DESCRIPTION:
     Details of a USB UICC command from remote uim
-------------------------------------------------------------------------------*/
typedef struct
{
  usb_uicc_cmd_enum_type                          command;
  uim_remote_card_error_type_enum_v01             cmd_err_code;
  uint32                                          apdu_id;         /* only used in apdu request */
  int                                             poll_counter;
  uint8                                         * data_ptr;        /* pointer to store apdu data from remote uim */
  uint32                                          data_len;
  uint8                                         * rsp_payload_ptr; /* the pointer of response payload data from usb driver */
  int                                             rsp_payload_len; /* the length of response payload data from usb driver */
  boolean                                         power_down_mode_valid; /* Used for power down req*/
  uim_remote_power_down_mode_enum_v01             power_down_mode; /* Used for power down mode*/
  boolean                                         rsp_timeout_valid;
  uint32                                          rsp_timeout;
  boolean                                         volt_class_valid;
  uim_remote_voltage_class_enum_v01               volt_class;
} usb_uicc_cmd_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_GLOBAL_DATA_TYPE

   DESCRIPTION:
     Details of USB UICC card
-------------------------------------------------------------------------------*/
typedef struct
{
  pthread_t                                       listen_evt_thread_id;
  pthread_t                                       open_thread_id;
#ifdef USB_UICC_QMI_UT
  pthread_t                                       ut_thread_id;
#endif /* USB_UICC_QMI_UT*/
  uim_remote_slot_type_enum_v01                   slot;                   /* slot where usb uicc card */
  uim_usb_state_enum_type                         usb_state;              /* usb state*/
  usb_uicc_timer_info_type                        enumeration_timer;      /* usb enumeration timer */
  usb_uicc_timer_info_type                        poll_timer;             /* poll timer */
  int                                             usb_uicc_fd;            /* iccid bridge file handle */
  usb_uicc_cmd_type                               usb_uicc_cmd;           /* cmd to be processed */
  uint8                                           usb_uicc_msg_seq;       /* message sequence of iccid */
  usb_uicc_state_enum_type                      * usb_uicc_state_ptr;     /* state pointer */
  usb_uicc_card_info_type                         card_info;              /* currently support single card */
  boolean                                         is_hardware_error;
#ifndef USB_UICC_QMI_UT
  qmi_client_type                                 qmi_uim_svc_client_ptr;
  int                                             qmi_msg_lib_handle;
#endif /* USB_UICC_QMI_UT */
  usb_uicc_bridge_enum_type                       bridge_type;            /* usb bridge type */
} usb_uicc_global_data_type;

/* -----------------------------------------------------------------------------
   ENUM:      USB_UICC_REQ_MSG_ENUM_TYPE

   DESCRIPTION:
     Command Pipe, Bulk-OUT Message type
-------------------------------------------------------------------------------*/
typedef enum
{
  USB_UICC_ICC_POWER_ON_REQ                       = 0x62,
  USB_UICC_ICC_POWER_OFF_REQ                      = 0x63,
  USB_UICC_GET_SLOT_STATUS_REQ                    = 0x65,
  USB_UICC_XFR_BLOCK_REQ                          = 0x6f,
  USB_UICC_GET_PARAMETERS_REQ                     = 0x6c,
  USB_UICC_RESET_PARAMETERS_REQ                   = 0x6d,
  USB_UICC_SET_PARAMETERS_REQ                     = 0x61,
  USB_UICC_ESCAPE_REQ                             = 0x6b,
  USB_UICC_ICC_CLOCK_REQ                          = 0x6e,
  USB_UICC_T0_APDU_REQ                            = 0x6a,
  USB_UICC_SECURE_REQ                             = 0x69,
  USB_UICC_MECHANICAL_REQ                         = 0x71,
  USB_UICC_ABORT_REQ                              = 0x72,
  USB_UICC_SET_DATA_RATE_AND_CLOCK_FREQUENCY      = 0x73,
#ifdef USB_UICC_UT
  USB_UICC_NONE_REQ                               = 0xff
#endif /* USB_UICC_UT */
} usb_uicc_req_msg_enum_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_REQ_MSG_HEADER_TYPE

   DESCRIPTION:
     Command Pipe, Bulk-OUT Message header type
-------------------------------------------------------------------------------*/
typedef struct
{
  usb_uicc_req_msg_enum_type                      req_msg_type;
  uint32                                          length;
  uint8                                           slot;
  uint8                                           seq;
} usb_uicc_req_msg_header_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_ICC_POWER_ON_REQ_DATA_TYPE

   DESCRIPTION:
     IccPowerOn data
-------------------------------------------------------------------------------*/
typedef struct
{
  /* iso 7816-12 spec, power_select shoud be set to 0x01;
     Smart-Card_CCID spec, this has four values
       (1) 0x00 automatic voltage selection
       (2) 0x01 5v
       (3) 0x02 3v
       (4) 0x03 1.8v
     here, we consider power_select as RFU
  */
  uint8                                           power_select;
  uint8                                           rfu[2];
} usb_uicc_icc_power_on_req_data_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_XFR_BLOCK_REQ_DATA_TYPE

   DESCRIPTION:
     XfrBlock request data
-------------------------------------------------------------------------------*/
typedef struct
{
  /* iso7816-12 spec, bwi is reserved, shall be set to 0x00;
     Smart-Card_CCID spec,  bwi used to extend the ccids block waiting timeout for current transter
  */
  uint8                                           bwi;
  uint16                                          level_parameter;
} usb_uicc_xfr_block_req_data_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_SET_PARAMETERS_REQ_DATA_TYPE

   DESCRIPTION:
     SetParameters request data
-------------------------------------------------------------------------------*/
typedef struct
{
  /* protocol num: 0x00 -- T0 protocol
                   0x01 -- T1 protocol
                   0x80 -- 2-wire protocol
                   0x81 -- 3-wire protocol
                   0x82 -- i2c protocol
  */
  uint8                                           protocol_num;
  uint8                                           rfu[2];
} usb_uicc_set_parameters_req_data_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_ICC_CLOCK_REQ_DATA_TYPE

   DESCRIPTION:
     IccClock request data
-------------------------------------------------------------------------------*/
typedef struct
{
  /* clock_command: 0x00 restarts clock
                    0x01 stops clock in the state shown in the bClockStop field
  */
  uint8                                           clock_command;
  uint8                                           rfu[2];
} usb_uicc_icc_clock_req_data_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_T0_APDU_REQ_DATA_TYPE

   DESCRIPTION:
     T0APDU request data
-------------------------------------------------------------------------------*/
typedef struct
{
  uint8                                           changes;
  uint8                                           class_get_response;
  uint8                                           class_envelop;
} usb_uicc_t0_apdu_req_data_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_SECURE_REQ_DATA_TYPE

   DESCRIPTION:
     Secure request data
-------------------------------------------------------------------------------*/
typedef struct
{
  uint8                                           bwi;
  uint16                                          level_parameter;
} usb_uicc_secure_req_data_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_MECHANICAL_REQ_DATA_TYPE

   DESCRIPTION:
     Mechanical request data
-------------------------------------------------------------------------------*/
typedef struct
{
  uint8                                           function;
  uint8                                           rfu[2];
} usb_uicc_mechanical_req_data_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_RFU_REQ_DATA_TYPE

   DESCRIPTION:
   Escape request data
   GetParameters request data
   ResetParameters request data
   abort request data
   SetDataRateAndClockFrequecny reqest data
   IccPowerOff request data
   GetSlotStatus request data
-------------------------------------------------------------------------------*/
typedef struct
{
  uint8                                           rfu[3];
} usb_uicc_rfu_req_data_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_REQ_MSG_TYPE

   DESCRIPTION:
     request message to usb uicc card
-------------------------------------------------------------------------------*/
typedef struct
{
  usb_uicc_req_msg_header_type                    req_msg_hdr;
  union
  {
    usb_uicc_icc_power_on_req_data_type           icc_power_on_req_data;
    usb_uicc_xfr_block_req_data_type              xfr_block_req_data;
    usb_uicc_set_parameters_req_data_type         set_parameters_req_data;
    usb_uicc_icc_clock_req_data_type              icc_clock_req_data;
    usb_uicc_t0_apdu_req_data_type                t0_apdu_req_data;
    usb_uicc_secure_req_data_type                 secure_req_data;
    usb_uicc_mechanical_req_data_type             mechanical_req_data;
    usb_uicc_rfu_req_data_type                    rfu_req_data;
  };
} usb_uicc_req_msg_type;

/* -----------------------------------------------------------------------------
   ENUM:      USB_UICC_RSP_MSG_ENUM_TYPE

   DESCRIPTION:
     Response Pipe, Bulk-IN Message type
-------------------------------------------------------------------------------*/
typedef enum
{
  USB_UICC_DATA_BLOCK_RSP                         = 0x80,
  USB_UICC_SLOT_STATUS_RSP                        = 0x81,
  USB_UICC_PARAMETERS_RSP                         = 0x82,
  USB_UICC_ESCAPE_RSP                             = 0x83,
  USB_UICC_DATA_RATE_AND_CLOCK_FREQUECNY_RSP      = 0x84,
} usb_uicc_rsp_msg_enum_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_RSP_MSG_HEADER_TYPE

   DESCRIPTION:
     Response Pipe, Bulk-IN Message header type
-------------------------------------------------------------------------------*/
typedef struct
{
  usb_uicc_rsp_msg_enum_type                      rsp_msg_type;
  uint32                                          length;
  uint8                                           slot;
  uint8                                           seq;
  uint8                                           status;
  uint8                                           err_code;
} usb_uicc_rsp_msg_header_type;

/* -----------------------------------------------------------------------------
   STRUCT:      USB_UICC_RSP_MSG_TYPE

   DESCRIPTION:
     RDR to PC message
-------------------------------------------------------------------------------*/
typedef struct
{
  usb_uicc_rsp_msg_header_type                    rsp_msg_hdr;
  union
  {
    uint8                                         chain_parameter; /* used in USB_UICC_DATA_BLOCK_RSP */
    uint8                                         clock_status;    /* used in USB_UICC_SLOT_STATUS_RSP */
    uint8                                         protocol_num;    /* used in USB_UICC_PARAMETERS_RSP */
    uint8                                         rfu;             /* used in USB_UICC_ESCAPE_RSP and USB_UICC_DATA_RATE_AND_CLOCK_FREQUECNY_RSP */
  };
} usb_uicc_rsp_msg_type;

#endif /* USB_UICC_DAEMON_H */
