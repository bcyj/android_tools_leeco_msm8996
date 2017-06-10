#ifndef CAT_SERVICE_02_H
#define CAT_SERVICE_02_H
/**
  @file card_application_toolkit_v02.h
  
  @brief This is the public header file which defines the cat service Data structures.

  This header file defines the types and structures that were defined in 
  cat. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were 
  defined in the IDL as messages contain mandatory elements, optional 
  elements, a combination of mandatory and optional elements (mandatory 
  always come before optionals in the structure), or nothing (null message)
   
  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to. 
   
  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:
   
  uint32_t test_opaque_len;
  uint8_t test_opaque[16];
   
  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set 
  by the decode routine and should be checked so that the correct number of 
  elements in the array will be accessed. 

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2009-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/cat/main/latest/api/card_application_toolkit_v02.h#24 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It requires encode/decode library version 5 or later
   It was generated on: Thu Jun 27 2013 (Spin 1)
   From IDL File: card_application_toolkit_v02.idl */

/** @defgroup cat_qmi_consts Constant values defined in the IDL */
/** @defgroup cat_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup cat_qmi_enums Enumerated types used in QMI messages */
/** @defgroup cat_qmi_messages Structures sent as QMI messages */
/** @defgroup cat_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup cat_qmi_accessor Accessor for QMI service object */
/** @defgroup cat_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup cat_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define CAT_V02_IDL_MAJOR_VERS 0x02
/** Revision Number of the IDL used to generate this file */
#define CAT_V02_IDL_MINOR_VERS 0x17
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define CAT_V02_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define CAT_V02_MAX_MESSAGE_ID 0x002E;
/** 
    @} 
  */


/** @addtogroup cat_qmi_consts 
    @{ 
  */
#define QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02 258
#define QMI_CAT_ENVELOPE_DATA_MAX_LENGTH_V02 258
#define QMI_CAT_TERMINAL_RESPONSE_MAX_LENGTH_V02 255
#define QMI_CAT_RAW_ENV_RSP_DATA_MAX_LENGTH_V02 255
#define QMI_CAT_DCS_ENCODED_STRING_MAX_LENGTH_V02 255
#define QMI_CAT_ICON_DATA_SIZE_MAX_V02 512
#define QMI_CAT_NUMBER_OF_ITEMS_MAX_V02 128
#define QMI_CAT_ITEM_TEXT_MAX_LENGTH_V02 255
#define QMI_CAT_ACTION_LIST_MAX_V02 255
#define QMI_CAT_NUMBER_OF_ICONS_MAX_V02 50
#define QMI_CAT_SMS_TPDU_MAX_LENGTH_V02 255
#define QMI_CAT_ADDRESS_MAX_LENGTH_V02 200
#define QMI_CAT_SUBADDRESS_MAX_LENGTH_V02 20
#define QMI_CAT_CAPABILITY_CONFIG_MAX_LENGTH_V02 255
#define QMI_CAT_DTMF_MAX_LENGTH_V02 255
#define QMI_CAT_URL_MAX_LENGTH_V02 255
#define QMI_CAT_BEARER_LIST_MAX_V02 258
#define QMI_CAT_NUMBER_OF_PROV_FILES_MAX_V02 128
#define QMI_CAT_FILE_PATH_MAX_LENGTH_V02 10
#define QMI_CAT_USSD_STRING_MAX_LENGTH_V02 255
#define QMI_CAT_NETWORK_ACCESS_NAME_MAX_LENGTH_V02 255
#define QMI_CAT_NETWORK_ADDRESS_MAX_LENGTH_V02 255
#define QMI_CAT_CHANNEL_DATA_MAX_LENGTH_V02 255
#define QMI_CAT_PDP_CONTEXT_ACT_MAX_LENGTH_V02 255
#define QMI_CAT_EPS_PDN_CONNECT_ACT_MAX_LENGTH_V02 255
#define QMI_CAT_SMS_PP_UICC_ACK_MAX_LENGTH_V02 128
#define QMI_CAT_TX_ID_MAX_LENGTH_V02 255
#define QMI_CAT_CAUSE_MAX_LENGTH_V02 30
#define QMI_CAT_SCWS_DATA_MAX_LENGTH_V02 1000
#define QMI_CAT_TR_ADDITIONAL_INFO_MAX_LENGTH_V02 10
#define QMI_CAT_TERMINAL_PROFILE_MAX_LENGTH_V02 80
#define QMI_CAT_EVT_REPORT_REQ_DISPLAY_TEXT_MASK_V02 0x00000001
#define QMI_CAT_EVT_REPORT_REQ_GET_INKEY_MASK_V02 0x00000002
#define QMI_CAT_EVT_REPORT_REQ_GET_INPUT_MASK_V02 0x00000004
#define QMI_CAT_EVT_REPORT_REQ_SETUP_MENU_MASK_V02 0x00000008
#define QMI_CAT_EVT_REPORT_REQ_SELECT_ITEM_MASK_V02 0x00000010
#define QMI_CAT_EVT_REPORT_REQ_SEND_SMS_MASK_V02 0x00000020
#define QMI_CAT_EVT_REPORT_REQ_SETUP_EVENT_USER_ACTIVITY_MASK_V02 0x00000040
#define QMI_CAT_EVT_REPORT_REQ_SETUP_EVENT_IDLE_SCREEN_NOTIF_MASK_V02 0x00000080
#define QMI_CAT_EVT_REPORT_REQ_SETUP_EVENT_LANGUAGE_SEL_NOTIF_MASK_V02 0x00000100
#define QMI_CAT_EVT_REPORT_REQ_SETUP_IDLE_MODE_TEXT_MASK_V02 0x00000200
#define QMI_CAT_EVT_REPORT_REQ_LANGUAGE_NOTIF_MASK_V02 0x00000400
#define QMI_CAT_EVT_REPORT_REQ_REFRESH_MASK_V02 0x00000800
#define QMI_CAT_EVT_REPORT_REQ_END_PROACTIVE_SESSION_MASK_V02 0x00001000
#define QMI_CAT_EVT_REPORT_REQ_PLAY_TONE_MASK_V02 0x00002000
#define QMI_CAT_EVT_REPORT_REQ_SETUP_CALL_MASK_V02 0x00004000
#define QMI_CAT_EVT_REPORT_REQ_SEND_DTMF_MASK_V02 0x00008000
#define QMI_CAT_EVT_REPORT_REQ_LAUNCH_BROWSER_MASK_V02 0x00010000
#define QMI_CAT_EVT_REPORT_REQ_SEND_SS_MASK_V02 0x00020000
#define QMI_CAT_EVT_REPORT_REQ_SEND_USSD_MASK_V02 0x00040000
#define QMI_CAT_EVT_REPORT_REQ_PROVIDE_LOCAL_INFO_LANGUAGE_MASK_V02 0x00080000
#define QMI_CAT_EVT_REPORT_REQ_BEARER_INDEPENDENT_PROTOCOL_MASK_V02 0x00100000
#define QMI_CAT_EVT_REPORT_REQ_SETUP_EVENT_BROWSER_TERMINATION_MASK_V02 0x00200000
#define QMI_CAT_EVT_REPORT_REQ_PROVIDE_LOCAL_INFO_TIME_MASK_V02 0x00400000
#define QMI_CAT_EVT_REPORT_REQ_ACTIVATE_MASK_V02 0x01000000
#define QMI_CAT_EVT_REPORT_REQ_SETUP_EVENT_HCI_CONN_MASK_V02 0x02000000
#define QMI_CAT_DEC_EVT_REPORT_REQ_DISPLAY_TEXT_MASK_V02 0x00000001
#define QMI_CAT_DEC_EVT_REPORT_REQ_GET_INKEY_MASK_V02 0x00000002
#define QMI_CAT_DEC_EVT_REPORT_REQ_GET_INPUT_MASK_V02 0x00000004
#define QMI_CAT_DEC_EVT_REPORT_REQ_SETUP_MENU_MASK_V02 0x00000008
#define QMI_CAT_DEC_EVT_REPORT_REQ_SELECT_ITEM_MASK_V02 0x00000010
#define QMI_CAT_DEC_EVT_REPORT_REQ_SEND_SMS_MASK_V02 0x00000020
#define QMI_CAT_DEC_EVT_REPORT_REQ_SETUP_EVENT_USER_ACTIVITY_MASK_V02 0x00000040
#define QMI_CAT_DEC_EVT_REPORT_REQ_SETUP_EVENT_IDLE_SCREEN_NOTIF_MASK_V02 0x00000080
#define QMI_CAT_DEC_EVT_REPORT_REQ_SETUP_EVENT_LANGUAGE_SEL_NOTIF_MASK_V02 0x00000100
#define QMI_CAT_DEC_EVT_REPORT_REQ_SETUP_IDLE_MODE_TEXT_MASK_V02 0x00000200
#define QMI_CAT_DEC_EVT_REPORT_REQ_LANGUAGE_NOTIF_MASK_V02 0x00000400
#define QMI_CAT_DEC_EVT_REPORT_REQ_END_PROACTIVE_SESSION_MASK_V02 0x00001000
#define QMI_CAT_DEC_EVT_REPORT_REQ_PLAY_TONE_MASK_V02 0x00002000
#define QMI_CAT_DEC_EVT_REPORT_REQ_SETUP_CALL_MASK_V02 0x00004000
#define QMI_CAT_DEC_EVT_REPORT_REQ_SEND_DTMF_MASK_V02 0x00008000
#define QMI_CAT_DEC_EVT_REPORT_REQ_LAUNCH_BROWSER_MASK_V02 0x00010000
#define QMI_CAT_DEC_EVT_REPORT_REQ_SEND_SS_MASK_V02 0x00020000
#define QMI_CAT_DEC_EVT_REPORT_REQ_SEND_USSD_MASK_V02 0x00040000
#define QMI_CAT_DEC_EVT_REPORT_REQ_PROVIDE_LOCAL_INFO_LANGUAGE_MASK_V02 0x00080000
#define QMI_CAT_DEC_EVT_REPORT_REQ_BEARER_INDEPENDENT_PROTOCOL_MASK_V02 0x00100000
#define QMI_CAT_DEC_EVT_REPORT_REQ_SCWS_EVENT_MASK_V02 0x00800000
#define QMI_CAT_DEC_EVT_REPORT_REQ_ACTIVATE_MASK_V02 0x01000000
#define QMI_CAT_DEC_EVT_REPORT_REQ_SETUP_EVENT_HCI_CONN_MASK_V02 0x02000000
#define QMI_CAT_DEC_EVT_REPORT_REQ_BEARER_INDEPENDENT_PROTOCOL_STATUS_MASK_V02 0x04000000
/**
    @}
  */

/*
 * cat_reset_req_msg is empty
 * typedef struct {
 * }cat_reset_req_msg_v02;
 */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Resets the QMI_CAT service state variables of the requesting
             control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */
}cat_reset_resp_msg_v02;  /* Message */
/**
    @}
  */

typedef uint32_t cat_set_event_report_full_func_mask_v02;
#define CAT_FULL_FUNC_EVT_REPORT_REQ_SEND_SMS_MASK_V02 ((cat_set_event_report_full_func_mask_v02)0x00000001) 
#define CAT_FULL_FUNC_EVT_REPORT_REQ_SETUP_CALL_MASK_V02 ((cat_set_event_report_full_func_mask_v02)0x00000002) 
#define CAT_FULL_FUNC_EVT_REPORT_REQ_SEND_DTMF_MASK_V02 ((cat_set_event_report_full_func_mask_v02)0x00000004) 
#define CAT_FULL_FUNC_EVT_REPORT_REQ_SEND_SS_MASK_V02 ((cat_set_event_report_full_func_mask_v02)0x00000008) 
#define CAT_FULL_FUNC_EVT_REPORT_REQ_SEND_USSD_MASK_V02 ((cat_set_event_report_full_func_mask_v02)0x00000010) 
typedef uint8_t cat_set_event_report_slot_mask_v02;
#define CAT_SET_EVENT_REPORT_SLOT_1_V02 ((cat_set_event_report_slot_mask_v02)0x01) 
#define CAT_SET_EVENT_REPORT_SLOT_2_V02 ((cat_set_event_report_slot_mask_v02)0x02) 
#define CAT_SET_EVENT_REPORT_SLOT_3_V02 ((cat_set_event_report_slot_mask_v02)0x04) 
#define CAT_SET_EVENT_REPORT_SLOT_4_V02 ((cat_set_event_report_slot_mask_v02)0x08) 
#define CAT_SET_EVENT_REPORT_SLOT_5_V02 ((cat_set_event_report_slot_mask_v02)0x10) 
/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Sets the QMI_CAT event reporting conditions for the requesting
             control point.
             \label{idl:SetEventReport} */
