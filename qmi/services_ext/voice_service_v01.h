#ifndef VOICE_SERVICE_H
#define VOICE_SERVICE_H
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        V O I C E _ S E R V I C E _ V 0 1  . H

GENERAL DESCRIPTION
  This is the public header file which defines the voice service Data structures.

 Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.  
 Qualcomm Technologies Proprietary and Confidential.

 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*
 * This header file defines the types and structures that were defined in 
 * cat. It contains the constant values defined, enums, structures,
 * messages, and service message IDs (in that order) Structures that were 
 * defined in the IDL as messages contain mandatory elements, optional 
 * elements, a combination of mandatory and optional elements (mandatory 
 * always come before optionals in the structure), or nothing (null message)
 *  
 * An optional element in a message is preceded by a uint8_t value that must be
 * set to true if the element is going to be included. When decoding a received
 * message, the uint8_t values will be set to true or false by the decode
 * routine, and should be checked before accessing the values that they
 * correspond to. 
 *  
 * Variable sized arrays are defined as static sized arrays with an unsigned
 * integer (32 bit) preceding it that must be set to the number of elements
 * in the array that are valid. For Example:
 *  
 * uint32_t test_opaque_len;
 * uint8_t test_opaque[16];
 *  
 * If only 4 elements are added to test_opaque[] then test_opaque_len must be
 * set to 4 before sending the message.  When decoding, the _len value is set 
 * by the decode routine and should be checked so that the correct number of 
 * elements in the array will be accessed. 
 */

/* This file was generated with Tool version 02.01 
   It was generated on: Fri Oct 15 2010
   From IDL File: */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Version Number of the IDL used to generate this file */
#define VOICE_V01_IDL_MAJOR_VERS 01
#define VOICE_V01_IDL_MINOR_VERS 02
#define VOICE_V01_IDL_TOOL_VERS 02

/* Const Definitions */

#define VOICE_NUMBER_MAX_V01 255
#define VOICE_CALLER_ID_MAX_V01 255
#define VOICE_DISPLAY_BUFFER_MAX_V01 255
#define VOICE_CALLER_NAME_MAX_V01 255
#define VOICE_FLASH_PAYLOAD_MAX_V01 4096
#define VOICE_DIGIT_BUFFER_MAX_V01 255

typedef struct {
  /* Optional */
  /*  DTMF events */
  uint8_t reg_dtmf_events_valid;	/* Must be set to true if reg_dtmf_events is being passed */
  uint8_t reg_dtmf_events;	/*  0x00 - DISABLE
         0x01 - ENABLE
	 */

  /* Optional */
  /*  Voice privacy events */
  uint8_t reg_voice_privacy_events_valid;	/* Must be set to true if reg_voice_privacy_events is being passed */
  uint8_t reg_voice_privacy_events;	/*  0x00 - DISABLE
         0x01 - ENABLE
     */
}voice_indication_register_req_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;	/*  Standard response type.  */
}voice_indication_register_resp_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Calling number */
  char calling_number[VOICE_NUMBER_MAX_V01 + 1];	/*  Number to be dialed in ASCII string */

  /* Optional */
  /*  Call type */
  uint8_t call_type_valid;	/* Must be set to true if call_type is being passed */
  uint8_t call_type;	/*  Call type
         0x00 - VOICE (automatic selection)
         0x08 - NON_STD_OTASP
         0x09 - EMERGENCY
	 */
}voice_dial_call_req_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Unique call identifier for the dialed call */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;	/*  Standard response type.  */
}voice_dial_call_resp_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Unique call identifier for the call that needs to be ended */
}voice_end_call_req_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Unique call identifier for the call that needs to be ended */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;	/*  Standard response type.  */
}voice_end_call_resp_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Unique call identifier for the call that needs to be answered */
}voice_answer_call_req_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Unique call identifier for the call that needs to be answered */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;	/*  Standard response type.  */
}voice_answer_call_resp_msg_v01;	/* Message */

