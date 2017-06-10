/******************************************************************************
  @file    qmi_cat_srvc.c
  @brief   The QMI CAT service layer.

  $Id: //depot/asic/sandbox/users/anilt/QMI_UIM/qmi/common/qmi_cat_srvc.c#2 $

  DESCRIPTION
  QMI CAT service routines.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_cat_srvc_init_client() needs to be called before sending or receiving of any
  QoS service messages

  ---------------------------------------------------------------------------
  Copyright (c) 2008,2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_i.h"
#include "qmi_service.h"
#include "qmi_cat_srvc.h"
#include "qmi_util.h"


#define QMI_CAT_STD_MSG_SIZE                                      QMI_MAX_STD_MSG_SIZE
#define QMI_CAT_MAX_APDU_SIZE                                     255
#define QMI_CAT_MAX_PARAM_BUF_SIZE                                QMI_CAT_MAX_APDU_SIZE + \
                                                                  sizeof(uint32_t) + sizeof(uint16_t)

/* Indication message IDs */
#define QMI_CAT_EVENT_REPORT_IND_MSG_ID                           (0x0001)
#define QMI_CAT_SCWS_OPEN_CHANNEL_IND_MSG_ID                      (0x0027)
#define QMI_CAT_SCWS_CLOSE_CHANNEL_IND_MSG_ID                     (0x0028)
#define QMI_CAT_SCWS_SEND_DATA_IND_MSG_ID                         (0x0029)

/* Request Response Message IDs */
#define QMI_CAT_RESET_MSG_ID                                      (0x0000)
#define QMI_CAT_SET_EVENT_REPORT_MSG_ID                           (0x0001)
#define QMI_CAT_SEND_TR_MSG_ID                                    (0x0021)
#define QMI_CAT_SEND_ENVELOPE_MSG_ID                              (0x0022)
#define QMI_CAT_EVENT_CONFIRMATION_MSG_ID                         (0x0026)
#define QMI_CAT_SCWS_OPEN_CHANNEL_MSG_ID                          (0x0027)
#define QMI_CAT_SCWS_CLOSE_CHANNEL_MSG_ID                         (0x0028)
#define QMI_CAT_SCWS_SEND_DATA_MSG_ID                             (0x0029)
#define QMI_CAT_SCWS_DATA_AVAILABLE_MSG_ID                        (0X002A)
#define QMI_CAT_SCWS_CHANNEL_STATUS_MSG_ID                        (0X002B)

/* Request TLV IDs */
#define QMI_CAT_MANDATORY_1_TLV_ID                                (0x01)
#define QMI_CAT_MANDATORY_2_TLV_ID                                (0x02)
#define QMI_CAT_MANDATORY_3_TLV_ID                                (0x03)
#define QMI_CAT_OPTIONAL_1_TLV_ID                                 (0x10)
#define QMI_CAT_OPTIONAL_2_TLV_ID                                 (0x11)
#define QMI_CAT_OPTIONAL_3_TLV_ID                                 (0x12)

/* Event report indication TLV IDs */
#define QMI_CAT_EVENT_REPORT_DISPLAY_TEXT                         (0x10)
#define QMI_CAT_EVENT_REPORT_GET_INKEY                            (0x11)
#define QMI_CAT_EVENT_REPORT_GET_INPUT_KEY                        (0x12)
#define QMI_CAT_EVENT_REPORT_SETUP_MENU                           (0x13)
#define QMI_CAT_EVENT_REPORT_SELECT_ITEM                          (0x14)
#define QMI_CAT_EVENT_REPORT_SETUP_EVENT_LIST                     (0x16)
#define QMI_CAT_EVENT_REPORT_SETUP_IDLE_MODE_TEXT                 (0x17)
#define QMI_CAT_EVENT_REPORT_LANGUAGE_NOTIFICATION                (0x18)
#define QMI_CAT_EVENT_REPORT_PLAY_TONE                            (0x47)
#define QMI_CAT_EVENT_REPORT_SETUP_CALL                           (0x48)
#define QMI_CAT_EVENT_REPORT_SEND_DTMF                            (0x49)
#define QMI_CAT_EVENT_REPORT_LAUNCH_BROWSER                       (0x4A)
#define QMI_CAT_EVENT_REPORT_SEND_SMS                             (0x4B)
#define QMI_CAT_EVENT_REPORT_SEND_SS                              (0x4C)
#define QMI_CAT_EVENT_REPORT_SEND_USSD                            (0x4D)
#define QMI_CAT_EVENT_REPORT_PROVIDE_LOCAL_INFO                   (0x4E)
#define QMI_CAT_EVENT_REPORT_SETUP_EVENT_LIST_RAW                 (0x4F)
#define QMI_CAT_EVENT_REPORT_OPEN_CHANNEL                         (0x51)
#define QMI_CAT_EVENT_REPORT_CLOSE_CHANNEL                        (0x52)
#define QMI_CAT_EVENT_REPORT_SEND_DATA                            (0x53)
#define QMI_CAT_EVENT_REPORT_RECEIVE_DATA                         (0x54)
#define QMI_CAT_EVENT_REPORT_SLOT                                 (0x50)
#define QMI_CAT_EVENT_REPORT_REFRESH_ALPHA                        (0x6A)


#define QMI_CAT_EVENT_REPORT_END_PROACTIVE_SESSION                (0x1A)

/* Bitmask for Set Event Report */
#define QMI_CAT_SET_EVENT_REPORT_DISPLAY_TEXT_MASK                (0x00000001)
#define QMI_CAT_SET_EVENT_REPORT_GET_INKEY                        (0x00000002)
#define QMI_CAT_SET_EVENT_REPORT_GET_INPUT                        (0x00000004)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_MENU                       (0x00000008)
#define QMI_CAT_SET_EVENT_REPORT_SELECT_ITEM                      (0x00000010)
#define QMI_CAT_SET_EVENT_REPORT_SEND_SMS                         (0x00000020)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_USER_ACTIVITY        (0x00000040)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_IDLE_SCREEN_NOTIFY   (0x00000080)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_LANGUAGE_SEL_NOTIFY  (0x00000100)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_IDLE_MODE_TEXT             (0x00000200)
#define QMI_CAT_SET_EVENT_REPORT_LANGUAGE_NOTIFICATION            (0x00000400)
#define QMI_CAT_SET_EVENT_REPORT_REFRESH                          (0x00000800)
#define QMI_CAT_SET_EVENT_REPORT_END_PROACTIVE_SESSION            (0x00001000)
#define QMI_CAT_SET_EVENT_REPORT_PLAY_TONE                        (0x00002000)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_CALL                       (0x00004000)
#define QMI_CAT_SET_EVENT_REPORT_SEND_DTMF                        (0x00008000)
#define QMI_CAT_SET_EVENT_REPORT_LAUNCH_BROWSER                   (0x00010000)
#define QMI_CAT_SET_EVENT_REPORT_SEND_SS                          (0x00020000)
#define QMI_CAT_SET_EVENT_REPORT_SEND_USSD                        (0x00040000)
#define QMI_CAT_SET_EVENT_REPORT_PROVIDE_LOCAL_INFO_LANG          (0x00080000)
#define QMI_CAT_SET_EVENT_REPORT_BIP                              (0x00100000)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_BROWSER_TERM         (0x00200000)
#define QMI_CAT_SET_EVENT_REPORT_PROVIDE_LOCAL_INFO_TIME          (0x00400000)
#define QMI_CAT_SET_EVENT_REPORT_SCWS                             (0x00800000)

#define QMI_CAT_EVENT_REPORT_UIM_REF_ID_SIZE                      (4)
#define QMI_CAT_EVENT_REPORT_UIM_CMD_LEN_SIZE                     (2)
#define QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE                          (QMI_CAT_EVENT_REPORT_UIM_REF_ID_SIZE + \
                                                                   QMI_CAT_EVENT_REPORT_UIM_CMD_LEN_SIZE)

#define QMI_CAT_SCWS_CH_ID_SIZE                                   (4)
#define QMI_CAT_SCWS_TOTAL_PACKETS_SIZE                           (1)
#define QMI_CAT_SCWS_CURRENT_PACKET_SIZE                          (1)
#define QMI_CAT_SCWS_DATA_LEN_SIZE                                (2)
#define QMI_CAT_SCWS_SEND_DATA_IND_STATIC_TLV_SIZE                (QMI_CAT_SCWS_CH_ID_SIZE +              \
                                                                   QMI_CAT_SCWS_TOTAL_PACKETS_SIZE +      \
                                                                   QMI_CAT_SCWS_CURRENT_PACKET_SIZE +     \
                                                                   QMI_CAT_SCWS_DATA_LEN_SIZE)

/* Macro for safe free */
#define QMI_CAT_FREE_IF_NOT_NULL(x)   if (x != NULL)   \
                                      {                \
                                        free(x);       \
                                        x = NULL;      \
                                      }

/*---------------------------------------------------------------------------
  Global variables
---------------------------------------------------------------------------*/
static int cat_service_initialized = FALSE;


/*---------------------------------------------------------------------------
  Local functions
---------------------------------------------------------------------------*/