typedef struct {

  /* Optional */
  /*  Event Reporting Request */
  uint8_t pc_evt_report_req_mask_valid;  /**< Must be set to true if pc_evt_report_req_mask is being passed */
  uint32_t pc_evt_report_req_mask;
  /**<   Event report request bitmask: \n
       - Bit 0  -- Display Text \n
       - Bit 1  -- Get Inkey \n
       - Bit 2  -- Get Input \n
       - Bit 3  -- Setup Menu \n
       - Bit 4  -- Select Item \n
       - Bit 5  -- Send SMS \n
       - Bit 6  -- Setup Event -- User Activity \n
       - Bit 7  -- Setup Event -- Idle Screen Notify \n
       - Bit 8  -- Setup Event -- Language Select Notify \n
       - Bit 9  -- Setup Idle Mode Text \n
       - Bit 10 -- Language Notification \n
       - Bit 11 -- Refresh/Refresh Alpha (Refresh when QMI_CAT is configured
                   in Gobi mode, Refresh Alpha in other cases) \n
       - Bit 12 -- End Proactive Session \n
       - Bit 13 -- Play Tone \n
       - Bit 14 -- Setup Call \n
       - Bit 15 -- Send DTMF \n
       - Bit 16 -- Launch Browser \n
       - Bit 17 -- Send SS \n
       - Bit 18 -- Send USSD \n
       - Bit 19 -- Provide Local Information -- Language \n
       - Bit 20 -- Bearer Independent Protocol \n
       - Bit 21 -- Setup Event -- Browser Termination \n
       - Bit 22 -- Provide Local Information -- Time \n
       - Bit 23 -- Clients must set this bit to zero \n
       - Bit 24 -- Activate \n
       - Bit 25 -- Setup Event -- HCI connectivity \n
       - Bit 26 -- Clients must set this bit to zero \n
       Each bit set indicates a request made to QMI_CAT to register the
       corresponding proactive command to the control point. All unlisted
       bits are reserved for future use and must be set to zero.
  */

  /* Optional */
  /*  Decoded Event Reporting Request */
  uint8_t pc_dec_evt_report_req_mask_valid;  /**< Must be set to true if pc_dec_evt_report_req_mask is being passed */
  uint32_t pc_dec_evt_report_req_mask;
  /**<   Decoded event report request bitmask: \n
       - Bit 0  -- Display Text \n
       - Bit 1  -- Get Inkey \n
       - Bit 2  -- Get Input \n
       - Bit 3  -- Setup Menu \n
       - Bit 4  -- Select Item \n
       - Bit 5  -- Send SMS \n
       - Bit 6  -- Setup Event -- User Activity \n
       - Bit 7  -- Setup Event -- Idle Screen Notify \n
       - Bit 8  -- Setup Event -- Language Select Notify \n
       - Bit 9  -- Setup Idle Mode Text \n
       - Bit 10 -- Language Notification \n
       - Bit 11 -- Refresh Alpha (not supported when QMI CAT is configured 
                   in Gobi mode) \n
       - Bit 12 -- End Proactive Session \n
       - Bit 13 -- Play Tone \n
       - Bit 14 -- Setup Call \n
       - Bit 15 -- Send DTMF \n
       - Bit 16 -- Launch Browser \n
       - Bit 17 -- Send SS \n
       - Bit 18 -- Send USSD \n
       - Bit 19 -- Provide Local Information -- Language \n
       - Bit 20 -- Bearer Independent Protocol \n
       - Bit 21 -- Setup Event -- Browser Termination \n
       - Bit 22 -- Clients must set this bit to zero \n
       - Bit 23 -- Smart Card Web Server \n
       - Bit 24 -- Activate \n
       - Bit 25 -- Setup Event -- HCI connectivity \n
       - Bit 26 -- Bearer Independent Protocol Status \n
       Each bit set indicates a request made to QMI_CAT to register the
       corresponding proactive command to the control point. All unlisted
       bits are reserved for future use and must be set to zero.
  */

  /* Optional */
  /*  Slot */
  uint8_t slot_mask_valid;  /**< Must be set to true if slot_mask is being passed */
  cat_set_event_report_slot_mask_v02 slot_mask;
  /**<   Slot used for the registration: \n
       - Bit 0  -- Slot 1 \n
       - Bit 1  -- Slot 2 \n
       - Bit 2  -- Slot 3 \n
       - Bit 3  -- Slot 4 \n
       - Bit 4  -- Slot 5 \n
       All other bits are reserved for future use. If the TLV is missing,
       the client is implicitly registering for all available slots.
  */

  /* Optional */
  /*  Full Function Event Reporting Request */
  uint8_t pc_full_func_evt_report_req_mask_valid;  /**< Must be set to true if pc_full_func_evt_report_req_mask is being passed */
  cat_set_event_report_full_func_mask_v02 pc_full_func_evt_report_req_mask;
  /**<   Full function event report request bitmask: \n
       - Bit 0  -- Send SMS \n
       - Bit 1  -- Setup Call \n
       - Bit 2  -- Send DTMF \n
       - Bit 3  -- Send SS \n
       - Bit 4  -- Send USSD \n
       Each bit set indicates a request made to QMI_CAT to enable/disable
       full function capability of the control point for the corresponding
       proactive command. All unlisted bits are reserved for future use and must
       be set to zero.

       The control point must register the corresponding proactive command with raw
       or decoded event report bitmask for receiving events.
  */
}cat_set_event_report_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Sets the QMI_CAT event reporting conditions for the requesting
             control point.
             \label{idl:SetEventReport} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */

  /* Optional */
  /*  Proactive Command Event Report Registration Status */
  uint8_t pc_evt_report_req_err_mask_valid;  /**< Must be set to true if pc_evt_report_req_err_mask is being passed */
  uint32_t pc_evt_report_req_err_mask;
  /**<   Proactive command event report registration error bitmask: \n
       - Bit 0  -- Display Text \n
       - Bit 1  -- Get Inkey \n
       - Bit 2  -- Get Input \n
       - Bit 3  -- Setup Menu \n
       - Bit 4  -- Select Item \n
       - Bit 5  -- Send SMS \n
       - Bit 6  -- Setup Event -- User Activity \n
       - Bit 7  -- Setup Event -- Idle Screen Notify \n
       - Bit 8  -- Setup Event -- Language Select Notify \n
       - Bit 9  -- Setup Idle Mode Text \n
       - Bit 10 -- Language Notification \n
       - Bit 11 -- Refresh/Refresh Alpha (Refresh when QMI_CAT is configured
                   in Gobi mode, Refresh Alpha in other cases) \n
       - Bit 12 -- End Proactive Session \n
       - Bit 13 -- Play Tone \n
       - Bit 14 -- Setup Call \n
       - Bit 15 -- Send DTMF \n
       - Bit 16 -- Launch Browser \n
       - Bit 17 -- Send SS \n
       - Bit 18 -- Send USSD \n
       - Bit 19 -- Provide Local Information -- Language \n
       - Bit 20 -- Bearer Independent Protocol \n
       - Bit 21 -- Setup Event -- Browser Termination \n
       - Bit 22 -- Provide Local Information -- Time \n
       - Bit 23 -- Clients are to ignore this bit \n
       - Bit 24 -- Activate \n
       - Bit 25 -- Setup Event -- HCI connectivity \n
       - Bit 26 -- Clients are to ignore this bit \n

       A set bit indicates that the corresponding proactive command has already
       been registered by another control point. If a bit that was not
       set by the control point is included, the control point ignores the bit.
  */

  /* Optional */
  /*  Proactive Command Decoded Event Report Registration Status */
  uint8_t pc_dec_evt_report_req_err_mask_valid;  /**< Must be set to true if pc_dec_evt_report_req_err_mask is being passed */
  uint32_t pc_dec_evt_report_req_err_mask;
  /**<   Proactive command decoded event report registration error bitmask: \n
       - Bit 0  -- Display Text \n
       - Bit 1  -- Get Inkey \n
       - Bit 2  -- Get Input \n
       - Bit 3  -- Setup Menu \n
       - Bit 4  -- Select Item \n
       - Bit 5  -- Send SMS \n
       - Bit 6  -- Setup Event -- User Activity \n
       - Bit 7  -- Setup Event -- Idle Screen Notify \n
       - Bit 8  -- Setup Event -- Language Select Notify \n
       - Bit 9  -- Setup Idle Mode Text \n
       - Bit 10 -- Language Notification \n
       - Bit 11 -- Refresh Alpha (not supported when QMI CAT is configured 
                   in Gobi mode) \n
       - Bit 12 -- End Proactive Session \n
       - Bit 13 -- Play Tone \n
       - Bit 14 -- Setup Call \n
       - Bit 15 -- Send DTMF \n
       - Bit 16 -- Launch Browser \n
       - Bit 17 -- Send SS \n
       - Bit 18 -- Send USSD \n
       - Bit 19 -- Provide Local Information -- Language \n
       - Bit 20 -- Bearer Independent Protocol \n
       - Bit 21 -- Setup Event -- Browser Termination \n
       - Bit 22 -- Clients are to ignore this bit \n
       - Bit 23 -- Smart Card Web Server \n
       - Bit 24 -- Activate \n
       - Bit 25 -- Setup Event -- HCI connectivity \n
       - Bit 26 -- Bearer Independent Protocol Status \n
       A set bit indicates that the corresponding proactive command has already
       been registered by another control point. If a bit which was not
       set by control point is included, control point should ignore the bit.
  */

  /* Optional */
  /*  Full Function Event Report Registration Status */
  uint8_t pc_full_func_evt_report_err_mask_valid;  /**< Must be set to true if pc_full_func_evt_report_err_mask is being passed */
  cat_set_event_report_full_func_mask_v02 pc_full_func_evt_report_err_mask;
  /**<   Full function event report request bitmask: \n
       - Bit 0  -- Send SMS \n
       - Bit 1  -- Setup Call \n
       - Bit 2  -- Send DTMF \n
       - Bit 3  -- Send SS \n
       - Bit 4  -- Send USSD \n
       A set bit indicates that QMI_CAT failed to enable/disable full function
       capability handling for the corresponding proactive command. If a bit that
       was not set by control point is included, control points are to ignore the
       bit.
  */
}cat_set_event_report_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_display_text_len;  /**< Must be set to # of elements in pc_display_text */
  uint8_t pc_display_text[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Display Text proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.1.*/
}cat_display_text_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_get_inkey_len;  /**< Must be set to # of elements in pc_get_inkey */
  uint8_t pc_get_inkey[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Get Inkey proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.2.*/
}cat_get_inkey_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_get_input_len;  /**< Must be set to # of elements in pc_get_input */
  uint8_t pc_get_input[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Get Input proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.3.*/
}cat_get_input_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_setup_menu_len;  /**< Must be set to # of elements in pc_setup_menu */
  uint8_t pc_setup_menu[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Setup Menu proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.7.*/
}cat_setup_menu_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_select_item_len;  /**< Must be set to # of elements in pc_select_item */
  uint8_t pc_select_item[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Select Item proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.8.*/
}cat_select_item_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t pc_cmd_type;
  /**<   Proactive command type that includes the alpha identifier: \n
       - 0x01 -- Sends an SMS proactive command \n
       All other values are reserved.
  */

  uint32_t alpha_identifier_len;  /**< Must be set to # of elements in alpha_identifier */
  uint8_t alpha_identifier[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Alpha identifier, as in \hyperref[S1]{[S1]}, Section 8.2.*/
}cat_alpha_id_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t pc_setup_evt_list;
  /**<   Setup event list bitmask: \n
       - Bit 0 -- User Activity Notify \n
       - Bit 1 -- Idle Screen Available \n
       - Bit 2 -- Language Selection Notify \n
       Each set bit indicates the availability of the corresponding event in
       the Setup Event list proactive command. All unlisted bits are reserved
       for future use and are ignored.
  */
}cat_setup_evt_list_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_setup_idle_mode_text_len;  /**< Must be set to # of elements in pc_setup_idle_mode_text */
  uint8_t pc_setup_idle_mode_text[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Setup Idle mode text proactive command, encoded as in
       \hyperref[S1]{[S1]}, \n Section 6.6.22.
  */
}cat_setup_idle_mode_text_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_lang_notification_len;  /**< Must be set to # of elements in pc_lang_notification */
  uint8_t pc_lang_notification[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Language Notification proactive command, encoded as in
       \hyperref[S1]{[S1]}, \n Section 6.6.25.
  */
}cat_lang_notification_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_REFRESH_STAGE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_REFRESH_STAGE_START_V02 = 0x01, 
  CAT_REFRESH_STAGE_SUCCESS_V02 = 0x02, 
  CAT_REFRESH_STAGE_FAILED_V02 = 0x03, 
  CAT_REFRESH_STAGE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_refresh_stage_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t refresh_mode;
  /**<   As indicated in \hyperref[S1]{[S1]}, Section 8.6 (Command Qualifier for Refresh).*/

  cat_refresh_stage_enum_v02 refresh_stage;
  /**<   Stage of the refresh procedure: \n
       - 0x01 -- Refresh start \n
       - 0x02 -- Refresh success \n
       - 0x03 -- Refresh failed
  */
}cat_refresh_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_PROACTIVE_SESSION_END_TYPE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_PROACTIVE_SESSION_END_TYPE_CARD_V02 = 0x01, 
  CAT_PROACTIVE_SESSION_END_TYPE_INTERNAL_V02 = 0x02, 
  CAT_PROACTIVE_SESSION_END_TYPE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_proactive_session_end_type_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_proactive_session_end_type_enum_v02 proactive_session_end_type;
  /**<   Proactive session end type: \n
       - 0x01 -- End proactive session command received from the card \n
       - 0x02 -- End proactive session internal to the ME
  */
}cat_proactive_session_end_type_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_COMMAND_ID_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_COMMAND_ID_DISPLAY_TEXT_V02 = 0x01, 
  CAT_COMMAND_ID_GET_INKEY_V02 = 0x02, 
  CAT_COMMAND_ID_GET_INPUT_V02 = 0x03, 
  CAT_COMMAND_ID_LAUNCH_BROWSER_V02 = 0x04, 
  CAT_COMMAND_ID_PLAY_TONE_V02 = 0x05, 
  CAT_COMMAND_ID_SELECT_ITEM_V02 = 0x06, 
  CAT_COMMAND_ID_SEND_SMS_V02 = 0x07, 
  CAT_COMMAND_ID_SEND_SS_V02 = 0x08, 
  CAT_COMMAND_ID_SEND_USSD_V02 = 0x09, 
  CAT_COMMAND_ID_SETUP_CALL_USER_CONFIRMATION_V02 = 0x0A, 
  CAT_COMMAND_ID_SETUP_CALL_ALPHA_DISPLAY_V02 = 0x0B, 
  CAT_COMMAND_ID_SETUP_MENU_V02 = 0x0C, 
  CAT_COMMAND_ID_SETUP_IDLE_TEXT_V02 = 0x0D, 
  CAT_COMMAND_ID_PROVIDE_LOCAL_LANG_INFO_V02 = 0x0E, 
  CAT_COMMAND_ID_SEND_DTMF_V02 = 0x0F, 
  CAT_COMMAND_ID_LANG_NOTIFICATION_V02 = 0x10, 
  CAT_COMMAND_ID_SETUP_EVENT_USER_ACTIVITY_V02 = 0x11, 
  CAT_COMMAND_ID_SETUP_EVENT_IDLE_SCREEN_NOTIFY_V02 = 0x12, 
  CAT_COMMAND_ID_SETUP_EVENT_LANGUAGE_SEL_NOTIFY_V02 = 0x13, 
  CAT_COMMAND_ID_OPEN_CHANNEL_V02 = 0x14, 
  CAT_COMMAND_ID_CLOSE_CHANNEL_V02 = 0x15, 
  CAT_COMMAND_ID_RECEIVE_DATA_V02 = 0x16, 
  CAT_COMMAND_ID_SEND_DATA_V02 = 0x17, 
  CAT_COMMAND_ID_ACTIVATE_V02 = 0x18, 
  CAT_COMMAND_ID_SETUP_EVENT_HCI_CONNECTIVITY_V02 = 0x19, 
  CAT_COMMAND_ID_REFRESH_ALPHA_V02 = 0x1A, 
  CAT_COMMAND_ID_SETUP_EVENT_BROWSER_TERMINATION_V02 = 0x20, 
  CAT_COMMAND_ID_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_command_id_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_command_id_enum_v02 command_id;
  /**<   ID of the proactive command: \n
       - 0x01 -- Display Text \n
       - 0x02 -- Get Inkey \n
       - 0x03 -- Get Input \n
       - 0x04 -- Launch Browser \n
       - 0x05 -- Play Tone \n
       - 0x06 -- Select Item \n
       - 0x07 -- Send SMS \n
       - 0x08 -- Send SS \n
       - 0x09 -- Send USSD \n
       - 0x0A -- Setup Call -- User Confirmation \n
       - 0x0B -- Setup Call -- Alpha Display \n
       - 0x0C -- Setup Menu \n
       - 0x0D -- Setup Idle Text \n
       - 0x0E -- Provide Local Information -- Language \n
       - 0x0F -- Send DTMF \n
       - 0x10 -- Language Notification \n
       - 0x11 -- Setup Event -- User Activity \n
       - 0x12 -- Setup Event -- Idle Screen Notify \n
       - 0x13 -- Setup Event -- Language Selection Notify \n
       - 0x14 -- Open Channel \n
       - 0x15 -- Close Channel \n
       - 0x16 -- Receive Data \n
       - 0x17 -- Send Data \n
       - 0x18 -- Activate \n
       - 0x19 -- Setup Event -- HCI Connectivity \n
       - 0x1A -- Refresh Alpha \n
       - 0X20 -- Setup Event -- Browser Termination \n
       All other values are reserved.
  */

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID (used internally by the QMI_CAT service).*/

  uint8_t command_number;
  /**<   Command number sent to the client in the proactive command for
       tracking purposes to match with the command number in the terminal response.
  */
}cat_decoded_header_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_DCS_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_DCS_7BIT_GSM_V02 = 0x00, 
  CAT_DCS_8BIT_GSM_V02 = 0x01, 
  CAT_DCS_8BIT_UCS2_V02 = 0x02, 
  CAT_DCS_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_dcs_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_dcs_enum_v02 dcs;
  /**<   Data coding scheme: \n
       - 0x00 -- 7-bit GSM \n
       - 0x01 -- 8-bit GSM \n
       - 0x02 -- UCS2
  */

  uint32_t text_len;  /**< Must be set to # of elements in text */
  uint8_t text[QMI_CAT_DCS_ENCODED_STRING_MAX_LENGTH_V02];
  /**<   Text string data in the specified data coding scheme.*/
}cat_dcs_encoded_text_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_HIGH_PRIORITY_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_HIGH_PRIORITY_DONT_CLEAR_SCREEN_V02 = 0x00, 
  CAT_HIGH_PRIORITY_CLEAR_SCREEN_V02 = 0x01, 
  CAT_HIGH_PRIORITY_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_high_priority_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_high_priority_enum_v02 high_priority;
  /**<   High priority value: \n
       - 0x00 -- Do not clear the screen \n
       - 0x01 -- Clear anything that is on the screen
  */
}cat_high_priority_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_USER_CONTROL_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_USER_CONTROL_DONT_ALLOW_SCREEN_CLEARING_V02 = 0x00, 
  CAT_USER_CONTROL_ALLOW_SCREEN_CLEARING_V02 = 0x01, 
  CAT_USER_CONTROL_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_user_control_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_user_control_enum_v02 user_control;
  /**<   User control: \n
       - 0x00 -- Do not allow the user to clear the screen \n
       - 0x01 -- Allow the user to clear the screen
  */
}cat_user_control_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_QUALIFIER_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_ICON_QUALIFIER_SELF_EXPLANATORY_V02 = 0x00, 
  CAT_ICON_QUALIFIER_NOT_SELF_EXPLANATORY_V02 = 0x01, 
  CAT_QUALIFIER_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_qualifier_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_ICS_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_ICON_ICS_UNKNOWN_V02 = 0x00, 
  CAT_ICON_ICS_BASIC_V02 = 0x01, 
  CAT_ICON_ICS_COLOR_V02 = 0x02, 
  CAT_ICS_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_ics_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_qualifier_enum_v02 qualifier;
  /**<   Icon qualifier: \n
       - 0x00 -- Icon is self-explanatory; it replaces the item text \n
       - 0x01 -- Icon is not self-explanatory; it displays along with the text
  */

  uint8_t height;
  /**<   Icon height (from the EF-IMG file). Represents the number of raster
       image points.
  */

  uint8_t width;
  /**<   Icon width (from the EF-IMG file). Represents the number of raster
       image points.
  */

  cat_ics_enum_v02 ics;
  /**<   Image coding scheme: \n
       - 0x00 -- Unknown \n
       - 0x01 -- Basic \n
       - 0x02 -- Color
  */

  uint8_t rec_num;
  /**<   Record number in the EF-IMG file.*/

  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[QMI_CAT_ICON_DATA_SIZE_MAX_V02];
  /**<   Image instance data in binary format.*/
}cat_icon_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_TIME_UNITS_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_TIME_UNITS_MINUTES_V02 = 0x00, 
  CAT_TIME_UNITS_SECONDS_V02 = 0x01, 
  CAT_TIME_UNITS_TENTHS_OF_SECONDS_V02 = 0x02, 
  CAT_TIME_UNITS_DURATION_NOT_PRESENT_V02 = -1, 
  CAT_TIME_UNITS_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_time_units_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_time_units_enum_v02 units;
  /**<   Time units: \n
      - 0x00 -- Minutes \n
      - 0x01 -- Seconds \n
      - 0x02 -- Tenths of seconds
  */

  uint8_t interval;
  /**<   Time interval; this number must be greater than zero
       (see \hyperref[S1]{[S1]}, Section 8.8).
  */
}cat_duration_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_RESPONSE_FORMAT_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_RESPONSE_FORMAT_SMS_DEFAULT_ALPHA_V02 = 0x00, 
  CAT_RESPONSE_FORMAT_YES_OR_NO_V02 = 0x01, 
  CAT_RESPONSE_FORMAT_NUMERICAL_ONLY_V02 = 0x02, 
  CAT_RESPONSE_FORMAT_UCS2_V02 = 0x03, 
  CAT_RESPONSE_FORMAT_IMMEDIATE_DIGIT_RESP_V02 = 0x04, 
  CAT_RESPONSE_FORMAT_YES_OR_NO_AND_IMMEDIATE_DIGIT_RESP_V02 = 0x05, 
  CAT_RESPONSE_FORMAT_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_response_format_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_response_format_enum_v02 response_format;
  /**<   Response format: \n
       - 0x00 -- SMS default alphabet \n
       - 0x01 -- Yes/No \n
       - 0x02 -- Numerical only \n
       - 0x03 -- UCS2 \n
       - 0x04 -- Immediate digit response\n
       - 0x05 -- Yes/No and immediate digit response
  */
}cat_response_format_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t help_available;
  /**<   Whether help is available: \n
       - 0x00 -- No help is available \n
       - 0x01 -- Help is available
  */
}cat_help_available_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_RESPONSE_PACKING_FORMAT_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_RESPONSE_PACKING_FORMAT_UNPACKED_V02 = 0x00, 
  CAT_RESPONSE_PACKING_FORMAT_PACKED_V02 = 0x01, 
  CAT_RESPONSE_PACKING_FORMAT_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_response_packing_format_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_response_packing_format_enum_v02 response_packing_format;
  /**<   Response packing format: \n
       - 0x00 -- Unpacked format \n
       - 0x01 -- Packed format
  */
}cat_response_packing_format_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t maximum_user_input;
  /**<   Maximum user input. A value of 0xFF indicates no maximum.*/

  uint8_t minimum_user_input;
  /**<   Minimum user input. A value of 0x00 indicates no minimum.*/
}cat_response_length_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_SHOW_USER_INPUT_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_SHOW_USER_INPUT_MASKED_V02 = 0x00, 
  CAT_SHOW_USER_INPUT_CLEAR_V02 = 0x01, 
  CAT_SHOW_USER_INPUT_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_show_user_input_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_show_user_input_enum_v02 show_user_input;
  /**<   Show user input: \n
       - 0x00 -- ME can show * characters \n
       - 0x01 -- ME can show user input
  */
}cat_show_user_input_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_TONE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_TONE_DIAL_TONE_V02 = 0x01, 
  CAT_TONE_CALLED_SUBSCRIBER_BUSY_V02 = 0x02, 
  CAT_TONE_CONGESTION_V02 = 0x03, 
  CAT_TONE_RADIO_PATH_ACK_V02 = 0x04, 
  CAT_TONE_RADIO_PATH_NOT_AVAILABLE_CALL_DROP_V02 = 0x05, 
  CAT_TONE_ERROR_TONE_V02 = 0x06, 
  CAT_TONE_CALL_WAITING_TONE_V02 = 0x07, 
  CAT_TONE_RINGING_TONE_V02 = 0x08, 
  CAT_TONE_GENERAL_BEEP_V02 = 0x09, 
  CAT_TONE_POSITIVE_ACK_TONE_V02 = 0x0A, 
  CAT_TONE_NEGATIVE_ACK_TONE_V02 = 0x0B, 
  CAT_TONE_RINGING_TONE_SELECT_BY_USER_V02 = 0x0C, 
  CAT_TONE_SMS_ALERT_TONE_SELECT_BY_USER_V02 = 0x0D, 
  CAT_TONE_NOT_IN_USE_V02 = -1, 
  CAT_TONE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_tone_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_tone_enum_v02 tone;
  /**<   Tone to be played: \n
       - 0x01 -- Dial tone \n
       - 0x02 -- Called subscriber busy tone \n
       - 0x03 -- Congestion tone \n
       - 0x04 -- Radio path ACK tone \n
       - 0x05 -- Radio path not available, call drop tone \n
       - 0x06 -- Error tone \n
       - 0x07 -- Call waiting tone \n
       - 0x08 -- Ringing tone \n
       - 0x09 -- General beep \n
       - 0x0A -- Positive ACK tone \n
       - 0x0B -- Negative ACK tone \n
       - 0x0C -- Ring tone selected by the user \n
       - 0x0D -- SMS alert tone selected by the user \n
       - -1   -- Not in use
  */
}cat_tone_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_SOFTKEY_SELECTION_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_SK_SELECTION_NOT_SELECTED_V02 = 0x00, 
  CAT_SK_SELECTION_SELECTED_V02 = 0x01, 
  CAT_SOFTKEY_SELECTION_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_softkey_selection_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_softkey_selection_enum_v02 softkey_selection;
  /**<   Softkey selection: \n
       - 0x00 -- Softkey is not selected \n
       - 0x01 -- Softkey is selected
  */
}cat_softkey_selection_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t item_id;
  /**<   ID of the item. Each item has a unique identifier from 0x01 to 0xFF.*/

  uint32_t item_text_len;  /**< Must be set to # of elements in item_text */
  uint8_t item_text[QMI_CAT_ITEM_TEXT_MAX_LENGTH_V02];
  /**<   Item text. Coded the same way that alpha is coded in the EF-ADN file
       (see \hyperref[S6]{[S6]}, clause 4.4.2.3).
  */
}cat_single_item_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t items_len;  /**< Must be set to # of elements in items */
  cat_single_item_type_v02 items[QMI_CAT_NUMBER_OF_ITEMS_MAX_V02];
}cat_items_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t item_id;
  /**<   ID of the item. Each item has a unique identifier from 0x01 to 0xFF.*/

  cat_dcs_enum_v02 dcs;
  /**<   Data coding scheme: \n
       - 0x00 -- 7-bit GSM \n
       - 0x01 -- 8-bit GSM \n
       - 0x02 -- UCS2
  */

  uint32_t item_text_len;  /**< Must be set to # of elements in item_text */
  uint8_t item_text[QMI_CAT_ITEM_TEXT_MAX_LENGTH_V02];
  /**<   Item text (see \hyperref[S6]{[S6]}, clause 4.4.2.3).
  */
}cat_single_item_with_dcs_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t items_len;  /**< Must be set to # of elements in items */
  cat_single_item_with_dcs_type_v02 items[QMI_CAT_NUMBER_OF_ITEMS_MAX_V02];
}cat_items_with_dcs_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t default_item;
  /**<   Default item to be selected. All values are valid, except 0xFF, which
       is reserved (see \hyperref[S1]{[S1]}, Section 8.10).
  */
}cat_default_item_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_NEXT_ACTION_LIST_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_NEXT_ACTION_LIST_SETUP_CALL_V02 = 0x00, 
  CAT_NEXT_ACTION_LIST_SEND_SS_V02 = 0x01, 
  CAT_NEXT_ACTION_LIST_SEND_USSD_V02 = 0x02, 
  CAT_NEXT_ACTION_LIST_SEND_SHORT_MESSAGE_V02 = 0x03, 
  CAT_NEXT_ACTION_LIST_LAUNCH_BROWSER_V02 = 0x04, 
  CAT_NEXT_ACTION_LIST_PLAY_TONE_V02 = 0x05, 
  CAT_NEXT_ACTION_LIST_DISPLAY_TEXT_V02 = 0x06, 
  CAT_NEXT_ACTION_LIST_GET_INKEY_V02 = 0x07, 
  CAT_NEXT_ACTION_LIST_GET_INPUT_V02 = 0x08, 
  CAT_NEXT_ACTION_LIST_SELECT_ITEM_V02 = 0x09, 
  CAT_NEXT_ACTION_LIST_SETUP_MENU_V02 = 0x0A, 
  CAT_NEXT_ACTION_LIST_SETUP_IDLE_MODE_TEXT_V02 = 0x0B, 
  CAT_NEXT_ACTION_LIST_END_OF_PROACTIVE_SESSION_V02 = 0x0C, 
  CAT_NEXT_ACTION_LIST_PROVIDE_LOCAL_INFORMATION_V02 = 0x0D, 
  CAT_NEXT_ACTION_LIST_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_next_action_list_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t next_action_list_len;  /**< Must be set to # of elements in next_action_list */
  cat_next_action_list_enum_v02 next_action_list[QMI_CAT_ACTION_LIST_MAX_V02];
  /**<   Item in the action list: \n
       - 0x00 -- Setup Call \n
       - 0x01 -- Send SS \n
       - 0x02 -- Send USSD \n
       - 0x03 -- Send Short Message \n
       - 0x04 -- Launch Browser \n
       - 0x05 -- Play Tone \n
       - 0x06 -- Display Text \n
       - 0x07 -- Get Inkey \n
       - 0x08 -- Get Input \n
       - 0x09 -- Select Item \n
       - 0x0A -- Setup Menu \n
       - 0x0B -- Setup Idle Mode Text \n
       - 0x0C -- End of the Proactive Session \n
       - 0x0D -- Provide Local Information
  */
}cat_next_action_indicator_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t display_icon_only;
  /**<   Whether to display the icon only: \n
       - 0x00 -- Icon is not self-explanatory, display icon with description \n
       - 0x01 -- Icon is self-explanatory, display only the icon
  */

  uint32_t icon_list_len;  /**< Must be set to # of elements in icon_list */
  cat_icon_type_v02 icon_list[QMI_CAT_NUMBER_OF_ICONS_MAX_V02];
}cat_icon_id_list_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_PRESENTATION_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_PRESENTATION_NOT_SPECIFIED_V02 = 0x00, 
  CAT_PRESENTATION_DATA_VALUE_V02 = 0x01, 
  CAT_PRESENTATION_NAVIGATION_V02 = 0x02, 
  CAT_PRESENTATION_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_presentation_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_presentation_enum_v02 presentation;
  /**<   Presentation type: \n
       - 0x00 -- Not specified \n
       - 0x01 -- Data value presentation \n
       - 0x02 -- Navigation presentation
  */
}cat_presentation_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t packing_required;
  /**<   Indicates whether packing is required: \n
       - 0x00 -- Packing is not required \n
       - 0x01 -- Packing is required
  */
}cat_packing_required_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t sms_tpdu_len;  /**< Must be set to # of elements in sms_tpdu */
  uint8_t sms_tpdu[QMI_CAT_SMS_TPDU_MAX_LENGTH_V02];
  /**<   SMS TPDU data, as specified in \hyperref[S6]{[S6]}.
  */
}cat_sms_tpdu_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t is_cdma_sms;
  /**<   CDMA SMS format indication: \n
       - 0x00 -- FALSE (3GPP format) \n
       - 0x01 -- TRUE (3GPP2 format) \n
       This defaults to FALSE if the TLV is not present.
  */
}cat_is_cdma_sms_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_TON_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_TON_UNKNOWN_V02 = 0x00, 
  CAT_TON_INTERNATIONAL_NUMBER_V02 = 0x01, 
  CAT_TON_NATIONAL_NUMBER_V02 = 0x02, 
  CAT_TON_NETWORK_SPECIFIC_NUMBER_V02 = 0x03, 
  CAT_TON_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_ton_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_NPI_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_NPI_UNKNOWN_V02 = 0x00, 
  CAT_NPI_ISDN_TELE_V02 = 0x01, 
  CAT_NPI_DATA_V02 = 0x02, 
  CAT_NPI_TELEX_V02 = 0x03, 
  CAT_NPI_PRIVATE_V02 = 0x04, 
  CAT_NPI_EXTENSION_RESERVED_V02 = 0x0F, 
  CAT_NPI_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_npi_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_ton_enum_v02 ton;
  /**<   TON of the address: \n
       - 0x00 -- Unknown \n
       - 0x01 -- International number \n
       - 0x02 -- National number \n
       - 0x03 -- Network-specific number
  */

  cat_npi_enum_v02 npi;
  /**<   NPI of the address: \n
       - 0x00 -- Unknown \n
       - 0x01 -- ISDN telephony \n
       - 0x02 -- Data NPI \n
       - 0x03 -- Telex NPI \n
       - 0x04 -- Private NPI \n
       - 0x0F -- Extension is reserved
  */

  uint32_t address_data_len;  /**< Must be set to # of elements in address_data */
  uint8_t address_data[QMI_CAT_ADDRESS_MAX_LENGTH_V02];
  /**<   Address -- Decoded BCD string in ASCII format. The maximum
       length of the address is 200 bytes (see \hyperref[S1]{[S1]}, \n Section 8.1).
  */
}cat_address_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_CALL_SETUP_REQUIREMENT_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_CALL_SETUP_REQ_NO_OTHER_CALLS_V02 = 0x00, 
  CAT_CALL_SETUP_REQ_HOLD_ACTIVE_CALLS_V02 = 0x01, 
  CAT_CALL_SETUP_REQ_DISCONNECT_ACTIVE_CALLS_V02 = 0x02, 
  CAT_CALL_SETUP_REQUIREMENT_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_call_setup_requirement_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_call_setup_requirement_enum_v02 call_setup_requirement;
  /**<   Call setup requirements: \n
       - 0x00 -- No other calls \n
       - 0x01 -- Hold active calls \n
       - 0x02 -- Disconnect active calls
  */
}cat_call_setup_requirement_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t redial_necessary;
  /**<   Indicates whether redial is necessary: \n
       - 0x00 -- Redial is not necessary \n
       - 0x01 -- Redial is necessary
  */

  cat_time_units_enum_v02 units;
  /**<   Time units: \n
       - 0x00 -- Minutes \n
       - 0x01 -- Seconds \n
       - 0x02 -- Tenths of seconds
  */

  uint8_t interval;
  /**<   Time interval. This value must be greater than zero if
       redial_necessary is set to 0x01 (see \hyperref[S1]{[S1]}, Section 8.8).
  */
}cat_redial_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t subaddress_len;  /**< Must be set to # of elements in subaddress */
  uint8_t subaddress[QMI_CAT_SUBADDRESS_MAX_LENGTH_V02];
  /**<   Subaddress in BCD format (two digits encoded in one byte).
       Maximum size of the subaddress is 20 bytes (see \hyperref[S1]{[S1]}, Section 8.3).
  */
}cat_subaddress_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t capability_config_data_len;  /**< Must be set to # of elements in capability_config_data */
  uint8_t capability_config_data[QMI_CAT_CAPABILITY_CONFIG_MAX_LENGTH_V02];
  /**<   Capability configuration data (see \hyperref[S1]{[S1]},\n Section 8.4).*/
}cat_capability_config_data_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t dtmf_data_len;  /**< Must be set to # of elements in dtmf_data */
  uint8_t dtmf_data[QMI_CAT_DTMF_MAX_LENGTH_V02];
  /**<   DTMF data in BCD format (two digits encoded in one byte) (see \hyperref[S1]{[S1]}, \n Section 8.44).*/
}cat_dtmf_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t spec_lang_notify;
  /**<   Whether there is a specific language notification: \n
       - 0x00 -- No \n
       - 0x01 -- Yes
  */
}cat_spec_lang_notify_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t language;
  /**<   Language value. Each language code is a pair of alphanumeric
       characters (defined in \hyperref[S3]{[S3]}). Each alphanumeric
       character is coded on one byte using the SMS default 7-bit coded alphabet,
       as defined in \hyperref[S1]{[S1]}, Section 8.45, with bit 8 set to 0.
  */
}cat_language_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_LAUNCH_MODE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_LAUNCH_MODE_NOT_ALREADY_LAUNCHED_V02 = 0x00, 
  CAT_LAUNCH_MODE_USE_EXISTING_BROWSER_V02 = 0x01, 
  CAT_LAUNCH_MODE_CLOSE_EXISTING_BROWSER_V02 = 0x02, 
  CAT_LAUNCH_MODE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_launch_mode_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_launch_mode_enum_v02 launch_mode;
  /**<   Launch mode: \n
       - 0x00 -- Launch if not already launched \n
       - 0x01 -- Use the existing browser \n
       - 0x02 -- Close the existing browser
  */
}cat_launch_mode_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t url_data_len;  /**< Must be set to # of elements in url_data */
  uint8_t url_data[QMI_CAT_URL_MAX_LENGTH_V02];
  /**<   URL (see \hyperref[S1]{[S1]}, Section 8.48).*/
}cat_url_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t browser_id;
  /**<   Browser ID (see \hyperref[S1]{[S1]}, Section 8.47).*/
}cat_browser_id_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_BEARER_LIST_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_BEARER_LIST_SMS_V02 = 0x00, 
  CAT_BEARER_LIST_CSD_V02 = 0x01, 
  CAT_BEARER_LIST_USSD_V02 = 0x02, 
  CAT_BEARER_LIST_GPRS_V02 = 0x03, 
  CAT_BEARER_LIST_DEFAULT_V02 = 0x04, 
  CAT_BEARER_LIST_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_bearer_list_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t bearer_list_len;  /**< Must be set to # of elements in bearer_list */
  cat_bearer_list_enum_v02 bearer_list[QMI_CAT_BEARER_LIST_MAX_V02];
  /**<   Bearer list: \n
       - 0x00 -- SMS \n
       - 0x01 -- CSD \n
       - 0x02 -- USSD bearer code \n
       - 0x03 -- GPRS \n
       - 0x04 -- Bearer default
  */
}cat_bearer_list_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t path_len;  /**< Must be set to # of elements in path */
  uint8_t path[QMI_CAT_FILE_PATH_MAX_LENGTH_V02];
  /**<   Path to the provisioning file (see \hyperref[S1]{[S1]},\n Section 8.50).*/
}cat_file_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t file_len;  /**< Must be set to # of elements in file */
  cat_file_type_v02 file[QMI_CAT_NUMBER_OF_PROV_FILES_MAX_V02];
}cat_prov_file_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_dcs_enum_v02 orig_dcs_from_sim;
  /**<   Original data coding scheme from the SIM: \n
       - 0x00 -- 7-bit GSM \n
       - 0x01 -- 8-bit GSM \n
       - 0x02 -- UCS2
  */

  cat_dcs_enum_v02 dcs;
  /**<   Data coding scheme: \n
       - 0x00 -- 7-bit GSM \n
       - 0x01 -- 8-bit GSM \n
       - 0x02 -- UCS2
  */

  uint32_t text_len;  /**< Must be set to # of elements in text */
  uint8_t text[QMI_CAT_USSD_STRING_MAX_LENGTH_V02];
  /**<   Text of USSD string (see \hyperref[S4]{[S4]}, \n Section 8.17).
  */
}cat_ussd_string_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t immediate_resp;
  /**<   Indicates whether an immediate response is required: \n
       - 0x00 -- No \n
       - 0x01 -- Yes
  */
}cat_immediate_response_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t notification_required;
  /**<   Indicates whether the notification for a setup event list is required: \n
       - 0 -- Notification is not required \n
       - 1 -- Notification is required
  */
}cat_notification_required_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_play_tone_len;  /**< Must be set to # of elements in pc_play_tone */
  uint8_t pc_play_tone[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Play Tone proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.5.*/
}cat_play_tone_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_setup_call_len;  /**< Must be set to # of elements in pc_setup_call */
  uint8_t pc_setup_call[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Setup Call proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.12.*/
}cat_setup_call_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_send_dtmf_len;  /**< Must be set to # of elements in pc_send_dtmf */
  uint8_t pc_send_dtmf[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Send DTMF proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.24.*/
}cat_send_dtmf_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_launch_browser_len;  /**< Must be set to # of elements in pc_launch_browser */
  uint8_t pc_launch_browser[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Launch Browser proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.26.*/
}cat_launch_browser_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_send_sms_len;  /**< Must be set to # of elements in pc_send_sms */
  uint8_t pc_send_sms[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Send SMS proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.9.*/
}cat_send_sms_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_send_ss_len;  /**< Must be set to # of elements in pc_send_ss */
  uint8_t pc_send_ss[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Send SS proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.10.*/
}cat_send_ss_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_send_ussd_len;  /**< Must be set to # of elements in pc_send_ussd */
  uint8_t pc_send_ussd[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Send USSD proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.11.*/
}cat_send_ussd_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_provide_local_info_len;  /**< Must be set to # of elements in pc_provide_local_info */
  uint8_t pc_provide_local_info[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Provide Local Information proactive command, encoded as in \hyperref[S1]{[S1]}, \n Section 6.6.15.*/
}cat_provide_local_info_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_setup_event_list_len;  /**< Must be set to # of elements in pc_setup_event_list */
  uint8_t pc_setup_event_list[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Setup Event List proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.16.*/
}cat_setup_event_list_raw_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_SLOT_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_SLOT1_V02 = 0x01, 
  CAT_SLOT2_V02 = 0x02, 
  CAT_SLOT3_V02 = 0x03, 
  CAT_SLOT4_V02 = 0x04, 
  CAT_SLOT5_V02 = 0x05, 
  CAT_SLOT_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_slot_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_slot_enum_v02 slot;
  /**<   Indicates the slot to be used: \n
       - 0x01 -- Slot 1 \n
       - 0x02 -- Slot 2 \n
       - 0x03 -- Slot 3 \n
       - 0x04 -- Slot 4 \n
       - 0x05 -- Slot 5 \n
       Other values are reserved for future use.
  */
}cat_slot_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_open_channel_len;  /**< Must be set to # of elements in pc_open_channel */
  uint8_t pc_open_channel[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Open Channel proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.27.*/
}cat_open_channel_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_close_channel_len;  /**< Must be set to # of elements in pc_close_channel */
  uint8_t pc_close_channel[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Close Channel proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.28.*/
}cat_close_channel_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_send_data_len;  /**< Must be set to # of elements in pc_send_data */
  uint8_t pc_send_data[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Send Data proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.30.*/
}cat_send_data_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_receive_data_len;  /**< Must be set to # of elements in pc_receive_data */
  uint8_t pc_receive_data[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Receive Data proactive command, encoded as in \hyperref[S1]{[S1]}, Section 6.6.29.*/
}cat_receive_data_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t on_demand_link_est;
  /**<   Indicates whether the link is required: \n
       - 0x00 -- Link is not required \n
       - 0x01 -- Link is required
  */
}cat_on_demand_link_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_CSD_BEARER_NAME_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_CSD_BEARER_NAME_DATA_CIRCUIT_ASYNCH_UDI_3_1KHZ_V02 = 0x00, 
  CAT_CSD_BEARER_NAME_DATA_CIRCUIT_SYNCH_UDI_3_1KHZ_V02 = 0x01, 
  CAT_CSD_BEARER_NAME_PAD_ACCESS_ASYNC_UDI_V02 = 0x02, 
  CAT_CSD_BEARER_NAME_PACKET_ACCESS_SYNCH_UDI_V02 = 0x03, 
  CAT_CSD_BEARER_NAME_DATA_CIRCUIT_ASYNCH_RDI_V02 = 0x04, 
  CAT_CSD_BEARER_NAME_DATA_CIRCUIT_SYNCH_RDI_V02 = 0x05, 
  CAT_CSD_BEARER_NAME_PAD_ACCESS_ASYNC_RDI_V02 = 0x06, 
  CAT_CSD_BEARER_NAME_PACKET_ACCESS_SYNC_RDI_V02 = 0x07, 
  CAT_CSD_BEARER_NAME_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_csd_bearer_name_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_CSD_BEARER_CONNECTION_ELEMENT_TYPE_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_CSD_BEARER_CONNECTION_ELEMENT_CE_TRANSPARENT_V02 = 0x00, 
  CAT_CSD_BEARER_CONNECTION_ELEMENT_CE_NON_TRANSPARENT_V02 = 0x01, 
  CAT_CSD_BEARER_CONNECTION_ELEMENT_CE_BOTH_TRANSPARENT_V02 = 0x02, 
  CAT_CSD_BEARER_CONNECTION_ELEMENT_CE_NOTH_NON_TRANSPARENT_V02 = 0x03, 
  CAT_CSD_BEARER_CONNECTION_ELEMENT_TYPE_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_csd_bearer_connection_element_type_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t speed;
  /**<   Data rate; same as the speed subparameter defined in \hyperref[S5]{[S5]}, \n Section 6.7.*/

  cat_csd_bearer_name_enum_v02 name;
  /**<   CSD bearer name: \n
       - 0x00 -- Data Circuit Asynchronous; UDI or 3.1 kHz modem\n
       - 0x01 -- Data Circuit Synchronous; UDI or 3.1 kHz modem\n
       - 0x02 -- PAD Access Asynchronous UDI \n
       - 0x03 -- Packet Access Synchronous UDI \n
       - 0x04 -- Data Circuit Asynchronous RDI \n
       - 0x05 -- Data Circuit Synchronous RDI \n
       - 0x06 -- PAD Access Asynchronous RDI \n
       - 0x07 -- Packet Access Synchronous RDI
  */

  cat_csd_bearer_connection_element_type_v02 connection_element;
  /**<   CSD bearer connection element: \n
       - 0x00 -- Transparent \n
       - 0x01 -- Nontransparent \n
       - 0x02 -- Both, transparent preferred \n
       - 0x03 -- Both, nontransparent preferred
  */
}cat_csd_bearer_description_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_PACKET_DATA_PROTOCOL_TYPE_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_PACKET_DATA_PROTOCOL_IP_V02 = 2, 
  CAT_PACKET_DATA_PROTOCOL_TYPE_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_packet_data_protocol_type_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t precedence_cls;
  /**<   Precedence class; same as the precedence subparameter defined in \hyperref[S4]{[S4]}, Section 8.52.2.*/

  uint8_t delay_cls;
  /**<   Delay class; same as the delay subparameter defined in \hyperref[S4]{[S4]}, \n Section 8.52.2.*/

  uint8_t reliability_cls;
  /**<   Reliability class; same as the reliability subparameter defined in \hyperref[S4]{[S4]}, \n Section 8.52.2.*/

  uint8_t peak_throughput;
  /**<   Peak throughput class; same as the peak subparameter defined in \hyperref[S4]{[S4]}, \n Section 8.52.2.*/

  uint8_t mean_throughput;
  /**<   Mean throughput class; same as the mean subparameter defined in \hyperref[S4]{[S4]}, Section 8.52.2.*/

  cat_packet_data_protocol_type_v02 pkt_data_protocol;
  /**<   Packet Data Protocol: \n
       - 0x02 -- IP \n
       All other values are reserved.
  */
}cat_gprs_bearer_description_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_EUTRAN_TRAFFIC_CLASS_TYPE_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_EUTRAN_TRAFFIC_CLASS_CONVERSATIONAL_V02 = 0x00, 
  CAT_EUTRAN_TRAFFIC_CLASS_STREAMING_V02 = 0x01, 
  CAT_EUTRAN_TRAFFIC_CLASS_INTERACTIVE_V02 = 0x02, 
  CAT_EUTRAN_TRAFFIC_CLASS_BACKGROUND_V02 = 0x03, 
  CAT_EUTRAN_TRAFFIC_CLASS_SUBSCRIBED_VALUE_V02 = 0x04, 
  CAT_EUTRAN_TRAFFIC_CLASS_TYPE_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_eutran_traffic_class_type_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_EUTRAN_DELIVERY_ORDER_TYPE_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_EUTRAN_DELIVERY_ORDER_NO_V02 = 0x00, 
  CAT_EUTRAN_DELIVERY_ORDER_YES_V02 = 0x01, 
  CAT_EUTRAN_DELIVERY_ORDER_SUBSCRIBED_VALUE_V02 = 0x02, 
  CAT_EUTRAN_DELIVERY_ORDER_TYPE_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_eutran_delivery_order_type_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_EUTRAN_DELIVERY_OF_ERR_SDU_TYPE_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_EUTRAN_DELIVERY_OF_ERR_SDU_NO_V02 = 0x00, 
  CAT_EUTRAN_DELIVERY_OF_ERR_SDU_YES_V02 = 0x01, 
  CAT_EUTRAN_DELIVERY_OF_ERR_SDU_NO_DETECT_V02 = 0x02, 
  CAT_EUTRAN_DELIVERY_OF_ERR_SDU_SUBSCRIBED_VALUE_V02 = 0x03, 
  CAT_EUTRAN_DELIVERY_OF_ERR_SDU_TYPE_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_eutran_delivery_of_err_sdu_type_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_eutran_traffic_class_type_v02 traffic_class;
  /**<   Indicates the type of application for which the UMTS bearer service is
       optimized: \n
       - 0x00 -- Conversational \n
       - 0x01 -- Streaming \n
       - 0x02 -- Interactive \n
       - 0x03 -- Background \n
       - 0x04 -- Subscribed value \n
       All other values are reserved.
  */

  uint16_t max_bitrate_ul;
  /**<   Maximum bitrate UL; same as the maximum bitrate UL subparameter defined
       in \hyperref[S4]{[S4]}, Section 8.52.3.
  */

  uint16_t max_bitrate_dl;
  /**<   Maximum bitrate DL; same as the maximum bitrate DL subparameter defined
       in \hyperref[S4]{[S4]}, Section 8.52.3.
  */

  uint16_t guaranteed_bitrate_ul;
  /**<   Guaranteed bitrate UL; same as the guaranteed bitrate UL subparameter
       defined in \hyperref[S4]{[S4]}, Section 8.52.3.
  */

  uint16_t guaranteed_bitrate_dl;
  /**<   Guaranteed bitrate DL; same as the guaranteed bitrate DL subparameter
       defined in \hyperref[S4]{[S4]}, Section 8.52.3.
  */

  cat_eutran_delivery_order_type_v02 delivery_order;
  /**<   Numeric parameter that indicates if the UMTS bearer will provide
       in-sequence SDU delivery: \n
       - 0x00 -- No \n
       - 0x01 -- Yes \n
       - 0x02 -- Subscribed value \n
       All other values are reserved.
  */

  uint8_t max_sdu_size;
  /**<   Maximum SDU size; same as the Maximum SDU size subparameter defined
       in \hyperref[S4]{[S4]}, Section 8.52.3.
  */

  uint8_t max_sdu_err_ratio;
  /**<   SDU error ratio; same as the SDU error ratio subparameter defined
       in \hyperref[S4]{[S4]}, Section 8.52.3.
  */

  uint8_t residual_bit_err_ratio;
  /**<   Residual bit error ratio; same as the residual bit error ratio
       subparameter defined in \hyperref[S4]{[S4]}, Section 8.52.3.
  */

  cat_eutran_delivery_of_err_sdu_type_v02 delivery_of_err_sdu;
  /**<   Numeric parameter that indicates if SDUs detected as erroneous
       will be delivered: \n
       - 0x00 -- No \n
       - 0x01 -- Yes \n
       - 0x02 -- No detect \n
       - 0x03 -- Subscribed value \n
       All other values are reserved.
  */

  uint8_t transfer_delay;
  /**<   Transfer delay; same as the transfer delay subparameter defined
       in \hyperref[S4]{[S4]}, Section 8.52.3.
  */

  uint8_t traffic_handling_pri;
  /**<   Traffic handling priority; same as the traffic handling priority
       subparameter defined in \hyperref[S4]{[S4]}, Section 8.52.3.
  */

  cat_packet_data_protocol_type_v02 pdp_type;
  /**<   PDP type: \n
       - 0x02 -- IP \n
       All other values are reserved.
  */
}cat_eutran_ext_param_bearer_description_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t qci;
  /**<   QCI (see \hyperref[S4]{[S4]}, Section 8.52.5).*/

  uint8_t max_bitrate_ul;
  /**<   Maximum bitrate UL (see \hyperref[S4]{[S4]},\n Section 8.52.5).*/

  uint8_t max_bitrate_dl;
  /**<   Maximum bitrate DL (see \hyperref[S4]{[S4]},\n Section 8.52.5).*/

  uint8_t guaranteed_bitrate_ul;
  /**<   Guaranteed bitrate UL (see \hyperref[S4]{[S4]},\n Section 8.52.5).*/

  uint8_t guaranteed_bitrate_dl;
  /**<   Guaranteed bitrate DL (see \hyperref[S4]{[S4]},\n Section 8.52.5).*/

  uint8_t max_bitrate_ul_ext;
  /**<   Maximum bitrate UL Ext (see \hyperref[S4]{[S4]},\n Section 8.52.5).*/

  uint8_t max_bitrate_dl_ext;
  /**<   Maximum bitrate DL Ext (see \hyperref[S4]{[S4]},\n Section 8.52.5).*/

  uint8_t guaranteed_bitrate_ul_ext;
  /**<   Guaranteed bitrate UL Ext (see \hyperref[S4]{[S4]},\n Section 8.52.5).*/

  uint8_t guaranteed_bitrate_dl_ext;
  /**<   Guaranteed bitrate DL Ext (see \hyperref[S4]{[S4]},\n Section 8.52.5).*/

  cat_packet_data_protocol_type_v02 pdp_type;
  /**<   PDP type: \n
       - 0x02 -- IP \n
       All other values are reserved.
  */
}cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t buffer_size;
  /**<   Buffer size.*/
}cat_buffer_size_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t text_len;  /**< Must be set to # of elements in text */
  uint8_t text[QMI_CAT_NETWORK_ACCESS_NAME_MAX_LENGTH_V02];
  /**<   Network access name encoded in ASCII character (see \hyperref[S4]{[S4]}, Section 8.61).*/
}cat_network_access_name_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_IP_ADDRESS_TYPE_TYPE_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_IP_ADDRESS_TYPE_NO_ADDRESS_GIVEN_V02 = 0x01, 
  CAT_IP_ADDRESS_TYPE_DYNAMIC_V02 = 0x02, 
  CAT_IP_ADDRESS_TYPE_IPV4_V02 = 0x03, 
  CAT_IP_ADDRESS_TYPE_IPV6_V02 = 0x04, 
  CAT_IP_ADDRESS_TYPE_TYPE_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_ip_address_type_type_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_ip_address_type_type_v02 address_type;
  /**<   Address type: \n
       - 0x01 -- No address given \n
       - 0x02 -- Dynamic \n
       - 0x03 -- IPv4 \n
       - 0x04 -- IPv6 \n
       All other values are reserved.
  */

  uint32_t address_data_len;  /**< Must be set to # of elements in address_data */
  uint8_t address_data[QMI_CAT_NETWORK_ADDRESS_MAX_LENGTH_V02];
  /**<   Address (see \hyperref[S1]{[S1]}, Section 8.58).*/
}cat_ip_address_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_TRANSPORT_PROTOCOL_TYPE_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_TRANSPORT_PROTOCOL_NOT_PRESENT_V02 = 0x00, 
  CAT_TRANSPORT_PROTOCOL_UDP_V02 = 0x01, 
  CAT_TRANSPORT_PROTOCOL_TCP_V02 = 0x02, 
  CAT_TRANSPORT_PROTOCOL_TYPE_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_transport_protocol_type_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_transport_protocol_type_v02 transport_protocol;
  /**<   Transport protocol: \n
       - 0x00 -- Not present \n
       - 0x01 -- UDP \n
       - 0x02 -- TCP \n
       All other values are reserved.
  */

  uint16_t port_number;
  /**<   Port number.*/
}cat_transport_level_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ch_data_length;
  /**<   Number of bytes that are available in the channel buffer, or the number
       of bytes that are requested in a Received Data command (see \hyperref[S1]{[S1]}, Section 8.54).
  */
}cat_channel_data_lenght_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t send_data_immediately;
  /**<   Indicates whether to send the data immediately: \n
       - 0x00 -- No, store the data in the Tx buffer \n
       - 0x01 -- Yes, send the data immediately
  */
}cat_send_data_immediately_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t channel_data_string_len;  /**< Must be set to # of elements in channel_data_string */
  uint8_t channel_data_string[QMI_CAT_CHANNEL_DATA_MAX_LENGTH_V02];
  /**<   Channel data string is considered by the terminal as binary coded on 8 bits
       (see \hyperref[S1]{[S1]}, Section 8.53).
  */
}cat_channel_data_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t ch_id;
  /**<   Channel ID (see \hyperref[S1]{[S1]}, Section 8.7).*/
}cat_channel_id_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_activate_len;  /**< Must be set to # of elements in pc_activate */
  uint8_t pc_activate[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Activate proactive command encoded as in \hyperref[S1]{[S1]}, Section 6.6.40.*/
}cat_activate_event_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_ACTIVATE_TARGET_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_TARGET_UICC_CLF_V02 = 0x01, 
  CAT_ACTIVATE_TARGET_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_activate_target_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_activate_target_enum_v02 target;
  /**<   Activate descriptor target (see \hyperref[S1]{[S1]},\n Section 8.89):\n
       - 0x01 -- UICC-CLF interface according to \hyperref[S10]{[S10]} \n
       All other values are reserved for future use.
  */
}cat_activate_descriptor_target_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_INDICATION_EXPECTED_RESPONSE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_EXPECTED_RESPONSE_TR_V02 = 0x00, 
  CAT_EXPECTED_RESPONSE_EVENT_CONF_V02 = 0x01, 
  CAT_INDICATION_EXPECTED_RESPONSE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_indication_expected_response_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_BIP_STATUS_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_BIP_STATUS_IN_PROGRESS_V02 = 0x00, /**<  In progress  */
  CAT_BIP_STATUS_END_V02 = 0x01, /**<  End  */
  CAT_BIP_STATUS_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_bip_status_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_channel_id_type_v02 channel_id;

  cat_bip_status_enum_v02 status;
  /**<   Bearer Independent Protocol Status: \n
      - CAT_BIP_STATUS_IN_PROGRESS (0x00) --  In progress 
      - CAT_BIP_STATUS_END (0x01) --  End 
 All other values are reserved for future use and are to be ignored by the
 control point.
 */
}cat_bip_status_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID.*/

  uint32_t pc_refresh_alpha_len;  /**< Must be set to # of elements in pc_refresh_alpha */
  uint8_t pc_refresh_alpha[QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02];
  /**<   Refresh proactive command encoded as in \hyperref[S1]{[S1]}, Section 6.6.13. \n
       This is sent only if the refresh command contains alpha to be displayed.
  */
}cat_refresh_alpha_evt_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Indication Message; Indicates a QMI_CAT event. */
typedef struct {

  /* Optional */
  /*  Display Text Event */
  uint8_t display_text_valid;  /**< Must be set to true if display_text is being passed */
  cat_display_text_event_type_v02 display_text;

  /* Optional */
  /*  Get Inkey Event */
  uint8_t get_inkey_valid;  /**< Must be set to true if get_inkey is being passed */
  cat_get_inkey_event_type_v02 get_inkey;

  /* Optional */
  /*  Get Input Event */
  uint8_t get_input_valid;  /**< Must be set to true if get_input is being passed */
  cat_get_input_event_type_v02 get_input;

  /* Optional */
  /*  Setup Menu Event */
  uint8_t setup_menu_valid;  /**< Must be set to true if setup_menu is being passed */
  cat_setup_menu_event_type_v02 setup_menu;

  /* Optional */
  /*  Select Item Event */
  uint8_t select_item_valid;  /**< Must be set to true if select_item is being passed */
  cat_select_item_event_type_v02 select_item;

  /* Optional */
  /*  Alpha Identifier Available (used only when QMI_CAT is configured in Gobi mode) */
  uint8_t pc_alpha_id_available_valid;  /**< Must be set to true if pc_alpha_id_available is being passed */
  cat_alpha_id_type_v02 pc_alpha_id_available;

  /* Optional */
  /*  Setup Event List Event (used only when QMI_CAT is configured in Gobi mode) */
  uint8_t pc_setup_evt_list_valid;  /**< Must be set to true if pc_setup_evt_list is being passed */
  cat_setup_evt_list_type_v02 pc_setup_evt_list;

  /* Optional */
  /*  Setup Idle Mode Text Event */
  uint8_t idle_mode_text_valid;  /**< Must be set to true if idle_mode_text is being passed */
  cat_setup_idle_mode_text_event_type_v02 idle_mode_text;

  /* Optional */
  /*  Language Notification Event */
  uint8_t lang_notification_valid;  /**< Must be set to true if lang_notification is being passed */
  cat_lang_notification_event_type_v02 lang_notification;

  /* Optional */
  /*  Refresh Event (used only when QMI_CAT is configured in Gobi mode) */
  uint8_t refresh_valid;  /**< Must be set to true if refresh is being passed */
  cat_refresh_event_type_v02 refresh;

  /* Optional */
  /*  End Proactive Session */
  uint8_t proactive_session_end_type_valid;  /**< Must be set to true if proactive_session_end_type is being passed */
  cat_proactive_session_end_type_type_v02 proactive_session_end_type;

  /* Optional */
  /*  Decoded Header ID */
  uint8_t decoded_header_valid;  /**< Must be set to true if decoded_header is being passed */
  cat_decoded_header_type_v02 decoded_header;

  /* Optional */
  /*  Text String */
  uint8_t text_string_valid;  /**< Must be set to true if text_string is being passed */
  cat_dcs_encoded_text_type_v02 text_string;

  /* Optional */
  /*  High Priority */
  uint8_t high_priority_valid;  /**< Must be set to true if high_priority is being passed */
  cat_high_priority_type_v02 high_priority;

  /* Optional */
  /*  User Control */
  uint8_t user_control_valid;  /**< Must be set to true if user_control is being passed */
  cat_user_control_type_v02 user_control;

  /* Optional */
  /*  Icon */
  uint8_t icon_valid;  /**< Must be set to true if icon is being passed */
  cat_icon_type_v02 icon;

  /* Optional */
  /*  Duration */
  uint8_t duration_valid;  /**< Must be set to true if duration is being passed */
  cat_duration_type_v02 duration;

  /* Optional */
  /*  Response Format */
  uint8_t response_format_valid;  /**< Must be set to true if response_format is being passed */
  cat_response_format_type_v02 response_format;

  /* Optional */
  /*  Help Available */
  uint8_t help_available_valid;  /**< Must be set to true if help_available is being passed */
  cat_help_available_type_v02 help_available;

  /* Optional */
  /*  Response Packing Format */
  uint8_t response_packing_format_valid;  /**< Must be set to true if response_packing_format is being passed */
  cat_response_packing_format_type_v02 response_packing_format;

  /* Optional */
  /*  Response Length */
  uint8_t response_length_valid;  /**< Must be set to true if response_length is being passed */
  cat_response_length_type_v02 response_length;

  /* Optional */
  /*  Show User Input */
  uint8_t show_user_input_valid;  /**< Must be set to true if show_user_input is being passed */
  cat_show_user_input_type_v02 show_user_input;

  /* Optional */
  /*  Tone */
  uint8_t tone_valid;  /**< Must be set to true if tone is being passed */
  cat_tone_type_v02 tone;

  /* Optional */
  /*  Softkey Selection */
  uint8_t softkey_selection_valid;  /**< Must be set to true if softkey_selection is being passed */
  cat_softkey_selection_type_v02 softkey_selection;

  /* Optional */
  /*  Items */
  uint8_t items_valid;  /**< Must be set to true if items is being passed */
  cat_items_type_v02 items;

  /* Optional */
  /*  Default Item */
  uint8_t default_item_valid;  /**< Must be set to true if default_item is being passed */
  cat_default_item_type_v02 default_item;

  /* Optional */
  /*  Next Action Indicator */
  uint8_t next_action_list_valid;  /**< Must be set to true if next_action_list is being passed */
  cat_next_action_indicator_type_v02 next_action_list;

  /* Optional */
  /*  Icon ID List */
  uint8_t icon_id_list_valid;  /**< Must be set to true if icon_id_list is being passed */
  cat_icon_id_list_type_v02 icon_id_list;

  /* Optional */
  /*  Presentation */
  uint8_t presentation_valid;  /**< Must be set to true if presentation is being passed */
  cat_presentation_type_v02 presentation;

  /* Optional */
  /*  Packing Required */
  uint8_t packing_required_valid;  /**< Must be set to true if packing_required is being passed */
  cat_packing_required_type_v02 packing_required;

  /* Optional */
  /*  SMS TPDU */
  uint8_t sms_tpdu_valid;  /**< Must be set to true if sms_tpdu is being passed */
  cat_sms_tpdu_type_v02 sms_tpdu;

  /* Optional */
  /*  Is CDMA SMS */
  uint8_t is_cdma_sms_valid;  /**< Must be set to true if is_cdma_sms is being passed */
  cat_is_cdma_sms_type_v02 is_cdma_sms;

  /* Optional */
  /*  Address */
  uint8_t address_valid;  /**< Must be set to true if address is being passed */
  cat_address_type_v02 address;

  /* Optional */
  /*  Call Setup Requirement */
  uint8_t call_setup_requirement_valid;  /**< Must be set to true if call_setup_requirement is being passed */
  cat_call_setup_requirement_type_v02 call_setup_requirement;

  /* Optional */
  /*  Redial */
  uint8_t redial_valid;  /**< Must be set to true if redial is being passed */
  cat_redial_type_v02 redial;

  /* Optional */
  /*  Subaddress */
  uint8_t subaddress_valid;  /**< Must be set to true if subaddress is being passed */
  cat_subaddress_type_v02 subaddress;

  /* Optional */
  /*  Capability Configuration */
  uint8_t capability_config_data_valid;  /**< Must be set to true if capability_config_data is being passed */
  cat_capability_config_data_type_v02 capability_config_data;

  /* Optional */
  /*  DTMF */
  uint8_t dtmf_data_valid;  /**< Must be set to true if dtmf_data is being passed */
  cat_dtmf_type_v02 dtmf_data;

  /* Optional */
  /*  Specific Language Notification */
  uint8_t spec_lang_notify_valid;  /**< Must be set to true if spec_lang_notify is being passed */
  cat_spec_lang_notify_type_v02 spec_lang_notify;

  /* Optional */
  /*  Language */
  uint8_t language_valid;  /**< Must be set to true if language is being passed */
  cat_language_type_v02 language;

  /* Optional */
  /*  Launch Mode */
  uint8_t launch_mode_valid;  /**< Must be set to true if launch_mode is being passed */
  cat_launch_mode_type_v02 launch_mode;

  /* Optional */
  /*  URL */
  uint8_t url_valid;  /**< Must be set to true if url is being passed */
  cat_url_type_v02 url;

  /* Optional */
  /*  Browser ID */
  uint8_t browswer_id_valid;  /**< Must be set to true if browswer_id is being passed */
  cat_browser_id_type_v02 browswer_id;

  /* Optional */
  /*  Bearer List */
  uint8_t bearer_list_valid;  /**< Must be set to true if bearer_list is being passed */
  cat_bearer_list_type_v02 bearer_list;

  /* Optional */
  /*  Provisioning Files */
  uint8_t prov_files_valid;  /**< Must be set to true if prov_files is being passed */
  cat_prov_file_type_v02 prov_files;

  /* Optional */
  /*  USSD String */
  uint8_t ussd_string_valid;  /**< Must be set to true if ussd_string is being passed */
  cat_ussd_string_type_v02 ussd_string;

  /* Optional */
  /*  Default Text */
  uint8_t default_text_valid;  /**< Must be set to true if default_text is being passed */
  cat_dcs_encoded_text_type_v02 default_text;

  /* Optional */
  /*  Immediate Response Required */
  uint8_t immediate_resp_valid;  /**< Must be set to true if immediate_resp is being passed */
  cat_immediate_response_type_v02 immediate_resp;

  /* Optional */
  /*  User Confirmation Alpha */
  uint8_t user_conf_alpha_valid;  /**< Must be set to true if user_conf_alpha is being passed */
  cat_dcs_encoded_text_type_v02 user_conf_alpha;

  /* Optional */
  /*  Setup Call Display Alpha */
  uint8_t setup_call_disp_alpha_valid;  /**< Must be set to true if setup_call_disp_alpha is being passed */
  cat_dcs_encoded_text_type_v02 setup_call_disp_alpha;

  /* Optional */
  /*  User Confirmation Icon */
  uint8_t user_conf_icon_valid;  /**< Must be set to true if user_conf_icon is being passed */
  cat_icon_type_v02 user_conf_icon;

  /* Optional */
  /*  Setup Call Display Icon */
  uint8_t setup_call_disp_icon_valid;  /**< Must be set to true if setup_call_disp_icon is being passed */
  cat_icon_type_v02 setup_call_disp_icon;

  /* Optional */
  /*  Gateway Proxy */
  uint8_t gateway_proxy_valid;  /**< Must be set to true if gateway_proxy is being passed */
  cat_dcs_encoded_text_type_v02 gateway_proxy;

  /* Optional */
  /*  Alpha */
  uint8_t alpha_valid;  /**< Must be set to true if alpha is being passed */
  cat_dcs_encoded_text_type_v02 alpha;

  /* Optional */
  /*  Notification Required */
  uint8_t notification_required_valid;  /**< Must be set to true if notification_required is being passed */
  cat_notification_required_type_v02 notification_required;

  /* Optional */
  /*  Play Tone Event */
  uint8_t play_tone_valid;  /**< Must be set to true if play_tone is being passed */
  cat_play_tone_event_type_v02 play_tone;

  /* Optional */
  /*  Setup Call Event */
  uint8_t setup_call_valid;  /**< Must be set to true if setup_call is being passed */
  cat_setup_call_event_type_v02 setup_call;

  /* Optional */
  /*  Send DTMF Event */
  uint8_t send_dtmf_valid;  /**< Must be set to true if send_dtmf is being passed */
  cat_send_dtmf_event_type_v02 send_dtmf;

  /* Optional */
  /*  Launch Browser Event */
  uint8_t launch_browser_valid;  /**< Must be set to true if launch_browser is being passed */
  cat_launch_browser_event_type_v02 launch_browser;

  /* Optional */
  /*  Send SMS Event */
  uint8_t send_sms_valid;  /**< Must be set to true if send_sms is being passed */
  cat_send_sms_event_type_v02 send_sms;

  /* Optional */
  /*  Send SS Event */
  uint8_t send_ss_valid;  /**< Must be set to true if send_ss is being passed */
  cat_send_ss_event_type_v02 send_ss;

  /* Optional */
  /*  Send USSD Event */
  uint8_t send_ussd_valid;  /**< Must be set to true if send_ussd is being passed */
  cat_send_ussd_event_type_v02 send_ussd;

  /* Optional */
  /*  Provide Local Information Event */
  uint8_t provide_local_info_valid;  /**< Must be set to true if provide_local_info is being passed */
  cat_provide_local_info_event_type_v02 provide_local_info;

  /* Optional */
  /*  Setup Event List Raw Event */
  uint8_t setup_event_list_raw_valid;  /**< Must be set to true if setup_event_list_raw is being passed */
  cat_setup_event_list_raw_event_type_v02 setup_event_list_raw;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;

  /* Optional */
  /*  Open Channel Event */
  uint8_t open_channel_valid;  /**< Must be set to true if open_channel is being passed */
  cat_open_channel_event_type_v02 open_channel;

  /* Optional */
  /*  Close Channel Event */
  uint8_t close_channel_valid;  /**< Must be set to true if close_channel is being passed */
  cat_close_channel_event_type_v02 close_channel;

  /* Optional */
  /*  Send Data Event */
  uint8_t send_data_valid;  /**< Must be set to true if send_data is being passed */
  cat_send_data_event_type_v02 send_data;

  /* Optional */
  /*  Receive Data Event */
  uint8_t receive_data_valid;  /**< Must be set to true if receive_data is being passed */
  cat_receive_data_event_type_v02 receive_data;

  /* Optional */
  /*  On Demand Link Establish */
  uint8_t on_demand_link_valid;  /**< Must be set to true if on_demand_link is being passed */
  cat_on_demand_link_type_v02 on_demand_link;

  /* Optional */
  /*  CSD Bearer Description */
  uint8_t csd_bearer_description_valid;  /**< Must be set to true if csd_bearer_description is being passed */
  cat_csd_bearer_description_type_v02 csd_bearer_description;

  /* Optional */
  /*  GPRS Bearer Description */
  uint8_t gprs_bearer_description_valid;  /**< Must be set to true if gprs_bearer_description is being passed */
  cat_gprs_bearer_description_type_v02 gprs_bearer_description;

  /* Optional */
  /*  EUTRAN External Parameter Bearer Description */
  uint8_t eutran_ext_param_bearer_description_valid;  /**< Must be set to true if eutran_ext_param_bearer_description is being passed */
  cat_eutran_ext_param_bearer_description_type_v02 eutran_ext_param_bearer_description;

  /* Optional */
  /*  EUTRAN External Mapped UTRAN PS Bearer Description */
  uint8_t eutran_ext_mapped_bearer_description_valid;  /**< Must be set to true if eutran_ext_mapped_bearer_description is being passed */
  cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02 eutran_ext_mapped_bearer_description;

  /* Optional */
  /*  Buffer Size */
  uint8_t buffer_size_valid;  /**< Must be set to true if buffer_size is being passed */
  cat_buffer_size_type_v02 buffer_size;

  /* Optional */
  /*  Network Access Name */
  uint8_t network_access_name_valid;  /**< Must be set to true if network_access_name is being passed */
  cat_network_access_name_type_v02 network_access_name;

  /* Optional */
  /*  Other Address */
  uint8_t other_address_valid;  /**< Must be set to true if other_address is being passed */
  cat_ip_address_type_v02 other_address;

  /* Optional */
  /*  User Login */
  uint8_t user_login_valid;  /**< Must be set to true if user_login is being passed */
  cat_dcs_encoded_text_type_v02 user_login;

  /* Optional */
  /*  User Password */
  uint8_t user_password_valid;  /**< Must be set to true if user_password is being passed */
  cat_dcs_encoded_text_type_v02 user_password;

  /* Optional */
  /*  Transport Level */
  uint8_t transport_level_valid;  /**< Must be set to true if transport_level is being passed */
  cat_transport_level_type_v02 transport_level;

  /* Optional */
  /*  Data Destination Address */
  uint8_t data_destination_address_valid;  /**< Must be set to true if data_destination_address is being passed */
  cat_ip_address_type_v02 data_destination_address;

  /* Optional */
  /*  Channel Data Length */
  uint8_t channel_data_length_valid;  /**< Must be set to true if channel_data_length is being passed */
  cat_channel_data_lenght_type_v02 channel_data_length;

  /* Optional */
  /*  Send Data Immediately */
  uint8_t send_data_immediately_valid;  /**< Must be set to true if send_data_immediately is being passed */
  cat_send_data_immediately_type_v02 send_data_immediately;

  /* Optional */
  /*  Channel Data */
  uint8_t channel_data_valid;  /**< Must be set to true if channel_data is being passed */
  cat_channel_data_type_v02 channel_data;

  /* Optional */
  /*  Channel ID */
  uint8_t channel_id_valid;  /**< Must be set to true if channel_id is being passed */
  cat_channel_id_type_v02 channel_id;

  /* Optional */
  /*  Items with DCS */
  uint8_t items_with_dcs_valid;  /**< Must be set to true if items_with_dcs is being passed */
  cat_items_with_dcs_type_v02 items_with_dcs;

  /* Optional */
  /*  Activate Event */
  uint8_t activate_valid;  /**< Must be set to true if activate is being passed */
  cat_activate_event_type_v02 activate;

  /* Optional */
  /*  Activate Descriptor Target */
  uint8_t activate_target_valid;  /**< Must be set to true if activate_target is being passed */
  cat_activate_descriptor_target_type_v02 activate_target;

  /* Optional */
  /*  Response Type */
  uint8_t rsp_type_valid;  /**< Must be set to true if rsp_type is being passed */
  cat_indication_expected_response_enum_v02 rsp_type;
  /**<   Response type:
       - 0x00 -- Terminal response \n
       - 0x01 -- Event confirmation \n
       All other values are reserved. \n
       Indicates the action that the control point is expected to perform
       after receiving and processing the indication. If it is missing,
       the behavior described in Appendix C applies.
  */

  /* Optional */
  /*  Bearer Independent Protocol Status */
  uint8_t bip_status_valid;  /**< Must be set to true if bip_status is being passed */
  cat_bip_status_type_v02 bip_status;

  /* Optional */
  /*  Refresh Alpha */
  uint8_t refresh_alpha_valid;  /**< Must be set to true if refresh_alpha is being passed */
  cat_refresh_alpha_evt_type_v02 refresh_alpha;
}cat_event_report_ind_msg_v02;  /* Message */
/**
    @}
  */