typedef struct {
  uint8_t call_id;	/*  Unique call identifier for the call */

  uint8_t call_event;	/*  0x00 - ORIGINATION - Phone originated a call
         0x01 - ANSWER - Incoming call was answered
         0x03 - END - Originated/incoming call was ended
         0x05 - INCOMING - Phone received an incoming call
         0x06 - CONNECT - Originated/incoming call was connected
	 */
}voice_call_status_type_v01;	/* Type */

typedef struct {
  /* Mandatory */
  /*  Call status */
  voice_call_status_type_v01 call_status;

  /* Optional */
  /*  Call end reason */
  uint8_t call_end_reason_valid;	/* Must be set to true if call_end_reason is being passed */
  uint16_t call_end_reason;	/*  The list of valid voice-related call end reasons can be 
	     found in Appendix A.1
	 */

  /* Optional */
  /*  Call type */
  uint8_t call_type_valid;	/* Must be set to true if call_type is being passed */
  uint8_t call_type;	/*  0x00 - VOICE
         0x06 - OTAPA
         0x07 - STD_OTASP
         0x08 - NON_STD_OTASP
         0x09 - EMERGENCY
	 */
}voice_call_status_ind_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Call identifier for the call to be queried for information */
}voice_get_call_info_req_msg_v01;	/* Message */

typedef struct {
  uint8_t call_id;	/*  Call identifier for the call queried for information */

  /*  Call state */
  uint8_t call_state;	/*  0x01 - ORIG - Call is in Origination state
         0x02 - INCOMING - Call is in Alerting state
         0x03 - CONVERSATION - Call is in Conversation state
         0x04 - CC_IN_PROGRESS - Call is originating but waiting for call control to complete
	 */

  /*  Call type */
  uint8_t call_type;	/*  0x00 - VOICE
         0x06 - OTAPA
         0x07 - STD_OTASP
         0x08 - NON_STD_OTASP
         0x09 - EMERGENCY
	 */

  uint8_t direction;	/*  0x01 - MO - Mobile-originated call
         0x02 - MT - Mobile-terminated call
	 */

  uint8_t mode;	/*  0x01 - CDMA */
}voice_call_information_type_v01;	/* Type */

typedef struct {
  uint8_t pi;	/*  Presentation indicator - See [S1, Table 2.7.4.4-1]
         for valid values of presentation indicator
	 */

  char number[VOICE_NUMBER_MAX_V01 + 1];	/*  Number in ASCII string */
}voice_number_type_v01;	/* Type */

typedef struct {
  /* Mandatory */
  /*  Call information */
  voice_call_information_type_v01 call_information;

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;	/*  Standard response type.  */

  /* Optional */
  /*  Number */
  uint8_t number_valid;	/* Must be set to true if number is being passed */
  voice_number_type_v01 number;

  /* Optional */
  /*  Service option */
  uint8_t srv_opt_valid;	/* Must be set to true if srv_opt is being passed */
  uint16_t srv_opt;	/*  Service option - See [S2, Table 3.1-1] for standard
         service option number assignments
	 */

  /* Optional */
  /*  Voice privacy */
  uint8_t voice_privacy_valid;	/* Must be set to true if voice_privacy is being passed */
  uint8_t voice_privacy;	/*  0x00 - STANDARD - Standard privacy
         0x01 - ENHANCED - Enhanced privacy
	 */

  /* Optional */
  /*  OTASP status */
  uint8_t otasp_status_valid;	/* Must be set to true if otasp_status is being passed */
  uint8_t otasp_status;	/*  OTASP status for OTASP call
         0x00 - SPL_UNLOCKED - SPL unlocked; only for user-initiated OTASP
         0x01 - SPC_RETRIES_EXCEEDED - SPC retries exceeded; only for user-initiated OTASP
         0x02 - AKEY_EXCHANGED - A-key exchanged; only for user-initiated OTASP
         0x03 - SSD_UPDATED - SSD updated; for both user-initiated OTASP and network-initiated OTASP (OTAPA)
         0x04 - NAM_DOWNLOADED - NAM downloaded; only for user-initiated OTASP
         0x05 - MDN_DOWNLOADED - MDN downloaded; only for user-initiated OTASP
         0x06 - IMSI_DOWNLOADED - IMSI downloaded; only for user-initiated OTASP
         0x07 - PRL_DOWNLOADED - PRL downloaded; only for user-initiated OTASP
         0x08 - COMMITTED - Commit successful; only for user-initiated OTASP
         0x09 - OTAPA_STARTED - OTAPA started; only for network-initiated OTASP (OTAPA)
         0x0A - OTAPA_STOPPED - OTAPA stopped; only for network-initiated OTASP (OTAPA)
         0x0B - OTAPA_ABORTED - OTAPA aborted; only for network-initiated OTASP (OTAPA)
	 */
}voice_get_call_info_resp_msg_v01;	/* Message */