/*===========================================================================
  FUNCTION  qmi_cat_handle_send_envelope_rsp
===========================================================================*/
static int qmi_cat_handle_send_envelope_rsp
(
  unsigned char                        * rx_msg,
  int                                    rx_msg_size,
  qmi_cat_send_envelope_rsp_type       * rsp_data
)
{
  unsigned long   type;
  unsigned long   length;
  unsigned char * value_ptr;

  /* Loop to read the TLVs, if present */
  while (rx_msg_size > 0)
  {
    /* Read a TLV */
    if (qmi_util_read_std_tlv (&rx_msg,
                               &rx_msg_size,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    QMI_DEBUG_MSG_1("qmi_cat_handle_send_envelope_rsp, type: 0x%x", type);

    /* Process the Envelope response TLV, ignore any other */
    if (type == QMI_CAT_OPTIONAL_1_TLV_ID)
    {
      READ_8_BIT_VAL (value_ptr, rsp_data->sw1);
      READ_8_BIT_VAL (value_ptr, rsp_data->sw2);
      READ_8_BIT_VAL (value_ptr, rsp_data->env_resp_data_len);
      if(rsp_data->env_resp_data_len > 0)
      {
        /* Allocate memory for the envelope response. Note that since this API can be
           called only synchronously, Client needs to free this up */
        rsp_data->env_resp_data_ptr = (unsigned char *) malloc(rsp_data->env_resp_data_len);
        if (rsp_data->env_resp_data_ptr == NULL)
        {
          return QMI_INTERNAL_ERR;
        }
        memcpy (rsp_data->env_resp_data_ptr, (void *)value_ptr, rsp_data->env_resp_data_len);
      }
    }
  } /* while */

  return QMI_NO_ERR;
} /* qmi_cat_handle_send_envelope_rsp */


/*===========================================================================
  FUNCTION  qmi_cat_prepare_indication_callback
===========================================================================*/
static int qmi_cat_prepare_indication_callback
(
  unsigned long                   msg_id,
  unsigned long                   type,
  unsigned long                   length,
  unsigned char                 * value_ptr,
  qmi_cat_indication_id_type    * ind_id,
  qmi_cat_indication_data_type  * ind_data,
  qmi_cat_slot_id_type          * slot_id
)
{
  uint8_t  temp_uint8  = 0;
  uint32_t temp_uint32 = 0;

  if (value_ptr == NULL || ind_id    == NULL || ind_data  == NULL)
  {
    return 1;
  }

  QMI_DEBUG_MSG_0("qmi_cat_prepare_indication_callback: Enter");

  /* Note on the free of internal pointer in the below messages:
     The reason this is done is because if there is ever a case where multiple TLVs
     are read, then the memory allocated for the previous TLV is silently leaked */

  switch (msg_id)
  {
    case QMI_CAT_EVENT_REPORT_IND_MSG_ID:
      switch(type)
      {
        /* Handling all these events in the same way */
        case QMI_CAT_EVENT_REPORT_DISPLAY_TEXT:
          *ind_id = QMI_CAT_SRVC_DISPLAY_TEXT_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->display_text_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->display_text_ind.length);
          if ((ind_data->display_text_ind.length <= 0) ||
              (ind_data->display_text_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->display_text_ind.value);

          ind_data->display_text_ind.value =
            (uint8_t*)malloc(ind_data->display_text_ind.length);
          if (ind_data->display_text_ind.value)
          {
            memcpy(ind_data->display_text_ind.value,
                   value_ptr,
                   ind_data->display_text_ind.length);
            value_ptr += ind_data->display_text_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_GET_INKEY:
          *ind_id = QMI_CAT_SRVC_GET_INKEY_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->get_inkey_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->get_inkey_ind.length);
          if ((ind_data->get_inkey_ind.length <= 0) ||
              (ind_data->get_inkey_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->get_inkey_ind.value);

          ind_data->get_inkey_ind.value =
            (uint8_t*)malloc(ind_data->get_inkey_ind.length);
          if (ind_data->get_inkey_ind.value)
          {
            memcpy(ind_data->get_inkey_ind.value,
                   value_ptr,
                   ind_data->get_inkey_ind.length);
            value_ptr += ind_data->get_inkey_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_GET_INPUT_KEY:
          *ind_id = QMI_CAT_SRVC_GET_INPUT_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->get_input_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->get_input_ind.length);
          if ((ind_data->get_input_ind.length <= 0) ||
              (ind_data->get_input_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->get_input_ind.value);

          ind_data->get_input_ind.value =
            (uint8_t*)malloc(ind_data->get_input_ind.length);
          if (ind_data->get_input_ind.value)
          {
            memcpy(ind_data->get_input_ind.value,
                   value_ptr,
                   ind_data->get_input_ind.length);
            value_ptr += ind_data->get_input_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SETUP_MENU:
          *ind_id = QMI_CAT_SRVC_SETUP_MENU_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->setup_menu_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->setup_menu_ind.length);
          if ((ind_data->setup_menu_ind.length <= 0) ||
              (ind_data->setup_menu_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->setup_menu_ind.value);

          ind_data->setup_menu_ind.value =
             (uint8_t*)malloc(ind_data->setup_menu_ind.length);
          if (ind_data->setup_menu_ind.value)
          {
            memcpy(ind_data->setup_menu_ind.value,
                   value_ptr,
                   ind_data->setup_menu_ind.length);
            value_ptr += ind_data->setup_menu_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SELECT_ITEM:
          *ind_id = QMI_CAT_SRVC_SELECT_ITEM_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->select_item_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->select_item_ind.length);
          if ((ind_data->select_item_ind.length <= 0) ||
              (ind_data->select_item_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->select_item_ind.value);

          ind_data->select_item_ind.value =
            (uint8_t*)malloc(ind_data->select_item_ind.length);
          if (ind_data->select_item_ind.value)
          {
            memcpy(ind_data->select_item_ind.value,
                   value_ptr,
                   ind_data->select_item_ind.length);
            value_ptr += ind_data->select_item_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SETUP_IDLE_MODE_TEXT:
          *ind_id = QMI_CAT_SRVC_SETUP_IDLE_MODE_TEXT_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->setup_idle_mode_text_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->setup_idle_mode_text_ind.length);
          if ((ind_data->setup_idle_mode_text_ind.length <= 0) ||
              (ind_data->setup_idle_mode_text_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->setup_idle_mode_text_ind.value);

          ind_data->setup_idle_mode_text_ind.value =
            (uint8_t*)malloc(ind_data->setup_idle_mode_text_ind.length);
          if (ind_data->setup_idle_mode_text_ind.value)
          {
            memcpy(ind_data->setup_idle_mode_text_ind.value,
                   value_ptr,
                   ind_data->setup_idle_mode_text_ind.length);
            value_ptr += ind_data->setup_idle_mode_text_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_LANGUAGE_NOTIFICATION:
          *ind_id = QMI_CAT_SRVC_LANGUAGE_NOTIFICATION_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->language_notification_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->language_notification_ind.length);
          if ((ind_data->language_notification_ind.length <= 0) ||
              (ind_data->language_notification_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->language_notification_ind.value);

          ind_data->language_notification_ind.value =
            (uint8_t*)malloc(ind_data->language_notification_ind.length);
          if (ind_data->language_notification_ind.value)
          {
            memcpy(ind_data->language_notification_ind.value,
                   value_ptr,
                   ind_data->language_notification_ind.length);
            value_ptr += ind_data->language_notification_ind.length;
          }
          break;


        case QMI_CAT_EVENT_REPORT_SETUP_EVENT_LIST:
          *ind_id = QMI_CAT_SRVC_SETUP_EVENT_LIST_IND;

          READ_32_BIT_VAL (value_ptr, temp_uint32);
          if (temp_uint32 & 0x00000001)
          {
            ind_data->setup_event_list_ind.user_activity = TRUE;
          }
          if (temp_uint32 & 0x00000002)
          {
            ind_data->setup_event_list_ind.idle_screen_available = TRUE;
          }
          if (temp_uint32 & 0x00000004)
          {
            ind_data->setup_event_list_ind.language_selection = TRUE;
          }
          if (temp_uint32 & 0x00000008)
          {
            ind_data->setup_event_list_ind.browser_termination = TRUE;
          }
          break;

        case QMI_CAT_EVENT_REPORT_PLAY_TONE:
          *ind_id = QMI_CAT_SRVC_PLAY_TONE_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->play_tone_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->play_tone_ind.length);
          if ((ind_data->play_tone_ind.length <= 0) ||
              (ind_data->play_tone_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->play_tone_ind.value);

          ind_data->play_tone_ind.value =
            (uint8_t*)malloc(ind_data->play_tone_ind.length);
          if (ind_data->play_tone_ind.value)
          {
            memcpy(ind_data->play_tone_ind.value,
                   value_ptr,
                   ind_data->play_tone_ind.length);
            value_ptr += ind_data->play_tone_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SETUP_CALL:
          *ind_id = QMI_CAT_SRVC_SETUP_CALL_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->setup_call_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->setup_call_ind.length);
          if ((ind_data->setup_call_ind.length <= 0) ||
              (ind_data->setup_call_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->setup_call_ind.value);

          ind_data->setup_call_ind.value =
            (uint8_t*)malloc(ind_data->setup_call_ind.length);
          if (ind_data->setup_call_ind.value)
          {
            memcpy(ind_data->setup_call_ind.value,
                   value_ptr,
                   ind_data->setup_call_ind.length);
            value_ptr += ind_data->setup_call_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SEND_DTMF:
          *ind_id = QMI_CAT_SRVC_SEND_DTMF_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->send_dtmf_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->send_dtmf_ind.length);
          if ((ind_data->send_dtmf_ind.length <= 0) ||
              (ind_data->send_dtmf_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->send_dtmf_ind.value);

          ind_data->send_dtmf_ind.value =
            (uint8_t*)malloc(ind_data->send_dtmf_ind.length);
          if (ind_data->send_dtmf_ind.value)
          {
            memcpy(ind_data->send_dtmf_ind.value,
                   value_ptr,
                   ind_data->send_dtmf_ind.length);
            value_ptr += ind_data->send_dtmf_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_LAUNCH_BROWSER:
          *ind_id = QMI_CAT_SRVC_LAUNCH_BROWSER_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->launch_browser_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->launch_browser_ind.length);
          if ((ind_data->launch_browser_ind.length <= 0) ||
              (ind_data->launch_browser_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->launch_browser_ind.value);

          ind_data->launch_browser_ind.value =
            (uint8_t*)malloc(ind_data->launch_browser_ind.length);
          if (ind_data->launch_browser_ind.value)
          {
            memcpy(ind_data->launch_browser_ind.value,
                   value_ptr,
                   ind_data->launch_browser_ind.length);
            value_ptr += ind_data->launch_browser_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SEND_SMS:
          *ind_id = QMI_CAT_SRVC_SEND_SMS_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->send_sms_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->send_sms_ind.length);
          if ((ind_data->send_sms_ind.length <= 0) ||
              (ind_data->send_sms_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->send_sms_ind.value);

          ind_data->send_sms_ind.value =
            (uint8_t*)malloc(ind_data->send_sms_ind.length);
          if (ind_data->send_sms_ind.value)
          {
            memcpy(ind_data->send_sms_ind.value,
                   value_ptr,
                   ind_data->send_sms_ind.length);
            value_ptr += ind_data->send_sms_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SEND_SS:
          *ind_id = QMI_CAT_SRVC_SEND_SS_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->send_ss_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->send_ss_ind.length);
          if ((ind_data->send_ss_ind.length <= 0) ||
              (ind_data->send_ss_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->send_ss_ind.value);

          ind_data->send_ss_ind.value =
            (uint8_t*)malloc(ind_data->send_ss_ind.length);
          if (ind_data->send_sms_ind.value)
          {
            memcpy(ind_data->send_ss_ind.value,
                   value_ptr,
                   ind_data->send_ss_ind.length);
            value_ptr += ind_data->send_ss_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SEND_USSD:
          *ind_id = QMI_CAT_SRVC_SEND_USSD_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->send_ussd_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->send_ussd_ind.length);
          if ((ind_data->send_ussd_ind.length <= 0) ||
              (ind_data->send_ussd_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->send_ussd_ind.value);

          ind_data->send_ussd_ind.value =
            (uint8_t*)malloc(ind_data->send_ussd_ind.length);
          if (ind_data->send_ussd_ind.value)
          {
            memcpy(ind_data->send_ussd_ind.value,
                   value_ptr,
                   ind_data->send_ussd_ind.length);
            value_ptr += ind_data->send_ussd_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_PROVIDE_LOCAL_INFO:
          *ind_id = QMI_CAT_SRVC_PROVIDE_LOCAL_INFO_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->provide_local_info_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->provide_local_info_ind.length);
          if ((ind_data->provide_local_info_ind.length <= 0) ||
              (ind_data->provide_local_info_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->provide_local_info_ind.value);

          ind_data->provide_local_info_ind.value =
            (uint8_t*)malloc(ind_data->provide_local_info_ind.length);
          if (ind_data->provide_local_info_ind.value)
          {
            memcpy(ind_data->provide_local_info_ind.value,
                   value_ptr,
                   ind_data->provide_local_info_ind.length);
            value_ptr += ind_data->provide_local_info_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SETUP_EVENT_LIST_RAW:
          *ind_id = QMI_CAT_SRVC_SETUP_EVENT_LIST_RAW_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->setup_event_list_raw_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->setup_event_list_raw_ind.length);
          if ((ind_data->setup_event_list_raw_ind.length <= 0) ||
              (ind_data->setup_event_list_raw_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->setup_event_list_raw_ind.value);

          ind_data->setup_event_list_raw_ind.value =
            (uint8_t*)malloc(ind_data->setup_event_list_raw_ind.length);
          if (ind_data->setup_event_list_raw_ind.value)
          {
            memcpy(ind_data->setup_event_list_raw_ind.value,
                   value_ptr,
                   ind_data->setup_event_list_raw_ind.length);
            value_ptr += ind_data->setup_event_list_raw_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_REFRESH_ALPHA:
          *ind_id = QMI_CAT_SRVC_REFRESH_ALPHA_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->refresh_alpha_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->refresh_alpha_ind.length);
          if ((ind_data->refresh_alpha_ind.length <= 0) ||
              (ind_data->refresh_alpha_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->refresh_alpha_ind.value);

          ind_data->refresh_alpha_ind.value =
            (uint8_t*)malloc(ind_data->refresh_alpha_ind.length);
          if (ind_data->refresh_alpha_ind.value)
          {
            memcpy(ind_data->refresh_alpha_ind.value,
                   value_ptr,
                   ind_data->refresh_alpha_ind.length);
            value_ptr += ind_data->refresh_alpha_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_OPEN_CHANNEL:
          *ind_id = QMI_CAT_SRVC_OPEN_CHANNEL_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->open_channel_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->open_channel_ind.length);
          if ((ind_data->open_channel_ind.length <= 0) ||
              (ind_data->open_channel_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->open_channel_ind.value);

          ind_data->open_channel_ind.value =
            (uint8_t*)malloc(ind_data->open_channel_ind.length);
          if (ind_data->open_channel_ind.value)
          {
            memcpy(ind_data->open_channel_ind.value,
                   value_ptr,
                   ind_data->open_channel_ind.length);
            value_ptr += ind_data->open_channel_ind.length;
          }
          break;

       case QMI_CAT_EVENT_REPORT_CLOSE_CHANNEL:
          *ind_id = QMI_CAT_SRVC_CLOSE_CHANNEL_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->close_channel_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->close_channel_ind.length);
          if ((ind_data->close_channel_ind.length <= 0) ||
              (ind_data->close_channel_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->close_channel_ind.value);

          ind_data->close_channel_ind.value =
            (uint8_t*)malloc(ind_data->close_channel_ind.length);
          if (ind_data->close_channel_ind.value)
          {
            memcpy(ind_data->close_channel_ind.value,
                   value_ptr,
                   ind_data->close_channel_ind.length);
            value_ptr += ind_data->close_channel_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SEND_DATA:
          *ind_id = QMI_CAT_SRVC_SEND_DATA_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->send_data_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->send_data_ind.length);
          if ((ind_data->send_data_ind.length <= 0) ||
              (ind_data->send_data_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->send_data_ind.value);

          ind_data->send_data_ind.value =
            (uint8_t*)malloc(ind_data->send_data_ind.length);
          if (ind_data->send_data_ind.value)
          {
            memcpy(ind_data->send_data_ind.value,
                   value_ptr,
                   ind_data->send_data_ind.length);
            value_ptr += ind_data->send_data_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_RECEIVE_DATA:
          *ind_id = QMI_CAT_SRVC_RECEIVE_DATA_IND;

          READ_32_BIT_VAL (value_ptr, ind_data->receive_data_ind.uim_ref_id);
          READ_16_BIT_VAL (value_ptr, ind_data->receive_data_ind.length);
          if ((ind_data->receive_data_ind.length <= 0) ||
              (ind_data->receive_data_ind.length > (length - QMI_CAT_UIM_REF_AND_CMD_LEN_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->receive_data_ind.value);

          ind_data->receive_data_ind.value =
            (uint8_t*)malloc(ind_data->receive_data_ind.length);
          if (ind_data->receive_data_ind.value)
          {
            memcpy(ind_data->receive_data_ind.value,
                   value_ptr,
                   ind_data->receive_data_ind.length);
            value_ptr += ind_data->receive_data_ind.length;
          }
          break;

        case QMI_CAT_EVENT_REPORT_END_PROACTIVE_SESSION:
          *ind_id = QMI_CAT_SRVC_END_PROACTIVE_SESSION_IND;
          break;

        case QMI_CAT_EVENT_REPORT_SLOT:
          /* Only need to read the slot id in this case */
          READ_8_BIT_VAL (value_ptr, temp_uint8);
          *slot_id = (qmi_cat_slot_id_type)temp_uint8;
          break;

        default:
          QMI_ERR_MSG_1 ("qmi_cat_prepare_indication_callback::Ignoring Invalid type %x\n ",type);
          break;
      }
      break;

    case QMI_CAT_SCWS_OPEN_CHANNEL_IND_MSG_ID:
      *ind_id = QMI_CAT_SRVC_SCWS_OPEN_CHANNEL_IND;
      switch (type)
      {
        case QMI_CAT_OPTIONAL_1_TLV_ID:
          READ_32_BIT_VAL (value_ptr, ind_data->scws_open_channel_ind.channel_id);
          READ_16_BIT_VAL (value_ptr, ind_data->scws_open_channel_ind.port);
          READ_16_BIT_VAL (value_ptr, ind_data->scws_open_channel_ind.buffer_size);
          break;

        case QMI_CAT_EVENT_REPORT_SLOT:
          READ_8_BIT_VAL (value_ptr, temp_uint8);
          *slot_id = (qmi_cat_slot_id_type)temp_uint8;
          QMI_DEBUG_MSG_1 ("read slot_id 0x%X\n ",*slot_id);
          break;

        default:
          QMI_DEBUG_MSG_1 ("qmi_cat_prepare_indication_callback:: Ignoring unsupported TLV 0x%X\n ",type);
          break;
      }
      break;

    case QMI_CAT_SCWS_CLOSE_CHANNEL_IND_MSG_ID:
      *ind_id = QMI_CAT_SRVC_SCWS_CLOSE_CHANNEL_IND;
      switch (type)
      {
        case QMI_CAT_OPTIONAL_1_TLV_ID:
          READ_32_BIT_VAL (value_ptr, ind_data->scws_close_channel_ind.channel_id);
          READ_8_BIT_VAL (value_ptr, ind_data->scws_close_channel_ind.channel_state);
          break;

        case QMI_CAT_EVENT_REPORT_SLOT:
          READ_8_BIT_VAL (value_ptr, temp_uint8);
          *slot_id = (qmi_cat_slot_id_type)temp_uint8;
          QMI_DEBUG_MSG_1 ("read slot_id 0x%X\n ",*slot_id);
          break;

        default:
          QMI_DEBUG_MSG_1 ("qmi_cat_prepare_indication_callback:: Ignoring unsupported TLV 0x%X\n ",type);
          break;
      }
      break;

    case QMI_CAT_SCWS_SEND_DATA_IND_MSG_ID:
      *ind_id = QMI_CAT_SRVC_SCWS_SEND_DATA_IND;
      switch (type)
      {
        case QMI_CAT_OPTIONAL_1_TLV_ID:
          READ_32_BIT_VAL (value_ptr, ind_data->scws_send_data_ind.channel_id);
          READ_8_BIT_VAL (value_ptr, ind_data->scws_send_data_ind.total_packets);
          READ_8_BIT_VAL (value_ptr, ind_data->scws_send_data_ind.current_packet);
          READ_16_BIT_VAL (value_ptr, ind_data->scws_send_data_ind.data_len);

          if ((ind_data->scws_send_data_ind.data_len <= 0) ||
              (ind_data->scws_send_data_ind.data_len > (length - QMI_CAT_SCWS_SEND_DATA_IND_STATIC_TLV_SIZE)))
          {
            return 1;
          }

          QMI_CAT_FREE_IF_NOT_NULL(ind_data->scws_send_data_ind.data_ptr);

          ind_data->scws_send_data_ind.data_ptr =
            (uint8_t*)malloc(ind_data->scws_send_data_ind.data_len);
          if (ind_data->scws_send_data_ind.data_ptr)
          {
            memcpy(ind_data->scws_send_data_ind.data_ptr,
                   value_ptr,
                   ind_data->scws_send_data_ind.data_len);
            value_ptr += ind_data->scws_send_data_ind.data_len;
          }
          break;

        case QMI_CAT_EVENT_REPORT_SLOT:
          READ_8_BIT_VAL (value_ptr, temp_uint8);
          *slot_id = (qmi_cat_slot_id_type)temp_uint8;
          QMI_DEBUG_MSG_1 ("read slot_id 0x%X\n ",*slot_id);
          break;

        default:
          QMI_DEBUG_MSG_1 ("qmi_cat_prepare_indication_callback:: Ignoring unsupported TLV 0x%X\n ",type);
          break;
      }
      break;

    default:
      QMI_ERR_MSG_1 ("qmi_cat_prepare_indication_callback::Unsupported msg 0x%X\n ",msg_id);
      return 1;
  }

  return 0;
} /* qmi_cat_prepare_indication_callback */

/*===========================================================================
  FUNCTION  qmi_cat_free_indication_callback
===========================================================================*/
static void qmi_cat_free_indication_callback
(
  qmi_cat_indication_id_type      ind_id,
  qmi_cat_indication_data_type  * ind_data
)
{
  if (ind_data == NULL)
  {
    return;
  }

  switch (ind_id)
  {
    case QMI_CAT_SRVC_DISPLAY_TEXT_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->display_text_ind.value);
      break;

    case QMI_CAT_SRVC_GET_INKEY_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->get_inkey_ind.value);
      break;

    case QMI_CAT_SRVC_GET_INPUT_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->get_input_ind.value);
      break;

    case QMI_CAT_SRVC_SETUP_MENU_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->setup_menu_ind.value);
      break;

    case QMI_CAT_SRVC_SELECT_ITEM_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->select_item_ind.value);
      break;

    case QMI_CAT_SRVC_SETUP_IDLE_MODE_TEXT_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->setup_idle_mode_text_ind.value);
      break;

    case QMI_CAT_SRVC_LANGUAGE_NOTIFICATION_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->language_notification_ind.value);
      break;

    case QMI_CAT_SRVC_SEND_SMS_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->send_sms_ind.value);
      break;

    case QMI_CAT_SRVC_PLAY_TONE_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->play_tone_ind.value);
      break;

    case QMI_CAT_SRVC_SETUP_CALL_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->setup_call_ind.value);
      break;

    case QMI_CAT_SRVC_SEND_DTMF_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->send_dtmf_ind.value);
      break;

    case QMI_CAT_SRVC_LAUNCH_BROWSER_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->launch_browser_ind.value);
      break;

    case QMI_CAT_SRVC_SEND_SS_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->send_ss_ind.value);
      break;

    case QMI_CAT_SRVC_SEND_USSD_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->send_ussd_ind.value);
      break;

    case QMI_CAT_SRVC_PROVIDE_LOCAL_INFO_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->provide_local_info_ind.value);
      break;

    case QMI_CAT_SRVC_SETUP_EVENT_LIST_RAW_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->setup_event_list_raw_ind.value);
      break;

    case QMI_CAT_SRVC_REFRESH_ALPHA_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->refresh_alpha_ind.value);
      break;

    case QMI_CAT_SRVC_OPEN_CHANNEL_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->open_channel_ind.value);
      break;

    case QMI_CAT_SRVC_CLOSE_CHANNEL_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->close_channel_ind.value);
      break;

    case QMI_CAT_SRVC_SEND_DATA_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->send_data_ind.value);
      break;

    case QMI_CAT_SRVC_RECEIVE_DATA_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->receive_data_ind.value);
      break;

    case QMI_CAT_SRVC_SCWS_SEND_DATA_IND:
      QMI_CAT_FREE_IF_NOT_NULL(ind_data->scws_send_data_ind.data_ptr);
      break;

    case QMI_CAT_SRVC_SETUP_EVENT_LIST_IND:
    case QMI_CAT_SRVC_END_PROACTIVE_SESSION_IND:
    case QMI_CAT_SRVC_SCWS_OPEN_CHANNEL_IND:
    case QMI_CAT_SRVC_SCWS_CLOSE_CHANNEL_IND:
      default:
      /* Nothing to do */
      break;
  }
} /* qmi_cat_free_indication_callback */

/*===========================================================================
  FUNCTION  qmi_cat_update_slot_indication_callback
===========================================================================*/
static void qmi_cat_update_slot_indication_callback
(
  unsigned long                   msg_id,
  unsigned long                   type,
  qmi_cat_slot_id_type            slot_id,
  qmi_cat_indication_data_type  * ind_data
)
{

  if (ind_data  == NULL)
  {
    return;
  }

  QMI_DEBUG_MSG_1("qmi_cat_update_slot_indication_callback: type 0x%X", type);

  /* Update slot id */
  switch (msg_id)
  {
    case QMI_CAT_EVENT_REPORT_IND_MSG_ID:
      switch(type)
      {
        case QMI_CAT_EVENT_REPORT_DISPLAY_TEXT:
          ind_data->display_text_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_GET_INKEY:
          ind_data->get_inkey_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_GET_INPUT_KEY:
          ind_data->get_input_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SETUP_MENU:
          ind_data->setup_menu_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SELECT_ITEM:
          ind_data->select_item_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SETUP_IDLE_MODE_TEXT:
          ind_data->setup_idle_mode_text_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_LANGUAGE_NOTIFICATION:
          ind_data->language_notification_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_PLAY_TONE:
          ind_data->play_tone_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SETUP_CALL:
          ind_data->setup_call_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SEND_SMS:
          ind_data->send_sms_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SEND_DTMF:
          ind_data->send_dtmf_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_LAUNCH_BROWSER:
          ind_data->launch_browser_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SEND_SS:
          ind_data->send_ss_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SEND_USSD:
          ind_data->send_ussd_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_PROVIDE_LOCAL_INFO:
          ind_data->provide_local_info_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SETUP_EVENT_LIST_RAW:
          ind_data->setup_event_list_raw_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_REFRESH_ALPHA:
          ind_data->refresh_alpha_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_OPEN_CHANNEL:
          ind_data->open_channel_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_CLOSE_CHANNEL:
          ind_data->close_channel_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SEND_DATA:
          ind_data->send_data_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_RECEIVE_DATA:
          ind_data->receive_data_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_SETUP_EVENT_LIST:
          ind_data->setup_event_list_ind.slot_id = slot_id;
          break;

        case QMI_CAT_EVENT_REPORT_END_PROACTIVE_SESSION:
          ind_data->end_proactive_session_ind.slot_id = slot_id;
          break;

        default:
          QMI_ERR_MSG_1 ("qmi_cat_update_slot_indication_callback: Invalid type for updation %x\n ",
                         type);
          break;
      }
      break;

    case QMI_CAT_SCWS_OPEN_CHANNEL_IND_MSG_ID:
      if (type != QMI_CAT_EVENT_REPORT_SLOT)
      {
        ind_data->scws_open_channel_ind.slot_id = slot_id;
      }
      break;

    case QMI_CAT_SCWS_CLOSE_CHANNEL_IND_MSG_ID:
      if (type != QMI_CAT_EVENT_REPORT_SLOT)
      {
        ind_data->scws_close_channel_ind.slot_id = slot_id;
      }
      break;

    case QMI_CAT_SCWS_SEND_DATA_IND_MSG_ID:
      if (type != QMI_CAT_EVENT_REPORT_SLOT)
      {
        ind_data->scws_send_data_ind.slot_id = slot_id;
      }
      break;

    default:
      QMI_ERR_MSG_1 ("qmi_cat_update_slot_indication_callback: Invalid msg_id for updation %x\n ",
                      msg_id);
      break;
  }

  return;
} /* qmi_cat_update_slot_indication_callback */

/*---------------------------------------------------------------------------
  Callback functions
---------------------------------------------------------------------------*/

/*===========================================================================
  FUNCTION  qmi_cat_srvc_indication_cb
===========================================================================*/
/*!
@brief
  This is the callback function that will be called by the generic
  services layer to report asynchronous indications.  This function will
  process the indication TLV's and then call the user registered
  functions with the indication data.

@return
  None.

@note

  - Dependencies

  - Side Effects
*/
/*=========================================================================*/
static void qmi_cat_srvc_indication_cb
(
  int                                 user_handle,
  qmi_service_id_type                 service_id,
  unsigned long                       msg_id,
  void                              * user_ind_msg_hdlr,
  void                              * user_ind_msg_hdlr_user_data,
  unsigned char                     * rx_msg_buf,
  int                                 rx_msg_len
)
{
  qmi_cat_indication_id_type      ind_id               = 0;
  qmi_cat_indication_data_type    ind_data;
  qmi_cat_indication_hdlr_type    user_ind_hdlr;
  unsigned long                   type                 = 0;
  unsigned long                   length               = 0;
  unsigned char                 * value_ptr            = NULL;
  unsigned long                   non_slot_report_type = QMI_CAT_EVENT_REPORT_SLOT;
  qmi_cat_slot_id_type            slot_id              = QMI_CAT_SLOT_1;


  if ((user_ind_msg_hdlr == NULL) || (rx_msg_len <= 0))
  {
    return;
  }

  /* Initialize data */
  memset((void*)&ind_data, 0x0, sizeof(qmi_cat_indication_data_type));

  QMI_DEBUG_MSG_1("qmi_cat_srvc_indication_cb: msg_id %d",msg_id );

  /* Loop to read the TLVs, if present */
  while (rx_msg_len > 0)
  {
    /* Read the TLV */
    if (qmi_util_read_std_tlv (&rx_msg_buf,
                               &rx_msg_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return;
    }

    /* Determine the indication ID and process appropriately */
    switch (msg_id)
    {
      case QMI_CAT_EVENT_REPORT_IND_MSG_ID:
      case QMI_CAT_SCWS_OPEN_CHANNEL_IND_MSG_ID:
      case QMI_CAT_SCWS_CLOSE_CHANNEL_IND_MSG_ID:
      case QMI_CAT_SCWS_SEND_DATA_IND_MSG_ID:
        if (qmi_cat_prepare_indication_callback(msg_id,
                                                type,
                                                length,
                                                value_ptr,
                                                &ind_id,
                                                &ind_data,
                                                &slot_id) != 0)
        {
          /* Free up any memory allocated before returning */
          qmi_cat_free_indication_callback(ind_id, &ind_data);
          QMI_ERR_MSG_0 ("qmi_cat_srvc_indication_cb::Error in event report callback preparation");
          return;
        }

        /* Save the type other than slot in case is read after slot TLV */
        if (type != QMI_CAT_EVENT_REPORT_SLOT)
        {
          non_slot_report_type = type;
        }

        /* Update indication data with slot (either default or received value)
           We do it here since ind_data has to be updated based on type of message */
        qmi_cat_update_slot_indication_callback(msg_id, non_slot_report_type, slot_id, &ind_data);

        break;

      default:
        QMI_ERR_MSG_1 ("qmi_cat_srvc_indication_cb::Invalid indication msg_id received %lx\n ",msg_id);
        return;
    } /* Switch */
  } /* while */

  /*lint -e{611} */
  user_ind_hdlr = (qmi_cat_indication_hdlr_type) user_ind_msg_hdlr;

  /* Call user registered handler */
  user_ind_hdlr (user_handle,
                 service_id,
                 user_ind_msg_hdlr_user_data,
                 ind_id,
                 &ind_data);

  /* Free up any memory allocated */
  qmi_cat_free_indication_callback(ind_id, &ind_data);
} /* qmi_cat_srvc_indication_cb */


/*===========================================================================
  FUNCTION  qmi_cat_srvc_async_cb
===========================================================================*/
/*!
@brief
  This is the callback function that will be called by the generic
  services layer to report asynchronous responses.  This function will
  process the asynchronous TLV's and then call the user registered
  functions with the reply data

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
static void qmi_cat_srvc_async_cb
(
  int                       user_handle,
  qmi_service_id_type       service_id,
  unsigned long             msg_id,
  int                       rsp_rc,
  int                       qmi_err_code,
  unsigned char           * reply_msg_data,
  int                       reply_msg_size,
  void                    * srvc_async_cb_data,
  void                    * user_async_cb_fn,
  void                    * user_async_cb_data
)
{
  qmi_cat_async_command_cb_type     user_cb;
  qmi_cat_rsp_data_type             rsp_data;
  unsigned long                     type       = 0;
  unsigned long                     length     = 0;
  unsigned char                   * value_ptr  = NULL;

  (void) srvc_async_cb_data;

  QMI_DEBUG_MSG_1("qmi_cat_srvc_async_cb: msg_id = 0x%x", msg_id);

  memset((void*)&rsp_data, 0x0, sizeof(rsp_data));

  /* Update the response error codes */
  rsp_data.sys_err_code = rsp_rc;
  rsp_data.qmi_err_code = qmi_err_code;

  /* Update response ID */
  switch (msg_id)
  {
    case QMI_CAT_SCWS_DATA_AVAILABLE_MSG_ID:
      rsp_data.rsp_id = QMI_CAT_SRVC_SCWS_DATA_AVAILABLE_RSP_MSG;
      break;

    default:
      QMI_ERR_MSG_2 ("Unhandled async reply msg, msg_id=%x, user=%x",
                     (unsigned int) msg_id, (unsigned int) user_handle);
      return;
  }

  /* Loop to read the TLVs, if present */
  while (reply_msg_size > 0)
  {
    if (qmi_util_read_std_tlv (&reply_msg_data,
                               &reply_msg_size,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_cat_srvc_async_cb: Malformed message, discarding");
      rsp_data.sys_err_code = QMI_SERVICE_ERR;
      rsp_data.qmi_err_code = QMI_SERVICE_ERR_MALFORMED_MSG;
      break;
    }

    switch (rsp_data.rsp_id)
    {
      case QMI_CAT_SRVC_SCWS_DATA_AVAILABLE_RSP_MSG:
        /* No TLV expected, so nothing to do ... */
        break;

      default:
        QMI_ERR_MSG_1 ("qmi_cat_srvc_async_cb: unhandled rsp_id = %x",
                       (unsigned int)rsp_data.rsp_id);
        break;
    }
  }

  /* If the reply indicates that the command was aborted, don't
  ** call the user callback */
  if ((rsp_rc == QMI_SERVICE_ERR) &&
      (qmi_err_code == QMI_SERVICE_ERR_ABORTED))
  {
    QMI_DEBUG_MSG_2 ("Discarding aborted reply, msg_id=%x, user=%x",
                     (unsigned int) msg_id, (unsigned int) user_handle);
  }
  else
  {
    /*lint -e{611} */
    user_cb = (qmi_cat_async_command_cb_type) user_async_cb_fn;

    /* Call the user callback */
    user_cb (user_handle,
             service_id,
             &rsp_data,
             user_async_cb_data);
  }

} /* qmi_cat_srvc_async_cb */


/*---------------------------------------------------------------------------
  Init/Deinit functions
---------------------------------------------------------------------------*/

/*===========================================================================
  FUNCTION  qmi_cat_srvc_init
===========================================================================*/
/*!
@brief
  This function is a callback that will be called once during client
  initialization

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int qmi_cat_srvc_init (void)
{
  int  rc = QMI_NO_ERR;
  if (!cat_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_CAT_SERVICE,
                                         qmi_cat_srvc_indication_cb);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_cat_srvc_init: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_cat_srvc_init: CAT successfully initialized");
      cat_service_initialized = TRUE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_cat_srvc_init: Init failed, CAT already initialized");
  }
  return rc;
} /* qmi_cat_srvc_init */


/*===========================================================================
  FUNCTION  qmi_cat_srvc_release
===========================================================================*/
/*!
@brief
  This function is a callback that will be called once during client
  release

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int qmi_cat_srvc_release (void)
{
  int  rc = QMI_NO_ERR;
  if (cat_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_CAT_SERVICE, NULL);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_cat_srvc_release: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_cat_srvc_release: CAT successfully released");
      cat_service_initialized = FALSE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_cat_srvc_release: Release failed, CAT not initialized");
  }
  return rc;
} /* qmi_cat_srvc_release */


/*===========================================================================
  FUNCTION  qmi_cat_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the CAT service.  This function
  must be called prior to calling any other CAT service functions.
  For the time being, the indication handler callback and user data
  should be set to NULL until this is implemented.  Also note that this
  function may be called multiple times to allow for multiple, independent
  clients.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
qmi_client_handle_type qmi_cat_srvc_init_client
(
  const char                    * dev_id,
  qmi_cat_indication_hdlr_type    user_rx_ind_msg_hdlr,
  void                          * user_rx_ind_msg_hdlr_user_data,
  int                           * qmi_err_code
)
{
  qmi_client_handle_type client_handle;
  qmi_connection_id_type conn_id;

  conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID(dev_id);

  if (conn_id == QMI_CONN_ID_INVALID)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Call common service layer initialization function */
  /*lint -e{611} */
  client_handle =  qmi_service_init (conn_id,
                                   QMI_CAT_SERVICE,
                                   (void *) user_rx_ind_msg_hdlr,
                                   user_rx_ind_msg_hdlr_user_data,
                                   qmi_err_code);

  return client_handle;
} /* qmi_cat_srvc_init_client */


/*===========================================================================
  FUNCTION  qmi_cat_srvc_release_client
===========================================================================*/
/*!
@brief
  This function is called to release a client created by the
  qmi_cat_srvc_init_client() function.  This function should be called
  for any client created when terminating a client process, especially
  if the modem processor is not reset.  The modem side QMI server has
  a limited number of clients that it will allocate, and if they are not
  released, we will run out.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
int qmi_cat_srvc_release_client
(
  int        user_handle,
  int      * qmi_err_code
)
{
  int rc;
  rc = qmi_service_release (user_handle, qmi_err_code);
  return rc;
} /* qmi_cat_srvc_release_client */


/*---------------------------------------------------------------------------
   Command handling functions from the Client
---------------------------------------------------------------------------*/

/*===========================================================================
  FUNCTION  qmi_cat_reset
===========================================================================*/
/*!
@brief
  Resets CAT service.  If the user_cb function pointer is set to NULL,
  then this function will be invoked synchronously, otherwise it will
  be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value in rsp_data will give you the QMI error reason.
  Otherwise, qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_cat_abort() command.

@note

  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - Resets CAT service
*/
/*=========================================================================*/
int qmi_cat_reset
(
  int    user_handle,
  int  * qmi_err_code
)
{
  unsigned char msg[QMI_CAT_STD_MSG_SIZE];
  int           msg_size;
  int rc;

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_CAT_SERVICE,
                                  QMI_CAT_RESET_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_CAT_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
} /* qmi_cat_reset */


/*===========================================================================
  FUNCTION  qmi_cat_set_event_report
===========================================================================*/
/*!
@brief
  Sets the mask of events that client wants to receive from QMI CAT
  service. This function is always executed synchronously.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value in rsp_data
  will give you the QMI error reason. Otherwise, qmi_err_code will have
  meaningless data.

@note
  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_cat_set_event_report
(
  int                                           client_handle,
  const qmi_cat_set_event_report_params_type  * params,
  int                                         * qmi_err_code
)
{
  unsigned char     msg[QMI_CAT_STD_MSG_SIZE];
  unsigned char     param_buf[QMI_CAT_MAX_PARAM_BUF_SIZE];

  int               msg_size      = 0;
  unsigned char   * tmp_msg_ptr   = NULL;
  unsigned char   * tmp_param_ptr = NULL;
  uint32_t          mask          = 0;
  int               rc            = 0;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  /* Compose the mask */
  mask |= (params->display_test                    ? QMI_CAT_SET_EVENT_REPORT_DISPLAY_TEXT_MASK : 0);
  mask |= (params->get_inkey                       ? QMI_CAT_SET_EVENT_REPORT_GET_INKEY : 0);
  mask |= (params->get_input                       ? QMI_CAT_SET_EVENT_REPORT_GET_INPUT: 0);
  mask |= (params->setup_menu                      ? QMI_CAT_SET_EVENT_REPORT_SETUP_MENU: 0);
  mask |= (params->select_item                     ? QMI_CAT_SET_EVENT_REPORT_SELECT_ITEM: 0);
  mask |= (params->send_sms                        ? QMI_CAT_SET_EVENT_REPORT_SEND_SMS: 0);
  mask |= (params->setup_event_user_activity       ? QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_USER_ACTIVITY: 0);
  mask |= (params->setup_event_idle_screen_notify  ? QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_IDLE_SCREEN_NOTIFY: 0);
  mask |= (params->setup_event_language_sel_notify ? QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_LANGUAGE_SEL_NOTIFY: 0);
  mask |= (params->setup_idle_mode_text            ? QMI_CAT_SET_EVENT_REPORT_SETUP_IDLE_MODE_TEXT: 0);
  mask |= (params->language_notification           ? QMI_CAT_SET_EVENT_REPORT_LANGUAGE_NOTIFICATION: 0);
  mask |= (params->refresh_alpha                   ? QMI_CAT_SET_EVENT_REPORT_REFRESH: 0);
  mask |= (params->end_proactive_session           ? QMI_CAT_SET_EVENT_REPORT_END_PROACTIVE_SESSION: 0);
  mask |= (params->play_tone                       ? QMI_CAT_SET_EVENT_REPORT_PLAY_TONE: 0);
  mask |= (params->setup_call                      ? QMI_CAT_SET_EVENT_REPORT_SETUP_CALL: 0);
  mask |= (params->send_dtmf                       ? QMI_CAT_SET_EVENT_REPORT_SEND_DTMF: 0);
  mask |= (params->launch_browser                  ? QMI_CAT_SET_EVENT_REPORT_LAUNCH_BROWSER: 0);
  mask |= (params->send_ss                         ? QMI_CAT_SET_EVENT_REPORT_SEND_SS: 0);
  mask |= (params->send_ussd                       ? QMI_CAT_SET_EVENT_REPORT_SEND_USSD: 0);
  mask |= (params->provide_local_info_lang         ? QMI_CAT_SET_EVENT_REPORT_PROVIDE_LOCAL_INFO_LANG: 0);
  mask |= (params->provide_local_info_time         ? QMI_CAT_SET_EVENT_REPORT_PROVIDE_LOCAL_INFO_TIME: 0);
  mask |= (params->bip                             ? QMI_CAT_SET_EVENT_REPORT_BIP: 0);
  mask |= (params->setup_event_browser_term        ? QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_BROWSER_TERM: 0);
  mask |= (params->scws                            ? QMI_CAT_SET_EVENT_REPORT_SCWS: 0);


  /*Prepare the Final TLV*/
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE);

  /* Construct TLV */
  tmp_param_ptr = param_buf;
  WRITE_32_BIT_VAL(tmp_param_ptr, mask);

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               (params->format == QMI_CAT_SET_EVENT_REPORT_RAW) ?
                                 QMI_CAT_OPTIONAL_1_TLV_ID :
                                 QMI_CAT_OPTIONAL_2_TLV_ID,
                               sizeof(uint32_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  /* Next, construct Slot TLV - (optional) */
  tmp_param_ptr = param_buf;
  WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)params->slot_mask));

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_OPTIONAL_3_TLV_ID,
                               sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_CAT_SERVICE,
                                  QMI_CAT_SET_EVENT_REPORT_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_CAT_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* We can ignore the response */
  return rc;
} /* qmi_cat_set_event_report */