/*
 * cat_get_service_state_req_msg is empty
 * typedef struct {
 * }cat_get_service_state_req_msg_v02;
 */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t cat_common_evt_reg_state_mask;
  /**<   Bitmask of events registered by all control points: \n
       - Bit 0  -- Display Text \n
       - Bit 1  -- Get Inkey \n
       - Bit 2  -- Get Input \n
       - Bit 3  -- Setup Menu \n
       - Bit 4  -- Select Item \n
       - Bit 5  -- Send SMS \n
       - Bit 6  -- Setup Event -- User Activity \n
       - Bit 7  -- Setup Event -- Idle Screen Notify \n
       - Bit 8  -- Setup Event -- Language Select Notify \n
       - Bit 9  -- Setup Idle Mode Text \n
       - Bit 10 -- Language Notification \n
       - Bit 11 -- Refresh/Refresh Alpha (Refresh when QMI_CAT is configured
                   in Gobi mode, Refresh Alpha in other cases) \n
       - Bit 12 -- End Proactive Session \n
       - Bit 13 -- Play Tone \n
       - Bit 14 -- Setup Call \n
       - Bit 15 -- Send DTMF \n
       - Bit 16 -- Launch Browser \n
       - Bit 17 -- Send SS \n
       - Bit 18 -- Send USSD \n
       - Bit 19 -- Provide Local Information -- Language \n
       - Bit 20 -- Bearer Independent Protocol \n
       - Bit 21 -- Setup Event -- Browser Termination \n
       - Bit 22 -- Provide Local Information -- Time \n
       - Bit 23 -- Clients are to ignore this bit \n
       - Bit 24 -- Activate \n
       - Bit 25 -- Setup Event -- HCI connectivity \n
       - Bit 26 -- Clients are to ignore this bit \n
       All unused bits are reserved for future use and are be ignored by the
       control point.
  */

  uint32_t pc_evt_report_mask;
  /**<   Bitmask of events registered by this control point: \n
       - Bit 0  -- Display Text \n
       - Bit 1  -- Get Inkey \n
       - Bit 2  -- Get Input \n
       - Bit 3  -- Setup Menu \n
       - Bit 4  -- Select Item \n
       - Bit 5  -- Send SMS \n
       - Bit 6  -- Setup Event -- User Activity \n
       - Bit 7  -- Setup Event -- Idle Screen Notify \n
       - Bit 8  -- Setup Event -- Language Select Notify \n
       - Bit 9  -- Setup Idle Mode Text \n
       - Bit 10 -- Language Notification \n
       - Bit 11 -- Refresh/Refresh Alpha (Refresh when QMI_CAT is configured
                   in Gobi mode, Refresh Alpha in other cases) \n
       - Bit 12 -- End Proactive Session \n
       - Bit 13 -- Play Tone \n
       - Bit 14 -- Setup Call \n
       - Bit 15 -- Send DTMF \n
       - Bit 16 -- Launch Browser \n
       - Bit 17 -- Send SS \n
       - Bit 18 -- Send USSD \n
       - Bit 19 -- Provide Local Information -- Language \n
       - Bit 20 -- Bearer Independent Protocol \n
       - Bit 21 -- Setup Event -- Browser Termination \n
       - Bit 22 -- Provide Local Information -- Time \n
       - Bit 23 -- Clients are to ignore this bit \n
       - Bit 24 -- Activate \n
       - Bit 25 -- Setup Event -- HCI connectivity \n
       - Bit 26 -- Clients are to ignore this bit \n
       All unused bits are reserved for future use and are ignored by the
       control point.
  */
}cat_service_state_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t cat_common_evt_reg_state_mask;
  /**<   Bitmask of decoded events registered by all control points: \n
       - Bit 0  -- Display Text \n
       - Bit 1  -- Get Inkey \n
       - Bit 2  -- Get Input \n
       - Bit 3  -- Setup Menu \n
       - Bit 4  -- Select Item \n
       - Bit 5  -- Send SMS \n
       - Bit 6  -- Setup Event -- User Activity \n
       - Bit 7  -- Setup Event -- Idle Screen Notify \n
       - Bit 8  -- Setup Event -- Language Select Notify \n
       - Bit 9  -- Setup Idle Mode Text \n
       - Bit 10 -- Language Notification \n
       - Bit 11 -- Refresh Alpha (not supported when QMI CAT is configured 
                   in Gobi mode) \n
       - Bit 12 -- End Proactive Session \n
       - Bit 13 -- Play Tone \n
       - Bit 14 -- Setup Call \n
       - Bit 15 -- Send DTMF \n
       - Bit 16 -- Launch Browser \n
       - Bit 17 -- Send SS \n
       - Bit 18 -- Send USSD \n
       - Bit 19 -- Provide Local Information -- Language \n
       - Bit 20 -- Bearer Independent Protocol \n
       - Bit 21 -- Setup Event -- Browser Termination \n
       - Bit 22 -- Clients are to ignore this bit \n
       - Bit 23 -- Smart Card Web Server \n
       - Bit 24 -- Activate \n
       - Bit 25 -- Setup Event -- HCI connectivity \n
       - Bit 26 -- Bearer Independent Protocol Status \n
       All unused bits are reserved for future use and are ignored by the
       control point.
  */

  uint32_t pc_evt_report_mask;
  /**<   Bitmask of decoded events registered by this control point: \n
       - Bit 0  -- Display Text \n
       - Bit 1  -- Get Inkey \n
       - Bit 2  -- Get Input \n
       - Bit 3  -- Setup Menu \n
       - Bit 4  -- Select Item \n
       - Bit 5  -- Send SMS \n
       - Bit 6  -- Setup Event -- User Activity \n
       - Bit 7  -- Setup Event -- Idle Screen Notify \n
       - Bit 8  -- Setup Event -- Language Select Notify \n
       - Bit 9  -- Setup Idle Mode Text \n
       - Bit 10 -- Language Notification \n
       - Bit 11 -- Refresh Alpha (not supported when QMI CAT is configured 
                   in Gobi mode) \n
       - Bit 12 -- End Proactive Session \n
       - Bit 13 -- Play Tone \n
       - Bit 14 -- Setup Call \n
       - Bit 15 -- Send DTMF \n
       - Bit 16 -- Launch Browser \n
       - Bit 17 -- Send SS \n
       - Bit 18 -- Send USSD \n
       - Bit 19 -- Provide Local Information -- Language \n
       - Bit 20 -- Bearer Independent Protocol \n
       - Bit 21 -- Setup Event -- Browser Termination \n
       - Bit 22 -- Clients are to ignore this bit \n
       - Bit 23 -- Smart Card Web Server \n
       - Bit 24 -- Activate \n
       - Bit 25 -- Setup Event -- HCI connectivity \n
       - Bit 26 -- Bearer Independent Protocol status \n
       All unused bits are reserved for future use and are ignored by the
       control point.
  */
}cat_decoded_service_state_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Queries the QMI_CAT service state. */
typedef struct {

  /* Mandatory */
  /*  CAT Service State */
  cat_service_state_type_v02 cat_service_state;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       - qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       - qmi_error_type  - Error code. Possible error code values are described in
                           the error codes section of each message definition.
  */

  /* Optional */
  /*  Decoded CAT Service State */
  uint8_t decoded_cat_service_state_valid;  /**< Must be set to true if decoded_cat_service_state is being passed */
  cat_decoded_service_state_type_v02 decoded_cat_service_state;

  /* Optional */
  /*  Full Function Event Service State */
  uint8_t pc_full_func_evt_report_mask_valid;  /**< Must be set to true if pc_full_func_evt_report_mask is being passed */
  cat_set_event_report_full_func_mask_v02 pc_full_func_evt_report_mask;
  /**<   Full function event report bitmask registered by this control point: \n
       - Bit 0  -- Send SMS \n
       - Bit 1  -- Setup call \n
       - Bit 2  -- Send DTMF \n
       - Bit 3  -- Send SS \n
       - Bit 4  -- Send USSD \n
       All unused bits are reserved for future use and are ignored by the control point.
  */
}cat_get_service_state_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID. This is the same reference ID as indicated
       in the event report indication for the relevant proactive command.
  */

  uint32_t terminal_response_len;  /**< Must be set to # of elements in terminal_response */
  uint8_t terminal_response[QMI_CAT_TERMINAL_RESPONSE_MAX_LENGTH_V02];
  /**<   Terminal response for the relevant proactive command, encoded as in
       \hyperref[S1]{[S1]}, Section 6.8.
  */
}cat_terminal_response_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Sends the terminal response to the proactive commands coming from
             the card.
            \label{idl:SendTR} */
