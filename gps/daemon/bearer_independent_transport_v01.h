#ifndef BIT_SERVICE_H
#define BIT_SERVICE_H
/**
  @file bearer_independent_transport_v01.h

  @brief This is the public header file which defines the bit service Data structures.

  This header file defines the types and structures that were defined in
  bit. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/interfaces/qmi/bit/main/latest/api/bearer_independent_transport_v01.h#5 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.5
   It was generated on: Thu Aug 23 2012
   From IDL File: bearer_independent_transport_v01.idl */

/** @defgroup bit_qmi_consts Constant values defined in the IDL */
/** @defgroup bit_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup bit_qmi_enums Enumerated types used in QMI messages */
/** @defgroup bit_qmi_messages Structures sent as QMI messages */
/** @defgroup bit_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup bit_qmi_accessor Accessor for QMI service object */
/** @defgroup bit_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup bit_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define BIT_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define BIT_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define BIT_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define BIT_V01_MAX_MESSAGE_ID 0x0031;
/**
    @}
  */


/** @addtogroup bit_qmi_consts
    @{
  */

/**  Max Length of the URL  */
#define BIT_CONST_URL_LEN_MAX_V01 254

/**  Max payload that can be sent and received over QMI  */
#define BIT_CONST_PAYLOAD_LEN_MAX_V01 2048
/**
    @}
  */

/** @addtogroup bit_qmi_enums
    @{
  */
typedef enum {
  BIT_PROTOCOL_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  BIT_ENUM_PROTOCOL_ANY_V01 = 0, /**<  Protocol Payload: Any  */
  BIT_ENUM_PROTOCOL_AGPS_SUPL_V01 = 1, /**<  Protocol Payload: Assisted GPS SUPL
       Ref: OMA-TS-ULP-V2_0-20120417-A  */
  BIT_ENUM_PROTOCOL_AGPS_IS801_V01 = 2, /**<  Protocol Payload: Assisged GPS IS801
       Ref: Position Determination Service Standard for Dual Mode Spread Spectrum Systems – 3GPP2 C.S0022-0  */
  BIT_ENUM_PROTOCOL_AGPS_V1_V01 = 3, /**<  Protocol Payload: Assisted GPS V1
       This protocol is now Deprecated  */
  BIT_ENUM_PROTOCOL_AGPS_V2_V01 = 4, /**<  Protocol Payload: Assisted GPS V2
       Refer: Agile 80-V6410-2 F  */
  BIT_ENUM_PROTOCOL_JCDMA_V01 = 5, /**<  Protocol Payload: JCDMA */
  BIT_ENUM_PROTOCOL_XTRA_T_V01 = 6, /**<  Protocol Payload: XTRA_T  */
  BIT_PROTOCOL_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}bit_protocol_enum_type_v01;
/**
    @}
  */

/** @addtogroup bit_qmi_enums
    @{
  */
typedef enum {
  BIT_LINK_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  BIT_ENUM_LINK_HTTP_V01 = 0, /**<  HTTP  */
  BIT_ENUM_LINK_TCP_V01 = 1, /**<  TCP  */
  BIT_ENUM_LINK_SMS_V01 = 2, /**<  SMS  */
  BIT_ENUM_LINK_UDP_V01 = 3, /**<  UDP  */
  BIT_LINK_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}bit_link_enum_type_v01;
/**
    @}
  */

typedef uint64_t bit_host_info_mask_type_v01;
#define BIT_MASK_IPV4_ADDR_AND_PORT_V01 ((bit_host_info_mask_type_v01)0x01ull) /**<  IP V4 Address and port is Present  */
#define BIT_MASK_IPV6_ADDR_AND_PORT_V01 ((bit_host_info_mask_type_v01)0x02ull) /**<  IP V6 Address and port is Present  */
#define BIT_MASK_HOST_URL_AND_PORT_V01 ((bit_host_info_mask_type_v01)0x04ull) /**<  URL and port is present   */
/** @addtogroup bit_qmi_aggregates
    @{
  */
