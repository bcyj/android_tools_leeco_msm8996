#ifndef CSD_SERVICE_01_H
#define CSD_SERVICE_01_H
/**
  @file core_sound_driver_v01.h

  @brief This is the public header file which defines the csd service Data structures.

  This header file defines the types and structures that were defined in
  csd. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.7
   It was generated on: Wed Jun 25 2014 (Spin 0)
   From IDL File: core_sound_driver_v01.idl */

/** @defgroup csd_qmi_consts Constant values defined in the IDL */
/** @defgroup csd_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup csd_qmi_enums Enumerated types used in QMI messages */
/** @defgroup csd_qmi_messages Structures sent as QMI messages */
/** @defgroup csd_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup csd_qmi_accessor Accessor for QMI service object */
/** @defgroup csd_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup csd_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define CSD_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define CSD_V01_IDL_MINOR_VERS 0x09
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define CSD_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define CSD_V01_MAX_MESSAGE_ID 0x00A7
/**
    @}
  */


/** @addtogroup csd_qmi_consts
    @{
  */

/**  Maximum length for the session name string. */
#define SESSION_NAME_MAX_LEN_V01 255

/**
   Device sample rate.


   Device bit per sample.


 Constant for device ID entries. One for Rx, and one for Tx. */
#define QMI_CSD_DEVICE_MAX_ENTRIES_V01 2

/**  Constant for extended devices ID entries. One for EC reference device */
#define QMI_CSD_EXTENDED_DEVICES_MAX_ENTRIES_V01 6

/**  Enables the AFE loopback.  */
#define QMI_CSD_DEV_AFE_LOOPBACK_ENABLE_V01 1

/**  Disables the AFE loopback.  */
#define QMI_CSD_DEV_AFE_LOOPBACK_DISABLE_V01 0

/**  Default setting for AFE loopback mode. All other values are reserved.
     Default: Enable.  */
#define QMI_CSD_DEV_AFE_LOOPBACK_DEFAULT_V01 1

/**  Disables the active noise cancellation feature.  */
#define QMI_CSD_DEV_CODEC_ANC_DISABLE_V01 0

/**  Enables the active noise cancellation feature.  */
#define QMI_CSD_DEV_CODEC_ANC_ENABLE_V01 1

/**  Maximum number of supported devices.  */
#define QMI_CSD_MAX_NUM_DEV_SUPPORTED_V01 256

/**
   DTMF high frequency types.


 1209 Hz.   */
#define QMI_CSD_DTMF_HIGH_FREQ_1209_V01 1209

/**  1336 Hz.   */
#define QMI_CSD_DTMF_HIGH_FREQ_1336_V01 1336

/**  1477 Hz.   */
#define QMI_CSD_DTMF_HIGH_FREQ_1477_V01 1477

/**  1633 Hz.   */
#define QMI_CSD_DTMF_HIGH_FREQ_1633_V01 1633

/**
   DTMF low frequency types.


 697 Hz.   */
#define QMI_CSD_DTMF_LOW_FREQ_697_V01 697

/**  770 Hz.   */
#define QMI_CSD_DTMF_LOW_FREQ_770_V01 770

/**  852 Hz.   */
#define QMI_CSD_DTMF_LOW_FREQ_852_V01 852

/**  941 Hz.   */
#define QMI_CSD_DTMF_LOW_FREQ_941_V01 941

/**
   DTMF gain value in Q13. */
#define QMI_CSD_DTMF_DEFAULT_GAIN_V01 0x4000

/**  GUID header for the RT proxy port configuration type.
  This header is used in association with QMI_CSD_IOCTL_DEV_CMD_CONFIGURE to
  indicate an RT proxy port device type configuration. */
#define QMI_CSD_DEV_CFG_PROXY_PORT_HEADER_V01 0x00011C0B

/**  DTMF detection enable flag.  */
#define QMI_CSD_VC_TX_DTMF_DETECTION_ENABLE_V01 1

/**  DTMF detection disable flag.  */
#define QMI_CSD_VC_TX_DTMF_DETECTION_DISABLE_V01 0
#define MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01 2048

/**  Indicates the Ready state.  */
#define QMI_CSD_VOC_STATEID_READY_V01 0x00011166

/**  Indicates the Not Ready state.  */
#define QMI_CSD_VOC_STATEID_NOT_READY_V01 0x00011167

/**  Set default device topologies obtained from ACDB.
     Client use this feature id value to set default topology
     that obtained from the ACDB for the voice call */
#define QMI_CSD_VC_FID_TOPOLOGY_DEFAULT_V01 0

/**  Set None topology.
     Client use this feature id value to set none topology
     for the voice call */
#define QMI_CSD_VC_FID_TOPOLOGY_NONE_V01 1

/**
  Voice stream DTMF definitions.

 DTMF generation direction.  */
#define QMI_CSD_VS_DTMF_GENERATION_DIRECTION_TX_V01 0

/**  DTMF generation mix with speech disable flag.  */
#define QMI_CSD_VS_DTMF_GENERATION_MIX_WITH_SPEECH_DISABLE_V01 0

/**  DTMF generation mix with speech enable flag.  */
#define QMI_CSD_VS_DTMF_GENERATION_MIX_WITH_SPEECH_ENABLE_V01 1

/**  DTMF detection enable flag.  */
#define QMI_CSD_VS_RX_DTMF_DETECTION_ENABLE_V01 1

/**  DTMF detection disable flag.  */
#define QMI_CSD_VS_RX_DTMF_DETECTION_DISABLE_V01 0

/**  CSD audio PP multichannel type.
                                          */
#define QMI_CSD_AUD_PP_MULTI_CH_NUM_V01 8

/**  CSD audio PP equalizer filter type.
                                          */
#define QMI_CSD_AUD_PP_EQ_SUB_BAND_MAX_NUM_V01 12

/**  CSD audio Windows Media Audio 10 Pro multichannel bitfield
    definitions.


 Front left channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_FL_V01 0x00000001

/**  Front right channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_FR_V01 0x00000002

/**  Front center channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_FC_V01 0x00000004

/**  Low frequency effects channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_LFE_V01 0x00000008

/**  Back left channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_LB_V01 0x00000010

/**  Back right channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_RB_V01 0x00000020

/**  Front left center channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_FLC_V01 0x00000040

/**  Front right center channel bitfield.   */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_FRC_V01 0x00000080

/**  Back center channel bitfield.      */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_BC_V01 0x00000100

/**  Surround left channel bitfield.    */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_SL_V01 0x00000200

/**  Surround right channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_SR_V01 0x00000400

/**  Top center channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_TC_V01 0x00000800

/**  Top front center channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_TFC_V01 0x00001000

/**  Top front left channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_TFL_V01 0x00002000

/**  Top front right channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_TFR_V01 0x00004000

/**  Top back left channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_TBL_V01 0x00008000

/**  Top back center channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_TBC_V01 0x00010000

/**  Top back right channel bitfield.  */
#define QMI_CSD_AS_WMA_MULTI_CHANNEL_TBR_V01 0x00020000

/**  Most significant bit reserved channel bitfield.  */
#define QMI_CSD_AS_WMA_CHANNEL_MSB_RESERVED_V01 0x80000000

/**  Windows Media Audio 10 Pro common multichannel setting definitions
    for CSD audio.


 No downmixing channel configuration selected.   */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_NULL_V01 -1

/**  Front center speaker is selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_V01 0x4

/**  Front left and front right speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_V01 0x3

/**  Front left, front right and front center speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_V01 0x7

/**  Front left, front right, back left and back right speakers are
     selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_BL_BR_V01 0x33

/**  Front left, front right, front center and back center speakers
     are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_BC_V01 0x107

/**  Front left, front right, front center, side left and side right
     speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_SL_SR_V01 0x0607

/**  Front left, front right, front center, back left and back right
     speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_BL_BR_V01 0x0037

/**  Front left, front right, front center, low frequency, side left
     and side right speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_LFE_SL_SR_V01 0x60F

/**  Front left, front right, front center, low frequency, back left
     and back right speakers are selected.      */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_LFE_BL_BR_V01 0x3F

/**  Front left, front right, front center, back center, side left
     and side right speakers are selected.      */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_BC_SL_SR_V01 0x707

/**  Front left, front right, front center, back left, back right
     and back center speakers are selected.     */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_BL_BR_BC_V01 0x137

/**  Front left, front right, low frequency, back center, side left
     and side right speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_LFE_BC_SL_SR_V01 0x70F

/**  Front left, front right, front center, low frequency, back left,
     back right and back center speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_LFE_BL_BR_BC_V01 0x13F

/**  Front left, front right, front center, back left, back right,
     side left and side right speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_BL_BR_SL_SR_V01 0x637

/**  Front left, front right, front center, low frequency, back left,
     back right, side left and side right speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_BL_BR_FLC_FRC_V01 0xF7

/**  Front left, front right, front center, low frequency, back left,
     back right, side left and side right speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_LFE_BL_BR_SL_SR_V01 0x63F

/**  Front left, front right, front center, low frequency, back left,
     back right, front left of center and front right of center
     speakers are selected.  */
#define QMI_CSD_AS_WMA_CHANNEL_CONFIG_FL_FR_FC_LFE_BL_BR_FLC_FRC_V01 0xFF
#define QMI_CSD_SPA_DATA_BUF_SIZE_V01 4096
#define QMI_CSD_MAX_NUM_AUDIO_STREAM_FOR_ONE_AC_V01 8
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_STATUS_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_EOK_V01 = 0x0, /**<  Success. The operation completed, and there were no errors.  */
  QMI_CSD_EFAILED_V01 = 0x00012313, /**<  General failure.  */
  QMI_CSD_EBADPARAM_V01 = 0x00012314, /**<  Invalid operation parameters.  */
  QMI_CSD_EUNSUPPORTED_V01 = 0x00012315, /**<  Unsupported routine or operation.  */
  QMI_CSD_EVERSION_V01 = 0x00012316, /**<  Unsupported version.  */
  QMI_CSD_EUNEXPECTED_V01 = 0x00012317, /**<  Unexpected problem was encountered.  */
  QMI_CSD_EPANIC_V01 = 0x00012318, /**<  Unhandled problem occurred.  */
  QMI_CSD_ENORESOURCE_V01 = 0x00012319, /**<  Unable to allocate resources.  */
  QMI_CSD_EHANDLE_V01 = 0x0001231A, /**<  Invalid handle.  */
  QMI_CSD_EALREADY_V01 = 0x0001231B, /**<  Operation is already processed.  */
  QMI_CSD_ENOTREADY_V01 = 0x0001231C, /**<  Operation is not ready to be processed.  */
  QMI_CSD_EPENDING_V01 = 0x0001231D, /**<  Operation is pending completion.  */
  QMI_CSD_EBUSY_V01 = 0x0001231E, /**<  Operation cannot be accepted or processed.  */
  QMI_CSD_EABORTED_V01 = 0x0001231F, /**<  Operation aborted due to an error.  */
  QMI_CSD_EPREEMPTED_V01 = 0x00012320, /**<  Operation was preempted by a higher priority.  */
  QMI_CSD_ECONTINUE_V01 = 0x00012321, /**<  Operation requires intervention to complete.  */
  QMI_CSD_EIMMEDIATE_V01 = 0x00012322, /**<  Operation requires immediate intervention to complete.  */
  QMI_CSD_ENOTIMPL_V01 = 0x00012323, /**<  Operation is not implemented.  */
  QMI_CSD_ENEEDMORE_V01 = 0x00012324, /**<  Operation requires more data or resources.  */
  QMI_CSD_ELPC_V01 = 0x00012325, /**<  Operation is a local procedure call.  */
  QMI_CSD_ETIMEOUT_V01 = 0x00012326, /**<  Operation timed out.  */
  QMI_CSD_ENOTFOUND_V01 = 0x00012327, /**<  Not found.  */
  QMI_CSD_EBADSTATE_V01 = 0x00012328, /**<  Operation cannot proceed due to an improper state.  */
  QMI_CSD_EQADSP_V01 = 0x00012329, /**<  Qualcomm aDSP return error status.   */
  QMI_CSD_STATUS_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_status_v01;
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}qmi_csd_query_driver_version_req_msg_v01;

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Queries the CSD version number. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Version Number */
  uint8_t csd_version_valid;  /**< Must be set to true if csd_version is being passed */
  uint32_t csd_version;
  /**<   CSD version format in uint32: 0xMMNNxxRR.\n
         Where:\n
         MM = Main version number.\n
         NN = Minor version number. \n
         RR = Revision number. \n
         For example, the returned value "uint32 csd_version" is:\n
         Main_version = (csd_version & 0xFF000000)>>24\n
         Minor_version = (csd_version & 0xFF0000)>>16\n
         Revision = (csd_version & 0xFF)
        */
}qmi_csd_query_driver_version_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}qmi_csd_init_req_msg_v01;

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Initializes the CSD before the CSD can provide any functionality. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   Globally Unique Identifier (GUID) for the CSD status. */
}qmi_csd_init_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}qmi_csd_deinit_req_msg_v01;

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Detaches the client from the CSD. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status  */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_deinit_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VS_DIRECTION_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VS_DIRECTION_TX_ONLY_V01 = 0, /**<  Tx only  */
  QMI_CSD_VS_DIRECTION_RX_ONLY_V01 = 1, /**<  Rx only  */
  QMI_CSD_VS_DIRECTION_TX_AND_RX_V01 = 2, /**<  Tx and Rx  */
  QMI_CSD_VS_DIRECTION_TX_AND_RX_LOOPBACK_V01 = 3,
  QMI_CSD_VS_DIRECTION_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_vs_direction_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_NETWORK_ID_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_NETWORK_ID_DEFAULT_V01 = 0x00010037, /**<  Default network.  */
  QMI_CSD_NETWORK_ID_CDMA_NB_V01 = 0x00010021, /**<  CDMA narrowband network.  */
  QMI_CSD_NETWORK_ID_CDMA_WB_V01 = 0x00010022, /**<  CDMA wideband network.  */
  QMI_CSD_NETWORK_ID_CDMA_WV_V01 = 0x00011100, /**<  CDMA WideVoice network.  */
  QMI_CSD_NETWORK_ID_GSM_NB_V01 = 0x00010023, /**<  GSM narrowband network.  */
  QMI_CSD_NETWORK_ID_GSM_WB_V01 = 0x00010024, /**<  GSM wideband network.  */
  QMI_CSD_NETWORK_ID_GSM_WV_V01 = 0x00011101, /**<  GSM WideVoice network.  */
  QMI_CSD_NETWORK_ID_WCDMA_NB_V01 = 0x00010025, /**<  WCDMA narrowband network.  */
  QMI_CSD_NETWORK_ID_WCDMA_WB_V01 = 0x00010026, /**<  WCDMA wideband network.  */
  QMI_CSD_NETWORK_ID_WCDMA_WV_V01 = 0x00011102, /**<  WCDMA WideVoice network.  */
  QMI_CSD_NETWORK_ID_VOIP_NB_V01 = 0x00011240, /**<  VOIP narrowband network.  */
  QMI_CSD_NETWORK_ID_VOIP_WB_V01 = 0x00011241, /**<  VOIP wideband network.  */
  QMI_CSD_NETWORK_ID_VOIP_WV_V01 = 0x00011242, /**<  VOIP WideVoice network.  */
  QMI_CSD_NETWORK_ID_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_network_id_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VOC_TYPE_ID_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_MEDIA_ID_NONE_V01 = 0x00010FC0, /**<  No media type.  */
  QMI_CSD_MEDIA_ID_13K_MODEM_V01 = 0x00010FC1, /**<  CDMA variable 13K vocoder modem format.  */
  QMI_CSD_MEDIA_ID_EVRC_MODEM_V01 = 0x00010FC2, /**<  CDMA enhanced variable rate vocoder modem format.  */
  QMI_CSD_MEDIA_ID_4GV_NB_MODEM_V01 = 0x00010FC3, /**<  CDMA fourth-generation narrowband vocoder modem format.  */
  QMI_CSD_MEDIA_ID_4GV_WB_MODEM_V01 = 0x00010FC4, /**<  CDMA fourth-generation wideband vocoder modem format.  */
  QMI_CSD_MEDIA_ID_4GV_NW_MODEM_V01 = 0x00010FC5, /**<  CDMA fourth-generation narrow-wide vocoder modem format.  */
  QMI_CSD_MEDIA_ID_AMR_NB_MODEM_V01 = 0x00010FC6, /**<  UMTS adaptive multirate narrowband vocoder modem format.  */
  QMI_CSD_MEDIA_ID_AMR_WB_MODEM_V01 = 0x00010FC7, /**<  UMTS adaptive multirate wideband vocoder modem format.  */
  QMI_CSD_MEDIA_ID_EFR_MODEM_V01 = 0x00010FC8, /**<  GSM enhanced full-rate vocoder modem format.  */
  QMI_CSD_MEDIA_ID_FR_MODEM_V01 = 0x00010FC9, /**<  GSM full-rate vocoder modem format.  */
  QMI_CSD_MEDIA_ID_HR_MODEM_V01 = 0x00010FCA, /**<  GSM half-rate vocoder modem format.  */
  QMI_CSD_MEDIA_ID_PCM_NB_V01 = 0x00010FCB, /**<  Linear pulse code modulation narrowband (16-bit, little-endian).  */
  QMI_CSD_MEDIA_ID_PCM_WB_V01 = 0x00010FCC, /**<  Linear pulse code modulation wideband (16-bit, little-endian).  */
  QMI_CSD_MEDIA_ID_G711_ALAW_V01 = 0x00010FCD, /**<  G.711 A-law; contains two 10-millisecond vocoder frames.  */
  QMI_CSD_MEDIA_ID_G711_MULAW_V01 = 0x00010FCE, /**<  G.711 Mu-law; contains two 10-millisecond vocoder frames.  */
  QMI_CSD_MEDIA_ID_G729_V01 = 0x00010FD0, /**<  G.729AB; contains two 10-millisecond vocoder frames.   */
  QMI_CSD_MEDIA_ID_G722_V01 = 0x00010FD1, /**<  G.722; contains one 20-millisecond vocoder frame.  */
  QMI_CSD_VOC_TYPE_ID_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_voc_type_id_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Opens a passive control Voice Stream (VS) and returns the
           corresponding VS handle. */
typedef struct {

  /* Mandatory */
  /*  Session Name */
  char session_name[SESSION_NAME_MAX_LEN_V01 + 1];
  /**<   Session name.*/
}qmi_csd_open_passive_control_voice_stream_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Opens a passive control Voice Stream (VS) and returns the
           corresponding VS handle. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Open Status  */
  uint8_t open_status_valid;  /**< Must be set to true if open_status is being passed */
  qmi_csd_status_v01 open_status;
  /**<   Open status.*/

  /* Optional */
  /*  Passive Control Voice Stream Handle */
  uint8_t qmi_csd_vs_passive_control_handle_valid;  /**< Must be set to true if qmi_csd_vs_passive_control_handle is being passed */
  uint32_t qmi_csd_vs_passive_control_handle;
  /**<   Passive control voice stream handle. */
}qmi_csd_open_passive_control_voice_stream_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
typedef struct {

  char session_name[SESSION_NAME_MAX_LEN_V01 + 1];
  /**<   Session name.*/

  qmi_csd_vs_direction_v01 direction;
  /**<   Direction in which the stream is flowing. Supported values:\n
         - 0 -- Tx only\n
         - 1 -- Rx only\n
         - 2 -- Tx and Rx
    */

  qmi_csd_voc_type_id_v01 enc_media_type;
  /**<   Tx vocoder type. See Appendix \ref{app:CSDVoiceMediaIDs} for information on
         media IDs.*/

  qmi_csd_voc_type_id_v01 dec_media_type;
  /**<   Rx vocoder type. See Appendix \ref{app:CSDVoiceMediaIDs} for information on
         media IDs. */

  qmi_csd_network_id_v01 network_id;
  /**<   Network ID. See Appendix \ref{app:CSDVoiceNetworkIDs} for information on
         network IDs. Default: 0. */
}qmi_csd_vs_open_full_control_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Opens a full control VS and returns the corresponding VS handle. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Open Structure */
  qmi_csd_vs_open_full_control_type_v01 qmi_csd_vs_open_payload;
}qmi_csd_open_full_control_voice_stream_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Opens a full control VS and returns the corresponding VS handle. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Open Status  */
  uint8_t open_status_valid;  /**< Must be set to true if open_status is being passed */
  qmi_csd_status_v01 open_status;
  /**<   Open status.           */

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t qmi_csd_vs_handle_valid;  /**< Must be set to true if qmi_csd_vs_handle is being passed */
  uint32_t qmi_csd_vs_handle;
  /**<   Unique handle for the voice stream.          */
}qmi_csd_open_full_control_voice_stream_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VC_DIRECTION_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VC_DIRECTION_TX_ONLY_V01 = 0,
  QMI_CSD_VC_DIRECTION_RX_ONLY_V01 = 1,
  QMI_CSD_VC_DIRECTION_TX_AND_RX_V01 = 2,
  QMI_CSD_VC_DIRECTION_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_vc_direction_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
typedef struct {

  char session_name[SESSION_NAME_MAX_LEN_V01 + 1];
  /**<   Session name.*/

  qmi_csd_vc_direction_v01 direction;
  /**<   Direction in which the stream is flowing. Supported values:\n
         - 0 -- Tx only\n
         - 1 -- Rx only\n
         - 2 -- Tx and Rx
    */

  qmi_csd_network_id_v01 network_id;
  /**<   Network ID. See Appendix \ref{app:CSDVoiceNetworkIDs} for information on
         network IDs. Default: 0.
    */
}qmi_csd_vc_open_full_control_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Opens a Voice Context (VC) and returns the corresponding VC handle. */
typedef struct {

  /* Mandatory */
  /*  Voice Context Open Structure */
  qmi_csd_vc_open_full_control_type_v01 qmi_csd_vc_open_payload;
}qmi_csd_open_voice_context_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Opens a Voice Context (VC) and returns the corresponding VC handle. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Open Status  */
  uint8_t open_status_valid;  /**< Must be set to true if open_status is being passed */
  qmi_csd_status_v01 open_status;
  /**<   Open status.           */

  /* Optional */
  /*  Voice Context Handle */
  uint8_t qmi_csd_vc_handle_valid;  /**< Must be set to true if qmi_csd_vc_handle is being passed */
  uint32_t qmi_csd_vc_handle;
  /**<   Unique handle for the voice context.          */
}qmi_csd_open_voice_context_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
typedef struct {

  char session_name[SESSION_NAME_MAX_LEN_V01 + 1];
  /**<   Session name.*/
}qmi_csd_vm_open_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Opens a Voice Manager (VM) and returns the corresponding VM handle. */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Open Structure */
  qmi_csd_vm_open_type_v01 qmi_csd_vm_open_payload;
}qmi_csd_open_voice_manager_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Opens a Voice Manager (VM) and returns the corresponding VM handle. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Open Status */
  uint8_t open_status_valid;  /**< Must be set to true if open_status is being passed */
  qmi_csd_status_v01 open_status;
  /**<   Open status.           */

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t qmi_csd_vm_handle_valid;  /**< Must be set to true if qmi_csd_vm_handle is being passed */
  uint32_t qmi_csd_vm_handle;
  /**<   Unique handle for the voice manager.        */
}qmi_csd_open_voice_manager_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}qmi_csd_open_device_control_req_msg_v01;

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Instructs the CSD to return the device control handle. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Open Status */
  uint8_t open_status_valid;  /**< Must be set to true if open_status is being passed */
  qmi_csd_status_v01 open_status;
  /**<   Open status.*/

  /* Optional */
  /*  Device Handle */
  uint8_t qmi_csd_device_handle_valid;  /**< Must be set to true if qmi_csd_device_handle is being passed */
  uint32_t qmi_csd_device_handle;
  /**<   Device handle.*/
}qmi_csd_open_device_control_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Closes the CSD stream or session using the specified handle.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/
}qmi_csd_close_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Closes the CSD stream or session using the specified handle.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Device Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Device handle.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_close_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_DEV_SR_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_DEV_SR_UNKNOWN_V01 = 0, /**<  Unknown sample rate.    */
  QMI_CSD_DEV_SR_8000_V01 = 8000, /**<  8-kHz sample rate.       */
  QMI_CSD_DEV_SR_16000_V01 = 16000, /**<  16-kHz sample rate.      */
  QMI_CSD_DEV_SR_48000_V01 = 48000, /**<  48-kHz sample rate.      */
  QMI_CSD_DEV_SR_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_dev_sr_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_DEV_BPS_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_DEV_BPS_UNKNOWN_V01 = 0, /**<  Unknown bits per sample.  */
  QMI_CSD_DEV_BPS_16_V01 = 16, /**<  16 bits per sample.    */
  QMI_CSD_DEV_BPS_24_V01 = 24, /**<  24 bits per sample.    */
  QMI_CSD_DEV_BPS_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_dev_bps_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**   one for in call recording device etc.>
 Device attributes.
 */
typedef struct {

  qmi_csd_dev_sr_v01 sample_rate;
  /**<   Sample rate. Supported values:\n
         - QMI_CSD_DEV_SR_8000 (8000) -- 8000 samples per second \n
         - QMI_CSD_DEV_SR_16000 (16000) -- 16000 samples per second \n
         - QMI_CSD_DEV_SR_48000 (48000) -- 48000 samples per second
    */

  qmi_csd_dev_bps_v01 bits_per_sample;
  /**<   Number of bits per sample. Supported values:\n
         - QMI_CSD_DEV_BPS_UNKNOWN (0) -- Unknown bits per sample \n
         - QMI_CSD_DEV_BPS_16 (16) -- 16 bits per sample \n
         - QMI_CSD_DEV_BPS_24 (24) -- 24 bits per sample */
}qmi_csd_dev_attrib_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Device information.
 */
typedef struct {

  uint32_t dev_id;
  /**<   Device ID. */

  qmi_csd_dev_attrib_v01 dev_attrib;
  /**<   Device attributes. */
}qmi_csd_dev_entry_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Enables the CSD device.
 */
typedef struct {

  uint32_t devs_len;  /**< Must be set to # of elements in devs */
  qmi_csd_dev_entry_v01 devs[QMI_CSD_DEVICE_MAX_ENTRIES_V01];
  /**<   Array of the device information for the devices to be enabled.
         Variable length array is converted to the number of actual devices to
         be enabled, followed by the actual device entry array. */
}qmi_csd_dev_enable_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables an audio device.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.   */

  /* Mandatory */
  /*  Device Enable */
  qmi_csd_dev_enable_v01 qmi_csd_dev_enable_cmd_payload;

  /* Optional */
  /*  Extended Devices */
  uint8_t extn_devices_valid;  /**< Must be set to true if extn_devices is being passed */
  uint32_t extn_devices_len;  /**< Must be set to # of elements in extn_devices */
  qmi_csd_dev_entry_v01 extn_devices[QMI_CSD_EXTENDED_DEVICES_MAX_ENTRIES_V01];
}qmi_csd_ioctl_dev_cmd_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables an audio device.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_dev_cmd_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**
  Structure for the audio front end restart.

 */
typedef struct {

  uint32_t tx_dev_id;
  /**<   (Tx) device ID. */

  uint32_t rx_dev_id;
  /**<   (Rx) device ID.  */

  uint32_t sample_rate;
  /**<   Sample rate to switch to */
}qmi_csd_dev_restart_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Dynamiclly restarts device at new sampling rate without bring down clocks. */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/

  /* Mandatory */
  /*  Restart Device */
  qmi_csd_dev_restart_v01 qmi_csd_dev_restart_payload;
}qmi_csd_ioctl_dev_cmd_restart_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Dynamiclly restarts device at new sampling rate without bring down clocks. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status. */
}qmi_csd_ioctl_dev_cmd_restart_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**
  Structure for disabling the device.

 */