typedef struct {

  /* Mandatory */
  /*  Terminal Response */
  cat_terminal_response_type_v02 terminal_response;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_send_tr_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t sw1;
  /**<   Value of SW1 of the response, as defined in \hyperref[S8]{[S8]} for ICC
       and \hyperref[S9]{[S9]} for UICC.
  */

  uint8_t sw2;
  /**<   Value of SW2 of the response as defined in \hyperref[S8]{[S8]} for ICC
       and \hyperref[S9]{[S9]} for UICC.
  */

  uint32_t tr_response_len;  /**< Must be set to # of elements in tr_response */
  uint8_t tr_response[QMI_CAT_TERMINAL_RESPONSE_MAX_LENGTH_V02];
  /**<   TR response data.*/
}cat_terminal_resp_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Sends the terminal response to the proactive commands coming from
             the card.
            \label{idl:SendTR} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */

  /* Optional */
  /*  TR Response */
  uint8_t tr_response_data_valid;  /**< Must be set to true if tr_response_data is being passed */
  cat_terminal_resp_type_v02 tr_response_data;
}cat_send_tr_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_ENV_CMD_TYPE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_ENVELOPE_CMD_TYPE_MENU_SELECTION_V02 = 0x01, 
  CAT_ENVELOPE_CMD_TYPE_EVENT_DL_USER_ACTIVITY_V02 = 0x02, 
  CAT_ENVELOPE_CMD_TYPE_EVENT_DL_IDLE_SCREEN_AVAIL_V02 = 0x03, 
  CAT_ENVELOPE_CMD_TYPE_EVENT_DL_LANGUAGE_SELECTION_V02 = 0x04, 
  CAT_ENVELOPE_CMD_TYPE_UNKNOWN_V02 = 0x05, 
  CAT_ENVELOPE_CMD_TYPE_EVENT_DL_BROWSER_TERMINATION_V02 = 0x06, 
  CAT_ENVELOPE_CMD_TYPE_SEND_CALL_CONTROL_V02 = 0x07, 
  CAT_ENVELOPE_CMD_TYPE_HCI_CONNECTIVITY_V02 = 0x08, 
  CAT_ENVELOPE_CMD_TYPE_SMS_PP_DATA_DL_V02 = 0x09, 
  CAT_ENVELOPE_CMD_TYPE_EVENT_DL_MT_CALL_V02 = 0x0A, 
  CAT_ENVELOPE_CMD_TYPE_EVENT_DL_CALL_CONNECTED_V02 = 0x0B, 
  CAT_ENVELOPE_CMD_TYPE_EVENT_DL_CALL_DISCONNECTED_V02 = 0x0C, 
  CAT_ENV_CMD_TYPE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_env_cmd_type_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_env_cmd_type_enum_v02 env_cmd_type;
  /**<   Envelope command type: \n
       - 0x01 -- Menu Selection \n
       - 0x02 -- Event DL User Activity \n
       - 0x03 -- Event DL Idle Screen Available \n
       - 0x04 -- Event DL Language Selection \n
       - 0x05 -- Unknown Type \n
       - 0x06 -- Event DL Browser Termination \n
       - 0x07 -- Send Call Control \n
       - 0x08 -- Event DL HCI Connectivity \n
       - 0x09 -- SMS-PP Data Download \n
       - 0x0A -- Event DL MT Call \n
       - 0x0B -- Event DL Call Connected \n
       - 0x0C -- Event DL Call Disconnected \n
       All other values are reserved.
  */

  uint32_t envelope_data_len;  /**< Must be set to # of elements in envelope_data */
  uint8_t envelope_data[QMI_CAT_ENVELOPE_DATA_MAX_LENGTH_V02];
  /**<   Encoded envelope response, as defined in \hyperref[S1]{[S1]}, Section 7.*/
}cat_envelope_cmd_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Sends an envelope command to the card. */
typedef struct {

  /* Mandatory */
  /*  Envelope Command */
  cat_envelope_cmd_type_v02 envelope_cmd;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_send_envelope_cmd_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t sw1;
  /**<   Value of SW1 of the response, as defined in \hyperref[S8]{[S8]}
       for ICC and \hyperref[S9]{[S9]} for UICC.
  */

  uint8_t sw2;
  /**<   Value of SW2 of the response, as defined in \hyperref[S8]{[S8]}
       for ICC and \hyperref[S9]{[S9]} for UICC.
  */

  uint32_t env_resp_data_len;  /**< Must be set to # of elements in env_resp_data */
  uint8_t env_resp_data[QMI_CAT_RAW_ENV_RSP_DATA_MAX_LENGTH_V02];
  /**<   Envelope response data.*/
}cat_envelope_resp_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Sends an envelope command to the card. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */

  /* Optional */
  /*  Raw Envelope Respone Data */
  uint8_t env_resp_data_valid;  /**< Must be set to true if env_resp_data is being passed */
  cat_envelope_resp_type_v02 env_resp_data;
}cat_send_envelope_cmd_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_FORMAT_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_FORMAT_RAW_V02 = 0x01, 
  CAT_FORMAT_DECODED_V02 = 0x02, 
  CAT_FORMAT_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_format_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t cmd_ref_id;
  /**<   Command reference ID.*/

  cat_format_enum_v02 format;
  /**<   Format in which to get the proactive command data: \n
       - 0x01 -- Raw \n
       - 0x02 -- Decoded
  */
}cat_proactive_command_input_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Retrieves the last proactive command from the modem. */
typedef struct {

  /* Mandatory */
  /*  Proactive Command Input */
  cat_proactive_command_input_type_v02 proactive_command_input;
}cat_get_event_report_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Retrieves the last proactive command from the modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */

  /* Optional */
  /*  Display Text Event */
  uint8_t display_text_valid;  /**< Must be set to true if display_text is being passed */
  cat_display_text_event_type_v02 display_text;

  /* Optional */
  /*  Get Inkey Event */
  uint8_t get_inkey_valid;  /**< Must be set to true if get_inkey is being passed */
  cat_get_inkey_event_type_v02 get_inkey;

  /* Optional */
  /*  Get Input Event */
  uint8_t get_input_valid;  /**< Must be set to true if get_input is being passed */
  cat_get_input_event_type_v02 get_input;

  /* Optional */
  /*  Setup Menu Event */
  uint8_t setup_menu_valid;  /**< Must be set to true if setup_menu is being passed */
  cat_setup_menu_event_type_v02 setup_menu;

  /* Optional */
  /*  Select Item Event */
  uint8_t select_item_valid;  /**< Must be set to true if select_item is being passed */
  cat_select_item_event_type_v02 select_item;

  /* Optional */
  /*  Alpha Identifier Available (used only when QMI_CAT is configured in Gobi mode) */
  uint8_t pc_alpha_id_available_valid;  /**< Must be set to true if pc_alpha_id_available is being passed */
  cat_alpha_id_type_v02 pc_alpha_id_available;

  /* Optional */
  /*  Setup Event List Event (used only when QMI_CAT is configured in Gobi mode) */
  uint8_t pc_setup_evt_list_valid;  /**< Must be set to true if pc_setup_evt_list is being passed */
  cat_setup_evt_list_type_v02 pc_setup_evt_list;

  /* Optional */
  /*  Setup Idle Mode Text Event */
  uint8_t idle_mode_text_valid;  /**< Must be set to true if idle_mode_text is being passed */
  cat_setup_idle_mode_text_event_type_v02 idle_mode_text;

  /* Optional */
  /*  Language Notification Event */
  uint8_t lang_notification_valid;  /**< Must be set to true if lang_notification is being passed */
  cat_lang_notification_event_type_v02 lang_notification;

  /* Optional */
  /*  Refresh Event (used only when QMI_CAT is configured in Gobi mode) */
  uint8_t refresh_valid;  /**< Must be set to true if refresh is being passed */
  cat_refresh_event_type_v02 refresh;

  /* Optional */
  /*  End Proactive Session */
  uint8_t proactive_session_end_type_valid;  /**< Must be set to true if proactive_session_end_type is being passed */
  cat_proactive_session_end_type_type_v02 proactive_session_end_type;

  /* Optional */
  /*  Decoded Header ID */
  uint8_t decoded_header_valid;  /**< Must be set to true if decoded_header is being passed */
  cat_decoded_header_type_v02 decoded_header;

  /* Optional */
  /*  Text String */
  uint8_t text_string_valid;  /**< Must be set to true if text_string is being passed */
  cat_dcs_encoded_text_type_v02 text_string;

  /* Optional */
  /*  High Priority */
  uint8_t high_priority_valid;  /**< Must be set to true if high_priority is being passed */
  cat_high_priority_type_v02 high_priority;

  /* Optional */
  /*  User Control */
  uint8_t user_control_valid;  /**< Must be set to true if user_control is being passed */
  cat_user_control_type_v02 user_control;

  /* Optional */
  /*  Icon */
  uint8_t icon_valid;  /**< Must be set to true if icon is being passed */
  cat_icon_type_v02 icon;

  /* Optional */
  /*  Duration */
  uint8_t duration_valid;  /**< Must be set to true if duration is being passed */
  cat_duration_type_v02 duration;

  /* Optional */
  /*  Response Format */
  uint8_t response_format_valid;  /**< Must be set to true if response_format is being passed */
  cat_response_format_type_v02 response_format;

  /* Optional */
  /*  Help Available */
  uint8_t help_available_valid;  /**< Must be set to true if help_available is being passed */
  cat_help_available_type_v02 help_available;

  /* Optional */
  /*  Response Packing Format */
  uint8_t response_packing_format_valid;  /**< Must be set to true if response_packing_format is being passed */
  cat_response_packing_format_type_v02 response_packing_format;

  /* Optional */
  /*  Response Length */
  uint8_t response_length_valid;  /**< Must be set to true if response_length is being passed */
  cat_response_length_type_v02 response_length;

  /* Optional */
  /*  Show User Input */
  uint8_t show_user_input_valid;  /**< Must be set to true if show_user_input is being passed */
  cat_show_user_input_type_v02 show_user_input;

  /* Optional */
  /*  Tone */
  uint8_t tone_valid;  /**< Must be set to true if tone is being passed */
  cat_tone_type_v02 tone;

  /* Optional */
  /*  Softkey Selection */
  uint8_t softkey_selection_valid;  /**< Must be set to true if softkey_selection is being passed */
  cat_softkey_selection_type_v02 softkey_selection;

  /* Optional */
  /*  Items */
  uint8_t items_valid;  /**< Must be set to true if items is being passed */
  cat_items_type_v02 items;

  /* Optional */
  /*  Default Item */
  uint8_t default_item_valid;  /**< Must be set to true if default_item is being passed */
  cat_default_item_type_v02 default_item;

  /* Optional */
  /*  Next Action Indicator */
  uint8_t next_action_list_valid;  /**< Must be set to true if next_action_list is being passed */
  cat_next_action_indicator_type_v02 next_action_list;

  /* Optional */
  /*  Icon ID List */
  uint8_t icon_id_list_valid;  /**< Must be set to true if icon_id_list is being passed */
  cat_icon_id_list_type_v02 icon_id_list;

  /* Optional */
  /*  Presentation */
  uint8_t presentation_valid;  /**< Must be set to true if presentation is being passed */
  cat_presentation_type_v02 presentation;

  /* Optional */
  /*  Packing Required */
  uint8_t packing_required_valid;  /**< Must be set to true if packing_required is being passed */
  cat_packing_required_type_v02 packing_required;

  /* Optional */
  /*  SMS TPDU */
  uint8_t sms_tpdu_valid;  /**< Must be set to true if sms_tpdu is being passed */
  cat_sms_tpdu_type_v02 sms_tpdu;

  /* Optional */
  /*  Is CDMA SMS */
  uint8_t is_cdma_sms_valid;  /**< Must be set to true if is_cdma_sms is being passed */
  cat_is_cdma_sms_type_v02 is_cdma_sms;

  /* Optional */
  /*  Address */
  uint8_t address_valid;  /**< Must be set to true if address is being passed */
  cat_address_type_v02 address;

  /* Optional */
  /*  Call Setup Requirement */
  uint8_t call_setup_requirement_valid;  /**< Must be set to true if call_setup_requirement is being passed */
  cat_call_setup_requirement_type_v02 call_setup_requirement;

  /* Optional */
  /*  Redial */
  uint8_t redial_valid;  /**< Must be set to true if redial is being passed */
  cat_redial_type_v02 redial;

  /* Optional */
  /*  Subaddress */
  uint8_t subaddress_valid;  /**< Must be set to true if subaddress is being passed */
  cat_subaddress_type_v02 subaddress;

  /* Optional */
  /*  Capability Configuration */
  uint8_t capability_config_data_valid;  /**< Must be set to true if capability_config_data is being passed */
  cat_capability_config_data_type_v02 capability_config_data;

  /* Optional */
  /*  DTMF */
  uint8_t dtmf_data_valid;  /**< Must be set to true if dtmf_data is being passed */
  cat_dtmf_type_v02 dtmf_data;

  /* Optional */
  /*  Specific Language Notification */
  uint8_t spec_lang_notify_valid;  /**< Must be set to true if spec_lang_notify is being passed */
  cat_spec_lang_notify_type_v02 spec_lang_notify;

  /* Optional */
  /*  Language */
  uint8_t language_valid;  /**< Must be set to true if language is being passed */
  cat_language_type_v02 language;

  /* Optional */
  /*  Launch Mode */
  uint8_t launch_mode_valid;  /**< Must be set to true if launch_mode is being passed */
  cat_launch_mode_type_v02 launch_mode;

  /* Optional */
  /*  URL */
  uint8_t url_valid;  /**< Must be set to true if url is being passed */
  cat_url_type_v02 url;

  /* Optional */
  /*  Browser ID */
  uint8_t browswer_id_valid;  /**< Must be set to true if browswer_id is being passed */
  cat_browser_id_type_v02 browswer_id;

  /* Optional */
  /*  Bearer List */
  uint8_t bearer_list_valid;  /**< Must be set to true if bearer_list is being passed */
  cat_bearer_list_type_v02 bearer_list;

  /* Optional */
  /*  Provisioning Files */
  uint8_t prov_files_valid;  /**< Must be set to true if prov_files is being passed */
  cat_prov_file_type_v02 prov_files;

  /* Optional */
  /*  USSD String */
  uint8_t ussd_string_valid;  /**< Must be set to true if ussd_string is being passed */
  cat_ussd_string_type_v02 ussd_string;

  /* Optional */
  /*  Default Text */
  uint8_t default_text_valid;  /**< Must be set to true if default_text is being passed */
  cat_dcs_encoded_text_type_v02 default_text;

  /* Optional */
  /*  Immediate Response Request */
  uint8_t immediate_resp_valid;  /**< Must be set to true if immediate_resp is being passed */
  cat_immediate_response_type_v02 immediate_resp;

  /* Optional */
  /*  User Confirmation Alpha */
  uint8_t user_conf_alpha_valid;  /**< Must be set to true if user_conf_alpha is being passed */
  cat_dcs_encoded_text_type_v02 user_conf_alpha;

  /* Optional */
  /*  Setup Call Display Alpha */
  uint8_t setup_call_disp_alpha_valid;  /**< Must be set to true if setup_call_disp_alpha is being passed */
  cat_dcs_encoded_text_type_v02 setup_call_disp_alpha;

  /* Optional */
  /*  User Confirmation Icon */
  uint8_t user_conf_icon_valid;  /**< Must be set to true if user_conf_icon is being passed */
  cat_icon_type_v02 user_conf_icon;

  /* Optional */
  /*  Setup Call Display Icon */
  uint8_t setup_call_disp_icon_valid;  /**< Must be set to true if setup_call_disp_icon is being passed */
  cat_icon_type_v02 setup_call_disp_icon;

  /* Optional */
  /*  Gateway Proxy */
  uint8_t gateway_proxy_valid;  /**< Must be set to true if gateway_proxy is being passed */
  cat_dcs_encoded_text_type_v02 gateway_proxy;

  /* Optional */
  /*  Alpha */
  uint8_t alpha_valid;  /**< Must be set to true if alpha is being passed */
  cat_dcs_encoded_text_type_v02 alpha;

  /* Optional */
  /*  Notification Required */
  uint8_t notification_required_valid;  /**< Must be set to true if notification_required is being passed */
  cat_notification_required_type_v02 notification_required;

  /* Optional */
  /*  Play Tone Event */
  uint8_t play_tone_valid;  /**< Must be set to true if play_tone is being passed */
  cat_play_tone_event_type_v02 play_tone;

  /* Optional */
  /*  Setup Call Event */
  uint8_t setup_call_valid;  /**< Must be set to true if setup_call is being passed */
  cat_setup_call_event_type_v02 setup_call;

  /* Optional */
  /*  Send DTMF Event */
  uint8_t send_dtmf_valid;  /**< Must be set to true if send_dtmf is being passed */
  cat_send_dtmf_event_type_v02 send_dtmf;

  /* Optional */
  /*  Launch Browser Event */
  uint8_t launch_browser_valid;  /**< Must be set to true if launch_browser is being passed */
  cat_launch_browser_event_type_v02 launch_browser;

  /* Optional */
  /*  Send SMS Event */
  uint8_t send_sms_valid;  /**< Must be set to true if send_sms is being passed */
  cat_send_sms_event_type_v02 send_sms;

  /* Optional */
  /*  Send SS Event */
  uint8_t send_ss_valid;  /**< Must be set to true if send_ss is being passed */
  cat_send_ss_event_type_v02 send_ss;

  /* Optional */
  /*  Send USSD Event */
  uint8_t send_ussd_valid;  /**< Must be set to true if send_ussd is being passed */
  cat_send_ussd_event_type_v02 send_ussd;

  /* Optional */
  /*  Provide Local Information Event */
  uint8_t provide_local_info_valid;  /**< Must be set to true if provide_local_info is being passed */
  cat_provide_local_info_event_type_v02 provide_local_info;

  /* Optional */
  /*  Setup Event List Raw Event */
  uint8_t setup_event_list_raw_valid;  /**< Must be set to true if setup_event_list_raw is being passed */
  cat_setup_event_list_raw_event_type_v02 setup_event_list_raw;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;

  /* Optional */
  /*  Open Channel Event */
  uint8_t open_channel_valid;  /**< Must be set to true if open_channel is being passed */
  cat_open_channel_event_type_v02 open_channel;

  /* Optional */
  /*  Close Channel Event */
  uint8_t close_channel_valid;  /**< Must be set to true if close_channel is being passed */
  cat_close_channel_event_type_v02 close_channel;

  /* Optional */
  /*  Send Data Event */
  uint8_t send_data_valid;  /**< Must be set to true if send_data is being passed */
  cat_send_data_event_type_v02 send_data;

  /* Optional */
  /*  Receive Data Event */
  uint8_t receive_data_valid;  /**< Must be set to true if receive_data is being passed */
  cat_receive_data_event_type_v02 receive_data;

  /* Optional */
  /*  On Demand Link Establish */
  uint8_t on_demand_link_valid;  /**< Must be set to true if on_demand_link is being passed */
  cat_on_demand_link_type_v02 on_demand_link;

  /* Optional */
  /*  CSD Bearer Description */
  uint8_t csd_bearer_description_valid;  /**< Must be set to true if csd_bearer_description is being passed */
  cat_csd_bearer_description_type_v02 csd_bearer_description;

  /* Optional */
  /*  GPRS Bearer Description */
  uint8_t gprs_bearer_description_valid;  /**< Must be set to true if gprs_bearer_description is being passed */
  cat_gprs_bearer_description_type_v02 gprs_bearer_description;

  /* Optional */
  /*  EUTRAN External Parameter Bearer Description */
  uint8_t eutran_ext_param_bearer_description_valid;  /**< Must be set to true if eutran_ext_param_bearer_description is being passed */
  cat_eutran_ext_param_bearer_description_type_v02 eutran_ext_param_bearer_description;

  /* Optional */
  /*  EUTRAN External Mapped UTRAN PS Bearer Description */
  uint8_t eutran_ext_mapped_bearer_description_valid;  /**< Must be set to true if eutran_ext_mapped_bearer_description is being passed */
  cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02 eutran_ext_mapped_bearer_description;

  /* Optional */
  /*  Buffer Size */
  uint8_t buffer_size_valid;  /**< Must be set to true if buffer_size is being passed */
  cat_buffer_size_type_v02 buffer_size;

  /* Optional */
  /*  Network Access Name */
  uint8_t network_access_name_valid;  /**< Must be set to true if network_access_name is being passed */
  cat_network_access_name_type_v02 network_access_name;

  /* Optional */
  /*  Other Address */
  uint8_t other_address_valid;  /**< Must be set to true if other_address is being passed */
  cat_ip_address_type_v02 other_address;

  /* Optional */
  /*  User Login */
  uint8_t user_login_valid;  /**< Must be set to true if user_login is being passed */
  cat_dcs_encoded_text_type_v02 user_login;

  /* Optional */
  /*  User Password */
  uint8_t user_password_valid;  /**< Must be set to true if user_password is being passed */
  cat_dcs_encoded_text_type_v02 user_password;

  /* Optional */
  /*  Transport Level */
  uint8_t transport_level_valid;  /**< Must be set to true if transport_level is being passed */
  cat_transport_level_type_v02 transport_level;

  /* Optional */
  /*  Data Destination Address */
  uint8_t data_destination_address_valid;  /**< Must be set to true if data_destination_address is being passed */
  cat_ip_address_type_v02 data_destination_address;

  /* Optional */
  /*  Channel Data Length */
  uint8_t channel_data_length_valid;  /**< Must be set to true if channel_data_length is being passed */
  cat_channel_data_lenght_type_v02 channel_data_length;

  /* Optional */
  /*  Send Data Immediately */
  uint8_t send_data_immediately_valid;  /**< Must be set to true if send_data_immediately is being passed */
  cat_send_data_immediately_type_v02 send_data_immediately;

  /* Optional */
  /*  Channel Data */
  uint8_t channel_data_valid;  /**< Must be set to true if channel_data is being passed */
  cat_channel_data_type_v02 channel_data;

  /* Optional */
  /*  Channel ID */
  uint8_t channel_id_valid;  /**< Must be set to true if channel_id is being passed */
  cat_channel_id_type_v02 channel_id;

  /* Optional */
  /*  Items with DCS */
  uint8_t items_with_dcs_valid;  /**< Must be set to true if items_with_dcs is being passed */
  cat_items_with_dcs_type_v02 items_with_dcs;

  /* Optional */
  /*  Activate Event */
  uint8_t activate_valid;  /**< Must be set to true if activate is being passed */
  cat_activate_event_type_v02 activate;

  /* Optional */
  /*  Activate Descriptor Target */
  uint8_t activate_target_valid;  /**< Must be set to true if activate_target is being passed */
  cat_activate_descriptor_target_type_v02 activate_target;

  /* Optional */
  /*  Response Type */
  uint8_t rsp_type_valid;  /**< Must be set to true if rsp_type is being passed */
  cat_indication_expected_response_enum_v02 rsp_type;
  /**<   Response type: \n
       - 0x00 -- Terminal response \n
       - 0x01 -- Event confirmation \n
       All other values are reserved. \n
       Indicates the action that the control point is expected to perform
       after receiving and processing the indication. If it is missing,
       the behavior described in Appendix C applies.
  */

  /* Optional */
  /*  Bearer Independent Protocol Status */
  uint8_t bip_status_valid;  /**< Must be set to true if bip_status is being passed */
  cat_bip_status_type_v02 bip_status;

  /* Optional */
  /*  Refresh Alpha */
  uint8_t refresh_alpha_valid;  /**< Must be set to true if refresh_alpha is being passed */
  cat_refresh_alpha_evt_type_v02 refresh_alpha;
}cat_get_event_report_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_RESPONSE_CMD_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_RESPONSE_CMD_DISPLAY_TEXT_V02 = 0x01, 
  CAT_RESPONSE_CMD_GET_INKEY_V02 = 0x02, 
  CAT_RESPONSE_CMD_GET_INPUT_V02 = 0x03, 
  CAT_RESPONSE_CMD_LAUNCH_BROWSER_V02 = 0x04, 
  CAT_RESPONSE_CMD_PLAY_TONE_V02 = 0x05, 
  CAT_RESPONSE_CMD_SELECT_ITEM_REQ_V02 = 0x06, 
  CAT_RESPONSE_CMD_SETUP_MENU_V02 = 0x07, 
  CAT_RESPONSE_CMD_SETUP_IDLE_TEXT_V02 = 0x08, 
  CAT_RESPONSE_CMD_PROVIDE_LOCAL_LANG_INFO_V02 = 0x09, 
  CAT_RESPONSE_CMD_SETUP_EVENT_USER_ACTIVITY_V02 = 0x0A, 
  CAT_RESPONSE_CMD_SETUP_EVENT_IDLE_SCREEN_NOTIFY_V02 = 0x0B, 
  CAT_RESPONSE_CMD_SETUP_EVENT_LANGUAGE_SEL_NOTIFY_V02 = 0x0C, 
  CAT_RESPONSE_CMD_LANGUAGE_NOTIFICATION_V02 = 0x0D, 
  CAT_RESPONSE_CMD_ACTIVATE_V02 = 0x0E, 
  CAT_RESPONSE_CMD_SETUP_EVENT_HCI_CONNECTIVITY_V02 = 0x0F, 
  CAT_RESPONSE_CMD_SETUP_EVENT_BROWSER_TERMINATION_V02 = 0x10, 
  CAT_RESPONSE_CMD_SEND_SMS_V02 = 0x11, 
  CAT_RESPONSE_CMD_SETUP_CALL_V02 = 0x12, 
  CAT_RESPONSE_CMD_SEND_DTMF_V02 = 0x13, 
  CAT_RESPONSE_CMD_SEND_SS_V02 = 0x14, 
  CAT_RESPONSE_CMD_SEND_USSD_V02 = 0x15, 
  CAT_RESPONSE_CMD_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_response_cmd_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_GENERAL_RESULT_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_RESULT_COMMAND_PERFORMED_SUCCESSFULLY_V02 = 0x00, 
  CAT_RESULT_COMMAND_PERFORMED_WITH_PARTIAL_COMPREHENSION_V02 = 0x01, 
  CAT_RESULT_COMMAND_PERFORMED_WITH_MISSING_INFORMATION_V02 = 0x02, 
  CAT_RESULT_REFRESH_PERFORMED_WITH_ADDITIONAL_EFS_READ_V02 = 0x03, 
  CAT_RESULT_COMMAND_SUCCESSFUL_BUT_REQUESTED_ICON_NOT_DISPLAYED_V02 = 0x04, 
  CAT_RESULT_COMMAND_PERFORMED_BUT_MODIFIED_BY_CALL_CONTROL_BY_NAA_V02 = 0x05, 
  CAT_RESULT_COMMAND_SUCCESSFUL_LIMITED_SERVICE_V02 = 0x06, 
  CAT_RESULT_COMMAND_PERFORMED_WITH_MODIFICATION_V02 = 0x07, 
  CAT_RESULT_REFRESH_PERFORMED_BUT_NAA_NOT_ACTIVE_V02 = 0x08, 
  CAT_RESULT_COMMAND_SUCCESSFUL_TONE_NOT_PLAYED_V02 = 0x09, 
  CAT_RESULT_PROACTIVE_UICC_SESSION_TERMINATED_BY_USER_V02 = 0x10, 
  CAT_RESULT_BACKWARD_MOVE_IN_SESSION_REQUESTED_BY_USER_V02 = 0x11, 
  CAT_RESULT_NO_RESPONSE_FROM_USER_V02 = 0x12, 
  CAT_RESULT_HELP_INFORMATION_REQUIRED_BY_THE_USER_V02 = 0x13, 
  CAT_RESULT_USSD_OR_SS_TRANSACTION_TERMINATED_BY_THE_USER_V02 = 0x14, 
  CAT_RESULT_TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND_V02 = 0x20, 
  CAT_RESULT_NETWORK_CURRENTLY_UNABLE_TO_PROCESS_COMMAND_V02 = 0x21, 
  CAT_RESULT_USER_DID_NOT_ACCEPT_PROACTIVE_COMMAND_V02 = 0x22, 
  CAT_RESULT_USER_CLEARED_DOWN_CALL_BEFORE_CONNECTION_OR_NETWORK_RELEASE_V02 = 0x23, 
  CAT_RESULT_ACTION_IN_CONTRADICTION_WITH_THE_CURRENT_TIMER_STATE_V02 = 0x24, 
  CAT_RESULT_INTERACTION_WITH_CALL_CONTROL_BY_NAA_TEMPORARY_PROBLEM_V02 = 0x25, 
  CAT_RESULT_LAUNCH_BROWSER_GENERIC_ERROR_V02 = 0x26, 
  CAT_RESULT_MMS_TEMPORARY_ERROR_V02 = 0x27, 
  CAT_RESULT_COMMAND_BEYOND_TERMINAL_CAPABILITIES_V02 = 0x30, 
  CAT_RESULT_COMMAND_TYPE_NOT_UNDERSTOOD_BY_TERMINAL_V02 = 0x31, 
  CAT_RESULT_COMMAND_DATA_NOT_UNDERSTOOD_BY_TERMINAL_V02 = 0x32, 
  CAT_RESULT_COMMAND_NUMBER_NOT_KNOWN_BY_TERMINAL_V02 = 0x33, 
  CAT_RESULT_SS_RETURN_ERROR_V02 = 0x34, 
  CAT_RESULT_SMS_RP_ERROR_V02 = 0x35, 
  CAT_RESULT_ERROR_REQUIRED_VALUES_ARE_MISSING_V02 = 0x36, 
  CAT_RESULT_USSD_RETURN_ERROR_V02 = 0x37, 
  CAT_RESULT_MULTIPLE_CARD_COMMANDS_ERROR_V02 = 0x38, 
  CAT_RESULT_INTERACTION_WITH_CC_BY_SIM_OR_MO_SM_CONTROL_BY_SIM_PROBLEM_V02 = 0x39, 
  CAT_RESULT_BEARER_INDEPENDENT_PROTOCOL_ERROR_V02 = 0x3A, 
  CAT_RESULT_ACCESS_TECHNOLOGY_UNABLE_TO_PROCESS_COMMAND_V02 = 0x3B, 
  CAT_RESULT_FRAMES_ERROR_V02 = 0x3C, 
  CAT_RESULT_MMS_ERROR_V02 = 0x3D, 
  CAT_GENERAL_RESULT_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_general_result_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uim_ref_id;
  /**<   Proactive command reference ID. This is the same reference ID as indicated
       in the event report indication for the relevant proactive command.
  */

  uint8_t command_number;
  /**<   Command number for which the terminal response is sent.*/

  cat_response_cmd_enum_v02 response_cmd;
  /**<   Type of proactive command for which the terminal response is sent: \n
       - 0x01 -- Display Text \n
       - 0x02 -- Get Inkey \n
       - 0x03 -- Get Input \n
       - 0x04 -- Launch Browser \n
       - 0x05 -- Play Tone \n
       - 0x06 -- Select Item Request \n
       - 0x07 -- Setup Menu \n
       - 0x08 -- Setup Idle Text \n
       - 0x09 -- Provide Local Information -- Language \n
       - 0x0A -- Setup Event -- User Activity \n
       - 0x0B -- Setup Event -- Idle Screen Notify \n
       - 0x0C -- Setup Event -- Language Select Notify \n
       - 0x0D -- Language Notification \n
       - 0x0E -- Activate \n
       - 0x0F -- Setup Event -- HCI Connectivity \n
       - 0x10 -- Setup Event -- Browser Termination \n
       - 0x11 -- Send SMS \n
       - 0x12 -- Setup Call \n
       - 0x13 -- Send DTMF \n
       - 0x14 -- Send SS \n
       - 0x15 -- Send USSD \n
       All other values are reserved.
  */

  cat_general_result_enum_v02 general_result;
  /**<   Result of the proactive command, as defined in \hyperref[S1]{[S1]}, Section 8.12.*/

  uint32_t tr_additional_info_len;  /**< Must be set to # of elements in tr_additional_info */
  uint8_t tr_additional_info[QMI_CAT_TR_ADDITIONAL_INFO_MAX_LENGTH_V02];
  /**<   Additional information is only required for some commands. \hyperref[S1]{[S1]},
       Section 8.12, describes the additional information. The maximum size
       is 10.
  */
}cat_terminal_response_command_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t identifier;
  /**<   Identifier of the item chosen: \n
       - 0x00 -- NULL identifier \n
       - 0x01 to 0xFF -- Value of the item
  */
}cat_tr_item_identifier_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_time_units_enum_v02 unit;
  /**<   Time units: \n
      - 0x00 -- Minutes \n
      - 0x01 -- Seconds \n
      - 0x02 -- Tenths of seconds \n
      - -1   -- Duration is not present
  */

  uint8_t interval;
  /**<   Time interval. This number must be greater than zero.*/

  cat_dcs_encoded_text_type_v02 get_inkey_text;
  /**<   Text of get inkey. If a Yes/No input required from the user, the
       DCS value is ignored and the string must contain only 1 byte: 0x00
       for No, 0x01 for Yes.
  */
}cat_tr_get_inkey_extra_info_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_TR_GET_INKEY_YES_NO_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_TEXT_NO_V02 = 0x00, 
  CAT_TEXT_YES_V02 = 0x01, 
  CAT_TR_GET_INKEY_YES_NO_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_tr_get_inkey_yes_no_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_time_units_enum_v02 unit;
  /**<   Time units: \n
      - 0x00 -- Minutes \n
      - 0x01 -- Seconds \n
      - 0x02 -- Tenths of seconds \n
      - -1   -- Duration is not present
  */

  uint8_t interval;
  /**<   Time interval. This number must be greater than zero.*/

  cat_tr_get_inkey_yes_no_enum_v02 get_inkey_yes_no;
  /**<   Yes/No input for get inkey: \n
      - 0x00  -- No \n
      - 0x01  -- Yes \n
       If a text input is required from the user, the Get Inkey Extra Info TLV
       must be used.
  */
}cat_tr_get_inkey_yes_no_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Sends the Terminal Response (TR) in decoded format to the
             proactive commands coming from the card. */
