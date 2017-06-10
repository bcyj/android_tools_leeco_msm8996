#ifndef QMI_CAT_SRVC_H
#define QMI_CAT_SRVC_H

/******************************************************************************
  @file    qmi_cat_srvc.h
  @brief   QMI message library CAT service definitions

  $Id: //depot/asic/sandbox/users/anilt/QMI_UIM/qmi/inc/qmi_cat_srvc.h#3 $

  DESCRIPTION
  This file contains common, external header file definitions for QMI
  interface library.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_cat_srvc_init_client() must be called to create one or more clients
  qmi_cat_srvc_release_client() must be called to delete each client when
  finished.

  ---------------------------------------------------------------------------
  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
  Basic types
---------------------------------------------------------------------------*/

typedef enum
{
  QMI_CAT_FALSE     = 0x00,
  QMI_CAT_TRUE      = 0x01
}qmi_cat_bool_type;


/*---------------------------------------------------------------------------
  ENUMS
---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
   ENUM:      QMI_CAT_CONFIRMATION_TYPE

   DESCRIPTION:
     Type of confirmation
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_CAT_CONF_FALSE     = 0x00,
  QMI_CAT_CONF_TRUE      = 0x01,
  QMI_CAT_CONF_NOT_SET   = 0x02,
}qmi_cat_confirmation_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_CAT_ENVELOPE_CMD_TYPE

   DESCRIPTION:
     Type of envelope command
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_CAT_ENVELOPE_TYPE_MENU_SELECTION                  = 1,
  QMI_CAT_ENVELOPE_TYPE_EVENT_DL_USER_ACTIVITY          = 2,
  QMI_CAT_ENVELOPE_TYPE_EVENT_DL_IDLE_SCREEN_AVAILABLE  = 3,
  QMI_CAT_ENVELOPE_TYPE_EVENT_DL_LANGUAGE_SELECTION     = 4,
  QMI_CAT_ENVELOPE_TYPE_UNKNOWN                         = 5,
  QMI_CAT_ENVELOPE_TYPE_EVENT_DL_BROWSER_TERM           = 6
} qmi_cat_envelope_cmd_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_CAT_REPORT_FORMAT_TYPE

   DESCRIPTION:
     Format of set event report
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_CAT_SET_EVENT_REPORT_RAW                  = 1,
  QMI_CAT_SET_EVENT_REPORT_DECODED              = 2
} qmi_cat_report_format_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_CAT_SLOT_ID_TYPE

   DESCRIPTION:
     Slot id
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_CAT_SLOT_1                                        = 1,
  QMI_CAT_SLOT_2                                        = 2,
  QMI_CAT_SLOT_3                                        = 3
} qmi_cat_slot_id_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_CAT_SCWS_CHANNEL_STATE_TYPE

   DESCRIPTION:
     Channel state
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_CAT_SCWS_CHANNEL_STATE_CLOSED                     = 0x00,
  QMI_CAT_SCWS_CHANNEL_STATE_LISTEN                     = 0x01,
  QMI_CAT_SCWS_CHANNEL_STATE_ESTABLISHED                = 0x02
} qmi_cat_scws_channel_state_type;

/* -----------------------------------------------------------------------------
   ENUM:      QMI_CAT_INDICATION_ID_TYPE

   DESCRIPTION:
     Indicates the type of QMI INDICATION
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_CAT_SRVC_DISPLAY_TEXT_IND,
  QMI_CAT_SRVC_GET_INKEY_IND,
  QMI_CAT_SRVC_GET_INPUT_IND,
  QMI_CAT_SRVC_SETUP_MENU_IND,
  QMI_CAT_SRVC_SELECT_ITEM_IND,
  QMI_CAT_SRVC_SETUP_EVENT_LIST_IND,
  QMI_CAT_SRVC_SETUP_IDLE_MODE_TEXT_IND,
  QMI_CAT_SRVC_LANGUAGE_NOTIFICATION_IND,
  QMI_CAT_SRVC_PLAY_TONE_IND,
  QMI_CAT_SRVC_SETUP_CALL_IND,
  QMI_CAT_SRVC_SEND_SMS_IND,
  QMI_CAT_SRVC_SEND_DTMF_IND,
  QMI_CAT_SRVC_LAUNCH_BROWSER_IND,
  QMI_CAT_SRVC_SEND_SS_IND,
  QMI_CAT_SRVC_SEND_USSD_IND,
  QMI_CAT_SRVC_PROVIDE_LOCAL_INFO_IND,
  QMI_CAT_SRVC_SETUP_EVENT_LIST_RAW_IND,
  QMI_CAT_SRVC_END_PROACTIVE_SESSION_IND,
  QMI_CAT_SRVC_OPEN_CHANNEL_IND,
  QMI_CAT_SRVC_CLOSE_CHANNEL_IND,
  QMI_CAT_SRVC_SEND_DATA_IND,
  QMI_CAT_SRVC_RECEIVE_DATA_IND,
  QMI_CAT_SRVC_SCWS_OPEN_CHANNEL_IND,
  QMI_CAT_SRVC_SCWS_CLOSE_CHANNEL_IND,
  QMI_CAT_SRVC_SCWS_SEND_DATA_IND,
  QMI_CAT_SRVC_REFRESH_ALPHA_IND
} qmi_cat_indication_id_type;


/* -----------------------------------------------------------------------------
   ENUM:      QMI_CAT_RSP_ID_TYPE

   DESCRIPTION:
     Indicates the type of QMI response
-------------------------------------------------------------------------------*/
typedef enum
{
  QMI_CAT_SRVC_NONE_RSP_MSG,
  QMI_CAT_SRVC_SET_EVENT_REPORT_RSP_MSG,
  QMI_CAT_SRVC_SEND_TR_RSP_MSG,
  QMI_CAT_SRVC_SEND_ENVELOPE_RSP_MSG,
  QMI_CAT_SRVC_EVENT_CONFIRMATION_RSP_MSG,
  QMI_CAT_SRVC_SCWS_OPEN_CHANNEL_RSP_MSG,
  QMI_CAT_SRVC_SCWS_CLOSE_CHANNEL_RSP_MSG,
  QMI_CAT_SRVC_SCWS_SEND_DATA_RSP_MSG,
  QMI_CAT_SRVC_SCWS_DATA_AVAILABLE_RSP_MSG,
  QMI_CAT_SRVC_SCWS_CHANNEL_STATUS_RSP_MSG
} qmi_cat_rsp_id_type;


