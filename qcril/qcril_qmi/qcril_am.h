/******************************************************************************
  @file    qcril_am.h
  @brief   qcril qmi - Audio Management

  DESCRIPTION
    Provides Audio Management APIs.

  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef QCRIL_AM_H
#define QCRIL_AM_H

#include <comdef.h>
#include <ril.h>

#define QCRIL_VOICE_MODEM_INDEX "persist.radio.voice.modem.index"

#define QCRIL_MAX_NUM_VOICE_MODEM 2

// TODO: did not find enums in audio header file. better to use them to assign following enums
typedef enum
{
    QCRIL_AM_CALL_STATE_MIN = 0,
    QCRIL_AM_CALL_STATE_INVALID = QCRIL_AM_CALL_STATE_MIN,
    QCRIL_AM_CALL_STATE_INACTIVE,
    QCRIL_AM_CALL_STATE_ACTIVE,
    QCRIL_AM_CALL_STATE_HOLD,
    QCRIL_AM_CALL_STATE_LOCAL_HOLD,
    QCRIL_AM_CALL_STATE_MAX
} qcril_am_call_state_type;

typedef enum
{
    QCRIL_AM_VS_MIN = 0,
    QCRIL_AM_VS_INVALID = QCRIL_AM_VS_MIN,
    QCRIL_AM_VS_IMS,
    QCRIL_AM_VS_VOICE,
    QCRIL_AM_VS_IMS_WLAN,
    QCRIL_AM_VS_MAX
} qcril_am_vs_type;

typedef enum
{
    QCRIL_AM_EVENT_MIN = 0,
    QCRIL_AM_EVENT_INVALID = QCRIL_AM_EVENT_MIN,
    QCRIL_AM_EVENT_IMS_ANSWER,
    QCRIL_AM_EVENT_IMS_ANSWER_FAIL,
    QCRIL_AM_EVENT_VOICE_ANSWER,
    QCRIL_AM_EVENT_VOICE_ANSWER_FAIL,
    QCRIL_AM_EVENT_SWITCH_CALL,
    QCRIL_AM_EVENT_SWITCH_CALL_FAIL,
    QCRIL_AM_EVENT_CALL_STATE_CHANGED,
    QCRIL_AM_EVENT_SRVCC_START,
    QCRIL_AM_EVENT_SRVCC_COMPLETE,
    QCRIL_AM_EVENT_SRVCC_FAIL,
    QCRIL_AM_EVENT_SRVCC_CANCEL,
    QCRIL_AM_EVENT_IMS_SRV_CHANGED,
    QCRIL_AM_EVENT_IMS_HANDOVER,
    QCRIL_AM_EVENT_AUDIO_RAT_CHANGED,
    QCRIL_AM_EVENT_LCH,
    QCRIL_AM_EVENT_UNLCH,
    QCRIL_AM_EVENT_INTER_RIL_CALL_STATE, // come with message qcril_am_inter_rild_msg_type
    QCRIL_AM_EVENT_MEDIA_SERVER_DIED,
    QCRIL_AM_EVENT_MAX
} qcril_am_event_type;

typedef struct
{
    uint32 voice_vsid;
    qcril_am_call_state_type call_state;
} qcril_am_audio_api_param_type;

typedef struct
{
    uint8 rild_id;
    qcril_am_audio_api_param_type param;
} qcril_am_inter_rild_msg_type;

#ifdef  __cplusplus
extern "C" {
#endif

/***************************************************************************************************
    @function
    qcril_am_pre_init

    @brief
    Initializes audio manager mutex, which will not be changed throughout the life time.
***************************************************************************************************/
void qcril_am_pre_init();

/***************************************************************************************************
    @function
    qcril_am_post_cleanup

    @brief
    Cleans up audio manager when ril is released.
***************************************************************************************************/
void qcril_am_post_cleanup();

/***************************************************************************************************
    @function
    qcril_am_state_reset

    @brief
    Resets audio manager state in start and SSR event.
***************************************************************************************************/
void qcril_am_state_reset();

/***************************************************************************************************
    @function
    qcril_am_set_call_audio_driver

    @brief
    Sets Voice call Driver based on voice subsystem type and call state.

    @param[in]
        vs_type
            voice subsystem type which is requested
        call_state
            call state of that voice subsystem

    @retval
    QCRIL_AM_ERR_NONE if function is successful, or error code otherwise.
***************************************************************************************************/
RIL_Errno qcril_am_set_call_audio_driver
(
    qcril_am_vs_type vs_type,
    qcril_am_call_state_type call_state
);

/***************************************************************************************************
    @function
    qcril_am_set_vsid

    @brief
    Sets vsid of a voice subsystem.
    For now, it is expected that only QCRIL_AM_VS_VOICE vsid will be set once getting corresponding
    information from NAS. QCRIL_AM_VS_IMS vsid will be hard coded.

    @param[in]
        vs_type
            voice subsystem type which is requested
        vsid
            voice subsystem id of such vs_type

    @retval
    QCRIL_AM_ERR_NONE if function is successful, or error code otherwise.
***************************************************************************************************/
RIL_Errno qcril_am_set_vsid(qcril_am_vs_type vs_type, uint32 vsid);

/***************************************************************************************************
    @function
    qcril_am_handle_event

    @brief
    Handles the events which possible audio actions may need. Once user posts events here,
    this function will take care of the necessary audio state transition.

    @param[in]
        event
            new event posted to handle

    @retval
    QCRIL_AM_ERR_NONE if function is successful, or error code otherwise.
***************************************************************************************************/
RIL_Errno qcril_am_handle_event(qcril_am_event_type event, const void *data);

void qcril_am_set_emergency_on_lte(boolean is_lte);
void qcril_am_reset_emergency_on_lte();

#ifdef  __cplusplus
}
#endif

#endif