typedef struct {

  uint32_t dev_ids_len;  /**< Must be set to # of elements in dev_ids */
  uint32_t dev_ids[QMI_CSD_DEVICE_MAX_ENTRIES_V01];
  /**<   Array of the device IDs to be disabled.
         The variable length array is converted to: Number of actual
         devices to be disabled, followed by the actual device ID array.
         Supported values: 0, 1, 2. */
}qmi_csd_dev_disable_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Disables one or two audio devices.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/

  /* Mandatory */
  /*  Device Disable */
  qmi_csd_dev_disable_v01 qmi_csd_dev_disable_cmd_payload;

  /* Optional */
  /*  Extended Devices */
  uint8_t extn_devices_valid;  /**< Must be set to true if extn_devices is being passed */
  uint32_t extn_devices_len;  /**< Must be set to # of elements in extn_devices */
  uint32_t extn_devices[QMI_CSD_EXTENDED_DEVICES_MAX_ENTRIES_V01];
  /**<   Extended devices. */
}qmi_csd_ioctl_dev_cmd_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Disables one or two audio devices.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_dev_cmd_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**
  Structure for the audio front end loopback.

 */
typedef struct {

  uint32_t tx_dev_id;
  /**<   Recording (Tx) device ID. */

  uint32_t rx_dev_id;
  /**<   Playback (Rx) device ID.  */

  uint8_t enable;
  /**<   Indicates whether the AFE is enabled: \n
                             - 1 -- Enable (Default)\n
                             - 0 -- Disable.*/

  uint16_t afe_mode;
  /**<   AFE loopback mode. Default: 1;
                             all other values are reserved.  */
}qmi_csd_dev_afe_loopback_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Controls the Audio Front End (AFE) loopback on an Rx and a Tx
           device.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/

  /* Mandatory */
  /*  Audio Front End Loopback */
  qmi_csd_dev_afe_loopback_v01 qmi_csd_dev_afe_loopback_cmd_payload;
}qmi_csd_ioctl_dev_cmd_afe_loopback_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Controls the Audio Front End (AFE) loopback on an Rx and a Tx
           device.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_dev_cmd_afe_loopback_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**   Structure to connect two devices together.
 */
typedef struct {

  uint8_t connect_flag;
  /**<   Indicates whether the devices are to be attached
                               (connected) or detached (disconnected):\n
                               - TRUE -- Connected (default) \n
                               - FALSE  -- Disconnected */

  uint32_t source_dev_id;
  /**<   Device ID for the source device. */

  uint32_t sink_dev_id;
  /**<   Device ID for the sink device. */
}qmi_csd_dev_connect_device_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Connects two devices together, one as a source and the other as a sink.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/

  /* Mandatory */
  /*  Device Pair Connection */
  qmi_csd_dev_connect_device_v01 qmi_csd_dev_connect_device_cmd_payload;
}qmi_csd_ioctl_dev_cmd_connect_device_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Connects two devices together, one as a source and the other as a sink.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_dev_cmd_connect_device_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**
  Controls the active noise cancellation feature.

 */
typedef struct {

  uint32_t rx_dev_id;
  /**<   Playback (Rx) device ID. */

  uint8_t enable;
  /**<   Indicates whether the ANC
                             feature is enabled:\n
                             - 1 -- Enable\n
                             - 0 -- Disable */
}qmi_csd_dev_anc_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Controls the Active Noise Cancellation (ANC) on a Rx device.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/

  /* Mandatory */
  /*  Active Noise Cancellation */
  qmi_csd_dev_anc_v01 qmi_csd_dev_anc_cmd_payload;
}qmi_csd_ioctl_dev_cmd_anc_control_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Controls the Active Noise Cancellation (ANC) on a Rx device.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_dev_cmd_anc_control_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_DEV_AFE_AANC_CTRL_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_DEV_AFE_AANC_DISABLE_V01 = 0, /**<  Disables the AFE AANC.  */
  QMI_CSD_DEV_AFE_AANC_ENABLE_V01 = 1, /**<  Enables the AFE AANC.  */
  QMI_CSD_DEV_AFE_AANC_ACDB_CTRL_V01 = 2, /**<  Use the value stored in the ACDB.  */
  QMI_CSD_DEV_AFE_AANC_CTRL_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_dev_afe_aanc_ctrl_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Controls the Adaptive Active Noise Cancellation (AANC) on a device.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle. */

  /* Mandatory */
  /*  Transmit Device ID */
  uint32_t tx_dev_id;
  /**<   Transmit device ID. */

  /* Mandatory */
  /*  Receive Device ID */
  uint32_t rx_dev_id;
  /**<   Receive device ID. */

  /* Mandatory */
  /*  Reference Device ID */
  uint32_t ref_dev_id;
  /**<   Reference device ID. */

  /* Mandatory */
  /*  Adaptive Active Noise Cancellation Control  */
  qmi_csd_dev_afe_aanc_ctrl_v01 aanc_ctrl;
  /**<   Adaptive active noise cancellation control. Values:\n
      - QMI_CSD_DEV_AFE_AANC_DISABLE (0) --  Disables the AFE AANC.
      - QMI_CSD_DEV_AFE_AANC_ENABLE (1) --  Enables the AFE AANC.
      - QMI_CSD_DEV_AFE_AANC_ACDB_CTRL (2) --  Use the value stored in the ACDB.
 */
}qmi_csd_ioctl_dev_cmd_aanc_control_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Controls the Adaptive Active Noise Cancellation (AANC) on a device.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status. */
}qmi_csd_ioctl_dev_cmd_aanc_control_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_CODEC_COMP_OPTIONS_ID_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_CODEC_COMP_DISABLE_V01 = 0, /**<  Disables companding.   */
  QMI_CSD_CODEC_COMP_ENABLE_STATIC_V01 = 1, /**<  Enables static companding.   */
  QMI_CSD_CODEC_COMP_ENABLE_DYNAMIC_V01 = 2, /**<  Enables dynamic companding.   */
  QMI_CSD_CODEC_COMP_OPTIONS_ID_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_codec_comp_options_id_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**
  Structures for the companding.

 */
typedef struct {

  uint32_t rx_dev_id;
  /**<   Playback (Rx) device ID. */

  qmi_csd_codec_comp_options_id_v01 qmi_csd_comp_options;
  /**<   Indicates the companding option. Supported values:\n
         - QMI_CSD_CODEC_COMP_DISABLE (0) -- Disables companding \n
         - QMI_CSD_CODEC_COMP_ENABLE_ STATIC (1) -- Enables static companding \n
         - QMI_CSD_CODEC_COMP_ENABLE_ DYNAMIC (2) -- Enables dynamic companding
    */
}qmi_csd_dev_companding_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Controls companding on a Rx device.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/

  /* Mandatory */
  /*  Companding Control */
  qmi_csd_dev_companding_v01 qmi_csd_dev_companding_cmd_payload;
}qmi_csd_ioctl_dev_cmd_companding_control_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Controls companding on a Rx device.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_dev_cmd_companding_control_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the maximum number of supported devices in the CSD driver.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/
}qmi_csd_ioctl_dev_cmd_get_max_device_nums_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the maximum number of supported devices in the CSD driver.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Maximum Number of Devices */
  uint8_t max_num_devices_valid;  /**< Must be set to true if max_num_devices is being passed */
  uint32_t max_num_devices;
  /**<   Maximum number of supported devices in the CSD driver. */
}qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_v01;  /* Message */
/**
    @}
  */

/**
   Device sample rate mask.
  */
typedef uint32_t qmi_csd_dev_sr_mask_v01;
#define QMI_CSD_DEV_SR_MASK_8000_V01 ((qmi_csd_dev_sr_mask_v01)0x00000001) /**<  8-kHz frequency.   */
#define QMI_CSD_DEV_SR_MASK_11025_V01 ((qmi_csd_dev_sr_mask_v01)0x00000002) /**<  11.025-kHz frequency.  */
#define QMI_CSD_DEV_SR_MASK_12000_V01 ((qmi_csd_dev_sr_mask_v01)0x00000004) /**<  12-kHz frequency.  */
#define QMI_CSD_DEV_SR_MASK_16000_V01 ((qmi_csd_dev_sr_mask_v01)0x00000008) /**<  16-kHz frequency.  */
#define QMI_CSD_DEV_SR_MASK_22050_V01 ((qmi_csd_dev_sr_mask_v01)0x00000010) /**<  22.05-kHz frequency.  */
#define QMI_CSD_DEV_SR_MASK_24000_V01 ((qmi_csd_dev_sr_mask_v01)0x00000020) /**<  24-kHz frequency.  */
#define QMI_CSD_DEV_SR_MASK_32000_V01 ((qmi_csd_dev_sr_mask_v01)0x00000040) /**<  32-kHz frequency.  */
#define QMI_CSD_DEV_SR_MASK_44100_V01 ((qmi_csd_dev_sr_mask_v01)0x00000080) /**<  44.1-kHz frequency.  */
#define QMI_CSD_DEV_SR_MASK_48000_V01 ((qmi_csd_dev_sr_mask_v01)0x00000100) /**<  48-kHz frequency.  */
/**
   Device sample width mask.
  */
typedef uint32_t qmi_csd_dev_bps_mask_v01;
#define QMI_CSD_DEV_BPS_MASK_16_V01 ((qmi_csd_dev_bps_mask_v01)0x00000001) /**<  16 bits per sample.  */
#define QMI_CSD_DEV_BPS_MASK_24_V01 ((qmi_csd_dev_bps_mask_v01)0x00000002) /**<  24 bits per sample.  */
#define QMI_CSD_DEV_BPS_MASK_32_V01 ((qmi_csd_dev_bps_mask_v01)0x00000004) /**<  32 bits per sample.  */
/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Capabilities record for a single device.
 */
typedef struct {

  uint32_t dev_id;
  /**<   Device ID. */

  qmi_csd_dev_sr_mask_v01 sr_bitmask;
  /**<   Bitmask of sample rates supported. */

  qmi_csd_dev_bps_mask_v01 bps_bitmask;
  /**<   Bitmask of bits per sample supported. */
}qmi_csd_dev_caps_entry_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**
  Full list of device capabilities.

 */
typedef struct {

  uint32_t num_devs;
  /**<   Number of devices to query. */

  uint32_t qmi_csd_dev_caps_list_len;  /**< Must be set to # of elements in qmi_csd_dev_caps_list */
  qmi_csd_dev_caps_entry_v01 qmi_csd_dev_caps_list[QMI_CSD_MAX_NUM_DEV_SUPPORTED_V01];
  /**<   Array of the returned full list of queried device capabilities. */
}qmi_csd_dev_caps_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the full list of device capabilities.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/
}qmi_csd_ioctl_dev_cmd_get_dev_caps_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the full list of device capabilities.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Device Capabilities */
  uint8_t qmi_csd_dev_caps_payload_valid;  /**< Must be set to true if qmi_csd_dev_caps_payload is being passed */
  qmi_csd_dev_caps_v01 qmi_csd_dev_caps_payload;
}qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**
    Controls DTMF generation.

 */
typedef struct {

  int64_t dtmf_duration_in_ms;
  /**<   Duration of the DTMF tone in milliseconds. The value must be >= -1.
         Supported values:\n
         - -1 -- Continuous DTMF of infinite duration \n
         - 0 -- Stops a continuous DTMF, if it was started \n
         - Any positive value -- Duration in milliseconds */

  uint16_t dtmf_high_freq;
  /**<   DTMF high-tone frequency. Supported values: 100 to 4000 Hz.*/

  uint16_t dtmf_low_freq;
  /**<   DTMF low-tone frequency. Supported values: 100 to 400 Hz.          */

  uint16_t dtmf_gain;
  /**<   DTMF volume setting: Q13 gain values.  */

  uint32_t dev_ids_len;  /**< Must be set to # of elements in dev_ids */
  uint32_t dev_ids[QMI_CSD_MAX_NUM_DEV_SUPPORTED_V01];
  /**<   List of device IDs that must be enabled/disabled for DTMF. The number
         of devices to enable/disable DTMF is followed by the
         dev_ids array filled with the actual number of device entries.
         The structure does not require the addition of uint16 num_devs.*/
}qmi_csd_dev_dtmf_ctrl_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables/disables Dual-Tone Multifrequency (DTMF) on the device.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/

  /* Mandatory */
  /*  DTMF Control */
  qmi_csd_dev_dtmf_ctrl_v01 qmi_csd_dev_dtmf_cmd_payload;
}qmi_csd_ioctl_dev_cmd_dtmf_control_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables/disables Dual-Tone Multifrequency (DTMF) on the device.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_dev_cmd_dtmf_control_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_DEV_SIDETONE_CTRL_ID_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_DEV_SIDETONE_DEFAULT_V01 = 0, /**<  Default setting for the sidetone.  */
  QMI_CSD_DEV_SIDETONE_ENABLE_V01 = 1, /**<  Enables the sidetone.  */
  QMI_CSD_DEV_SIDETONE_DISABLE_V01 = 2, /**<  Disables the sidetone.  */
  QMI_CSD_DEV_SIDETONE_CTRL_ID_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_dev_sidetone_ctrl_id_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**
   Device sidetone type.


    Payload for CSD_DEV_CMD_SIDETONE_CONTROL.

 */
typedef struct {

  qmi_csd_dev_sidetone_ctrl_id_v01 sidetone_ctrl;
  /**<   Command ID for the sidetone control. Supported values:\n
         - QMI_CSD_DEV_SIDETONE_DEFAULT (0) -- Default setting for the sidetone \n
         - QMI_CSD_DEV_SIDETONE_ENABLE (1) -- Enables the sidetone \n
         - QMI_CSD_DEV_SIDETONE_DISABLE (2) -- Disables the sidetone
    */

  uint32_t rx_dev_id;
  /**<   Playback (Rx) device ID.  */

  uint32_t tx_dev_id;
  /**<   Recording (Tx) device ID.  */
}qmi_csd_dev_sidetone_ctrl_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables/disables the sidetone on the Rx/Tx device pair.  */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/

  /* Mandatory */
  /*  Sidetone Control  */
  qmi_csd_dev_sidetone_ctrl_v01 qmi_csd_dev_sidetone_cmd_payload;
}qmi_csd_ioctl_dev_cmd_sidetone_control_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables/disables the sidetone on the Rx/Tx device pair.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_dev_cmd_sidetone_control_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**   Structure for configuring the rt_proxy_port device type.
 */