/*===========================================================================
  FUNCTION  qmi_cat_send_tr
===========================================================================*/
/*!
@brief
  Sends the Terminal Response for a proactive command previosly received
  by the client. This function is always executed synchronously.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value in rsp_data
  will give you the QMI error reason. Otherwise, qmi_err_code will have
  meaningless data.

@note
  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_cat_send_tr
(
  int                                 client_handle,
  const qmi_cat_send_tr_params_type * params,
  int                               * qmi_err_code
)
{
  unsigned char     msg[QMI_CAT_STD_MSG_SIZE];
  unsigned char     param_buf[QMI_CAT_MAX_PARAM_BUF_SIZE];

  int               msg_size      = 0;
  unsigned char   * tmp_msg_ptr   = NULL;
  unsigned char   * tmp_param_ptr = NULL;
  uint32_t          mask          = 0;
  int               rc            = 0;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  /*Prepare the Final TLV*/
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE);

  /* Construct Terminal response TLV (mandatory) */
  tmp_param_ptr = param_buf;
  WRITE_32_BIT_VAL(tmp_param_ptr, params->uim_ref_id);
  WRITE_16_BIT_VAL(tmp_param_ptr, params->tr_length);
  if(params->tr_length > 0 &&
     params->tr_length <= QMI_CAT_MAX_APDU_SIZE &&
     params->tr_value != NULL)
  {
    memcpy(tmp_param_ptr, (void *)params->tr_value, params->tr_length);
    tmp_param_ptr += params->tr_length;
  }
  else
  {
    QMI_ERR_MSG_2("qmi_cat_send_tr: Invalid parameters, tr_length: 0x%x, tr_value: %p",
                  params->tr_length, params->tr_value);
    return QMI_SERVICE_ERR;
  }

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_MANDATORY_1_TLV_ID,
                               sizeof(uint32_t) + sizeof(uint16_t) + params->tr_length,
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  /* Next, construct Slot TLV - (optional) */
  tmp_param_ptr = param_buf;
  WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)params->slot_id));

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_OPTIONAL_1_TLV_ID,
                               sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_CAT_SERVICE,
                                  QMI_CAT_SEND_TR_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_CAT_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
} /* qmi_cat_send_tr */