typedef struct {

  bit_host_info_mask_type_v01 validity_mask;
  /**<   Indicates which of the fields are valid. Valid Masks
      - BIT_MASK_IPV4_ADDR_AND_PORT (0x01) --  IP V4 Address and port is Present
      - BIT_MASK_IPV6_ADDR_AND_PORT (0x02) --  IP V6 Address and port is Present
      - BIT_MASK_HOST_URL_AND_PORT (0x04) --  URL and port is present
 */

  uint32_t ipv4_addr;
  /**<   IP V4 Address, if validity Mask
       indicates that host info is IPv4 */

  uint16_t ipv4_port;
  /**<   IPV4 Port, if validity Mask
       indicates that host info is IPv4 */

  uint8_t ipv6_addr[16];
  /**<   IP V6 Address, if validity Mask
       indicates that host info is IPv6 */

  uint16_t ipv6_port;
  /**<   IPV6 Port, if validity Mask
       indicates that host info is IPv6 */

  char url[BIT_CONST_URL_LEN_MAX_V01 + 1];
  /**<   URL of the host if validity Mask
       indicates that the URL in host info is valid
       Max Bytes: 255 (including the Null Terminator) */

  uint16_t url_port;
  /**<   Port, if validity Mask
       indicates that the URL in host info is valid */
}bit_host_info_struct_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Acknowledgment Message; Generic acknowledgement sent by BIT-client to acknowledge
                    the receipt of message from the BIT-Service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}bit_ack_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Response Message; Generic response sent by BIT-Service to acknowledge
                    the receipt of request from the BIT-Client */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
       Response for the Request operation. QMI_ERR_SUCCESS indicates that the
       BIT-Service has accepted the request and will send the status later, or
       other error codes can be used to indicate that the request has not been
       accepted.
  */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result. */
}bit_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Response Message; Generic response sent by BIT-Service to acknowledge
                    the receipt of request from the BIT-Client */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
       Response for the Request operation. QMI_ERR_SUCCESS indicates that the
       BIT-Service has accepted the request and will send the status later, or
       other error codes can be used to indicate that the request has not been
       accepted.
  */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result. */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<   Session handle data passed in the Request by the BIT Client  */
}bit_session_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Request Message; Inform BIT service that the client will start using its
           service. */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the transaction.
       This transaction ID needs to be echoed back along with the result */
}bit_open_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Indication Message; Informs the client with the final result of the open request. */
typedef struct {

  /* Mandatory */
  /*  Result of Open Request */
  qmi_response_type_v01 status;
  /**<   Status of the request. */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   A unique identifier is allocated by the BIT-Client
       to identify this transaction. This is provided in the bit_open_req_msg.
       This transaction ID needs to be echoed back along with the result. */
}bit_open_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Request Message; Informs the client with the final result of the close request. */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */
}bit_close_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Indication Message; Inform BIT service that the client will stop using its
           service. */
