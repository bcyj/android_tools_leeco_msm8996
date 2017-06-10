#ifndef _RTC_Q5_INTF_H_
#define _RTC_Q5_INTF_H_
/*============================================================================

FILE:       rtc_q5_intf.h

DESCRIPTION: This header file contains all the definitions necessary for RTC on 8x25

PUBLIC CLASSES:  Not Applicable

INITIALIZATION AND SEQUENCING REQUIREMENTS:  N/A

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

typedef struct __Interface_size_map_type
{   uint32_t interface_id;
    uint32_t interface_size;
}interface_size_map_type;

/* error codes from audio driver */
typedef enum {
    RTC_SUCCESS,
    RTC_ERR_INVALID_DEVICE,
    RTC_ERR_DEVICE_INACTIVE,
    RTC_ERR_INVALID_ABID,
    RTC_DSP_FAILURE,
    RTC_DSP_FEATURE_NOT_AVAILABLE,
    RTC_ERR_INVALID_LEN,
    RTC_ERR_UNKNOWN_FAILURE,
    RTC_PENDING_RESPONSE,
}rtc_errorcodes;

#define INF(x) x,x##_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#ifdef _ENABLE_QC_MSG_LOG_
    #ifdef _ANDROID_
        #include <utils/Log.h>
        #define DEBUG_PRINT_ERROR LOGE
        #define DEBUG_PRINT LOGI
        #define DEBUG_DETAIL LOGV
    #else
        #define DEBUG_PRINT_ERROR printf
        #define DEBUG_PRINT printf
        #define DEBUG_DETAIL printf
        #define LOGE printf
    #endif // _ANDROID_
#else
    #define DEBUG_PRINT_ERROR
    #define DEBUG_PRINT
    #define DEBUG_DETAIL
    #define LOGE
#endif // _ENABLE_QC_MSG_LOG_


// These macros are taken from acdb_common.h

/*------------------------------------------
** Algorithm Aspect IDs
*-------------------------------------------*/
#define IID_ENABLE_FLAG                 0x0108b6b9

#define IID_ENABLE_FLAG_SIZE                        1
#define IID_ECHO_CANCELLER_VERSION_SIZE  2
#define IID_ECHO_CANCELLER_MODE_SIZE    2
#define IID_ECHO_CANCELLER_NOISE_SUPPRESSOR_ENABLE_SIZE 1
#define IID_ECHO_CANCELLER_PARAMETERS_SIZE 32
#define IID_ECHO_CANCELLER_NEXTGEN_NB_PARAMETERS_SIZE (38*2)
#define IID_ECHO_CANCELLER_NEXTGEN_WB_PARAMETERS_SIZE (38*2)
#define IID_FLUENCE3_PARAMETERS_SIZE 486 /*FU*/
#define IID_AFE_VOLUME_CONTROL_SIZE 6
#define IID_GAIN_SIZE 2
#define IID_VOICE_FIR_FILTER_SIZE 14
#define IID_VOICE_IIR_FILTER_SIZE 114
#define IID_RX_DBM_OFFSET_SIZE 2
#define IID_AGC2_SIZE 38
#define IID_AVC2_SIZE 82
#define IID_RVE_PARAM_SIZE 78

#define IID_WIDE_VOICE_PARAM_SIZE 60
#define IID_ST_GAIN_PARAMS_SIZE 4
#define IID_SLOPE_IIR_CODEC_FORMAT_REG_SIZE 2
#define IID_FENS_PARAM_SIZE 94

#define IID_STF_COEFF_SIZE 100
#define IID_AUDIO_IIR_COEFF_SIZE 100
#define IID_MBADRC_PARAMETERS_SIZE 8
#define IID_MBADRC_EXT_BUFF_SIZE 392
#define IID_MBADRC_BAND_CONFIG_SIZE 100
#define IID_QAFX_PARAMETERS_SIZE 2
#define IID_QCONCERT_PLUS_PARAMETERS_SIZE 4
#define IID_AUDIO_AGC_PARAMETERS_SIZE 42
#define IID_AUDIO_RTC_AGC_PARAMETERS_SIZE 46
#define IID_AUDIO_RTC_TX_IIR_COEFF_SIZE 100

#define IID_NS_PARAMETERS_SIZE 14

#define IID_SLOW_TALK_PARAM_SIZE 80

#define AV_VOLUME_TABLE_RX_MAX_SIZE 202

#define IID_VOICE_DTMF_DETECTION_RX_PARAM_SIZE 38
#define IID_VOICE_DTMF_DETECTION_TX_PARAM_SIZE 32