typedef struct {

  /* Mandatory */
  /*  Terminal Response */
  cat_terminal_response_command_type_v02 terminal_response;

  /* Optional */
  /*  Text String */
  uint8_t text_string_valid;  /**< Must be set to true if text_string is being passed */
  cat_dcs_encoded_text_type_v02 text_string;

  /* Optional */
  /*  Item Identifier */
  uint8_t identifier_valid;  /**< Must be set to true if identifier is being passed */
  cat_tr_item_identifier_type_v02 identifier;

  /* Optional */
  /*  Get Inkey Extra Info */
  uint8_t get_inkey_extra_info_valid;  /**< Must be set to true if get_inkey_extra_info is being passed */
  cat_tr_get_inkey_extra_info_type_v02 get_inkey_extra_info;

  /* Optional */
  /*  Language Info */
  uint8_t language_valid;  /**< Must be set to true if language is being passed */
  cat_language_type_v02 language;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;

  /* Optional */
  /*  Get Inkey Yes/No Info */
  uint8_t gst_inkey_yes_no_info_valid;  /**< Must be set to true if gst_inkey_yes_no_info is being passed */
  cat_tr_get_inkey_yes_no_type_v02 gst_inkey_yes_no_info;
}cat_send_decoded_tr_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Sends the Terminal Response (TR) in decoded format to the
             proactive commands coming from the card. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */

  /* Optional */
  /*  TR Response */
  uint8_t tr_response_data_valid;  /**< Must be set to true if tr_response_data is being passed */
  cat_terminal_resp_type_v02 tr_response_data;
}cat_send_decoded_tr_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_DECODED_ENV_CMD_TYPE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_DECODED_ENVELOPE_CMD_TYPE_MENU_SELECTION_V02 = 0x01, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_EVENT_DL_LANGUAGE_SELECTION_V02 = 0x02, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_EVENT_DL_USER_ACTIVITY_V02 = 0x03, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_EVENT_DL_IDLE_SCREEN_AVAIL_V02 = 0x04, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_SEND_CALL_CONTROL_V02 = 0x05, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_HCI_CONNECTIVITY_V02 = 0x06, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_BROWSER_TERMINATION_V02 = 0x07, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_SMS_PP_DATA_DL_V02 = 0x08, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_EVENT_DL_MT_CALL_V02 = 0x09, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_EVENT_DL_MT_CALL_CONNECTED_V02 = 0x0A, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_EVENT_DL_MO_CALL_CONNECTED_V02 = 0x0B, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_EVENT_DL_CALL_DISCONNECTED_NEAR_END_V02 = 0x0C, 
  CAT_DECODED_ENVELOPE_CMD_TYPE_EVENT_DL_CALL_DISCONNECTED_FAR_END_V02 = 0x0D, 
  CAT_DECODED_ENV_CMD_TYPE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_decoded_env_cmd_type_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  cat_decoded_env_cmd_type_enum_v02 env_cmd_type;
  /**<   Decoded envelope command type. See Appendix \ref{idl:EnvCmdTLVs}
       for information on mandatory and optional TLVs for each envelope command. \n
       - 0x01 -- Menu Selection \n
       - 0x02 -- Event DL Language Selection \n
       - 0x03 -- Event DL User Activity \n
       - 0x04 -- Event DL Idle Screen Available \n
       - 0x05 -- Send Call Control \n
       - 0x06 -- Event DL HCI Connectivity \n
       - 0x07 -- Event DL Browser Termination \n
       - 0x08 -- SMS-PP Data Download \n
       - 0x09 -- Event DL MT Call \n
       - 0x0A -- Event DL MT Call Connected \n
       - 0x0B -- Event DL MO Call Connected \n
       - 0x0C -- Event DL Call Disconnected near end\n
       - 0x0D -- Event DL Call Disconnected far end\n
       All other values are reserved.
  */
}cat_decoded_envelope_cmd_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t identifier;
  /**<   Identifier of the item chosen.
  */
}cat_decoded_envelope_identifier_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t help_request;
  /**<   Whether help is requested: \n
       - 0x00 -- No help is requested \n
       - 0x01 -- Help is requested
  */
}cat_decoded_envelope_help_request_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t pdp_context_act_data_len;  /**< Must be set to # of elements in pdp_context_act_data */
  uint8_t pdp_context_act_data[QMI_CAT_PDP_CONTEXT_ACT_MAX_LENGTH_V02];
  /**<   PDP context activation data. Coded as the Activate PDP Context
       Request message, specified in \hyperref[S6]{[S6]}.
  */
}cat_pdp_context_act_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t eps_pdn_connect_act_data_len;  /**< Must be set to # of elements in eps_pdn_connect_act_data */
  uint8_t eps_pdn_connect_act_data[QMI_CAT_EPS_PDN_CONNECT_ACT_MAX_LENGTH_V02];
  /**<   EPS PDN connect activation data; coded as the PDN Connectivity
       Request message, specified in \hyperref[S7]{[S7]}.
  */
}cat_eps_pdn_connect_act_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_BROWSER_TERM_CAUSE_TYPE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_BROWSER_TERM_CAUSE_TYPE_USER_TERMINATED_V02 = 0x00000000, 
  CAT_BROWSER_TERM_CAUSE_TYPE_ERROR_V02 = 0x00000001, 
  CAT_BROWSER_TERM_CAUSE_TYPE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_browser_term_cause_type_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_ACCESS_TECHNOLOGY_TYPE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_ACCESS_NONE_V02 = 0x00000000, 
  CAT_ACCESS_TECH_GSM_V02 = 0x00000001, 
  CAT_ACCESS_TECH_UTRAN_V02 = 0x00000002, 
  CAT_ACCESS_TECH_CDMA_V02 = 0x00000003, 
  CAT_ACCESS_TECH_LTE_V02 = 0x00000004, 
  CAT_ACCESS_TECHNOLOGY_TYPE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_access_technology_type_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_CALL_TYPE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_CALL_CONTROL_VOICE_V02 = 0x00000000, 
  CAT_CALL_CONTROL_SS_V02 = 0x00000001, 
  CAT_CALL_CONTROL_USSD_V02 = 0x00000002, 
  CAT_CALL_CONTROL_SMS_V02 = 0x00000003, 
  CAT_CALL_TYPE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_call_type_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Sends an envelope command in decoded format to the card. */