typedef struct {

  /* Mandatory */
  /*  Result of close Request */
  qmi_response_type_v01 status;
  /**<   Status of the request. */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result. */
}bit_close_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Request Message; Request to BIT service to open a data connection.  */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Link Type */
  bit_link_enum_type_v01 link;
  /**<   Type of data connection that needs to be brought up.
 Valid Values:
      - BIT_ENUM_LINK_HTTP (0) --  HTTP
      - BIT_ENUM_LINK_TCP (1) --  TCP
      - BIT_ENUM_LINK_SMS (2) --  SMS
      - BIT_ENUM_LINK_UDP (3) --  UDP
 */

  /* Optional */
  /*  Protocol Type */
  uint8_t protocol_valid;  /**< Must be set to true if protocol is being passed */
  bit_protocol_enum_type_v01 protocol;
  /**<   Protocol Data Units carried as payload. Valid Values:
      - BIT_ENUM_PROTOCOL_ANY (0) --  Protocol Payload: Any
      - BIT_ENUM_PROTOCOL_AGPS_SUPL (1) --  Protocol Payload: Assisted GPS SUPL
       Ref: OMA-TS-ULP-V2_0-20120417-A
      - BIT_ENUM_PROTOCOL_AGPS_IS801 (2) --  Protocol Payload: Assisged GPS IS801
       Ref: Position Determination Service Standard for Dual Mode Spread Spectrum Systems – 3GPP2 C.S0022-0
      - BIT_ENUM_PROTOCOL_AGPS_V1 (3) --  Protocol Payload: Assisted GPS V1
       This protocol is now Deprecated
      - BIT_ENUM_PROTOCOL_AGPS_V2 (4) --  Protocol Payload: Assisted GPS V2
       Refer: Agile 80-V6410-2 F
      - BIT_ENUM_PROTOCOL_JCDMA (5) --  Protocol Payload: JCDMA
      - BIT_ENUM_PROTOCOL_XTRA_T (6) --  Protocol Payload: XTRA_T
 Default : BIT_ENUM_PROTOCOL_ANY */

  /* Optional */
  /*  Host Information */
  uint8_t host_info_valid;  /**< Must be set to true if host_info is being passed */
  bit_host_info_struct_type_v01 host_info;
  /**<   Information of the host to initiate a connection with. If this TLV
       is not provided the defaults provisioned in the service will be
       used */
}bit_connect_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Indication Message; Informs the client with the final result of the connect request. */
typedef struct {

  /* Mandatory */
  /*  Result of close Request */
  qmi_response_type_v01 status;
  /**<   Status of the request. */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Optional */
  /*  Session Handle */
  uint8_t session_handle_valid;  /**< Must be set to true if session_handle is being passed */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session. If the status is SUCCESS then this optional field
        must be provided. The session handle remains valid until the
        BIT-client disconnects or the BIT-Service terminates service
        to the BIT-client */
}bit_connect_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Request Message; Request BIT service to tear down the existing data-connection. */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session */
}bit_disconnect_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Indication Message; Informs the client with the final result of the disconnect request. */
typedef struct {

  /* Mandatory */
  /*  Result of close Request */
  qmi_response_type_v01 status;
  /**<   Status of the request. */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session */
}bit_disconnect_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Request Message; Request BIT service to send data to the network on the existing
             connection */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session */

  /* Mandatory */
  /*  Data to be sent to network */
  uint32_t payload_len;  /**< Must be set to # of elements in payload */
  uint8_t payload[BIT_CONST_PAYLOAD_LEN_MAX_V01];
  /**<   Payload that needs to be sent to the network.
       Max Length: 2048 */
}bit_send_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Indication Message; Informs the client with the final result of the send Request */
typedef struct {

  /* Mandatory */
  /*  Result of close Request */
  qmi_response_type_v01 status;
  /**<   Status of the request. */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<   Unique identifier allocated by the BIT Service to identify the
       session */

  /* Optional */
  /*  Bytes Sent */
  uint8_t bytes_sent_valid;  /**< Must be set to true if bytes_sent is being passed */
  uint32_t bytes_sent;
  /**<   If the status is QMI_RESULT_SUCCESS then this field this field should
       specify the number of bytes sent to the network */
}bit_send_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Request Message; Used to notify the BIT Service that the BIT-Client is ready
             to receive data */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session */

  /* Mandatory */
  /*  Ready to Receive */
  uint8_t rtr;
  /**<    Used to notify the BIT-Service if the Client is ready to receive data.
        Valid Values:\n
        - 0 -- Not Ready to receive data.
        - 1 -- Ready to receive data. */

  /* Optional */
  /*  Maximum payload that can be received */
  uint8_t max_recv_payload_size_valid;  /**< Must be set to true if max_recv_payload_size is being passed */
  uint32_t max_recv_payload_size;
  /**<   The maximum data that the BIT-client can receive. The
       BIT-client receives data through the QMI_BIT_DATA_RECEIVED_IND,
       the payload should not exceed the size indicated in this TLV.

       The indicated maximum payload size is applicable to the next
       packet that is sent to the BIT-client.

       This value shall be less than or equal to BIT_CONST_PAYLOAD_LEN_MAX which
       is the maximum payload that can be sent in the QMI_BIT_DATA_RECEIVED_IND.

       Default: BIT_CONST_PAYLOAD_LEN_MAX */
}bit_ready_to_receive_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Indication Message; Message sent from the BIT Service with the data for the
             BIT-Client */