typedef struct {
  uint8_t call_id;	/*  Call identifier for the call */

  uint8_t otasp_status;	/*  OTASP status for OTASP call
         0x00 - SPL_UNLOCKED - SPL unlocked; only for user-initiated OTASP
         0x01 - SPC_RETRIES_EXCEEDED - SPC retries exceeded; only for user-initiated OTASP
         0x02 - AKEY_EXCHANGED - A-key exchanged; only for user-initiated OTASP
         0x03 - SSD_UPDATED - SSD updated; for both user-initiated OTASP and network-initiated OTASP (OTAPA)
         0x04 - NAM_DOWNLOADED - NAM downloaded; only for user-initiated OTASP
         0x05 - MDN_DOWNLOADED - MDN downloaded; only for user-initiated OTASP
         0x06 - IMSI_DOWNLOADED - IMSI downloaded; only for user-initiated OTASP
         0x07 - PRL_DOWNLOADED - PRL downloaded; only for user-initiated OTASP
         0x08 - COMMITTED - Commit successful; only for user-initiated OTASP
         0x09 - OTAPA_STARTED - OTAPA started; only for network-initiated OTASP (OTAPA)
         0x0A - OTAPA_STOPPED - OTAPA stopped; only for network-initiated OTASP (OTAPA)
         0x0B - OTAPA_ABORTED - OTAPA aborted; only for network-initiated OTASP (OTAPA)
	 */
}voice_otasp_status_information_type_v01;	/* Type */

typedef struct {
  /* Mandatory */
  /*  OTASP status information */
  voice_otasp_status_information_type_v01 otasp_status_information;
}voice_otasp_status_ind_msg_v01;	/* Message */

typedef struct {
  uint8_t signal_type;	/*  Signal type - Refer to [S1, Table 3.7.5.5-1] for
         valid signal type values
	 */

  uint8_t alert_pitch;	/*  Alert pitch - Refer [S1, Table 3.7.5.5-2] for
         valid alert pitch values
	 */

  uint8_t signal;	/*  Signal Tone - Refer to [S1, Tables 3.7.5.5-3,
         3.7.5.5-4, and 3.7.5.5-5] for valid signal tones
	 */
}voice_signal_information_type_v01;	/* Type */

