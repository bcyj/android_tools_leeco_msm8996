#ifndef IMSVT_SERVICE_H
#define IMSVT_SERVICE_H
/**
  @file ip_multimedia_subsystem_video_telephony_v01.h
  
  @brief This is the public header file which defines the imsvt service Data structures.

  This header file defines the types and structures that were defined in 
  imsvt. It contains the constant values defined, enums, structures,
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

  $Header: //source/qcom/qct/interfaces/qmi/imsvt/main/latest/api/ip_multimedia_subsystem_video_telephony_v01.h#6 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.5 
   It was generated on: Thu Sep 27 2012
   From IDL File: ip_multimedia_subsystem_video_telephony_v01.idl */

/** @defgroup imsvt_qmi_consts Constant values defined in the IDL */
/** @defgroup imsvt_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup imsvt_qmi_enums Enumerated types used in QMI messages */
/** @defgroup imsvt_qmi_messages Structures sent as QMI messages */
/** @defgroup imsvt_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup imsvt_qmi_accessor Accessor for QMI service object */
/** @defgroup imsvt_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup imsvt_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define IMSVT_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define IMSVT_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define IMSVT_V01_IDL_TOOL_VERS 0x05
/** Maximum Defined Message ID */
#define IMSVT_V01_MAX_MESSAGE_ID 0x0030;
/** 
    @} 
  */


/** @addtogroup imsvt_qmi_consts 
    @{ 
  */
#define IMSVT_RESPONSE_DETAILS_MAX_V01 128
#define IMSVT_VOL_HEADER_MAX_V01 255
#define IMSVT_NAL_HEADER_MAX_V01 255
#define IMSVT_IP_ADDR_MAX_V01 40
/**
    @}
  */

/** @addtogroup imsvt_qmi_enums
    @{
  */
