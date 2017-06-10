#ifndef IMSS_SERVICE_01_H
#define IMSS_SERVICE_01_H
/**
  @file ip_multimedia_subsystem_settings_v01.h
  
  @brief This is the public header file which defines the imss service Data structures.

  This header file defines the types and structures that were defined in 
  imss. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/imss/main/latest/api/ip_multimedia_subsystem_settings_v01.h#23 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.3 
   It was generated on: Wed Jul 31 2013 (Spin 0)
   From IDL File: ip_multimedia_subsystem_settings_v01.idl */

/** @defgroup imss_qmi_consts Constant values defined in the IDL */
/** @defgroup imss_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup imss_qmi_enums Enumerated types used in QMI messages */
/** @defgroup imss_qmi_messages Structures sent as QMI messages */
/** @defgroup imss_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup imss_qmi_accessor Accessor for QMI service object */
/** @defgroup imss_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup imss_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define IMSS_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define IMSS_V01_IDL_MINOR_VERS 0x0F
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define IMSS_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define IMSS_V01_MAX_MESSAGE_ID 0x0049;
/** 
    @} 
  */


/** @addtogroup imss_qmi_consts 
    @{ 
  */
#define IMS_SETTINGS_STRING_LEN_MAX_V01 255
#define IMS_SETTINGS_CONFIG_PROXY_ROUTE_LEN_V01 255
#define IMS_SETTINGS_REG_CONFIG_USER_NAME_LEN_V01 127
#define IMS_SETTINGS_REG_CONFIG_PRIVATE_URI_LEN_V01 127
#define IMS_SETTINGS_REG_CONFIG_DISPLAY_NAME_LEN_V01 63
#define IMS_SETTINGS_REG_PDPD_PROFILE_NAME_LEN_V01 31
#define IMS_SETTINGS_RCS_FEATURE_TAG_LIST_LEN_V01 269
#define IMS_SETTINGS_PRESENCE_PUBLISH_ETAG_LEN_V01 127
#define IMS_SETTINGS_VOIP_AMR_MODE_STR_LEN_V01 31
#define IMS_SETTINGS_VOIP_AMR_WB_MODE_STR_LEN_V01 31
#define IMS_SETTINGS_POL_MGR_APN_NAME_STR_LEN_V01 49
#define IMS_SETTINGS_POL_MGR_RAT_APN_SIZE_V01 10
#define IMS_SETTINGS_POL_MGR_RAT_APN_FB_SIZE_V01 10

/**  Enumeration for all the IMS Settings service specific Response 
     messages */
#define IMS_SETTINGS_POL_MGR_APN_SIZE_V01 6
/**
    @}
  */

/** @addtogroup imss_qmi_enums
    @{
  */