typedef struct {

  uint32_t cfg_hdr;
  /**<   GUID header for a configuration structure type. */

  uint32_t num_channels;
  /**<   Number of channels. Supported values: 1 to 8. */

  uint16_t interleaved;
  /**<   Indicates whether the data exchanged between an AFE and RT port is
         interleaved. Supported values:\n
         - 0 -- Noninterleaved \n
         - 1 -- Interleaved */

  uint16_t frame_size;
  /**<   Maximum transaction buffer size in bytes, including all channels.
        Supported values: > 0. \n
        For example, a 5-ms buffer for 16-bit, 16-kHz Mono PCM samples
        has a frame size of:\n
        5(ms)*2(bytes/sample)*16(kHz) = 160 bytes  */

  uint16_t jitter_allowance;
  /**<   Configures the amount of jitter in bytes that the port allows.
         For example, if +/- 10 msec of jitter is
         anticipated in the timing of sending frames to the port and the
         configuration is 16-kHz Mono 16-bit samples, this field is:\n
         10 msec * 16 samples/msec * 2 bytes/sample = 320 */

  uint16_t low_water_mark;
  /**<   Low watermark in bytes, including all channels. If the number of
         bytes in an internal circular buffer is less than low_water_mark,
         the low_water_mark event is sent to applications via the
         AFE_EVENT_RT_PROXY_PORT_STATUS event. Supported values:\n
         - 0 -- Do not send the low_water_mark event.\n
         - > 0 -- Send the low_water_mark in bytes to trigger the event.\n
         Note: Use of the watermark event is optional. It is used for
         debugging purposes. */

  uint16_t high_water_mark;
  /**<   High watermark in bytes, including all channels. If the number of
         bytes in an internal circular buffer exceeds (TOTAL_CIRC_BUF_SIZE:
         high_water_mark), the high_water_mark event is sent to applications
         via the AFE_EVENT_RT_PROXY_PORT_STATUS event. Supported values:\n
         - 0 -- Do not send the high_water_mark event.\n
         - > 0 -- Send the high_water_mark event if the circular buffer
                  fullness exceeds (TOTAL_CIRC_BUF_SIZE: high_water_mark).\n
         Note: Use of the watermark event is optional. It is used for
         debugging purposes. */
}qmi_csd_dev_rt_proxy_port_cfg_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures a Real-Time (RT) proxy port. */
typedef struct {

  /* Mandatory */
  /*  Device Handle */
  uint32_t handle;
  /**<   Device handle.*/

  /* Mandatory */
  /*  Device ID */
  uint32_t dev_id;
  /**<   Device ID.*/

  /* Mandatory */
  /*  RT Port Proxy Configuration */
  qmi_csd_dev_rt_proxy_port_cfg_v01 qmi_csd_dev_rt_proxy_port_cfg_payload;
}qmi_csd_ioctl_dev_cmd_configure_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures a Real-Time (RT) proxy port. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_dev_cmd_configure_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VC_CMD_SET_DEVICE_CONFIG.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t tx_dev_num;
  /**<   CSD Tx device number. */

  uint32_t rx_dev_num;
  /**<   CSD Rx device number. */

  uint32_t tx_dev_sr;
  /**<   CSD Tx device sampling rate in Hz. */

  uint32_t rx_dev_sr;
  /**<   CSD Rx device sampling rate in Hz. */
}qmi_csd_vc_ioctl_set_device_config_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the device configuration on the voice processing context.  */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context. */

  /* Mandatory */
  /*  Device Configuration */
  qmi_csd_vc_ioctl_set_device_config_v01 qmi_csd_vc_ioctl_set_device_config_payload;

  /* Optional */
  /*  Echo Cancellation Reference Device */
  uint8_t ec_ref_dev_num_valid;  /**< Must be set to true if ec_ref_dev_num is being passed */
  uint32_t ec_ref_dev_num;
  /**<   CSD echo cancellation reference device ID. */
}qmi_csd_ioctl_vc_cmd_set_device_config_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the device configuration on the voice processing context.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables the voice processing context. */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context.*/

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */
}qmi_csd_ioctl_vc_cmd_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables the voice processing context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context.  */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.      */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Disables the voice processing context. */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context.*/

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */
}qmi_csd_ioctl_vc_cmd_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Disables the voice processing context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VC_CMD_SET_RX_VOLUME_INDEX.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  uint32_t vol_index;
  /**<   Rx target volume index to be set to context. */
}qmi_csd_vc_ioctl_set_rx_volume_index_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the Rx volume calibration based on the Rx volume index.  */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context.*/

  /* Mandatory */
  /*  Rx Volume Index */
  qmi_csd_vc_ioctl_set_rx_volume_index_v01 qmi_csd_vc_ioctl_set_rx_volume_index_payload;
}qmi_csd_ioctl_vc_cmd_set_rx_volume_index_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the Rx volume calibration based on the Rx volume index.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context.  */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VC_CMD_SET_NUMBER_OF_VOLUME_STEPS.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  uint32_t value;
  /**<   Number of volume steps.  */
}qmi_csd_vc_ioctl_set_number_of_volume_steps_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the total number of Rx volume steps.  */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context.*/

  /* Mandatory */
  /*  Number of Volume Steps */
  qmi_csd_vc_ioctl_set_number_of_volume_steps_v01 qmi_csd_vc_ioctl_set_number_of_volume_steps_payload;
}qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the total number of Rx volume steps.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context.  */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_VC_IOCTL_SET_RX_VOLUME_STEP.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  uint32_t vol_step;
  /**<   Rx target volume step to be set to context. */

  uint16_t ramp_duration;
  /**<   Ramp duration to disable/enable the Mute feature. Range: 0 to 5000 ms. */
}qmi_csd_vc_ioctl_set_rx_volume_step_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets a specific volume step.  */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context.*/

  /* Mandatory */
  /*  Rx Volume Step */
  qmi_csd_vc_ioctl_set_rx_volume_step_v01 qmi_csd_vc_ioctl_set_rx_volume_step_payload;
}qmi_csd_ioctl_vc_cmd_set_rx_volume_step_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets a specific volume step.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context.  */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VC_MUTE_DIR_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VC_MUTE_DIR_TX_ONLY_V01 = 0, /**<  Tx only.  */
  QMI_CSD_VC_MUTE_DIR_RX_ONLY_V01 = 1, /**<  Rx only.  */
  QMI_CSD_VC_MUTE_DIR_TX_AND_RX_V01 = 2, /**<  Tx and Rx.  */
  QMI_CSD_VC_MUTE_DIR_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_vc_mute_dir_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VC_MUTE_EN_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VC_MUTE_DISABLE_V01 = 0,
  QMI_CSD_VC_MUTE_ENABLE_V01 = 1,
  QMI_CSD_VC_MUTE_EN_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_vc_mute_en_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Unmute.
 Mute.
 Payload structure for CSD_IOCTL_VC_CMD_SET_MUTE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  qmi_csd_vc_mute_dir_v01 direction;
  /**<   Direction in which the stream is flowing:\n
         - 0 -- Tx only \n
         - 1 -- Rx only\n
         - 2 -- Tx and Rx */

  qmi_csd_vc_mute_en_v01 mute_flag;
  /**<   Mute disable/enable:\n
         - 0 -- Unmute\n
         - 1 -- Mute */
}qmi_csd_vc_ioctl_set_mute_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the mute control.   */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context.*/

  /* Mandatory */
  /*  Mute Control */
  qmi_csd_vc_ioctl_set_mute_v01 qmi_csd_vc_ioctl_set_mute_payload;

  /* Optional */
  /*  Mute Ramp Duration for Smooth Effect */
  uint8_t ramp_duration_valid;  /**< Must be set to true if ramp_duration is being passed */
  uint16_t ramp_duration;
  /**<   Ramp duration to disable/enable the Mute feature. Range: 0 to 5000 ms. */
}qmi_csd_ioctl_vc_cmd_set_mute_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the mute control.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VC_CMD_SET_TX_DTMF_DETECTION.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  uint32_t enable;
  /**<   Enables/disables Tx DTMF detection. Supported values:\n
         - 1 -- Enable \n
         - 0 -- Disable   */
}qmi_csd_vc_ioctl_tx_dtmf_detect_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables/disables Tx DTMF detection. DTMF detection status is sent
           only to the client enabling Tx DTMF detection via the
           QMI_CSD_IOCTL_VC_CMD_SET_ TX_DTMF_DETECTION command. */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context. */

  /* Mandatory */
  /*  Tx DTMF Detection */
  qmi_csd_vc_ioctl_tx_dtmf_detect_v01 qmi_csd_vc_ioctl_tx_dtmf_detect_payload;
}qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables/disables Tx DTMF detection. DTMF detection status is sent
           only to the client enabling Tx DTMF detection via the
           QMI_CSD_IOCTL_VC_CMD_SET_ TX_DTMF_DETECTION command. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates that the Tx DTMF tone is detected. */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context.*/

  /* Mandatory */
  /*  Low DTMF Frequency Detection */
  uint16_t dtmf_low_freq;
  /**<   Low DTMF tone that was detected.              */

  /* Mandatory */
  /*  High DTMF Frequency Detection */
  uint16_t dtmf_high_freq;
  /**<   High DTMF tone that was detected.        */
}qmi_csd_ioctl_vc_tx_dtmf_detected_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VC_CMD_SET_UI_PROPERTY.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  uint32_t module_id;
  /**<   ID of the module to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on module IDs. */

  uint32_t param_id;
  /**<   ID of the parameter to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on parameter IDs. */

  uint32_t param_data_len;  /**< Must be set to # of elements in param_data */
  uint8_t param_data[MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01];
  /**<   Actual data for the module ID and parameter ID. Refer to
         \hyperref[Q3]{[Q3]} for information on the payload for different
         module/parameter IDs. */
}qmi_csd_vc_ioctl_set_ui_property_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets a UI-controlled property.   */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context.*/

  /* Mandatory */
  /*  UI Property */
  qmi_csd_vc_ioctl_set_ui_property_v01 qmi_csd_vc_ioctl_set_ui_property_payload;
}qmi_csd_ioctl_vc_cmd_set_ui_property_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets a UI-controlled property.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context.  */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for request side for
           CSD_IOCTL_VC_CMD_GET_UI_PROPERTY.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  uint32_t module_id;
  /**<   ID of the module to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on module IDs. */

  uint32_t param_id;
  /**<   ID of the parameter to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on parameter IDs. */
}qmi_csd_vc_ioctl_get_ui_property_req_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for response side for
           CSD_IOCTL_VC_CMD_GET_UI_PROPERTY.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  uint32_t module_id;
  /**<   ID of the module to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on module IDs. */

  uint32_t param_id;
  /**<   ID of the parameter to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on parameter IDs. */

  uint32_t param_data_len;  /**< Must be set to # of elements in param_data */
  uint8_t param_data[MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01];
  /**<   Actual data for the module ID and parameter ID. Refer to
         \hyperref[Q3]{[Q3]} for information on the payload for different
         module/parameter IDs. */
}qmi_csd_vc_ioctl_get_ui_property_resp_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the current value of a UI-controlled property.   */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context.  */

  /* Mandatory */
  /*  UI Property */
  qmi_csd_vc_ioctl_get_ui_property_req_v01 qmi_csd_vc_ioctl_get_ui_property_req_payload;

  /* Optional */
  /*  Get UI Param Size */
  uint8_t param_size_valid;  /**< Must be set to true if param_size is being passed */
  uint32_t param_size;
  /**<   Data size of this module ID and parameter ID combination.
         The default value is 16 bytes. */
}qmi_csd_ioctl_vc_cmd_get_ui_property_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the current value of a UI-controlled property.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context.  */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  UI Property Payload */
  uint8_t qmi_csd_vc_ioctl_get_ui_property_resp_payload_valid;  /**< Must be set to true if qmi_csd_vc_ioctl_get_ui_property_resp_payload is being passed */
  qmi_csd_vc_ioctl_get_ui_property_resp_v01 qmi_csd_vc_ioctl_get_ui_property_resp_payload;
}qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates the state transition of the voice context to/from the
           Run state. */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context.*/

  /* Mandatory */
  /*  Voice Context State */
  uint32_t qmi_csd_ioctl_vc_state_id;
  /**<   Voice context state.       */
}qmi_csd_ioctl_vc_state_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VC_CAL_FEATURE_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VC_FID_CAL_VOLUME_V01 = 0, /**<  Set the volume calibration feature ID  */
  QMI_CSD_VC_FID_TOPOLOGY_V01 = 1, /**<  Set the topology feature ID  */
  QMI_CSD_VC_CAL_FEATURE_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_vc_cal_feature_type_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t feature_id;
  /**<   Feature id value that will be set for
       specific feature */

  qmi_csd_vc_cal_feature_type_v01 feature_type;
  /**<   Specify the type of feature
         0 -  volume calibration feature id
         1-   topology feature id */
}qmi_csd_vc_cal_feature_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for QMI_CSD_IOCTL_VC_CMD_SET_CAL_FEATURE_ID.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  qmi_csd_vc_cal_feature_v01 cal_feature_id;
  /**<   calibration feature ID to be set: */
}qmi_csd_vc_set_cal_feature_id_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Set the specific calibration feature ID. */
typedef struct {

  /* Mandatory */
  /*  Voice Context Handle */
  uint32_t handle;
  /**<   Unique handle for the voice context. */

  /* Mandatory */
  /*  set calibration feature */
  qmi_csd_vc_set_cal_feature_id_v01 qmi_csd_vc_set_cal_feature_id_payload;
  /**<   Set calibration feature id payload. */
}qmi_csd_vc_set_cal_feature_id_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Set the specific calibration feature ID. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice context. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_vc_set_cal_feature_id_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_SET_MEDIA_TYPE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  qmi_csd_voc_type_id_v01 rx_media_id;
  /**<   Sets the Rx vocoder type. See Appendix \ref{app:CSDVoiceMediaIDs} for
         information on media IDs. */

  qmi_csd_voc_type_id_v01 tx_media_id;
  /**<   Sets the Tx vocoder type. See Appendix \ref{app:CSDVoiceMediaIDs} for
         information on media IDs. */
}qmi_csd_vs_ioctl_set_media_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the vocoder media type on the stream.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.  */

  /* Mandatory */
  /*  Media Type */
  qmi_csd_vs_ioctl_set_media_type_v01 qmi_csd_vs_ioctl_set_media_type_payload;
}qmi_csd_ioctl_vs_cmd_set_media_type_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the vocoder media type on the stream.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VS_MUTE_DIR_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VS_MUTE_DIR_TX_ONLY_V01 = 0, /**<  Tx only.  */
  QMI_CSD_VS_MUTE_DIR_RX_ONLY_V01 = 1, /**<  Rx only.  */
  QMI_CSD_VS_MUTE_DIR_TX_AND_RX_V01 = 2, /**<  Tx and Rx.  */
  QMI_CSD_VS_MUTE_DIR_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_vs_mute_dir_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VS_MUTE_EN_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VS_MUTE_DISABLE_V01 = 0,
  QMI_CSD_VS_MUTE_WITH_SILENCE_V01 = 1,
  QMI_CSD_VS_MUTE_WITH_COMF_NOISE_V01 = 2,
  QMI_CSD_VS_MUTE_EN_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_vs_mute_en_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Unmute.
 Mute with silence.
 Mute with comfortable noise.
 Payload structure for CSD_IOCTL_VS_CMD_SET_MUTE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  qmi_csd_vs_mute_dir_v01 direction;
  /**<   Direction in which the stream is flowing:\n
         - 0 -- Tx only\n
         - 1 -- Rx only\n
         - 2 -- Tx and Rx
     */

  qmi_csd_vs_mute_en_v01 mute_flag;
  /**<   Mute status:\n
         - 0 -- Unmute\n
         - 1 -- Mute with silence\n
         - 2 -- Mute with comfort noise generation
     */
}qmi_csd_vs_ioctl_set_mute_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the mute control.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  Mute Control */
  qmi_csd_vs_ioctl_set_mute_v01 qmi_csd_vs_ioctl_set_mute_payload;

  /* Optional */
  /*  Ramp Duration */
  uint8_t ramp_duration_valid;  /**< Must be set to true if ramp_duration is being passed */
  uint16_t ramp_duration;
  /**<   Ramp duration to disable/enable the Mute feature. Range: 0 to 5000 ms. */
}qmi_csd_ioctl_vs_cmd_set_mute_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the mute control.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_SET_ENCODER_DTX_MODE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  uint32_t enable;
  /**<   Toggle DTX on/off:\n
         - 0 -- Disable\n
         - 1 -- Enable */
}qmi_csd_vs_ioctl_set_encoder_dtx_mode_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the common encoder Discontinuous Transmission (DTX) mode.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  Encoder DTX Mode */
  qmi_csd_vs_ioctl_set_encoder_dtx_mode_v01 qmi_csd_vs_ioctl_set_encoder_dtx_mode_payload;
}qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the common encoder Discontinuous Transmission (DTX) mode.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_SET_DEC_TIMEWARP.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  uint16_t enable_time_warp;
  /**<   Toggle time warping on or off:\n
         - 0x0000 -- Disable\n
         - 0x0001 -- Enable */

  uint16_t factor;
  /**<   Sets the playback compression and expansion factor. This factor is
         also known as the time warping expansion length. Supported values:\n
         Narrowband:\n
         - 80 to 160 -- Compression\n
         - 160 to 320 -- Expansion\n
         Wideband:\n
         - 160 to 320 -- Compression\n
         - 320 to 640 -- Expansion */

  uint16_t enable_phase_match;
  /**<   Toggle phase matching on or off:\n
         - 0x0000 -- Disable\n
         - 0x0001 -- Enable */

  uint16_t run_length;
  /**<   Run length is equal to the number of consecutive erasures the
         decoder has decoded immediately prior to the decoding of the current
         packet. Supported values: > 0.  */

  int16_t phase_offset;
  /**<   Phase offset is equal to the difference between the number of
         frames encoded and decoded. Supported values: -2, -1, 0, 1, and 2. */
}qmi_csd_vs_ioctl_set_dec_timewarp_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the common decoder time warping parameter. This command can be
           sent on a per frame basis depending on the compression and expansion
           requirement.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  Timewarp Configuration */
  qmi_csd_vs_ioctl_set_dec_timewarp_v01 qmi_csd_vs_ioctl_set_dec_timewarp_payload;
}qmi_csd_ioctl_vs_cmd_set_dec_timewarp_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the common decoder time warping parameter. This command can be
           sent on a per frame basis depending on the compression and expansion
           requirement.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VOC_ENC_RATE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  BLANK_FRAME_V01 = 0, /**<  Blank frame.  */
  EITHTH_RATE_V01 = 1, /**<  Eighth rate.  */
  QUARTER_RATE_V01 = 2, /**<  Quarter rate.  */
  HALF_RATE_V01 = 3, /**<  Half rate.  */
  FULL_RATE_V01 = 4, /**<  Full rate.  */
  QMI_CSD_VOC_ENC_RATE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_voc_enc_rate_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_SET_ENC_MINMAX_RATE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  qmi_csd_voc_enc_rate_v01 min_rate;
  /**<   Sets the lower bound encoder rate:\n
         - 0x0000 -- Blank frame\n
         - 0x0001 -- Eighth rate\n
         - 0x0002 -- Quarter rate\n
         - 0x0003 -- Half rate\n
         - 0x0004 -- Full rate      */

  qmi_csd_voc_enc_rate_v01 max_rate;
  /**<   Sets the upper bound encoder rate:\n
         - 0x0000 -- Blank frame\n
         - 0x0001 -- Eighth rate\n
         - 0x0002 -- Quarter rate\n
         - 0x0003 -- Half rate \n
         - 0x0004 -- Full rate      */
}qmi_csd_vs_ioctl_set_enc_minmax_rate_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the CDMA-specific encoder minimum and maximum rate.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Mandatory */
  /*  Encoder Rate */
  qmi_csd_vs_ioctl_set_enc_minmax_rate_v01 qmi_csd_vs_ioctl_set_enc_minmax_rate_payload;
}qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the CDMA-specific encoder minimum and maximum rate.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_SET_ENC_RATE_MODULATION.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t mode;
  /**<   Sets the vocoder reduced rate modulation mode.

     The bit structure for the mode is:\n
     - b0 -- Vocoder rate modulation is enabled when 1, and disabled when 0 \n
     - b1 -- Select X=S when 1, and select X=1/S when 0 \n
     - b9 to b2 -- Rate limit factor is the value of S \n
     - b31 to b10 -- Reserved; keep as zeros

     See Appendix \ref{app:vrModulation} for more information.
    */
}qmi_csd_vs_ioctl_set_enc_rate_mod_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the CDMA-specific encoder rate modulation.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.  */

  /* Mandatory */
  /*  Encoder Rate Modulation */
  qmi_csd_vs_ioctl_set_enc_rate_mod_v01 qmi_csd_vs_ioctl_set_enc_rate_mod_payload;
}qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the CDMA-specific encoder rate modulation.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  VOC_RATE_QCELP13K_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VOC_RATE_QCELP13k_144_V01 = 0, /**<  Sets the QCELP13K vocoder rate to 14.4 kbps.  */
  QMI_CSD_VOC_RATE_QCELP13k_122_V01 = 1, /**<  Sets the QCELP13K vocoder rate to 12.2 kbps.  */
  QMI_CSD_VOC_RATE_QCELP13k_112_V01 = 2, /**<  Sets the QCELP13K vocoder rate to 11.2 kbps.  */
  QMI_CSD_VOC_RATE_QCELP13k_90_V01 = 3, /**<  Sets the QCELP13K vocoder rate to 9.0 kbps.  */
  QMI_CSD_VOC_RATE_QCELP13k_72_V01 = 4, /**<  Sets the QCELP13K vocoder rate to 7.2 kbps.  */
  VOC_RATE_QCELP13K_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}voc_rate_qcelp13k_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_VOC_QCELP13K_SET_RATE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  voc_rate_qcelp13k_v01 rate;
  /**<   Sets the QCELP13K vocoder rate:\n
         - 0x00000000 -- 14.4 kbps\n
         - 0x00000001 -- 12.2 kbps\n
         - 0x00000002 -- 11.2 kbps\n
         - 0x00000003 --  9.0 kbps\n
         - 0x00000004 --  7.2 kbps */
}qmi_csd_vs_ioctl_voc_qcelp13k_set_rate_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the Qualcomm Code Excited Linear Prediction (QCELP) 13k encoder
           rate.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.  */

  /* Mandatory */
  /*  QCELP13K Vocoder Rate */
  qmi_csd_vs_ioctl_voc_qcelp13k_set_rate_v01 qmi_csd_vs_ioctl_voc_qcelp13k_set_rate_payload;
}qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the Qualcomm Code Excited Linear Prediction (QCELP) 13k encoder
           rate.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  VOC_RATE_4GVNB_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VOC_RATE_4GVNB_100_V01 = 0, /**<  Sets the narrowband vocoder rate to 10.0 kbps.  */
  QMI_CSD_VOC_RATE_4GVNB_85_V01 = 1, /**<  Sets the narrowband vocoder rate to 8.5 kbps.  */
  QMI_CSD_VOC_RATE_4GVNB_75_V01 = 2, /**<  Sets the narrowband vocoder rate to 7.5 kbps.  */
  QMI_CSD_VOC_RATE_4GVNB_70_V01 = 3, /**<  Sets the narrowband vocoder rate to 7.0 kbps.  */
  QMI_CSD_VOC_RATE_4GVNB_66_V01 = 4, /**<  Sets the narrowband vocoder rate to 6.6 kbps.  */
  QMI_CSD_VOC_RATE_4GVNB_62_V01 = 5, /**<  Sets the narrowband vocoder rate to 6.2 kbps.  */
  QMI_CSD_VOC_RATE_4GVNB_58_V01 = 6, /**<  Sets the narrowband vocoder rate to 5.8 kbps.  */
  QMI_CSD_VOC_RATE_4GVNB_48_V01 = 7, /**<  Sets the narrowband vocoder rate to 4.8 kbps.  */
  VOC_RATE_4GVNB_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}voc_rate_4gvnb_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_VOC_4GVNB_SET_RATE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  voc_rate_4gvnb_v01 rate;
  /**<   Sets the narrowband vocoder rate:\n
         - 0x00000000 -- 10.0 kbps\n
         - 0x00000001 --  8.5 kbps\n
         - 0x00000002 --  7.5 kbps\n
         - 0x00000003 --  7.0 kbps\n
         - 0x00000004 --  6.6 kbps\n
         - 0x00000005 --  6.2 kbps\n
         - 0x00000006 --  5.8 kbps\n
         - 0x00000007 --  4.8 kbps  */
}qmi_csd_vs_ioctl_voc_4gvnb_set_rate_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the Fourth-Generation Narrowband Vocoder (4GV-NB) encoder rate.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Mandatory */
  /*  4GV-NB Vocoder Rate */
  qmi_csd_vs_ioctl_voc_4gvnb_set_rate_v01 qmi_csd_vs_ioctl_voc_4gvnb_set_rate_payload;
}qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the Fourth-Generation Narrowband Vocoder (4GV-NB) encoder rate.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  VOC_RATE_4GVWB_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VOC_RATE_4GVWB_85_V01 = 0, /**<  Sets the wideband vocoder rate to 8.5 kbps. */
  QMI_CSD_VOC_RATE_4GVWB_100_V01 = 4, /**<  Sets the wideband vocoder rate to 10.0 kbps. */
  QMI_CSD_VOC_RATE_4GVWB_48_V01 = 7, /**<  Sets the wideband vocoder rate to 4.8 kbps. */
  VOC_RATE_4GVWB_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}voc_rate_4gvwb_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_VOC_4GVWB_SET_RATE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  voc_rate_4gvwb_v01 rate;
  /**<   Sets the wideband vocoder rate:\n
         - 0x00000000 --  8.5 kbps\n
         - 0x00000004 -- 10.0 kbps\n
         - 0x00000007 --  4.8 kbps */
}qmi_csd_vs_ioctl_voc_4gvwb_set_rate_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the Fourth-Generation Wideband Vocoder (4GV-WB) encoder rate.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.  */

  /* Mandatory */
  /*  4GV-WB Vocoder Rate */
  qmi_csd_vs_ioctl_voc_4gvwb_set_rate_v01 qmi_csd_vs_ioctl_voc_4gvwb_set_rate_payload;
}qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the Fourth-Generation Wideband Vocoder (4GV-WB) encoder rate.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  VOC_AMR_ENC_RATE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VOC_AMR_ENC_RATE_475_V01 = 0, /**<  Sets the AMR encoder rate to 4.75 kbps. */
  QMI_CSD_VOC_AMR_ENC_RATE_515_V01 = 1, /**<  Sets the AMR encoder rate to 5.15 kbps. */
  QMI_CSD_VOC_AMR_ENC_RATE_590_V01 = 2, /**<  Sets the AMR encoder rate to 5.90 kbps. */
  QMI_CSD_VOC_AMR_ENC_RATE_670_V01 = 3, /**<  Sets the AMR encoder rate to 6.70 kbps. */
  QMI_CSD_VOC_AMR_ENC_RATE_740_V01 = 4, /**<  Sets the AMR encoder rate to 7.40 kbps. */
  QMI_CSD_VOC_AMR_ENC_RATE_795_V01 = 5, /**<  Sets the AMR encoder rate to 7.95 kbps. */
  QMI_CSD_VOC_AMR_ENC_RATE_102_V01 = 6, /**<  Sets the AMR encoder rate to 10.2 kbps. */
  QMI_CSD_VOC_AMR_ENC_RATE_122_V01 = 7, /**<  Sets the AMR encoder rate to 12.2 kbps. */
  VOC_AMR_ENC_RATE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}voc_amr_enc_rate_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_VOC_AMR_SET_ENC_RATE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  voc_amr_enc_rate_v01 mode;
  /**<   Sets the AMR encoder rate:\n
         - 0x00000000 -- 4.75 kbps\n
         - 0x00000001 -- 5.15 kbps\n
         - 0x00000002 -- 5.90 kbps\n
         - 0x00000003 -- 6.70 kbps\n
         - 0x00000004 -- 7.40 kbps\n
         - 0x00000005 -- 7.95 kbps\n
         - 0x00000006 -- 10.2 kbps\n
         - 0x00000007 -- 12.2 kbps  */
}qmi_csd_vs_ioctl_voc_amr_set_enc_rate_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the Adaptive Multirate (AMR) encoder rate.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.  */

  /* Mandatory */
  /*  AMR Encoder Rate */
  qmi_csd_vs_ioctl_voc_amr_set_enc_rate_v01 qmi_csd_vs_ioctl_voc_amr_set_enc_rate_payload;
}qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the Adaptive Multirate (AMR) encoder rate.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  VOC_AMRWB_ENC_RATE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VOC_AMRWB_ENC_RATE_660_V01 = 0, /**<  Sets the AMR-WB encoder encoder rate to 6.60 kbps. */
  QMI_CSD_VOC_AMRWB_ENC_RATE_885_V01 = 1, /**<  Sets the AMR-WB encoder encoder rate to 8.85 kbps. */
  QMI_CSD_VOC_AMRWB_ENC_RATE_1265_V01 = 2, /**<  Sets the AMR-WB encoder encoder rate to 12.65 kbps. */
  QMI_CSD_VOC_AMRWB_ENC_RATE_1425_V01 = 3, /**<  Sets the AMR-WB encoder encoder rate to 14.25 kbps. */
  QMI_CSD_VOC_AMRWB_ENC_RATE_1585_V01 = 4, /**<  Sets the AMR-WB encoder encoder rate to 15.85 kbps. */
  QMI_CSD_VOC_AMRWB_ENC_RATE_1825_V01 = 5, /**<  Sets the AMR-WB encoder encoder rate to 18.25 kbps. */
  QMI_CSD_VOC_AMRWB_ENC_RATE_1985_V01 = 6, /**<  Sets the AMR-WB encoder encoder rate to 19.85 kbps. */
  QMI_CSD_VOC_AMRWB_ENC_RATE_2305_V01 = 7, /**<  Sets the AMR-WB encoder encoder rate to 23.05 kbps. */
  QMI_CSD_VOC_AMRWB_ENC_RATE_2335_V01 = 8, /**<  Sets the AMR-WB encoder encoder rate to 23.85 kbps. */
  VOC_AMRWB_ENC_RATE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}voc_amrwb_enc_rate_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_VOC_AMRWB_SET_ENC_RATE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  voc_amrwb_enc_rate_v01 mode;
  /**<   Sets the AMR-WB encoder rate:\n
         - 0x00000000 --  6.60 kbps\n
         - 0x00000001 --  8.85 kbps\n
         - 0x00000002 -- 12.65 kbps\n
         - 0x00000003 -- 14.25 kbps\n
         - 0x00000004 -- 15.85 kbps\n
         - 0x00000005 -- 18.25 kbps\n
         - 0x00000006 -- 19.85 kbps\n
         - 0x00000007 -- 23.05 kbps\n
         - 0x00000008 -- 23.85 kbps  */
}qmi_csd_vs_ioctl_voc_amrwb_set_enc_rate_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the wideband AMR (AMR-WB) encoder rate.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.  */

  /* Mandatory */
  /*  AMR-WB Encoder Rate */
  qmi_csd_vs_ioctl_voc_amrwb_set_enc_rate_v01 qmi_csd_vs_ioctl_voc_amrwb_set_enc_rate_payload;
}qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the wideband AMR (AMR-WB) encoder rate.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_SET_DTMF_GENERATION.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint16_t direction;
  /**<   DTMF generation direction. Supported values:
         QMI_CSD_VS_DTMF_GENERATION_ DIRECTION_TX.

         Rx DTMF generation is available on the audio stream side,
         which is not supported in the initial version.
         It is also available from the device control side.   */

  uint16_t mix_flag;
  /**<   Mix with speech flag. Supported values:\n
         - 1 -- Generated DTMF is mixed with the speech \n
         - 0 -- Generated DTMF replaces the speech */

  uint16_t tone_1;
  /**<   DTMF tone 1. Supported values: 100 to 4000 Hz. */

  uint16_t tone_2;
  /**<   DTMF tone 2. Supported values: 100 to 4000 Hz. */

  uint16_t gain;
  /**<   DTMF tone gain. Supported values: Linear value in Q13 format. This
         value must be set to a negative gain because the level of tone
         generation is fixed at 0 dBFS. */

  int16_t duration;
  /**<   Duration of the tone. Duration includes ramp-up and ramp-down periods.
         The ramp-up and ramp-down periods are 1 ms and 2 ms, respectively.
         Supported values:\n
         - -1 -- Infinite duration; the client sends 0 (stops the infinite
                 tone) duration to end the tone \n
         -  0 -- Stops the infinite tone \n
         - > 0 -- Finite duration in milliseconds */
}qmi_csd_vs_ioctl_set_dtmf_generation_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Starts/stops DTMF generation. The completion of DTMF generation,
           either due to a Stop command or because of the requested duration
           has elapsed, is indicated to the client via the
           QMI_CSD_IOCTL_VS_CMD_DTMF_ GENERATION_ENDED_IND indication message. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.  */

  /* Mandatory */
  /*  DTMF Generation Configuration */
  qmi_csd_vs_ioctl_set_dtmf_generation_v01 qmi_csd_vs_ioctl_set_dtmf_generation_payload;
}qmi_csd_ioctl_vs_cmd_set_dtmf_generation_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Starts/stops DTMF generation. The completion of DTMF generation,
           either due to a Stop command or because of the requested duration
           has elapsed, is indicated to the client via the
           QMI_CSD_IOCTL_VS_CMD_DTMF_ GENERATION_ENDED_IND indication message. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates to the stream client that the generation of DTMF tone
           has ended. This indication is sent by the stream to the client that
           enabled DTMF generation when the client issues a Stop command or the
           duration requested by the client has elapsed. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Mandatory */
  /*  DTMF Tone Direction */
  uint16_t direction;
  /**<   Direction in which the DTMF tone has been generated.
          Supported values: QMI_CSD_VS_DTMF_GENERATION_ DIRECTION_TX. */
}qmi_csd_ioctl_vs_dtmf_generation_ended_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_SET_RX_DTMF_DETECTION.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t enable;
  /**<   Enables/disables Rx DTMF detection. Supported values:\n
         - 1 -- Enable\n
         - 0 -- Disable  */
}qmi_csd_vs_ioctl_set_rx_dtmf_detection_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables/disables Rx DTMF detection. The DTMF tone detection status
           is sent to the client sending this command via the
           QMI_CSD_IOCTL_VS_CMD_RX_ DTMF_DETECTION_IND indication message. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  Rx DTMF Detection */
  qmi_csd_vs_ioctl_set_rx_dtmf_detection_v01 qmi_csd_vs_ioctl_set_rx_dtmf_detection_payload;
}qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables/disables Rx DTMF detection. The DTMF tone detection status
           is sent to the client sending this command via the
           QMI_CSD_IOCTL_VS_CMD_RX_ DTMF_DETECTION_IND indication message. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates the Rx DTMF tone detected. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  DTMF Low Frequency Detection */
  uint16_t dtmf_low_freq;
  /**<   Low DTMF tone is detected.              */

  /* Mandatory */
  /*  DTMF High Frequency Detection */
  uint16_t dtmf_high_freq;
  /**<   High DTMF tone is detected.         */
}qmi_csd_ioctl_vs_rx_dtmf_detected_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VS_CMD_SET_UI_PROPERTY.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t module_id;
  /**<   ID of the module to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on module IDs. */

  uint32_t param_id;
  /**<   ID of the parameter to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on parameter IDs. */

  uint32_t param_data_len;  /**< Must be set to # of elements in param_data */
  uint8_t param_data[MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01];
  /**<   Actual data for the module ID and parameter ID. Refer to
         \hyperref[Q3]{[Q3]} for information on the payload for different
         module/parameter IDs. */
}qmi_csd_vs_ioctl_set_ui_property_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets a UI-controlled property of the voice stream. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  UI Property Configuration */
  qmi_csd_vs_ioctl_set_ui_property_v01 qmi_csd_vs_ioctl_set_ui_property_payload;
}qmi_csd_ioctl_vs_cmd_set_ui_property_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets a UI-controlled property of the voice stream. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for request side for
           CSD_IOCTL_VS_CMD_GET_UI_PROPERTY.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t module_id;
  /**<   ID of the module to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on module IDs. */

  uint32_t param_id;
  /**<   ID of the parameter to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on parameter IDs. */
}qmi_csd_vs_ioctl_get_ui_property_req_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for the response side for
           CSD_IOCTL_VS_CMD_GET_UI_PROPERTY.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t module_id;
  /**<   ID of the module to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on module IDs. */

  uint32_t param_id;
  /**<   ID of the parameter to be configured. Refer to \hyperref[Q3]{[Q3]}
         for information on parameter IDs. */

  uint32_t param_data_len;  /**< Must be set to # of elements in param_data */
  uint8_t param_data[MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01];
  /**<   Actual data for the module ID and parameter ID. Refer to
         \hyperref[Q3]{[Q3]} for information on the payload for different
         module/parameter IDs. */
}qmi_csd_vs_ioctl_get_ui_property_resp_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the current value of a UI-controlled property on a voice
           stream.   */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  UI Property  */
  qmi_csd_vs_ioctl_get_ui_property_req_v01 qmi_csd_vs_ioctl_get_ui_property_req_payload;

  /* Optional */
  /*  Get UI Param Size */
  uint8_t param_size_valid;  /**< Must be set to true if param_size is being passed */
  uint32_t param_size;
  /**<   Data size of this module ID and parameter ID combination.
         The default value is 16 bytes. */
}qmi_csd_ioctl_vs_cmd_get_ui_property_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the current value of a UI-controlled property on a voice
           stream.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  UI Property Payload */
  uint8_t qmi_csd_vs_ioctl_get_ui_property_resp_payload_valid;  /**< Must be set to true if qmi_csd_vs_ioctl_get_ui_property_resp_payload is being passed */
  qmi_csd_vs_ioctl_get_ui_property_resp_v01 qmi_csd_vs_ioctl_get_ui_property_resp_payload;
}qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VS_TAP_POINT_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VS_TAP_POINT_NONE_V01 = 0x00010F78, /**<  Do not record the Rx path.  */
  QMI_CSD_VS_TAP_POINT_STREAM_END_V01 = 0x00010F79, /**<  Rx tap point is at the end of the
                                             stream.  */
  QMI_CSD_VS_TAP_POINT_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_vs_tap_point_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_VS_RECORDING_MODE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_VS_RECORD_MODE_TX_RX_STEREO_V01 = 0x00010F7A,
  QMI_CSD_VS_RECORD_MODE_TX_RX_MIXING_V01 = 0x00010F7B,
  QMI_CSD_VS_RECORDING_MODE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_vs_recording_mode_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Starts recording the conversation based on the specified direction
           of the recording. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  /* Mandatory */
  /*  Rx Tap Point */
  uint32_t rx_tap_point;
  /**<   Tap point to use on the Rx path. Supported values:\n
         - QMI_CSD_VS_TAP_POINT_NONE (0x00010F78) -- Do not record the Rx
           path.\n
         - QMI_CSD_VS_TAP_POINT_STREAM_END (0x00010F79) -- Rx tap point is at
           the end of the stream. */

  /* Mandatory */
  /*  Tx Tap Point */
  uint32_t tx_tap_point;
  /**<   Tap point to use on the Tx path. Supported values:\n
         - QMI_CSD_VS_TAP_POINT_NONE (0x00010F78) -- Do not record the Tx
           path.\n
         - QMI_CSD_VS_TAP_POINT_STREAM_END (0x00010F79) -- Tx tap point is at
           the end of the stream. */

  /* Optional */
  /*  Device ID */
  uint8_t dev_id_valid;  /**< Must be set to true if dev_id is being passed */
  uint32_t dev_id;
  /**<   Conversation data is available on the recording device ID.
         Data is routed to the AFE port of the device ID indicated. */

  /* Optional */
  /*  Recording Mode */
  uint8_t mode_valid;  /**< Must be set to true if mode is being passed */
  qmi_csd_vs_recording_mode_t_v01 mode;
  /**<   Recording mode. Supported values: \n
          - QMI_CSD_VS_RECORD_MODE_TX_RX_ STEREO (0x00010F7A) -- L,R format
            is recorded from the AFE.\n
          - QMI_CSD_VS_RECORD_MODE_TX_RX_ MIXING (0x00010F7B) -- L+R format
            is recorded from the AFE.
      */
}qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Starts recording the conversation based on the specified direction
           of the recording. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Stop recording the conversation. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */
}qmi_csd_ioctl_vs_cmd_stop_record_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Stop recording the conversation. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Starts the injection of playback into the voice call. The
            playback on the device ID provided is injected into the Tx path
            of the conversation on the voice call. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  /* Mandatory */
  /*  Device ID */
  uint32_t dev_id;
  /**<   Audio received at the device ID is delivered to the Tx voice call
          path. */
}qmi_csd_ioctl_vs_cmd_start_playback_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Starts the injection of playback into the voice call. The
            playback on the device ID provided is injected into the Tx path
            of the conversation on the voice call. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Stops the mixing of audio with the voice Tx. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */
}qmi_csd_ioctl_vs_cmd_stop_playback_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Stops the mixing of audio with the voice Tx. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice stream. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates the voice stream's state transition to/from the Run
           state. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/

  /* Mandatory */
  /*  Voice Stream State */
  uint32_t qmi_csd_ioctl_vc_state_id;
  /**<   Voice stream state.       */
}qmi_csd_ioctl_vs_state_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates to the stream client that an encoder buffer is
           available for pickup. The media type of the buffer is as
           passed to the stream in QMI_CSM_OPEN_VOICE_STREAM. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/
}qmi_csd_ioctl_vs_enc_buffer_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates to the stream client that a decoder buffer must be
           provided. The media type of the buffer is as passed to the stream in
           QMI_CSM_OPEN_VOICE_STREAM. */