typedef struct {

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session */

  /* Mandatory */
  /*  Sequence Number */
  uint64_t seq_num;
  /**<   Sequence number of the packet that is being sent to the BIT-client.
       The sequence number needs to start from 1. Out of order sequence numbers
       will not be accepted. For a given session handle this number must
       increase monotonically, if a sequence number is not accepted by the
       BIT-client, the BIT-Service needs to send the same sequence number
       again. It is advised that the BIT-Service sends packets after it
       receives an acknowledgement for the previous packet.
       NOTE: Its expected that the sequence number is monotonically increasing.
             The sequence number is associated with a session handle. When the
             session handle is released it is expected that the sequence number
             restarts again from 1. */

  /* Mandatory */
  /*  Payload received from network */
  uint32_t payload_len;  /**< Must be set to # of elements in payload */
  uint8_t payload[BIT_CONST_PAYLOAD_LEN_MAX_V01];
  /**<   Payload received from the network.
       Max Length: 2048         */
}bit_data_received_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Request Message; Used to notify the status of the last data received. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the transaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session */

  /* Mandatory */
  /*  Sequence Number */
  uint64_t seq_num;
  /**<   Sequence number of the packet for which the status is being sent to the
       BIT-Service. */

  /* Optional */
  uint8_t max_recv_payload_size_valid;  /**< Must be set to true if max_recv_payload_size is being passed */
  uint32_t max_recv_payload_size;
  /**<   If response is QMI_ERR_SUCCESS then this TLV will be sent, and this TLV
       indicates the maximum payload length that the BIT-Service can send
       in the next QMI_BIT_DATA_RECEIVED_IND to the BIT-Client.

       This value shall be less than or equal to BIT_CONST_PAYLOAD_LEN_MAX which
       is the maximum payload that can be sent in the QMI_BIT_DATA_RECEIVED_IND.

       Default: BIT_CONST_PAYLOAD_LEN_MAX
       */
}bit_data_received_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Request Message; Used to notify the service that the modem can now be forced to
             dormancy.  */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session */

  /* Mandatory */
  /*  Dormancy State */
  uint8_t dormancy_state;
  /**<   The dormancy configuration. Valid Values:
       - 0 -- Dormant
       - 1 -- Active */
}bit_set_dormancy_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Indication Message; Status indication from the BIT Service for the Set Dormancy
             Config Request */