#define IID_AUDIO_CALIBRATION_GAIN_RX_SIZE 4
#define IID_AUDIO_CALIBRATION_GAIN_TX_SIZE 4
#define IID_VOICE_GAIN_RX_SIZE 2
#define IID_VOICE_GAIN_TX_SIZE 2

#define IID_RTC_MBADRC_PARAMETERS_SIZE 508
#define IID_AUDIO_RTC_VOLUME_PAN_PARAMETERS_SIZE 4
#define IID_AUDIO_RTC_SPA_PARAMETERS_SIZE 68
#define IID_AUDIO_RTC_EQUALIZER_PARAMETERS_SIZE 268

#define IID_AUDIO_VOLUME_TABLE_RX_SIZE 34
#define IID_VOICE_VOLUME_TABLE_RX_SIZE 34
#define IID_AUDIO_PBE_RX_ENABLE_FLAG_SIZE 2
#define IID_PBE_CONFIG_PARAMETERS_SIZE 228
#define IID_FE_PCM_FILTER_PARAM_SIZE 222
#define IID_VOICE_RX_PBE_PARAM_SIZE 230

#define IID_ECHO_CANCELLER_VERSION  0x00010042
#define IID_ECHO_CANCELLER_MODE     0x00010043
#define IID_ECHO_CANCELLER_NOISE_SUPPRESSOR_ENABLE 0x00010044
#define IID_ECHO_CANCELLER_PARAMETERS 0x00010045
#define IID_ECHO_CANCELLER_NEXTGEN_NB_PARAMETERS 0x00010046
#define IID_ECHO_CANCELLER_NEXTGEN_WB_PARAMETERS 0x00010047

#define IID_FLUENCE3_PARAMETERS 0x000111B0

#define IID_AFE_VOLUME_CONTROL 0x00010049
#define IID_GAIN 0x0001004A
#define IID_VOICE_FIR_FILTER 0x0001004B
#define IID_VOICE_IIR_FILTER 0x0001004C
#define IID_RVE_PARAM 0x00010086
/*#define IID_RX_DBM_OFFSET 0x0001004D*/
#define IID_AGC2 0x000100B0
#define IID_AVC2 0x000100B1
#define IID_WIDE_VOICE_PARAM 0x0001008F
#define IID_ST_GAIN_PARAMS 0x00010091
#define IID_SLOPE_IIR_CODEC_FORMAT_REG 0x00010093
#define IID_FENS_PARAM 0x00010097

#define ABID_SLOW_TALK 0x000100AD
#define IID_SLOW_TALK_PARAM 0x000100AE

#define ABID_SIDETONE_GAIN 0x00010095
#define ABID_TX_VOICE_GAIN 0x00010051
#define ABID_TX_DTMF_GAIN 0x00010052
#define ABID_CODEC_TX_GAIN 0x00010053
#define ABID_HSSD 0x00010054
#define ABID_TX_AGC2 0x000100B6
#define ABID_TX_VOICE_FIR 0x00010056
#define ABID_TX_VOICE_IIR 0x00010057
#define ABID_ECHO_CANCELLER 0x00010058
#define ABID_ECHO_CANCELLER_NB_LVHF 0x00010059
#define ABID_ECHO_CANCELLER_WB_LVHF 0x0001005A
#define ABID_FLUENCE3 0x000111AF
#define ABID_CODEC_RX_GAIN 0x0001005C
#define ABID_RX_DBM_OFFSET 0x0001005D
#define ABID_RX_AGC2 0x000100B4
#define ABID_AVC2 0x000100B5
#define ABID_RX_VOICE_FIR 0x00010060
#define ABID_RX_VOICE_IIR 0x00010061
#define ABID_AFE_VOL_CTRL 0x00010067
#define ABID_RVE_PARAM_RX 0x00010087
#define ABID_WIDE_VOICE 0x00010090
#define ABID_SLOPE_IIR 0x00010092
#define ABID_FENS_RX 0x00010096

#define ABID_SLOW_TALK 0x000100AD
#define IID_SLOW_TALK_PARAM 0x000100AE

#define ABID_DTMF_VOLUME_TABLE_2 0x00011269
#define IID_DTMF_VOLUME_LEVEL_2 0x0001126A
#define IID_DTMF_VOLUME_TABLE_2 0x00011266
#define IID_INCALL_DTMF_VOLUME_TABLE_2 0x00011267

#define IID_DTMF_VOLUME_LEVEL_2_SIZE 2
#define IID_DTMF_VOLUME_TABLE_2_SIZE 200
#define IID_INCALL_DTMF_VOLUME_TABLE_2_SIZE 200

#define ABID_VOICE_DTMF_DETECTION_RX 0x00011169
#define ABID_VOICE_DTMF_DETECTION_TX 0x0001116A
#define IID_VOICE_DTMF_DETECTION_RX_PARAM 0x0001116B
#define IID_VOICE_DTMF_DETECTION_TX_PARAM 0x0001116C