typedef struct {

  /* Mandatory */
  /*  Voice Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.*/
}qmi_csd_ioctl_vs_dec_buffer_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VM_CMD_ATTACH_STREAM.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t stream_handle;
  /**<   Stream to attach. */
}qmi_csd_vm_ioctl_attach_stream_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Attaches a voice stream to the voice manager.  */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager.*/

  /* Mandatory */
  /*  Attach Voice Stream */
  qmi_csd_vm_ioctl_attach_stream_v01 qmi_csd_vm_ioctl_attach_stream_payload;
}qmi_csd_ioctl_vm_cmd_attach_stream_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Attaches a voice stream to the voice manager.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VM_CMD_DETACH_STREAM.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t stream_handle;
  /**<   Stream to detach. */
}qmi_csd_vm_ioctl_detach_stream_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Detaches a voice stream from the voice manager.  */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice stream.        */

  /* Mandatory */
  /*  Detach Voice Stream  */
  qmi_csd_vm_ioctl_detach_stream_v01 qmi_csd_vm_ioctl_detach_stream_payload;
}qmi_csd_ioctl_vm_cmd_detach_stream_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Detaches a voice stream from the voice manager.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Manager Handle  */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager.     */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VM_CMD_ATTACH_CONTEXT.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t context_handle;
  /**<   Voice context handle opened by the QMI_CSD_OPEN_VOICE_CONTEXT_REQ message,
         which must be attached. */
}qmi_csd_vm_ioctl_attach_context_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Attaches a voice context to the voice manager.  */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager.           */

  /* Mandatory */
  /*  Attach Voice Context */
  qmi_csd_vm_ioctl_attach_context_v01 qmi_csd_vm_ioctl_attach_context_payload;
}qmi_csd_ioctl_vm_cmd_attach_context_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Attaches a voice context to the voice manager.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VM_CMD_DETACH_CONTEXT.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t context_handle;
  /**<   Voice context handle having been opened upon
         QMI_CSD_OPEN_VOICE_CONTEXT_REQ message, which must be attached. */
}qmi_csd_vm_ioctl_detach_context_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Detaches a voice context from the voice manager.  */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager.*/

  /* Mandatory */
  /*  Detach Context Control */
  qmi_csd_vm_ioctl_detach_context_v01 qmi_csd_vm_ioctl_detach_context_payload;
}qmi_csd_ioctl_vm_cmd_detach_context_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Detaches a voice context from the voice manager.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Starts voice on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager.*/

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */
}qmi_csd_ioctl_vm_cmd_start_voice_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Starts voice on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager.*/

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Pause voice on the voice manager. */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the completed command.  */
}qmi_csd_ioctl_vm_cmd_pause_voice_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Pause voice on the voice manager. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the completed command.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status. */
}qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Standby voice on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the completed command.  */
}qmi_csd_ioctl_vm_cmd_standby_voice_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Standby voice on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the completed command.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status. */
}qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Stops voice on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager.*/

  /* Mandatory */
  /*  Transaction Identifier */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.         */
}qmi_csd_ioctl_vm_cmd_stop_voice_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Stops voice on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager.*/

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VM_CMD_SET_NETWORK.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  qmi_csd_network_id_v01 network_id;
  /**<   Network ID. See Appendix \ref{app:CSDVoiceNetworkIDs} for information
         on network IDs. Default: 0. */
}qmi_csd_vm_ioctl_set_network_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the network type on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager.*/

  /* Mandatory */
  /*  Network Configuration */
  qmi_csd_vm_ioctl_set_network_v01 qmi_csd_vm_ioctl_set_network_payload;
}qmi_csd_ioctl_vm_cmd_set_network_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the network type on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VM_CMD_SET_VOICE_TIMING.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  uint16_t mode;
  /**<   Vocoder frame synchronization mode:\n
         - 0 -- No frame synchronization \n
         - 1 -- Hard vocoder frame reference interrupt; 20 ms */

  uint16_t enc_offset;
  /**<   Offset in microseconds from the vocoder frame reference to deliver a
         Tx vocoder packet. The offset must be less than \n 20 ms. */

  uint16_t dec_req_offset;
  /**<   Offset in microseconds from the vocoder frame reference to request
         a Rx vocoder packet. The offset must be less than \n 20 ms. */

  uint16_t dec_offset;
  /**<   Offset in microseconds from the vocoder frame reference to indicate
         the deadline to receive an Rx vocoder packet. The offset must be less
         than 20 ms. Rx vocoder packets received after this deadline are not
         guaranteed to be processed. */
}qmi_csd_vm_ioctl_set_voice_timing_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the voice timing parameter on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager.*/

  /* Mandatory */
  /*  Voice Timing Configuration */
  qmi_csd_vm_ioctl_set_voice_timing_v01 qmi_csd_vm_ioctl_set_voice_timing_payload;
}qmi_csd_ioctl_vm_cmd_set_voice_timing_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the voice timing parameter on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  TTY_MODE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_TTY_DISABLE_V01 = 0, /**<  Teletypewriter (TTY) disabled mode type.  */
  QMI_CSD_TTY_HCO_V01 = 1, /**<  Hearing Carry Over (HCO) mode type.  */
  QMI_CSD_TTY_VCO_V01 = 2, /**<  Voice Carry Over (VCO) mode type.  */
  QMI_CSD_TTY_FULL_V01 = 3, /**<  Full mode type.  */
  TTY_MODE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}tty_mode_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VM_CMD_SET_TTY_MODE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  tty_mode_v01 mode;
  /**<   Mode type:\n
         - 0 -- Teletypewriter (TTY) is disabled.\n
         - 1 -- Hearing Carry Over (HCO)\n
         - 2 -- Voice Carry Over (VCO)\n
         - 3 -- Full */
}qmi_csd_vm_ioctl_set_tty_mode_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the TTY mode on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager.*/

  /* Mandatory */
  /*  TTY Mode Type */
  qmi_csd_vm_ioctl_set_tty_mode_v01 qmi_csd_vm_ioctl_set_tty_mode_payload;
}qmi_csd_ioctl_vm_cmd_set_tty_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the TTY mode on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  Payload structure for CSD_IOCTL_VM_CMD_SET_WIDEVOICE.
 */
typedef struct {

  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed. */

  uint32_t enable;
  /**<   WideVoice enable/disable:\n
         - 1 -- Enable\n
         - 0 -- Disable */
}qmi_csd_vm_ioctl_set_widevoice_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets WideVoice on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Voice Manager Handle */
  uint32_t handle;
  /**<   Unique handle for the voice manager.*/

  /* Mandatory */
  /*  WideVoice Configuration */
  qmi_csd_vm_ioctl_set_widevoice_v01 qmi_csd_vm_ioctl_set_widevoice_payload;
}qmi_csd_ioctl_vm_cmd_set_widevoice_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets WideVoice on the voice manager.   */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Voice Manager Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the voice manager. */

  /* Optional */
  /*  Transaction Identifier */
  uint8_t cmd_token_valid;  /**< Must be set to true if cmd_token is being passed */
  uint32_t cmd_token;
  /**<   Transaction identifier provided by the client that allows the client
         to identify the command that completed.  */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_OPEN_OPCODE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_OPEN_OP_R_V01 = 0x01, /**<  Open for read.  */
  QMI_CSD_OPEN_OP_W_V01 = 0x02, /**<  Open for write.  */
  QMI_CSD_OPEN_OP_RW_V01 = 0x03, /**<  Open for read and write.  */
  QMI_CSD_AS_OPEN_OPCODE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_open_opcode_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_DATA_MODE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_DATA_MODE_ASYNC_V01 = 0x00, /**<  Asynchronous data call.  */
  QMI_CSD_AS_DATA_MODE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_data_mode_v01;
/**
    @}
  */

/**  CSD audio stream opcode.

 CSD audio stream call mode type.

 CSD Audio Stream Open Mask
                                          */
typedef uint32_t qmi_csd_as_open_mask_v01;
#define QMI_CSD_AS_OPEN_MASK_SESSION_ID_V01 ((qmi_csd_as_open_mask_v01)0x00000001) /**<  Session ID mask   */
#define QMI_CSD_AS_OPEN_MASK_SR_CM_NOTIFY_V01 ((qmi_csd_as_open_mask_v01)0x00000002) /**<  Sample rate or channel mode change notification mask  */
#define QMI_CSD_AS_OPEN_MASK_META_FRAMES_V01 ((qmi_csd_as_open_mask_v01)0x00000004) /**<  Multiframe support mask; this mask is only valid on Tx streams  */
#define QMI_CSD_AS_OPEN_MASK_GAPLESS_V01 ((qmi_csd_as_open_mask_v01)0x00000008) /**<  Gapless mode mask; this mask is only valid for playback  */
#define QMI_CSD_AS_OPEN_MASK_LOW_POWER_MODE_V01 ((qmi_csd_as_open_mask_v01)0x00000010) /**<  Low Power mode; indicates that the session is to be opened in Low
             Power mode  */
/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_OPEN_FORMAT_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FORMAT_UNKNOWN_V01 = 0,
  QMI_CSD_AS_FORMAT_PCM_V01 = 1,
  QMI_CSD_AS_FORMAT_ADPCM_V01 = 2,
  QMI_CSD_AS_FORMAT_MP3_V01 = 3,
  QMI_CSD_AS_FORMAT_RA_V01 = 4,
  QMI_CSD_AS_FORMAT_WMA_V01 = 5,
  QMI_CSD_AS_FORMAT_AAC_V01 = 6,
  QMI_CSD_AS_FORMAT_MIDI_V01 = 7,
  QMI_CSD_AS_FORMAT_YADPCM_V01 = 8,
  QMI_CSD_AS_FORMAT_QCELP8K_V01 = 9,
  QMI_CSD_AS_FORMAT_AMRNB_V01 = 10,
  QMI_CSD_AS_FORMAT_AMRWB_V01 = 11,
  QMI_CSD_AS_FORMAT_EVRC_V01 = 12,
  QMI_CSD_AS_FORMAT_WMAPRO_V01 = 13,
  QMI_CSD_AS_FORMAT_QCELP13K_V01 = 14,
  QMI_CSD_AS_FORMAT_SBC_V01 = 15,
  QMI_CSD_AS_FORMAT_EVRCB_V01 = 16,
  QMI_CSD_AS_FORMAT_AMRWBPLUS_V01 = 17,
  QMI_CSD_AS_FORMAT_AC3_V01 = 18,
  QMI_CSD_AS_FORMAT_EVRCWB_V01 = 19,
  QMI_CSD_AS_FORMAT_FLAC_V01 = 20,
  QMI_CSD_AS_FORMAT_VORBIS_V01 = 21,
  QMI_CSD_AS_FORMAT_G711ALAW_V01 = 22,
  QMI_CSD_AS_FORMAT_G711ULAW_V01 = 23,
  QMI_CSD_AS_FORMAT_G729A_V01 = 24,
  QMI_CSD_AS_FORMAT_DTMF_V01 = 25,
  QMI_CSD_AS_FORMAT_GSMFR_V01 = 26,
  QMI_CSD_AS_FORMAT_EAC3_V01 = 27,
  QMI_CSD_AS_OPEN_FORMAT_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_open_format_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  CSD audio format type.

 */
typedef struct {

  qmi_csd_as_open_opcode_v01 op_code;
  /**<   Operation code for the audio stream. */

  qmi_csd_as_data_mode_v01 data_mode;
  /**<   Defines the behavior of data path APIs.
         Currently only supports asynchronous calls. */

  qmi_csd_as_open_format_v01 format_type_rx;
  /**<   Format type for playback. See Appendix \ref{app:CSDAudioFormatTypes} for
         information on supported audio formats. */

  qmi_csd_as_open_format_v01 format_type_tx;
  /**<   Format type for recording. See Appendix \ref{app:CSDAudioFormatTypes} for
         information on supported audio formats.  */

  qmi_csd_as_open_mask_v01 open_mask;
  /**<   Specifies the open mode, and indicates the optional fields that are
         present to support the open fields. Supported values: \begin{itemize1}
         \item 0 -- Field does not exist
         \item 1 -- Field exists; supported values: \begin{itemize1}
                \item Bit 0 -- Session ID mask
                \item Bit 1 -- Sample rate or channel mode change notification
                           mask
                \item Bit 2 -- Multiframe support mask; this mask is only valid on
                           Tx streams
                \item Bit 3 -- Gapless mode mask; this mask is only valid for
                           playback
                \item Bits 4 to 31 -- Reserved = 0
                \end{itemize1}
          \vspace{-0.18in} \end{itemize1}
    */

  uint32_t session_id;
  /**<   Session ID for the stream. This client-supplied handle identifies a
         specific session. (Optional) */

  uint32_t frames_per_buf;
  /**<   Number of encoded frames that can be packed
                                 into each encoder buffer. Default: 1. */
}qmi_csd_as_open_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Opens an audio stream and returns the corresponding audio stream
           handle. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Open Structure */
  qmi_csd_as_open_t_v01 qmi_csd_as_open_payload;
}qmi_csd_open_audio_stream_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Opens an audio stream and returns the corresponding audio stream
           handle. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Open Status */
  uint8_t open_status_valid;  /**< Must be set to true if open_status is being passed */
  qmi_csd_status_v01 open_status;
  /**<   Open status.           */

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t qmi_csd_as_handle_valid;  /**< Must be set to true if qmi_csd_as_handle is being passed */
  uint32_t qmi_csd_as_handle;
  /**<   Unique handle for the audio stream.         */
}qmi_csd_open_audio_stream_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AC_OPEN_CATEGORY_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AC_CATEGORY_GENERIC_PLAYBACK_V01 = 1, /**<  Playback category.  */
  QMI_CSD_AC_CATEGORY_GENERIC_RECORDING_V01 = 2, /**<  Recording category.  */
  QMI_CSD_AC_CATEGORY_SYSTEM_SOUND_V01 = 3, /**<  System sound category.  */
  QMI_CSD_AC_CATEGORY_VOICE_RECOGNITION_V01 = 4, /**<  Voice recognition category.  */
  QMI_CSD_AC_OPEN_CATEGORY_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_ac_open_category_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AC_OPEN_MODE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AC_MODE_NON_LIVE_V01 = 0, /**<  Non-live mode sample buffered in the
								        AFE.  */
  QMI_CSD_AC_MODE_LIVE_V01 = 1, /**<  Live mode. The sample is not buffered
								        if the polling is not fast enough.  */
  QMI_CSD_AC_OPEN_MODE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_ac_open_mode_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AC_OPEN_SR_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AC_SR_8K_V01 = 8000, /**<  8 kHz. */
  QMI_CSD_AC_SR_16K_V01 = 16000, /**<  16 kHz.  */
  QMI_CSD_AC_SR_48K_V01 = 48000, /**<  48 kHz.  */
  QMI_CSD_AC_OPEN_SR_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_ac_open_sr_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  CSD audio context categories.

 CSD audio context mode.

 CSD audio context sampling rate.

 */
typedef struct {

  qmi_csd_ac_open_category_v01 ac_category;
  /**<   Audio context category. Supported values: \n
         - 1 -- Playback category \n
         - 2 -- Recording category \n
         - 3 -- System sound category \n
         - 4 -- Voice recognition category
    */

  qmi_csd_ac_open_mode_v01 ac_mode;
  /**<   Running mode for the audio context. Live mode drops the data buffer.
         Non-live mode blocks the data buffer when there is no output buffer.
         Supported values:\n
         - 0 -- Non-live mode sample is buffered in the AFE.\n
         - 1 -- Live mode. The sample is not buffered if the polling is
                not fast enough. */

  uint32_t dev_id;
  /**<   Device ID. Supported values are OEM-defined. */

  qmi_csd_ac_open_sr_v01 sample_rate;
  /**<   Sample rate for the audio context. Supported values: \n
         - QMI_CSD_AC_SR_8K (8000) -- 8000 samples per second \n
         - QMI_CSD_AC_SR_16K (16000) -- 16000 samples per second \n
         - QMI_CSD_AC_SR_48K (48000) -- 48000 samples per second
    */
}qmi_csd_ac_open_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Opens an audio context and returns the corresponding audio context
           handle. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Open Structure */
  qmi_csd_ac_open_t_v01 qmi_csd_ac_open_payload;
}qmi_csd_open_audio_context_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Opens an audio context and returns the corresponding audio context
           handle. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Open Status */
  uint8_t open_status_valid;  /**< Must be set to true if open_status is being passed */
  qmi_csd_status_v01 open_status;
  /**<   Open status.           */

  /* Optional */
  /*  Audio Context Handle */
  uint8_t qmi_csd_ac_handle_valid;  /**< Must be set to true if qmi_csd_ac_handle is being passed */
  uint32_t qmi_csd_ac_handle;
  /**<   Audio context handle.      */
}qmi_csd_open_audio_context_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_TS_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_TS_UNKNOWN_V01 = 0, /**<  Invalid timestamp  */
  QMI_CSD_AS_TS_ABSOLUTE_V01 = 1, /**<  Absolute timestamp  */
  QMI_CSD_AS_TS_RELATIVE_V01 = 2, /**<  Relative timestamp  */
  QMI_CSD_AS_TS_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_ts_type_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  CSD audio stream timestamp type.

 */
typedef struct {

  qmi_csd_as_ts_type_v01 ts_type;
  /**<   Type of timestamp: \n
         - 0 -- Invalid timestamp \n
         - 1 -- Absolute timestamp \n
         - 2 -- Relative timestamp
    */

  uint32_t ts_high;
  /**<   Upper 32 bits of the microsecond timestamp. */

  uint32_t ts_low;
  /**<   Lower 32 bits of the microsecond timestamp. */
}qmi_csd_as_ts_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Starts an audio stream session. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Mandatory */
  /*  Audio Stream Timestamp Structure */
  qmi_csd_as_ts_t_v01 qmi_csd_as_ts_payload;
}qmi_csd_as_cmd_start_session_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Starts an audio stream session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_start_session_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Stops an audio stream session. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_stop_session_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Stops an audio stream session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_stop_session_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Flushes an audio stream. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_flush_stream_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Flushes an audio stream. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_flush_stream_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Flushes the Tx path in the Read-Write stream. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_flush_stream_tx_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Flushes the Tx path in the Read-Write stream. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.          */
}qmi_csd_as_cmd_flush_stream_tx_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the volume step range. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_get_vol_levels_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the volume step range. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  Volume Step Range */
  uint8_t num_levels_valid;  /**< Must be set to true if num_levels is being passed */
  uint32_t num_levels;
  /**<   Range for the volume level steps. */
}qmi_csd_as_cmd_get_vol_levels_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Used in Audio/Video (AV) synchronization to get the current Digital
           Signal Processor (DSP) time in microseconds. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/
}qmi_csd_as_cmd_get_dsp_clk_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Used in Audio/Video (AV) synchronization to get the current Digital
           Signal Processor (DSP) time in microseconds. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  Current DSP Time in Microseconds */
  uint8_t qmi_csd_as_ts_payload_valid;  /**< Must be set to true if qmi_csd_as_ts_payload is being passed */
  qmi_csd_as_ts_t_v01 qmi_csd_as_ts_payload;
}qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the rendered Pulse Code Modulation (PCM) sample time based
           on the start time of the playback or flush point in microseconds. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_get_rendered_time_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the rendered Pulse Code Modulation (PCM) sample time based
           on the start time of the playback or flush point in microseconds. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  Rendered Time in Microseconds */
  uint8_t qmi_csd_as_ts_payload_valid;  /**< Must be set to true if qmi_csd_as_ts_payload is being passed */
  qmi_csd_as_ts_t_v01 qmi_csd_as_ts_payload;
}qmi_csd_as_cmd_get_rendered_time_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the session ID for an audio stream.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_get_session_id_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the session ID for an audio stream.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  Session ID */
  uint8_t qmi_csd_session_id_valid;  /**< Must be set to true if qmi_csd_session_id is being passed */
  uint32_t qmi_csd_session_id;
  /**<   Current session ID. */
}qmi_csd_as_cmd_get_session_id_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_SR_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_SR_8K_V01 = 8000, /**<  8 kHz.  */
  QMI_CSD_AS_FMT_SR_16K_V01 = 16000, /**<  16 kHz.  */
  QMI_CSD_AS_FMT_SR_48K_V01 = 48000, /**<  48 kHz.  */
  QMI_CSD_AS_FMT_SR_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_sr_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_BPS_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_BPS_8K_V01 = 8, /**<  8 bit per sample.  */
  QMI_CSD_AS_FMT_BPS_16K_V01 = 16, /**<  16 bit per sample.  */
  QMI_CSD_AS_FMT_BPS_24K_V01 = 24, /**<  24 bit per sample.  */
  QMI_CSD_AS_FMT_BPS_32K_V01 = 32, /**<  32 bit per sample.  */
  QMI_CSD_AS_FMT_BPS_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_bps_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_CH_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_CH_MONO_V01 = 1, /**<  Mono.  */
  QMI_CSD_AS_FMT_CH_STEREO_V01 = 2, /**<  Stereo.  */
  QMI_CSD_AS_FMT_CH_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_ch_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_PCM_SIGN_FLAG_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_PCM_UNSIGNED_V01 = 0, /**<  Unsigned.  */
  QMI_CSD_AS_FMT_PCM_SIGNED_V01 = 1, /**<  Signed.  */
  QMI_CSD_AS_FMT_PCM_SIGN_FLAG_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_pcm_sign_flag_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_PCM_INTERLEAVE_FLAG_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_PCM_NONINTERLEAVED_V01 = 0, /**<  Noninterleaved.  */
  QMI_CSD_AS_FMT_PCM_INTERLEAVED_V01 = 1, /**<  Interleaved.  */
  QMI_CSD_AS_FMT_PCM_INTERLEAVE_FLAG_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_pcm_interleave_flag_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Rx stream to PCM format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  PCM Stream Sample Rate */
  qmi_csd_as_fmt_sr_v01 sample_rate;
  /**<   Sample rate for the PCM stream. Supported values:\n
         - QMI_CSD_AS_FMT_SR_8K (8000) -- 8000 samples per second \n
         - QMI_CSD_AS_FMT_SR_16K (16000) -- 16000 samples per second \n
         - QMI_CSD_AS_FMT_SR_48K (48000) -- 48000 samples per second
    */

  /* Mandatory */
  /*  Channel Allocation */
  qmi_csd_as_fmt_ch_v01 channels;
  /**<   Channel allocation:\n
                                     - 1 -- Mono\n
                                     - 2 -- Stereo */

  /* Mandatory */
  /*  Bits Per Sample */
  uint16_t bit_per_sample;
  /**<   Bits per sample setup. */

  /* Mandatory */
  /*  PCM Sign Flag */
  qmi_csd_as_fmt_pcm_sign_flag_v01 sign_flag;
  /**<   Sign flag for the PCM sample:\n
                                     - 0 -- Unsigned\n
                                     - 1 -- Signed */

  /* Mandatory */
  /*  PCM Interleaved Flag */
  qmi_csd_as_fmt_pcm_interleave_flag_v01 interleave_flag;
  /**<   Interleaved flag for the PCM sample:\n
                                     - 0 -- Noninterleaved \n
                                     - 1 -- Interleaved */
}qmi_csd_as_cmd_set_stream_fmt_rx_pcm_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Rx stream to PCM format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Rx stream to Adaptive Differential Pulse Code
           Modulation (ADPCM) or raw Yamaha 4-bit ADPCM (YADPCM) format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Mandatory */
  /*  Channel Configuration */
  qmi_csd_as_fmt_ch_v01 channels;
  /**<   Channel configuration:\n
                                         - 1 -- Mono\n
                                         - 2 -- Stereo */

  /* Mandatory */
  /*  Bits Per Sample */
  qmi_csd_as_fmt_bps_v01 bit_per_sample;
  /**<   Bits per sample setup. Supported values:\n
         - QMI_CSD_AS_FMT_BPS_8K (8) -- 8 bit per sample \n
         - QMI_CSD_AS_FMT_BPS_16K (16) -- 16 bit per sample \n
         - QMI_CSD_AS_FMT_BPS_24K (24) -- 24 bit per sample \n
         - QMI_CSD_AS_FMT_BPS_32K (32) -- 32 bit per sample
    */

  /* Mandatory */
  /*  PCM Stream Sample Rate */
  qmi_csd_as_fmt_sr_v01 sample_rate;
  /**<   Sample rate for the PCM stream. Supported values:\n
         - QMI_CSD_AS_FMT_SR_8K (8000) -- 8000 samples per second \n
         - QMI_CSD_AS_FMT_SR_16K (16000) -- 16000 samples per second \n
         - QMI_CSD_AS_FMT_SR_48K (48000) -- 48000 samples per second
    */

  /* Optional */
  /*  ADPCM Block Size */
  uint8_t nBlockSize_valid;  /**< Must be set to true if nBlockSize is being passed */
  uint32_t nBlockSize;
  /**<   Block size for the ADPCM.
                                         Not used by the YADPCM.   */
}qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Rx stream to Adaptive Differential Pulse Code
           Modulation (ADPCM) or raw Yamaha 4-bit ADPCM (YADPCM) format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_MIDI_MODE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_MIDI_MODE_0_V01 = 0, /**<  All file formats not included in MIDI mode 1 and mode 2:
         MIDI, SMAF, and PMD.  */
  QMI_CSD_AS_MIDI_MODE_1_V01 = 1, /**<  MA2 or MA3 synthetic music mobile application format type.  */
  QMI_CSD_AS_MIDI_MODE_2_V01 = 2, /**<  MA5 synthetic music mobile application format type.  */
  QMI_CSD_AS_MIDI_MODE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_midi_mode_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Rx stream to Musical Instrument Digital
           Interface (MIDI) format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  MIDI Mode */
  qmi_csd_as_midi_mode_t_v01 mode;
  /**<   MIDI mode. Supported values:\n
         - QMI_CSD_AS_MIDI_MODE_0 (0) -- All file formats not included in MIDI
           mode 1 and mode 2: MIDI, SMAF, and PMD.\n
         - QMI_CSD_AS_MIDI_MODE_1 (1) -- MA2 or MA3 synthetic music mobile
           application format type.\n
         - QMI_CSD_AS_MIDI_MODE_2 (2) -- MA5 synthetic music mobile application
           format type.
     */
}qmi_csd_as_cmd_set_stream_fmt_rx_midi_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Rx stream to Musical Instrument Digital
           Interface (MIDI) format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_WMA_TAG_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_WMA_TAG_STANDARD_V01 = 0x161, /**<  Standard.  */
  QMI_CSD_AS_WMA_TAG_PROFESSIONAL_V01 = 0x162, /**<  Professional.  */
  QMI_CSD_AS_WMA_TAG_LOSELESS_V01 = 0x163,
  QMI_CSD_AS_WMA_TAG_LOSSLESS_V01 = 0x163, /**<  Lossless.  */
  QMI_CSD_AS_WMA_TAG_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_wma_tag_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  CSD audio stream Windows Media Audio tag.

 */