typedef enum {
  IMSVT_VIDEO_CODEC_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSVT_CODEC_MPEG4_XVID_V01 = 0x00, 
  IMSVT_CODEC_MPEG4_ISO_V01 = 0x01, 
  IMSVT_CODEC_H263_V01 = 0x02, 
  IMSVT_CODEC_H264_V01 = 0x03, 
  IMSVT_VIDEO_CODEC_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsvt_video_codec_enum_v01;
/**
    @}
  */

/** @addtogroup imsvt_qmi_enums
    @{
  */
typedef enum {
  IMSVT_H264_PROFILE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSVT_H264_PROFILE_BASELINE_V01 = 0x00, 
  IMSVT_H264_PROFILE_MAIN_V01 = 0x01, 
  IMSVT_H264_PROFILE_EXTENDED_V01 = 0x02, 
  IMSVT_H264_PROFILE_HIGH_V01 = 0x03, 
  IMSVT_H264_PROFILE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsvt_h264_profile_enum_v01;
/**
    @}
  */

/** @addtogroup imsvt_qmi_enums
    @{
  */
typedef enum {
  IMSVT_H264_LEVEL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSVT_H264_LEVEL1_V01 = 0x00, 
  IMSVT_H264_LEVEL1B_V01 = 0x01, 
  IMSVT_H264_LEVEL11_V01 = 0x02, 
  IMSVT_H264_LEVEL12_V01 = 0x03, 
  IMSVT_H264_LEVEL13_V01 = 0x04, 
  IMSVT_H264_LEVEL2_V01 = 0x05, 
  IMSVT_H264_LEVEL21_V01 = 0x06, 
  IMSVT_H264_LEVEL22_V01 = 0x07, 
  IMSVT_H264_LEVEL3_V01 = 0x08, 
  IMSVT_H264_LEVEL31_V01 = 0x09, 
  IMSVT_H264_LEVEL32_V01 = 0x0A, 
  IMSVT_H264_LEVEL4_V01 = 0x0B, 
  IMSVT_H264_LEVEL41_V01 = 0x0C, 
  IMSVT_H264_LEVEL42_V01 = 0x0D, 
  IMSVT_H264_LEVEL5_V01 = 0x0E, 
  IMSVT_H264_LEVEL51_V01 = 0x0F, 
  IMSVT_H264_LEVEL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsvt_h264_level_enum_v01;
/**
    @}
  */

/** @addtogroup imsvt_qmi_enums
    @{
  */
typedef enum {
  IMSVT_DEV_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSVT_PLAYER_V01 = 0x0, 
  IMSVT_RECORDER_V01 = 0x01, 
  IMSVT_DEV_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsvt_dev_enum_v01;
/**
    @}
  */

/** @addtogroup imsvt_qmi_enums
    @{
  */
typedef enum {
  IMSVT_ADDR_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSVT_IPV4_V01 = 0x0, 
  IMSVT_IPV6_V01 = 0x01, 
  IMSVT_ADDR_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsvt_addr_type_enum_v01;
/**
    @}
  */

/** @addtogroup imsvt_qmi_enums
    @{
  */
typedef enum {
  IMSVT_H263_PROFILE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSVT_H263_PROFILE_BASELINE_V01 = 0x00, 
  IMSVT_H263_PROFILE_H320_CODING_V01 = 0x01, 
  IMSVT_H263_PROFILE_BACKWARD_COMPATIBLE_V01 = 0x02, 
  IMSVT_H263_PROFILE_ISWV2_V01 = 0x03, 
  IMSVT_H263_PROFILE_ISWV3_V01 = 0x04, 
  IMSVT_H263_PROFILE_HIGH_COMPRESSION_V01 = 0x05, 
  IMSVT_H263_PROFILE_INTERNET_V01 = 0x06, 
  IMSVT_H263_PROFILE_INTERLACE_V01 = 0x07, 
  IMSVT_H263_PROFILE_HIGH_LATENCY_V01 = 0x08, 
  IMSVT_H263_PROFILE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsvt_h263_profile_enum_v01;
/**
    @}
  */

/** @addtogroup imsvt_qmi_enums
    @{
  */
typedef enum {
  IMSVT_H263_LEVEL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSVT_H263_LEVEL10_V01 = 0x00, 
  IMSVT_H263_LEVEL20_V01 = 0x01, 
  IMSVT_H263_LEVEL30_V01 = 0x02, 
  IMSVT_H263_LEVEL40_V01 = 0x03, 
  IMSVT_H263_LEVEL50_V01 = 0x04, 
  IMSVT_H263_LEVEL60_V01 = 0x05, 
  IMSVT_H263_LEVEL70_V01 = 0x06, 
  IMSVT_H263_LEVEL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsvt_h263_level_enum_v01;
/**
    @}
  */

/** @addtogroup imsvt_qmi_enums
    @{
  */
typedef enum {
  IMSVT_VIDEO_CODEC_ERROR_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSVT_VIDEO_CODEC_GENERIC_ERROR_V01 = 0x00, 
  IMSVT_VIDEO_ENCODER_ERROR_V01 = 0x01, 
  IMSVT_VIDEO_DECODER_ERROR_V01 = 0x02, 
  IMSVT_VIDEO_CODEC_ERROR_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsvt_video_codec_error_enum_v01;
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  imsvt_h264_profile_enum_v01 profile_type;
  /**<   H.264 profile type. Values: \n
       - 0 -- Baseline profile \n
       - 1 -- Main profile \n
       - 2 -- Extended profile \n
       - 3 -- High profile
  */

  /*  H.264 Profile level  */
  imsvt_h264_level_enum_v01 profile_level;
  /**<   H.264 profile level. Values: \n
       - 0 -- Level 1 \n
       - 1 -- Level 1b \n
       - 2 -- Level 1.1 \n
       - 3 -- Level 1.2 \n
       - 4 -- Level 1.3 \n
       - 5 -- Level 2 \n
       - 6 -- Level 2.1 \n
       - 7 -- Level 2.2 \n
       - 8 -- Level 3 \n
       - 9 -- Level 3.1 \n
       - 10 -- Level 3.2 \n
       - 11 -- Level 4 \n
       - 12 -- Level 4.1 \n
       - 13 -- Level 4.2 \n
       - 14 -- Level 5 \n
       - 15 -- Level 5.1
  */
}imsvt_h264_parameters_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t profile_level_id;
  /**<   MP4 profile level ID type.
  */
}imsvt_mp4_parameters_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  imsvt_video_codec_enum_v01 codec;
  /**<   Codec to be used. Values: \begin{itemize1}
       \item 0 -- XVID MPEG-4 codec 
       \item 1 -- ISO MPEG-4 codec 
       \item 2 -- H.263 codec 
       \item 3 -- H.264 codec 
       \item 4 to 255 -- Reserved for future extension to add more video codec 
                         types
       \vspace{-0.18in}
       \end{itemize1}
  */

  int32_t width;
  /**<   Width of the video capture.
  */

  int32_t height;
  /**<   Height of the video capture.
  */

  int32_t bit_rate;
  /**<   Output bitrate expected from the encoder in kbps.
  */

  int32_t frame_rate;
  /**<   Frame rate of the capture in frames per second.
  */

  uint32_t clock_rate;
  /**<   Sampling rate. */

  uint32_t lipsynch_drop_upper_limit;
  /**<   Upper bound value for the video frame that falls under the drop set.
  */

  uint32_t lipsynch_drop_lower_limit;
  /**<   Lower bound value for the video frame that falls under the drop set.
  */

  uint8_t lip_sync_enable;
  /**<   Enable/disable lip synchronization.
  */
}imsvt_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  imsvt_h264_parameters_v01 h264_config;
  /**<   H.264 encoder custom configuration.
  */
}imsvt_h264_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  imsvt_mp4_parameters_v01 mp4_config;
  /**<   H.264 encoder custom configuration.
  */
}imsvt_mp4_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t n_imsntp_time;
  /**<   Most significant NTP timestamp.
  */

  uint32_t n_ilsntp_time;
  /**<   Least significant NTP timestamp.
  */

  uint32_t n_irtp_time_stamp;
  /**<   RTP timestamp.
  */
}imsvt_time_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t volheader_len;  /**< Must be set to # of elements in volheader */
  char volheader[IMSVT_VOL_HEADER_MAX_V01];
  /**<   Buffer containing the VOL header.
  */
}imsvt_vol_header_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t nalheader_len;  /**< Must be set to # of elements in nalheader */
  char nalheader[IMSVT_NAL_HEADER_MAX_V01];
  /**<   Buffer containing the NAL header.
  */
}imsvt_nal_header_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  uint16_t port;
  /**<   Video end point port.
  */

  imsvt_addr_type_enum_v01 iptype;
  /**<   IP address type. Values: \n
       - 0 -- IPv4 address type \n
       - 1 -- IPv6 address type
  */

  uint32_t ipaddr_len;  /**< Must be set to # of elements in ipaddr */
  char ipaddr[IMSVT_IP_ADDR_MAX_V01];
  /**<   Buffer containing the IP address of Iface.*/
}imsvt_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t audio_clock_rate;
  /**<   Sampling rate of audio. Required for the Lip Sync algorithm.
  */

  uint32_t audio_packet_interval;
  /**<   Audio packet interval. Required for the Lip Sync algorithm.
  */
}imsvt_audio_ls_parameters_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  imsvt_h263_profile_enum_v01 profile_type;
  /**<   H.263 profile type. Values: \n
     - 0 -- Baseline profile \n
     - 1 -- H320 Coding profile \n
     - 2 -- Backward Compatible profile \n
     - 3 -- ISWV2 profile \n
     - 4 -- ISWV3 profile \n
     - 5 -- High Compression profile \n
     - 6 -- Internet profile \n
     - 7 -- Interlace profile \n
     - 8 -- High Latency profile 
  */

  imsvt_h263_level_enum_v01 profile_level;
  /**<   H.263 profile level. Values: \n
     - 0 -- Level 10 \n
     - 1 -- Level 20 \n
     - 2 -- Level 30 \n
     - 3 -- Level 40 \n
     - 4 -- Level 50 \n
     - 5 -- Level 60 \n
     - 6 -- Level 70 
  */
}imsvt_h263_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_aggregates
    @{
  */
typedef struct {

  imsvt_video_codec_error_enum_v01 error_value;
  /**<   Video codec error. Values: \n
     - 0 -- Generic error in the video codec \n
     - 1 -- Error in the video encoder \n
     - 2 -- Error in the video decoder 
  */
}imsvt_video_codec_error_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Acknowledgement Message;  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}imsvt_ack_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Request Message; Initializes the video codec. */
typedef struct {

  /* Mandatory */
  /*  Common Video Codec Configuration */
  imsvt_config_type_v01 video_codec_config;

  /* Mandatory */
  /*  Video Device Type */
  imsvt_dev_enum_v01 video_device;
  /**<   Video device type. Values: \begin{itemize1}
       \item 0 -- Player 
       \item 1 -- Recorder 
       \item 2 to 255 -- Reserved for future extension to add more device types
	   \vspace{-0.18in}
       \end{itemize1}
  */

  /* Mandatory */
  /*  IP Address and Port */
  imsvt_addr_type_v01 ip_addr;

  /* Mandatory */
  /*  Lip Sync Parameters of the Audio Codec */
  imsvt_audio_ls_parameters_v01 audio_param;

  /* Optional */
  /*  H.264 Video Codec Configuration */
  uint8_t h264_config_valid;  /**< Must be set to true if h264_config is being passed */
  imsvt_h264_config_type_v01 h264_config;

  /* Optional */
  /*  MP4 Video Codec Configuration */
  uint8_t mp4_config_valid;  /**< Must be set to true if mp4_config is being passed */
  imsvt_mp4_config_type_v01 mp4_config;

  /* Optional */
  /*  NAL Header Packet */
  uint8_t nal_header_valid;  /**< Must be set to true if nal_header is being passed */
  imsvt_nal_header_info_type_v01 nal_header;

  /* Optional */
  /*  VOL Header Packet */
  uint8_t vol_header_valid;  /**< Must be set to true if vol_header is being passed */
  imsvt_vol_header_info_type_v01 vol_header;

  /* Optional */
  /*  H.263 Video Codec Configuration */
  uint8_t h263_config_valid;  /**< Must be set to true if h263_config is being passed */
  imsvt_h263_config_type_v01 h263_config;
}imsvt_initialize_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Initializes the video codec. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */

  /* Optional */
  /*  Video Device Type */
  uint8_t video_device_valid;  /**< Must be set to true if video_device is being passed */
  imsvt_dev_enum_v01 video_device;
  /**<   Video device type. Values: \begin{itemize1}
       \item 0 -- Player 
       \item 1 -- Recorder 
       \item 2 to 255 -- Reserved for future extension to add more device types
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Optional */
  /*  IP Address and Port */
  uint8_t ip_addr_valid;  /**< Must be set to true if ip_addr is being passed */
  imsvt_addr_type_v01 ip_addr;
}imsvt_initialize_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Request Message; Uninitializes the video codec. */
typedef struct {

  /* Mandatory */
  /*  Video Device Type */
  imsvt_dev_enum_v01 video_device;
  /**<   Video device type. Values: \begin{itemize1}
       \item 0 -- Player 
       \item 1 -- Recorder 
       \item 2 to 255 -- Reserved for future extension to add more device types
       \vspace{-0.18in}
       \end{itemize1}
  */
}imsvt_uninitialize_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Uninitializes the video codec. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */

  /* Optional */
  /*  Video Device Type */
  uint8_t video_device_valid;  /**< Must be set to true if video_device is being passed */
  imsvt_dev_enum_v01 video_device;
  /**<   Video device type. Values: \begin{itemize1}
       \item 0 -- Player 
       \item 1 -- Recorder 
       \item 2 to 255 -- Reserved for future extension to add more device types
       \vspace{-0.18in}
       \end{itemize1}
  */
}imsvt_uninitialize_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Request Message; Configures the video codec. */
typedef struct {

  /* Mandatory */
  /*  Video Device Type */
  imsvt_dev_enum_v01 video_device;
  /**<   Video device type. Values: \begin{itemize1}
       \item 0 -- Player 
       \item1 -- Recorder 
       \item2 to 255 -- Reserved for future extension to add more device type
       \vspace{-0.18in}
       \end{itemize1}
  */

  /* Mandatory */
  /*  Codec Config to be Set */
  imsvt_config_type_v01 video_codec_config;

  /* Optional */
  /*  H.264 Video Codec Configuration */
  uint8_t h264_config_valid;  /**< Must be set to true if h264_config is being passed */
  imsvt_h264_config_type_v01 h264_config;

  /* Optional */
  /*  MP4 Video Codec Configuration */
  uint8_t mp4_config_valid;  /**< Must be set to true if mp4_config is being passed */
  imsvt_mp4_config_type_v01 mp4_config;

  /* Optional */
  /*  NAL Header Packet */
  uint8_t nal_header_valid;  /**< Must be set to true if nal_header is being passed */
  imsvt_nal_header_info_type_v01 nal_header;

  /* Optional */
  /*  VOL Header Packet */
  uint8_t vol_header_valid;  /**< Must be set to true if vol_header is being passed */
  imsvt_vol_header_info_type_v01 vol_header;

  /* Optional */
  /*  H.263 Video Codec Configuration */
  uint8_t h263_config_valid;  /**< Must be set to true if h263_config is being passed */
  imsvt_h263_config_type_v01 h263_config;

  /* Optional */
  /*  Maximum NAL Unit Size  */
  uint8_t max_nalu_size_valid;  /**< Must be set to true if max_nalu_size is being passed */
  uint32_t max_nalu_size;
  /**<   Maximum NAL unit size in bytes. \n
   This parameter controls the maximum size of the encoded NAL units returned
   by the video encoder.
  */

  /* Optional */
  /*  Lip Sync Parameters of the Audio Codec */
  uint8_t audio_param_valid;  /**< Must be set to true if audio_param is being passed */
  imsvt_audio_ls_parameters_v01 audio_param;
}imsvt_codec_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Configures the video codec. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */

  /* Optional */
  /*  Video Device Type */
  uint8_t video_device_valid;  /**< Must be set to true if video_device is being passed */
  imsvt_dev_enum_v01 video_device;
  /**<   Video device type. Values: \begin{itemize1}
       \item 0 -- Player 
       \item 1 -- Recorder 
       \item 2 to 255 -- Reserved for future extension to add more device types
	   \vspace{-0.18in}
       \end{itemize1}
  */
}imsvt_codec_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * imsvt_record_stop_req_msg is empty
 * typedef struct {
 * }imsvt_record_stop_req_msg_v01;
 */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Stops the recording of video. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}imsvt_record_stop_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * imsvt_record_start_req_msg is empty
 * typedef struct {
 * }imsvt_record_start_req_msg_v01;
 */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Starts the recording of video. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}imsvt_record_start_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * imsvt_play_stop_req_msg is empty
 * typedef struct {
 * }imsvt_play_stop_req_msg_v01;
 */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Stops the playing of video. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}imsvt_play_stop_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * imsvt_play_start_req_msg is empty
 * typedef struct {
 * }imsvt_play_start_req_msg_v01;
 */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Starts the playing of video. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}imsvt_play_start_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Request Message; Sends the last audio play time. */