/*---------------------------------------------------------------------------
  STRUCTURES USED FOR QMI REQUESTS
---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SET_EVENT_REPORT_PARAMS_TYPE

   DESCRIPTION:   Structure used for set event report command
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_cat_report_format_type       format;
  unsigned char                    slot_mask;
  qmi_cat_bool_type                display_test;
  qmi_cat_bool_type                get_inkey;
  qmi_cat_bool_type                get_input;
  qmi_cat_bool_type                setup_menu;
  qmi_cat_bool_type                select_item;
  qmi_cat_bool_type                send_sms;
  qmi_cat_bool_type                setup_event_user_activity;
  qmi_cat_bool_type                setup_event_idle_screen_notify;
  qmi_cat_bool_type                setup_event_language_sel_notify;
  qmi_cat_bool_type                setup_idle_mode_text;
  qmi_cat_bool_type                language_notification;
  qmi_cat_bool_type                refresh_alpha;
  qmi_cat_bool_type                end_proactive_session;
  qmi_cat_bool_type                play_tone;
  qmi_cat_bool_type                setup_call;
  qmi_cat_bool_type                send_dtmf;
  qmi_cat_bool_type                launch_browser;
  qmi_cat_bool_type                send_ss;
  qmi_cat_bool_type                send_ussd;
  qmi_cat_bool_type                provide_local_info_lang;
  qmi_cat_bool_type                provide_local_info_time;
  qmi_cat_bool_type                bip;
  qmi_cat_bool_type                setup_event_browser_term;
  qmi_cat_bool_type                scws;
} qmi_cat_set_event_report_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SEND_TR_PARAMS_TYPE

   DESCRIPTION:   Structure used for send TR command
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned int                     uim_ref_id;
  unsigned short                   tr_length;
  unsigned char                  * tr_value;
  qmi_cat_slot_id_type             slot_id;
} qmi_cat_send_tr_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SEND_ENVELOPE_PARAMS_TYPE

   DESCRIPTION:   Structure used for send envelope command
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_cat_envelope_cmd_type        envelope_cmd_type;
  unsigned short                   envelope_length;
  unsigned char                  * envelope_value;
  qmi_cat_slot_id_type             slot_id;
} qmi_cat_send_envelope_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_EVENT_CONFIRMATION_PARAMS_TYPE

   DESCRIPTION:   Structure used for event confirmation command
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_cat_confirmation_type        user;
  qmi_cat_confirmation_type        icon;
  qmi_cat_slot_id_type             slot_id;
} qmi_cat_event_confirmation_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SCWS_CHANNEL_PARAMS_TYPE

   DESCRIPTION:   Structure used for open/close channel request
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned int                       channel_id;
  qmi_cat_slot_id_type               slot_id;
  qmi_cat_scws_channel_state_type    channel_state;
} qmi_cat_scws_channel_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SCWS_SEND_DATA_PARAMS_TYPE

   DESCRIPTION:   Structure used for send data request
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned int                       channel_id;
  qmi_cat_slot_id_type               slot_id;
  qmi_cat_bool_type                  result;
} qmi_cat_scws_send_data_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SCWS_DATA_AVAILABLE_PARAMS_TYPE

   DESCRIPTION:   Structure used for data available request
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned int                       channel_id;
  qmi_cat_slot_id_type               slot_id;
  unsigned short                     data_len;
  unsigned short                     remaining_data_len;
  unsigned char                    * data_ptr;
} qmi_cat_scws_data_available_params_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SEND_ENVELOPE_RSP_TYPE

   DESCRIPTION:   Structure used for envelope response.
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned char                      sw1;
  unsigned char                      sw2;
  unsigned short                     env_resp_data_len;
  unsigned char                    * env_resp_data_ptr;
} qmi_cat_send_envelope_rsp_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_GENERIC_RAW_IND_TYPE

   DESCRIPTION:   Structure used for generic proactive commands
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned int                     uim_ref_id;
  unsigned short                   length;
  unsigned char                  * value;
  qmi_cat_slot_id_type             slot_id;
} qmi_cat_generic_raw_ind_type;


/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SETUP_EVENT_LIST_IND_TYPE

   DESCRIPTION:   Structure used for setup event list proactive command
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_cat_bool_type                user_activity;
  qmi_cat_bool_type                idle_screen_available;
  qmi_cat_bool_type                language_selection;
  qmi_cat_bool_type                browser_termination;
  qmi_cat_slot_id_type             slot_id;
} qmi_cat_setup_event_list_ind_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_END_SESSION_IND_TYPE

   DESCRIPTION:   Structure used for end proactive session command. This
                  is a dummy structure and the value is not really meaningful.
-------------------------------------------------------------------------------*/
typedef struct
{
  qmi_cat_slot_id_type             slot_id;
} qmi_cat_end_session_ind_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SCWS_OPEN_CHANNEL_IND_TYPE

   DESCRIPTION:   Structure used for SCWS open channel indication.
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned int                       channel_id;
  unsigned short                     port;
  unsigned short                     buffer_size;
  qmi_cat_slot_id_type               slot_id;
} qmi_cat_scws_open_channel_ind_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SCWS_CLOSE_CHANNEL_IND_TYPE

   DESCRIPTION:   Structure used for SCWS close channel indication.
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned int                       channel_id;
  qmi_cat_scws_channel_state_type    channel_state;
  qmi_cat_slot_id_type               slot_id;
} qmi_cat_scws_close_channel_ind_type;