/*===========================================================================
  FUNCTION  qmi_cat_send_envelope
===========================================================================*/
/*!
@brief
  Sends an envelope command to the card. This function is always
  executed synchronously.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value in rsp_data
  will give you the QMI error reason. Otherwise, qmi_err_code will have
  meaningless data.

@note
  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_cat_send_envelope
(
  int                                       client_handle,
  const qmi_cat_send_envelope_params_type * req_params,
  qmi_cat_send_envelope_rsp_type          * rsp_params,
  int                                     * qmi_err_code
)
{
  unsigned char     msg[QMI_CAT_STD_MSG_SIZE];
  unsigned char     param_buf[QMI_CAT_MAX_PARAM_BUF_SIZE];

  int               msg_size      = 0;
  unsigned char   * tmp_msg_ptr   = NULL;
  unsigned char   * tmp_param_ptr = NULL;
  uint32_t          mask          = 0;
  int               rc            = 0;

  if ((!req_params) || (!qmi_err_code))
  {
    return QMI_SERVICE_ERR;
  }

  /*Prepare the Final TLV*/
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE);

  /* Construct Envelope TLV (mandatory) */
  tmp_param_ptr = param_buf;
  WRITE_16_BIT_VAL(tmp_param_ptr, ((uint16_t)req_params->envelope_cmd_type));
  WRITE_16_BIT_VAL(tmp_param_ptr, req_params->envelope_length);
  if(req_params->envelope_length > 0 &&
     req_params->envelope_length <= QMI_CAT_MAX_APDU_SIZE &&
     req_params->envelope_value != NULL)
  {
    memcpy(tmp_param_ptr, (void *)req_params->envelope_value, req_params->envelope_length);
    tmp_param_ptr += req_params->envelope_length;
  }
  else
  {
    QMI_ERR_MSG_2("qmi_cat_send_tr: Invalid parameters, envelope_length: 0x%x, envelope_value: %p",
                  req_params->envelope_length, req_params->envelope_value);
    return QMI_SERVICE_ERR;
  }

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_MANDATORY_1_TLV_ID,
                               sizeof(uint16_t) + sizeof(uint16_t) + req_params->envelope_length,
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  /* Next, construct Slot TLV - (optional) */
  tmp_param_ptr = param_buf;
  WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)req_params->slot_id));

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_OPTIONAL_1_TLV_ID,
                               sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_CAT_SERVICE,
                                  QMI_CAT_SEND_ENVELOPE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_CAT_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* We update the response only if a response pointer was provided */
  if ((rc == QMI_NO_ERR) && (rsp_params != NULL))
  {
    memset (rsp_params, 0, sizeof(qmi_cat_send_envelope_rsp_type));
    if (qmi_cat_handle_send_envelope_rsp (msg, msg_size, rsp_params) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_cat_send_envelope: qmi_cat_handle_send_envelope_rsp returned error");
      rc = QMI_INTERNAL_ERR;
      *qmi_err_code = QMI_INTERNAL_ERR;
    }
  }
  return rc;
} /* qmi_cat_send_envelope */


