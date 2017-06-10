#ifndef IMSRTP_SERVICE_01_H
#define IMSRTP_SERVICE_01_H
/**
  @file ip_multimedia_subsystem_rtp_v01.h
  
  @brief This is the public header file which defines the imsrtp service Data structures.

  This header file defines the types and structures that were defined in 
  imsrtp. It contains the constant values defined, enums, structures,
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


  $Header: //source/qcom/qct/interfaces/qmi/imsrtp/main/latest/api/ip_multimedia_subsystem_rtp_v01.h#26 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It requires encode/decode library version 5 or later
   It was generated on: Tue Jun  4 2013 (Spin 0)
   From IDL File: ip_multimedia_subsystem_rtp_v01.idl */

/** @defgroup imsrtp_qmi_consts Constant values defined in the IDL */
/** @defgroup imsrtp_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup imsrtp_qmi_enums Enumerated types used in QMI messages */
/** @defgroup imsrtp_qmi_messages Structures sent as QMI messages */
/** @defgroup imsrtp_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup imsrtp_qmi_accessor Accessor for QMI service object */
/** @defgroup imsrtp_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup imsrtp_qmi_version 
    @{ 
  */ 
/** Major Version Number of the IDL used to generate this file */
#define IMSRTP_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define IMSRTP_V01_IDL_MINOR_VERS 0x10
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define IMSRTP_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define IMSRTP_V01_MAX_MESSAGE_ID 0x0042;
/** 
    @} 
  */


/** @addtogroup imsrtp_qmi_consts 
    @{ 
  */

/**  Maximum length of the IP address.  */
#define IMSRTP_IP_STR_LEN_V01 40

/**  Maximum length for the SDES item.  */
#define IMSRTP_RTCP_SDES_ITEM_LEN_V01 256

/**  Max Length of VOL header.  */
#define IMS_RTP_VOL_HEADER_MAX_V01 255

/**  Max Length of NAL header.  */
#define IMS_RTP_NAL_HEADER_MAX_V01 255

/**  Maximum length of an APN name.  */
#define IMSRTP_APN_NAME_LEN_V01 100

/**  Maximum length of Secured RTP Master Key.  */
#define IMSRTP_MASTERKEY_STR_LEN_V01 255
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_VIDEO_CODEC_ERROR_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_RTP_VIDEO_CODEC_GENERIC_ERROR_V01 = 0x00, /**<  Generic error.  */
  IMS_RTP_VIDEO_ENCODER_ERROR_V01 = 0x01, /**<  Video encoder error.  */
  IMS_RTP_VIDEO_DECODER_ERROR_V01 = 0x02, /**<  Video Decoder error.  */
  IMSRTP_VIDEO_CODEC_ERROR_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_video_codec_error_enum_v01;
/**
    @}
  */

/**  Enumeration for error in Video Codec(Encoder/Decoder) during 
    encoding/decoding operation. 
 Resolution bitmask type. Represents the resolutions for video. 
 */
typedef uint64_t imsrtp_video_resolution_mask_type_v01;
#define IMSRTP_VIDEO_RESOLUTION_SQCIF_V01 ((imsrtp_video_resolution_mask_type_v01)0x01ull) 
#define IMSRTP_VIDEO_RESOLUTION_QQVGA_V01 ((imsrtp_video_resolution_mask_type_v01)0x02ull) 
#define IMSRTP_VIDEO_RESOLUTION_QCIF_V01 ((imsrtp_video_resolution_mask_type_v01)0x04ull) 
#define IMSRTP_VIDEO_RESOLUTION_QVGA_V01 ((imsrtp_video_resolution_mask_type_v01)0x08ull) 
#define IMSRTP_VIDEO_RESOLUTION_CIF_V01 ((imsrtp_video_resolution_mask_type_v01)0x10ull) 
#define IMSRTP_VIDEO_RESOLUTION_VGA_V01 ((imsrtp_video_resolution_mask_type_v01)0x20ull) 
/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_VIDEO_CODEC_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_RTP_VIDEO_CODEC_MPEG4_XVID_V01 = 0x00, /**<  XVID MPEG-4 codec.  */
  IMS_RTP_VIDEO_CODEC_MPEG4_ISO_V01 = 0x01, /**<  ISO MPEG-4 codec.  */
  IMS_RTP_VIDEO_CODEC_H263_V01 = 0x02, /**<  H.263 codec.  */
  IMS_RTP_VIDEO_CODEC_H264_V01 = 0x03, /**<  H.264 codec.  */
  IMSRTP_VIDEO_CODEC_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_video_codec_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_VIDEO_H264_PROFILE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_RTP_VIDEO_H264_PROFILE_BASELINE_V01 = 0x00, /**<  Baseline profile.  */
  IMS_RTP_VIDEO_H264_PROFILE_MAIN_V01 = 0x01, /**<  Main profile.  */
  IMS_RTP_VIDEO_H264_PROFILE_EXTENDED_V01 = 0x02, /**<  Extended profile.  */
  IMS_RTP_VIDEO_H264_PROFILE_HIGH_V01 = 0x03, /**<  High profile.  */
  IMSRTP_VIDEO_H264_PROFILE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_video_h264_profile_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_VIDEO_H264_LEVEL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_RTP_VIDEO_H264_LEVEL1_V01 = 0x00, /**<  Level 1.  */
  IMS_RTP_VIDEO_H264_LEVEL1B_V01 = 0x01, /**<  Level 1b.  */
  IMS_RTP_VIDEO_H264_LEVEL11_V01 = 0x02, /**<  Level 1.1.  */
  IMS_RTP_VIDEO_H264_LEVEL12_V01 = 0x03, /**<  Level 1.2.  */
  IMS_RTP_VIDEO_H264_LEVEL13_V01 = 0x04, /**<  Level 1.3.  */
  IMS_RTP_VIDEO_H264_LEVEL2_V01 = 0x05, /**<  Level 2.  */
  IMS_RTP_VIDEO_H264_LEVEL21_V01 = 0x06, /**<  Level 2.1.  */
  IMS_RTP_VIDEO_H264_LEVEL22_V01 = 0x07, /**<  Level 2.2.  */
  IMS_RTP_VIDEO_H264_LEVEL3_V01 = 0x08, /**<  Level 3.  */
  IMS_RTP_VIDEO_H264_LEVEL31_V01 = 0x09, /**<  Level 3.1.  */
  IMS_RTP_VIDEO_H264_LEVEL32_V01 = 0x0A, /**<  Level 3.2.  */
  IMS_RTP_VIDEO_H264_LEVEL4_V01 = 0x0B, /**<  Level 4.  */
  IMS_RTP_VIDEO_H264_LEVEL41_V01 = 0x0C, /**<  Level 4.1.  */
  IMS_RTP_VIDEO_H264_LEVEL42_V01 = 0x0D, /**<  Level 4.2.  */
  IMS_RTP_VIDEO_H264_LEVEL5_V01 = 0x0E, /**<  Level 5.  */
  IMS_RTP_VIDEO_H264_LEVEL51_V01 = 0x0F, /**<  Level 5.1.  */
  IMSRTP_VIDEO_H264_LEVEL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_video_h264_level_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_VIDEO_H263_PROFILE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_RTP_VIDEO_H263_PROFILE_BASELINE_V01 = 0x00, /**<  Baseline profile.  */
  IMS_RTP_VIDEO_H263_PROFILE_H320_CODING_V01 = 0x01, /**<  H320 profile.  */
  IMS_RTP_VIDEO_H263_PROFILE_BACKWARD_COMPATIBLE_V01 = 0x02, /**<  Backward Compatible profile.  */
  IMS_RTP_VIDEO_H263_PROFILE_ISWV2_V01 = 0x03, /**<  ISWV2 profile.  */
  IMS_RTP_VIDEO_H263_PROFILE_ISWV3_V01 = 0x04, /**<  ISWV3 profile.  */
  IMS_RTP_VIDEO_H263_PROFILE_HIGH_COMPRESSION_V01 = 0x05, /**<  High Compression profile.  */
  IMS_RTP_VIDEO_H263_PROFILE_INTERNET_V01 = 0x06, /**<  Internet profile.  */
  IMS_RTP_VIDEO_H263_PROFILE_INTERLACE_V01 = 0x07, /**<  Interlace profile.  */
  IMS_RTP_VIDEO_H263_PROFILE_HIGH_LATENCY_V01 = 0x08, /**<  High Latency profile.  */
  IMSRTP_VIDEO_H263_PROFILE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_video_h263_profile_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_VIDEO_H263_LEVEL_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMS_RTP_VIDEO_H263_LEVEL10_V01 = 0x00, /**<  Level 10.  */
  IMS_RTP_VIDEO_H263_LEVEL20_V01 = 0x01, /**<  Level 20.  */
  IMS_RTP_VIDEO_H263_LEVEL30_V01 = 0x02, /**<  Level 30.  */
  IMS_RTP_VIDEO_H263_LEVEL40_V01 = 0x03, /**<  Level 40.  */
  IMS_RTP_VIDEO_H263_LEVEL50_V01 = 0x04, /**<  Level 50.  */
  IMS_RTP_VIDEO_H263_LEVEL60_V01 = 0x05, /**<  Level 60.  */
  IMS_RTP_VIDEO_H263_LEVEL70_V01 = 0x06, /**<  Level 70.  */
  IMS_RTP_VIDEO_H263_LEVEL45_V01 = 0x07, /**<  Level 45.  */
  IMSRTP_VIDEO_H263_LEVEL_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_video_h263_level_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  imsrtp_video_codec_enum_v01 codec;
  /**<   Codec to be used. Values: \begin{itemize1}
       \item 0 -- XVID MPEG-4 codec
       \item 1 -- ISO MPEG-4 codec
       \item 2 -- H.263 codec
       \item 3 -- H.264 codec
       \item 4 to 255 -- Reserved for future extension to add more video 
                         codec types
       \vspace{-12pt}
       \end{itemize1}
  */

  int32_t width;
  /**<   Width of the video capture. */

  int32_t height;
  /**<   Height of the video capture. */

  int32_t bit_rate;
  /**<   Output bitrate expected from the encoder in kbps. */

  int32_t frame_rate;
  /**<   Frame rate of the capture in frames per second. */

  uint32_t clock_rate;
  /**<   Sampling rate. */

  uint32_t lip_sync_drop_upper_limit;
  /**<   Upper bound value for the video frame that falls under the drop set. */

  uint32_t lip_sync_drop_lower_limit;
  /**<   Lower bound value for the video frame that falls under the drop set. */

  uint8_t lip_sync_enable;
  /**<   Enable/disable lip synchronization. */

  uint32_t lip_sync_audio_clock_rate;
  /**<   Sampling rate of audio needed for the Lip Sync algorithm. */

  uint32_t lip_sync_audio_packet_interval;
  /**<   Audio packet interval needed for the Lip Sync algorithm. */
}imsrtp_video_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  imsrtp_video_codec_enum_v01 codec;
  /**<   Codec to be used. Values: \begin{itemize1}
       \item 0 -- XVID MPEG-4 codec
       \item 1 -- ISO MPEG-4 codec
       \item 2 -- H.263 codec
       \item 3 -- H.264 codec
       \item 4 to 255 -- Reserved for future extension to add more video 
                         codec types
       \vspace{-12pt}
       \end{itemize1}
   */

  imsrtp_video_resolution_mask_type_v01 resolutions_supported;
  /**<   Resolution to be used. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_VIDEO_ RESOLUTION_SQCIF -- Supports the Sub-Quarter CIF resolution
       \item 0x02 -- IMSRTP_VIDEO_ RESOLUTION_QQVGA -- Supports the Quarter-Quarter VGA resolution
       \item 0x04 -- IMSRTP_VIDEO_ RESOLUTION_QCIF -- Supports the Quarter CIF resolution
       \item 0x08 -- IMSRTP_VIDEO_ RESOLUTION_QVGA -- Supports the Quarter VGA resolution
       \item 0x10 -- IMSRTP_VIDEO_ RESOLUTION_CIF -- Supports the Common Intermediate Format resolution
       \item 0x20 -- IMSRTP_VIDEO_ RESOLUTION_VGA -- Supports the Video Graphics Array resolution
       \vspace{-12pt}
       \end{itemize1}
   */

  uint16_t min_bit_rate;
  /**<   Minimum encoder bitrate in kbps. */

  uint16_t min_frame_rate;
  /**<   Minimum encoder frame rate in frames/sec. */
}imsrtp_video_capability_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  imsrtp_video_h264_profile_enum_v01 profile_type;
  /**<   H.264 profile type. Values: \begin{itemize1}
       \item 0 -- Baseline profile
       \item 1 -- Main profile
       \item 2 -- Extended profile
       \item 3 -- High profile
       \vspace{-12pt}
       \end{itemize1}
  */

  imsrtp_video_h264_level_enum_v01 profile_level;
  /**<   H.264 profile level. Values: \begin{itemize1}
       \item 0 -- Level 1
       \item 1 -- Level 1b
       \item 2 -- Level 1.1
       \item 3 -- Level 1.2
       \item 4 -- Level 1.3
       \item 5 -- Level 2
       \item 6 -- Level 2.1
       \item 7 -- Level 2.2
       \item 8 -- Level 3
       \item 9 -- Level 3.1
       \item 10 -- Level 3.2
       \item 11 -- Level 4
       \item 12 -- Level 4.1
       \item 13 -- Level 4.2
       \item 14 -- Level 5
       \item 15 -- Level 5.1
       \vspace{-12pt}
       \end{itemize1}
  */

  uint32_t nalheader_len;  /**< Must be set to # of elements in nalheader */
  uint8_t nalheader[IMS_RTP_NAL_HEADER_MAX_V01];
  /**<   H.264 NAL header. */
}imsrtp_video_h264_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t profile_level_id;
  /**<   MP4 profile level ID type. */

  uint32_t volheader_len;  /**< Must be set to # of elements in volheader */
  uint8_t volheader[IMS_RTP_VOL_HEADER_MAX_V01];
  /**<   MPEG-4 VOL header. */
}imsrtp_video_mp4_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  imsrtp_video_h263_profile_enum_v01 profile_type;
  /**<   H.263 profile type. Values: \begin{itemize1}
       \item 0 -- Baseline profile
       \item 1 -- H320 Coding profile
       \item 2 -- Backward Compatible profile
       \item 3 -- ISWV2 profile
       \item 4 -- ISWV3 profile
       \item 5 -- High Compression profile
       \item 6 -- Internet profile
       \item 7 -- Interlace profile
       \item 8 -- High Latency profile
       \vspace{-12pt}
       \end{itemize1}
  */

  imsrtp_video_h263_level_enum_v01 profile_level;
  /**<   H263 profile level. Values: \begin{itemize1}
       \item 0 -- Level 10
       \item 1 -- Level 20
       \item 2 -- Level 30
       \item 3 -- Level 40
       \item 4 -- Level 50
       \item 5 -- Level 60
       \item 6 -- Level 70
       \item 7 -- Level 45
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_video_h263_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t n_imsntp_time;
  /**<   Most significant NTP timestamp. */

  uint32_t n_ilsntp_time;
  /**<   Least significant NTP timestamp. */

  uint32_t n_irtp_time_stamp;
  /**<   RTP timestamp. */
}imsrtp_video_time_info_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_IP_VER_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_IPV4_V01 = 0x00, /**<  32-bit IPv4 address.  */
  IMSRTP_IPV6_V01 = 0x01, /**<  128-bit IPv6 address. Interface-specific IP address generated by the 
         application and given to the RTP.  */
  IMSRTP_IP_VER_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_ip_ver_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMS_RTP_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_SUCCESS_V01 = 0x00, /**<  Operation requested was successful.  */
  IMSRTP_NORESOURCES_V01 = 0x01, /**<  Operation failed; no resources.  */
  IMSRTP_WRONG_PARAM_V01 = 0x02, /**<  Operation failed; at least one parameter was unacceptable.  */
  IMSRTP_ERR_FATAL_V01 = 0x03, /**<  Unknown but fatal error.  */
  IMSRTP_PORT_NOTAVAILABLE_V01 = 0x04, /**<  Unable to bind to a specific I/B port.  */
  IMSRTP_NOT_SUPPORTED_V01 = 0x05, /**<  Feature or primitive is not supported at this time.  */
  IMSRTP_QDJ_ENQ_ERR_V01 = 0x06, /**<  Error; enqueueing failed in jitter buffer.  */
  IMSRTP_MEDIA_TX_ONLY_ACTIVE_V01 = 0x07, /**<  Media flow Tx only active.  */
  IMSRTP_MEDIA_RX_ONLY_ACTIVE_V01 = 0x08, /**<  Media flow Rx only active.  */
  IMSRTP_EXT_IPV6_ADDR_DELETED_V01 = 0x09, /**<  External IPV6 address is deleted. 

         Note: For values greater than Max enum, the client is to treat this as 
         IMSRTP_ERR_FATAL.
   */
  IMS_RTP_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_rtp_status_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  imsrtp_ip_ver_enum_v01 ipaddr_type;
  /**<   IP address version type. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_IPV4 -- 32-bit IPv4 address 
       \item 0x01 -- IMSRTP_IPV6 -- 128-bit IPv6 address; interface-specific IP 
                                    address generated by the application and  
                                    given to the RTP
       \vspace{-12pt}
       \end{itemize1}
       */

  char ipaddr[IMSRTP_IP_STR_LEN_V01 + 1];
  /**<   Buffer containing the IP address. */
}imsrtp_ip_addr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  char cname[IMSRTP_RTCP_SDES_ITEM_LEN_V01 + 1];
  /**<   Provides canonical end-point identifiers (CNAME) to all session 
       participants. */

  uint16_t rtcp_ib_port;
  /**<   Inbound port for receiving RTCP reports. */

  uint16_t rtcp_ob_port;
  /**<   RTCP outbound port through which RTCP packets are sent. */

  uint8_t rtcp_tx_enabled;
  /**<   Set to TRUE if RTCP reports are to be sent.  */
}imsrtp_rtcp_params_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_CODEC_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_AMR_NB_CODEC_V01 = 0x00, /**<  AMR-NB audio codec.  */
  IMSRTP_AMR_WB_CODEC_V01 = 0x01, /**<  AMR-WB audio codec.  */
  IMSRTP_H264_CODEC_V01 = 0x02, /**<  H.264 video codec.  */
  IMSRTP_TEXT_CODEC_V01 = 0x03, /**<  RTP text stream  */
  IMSRTP_CODEC_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_codec_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_CODEC_MODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_CODEC_MODE_0_V01 = 0x00, 
  IMSRTP_CODEC_MODE_1_V01 = 0x01, 
  IMSRTP_CODEC_MODE_2_V01 = 0x02, 
  IMSRTP_CODEC_MODE_3_V01 = 0x03, 
  IMSRTP_CODEC_MODE_4_V01 = 0x04, 
  IMSRTP_CODEC_MODE_5_V01 = 0x05, 
  IMSRTP_CODEC_MODE_6_V01 = 0x06, 
  IMSRTP_CODEC_MODE_7_V01 = 0x07, 
  IMSRTP_CODEC_MODE_8_V01 = 0x08, 
  IMSRTP_CODEC_MODE_9_V01 = 0x09, 
  IMSRTP_CODEC_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_codec_mode_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  imsrtp_codec_enum_v01 codec;
  /**<   Codec to be used. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_AMR_NB_CODEC  
       \item 0x01 -- IMSRTP_AMR_WB_CODEC  
       \item 0x02 -- IMSRTP_H264_CODEC  
       \item 0x03 -- IMSRTP_TEXT_CODEC
       \item Other values are for future use
       \vspace{-12pt}
       \end{itemize1}
  */

  imsrtp_codec_mode_enum_v01 codec_mode;
  /**<   Media audio codec mode. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_CODEC_MODE_0 -- 4.75 kbps for AMR, 6.6 kbps for AMR-WB   
       \item 0x01 -- IMSRTP_CODEC_MODE_1 -- 5.15 kbps for AMR, 8.855 kbps for AMR-WB 
       \item 0x02 -- IMSRTP_CODEC_MODE_2 -- 5.9 kbps for AMR, 12.65 kbps for AMR-WB 
       \item 0x03 -- IMSRTP_CODEC_MODE_3 -- 6.7 kbps for AMR, 14.25 kbps for AMR-WB 
       \item 0x04 -- IMSRTP_CODEC_MODE_4 -- 7.4 kbps for AMR, 15.85 kbps for AMR-WB 
       \item 0x05 -- IMSRTP_CODEC_MODE_5 -- 7.95 kbps for AMR, 18.25 kbps for AMR-WB 
       \item 0x06 -- IMSRTP_CODEC_MODE_6 -- 10.2 kbps for AMR, 19.85 kbps for AMR-WB 
       \item 0x07 -- IMSRTP_CODEC_MODE_7 -- 12.2 kbps for AMR, 23.05 kbps for AMR-WB 
       \item 0x08 -- IMSRTP_CODEC_MODE_8 -- Silence frame for AMR, 23.85 kbps for AMR-WB 
       \item 0x09 -- IMSRTP_CODEC_MODE_9 -- Silence frame for AMR-WB 
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_codec_params_type_v01;  /* Type */
/**
    @}
  */