/* -----------------------------------------------------------------------------
   STRUCTURE:    QMI_CAT_SCWS_SEND_DATA_IND_TYPE

   DESCRIPTION:   Structure used for SCWS send data indication.
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned int                       channel_id;
  qmi_cat_slot_id_type               slot_id;
  unsigned char                      total_packets;
  unsigned char                      current_packet;
  unsigned short                     data_len;
  unsigned char                    * data_ptr;
} qmi_cat_scws_send_data_ind_type;


/*---------------------------------------------------------------------------
  QMI INDICATIONS
---------------------------------------------------------------------------*/
typedef union
{
  qmi_cat_generic_raw_ind_type           display_text_ind;
  qmi_cat_generic_raw_ind_type           get_inkey_ind;
  qmi_cat_generic_raw_ind_type           get_input_ind;
  qmi_cat_generic_raw_ind_type           setup_menu_ind;
  qmi_cat_generic_raw_ind_type           select_item_ind;
  qmi_cat_setup_event_list_ind_type      setup_event_list_ind;
  qmi_cat_generic_raw_ind_type           setup_idle_mode_text_ind;
  qmi_cat_generic_raw_ind_type           language_notification_ind;
  qmi_cat_generic_raw_ind_type           play_tone_ind;
  qmi_cat_generic_raw_ind_type           setup_call_ind;
  qmi_cat_generic_raw_ind_type           send_sms_ind;
  qmi_cat_generic_raw_ind_type           send_dtmf_ind;
  qmi_cat_generic_raw_ind_type           launch_browser_ind;
  qmi_cat_generic_raw_ind_type           send_ss_ind;
  qmi_cat_generic_raw_ind_type           send_ussd_ind;
  qmi_cat_generic_raw_ind_type           provide_local_info_ind;
  qmi_cat_generic_raw_ind_type           setup_event_list_raw_ind;
  qmi_cat_end_session_ind_type           end_proactive_session_ind;
  qmi_cat_generic_raw_ind_type           open_channel_ind;
  qmi_cat_generic_raw_ind_type           close_channel_ind;
  qmi_cat_generic_raw_ind_type           send_data_ind;
  qmi_cat_generic_raw_ind_type           receive_data_ind;
  qmi_cat_scws_open_channel_ind_type     scws_open_channel_ind;
  qmi_cat_scws_close_channel_ind_type    scws_close_channel_ind;
  qmi_cat_scws_send_data_ind_type        scws_send_data_ind;
  qmi_cat_generic_raw_ind_type           refresh_alpha_ind;
} qmi_cat_indication_data_type;