typedef enum {
  IMS_SETTINGS_RSP_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_SETTINGS_MSG_NO_ERR_V01 = 0, /**<  No error  */
  IMS_SETTINGS_MSG_IMS_NOT_READY_V01 = 1, /**<  Service indication  */
  IMS_SETTINGS_MSG_FILE_NOT_AVAILABLE_V01 = 2, /**<  Settings file is not available  */
  IMS_SETTINGS_MSG_READ_FAILED_V01 = 3, /**<  Read failure  */
  IMS_SETTINGS_MSG_WRITE_FAILED_V01 = 4, /**<  Write failure  */
  IMS_SETTINGS_MSG_OTHER_INTERNAL_ERR_V01 = 5, /**<  Other settings internal error  */
  IMS_SETTINGS_RSP_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_settings_rsp_enum_v01;
/**
    @}
  */

/** @addtogroup imss_qmi_enums
    @{
  */
typedef enum {
  IMS_SETTINGS_SMS_FORMAT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_SETTINGS_SMS_FORMAT_3GPP2_V01 = 0, /**<  3GPP2      */
  IMS_SETTINGS_SMS_FORMAT_3GPP_V01 = 1, /**<  3GPP  */
  IMS_SETTINGS_SMS_FORMAT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_settings_sms_format_enum_v01;
/**
    @}
  */

/** @addtogroup imss_qmi_enums
    @{
  */
typedef enum {
  IMS_SETTINGS_H264_PROFILE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_SETTINGS_H264_PROFILE_BASELINE_V01 = 0x00, /**<  Baseline profile */
  IMS_SETTINGS_H264_PROFILE_MAIN_V01 = 0x01, /**<  Main profile */
  IMS_SETTINGS_H264_PROFILE_EXTENDED_V01 = 0x02, /**<  Extended profile */
  IMS_SETTINGS_H264_PROFILE_HIGH_V01 = 0x03, /**<  High profile */
  IMS_SETTINGS_H264_PROFILE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_settings_h264_profile_enum_v01;
/**
    @}
  */

/** @addtogroup imss_qmi_enums
    @{
  */
typedef enum {
  IMS_SETTINGS_H264_LEVEL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_SETTINGS_H264_LEVEL1_V01 = 0x00, /**<  Level 1 */
  IMS_SETTINGS_H264_LEVEL1B_V01 = 0x01, /**<  Level 1b */
  IMS_SETTINGS_H264_LEVEL11_V01 = 0x02, /**<  Level 1.1 */
  IMS_SETTINGS_H264_LEVEL12_V01 = 0x03, /**<  Level 1.2 */
  IMS_SETTINGS_H264_LEVEL13_V01 = 0x04, /**<  Level 1.3 */
  IMS_SETTINGS_H264_LEVEL2_V01 = 0x05, /**<  Level 2 */
  IMS_SETTINGS_H264_LEVEL21_V01 = 0x06, /**<  Level 2.1 */
  IMS_SETTINGS_H264_LEVEL22_V01 = 0x07, /**<  Level 2.2 */
  IMS_SETTINGS_H264_LEVEL3_V01 = 0x08, /**<  Level 3 */
  IMS_SETTINGS_H264_LEVEL31_V01 = 0x09, /**<  Level 3.1 */
  IMS_SETTINGS_H264_LEVEL32_V01 = 0x0A, /**<  Level 3.2 */
  IMS_SETTINGS_H264_LEVEL4_V01 = 0x0B, /**<  Level 4 */
  IMS_SETTINGS_H264_LEVEL41_V01 = 0x0C, /**<  Level 4.1 */
  IMS_SETTINGS_H264_LEVEL42_V01 = 0x0D, /**<  Level 4.2 */
  IMS_SETTINGS_H264_LEVEL5_V01 = 0x0E, /**<  Level 5 */
  IMS_SETTINGS_H264_LEVEL51_V01 = 0x0F, /**<  Level 5.1 */
  IMS_SETTINGS_H264_LEVEL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_settings_h264_level_enum_v01;
/**
    @}
  */

/** @addtogroup imss_qmi_enums
    @{
  */
typedef enum {
  IMS_VIDEO_RESOLUTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_SETTINGS_SQCIF_RESOLUTION_V01 = 0x00, /**<  SQCIF */
  IMS_SETTINGS_QCIF_RESOLUTION_V01 = 0x01, /**<  QCIF */
  IMS_SETTINGS_CIF_RESOLUTION_V01 = 0x02, /**<  CIF */
  IMS_SETTINGS_QQVGA_RESOLUTION_V01 = 0x03, /**<  QQVGA */
  IMS_SETTINGS_QVGA_RESOLUTION_V01 = 0x04, /**<  QVGA */
  IMS_SETTINGS_VGA_RESOLUTION_V01 = 0x05, /**<  VGA  */
  IMS_VIDEO_RESOLUTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_video_resolution_enum_v01;
/**
    @}
  */

/** @addtogroup imss_qmi_enums
    @{
  */
typedef enum {
  IMS_SETTINGS_VIDEO_CODEC_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_SETTINGS_CODEC_MPEG4_XVID_V01 = 0x00, /**<  XVID MPEG4 codec */
  IMS_SETTINGS_CODEC_MPEG4_ISO_V01 = 0x01, /**<  ISO MPEG4 codec */
  IMS_SETTINGS_CODEC_H263_V01 = 0x02, /**<  H.263 codec */
  IMS_SETTINGS_CODEC_H264_V01 = 0x03, /**<  H.264 codec */
  IMS_SETTINGS_VIDEO_CODEC_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_settings_video_codec_enum_v01;
/**
    @}
  */

/** @addtogroup imss_qmi_enums
    @{
  */
typedef enum {
  IMS_SETTINGS_CONFIG_AUTH_SCHEME_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_SETTINGS_CONFIG_AUTH_SCHEME_NONE_V01 = 0x00, /**<  AUTHENTICATION SCHEME NONE  */
  IMS_SETTINGS_CONFIG_AUTH_SCHEME_DIGEST_V01 = 0x01, /**<  AUTHENTICATION SCHEME DIGEST  */
  IMS_SETTINGS_CONFIG_AUTH_SCHEME_SAG_V01 = 0x02, /**<  AUTHENTICATION SCHEME TOKEN  */
  IMS_SETTINGS_CONFIG_AUTH_SCHEME_AKA_V01 = 0x03, /**<  AUTHENTICATION SCHEME AKA  */
  IMS_SETTINGS_CONFIG_AUTH_SCHEME_CAVE_V01 = 0x04, /**<  AUTHENTICATION SCHEME CAVE  */
  IMS_SETTINGS_CONFIG_AUTH_SCHEME_AKAV2_V01 = 0x05, /**<  AUTHENTICATION SCHEME AKAv2  */
  IMS_SETTINGS_CONFIG_AUTH_SCHEME_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_settings_config_auth_scheme_enum_v01;
/**
    @}
  */

/** @addtogroup imss_qmi_enums
    @{
  */
typedef enum {
  IMS_SETTINGS_CONFIG_INITIAL_AUTH_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_SETTINGS_CONFIG_INITIAL_AUTH_NONE_V01 = 0x00, /**<  AUTHORIZATION NONE  */
  IMS_SETTINGS_CONFIG_INITIAL_AUTH_AUTHORIZATION_V01 = 0x01, /**<  AUTHORIZATION  */
  IMS_SETTINGS_CONFIG_INITIAL_AUTH_PROXY_AUTHORIZATION_V01 = 0x02, /**<  PROXY AUTHORIZATION  */
  IMS_SETTINGS_CONFIG_INITIAL_AUTH_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_settings_config_initial_auth_type_enum_v01;
/**
    @}
  */

/** @addtogroup imss_qmi_enums
    @{
  */
typedef enum {
  IMS_SETTINGS_REGMGR_CONFIG_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_SETTINGS_REGMGR_CONFIG_IETF_V01 = 0x00, /**<  IETF Configuration mode */
  IMS_SETTINGS_REGMGR_CONFIG_EARLY_IMS_V01 = 0x01, /**<  Early IMS Configuration mode */
  IMS_SETTINGS_REGMGR_CONFIG_IMS_V01 = 0x02, /**<  IMS Configuration mode */
  IMS_SETTINGS_REGMGR_CONFIG_IMS_NO_IPSEC_V01 = 0x03, /**<  IMS NO IPSec Configuration mode */
  IMS_SETTINGS_REGMGR_CONFIG_IMS_NONE_V01 = 0x04, /**<  NO Configuration mode */
  IMS_SETTINGS_REGMGR_CONFIG_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_settings_regmgr_config_mode_enum_v01;
/**
    @}
  */

typedef uint64_t ims_settings_integ_algo_mask_v01;
#define IMS_SETTINGS_INTEG_ALGO_HMAC_SHA_1_96_V01 ((ims_settings_integ_algo_mask_v01)0x01ull) /**<  HMAC-SHA-1-96 algorithm is used for IPSec integrity  */
#define IMS_SETTINGS_INTEG_ALGO_HMAC_MD5_96_V01 ((ims_settings_integ_algo_mask_v01)0x02ull) /**<  HMAC-MD5-96 algorithm is used for IPSec integrity  */
typedef uint64_t ims_settings_encrypt_algo_mask_v01;
#define IMS_SETTINGS_ENCRYPT_ALGO_NULL_V01 ((ims_settings_encrypt_algo_mask_v01)0x01ull) /**<  NULL algorithm is used for IPSec encryption  */
#define IMS_SETTINGS_ENCRYPT_ALGO_AES_CBC_V01 ((ims_settings_encrypt_algo_mask_v01)0x02ull) /**<  AES-CBC algorithm is used for IPSec encryption  */
#define IMS_SETTINGS_ENCRYPT_ALGO_DES_EDE3_CBC_V01 ((ims_settings_encrypt_algo_mask_v01)0x04ull) /**<  DES-EDE3-CBC algorithm is used for IPSec encryption  */
typedef uint64_t ims_settings_service_type_mask_v01;
#define IMS_SETTINGS_SERVICE_TYPE_VOLTE_V01 ((ims_settings_service_type_mask_v01)0x01ull) /**<  Bit mask to indicate the VOLTE service.  */
#define IMS_SETTINGS_SERVICE_TYPE_VT_V01 ((ims_settings_service_type_mask_v01)0x02ull) /**<  Bit mask to indicate the Video Telephony service.  */
#define IMS_SETTINGS_SERVICE_TYPE_SMS_V01 ((ims_settings_service_type_mask_v01)0x04ull) /**<  Bit mask to indicate the SMS service.  */
#define IMS_SETTINGS_SERVICE_TYPE_IM_V01 ((ims_settings_service_type_mask_v01)0x08ull) /**<  Bit mask to indicate the Instant Messaging service.  */
#define IMS_SETTINGS_SERVICE_TYPE_VS_V01 ((ims_settings_service_type_mask_v01)0x10ull) /**<  Bit mask to indicate the Video Share service.  */
#define IMS_SETTINGS_SERVICE_TYPE_IS_V01 ((ims_settings_service_type_mask_v01)0x20ull) /**<  Bit mask to indicate the Image Share service.  */
#define IMS_SETTINGS_SERVICE_TYPE_MSRP_V01 ((ims_settings_service_type_mask_v01)0x40ull) /**<  Bit mask to indicate the MSRP service.  */
#define IMS_SETTINGS_SERVICE_TYPE_GL_V01 ((ims_settings_service_type_mask_v01)0x80ull) /**<  Bit mask to indicate the Geo-Location service.  */
#define IMS_SETTINGS_SERVICE_TYPE_PRESENCE_V01 ((ims_settings_service_type_mask_v01)0x100ull) /**<  Bit mask to indicate the Presence service.  */
#define IMS_SETTINGS_SERVICE_TYPE_FT_V01 ((ims_settings_service_type_mask_v01)0x200ull) /**<  Bit mask to indicate the File Transfer service.  */
#define IMS_SETTINGS_SERVICE_TYPE_RCS_ALL_V01 ((ims_settings_service_type_mask_v01)0x400ull) /**<  Bit mask to indicate all the RCS services.  */
#define IMS_SETTINGS_SERVICE_TYPE_DEFAULT_V01 ((ims_settings_service_type_mask_v01)0x8000ull) /**<  Bit mask to indicate the default services.\n
       If default service is enabled, operator mode will take preference.  */
/** @addtogroup imss_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t rat;

  uint8_t apn_type_apn_index;

  uint16_t service_mask;

  uint8_t auth_type_security_type;

  uint8_t ip_type_info;
}ims_settings_pol_man_rat_apn_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imss_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t pol_mgr_rat_apn_fallback;

  uint16_t pol_mgr_service_priority_wwan;
}ims_settings_pol_mgr_rat_apn_fb_sp_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imss_qmi_aggregates
    @{
  */
typedef struct {

  char pol_mgr_apn_name[IMS_SETTINGS_POL_MGR_APN_NAME_STR_LEN_V01 + 1];
}ims_settings_pol_mgr_apn_name_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the IMS Session Initiation Protocol (SIP) configuration parameters for the 
             requesting control point. */
typedef struct {

  /* Optional */
  /*  SIP Port Number */
  uint8_t sip_local_port_valid;  /**< Must be set to true if sip_local_port is being passed */
  uint16_t sip_local_port;
  /**<   Primary call session control function SIP port number.  */

  /* Optional */
  /*  Timer SIP Registration */
  uint8_t timer_sip_reg_valid;  /**< Must be set to true if timer_sip_reg is being passed */
  uint32_t timer_sip_reg;
  /**<   Initial SIP registration duration, in seconds, from the User Equipment (UE). */

  /* Optional */
  /*  Subscribe Timer */
  uint8_t subscribe_timer_valid;  /**< Must be set to true if subscribe_timer is being passed */
  uint32_t subscribe_timer;
  /**<   Duration, in seconds, of the subscription by the UE for IMS registration notifications. */

  /* Optional */
  /*  Timer T1   */
  uint8_t timer_t1_valid;  /**< Must be set to true if timer_t1 is being passed */
  uint32_t timer_t1;
  /**<   RTT estimate, in milliseconds. */

  /* Optional */
  /*  Timer T2  */
  uint8_t timer_t2_valid;  /**< Must be set to true if timer_t2 is being passed */
  uint32_t timer_t2;
  /**<   Maximum retransmit interval, in milliseconds, for non-invite requests 
         and invite responses. */

  /* Optional */
  /*  Timer TF  */
  uint8_t timer_tf_valid;  /**< Must be set to true if timer_tf is being passed */
  uint32_t timer_tf;
  /**<   Non-invite transaction timeout timer, in milliseconds.  */

  /* Optional */
  /*  Sigcomp Status */
  uint8_t sigcomp_enabled_valid;  /**< Must be set to true if sigcomp_enabled is being passed */
  uint8_t sigcomp_enabled;
  /**<   Values: \n
        -TRUE -- Enable  \n
        -FALSE -- Disable 
    */

  /* Optional */
  /*  Timer TJ */
  uint8_t timer_tj_valid;  /**< Must be set to true if timer_tj is being passed */
  uint16_t timer_tj;
  /**<   Wait time, in milliseconds, for the non-invite request retransmission. 
         If the value exceeds the range of uint16, it is set to 0xFFFF. 
    */

  /* Optional */
  /*  Timer TJ Extended */
  uint8_t timer_tj_ext_valid;  /**< Must be set to true if timer_tj_ext is being passed */
  uint32_t timer_tj_ext;
  /**<   Wait time, in milliseconds, for the non-invite request retransmission. */

  /* Optional */
  /*  Keep Alive Status */
  uint8_t keepalive_enabled_valid;  /**< Must be set to true if keepalive_enabled is being passed */
  uint8_t keepalive_enabled;
  /**<   Values: \n
        -TRUE -- Enable  \n
        -FALSE -- Disable (default)
    */

  /* Optional */
  /*  NAT-RTO Timer Value */
  uint8_t nat_rto_timer_valid;  /**< Must be set to true if nat_rto_timer is being passed */
  uint32_t nat_rto_timer;
  /**<   Request timeout value, in milliseconds, used in NAT implementation. 
         Default value is 500. 
    */
}ims_settings_set_sip_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the IMS Session Initiation Protocol (SIP) configuration parameters for the 
             requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
      - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      - qmi_error_type  -- Error code. Possible error code values are described in
                        the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   Settings standard response type. A settings-specific error code is 
         returned when the standard response error type is QMI_ERR_CAUSE_CODE. \n
         Values: \n
         - 0 -- No error \n
         - 1 -- Not ready \n
         - 2 -- File not available \n
         - 3 -- Message read failed \n
         - 4 -- Message write failed \n
         - 5 -- Other internal error 
    */
}ims_settings_set_sip_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the IMS registration manager configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Primary Call Session Control Function Port (CSCF) */
  uint8_t regmgr_config_pcscf_port_valid;  /**< Must be set to true if regmgr_config_pcscf_port is being passed */
  uint16_t regmgr_config_pcscf_port;
  /**<   Primary call session control function port. */

  /* Optional */
  /*  CSCF Port */
  uint8_t regmgr_primary_cscf_valid;  /**< Must be set to true if regmgr_primary_cscf is being passed */
  char regmgr_primary_cscf[IMS_SETTINGS_STRING_LEN_MAX_V01 + 1];
  /**<   Call session control port, fully qualified domain name. */

  /* Optional */
  /*  IMS Test Mode */
  uint8_t ims_test_mode_enabled_valid;  /**< Must be set to true if ims_test_mode_enabled is being passed */
  uint8_t ims_test_mode_enabled;
  /**<  
        Values: \n
        -TRUE -- Enable, no IMS registration \n
        -FALSE -- Disable, IMS registration is initiated
    */
}ims_settings_set_reg_mgr_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the IMS registration manager configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
       - qmi_error_type  -- Error code. Possible error code values are described in
                            the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response 
         error type is QMI_ERR_CAUSE_CODE.
    */
}ims_settings_set_reg_mgr_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the IMS SMS configuration parameters for the 
             requesting control point. */
typedef struct {

  /* Optional */
  /*  SMS Format */
  uint8_t sms_format_valid;  /**< Must be set to true if sms_format is being passed */
  ims_settings_sms_format_enum_v01 sms_format;
  /**<   Values: 
      - IMS_SETTINGS_SMS_FORMAT_3GPP2 (0) --  3GPP2     
      - IMS_SETTINGS_SMS_FORMAT_3GPP (1) --  3GPP 
 */

  /* Optional */
  /*  SMS Over IP Network Indication Flag  */
  uint8_t sms_over_ip_network_indication_valid;  /**< Must be set to true if sms_over_ip_network_indication is being passed */
  uint8_t sms_over_ip_network_indication;
  /**<   Values: \n
        -TRUE -- Turn on Mobile-Originated (MO) SMS \n
        -FALSE -- Turn off MO SMS
    */

  /* Optional */
  /*  Phone Context Universal Resource Identifier */
  uint8_t phone_context_uri_valid;  /**< Must be set to true if phone_context_uri is being passed */
  char phone_context_uri[IMS_SETTINGS_STRING_LEN_MAX_V01 + 1];
  /**<   Phone context universal resource identifier. */
}ims_settings_set_sms_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the IMS SMS configuration parameters for the 
             requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       - qmi_error_type  -- Error code. Possible error code values are described in
                            the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type  */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response 
         error type is QMI_ERR_CAUSE_CODE.
    */
}ims_settings_set_sms_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the IMS user configuration parameters for the requesting 
             control point. */
typedef struct {

  /* Optional */
  /*  IMS Domain Name */
  uint8_t ims_domain_valid;  /**< Must be set to true if ims_domain is being passed */
  char ims_domain[IMS_SETTINGS_STRING_LEN_MAX_V01 + 1];
  /**<   IMS domain name. */
}ims_settings_set_user_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the IMS user configuration parameters for the requesting 
             control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
       - qmi_error_type  -- Error code. Possible error code values are described in
                            the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */
}ims_settings_set_user_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the IMS Voice over Internet Protocol (VoIP) configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Session Duration */
  uint8_t session_expiry_timer_valid;  /**< Must be set to true if session_expiry_timer is being passed */
  uint16_t session_expiry_timer;
  /**<   Session duration, in seconds. */

  /* Optional */
  /*  Minimum Session Timer */
  uint8_t min_session_expiry_valid;  /**< Must be set to true if min_session_expiry is being passed */
  uint16_t min_session_expiry;
  /**<   Minimum allowed value for session timer, in seconds. */

  /* Optional */
  /*  Enable AMR WB  */
  uint8_t amr_wb_enable_valid;  /**< Must be set to true if amr_wb_enable is being passed */
  uint8_t amr_wb_enable;
  /**<   Flag to enable/disable Adaptive Multirate codec (AMR) Wideband (WB) audio. \n
        Values: \n
        -TRUE -- Enable \n
        -FALSE -- Disable 
    */

  /* Optional */
  /*  Enable SCR for AMR */
  uint8_t scr_amr_enable_valid;  /**< Must be set to true if scr_amr_enable is being passed */
  uint8_t scr_amr_enable;
  /**<   Flag to enable/disable Source Controlled Rate (SCR) for 
         AMR narrowband (NB). \n
        Values: \n
        -TRUE -- Enable \n
        -FALSE -- Disable  
    */

  /* Optional */
  /*  Enable SCR for AMR WB */
  uint8_t scr_amr_wb_enable_valid;  /**< Must be set to true if scr_amr_wb_enable is being passed */
  uint8_t scr_amr_wb_enable;
  /**<   Flag to enable/disable SCR for AMR WB audio. \n
        Values: \n
        -TRUE -- Enable \n
        -FALSE -- Disable
    */

  /* Optional */
  /*  AMR NB Modes Allowed */
  uint8_t amr_mode_valid;  /**< Must be set to true if amr_mode is being passed */
  uint8_t amr_mode;
  /**<   Bitmask for AMR NB modes allowed. \n
        Values: \n
        - 0x1 -- 4.75 kbps \n
        - 0x2 -- 5.15 kbps \n
        - 0x4 -- 5.9 kbps \n
        - 0x8 -- 6.17 kbps \n
        - 0x10 -- 7.4 kbps \n
        - 0x20 -- 7.95 kbps \n
        - 0x40 -- 10.2 kbps \n
        - 0x80 -- 12.2 kbps
    */

  /* Optional */
  /*  AMR WB Modes Allowed */
  uint8_t amr_wb_mode_valid;  /**< Must be set to true if amr_wb_mode is being passed */
  uint16_t amr_wb_mode;
  /**<   Bitmask for AMR WB modes allowed. \n
        Values: \n
        - 0x1 - 6.60 kbps \n
        - 0x2 - 8.85 kbps \n
        - 0x4 - 12.65 kbps \n
        - 0x8 - 14.25 kbps \n
        - 0x10 - 15.85 kbps \n
        - 0x20 - 18.25 kbps \n
        - 0x40 - 19.85 kbps \n 
        - 0x80 - 23.05 kbps \n
        - 0x100 - 23.85 kbps
    */

  /* Optional */
  /*  AMR Octet Aligned */
  uint8_t amr_octet_align_valid;  /**< Must be set to true if amr_octet_align is being passed */
  uint8_t amr_octet_align;
  /**<    Flag indicating whether the octet is aligned for AMR NB audio. \n
        Values: \n
        -TRUE -- Aligned \n
        -FALSE -- Not aligned, Bandwidth Efficient mode
    */

  /* Optional */
  /*  AMR WB Octet Aligned */
  uint8_t amr_wb_octet_align_valid;  /**< Must be set to true if amr_wb_octet_align is being passed */
  uint8_t amr_wb_octet_align;
  /**<   Flag indicating if the octet is aligned for AMR WB audio. \n
        Values: \n
        -TRUE -- Aligned \n
        -FALSE -- Not aligned, Bandwidth Efficient mode
    */

  /* Optional */
  /*  Ringing Timer  */
  uint8_t ringing_timer_valid;  /**< Must be set to true if ringing_timer is being passed */
  uint16_t ringing_timer;
  /**<   Duration of the ringing timer, in seconds. The ringing timer 
         starts on the ringing event. If the call is not answered within 
         the duration of this timer, the call is disconnected.
    */

  /* Optional */
  /*  Ringback Timer Duration  */
  uint8_t ringback_timer_valid;  /**< Must be set to true if ringback_timer is being passed */
  uint16_t ringback_timer;
  /**<   Duration of the ringback timer, in seconds. The ringback timer
         starts on the ringback event. If the call is not answered within
         the duration of this timer, the call is disconnected.
    */

  /* Optional */
  /*  RTP/RTCP Inactivity Timer Duration */
  uint8_t rtp_rtcp_inactivity_timer_valid;  /**< Must be set to true if rtp_rtcp_inactivity_timer is being passed */
  uint16_t rtp_rtcp_inactivity_timer;
  /**<   Duration of the RTP/RTCP inactivity timer, in seconds. If no 
         RTP/RTCP packet is received prior to the expiration of this timer, 
         the call is disconnected.
    */

  /* Optional */
  /*  String Consisting of AMR NB Modes Allowed. */
  uint8_t amr_mode_str_valid;  /**< Must be set to true if amr_mode_str is being passed */
  char amr_mode_str[IMS_SETTINGS_VOIP_AMR_MODE_STR_LEN_V01 + 1];
  /**<   String consisting of AMR NB modes allowed. */

  /* Optional */
  /*  String Consisting of AMR WB Modes Allowed. */
  uint8_t amr_wb_mode_str_valid;  /**< Must be set to true if amr_wb_mode_str is being passed */
  char amr_wb_mode_str[IMS_SETTINGS_VOIP_AMR_WB_MODE_STR_LEN_V01 + 1];
  /**<   String consisting of AMR WB modes allowed. */
}ims_settings_set_voip_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the IMS Voice over Internet Protocol (VoIP) configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
       - qmi_error_type  -- Error code. Possible error code values are described in
                        the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */
}ims_settings_set_voip_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the IMS presence-related configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Publish Expiry Timer    */
  uint8_t publish_expiry_timer_valid;  /**< Must be set to true if publish_expiry_timer is being passed */
  uint32_t publish_expiry_timer;
  /**<   Publish timer, in seconds, when publish is sent on an IMS network using  
         4G radio access technology.    
    */

  /* Optional */
  /*  Publish Extended Expiry Timer */
  uint8_t publish_extended_expiry_timer_valid;  /**< Must be set to true if publish_extended_expiry_timer is being passed */
  uint32_t publish_extended_expiry_timer;
  /**<   Publish extended timer, in seconds, when publish is sent on an IMS network 
         in a non-4G radio access technology or when in Airplane 
         Power-Down mode in a 4G radio access technology.
    */

  /* Optional */
  /*  Minimum Publish Interval */
  uint8_t minimum_publish_interval_valid;  /**< Must be set to true if minimum_publish_interval is being passed */
  uint32_t minimum_publish_interval;
  /**<   Duration, in seconds, between successive publish requests.
    */

  /* Optional */
  /*  Capability Poll List Subscription Expiry Timer */
  uint8_t capability_poll_list_subscription_expiry_timer_valid;  /**< Must be set to true if capability_poll_list_subscription_expiry_timer is being passed */
  uint32_t capability_poll_list_subscription_expiry_timer;
  /**<   Expiry timer value, in seconds, for the list subscription request. */

  /* Optional */
  /*  Discovery Capability Enabled */
  uint8_t capability_discovery_enable_valid;  /**< Must be set to true if capability_discovery_enable is being passed */
  uint8_t capability_discovery_enable;
  /**<   Flag indicating whether discovery capability is enabled. \n
         Values: \n
         -TRUE -- Presence publishes/subscribes and processes any notification received. \n
         -FALSE -- Presence does not publish/subscribe and 
                  ignores any notification received */

  /* Optional */
  /*  Cache Capability Expiration */
  uint8_t capabilites_cache_expiration_valid;  /**< Must be set to true if capabilites_cache_expiration is being passed */
  uint32_t capabilites_cache_expiration;
  /**<   Duration of time, in seconds, for which the retrieved capability is considered valid.
    */

  /* Optional */
  /*  Cache Availability Expiration */
  uint8_t availability_cache_expiration_valid;  /**< Must be set to true if availability_cache_expiration is being passed */
  uint32_t availability_cache_expiration;
  /**<   Duration of time, in seconds, for which the retrieved availability is considered valid. */

  /* Optional */
  /*  Capability Poll Interval */
  uint8_t capability_poll_interval_valid;  /**< Must be set to true if capability_poll_interval is being passed */
  uint32_t capability_poll_interval;
  /**<   Duration of time, in seconds, between successive capability polling.  */

  /* Optional */
  /*  Maximum Subscription List Entries */
  uint8_t max_subcription_list_entries_valid;  /**< Must be set to true if max_subcription_list_entries is being passed */
  uint32_t max_subcription_list_entries;
  /**<   Maximum number of entries that can be kept in the list subscription.
    */

  /* Optional */
  /*  VoLTE User Opted In Status */
  uint8_t volte_user_opted_in_status_valid;  /**< Must be set to true if volte_user_opted_in_status is being passed */
  uint8_t volte_user_opted_in_status;
  /**<   Flag indicating whether VoLTE service is accepted by the user. 
         Values: \n
         -TRUE -- Accepted \n
         -FALSE -- Not accepted
    */

  /* Optional */
  /*  Last Published ETAG */
  uint8_t last_publish_etag_valid;  /**< Must be set to true if last_publish_etag is being passed */
  char last_publish_etag[IMS_SETTINGS_PRESENCE_PUBLISH_ETAG_LEN_V01 + 1];
  /**<   Last published ETAG. */

  /* Optional */
  /*  Last Published Time */
  uint8_t last_published_time_valid;  /**< Must be set to true if last_published_time is being passed */
  uint32_t last_published_time;
  /**<   Last published time. */

  /* Optional */
  /*  Last Negotiated Published Expire */
  uint8_t last_negotiated_published_expire_valid;  /**< Must be set to true if last_negotiated_published_expire is being passed */
  uint32_t last_negotiated_published_expire;
  /**<   Last negotiated published expire, in seconds. */

  /* Optional */
  /*  GZIP Enabled */
  uint8_t gzip_enabled_valid;  /**< Must be set to true if gzip_enabled is being passed */
  uint8_t gzip_enabled;
  /**<   Flag indicating whether GZIP compression enabled. 
         Values: \n
         -True -- Enabled \n
         -False -- Disabled
                  */

  /* Optional */
  /*  Presence Notification Wait Duration */
  uint8_t presence_notify_wait_duration_valid;  /**< Must be set to true if presence_notify_wait_duration is being passed */
  uint16_t presence_notify_wait_duration;
  /**<   Presence notification wait duration, in seconds. */

  /* Optional */
  /*  Publish Error Recovery Timer */
  uint8_t publish_error_recovery_timer_valid;  /**< Must be set to true if publish_error_recovery_timer is being passed */
  uint32_t publish_error_recovery_timer;
  /**<   Publish error recovery timer, in seconds. */
}ims_settings_set_presence_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the IMS presence-related configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
         - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
         - qmi_error_type  -- Error code. Possible error code values are described in
                        the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type  */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */
}ims_settings_set_presence_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the IMS media-related configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Optional */
  /*  H.264 Profile */
  uint8_t h264_profile_valid;  /**< Must be set to true if h264_profile is being passed */
  ims_settings_h264_profile_enum_v01 h264_profile;
  /**<   Profile used for the H.264 codec. Values: \n
      - IMS_SETTINGS_H264_PROFILE_BASELINE (0x00) --  Baseline profile
      - IMS_SETTINGS_H264_PROFILE_MAIN (0x01) --  Main profile
      - IMS_SETTINGS_H264_PROFILE_EXTENDED (0x02) --  Extended profile
      - IMS_SETTINGS_H264_PROFILE_HIGH (0x03) --  High profile*/

  /* Optional */
  /*  H.264 Level */
  uint8_t h264_level_valid;  /**< Must be set to true if h264_level is being passed */
  ims_settings_h264_level_enum_v01 h264_level;
  /**<   Level used for the H.264 codec. Values: \n
      - IMS_SETTINGS_H264_LEVEL1 (0x00) --  Level 1
      - IMS_SETTINGS_H264_LEVEL1B (0x01) --  Level 1b
      - IMS_SETTINGS_H264_LEVEL11 (0x02) --  Level 1.1
      - IMS_SETTINGS_H264_LEVEL12 (0x03) --  Level 1.2
      - IMS_SETTINGS_H264_LEVEL13 (0x04) --  Level 1.3
      - IMS_SETTINGS_H264_LEVEL2 (0x05) --  Level 2
      - IMS_SETTINGS_H264_LEVEL21 (0x06) --  Level 2.1
      - IMS_SETTINGS_H264_LEVEL22 (0x07) --  Level 2.2
      - IMS_SETTINGS_H264_LEVEL3 (0x08) --  Level 3
      - IMS_SETTINGS_H264_LEVEL31 (0x09) --  Level 3.1
      - IMS_SETTINGS_H264_LEVEL32 (0x0A) --  Level 3.2
      - IMS_SETTINGS_H264_LEVEL4 (0x0B) --  Level 4
      - IMS_SETTINGS_H264_LEVEL41 (0x0C) --  Level 4.1
      - IMS_SETTINGS_H264_LEVEL42 (0x0D) --  Level 4.2
      - IMS_SETTINGS_H264_LEVEL5 (0x0E) --  Level 5
      - IMS_SETTINGS_H264_LEVEL51 (0x0F) --  Level 5.1*/

  /* Optional */
  /*  Video Bitrate */
  uint8_t video_bitrate_valid;  /**< Must be set to true if video_bitrate is being passed */
  uint16_t video_bitrate;
  /**<   Bitrate of the video, in kbps. */

  /* Optional */
  /*  Video Refresh Rate */
  uint8_t video_frames_per_second_valid;  /**< Must be set to true if video_frames_per_second is being passed */
  uint8_t video_frames_per_second;
  /**<   Video refresh rate, in frames per second. */

  /* Optional */
  /*  Video Display Resolution  */
  uint8_t video_resolution_valid;  /**< Must be set to true if video_resolution is being passed */
  ims_video_resolution_enum_v01 video_resolution;
  /**<   Resolution of the video display. Values: \n
      - IMS_SETTINGS_SQCIF_RESOLUTION (0x00) --  SQCIF
      - IMS_SETTINGS_QCIF_RESOLUTION (0x01) --  QCIF
      - IMS_SETTINGS_CIF_RESOLUTION (0x02) --  CIF
      - IMS_SETTINGS_QQVGA_RESOLUTION (0x03) --  QQVGA
      - IMS_SETTINGS_QVGA_RESOLUTION (0x04) --  QVGA
      - IMS_SETTINGS_VGA_RESOLUTION (0x05) --  VGA */

  /* Optional */
  /*  Video Codec */
  uint8_t video_codec_valid;  /**< Must be set to true if video_codec is being passed */
  ims_settings_video_codec_enum_v01 video_codec;
  /**<   Codec used for the video. Values: \n
      - IMS_SETTINGS_CODEC_MPEG4_XVID (0x00) --  XVID MPEG4 codec
      - IMS_SETTINGS_CODEC_MPEG4_ISO (0x01) --  ISO MPEG4 codec
      - IMS_SETTINGS_CODEC_H263 (0x02) --  H.263 codec
      - IMS_SETTINGS_CODEC_H264 (0x03) --  H.264 codec*/
}ims_settings_set_media_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the IMS media-related configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
        - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
        - qmi_error_type  -- Error code. Possible error code values are described in
                        the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */
}ims_settings_set_media_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the IMS qipcall-related configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Optional */
  /*  VT Calling Status */
  uint8_t vt_calling_enabled_valid;  /**< Must be set to true if vt_calling_enabled is being passed */
  uint8_t vt_calling_enabled;
  /**<   Values: \n
        -TRUE -- Enable  \n
        -FALSE -- Disable 
    */

  /* Optional */
  /*  Mobile Data Status */
  uint8_t mobile_data_enabled_valid;  /**< Must be set to true if mobile_data_enabled is being passed */
  uint8_t mobile_data_enabled;
  /**<   Values: \n
        -TRUE -- Enable  \n
        -FALSE -- Disable 
    */

  /* Optional */
  /*  Volte Status */
  uint8_t volte_enabled_valid;  /**< Must be set to true if volte_enabled is being passed */
  uint8_t volte_enabled;
  /**<   Values: \n
        -True -- Enable  \n
        -False -- Disable 
    */
}ims_settings_set_qipcall_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the IMS qipcall-related configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
        - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
        - qmi_error_type  -- Error code. Possible error code values are described in
                        the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */
}ims_settings_set_qipcall_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the IMS registration manager extended configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Re-registration Delay */
  uint8_t reregistration_delay_valid;  /**< Must be set to true if reregistration_delay is being passed */
  uint16_t reregistration_delay;
  /**<  IMS re-registration wait time when RAT transition from eHRPD to LTE, in seconds*/
}ims_settings_set_reg_mgr_extended_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the IMS registration manager extended configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
       - qmi_error_type  -- Error code. Possible error code values are described in
                            the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   Settings-specific error code is returned when the standard response 
         error type is QMI_ERR_CAUSE_CODE.
    */
}ims_settings_set_reg_mgr_extended_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the IMS policy manager configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Policy Manager RAT APN Information Array */
  uint8_t pol_mgr_rat_apn_info_valid;  /**< Must be set to true if pol_mgr_rat_apn_info is being passed */
  ims_settings_pol_man_rat_apn_info_v01 pol_mgr_rat_apn_info[IMS_SETTINGS_POL_MGR_RAT_APN_SIZE_V01];
  /**<  Array of RAT and APN and their information parameters */

  /* Optional */
  /*  Policy Manager RAT APN FallBack and Service Priority Information Array */
  uint8_t pol_mgr_rat_apn_fb_sp_info_valid;  /**< Must be set to true if pol_mgr_rat_apn_fb_sp_info is being passed */
  ims_settings_pol_mgr_rat_apn_fb_sp_info_v01 pol_mgr_rat_apn_fb_sp_info[IMS_SETTINGS_POL_MGR_RAT_APN_FB_SIZE_V01];
  /**<  Array of RAT_APN and their fall back & service priority information parameters */

  /* Optional */
  /*  Policy Manager Allowed Services over WLAN */
  uint8_t pol_mgr_allowed_services_wlan_valid;  /**< Must be set to true if pol_mgr_allowed_services_wlan is being passed */
  ims_settings_service_type_mask_v01 pol_mgr_allowed_services_wlan;
  /**<   Bit mask indicating the services which are allowed over WLAN. */

  /* Optional */
  /*  Policy Manager Add All Feature Tags */
  uint8_t pol_mgr_add_all_fts_valid;  /**< Must be set to true if pol_mgr_add_all_fts is being passed */
  uint8_t pol_mgr_add_all_fts;
  /**<  Whether to add all feature tag list or application */

  /* Optional */
  /*  Policy Manager ACS Priority */
  uint8_t pol_mgr_acs_priority_valid;  /**< Must be set to true if pol_mgr_acs_priority is being passed */
  uint8_t pol_mgr_acs_priority;
  /**<  Priority of ACS values */

  /* Optional */
  /*  Policy Manager ISIM Priority */
  uint8_t pol_mgr_isim_priority_valid;  /**< Must be set to true if pol_mgr_isim_priority is being passed */
  uint8_t pol_mgr_isim_priority;
  /**<  Priority of ISIM values */

  /* Optional */
  /*  Policy Manager NV Priority */
  uint8_t pol_mgr_nv_priority_valid;  /**< Must be set to true if pol_mgr_nv_priority is being passed */
  uint8_t pol_mgr_nv_priority;
  /**<  Priority of pre-configuration NV values */

  /* Optional */
  /*  Policy Manager PCO Priority */
  uint8_t pol_mgr_pco_priority_valid;  /**< Must be set to true if pol_mgr_pco_priority is being passed */
  uint8_t pol_mgr_pco_priority;
  /**<  Priority of PCO values */

  /* Optional */
  /*  Policy Manager IMS Service Priority */
  uint8_t pol_mgr_ims_service_status_valid;  /**< Must be set to true if pol_mgr_ims_service_status is being passed */
  ims_settings_service_type_mask_v01 pol_mgr_ims_service_status;
  /**<   Bit mask indicating the services that are enabled in the device. */

  /* Optional */
  /*  Policy Manager APN Name List */
  uint8_t pol_mgr_apn_name_valid;  /**< Must be set to true if pol_mgr_apn_name is being passed */
  ims_settings_pol_mgr_apn_name_v01 pol_mgr_apn_name[IMS_SETTINGS_POL_MGR_APN_SIZE_V01];
  /**<  IMS APN name */
}ims_settings_set_pol_mgr_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the IMS policy manager configuration parameters 
             for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
       - qmi_error_type  -- Error code. Possible error code values are described in
                            the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   Settings-specific error code is returned when the standard response 
         error type is QMI_ERR_CAUSE_CODE.
    */
}ims_settings_set_pol_mgr_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_sip_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the SIP configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Settings standard response type. A settings-specific error code is returned 
         when the standard response error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   Settings response. */

  /* Optional */
  /*  SIP Local Port */
  uint8_t sip_local_port_valid;  /**< Must be set to true if sip_local_port is being passed */
  uint16_t sip_local_port;
  /**<   Primary call session control function SIP port number.
    */

  /* Optional */
  /*  SIP Registration Timer */
  uint8_t timer_sip_reg_valid;  /**< Must be set to true if timer_sip_reg is being passed */
  uint32_t timer_sip_reg;
  /**<   Initial SIP registration duration, in seconds, from the UE.
    */

  /* Optional */
  /*  Subscribe Timer */
  uint8_t subscribe_timer_valid;  /**< Must be set to true if subscribe_timer is being passed */
  uint32_t subscribe_timer;
  /**<   Duration, in seconds, of the subscription by the UE for IMS registration 
         notifications.
    */

  /* Optional */
  /*  Timer T1  */
  uint8_t timer_t1_valid;  /**< Must be set to true if timer_t1 is being passed */
  uint32_t timer_t1;
  /**<   RTT estimate, in milliseconds.
    */

  /* Optional */
  /*  Timer T2  */
  uint8_t timer_t2_valid;  /**< Must be set to true if timer_t2 is being passed */
  uint32_t timer_t2;
  /**<   Maximum retransmit interval, in milliseconds, for non-invite requests 
         and invite responses.
    */

  /* Optional */
  /*  Timer TF  */
  uint8_t timer_tf_valid;  /**< Must be set to true if timer_tf is being passed */
  uint32_t timer_tf;
  /**<   Non-invite transaction timeout timer, in milliseconds.
    */

  /* Optional */
  /*  Sigcomp Status */
  uint8_t sigcomp_enabled_valid;  /**< Must be set to true if sigcomp_enabled is being passed */
  uint8_t sigcomp_enabled;
  /**<  
        Values: \n
        -TRUE -- SigComp enabled \n
        -FALSE -- SigComp disabled
    */

  /* Optional */
  /*  Timer TJ */
  uint8_t timer_tj_valid;  /**< Must be set to true if timer_tj is being passed */
  uint16_t timer_tj;
  /**<   Wait time, in milliseconds, for the non-invite request retransmission. 
         If the value exceeds the range of uint16, it is set to 0xFFFF.
    */

  /* Optional */
  /*  Timer TJ Extended */
  uint8_t timer_tj_ext_valid;  /**< Must be set to true if timer_tj_ext is being passed */
  uint32_t timer_tj_ext;
  /**<   Wait time, in milliseconds, for the non-invite request retransmission. */

  /* Optional */
  /*  Keep Alive Status */
  uint8_t keepalive_enabled_valid;  /**< Must be set to true if keepalive_enabled is being passed */
  uint8_t keepalive_enabled;
  /**<   Values: \n
        -TRUE -- Enable  \n
        -FALSE -- Disable 
    */

  /* Optional */
  /*  NAT-RTO Timer Value */
  uint8_t nat_rto_timer_valid;  /**< Must be set to true if nat_rto_timer is being passed */
  uint32_t nat_rto_timer;
  /**<   Requests timeout value, in milliseconds, used in NAT implementation. */
}ims_settings_get_sip_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_reg_mgr_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the registration manager configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
       error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  Proxy Call Session Control Function Port */
  uint8_t regmgr_config_pcscf_port_valid;  /**< Must be set to true if regmgr_config_pcscf_port is being passed */
  uint16_t regmgr_config_pcscf_port;
  /**<   Proxy CSCF port.*/

  /* Optional */
  /*  Primary CSCF Port */
  uint8_t regmgr_primary_cscf_valid;  /**< Must be set to true if regmgr_primary_cscf is being passed */
  char regmgr_primary_cscf[IMS_SETTINGS_STRING_LEN_MAX_V01 + 1];
  /**<   Primary CSCF port, fully qualified domain name.  */

  /* Optional */
  /*  IMS Test Mode */
  uint8_t ims_test_mode_valid;  /**< Must be set to true if ims_test_mode is being passed */
  uint8_t ims_test_mode;
  /**<  
        Values: \n
        -TRUE -- Enabled \n
        -FALSE -- Disabled
    */
}ims_settings_get_reg_mgr_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_sms_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the SMS configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   Settings response.*/

  /* Optional */
  /*  SMS Format */
  uint8_t sms_format_valid;  /**< Must be set to true if sms_format is being passed */
  ims_settings_sms_format_enum_v01 sms_format;
  /**<   Values: 
      - IMS_SETTINGS_SMS_FORMAT_3GPP2 (0) --  3GPP2     
      - IMS_SETTINGS_SMS_FORMAT_3GPP (1) --  3GPP 
 */

  /* Optional */
  /*  SMS Over IP Network Indication Flag  */
  uint8_t sms_over_ip_network_indication_valid;  /**< Must be set to true if sms_over_ip_network_indication is being passed */
  uint8_t sms_over_ip_network_indication;
  /**<  
        Values: \n
        - TRUE -- MO SMS Turned ON \n
        - FALSE -- MO SMS Turned OFF
    */

  /* Optional */
  /*  Phone Context Universal Resource Identifier */
  uint8_t phone_context_uri_valid;  /**< Must be set to true if phone_context_uri is being passed */
  char phone_context_uri[IMS_SETTINGS_STRING_LEN_MAX_V01 + 1];
  /**<   Phone context universal resource identifier. */
}ims_settings_get_sms_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_user_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the user configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. A settings-specific error code is returned when the standard response
       error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   Settings response. */

  /* Optional */
  /*  IMS Domain Name */
  uint8_t ims_domain_valid;  /**< Must be set to true if ims_domain is being passed */
  char ims_domain[IMS_SETTINGS_STRING_LEN_MAX_V01 + 1];
  /**<   IMS domain name. */
}ims_settings_get_user_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_voip_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the VoIP configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  Session Duration */
  uint8_t session_expiry_timer_valid;  /**< Must be set to true if session_expiry_timer is being passed */
  uint16_t session_expiry_timer;
  /**<   Session duration, in seconds. */

  /* Optional */
  /*  Minimum Session Timer */
  uint8_t min_session_expiry_valid;  /**< Must be set to true if min_session_expiry is being passed */
  uint16_t min_session_expiry;
  /**<   Minimum allowed value, in seconds, for session timer. */

  /* Optional */
  /*  Enable AMR WB  */
  uint8_t amr_wb_enable_valid;  /**< Must be set to true if amr_wb_enable is being passed */
  uint8_t amr_wb_enable;
  /**<   Flag indicating AMR WB audio. \n
        Values: \n
        -TRUE -- Enabled \n
        -FALSE -- Disabled
    */

  /* Optional */
  /*  Enable SCR AMR  */
  uint8_t scr_amr_enable_valid;  /**< Must be set to true if scr_amr_enable is being passed */
  uint8_t scr_amr_enable;
  /**<   Flag indicating SCR for AMR NB audio. \n
        Values: \n
        -TRUE -- Enabled \n
        -FALSE -- Disabled
    */

  /* Optional */
  /*  Enable SCR AMR WB */
  uint8_t scr_amr_wb_enable_valid;  /**< Must be set to true if scr_amr_wb_enable is being passed */
  uint8_t scr_amr_wb_enable;
  /**<   Flag indicating SCR for AMR WB audio. \n
        Values: \n
        -TRUE -- Enabled \n
        -FALSE -- Disabled
    */

  /* Optional */
  /*  AMR NB Mode */
  uint8_t amr_mode_valid;  /**< Must be set to true if amr_mode is being passed */
  uint8_t amr_mode;
  /**<   Bitmask indicating AMR NB modes. \n
        Values: \n
        - 0x1 -- 4.75 kbps \n
        - 0x2 -- 5.15 kbps \n
        - 0x4 -- 5.9 kbps \n
        - 0x8 -- 6.17 kbps \n
        - 0x10 -- 7.4 kbps \n
        - 0x20 -- 7.95 kbps \n
        - 0x40 -- 10.2 kbps \n
        - 0x80 -- 12.2 kbps 
    */

  /* Optional */
  /*  AMR WB Mode */
  uint8_t amr_wb_mode_valid;  /**< Must be set to true if amr_wb_mode is being passed */
  uint16_t amr_wb_mode;
  /**<   Bitmask indicating AMR WB modes. \n
        Values: \n
        - 0x1 -- 6.60 kbps \n
        - 0x2 -- 8.85 kbps \n
        - 0x4 -- 12.65 kbps \n
        - 0x8 -- 14.25 kbps \n
        - 0x10 -- 15.85 kbps \n
        - 0x20 -- 18.25 kbps \n
        - 0x40 -- 19.85 kbps \n
        - 0x80 -- 23.05 kbps \n
        - 0x100 -- 23.85 kbps 
    */

  /* Optional */
  /*  AMR NB Octet Aligned */
  uint8_t amr_octet_align_valid;  /**< Must be set to true if amr_octet_align is being passed */
  uint8_t amr_octet_align;
  /**<   Flag indicating whether the octet is aligned for AMR NB audio. \n
        Values: \n
        -TRUE -- Octet aligned. \n
        -FALSE -- Octet not aligned, Bandwidth Efficient Mode
    */

  /* Optional */
  /*  AMR WB Octet Aligned */
  uint8_t amr_wb_octet_align_valid;  /**< Must be set to true if amr_wb_octet_align is being passed */
  uint8_t amr_wb_octet_align;
  /**<   Flag indicating whether the octet is aligned for AMR WB audio. \n
        Values: \n
        -TRUE -- Octet aligned \n
        -FALSE -- Octet not aligned, Bandwidth Efficient Mode
    */

  /* Optional */
  /*  Ringing Timer Duration */
  uint8_t ringing_timer_valid;  /**< Must be set to true if ringing_timer is being passed */
  uint16_t ringing_timer;
  /**<   Duration, in seconds, of the ringing timer. The ringing
         timer is started on the ringing event. If the call is not answered
         within the duration of this timer, the call is
         disconnected.
    */

  /* Optional */
  /*  Ringback Timer Duration */
  uint8_t ringback_timer_valid;  /**< Must be set to true if ringback_timer is being passed */
  uint16_t ringback_timer;
  /**<   Duration, in seconds, of the ringback timer. The ringback
         timer is started on the ringback event. If the call is not 
         answered within the duration of this timer, the call is
         disconnected.
    */

  /* Optional */
  /*  RTP/RTCP Inactivity Timer Duration  */
  uint8_t rtp_rtcp_inactivity_timer_valid;  /**< Must be set to true if rtp_rtcp_inactivity_timer is being passed */
  uint16_t rtp_rtcp_inactivity_timer;
  /**<   Duration, in seconds, of the RTP/RTCP inactivity timer. If no
         RTP/RTCP packet is received prior to the expiration of this timer, 
         the call is disconnected.
    */
}ims_settings_get_voip_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_presence_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the presence-related configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Settings Standard Response Type  */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
       */

  /* Optional */
  /*  Publish Timer */
  uint8_t publish_expiry_timer_valid;  /**< Must be set to true if publish_expiry_timer is being passed */
  uint32_t publish_expiry_timer;
  /**<   Publish timer, in seconds, when publish is sent on an IMS network
         using 4G radio access technology. */

  /* Optional */
  /*  Publish Extended Expiry  */
  uint8_t publish_extended_expiry_timer_valid;  /**< Must be set to true if publish_extended_expiry_timer is being passed */
  uint32_t publish_extended_expiry_timer;
  /**<    Publish extended timer, in seconds, when publish is sent on an IMS network
          in a non-4G radio access technology, or when in Airplane Power-Down mode 
          in a 4G radio access technology.
     */

  /* Optional */
  /*  Minimum Publish Interval */
  uint8_t minimum_publish_interval_valid;  /**< Must be set to true if minimum_publish_interval is being passed */
  uint32_t minimum_publish_interval;
  /**<   Duration, in seconds, between successive publish requests. */

  /* Optional */
  /*  Capability Poll List Subscription Expiry Timer */
  uint8_t capability_poll_list_subscription_expiry_timer_valid;  /**< Must be set to true if capability_poll_list_subscription_expiry_timer is being passed */
  uint32_t capability_poll_list_subscription_expiry_timer;
  /**<   Expiry timer value, in seconds, for the list subscription request. */

  /* Optional */
  /*  Discovery Capability Enabled */
  uint8_t capability_discovery_enable_valid;  /**< Must be set to true if capability_discovery_enable is being passed */
  uint8_t capability_discovery_enable;
  /**<   Flag indicating whether or not discovery capability is enabled.
        Values: \n
        -TRUE -- Presence publishes/subscribes and processes 
                 any notifications received \n
        -FALSE -- Presence does not publish/subscribe and ignores 
                  any notification received
    */

  /* Optional */
  /*  Cache Capability Expiration */
  uint8_t capabilites_cache_expiration_valid;  /**< Must be set to true if capabilites_cache_expiration is being passed */
  uint32_t capabilites_cache_expiration;
  /**<   Duration of time, in seconds, for which the retrieved capability is 
         considered valid. */

  /* Optional */
  /*  Cache Availability Expiration */
  uint8_t availability_cache_expiration_valid;  /**< Must be set to true if availability_cache_expiration is being passed */
  uint32_t availability_cache_expiration;
  /**<   Duration of time, in seconds, for which the retrieved capability is 
         considered valid.  */

  /* Optional */
  /*  Capability Poll Interval */
  uint8_t capability_poll_interval_valid;  /**< Must be set to true if capability_poll_interval is being passed */
  uint32_t capability_poll_interval;
  /**<   Duration of time, in seconds, for which the retrieved capability is 
         considered valid. */

  /* Optional */
  /*  Maximum Subscription List Entries */
  uint8_t max_subcription_list_entries_valid;  /**< Must be set to true if max_subcription_list_entries is being passed */
  uint32_t max_subcription_list_entries;
  /**<    Maximum number of entries that can be kept in the list subscription. */

  /* Optional */
  /*  VoLTE User Opted In Status */
  uint8_t volte_user_opted_in_status_valid;  /**< Must be set to true if volte_user_opted_in_status is being passed */
  uint8_t volte_user_opted_in_status;
  /**<    Flag indicating whether or not VoLTE service is accepted by the user.
          Values: \n
          -TRUE -- Accepted \n
          -FALSE -- Not accepted
    */

  /* Optional */
  /*  Last Published ETAG */
  uint8_t last_publish_etag_valid;  /**< Must be set to true if last_publish_etag is being passed */
  char last_publish_etag[IMS_SETTINGS_PRESENCE_PUBLISH_ETAG_LEN_V01 + 1];
  /**<   Last published ETAG. */

  /* Optional */
  /*  Last Published Time */
  uint8_t last_published_time_valid;  /**< Must be set to true if last_published_time is being passed */
  uint32_t last_published_time;
  /**<   Last published time. */

  /* Optional */
  /*  Last Negotiated Published Expire */
  uint8_t last_negotiated_published_expire_valid;  /**< Must be set to true if last_negotiated_published_expire is being passed */
  uint32_t last_negotiated_published_expire;
  /**<   Last negotiated published expire, in seconds. */

  /* Optional */
  /*  GZIP Enabled */
  uint8_t gzip_enabled_valid;  /**< Must be set to true if gzip_enabled is being passed */
  uint8_t gzip_enabled;
  /**<   Flag indicating whether GZIP compression enabled. 
         Values: \n
         -True -- Enabled \n
         -False -- Disabled
                  */

  /* Optional */
  /*  Presence Notification Wait Duration */
  uint8_t presence_notify_wait_duration_valid;  /**< Must be set to true if presence_notify_wait_duration is being passed */
  uint16_t presence_notify_wait_duration;
  /**<   Presence notification wait duration, in seconds. */

  /* Optional */
  /*  Publish Error Recovery Timer */
  uint8_t publish_error_recovery_timer_valid;  /**< Must be set to true if publish_error_recovery_timer is being passed */
  uint32_t publish_error_recovery_timer;
  /**<   Publish error recovery timer, in seconds. */
}ims_settings_get_presence_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_media_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the media-related configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
       - qmi_error_type  -- Error code. Possible error code values are described in
                        the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  H.264 Profile */
  uint8_t h264_profile_valid;  /**< Must be set to true if h264_profile is being passed */
  ims_settings_h264_profile_enum_v01 h264_profile;
  /**<   Profile used for H.264 codec. Values: \n
      - IMS_SETTINGS_H264_PROFILE_BASELINE (0x00) --  Baseline profile
      - IMS_SETTINGS_H264_PROFILE_MAIN (0x01) --  Main profile
      - IMS_SETTINGS_H264_PROFILE_EXTENDED (0x02) --  Extended profile
      - IMS_SETTINGS_H264_PROFILE_HIGH (0x03) --  High profile */

  /* Optional */
  /*  H.264 Level */
  uint8_t h264_level_valid;  /**< Must be set to true if h264_level is being passed */
  ims_settings_h264_level_enum_v01 h264_level;
  /**<   Level used for H.264 codec. Values: \n
      - IMS_SETTINGS_H264_LEVEL1 (0x00) --  Level 1
      - IMS_SETTINGS_H264_LEVEL1B (0x01) --  Level 1b
      - IMS_SETTINGS_H264_LEVEL11 (0x02) --  Level 1.1
      - IMS_SETTINGS_H264_LEVEL12 (0x03) --  Level 1.2
      - IMS_SETTINGS_H264_LEVEL13 (0x04) --  Level 1.3
      - IMS_SETTINGS_H264_LEVEL2 (0x05) --  Level 2
      - IMS_SETTINGS_H264_LEVEL21 (0x06) --  Level 2.1
      - IMS_SETTINGS_H264_LEVEL22 (0x07) --  Level 2.2
      - IMS_SETTINGS_H264_LEVEL3 (0x08) --  Level 3
      - IMS_SETTINGS_H264_LEVEL31 (0x09) --  Level 3.1
      - IMS_SETTINGS_H264_LEVEL32 (0x0A) --  Level 3.2
      - IMS_SETTINGS_H264_LEVEL4 (0x0B) --  Level 4
      - IMS_SETTINGS_H264_LEVEL41 (0x0C) --  Level 4.1
      - IMS_SETTINGS_H264_LEVEL42 (0x0D) --  Level 4.2
      - IMS_SETTINGS_H264_LEVEL5 (0x0E) --  Level 5
      - IMS_SETTINGS_H264_LEVEL51 (0x0F) --  Level 5.1 */

  /* Optional */
  /*  Video Bitrate */
  uint8_t video_bitrate_valid;  /**< Must be set to true if video_bitrate is being passed */
  uint16_t video_bitrate;
  /**<   Bitrate of the video, in kbps. */

  /* Optional */
  /*  Video Refresh Rate */
  uint8_t video_frames_per_second_valid;  /**< Must be set to true if video_frames_per_second is being passed */
  uint8_t video_frames_per_second;
  /**<   Video refresh rate, in frames per second. */

  /* Optional */
  /*  Video Display Resolution  */
  uint8_t video_resolution_valid;  /**< Must be set to true if video_resolution is being passed */
  ims_video_resolution_enum_v01 video_resolution;
  /**<   Resolution of the video display. Values: \n
      - IMS_SETTINGS_SQCIF_RESOLUTION (0x00) --  SQCIF
      - IMS_SETTINGS_QCIF_RESOLUTION (0x01) --  QCIF
      - IMS_SETTINGS_CIF_RESOLUTION (0x02) --  CIF
      - IMS_SETTINGS_QQVGA_RESOLUTION (0x03) --  QQVGA
      - IMS_SETTINGS_QVGA_RESOLUTION (0x04) --  QVGA
      - IMS_SETTINGS_VGA_RESOLUTION (0x05) --  VGA  */

  /* Optional */
  /*  Video Codec */
  uint8_t video_codec_valid;  /**< Must be set to true if video_codec is being passed */
  ims_settings_video_codec_enum_v01 video_codec;
  /**<   Codec used for the video. Values: \n
      - IMS_SETTINGS_CODEC_MPEG4_XVID (0x00) --  XVID MPEG4 codec
      - IMS_SETTINGS_CODEC_MPEG4_ISO (0x01) --  ISO MPEG4 codec
      - IMS_SETTINGS_CODEC_H263 (0x02) --  H.263 codec
      - IMS_SETTINGS_CODEC_H264 (0x03) --  H.264 codec */
}ims_settings_get_media_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_qipcall_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the qipcall-related configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
       - qmi_error_type  -- Error code. Possible error code values are described in
                        the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  VT Calling Status */
  uint8_t vt_calling_enabled_valid;  /**< Must be set to true if vt_calling_enabled is being passed */
  uint8_t vt_calling_enabled;
  /**<   Values: \n
        -TRUE -- Enable  \n
        -FALSE -- Disable 
    */

  /* Optional */
  /*  Mobile Data Status */
  uint8_t mobile_data_enabled_valid;  /**< Must be set to true if mobile_data_enabled is being passed */
  uint8_t mobile_data_enabled;
  /**<   Values: \n
        -TRUE -- Enable  \n
        -FALSE -- Disable 
    */

  /* Optional */
  /*  Volte Status */
  uint8_t volte_enabled_valid;  /**< Must be set to true if volte_enabled is being passed */
  uint8_t volte_enabled;
  /**<   Values: \n
        -True -- Enable  \n
        -False -- Disable 
    */
}ims_settings_get_qipcall_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_reg_mgr_extended_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the registration manager extended configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   Settings-specific error code is returned when the standard response
       error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  Re-registration Delay */
  uint8_t reregistration_delay_valid;  /**< Must be set to true if reregistration_delay is being passed */
  uint16_t reregistration_delay;
  /**<  IMS re-registration wait time when RAT transition from eHRPD to LTE, in seconds*/
}ims_settings_get_reg_mgr_extended_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_pol_mgr_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the policy manager configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Settings Standard Response Type */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   Settings-specific error code is returned when the standard response
       error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  Policy Manager RAT APN Information Array */
  uint8_t pol_mgr_rat_apn_info_valid;  /**< Must be set to true if pol_mgr_rat_apn_info is being passed */
  ims_settings_pol_man_rat_apn_info_v01 pol_mgr_rat_apn_info[IMS_SETTINGS_POL_MGR_RAT_APN_SIZE_V01];
  /**<  Array of RAT and APN information parameters */

  /* Optional */
  /*  Policy Manager RAT APN FB SP Information Array */
  uint8_t pol_mgr_rat_apn_fb_sp_info_valid;  /**< Must be set to true if pol_mgr_rat_apn_fb_sp_info is being passed */
  ims_settings_pol_mgr_rat_apn_fb_sp_info_v01 pol_mgr_rat_apn_fb_sp_info[IMS_SETTINGS_POL_MGR_RAT_APN_FB_SIZE_V01];
  /**<  Array of RAT_APN and their fall back & service priority information parameters */

  /* Optional */
  /*  Policy Manager Allowed Services over WLAN */
  uint8_t pol_mgr_allowed_services_wlan_valid;  /**< Must be set to true if pol_mgr_allowed_services_wlan is being passed */
  ims_settings_service_type_mask_v01 pol_mgr_allowed_services_wlan;
  /**<   Bit mask indicating the services which are allowed over WLAN */

  /* Optional */
  /*  Policy Manager Add All Feature Tags */
  uint8_t pol_mgr_add_all_fts_valid;  /**< Must be set to true if pol_mgr_add_all_fts is being passed */
  uint8_t pol_mgr_add_all_fts;
  /**<  Whether to add all feature tag list or application */

  /* Optional */
  /*  Policy Manager ACS Priority */
  uint8_t pol_mgr_acs_priority_valid;  /**< Must be set to true if pol_mgr_acs_priority is being passed */
  uint8_t pol_mgr_acs_priority;
  /**<  Priority of ACS values */

  /* Optional */
  /*  Policy Manager ISIM Priority */
  uint8_t pol_mgr_isim_priority_valid;  /**< Must be set to true if pol_mgr_isim_priority is being passed */
  uint8_t pol_mgr_isim_priority;
  /**<  Priority of ISIM values */

  /* Optional */
  /*  Policy Manager NV Priority */
  uint8_t pol_mgr_nv_priority_valid;  /**< Must be set to true if pol_mgr_nv_priority is being passed */
  uint8_t pol_mgr_nv_priority;
  /**<  Priority of Pre Config NV values */

  /* Optional */
  /*  Policy Manager PCO Priority */
  uint8_t pol_mgr_pco_priority_valid;  /**< Must be set to true if pol_mgr_pco_priority is being passed */
  uint8_t pol_mgr_pco_priority;
  /**<  Priority of PCO values */

  /* Optional */
  /*  Policy Manager IMS Service Priority */
  uint8_t pol_mgr_ims_service_status_valid;  /**< Must be set to true if pol_mgr_ims_service_status is being passed */
  ims_settings_service_type_mask_v01 pol_mgr_ims_service_status;
  /**<   Bit mask indicating the services that are enabled in the device */

  /* Optional */
  /*  Policy Manager APN Name List */
  uint8_t pol_mgr_apn_name_valid;  /**< Must be set to true if pol_mgr_apn_name is being passed */
  ims_settings_pol_mgr_apn_name_v01 pol_mgr_apn_name[IMS_SETTINGS_POL_MGR_APN_SIZE_V01];
  /**<  IMS APN name */
}ims_settings_get_pol_mgr_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_sip_read_only_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the SIP read-only-related configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  Timer T4 */
  uint8_t timer_t4_valid;  /**< Must be set to true if timer_t4 is being passed */
  uint32_t timer_t4;
  /**<   Maximum duration, in milliseconds, that a message remains in the network.                                                        
    */

  /* Optional */
  /*  TCP Threshold Value */
  uint8_t tcp_threshold_value_valid;  /**< Must be set to true if tcp_threshold_value is being passed */
  uint16_t tcp_threshold_value;
  /**<   Defines the packet size limiting value, in bytes.                                                        
    */

  /* Optional */
  /*  Compact Form Enabled */
  uint8_t compact_form_enabled_valid;  /**< Must be set to true if compact_form_enabled is being passed */
  uint8_t compact_form_enabled;
  /**<   Indicates whether the SIP compact form is enabled.                                                        
    */

  /* Optional */
  /*  Authentication Scheme */
  uint8_t settings_auth_scheme_valid;  /**< Must be set to true if settings_auth_scheme is being passed */
  ims_settings_config_auth_scheme_enum_v01 settings_auth_scheme;
  /**<   Authentication scheme configuration. Values: \n
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_NONE (0x00) --  AUTHENTICATION SCHEME NONE 
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_DIGEST (0x01) --  AUTHENTICATION SCHEME DIGEST 
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_SAG (0x02) --  AUTHENTICATION SCHEME TOKEN 
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_AKA (0x03) --  AUTHENTICATION SCHEME AKA 
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_CAVE (0x04) --  AUTHENTICATION SCHEME CAVE 
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_AKAV2 (0x05) --  AUTHENTICATION SCHEME AKAv2 
 */

  /* Optional */
  /*  Initial Authorization Type */
  uint8_t settings_initial_auth_config_valid;  /**< Must be set to true if settings_initial_auth_config is being passed */
  ims_settings_config_initial_auth_type_enum_v01 settings_initial_auth_config;
  /**<   Initial authorization type value. Values: \n
      - IMS_SETTINGS_CONFIG_INITIAL_AUTH_NONE (0x00) --  AUTHORIZATION NONE 
      - IMS_SETTINGS_CONFIG_INITIAL_AUTH_AUTHORIZATION (0x01) --  AUTHORIZATION 
      - IMS_SETTINGS_CONFIG_INITIAL_AUTH_PROXY_AUTHORIZATION (0x02) --  PROXY AUTHORIZATION 
 */

  /* Optional */
  /*  Authorization Header Value */
  uint8_t auth_header_value_valid;  /**< Must be set to true if auth_header_value is being passed */
  char auth_header_value[IMS_SETTINGS_CONFIG_PROXY_ROUTE_LEN_V01 + 1];
  /**<   Authorization header value. */

  /* Optional */
  /*  Proxy Route Value */
  uint8_t proxy_route_value_valid;  /**< Must be set to true if proxy_route_value is being passed */
  char proxy_route_value[IMS_SETTINGS_CONFIG_PROXY_ROUTE_LEN_V01 + 1];
  /**<   Route value to be used by the shared configuration. */
}ims_settings_get_sip_read_only_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_network_read_only_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the network read-only-related configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  IPv6 Enabled */
  uint8_t ipv6_enabled_valid;  /**< Must be set to true if ipv6_enabled is being passed */
  uint8_t ipv6_enabled;
  /**<   Indicates whether the IPV6 address is enabled. */

  /* Optional */
  /*  IP Sec Integrity Scheme */
  uint8_t ip_sec_int_scheme_valid;  /**< Must be set to true if ip_sec_int_scheme is being passed */
  ims_settings_integ_algo_mask_v01 ip_sec_int_scheme;
  /**<   Bitmask indicating the integrity algorithm combination. Values: \n
      - IMS_SETTINGS_INTEG_ALGO_HMAC_SHA_1_96 (0x01) --  HMAC-SHA-1-96 algorithm is used for IPSec integrity 
      - IMS_SETTINGS_INTEG_ALGO_HMAC_MD5_96 (0x02) --  HMAC-MD5-96 algorithm is used for IPSec integrity  
 */

  /* Optional */
  /*  IP Sec Encryption Algorithm */
  uint8_t ip_sec_enc_algo_valid;  /**< Must be set to true if ip_sec_enc_algo is being passed */
  ims_settings_encrypt_algo_mask_v01 ip_sec_enc_algo;
  /**<   Bitmask indicating the IPSec encryption algorithm combination. Values: \n
      - IMS_SETTINGS_ENCRYPT_ALGO_NULL (0x01) --  NULL algorithm is used for IPSec encryption 
      - IMS_SETTINGS_ENCRYPT_ALGO_AES_CBC (0x02) --  AES-CBC algorithm is used for IPSec encryption 
      - IMS_SETTINGS_ENCRYPT_ALGO_DES_EDE3_CBC (0x04) --  DES-EDE3-CBC algorithm is used for IPSec encryption  
 */
}ims_settings_get_network_read_only_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_voip_read_only_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the VoIP read-only-related configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  VoIP Configuration Expiration */
  uint8_t voip_config_expires_valid;  /**< Must be set to true if voip_config_expires is being passed */
  uint16_t voip_config_expires;
  /**<   VoIP configuration expiration timer. */

  /* Optional */
  /*  VoIP Session Timer Enabled */
  uint8_t voip_session_timer_enabled_valid;  /**< Must be set to true if voip_session_timer_enabled is being passed */
  uint8_t voip_session_timer_enabled;
  /**<   Indicates whether the VoIP session is timer enabled. */
}ims_settings_get_voip_read_only_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_user_read_only_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the user read-only-related configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  Registration Configuration User Name */
  uint8_t reg_config_userName_valid;  /**< Must be set to true if reg_config_userName is being passed */
  char reg_config_userName[IMS_SETTINGS_REG_CONFIG_USER_NAME_LEN_V01 + 1];
  /**<   Registration configuration user name. */

  /* Optional */
  /*  Registration Configuration Private URI */
  uint8_t reg_config_privateURI_valid;  /**< Must be set to true if reg_config_privateURI is being passed */
  char reg_config_privateURI[IMS_SETTINGS_REG_CONFIG_PRIVATE_URI_LEN_V01 + 1];
  /**<   Registration configuration private URI. */

  /* Optional */
  /*  Registration Configuration Display name */
  uint8_t reg_config_displayName_valid;  /**< Must be set to true if reg_config_displayName is being passed */
  uint16_t reg_config_displayName[IMS_SETTINGS_REG_CONFIG_DISPLAY_NAME_LEN_V01 + 1];
  /**<   Registration configuration display name. */
}ims_settings_get_user_read_only_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_reg_mgr_read_only_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the registration manager read-only-related configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  Registration Configuration Mode */
  uint8_t settings_regmgr_mode_config_valid;  /**< Must be set to true if settings_regmgr_mode_config is being passed */
  ims_settings_regmgr_config_mode_enum_v01 settings_regmgr_mode_config;
  /**<   Registration configuration mode value. Values:
      - IMS_SETTINGS_REGMGR_CONFIG_IETF (0x00) --  IETF Configuration mode
      - IMS_SETTINGS_REGMGR_CONFIG_EARLY_IMS (0x01) --  Early IMS Configuration mode
      - IMS_SETTINGS_REGMGR_CONFIG_IMS (0x02) --  IMS Configuration mode
      - IMS_SETTINGS_REGMGR_CONFIG_IMS_NO_IPSEC (0x03) --  IMS NO IPSec Configuration mode
      - IMS_SETTINGS_REGMGR_CONFIG_IMS_NONE (0x04) --  NO Configuration mode
 */

  /* Optional */
  /*  RegMgr PDP Profile Name */
  uint8_t regmgr_pdp_profilename_valid;  /**< Must be set to true if regmgr_pdp_profilename is being passed */
  char regmgr_pdp_profilename[IMS_SETTINGS_REG_PDPD_PROFILE_NAME_LEN_V01 + 1];
  /**<   Registration manager PDP profile name. */
}ims_settings_get_reg_mgr_read_only_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_rcs_auto_config_read_only_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the RCS automatic configuration read-only-related configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  Device Type */
  uint8_t rcsOnly_device_type_valid;  /**< Must be set to true if rcsOnly_device_type is being passed */
  uint8_t rcsOnly_device_type;
  /**<   RCS device type configuration. */

  /* Optional */
  /*  RCS PDP Profile Name */
  uint8_t rcs_pdp_profilename_valid;  /**< Must be set to true if rcs_pdp_profilename is being passed */
  char rcs_pdp_profilename[IMS_SETTINGS_REG_PDPD_PROFILE_NAME_LEN_V01 + 1];
  /**<   RCS APN profile name.   */

  /* Optional */
  /*  Internet PDP Profile Name */
  uint8_t internet_pdp_profilename_valid;  /**< Must be set to true if internet_pdp_profilename is being passed */
  char internet_pdp_profilename[IMS_SETTINGS_REG_PDPD_PROFILE_NAME_LEN_V01 + 1];
  /**<   Internet APN profile name.       */

  /* Optional */
  /*  PCO Configuration Priority */
  uint8_t pco_config_priority_valid;  /**< Must be set to true if pco_config_priority is being passed */
  uint8_t pco_config_priority;
  /**<   Priority of PCO configuration */

  /* Optional */
  /*  ISIM Configuration Priority */
  uint8_t isim_config_priority_valid;  /**< Must be set to true if isim_config_priority is being passed */
  uint8_t isim_config_priority;
  /**<   Priority of ISIM configuration. */

  /* Optional */
  /*  Preconfiguration Priority */
  uint8_t preconfig_priority_valid;  /**< Must be set to true if preconfig_priority is being passed */
  uint8_t preconfig_priority;
  /**<   Preconfiguration priority. */

  /* Optional */
  /*  Automatic Configuration Priority */
  uint8_t autoconfig_priority_valid;  /**< Must be set to true if autoconfig_priority is being passed */
  uint8_t autoconfig_priority;
  /**<   Automatic configuration priority. */

  /* Optional */
  /*  RCS LTE FT List */
  uint8_t rcs_lte_ft_list_valid;  /**< Must be set to true if rcs_lte_ft_list is being passed */
  char rcs_lte_ft_list[IMS_SETTINGS_RCS_FEATURE_TAG_LIST_LEN_V01 + 1];
  /**<   List of RCS FTs to be supported in the LTE RAT.  */

  /* Optional */
  /*  RCS HSPA FT List */
  uint8_t rcs_hspa_ft_list_valid;  /**< Must be set to true if rcs_hspa_ft_list is being passed */
  char rcs_hspa_ft_list[IMS_SETTINGS_RCS_FEATURE_TAG_LIST_LEN_V01 + 1];
  /**<   List of RCS FTs to be supported in the HSPA RAT.  */

  /* Optional */
  /*  RCS WiFi FT List */
  uint8_t rcs_wifi_ft_list_valid;  /**< Must be set to true if rcs_wifi_ft_list is being passed */
  char rcs_wifi_ft_list[IMS_SETTINGS_RCS_FEATURE_TAG_LIST_LEN_V01 + 1];
  /**<   List of RCS FTs to be supported in the WIFI RAT.      */
}ims_settings_get_rcs_auto_config_read_only_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of 
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ims_settings_get_rcs_imscore_auto_config_read_only_config_req_msg_v01;

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Retrieves the RCS IMS core automatic configuration read-only-related configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
    */

  /* Optional */
  /*  Settings Response */
  uint8_t settings_resp_valid;  /**< Must be set to true if settings_resp is being passed */
  ims_settings_rsp_enum_v01 settings_resp;
  /**<   A settings-specific error code is returned when the standard response
         error type is QMI_ERR_CAUSE_CODE.
    */

  /* Optional */
  /*  RCS Timer T1 */
  uint8_t rcs_timer_t1_valid;  /**< Must be set to true if rcs_timer_t1 is being passed */
  uint32_t rcs_timer_t1;
  /**<   SIP timer 1 is retrieved using RCS automatic configuration. */

  /* Optional */
  /*  RCS Timer T2 */
  uint8_t rcs_timer_t2_valid;  /**< Must be set to true if rcs_timer_t2 is being passed */
  uint32_t rcs_timer_t2;
  /**<   SIP timer 2 is retrieved using RCS automatic configuration. */

  /* Optional */
  /*  RCS Timer T4 */
  uint8_t rcs_timer_t4_valid;  /**< Must be set to true if rcs_timer_t4 is being passed */
  uint32_t rcs_timer_t4;
  /**<   SIP timer 4 is retrieved using RCS automatic configuration. */
}ims_settings_get_rcs_imscore_auto_config_read_only_config_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Request Message; Sets the registration state for various settings service indications 
             for the requesting control points. */