typedef struct {
  uint8_t pi;	/*  Presentation indicator - Refer to [S1, Table
         2.7.4.4-1] for valid values of presentation
         indicator
	 */

  char caller_id[VOICE_CALLER_ID_MAX_V01 + 1];	/*  Caller ID in ASCII string */
}voice_caller_id_information_type_v01;	/* Type */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Call identifier for the call */

  /* Optional */
  /*  Signal information */
  uint8_t signal_information_valid;	/* Must be set to true if signal_information is being passed */
  voice_signal_information_type_v01 signal_information;

  /* Optional */
  /*  Caller ID information */
  uint8_t caller_id_information_valid;	/* Must be set to true if caller_id_information is being passed */
  voice_caller_id_information_type_v01 caller_id_information;

  /* Optional */
  /*  Display information */
  uint8_t display_buffer_valid;	/* Must be set to true if display_buffer is being passed */
  char display_buffer[VOICE_DISPLAY_BUFFER_MAX_V01 + 1];	/*  Display buffer containing the display ASCII string. */

  /* Optional */
  /*  Extended display information */
  uint8_t ext_display_buffer_valid;	/* Must be set to true if ext_display_buffer is being passed */
  char ext_display_buffer[VOICE_DISPLAY_BUFFER_MAX_V01 + 1];	/*  Extended display buffer containing the display
         text - Refer to [S1, Section 3.7.5.16] for the
         format information of the buffer contents
	 */

  /* Optional */
  /*  Caller name information */
  uint8_t caller_name_valid;	/* Must be set to true if caller_name is being passed */
  char caller_name[VOICE_CALLER_NAME_MAX_V01 + 1];	/*  Caller name in ASCII string */

  /* Optional */
  /*  Call waiting indicator */
  uint8_t call_waiting_valid;	/* Must be set to true if call_waiting is being passed */
  uint8_t call_waiting;	/*  0x01 - New call waiting */
}voice_info_rec_ind_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Call ID associated with current call */

  /* Optional */
  /*  Flash payload */
  uint8_t flash_payload_valid;	/* Must be set to true if flash_payload is being passed */
  char flash_payload[VOICE_FLASH_PAYLOAD_MAX_V01 + 1];	/*  Payload in ASCII to be sent in Flash */
}voice_send_flash_req_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Call ID associated with the current call */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;	/*  Standard response type.  */
}voice_send_flash_resp_msg_v01;	/* Message */

typedef struct {
  uint8_t call_id;	/*  Call ID associated with current call */

  char digit_buffer[VOICE_DIGIT_BUFFER_MAX_V01 + 1];	/*  DTMF digit buffer in ASCII string */
}voice_burst_dtmf_information_type_v01;	/* Type */

typedef struct {
  /* Mandatory */
  /*  Burst dtfm information */
  voice_burst_dtmf_information_type_v01 burst_dtmf_information;
}voice_burst_dtmf_req_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Call ID associated with the current call */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;	/*  Standard response type.  */
}voice_burst_dtmf_resp_msg_v01;	/* Message */

typedef struct {
  uint8_t call_id;	/*  Call ID associated with the current call */

  uint8_t digit;	/*  DTMF digit in ASCII */
}voice_cont_dtmf_information_type_v01;	/* Type */

typedef struct {
  /* Mandatory */
  /*  Continuous DTMF Information */
  voice_cont_dtmf_information_type_v01 cont_dtmf_information;
}voice_start_cont_dtmf_req_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Call ID associated with the current call */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;	/*  Standard response type.  */
}voice_start_cont_dtmf_resp_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Continuous DTMF Information */
  voice_cont_dtmf_information_type_v01 cont_dtmf_information;
}voice_stop_cont_dtmf_req_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Call ID */
  uint8_t call_id;	/*  Call ID associated with the current call */

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;	/*  Standard response type.  */
}voice_stop_cont_dtmf_resp_msg_v01;	/* Message */

typedef struct {
  uint8_t call_id;	/*  Call identifier for the current call */

  uint8_t dtmf_event;	/*  DTMF event
       0x00 - REV_BURST_DTMF - Sends a CDMAburst DTMF
       0x01 - REV_START_CONT_DTMF - Starts a continuous DTMF tone
       0x03 - REV_STOP_CONT_DTMF - Stops a continuous DTMF tone
       0x05 - FWD_BURST_DTMF - Received a CDMA-burst DTMF message
       0x06 - FWD_START_CONT_DTMF - Received a start-continuous DTMF tone order
       0x07 - FWD_STOP_CONT_DTMF - Received a stop-continuous DTMF tone order
   */

  char digit_buffer[VOICE_DIGIT_BUFFER_MAX_V01 + 1];	/*  DTMF digit buffer in ASCII string */
}voice_dtmf_information_type_v01;	/* Type */