typedef struct {

  /* Mandatory */
  /*  Last Played Audio Timestamp */
  uint32_t last_play_time_stamp;
  /**<   Last played audio timestamp. */
}imsvt_last_audio_play_time_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Sends the last audio play time. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}imsvt_last_audio_play_time_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Request Message; Reports the sender RTP audio packet statistics. */
typedef struct {

  /* Mandatory */
  /*  Timestamp Information */
  imsvt_time_info_type_v01 time_info;
}imsvt_audio_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Reports the sender RTP audio packet statistics. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}imsvt_audio_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Request Message; Reports the sender RTP video packet statistics. */
typedef struct {

  /* Mandatory */
  /*  Timestamp Information */
  imsvt_time_info_type_v01 time_info;
}imsvt_video_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Reports the sender RTP video packet statistics. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}imsvt_video_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Request Message; Generates the H.264 SPS/PPS message. */
typedef struct {

  /* Mandatory */
  /*  Codec Config */
  imsvt_config_type_v01 video_codec_config;

  /* Mandatory */
  /*  H.264 Video Codec Config */
  imsvt_h264_config_type_v01 h264_config;
}imsvt_gen_h264_sps_pps_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Generates the H.264 SPS/PPS message. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */

  /* Optional */
  /*  NAL Header */
  uint8_t nal_header_valid;  /**< Must be set to true if nal_header is being passed */
  imsvt_nal_header_info_type_v01 nal_header;
}imsvt_gen_h264_sps_pps_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Request Message; Generates the MP4 VOL header. */
typedef struct {

  /* Mandatory */
  /*  Codec Configuration */
  imsvt_config_type_v01 video_codec_config;

  /* Mandatory */
  /*  MP4 Video Codec Configuration */
  imsvt_mp4_config_type_v01 mp4_config;
}imsvt_gen_mp4_vol_header_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Generates the MP4 VOL header. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */

  /* Optional */
  /*  VOL Header */
  uint8_t vol_header_valid;  /**< Must be set to true if vol_header is being passed */
  imsvt_vol_header_info_type_v01 vol_header;
}imsvt_gen_mp4_vol_header_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Request Message; Requests that the video encoder adapt to a new bitrate. */
typedef struct {

  /* Mandatory */
  /*  Bitrate */
  uint32_t bit_rate;
  /**<   Bitrate in kilobits per second. 
  */
}imsvt_video_bitrate_adapt_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Requests that the video encoder adapt to a new bitrate. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}imsvt_video_bitrate_adapt_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * imsvt_h264_idr_generate_req_msg is empty
 * typedef struct {
 * }imsvt_h264_idr_generate_req_msg_v01;
 */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Requests that the video encoder generate an IDR frame. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}imsvt_h264_idr_generate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Request Message; Feeds the audio/video synchronization timestamp reference. */