/*===========================================================================
  FUNCTION  qmi_cat_event_confirmation
===========================================================================*/
/*!
@brief
  Used to cofnirm a network related command. This function is always
  executed synchronously.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value in rsp_data
  will give you the QMI error reason. Otherwise, qmi_err_code will have
  meaningless data.

@note
  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_cat_event_confirmation
(
  int                                            client_handle,
  const qmi_cat_event_confirmation_params_type * params,
  int                                          * qmi_err_code
)
{
  unsigned char     msg[QMI_CAT_STD_MSG_SIZE];
  unsigned char     param_buf[QMI_CAT_MAX_PARAM_BUF_SIZE];

  int               msg_size      = 0;
  unsigned char   * tmp_msg_ptr   = NULL;
  unsigned char   * tmp_param_ptr = NULL;
  uint32_t          mask          = 0;
  int               rc            = 0;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  /*Prepare the Final TLV*/
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE);

  /* Construct User confirmation TLV (optional) */
  if (params->user == QMI_CAT_CONF_FALSE || params->user == QMI_CAT_CONF_TRUE)
  {
    tmp_param_ptr = param_buf;
    WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)params->user));

    rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_CAT_OPTIONAL_1_TLV_ID,
                                 sizeof(uint8_t),
                                 (void *)param_buf);
    if (rc < 0)
    {
      return rc;
    }
  }

  /* Next, construct Icon display TLV (optional) */
  if (params->icon == QMI_CAT_CONF_FALSE || params->icon == QMI_CAT_CONF_TRUE)
  {
    tmp_param_ptr = param_buf;
    WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)params->icon));

    rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_CAT_OPTIONAL_2_TLV_ID,
                                 sizeof(uint8_t),
                                 (void *)param_buf);
    if (rc < 0)
    {
      return rc;
    }
  }

  /* Next, construct Slot TLV - (optional) */
  tmp_param_ptr = param_buf;
  WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)params->slot_id));

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_OPTIONAL_3_TLV_ID,
                               sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_CAT_SERVICE,
                                  QMI_CAT_EVENT_CONFIRMATION_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_CAT_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
} /* qmi_cat_event_confirmation */