typedef struct {

  /* Optional */
  /*  SIP Configuration */
  uint8_t sip_config_valid;  /**< Must be set to true if sip_config is being passed */
  uint8_t sip_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Registration Manager Configuration */
  uint8_t reg_mgr_config_valid;  /**< Must be set to true if reg_mgr_config is being passed */
  uint8_t reg_mgr_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  SMS Configuration */
  uint8_t sms_config_valid;  /**< Must be set to true if sms_config is being passed */
  uint8_t sms_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  User Configuration */
  uint8_t user_config_valid;  /**< Must be set to true if user_config is being passed */
  uint8_t user_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  VoIP Configuration */
  uint8_t voip_config_valid;  /**< Must be set to true if voip_config is being passed */
  uint8_t voip_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  Presence Configuration */
  uint8_t presence_config_valid;  /**< Must be set to true if presence_config is being passed */
  uint8_t presence_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  Media Configuration */
  uint8_t media_config_valid;  /**< Must be set to true if media_config is being passed */
  uint8_t media_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  QIPcall Configuration */
  uint8_t qipcall_config_valid;  /**< Must be set to true if qipcall_config is being passed */
  uint8_t qipcall_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  SIP Read-only Configuration */
  uint8_t sip_read_only_config_valid;  /**< Must be set to true if sip_read_only_config is being passed */
  uint8_t sip_read_only_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  Network Read-only Configuration */
  uint8_t network_read_only_config_valid;  /**< Must be set to true if network_read_only_config is being passed */
  uint8_t network_read_only_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable 
  */

  /* Optional */
  /*  Registration Manager Extended Configuration */
  uint8_t reg_mgr_extended_config_valid;  /**< Must be set to true if reg_mgr_extended_config is being passed */
  uint8_t reg_mgr_extended_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Policy Manager Configuration */
  uint8_t pol_mgr_config_valid;  /**< Must be set to true if pol_mgr_config is being passed */
  uint8_t pol_mgr_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */
}ims_settings_config_ind_reg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Response Message; Sets the registration state for various settings service indications 
             for the requesting control points. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}ims_settings_config_ind_reg_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the SIP configuration parameters change. */