typedef void (*qmi_cat_indication_hdlr_type)
(
  int                            user_handle,
  qmi_service_id_type            service_id,
  void                         * user_data,
  qmi_cat_indication_id_type     ind_id,
  qmi_cat_indication_data_type * ind_data_ptr
);


/*---------------------------------------------------------------------------
  QMI asynchronous response
---------------------------------------------------------------------------*/
typedef struct
{
  int                                       sys_err_code;
  int                                       qmi_err_code;
  qmi_cat_rsp_id_type                       rsp_id;
} qmi_cat_rsp_data_type;


typedef void (*qmi_cat_async_command_cb_type)
(
  int                            user_handle,
  qmi_service_id_type            service_id,
  qmi_cat_rsp_data_type        * rsp_data_ptr,
  void                         * user_data
);


/*===========================================================================
  FUNCTION  qmi_cat_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the CAT service.  This function
  must be called prior to calling any other CAT service functions.
  Note that this function may be called multiple times to allow for
  multiple, independent clients.

@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
EXTERN qmi_client_handle_type qmi_cat_srvc_init_client
(
  const char                   * dev_id,
  qmi_cat_indication_hdlr_type   user_rx_ind_msg_hdlr,
  void                         * user_rx_ind_msg_hdlr_user_data,
  int                          * qmi_err_code
);

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
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
EXTERN int qmi_cat_srvc_release_client
(
  int      user_handle,
  int    * qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_cat_reset
===========================================================================*/
/*!
@brief
  Resets CAT service. This function is always executed synchronously.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value in rsp_data
  will give you the QMI error reason. Otherwise, qmi_err_code will have
  meaningless data.

@note
  - Dependencies
    - qmi_cat_srvc_init_client() must be called before calling this.

  - Side Effects
    - Resets CAT service
*/
/*=========================================================================*/
EXTERN int qmi_cat_reset
(
  int      user_handle,
  int    * qmi_err_code
);

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
EXTERN int qmi_cat_set_event_report
(
  int                                           client_handle,
  const qmi_cat_set_event_report_params_type  * params,
  int                                         * qmi_err_code
);

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
EXTERN int qmi_cat_send_tr
(
  int                                 client_handle,
  const qmi_cat_send_tr_params_type * params,
  int                               * qmi_err_code
);

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
EXTERN int qmi_cat_send_envelope
(
  int                                       client_handle,
  const qmi_cat_send_envelope_params_type * req_params,
  qmi_cat_send_envelope_rsp_type          * rsp_params,
  int                                     * qmi_err_code
);

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
EXTERN int qmi_cat_event_confirmation
(
  int                                            client_handle,
  const qmi_cat_event_confirmation_params_type * params,
  int                                          * qmi_err_code
);

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
EXTERN int qmi_cat_scws_open_channel
(
  int                                            client_handle,
  const qmi_cat_scws_channel_params_type       * params,
  int                                          * qmi_err_code
);

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
EXTERN int qmi_cat_scws_close_channel
(
  int                                            client_handle,
  const qmi_cat_scws_channel_params_type       * params,
  int                                          * qmi_err_code
);

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
EXTERN int qmi_cat_scws_send_data
(
  int                                            client_handle,
  const qmi_cat_scws_send_data_params_type     * params,
  int                                          * qmi_err_code
);

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
EXTERN int qmi_cat_scws_data_available
(
  int                                                client_handle,
  const qmi_cat_scws_data_available_params_type    * params,
  qmi_cat_async_command_cb_type                      user_cb,
  void                                             * user_data,
  int                                              * qmi_err_code
);

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
EXTERN int qmi_cat_scws_channel_status
(
  int                                            client_handle,
  const qmi_cat_scws_channel_params_type       * params,
  int                                          * qmi_err_code
);

#define setup_event_list_raw	send_ussd


#ifdef __cplusplus
}
#endif

#endif  /* QMI_CAT_SRVC_H */