typedef struct {

  qmi_csd_as_wma_tag_t_v01 tag;
  /**<   Windows Media Audio 9 tag field. Specifies the unique ID of the codec
         used to encode the audio data. Supported values:\n
         - QMI_CSD_AS_WMA_TAG_STANDARD (0x161) -- Standard \n
         - QMI_CSD_AS_WMA_TAG_ PROFESSIONAL (0x162) -- Professional \n
         - QMI_CSD_AS_WMA_TAG_LOSSLESS (0x163) -- Lossless
    */

  qmi_csd_as_fmt_ch_v01 channels;
  /**<   Channel allocation:\n
                                     - 1 -- Mono\n
                                     - 2 -- Stereo */

  qmi_csd_as_fmt_sr_v01 sample_rate;
  /**<   Sample rate for the Windows Media Audio stream. Supported values: \n
         - QMI_CSD_AS_FMT_SR_8K (8000) -- 8000 samples per second \n
         - QMI_CSD_AS_FMT_SR_16K (16000) -- 16000 samples per second \n
         - QMI_CSD_AS_FMT_SR_48K (48000) -- 48000 samples per second
    */

  uint32_t byte_per_second;
  /**<   Average compressed stream rate in bytes
                                     per second. */

  uint16_t block_align;
  /**<   Alignment for the stream. */

  uint16_t valid_bit_per_sample;
  /**<   Valid bit width per sample. */

  uint32_t channel_mask;
  /**<   Channel mask. */

  uint16_t encode_opt;
  /**<   Encoding option as per Windows Media Audio 9. */

  uint32_t drc_peak_ref;
  /**<   Peak reference for dynamic range compression. */

  uint32_t drc_peak_target;
  /**<   Peak target for dynamic range compression. */

  uint32_t drc_average_ref;
  /**<   Average reference for dynamic range compression. */

  uint32_t drc_average_target;
  /**<   Average target for dynamic range compression. */

  uint16_t version_num;
  /**<   Version. */

  uint16_t virtual_pkt_len;
  /**<   Virtual packet length. */
}qmi_csd_as_stream_fmt_rx_wmav9_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Rx stream to Windows
           Media@latexonly\textsuperscript{\textregistered} @endlatexonly
           Audio 9 format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Windows Media Audio 9 Audio Stream Format */
  qmi_csd_as_stream_fmt_rx_wmav9_v01 qmi_csd_as_stream_fmt_rx_wmav9_payload;
}qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Rx stream to Windows
           Media@latexonly\textsuperscript{\textregistered} @endlatexonly
           Audio 9 format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
typedef struct {

  qmi_csd_as_wma_tag_t_v01 tag;
  /**<   Windows Media Audio 10 Pro tag field. Specifies the unique ID of the
         codec used to encode the audio data. Supported values: \n
         - QMI_CSD_AS_WMA_TAG_STANDARD (0x161) -- Standard \n
         - QMI_CSD_AS_WMA_TAG_PROFESSIONAL (0x162) -- Professional \n
         - QMI_CSD_AS_WMA_TAG_LOSSLESS (0x163) -- Lossless
    */

  qmi_csd_as_fmt_ch_v01 channels;
  /**<   Channel allocation. Supported values: \n
                                      - 1 -- Mono\n
                                      - 2 -- Stereo */

  qmi_csd_as_fmt_sr_v01 sample_rate;
  /**<   Sample rate for the Windows Media Audio stream. Supported values:\n
         - QMI_CSD_AS_FMT_SR_8K (8000) -- 8000 samples per second \n
         - QMI_CSD_AS_FMT_SR_16K (16000) -- 16000 samples per second \n
         - QMI_CSD_AS_FMT_SR_48K (48000) -- 48000 samples per second
    */

  uint32_t byte_per_second;
  /**<   Average compressed stream rate in
                                          bytes per second. */

  uint16_t block_align;
  /**<   Block alignment for the stream. */

  uint16_t valid_bit_per_sample;
  /**<   Valid bit width per sample. */

  uint32_t channel_mask;
  /**<   Channel mask. */

  uint16_t encode_opt;
  /**<   Encoding option per Windows Media Audio 10 Pro. */

  uint16_t adv_encode_opt;
  /**<   Advanced encode option per Windows Media Audio 10 Pro. */

  uint32_t adv_encode_opt2;
  /**<   Advanced encode option2 per Windows Media Audio 10 Pro. */

  uint32_t drc_peak_ref;
  /**<   Peak reference for dynamic range compression. */

  uint32_t drc_peak_target;
  /**<   Peak target for dynamic range compression. */

  uint32_t drc_average_ref;
  /**<   Average reference for dynamic range compression. */

  uint32_t drc_average_target;
  /**<   Average target for dynamic range compression. */

  uint16_t version_num;
  /**<   Version number. */

  uint16_t virtual_pkt_len;
  /**<   Virtual packet length. */
}qmi_csd_as_stream_fmt_rx_wmav10_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Rx stream to Windows Media Audio
           10 Pro format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Windows Media Audio 10 Pro Audio Stream Format */
  qmi_csd_as_stream_fmt_rx_wmav10_v01 qmi_csd_as_stream_fmt_rx_wmav10_payload;
}qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Rx stream to Windows Media Audio
           10 Pro format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_AAC_FMT_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_AAC_ADTS_V01 = 0, /**<  Audio data transport stream advanced audio codec format.  */
  QMI_CSD_AS_FMT_AAC_LOAS_V01 = 1, /**<  Low overhead audio stream advanced audio codec format.    */
  QMI_CSD_AS_FMT_AAC_ADIF_V01 = 2, /**<  Audio data interchange advanced audio codec format.  */
  QMI_CSD_AS_FMT_AAC_RAW_V01 = 3, /**<  Raw advanced audio codec format.  */
  QMI_CSD_AS_AAC_FMT_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_aac_fmt_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_AAC_AOT_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_AAC_OT_LC_V01 = 2, /**<  Lossy Compression (LC) object type.  */
  QMI_CSD_AS_FMT_AAC_OT_SBR_V01 = 5, /**<  Spectral Band Replication (SBR) object type.  */
  QMI_CSD_AS_FMT_AAC_OT_BSAC_V01 = 22, /**<  Bit-Sliced Arithmetic Coding (BSAC) object type.  */
  QMI_CSD_AS_FMT_AAC_OT_PS_V01 = 29, /**<  Parametric Stereo (PS) object type.  */
  QMI_CSD_AS_AAC_AOT_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_aac_aot_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  CSD Audio Stream AAC Format

 CSD Audio Stream AAC Object Type

 */
typedef struct {

  qmi_csd_as_fmt_sr_v01 sample_rate;
  /**<   Sample rate for the AAC stream. Supported values:\n
           - QMI_CSD_AS_FMT_SR_8K (8000) -- 8000 samples per second \n
           - QMI_CSD_AS_FMT_SR_16K (16000) -- 16000 samples per second \n
           - QMI_CSD_AS_FMT_SR_48K (48000) -- 48000 samples per second
      */

  qmi_csd_as_fmt_ch_v01 channels;
  /**<   Channel configuration:\n
           - 1 -- Mono\n
           - 2 -- Stereo */

  qmi_csd_as_aac_fmt_t_v01 format;
  /**<   AAC format type:\n
           - 0 -- Audio Data Transport Stream (ADTS)\n
           - 1 -- Low Overhead Audio Stream (LOAS)\n
           - 2 -- Audio Data Interchange Format (ADIF)\n
           - 3 -- Raw */

  qmi_csd_as_aac_aot_t_v01 aud_obj_type;
  /**<   Audio object type. Supported values: \n
           - 2 -- Lossy Compression (LC) object type \n
           - 5 -- Spectral Band Replication (SBR) object type \n
           - 22 -- Bit-Sliced Arithmetic Coding (BSAC) object type \n
           - 29 -- Parametric stereo AAC object type */

  uint16_t ep_cfg;
  /**<   Indicates the configuration of the error protection scheme
           (0, 1, 2, 3). This information is retrieved from the MP4 header
           and is required by the DSP only when the value of Ahead Of Time
           (AOT) is 17. Currently, only epConfig=0 is supported. */

  uint8_t section_DRF;
  /**<   Indicates whether the Virtual CodeBook (VCB11) error resilience tool
           is used:\n
           - 1 -- VCB11 is used \n
           - 0 -- VCB11 is not used \n
           This information is retrieved from the MP4 header.\n
           Note: This field must be zero if (AOT!=17). */

  uint8_t scale_factor_DRF;
  /**<   Indicates whether the Reversible Variable Length Coding (RVLC)
           error resilience tool is used:\n
           - 1 -- RVLC is used \n
           - 0 -- RVLC is not used \n
           This information is retrieved from the MP4 header.\n
           Note: This field must be zero if (AOT!=17). */

  uint8_t spectral_DRF;
  /**<   Indicates whether the Huffman Codeword Reordering (HCR) error
           resilience tool is used:\n
           - 1 -- HCR is used \n
           - 0 -- HCR is not used \n
           This information is retrieved from the MP4 header.\n
           Note: This field must be zero if (AOT!=17). */

  uint8_t sbr_on_flag;
  /**<   Enables/disables spectral band replication:\n
           - 1 -- Turns on SBR if present in the bitstream \n
           - 0 -- Turns off SBR */

  uint8_t sbr_ps_flag;
  /**<   Enables/disables the parametric stereo AAC flag.\n
           - 1 -- Turns on PS if present in the bitstream \n
           - 0 -- Turns off PS */

  uint32_t bit_rate;
  /**<   Bitrate.  */
}qmi_csd_as_stream_fmt_rx_aac_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Rx stream to AAC format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  AAC Audio Stream Format Payload  */
  qmi_csd_as_stream_fmt_rx_aac_v01 qmi_csd_as_stream_fmt_rx_aac_payload;
}qmi_csd_as_cmd_set_stream_fmt_rx_aac_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Rx stream to AAC format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Rx stream to G.711 format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  G.711 Stream Sample Rate */
  qmi_csd_as_fmt_sr_v01 sample_rate;
  /**<   Sample rate for the G.711 stream. Supported value:\n
         - QMI_CSD_AS_FMT_SR_8K (8000) -- 8000 samples per second
    */
}qmi_csd_as_cmd_set_stream_fmt_rx_g711_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Rx stream to G.711 format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
typedef struct {

  uint8_t strm_info_present;
  /**<   Specifies whether METADAT_BLOCK_STREAMINFO is parsed
        successfully. When bStrmInfoPresent is set to:\n
        - 1 -- METADAT_BLOCK_STREAMINFO is successfully parsed.\n
        - 0 -- FLAC decoder tries to get the stream information from the
        frame header. */

  uint16_t min_blk_size;
  /**<   Minimum block size in samples used in the stream.  */

  uint16_t max_blk_size;
  /**<   Maximum block size in samples used in the stream. When
        minBlkSize == maxBlkSize, a fixed block size stream is implied.  */

  uint16_t channels;
  /**<   Number of channels. FLAC supports 1 to 8 channels. */

  uint16_t sample_size;
  /**<   Bits per sample. FLAC supports 4 to 32 bits per sample. */

  uint32_t sample_rate;
  /**<   Sample rate for FLAC. */

  uint32_t min_frame_size;
  /**<   Minimum frame size in bytes used in the stream. A value of zero means
        the value is not known.  */

  uint32_t max_frame_size;
  /**<   Maximum frame size in bytes used in the stream. A value of zero means
        the value is not known.     */

  uint16_t md5_sum[8];
  /**<   MD5 Message-Digest Algorithm signature of the unencoded audio data.
        This allows the decoder to determine whether an error exists in the
        audio data even when the error does not result in an invalid
        bitstream. */
}qmi_csd_as_stream_fmt_rx_flac_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Rx stream to Free Lossless Audio Codec (FLAC)
           format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  FLAC Audio Stream Format Payload */
  qmi_csd_as_stream_fmt_rx_flac_v01 qmi_csd_as_stream_fmt_rx_flac_payload;
}qmi_csd_as_cmd_set_stream_fmt_rx_flac_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Rx stream to Free Lossless Audio Codec (FLAC)
           format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_VORBIS_SR_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_VORBIS_SR_48000_V01 = 48000,
  QMI_CSD_AS_VORBIS_SR_44100_V01 = 44100,
  QMI_CSD_AS_VORBIS_SR_32000_V01 = 32000,
  QMI_CSD_AS_VORBIS_SR_24000_V01 = 24000,
  QMI_CSD_AS_VORBIS_SR_22050_V01 = 22050,
  QMI_CSD_AS_VORBIS_SR_16000_V01 = 16000,
  QMI_CSD_AS_VORBIS_SR_12000_V01 = 12000,
  QMI_CSD_AS_VORBIS_SR_11025_V01 = 11025,
  QMI_CSD_AS_VORBIS_SR_8000_V01 = 8000,
  QMI_CSD_AS_VORBIS_SR_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_vorbis_sr_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_VORBIS_BS_FMT_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_VORBIS_BS_FMT_RAW_V01 = 0,
  QMI_CSD_AS_VORBIS_BS_FMT_TRANSCODE_V01 = 1,
  QMI_CSD_AS_VORBIS_BS_FMT_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_vorbis_bs_fmt_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Rx stream to Vorbis format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Vorbis Sample Rate */
  qmi_csd_as_vorbis_sr_t_v01 sample_rate;
  /**<   Sample rate for the Vorbis stream. Supported values:\n
        - QMI_CSD_AS_VORBIS_SR_48000 (48000) -- 48000 samples per second \n
        - QMI_CSD_AS_VORBIS_SR_44100 (44100) -- 44100 samples per second \n
        - QMI_CSD_AS_VORBIS_SR_32000 (32000) -- 32000 samples per second \n
        - QMI_CSD_AS_VORBIS_SR_24000 (24000) -- 24000 samples per second \n
        - QMI_CSD_AS_VORBIS_SR_22050 (22050) -- 22050 samples per second \n
        - QMI_CSD_AS_VORBIS_SR_16000 (16000) -- 16000 samples per second \n
        - QMI_CSD_AS_VORBIS_SR_12000 (12000) -- 12000 samples per second \n
        - QMI_CSD_AS_VORBIS_SR_11025 (11025) -- 11025 samples per second \n
        - QMI_CSD_AS_VORBIS_SR_8000 (8000) -- 8000 samples per second
   */

  /* Mandatory */
  /*  Vorbis Stream Channels */
  qmi_csd_as_fmt_ch_v01 channels;
  /**<   Number of channels encoded in the Vorbis stream.
      Supported values:\n
      - 1 -- Mono\n
      - 2 -- Stereo
    */

  /* Mandatory */
  /*  Encoded Data Nominal Bitrate */
  uint32_t bit_rate;
  /**<   Nominal bitrate of the encoded data.
    */

  /* Mandatory */
  /*  Encoded Data Minimum Bitrate */
  uint32_t min_bit_rate;
  /**<   Minimim bitrate of the encoded data.
    */

  /* Mandatory */
  /*  Encoded Data Maximum Bitrate */
  uint32_t max_bit_rate;
  /**<   Maximum bitrate of the encoded data.
    */

  /* Mandatory */
  /*  PCM Width Resolution */
  qmi_csd_as_fmt_bps_v01 bits_per_sample;
  /**<   PCM width resolution to be played by the decoder. */

  /* Mandatory */
  /*  Bit Stream Format  */
  qmi_csd_as_vorbis_bs_fmt_t_v01 bit_stream_fmt;
  /**<   Bit stream format:\n
           - 0 -- Raw bitstream (default)\n
           - 1 -- Transcoded bitstream */
}qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Rx stream to Vorbis format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_AMRWBPLUS_ISF_INDEX_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_0_V01 = 0, /**<  AMR-WB+ internal sampling frequency is N/A.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_1_V01 = 1, /**<  AMR-WB+ internal sampling frequency is 12800 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_2_V01 = 2, /**<  AMR-WB+ internal sampling frequency is 14400 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_3_V01 = 3, /**<  AMR-WB+ internal sampling frequency is 16000 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_4_V01 = 4, /**<  AMR-WB+ internal sampling frequency is 17067 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_5_V01 = 5, /**<  AMR-WB+ internal sampling frequency is 19200 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_6_V01 = 6, /**<  AMR-WB+ internal sampling frequency is 21333 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_7_V01 = 7, /**<  AMR-WB+ internal sampling frequency is 24000 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_8_V01 = 8, /**<  AMR-WB+ internal sampling frequency is 25600 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_9_V01 = 9, /**<  AMR-WB+ internal sampling frequency is 28800 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_10_V01 = 10, /**<  AMR-WB+ internal sampling frequency is 32000 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_11_V01 = 11, /**<  AMR-WB+ internal sampling frequency is 34133 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_12_V01 = 12, /**<  AMR-WB+ internal sampling frequency is 36000 Hz.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_ISF_13_V01 = 13, /**<  AMR-WB+ internal sampling frequency is 38400 Hz.              */
  QMI_CSD_AS_FMT_AMRWBPLUS_ISF_INDEX_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_amrwbplus_isf_index_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_AMR_FRAME_FORMAT_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_AMR_FF_CONFORMANCE_V01 = 0, /**<  AMR conformance (standard) format.  */
  QMI_CSD_AS_FMT_AMR_FF_IF1_V01 = 1, /**<  AMR interface format. 1 */
  QMI_CSD_AS_FMT_AMR_FF_IF2_V01 = 2, /**<  AMR interface format 2. */
  QMI_CSD_AS_FMT_AMR_FF_FSF_V01 = 3, /**<  AMR file storage format.  */
  QMI_CSD_AS_FMT_AMR_FF_RTP_V01 = 4, /**<  Real-Time Transport Protocol payload.  */
  QMI_CSD_AS_FMT_AMR_FF_ITU_V01 = 5, /**<  ITU format.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_FF_TIF_V01 = 6, /**<  AMR-WB+ transport interface format.  */
  QMI_CSD_AS_FMT_AMR_WB_PLUS_FF_FSF_V01 = 7, /**<  AMR-WB+ file storage format.  */
  QMI_CSD_AS_FMT_AMR_FRAME_FORMAT_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_amr_frame_format_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_AMRWB_BAND_MODE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_AMR_BM_WB0_V01 = 0, /**<  AMRWB mode 0 is 6600 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB1_V01 = 1, /**<  AMRWB mode 1 is 8850 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB2_V01 = 2, /**<  AMRWB mode 2 is 12650 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB3_V01 = 3, /**<  AMRWB mode 3 is 14250 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB4_V01 = 4, /**<  AMRWB mode 4 is 15850 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB5_V01 = 5, /**<  AMRWB mode 5 is 18250 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB6_V01 = 6, /**<  AMRWB mode 6 is 19850 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB7_V01 = 7, /**<  AMRWB mode 7 is 23050 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB8_V01 = 8, /**<  AMRWB mode 8 is 23850 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB9_V01 = 9, /**<  AMRWB mode 9 is the silence indicator.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS10_V01 = 10, /**<  AMR-WB+ mode 10 is 13600 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS11_V01 = 11, /**<  AMR-WB+ mode 11 is 18000 bps stereo.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS12_V01 = 12, /**<  AMR-WB+ mode 12 is 24000 bps.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS13_V01 = 13, /**<  AMR-WB+ mode 13 is 24000 bps stereo.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS14_V01 = 14, /**<  AMR-WB+ mode 14 is frame erasure.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS15_V01 = 15, /**<  AMR-WB+ mode 15 is no data.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS16_V01 = 16, /**<  AMR-WB+ mode 16.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS17_V01 = 17, /**<  AMR-WB+ mode 17.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS18_V01 = 18, /**<  AMR-WB+ mode 18.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS19_V01 = 19, /**<  AMR-WB+ mode 19.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS20_V01 = 20, /**<  AMR-WB+ mode 20.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS21_V01 = 21, /**<  AMR-WB+ mode 21.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS22_V01 = 22, /**<  AMR-WB+ mode 22.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS23_V01 = 23, /**<  AMR-WB+ mode 23.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS24_V01 = 24, /**<  AMR-WB+ mode 24.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS25_V01 = 25, /**<  AMR-WB+ mode 25.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS26_V01 = 26, /**<  AMR-WB+ mode 26.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS27_V01 = 27, /**<  AMR-WB+ mode 27.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS28_V01 = 28, /**<  AMR-WB+ mode 28.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS29_V01 = 29, /**<  AMR-WB+ mode 29.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS30_V01 = 30, /**<  AMR-WB+ mode 30.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS31_V01 = 31, /**<  AMR-WB+ mode 31.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS32_V01 = 32, /**<  AMR-WB+ mode 32.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS33_V01 = 33, /**<  AMR-WB+ mode 33.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS34_V01 = 34, /**<  AMR-WB+ mode 34.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS35_V01 = 35, /**<  AMR-WB+ mode 35.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS36_V01 = 36, /**<  AMR-WB+ mode 36.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS37_V01 = 37, /**<  AMR-WB+ mode 37.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS38_V01 = 38, /**<  AMR-WB+ mode 38.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS39_V01 = 39, /**<  AMR-WB+ mode 39.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS40_V01 = 40, /**<  AMR-WB+ mode 40.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS41_V01 = 41, /**<  AMR-WB+ mode 41.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS42_V01 = 42, /**<  AMR-WB+ mode 42.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS43_V01 = 43, /**<  AMR-WB+ mode 43.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS44_V01 = 44, /**<  AMR-WB+ mode 44.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS45_V01 = 45, /**<  AMR-WB+ mode 45.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS46_V01 = 46, /**<  AMR-WB+ mode 46.  */
  QMI_CSD_AS_FMT_AMR_BM_WB_PLUS47_V01 = 47, /**<  AMR-WB+ mode 47.  */
  QMI_CSD_AS_FMT_AMRWB_BAND_MODE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_amrwb_band_mode_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_AMRWB_DTX_MODE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_AMR_DTX_MODE0_V01 = 0, /**<  AMR DTX mode value.  */
  QMI_CSD_AS_FMT_AMRWB_DTX_MODE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_amrwb_dtx_mode_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Rx stream to Extended Adaptive Multirate
           Wideband (AMR-WB+) format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Number of Channels */
  qmi_csd_as_fmt_ch_v01 channels;
  /**<   Number of channels. Supported values:\n
         - 1 -- Mono\n
         - 2 -- Stereo */

  /* Mandatory */
  /*  AMR Band Mode */
  qmi_csd_as_fmt_amrwb_band_mode_t_v01 amr_band_mode;
  /**<   AMR band mode value. Supported values: 0 to 47. See Appendix
         \ref{app:CSDasAMRWBPLUSbandModes}. */

  /* Mandatory */
  /*  AMR DTX Mode */
  qmi_csd_as_fmt_amrwb_dtx_mode_t_v01 amr_dtx_mode;
  /**<   AMR DTX mode value. Currently only 0 is supported. */

  /* Mandatory */
  /*  AMR Frame Format */
  qmi_csd_as_fmt_amr_frame_format_t_v01 amr_frame_fmt;
  /**<   AMR frame format value. See Appendix \ref{app:CSDasAMRframeFormats}. */

  /* Mandatory */
  /*  AMR Line Spectral Frequency */
  qmi_csd_as_fmt_amrwbplus_isf_index_t_v01 amr_lsf_idx;
  /**<   AMR Line Spectral Frequency (LSF) index value.
         Supported values: 0 to 13.
         See Appendix \ref{app:CSDasAMRWBPLUSisfIndex}. */
}qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Rx stream to Extended Adaptive Multirate
           Wideband (AMR-WB+) format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Tx stream to PCM format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Mandatory */
  /*  Real Sample Rate */
  uint32_t sample_rate;
  /**<   Real sample rate. */

  /* Mandatory */
  /*  PCM Channel Configuration */
  qmi_csd_as_fmt_ch_v01 channels;
  /**<   Channel configuration for the PCM:\n
                                     - 1 -- Mono\n
                                     - 2 -- Stereo */

  /* Mandatory */
  /*  Real Bit Per Sample Number */
  uint16_t bit_per_sample;
  /**<   Real bit per sample number. */

  /* Mandatory */
  /*  PCM Sign Flag */
  qmi_csd_as_fmt_pcm_sign_flag_v01 sign_flag;
  /**<   Sign flag for the PCM sample:\n
                                     - 0 -- Unsigned\n
                                     - 1 -- Signed */

  /* Mandatory */
  /*  PCM Interleave Flag */
  qmi_csd_as_fmt_pcm_interleave_flag_v01 interleave_flag;
  /**<   Interleave flag for the PCM sample:\n
                                     - 0 -- Noninterleaved\n
                                     - 1 -- Interleaved */
}qmi_csd_as_cmd_set_stream_fmt_tx_pcm_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Tx stream to PCM format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Tx stream to Advanced Audio Codec (AAC) format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  AAC Stream Sample Rate */
  uint32_t sample_rate;
  /**<   Sample rate for the AAC stream. */

  /* Mandatory */
  /*  AAC Channel Configuration */
  qmi_csd_as_fmt_ch_v01 channels;
  /**<   Channel configuration for the AAC:\n
                                     - 1 -- Mono\n
                                     - 2 -- Stereo */

  /* Mandatory */
  /*  AAC Format */
  qmi_csd_as_aac_fmt_t_v01 format;
  /**<   AAC format. Supported values: \n
           - 0 -- Audio data transport stream AAC format \n
           - 1 -- Low overhead audio stream AAC format \n
           - 2 -- Audio data interchange AAC format \n
           - 3 -- Raw AAC format */

  /* Mandatory */
  /*  AAC Stream Bitrate */
  uint32_t bit_rate;
  /**<   Bitrate of the AAC stream.  */

  /* Mandatory */
  /*  AAC Encode Mode */
  qmi_csd_as_aac_aot_t_v01 encoder_mode;
  /**<   AAC encoder mode. Supported values: \n
           - 2 -- Lossy compression object type \n
           - 5 -- Spectral band replication object type \n
           - 22 -- Bit-sliced arithmetic coding object type \n
           - 29 -- Parametric stereo AAC object type */
}qmi_csd_as_cmd_set_stream_fmt_tx_aac_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Tx stream to Advanced Audio Codec (AAC) format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Tx stream to G.711 format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  G.711 Sample Rate */
  qmi_csd_as_fmt_sr_v01 sample_rate;
  /**<   Sample rate for G.711. Supported values:\n
         - QMI_CSD_AS_FMT_SR_8K (8000) -- 8000 samples per second \n
         - QMI_CSD_AS_FMT_SR_16K (16000) -- 16000 samples per second \n
         - QMI_CSD_AS_FMT_SR_48K (48000) -- 48000 samples per second
     */
}qmi_csd_as_cmd_set_stream_fmt_tx_g711_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Tx stream to G.711 format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_AMRNB_MODE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_AMR_MODE_NB_0_V01 = 0, /**<  AMR-NB mode 0 is 4750 bps.  */
  QMI_CSD_AS_AMR_MODE_NB_1_V01 = 1, /**<  AMR-NB mode 1 is 5150 bps.  */
  QMI_CSD_AS_AMR_MODE_NB_2_V01 = 2, /**<  AMR-NB mode 2 is 5900 bps.  */
  QMI_CSD_AS_AMR_MODE_NB_3_V01 = 3, /**<  AMR-NB mode 3 is 6700 bps.  */
  QMI_CSD_AS_AMR_MODE_NB_4_V01 = 4, /**<  AMR-NB mode 4 is 7400 bps.  */
  QMI_CSD_AS_AMR_MODE_NB_5_V01 = 5, /**<  AMR-NB mode 5 is 950 bps.  */
  QMI_CSD_AS_AMR_MODE_NB_6_V01 = 6, /**<  AMR-NB mode 6 is 10200 bps.  */
  QMI_CSD_AS_AMR_MODE_NB_7_V01 = 7, /**<  AMR-NB mode 7 is 12200 bps.             */
  QMI_CSD_AS_FMT_AMRNB_MODE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_amrnb_mode_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_AMR_DTX_MODE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_AMR_DTX_OFF_V01 = 0, /**<  Discontinuous transmission is disabled.  */
  QMI_CSD_AS_AMR_DTX_VAD1_V01 = 1, /**<  Voice activity detector 1 is enabled.  */
  QMI_CSD_AS_AMR_DTX_VAD2_V01 = 2, /**<  Voice activity detector 2 is enabled.  */
  QMI_CSD_AS_AMR_DTX_AUTO_V01 = 3, /**<  Codec selects automatically.  */
  QMI_CSD_AS_AMR_DTX_EFR_V01 = 4, /**<  Discontinuous transmission as enhanced
                                        full rate instead of adaptive multirate
                                        codec standard.           */
  QMI_CSD_AS_FMT_AMR_DTX_MODE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_amr_dtx_mode_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Tx stream to AMR-NB format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Audio Stream Encode Mode */
  qmi_csd_as_fmt_amrnb_mode_t_v01 encoder_mode;
  /**<   Encoder mode for the audio stream. Supported values for AMR_NB: \n
           - 0 -- 4750 bps\n
           - 1 -- 5150 bps\n
           - 2 -- 5900 bps\n
           - 3 -- 6700 bps\n
           - 4 -- 7400 bps\n
           - 5 -- 7950 bps\n
           - 6 -- 10200 bps\n
           - 7 -- 12220 bps*/

  /* Mandatory */
  /*  Audio Stream DTX Mode */
  qmi_csd_as_fmt_amr_dtx_mode_t_v01 dtx_mode;
  /**<   Discontinuous Transmission mode. Supported values:\n
           - 0 -- Disables DTX \n
           - 1 -- Enables voice activity detector 1 \n
           - 2 -- Enables voice activity detector 2 \n
           - 3 -- Codec selects automatically \n
           - 4 -- DTX uses EFR instead of AMR codec standard */
}qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Tx stream to AMR-NB format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_AMRWB_MODE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_AMR_MODE_WB_0_V01 = 0, /**<  AMR-WB mode 0 is 6600 bps  */
  QMI_CSD_AS_AMR_MODE_WB_1_V01 = 1, /**<  AMR-WB mode 1 is 8850 bps  */
  QMI_CSD_AS_AMR_MODE_WB_2_V01 = 2, /**<  AMR-WB mode 2 is 12650 bps  */
  QMI_CSD_AS_AMR_MODE_WB_3_V01 = 3, /**<  AMR-WB mode 3 is 14250 bps  */
  QMI_CSD_AS_AMR_MODE_WB_4_V01 = 4, /**<  AMR-WB mode 4 is 15850 bps  */
  QMI_CSD_AS_AMR_MODE_WB_5_V01 = 5, /**<  AMR-WB mode 5 is 18250 bps  */
  QMI_CSD_AS_AMR_MODE_WB_6_V01 = 6, /**<  AMR-WB mode 6 is 19850 bps  */
  QMI_CSD_AS_AMR_MODE_WB_7_V01 = 7, /**<  AMR-WB mode 7 is 23050 bps  */
  QMI_CSD_AS_AMR_MODE_WB_8_V01 = 8, /**<  AMR-WB mode 8 is 23850 bps          */
  QMI_CSD_AS_FMT_AMRWB_MODE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_amrwb_mode_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Tx stream to AMR-WB format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Audio Stream Encode Mode */
  qmi_csd_as_fmt_amrwb_mode_t_v01 encoder_mode;
  /**<   Encoder mode for the audio stream. Supported values for AMR_WB: \n
           - 0 -- 6600 bps\n
           - 1 -- 8850 bps\n
           - 2 -- 12650 bps\n
           - 3 -- 14250 bps\n
           - 4 -- 15850 bps\n
           - 5 -- 18250 bps\n
           - 6 -- 19850 bps\n
           - 7 -- 23050 bps\n
           - 8 -- 23850 bps */

  /* Mandatory */
  /*  Audio Stream DTX Mode */
  qmi_csd_as_fmt_amr_dtx_mode_t_v01 dtx_mode;
  /**<   Discontinuous Transmission mode. Supported values:\n
           - 0 -- Disables DTX \n
           - 1 -- Enables voice activity detector 1 \n
           - 2 -- Enables voice activity detector 2 \n
           - 3 -- Codec selects automatically \n
           - 4 -- DTX uses EFR instead of the AMR codec standard */
}qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Tx stream to AMR-WB format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_QCELP13K_BR_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_QCELP_BR_0_V01 = 0, /**<  14.4 kbps.  */
  QMI_CSD_AS_FMT_QCELP_BR_1_V01 = 1, /**<  12.2 kbps.  */
  QMI_CSD_AS_FMT_QCELP_BR_2_V01 = 2, /**<  11.2 kbps.  */
  QMI_CSD_AS_FMT_QCELP_BR_3_V01 = 3, /**<  9 kbps.  */
  QMI_CSD_AS_FMT_QCELP_BR_4_V01 = 4, /**<  7.2 kbps.            */
  QMI_CSD_AS_FMT_QCELP13K_BR_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_qcelp13k_br_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_ENC_RATE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  ENC_EITHTH_RATE_V01 = 1, /**<  Eighth rate  */
  ENC_QUARTER_RATE_V01 = 2, /**<  Quarter rate  */
  ENC_HALF_RATE_V01 = 3, /**<  Half rate  */
  ENC_FULL_RATE_V01 = 4, /**<  Full rate  */
  QMI_CSD_ENC_RATE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_enc_rate_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Tx stream to QCELP13K format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Minimum CDMA Encoded Frame Rate */
  qmi_csd_enc_rate_t_v01 min_frame_rate;
  /**<   Minimum CDMA encoded frame rate. Supported values: \n
           - 1 -- Eighth rate \n
           - 2 -- Quarter rate \n
           - 3 -- Half rate \n
           - 4 -- Full rate */

  /* Mandatory */
  /*  Maximum CDMA Encoded Frame Rate */
  qmi_csd_enc_rate_t_v01 max_frame_rate;
  /**<   Maximum CDMA encoded frame rate. Supported values: \n
           - 1 -- Eighth rate \n
           - 2 -- Quarter rate \n
           - 3 -- Half rate \n
           - 4 -- Full rate */

  /* Mandatory */
  /*  Reduced CDMA Encoded Frame Rate */
  qmi_csd_as_fmt_qcelp13k_br_t_v01 reduce_rate_cmd;
  /**<   Reduced CDMA encoded frame rate. Supported values: \n
           - 0 -- 14.4 kbps\n
           - 1 -- 12.2 kbps\n
           - 2 -- 11.2 kbps\n
           - 3 -- 9 kbps\n
           - 4 -- 7.2 kbps */

  /* Mandatory */
  /*  Rate Modulation */
  uint16_t rate_mod_cmd;
  /**<   Rate modulation. Supported values: \begin{itemize1}
         \item Bit 0: \begin{itemize1}
                 \item 1 -- Rate control is enabled
                 \end{itemize1}
         \item Bit 1: \begin{itemize1}
                \item 1 -- Limits the maximum number of consecutive full rate frames
                            with the number supplied in bits 2 to 9
                \item 0 -- Forces the minimum number of non-full rate frames in
                           between two full rate frames to the number supplied in bits
                           2 to 9

                Note: In both cases, half rate is substituted for full rate when
                necessary.
                \end{itemize1}
         \item Bits 9 to 2 -- Number of frames
         \item Bits 15 to 10 -- Reserved and set to 0
       \vspace{-0.18in} \end{itemize1}
       */
}qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Tx stream to QCELP13K format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_EVRC_DTX_MODE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_EVRC_DTX_DISABLE_V01 = 0,
  QMI_CSD_AS_EVRC_DTX_ENABLE_V01 = 1,
  QMI_CSD_AS_FMT_EVRC_DTX_MODE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_evrc_dtx_mode_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Tx stream to Enhanced Variable Rate Codec
           (EVRC) format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Bitrate Control Command */
  uint16_t bit_rate_ctrl;
  /**<   Bitrate control command. For EVRC, this is used as the average bitrate
         control command. Supported values: \begin{itemize1}
         \item Bit 0: \begin{itemize1}
                 \item 0 -- Rate control is disabled
                 \item 1 -- Rate control is enabled
                 \end{itemize1}
         \item Bit 1: \begin{itemize1}
                \item 1 -- Limits the maximum number of consecutive full rate
                           frames with the number supplied in bits 2 to 9
                \item 0 -- Forces the minimum number of non-full rate frames in
                           between two full rate frames to the number supplied in
                           bits 2 to 9
                \end{itemize1}
         \item Bits 9 to 2 -- Number of frames
         \item Bits 15 to 10 -- Reserved and set to 0
         \vspace{-0.18in} \end{itemize1}
    */

  /* Mandatory */
  /*  Minimum CDMA Encoded Frame Rate */
  qmi_csd_enc_rate_t_v01 min_frame_rate;
  /**<   Minimum CDMA encoded frame rate. Supported values: \n
             - 1 -- Eighth rate\n
             - 2 -- Quarter rate\n
             - 3 -- Half rate\n
             - 4 -- Full rate   */

  /* Mandatory */
  /*  Maximum CDMA Encoded Frame Rate */
  qmi_csd_enc_rate_t_v01 max_frame_rate;
  /**<   Maximum CDMA encoded frame rate. Supported values: \n
             - 1 -- Eighth rate\n
             - 2 -- Quarter rate\n
             - 3 -- Half rate\n
             - 4 -- Full rate   */

  /* Mandatory */
  /*  DTX Mode Enable Flag */
  qmi_csd_as_fmt_evrc_dtx_mode_t_v01 dtx_mode;
  /**<   DTX mode enable flag. Supported values:\n
         - 0 -- Disable\n
         - > 0 -- Enable */
}qmi_csd_as_cmd_set_stream_fmt_tx_evrc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Tx stream to Enhanced Variable Rate Codec
           (EVRC) format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_EVRCB_BT_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_EVRCB_BR_0_V01 = 0, /**<  9.3 kbps.  */
  QMI_CSD_AS_FMT_EVRCB_BR_1_V01 = 1, /**<  8.5 kbps.  */
  QMI_CSD_AS_FMT_EVRCB_BR_2_V01 = 2, /**<  7.5 kbps.  */
  QMI_CSD_AS_FMT_EVRCB_BR_3_V01 = 3, /**<  7.0 kbps.  */
  QMI_CSD_AS_FMT_EVRCB_BR_4_V01 = 4, /**<  6.6 kbps.  */
  QMI_CSD_AS_FMT_EVRCB_BR_5_V01 = 5, /**<  6.2 kbps.  */
  QMI_CSD_AS_FMT_EVRCB_BR_6_V01 = 6, /**<  5.8 kbps.  */
  QMI_CSD_AS_FMT_EVRCB_BR_7_V01 = 7, /**<  4.8 kbps.       */
  QMI_CSD_AS_FMT_EVRCB_BT_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_evrcb_bt_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Tx stream to EVRCB format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Bitrate Control */
  qmi_csd_as_fmt_evrcb_bt_t_v01 bit_rate_ctrl;
  /**<   Bitrate control command. For EVRC-B, uses the predefined values:\n
           - 0 -- 9.3 kbps\n
           - 1 -- 8.5 kbps\n
           - 2 -- 7.5 kbps\n
           - 3 -- 7.0 kbps\n
           - 4 -- 6.6 kbps\n
           - 5 -- 6.2 kbps\n
           - 6 -- 5.8 kbps\n
           - 7 -- 4.8 kbps */

  /* Mandatory */
  /*  Minimum CDMA Encoded Frame Rate */
  qmi_csd_enc_rate_t_v01 min_frame_rate;
  /**<   Minimum CDMA encoded frame rate. Supported values: \n
             - 1 -- Eighth rate\n
             - 2 -- Quarter rate\n
             - 3 -- Half rate\n
             - 4 -- Full rate   */

  /* Mandatory */
  /*  Maximum CDMA Encoded Frame Rate */
  qmi_csd_enc_rate_t_v01 max_frame_rate;
  /**<   Maximum CDMA encoded frame rate. Supported values: \n
             - 1 -- Eighth rate\n
             - 2 -- Quarter rate\n
             - 3 -- Half rate\n
             - 4 -- Full rate   */

  /* Mandatory */
  /*  DTX Mode Enable Flag */
  qmi_csd_as_fmt_evrc_dtx_mode_t_v01 dtx_mode;
  /**<   DTX mode enable flag. Supported values:\n
         - 0 -- Disable\n
         - > 0 -- Enable */
}qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Tx stream to EVRCB format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_EVRCWB_BT_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_EVRCWB_BR_0_V01 = 0, /**<  8.5 kbps.  */
  QMI_CSD_AS_FMT_EVRCWB_BR_4_V01 = 4, /**<  9.3 kbps.  */
  QMI_CSD_AS_FMT_EVRCWB_BR_7_V01 = 7, /**<  4.8 kbps.   */
  QMI_CSD_AS_FMT_EVRCWB_BT_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_evrcwb_bt_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Tx stream to EVRCWB format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Bitrate Control */
  qmi_csd_as_fmt_evrcwb_bt_t_v01 bit_rate_ctrl;
  /**<   Bitrate control. For EVRC-WB, uses the predefined values:\n
           - 0 -- 9.3 kbps\n
           - 4 -- 6.6 kbps\n
           - 7 -- 4.8 kbps*/

  /* Mandatory */
  /*  Minimum CDMA Encoded Frame Rate */
  qmi_csd_enc_rate_t_v01 min_frame_rate;
  /**<   Minimum CDMA encoded frame rate. Supported values: \n
             - 1 -- Eighth rate\n
             - 2 -- Quarter rate\n
             - 3 -- Half rate\n
             - 4 -- Full rate   */

  /* Mandatory */
  /*  Maximum CDMA Encoded Frame Rate */
  qmi_csd_enc_rate_t_v01 max_frame_rate;
  /**<   Maximum CDMA encoded frame rate. Supported values: \n
             - 1 -- Eighth rate\n
             - 2 -- Quarter rate\n
             - 3 -- Half rate\n
             - 4 -- Full rate   */

  /* Mandatory */
  /*  DTX Mode */
  qmi_csd_as_fmt_evrc_dtx_mode_t_v01 dtx_mode;
  /**<   DTX mode. Supported values:\n
         - 0 -- Disable\n
         - > 0 -- Enable */
}qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Tx stream to EVRCWB format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_SBC_ALLOC_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_FMT_SBC_ALLOC_LOUDNESS_V01 = 0, /**<  Loudness mode for subband
                                                 coding.  */
  QMI_CSD_AS_FMT_SBC_ALLOC_SNR_V01 = 1, /**<  Signal-to-Noise Ratio mode
                                                 for subband coding.  */
  QMI_CSD_AS_FMT_SBC_ALLOC_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_sbc_alloc_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_SBC_SUBBD_NUM_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_SBC_SUBBAND_4_V01 = 4, /**<  4-subband SBC.  */
  QMI_CSD_AS_SBC_SUBBAND_8_V01 = 8, /**<  8-subband SBC.     */
  QMI_CSD_AS_FMT_SBC_SUBBD_NUM_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_sbc_subbd_num_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_SBC_BLK_LEN_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_SBC_BLK_LEN_4_V01 = 4,
  QMI_CSD_AS_SBC_BLK_LEN_8_V01 = 8,
  QMI_CSD_AS_SBC_BLK_LEN_12_V01 = 12,
  QMI_CSD_AS_SBC_BLK_LEN_16_V01 = 16,
  QMI_CSD_AS_FMT_SBC_BLK_LEN_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_sbc_blk_len_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_FMT_SBC_CH_MODE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_SBC_CH_MODE_MONO_V01 = 1, /**<  Mono channel.  */
  QMI_CSD_AS_SBC_CH_MODE_STEREO_V01 = 2, /**<  Stereo channel.  */
  QMI_CSD_AS_SBC_CH_MODE_DUALMONO_V01 = 8, /**<  Dual/Mono mode.  */
  QMI_CSD_AS_SBC_CH_MODE_JOINTSTEREO_V01 = 9, /**<  Joint Stereo mode  */
  QMI_CSD_AS_FMT_SBC_CH_MODE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_fmt_sbc_ch_mode_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the audio Tx stream to Subband Coding (SBC) format.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Number of Subbands */
  qmi_csd_as_fmt_sbc_subbd_num_t_v01 sub_bands;
  /**<   Number of subbands. Supported values:
                                         4 or 8. */

  /* Mandatory */
  /*  Block Length */
  qmi_csd_as_fmt_sbc_blk_len_t_v01 block_len;
  /**<   Block length. Supported values: 4, 8,
                                         12, or 16. */

  /* Mandatory */
  /*  Channel Mode */
  qmi_csd_as_fmt_sbc_ch_mode_t_v01 channel_mode;
  /**<   Channel Allocation mode for SBC. Supported
                                     values: \n
                                     - 1 -- Mono channel.\n
                                     - 2 -- Stereo channel.\n
                                     - 8 -- Dual/Mono mode.\n
                                     - 9 -- Joint Stereo mode.*/

  /* Mandatory */
  /*  SBC Allocation Mode */
  qmi_csd_as_fmt_sbc_alloc_t_v01 alloc_method;
  /**<   SBC Allocation mode. Supported values:\n
                                     - 0 -- Loudness\n
                                     - 1 -- Signal-to-Noise Ratio (SNR) */

  /* Mandatory */
  /*  Bits Per Second */
  uint32_t bit_rate;
  /**<   Bits per second. */

  /* Mandatory */
  /*  Sample Rate */
  uint32_t sample_rate;
  /**<   Sample rate. Supported values:\n
                                     - 0 -- Native mode \n
                                     - Specify a sample rate */
}qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the audio Tx stream to Subband Coding (SBC) format.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sends an End Of Stream (EOS) indication for the audio stream. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_set_stream_eos_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sends an End Of Stream (EOS) indication for the audio stream. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_set_stream_eos_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the master gain. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Master Gain Step Level */
  uint16_t master_gain_step;
  /**<   Step level of the master gain. One of the values within the
         range of values returned by QMI_CSD_AS_CMD_GET_VOL_LEVELS_ RESP. */
}qmi_csd_as_cmd_config_pp_vol_master_gain_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the master gain. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/
}qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the stereo gain. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Mandatory */
  /*  Left Channel Gain Step Level */
  uint16_t left_ch_gain_step;
  /**<   Step level of the left channel gain. One of the values within
         the range of values returned by QMI_CSD_AS_CMD_GET_VOL_LEVELS_ REQ. */

  /* Mandatory */
  /*  Right Channel Gain Step Level */
  uint16_t right_ch_gain_step;
  /**<   Step level of the right channel gain. One of the values within
         the range of values returned by QMI_CSD_AS_CMD_GET_VOL_LEVELS_ REQ. */
}qmi_csd_as_cmd_config_pp_vol_stereo_gain_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the stereo gain. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AUD_PP_MULTI_CH_TYPE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AUD_PP_MULTI_CH_VOL_LEFT_V01 = 1, /**<  Left channel type  */
  QMI_CSD_AUD_PP_MULTI_CH_VOL_RIGHT_V01 = 2, /**<  Right channel type  */
  QMI_CSD_AUD_PP_MULTI_CH_VOL_CENTER_V01 = 3, /**<  Center channel type  */
  QMI_CSD_AUD_PP_MULTI_CH_VOL_LEFT_SURROUND_V01 = 4, /**<  Left surround channel type  */
  QMI_CSD_AUD_PP_MULTI_CH_VOL_RIGHT_SURROUND_V01 = 5, /**<  Right surround channel type  */
  QMI_CSD_AUD_PP_MULTI_CH_VOL_LEFT_BACK_V01 = 6, /**<  Left back channel type  */
  QMI_CSD_AUD_PP_MULTI_CH_VOL_RIGHT_BACK_V01 = 7, /**<  Right back channel type  */
  QMI_CSD_AUD_PP_MULTI_CH_VOL_SUBWOOFER_V01 = 8, /**<  Subwoofer channel type  */
  QMI_CSD_AUD_PP_MULTI_CH_TYPE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_aud_pp_multi_ch_type_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