typedef struct {

  /* Optional */
  /*  SIP Port Number */
  uint8_t sip_local_port_valid;  /**< Must be set to true if sip_local_port is being passed */
  uint16_t sip_local_port;
  /**<   Primary call session control function SIP port number.
    */

  /* Optional */
  /*  SIP Registration Timer */
  uint8_t timer_sip_reg_valid;  /**< Must be set to true if timer_sip_reg is being passed */
  uint32_t timer_sip_reg;
  /**<   Initial SIP registration duration, in seconds, from the UE.
    */

  /* Optional */
  /*  Subscribe Timer */
  uint8_t subscribe_timer_valid;  /**< Must be set to true if subscribe_timer is being passed */
  uint32_t subscribe_timer;
  /**<   Duration, in seconds, of the subscription by the UE for IMS registration 
         notifications.
    */

  /* Optional */
  /*  Timer T1  */
  uint8_t timer_t1_valid;  /**< Must be set to true if timer_t1 is being passed */
  uint32_t timer_t1;
  /**<   RTT estimate, in milliseconds.
    */

  /* Optional */
  /*  Timer T2   */
  uint8_t timer_t2_valid;  /**< Must be set to true if timer_t2 is being passed */
  uint32_t timer_t2;
  /**<   Maximum retransmit interval, in milliseconds, for non-invite requests 
         and invite responses.
    */

  /* Optional */
  /*  Timer TF   */
  uint8_t timer_tf_valid;  /**< Must be set to true if timer_tf is being passed */
  uint32_t timer_tf;
  /**<   Non-invite transaction timeout timer, in milliseconds.
    */

  /* Optional */
  /*  Sigcomp Status */
  uint8_t sigcomp_enabled_valid;  /**< Must be set to true if sigcomp_enabled is being passed */
  uint8_t sigcomp_enabled;
  /**<  
        Values: \n
        -TRUE -- Enabled \n
        -FALSE -- Disabled
    */

  /* Optional */
  /*  Timer TJ */
  uint8_t timer_tj_valid;  /**< Must be set to true if timer_tj is being passed */
  uint16_t timer_tj;
  /**<   Wait time, in milliseconds, for the non-invite request retransmission. 
         If the value exceeds the range of uint16, it is set to 0xFFFF. */

  /* Optional */
  /*  Timer TJ Extended */
  uint8_t timer_tj_ext_valid;  /**< Must be set to true if timer_tj_ext is being passed */
  uint32_t timer_tj_ext;
  /**<   Wait time, in milliseconds, for the non-invite request retransmission. */

  /* Optional */
  /*  Keep Alive Status */
  uint8_t keepalive_enabled_valid;  /**< Must be set to true if keepalive_enabled is being passed */
  uint8_t keepalive_enabled;
  /**<   Values: \n
        -TRUE -- Enable  \n
        -FALSE -- Disable 
    */

  /* Optional */
  /*  NAT-RTO Timer Value */
  uint8_t nat_rto_timer_valid;  /**< Must be set to true if nat_rto_timer is being passed */
  uint32_t nat_rto_timer;
  /**<   Request timeout value, in milliseconds, used in NAT implementation. */
}ims_settings_sip_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the registration manager configuration 
             parameters change. */