typedef struct {

  /* Mandatory */
  /*  64-bit AV Sync Feed */
  uint64_t n_iavsync_time;
  /**<   64-bit audio/video synchronization timestamp. */
}imsvt_av_sync_feed_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Response Message; Feeds the audio/video synchronization timestamp reference. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.
  Standard response type. Contains the following data members:
      qmi_result_type - QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      qmi_error_type  - Error code. Possible error code values are described in
                        the error codes section of each message definition.
  */
}imsvt_av_sync_feed_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * imsvt_service_available_ind_msg is empty
 * typedef struct {
 * }imsvt_service_available_ind_msg_v01;
 */

/** @addtogroup imsvt_qmi_messages
    @{
  */
/** Indication Message; Indication to report the asynchronous errors to the client on the 
               modem processor. */
typedef struct {

  /* Mandatory */
  /*  Asynchronous Error During Video Codec Operation */
  imsvt_video_codec_error_type_v01 video_codec_error;
}imsvt_video_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup imsvt_qmi_msg_ids
    @{
  */
#define QMI_IMSVT_INITIALIZE_RESP_V01 0x0020
#define QMI_IMSVT_INITIALIZE_ACK_V01 0x0020
#define QMI_IMSVT_INITIALIZE_REQ_V01 0x0020
#define QMI_IMSVT_UNINITIALIZE_RESP_V01 0x0021
#define QMI_IMSVT_UNINITIALIZE_ACK_V01 0x0021
#define QMI_IMSVT_UNINITIALIZE_REQ_V01 0x0021
#define QMI_IMSVT_CODEC_CONFIG_RESP_V01 0x0022
#define QMI_IMSVT_CODEC_CONFIG_ACK_V01 0x0022
#define QMI_IMSVT_CODEC_CONFIG_REQ_V01 0x0022
#define QMI_IMSVT_RECORD_STOP_RESP_V01 0x0023
#define QMI_IMSVT_RECORD_STOP_ACK_V01 0x0023
#define QMI_IMSVT_RECORD_STOP_REQ_V01 0x0023
#define QMI_IMSVT_RECORD_START_RESP_V01 0x0024
#define QMI_IMSVT_RECORD_START_ACK_V01 0x0024
#define QMI_IMSVT_RECORD_START_REQ_V01 0x0024
#define QMI_IMSVT_PLAY_STOP_RESP_V01 0x0025
#define QMI_IMSVT_PLAY_STOP_ACK_V01 0x0025
#define QMI_IMSVT_PLAY_STOP_REQ_V01 0x0025
#define QMI_IMSVT_PLAY_START_RESP_V01 0x0026
#define QMI_IMSVT_PLAY_START_ACK_V01 0x0026
#define QMI_IMSVT_PLAY_START_REQ_V01 0x0026
#define QMI_IMSVT_GEN_H264_SPS_PPS_RESP_V01 0x0027
#define QMI_IMSVT_GEN_H264_SPS_PPS_ACK_V01 0x0027
#define QMI_IMSVT_GEN_H264_SPS_PPS_REQ_V01 0x0027
#define QMI_IMSVT_GEN_MP4_VOL_HEADER_RESP_V01 0x0028
#define QMI_IMSVT_GEN_MP4_VOL_HEADER_ACK_V01 0x0028
#define QMI_IMSVT_GEN_MP4_VOL_HEADER_REQ_V01 0x0028
#define QMI_IMSVT_LAST_AUDIO_PLAY_TIME_RESP_V01 0x0029
#define QMI_IMSVT_LAST_AUDIO_PLAY_TIME_ACK_V01 0x0029
#define QMI_IMSVT_LAST_AUDIO_PLAY_TIME_REQ_V01 0x0029
#define QMI_IMSVT_AUDIO_REPORT_RESP_V01 0x002A
#define QMI_IMSVT_AUDIO_REPORT_ACK_V01 0x002A
#define QMI_IMSVT_AUDIO_REPORT_REQ_V01 0x002A
#define QMI_IMSVT_VIDEO_REPORT_RESP_V01 0x002B
#define QMI_IMSVT_VIDEO_REPORT_ACK_V01 0x002B
#define QMI_IMSVT_VIDEO_REPORT_REQ_V01 0x002B
#define QMI_IMSVT_VIDEO_BIT_RATE_ADAPT_RESP_V01 0x002C
#define QMI_IMSVT_VIDEO_BIT_RATE_ADAPT_ACK_V01 0x002C
#define QMI_IMSVT_VIDEO_BIT_RATE_ADAPT_REQ_V01 0x002C
#define QMI_IMSVT_H264_IDR_GENERATE_RESP_V01 0x002D
#define QMI_IMSVT_H264_IDR_GENERATE_ACK_V01 0x002D
#define QMI_IMSVT_H264_IDR_GENERATE_REQ_V01 0x002D
#define QMI_IMSVT_AV_SYNC_FEED_RESP_V01 0x002E
#define QMI_IMSVT_AV_SYNC_FEED_ACK_V01 0x002E
#define QMI_IMSVT_AV_SYNC_FEED_REQ_V01 0x002E
#define QMI_IMSVT_SERVICE_AVAILABLE_IND_V01 0x002F
#define QMI_IMSVT_SERVICE_AVAILABLE_ACK_V01 0x002F
#define QMI_IMSVT_VIDEO_ERROR_IND_V01 0x0030
#define QMI_IMSVT_VIDEO_ERROR_ACK_V01 0x0030
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro imsvt_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type imsvt_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define imsvt_get_service_object_v01( ) \
          imsvt_get_service_object_internal_v01( \
            IMSVT_V01_IDL_MAJOR_VERS, IMSVT_V01_IDL_MINOR_VERS, \
            IMSVT_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