#define ABID_AUDIO_CALIBRATION_GAIN_TX 0x00011149
#define ABID_AUDIO_CALIBRATION_GAIN_RX 0x00011162
#define IID_AUDIO_CALIBRATION_GAIN_TX 0x00011171
#define IID_AUDIO_CALIBRATION_GAIN_RX 0x00011163

#define ABID_VOICE_GAIN_RX 0x00011172
#define IID_VOICE_GAIN_RX 0x00011173

#define ABID_VOICE_GAIN_TX 0x00011175
#define IID_VOICE_GAIN_TX 0x00011176

#define ABID_AUDIO_VOLUME_TABLE_RX 0x0001117B
#define ABID_VOICE_VOLUME_TABLE_RX 0x00011177
#define ABID_VOICE_FE_PCM_FILTER 0x000111A1
#define ABID_AUDIO_PBE_RX 0x00011197

#define IID_AUDIO_VOLUME_TABLE_RX 0x0001117C
#define IID_VOICE_VOLUME_TABLE_RX 0x00011178
#define IID_AUDIO_PBE_RX_ENABLE_FLAG 0x00011199
#define IID_PBE_CONFIG_PARAMETERS 0x00011198
#define IID_FE_PCM_FILTER_PARAM 0x000111A0

#define ABID_VOICE_RX_PBE 0x000111AC
#define IID_VOICE_RX_PBE_PARAM 0x000111AD

#define ABID_AUDIO_LPA_ENABLE_FLAG_LIST 0x000111E3
#define IID_AUDIO_LPA_ENABLE_FLAG_LIST 0x000111E2
#define IID_AUDIO_LPA_ENABLE_FLAG_LIST_SIZE 22


/******************AUDIO IDS*******************/
#define ABID_AUDIO_AGC_TX 0x00010068
#define ABID_AUDIO_NS_TX 0x00010069
#define ABID_VOICE_NS 0x0001006A
#define ABID_AUDIO_IIR_TX 0x0001006B
#define ABID_AUDIO_IIR_RX 0x0001006C
#define ABID_AUDIO_MBADRC_RX 0x0001006E
#define ABID_AUDIO_QAFX_RX 0x0001006F
#define ABID_AUDIO_QCONCERT_PLUS_RX 0x0001009B
#define ABID_AUDIO_STF_RX 0x00010071

#define ABID_AUDIO_RTC_MBADRC_RX 0x0001118A
#define ABID_AUDIO_RTC_VOLUME_PAN_RX 0x0001118C
#define ABID_AUDIO_RTC_SPA 0x0001118E
#define ABID_AUDIO_RTC_EQUALIZER_PARAMETERS 0x0001119F

#define IID_AUDIO_AGC_PARAMETERS 0x0001007E
#define IID_NS_PARAMETERS 0x00010072
#define IID_AUDIO_IIR_COEFF 0x00010073
#define IID_MBADRC_EXT_BUFF 0x00010075
#define IID_MBADRC_BAND_CONFIG 0x00010076
#define IID_MBADRC_PARAMETERS 0x00010077
#define IID_QAFX_PARAMETERS 0x00010079
#define IID_QCONCERT_PLUS_PARAMETERS 0x0001009C
#define IID_STF_COEFF 0x0001007B

#define IID_RTC_MBADRC_PARAMETERS 0x0001118B
#define IID_AUDIO_RTC_VOLUME_PAN_PARAMETERS 0x0001118D
#define IID_AUDIO_RTC_SPA_PARAMETERS 0x0001118F
#define IID_AUDIO_RTC_EQUALIZER_PARAMETERS 0x0001119E
#define IID_AUDIO_RTC_AGC_PARAMETERS 0x000111A7
#define IID_AUDIO_RTC_TX_IIR_COEFF 0x000111A8

/**
 * FUNCTION : rtc_get_q5_cal_data
 *
 * DESCRIPTION : Get real time calibration data
 *
 * DEPENDENCIES : DSP must be active, Audcal should be initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void rtc_get_q5_cal_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

/**
 * FUNCTION : rtc_set_q5_cal_data
 *
 * DESCRIPTION : Set real time calibration data
 *
 * DEPENDENCIES : DSP must be active, Audcal should be initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void rtc_set_q5_cal_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

/**
 * FUNCTION : rtc_query_all_active_devices
 *
 * DESCRIPTION : Query for all the active devices
 *
 * DEPENDENCIES : DSP must be active, Audcal should be initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void rtc_query_all_active_devices (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );
#endif /* _RTC_Q5_INTF_H_ */