typedef struct {

  /* Optional */
  /*  Primary CSCF Port */
  uint8_t regmgr_config_pcscf_port_valid;  /**< Must be set to true if regmgr_config_pcscf_port is being passed */
  uint16_t regmgr_config_pcscf_port;
  /**<   Primary CSCF port. */

  /* Optional */
  /*  CSCF Port */
  uint8_t regmgr_primary_cscf_valid;  /**< Must be set to true if regmgr_primary_cscf is being passed */
  char regmgr_primary_cscf[IMS_SETTINGS_STRING_LEN_MAX_V01 + 1];
  /**<   CSCF port, fully qualified domain name.  */

  /* Optional */
  /*  IMS Test Mode */
  uint8_t ims_test_mode_valid;  /**< Must be set to true if ims_test_mode is being passed */
  uint8_t ims_test_mode;
  /**<  
        Values: \n
        -TRUE -- Enabled \n
        -FALSE -- Disabled
    */
}ims_settings_reg_mgr_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the SMS configuration parameters change. */
typedef struct {

  /* Optional */
  /*  SMS Format */
  uint8_t sms_format_valid;  /**< Must be set to true if sms_format is being passed */
  ims_settings_sms_format_enum_v01 sms_format;
  /**<   Values: 
      - IMS_SETTINGS_SMS_FORMAT_3GPP2 (0) --  3GPP2     
      - IMS_SETTINGS_SMS_FORMAT_3GPP (1) --  3GPP 
 */

  /* Optional */
  /*  SMS Over IP Network Indication Flag */
  uint8_t sms_over_ip_network_indication_valid;  /**< Must be set to true if sms_over_ip_network_indication is being passed */
  uint8_t sms_over_ip_network_indication;
  /**<  
        Values: \n
        -TRUE -- MO SMS turned on \n
        -FALSE -- MO SMS turned off
    */

  /* Optional */
  /*  Phone Context Universal Resource Identifier */
  uint8_t phone_context_uri_valid;  /**< Must be set to true if phone_context_uri is being passed */
  char phone_context_uri[IMS_SETTINGS_STRING_LEN_MAX_V01 + 1];
  /**<   Phone context universal resource identifier. */
}ims_settings_sms_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the user configuration parameters change. */
typedef struct {

  /* Optional */
  /*  IMS Domain Name */
  uint8_t ims_domain_valid;  /**< Must be set to true if ims_domain is being passed */
  char ims_domain[IMS_SETTINGS_STRING_LEN_MAX_V01 + 1];
  /**<   IMS domain name. */
}ims_settings_user_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the VoIP configuration parameters change. */
typedef struct {

  /* Optional */
  /*  Session Duration */
  uint8_t session_expiry_timer_valid;  /**< Must be set to true if session_expiry_timer is being passed */
  uint16_t session_expiry_timer;
  /**<   Session duration, in seconds. */

  /* Optional */
  /*  Minimum Session Timer */
  uint8_t min_session_expiry_valid;  /**< Must be set to true if min_session_expiry is being passed */
  uint16_t min_session_expiry;
  /**<   Minimum allowed value, in seconds, for the session timer. */

  /* Optional */
  /*  Enable AMR WB  */
  uint8_t amr_wb_enable_valid;  /**< Must be set to true if amr_wb_enable is being passed */
  uint8_t amr_wb_enable;
  /**<   Flag indicating AMR WB audio. \n
        Values: \n
        -TRUE -- Enabled \n
        -FALSE -- Disabled
    */

  /* Optional */
  /*  Enable SCR AMR NB */
  uint8_t scr_amr_enable_valid;  /**< Must be set to true if scr_amr_enable is being passed */
  uint8_t scr_amr_enable;
  /**<   Flag indicating SCR for AMR NB audio. \n
        Values: \n
        -TRUE -- Enabled \n
        -FALSE -- Disabled
    */

  /* Optional */
  /*  Enable SCR AMR WB  */
  uint8_t scr_amr_wb_enable_valid;  /**< Must be set to true if scr_amr_wb_enable is being passed */
  uint8_t scr_amr_wb_enable;
  /**<   Flag indicating SCR for AMR WB audio. \n
        Values: \n
        -TRUE -- Enabled \n
        -FALSE -- Disabled
    */

  /* Optional */
  /*  AMR NB Mode */
  uint8_t amr_mode_valid;  /**< Must be set to true if amr_mode is being passed */
  uint8_t amr_mode;
  /**<   Bitmask indicating AMR NB modes. \n
        Values: \n
        - 0x1 -- 4.75 kbps \n
        - 0x2 -- 5.15 kbps \n
        - 0x4 -- 5.9 kbps \n
        - 0x8 -- 6.17 kbps \n
        - 0x10 -- 7.4 kbps \n
        - 0x20 -- 7.95 kbps \n
        - 0x40 -- 10.2 kbps \n
        - 0x80 -- 12.2 kbps
    */

  /* Optional */
  /*  AMR WB Mode */
  uint8_t amr_wb_mode_valid;  /**< Must be set to true if amr_wb_mode is being passed */
  uint16_t amr_wb_mode;
  /**<   Bitmask indicating AMR WB modes. \n
        Values: \n
        - 0x1 -- 6.60 kbps \n
        - 0x2 -- 8.85 kbps \n
        - 0x4 -- 12.65 kbps \n
        - 0x8 -- 14.25 kbps \n
        - 0x10 -- 15.85 kbps \n
        - 0x20 -- 18.25 kbps \n
        - 0x40 -- 19.85 kbps \n
        - 0x80 -- 23.05 kbps \n
        - 0x100 -- 23.85 kbps
    */

  /* Optional */
  /*  AMR NB Octet Aligned */
  uint8_t amr_octet_align_valid;  /**< Must be set to true if amr_octet_align is being passed */
  uint8_t amr_octet_align;
  /**<   Flag indicating if the octet is aligned for AMR NB audio. \n
        Values: \n
        -TRUE -- Octet aligned \n
        -FALSE -- Octet not aligned, Bandwidth Efficient mode
    */

  /* Optional */
  /*  AMR WB Octet Aligned */
  uint8_t amr_wb_octet_align_valid;  /**< Must be set to true if amr_wb_octet_align is being passed */
  uint8_t amr_wb_octet_align;
  /**<   Flag indicating if the octet is aligned for AMR WB audio. \n
        Values: \n
        -TRUE -- Octet aligned \n
        -FALSE -- Octet not aligned, Bandwidth Efficient mode
    */

  /* Optional */
  /*  Ringing Timer Duration */
  uint8_t ringing_timer_valid;  /**< Must be set to true if ringing_timer is being passed */
  uint16_t ringing_timer;
  /**<   Duration, in seconds, of the ringing timer. The ringing timer 
         starts on the ringing event. If the call is not answered within
         the duration of this timer, the call is disconnected.
    */

  /* Optional */
  /*  Ringback Timer Duration */
  uint8_t ringback_timer_valid;  /**< Must be set to true if ringback_timer is being passed */
  uint16_t ringback_timer;
  /**<   Duration, in seconds, of the ringback timers. The ringback timer 
         starts on the ringback event. If the call is not answered within
         the duration of this timer, the call is disconnected.
    */

  /* Optional */
  /*  RTP/RTCP Inactivity Timer Duration */
  uint8_t rtp_rtcp_inactivity_timer_valid;  /**< Must be set to true if rtp_rtcp_inactivity_timer is being passed */
  uint16_t rtp_rtcp_inactivity_timer;
  /**<   Duration, in seconds, of the RTP/RTCP inactivity timer. If no 
         RTP/RTCP packet is received prior to the expiration of this timer, 
         the call is disconnected.
    */
}ims_settings_voip_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the presence-related configuration parameters change. */
typedef struct {

  /* Optional */
  /*  Publish Timer  */
  uint8_t publish_expiry_timer_valid;  /**< Must be set to true if publish_expiry_timer is being passed */
  uint32_t publish_expiry_timer;
  /**<   Publish timer, in seconds, when publish is sent on an IMS network using 4G
         radio access technology.
    */

  /* Optional */
  /*  Publish Extended Timer  */
  uint8_t publish_extended_expiry_timer_valid;  /**< Must be set to true if publish_extended_expiry_timer is being passed */
  uint32_t publish_extended_expiry_timer;
  /**<   Publish extended timer, in seconds, when publish is sent
         on an IMS network using non-4G radio access technology, 
         or when in Airplane Power-Down mode using 4G radio access technology.
    */

  /* Optional */
  /*  Minimum Publish Interval */
  uint8_t minimum_publish_interval_valid;  /**< Must be set to true if minimum_publish_interval is being passed */
  uint32_t minimum_publish_interval;
  /**<   Duration of time, in seconds, between successive publish requests.
    */

  /* Optional */
  /*  Capability Poll List Subscription Expiry Timer */
  uint8_t capability_poll_list_subscription_expiry_timer_valid;  /**< Must be set to true if capability_poll_list_subscription_expiry_timer is being passed */
  uint32_t capability_poll_list_subscription_expiry_timer;
  /**<   Timer, in seconds, for the list subscribe request.
    */

  /* Optional */
  /*  Discovery Capability Enabled */
  uint8_t capability_discovery_enable_valid;  /**< Must be set to true if capability_discovery_enable is being passed */
  uint8_t capability_discovery_enable;
  /**<   Flag indicating whether or not discovery capability is enabled.
        Values: \n
        -TRUE -- Presence publishes/subscribes and processes any notifications received \n
        -FALSE -- Presence does not publish/subscribe and 
                  ignores any notification received
    */

  /* Optional */
  /*  Cache Capability Expiration */
  uint8_t capabilites_cache_expiration_valid;  /**< Must be set to true if capabilites_cache_expiration is being passed */
  uint32_t capabilites_cache_expiration;
  /**<   Duration of time, in seconds, for which the retrieved capability is considered valid.
    */

  /* Optional */
  /*  Cache Availability Expiration */
  uint8_t availability_cache_expiration_valid;  /**< Must be set to true if availability_cache_expiration is being passed */
  uint32_t availability_cache_expiration;
  /**<   Duration of time, in seconds, for which the retrieved availability is considered valid.
    */

  /* Optional */
  /*  Capability Poll Interval */
  uint8_t capability_poll_interval_valid;  /**< Must be set to true if capability_poll_interval is being passed */
  uint32_t capability_poll_interval;
  /**<   Duration of time, in seconds, for which the retrieved availability is considered valid.
    */

  /* Optional */
  /*  Maximum Subscription List Entries */
  uint8_t max_subcription_list_entries_valid;  /**< Must be set to true if max_subcription_list_entries is being passed */
  uint32_t max_subcription_list_entries;
  /**<   Maximum number of entries that can be kept in the list subscription.
    */

  /* Optional */
  /*  VoLTE User Opted In Status */
  uint8_t volte_user_opted_in_status_valid;  /**< Must be set to true if volte_user_opted_in_status is being passed */
  uint8_t volte_user_opted_in_status;
  /**<   Flag indicating whether or not VoLTE service is accepted by the user. 
         Values: \n
         -TRUE -- Accepted \n
         -FA:SE -- Not accepted
    */

  /* Optional */
  /*  Last Published ETAG */
  uint8_t last_publish_etag_valid;  /**< Must be set to true if last_publish_etag is being passed */
  char last_publish_etag[IMS_SETTINGS_PRESENCE_PUBLISH_ETAG_LEN_V01 + 1];
  /**<   Last published ETAG. */

  /* Optional */
  /*  Last Published Time */
  uint8_t last_published_time_valid;  /**< Must be set to true if last_published_time is being passed */
  uint32_t last_published_time;
  /**<   Last published time. */

  /* Optional */
  /*  Last Negotiated Published Expire */
  uint8_t last_negotiated_published_expire_valid;  /**< Must be set to true if last_negotiated_published_expire is being passed */
  uint32_t last_negotiated_published_expire;
  /**<   Last negotiated published expire, in seconds. */

  /* Optional */
  /*  GZIP Enabled */
  uint8_t gzip_enabled_valid;  /**< Must be set to true if gzip_enabled is being passed */
  uint8_t gzip_enabled;
  /**<   Flag indicating whether GZIP compression enabled. 
         Values: \n
         -True -- Enabled \n
         -False -- Disabled
                  */

  /* Optional */
  /*  Presence Notification Wait Duration */
  uint8_t presence_notify_wait_duration_valid;  /**< Must be set to true if presence_notify_wait_duration is being passed */
  uint16_t presence_notify_wait_duration;
  /**<   Presence notification wait duration, in seconds. */

  /* Optional */
  /*  Publish Error Recovery Timer */
  uint8_t publish_error_recovery_timer_valid;  /**< Must be set to true if publish_error_recovery_timer is being passed */
  uint32_t publish_error_recovery_timer;
  /**<   Publish error recovery timer, in seconds. */
}ims_settings_presence_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the media-related configuration parameters change. */
typedef struct {

  /* Optional */
  /*  H.264 Profile */
  uint8_t h264_profile_valid;  /**< Must be set to true if h264_profile is being passed */
  ims_settings_h264_profile_enum_v01 h264_profile;
  /**<   Profile used for H.264 codec. Values: \n
      - IMS_SETTINGS_H264_PROFILE_BASELINE (0x00) --  Baseline profile
      - IMS_SETTINGS_H264_PROFILE_MAIN (0x01) --  Main profile
      - IMS_SETTINGS_H264_PROFILE_EXTENDED (0x02) --  Extended profile
      - IMS_SETTINGS_H264_PROFILE_HIGH (0x03) --  High profile */

  /* Optional */
  /*  H.264 Level */
  uint8_t h264_level_valid;  /**< Must be set to true if h264_level is being passed */
  ims_settings_h264_level_enum_v01 h264_level;
  /**<   Level used for H.264 codec. Values: \n
      - IMS_SETTINGS_H264_LEVEL1 (0x00) --  Level 1
      - IMS_SETTINGS_H264_LEVEL1B (0x01) --  Level 1b
      - IMS_SETTINGS_H264_LEVEL11 (0x02) --  Level 1.1
      - IMS_SETTINGS_H264_LEVEL12 (0x03) --  Level 1.2
      - IMS_SETTINGS_H264_LEVEL13 (0x04) --  Level 1.3
      - IMS_SETTINGS_H264_LEVEL2 (0x05) --  Level 2
      - IMS_SETTINGS_H264_LEVEL21 (0x06) --  Level 2.1
      - IMS_SETTINGS_H264_LEVEL22 (0x07) --  Level 2.2
      - IMS_SETTINGS_H264_LEVEL3 (0x08) --  Level 3
      - IMS_SETTINGS_H264_LEVEL31 (0x09) --  Level 3.1
      - IMS_SETTINGS_H264_LEVEL32 (0x0A) --  Level 3.2
      - IMS_SETTINGS_H264_LEVEL4 (0x0B) --  Level 4
      - IMS_SETTINGS_H264_LEVEL41 (0x0C) --  Level 4.1
      - IMS_SETTINGS_H264_LEVEL42 (0x0D) --  Level 4.2
      - IMS_SETTINGS_H264_LEVEL5 (0x0E) --  Level 5
      - IMS_SETTINGS_H264_LEVEL51 (0x0F) --  Level 5.1 */

  /* Optional */
  /*  Video Bitrate */
  uint8_t video_bitrate_valid;  /**< Must be set to true if video_bitrate is being passed */
  uint16_t video_bitrate;
  /**<   Bitrate of the video, in kbps. */

  /* Optional */
  /*  Video Refresh Rate */
  uint8_t video_frames_per_second_valid;  /**< Must be set to true if video_frames_per_second is being passed */
  uint8_t video_frames_per_second;
  /**<   Video refresh rate, in frames per second. */

  /* Optional */
  /*  Video Display Resolution  */
  uint8_t video_resolution_valid;  /**< Must be set to true if video_resolution is being passed */
  ims_video_resolution_enum_v01 video_resolution;
  /**<   Resolution of the video display. Values: \n
      - IMS_SETTINGS_SQCIF_RESOLUTION (0x00) --  SQCIF
      - IMS_SETTINGS_QCIF_RESOLUTION (0x01) --  QCIF
      - IMS_SETTINGS_CIF_RESOLUTION (0x02) --  CIF
      - IMS_SETTINGS_QQVGA_RESOLUTION (0x03) --  QQVGA
      - IMS_SETTINGS_QVGA_RESOLUTION (0x04) --  QVGA
      - IMS_SETTINGS_VGA_RESOLUTION (0x05) --  VGA  */

  /* Optional */
  /*  Video Codec */
  uint8_t video_codec_valid;  /**< Must be set to true if video_codec is being passed */
  ims_settings_video_codec_enum_v01 video_codec;
  /**<   Codec used for the video. Values: \n
      - IMS_SETTINGS_CODEC_MPEG4_XVID (0x00) --  XVID MPEG4 codec
      - IMS_SETTINGS_CODEC_MPEG4_ISO (0x01) --  ISO MPEG4 codec
      - IMS_SETTINGS_CODEC_H263 (0x02) --  H.263 codec
      - IMS_SETTINGS_CODEC_H264 (0x03) --  H.264 codec */
}ims_settings_media_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the qipcall-related configuration parameters change. */
typedef struct {

  /* Optional */
  /*  VT Calling Status */
  uint8_t vt_calling_enabled_valid;  /**< Must be set to true if vt_calling_enabled is being passed */
  uint8_t vt_calling_enabled;
  /**<   Values: \n
        -TRUE -- Enable  \n
        -FALSE -- Disable 
    */

  /* Optional */
  /*  Mobile Data Status */
  uint8_t mobile_data_enabled_valid;  /**< Must be set to true if mobile_data_enabled is being passed */
  uint8_t mobile_data_enabled;
  /**<   Values: \n
        -TRUE -- Enable  \n
        -FALSE -- Disable 
    */

  /* Optional */
  /*  Volte Status */
  uint8_t volte_enabled_valid;  /**< Must be set to true if volte_enabled is being passed */
  uint8_t volte_enabled;
  /**<   Values: \n
        -True -- Enable  \n
        -False -- Disable 
    */
}ims_settings_qipcall_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the registration manager extended configuration 
             parameters change. */