/**  Mode map bitmask type. Represents the codec modes for AMR and AMR-WB codec. 
    Values: \begin{itemize1}
       \item 0 to 7 for AMR-NB 
       \item 0 to 8 for AMR-WB 
       \vspace{-12pt}
       \end{itemize1} 
 */
typedef uint64_t imsrtp_modemap_mask_type_v01;
#define IMSRTP_CODEC_MODEMAP_0_V01 ((imsrtp_modemap_mask_type_v01)0x01ull) 
#define IMSRTP_CODEC_MODEMAP_1_V01 ((imsrtp_modemap_mask_type_v01)0x02ull) 
#define IMSRTP_CODEC_MODEMAP_2_V01 ((imsrtp_modemap_mask_type_v01)0x04ull) 
#define IMSRTP_CODEC_MODEMAP_3_V01 ((imsrtp_modemap_mask_type_v01)0x08ull) 
#define IMSRTP_CODEC_MODEMAP_4_V01 ((imsrtp_modemap_mask_type_v01)0x10ull) 
#define IMSRTP_CODEC_MODEMAP_5_V01 ((imsrtp_modemap_mask_type_v01)0x20ull) 
#define IMSRTP_CODEC_MODEMAP_6_V01 ((imsrtp_modemap_mask_type_v01)0x40ull) 
#define IMSRTP_CODEC_MODEMAP_7_V01 ((imsrtp_modemap_mask_type_v01)0x80ull) 
#define IMSRTP_CODEC_MODEMAP_8_V01 ((imsrtp_modemap_mask_type_v01)0x100ull) 
/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t octet_tx_align;
  /**<   Indicates whether the Tx format is octet aligned. A zero value means the 
       format is not octet aligned, and a bandwidth-efficient format will be 
       selected.
   */

  uint8_t octet_rx_align;
  /**<   Indicates whether the Rx format is octet aligned. A zero value means the 
       format is not octet aligned, and a bandwidth-efficient format will be 
       selected. */

  uint8_t mode_map_valid;
  /**<   Indicates whether the mode map is valid. */

  imsrtp_modemap_mask_type_v01 mode_map;
  /**<   Codec modes negotiated in the SDP for this session. This is a bitmask 
       variable that indicates modes negotiated for this session. 
       Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_CODEC_ MODEMAP_0 -- AMR-NB and AMR-WB   
       \item 0x02 -- IMSRTP_CODEC_ MODEMAP_1 -- AMR-NB and AMR-WB 
       \item 0x04 -- IMSRTP_CODEC_ MODEMAP_2 -- AMR-NB and AMR-WB 
       \item 0x08 -- IMSRTP_CODEC_ MODEMAP_3 -- AMR-NB and AMR-WB 
       \item 0x10 -- IMSRTP_CODEC_ MODEMAP_4 -- AMR-NB and AMR-WB 
       \item 0x20 -- IMSRTP_CODEC_ MODEMAP_5 -- AMR-NB and AMR-WB 
       \item 0x40 -- IMSRTP_CODEC_ MODEMAP_6 -- AMR-NB and AMR-WB 
       \item 0x80 -- IMSRTP_CODEC_ MODEMAP_7 -- AMR-NB and AMR-WB 
       \item 0x100 -- IMSRTP_CODEC_ MODEMAP_8 -- AMR-WB only
       \vspace{-12pt}
       \end{itemize1}
  */

  uint16_t max_tx_red;
  /**<   Maximum redundancy that a packet can carry for this session. This is the
       maximum duration in milliseconds that can elapse between the primary 
       (first) transmission of a frame and any redundant transmission that the 
       sender will use. Values: 0 (no redundancy will be used) to 65535 ms.
  */

  uint16_t max_rx_red;
  /**<   Maximum redundancy that a packet can carry for this session. This 
       parameter allows a receiver to have a bounded delay when redundancy is 
       used. Values: 0 (no redundancy will be used) to 65535 ms. */

  uint8_t tx_cmr;
  /**<   Indicates a codec mode request sent to the speech encoder at the site of 
       the receiver of this payload. The value of the CMR field is set to the 
       frame type index of the corresponding speech mode being requested. 
       Values: \begin{itemize1}
       \item 0 to 7 -- AMR (refer to RFC 3267 \hyperref[S1]{[S1]}) 
       \item 0 to 8 -- AMR-WB (refer to RFC 3267 \hyperref[S1]{[S1]}) 
       \item 15 -- No mode request is present 
       \item Other values are for future use 
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_amr_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t packetization_mode_valid;
  /**<   Indicates whether the packetization mode is valid. */

  uint8_t packetization_mode;
  /**<   H.264 video packetization mode. Values: 0 to 2.\begin{itemize1}
       \item 0 -- Single NAL mode 
       \item 1 -- Noninterleaved mode 
       \item 2 -- Interleaved mode
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_h264_config_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t rx_pt_type;
  /**<   Indicates the static or dynamic payload type negotiated for the Rx path 
       for the RTP session while a call was set up through the SDP. This value 
       is compared to the PT value in the incoming RTP packet and honored only 
       when it matches.

       Values: 0 to 127.
  */

  uint8_t tx_pt_type;
  /**<   Indicates the static or dynamic payload type negotiated for the Tx path 
       for the RTP session while a call was set up through the SDP. This value 
       is embedded in the RTP packet before it is sent out.

       Values: 0 to 127.
  */

  uint16_t ptime;
  /**<   Time in milliseconds in each packet. If ptime is set to zero, an 
       appropriate default value determined by the service will be used.
  */

  uint16_t maxptime;
  /**<   Maximum time in milliseconds for each packet. If maxptime is set to 
       zero, an appropriate default value determined by the service will be 
       used.
  */

  uint16_t dtmf_clock_rate;
  /**<   DTMF clock rate for this session. */

  uint8_t dtmf_pt_type;
  /**<   Payload type is set to DTMF packet. */
}imsrtp_session_params_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t cps;
  /**<   Maximum number of characters that can be transmitted per
       second. Recommended maximum value: 30.   
  */

  uint8_t num_red_pkts;
  /**<   Number of redundancy packets.
  */

  uint8_t red_payload;
  /**<   Payload number of the redundancy packet 
       (refer to RFC 4103 \hyperref[S5]{[S5]}).
  */

  uint8_t t140_payload;
  /**<   Payload number of the t140 packet 
       (refer to RFC 4103 \hyperref[S5]{[S5]}).
  */
}imsrtp_text_config_params_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_UI_ORIENTATION_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_LANDSCAPE_V01 = 0, /**<  Landscape mode.  */
  IMSRTP_PORTRAIT_V01 = 1, /**<  Portrait mode.  */
  IMSRTP_UI_ORIENTATION_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_ui_orientation_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_SRTP_SEC_ALGO_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_AES_CM_128_V01 = 0, /**<  Advanced Encryption Standard counter mode with 128-bit encryption used.  */
  IMSRTP_AES_F8_128_V01 = 1, /**<  Advanced Encryption Standard F8 mode with 128-bit encryption used.  */
  IMSRTP_SRTP_SEC_ALGO_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_srtp_sec_algo_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_SRTP_HASH_ALGO_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_HMAC_SHA1_80_V01 = 0, /**<  Keyed-Hash Message Authentication Code Secured Hash Algorithm with 
       80-bits authentication.  */
  IMSRTP_HMAC_SHA1_32_V01 = 1, /**<  Keyed-Hash Message Authentication Code Secured Hash Algorithm with 
       32-bits authentication.  */
  IMSRTP_SRTP_HASH_ALGO_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_srtp_hash_algo_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  char master_key[IMSRTP_MASTERKEY_STR_LEN_V01 + 1];
  /**<   Buffer containing the Secured RTP master key in ASCII Format. */

  imsrtp_srtp_sec_algo_v01 sec_algo;
  /**<   Enum representing security algorithms. */

  imsrtp_srtp_hash_algo_v01 hash_algo;
  /**<   Enum representing hashing algorithms. */

  uint8_t is_authentication_enabled;
  /**<   Authentication Enabled/disabled. */
}imsrtp_srtp_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t rtcp_nack_fb_enabled;
  /**<   Negative acknowledgements picture loss indication. */

  uint8_t rtcp_pli_fb_enabled;
  /**<   Packet loss indication */

  uint8_t rtcp_fir_fb_enabled;
  /**<   Full Intra request */

  uint8_t rtcp_tmmbr_fb_enabled;
  /**<   The Temporary Maximum Media Bit-rate Request/Notification */
}qvp_rtcp_fb_messages_v01;  /* Type */
/**
    @}
  */