typedef struct {

  /* Mandatory */
  /*  Result of close Request */
  qmi_response_type_v01 status;
  /**<   Status of the request. */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session */
}bit_set_dormancy_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Request Message; Request the BIT-Service to send the local IP Address */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session */
}bit_get_local_host_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup bit_qmi_messages
    @{
  */
/** Indication Message; Status indication from the BIT Service for the get locatl host
             info request */
typedef struct {

  /* Mandatory */
  /*  Result of close Request */
  qmi_response_type_v01 status;
  /**<   Status of the request. */

  /* Mandatory */
  /*  Transaction ID */
  uint32_t transaction_id;
  /**<   Unique identifier allocated by the BIT-Client for the trasaction.
       This transaction ID needs to be echoed back along with the result */

  /* Mandatory */
  /*  Session Handle */
  uint64_t session_handle;
  /**<    Unique identifier allocated by the BIT Service to identify the
        session */

  /* Optional */
  /*  Local-Host information */
  uint8_t local_host_info_valid;  /**< Must be set to true if local_host_info is being passed */
  bit_host_info_struct_type_v01 local_host_info;
  /**<    Local Host information that is available */
}bit_get_local_host_info_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/*
 * bit_service_ready_ind_msg is empty
 * typedef struct {
 * }bit_service_ready_ind_msg_v01;
 */

/*Service Message Definition*/
/** @addtogroup bit_qmi_msg_ids
    @{
  */
#define QMI_BIT_OPEN_RESP_V01 0x0020
#define QMI_BIT_OPEN_ACK_V01 0x0020
#define QMI_BIT_OPEN_REQ_V01 0x0020
#define QMI_BIT_OPEN_STATUS_IND_V01 0x0021
#define QMI_BIT_OPEN_STATUS_ACK_V01 0x0021
#define QMI_BIT_CLOSE_RESP_V01 0x0022
#define QMI_BIT_CLOSE_ACK_V01 0x0022
#define QMI_BIT_CLOSE_REQ_V01 0x0022
#define QMI_BIT_CLOSE_STATUS_IND_V01 0x0023
#define QMI_BIT_CLOSE_STATUS_ACK_V01 0x0023
#define QMI_BIT_CONNECT_RESP_V01 0x0024
#define QMI_BIT_CONNECT_ACK_V01 0x0024
#define QMI_BIT_CONNECT_REQ_V01 0x0024
#define QMI_BIT_CONNECT_STATUS_IND_V01 0x0025
#define QMI_BIT_CONNECT_STATUS_ACK_V01 0x0025
#define QMI_BIT_DISCONNECT_RESP_V01 0x0026
#define QMI_BIT_DISCONNECT_ACK_V01 0x0026
#define QMI_BIT_DISCONNECT_REQ_V01 0x0026
#define QMI_BIT_DISCONNECT_STATUS_IND_V01 0x0027
#define QMI_BIT_DISCONNECT_STATUS_ACK_V01 0x0027
#define QMI_BIT_SEND_RESP_V01 0x0028
#define QMI_BIT_SEND_ACK_V01 0x0028
#define QMI_BIT_SEND_REQ_V01 0x0028
#define QMI_BIT_SEND_STATUS_IND_V01 0x0029
#define QMI_BIT_SEND_STATUS_ACK_V01 0x0029
#define QMI_BIT_READY_TO_RECEIVE_RESP_V01 0x002A
#define QMI_BIT_READY_TO_RECEIVE_ACK_V01 0x002A
#define QMI_BIT_READY_TO_RECEIVE_REQ_V01 0x002A
#define QMI_BIT_DATA_RECEIVED_IND_V01 0x002B
#define QMI_BIT_DATA_RECEIVED_ACK_V01 0x002B
#define QMI_BIT_DATA_RECEIVED_STATUS_RESP_V01 0x002C
#define QMI_BIT_DATA_RECEIVED_STATUS_ACK_V01 0x002C
#define QMI_BIT_DATA_RECEIVED_STATUS_REQ_V01 0x002C
#define QMI_BIT_SET_DORMANCY_RESP_V01 0x002D
#define QMI_BIT_SET_DORMANCY_ACK_V01 0x002D
#define QMI_BIT_SET_DORMANCY_REQ_V01 0x002D
#define QMI_BIT_SET_DORMANCY_STATUS_IND_V01 0x002E
#define QMI_BIT_SET_DORMANCY_STATUS_ACK_V01 0x002E
#define QMI_BIT_GET_LOCAL_HOST_INFO_RESP_V01 0x002F
#define QMI_BIT_GET_LOCAL_HOST_INFO_ACK_V01 0x002F
#define QMI_BIT_GET_LOCAL_HOST_INFO_REQ_V01 0x002F
#define QMI_BIT_GET_LOCAL_HOST_INFO_STATUS_IND_V01 0x0030
#define QMI_BIT_GET_LOCAL_HOST_INFO_STATUS_ACK_V01 0x0030
#define QMI_BIT_SERVICE_READY_IND_V01 0x0031
#define QMI_BIT_SERVICE_READY_ACK_V01 0x0031
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro bit_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type bit_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define bit_get_service_object_v01( ) \
          bit_get_service_object_internal_v01( \
            BIT_V01_IDL_MAJOR_VERS, BIT_V01_IDL_MINOR_VERS, \
            BIT_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