typedef struct {
  /* Mandatory */
  /*  DTMF information */
  voice_dtmf_information_type_v01 dtmf_information;

  /* Optional */
  /*  DTMF pulse width */
  uint8_t on_length_valid;	/* Must be set to true if on_length is being passed */
  uint8_t on_length;	/*  DTMF pulse width - See Refer to [S1, Table
       2.7.2.3.2.7-1] for the recommended DTMF pulse
       width
   */

  /* Optional */
  /*  DTMF Inter-digit Interval */
  uint8_t off_length_valid;	/* Must be set to true if off_length is being passed */
  uint8_t off_length;	/*  DTMF inter-digit interval - See [S1, Table
       2.7.2.3.2.7-2] for the recommended minimum
       DTMF inter-digit interval
   */
}voice_dtmf_ind_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Voice privacy preference */
  uint8_t privacy_pref;	/*  0x00 - STANDARD - Standard privacy
       0x01 - ENHANCED - Enhanced privacy
   */
}voice_set_preferred_privacy_req_msg_v01;	/* Message */

typedef struct {
  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;	/*  Standard response type.  */
}voice_set_preferred_privacy_resp_msg_v01;	/* Message */

typedef struct {
  uint8_t call_id;	/*  Call identifier for the call */

  uint8_t voice_privacy;	/*  0x00 - STANDARD - Standard privacy
       0x01 - ENHANCED - Enhanced privacy
   */
}voice_privacy_information_type_v01;	/* Type */

typedef struct {
  /* Mandatory */
  /*  Voice privacy information */
  voice_privacy_information_type_v01 voice_privacy_information;
}voice_privacy_ind_msg_v01;	/* Message */

/*Service Message Definition*/
#define QMI_VOICE_INDICATION_REGISTER_REQ_V01 0x0003
#define QMI_VOICE_INDICATION_REGISTER_RESP_V01 0x0003
#define QMI_VOICE_DIAL_CALL_REQ_V01 0x0020
#define QMI_VOICE_DIAL_CALL_RESP_V01 0x0020
#define QMI_VOICE_END_CALL_REQ_V01 0x0021
#define QMI_VOICE_END_CALL_RESP_V01 0x0021
#define QMI_VOICE_ANSWER_CALL_REQ_V01 0x0022
#define QMI_VOICE_ANSWER_CALL_RESP_V01 0x0022
#define QMI_VOICE_CALL_STATUS_IND_V01 0x0023
#define QMI_VOICE_GET_CALL_INFO_REQ_V01 0x0024
#define QMI_VOICE_GET_CALL_INFO_RESP_V01 0x0024
#define QMI_VOICE_OTASP_STATUS_IND_V01 0x0025
#define QMI_VOICE_INFO_REC_IND_V01 0x0026
#define QMI_VOICE_SEND_FLASH_REQ_V01 0x0027
#define QMI_VOICE_SEND_FLASH_RESP_V01 0x0027
#define QMI_VOICE_BURST_DTMF_REQ_V01 0x0028
#define QMI_VOICE_BURST_DTMF_RESP_V01 0x0028
#define QMI_VOICE_START_CONT_DTMF_REQ_V01 0x0029
#define QMI_VOICE_START_CONT_DTMF_RESP_V01 0x0029
#define QMI_VOICE_STOP_CONT_DTMF_REQ_V01 0x002A
#define QMI_VOICE_STOP_CONT_DTMF_RESP_V01 0x002A
#define QMI_VOICE_DTMF_IND_V01 0x002B
#define QMI_VOICE_SET_PREFERRED_PRIVACY_REQ_V01 0x002C
#define QMI_VOICE_SET_PREFERRED_PRIVACY_RESP_V01 0x002C
#define QMI_VOICE_PRIVACY_IND_V01 0x002D

/* Service Object Accessor */
qmi_idl_service_object_type voice_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
#define voice_get_service_object_v01( ) \
          voice_get_service_object_internal_v01( \
            VOICE_V01_IDL_MAJOR_VERS, VOICE_V01_IDL_MINOR_VERS, \
            VOICE_V01_IDL_TOOL_VERS )


#ifdef __cplusplus
}
#endif
#endif