typedef struct {

  /* Mandatory */
  /*  Envelope Command */
  cat_decoded_envelope_cmd_type_v02 env_cmd_type;

  /* Optional */
  /*  Item Identifier */
  uint8_t identifier_valid;  /**< Must be set to true if identifier is being passed */
  cat_decoded_envelope_identifier_type_v02 identifier;

  /* Optional */
  /*  Help Request */
  uint8_t help_request_valid;  /**< Must be set to true if help_request is being passed */
  cat_decoded_envelope_help_request_type_v02 help_request;

  /* Optional */
  /*  Language */
  uint8_t language_valid;  /**< Must be set to true if language is being passed */
  cat_language_type_v02 language;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;

  /* Optional */
  /*  Address */
  uint8_t address_valid;  /**< Must be set to true if address is being passed */
  cat_address_type_v02 address;

  /* Optional */
  /*  Subaddress */
  uint8_t sub_address_valid;  /**< Must be set to true if sub_address is being passed */
  cat_subaddress_type_v02 sub_address;

  /* Optional */
  /*  Capability Configuration Parameter 1 */
  uint8_t capability_config_param1_valid;  /**< Must be set to true if capability_config_param1 is being passed */
  cat_capability_config_data_type_v02 capability_config_param1;

  /* Optional */
  /*  Capability Configuration Parameter 2 */
  uint8_t capability_config_param2_valid;  /**< Must be set to true if capability_config_param2 is being passed */
  cat_capability_config_data_type_v02 capability_config_param2;

  /* Optional */
  /*  USSD String */
  uint8_t ussd_string_valid;  /**< Must be set to true if ussd_string is being passed */
  cat_dcs_encoded_text_type_v02 ussd_string;

  /* Optional */
  /*  PDP Context Activation */
  uint8_t pdp_context_act_valid;  /**< Must be set to true if pdp_context_act is being passed */
  cat_pdp_context_act_type_v02 pdp_context_act;

  /* Optional */
  /*  EPS PDN Connect Activation */
  uint8_t eps_pdn_connect_act_valid;  /**< Must be set to true if eps_pdn_connect_act is being passed */
  cat_eps_pdn_connect_act_type_v02 eps_pdn_connect_act;

  /* Optional */
  /*  Browser Termination Cause */
  uint8_t browser_term_cause_valid;  /**< Must be set to true if browser_term_cause is being passed */
  cat_browser_term_cause_type_enum_v02 browser_term_cause;
  /**<   Browser termination cause: \n
       - 0x00000000 -- CAT_BROWSER_ TERM_CAUSE_TYPE_USER_ TERMINATED -- User terminated the browser \n
       - 0x00000001 -- CAT_BROWSER_ TERM_CAUSE_TYPE_ERROR -- Browser terminated due to error
  */

  /* Optional */
  /*  SMS TPDU */
  uint8_t sms_tpdu_valid;  /**< Must be set to true if sms_tpdu is being passed */
  cat_sms_tpdu_type_v02 sms_tpdu;

  /* Optional */
  /*  Is CDMA SMS */
  uint8_t is_cdma_sms_valid;  /**< Must be set to true if is_cdma_sms is being passed */
  cat_is_cdma_sms_type_v02 is_cdma_sms;

  /* Optional */
  /*  Radio Access Technology */
  uint8_t rat_valid;  /**< Must be set to true if rat is being passed */
  cat_access_technology_type_enum_v02 rat;
  /**<   Access technology type: \n
       - 0x00000000 -- CAT_ACCESS_TECH_NONE -- RAT is unknown \n
       - 0x00000001 -- CAT_ACCESS_TECH_GSM -- GSM is used \n
       - 0x00000002 -- CAT_ACCESS_TECH_UTRAN -- UTRAN is used \n
       - 0x00000003 -- CAT_ACCESS_TECH_CDMA -- CDMA is used \n
       - 0x00000004 -- CAT_ACCESS_TECH_LTE -- LTE is used
  */

  /* Optional */
  /*  Call Type */
  uint8_t call_type_valid;  /**< Must be set to true if call_type is being passed */
  cat_call_type_enum_v02 call_type;
  /**<   Call Type: \n
       - 0x00000000 -- CAT_VOICE -- Voice \n
       - 0x00000001 -- CAT_SS -- SS \n
       - 0x00000002 -- CAT_USSD -- USSD \n
       - 0x00000003 -- CAT_SMS -- SMS
  */

  /* Optional */
  /*  Transaction ID */
  uint8_t transaction_id_valid;  /**< Must be set to true if transaction_id is being passed */
  uint32_t transaction_id_len;  /**< Must be set to # of elements in transaction_id */
  uint8_t transaction_id[QMI_CAT_TX_ID_MAX_LENGTH_V02];
  /**<   Call transaction ID (see \hyperref[S1]{[S1]},\n Section 8.28).*/

  /* Optional */
  /*  RP Address */
  uint8_t rp_dest_address_valid;  /**< Must be set to true if rp_dest_address is being passed */
  cat_address_type_v02 rp_dest_address;

  /* Optional */
  /*  TP Address */
  uint8_t tp_dest_address_valid;  /**< Must be set to true if tp_dest_address is being passed */
  cat_address_type_v02 tp_dest_address;

  /* Optional */
  /*  Cause */
  uint8_t cause_valid;  /**< Must be set to true if cause is being passed */
  uint32_t cause_len;  /**< Must be set to # of elements in cause */
  uint8_t cause[QMI_CAT_CAUSE_MAX_LENGTH_V02];
  /**<   Cause (see \hyperref[S1]{[S1]},\n Section 8.26).*/
}cat_send_decoded_envelope_cmd_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_ENV_CALL_CONTROL_RESULT_TYPE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_ENVELOPE_CALL_CONTROL_RESULT_ALLOWED_NO_MOD_V02 = 0x00, 
  CAT_ENVELOPE_CALL_CONTROL_RESULT_NOT_ALLOWED_V02 = 0x01, 
  CAT_ENVELOPE_CALL_CONTROL_RESULT_ALLOWED_WITH_MOD_V02 = 0x02, 
  CAT_ENV_CALL_CONTROL_RESULT_TYPE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_env_call_control_result_type_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_BC_REPEAT_INDICATOR_TYPE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_BC_REPEAT_INDICATOR_ALTERNATE_MODE_V02 = 0x00, 
  CAT_BC_REPEAT_INDICATOR_SEQUENTIAL_MODE_V02 = 0x01, 
  CAT_BC_REPEAT_INDICATOR_TYPE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_bc_repeat_indicator_type_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Sends an envelope command in decoded format to the card. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */

  /* Optional */
  /*  Call Control Result */
  uint8_t cc_result_valid;  /**< Must be set to true if cc_result is being passed */
  cat_env_call_control_result_type_enum_v02 cc_result;
  /**<   Call control result: \n
       - 0x00 -- Call control result is allowed with no modification \n
       - 0x01 -- Call control result is not allowed \n
       - 0x02 -- Call control result is allowed with modification
  */

  /* Optional */
  /*  Address */
  uint8_t address_valid;  /**< Must be set to true if address is being passed */
  cat_address_type_v02 address;

  /* Optional */
  /*  Subaddress */
  uint8_t sub_address_valid;  /**< Must be set to true if sub_address is being passed */
  cat_subaddress_type_v02 sub_address;

  /* Optional */
  /*  Capability Configuration Parameter 1 */
  uint8_t capability_config_param1_valid;  /**< Must be set to true if capability_config_param1 is being passed */
  cat_capability_config_data_type_v02 capability_config_param1;

  /* Optional */
  /*  Capability Configuration Parameter 2 */
  uint8_t capability_config_param2_valid;  /**< Must be set to true if capability_config_param2 is being passed */
  cat_capability_config_data_type_v02 capability_config_param2;

  /* Optional */
  /*  USSD String */
  uint8_t ussd_string_valid;  /**< Must be set to true if ussd_string is being passed */
  cat_dcs_encoded_text_type_v02 ussd_string;

  /* Optional */
  /*  PDP Context Activation */
  uint8_t pdp_context_act_valid;  /**< Must be set to true if pdp_context_act is being passed */
  cat_pdp_context_act_type_v02 pdp_context_act;

  /* Optional */
  /*  EPS PDN Connect Activation */
  uint8_t eps_pdn_connect_act_valid;  /**< Must be set to true if eps_pdn_connect_act is being passed */
  cat_eps_pdn_connect_act_type_v02 eps_pdn_connect_act;

  /* Optional */
  /*  Alpha */
  uint8_t alpha_valid;  /**< Must be set to true if alpha is being passed */
  cat_dcs_encoded_text_type_v02 alpha;

  /* Optional */
  /*  BC Repeat Indicator */
  uint8_t bc_repeat_ind_valid;  /**< Must be set to true if bc_repeat_ind is being passed */
  cat_bc_repeat_indicator_type_enum_v02 bc_repeat_ind;
  /**<    Bearer capability repeat indicator: \n
         - 0x00 -- Alternate mode \n
         - 0x01 -- Sequential mode
   */

  /* Optional */
  /*  SMS-PP Data Download UICC Acknowledgment */
  uint8_t sms_pp_uicc_acknowledge_valid;  /**< Must be set to true if sms_pp_uicc_acknowledge is being passed */
  uint32_t sms_pp_uicc_acknowledge_len;  /**< Must be set to # of elements in sms_pp_uicc_acknowledge */
  uint8_t sms_pp_uicc_acknowledge[QMI_CAT_SMS_PP_UICC_ACK_MAX_LENGTH_V02];
  /**<   SMS-PP data download envelope response, as defined in \hyperref[S4]{[S4]},
       Section 7.1.
  */

  /* Optional */
  /*  RP Address */
  uint8_t rp_dest_address_valid;  /**< Must be set to true if rp_dest_address is being passed */
  cat_address_type_v02 rp_dest_address;

  /* Optional */
  /*  TP Address */
  uint8_t tp_dest_address_valid;  /**< Must be set to true if tp_dest_address is being passed */
  cat_address_type_v02 tp_dest_address;
}cat_send_decoded_envelope_cmd_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t confirm;
  /**<   User confirmed: \n
       - 0x00 -- No \n
       - 0x01 -- Yes
  */
}cat_user_confirm_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t display;
  /**<   Icon is displayed: \n
       - 0x00 -- No \n
       - 0x01 -- Yes
  */
}cat_user_icon_display_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Sends user and icon confirmation for network-related commands. */
typedef struct {

  /* Optional */
  /*  User Confirmed */
  uint8_t confirm_valid;  /**< Must be set to true if confirm is being passed */
  cat_user_confirm_type_v02 confirm;

  /* Optional */
  /*  Icon is Displayed */
  uint8_t display_valid;  /**< Must be set to true if display is being passed */
  cat_user_icon_display_type_v02 display;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_event_confirmation_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Sends user and icon confirmation for network-related commands. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */
}cat_event_confirmation_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_SCWS_CHANNEL_STATE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_SCWS_CHANNEL_CLOSED_STATE_V02 = 0x00, 
  CAT_SCWS_CHANNEL_LISTEN_STATE_V02 = 0x01, 
  CAT_SCWS_CHANNEL_ESTABLISHED_STATE_V02 = 0x02, 
  CAT_SCWS_CHANNEL_STATE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_scws_channel_state_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ch_id;
  /**<   Channel ID.*/

  cat_scws_channel_state_enum_v02 state;
  /**<   Channel state: \n
        - 0x00 -- Closed state \n
        - 0x01 -- Listen state \n
        - 0x02 -- Established state \n
        Other values are reserved for future use.
  */
}cat_scws_channel_status_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Sends the Open Channel indication to the Smart Card Web Server
    (SCWS) Agent. */