/*===========================================================================
  FUNCTION  qmi_cat_scws_open_channel
===========================================================================*/
/*!
@brief
  Used to send the status of the socket for the open channel indication that
  was previously sent to the control point. This function is always executed
  synchronously.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value in rsp_data
  will give you the QMI error reason. Otherwise, qmi_err_code will have
  meaningless data.

@note
  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_cat_scws_open_channel
(
  int                                            client_handle,
  const qmi_cat_scws_channel_params_type       * params,
  int                                          * qmi_err_code
)
{
  unsigned char     msg[QMI_CAT_STD_MSG_SIZE];
  unsigned char     param_buf[QMI_CAT_MAX_PARAM_BUF_SIZE];

  int               msg_size      = 0;
  unsigned char   * tmp_msg_ptr   = NULL;
  unsigned char   * tmp_param_ptr = NULL;
  int               rc            = 0;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  /*Prepare the Final TLV*/
  tmp_msg_ptr   = QMI_SRVC_PDU_PTR(msg);
  msg_size      = QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE);
  tmp_param_ptr = param_buf;

  /* Construct channel state TLV */
  WRITE_32_BIT_VAL (tmp_param_ptr, params->channel_id);
  WRITE_8_BIT_VAL (tmp_param_ptr, params->channel_state);

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_MANDATORY_1_TLV_ID,
                               sizeof(uint32_t) + sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  /* Next construct the Slot ID TLV */
  tmp_param_ptr = param_buf;
  WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)params->slot_id));

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_OPTIONAL_1_TLV_ID,
                               sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_CAT_SERVICE,
                                  QMI_CAT_SCWS_OPEN_CHANNEL_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_CAT_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
} /* qmi_cat_scws_open_channel */