typedef struct {

  /* Optional */
  /*  Re-registration Delay */
  uint8_t reregistration_delay_valid;  /**< Must be set to true if reregistration_delay is being passed */
  uint16_t reregistration_delay;
  /**<  IMS re-registration wait time when RAT transition from eHRPD to LTE, in seconds*/
}ims_settings_reg_mgr_extended_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the policy manager configuration 
             parameters change. */
typedef struct {

  /* Optional */
  /*  Policy Manager RAT APN Information Array */
  uint8_t pol_mgr_rat_apn_info_valid;  /**< Must be set to true if pol_mgr_rat_apn_info is being passed */
  ims_settings_pol_man_rat_apn_info_v01 pol_mgr_rat_apn_info[IMS_SETTINGS_POL_MGR_RAT_APN_SIZE_V01];
  /**<  Array of RAT and APN information parameters */

  /* Optional */
  /*  Policy Manager RAT APN Fall Back and Service Priority Information Array */
  uint8_t pol_mgr_rat_apn_fb_sp_info_valid;  /**< Must be set to true if pol_mgr_rat_apn_fb_sp_info is being passed */
  ims_settings_pol_mgr_rat_apn_fb_sp_info_v01 pol_mgr_rat_apn_fb_sp_info[IMS_SETTINGS_POL_MGR_RAT_APN_FB_SIZE_V01];
  /**<  Array of RAT_APN and their fall back & service priority information parameters */

  /* Optional */
  /*  Policy Manager Allowed Services over WLAN */
  uint8_t pol_mgr_allowed_services_wlan_valid;  /**< Must be set to true if pol_mgr_allowed_services_wlan is being passed */
  ims_settings_service_type_mask_v01 pol_mgr_allowed_services_wlan;
  /**<   Bit mask indicating the services which are allowed over WLAN */

  /* Optional */
  /*  Policy Manager Add All Feature Tags */
  uint8_t pol_mgr_add_all_fts_valid;  /**< Must be set to true if pol_mgr_add_all_fts is being passed */
  uint8_t pol_mgr_add_all_fts;
  /**<  Whether to add all feature tag list or application */

  /* Optional */
  /*  Policy Manager ACS Priority */
  uint8_t pol_mgr_acs_priority_valid;  /**< Must be set to true if pol_mgr_acs_priority is being passed */
  uint8_t pol_mgr_acs_priority;
  /**<  Priority of ACS values */

  /* Optional */
  /*  Policy Manager ISIM Priority */
  uint8_t pol_mgr_isim_priority_valid;  /**< Must be set to true if pol_mgr_isim_priority is being passed */
  uint8_t pol_mgr_isim_priority;
  /**<  Priority of ISIM values */

  /* Optional */
  /*  Policy Manager NV Priority */
  uint8_t pol_mgr_nv_priority_valid;  /**< Must be set to true if pol_mgr_nv_priority is being passed */
  uint8_t pol_mgr_nv_priority;
  /**<  Priority of pre-configuration NV values */

  /* Optional */
  /*  Policy Manager PCO Priority */
  uint8_t pol_mgr_pco_priority_valid;  /**< Must be set to true if pol_mgr_pco_priority is being passed */
  uint8_t pol_mgr_pco_priority;
  /**<  Priority of PCO values */

  /* Optional */
  /*  Policy Manager IMS Service Priority */
  uint8_t pol_mgr_ims_service_status_valid;  /**< Must be set to true if pol_mgr_ims_service_status is being passed */
  ims_settings_service_type_mask_v01 pol_mgr_ims_service_status;
  /**<   Bit mask indicating the services that are enabled in the device */

  /* Optional */
  /*  Policy Manager APN Name List */
  uint8_t pol_mgr_apn_name_valid;  /**< Must be set to true if pol_mgr_apn_name is being passed */
  ims_settings_pol_mgr_apn_name_v01 pol_mgr_apn_name[IMS_SETTINGS_POL_MGR_APN_SIZE_V01];
  /**<  IMS APN name */
}ims_settings_pol_mgr_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the SIP read-only-related configuration parameters change. */
typedef struct {

  /* Optional */
  /*  Timer T4 */
  uint8_t timer_t4_valid;  /**< Must be set to true if timer_t4 is being passed */
  uint32_t timer_t4;
  /**<   Maximum duration, in milliseconds, that a message remains in the network.                                           
    */

  /* Optional */
  /*  TCP Threshold Value */
  uint8_t tcp_threshold_value_valid;  /**< Must be set to true if tcp_threshold_value is being passed */
  uint16_t tcp_threshold_value;
  /**<   Defines the packet size limiting value, in bytes.                                                         
    */

  /* Optional */
  /*  Compact Form Enabled */
  uint8_t compact_form_enabled_valid;  /**< Must be set to true if compact_form_enabled is being passed */
  uint8_t compact_form_enabled;
  /**<   Indicates whether the SIP compact form is enabled.                                                        
    */

  /* Optional */
  /*  Authentication Scheme */
  uint8_t settings_auth_scheme_valid;  /**< Must be set to true if settings_auth_scheme is being passed */
  ims_settings_config_auth_scheme_enum_v01 settings_auth_scheme;
  /**<   Authentication scheme configuration. Values: \n
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_NONE (0x00) --  AUTHENTICATION SCHEME NONE 
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_DIGEST (0x01) --  AUTHENTICATION SCHEME DIGEST 
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_SAG (0x02) --  AUTHENTICATION SCHEME TOKEN 
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_AKA (0x03) --  AUTHENTICATION SCHEME AKA 
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_CAVE (0x04) --  AUTHENTICATION SCHEME CAVE 
      - IMS_SETTINGS_CONFIG_AUTH_SCHEME_AKAV2 (0x05) --  AUTHENTICATION SCHEME AKAv2 
 */

  /* Optional */
  /*  Initial Authorization Type */
  uint8_t settings_initial_auth_config_valid;  /**< Must be set to true if settings_initial_auth_config is being passed */
  ims_settings_config_initial_auth_type_enum_v01 settings_initial_auth_config;
  /**<   Initial authorization type value. Values: \n
      - IMS_SETTINGS_CONFIG_INITIAL_AUTH_NONE (0x00) --  AUTHORIZATION NONE 
      - IMS_SETTINGS_CONFIG_INITIAL_AUTH_AUTHORIZATION (0x01) --  AUTHORIZATION 
      - IMS_SETTINGS_CONFIG_INITIAL_AUTH_PROXY_AUTHORIZATION (0x02) --  PROXY AUTHORIZATION 
 */

  /* Optional */
  /*  Authorization Header Value */
  uint8_t auth_header_value_valid;  /**< Must be set to true if auth_header_value is being passed */
  char auth_header_value[IMS_SETTINGS_CONFIG_PROXY_ROUTE_LEN_V01 + 1];
  /**<   Authorization header value. */

  /* Optional */
  /*  Proxy Route Value */
  uint8_t proxy_route_value_valid;  /**< Must be set to true if proxy_route_value is being passed */
  char proxy_route_value[IMS_SETTINGS_CONFIG_PROXY_ROUTE_LEN_V01 + 1];
  /**<   Route value to be used by the shared configuration. */
}ims_settings_sip_read_only_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imss_qmi_messages
    @{
  */
/** Indication Message; Indicates when the network read-only-related configuration 
             parameters change. */
typedef struct {

  /* Optional */
  /*  IPv6 Enabled */
  uint8_t ipv6_enabled_valid;  /**< Must be set to true if ipv6_enabled is being passed */
  uint8_t ipv6_enabled;
  /**<   Indicates whether the IPv6 address is enabled. */

  /* Optional */
  /*  IP Sec Integrity Scheme */
  uint8_t ip_sec_int_scheme_valid;  /**< Must be set to true if ip_sec_int_scheme is being passed */
  ims_settings_integ_algo_mask_v01 ip_sec_int_scheme;
  /**<   Bitmask indicating the integrity algorithm combination. Values: \n
      - IMS_SETTINGS_INTEG_ALGO_HMAC_SHA_1_96 (0x01) --  HMAC-SHA-1-96 algorithm is used for IPSec integrity 
      - IMS_SETTINGS_INTEG_ALGO_HMAC_MD5_96 (0x02) --  HMAC-MD5-96 algorithm is used for IPSec integrity  
 */

  /* Optional */
  /*  IP Sec Encryption Algorithm */
  uint8_t ip_sec_enc_algo_valid;  /**< Must be set to true if ip_sec_enc_algo is being passed */
  ims_settings_encrypt_algo_mask_v01 ip_sec_enc_algo;
  /**<   Bitmask indicating the IPSec encryption algorithm combination. Values: \n
      - IMS_SETTINGS_ENCRYPT_ALGO_NULL (0x01) --  NULL algorithm is used for IPSec encryption 
      - IMS_SETTINGS_ENCRYPT_ALGO_AES_CBC (0x02) --  AES-CBC algorithm is used for IPSec encryption 
      - IMS_SETTINGS_ENCRYPT_ALGO_DES_EDE3_CBC (0x04) --  DES-EDE3-CBC algorithm is used for IPSec encryption  
 */
}ims_settings_network_read_only_config_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup imss_qmi_msg_ids
    @{
  */
#define QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_IMS_SETTINGS_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_IMS_SETTINGS_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_IMS_SETTINGS_SET_SIP_CONFIG_REQ_V01 0x0020
#define QMI_IMS_SETTINGS_SET_SIP_CONFIG_RSP_V01 0x0020
#define QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_REQ_V01 0x0021
#define QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_RSP_V01 0x0021
#define QMI_IMS_SETTINGS_SET_SMS_CONFIG_REQ_V01 0x0022
#define QMI_IMS_SETTINGS_SET_SMS_CONFIG_RSP_V01 0x0022
#define QMI_IMS_SETTINGS_SET_USER_CONFIG_REQ_V01 0x0023
#define QMI_IMS_SETTINGS_SET_USER_CONFIG_RSP_V01 0x0023
#define QMI_IMS_SETTINGS_SET_VOIP_CONFIG_REQ_V01 0x0024
#define QMI_IMS_SETTINGS_SET_VOIP_CONFIG_RSP_V01 0x0024
#define QMI_IMS_SETTINGS_GET_SIP_CONFIG_REQ_V01 0x0025
#define QMI_IMS_SETTINGS_GET_SIP_CONFIG_RSP_V01 0x0025
#define QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_REQ_V01 0x0026
#define QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_RSP_V01 0x0026
#define QMI_IMS_SETTINGS_GET_SMS_CONFIG_REQ_V01 0x0027
#define QMI_IMS_SETTINGS_GET_SMS_CONFIG_RSP_V01 0x0027
#define QMI_IMS_SETTINGS_GET_USER_CONFIG_REQ_V01 0x0028
#define QMI_IMS_SETTINGS_GET_USER_CONFIG_RSP_V01 0x0028
#define QMI_IMS_SETTINGS_GET_VOIP_CONFIG_REQ_V01 0x0029
#define QMI_IMS_SETTINGS_GET_VOIP_CONFIG_RSP_V01 0x0029
#define QMI_IMS_SETTINGS_CONFIG_IND_REG_REQ_V01 0x002A
#define QMI_IMS_SETTINGS_CONFIG_IND_REG_RSP_V01 0x002A
#define QMI_IMS_SETTINGS_SIP_CONFIG_IND_V01 0x002B
#define QMI_IMS_SETTINGS_REG_MGR_CONFIG_IND_V01 0x002C
#define QMI_IMS_SETTINGS_SMS_CONFIG_IND_V01 0x002D
#define QMI_IMS_SETTINGS_USER_CONFIG_IND_V01 0x002E
#define QMI_IMS_SETTINGS_VOIP_CONFIG_IND_V01 0x002F
#define QMI_IMS_SETTINGS_SET_PRESENCE_CONFIG_REQ_V01 0x0030
#define QMI_IMS_SETTINGS_SET_PRESENCE_CONFIG_RSP_V01 0x0030
#define QMI_IMS_SETTINGS_GET_PRESENCE_CONFIG_REQ_V01 0x0031
#define QMI_IMS_SETTINGS_GET_PRESENCE_CONFIG_RSP_V01 0x0031
#define QMI_IMS_SETTINGS_PRESENCE_CONFIG_IND_V01 0x0032
#define QMI_IMS_SETTINGS_SET_MEDIA_CONFIG_REQ_V01 0x0033
#define QMI_IMS_SETTINGS_SET_MEDIA_CONFIG_RSP_V01 0x0033
#define QMI_IMS_SETTINGS_GET_MEDIA_CONFIG_REQ_V01 0x0034
#define QMI_IMS_SETTINGS_GET_MEDIA_CONFIG_RSP_V01 0x0034
#define QMI_IMS_SETTINGS_MEDIA_CONFIG_IND_V01 0x0035
#define QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_REQ_V01 0x0036
#define QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_RSP_V01 0x0036
#define QMI_IMS_SETTINGS_GET_QIPCALL_CONFIG_REQ_V01 0x0037
#define QMI_IMS_SETTINGS_GET_QIPCALL_CONFIG_RSP_V01 0x0037
#define QMI_IMS_SETTINGS_QIPCALL_CONFIG_IND_V01 0x0038
#define QMI_IMS_SETTINGS_GET_SIP_READ_ONLY_CONFIG_REQ_V01 0x0039
#define QMI_IMS_SETTINGS_GET_SIP_READ_ONLY_CONFIG_RSP_V01 0x0039
#define QMI_IMS_SETTINGS_SIP_READ_ONLY_CONFIG_IND_V01 0x003A
#define QMI_IMS_SETTINGS_GET_NETWORK_READ_ONLY_CONFIG_REQ_V01 0x003D
#define QMI_IMS_SETTINGS_GET_NETWORK_READ_ONLY_CONFIG_RSP_V01 0x003D
#define QMI_IMS_SETTINGS_NETWORK_READ_ONLY_CONFIG_IND_V01 0x003E
#define QMI_IMS_SETTINGS_GET_VOIP_READ_ONLY_CONFIG_REQ_V01 0x003F
#define QMI_IMS_SETTINGS_GET_VOIP_READ_ONLY_CONFIG_RSP_V01 0x003F
#define QMI_IMS_SETTINGS_GET_USER_READ_ONLY_CONFIG_REQ_V01 0x0040
#define QMI_IMS_SETTINGS_GET_USER_READ_ONLY_CONFIG_RSP_V01 0x0040
#define QMI_IMS_SETTINGS_GET_REG_MGR_READ_ONLY_CONFIG_REQ_V01 0x0041
#define QMI_IMS_SETTINGS_GET_REG_MGR_READ_ONLY_CONFIG_RSP_V01 0x0041
#define QMI_IMS_SETTINGS_GET_RCS_AUTO_CONFIG_READ_ONLY_CONFIG_REQ_V01 0x0042
#define QMI_IMS_SETTINGS_GET_RCS_AUTO_CONFIG_READ_ONLY_CONFIG_RSP_V01 0x0042
#define QMI_IMS_SETTINGS_GET_RCS_IMSCORE_AUTO_CONFIG_READ_ONLY_CONFIG_REQ_V01 0x0043
#define QMI_IMS_SETTINGS_GET_RCS_IMSCORE_AUTO_CONFIG_READ_ONLY_CONFIG_RSP_V01 0x0043
#define QMI_IMS_SETTINGS_SET_REG_MGR_EXTENDED_CONFIG_REQ_V01 0x0044
#define QMI_IMS_SETTINGS_SET_REG_MGR_EXTENDED_CONFIG_RSP_V01 0x0044
#define QMI_IMS_SETTINGS_GET_REG_MGR_EXTENDED_CONFIG_REQ_V01 0x0045
#define QMI_IMS_SETTINGS_GET_REG_MGR_EXTENDED_CONFIG_RSP_V01 0x0045
#define QMI_IMS_SETTINGS_REG_MGR_EXTENDED_CONFIG_IND_V01 0x0046
#define QMI_IMS_SETTINGS_SET_POL_MGR_CONFIG_REQ_V01 0x0047
#define QMI_IMS_SETTINGS_SET_POL_MGR_CONFIG_RSP_V01 0x0047
#define QMI_IMS_SETTINGS_GET_POL_MGR_CONFIG_REQ_V01 0x0048
#define QMI_IMS_SETTINGS_GET_POL_MGR_CONFIG_RSP_V01 0x0048
#define QMI_IMS_SETTINGS_POL_MGR_CONFIG_IND_V01 0x0049
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro imss_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type imss_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define imss_get_service_object_v01( ) \
          imss_get_service_object_internal_v01( \
            IMSS_V01_IDL_MAJOR_VERS, IMSS_V01_IDL_MINOR_VERS, \
            IMSS_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