typedef struct {

  /* Mandatory */
  /*  Channel Status */
  cat_scws_channel_status_type_v02 channel_status;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_scws_open_channel_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Sends the Open Channel indication to the Smart Card Web Server
    (SCWS) Agent. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */
}cat_scws_open_channel_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ch_id;
  /**<   Channel ID to be used for the SCWS connection.*/

  uint16_t port;
  /**<   Port for the local TCP socket.*/

  uint16_t buffer_size;
  /**<   Buffer size to be used.*/
}cat_scws_open_channel_info_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Indication Message; Indicates that a new Open Channel is required for the SCWS. */
typedef struct {

  /* Optional */
  /*  Open Channel Information */
  uint8_t open_channel_info_valid;  /**< Must be set to true if open_channel_info is being passed */
  cat_scws_open_channel_info_type_v02 open_channel_info;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;

  /* Optional */
  /*  Alpha */
  uint8_t alpha_valid;  /**< Must be set to true if alpha is being passed */
  cat_dcs_encoded_text_type_v02 alpha;
}cat_scws_open_channel_ind_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Sends the Close Channel indication to the SCWS Agent. */
typedef struct {

  /* Mandatory */
  /*  Channel Status */
  cat_scws_channel_status_type_v02 channel_status;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_scws_close_channel_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Sends the Close Channel indication to the SCWS Agent. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */
}cat_scws_close_channel_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ch_id;
  /**<   Channel ID to be used for the SCWS connection.*/

  cat_scws_channel_state_enum_v02 state;
  /**<   Channel state: \n
        - 0x00 -- Closed state; indicates that the socket must be closed \n
        - 0x01 -- Listen state; indicates that the client needs to be disconnected;
                 the socket remains open in the Listen state\n
        - 0x02 -- Established state \n
        Other values are reserved for future use.
  */
}cat_scws_close_channel_info_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Indication Message; Indicates that a new Close Channel is required for the SCWS. */
typedef struct {

  /* Optional */
  /*  Close Channel Information */
  uint8_t close_channel_info_valid;  /**< Must be set to true if close_channel_info is being passed */
  cat_scws_close_channel_info_type_v02 close_channel_info;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_scws_close_channel_ind_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ch_id;
  /**<   Channel ID*/

  uint8_t result;
  /**<   Result of the Send Data command: \n
        - 0x00 -- Failed \n
        - 0x01 -- Success
  */
}cat_scws_send_data_result_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Sends data to the SCWS Agent. */
typedef struct {

  /* Mandatory */
  /*  Channel Status */
  cat_scws_send_data_result_type_v02 result;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_scws_send_data_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Sends data to the SCWS Agent. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */
}cat_scws_send_data_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ch_id;
  /**<   Channel ID to be used to send the data.*/

  uint8_t total_packets;
  /**<   Total number of packets.*/

  uint8_t current_packet;
  /**<   Current packet.*/

  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[QMI_CAT_SCWS_DATA_MAX_LENGTH_V02];
  /**<   Data to be sent.*/
}cat_scws_send_data_info_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Indication Message; Indicates that the SCWS Agent must send data. */
typedef struct {

  /* Optional */
  /*  Send Data Information */
  uint8_t send_data_info_valid;  /**< Must be set to true if send_data_info is being passed */
  cat_scws_send_data_info_type_v02 send_data_info;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_scws_send_data_ind_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ch_id;
  /**<   Channel ID.*/

  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[QMI_CAT_SCWS_DATA_MAX_LENGTH_V02];
  /**<   Data that is received.*/
}cat_scws_data_available_type_v02;  /* Type */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Indicates that data is available. */
typedef struct {

  /* Mandatory */
  /*  Remaining Data */
  cat_scws_data_available_type_v02 result;

  /* Mandatory */
  /*  Length of the Remaining Data */
  uint16_t remaining_data_len;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_scws_data_available_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Indicates that data is available. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */
}cat_scws_data_available_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Informs the modem about a change in the channel state. */
typedef struct {

  /* Mandatory */
  /*  Channel Status */
  cat_scws_channel_status_type_v02 channel_status;

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_scws_channel_status_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Informs the modem about a change in the channel state. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */
}cat_scws_channel_status_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Retrieves the current modem terminal profile. */
typedef struct {

  /* Optional */
  /*  Slot */
  uint8_t slot_valid;  /**< Must be set to true if slot is being passed */
  cat_slot_type_v02 slot;
}cat_get_terminal_profile_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Retrieves the current modem terminal profile. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */

  /* Optional */
  /*  Raw Terminal Profile Data */
  uint8_t terminal_profile_data_valid;  /**< Must be set to true if terminal_profile_data is being passed */
  uint32_t terminal_profile_data_len;  /**< Must be set to # of elements in terminal_profile_data */
  uint8_t terminal_profile_data[QMI_CAT_TERMINAL_PROFILE_MAX_LENGTH_V02];
}cat_get_terminal_profile_resp_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_enums
    @{
  */
typedef enum {
  CAT_CONFIG_MODE_ENUM_MIN_ENUM_VAL_V02 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CAT_CONFIG_MODE_DISABLED_V02 = 0x00, 
  CAT_CONFIG_MODE_GOBI_V02 = 0x01, 
  CAT_CONFIG_MODE_ANDROID_V02 = 0x02, 
  CAT_CONFIG_MODE_DECODED_V02 = 0x03, 
  CAT_CONFIG_MODE_DECODED_PULLONLY_V02 = 0x04, 
  CAT_CONFIG_MODE_CUSTOM_RAW_V02 = 0x05, 
  CAT_CONFIG_MODE_CUSTOM_DECODED_V02 = 0x06, 
  CAT_CONFIG_MODE_ENUM_MAX_ENUM_VAL_V02 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}cat_config_mode_enum_v02;
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Request Message; Changes the configuration of the QMI_CAT service. */
typedef struct {

  /* Mandatory */
  /*  Configuration Mode */
  cat_config_mode_enum_v02 cat_config_mode;
  /**<   QMI_CAT configuration mode: \n
       - 0x00 -- Disabled mode \n
       - 0x01 -- Gobi mode \n
       - 0x02 -- Android mode \n
       - 0x03 -- Decoded mode \n
       - 0x04 -- Decoded Pull-only mode \n
       - 0x05 -- Custom Raw mode (allows a customizable terminal profile for raw format)\n
       - 0x06 -- Custom Decoded mode (allows a customizable terminal profile for decoded format)\n
       Other values are reserved for future use.
  */

  /* Optional */
  /*  Custom Terminal Profile Data */
  uint8_t custom_tp_valid;  /**< Must be set to true if custom_tp is being passed */
  uint32_t custom_tp_len;  /**< Must be set to # of elements in custom_tp */
  uint8_t custom_tp[QMI_CAT_TERMINAL_PROFILE_MAX_LENGTH_V02];
  /**<   Custom terminal profile, encoded as in \hyperref[S1]{[S1]}, Section 5.2. \n
       The first byte of the TP bitmask starts from custom_tp[0]. \n
       This TLV is used only for custom modes and ignored in all other cases.
  */
}cat_set_configuration_req_msg_v02;  /* Message */
/**
    @}
  */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Changes the configuration of the QMI_CAT service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */
}cat_set_configuration_resp_msg_v02;  /* Message */
/**
    @}
  */