/**  Direction bitmask type.  */
typedef uint64_t imsrtp_session_direction_mask_type_v01;
#define IMSRTP_TX_V01 ((imsrtp_session_direction_mask_type_v01)0x01ull) /**<  Transmit only direction.  */
#define IMSRTP_RX_V01 ((imsrtp_session_direction_mask_type_v01)0x02ull) /**<  Receive only direction.  */
/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_DCM_RAT_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_DCM_RAT_NONE_V01 = 0, /**<  None.   */
  IMSRTP_DCM_RAT_GPRS_V01 = 1, /**<  General packet radio services.     */
  IMSRTP_DCM_RAT_EDGE_V01 = 2, /**<  Enhanced data GSM environment.    */
  IMSRTP_DCM_RAT_WCDMA_V01 = 3, /**<  WCDMA.    */
  IMSRTP_DCM_RAT_WLAN_V01 = 4, /**<  WLAN.    */
  IMSRTP_DCM_RAT_CDMA_V01 = 5, /**<  CDMA 1.x.    */
  IMSRTP_DCM_RAT_IWLAN_V01 = 6, /**<  IWLAN.    */
  IMSRTP_DCM_RAT_DOR0_V01 = 7, /**<  CDMA DO Rev0.    */
  IMSRTP_DCM_RAT_DORA_V01 = 8, /**<  CDMA DO RevA.    */
  IMSRTP_DCM_RAT_EHRPD_V01 = 9, /**<  CDMA evolved high-rate packet data.    */
  IMSRTP_DCM_RAT_LTE_V01 = 10, /**<  Long-term evolution.    */
  IMSRTP_DCM_RAT_DORB_V01 = 11, /**<  CDMA DO RevB.    */
  IMSRTP_DCM_RAT_EPC_V01 = 12, /**<  EPC.    */
  IMSRTP_DCM_RAT_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_dcm_rat_type_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_DCM_PROFILE_IPADDR_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_DCM_UNKNOWN_IP_TYPE_V01 = 0, /**<  Unknown.       */
  IMSRTP_DCM_IPV4_V01 = 1, /**<  IPv4 address.  */
  IMSRTP_DCM_IPV6_V01 = 2, /**<  IPv6 address.  */
  IMSRTP_DCM_PROFILE_IPADDR_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_dcm_profile_ipaddr_type_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_DCM_APN_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_DCM_APN_IMS_V01 = 1, /**<  IP multimedia subsystem.  */
  IMSRTP_DCM_APN_INTERNET_V01 = 2, /**<  Internet.    */
  IMSRTP_DCM_APN_EMERGENCY_V01 = 3, /**<  Emergency.    */
  IMSRTP_DCM_APN_RCS_V01 = 4, /**<  RCS.    */
  IMSRTP_DCM_APN_UT_V01 = 5, /**<  UT.    */
  IMSRTP_DCM_APN_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_dcm_apn_type_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Initializes the RTP stack on the modem. */