typedef struct {

  qmi_csd_aud_pp_multi_ch_type_t_v01 ch_type;
  /**<   Channel type. Supported values:\n
         - 1 -- Left channel type \n
         - 2 -- Right channel type \n
         - 3 -- Center channel type \n
         - 4 -- Left surround channel type \n
         - 5 -- Right surround channel type \n
         - 6 -- Left back channel type \n
         - 7 -- Right back channel type \n
         - 8 -- Subwoofer channel type */

  uint16_t gain_idx;
  /**<   One of the out-of-range indices returned in the
         QMI_CSD_AS_CMD_GET_VOL_LEVELS_ RESP command. */
}qmi_csd_aud_pp_vol_multi_ch_gain_entry_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the multichannel gain. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Multichannel Gain Volume Levels */
  uint32_t multi_ch_gain_len;  /**< Must be set to # of elements in multi_ch_gain */
  qmi_csd_aud_pp_vol_multi_ch_gain_entry_v01 multi_ch_gain[QMI_CSD_AUD_PP_MULTI_CH_NUM_V01];
}qmi_csd_as_cmd_config_pp_vol_multichannel_gain_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the multichannel gain. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AUD_PP_VOL_MUTE_FLAG_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AUD_PP_VOL_UNMUTE_V01 = 0,
  QMI_CSD_AUD_PP_VOL_MUTE_V01 = 1,
  QMI_CSD_AUD_PP_VOL_MUTE_FLAG_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_aud_pp_vol_mute_flag_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the mute/unmute control. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Mute Mode */
  qmi_csd_aud_pp_vol_mute_flag_t_v01 mute;
  /**<   Mute disable/enable:\n
         - 0 -- Unmute\n
         - 1 -- Mute */
}qmi_csd_as_cmd_config_pp_vol_mute_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the mute/unmute control. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AUD_PP_EQ_FILTER_TYPE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AUD_PP_EQ_FILTER_TYPE_NONE_V01 = 0, /**<  Equalizer filter type is unknown.  */
  QMI_CSD_AUD_PP_EQ_FILTER_TYPE_BASS_BOOST_V01 = 1, /**<  Equalizer filter type is bass boost.  */
  QMI_CSD_AUD_PP_EQ_FILTER_TYPE_BASS_CUT_V01 = 2, /**<  Equalizer filter type is bass cut.  */
  QMI_CSD_AUD_PP_EQ_FILTER_TYPE_TREBLE_BOOST_V01 = 3, /**<  Equalizer filter type is treble boost.  */
  QMI_CSD_AUD_PP_EQ_FILTER_TYPE_TREBLE_CUT_V01 = 4, /**<  Equalizer filter type is treble cut.  */
  QMI_CSD_AUD_PP_EQ_FILTER_TYPE_BAND_BOOST_V01 = 5, /**<  Equalizer filter type is band boost.  */
  QMI_CSD_AUD_PP_EQ_FILTER_TYPE_BAND_CUT_V01 = 6, /**<  Equalizer filter type is band cut.  */
  QMI_CSD_AUD_PP_EQ_FILTER_TYPE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_aud_pp_eq_filter_type_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AUD_PP_EQ_ENABLE_FLAG_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AUD_PP_EQ_ENABLE_V01 = 1,
  QMI_CSD_AUD_PP_EQ_DISABLE_V01 = 0,
  QMI_CSD_AUD_PP_EQ_ENABLE_FLAG_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_aud_pp_eq_enable_flag_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  CSD audio PP equalizer enable flag.

 */
typedef struct {

  uint32_t band_idx;
  /**<   Band index. */

  qmi_csd_aud_pp_eq_filter_type_t_v01 filter_type;
  /**<   Equalizer filter type. Supported values are:\n
         - 0 -- Unknown\n
         - 1 -- Bass boost\n
         - 2 -- Bass cut\n
         - 3 -- Treble boost \n
         - 4 -- Treble cut\n
         - 5 -- Band boost \n
         - 6 -- Band cut
    */

  uint32_t center_freq_in_hz;
  /**<   Filter band center frequency. */

  int32_t filter_gain;
  /**<   Filter band initial gain in dB.
                                         Supported values: +12 dB to -12 dB
                                         with 1 dB increments. */

  int32_t lq_factor;
  /**<   Filter band quality factor expressed as a q-8 number; a fixed point
            number with a q factor of 8 (e.g.,
            @latexonly $3000/(2\textsuperscript{8})$@endlatexonly). */
}qmi_csd_aud_pp_eq_subband_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables, configures, or disables the equalizer for the audio stream. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  Equalizer Subband Configuration */
  uint8_t eq_bands_valid;  /**< Must be set to true if eq_bands is being passed */
  uint32_t eq_bands_len;  /**< Must be set to # of elements in eq_bands */
  qmi_csd_aud_pp_eq_subband_t_v01 eq_bands[QMI_CSD_AUD_PP_EQ_SUB_BAND_MAX_NUM_V01];
}qmi_csd_as_cmd_config_pp_eq_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables, configures, or disables the equalizer for the audio stream. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.          */
}qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AUD_PP_QCPR_PRESET_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AUD_PP_QCPR_PRESET_MEDIUMROOM_V01 = 1, /**<  Medium room QCPR preset.  */
  QMI_CSD_AUD_PP_QCPR_PRESET_MEDIUMHALL_V01 = 2, /**<  Medium hall QCPR preset.  */
  QMI_CSD_AUD_PP_QCPR_PRESET_CONCERTHALL_V01 = 3, /**<  Concert hall QCPR preset.   */
  QMI_CSD_AUD_PP_QCPR_PRESET_SURROUND_V01 = 4, /**<  Surround QCPR preset.   */
  QMI_CSD_AUD_PP_QCPR_PRESET_WARMSTAGE_V01 = 5, /**<  Warm stage QCPR preset.  */
  QMI_CSD_AUD_PP_QCPR_PRESET_CRYSTAL_V01 = 6, /**<  Crystal QCPR preset.   */
  QMI_CSD_AUD_PP_QCPR_PRESET_LIVINGROOM_V01 = 7, /**<  Living room QCPR preset.  */
  QMI_CSD_AUD_PP_QCPR_PRESET_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_aud_pp_qcpr_preset_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AUD_PP_QCPR_STRENGTH_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AUD_PP_QCPR_STRENGTH_TWO_V01 = 1, /**<  QCPR strength two.    */
  QMI_CSD_AUD_PP_QCPR_STRENGTH_THREE_V01 = 2, /**<  QCPR strength three.   */
  QMI_CSD_AUD_PP_QCPR_STRENGTH_FOUR_V01 = 3, /**<  QCPR strength four.   */
  QMI_CSD_AUD_PP_QCPR_STRENGTH_FIVE_V01 = 4, /**<  QCPR strength five.   */
  QMI_CSD_AUD_PP_QCPR_STRENGTH_SIX_V01 = 5, /**<  QCPR strength six.   */
  QMI_CSD_AUD_PP_QCPR_STRENGTH_SEVEN_V01 = 6, /**<  QCPR strength seven.   */
  QMI_CSD_AUD_PP_QCPR_STRENGTH_EIGHT_V01 = 7, /**<  QCPR strength eight.   */
  QMI_CSD_AUD_PP_QCPR_STRENGTH_NINE_V01 = 8, /**<  QCPR strength nine.   */
  QMI_CSD_AUD_PP_QCPR_STRENGTH_TEN_V01 = 9, /**<  QCPR strength ten.   */
  QMI_CSD_AUD_PP_QCPR_STRENGTH_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_aud_pp_qcpr_strength_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AUD_PP_QCPR_ENABLE_FLAG_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AUD_PP_QCPR_DISABLE_V01 = 0,
  QMI_CSD_AUD_PP_QCPR_ENABLE_V01 = 1,
  QMI_CSD_AUD_PP_QCPR_CONFIG_V01 = 2,
  QMI_CSD_AUD_PP_QCPR_ENABLE_FLAG_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_aud_pp_qcpr_enable_flag_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  CSD audio PP QCPR preset type.

 CSD audio PP QCPR strength.

 CSD audio PP QCPR enable flag.

 QCPR configuration parameters.
 */