/*
 * cat_get_configuration_req_msg is empty
 * typedef struct {
 * }cat_get_configuration_req_msg_v02;
 */

/** @addtogroup cat_qmi_messages
    @{
  */
/** Response Message; Gets the configuration of the QMI_CAT service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
       qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       qmi_error_type  - Error code. Possible error code values are described in
                         the error codes section of each message definition.
  */

  /* Optional */
  /*  Configuration Mode */
  uint8_t cat_config_mode_valid;  /**< Must be set to true if cat_config_mode is being passed */
  cat_config_mode_enum_v02 cat_config_mode;
  /**<   Current QMI_CAT configuration mode: \n
       - 0x00 -- Disabled mode \n
       - 0x01 -- Gobi mode \n
       - 0x02 -- Android mode \n
       - 0x03 -- Decoded mode \n
       - 0x04 -- Decoded Pull-only mode \n
       - 0x05 -- Custom Raw mode (allows a customizable terminal profile for raw format)\n
       - 0x06 -- Custom Decoded mode (allows a customizable terminal profile for decoded format)\n
       Other values are reserved for future use.
  */

  /* Optional */
  /*  Custom Terminal Profile Data */
  uint8_t custom_tp_valid;  /**< Must be set to true if custom_tp is being passed */
  uint32_t custom_tp_len;  /**< Must be set to # of elements in custom_tp */
  uint8_t custom_tp[QMI_CAT_TERMINAL_PROFILE_MAX_LENGTH_V02];
  /**<   Custom terminal profile, encoded as in \hyperref[S1]{[S1]}, Section 5.2. \n
       The first byte of the TP bitmask starts from custom_tp[0]. \n
       This TLV is used only for custom modes and ignored in all other cases
  */
}cat_get_configuration_resp_msg_v02;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup cat_qmi_msg_ids
    @{
  */
#define QMI_CAT_RESET_REQ_V02 0x0000
#define QMI_CAT_RESET_RESP_V02 0x0000
#define QMI_CAT_SET_EVENT_REPORT_REQ_V02 0x0001
#define QMI_CAT_SET_EVENT_REPORT_RESP_V02 0x0001
#define QMI_CAT_EVENT_REPORT_IND_V02 0x0001
#define QMI_CAT_GET_SUPPORTED_MSGS_REQ_V02 0x001E
#define QMI_CAT_GET_SUPPORTED_MSGS_RESP_V02 0x001E
#define QMI_CAT_GET_SUPPORTED_FIELDS_REQ_V02 0x001F
#define QMI_CAT_GET_SUPPORTED_FIELDS_RESP_V02 0x001F
#define QMI_CAT_GET_SERVICE_STATE_REQ_V02 0x0020
#define QMI_CAT_GET_SERVICE_STATE_RESP_V02 0x0020
#define QMI_CAT_SEND_TR_REQ_V02 0x0021
#define QMI_CAT_SEND_TR_RESP_V02 0x0021
#define QMI_CAT_SEND_ENVELOPE_CMD_REQ_V02 0x0022
#define QMI_CAT_SEND_EVENLOPE_CMD_RESP_V02 0x0022
#define QMI_CAT_GET_EVENT_REPORT_REQ_V02 0x0023
#define QMI_CAT_GET_EVENT_REPORT_RESP_V02 0x0023
#define QMI_CAT_SEND_DECODED_TR_REQ_V02 0x0024
#define QMI_CAT_SEND_DECODED_TR_RESP_V02 0x0024
#define QMI_CAT_SEND_DECODED_ENVELOPE_CMD_REQ_V02 0x0025
#define QMI_CAT_SEND_DECODED_ENVELOPE_CMD_RESP_V02 0x0025
#define QMI_CAT_EVENT_CONFIRMATION_REQ_V02 0x0026
#define QMI_CAT_EVENT_CONFIRMATION_RESP_V02 0x0026
#define QMI_CAT_SCWS_OPEN_CHANNEL_REQ_V02 0x0027
#define QMI_CAT_SCWS_OPEN_CHANNEL_RESP_V02 0x0027
#define QMI_CAT_SCWS_OPEN_CHANNEL_IND_V02 0x0027
#define QMI_CAT_SCWS_CLOSE_CHANNEL_REQ_V02 0x0028
#define QMI_CAT_SCWS_CLOSE_CHANNEL_RESP_V02 0x0028
#define QMI_CAT_SCWS_CLOSE_CHANNEL_IND_V02 0x0028
#define QMI_CAT_SCWS_SEND_DATA_REQ_V02 0x0029
#define QMI_CAT_SCWS_SEND_DATA_RESP_V02 0x0029
#define QMI_CAT_SCWS_SEND_DATA_IND_V02 0x0029
#define QMI_CAT_SCWS_DATA_AVAILABLE_REQ_V02 0x002A
#define QMI_CAT_SCWS_DATA_AVAILABLEA_RESP_V02 0x002A
#define QMI_CAT_SCWS_CHANNEL_STATUS_REQ_V02 0x002B
#define QMI_CAT_SCWS_CHANNEL_STATUS_RESP_V02 0x002B
#define QMI_CAT_GET_TERMINAL_PROFILE_REQ_V02 0x002C
#define QMI_CAT_GET_TERMINAL_PROFILE_RESP_V02 0x002C
#define QMI_CAT_SET_CONFIGURATION_REQ_V02 0x002D
#define QMI_CAT_SET_CONFIGURATION_RESP_V02 0x002D
#define QMI_CAT_GET_CONFIGURATION_REQ_V02 0x002E
#define QMI_CAT_GET_CONFIGURATION_RESP_V02 0x002E
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro cat_get_service_object_v02( ) that takes in no arguments. */
qmi_idl_service_object_type cat_get_service_object_internal_v02
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define cat_get_service_object_v02( ) \
          cat_get_service_object_internal_v02( \
            CAT_V02_IDL_MAJOR_VERS, CAT_V02_IDL_MINOR_VERS, \
            CAT_V02_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