typedef struct {

  /* Mandatory */
  /*  Profile ID */
  int32_t profile_id;
  /**<   Index to the profile.
  */
}imsrtp_initialize_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Initializes the RTP stack on the modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_initialize_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Initializes the RTP stack on the modem. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values:\begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Optional */
  /*  Application Identifier */
  uint8_t app_id_valid;  /**< Must be set to true if app_id is being passed */
  uint8_t app_id;
  /**<   Unique identifier for the application using the RTP stack. */
}imsrtp_initialize_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Uninitializes the RTP stack on the modem. */
typedef struct {

  /* Mandatory */
  /*  Application Identifier */
  uint8_t app_id;
  /**<   Unique identifier for the application using the RTP stack. */
}imsrtp_uninitialize_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Uninitializes the RTP stack on the modem. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_uninitialize_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Uninitializes the RTP stack on the modem. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  Application Identifier */
  uint8_t app_id;
  /**<   Unique identifier for the application using the RTP stack. */
}imsrtp_uninitialize_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Initializes the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Application Identifier */
  uint8_t app_id;
  /**<   Unique identifier for the application using the RTP stack. */

  /* Mandatory */
  /*  RTP Inbound Port */
  uint16_t rtp_ib_port;
  /**<   RTP inbound port used to listen for incoming RTP packets. */

  /* Mandatory */
  /*  IP Address and Type */
  imsrtp_ip_addr_type_v01 ipaddr_type;

  /* Optional */
  /*  RTP Outbound Port */
  uint8_t rtp_ob_local_port_valid;  /**< Must be set to true if rtp_ob_local_port is being passed */
  uint16_t rtp_ob_local_port;
  /**<   RTP outbound port through which RTP packets are sent. If this port is 
       absent or is set to zero, an ephemeral port is used. */

  /* Optional */
  /*  Remote Transport IP Address */
  uint8_t remote_ip_valid;  /**< Must be set to true if remote_ip is being passed */
  imsrtp_ip_addr_type_v01 remote_ip;

  /* Optional */
  /*  Remote Port */
  uint8_t remote_port_valid;  /**< Must be set to true if remote_port is being passed */
  uint16_t remote_port;
  /**<   Remote port to which RTP packets are sent. */

  /* Optional */
  /*  Codec Parameters */
  uint8_t codec_param_valid;  /**< Must be set to true if codec_param is being passed */
  imsrtp_codec_params_type_v01 codec_param;

  /* Optional */
  /*  Session Parameters  */
  uint8_t session_param_valid;  /**< Must be set to true if session_param is being passed */
  imsrtp_session_params_type_v01 session_param;

  /* Optional */
  /*  AMR Media Codec Parameters */
  uint8_t amr_config_type_valid;  /**< Must be set to true if amr_config_type is being passed */
  imsrtp_amr_config_type_v01 amr_config_type;

  /* Optional */
  /*  H.264 Media Codec Parameters */
  uint8_t h264_config_type_valid;  /**< Must be set to true if h264_config_type is being passed */
  imsrtp_h264_config_type_v01 h264_config_type;

  /* Optional */
  /*  RTCP Parameters */
  uint8_t rtcp_param_valid;  /**< Must be set to true if rtcp_param is being passed */
  imsrtp_rtcp_params_type_v01 rtcp_param;

  /* Optional */
  /*  Cookie */
  uint8_t cookie_valid;  /**< Must be set to true if cookie is being passed */
  uint32_t cookie;
  /**<   Value provided by the application to distinguish the session 
       initialization requests. This value is returned in the corresponding 
       session initialization indication so the application can correlate 
       the indication with the request. 
  */

  /* Optional */
  /*  Common Video Codec Configuration */
  uint8_t video_codec_config_valid;  /**< Must be set to true if video_codec_config is being passed */
  imsrtp_video_config_type_v01 video_codec_config;

  /* Optional */
  /*  H.264 Video Codec Configuration */
  uint8_t h264_specific_config_valid;  /**< Must be set to true if h264_specific_config is being passed */
  imsrtp_video_h264_config_v01 h264_specific_config;

  /* Optional */
  /*  MP4 Video Codec Configuration */
  uint8_t mp4_specific_config_valid;  /**< Must be set to true if mp4_specific_config is being passed */
  imsrtp_video_mp4_config_v01 mp4_specific_config;

  /* Optional */
  /*  H.263 Video Codec Configuration */
  uint8_t h263_specific_config_valid;  /**< Must be set to true if h263_specific_config is being passed */
  imsrtp_video_h263_config_v01 h263_specific_config;

  /* Optional */
  /*  Enable DTX */
  uint8_t enable_dtx_valid;  /**< Must be set to true if enable_dtx is being passed */
  uint8_t enable_dtx;
  /**<   Enables DTX. Values: \n
       - TRUE -- Enabled \n 
       - FALSE -- Disabled

       If the TLV is not included in the request, the default value 
       on the phone is used. 
  */

  /* Optional */
  /*  RAT Type */
  uint8_t dcm_rat_valid;  /**< Must be set to true if dcm_rat is being passed */
  imsrtp_dcm_rat_type_v01 dcm_rat;
  /**<   Radio access technology associated with this profile. \begin{itemize1}
       \item 0 -- IMSRTP_DCM_RAT_NONE -- None
       \item IMSRTP_DCM_RAT_GPRS -- GPRS  
       \item IMSRTP_DCM_RAT_EDGE -- EDGE  
       \item IMSRTP_DCM_RAT_WCDMA -- WCDMA  
       \item IMSRTP_DCM_RAT_WLAN -- WLAN 
       \item IMSRTP_DCM_RAT_CDMA -- CDMA 1.x
       \item IMSRTP_DCM_RAT_IWLAN -- IWLAN  
       \item IMSRTP_DCM_RAT_DOR0 -- CDMA DO Rev0  
       \item IMSRTP_DCM_RAT_DORA -- CDMA DO RevA  
       \item IMSRTP_DCM_RAT_EHRPD -- CDMA eHRPD  
       \item IMSRTP_DCM_RAT_LTE -- LTE  
       \item IMSRTP_DCM_RAT_DORB -- CDMA DO RevB  
       \item IMSRTP_DCM_RAT_EPC -- EPC 
       \end{itemize1} 
        
       If any of dcm_rat, apn_name, apn_type, or apn_addr_type are specified, 
       all must be specified.
  */

  /* Optional */
  /*  APN Name */
  uint8_t apn_name_valid;  /**< Must be set to true if apn_name is being passed */
  char apn_name[IMSRTP_APN_NAME_LEN_V01 + 1];
  /**<   APN name associated with this profile in presentation format. 
   
       If any of dcm_rat, apn_name, apn_type, or apn_addr_type are specified, 
       all must be specified.
  */

  /* Optional */
  /*  APN Type */
  uint8_t apn_type_valid;  /**< Must be set to true if apn_type is being passed */
  imsrtp_dcm_apn_type_v01 apn_type;
  /**<   APN type. \begin{itemize1}
       \item 1 -- IMSRTP_DCM_APN_IMS -- IMS
       \item IMSRTP_DCM_APN_INTERNET -- Internet  
       \item IMSRTP_DCM_APN_EMERGENCY -- Emergency  
       \item IMSRTP_DCM_APN_RCS -- RCS  
       \item IMSRTP_DCM_APN_UT -- UT 
       \end{itemize1} 

       If any of dcm_rat, apn_name, apn_type, or apn_addr_type are specified, 
       all must be specified.
  */

  /* Optional */
  /*  Address Type for PDN Bringup */
  uint8_t apn_addr_type_valid;  /**< Must be set to true if apn_addr_type is being passed */
  imsrtp_dcm_profile_ipaddr_type_v01 apn_addr_type;
  /**<   Address type used during PDN bringup.\begin{itemize1}
       \item 0 -- IMSRTP_DCM_UNKNOWN_ IP_TYPE -- UNKNOWN  
       \item IMSRTP_DCM_IPV4 -- IPV4 Address
       \item IMSRTP_DCM_IPV6 -- IPV6 Address
       \end{itemize1}

       If any of dcm_rat, apn_name, apn_type, or apn_addr_type are specified, 
       all must be specified.
  */

  /* Optional */
  /*  Enable RTCP XR */
  uint8_t enable_rtcp_xr_valid;  /**< Must be set to true if enable_rtcp_xr is being passed */
  uint8_t enable_rtcp_xr;
  /**<   Enables RTCP XR. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the Enable RTCP XR TLV is not included in the request, RTCP XR is 
       disabled. 
  */

  /* Optional */
  /*  RTCP Remote Port */
  uint8_t rtcp_remote_port_valid;  /**< Must be set to true if rtcp_remote_port is being passed */
  uint16_t rtcp_remote_port;
  /**<   Remote port to which RTCP packets are sent. */

  /* Optional */
  /*  Session Direction Bitmask */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Bitmask for the session direction. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction  
       \item 0x02 -- IMSRTP_RX -- Receive only direction 
       \vspace{-12pt}
       \end{itemize1}
  */

  /* Optional */
  /*  Reliable Transmission */
  uint8_t enable_reliable_transmission_valid;  /**< Must be set to true if enable_reliable_transmission is being passed */
  uint8_t enable_reliable_transmission;
  /**<   Enables reliable transmission. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the Reliable Transmission TLV is not included in the request, 
       reliable transmission is disabled. 
  */

  /* Optional */
  /*  File Streaming */
  uint8_t enable_file_streaming_valid;  /**< Must be set to true if enable_file_streaming is being passed */
  uint8_t enable_file_streaming;
  /**<   Enables file streaming. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the File Streaming TLV is not included in the request, file streaming
       is disabled. 
  */

  /* Optional */
  /*  Stream Source ID */
  uint8_t stream_source_id_valid;  /**< Must be set to true if stream_source_id is being passed */
  uint16_t stream_source_id;
  /**<   Stream source ID. When this TLV is not present, a default stream source 
       ID is used. */

  /* Optional */
  /*  RTP MTU Size */
  uint8_t rtp_mtu_size_valid;  /**< Must be set to true if rtp_mtu_size is being passed */
  uint16_t rtp_mtu_size;
  /**<   RTP maximum transfer unit in bytes. When this TLV is not present, 
       a default size provisioned for the phone is used. */

  /* Optional */
  /*  RTP Text Params */
  uint8_t rtp_txt_params_valid;  /**< Must be set to true if rtp_txt_params is being passed */
  imsrtp_text_config_params_v01 rtp_txt_params;
  /**<   RTP text-related parameters. */

  /* Optional */
  /*  RTP Text Params of Remote UE */
  uint8_t rtp_remote_txt_params_valid;  /**< Must be set to true if rtp_remote_txt_params is being passed */
  imsrtp_text_config_params_v01 rtp_remote_txt_params;
  /**<   RTP text-related parameters of the remote UE. */

  /* Optional */
  /*  Coordinated Video Orientation  */
  uint8_t enable_cvo_valid;  /**< Must be set to true if enable_cvo is being passed */
  uint8_t enable_cvo;
  /**<   Enables coordinated video orientation. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the coordinated video orientation TLV is not included in the request,
       coordinated video orientation is disabled. 
  */

  /* Optional */
  /*  Secured RTP  */
  uint8_t enable_srtp_valid;  /**< Must be set to true if enable_srtp is being passed */
  uint8_t enable_srtp;
  /**<   Enables Secured RTP. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the Secured RTP TLV is not included in the request, Secured RTP 
       is disabled. 
  */

  /* Optional */
  /*  Transmit Secured RTP Information */
  uint8_t tx_srtp_info_valid;  /**< Must be set to true if tx_srtp_info is being passed */
  imsrtp_srtp_info_v01 tx_srtp_info;
  /**<   Transmit Secured RTP related parameters. */

  /* Optional */
  /*  Receive Secured RTP Information */
  uint8_t rx_srtp_info_valid;  /**< Must be set to true if rx_srtp_info is being passed */
  imsrtp_srtp_info_v01 rx_srtp_info;
  /**<   Receive Secured RTP related parameters. */

  /* Optional */
  /*  Enable Transmission of RTCP Feedback Packets for AVPF Profile */
  uint8_t rtcp_tx_fb_enable_valid;  /**< Must be set to true if rtcp_tx_fb_enable is being passed */
  uint8_t rtcp_tx_fb_enable;
  /**<   Enables transmission of RTCP feedback packets. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If Enable Transmission of RTCP feedback packets TLV is not included
       in the request, transmission of RTCP feedback packets is disabled. 
  */

  /* Optional */
  /*  RTCP Feedback Messages Supported */
  uint8_t fb_messages_valid;  /**< Must be set to true if fb_messages is being passed */
  qvp_rtcp_fb_messages_v01 fb_messages;
  /**<   RTCP feedback messages supported.
       This TLV will be ignored if rtcp_tx_fb_enable TLV is disabled.
  */
}imsrtp_session_initialize_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Initializes the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_session_initialize_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Initializes the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  Application Identifier */
  uint8_t app_id;
  /**<   Unique identifier for the application using the RTP stack. */

  /* Optional */
  /*  RTP Session Identifier  */
  uint8_t session_id_valid;  /**< Must be set to true if session_id is being passed */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Optional */
  /*  Cookie */
  uint8_t cookie_valid;  /**< Must be set to true if cookie is being passed */
  uint32_t cookie;
  /**<   Value provided by the application in the session initialization 
       request to distinguish between multiple session initialization 
       requests. 
  */

  /* Optional */
  /*  Session Direction Bitmask */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Bitmask for the session direction. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction  
       \item 0x02 -- IMSRTP_RX -- Receive only direction 
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_sesion_initialize_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Uninitializes the RTP session. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier   */
  uint8_t session_id;
  /**<   Unique identifier of the RTP session. */
}imsrtp_session_uninitialize_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Uninitializes the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}imsrtp_session_uninitialize_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Uninitializes the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  RTP Session Identifier  */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */
}imsrtp_session_uninitialize_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Configures the RTP session. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier   */
  uint8_t session_id;
  /**<   Unique identifier of the RTP session. */

  /* Optional */
  /*  RTP Outbound Port - Deprecated (Use QMI_IMSRTP_SESSION_ INITIALIZE_REQ to  
      set the RTP outbound port.)  */
  uint8_t rtp_ob_local_port_valid;  /**< Must be set to true if rtp_ob_local_port is being passed */
  uint16_t rtp_ob_local_port;
  /**<   RTP outbound port through which the RTP packets are sent. 
  */

  /* Optional */
  /*  RTP Inbound Port - Deprecated (Use QMI_IMSRTP_SESSION_ INITIALIZE_REQ to  
      set the RTP inbound port.)  */
  uint8_t rtp_ib_port_valid;  /**< Must be set to true if rtp_ib_port is being passed */
  uint16_t rtp_ib_port;
  /**<   RTP inbound port from which RTP packets are received. */

  /* Optional */
  /*  Remote Transport IP Address  */
  uint8_t remote_ip_valid;  /**< Must be set to true if remote_ip is being passed */
  imsrtp_ip_addr_type_v01 remote_ip;

  /* Optional */
  /*  Remote Port */
  uint8_t remote_port_valid;  /**< Must be set to true if remote_port is being passed */
  uint16_t remote_port;
  /**<   Remote port to which RTP packets are sent. */

  /* Optional */
  /*  Codec Parameters */
  uint8_t codec_param_valid;  /**< Must be set to true if codec_param is being passed */
  imsrtp_codec_params_type_v01 codec_param;

  /* Optional */
  /*  Session Parameters */
  uint8_t session_param_valid;  /**< Must be set to true if session_param is being passed */
  imsrtp_session_params_type_v01 session_param;

  /* Optional */
  /*  AMR Media Codec Parameters  */
  uint8_t amr_config_type_valid;  /**< Must be set to true if amr_config_type is being passed */
  imsrtp_amr_config_type_v01 amr_config_type;

  /* Optional */
  /*  H.264 Media Codec Parameters */
  uint8_t h264_config_type_valid;  /**< Must be set to true if h264_config_type is being passed */
  imsrtp_h264_config_type_v01 h264_config_type;

  /* Optional */
  /*  RTCP Parameters - Deprecated (Use QMI_IMSRTP_SESSION_ INITIALIZE_REQ to 
      set the RTCP parameters.)  */
  uint8_t rtcp_param_valid;  /**< Must be set to true if rtcp_param is being passed */
  imsrtp_rtcp_params_type_v01 rtcp_param;

  /* Optional */
  /*  Common Video Codec Configuration */
  uint8_t video_codec_config_valid;  /**< Must be set to true if video_codec_config is being passed */
  imsrtp_video_config_type_v01 video_codec_config;

  /* Optional */
  /*  H.264 Video Codec Configuration */
  uint8_t h264_specific_config_valid;  /**< Must be set to true if h264_specific_config is being passed */
  imsrtp_video_h264_config_v01 h264_specific_config;

  /* Optional */
  /*  MP4 Video Codec Configuration */
  uint8_t mp4_specific_config_valid;  /**< Must be set to true if mp4_specific_config is being passed */
  imsrtp_video_mp4_config_v01 mp4_specific_config;

  /* Optional */
  /*  H.263 Video Codec Configuration */
  uint8_t h263_specific_config_valid;  /**< Must be set to true if h263_specific_config is being passed */
  imsrtp_video_h263_config_v01 h263_specific_config;

  /* Optional */
  /*  RTCP Bandwidth Allocated to Active Data Senders */
  uint8_t rs_bandwidth_valid;  /**< Must be set to true if rs_bandwidth is being passed */
  uint16_t rs_bandwidth;
  /**<   RTCP bandwidth in bits per second for active data senders. */

  /* Optional */
  /*  RTCP Bandwidth Allocated to Other Participants in RTP Session */
  uint8_t rr_bandwidth_valid;  /**< Must be set to true if rr_bandwidth is being passed */
  uint16_t rr_bandwidth;
  /**<   RTCP bandwidth in bits per second for other participants in a session. */

  /* Optional */
  /*  Enable DTX */
  uint8_t enable_dtx_valid;  /**< Must be set to true if enable_dtx is being passed */
  uint8_t enable_dtx;
  /**<    Enables DTX. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the Enable DTX TLV is not included in the request, the default value 
       on the phone is used.  
  */

  /* Optional */
  /*  Enable RTCP XR */
  uint8_t enable_rtcp_xr_valid;  /**< Must be set to true if enable_rtcp_xr is being passed */
  uint8_t enable_rtcp_xr;
  /**<   Enables RTCP XR. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the Enable RTCP XR TLV is not included in the request, RTCP XR is disabled. 
  */

  /* Optional */
  /*  RTCP Remote Port */
  uint8_t rtcp_remote_port_valid;  /**< Must be set to true if rtcp_remote_port is being passed */
  uint16_t rtcp_remote_port;
  /**<   Remote port to which RTCP packets are sent. */

  /* Optional */
  /*  Session Direction Bitmask */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Bitmask for the session direction. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction  
       \item 0x02 -- IMSRTP_RX -- Receive only direction 
       \vspace{-12pt}
       \end{itemize1}
  */

  /* Optional */
  /*  Cookie */
  uint8_t cookie_valid;  /**< Must be set to true if cookie is being passed */
  uint32_t cookie;
  /**<   Value provided by the application to distinguish the session  
       configuration requests. This value is returned in the corresponding  
       session configuration indication so the application can correlate the  
       indication with the request. 
  */

  /* Optional */
  /*  Reliable Transmission */
  uint8_t enable_reliable_transmission_valid;  /**< Must be set to true if enable_reliable_transmission is being passed */
  uint8_t enable_reliable_transmission;
  /**<   Enables reliable transmission. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the Reliable Transmission TLV is not included in the request, 
       reliable transmission is disabled. 
  */

  /* Optional */
  /*  File Streaming */
  uint8_t enable_file_streaming_valid;  /**< Must be set to true if enable_file_streaming is being passed */
  uint8_t enable_file_streaming;
  /**<   Enables file streaming. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the File Streaming TLV is not included in the request, file streaming 
       is disabled. 
  */

  /* Optional */
  /*  Stream Source ID */
  uint8_t stream_source_id_valid;  /**< Must be set to true if stream_source_id is being passed */
  uint16_t stream_source_id;
  /**<   Stream source ID. When this TLV is not present, a default stream source 
       ID is used. */

  /* Optional */
  /*  RTP MTU Size */
  uint8_t rtp_mtu_size_valid;  /**< Must be set to true if rtp_mtu_size is being passed */
  uint16_t rtp_mtu_size;
  /**<   RTP maximum transfer unit in bytes. When this TLV is not present, 
       a default size provisioned for the phone is used. */

  /* Optional */
  /*  RTP Text Params */
  uint8_t rtp_txt_params_valid;  /**< Must be set to true if rtp_txt_params is being passed */
  imsrtp_text_config_params_v01 rtp_txt_params;
  /**<   RTP text-related parameters. */

  /* Optional */
  /*  RTP Text Params of Remote UE */
  uint8_t rtp_remote_txt_params_valid;  /**< Must be set to true if rtp_remote_txt_params is being passed */
  imsrtp_text_config_params_v01 rtp_remote_txt_params;
  /**<   RTP text-related parameters of the remote UE. */

  /* Optional */
  /*  UI Orientation */
  uint8_t ui_orientation_valid;  /**< Must be set to true if ui_orientation is being passed */
  imsrtp_ui_orientation_v01 ui_orientation;
  /**<   UI orientation. */

  /* Optional */
  /*  Transmit Guaranteed Bitrate */
  uint8_t tx_gbr_valid;  /**< Must be set to true if tx_gbr is being passed */
  uint32_t tx_gbr;
  /**<   Guaranteed bitrate in kbps. */

  /* Optional */
  /*  Transmit Maximum Bitrate */
  uint8_t tx_mbr_valid;  /**< Must be set to true if tx_mbr is being passed */
  uint32_t tx_mbr;
  /**<   Maximum bitrate in kbps. */

  /* Optional */
  /*  Receive Guaranteed Bitrate */
  uint8_t rx_gbr_valid;  /**< Must be set to true if rx_gbr is being passed */
  uint32_t rx_gbr;
  /**<   Guaranteed bitrate in kbps. */

  /* Optional */
  /*  Receive Maximum Bitrate */
  uint8_t rx_mbr_valid;  /**< Must be set to true if rx_mbr is being passed */
  uint32_t rx_mbr;
  /**<   Maximum bitrate in kbps. */

  /* Optional */
  /*  Coordinated Video Orientation  */
  uint8_t enable_cvo_valid;  /**< Must be set to true if enable_cvo is being passed */
  uint8_t enable_cvo;
  /**<   Enables coordinated video orientation. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the coordinated video orientation TLV is not included in the request,
       coordinated video orientation is disabled. 
  */

  /* Optional */
  /*  Secured RTP  */
  uint8_t enable_srtp_valid;  /**< Must be set to true if enable_srtp is being passed */
  uint8_t enable_srtp;
  /**<   Enables Secured RTP . Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If the Secured RTP TLV is not included in the request, Secured RTP 
       is disabled. 
  */

  /* Optional */
  /*  Transmit Secured RTP Information */
  uint8_t tx_srtp_info_valid;  /**< Must be set to true if tx_srtp_info is being passed */
  imsrtp_srtp_info_v01 tx_srtp_info;
  /**<   Transmit Secured RTP related parameters */

  /* Optional */
  /*  Receive Secured RTP Information */
  uint8_t rx_srtp_info_valid;  /**< Must be set to true if rx_srtp_info is being passed */
  imsrtp_srtp_info_v01 rx_srtp_info;
  /**<   Receive Secured RTP related parameters */

  /* Optional */
  /*  Enable Transmission of RTCP Feedback Packets for AVPF Profile */
  uint8_t rtcp_tx_fb_enable_valid;  /**< Must be set to true if rtcp_tx_fb_enable is being passed */
  uint8_t rtcp_tx_fb_enable;
  /**<   Enables transmission of RTCP feedback packets. Values: \n
       - TRUE -- Enabled \n
       - FALSE -- Disabled

       If Enable Transmission of RTCP feedback packets TLV is not included
       in the request, transmission of RTCP feedback packets is disabled. 
  */

  /* Optional */
  /*  RTCP Feedback Messages Supported */
  uint8_t fb_messages_valid;  /**< Must be set to true if fb_messages is being passed */
  qvp_rtcp_fb_messages_v01 fb_messages;
  /**<   RTCP feedback messages supported.
       This TLV will be ignored if rtcp_tx_fb_enable TLV is disabled.
  */
  /* Optional */
  /*  Coordinated Video Orientation Tag ID */
  uint8_t cvo_tag_id_valid;  /**< Must be set to true if cvo_tag_id is being passed */
  uint16_t cvo_tag_id;
  /**<   Coordinated video orientation Tag ID used in RTP extension header.
       If cvo_tag_id is not included in the request, 
       an appropriate default value determined by the service will be 
       used.
  */

  /* Optional */
  /*  Enable Early Media Blocking */
  uint8_t enable_early_media_blocking_valid;  /**< Must be set to true if enable_early_media_blocking is being passed */
  uint8_t enable_early_media_blocking;
  /**<   Enables Early Media blocking. Values: \n
       - TRUE -- Enabled \n 
       - FALSE -- Disabled

       If the TLV is not included in the request, early media blocking is disabled. 
  */
}imsrtp_session_configure_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Configures the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}imsrtp_session_configure_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Configures the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  RTP Session Identifier   */
  uint8_t session_id;
  /**<   Unique identifier of the RTP session. */

  /* Optional */
  /*  Session Direction Bitmask */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Bitmask for the session direction. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction  
       \item 0x02 -- IMSRTP_RX -- Receive only direction 
       \vspace{-12pt}
       \end{itemize1}
  */

  /* Optional */
  /*  Cookie */
  uint8_t cookie_valid;  /**< Must be set to true if cookie is being passed */
  uint32_t cookie;
  /**<   Value provided by the application in the session configuration request 
       to distinguish between multiple session configuration requests.  
  */
}imsrtp_session_configure_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Starts the RTP session. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier  */
  uint8_t session_id;
  /**<   Unique identifier of the RTP session. */

  /* Optional */
  /*  Session Direction Bitmask */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Bitmask for the session direction. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction  
       \item 0x02 -- IMSRTP_RX -- Receive only direction 
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_session_start_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Starts the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */
}imsrtp_session_start_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Starts the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier of the RTP session. */

  /* Optional */
  /*  Session Direction Bitmask */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Bitmask for the session direction. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction  
       \item 0x02 -- IMSRTP_RX -- Receive only direction 
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_session_start_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Stops the RTP session. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier  */
  uint8_t session_id;
  /**<   Unique identifier of the RTP session. */

  /* Optional */
  /*  Session Direction Bitmask */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Bitmask for the session direction. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction 
       \item 0x02 -- IMSRTP_RX -- Receive only direction
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_session_stop_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Stops the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_session_stop_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Stops the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  RTP Session Identifier   */
  uint8_t session_id;
  /**<   Unique identifier of the RTP session. */

  /* Optional */
  /*  Session Direction Bitmask */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Bitmask for the session direction. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction  
       \item 0x02 -- IMSRTP_RX -- Receive only direction 
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_session_stop_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Pauses the RTP session for a specific direction. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier  */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Optional */
  /*  Session Direction Bitmask */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Bitmask for the session direction. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction  
       \item 0x02 -- IMSRTP_RX -- Receive only direction  
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_session_pause_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Pauses the RTP session for a specific direction. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_session_pause_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Resumes the RTP session. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier  */
  uint8_t session_id;
  /**<   Unique identifier of the RTP session. */

  /* Optional */
  /*  Session Direction */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Session direction to be resumed. Values: \begin{itemize1}
       \vspace{-12pt}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction  
       \item 0x02 -- IMSRTP_RX -- Receive only direction  
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_session_resume_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Resumes the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_session_resume_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Configures RTCP reports for the session. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier  */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Mandatory */
  /*  Enable RTCP Reports */
  uint8_t enable;
  /**<   Indicates whether RTCP reports are to be enabled. */

  /* Optional */
  /*  RTCP Reports Interval */
  uint8_t report_interval_valid;  /**< Must be set to true if report_interval is being passed */
  uint16_t report_interval;
  /**<   RTCP reports interval in seconds. */
}imsrtp_configure_rtcp_reports_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Configures RTCP reports for the session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_configure_rtcp_reports_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Configures RTCP reports for the session. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */
}imsrtp_configure_rtcp_reports_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Enables and configures RTP link monitoring. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Mandatory */
  /*  Enable RTCP Reports  */
  uint8_t enable;
  /**<   Indicates whether RTCP reports are to be enabled. */

  /* Optional */
  /*  RTP Link Monitoring Interval */
  uint8_t interval_valid;  /**< Must be set to true if interval is being passed */
  uint16_t interval;
  /**<   RTP link monitoring interval in seconds. */
}imsrtp_configure_rtp_link_monitoring_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Enables and configures RTP link monitoring. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_configure_rtp_link_monitoring_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Enables and configures RTP link monitoring. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */
}imsrtp_configure_rtp_link_monitoring_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Configures RTCP link monitoring. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Mandatory */
  /*  Enable RTCP Reports  */
  uint8_t enable;
  /**<   Indicates whether RTCP reports are to be enabled. */

  /* Optional */
  /*  RTP Link Monitoring Interval */
  uint8_t interval_valid;  /**< Must be set to true if interval is being passed */
  uint16_t interval;
  /**<   RTP link monitoring interval in seconds. */
}imsrtp_configure_rtcp_link_monitoring_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Configures RTCP link monitoring. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_configure_rtcp_link_monitoring_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Configures RTCP link monitoring. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */
}imsrtp_configure_rtcp_link_monitoring_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Request to send DTMF packets. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Mandatory */
  /*  DTMF Digit */
  uint8_t dtmf_digit;
  /**<   Actual DTMF digit to transport. */

  /* Mandatory */
  /*  DTMF Digit Volume  */
  uint8_t volume;
  /**<   Volume of the DTMF digit. Values: 0 to 63 dBm0.*/

  /* Mandatory */
  /*  DTMF Tone Duration */
  uint16_t duration;
  /**<   DTMF tone duration in milliseconds. */
}imsrtp_send_dtmf_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Request to send DTMF packets. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_send_dtmf_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Starts sending continuous DTMF packets. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Mandatory */
  /*  DTMF Digit  */
  uint8_t dtmf_digit;
  /**<   Actual DTMF digit to transport. */

  /* Mandatory */
  /*  DTMF Digit Volume */
  uint8_t volume;
  /**<   Volume of the DTMF digit. Values: 0 to 63 dBm0. */
}imsrtp_start_continuous_dtmf_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Starts sending continuous DTMF packets. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_start_continuous_dtmf_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Stops sending continuous DTMF packets. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */
}imsrtp_stop_continuous_dtmf_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Stops sending continuous DTMF packets. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_stop_continuous_dtmf_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Flushes the audio jitter buffer. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */
}imsrtp_flush_audio_jitterbuffer_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Flushes the audio jitter buffer. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_flush_audio_jitterbuffer_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Gets the RTP session status. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */
}imsrtp_get_session_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Gets the RTP session status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_get_session_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Gets the RTP session status. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Optional */
  /*  RTP Session Active Status */
  uint8_t session_active_valid;  /**< Must be set to true if session_active is being passed */
  uint8_t session_active;
  /**<   Indicates whether the RTP session is active. */

  /* Optional */
  /*  Session State Bitmask  */
  uint8_t direction_valid;  /**< Must be set to true if direction is being passed */
  imsrtp_session_direction_mask_type_v01 direction;
  /**<   Bitmask for the session direction paused. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_TX -- Transmit only direction  
       \item 0x02 -- IMSRTP_RX -- Receive only direction  
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_get_session_status_ind_v01;  /* Message */
/**
    @}
  */