/*===========================================================================
  FUNCTION  qmi_cat_scws_close_channel
===========================================================================*/
/*!
@brief
  Used to send the confirmation status of the socket for the close channel
  indication that was previously sent to the control point. This function is
  always executed synchronously.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value in rsp_data
  will give you the QMI error reason. Otherwise, qmi_err_code will have
  meaningless data.

@note
  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_cat_scws_close_channel
(
  int                                            client_handle,
  const qmi_cat_scws_channel_params_type       * params,
  int                                          * qmi_err_code
)
{
  unsigned char     msg[QMI_CAT_STD_MSG_SIZE];
  unsigned char     param_buf[QMI_CAT_MAX_PARAM_BUF_SIZE];

  int               msg_size      = 0;
  unsigned char   * tmp_msg_ptr   = NULL;
  unsigned char   * tmp_param_ptr = NULL;
  int               rc            = 0;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  /*Prepare the Final TLV*/
  tmp_msg_ptr   = QMI_SRVC_PDU_PTR(msg);
  msg_size      = QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE);
  tmp_param_ptr = param_buf;

  /* Construct channel state TLV */
  WRITE_32_BIT_VAL (tmp_param_ptr, params->channel_id);
  WRITE_8_BIT_VAL (tmp_param_ptr, params->channel_state);

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_MANDATORY_1_TLV_ID,
                               sizeof(uint32_t) + sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  /* Next construct the Slot ID TLV */
  tmp_param_ptr = param_buf;
  WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)params->slot_id));

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_OPTIONAL_1_TLV_ID,
                               sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_CAT_SERVICE,
                                  QMI_CAT_SCWS_CLOSE_CHANNEL_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_CAT_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
} /* qmi_cat_scws_close_channel */