typedef struct {

  qmi_csd_aud_pp_qcpr_preset_t_v01 preset;
  /**<   Preset value for QCPR. */

  qmi_csd_aud_pp_qcpr_strength_t_v01 strength;
  /**<   Strength value for QCPR. */
}qmi_csd_aud_pp_qcpr_config_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables, configures, or disables Qconcert
           @latexonly \texttrademark~@endlatexonly Plus Reverb (QCPR) for the audio
           stream. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  QCPR Configuration */
  uint8_t config_valid;  /**< Must be set to true if config is being passed */
  qmi_csd_aud_pp_qcpr_config_t_v01 config;
}qmi_csd_as_cmd_config_pp_qcpr_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables, configures, or disables Qconcert
           @latexonly \texttrademark~@endlatexonly Plus Reverb (QCPR) for the audio
           stream. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.          */
}qmi_csd_as_cmd_config_pp_qcpr_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AUD_PP_SPA_SAMPLE_PTS_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AUD_PP_SPA_SAMPLEPOINT_32_V01 = 32, /**<  SPA sample point 32.  */
  QMI_CSD_AUD_PP_SPA_SAMPLEPOINT_64_V01 = 64, /**<  SPA sample point 64.  */
  QMI_CSD_AUD_PP_SPA_SAMPLEPOINT_128_V01 = 128, /**<  SPA sample point 128.  */
  QMI_CSD_AUD_PP_SPA_SAMPLEPOINT_256_V01 = 256, /**<  SPA sample point 256.  */
  QMI_CSD_AUD_PP_SPA_SAMPLE_PTS_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_aud_pp_spa_sample_pts_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AUD_PP_SPA_ENABLE_FLAG_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AUD_PP_SPA_DISABLE_V01 = 0,
  QMI_CSD_AUD_PP_SPA_ENABLE_V01 = 1,
  QMI_CSD_AUD_PP_SPA_CONFIG_V01 = 2,
  QMI_CSD_AUD_PP_SPA_ENABLE_FLAG_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_aud_pp_spa_enable_flag_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_aggregates
    @{
  */
/**  CSD audio PP SPA sample point type.

 CSD audio PP SPA enable flag.

 Spectrum analyzer configuration parameters.
 */
typedef struct {

  uint32_t sample_interval;
  /**<   Sample interval in terms of number of samples.
         Supported values: >= 512 samples. */

  qmi_csd_aud_pp_spa_sample_pts_t_v01 sample_points;
  /**<   Specifies the sample points for the SPA
         filter. Supported values: 32, 64, 128, and 256. */
}qmi_csd_aud_pp_spa_config_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables, configures, or disables the Spectrum Analyzer (SPA) for the
           audio stream. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  SPA Configuration */
  uint8_t config_valid;  /**< Must be set to true if config is being passed */
  qmi_csd_aud_pp_spa_config_t_v01 config;
}qmi_csd_as_cmd_config_pp_spa_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables, configures, or disables the Spectrum Analyzer (SPA) for the
           audio stream. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.          */
}qmi_csd_as_cmd_config_pp_spa_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AUD_PP_TSM_ENABLE_FLAG_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AUD_PP_TSM_DISABLE_V01 = 0,
  QMI_CSD_AUD_PP_TSM_ENABLE_V01 = 1,
  QMI_CSD_AUD_PP_TSM_CONFIG_V01 = 2,
  QMI_CSD_AUD_PP_TSM_ENABLE_FLAG_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_aud_pp_tsm_enable_flag_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables, configures, or disables Time Scale Modification (TSM) for
           the audio stream. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  TSM Factor */
  uint8_t tsm_factor_valid;  /**< Must be set to true if tsm_factor is being passed */
  uint16_t tsm_factor;
  /**<   Time scale modification factor in
         Q11. Supported values: 1024 to 16384. */
}qmi_csd_as_cmd_config_pp_tsm_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables, configures, or disables Time Scale Modification (TSM) for
           the audio stream. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_config_pp_tsm_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the spectrum-analyzed data for the audio stream
           from the driver. Only Asynchronous mode is supported.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_get_spa_data_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the spectrum-analyzed data for the audio stream
           from the driver. Only Asynchronous mode is supported.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_get_spa_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_DUALMONO_REMAP_TYPE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_DUALMONO_REMAP_TYPE1_V01 = 0, /**<  First Single Channel Element (SCE) to the left channel and the second
         SCE to the right channel.  */
  QMI_CSD_AS_DUALMONO_REMAP_TYPE2_V01 = 1, /**<  First SCE to the right channel, and the second SCE to the left
         channel.   */
  QMI_CSD_AS_DUALMONO_REMAP_TYPE3_V01 = 2, /**<  First SCE to both the left and right channels.   */
  QMI_CSD_AS_DUALMONO_REMAP_TYPE4_V01 = 4, /**<  Second SCE to both the left and right channels.  */
  QMI_CSD_AS_DUALMONO_REMAP_TYPE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_dualmono_remap_type_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the dual/mono mapping configuration. This is currently used by
           the Integrated Services Digital Broadcasting -- Terrestrial (ISDB-T)
           feature only. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Mandatory */
  /*  Dual/Mono Remap Configuration */
  qmi_csd_as_dualmono_remap_type_t_v01 remap_type;
  /**<   Dual/mono remap configuration. Supported values:\n
         - 0 -- First Single Channel Element (SCE) to the left channel and the
                second SCE to the right channel.\n
         - 1 -- First SCE to the right channel, and the second SCE to the left
                channel.\n
         - 2 -- First SCE to both the left and right channels. \n
         - 4 -- Second SCE to both the left and right channels.
    */
}qmi_csd_as_cmd_set_dualmono_remap_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the dual/mono mapping configuration. This is currently used by
           the Integrated Services Digital Broadcasting -- Terrestrial (ISDB-T)
           feature only. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_set_dualmono_remap_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Adjusts the session time. This command sets the sample number to be
           added or dropped for the ISDB-T feature.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  High Time in Microseconds */
  uint32_t time_high;
  /**<   Upper 32 bits of the 64-bit adjustment to the session clock in
           microseconds. */

  /* Mandatory */
  /*  Low Time in Microseconds */
  uint32_t time_low;
  /**<   Lower 32 bits of the 64-bit adjustment to the session clock in
           microseconds. */
}qmi_csd_as_cmd_adjust_session_clock_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Adjusts the session time. This command sets the sample number to be
           added or dropped for the ISDB-T feature.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.  */

  /* Optional */
  /*  Low Time Estimated Processing Time */
  uint8_t estimated_processing_time_low_valid;  /**< Must be set to true if estimated_processing_time_low is being passed */
  uint32_t estimated_processing_time_low;
  /**<   Lower 32 bits of the 64-bit estimated processing time in microseconds.
         Provides the time duration the DSP needs to finish the adjustment.  */

  /* Optional */
  /*  High Time Estimated Processing Time */
  uint8_t estimated_processing_time_high_valid;  /**< Must be set to true if estimated_processing_time_high is being passed */
  uint32_t estimated_processing_time_high;
  /**<   Upper 32 bits of the 64-bit estimated processing time in microseconds.
         Provides the time duration the DSP needs to finish the adjustment. */
}qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_AAC_SBR_PS_TYPE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_AAC_SBR_PS_TYPE1_V01 = 0, /**<  SBR is off, and parametric stereo AAC is off. */
  QMI_CSD_AS_AAC_SBR_PS_TYPE2_V01 = 1, /**<  SBR is on, and parametric stereo AAC is off.  */
  QMI_CSD_AS_AAC_SBR_PS_TYPE3_V01 = 2, /**<  SBR is on, and parametric stereo AAC is on.  */
  QMI_CSD_AS_AAC_SBR_PS_TYPE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_aac_sbr_ps_type_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the Spectral Band Replication (SBR) flag or the Parametric
           Stereo (PS) flag for the AAC format. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Mandatory */
  /*  AAC SBR PS Configuration */
  qmi_csd_as_aac_sbr_ps_type_t_v01 type;
  /**<   AAC SBR PS configuration. Supported values:\n
         - 0 -- SBR is off, and parametric stereo AAC is off \n
         - 1 -- SBR is on, and parametric stereo AAC is off \n
         - 2 -- SBR is on, and parametric stereo AAC is on */
}qmi_csd_as_cmd_set_aac_sbr_ps_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the Spectral Band Replication (SBR) flag or the Parametric
           Stereo (PS) flag for the AAC format. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Starts/stops the DTMF signal. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  DTMF Tone 1 */
  uint16_t tone_1;
  /**<   First tone frequency for DTMF. Supported values: 100 to 4000 Hz. */

  /* Mandatory */
  /*  DTMF Tone 2 */
  uint16_t tone_2;
  /**<   Second tone frequency for DTMF. Supported values: 100 to 4000 Hz. */

  /* Mandatory */
  /*  DTMF Gain */
  uint16_t gain_index;
  /**<    DTMF gain. */

  /* Mandatory */
  /*  DTMF Tone Duration */
  int16_t duration;
  /**<   Duration of the DTMF tone in milliseconds. Supported values:\n
           - -1 -- Infinite duration \n
           -  0 -- Disables/stops the infinite tone \n
           -  > 0 -- Finite duration in milliseconds */
}qmi_csd_as_cmd_dtmf_ctl_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Starts/stops the DTMF signal. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_dtmf_ctl_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the stream information properties for this session. This
           includes the maximum buffer size supported and the type of memory to
           be passed to the CSD. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Mandatory */
  /*  Maximum Buffer Size */
  uint32_t max_buf_size;
  /**<   Maximum buffer size to be passed to the CSD. */
}qmi_csd_as_cmd_set_stream_info_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the stream information properties for this session. This
           includes the maximum buffer size supported and the type of memory to
           be passed to the CSD. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_as_cmd_set_stream_info_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the last rendered byte offset of the bitstream. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_get_rendered_byte_offset_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the last rendered byte offset of the bitstream. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  Byte Offset */
  uint8_t offset_valid;  /**< Must be set to true if offset is being passed */
  uint64_t offset;
  /**<   Byte offset.      */
}qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the MIDI sequence associated with a MIDI playback session. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_get_midi_sequence_id_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the MIDI sequence associated with a MIDI playback session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Optional */
  /*  Sequence ID */
  uint8_t sequence_id_valid;  /**< Must be set to true if sequence_id is being passed */
  uint8_t sequence_id;
  /**<   Sequence ID. */
}qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_ENC_FMT_TYPE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_ENC_FMT_TYPE_AAC_V01 = 6, /**<  Encoder type AAC is 6.  */
  QMI_CSD_AS_ENC_FMT_TYPE_SBC_V01 = 15, /**<  Encoder type SBC is 15.  */
  QMI_CSD_AS_ENC_FMT_TYPE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_enc_fmt_type_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Dynamically changes the encoder bitrate during a recoding session. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Playback Format Type */
  qmi_csd_as_enc_fmt_type_t_v01 fmt_type;
  /**<   Format type for playback. Supported values:\n
                                 - 6 -- Format for AAC \n
                                 - 15 -- Format for SBC */

  /* Mandatory */
  /*  Bitrate */
  uint32_t bit_rate;
  /**<   New bitrate applied to future incoming encoded
                                 streams by clients.

                                 For SBC encoder, maximum supported bitrate:\n
                                 - 320 kbps for mono channel \n
                                 - 512 kbps for stereo channels \n

                                 For AAC encoder:\n
                                 Input sampling frequency (f_s) in hertz \n
                                 Minimum values: \n
                                 - Min(24000, 0.5*f_s); AAC_LC (mono) \n
                                 - Min(24000, f_s); AAC_LC (stereo) \n
                                 - 24000; AAC+ (mono), AAC+ (stereo), and
                                   eAAC+ \n
                                 Maximum values:\n
                                 - Min(192000, 6*f_s); AAC_LC (mono) \n
                                 - Min(192000, 12*f_s); AAC_LC (stereo) \n
                                 - Min(192000, 6*f_s); AAC+ (mono).\n
                                 - Min(192000, 12*f_s); AAC+ (stereo) \n
                                 - Min(192000, 12*f_s); eAAC+ */
}qmi_csd_as_cmd_encoder_bit_rate_update_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Dynamically changes the encoder bitrate during a recoding session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/
}qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the Windows Media Audio 10 Pro audio decoder
           to output PCM samples based on a multiple channel configuration
           defined by the client.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Mandatory */
  /*  Channel Configuration */
  int32_t channel_mask;
  /**<   Channel configuration can be set by the client to determine the type
         of downmixing that is to be applied by the decoder to get the desired
         output channels. Supported values: \n
         - -1 -- No decoder downmixing is required.
         - Any valid combination of Windows Media Audio 10 Pro
           channel bitfields. See Appendix \ref{app:WMA10channelBitfields} for
           information on multiple channel bitfield definitions. See Appendix
           \ref{app:channelConfigWMA10} for information on popular multiple-channel
           configuration settings. */
}qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the Windows Media Audio 10 Pro audio decoder
           to output PCM samples based on a multiple channel configuration
           defined by the client.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_EAC3_MULTI_CH_CONFIG_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_NULL_V01 = -1, /**<  No downmixing channel configuration selected.     */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FC_V01 = 1, /**<  Front center speaker is selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FR_V01 = 2, /**<  Front left and front right speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FR_FC_V01 = 3, /**<  Front left, front right, and front center speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FR_SL_V01 = 4, /**<  Front left, front right, and surround left speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_SL_V01 = 5, /**<  Front left, front center, front center, and surround left speakers
         are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FR_SL_SR_V01 = 6, /**<  Front left, front right, surround left, and surround right speakers
         are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_SL_SR_V01 = 7, /**<  Front left, front center, front right, surround left, and surround
         right speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_CVH_V01 = 8, /**<  Front left, front center, front right, and center vertical height
         speakers are selected.     */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FR_SL_SR_ST_V01 = 9, /**<  Front left, front right, surround left, surround right, and
         surround top speakers are selected.    */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_SL_SR_ST_V01 = 10, /**<  Front left, front center, front right, surround left, surround
         right, and surround top speakers are selected.      */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_SL_SR_CVH_V01 = 11, /**<  Front left, front center, front right, surround left, surround
         right, and center vertical height speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_CL_CR_V01 = 12, /**<  Front left, front center, front right, center left, and center
         right speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FR_SL_SR_LW_RW_V01 = 13, /**<  Front left, front right, surround left, surround right, left wide,
         and right wide speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FR_SL_SR_LVH_RVH_V01 = 14, /**<  Front left, front right, surround left, surround right, left
         vertical height, and right vertical height speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FR_SL_SR_SLD_SRD_V01 = 15, /**<  Front left, front right, surround left, surround right, surround
         left direct, and surround right direct speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FR_SL_SR_SLR_SRR_V01 = 16, /**<  Front left, front right, surround left, surround right, surround
         left rear, and surround right rear speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FR_SL_SR_CL_CR_V01 = 17, /**<  Front left, front right, surround left, surround right, center
         left, and center right speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_SL_SR_LW_RW_V01 = 18, /**<  Front left, front center, front right, surround left, surround
         right, left wide, and right wide speakers are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_SL_SR_LVH_RVH_V01 = 19, /**<  Front left, front center, front right, surround left, surround
         right, left vertical height, and right vertical height speakers
         are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_SL_SR_SLD_SRD_V01 = 20, /**<  Front left, front center, front right, surround left, surround
         right, surround left direct, and surround right direct speakers
         are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_SL_SR_SLR_SRR_V01 = 21, /**<  Front left, front center, front right, surround left, surround
         right, surround left rear, and surround right rear speakers
         are selected.  */
  QMI_CSD_AS_EAC3_CHANNEL_CONFIG_FL_FC_FR_SL_SR_TS_CVH_V01 = 22, /**<  Front left, front center, front right, surround left, surround
         right, surround top, and center vertical height speakers are
         selected.  */
  QMI_CSD_AS_EAC3_MULTI_CH_CONFIG_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_eac3_multi_ch_config_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AS_EAC3_CH_DEF_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AS_EAC3_CHANNEL_NULL_V01 = 0, /**<  Invalid channel.   */
  QMI_CSD_AS_EAC3_CHANNEL_FL_V01 = 1, /**<  Front left channel.    */
  QMI_CSD_AS_EAC3_CHANNEL_FC_V01 = 2, /**<  Front center channel.      */
  QMI_CSD_AS_EAC3_CHANNEL_FR_V01 = 3, /**<  Front right channel.   */
  QMI_CSD_AS_EAC3_CHANNEL_SL_V01 = 4, /**<  Surround left channel.     */
  QMI_CSD_AS_EAC3_CHANNEL_SR_V01 = 5, /**<  Surround right channel.    */
  QMI_CSD_AS_EAC3_CHANNEL_LFE_V01 = 6, /**<  Low frequency channel.      */
  QMI_CSD_AS_EAC3_CHANNEL_EX_1_V01 = 7, /**<  Extension X1 channel.      */
  QMI_CSD_AS_EAC3_CHANNEL_EX_2_V01 = 8, /**<  Extension X2 channel.      */
  QMI_CSD_AS_EAC3_CH_DEF_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_as_eac3_ch_def_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Configures the EAC3 audio decoder to output PCM samples based on
           a multiple channel configuration defined by the client.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */

  /* Mandatory */
  /*  Number of Decoder Output Channels */
  uint32_t num_channels;
  /**<   Number of decoder output channels. Supported values: 1 to 8.  */

  /* Mandatory */
  /*  Channel Configuration */
  qmi_csd_as_eac3_multi_ch_config_t_v01 channel_config;
  /**<   Channel configuration can be set by the client to determine the number
         of downmixed channels present in the decoder output. See Appendix
         \ref{app:channelConfigEAC3} for EAC3 channel configurations. */

  /* Mandatory */
  /*  Channel Mapping Array */
  qmi_csd_as_eac3_ch_def_t_v01 channel_mapping[8];
  /**<   Channel mapping array. Supported values:\n
    - 0 -- Invalid channel \n
    - 1 -- Front left channel \n
    - 2 -- Front center channel \n
    - 3 -- Front right channel  \n
    - 4 -- Surround left channel \n
    - 5 -- Surround right channel \n
    - 6 -- Low frequency channel \n
    - 7 -- Extension X1 channel \n
    - 8 -- Extension X2 channel */
}qmi_csd_as_cmd_config_decoder_multi_channel_eac3_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Configures the EAC3 audio decoder to output PCM samples based on
           a multiple channel configuration defined by the client.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Stream Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio stream. */
}qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates the End of Stream event information (i.e, all data has
           been rendered) to the stream client. This indication is enabled when
           the stream is opened and is disabled when the stream is closed. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/
}qmi_csd_as_evt_eos_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates a sample rate change or channel configuration change
           information to the stream client.  */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Mandatory */
  /*  Stream Sample Rate */
  uint32_t sample_rate;
  /**<   Stream sample rate.   */

  /* Mandatory */
  /*  Channel Configuration */
  uint16_t num_channels;
  /**<   Channel configuration. */

  /* Mandatory */
  /*  Channel Mapping Array */
  uint8_t channel_mapping[8];
  /**<   Channel mapping array. */
}qmi_csd_as_evt_sr_cm_change_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates the asynchronous spectrum-analyzed buffer production
           information to the stream client. The driver publishes EVT Done once
           the driver is done producing the buffer with spectrum-analyzed data. */