/**  RTP services for which clients can register.  */
typedef uint64_t imsrtp_services_mask_type_v01;
#define IMSRTP_ERROR_SERVICE_V01 ((imsrtp_services_mask_type_v01)0x01ull) 
#define IMSRTP_RTCP_REPORTS_SERVICE_V01 ((imsrtp_services_mask_type_v01)0x02ull) 
#define IMSRTP_VIDEO_IDR_GENERATE_SERVICE_V01 ((imsrtp_services_mask_type_v01)0x04ull) 
#define IMSRTP_VIDEO_BITRATE_ADAPT_SERVICE_V01 ((imsrtp_services_mask_type_v01)0x08ull) 
/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Registers the RTP for various services. */
typedef struct {

  /* Mandatory */
  /*  Application Identifier  */
  uint8_t app_id;
  /**<   Unique identifier for the application using the RTP stack. */

  /* Mandatory */
  /*  RTP Services Bitmask */
  imsrtp_services_mask_type_v01 rtp_service;
  /**<   Bitmask element for RTP services. Values: \begin{itemize1}
       \item 0x01 -- IMSRTP_ERROR_SERVICE -- Indicates the client wants to 
                     register to receive unsolicited error indications
       \item 0x02 -- IMSRTP_RTCP_REPORTS_ SERVICE -- Indicates the client wants 
                     to register to receive RTCP reports
       \item 0x03 -- IMSRTP_VIDEO_IDR_GENERATE_SERVICE -- Indicates the client 
                     wants to register to receive Video IDR generation 
                     indications
       \item 0x04 -- IMSRTP_VIDEO_BITRATE_ADAPT_SERVICE  -- Indicates the 
                     client wants to register to receive Video bitrate 
                     adapt indications
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_register_rtp_services_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Registers the RTP for various services. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_register_rtp_services_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Sends unsolicited errors that occurred in the RTP. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \item 0x09 -- IMSRTP_EXT_IPV6_ADDR_ DELETED -- External IPv6 address is 
                     deleted
       \end{itemize1}
       
       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */
}imsrtp_error_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ssrc;
  /**<   SSRC ID from the sender. */

  uint32_t hi_ntp;
  /**<   Most significant NTP timestamp. */

  uint32_t lo_ntp;
  /**<   Least significant NTP timestamp. */

  uint32_t tstamp;
  /**<   Streams the timestamp when the timestamp NTP was taken. */

  uint32_t num_pkts;
  /**<   Number of packets transmitted. */

  uint32_t num_bytes;
  /**<   Number of bytes transmitted. */
}imsrtp_rtcp_sr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ssrc;
  /**<   SSRC ID from the sender. */

  uint32_t s_ssrc;
  /**<   SSRC ID of the source. */

  uint8_t frac_lost;
  /**<   Fraction of RTP data packets since the previous SR or RR packet was 
        sent, expressed as a fixed point number with the binary point at the 
        left edge of the field. If the loss is negative due to duplication, 
        the fraction lost is set to zero by the client.
   */

  uint32_t tot_lost;
  /**<   Total number of RTP data packets from the source SSRC that have been 
        lost since the beginning of reception. */

  uint32_t ext_seq;
  /**<   Extended highest sequence.  */

  uint32_t est_jitter;
  /**<   Estimated jitter.  */

  uint32_t lsr;
  /**<   Last Sender Report timestamp. If no SR has been received yet, the field 
        is set to zero.
    */

  uint32_t dslr;
  /**<   Delay since last the Sender Report. If no SR has been received yet,  
        the field is set to zero. The delay is expressed in units of 
        1/65536 seconds. The first 16 bits are represented as an integer part, 
        and the last 16 bits are represented as a fraction part.
   */

  uint32_t rtt;
  /**<   Round trip time calculated. The round trip delay is expressed in seconds. 
        The first 16 bits are represented as an integer part, and the last 
        16 bits are represented as a fraction part.
   */
}imsrtp_rtcp_rr_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMSRTP_RTCP_SDES_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_RTCP_END_V01 = 0x00, 
  IMSRTP_RTCP_CNAME_V01 = 0x01, 
  IMSRTP_RTCP_NAME_V01 = 0x02, 
  IMSRTP_RTCP_EMAIL_V01 = 0x03, 
  IMSRTP_RTCP_PHONE_V01 = 0x04, 
  IMSRTP_RTCP_LOC_V01 = 0x05, 
  IMSRTP_RTCP_TOOL_V01 = 0x06, 
  IMSRTP_RTCP_NOTE_V01 = 0x07, 
  IMSRTP_RTCP_PRIV_V01 = 0x08, 
  IMSRTP_RTCP_SDES_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsrtp_rtcp_sdes_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ssrc;
  /**<   Synchronization source identifier uniquely identifies the source of a 
       session. */

  imsrtp_rtcp_sdes_enum_v01 subtype;
  /**<   SDES item type. Values: \begin{itemize1}
       \item 0x0 -- IMSRTP_RTCP_END 
       \item 0x1 -- IMSRTP_RTCP_CNAME 
       \item 0x2 -- IMSRTP_RTCP_NAME    
       \item 0x3 -- IMSRTP_RTCP_EMAIL  
       \item 0x4 -- IMSRTP_RTCP_PHONE  
       \item 0x5 -- IMSRTP_RTCP_LOC     
       \item 0x6 -- IMSRTP_RTCP_TOOL   
       \item 0x7 -- IMSRTP_RTCP_NOTE   
       \item 0x8 -- IMSRTP_RTCP_PRIV 
       \vspace{-12pt}
       \end{itemize1}
  */

  char data[IMSRTP_RTCP_SDES_ITEM_LEN_V01 + 1];
  /**<   SDES item. */
}imsrtp_rtcp_sdes_item_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMS_RTP_RX_CONFIG_PACKET_LOSS_CONCEALMENT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_PLC_UNSPECIFIED_V01 = 0x00, /**<  No information is available concerning the use of PLC.  */
  IMSRTP_PLC_DISABLED_V01 = 0x01, /**<  Silence is inserted in place of the lost packets.  */
  IMSRTP_PLC_ENHANCED_V01 = 0x02, /**<  Enhanced interpolation algorithm is used. Algorithms of this 
         type can effectively conceal high packet loss rates.  */
  IMSRTP_PLC_STANDARD_V01 = 0x03, /**<  Simple replay or an interpolation algorithm is used to fill in the  
         missing packet. This approach can typically conceal isolated lost 
         packets at low packet loss rates.  */
  IMS_RTP_RX_CONFIG_PACKET_LOSS_CONCEALMENT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_rtp_rx_config_packet_loss_concealment_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_enums
    @{
  */
typedef enum {
  IMS_RTP_RX_CONFIG_JITTER_BUFFER_ADAPTIVE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSRTP_JBA_UNKNOWN_V01 = 0x00, /**<  No information is available concerning the jitter buffer size.  */
  IMSRTP_JBA_NON_ADAPTIVE_V01 = 0x01, /**<  Jitter buffer size is maintained at a fixed level.  */
  IMSRTP_JBA_ADAPTIVE_V01 = 0x02, /**<  Jitter buffer size is dynamically adjusted to accommodate varying 
         levels of jitter.  */
  IMS_RTP_RX_CONFIG_JITTER_BUFFER_ADAPTIVE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ims_rtp_rx_config_jitter_buffer_adaptive_enum_v01;
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ssrc;
  /**<   SSRC ID from the sender. */

  uint8_t loss_rate;
  /**<    Fraction of RTP data packets from the source lost since the beginning
         of reception. This value is expressed as a fixed-point number with the 
         binary point at the left edge of the field. \n 
         To calculate this value: \n 
         1. Divide the total number of packets lost (after
         the effects of applying any error protection such as FEC) by
         the total number of packets expected. \n 
         2. Multiply the division result by 256, limiting the maximum value 
         to 255 to avoid overflow. \n 
         3. Take the integer part as the value.
   */

  uint8_t discard_rate;
  /**<    Fraction of RTP data packets from the source that have been discarded 
         since the beginning of reception, due to late or early arrival, or 
         underrun or overflow at the receiving jitter buffer. This value is 
         expressed as a fixed-point number with the binary point at the left
         edge of the field. \n 
         To calculate this value:  \n 
         1. Divide the total number of packets discarded (excluding duplicate 
         packet discards) by the total number of packets expected. \n
         2. Multiply the division result by 256, limiting the maximum value to 
         255 to avoid overflow. \n 
         3. Take the integer part as the value.
   */

  uint8_t burst_density;
  /**<    Fraction of RTP data packets within burst periods since the beginning 
         of reception that were either lost or discarded. This value is 
         expressed as a fixed-point number with the binary point at the left 
         edge of the field. \n 
         To calculate this value:  \n  
         1. Divide the total number of packets lost or discarded (excluding 
         duplicate packet discards) within burst periods by	the total number of
         packets expected within the burst periods. \n 
         2. Multiply the division result by 256, limiting the maximum value to 
         255 to avoid overflow. \n 
         3. Take the integer part as the value. This field is set to zero if  
         no packets have been received.
   */

  uint8_t gap_density;
  /**<    Fraction of RTP data packets within inter-burst gaps since	the  
         beginning of reception that were either lost or discarded. The value
         is expressed as a fixed-point number with the binary point at the left
         edge of the field.  \n 
         To calculate this value:  \n 
         1. Divide the total number of packets lost or discarded (excluding 
         duplicate packet discards) within gap periods by the total number of 
         packets expected within the gap periods. \n 
         2. Multiply the division result by 256, limiting the maximum value to 
         255 to avoid overflow. \n 
         3. Take the integer part as the value. This field is set to zero if 
         no packets have been received.
   */

  uint16_t burst_duration;
  /**<    Mean duration, expressed in milliseconds, of the burst periods that 
         have occurred since the beginning of reception. The duration of each
         period is calculated based upon the packets that mark the beginning  
         and end of that period. The duration is equal to the timestamp of the  
         end packet, plus the duration of the end packet, minus the timestamp 
         of the beginning packet. \n
         \vspace{-3pt}
         If the actual values are not available, estimated values are used. If 
         there have been no burst periods, the burst duration value is zero.
   */

  uint16_t gap_duration;
  /**<    Mean duration, expressed in milliseconds, of the gap periods that have 
         occurred since the beginning of reception. The duration of each period 
         is calculated based upon the packet that marks the end of the prior 
         burst and the packet that marks the beginning of the subsequent burst. 
         The duration is equal to the timestamp of the subsequent burst packet, 
         minus the timestamp of the prior burst packet, plus the duration of 
         the prior burst packet. \n
         \vspace{-3pt}
         If the actual values are not available, estimated values are used. \n
         \vspace{-3pt}
         When a gap occurs at the beginning of reception, the sum of the 
         timestamp of the prior burst packet and the duration of the prior 
         burst packet are replaced by the reception start time. \n
         \vspace{-3pt}
         When a gap occurs at the end of reception, the timestamp of the
         subsequent burst packet is replaced by the reception end time. \n
         \vspace{-3pt}
         If there have been no gap periods, the gap duration value is zero.
   */

  uint16_t round_trip_delay;
  /**<    Most recently calculated round-trip time between the RTP interfaces, 
         expressed in milliseconds.
   */

  uint16_t end_system_delay;
  /**<    Most recently estimated end system delay, expressed in milliseconds.
         The end system delay is defined as the sum of the total sample 
         accumulation and encoding delay associated with the sending direction
         and the jitter buffer, decoding, and playout buffer delay associated 
         with the receiving direction. This delay may be estimated or measured.

         If the service is unable to provide the data, the value is set to 
         zero.
   */

  uint8_t signal_level;
  /**<    Voice signal relative level defined as the ratio of the signal level 
         to a 0 dBm0 reference, expressed in decibels as a signed integer in 
         two's complement format. This is measured for packets containing 
         speech energy only.

         A value of 127 indicates that this parameter is unavailable.
   */

  uint8_t noise_level;
  /**<    Noise level defined as the ratio of the silent period background noise
         level to a 0 dBm0 reference, expressed in decibels as a signed integer
         in two's complement format.

         A value of 127 indicates that this parameter is unavailable.
   */

  uint8_t residual_echo_return_loss;
  /**<    Residual echo return loss value measured directly by the VoIP-end 
         system's echo canceller or may be estimated by adding the Echo Return 
         Loss (ERL) and Echo Return Loss Enhancement (ERLE) values reported by 
         the echo canceller.
   */

  uint8_t gap_threshhold;
  /**<    Gap threshold. This field contains the value used for this report 
         block to determine if a gap exists.

         This value corresponds to the Gmin value in the RTCP XR VoIP metrics 
         report.
   */

  uint8_t r_factor;
  /**<    Voice quality metric that describes the segment of the call carried 
         over this RTP session. Values: an integer. Range: 0 to 100. \n
         - 94 -- Toll quality \n
         - >=50 -- Unusable

         A value of 127 indicates that this parameter is unavailable.
   */

  uint8_t ext_r_factor;
  /**<    Voice quality metric that describes the segment of the call that is 
         carried over a network segment external to the RTP segment.

         A value of 127 indicates that this parameter is unavailable.
   */

  uint8_t mos_lq;
  /**<    Estimated mean opinion score for listening quality. This voice quality  
         metric is on a scale from 1 to 5, in which 5 represents excellent 
         quality and 1 represents unacceptable quality. Values: an integer. 
         Range: 10 to 50, corresponding to MOS x 10 (e.g., a value of 35  
         corresponds to an estimated MOS score of 3.5).

         A value of 127 indicates that this parameter is unavailable.
   */

  uint8_t mos_cq;
  /**<    Estimated mean opinion score for conversational quality. This metric 
         includes the effects of delay and other factors that affect 
         conversational quality. Values: an integer. Range: 10 to 50.

         A value of 127 indicates that this parameter is unavailable.
   */

  ims_rtp_rx_config_packet_loss_concealment_enum_v01 packet_loss_concealment;
  /**<   Packet loss concealment. \begin{itemize1}
        \item 0x00 -- IMSRTP_PLC_UNSPECIFIED -- No information available 
                      concerning the use of PLC 
        \item IMSRTP_PLC_DISABLED -- Silence is being inserted in place of lost 
                      packets
        \item IMSRTP_PLC_ENHANCED -- Enhanced interpolation algorithm is being 
                      used; algorithms of this type are able to conceal high 
                      packet loss rates effectively
        \item IMSRTP_PLC_STANDARD -- Simple replay or interpolation algorithm 
                      is being used to fill-in the missing packet; this
                      approach is typically able to conceal isolated lost 
                      packets at low packet loss rates
        \vspace{-12pt}
        \end{itemize1} 
    */

  ims_rtp_rx_config_jitter_buffer_adaptive_enum_v01 jitter_buffer_adaptive;
  /**<   Jitter buffer adaptiveness. \begin{itemize1}
        \item 0x00 -- IMSRTP_JBA_UNKNOWN -- No information available concerning 
                      the jitter buffer size
        \item IMSRTP_JBA_NON_ADAPTIVE -- Jitter buffer size is maintained at a 
                      fixed level
        \item IMSRTP_JBA_ADAPTIVE -- Jitter buffer size is being dynamically 
                      adjusted to deal with varying levels of jitter.
        \vspace{-12pt}
        \end{itemize1}*/

  uint8_t jitter_buffer_rate;
  /**<    Implementation-specific adjustment rate of a jitter buffer in adaptive 
         mode (if J = adjustment rate 0 to 15). This parameter is defined in 
         terms of the approximate time taken to fully adjust to a step change  
         in peak-to-peak jitter	from 30 ms to 100 ms such that: 
         \n
         \n
         adjustment time = 2 * J * frame size (ms)
         \n
         \n
         A value of 0 indicates that the adjustment time is unknown for this 
         implementation.
   */

  uint16_t jitter_buffer_nominal;
  /**<    Current nominal jitter buffer delay in milliseconds that corresponds 
         to the nominal jitter buffer delay for packets that arrive exactly on 
         time.
   */

  uint16_t jitter_buffer_maximum;
  /**<    Current maximum jitter buffer delay in milliseconds that corresponds 
         to the earliest arriving packet that is not to be discarded.
   */

  uint16_t jitter_buffer_abs_max;
  /**<    Absolute maximum delay in milliseconds that the adaptive jitter buffer 
         can reach under worst case conditions.

         If this value exceeds 65535 ms, the field is set to 65535.
         
   */
}imsrtp_rtcp_xr_voip_metrics_report_block_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ssrc;
  /**<   SSRC ID from the receiver. */

  uint32_t last_rr_timestamp;
  /**<    Middle 32 bits out of 64 in the NTP timestamp received as part of a 
         Receiver Reference Time Report Block from participant SSRC_n.  

         If a block of this type has not been received, the field is set to 
         zero. 
   */

  uint32_t delay_since_last_rr;
  /**<    Delay, expressed in units of 1/65536 sec, between receiving the 
         last Receiver Reference Time Report Block from	participant SSRC_n 
         and sending this DLRR Report Block. 
 
         If a Receiver Reference Time Report Block has not been received
         from SSRC_n, the DLRR field is set to zero. 
   */

  float rtt;
  /**<   Round-trip time. The round trip delay is expressed in seconds. 
        The first 16 bits are represented as an integer part, and the last 
        16 bits are represented as a fraction part.
   */
}imsrtp_rtcp_xr_dllr_report_block_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t ssrc;
  /**<   SSRC ID from the source. */

  uint16_t begin_seq;
  /**<    First sequence number on which this block reports. */

  uint16_t end_seq;
  /**<    Last sequence number that this block reports on plus one. */

  uint32_t lost_packets;
  /**<    Number of lost packets in the sequence number interval defined by 
         begin_seq and end_seq. */

  uint32_t dup_packets;
  /**<   Number of duplicate packets in the sequence number interval defined by 
        begin_seq and end_seq.  */

  uint32_t min_jitter;
  /**<   Minimum relative transit time between two packets in the sequence 
        number interval defined by begin_seq and end_seq. All jitter values 
        are measured as the difference between a packet's RTP timestamp and 
        the reporter's clock at the time of arrival, measured in milliseconds.
   */

  uint32_t max_jitter;
  /**<   Maximum relative transit time between two packets in the sequence 
        number interval defined by begin_seq and end_seq, measured in 
        milliseconds.
   */

  uint32_t mean_jitter;
  /**<   Mean relative transit time between each two packet series in the 
        sequence number interval defined by begin_seq and end_seq, rounded to 
        the nearest value expressible as an RTP timestamp.
   */

  uint32_t dev_jitter;
  /**<    Standard deviation of the relative transit time between
         each two packet series in the sequence number interval defined by 
         begin_seq and end_seq.
   */

  imsrtp_ip_ver_enum_v01 ipaddr_type;
  /**<   IP address type used. Values: \begin{itemize1}
        \item 0x00 -- IMSRTP_IPV4 -- 32-bit IPv4 address 
        \item 0x01 -- IMSRTP_IPV6 -- 128-bit IPv6 address; interface-specific  
                                     IP address generated by the application   
                                     and given to the RTP
        \vspace{-12pt}                              
        \end{itemize1}
   */

  uint8_t min_ttl_or_hl;
  /**<    Minimum TTL or hop limit value of data packets in the sequence 
         number range.

         For an IPv4 address type, this parameter is measured in seconds.

         For IPv6, it is measured in an integer number of hops.														.
   */

  uint8_t max_ttl_or_hl;
  /**<    Maximum TTL or hop limit value of data packets in the sequence number 
         range.

         For an IPv4 address type, this parameter is measured in seconds. 

         For IPv6, it is measured in an integer number of hops.
   */

  uint8_t mean_ttl_or_hl;
  /**<   Mean TTL or hop limit value of data packets in the sequence number 
        range, rounded to the nearest integer.

        For an IPV4 address type, this parameter is measured in seconds. 

        For IPv6, it is measured in an integer number of hops.
   */

  uint8_t dev_ttl_or_hl;
  /**<   Standard deviation of TTL or hop limit value of data packets in the 
        sequence number range.

        For an IPV4 address type, this parameter is measured in seconds.

        For IPv6, it is measured in an integer number of hops.
   */
}imsrtp_rtcp_xr_statistics_summary_report_block_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Sends various RTCP reports to the client. */
typedef struct {

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values: \begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Optional */
  /*  Sender Report Information  */
  uint8_t sr_info_valid;  /**< Must be set to true if sr_info is being passed */
  imsrtp_rtcp_sr_type_v01 sr_info;

  /* Optional */
  /*  Receiver Report Information  */
  uint8_t rr_info_valid;  /**< Must be set to true if rr_info is being passed */
  imsrtp_rtcp_rr_type_v01 rr_info;

  /* Optional */
  /*  SDES Report Information */
  uint8_t sdes_info_valid;  /**< Must be set to true if sdes_info is being passed */
  imsrtp_rtcp_sdes_item_type_v01 sdes_info;

  /* Optional */
  /*  XR Receiver Reference Time */
  uint8_t xr_receiver_reference_time_valid;  /**< Must be set to true if xr_receiver_reference_time is being passed */
  uint64_t xr_receiver_reference_time;
  /**<   Network Time Protocol timestamp. */

  /* Optional */
  /*  XR Statistics Summary Report Information */
  uint8_t xr_statistics_summary_report_valid;  /**< Must be set to true if xr_statistics_summary_report is being passed */
  imsrtp_rtcp_xr_statistics_summary_report_block_type_v01 xr_statistics_summary_report;

  /* Optional */
  /*  XR DLLR Report Information */
  uint8_t xr_dllr_report_valid;  /**< Must be set to true if xr_dllr_report is being passed */
  imsrtp_rtcp_xr_dllr_report_block_type_v01 xr_dllr_report;

  /* Optional */
  /*  XR VoIP Metrics Report Information */
  uint8_t xr_voip_metrics_report_valid;  /**< Must be set to true if xr_voip_metrics_report is being passed */
  imsrtp_rtcp_xr_voip_metrics_report_block_type_v01 xr_voip_metrics_report;
}imsrtp_rtcp_reports_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Sends the last audio play time. */
typedef struct {

  /* Mandatory */
  /*  Last Played Audio Timestamp */
  uint32_t last_play_time_stamp;
  /**<   Last played audio timestamp. */
}imsrtp_last_audio_play_time_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
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
}imsrtp_last_audio_play_time_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Reports the sender RTP audio packet statistics. */
typedef struct {

  /* Mandatory */
  /*  Timestamp Information */
  imsrtp_video_time_info_type_v01 time_info;
}imsrtp_audio_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
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
}imsrtp_audio_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Reports the sender RTP video packet statistics. */
typedef struct {

  /* Mandatory */
  /*  Timestamp Information */
  imsrtp_video_time_info_type_v01 time_info;
}imsrtp_video_report_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
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
}imsrtp_video_report_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Generates the H.264 SPS/PPS message. */
typedef struct {

  /* Mandatory */
  /*  Codec Configuration */
  imsrtp_video_config_type_v01 video_codec_config;

  /* Mandatory */
  /*  H.264 Video Codec Configuration */
  imsrtp_video_h264_config_v01 h264_config;
}imsrtp_gen_h264_sps_pps_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
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
  /*  H.264 NAL Header in ASCII Format */
  uint8_t nalheader_valid;  /**< Must be set to true if nalheader is being passed */
  uint32_t nalheader_len;  /**< Must be set to # of elements in nalheader */
  uint8_t nalheader[IMS_RTP_NAL_HEADER_MAX_V01];
  /**<   H.264 NAL header. */
}imsrtp_gen_h264_sps_pps_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Generates the MP4 VOL header. */
typedef struct {

  /* Mandatory */
  /*  Codec Configuration */
  imsrtp_video_config_type_v01 video_codec_config;

  /* Mandatory */
  /*  MP4 Video Codec Configuration */
  imsrtp_video_mp4_config_v01 mp4_config;
}imsrtp_gen_mp4_vol_header_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
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
  /*  MPEG4 VOL Header in ASCII Format */
  uint8_t volheader_valid;  /**< Must be set to true if volheader is being passed */
  uint32_t volheader_len;  /**< Must be set to # of elements in volheader */
  uint8_t volheader[IMS_RTP_VOL_HEADER_MAX_V01];
  /**<   MPEG-4 VOL header. */
}imsrtp_gen_mp4_vol_header_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Adapts the encoder to a new bitrate. */
typedef struct {

  /* Mandatory */
  /*  Bitrate */
  uint32_t bit_rate;
  /**<   Bitrate in kbps. */
}imsrtp_video_bitrate_adapt_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Adapts the encoder to a new bitrate. */
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
}imsrtp_video_bitrate_adapt_resp_msg_v01;  /* Message */
/**
    @}
  */