/*===========================================================================
  FUNCTION  qmi_cat_scws_send_data
===========================================================================*/
/*!
@brief
  Used to convey the result of the operation of sending data to the socket for
  the send data indication posted to the control point. This function is
  always executed synchronously.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value in rsp_data
  will give you the QMI error reason. Otherwise, qmi_err_code will have
  meaningless data.

@note
  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_cat_scws_send_data
(
  int                                            client_handle,
  const qmi_cat_scws_send_data_params_type     * params,
  int                                          * qmi_err_code
)
{
  unsigned char     msg[QMI_CAT_STD_MSG_SIZE];
  unsigned char     param_buf[QMI_CAT_MAX_PARAM_BUF_SIZE];

  int               msg_size      = 0;
  unsigned char   * tmp_msg_ptr   = NULL;
  unsigned char   * tmp_param_ptr = NULL;
  int               rc            = 0;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  /*Prepare the Final TLV*/
  tmp_msg_ptr   = QMI_SRVC_PDU_PTR(msg);
  msg_size      = QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE);
  tmp_param_ptr = param_buf;

  /* Construct channel data result TLV */
  WRITE_32_BIT_VAL (tmp_param_ptr, params->channel_id);
  WRITE_8_BIT_VAL (tmp_param_ptr, params->result);

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_MANDATORY_1_TLV_ID,
                               sizeof(uint32_t) + sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  /* Next construct the Slot ID TLV */
  tmp_param_ptr = param_buf;
  WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)params->slot_id));

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_OPTIONAL_1_TLV_ID,
                               sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_CAT_SERVICE,
                                  QMI_CAT_SCWS_SEND_DATA_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_CAT_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
} /* qmi_cat_scws_send_data */


/*===========================================================================
  FUNCTION  qmi_cat_scws_data_available
===========================================================================*/
/*!
@brief
  Used to send the data written on the socket from the client to the card.
  This function is always executed synchronously.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value in rsp_data
  will give you the QMI error reason. Otherwise, qmi_err_code will have
  meaningless data.

@note
  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_cat_scws_data_available
(
  int                                                client_handle,
  const qmi_cat_scws_data_available_params_type    * params,
  qmi_cat_async_command_cb_type                      user_cb,
  void                                             * user_data,
  int                                              * qmi_err_code
)
{
  unsigned char      msg[QMI_MAX_MSG_SIZE];
  int                msg_size            = 0;
  unsigned char    * tmp_msg_ptr         = NULL;
  unsigned char    * allocated_param_ptr = NULL;
  unsigned char    * tmp_param_ptr       = NULL;
  unsigned int       max_msg_size        = 0;
  unsigned long      size                = 0;
  int                rc                  = 0;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  /* Calculate max buffer size - (params data + headers) */
  max_msg_size = (params->data_len <= 0) ? QMI_CAT_STD_MSG_SIZE :
                 ((unsigned int)params->data_len + QMI_CAT_STD_MSG_SIZE);

  /* Allocate buffer for the message dynamically */
  allocated_param_ptr = (unsigned char *) malloc (max_msg_size);
  if (!allocated_param_ptr)
  {
    QMI_ERR_MSG_0 ("Dynamic memory allocation failed for qmi_cat_scws_data_available\n");
    return QMI_INTERNAL_ERR;
  }

  QMI_DEBUG_MSG_1("qmi_cat_scws_data_available: %d bytes allocated for msg", max_msg_size);

  /*Prepare the Final TLV*/
  tmp_msg_ptr   = QMI_SRVC_PDU_PTR(msg);
  msg_size      = QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE);
  tmp_param_ptr = allocated_param_ptr;

  /* Construct channel data TLV */
  size = sizeof(uint32_t) + sizeof(uint16_t);
  WRITE_32_BIT_VAL (tmp_param_ptr, params->channel_id);
  WRITE_16_BIT_VAL (tmp_param_ptr, params->data_len);
  if ((params->data_ptr != NULL) && (params->data_len > 0))
  {
    memcpy (tmp_param_ptr, params->data_ptr, params->data_len);
    tmp_param_ptr += params->data_len;
    size += params->data_len;
  }

  if (size > (unsigned long)msg_size)
  {
    QMI_ERR_MSG_0 ("Error in buffer size: max_msg_size is more than QMI_MAX_MSG_SIZE\n");
    free(allocated_param_ptr);
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_MANDATORY_1_TLV_ID,
                               size,
                               (void *)(tmp_param_ptr - size));
  if (rc < 0)
  {
    goto data_available_ret;
  }

  /* Construct remaining data length TLV */
  WRITE_16_BIT_VAL (tmp_param_ptr, params->remaining_data_len);
  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_MANDATORY_2_TLV_ID,
                               sizeof(uint16_t),
                               (void *)(tmp_param_ptr - sizeof(uint16_t)));
  if (rc < 0)
  {
    goto data_available_ret;
  }

  /* Construct Slot ID TLV */
  WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)params->slot_id));
  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_OPTIONAL_1_TLV_ID,
                               sizeof(uint8_t),
                               (void *)(tmp_param_ptr - sizeof(uint8_t)));
  if (rc < 0)
  {
    goto data_available_ret;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_CAT_SERVICE,
                                     QMI_CAT_SCWS_DATA_AVAILABLE_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE) - msg_size,
                                     qmi_cat_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_CAT_SERVICE,
                                    QMI_CAT_SCWS_DATA_AVAILABLE_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_MAX_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    qmi_err_code);
  }

data_available_ret:
  free(allocated_param_ptr);
  return rc;
} /* qmi_cat_scws_data_available */


/*===========================================================================
  FUNCTION  qmi_cat_scws_channel_status
===========================================================================*/
/*!
@brief
  Used to send the status of the channel to the card. This function is always
  executed synchronously.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value in rsp_data
  will give you the QMI error reason. Otherwise, qmi_err_code will have
  meaningless data.

@note
  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_cat_scws_channel_status
(
  int                                            client_handle,
  const qmi_cat_scws_channel_params_type       * params,
  int                                          * qmi_err_code
)
{
  unsigned char     msg[QMI_CAT_STD_MSG_SIZE];
  unsigned char     param_buf[QMI_CAT_MAX_PARAM_BUF_SIZE];

  int               msg_size      = 0;
  unsigned char   * tmp_msg_ptr   = NULL;
  unsigned char   * tmp_param_ptr = NULL;
  int               rc            = 0;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  /*Prepare the Final TLV*/
  tmp_msg_ptr   = QMI_SRVC_PDU_PTR(msg);
  msg_size      = QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE);
  tmp_param_ptr = param_buf;

  /* Construct channel state TLV */
  WRITE_32_BIT_VAL (tmp_param_ptr, params->channel_id);
  WRITE_8_BIT_VAL (tmp_param_ptr, params->channel_state);

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_MANDATORY_1_TLV_ID,
                               sizeof(uint32_t) + sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  /* Next construct the Slot ID TLV */
  tmp_param_ptr = param_buf;
  WRITE_8_BIT_VAL(tmp_param_ptr, ((uint8_t)params->slot_id));

  rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                               &msg_size,
                               QMI_CAT_OPTIONAL_1_TLV_ID,
                               sizeof(uint8_t),
                               (void *)param_buf);
  if (rc < 0)
  {
    return rc;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_CAT_SERVICE,
                                  QMI_CAT_SCWS_CHANNEL_STATUS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_CAT_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_CAT_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
} /* qmi_cat_scws_channel_status */