typedef struct {

  /* Mandatory */
  /*  Audio Stream Handle */
  uint32_t handle;
  /**<   Unique handle for the audio stream.*/

  /* Mandatory */
  /*  SPA Data Buffer Information */
  uint32_t spa_data_len;  /**< Must be set to # of elements in spa_data */
  uint8_t spa_data[QMI_CSD_SPA_DATA_BUF_SIZE_V01];
  /**<   For detailed SPA data buffer format, refer to \hyperref[Q3]{[Q3]}. */
}qmi_csd_as_evt_spa_buf_ready_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Attaches streams to the audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t ac_handle;
  /**<   Unique handle for the audio context. */

  /* Mandatory */
  /*  Audio Stream Handles Array */
  uint32_t as_handles_len;  /**< Must be set to # of elements in as_handles */
  uint32_t as_handles[QMI_CSD_MAX_NUM_AUDIO_STREAM_FOR_ONE_AC_V01];
  /**<   Array of the audio stream handles. */
}qmi_csd_ac_cmd_as_attach_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Attaches streams to the audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Audio context handle. */
}qmi_csd_ac_cmd_as_attach_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Detaches streams from the audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t ac_handle;
  /**<   Audio context handle. */

  /* Mandatory */
  /*  Audio Stream Handles Array */
  uint32_t as_handles_len;  /**< Must be set to # of elements in as_handles */
  uint32_t as_handles[QMI_CSD_MAX_NUM_AUDIO_STREAM_FOR_ONE_AC_V01];
  /**<   Array of the audio stream handles. */
}qmi_csd_ac_cmd_as_detach_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Detaches streams from the audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Audio context handle. */
}qmi_csd_ac_cmd_as_detach_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the device ID information for an audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t ac_handle;
  /**<   Unique handle for the audio context. */

  /* Mandatory */
  /*  Device Sample Rate */
  qmi_csd_dev_sr_v01 sample_rate;
  /**<   Sample rate for the device. Supported values:\n
         - QMI_CSD_DEV_SR_8000 (8000) -- 8000 samples per second \n
         - QMI_CSD_DEV_SR_16000 (16000) -- 16000 samples per second \n
         - QMI_CSD_DEV_SR_48000 (48000) -- 48000 samples per second
    */

  /* Mandatory */
  /*  Device ID */
  uint32_t dev_id;
  /**<   Device ID. */
}qmi_csd_ac_cmd_set_device_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the device ID information for an audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Audio context handle. */
}qmi_csd_ac_cmd_set_device_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables the audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t ac_handle;
  /**<   Unique handle for the audio context. */
}qmi_csd_ac_cmd_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables the audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Audio context handle. */
}qmi_csd_ac_cmd_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Disables the audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t ac_handle;
  /**<   Audio context handle. */
}qmi_csd_ac_cmd_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Disables the audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Audio context handle. */
}qmi_csd_ac_cmd_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the master gain for the audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t handle;
  /**<   Audio context handle. */

  /* Mandatory */
  /*  Master Gain Step Level */
  uint16_t master_gain_step;
  /**<   Step level of the master gain. One of the values within the
         range of values returned by QMI_CSD_AS_CMD_GET_VOL_LEVELS_ RESP. */
}qmi_csd_ac_cmd_config_pp_vol_master_gain_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the master gain for the audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Audio context handle. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the stereo gain for the audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t handle;
  /**<   Audio context handle. */

  /* Mandatory */
  /*  Left Channel Gain Step Level */
  uint16_t left_ch_gain_step;
  /**<   Step level of the left channel gain. One of the values within
         the range of values returned by QMI_CSD_AS_CMD_GET_VOL_LEVELS_ RESP. */

  /* Mandatory */
  /*  Right Channel Gain Step Level */
  uint16_t right_ch_gain_step;
  /**<   Step level of the right channel gain. One of the values within
         the range of values returned by QMI_CSD_AS_CMD_GET_VOL_LEVELS_ RESP. */
}qmi_csd_ac_cmd_config_pp_vol_stereo_gain_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the stereo gain for the audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Audio context handle. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the multichannel gain for the audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t handle;
  /**<   Audio context handle. */

  /* Mandatory */
  /*  Multichannel Gain Volume Levels */
  uint32_t multi_ch_gain_len;  /**< Must be set to # of elements in multi_ch_gain */
  qmi_csd_aud_pp_vol_multi_ch_gain_entry_v01 multi_ch_gain[QMI_CSD_AUD_PP_MULTI_CH_NUM_V01];
}qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the multichannel gain for the audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Audio context handle. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets the mute/unmute control for the audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t handle;
  /**<   Audio context handle.*/

  /* Mandatory */
  /*  Mute Mode */
  qmi_csd_aud_pp_vol_mute_flag_t_v01 mute;
  /**<   Mute mode:\n
         - 0 -- Unmute\n
         - 1 -- Mute
    */
}qmi_csd_ac_cmd_config_pp_vol_mute_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets the mute/unmute control for the audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Audio context handle. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables, configures, or disables the equalizer for the audio
           context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t handle;
  /**<   Audio context handle. */

  /* Optional */
  /*  Equalizer Subband Configuration */
  uint8_t eq_bands_valid;  /**< Must be set to true if eq_bands is being passed */
  uint32_t eq_bands_len;  /**< Must be set to # of elements in eq_bands */
  qmi_csd_aud_pp_eq_subband_t_v01 eq_bands[QMI_CSD_AUD_PP_EQ_SUB_BAND_MAX_NUM_V01];
}qmi_csd_ac_cmd_config_pp_eq_enable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables, configures, or disables the equalizer for the audio
           context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Audio context handle. */

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables, configures, or disables QCPR for the audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t handle;
  /**<   Unique handle for the audio context.*/

  /* Optional */
  /*  QCPR Configuration */
  uint8_t config_valid;  /**< Must be set to true if config is being passed */
  qmi_csd_aud_pp_qcpr_config_t_v01 config;
}qmi_csd_ac_cmd_config_pp_qcpr_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables, configures, or disables QCPR for the audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio context.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Enables, configures, or disables the spectrum analyzer (SPA) for the
           audio context. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t handle;
  /**<   Unique handle for the audio context.*/

  /* Optional */
  /*  SPA Configuration */
  uint8_t config_valid;  /**< Must be set to true if config is being passed */
  qmi_csd_aud_pp_spa_config_t_v01 config;
}qmi_csd_ac_cmd_config_pp_spa_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Enables, configures, or disables the spectrum analyzer (SPA) for the
           audio context. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio context.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.           */
}qmi_csd_ac_cmd_config_pp_spa_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Gets the spectrum-analyzed data for the audio context from the
           driver. Only Asynchronous mode is supported.  */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t handle;
  /**<   Unique handle for the audio context. */
}qmi_csd_ac_cmd_get_spa_data_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Gets the spectrum-analyzed data for the audio context from the
           driver. Only Asynchronous mode is supported.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio context. */
}qmi_csd_ac_cmd_get_spa_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_enums
    @{
  */
typedef enum {
  QMI_CSD_AC_MULTI_CH_MAPPING_DEF_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_NULL_V01 = 0, /**<  Not a valid channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_FL_V01 = 1, /**<  Front left channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_FR_V01 = 2, /**<  Front right channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_FC_V01 = 3, /**<  Front center channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_LS_V01 = 4, /**<  Left surround channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_RS_V01 = 5, /**<  Right surround channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_LFE_V01 = 6, /**<  Low frequency effects channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_LB_V01 = 7, /**<  Left back channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_RB_V01 = 8, /**<  Right back channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_CS_V01 = 9, /**<  Center surround channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_TS_V01 = 10, /**<  Top surround channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_CVH_V01 = 11, /**<  Center vertical height channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_MS_V01 = 12, /**<  Mono surround channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_FLC_V01 = 13, /**<  Front left center channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_FRC_V01 = 14, /**<  Front right center channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_RLC_V01 = 15, /**<  Rear left center channel.  */
  QMI_CSD_AC_MULTI_CHANNEL_MAPPING_CHANNEL_RRC_V01 = 16, /**<  Rear right center channel.  */
  QMI_CSD_AC_MULTI_CH_MAPPING_DEF_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_csd_ac_multi_ch_mapping_def_t_v01;
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Request Message; Sets up multiple channels for the audio context. This command
           applies to the Rx device only. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t handle;
  /**<   Unique handle for the audio context.*/

  /* Mandatory */
  /*  Number of Channels */
  uint16_t num_channels;
  /**<   Number of channels to be set up by the client.
         Supported values: 3 to 8. */

  /* Mandatory */
  /*  Channel Mapping */
  qmi_csd_ac_multi_ch_mapping_def_t_v01 channel_mapping[8];
  /**<   Channel layout mapping of the audio device. Supported values:\n
         - 0 -- Not a valid channel \n
         - 1 -- Front left channel \n
         - 2 -- Front right channel \n
         - 3 -- Front center channel \n
         - 4 -- Left surround channel \n
         - 5 -- Right surround channel \n
         - 6 -- Low frequency effects channel \n
         - 7 -- Left back channel \n
         - 8 -- Right back channel \n
         - 9 -- Center surround channel \n
         - 10 -- Top surround channel \n
         - 11 -- Center vertical height channel \n
         - 12 -- Mono surround channel \n
         - 13 -- Front left center channel \n
         - 14 -- Front right center channel \n
         - 15 -- Rear left center channel \n
         - 16 -- Rear right center channel
 */
}qmi_csd_ac_cmd_config_multi_channel_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Response Message; Sets up multiple channels for the audio context. This command
           applies to the Rx device only. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  CSD Status */
  uint8_t qmi_csd_status_code_valid;  /**< Must be set to true if qmi_csd_status_code is being passed */
  qmi_csd_status_v01 qmi_csd_status_code;
  /**<   GUID for the CSD status.*/

  /* Optional */
  /*  Audio Context Handle */
  uint8_t handle_valid;  /**< Must be set to true if handle is being passed */
  uint32_t handle;
  /**<   Unique handle for the audio context. */
}qmi_csd_ac_cmd_config_multi_channel_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup csd_qmi_messages
    @{
  */
/** Indication Message; Indicates the asynchronous spectrum-analyzed buffer production
           information to the context client. The driver publishes EVT Done
           once the driver is done producing the buffer with spectrum-analyzed
           data. */
typedef struct {

  /* Mandatory */
  /*  Audio Context Handle */
  uint32_t handle;
  /**<   Unique handle for the audio context.*/

  /* Mandatory */
  /*  SPA Data Buffer Information */
  uint32_t spa_data_len;  /**< Must be set to # of elements in spa_data */
  uint8_t spa_data[QMI_CSD_SPA_DATA_BUF_SIZE_V01];
  /**<   For detailed SPA data buffer format, refer to \hyperref[Q3]{[Q3]}. */
}qmi_csd_ac_evt_spa_buf_ready_ind_msg_v01;  /* Message */
/**
    @}
  */

/*Service Message Definition*/
/** @addtogroup csd_qmi_msg_ids
    @{
  */
#define QMI_CSD_QUERY_DRIVER_VERSION_REQ_V01 0x0020
#define QMI_CSD_QUERY_DRIVER_VERSION_RESP_V01 0x0020
#define QMI_CSD_INIT_REQ_V01 0x0021
#define QMI_CSD_INIT_RESP_V01 0x0021
#define QMI_CSD_DEINIT_REQ_V01 0x0022
#define QMI_CSD_DEINIT_RESP_V01 0x0022
#define QMI_CSD_OPEN_PASSIVE_CONTROL_VOICE_STREAM_REQ_V01 0x0023
#define QMI_CSD_OPEN_PASSIVE_CONTROL_VOICE_STREAM_RESP_V01 0x0023
#define QMI_CSD_OPEN_FULL_CONTROL_VOICE_STREAM_REQ_V01 0x0024
#define QMI_CSD_OPEN_FULL_CONTROL_VOICE_STREAM_RESP_V01 0x0024
#define QMI_CSD_OPEN_VOICE_CONTEXT_REQ_V01 0x0025
#define QMI_CSD_OPEN_VOICE_CONTEXT_RESP_V01 0x0025
#define QMI_CSD_OPEN_VOICE_MANAGER_REQ_V01 0x0026
#define QMI_CSD_OPEN_VOICE_MANAGER_RESP_V01 0x0026
#define QMI_CSD_OPEN_DEVICE_CONTROL_REQ_V01 0x0027
#define QMI_CSD_OPEN_DEVICE_CONTROL_RESP_V01 0x0027
#define QMI_CSD_CLOSE_REQ_V01 0x0028
#define QMI_CSD_CLOSE_RESP_V01 0x0028
#define QMI_CSD_IOCTL_DEV_CMD_ENABLE_REQ_V01 0x0029
#define QMI_CSD_IOCTL_DEV_CMD_ENABLE_RESP_V01 0x0029
#define QMI_CSD_IOCTL_DEV_CMD_DISABLE_REQ_V01 0x002A
#define QMI_CSD_IOCTL_DEV_CMD_DISABLE_RESP_V01 0x002A
#define QMI_CSD_IOCTL_DEV_CMD_AFE_LOOPBACK_REQ_V01 0x002B
#define QMI_CSD_IOCTL_DEV_CMD_AFE_LOOPBACK_RESP_V01 0x002B
#define QMI_CSD_IOCTL_DEV_CMD_ANC_CONTROL_REQ_V01 0x002C
#define QMI_CSD_IOCTL_DEV_CMD_ANC_CONTROL_RESP_V01 0x002C
#define QMI_CSD_IOCTL_DEV_CMD_COMPANDING_CONTROL_REQ_V01 0x002D
#define QMI_CSD_IOCTL_DEV_CMD_COMPANDING_CONTROL_RESP_V01 0x002D
#define QMI_CSD_IOCTL_DEV_CMD_GET_MAX_DEVICE_NUMS_REQ_V01 0x002E
#define QMI_CSD_IOCTL_DEV_CMD_GET_MAX_DEVICE_NUMS_RESP_V01 0x002E
#define QMI_CSD_IOCTL_DEV_CMD_GET_DEV_CAPS_REQ_V01 0x002F
#define QMI_CSD_IOCTL_DEV_CMD_GET_DEV_CAPS_RESP_V01 0x002F
#define QMI_CSD_IOCTL_DEV_CMD_DTMF_CONTROL_REQ_V01 0x0030
#define QMI_CSD_IOCTL_DEV_CMD_DTMF_CONTROL_RESP_V01 0x0030
#define QMI_CSD_IOCTL_DEV_CMD_SIDETONE_CONTROL_REQ_V01 0x0031
#define QMI_CSD_IOCTL_DEV_CMD_SIDETONE_CONTROL_RESP_V01 0x0031
#define QMI_CSD_IOCTL_DEV_CMD_CONFIGURE_REQ_V01 0x0032
#define QMI_CSD_IOCTL_DEV_CMD_CONFIGURE_RESP_V01 0x0032
#define QMI_CSD_IOCTL_VC_CMD_SET_DEVICE_CONFIG_REQ_V01 0x0033
#define QMI_CSD_IOCTL_VC_CMD_SET_DEVICE_CONFIG_RESP_V01 0x0033
#define QMI_CSD_IOCTL_VC_CMD_ENABLE_REQ_V01 0x0034
#define QMI_CSD_IOCTL_VC_CMD_ENABLE_RESP_V01 0x0034
#define QMI_CSD_IOCTL_VC_CMD_DISABLE_REQ_V01 0x0035
#define QMI_CSD_IOCTL_VC_CMD_DISABLE_RESP_V01 0x0035
#define QMI_CSD_IOCTL_VC_CMD_SET_RX_VOLUME_INDEX_REQ_V01 0x0036
#define QMI_CSD_IOCTL_VC_CMD_SET_RX_VOLUME_INDEX_RESP_V01 0x0036
#define QMI_CSD_IOCTL_VC_CMD_SET_MUTE_REQ_V01 0x0037
#define QMI_CSD_IOCTL_VC_CMD_SET_MUTE_RESP_V01 0x0037
#define QMI_CSD_IOCTL_VC_CMD_SET_TX_DTMF_DETECTION_REQ_V01 0x0038
#define QMI_CSD_IOCTL_VC_CMD_SET_TX_DTMF_DETECTION_RESP_V01 0x0038
#define QMI_CSD_IOCTL_VC_TX_DTMF_DETECTED_IND_V01 0x0039
#define QMI_CSD_IOCTL_VC_CMD_SET_UI_PROPERTY_REQ_V01 0x003A
#define QMI_CSD_IOCTL_VC_CMD_SET_UI_PROPERTY_RESP_V01 0x003A
#define QMI_CSD_IOCTL_VC_CMD_GET_UI_PROPERTY_REQ_V01 0x003B
#define QMI_CSD_IOCTL_VC_CMD_GET_UI_PROPERTY_RESP_V01 0x003B
#define QMI_CSD_IOCTL_VC_STATE_IND_V01 0x003C
#define QMI_CSD_IOCTL_VS_CMD_SET_MEDIA_TYPE_REQ_V01 0x003D
#define QMI_CSD_IOCTL_VS_CMD_SET_MEDIA_TYPE_RESP_V01 0x003D
#define QMI_CSD_IOCTL_VS_CMD_SET_MUTE_REQ_V01 0x003E
#define QMI_CSD_IOCTL_VS_CMD_SET_MUTE_RESP_V01 0x003E
#define QMI_CSD_IOCTL_VS_CMD_SET_ENCODER_DTX_MODE_REQ_V01 0x003F
#define QMI_CSD_IOCTL_VS_CMD_SET_ENCODER_DTX_MODE_RESP_V01 0x003F
#define QMI_CSD_IOCTL_VS_CMD_SET_DEC_TIMEWARP_REQ_V01 0x0040
#define QMI_CSD_IOCTL_VS_CMD_SET_DEC_TIMEWARP_RESP_V01 0x0040
#define QMI_CSD_IOCTL_VS_CMD_SET_ENC_MINMAX_RATE_REQ_V01 0x0041
#define QMI_CSD_IOCTL_VS_CMD_SET_ENC_MINMAX_RATE_RESP_V01 0x0041
#define QMI_CSD_IOCTL_VS_CMD_SET_ENC_RATE_MODULATION_REQ_V01 0x0042
#define QMI_CSD_IOCTL_VS_CMD_SET_ENC_RATE_MODULATION_RESP_V01 0x0042
#define QMI_CSD_IOCTL_VS_CMD_VOC_QCELP13K_SET_RATE_REQ_V01 0x0043
#define QMI_CSD_IOCTL_VS_CMD_VOC_QCELP13K_SET_RATE_RESP_V01 0x0043
#define QMI_CSD_IOCTL_VS_CMD_VOC_4GVNB_SET_RATE_REQ_V01 0x0044
#define QMI_CSD_IOCTL_VS_CMD_VOC_4GVNB_SET_RATE_RESP_V01 0x0044
#define QMI_CSD_IOCTL_VS_CMD_VOC_4GVWB_SET_RATE_REQ_V01 0x0045
#define QMI_CSD_IOCTL_VS_CMD_VOC_4GVWB_SET_RATE_RESP_V01 0x0045
#define QMI_CSD_IOCTL_VS_CMD_VOC_AMR_SET_ENC_RATE_REQ_V01 0x0046
#define QMI_CSD_IOCTL_VS_CMD_VOC_AMR_SET_ENC_RATE_RESP_V01 0x0046
#define QMI_CSD_IOCTL_VS_CMD_VOC_AMRWB_SET_ENC_RATE_REQ_V01 0x0047
#define QMI_CSD_IOCTL_VS_CMD_VOC_AMRWB_SET_ENC_RATE_RESP_V01 0x0047
#define QMI_CSD_IOCTL_VS_CMD_SET_DTMF_GENERATION_REQ_V01 0x0048
#define QMI_CSD_IOCTL_VS_CMD_SET_DTMF_GENERATION_RESP_V01 0x0048
#define QMI_CSD_IOCTL_VS_DTMF_GENERATION_ENDED_IND_V01 0x0049
#define QMI_CSD_IOCTL_VS_CMD_SET_RX_DTMF_DETECTION_REQ_V01 0x004A
#define QMI_CSD_IOCTL_VS_CMD_SET_RX_DTMF_DETECTION_RESP_V01 0x004A
#define QMI_CSD_IOCTL_VS_RX_DTMF_DETECTED_IND_V01 0x004B
#define QMI_CSD_IOCTL_VS_CMD_SET_UI_PROPERTY_REQ_V01 0x004C
#define QMI_CSD_IOCTL_VS_CMD_SET_UI_PROPERTY_RESP_V01 0x004C
#define QMI_CSD_IOCTL_VS_CMD_GET_UI_PROPERTY_REQ_V01 0x004D
#define QMI_CSD_IOCTL_VS_CMD_GET_UI_PROPERTY_RESP_V01 0x004D
#define QMI_CSD_IOCTL_VS_CMD_START_RECORD_REQ_V01 0x004E
#define QMI_CSD_IOCTL_VS_CMD_START_RECORD_RESP_V01 0x004E
#define QMI_CSD_IOCTL_VS_CMD_STOP_RECORD_REQ_V01 0x004F
#define QMI_CSD_IOCTL_VS_CMD_STOP_RECORD_RESP_V01 0x004F
#define QMI_CSD_IOCTL_VS_STATE_IND_V01 0x0050
#define QMI_CSD_IOCTL_VS_ENC_BUFFER_IND_V01 0x0051
#define QMI_CSD_IOCTL_VS_DEC_BUFFER_IND_V01 0x0052
#define QMI_CSD_IOCTL_VM_CMD_ATTACH_STREAM_REQ_V01 0x0053
#define QMI_CSD_IOCTL_VM_CMD_ATTACH_STREAM_RESP_V01 0x0053
#define QMI_CSD_IOCTL_VM_CMD_DETACH_STREAM_REQ_V01 0x0054
#define QMI_CSD_IOCTL_VM_CMD_DETACH_STREAM_RESP_V01 0x0054
#define QMI_CSD_IOCTL_VM_CMD_ATTACH_CONTEXT_REQ_V01 0x0055
#define QMI_CSD_IOCTL_VM_CMD_ATTACH_CONTEXT_RESP_V01 0x0055
#define QMI_CSD_IOCTL_VM_CMD_DETACH_CONTEXT_REQ_V01 0x0056
#define QMI_CSD_IOCTL_VM_CMD_DETACH_CONTEXT_RESP_V01 0x0056
#define QMI_CSD_IOCTL_VM_CMD_START_VOICE_REQ_V01 0x0057
#define QMI_CSD_IOCTL_VM_CMD_START_VOICE_RESP_V01 0x0057
#define QMI_CSD_IOCTL_VM_CMD_STOP_VOICE_REQ_V01 0x0058
#define QMI_CSD_IOCTL_VM_CMD_STOP_VOICE_RESP_V01 0x0058
#define QMI_CSD_IOCTL_VM_CMD_SET_NETWORK_REQ_V01 0x0059
#define QMI_CSD_IOCTL_VM_CMD_SET_NETWORK_RESP_V01 0x0059
#define QMI_CSD_IOCTL_VM_CMD_SET_VOICE_TIMING_REQ_V01 0x005A
#define QMI_CSD_IOCTL_VM_CMD_SET_VOICE_TIMING_RESP_V01 0x005A
#define QMI_CSD_IOCTL_VM_CMD_SET_TTY_MODE_REQ_V01 0x005B
#define QMI_CSD_IOCTL_VM_CMD_SET_TTY_MODE_RESP_V01 0x005B
#define QMI_CSD_IOCTL_VM_CMD_SET_WIDEVOICE_REQ_V01 0x005C
#define QMI_CSD_IOCTL_VM_CMD_SET_WIDEVOICE_RESP_V01 0x005C
#define QMI_CSD_OPEN_AUDIO_STREAM_REQ_V01 0x005D
#define QMI_CSD_OPEN_AUDIO_STREAM_RESP_V01 0x005D
#define QMI_CSD_OPEN_AUDIO_CONTEXT_REQ_V01 0x005E
#define QMI_CSD_OPEN_AUDIO_CONTEXT_RESP_V01 0x005E
#define QMI_CSD_AS_CMD_START_SESSION_REQ_V01 0x005F
#define QMI_CSD_AS_CMD_START_SESSION_RESP_V01 0x005F
#define QMI_CSD_AS_CMD_STOP_SESSION_REQ_V01 0x0060
#define QMI_CSD_AS_CMD_STOP_SESSION_RESP_V01 0x0060
#define QMI_CSD_AS_CMD_FLUSH_STREAM_REQ_V01 0x0061
#define QMI_CSD_AS_CMD_FLUSH_STREAM_RESP_V01 0x0061
#define QMI_CSD_AS_CMD_FLUSH_STREAM_TX_REQ_V01 0x0062
#define QMI_CSD_AS_CMD_FLUSH_STREAM_TX_RESP_V01 0x0062
#define QMI_CSD_AS_CMD_GET_VOL_LEVELS_REQ_V01 0x0063
#define QMI_CSD_AS_CMD_GET_VOL_LEVELS_RESP_V01 0x0063
#define QMI_CSD_AS_CMD_GET_DSP_CLK_REQ_V01 0x0064
#define QMI_CSD_AS_CMD_GET_DSP_CLK_RESP_V01 0x0064
#define QMI_CSD_AS_CMD_GET_RENDERED_TIME_REQ_V01 0x0065
#define QMI_CSD_AS_CMD_GET_RENDERED_TIME_RESP_V01 0x0065
#define QMI_CSD_AS_CMD_GET_SESSION_ID_REQ_V01 0x0066
#define QMI_CSD_AS_CMD_GET_SESSION_ID_RESP_V01 0x0066
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_PCM_REQ_V01 0x0067
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_PCM_RESP_V01 0x0067
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_ADPCM_REQ_V01 0x0068
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_ADPCM_RESP_V01 0x0068
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_MIDI_REQ_V01 0x0069
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_MIDI_RESP_V01 0x0069
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_WMAV9_REQ_V01 0x006A
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_WMAV9_RESP_V01 0x006A
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_WMAV10_REQ_V01 0x006B
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_WMAV10_RESP_V01 0x006B
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_AAC_REQ_V01 0x006C
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_AAC_RESP_V01 0x006C
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_G711_REQ_V01 0x006D
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_G711_RESP_V01 0x006D
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_FLAC_REQ_V01 0x006E
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_FLAC_RESP_V01 0x006E
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_VORBIS_REQ_V01 0x006F
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_VORBIS_RESP_V01 0x006F
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_AMRWBPLUS_REQ_V01 0x0070
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_AMRWBPLUS_RESP_V01 0x0070
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_PCM_REQ_V01 0x0071
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_PCM_RESP_V01 0x0071
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AAC_REQ_V01 0x0072
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AAC_RESP_V01 0x0072
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_G711_REQ_V01 0x0073
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_G711_RESP_V01 0x0073
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AMRNB_REQ_V01 0x0074
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AMRNB_RESP_V01 0x0074
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AMRWB_REQ_V01 0x0075
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AMRWB_RESP_V01 0x0075
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_QCELP13K_REQ_V01 0x0076
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_QCELP13K_RESP_V01 0x0076
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRC_REQ_V01 0x0077
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRC_RESP_V01 0x0077
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRCB_REQ_V01 0x0078
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRCB_RESP_V01 0x0078
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRCWB_REQ_V01 0x0079
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRCWB_RESP_V01 0x0079
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_SBC_REQ_V01 0x007A
#define QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_SBC_RESP_V01 0x007A
#define QMI_CSD_AS_CMD_SET_STREAM_EOS_REQ_V01 0x007B
#define QMI_CSD_AS_CMD_SET_STREAM_EOS_RESP_V01 0x007B
#define QMI_CSD_AS_EVT_EOS_IND_V01 0x007B
#define QMI_CSD_AS_CMD_CONFIG_PP_VOL_MASTER_GAIN_REQ_V01 0x007C
#define QMI_CSD_AS_CMD_CONFIG_PP_VOL_MASTER_GAIN_RESP_V01 0x007C
#define QMI_CSD_AS_CMD_CONFIG_PP_VOL_STEREO_GAIN_REQ_V01 0x007D
#define QMI_CSD_AS_CMD_CONFIG_PP_VOL_STEREO_GAIN_RESP_V01 0x007D
#define QMI_CSD_AS_CMD_CONFIG_PP_VOL_MULTICHANNEL_GAIN_REQ_V01 0x007E
#define QMI_CSD_AS_CMD_CONFIG_PP_VOL_MULTICHANNEL_GAIN_RESP_V01 0x007E
#define QMI_CSD_AS_CMD_CONFIG_PP_VOL_MUTE_REQ_V01 0x007F
#define QMI_CSD_AS_CMD_CONFIG_PP_VOL_MUTE_RESP_V01 0x007F
#define QMI_CSD_AS_CMD_CONFIG_PP_EQ_ENABLE_REQ_V01 0x0080
#define QMI_CSD_AS_CMD_CONFIG_PP_EQ_ENABLE_RESP_V01 0x0080
#define QMI_CSD_AS_CMD_CONFIG_PP_QCPR_REQ_V01 0x0081
#define QMI_CSD_AS_CMD_CONFIG_PP_QCPR_RESP_V01 0x0081
#define QMI_CSD_AS_CMD_CONFIG_PP_SPA_REQ_V01 0x0082
#define QMI_CSD_AS_CMD_CONFIG_PP_SPA_RESP_V01 0x0082
#define QMI_CSD_AS_CMD_CONFIG_PP_TSM_REQ_V01 0x0083
#define QMI_CSD_AS_CMD_CONFIG_PP_TSM_RESP_V01 0x0083
#define QMI_CSD_AS_CMD_GET_SPA_DATA_REQ_V01 0x0084
#define QMI_CSD_AS_CMD_GET_SPA_DATA_RESP_V01 0x0084
#define QMI_CSD_AS_EVT_SPA_BUF_READY_IND_V01 0x0084
#define QMI_CSD_AS_CMD_SET_DUAL_MONO_REMAP_REQ_V01 0x0085
#define QMI_CSD_AS_CMD_SET_DUAL_MONO_REMAP_RESP_V01 0x0085
#define QMI_CSD_AS_CMD_ADJUST_SESSION_CLOCK_REQ_V01 0x0086
#define QMI_CSD_AS_CMD_ADJUST_SESSION_CLOCK_RESP_V01 0x0086
#define QMI_CSD_AS_CMD_SET_AAC_SBR_PS_REQ_V01 0x0087
#define QMI_CSD_AS_CMD_SET_AAC_SBR_PS_RESP_V01 0x0087
#define QMI_CSD_AS_CMD_DTMF_CTL_REQ_V01 0x0088
#define QMI_CSD_AS_CMD_DTMF_CTL_RESP_V01 0x0088
#define QMI_CSD_AS_CMD_SET_STREAM_INFO_REQ_V01 0x0089
#define QMI_CSD_AS_CMD_SET_STREAM_INFO_RESP_V01 0x0089
#define QMI_CSD_AS_CMD_GET_RENDERED_BYTE_OFFSET_REQ_V01 0x008A
#define QMI_CSD_AS_CMD_GET_RENDERED_BYTE_OFFSET_RESP_V01 0x008A
#define QMI_CSD_AS_CMD_GET_MIDI_SEQUENCE_ID_REQ_V01 0x008B
#define QMI_CSD_AS_CMD_GET_MIDI_SEQUENCE_ID_RESP_V01 0x008B
#define QMI_CSD_AS_CMD_ENCODER_BIT_RATE_UPDATE_REQ_V01 0x008C
#define QMI_CSD_AS_CMD_ENCODER_BIT_RATE_UPDATE_RESP_V01 0x008C
#define QMI_CSD_AS_CMD_CONFIG_DECODER_MULTI_CHANNEL_WMAV10_REQ_V01 0x008D
#define QMI_CSD_AS_CMD_CONFIG_DECODER_MULTI_CHANNEL_WMAV10_RESP_V01 0x008D
#define QMI_CSD_AS_CMD_CONFIG_DECODER_MULTI_CHANNEL_EAC3_REQ_V01 0x008E
#define QMI_CSD_AS_CMD_CONFIG_DECODER_MULTI_CHANNEL_EAC3_RESP_V01 0x008E
#define QMI_CSD_AS_EVT_SR_CM_CHANGE_IND_V01 0x008F
#define QMI_CSD_AC_CMD_AS_ATTACH_REQ_V01 0x0090
#define QMI_CSD_AC_CMD_AS_ATTACH_RESP_V01 0x0090
#define QMI_CSD_AC_CMD_AS_DETACH_REQ_V01 0x0091
#define QMI_CSD_AC_CMD_AS_DETACH_RESP_V01 0x0091
#define QMI_CSD_AC_CMD_SET_DEVICE_REQ_V01 0x0092
#define QMI_CSD_AC_CMD_SET_DEVICE_RESP_V01 0x0092
#define QMI_CSD_AC_CMD_ENABLE_REQ_V01 0x0093
#define QMI_CSD_AC_CMD_ENABLE_RESP_V01 0x0093
#define QMI_CSD_AC_CMD_DISABLE_REQ_V01 0x0094
#define QMI_CSD_AC_CMD_DISABLE_RESP_V01 0x0094
#define QMI_CSD_AC_CMD_CONFIG_PP_VOL_MASTER_GAIN_REQ_V01 0x0095
#define QMI_CSD_AC_CMD_CONFIG_PP_VOL_MASTER_GAIN_RESP_V01 0x0095
#define QMI_CSD_AC_CMD_CONFIG_PP_VOL_STEREO_GAIN_REQ_V01 0x0096
#define QMI_CSD_AC_CMD_CONFIG_PP_VOL_STEREO_GAIN_RESP_V01 0x0096
#define QMI_CSD_AC_CMD_CONFIG_PP_VOL_MULTICHANNEL_GAIN_REQ_V01 0x0097
#define QMI_CSD_AC_CMD_CONFIG_PP_VOL_MULTICHANNEL_GAIN_RESP_V01 0x0097
#define QMI_CSD_AC_CMD_CONFIG_PP_VOL_MUTE_REQ_V01 0x0098
#define QMI_CSD_AC_CMD_CONFIG_PP_VOL_MUTE_RESP_V01 0x0098
#define QMI_CSD_AC_CMD_CONFIG_PP_EQ_ENABLE_REQ_V01 0x0099
#define QMI_CSD_AC_CMD_CONFIG_PP_EQ_ENABLE_RESP_V01 0x0099
#define QMI_CSD_AC_CMD_CONFIG_PP_QCPR_REQ_V01 0x009A
#define QMI_CSD_AC_CMD_CONFIG_PP_QCPR_RESP_V01 0x009A
#define QMI_CSD_AC_CMD_CONFIG_PP_SPA_REQ_V01 0x009B
#define QMI_CSD_AC_CMD_CONFIG_PP_SPA_RESP_V01 0x009B
#define QMI_CSD_AC_CMD_GET_SPA_DATA_REQ_V01 0x009C
#define QMI_CSD_AC_CMD_GET_SPA_DATA_RESP_V01 0x009C
#define QMI_CSD_AC_EVT_SPA_BUF_READY_IND_V01 0x009C
#define QMI_CSD_AC_CMD_CONFIG_MULTI_CHANNEL_REQ_V01 0x009D
#define QMI_CSD_AC_CMD_CONFIG_MULTI_CHANNEL_RESP_V01 0x009D
#define QMI_CSD_IOCTL_DEV_CMD_CONNECT_DEVICE_REQ_V01 0x009E
#define QMI_CSD_IOCTL_DEV_CMD_CONNECT_DEVICE_RESP_V01 0x009E
#define QMI_CSD_IOCTL_VC_CMD_SET_NUMBER_OF_VOLUME_STEPS_REQ_V01 0x009F
#define QMI_CSD_IOCTL_VC_CMD_SET_NUMBER_OF_VOLUME_STEPS_RESP_V01 0x009F
#define QMI_CSD_IOCTL_VC_CMD_SET_RX_VOLUME_STEP_REQ_V01 0x00A0
#define QMI_CSD_IOCTL_VC_CMD_SET_RX_VOLUME_STEP_RESP_V01 0x00A0
#define QMI_CSD_IOCTL_VS_CMD_START_PLAYBACK_REQ_V01 0x00A1
#define QMI_CSD_IOCTL_VS_CMD_START_PLAYBACK_RESP_V01 0x00A1
#define QMI_CSD_IOCTL_VS_CMD_STOP_PLAYBACK_REQ_V01 0x00A2
#define QMI_CSD_IOCTL_VS_CMD_STOP_PLAYBACK_RESP_V01 0x00A2
#define QMI_CSD_IOCTL_VM_CMD_STANDBY_VOICE_REQ_V01 0x00A3
#define QMI_CSD_IOCTL_VM_CMD_STANDBY_VOICE_RESP_V01 0x00A3
#define QMI_CSD_IOCTL_DEV_CMD_AANC_CONTROL_REQ_V01 0x00A4
#define QMI_CSD_IOCTL_DEV_CMD_AANC_CONTROL_RESP_V01 0x00A4
#define QMI_CSD_IOCTL_VM_CMD_PAUSE_VOICE_REQ_V01 0x00A5
#define QMI_CSD_IOCTL_VM_CMD_PAUSE_VOICE_RESP_V01 0x00A5
#define QMI_CSD_IOCTL_DEV_CMD_RESTART_REQ_V01 0x00A6
#define QMI_CSD_IOCTL_DEV_CMD_RESTART_RESP_V01 0x00A6
#define QMI_CSD_IOCTL_VC_CMD_SET_CAL_FEATURE_ID_REQ_V01 0x00A7
#define QMI_CSD_IOCTL_VC_CMD_SET_CAL_FEATURE_ID_RESP_V01 0x00A7
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro csd_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type csd_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define csd_get_service_object_v01( ) \
          csd_get_service_object_internal_v01( \
            CSD_V01_IDL_MAJOR_VERS, CSD_V01_IDL_MINOR_VERS, \
            CSD_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