/*
 * imsrtp_h264_idr_generate_req_msg is empty
 * typedef struct {
 * }imsrtp_h264_idr_generate_req_msg_v01;
 */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Generates an IDR frame. */
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
}imsrtp_h264_idr_generate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Feeds the AV sync timestamp reference. */
typedef struct {

  /* Mandatory */
  /*  64-bit AV Sync Feed */
  uint64_t n_iavsync_time;
  /**<   64-bit audio/video synchronization timestamp. */
}imsrtp_av_sync_feed_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Feeds the AV sync timestamp reference. */
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
}imsrtp_av_sync_feed_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Reports asynchronous errors to the client on the modem. */
typedef struct {

  /* Mandatory */
  /*  Asynchronous Error During Video Codec Operation */
  imsrtp_video_codec_error_enum_v01 video_codec_error;
  /**<   Video codec error. Values: \begin{itemize1}
       \item 0 -- Generic
       \item 1 -- Video encoder
       \item 2 -- Video decoder
       \vspace{-12pt}
       \end{itemize1}
  */
}imsrtp_video_error_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Uninitializes all the RTP sessions. */
typedef struct {

  /* Mandatory */
  /*  Application Identifier */
  uint8_t app_id;
  /**<   Unique identifier for the application using the RTP stack. */
}imsrtp_uninitialize_all_rtp_session_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Uninitializes all the RTP sessions. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_uninitialize_all_rtp_session_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Uninitializes all the RTP sessions. */
typedef struct {

  /* Mandatory */
  /*  Application Identifier */
  uint8_t app_id;
  /**<   Unique identifier for the application using the RTP stack. */

  /* Mandatory */
  /*  Indication Status */
  ims_rtp_status_enum_v01 status;
  /**<   Standard RTP status. Values:\begin{itemize1}
       \item 0x00 -- IMSRTP_SUCCESS -- Operation requested was successful 
       \item 0x01 -- IMSRTP_NORESOURCES -- Operation failed; no resources 
       \item 0x02 -- IMSRTP_WRONG_PARAM -- Operation failed; at least one 
                     parameter was unacceptable 
       \item 0x03 -- IMSRTP_ERR_FATAL -- Unknown but fatal error 
       \item 0x04 -- IMSRTP_PORT_ NOTAVAILABLE -- Unable to bind to a specific 
                     I/B port
       \item 0x05 -- IMSRTP_NOT_SUPPORTED -- Feature or primitive is not 
                     supported at this time 
       \item 0x06 -- IMSRTP_QDJ_ENQ_ERR -- Error; enqueueing failed in jitter 
                     buffer 
       \item 0x07 -- IMSRTP_MEDIA_TX_ ONLY_ACTIVE -- Media flow Tx only is 
                     active 
       \item 0x08 -- IMSRTP_MEDIA_RX_ ONLY_ACTIVE -- Media flow Rx only is 
                     active
       \end{itemize1}

       Note: For values greater than Max enum, the client is to treat this as 
       IMSRTP_ERR_FATAL. 
  */
}imsrtp_uninitialize_all_rtp_session_ind_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Sends unsolicited video IDR generation indications. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier  */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */
}imsrtp_video_idr_generate_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Indication Message; Sends unsolicited video bitrate indications. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier  */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Mandatory */
  /*  Bitrate */
  uint32_t bit_rate;
  /**<   Bitrate in kbps. */
}imsrtp_video_bitrate_adapt_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Sends the video capability message during the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Unique RTP Session Identifier  */
  uint8_t session_id;
  /**<   Unique identifier of the RTP session. */

  /* Mandatory */
  /*  Common Video Capability Configuration */
  imsrtp_video_capability_v01 video_capability;

  /* Optional */
  /*  Preferred Bitrate */
  uint8_t preferred_bit_rate_valid;  /**< Must be set to true if preferred_bit_rate is being passed */
  uint16_t preferred_bit_rate;
  /**<   Preferred bitrate in kbps. If not set, the value defaults to the bit_rate 
      value in the optional Common Video Codec Configuration TLV for the 
      QMI_IMSRTP_SESSION_ CONFIGURE_REQ message. */

  /* Optional */
  /*  Preferred Frame Rate */
  uint8_t preferred_frame_rate_valid;  /**< Must be set to true if preferred_frame_rate is being passed */
  uint8_t preferred_frame_rate;
  /**<   Preferred frame rate. If not set, the value defaults to the frame_rate 
       value in the optional Common Video Codec Configuration TLV for the 
       QMI_IMSRTP_SESSION_ CONFIGURE_REQ message. */

  /* Optional */
  /*  Preferred Width */
  uint8_t preferred_width_valid;  /**< Must be set to true if preferred_width is being passed */
  uint32_t preferred_width;
  /**<   Preferred width. If not set, the value defaults to the width value in  
       the optional Common Video Codec Configuration TLV for the 
       QMI_IMSRTP_SESSION_ CONFIGURE_REQ message. */

  /* Optional */
  /*  Preferred Height */
  uint8_t preferred_height_valid;  /**< Must be set to true if preferred_height is being passed */
  uint32_t preferred_height;
  /**<   Preferred height. If not set, the value defaults to the height value in 
       the optional Common Video Codec Configuration TLV for the 
       QMI_IMSRTP_SESSION_ CONFIGURE_REQ message. */
}imsrtp_session_video_capability_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Sends the video capability message during the RTP session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_session_video_capability_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Configures NAT over RTP. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Mandatory */
  /*  Enable NAT Over RTP  */
  uint8_t enable;
  /**<   Indicates whether NAT over RTP is to be enabled. */

  /* Optional */
  /*  NAT Over RTP Interval */
  uint8_t interval_valid;  /**< Must be set to true if interval is being passed */
  uint16_t interval;
  /**<   NAT over RTP interval in seconds. */
}imsrtp_configure_nat_over_rtp_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Configures NAT over RTP. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_configure_nat_over_rtp_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Request Message; Configures NAT over RTCP. */
typedef struct {

  /* Mandatory */
  /*  RTP Session Identifier */
  uint8_t session_id;
  /**<   Unique identifier for the RTP session. */

  /* Mandatory */
  /*  Enable NAT Over RTCP  */
  uint8_t enable;
  /**<   Indicates whether NAT over RTCP is to be enabled. */

  /* Optional */
  /*  NAT Over RTP Interval */
  uint8_t interval_valid;  /**< Must be set to true if interval is being passed */
  uint16_t interval;
  /**<   NAT over RTP interval in seconds. */
}imsrtp_configure_nat_over_rtcp_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsrtp_qmi_messages
    @{
  */
/** Response Message; Configures NAT over RTCP. */
typedef struct {

  /* Mandatory */
  /*  Result Code   */
  qmi_response_type_v01 resp;
  /**<   Standard response type*/
}imsrtp_configure_nat_over_rtcp_resp_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup imsrtp_qmi_msg_ids
    @{
  */
#define QMI_IMSRTP_INITIALIZE_REQ_V01 0x0020
#define QMI_IMSRTP_INITIALIZE_RESP_V01 0x0020
#define QMI_IMSRTP_INITIALIZE_IND_V01 0x0020
#define QMI_IMSRTP_UNINITIALIZE_REQ_V01 0x0021
#define QMI_IMSRTP_UNINITIALIZE_RESP_V01 0x0021
#define QMI_IMSRTP_UNINITIALIZE_IND_V01 0x0021
#define QMI_IMSRTP_SESSION_INITIALIZE_REQ_V01 0x0022
#define QMI_IMSRTP_SESSION_INITIALIZE_RESP_V01 0x0022
#define QMI_IMSRTP_SESSION_INITIALIZE_IND_V01 0x0022
#define QMI_IMSRTP_SESSION_UNINITIALIZE_REQ_V01 0x0023
#define QMI_IMSRTP_SESSION_UNINITIALIZE_RESP_V01 0x0023
#define QMI_IMSRTP_SESSION_UNINITIALIZE_IND_V01 0x0023
#define QMI_IMSRTP_SESSION_CONFIGURE_REQ_V01 0x0024
#define QMI_IMSRTP_SESSION_CONFIGURE_RESP_V01 0x0024
#define QMI_IMSRTP_SESSION_CONFIGURE_IND_V01 0x0024
#define QMI_IMSRTP_SESSION_START_REQ_V01 0x0025
#define QMI_IMSRTP_SESSION_START_RESP_V01 0x0025
#define QMI_IMSRTP_SESSION_START_IND_V01 0x0025
#define QMI_IMSRTP_SESSION_STOP_REQ_V01 0x0026
#define QMI_IMSRTP_SESSION_STOP_RESP_V01 0x0026
#define QMI_IMSRTP_SESSION_STOP_IND_V01 0x0026
#define QMI_IMSRTP_SESSION_PAUSE_REQ_V01 0x0027
#define QMI_IMSRTP_SESSION_PAUSE_RESP_V01 0x0027
#define QMI_IMSRTP_SESSION_RESUME_REQ_V01 0x0028
#define QMI_IMSRTP_SESSION_RESUME_RESP_V01 0x0028
#define QMI_IMSRTP_CONFIGURE_RTCP_REPORTS_REQ_V01 0x0029
#define QMI_IMSRTP_CONFIGURE_RTCP_REPORTS_RESP_V01 0x0029
#define QMI_IMSRTP_CONFIGURE_RTCP_REPORTS_IND_V01 0x0029
#define QMI_IMSRTP_CONFIGURE_RTP_LINK_MONITOR_REQ_V01 0x002A
#define QMI_IMSRTP_CONFIGURE_RTP_LINK_MONITOR_RESP_V01 0x002A
#define QMI_IMSRTP_CONFIGURE_RTP_LINK_MONITORING_IND_V01 0x002A
#define QMI_IMSRTP_CONFIGURE_RTCP_LINK_MONITOR_REQ_V01 0x002B
#define QMI_IMSRTP_CONFIGURE_RTCP_LINK_MONITOR_RESP_V01 0x002B
#define QMI_IMSRTP_CONFIGURE_RTCP_LINK_MONITORING_IND_V01 0x002B
#define QMI_IMSRTP_SEND_DTMF_REQ_V01 0x002C
#define QMI_IMSRTP_SEND_DTMF_RESP_V01 0x002C
#define QMI_IMSRTP_START_CONTINUOUS_DTMF_REQ_V01 0x002D
#define QMI_IMSRTP_START_CONTINUOUS_DTMF_RESP_V01 0x002D
#define QMI_IMSRTP_STOP_CONTINUOUS_DTMF_REQ_V01 0x002E
#define QMI_IMSRTP_STOP_CONTINUOUS_DTMF_RESP_V01 0x002E
#define QMI_IMSRTP_FLUSH_AUDIO_JITTER_BUFFER_REQ_V01 0x002F
#define QMI_IMSRTP_FLUSH_AUDIO_JITTER_BUFFER_RESP_V01 0x002F
#define QMI_IMSRTP_REGISTER_RTP_SERVICES_REQ_V01 0x0030
#define QMI_IMSRTP_REGISTER_RTP_SERVICES_RESP_V01 0x0030
#define QMI_IMSRTP_GET_SESSION_STATUS_REQ_V01 0x0031
#define QMI_IMSRTP_GET_SESSION_STATUS_RESP_V01 0x0031
#define QMI_IMSRTP_GET_SESSION_STATUS_IND_V01 0x0031
#define QMI_IMSRTP_ERROR_IND_V01 0x0032
#define QMI_IMSRTP_RTCP_REPORTS_IND_V01 0x0033
#define QMI_IMS_RTP_LAST_AUDIO_PLAY_TIME_REQ_V01 0x0034
#define QMI_IMS_RTP_LAST_AUDIO_PLAY_TIME_RESP_V01 0x0034
#define QMI_IMS_RTP_AUDIO_REPORT_REQ_V01 0x0035
#define QMI_IMS_RTP_AUDIO_REPORT_RESP_V01 0x0035
#define QMI_IMS_RTP_VIDEO_REPORT_REQ_V01 0x0036
#define QMI_IMS_RTP_VIDEO_REPORT_RESP_V01 0x0036
#define QMI_IMS_RTP_VIDEO_BIT_RATE_ADAPT_REQ_V01 0x0037
#define QMI_IMS_RTP_VIDEO_BIT_RATE_ADAPT_RESP_V01 0x0037
#define QMI_IMS_RTP_H264_IDR_GENERATE_REQ_V01 0x0038
#define QMI_IMS_RTP_H264_IDR_GENERATE_RESP_V01 0x0038
#define QMI_IMS_RTP_AV_SYNC_FEED_REQ_V01 0x0039
#define QMI_IMS_RTP_AV_SYNC_FEED_RESP_V01 0x0039
#define QMI_IMS_RTP_VIDEO_ERROR_IND_V01 0x003A
#define QMI_IMS_RTP_GEN_H264_SPS_PPS_REQ_V01 0x003B
#define QMI_IMS_RTP_GEN_H264_SPS_PPS_RESP_V01 0x003B
#define QMI_IMS_RTP_GEN_MP4_VOL_HEADER_REQ_V01 0x003C
#define QMI_IMS_RTP_GEN_MP4_VOL_HEADER_RESP_V01 0x003C
#define QMI_IMSRTP_UNINITIALIZE_ALL_RTP_SESSION_REQ_V01 0x003D
#define QMI_IMSRTP_UNINITIALIZE_ALL_RTP_SESSION_RESP_V01 0x003D
#define QMI_IMSRTP_UNINITIALIZE_ALL_RTP_SESSION_IND_V01 0x003D
#define QMI_IMS_RTP_VIDEO_IDR_GENERATE_IND_V01 0x003E
#define QMI_IMS_RTP_VIDEO_BIT_RATE_ADAPT_IND_V01 0x003F
#define QMI_IMSRTP_SESSION_VIDEO_CAPABILITY_REQ_V01 0x0040
#define QMI_IMSRTP_SESSION_VIDEO_CAPABILITY_RESP_V01 0x0040
#define QMI_IMSRTP_CONFIGURE_NAT_OVER_RTP_REQ_V01 0x0041
#define QMI_IMSRTP_CONFIGURE_NAT_OVER_RTP_RESP_V01 0x0041
#define QMI_IMSRTP_CONFIGURE_NAT_OVER_RTCP_REQ_V01 0x0042
#define QMI_IMSRTP_CONFIGURE_NAT_OVER_RTCP_RESP_V01 0x0042
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor 
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro imsrtp_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type imsrtp_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );
 
/** This macro should be used to get the service object */ 
#define imsrtp_get_service_object_v01( ) \
          imsrtp_get_service_object_internal_v01( \
            IMSRTP_V01_IDL_MAJOR_VERS, IMSRTP_V01_IDL_MINOR_VERS, \
            IMSRTP_V01_IDL_TOOL_VERS )
/** 
    @} 
  */


#ifdef __cplusplus
}
#endif
#endif

