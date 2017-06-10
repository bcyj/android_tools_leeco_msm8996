/*===========================================================================
  Copyright (c) 2013, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/30/13   xj      Initial version
03/27/14   xj      Add log print for power down mode
04/18/14   xj      Use bridge type to determine enumerate/terminate actions
04/25/14   xj      Close ccid bridge before unload usb module
05/20/14   xj      Correct assert variable in usb_uicc_send_apdu_req
06/04/14   xj      Add logs in case of FAILURE APDU request
06/23/14   xj      Added QMI nitialization retry mechanism
07/04/14   xj      Add response timeout interface to usb ccid driver
09/29/14   xj      Add SSR support
10/20/14   xj      Use default persist prefix as sys.*
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <cutils/properties.h>
#include "usb_uicc_daemon.h"
#include "usb_uicc_qmi.h"

/*===========================================================================

                           MACROS

===========================================================================*/
/* Android system property for fetching the modem type */
#define USB_UICC_PROPERTY_BASEBAND                                "ro.baseband"

/* Android system property values for various modem types */
#define USB_UICC_PROP_BASEBAND_VALUE_SVLTE_1                      "svlte1"
#define USB_UICC_PROP_BASEBAND_VALUE_SVLTE_2A                     "svlte2a"
#define USB_UICC_PROP_BASEBAND_VALUE_CSFB                         "csfb"
#define USB_UICC_PROP_BASEBAND_VALUE_MSM                          "msm"
#define USB_UICC_PROP_BASEBAND_VALUE_APQ                          "apq"
#define USB_UICC_QMI_SVC_TIMEOUT_MS                               1000 /* Time Out in ms */
#define USB_UICC_QMI_INIT_MAX_RETRIES                             30
#define USB_UICC_ENABLED                                          "sys.usb_uicc.enabled"
#define USB_UICC_LOADING                                          "sys.usb_uicc.loading"
/*===========================================================================

                           GLOBALS

===========================================================================*/
extern usb_uicc_global_data_type      usb_uicc_global;
/* usb uicc power up states */
extern usb_uicc_state_enum_type       USB_UICC_POWER_UP_STATES[];
/* usb uicc power down states */
extern usb_uicc_state_enum_type       USB_UICC_POWER_DOWN_STATES[];
/* usb uicc apdu states */
extern usb_uicc_state_enum_type       USB_UICC_APDU_STATES[];
/* Global mutex */
extern pthread_mutex_t                usb_uicc_mutex;
/* Global semaphore */
extern sem_t                          usb_uicc_sem;
#ifdef USB_UICC_UT
extern sem_t                          usb_uicc_ut_sem;
#endif
/* heap buffer used to store apdu from remote uim server */
static uint8                        * apdu_mem_buffer_ptr = NULL;
/* static buffer used to store apdu from remote uim server */
static uint8                          apdu_static_buffer [STATIC_BUFFER_MAX_SIZE];

/*===========================================================================

                               EXTERNAL FUNCTIONS

===========================================================================*/


/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/
static boolean usb_uicc_send_evt_req
(
  uim_remote_event_req_msg_v01 * evt_request_ptr
);
static boolean usb_uicc_send_apdu_req
(
  uim_remote_apdu_req_msg_v01 * apdu_request_ptr
);
static void usb_uicc_power_up_cmd
(
  uim_remote_card_power_up_ind_msg_v01 *card_power_up_ind_msg_ptr
);
static void usb_uicc_power_down_cmd
(
  uim_remote_card_power_down_ind_msg_v01 *card_power_down_ind_msg_ptr
);
static void usb_uicc_send_apdu_cmd
(
  uim_remote_apdu_ind_msg_v01 *apdu_ind_msg_ptr
);
static void usb_uicc_reset_cmd
(
  uim_remote_card_reset_ind_msg_v01* reset_ind_msg_ptr
);
#ifndef USB_UICC_QMI_UT
static void usb_uicc_client_async_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                         * qmi_response_ptr,
  unsigned int                   resp_c_struct_len,
  void                         * resp_cb_data_ptr,
  qmi_client_error_type          transp_err
);
static void usb_uicc_client_indication_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                         * ind_buf_ptr,
  int                            ind_buf_len,
  void                         * ind_cb_data
);

#endif /* USB_UICC_QMI_UT */
#ifdef USB_UICC_QMI_UT
static void usb_uicc_ut_thread
(
  void * in_param
);
#endif /* USB_UICC_QMI_UT */

/*=========================================================================

  FUNCTION:  usb_uicc_send_write

===========================================================================*/

/*!
    @brief
    Performs to print data buffer.

    @return
    boolean.
*/
/*=========================================================================*/
void usb_uicc_print_data
(
  const uint8                  *data_ptr,
  uint32                        data_len
)
{
  #define USB_UICC_MAX_BYTES_PER_LINE    16
  #define USB_UICC_MAX_BUF_SIZE          ((USB_UICC_MAX_BYTES_PER_LINE * 5) + 2)
  uint32 i = 0;
  char  *p = NULL;
  char  log_buffer[USB_UICC_MAX_BUF_SIZE] = {0};
  unsigned char val;

  if ((data_len <= 0) || (NULL == data_ptr))
  {
    LOGI("PRINT DATA: Invalid input param data_ptr 0x%x data len %d \n",
         data_ptr, (int)data_len);
    return;
  }

  while (data_len > 0)
  {
    p = log_buffer;

    for (i = 0; (i < USB_UICC_MAX_BYTES_PER_LINE) && (data_len > 0); i++)
    {
      /* First digit */
      val = ( *data_ptr >> 4 ) & 0x0F;
      if (val <= 9)
      {
        *p++ = val + '0';
      }
      else
      {
        *p++ = (val - 10) + 'A';
      }

      /* Second digit... ugly copied code */
      val = *data_ptr & 0x0F;
      if (val <= 9)
      {
        *p++ = val + '0';
      }
      else
      {
        *p++ = (val - 10) + 'A';
      }

      /* Add a space, and increment data_ptr */
      *p++ = ' ';
      data_ptr++;
      data_len--;
    }

    /* Add \n and NULL terminator and print out */
    *p++ = '\n';
    *p = '\0';
    LOGD("Hex bytes: %s", log_buffer);
  }

} /* usb_uicc_print_data */

/*=========================================================================

  FUNCTION:  usb_uicc_cancel_thread

===========================================================================*/
/*!
    @brief
    To cancel open/ioctl thread.

    @return
    None.
*/
/*=========================================================================*/
void usb_uicc_cancel_thread
(
  pthread_t pthread_id
)
{
  int status = 0;
  if ((-1 == pthread_id) || (pthread_kill(pthread_id, 0) == ESRCH))
  {
    LOGI("thread id (0x%x) already exit\n", (unsigned int)pthread_id);
    return;
  }

  status = pthread_kill(pthread_id, SIGUSR1);
  if (0 == status)
  {
    usleep(100); /* waiting */
    status = pthread_kill(pthread_id, 0); /* get thread status */
    if (ESRCH == status)
    {
      LOGI("thread id (0x%x) does not exit or already exit!\n", (unsigned int)pthread_id);
    }
    else if (EINVAL == status)
    {
      LOGE("invalid signal!\n");
    }
    else
    {
      LOGE("failed -- thread id (0x%x) is still alive!!\n", (unsigned int)pthread_id);
    }
  }
  else
  {
    LOGE("failed to exit thread id (0x%x)!\n", (unsigned int)pthread_id);
  }
} /* usb_uicc_cancel_thread */

/*=========================================================================

  FUNCTION:  usb_uicc_open_ccid_device

===========================================================================*/
/*!
    @brief
    Load usb driver and open ccid.
    It will take about (1038.061 + 1500)ms totally in the worst case

    @return
    -1     failed
    others file descriptor
*/
/*=========================================================================*/
int usb_uicc_open_ccid_device
(
  const char * pathname,
  int          flags
)
{
  const int max_delay     = 180; /* max interval time */
  int       fd            = -1;
  int       i             = 0;
  char      value[PROPERTY_VALUE_MAX] = {'\0'};

  property_set(USB_UICC_LOADING, "0");
  property_set(USB_UICC_ENABLED, "1");

  /*
    Take 1038.061ms totally to query the flag in the worst case.
    Query interval time will be decreased as below.
    180ms, 90ms, 60ms, 45ms, 36ms, 30ms, ...
  */
  for (i = 1; i < max_delay; ++i)
  {
    usleep(1000*max_delay/i);
    if (property_get(USB_UICC_LOADING, value, "0") && (!strncmp(value, "1", 1)))
    {
      break;
    }
  }

  if (i == max_delay)
  {
    LOGE("fail to load usb driver!");
    return -1;
  }

  /*
    Try 3 times to open ccid bridge.
    500ms timeout for open ccid bridge.
    Totally, it will take 3*500 ms in the worst case.
  */
  for (i = 0; i < 3; ++i)
  {
    if ((fd = open(pathname, flags)) != -1)
    {
      break;
    }
  }
  return fd;
} /* usb_uicc_open_ccid_device */

/*=========================================================================

  FUNCTION:  usb_uicc_close_ccid_device

===========================================================================*/
/*!
    @brief
    close ccid and write card removal flag to hub enable fd.

    @return
    none.
*/
/*=========================================================================*/
void usb_uicc_close_ccid_device
(
  int fd
)
{
  if (-1 != fd)
  {
    close(fd);
  }

  property_set(USB_UICC_ENABLED, "0");
} /* usb_uicc_close_ccid_device */

/*=========================================================================

  FUNCTION:  usb_uicc_detach_usb

===========================================================================*/
/*!
    @brief
    Deattach usb if any error when read or write.

    @return
    None.
*/
/*=========================================================================*/
void usb_uicc_detach_usb
(
  void
)
{
  if (-1 != usb_uicc_global.listen_evt_thread_id)
  {
    usb_uicc_cancel_thread(usb_uicc_global.listen_evt_thread_id);
    usb_uicc_global.listen_evt_thread_id      = -1;
  }

  if (-1 != usb_uicc_global.open_thread_id)
  {
    usb_uicc_cancel_thread(usb_uicc_global.open_thread_id);
    usb_uicc_global.open_thread_id            = -1;
  }

  usb_uicc_close_ccid_device(usb_uicc_global.usb_uicc_fd);
  usb_uicc_global.usb_uicc_fd                 = -1;

  usb_uicc_global.card_info.card_state        = USB_UICC_CARD_ERROR;
  usb_uicc_global.usb_state                   = UIM_USB_STATE_DISCONNECTED;
} /* usb_uicc_detach_usb */

/*=========================================================================

  FUNCTION:  usb_uicc_reset_global_data

===========================================================================*/
/*!
    @brief
    To reset global data except qmi_msg_lib_handle and qmi_uim_svc_client_ptr.

    @return
    None.
*/
/*=========================================================================*/
void usb_uicc_reset_global_data
(
  boolean detach_usb
)
{
  /* check if need detach usb */
  if (TRUE == detach_usb)
  {
    usb_uicc_detach_usb();
    usb_uicc_global.usb_state = UIM_USB_STATE_UNKNOWN;
  }

  usb_uicc_global.usb_uicc_msg_seq           = 0x00;
  usb_uicc_global.is_hardware_error          = FALSE;
  usb_uicc_global.card_info.card_state       = USB_UICC_CARD_NOT_INIT;
  usb_uicc_global.card_info.atr_len          = 0;
  memset(usb_uicc_global.card_info.atr, 0x00, QMI_UIM_REMOTE_MAX_ATR_LEN_V01);

  /* delete timer*/
  if (0 != usb_uicc_global.enumeration_timer.timer_id)
  {
    timer_delete(usb_uicc_global.enumeration_timer.timer_id);
  }
  usb_uicc_global.enumeration_timer.timer_id      = 0;
  usb_uicc_global.enumeration_timer.timer_started = FALSE;

  if (0 != usb_uicc_global.poll_timer.timer_id)
  {
    timer_delete(usb_uicc_global.poll_timer.timer_id);
  }
  usb_uicc_global.poll_timer.timer_id      = 0;
  usb_uicc_global.poll_timer.timer_started = FALSE;

  memset(apdu_static_buffer, 0x00, sizeof(apdu_static_buffer));
  if (NULL != apdu_mem_buffer_ptr)
  {
    free(apdu_mem_buffer_ptr);
    apdu_mem_buffer_ptr = NULL;
  }
} /* usb_uicc_reset_global_data */

/*=========================================================================

  FUNCTION:  usb_uicc_send_evt_req

===========================================================================*/

/*!
    @brief
    Performs to send card event to qmi remote uim server.

    @return
    boolean.
*/
/*=========================================================================*/
static boolean usb_uicc_send_evt_req
(
  uim_remote_event_req_msg_v01   * evt_request_ptr
)
{
#ifndef USB_UICC_QMI_UT
  qmi_txn_handle                        txn_handle;
  qmi_client_error_type                 qmi_err_code   = 0;
  uim_remote_event_resp_msg_v01        *evt_resp_ptr   = NULL;

  assert (evt_request_ptr != NULL);

  evt_resp_ptr =
    (uim_remote_event_resp_msg_v01*)malloc(sizeof(uim_remote_event_resp_msg_v01));
  if(evt_resp_ptr == NULL)
  {
    LOGE("Couldn't allocate memory for uim_remote_event_resp_msg_v01 ! \n");
    return FALSE;
  }

  /* Init parameters */
  memset(evt_resp_ptr, 0, sizeof(uim_remote_event_resp_msg_v01));

  qmi_err_code = qmi_client_send_msg_async(usb_uicc_global.qmi_uim_svc_client_ptr,
                                           QMI_UIM_REMOTE_EVENT_REQ_V01,
                                           (void*) evt_request_ptr,
                                           sizeof(*evt_request_ptr),
                                           (void*) evt_resp_ptr,
                                           sizeof(*evt_resp_ptr),
                                           usb_uicc_client_async_cb,
                                           NULL,
                                           &txn_handle);

  if(qmi_err_code == QMI_NO_ERR)
  {
    if (0 == evt_request_ptr->error_cause_valid)
    {
      LOGI("card event to remote uim = 0x%x\n", evt_request_ptr->event_info.event);
    }
    else
    {
      LOGI("card event to remote uim = 0x%x, error_cause = %d\n", evt_request_ptr->event_info.event, evt_request_ptr->error_cause);
    }
    if (evt_request_ptr->event_info.event == UIM_REMOTE_CARD_INSERTED_V01)
    {
      LOGI("ATR = ");
      usb_uicc_print_data(evt_request_ptr->atr, evt_request_ptr->atr_len);
    }
    return TRUE;
  }

  /* On error free the allocated response buffer */
  LOGE("Error for QMI_UIM_REMOTE_EVENT_REQ_V01, qmi_err_code: 0x%x \n", qmi_err_code);
  if (evt_resp_ptr)
  {
    free(evt_resp_ptr);
    evt_resp_ptr = NULL;
  }
  return FALSE;
#else
  assert(evt_request_ptr != NULL);
  LOGI("card event = 0x%x\n", evt_request_ptr->event_info.event);
  if (evt_request_ptr->event_info.event == UIM_REMOTE_CARD_RESET_V01)
  {
    LOGI("ATR = ");
    usb_uicc_print_data(evt_request_ptr->atr, evt_request_ptr->atr_len);
  }
  sem_post(&usb_uicc_ut_sem);
  return TRUE;
#endif /* USB_UICC_QMI_UT */
} /* usb_uicc_send_evt_req */

/*=========================================================================

  FUNCTION:  usb_uicc_send_apdu_req

===========================================================================*/

/*!
    @brief
    Performs to send apdu response to qmi remote uim server.

    @return
    boolean.
*/
/*=========================================================================*/
static boolean usb_uicc_send_apdu_req
(
  uim_remote_apdu_req_msg_v01   * apdu_request_ptr
)
{
#ifndef USB_UICC_QMI_UT
  qmi_txn_handle                        txn_handle;
  qmi_client_error_type                 qmi_err_code   = 0;
  uim_remote_apdu_resp_msg_v01        * apdu_resp_ptr  = NULL;

  assert (apdu_request_ptr != NULL);

  apdu_resp_ptr =
    (uim_remote_apdu_resp_msg_v01*)malloc(sizeof(uim_remote_apdu_resp_msg_v01));
  if(apdu_resp_ptr == NULL)
  {
    LOGE("Couldn't allocate memory for uim_remote_apdu_resp_msg_v01 ! \n");
    return FALSE;
  }

  /* Init parameters */
  memset(apdu_resp_ptr, 0, sizeof(uim_remote_apdu_resp_msg_v01));

  qmi_err_code = qmi_client_send_msg_async(usb_uicc_global.qmi_uim_svc_client_ptr,
                                           QMI_UIM_REMOTE_APDU_REQ_V01,
                                           (void*) apdu_request_ptr,
                                           sizeof(*apdu_request_ptr),
                                           (void*) apdu_resp_ptr,
                                           sizeof(*apdu_resp_ptr),
                                           usb_uicc_client_async_cb,
                                           NULL,
                                           &txn_handle);

  if(qmi_err_code == QMI_NO_ERR)
  {
    LOGI("apdu response with id = %d, status = %d\n", apdu_request_ptr->apdu_id, apdu_request_ptr->apdu_status);
    return TRUE;
  }

  /* On error free the allocated response buffer */
  LOGE("Error for QMI_UIM_REMOTE_APDU_REQ_V01, qmi_err_code: 0x%x \n", qmi_err_code);
  if (apdu_resp_ptr)
  {
    free(apdu_resp_ptr);
    apdu_resp_ptr = NULL;
  }
  return FALSE;
#else
  assert(apdu_request_ptr != NULL);
  LOGI("apdu response with id = %d, status = %d\n", apdu_request_ptr->apdu_id, apdu_request_ptr->apdu_status);
  sem_post(&usb_uicc_ut_sem);
  return TRUE;
#endif /* USB_UICC_QMI_UT */
} /* usb_uicc_send_apdu_req */

/*=========================================================================

  FUNCTION:  usb_uicc_create_and_send_remote_evt_req

===========================================================================*/

/*!
    @brief
    Performs to create and send event to qmi remote uim server.

    @return
    boolean.
*/
/*=========================================================================*/
void usb_uicc_create_and_send_remote_evt_req
(
  uim_remote_event_type_enum_v01 evt_type
)
{
  uim_remote_event_req_msg_v01 * evt_req_ptr = NULL;
  evt_req_ptr = (uim_remote_event_req_msg_v01 *)malloc(sizeof(uim_remote_event_req_msg_v01));
  if(evt_req_ptr != NULL)
  {
    memset(evt_req_ptr, 0x00, sizeof(uim_remote_event_req_msg_v01));
    evt_req_ptr->event_info.event = evt_type;
    evt_req_ptr->event_info.slot  = usb_uicc_global.slot;

    switch (evt_type)
    {
      case UIM_REMOTE_CONNECTION_AVAILABLE_V01:
        evt_req_ptr->event_info.slot      = UIM_REMOTE_SLOT_1_V01; /* currently, only slot 1 supports usb uicc */
        evt_req_ptr->error_cause_valid    = FALSE;
        evt_req_ptr->atr_valid            = FALSE;
        evt_req_ptr->wakeup_support_valid = FALSE;
        break;
      case UIM_REMOTE_CONNECTION_UNAVAILABLE_V01:
        evt_req_ptr->error_cause_valid    = FALSE;
        evt_req_ptr->atr_valid            = FALSE;
        evt_req_ptr->wakeup_support_valid = FALSE;
        break;
      case UIM_REMOTE_CARD_ERROR_V01:
        evt_req_ptr->error_cause_valid    = TRUE;
        evt_req_ptr->error_cause          = usb_uicc_global.usb_uicc_cmd.cmd_err_code;
        evt_req_ptr->atr_valid            = FALSE;
        evt_req_ptr->wakeup_support_valid = FALSE;
        break;
      case UIM_REMOTE_CARD_WAKEUP_V01:
        evt_req_ptr->error_cause_valid    = FALSE;
        evt_req_ptr->atr_valid            = FALSE;
        evt_req_ptr->wakeup_support_valid = TRUE;
        evt_req_ptr->wakeup_support       = TRUE;
        break;
      case UIM_REMOTE_CARD_INSERTED_V01:
      case UIM_REMOTE_CARD_RESET_V01:
        evt_req_ptr->error_cause_valid    = FALSE;
        evt_req_ptr->wakeup_support_valid = FALSE;
        evt_req_ptr->atr_len              = usb_uicc_global.card_info.atr_len;
        evt_req_ptr->atr_valid            = TRUE;
        memcpy(evt_req_ptr->atr,
               usb_uicc_global.card_info.atr,
               usb_uicc_global.card_info.atr_len);

        break;
      default:
        LOGI("other evt here.\n");
    }

    if(!usb_uicc_send_evt_req(evt_req_ptr))
    {
      LOGE("usb_uicc_send_evt_req failed\n");
    }
    free(evt_req_ptr);
  }
  else
  {
    LOGE("Couldn't allocate memory for uim_remote_event_req_msg_v01\n");
  }
} /* usb_uicc_create_and_send_remote_evt_req */

/*=========================================================================

  FUNCTION:  usb_uicc_create_and_send_remote_apdu_req

===========================================================================*/

/*!
    @brief
    Performs to create and send apdu response to qmi remote uim server.

    @return
    None.
*/
/*=========================================================================*/
void usb_uicc_create_and_send_remote_apdu_req
(
  qmi_result_type_v01 qmi_result_type
)
{
  uim_remote_apdu_req_msg_v01 *apdu_req_ptr = NULL;
  apdu_req_ptr = (uim_remote_apdu_req_msg_v01 *)malloc(sizeof(uim_remote_apdu_req_msg_v01));

  if (apdu_req_ptr != NULL)
  {
    memset(apdu_req_ptr, 0x00, sizeof(uim_remote_apdu_req_msg_v01));
    apdu_req_ptr->apdu_id = usb_uicc_global.usb_uicc_cmd.apdu_id;
    apdu_req_ptr->slot = usb_uicc_global.slot;
    apdu_req_ptr->apdu_status = qmi_result_type;

    if (qmi_result_type == QMI_RESULT_FAILURE_V01)
    {
      apdu_req_ptr->response_apdu_info_valid = FALSE;
      apdu_req_ptr->response_apdu_segment_valid = FALSE;
      LOGI("failure R-APDU to modem");
    }
    else if (qmi_result_type == QMI_RESULT_SUCCESS_V01)
    {
      apdu_req_ptr->response_apdu_info_valid = TRUE;
      apdu_req_ptr->response_apdu_info.response_apdu_segment_offset = 0;
      apdu_req_ptr->response_apdu_segment_valid = TRUE;

      if (usb_uicc_global.usb_uicc_cmd.rsp_payload_len <= QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01)
      {
        apdu_req_ptr->response_apdu_info.total_response_apdu_size = usb_uicc_global.usb_uicc_cmd.rsp_payload_len;
        apdu_req_ptr->response_apdu_segment_len = usb_uicc_global.usb_uicc_cmd.rsp_payload_len;
        memcpy(apdu_req_ptr->response_apdu_segment,
               usb_uicc_global.usb_uicc_cmd.rsp_payload_ptr,
               usb_uicc_global.usb_uicc_cmd.rsp_payload_len);
      }
      else
      {
        apdu_req_ptr->response_apdu_info.total_response_apdu_size = QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01;
        apdu_req_ptr->response_apdu_segment_len = QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01;
        memcpy(apdu_req_ptr->response_apdu_segment,
               usb_uicc_global.usb_uicc_cmd.rsp_payload_ptr,
               QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01);
        LOGE("response of apdu to remote uim server is truncated.\n");
      }
      LOGI("success R-APDU to modem: ");
      usb_uicc_print_data(apdu_req_ptr->response_apdu_segment, apdu_req_ptr->response_apdu_segment_len);
    }
    else
    {
      LOGE("never hit here!\n");
    }

    if(!usb_uicc_send_apdu_req(apdu_req_ptr))
    {
      LOGE("usb_uicc_send_apdu_req failed\n");
    }
    free(apdu_req_ptr);
  }
  else
  {
    LOGE("Couldn't allocate memory for uim_remote_apdu_req_msg_v01\n");
  }
} /* usb_uicc_create_and_send_remote_apdu_req */


/*=========================================================================

  FUNCTION:  usb_uicc_power_up

===========================================================================*/

/*!
    @brief
    To start power up procedure.

    @return
    None.
*/
/*=========================================================================*/
static void usb_uicc_power_up_cmd
(
  uim_remote_card_power_up_ind_msg_v01 *card_power_up_ind_msg_ptr
)
{
  assert (NULL != card_power_up_ind_msg_ptr);
  LOGI("received power up command.\n");

  USB_UICC_ENTER_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");
  usb_uicc_global.usb_uicc_cmd.command           = USB_UICC_POWER_UP_CMD;
  usb_uicc_global.usb_uicc_state_ptr             = USB_UICC_POWER_UP_STATES;
  usb_uicc_global.slot                           = card_power_up_ind_msg_ptr->slot;
  usb_uicc_global.usb_uicc_cmd.data_ptr          = NULL;
  usb_uicc_global.usb_uicc_cmd.data_len          = 0;
  usb_uicc_global.usb_uicc_cmd.rsp_timeout_valid = card_power_up_ind_msg_ptr->response_timeout_valid;
  usb_uicc_global.usb_uicc_cmd.volt_class_valid  = card_power_up_ind_msg_ptr->voltage_class_valid;


  if (usb_uicc_global.usb_uicc_cmd.rsp_timeout_valid == TRUE)
  {
    usb_uicc_global.usb_uicc_cmd.rsp_timeout = card_power_up_ind_msg_ptr->response_timeout;
    LOGI("rsp timeout = %u(ms)", usb_uicc_global.usb_uicc_cmd.rsp_timeout);
  }

  if (usb_uicc_global.usb_uicc_cmd.volt_class_valid == TRUE)
  {
    usb_uicc_global.usb_uicc_cmd.volt_class = card_power_up_ind_msg_ptr->voltage_class;
    LOGI("volt_class = %d", usb_uicc_global.usb_uicc_cmd.volt_class);
  }

  if ( (usb_uicc_global.usb_state == UIM_USB_STATE_CONNECTED) &&
       (usb_uicc_global.card_info.card_state == USB_UICC_CARD_PRESENT_ACTIVE) )
  {
    usb_uicc_global.usb_uicc_state_ptr += 6; /* move to USB_UICC_INIT_DONE_ST */
    usb_uicc_global.usb_uicc_cmd.rsp_payload_ptr = usb_uicc_global.card_info.atr;
    usb_uicc_global.usb_uicc_cmd.rsp_payload_len = usb_uicc_global.card_info.atr_len;
  }
  else if ( (usb_uicc_global.usb_state == UIM_USB_STATE_CONNECTED) &&
            (usb_uicc_global.card_info.card_state != USB_UICC_CARD_PRESENT_ACTIVE) )
  {
    usb_uicc_reset_global_data(FALSE);
    usb_uicc_global.usb_uicc_state_ptr += 2; /* move to USB_UICC_POWER_OFF_ST */
  }
  else
  {
    usb_uicc_reset_global_data(TRUE);
    usb_uicc_global.usb_uicc_state_ptr += 1; /* move to USB_UICC_ATTACH_ST */
  }
  USB_UICC_LEAVE_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");

  sem_post(&usb_uicc_sem);
  LOGI("post power up command into daemon.\n");
} /* usb_uicc_power_up_cmd */

/*=========================================================================

  FUNCTION:  usb_uicc_power_down

===========================================================================*/

/*!
    @brief
    To start power down procedure.

    @return
    None.
*/
/*=========================================================================*/
static void usb_uicc_power_down_cmd
(
  uim_remote_card_power_down_ind_msg_v01 *card_power_down_ind_msg_ptr
)
{
  assert(NULL != card_power_down_ind_msg_ptr);
  LOGI("received power down command.\n");

  USB_UICC_ENTER_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");
  usb_uicc_global.usb_uicc_cmd.command               = USB_UICC_POWER_DOWN_CMD;
  usb_uicc_global.usb_uicc_state_ptr                 = USB_UICC_POWER_DOWN_STATES;
  usb_uicc_global.slot                               = card_power_down_ind_msg_ptr->slot;
  usb_uicc_global.usb_uicc_cmd.data_ptr              = NULL;
  usb_uicc_global.usb_uicc_cmd.data_len              = 0;
  usb_uicc_global.usb_uicc_cmd.power_down_mode_valid = card_power_down_ind_msg_ptr->mode_valid;

  if (usb_uicc_global.usb_uicc_cmd.power_down_mode_valid == TRUE)
  {
    usb_uicc_global.usb_uicc_cmd.power_down_mode = card_power_down_ind_msg_ptr->mode;
  }

  LOGI("power down mode = %d, mode_valid = %d\n",
                   usb_uicc_global.usb_uicc_cmd.power_down_mode,
                   usb_uicc_global.usb_uicc_cmd.power_down_mode_valid);

  if (UIM_USB_STATE_CONNECTED != usb_uicc_global.usb_state)
  {
    usb_uicc_global.usb_uicc_state_ptr += 2;
    if (usb_uicc_global.usb_uicc_cmd.power_down_mode_valid == TRUE &&
        usb_uicc_global.usb_uicc_cmd.power_down_mode == UIM_REMOTE_POWER_DOWN_TELECOM_INTERFACE_V01)
    {
      usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN_TELECOM_V01;
    }
    else
    {
      usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN_V01;
    }
  }

  USB_UICC_LEAVE_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");

  sem_post(&usb_uicc_sem);
  LOGI("post power down command into daemon.\n");
} /* usb_uicc_power_down_cmd */

/*=========================================================================

  FUNCTION:  usb_uicc_send_apdu

===========================================================================*/

/*!
    @brief
    To start send apdu procedure.

    @return
    None.
*/
/*=========================================================================*/
static void usb_uicc_send_apdu_cmd
(
  uim_remote_apdu_ind_msg_v01 *apdu_ind_msg_ptr
)
{
  assert(NULL != apdu_ind_msg_ptr);
  LOGI("receive send apdu command with id = %d.\n", apdu_ind_msg_ptr->apdu_id);

  USB_UICC_ENTER_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");
  usb_uicc_global.usb_uicc_cmd.command  = USB_UICC_SEND_APDU_CMD;
  usb_uicc_global.usb_uicc_state_ptr    = USB_UICC_APDU_STATES;
  usb_uicc_global.slot                  = apdu_ind_msg_ptr->slot;
  usb_uicc_global.usb_uicc_cmd.apdu_id  = apdu_ind_msg_ptr->apdu_id;

  if ( (apdu_ind_msg_ptr->command_apdu_len <= 0) ||
       (apdu_ind_msg_ptr->command_apdu_len > QMI_UIM_REMOTE_MAX_COMMAND_APDU_LEN_V01) )
  {
    usb_uicc_global.usb_uicc_cmd.data_ptr = NULL;
    usb_uicc_global.usb_uicc_cmd.data_len = 0;
    LOGE("invalid apdu len = %d", apdu_ind_msg_ptr->command_apdu_len);
  }
  else if (apdu_ind_msg_ptr->command_apdu_len <= STATIC_BUFFER_MAX_SIZE)
  {
    usb_uicc_global.usb_uicc_cmd.data_ptr = apdu_static_buffer;
    usb_uicc_global.usb_uicc_cmd.data_len = apdu_ind_msg_ptr->command_apdu_len;
    memcpy(apdu_static_buffer, apdu_ind_msg_ptr->command_apdu, apdu_ind_msg_ptr->command_apdu_len);
  }
  else /* seems never here, QMI_UIM_REMOTE_MAX_COMMAND_APDU_LEN_V01 always less than STATIC_BUFFER_MAX_SIZE */
  {
    if (NULL != apdu_mem_buffer_ptr)
    {
      free(apdu_mem_buffer_ptr);
      apdu_mem_buffer_ptr = NULL;
    }

    apdu_mem_buffer_ptr = (uint8*)malloc(apdu_ind_msg_ptr->command_apdu_len);
    if (NULL == apdu_mem_buffer_ptr)
    {
      usb_uicc_global.usb_uicc_cmd.data_ptr = NULL;
      usb_uicc_global.usb_uicc_cmd.data_len = 0;
    }
    else
    {
      usb_uicc_global.usb_uicc_cmd.data_ptr = apdu_mem_buffer_ptr;
      usb_uicc_global.usb_uicc_cmd.data_len = apdu_ind_msg_ptr->command_apdu_len;
      memcpy(apdu_mem_buffer_ptr, apdu_ind_msg_ptr->command_apdu, apdu_ind_msg_ptr->command_apdu_len);
    }
  }
  LOGI("apdu from modem: ");
  usb_uicc_print_data(usb_uicc_global.usb_uicc_cmd.data_ptr, usb_uicc_global.usb_uicc_cmd.data_len);
  USB_UICC_LEAVE_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");

  sem_post(&usb_uicc_sem);
  LOGI("post send apdu command into daemon.\n");
} /* usb_uicc_send_apdu_cmd */

/*=========================================================================

  FUNCTION:  usb_uicc_reset_cmd

===========================================================================*/

/*!
    @brief
    To start reset card procedure.

    @return
    None.
*/
/*=========================================================================*/
static void usb_uicc_reset_cmd
(
  uim_remote_card_reset_ind_msg_v01* reset_ind_msg_ptr
)
{
  assert(NULL != reset_ind_msg_ptr);
  LOGI("receive reset command.\n");

  USB_UICC_ENTER_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");

  usb_uicc_global.usb_uicc_cmd.command  = USB_UICC_RESET_CMD;
  usb_uicc_global.usb_uicc_state_ptr    = USB_UICC_POWER_UP_STATES;
  usb_uicc_global.slot                  = reset_ind_msg_ptr->slot;
  usb_uicc_global.usb_uicc_cmd.data_ptr = NULL;
  usb_uicc_global.usb_uicc_cmd.data_len = 0;

  if (UIM_USB_STATE_CONNECTED == usb_uicc_global.usb_state)
  {
    usb_uicc_reset_global_data(FALSE);
    usb_uicc_global.usb_uicc_state_ptr += 2; /* move to POWER_OFF_ST */
  }
  else
  {
    usb_uicc_reset_global_data(TRUE);
    usb_uicc_global.usb_uicc_state_ptr += 1; /* move to USB_UICC_ATTACH_ST */
  }

  USB_UICC_LEAVE_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");
  sem_post(&usb_uicc_sem);
  LOGI("post reset command into daemon.\n");
} /* usb_uicc_reset_cmd */

/*===========================================================================
  FUNCTION  usb_uicc_client_async_cb
===========================================================================*/
/*!
@brief
  Callback function that will be called by the QCCI services layer to
  report asynchronous responses. It reports the response to parsing function.

@return
  None.

@note

  - Dependencies
  - Side Effects
*/
/*=========================================================================*/
#ifndef USB_UICC_QMI_UT
static void usb_uicc_client_async_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                         * qmi_response_ptr,
  unsigned int                   resp_c_struct_len,
  void                         * resp_cb_data_ptr,
  qmi_client_error_type          transp_err
)
{
  uim_remote_event_resp_msg_v01 * evt_rsp_ptr   = NULL;
  uim_remote_apdu_resp_msg_v01  * apdu_rsp_ptr  = NULL;
  uim_remote_reset_resp_msg_v01 * reset_rsp_ptr = NULL;

  assert(user_handle != NULL);
  assert(qmi_response_ptr != NULL);


  LOGI("usb_uicc_client_async_cb for msg_id : %d \n", msg_id);

  /* add process here */
  switch(msg_id)
  {
    case QMI_UIM_REMOTE_EVENT_RESP_V01:
      evt_rsp_ptr = (uim_remote_event_resp_msg_v01 *)qmi_response_ptr;
      if(evt_rsp_ptr->resp.result == QMI_RESULT_SUCCESS_V01)
      {
        LOGI("response is successful.\n");
      }
      else
      {
        LOGE("response error: 0x%x.\n",evt_rsp_ptr->resp.error);
      }
      break;
    case QMI_UIM_REMOTE_APDU_RESP_V01:
      apdu_rsp_ptr = (uim_remote_apdu_resp_msg_v01 *)qmi_response_ptr;
      if(apdu_rsp_ptr->resp.result == QMI_RESULT_SUCCESS_V01)
      {
        LOGI("response is successful.\n");
      }
      else
      {
        LOGE("response error: 0x%x.\n",apdu_rsp_ptr->resp.error);
      }
      break;
    case QMI_UIM_REMOTE_RESET_RESP_V01:
      reset_rsp_ptr = (uim_remote_reset_resp_msg_v01 *)qmi_response_ptr;
      if(reset_rsp_ptr->resp.result == QMI_RESULT_SUCCESS_V01)
      {
        LOGI("response is successful.\n");
      }
      else
      {
        LOGE("response error: 0x%x.\n",reset_rsp_ptr->resp.error);
      }
      break;
    default:
      LOGE("usb_uicc_client_async_cb: unsupported msg_id = %d.\n", msg_id);
  }

  /* Free response buffer that was allocated in the request */
  free(qmi_response_ptr);
} /* usb_uicc_client_async_cb */


/*===========================================================================
  FUNCTION  usb_uicc_client_indication_cb
===========================================================================*/
/*!
@brief
  Callback function that will be called by the QCCI services layer to
  report asynchronous indications.  This function currently monitors only
  card status & sap indication. It updates the global and decides to send
  the IOCTL to USB CCID Driver or not.

@return
  None.

@note

  - Dependencies
  - Side Effects
*/
/*=========================================================================*/
static void usb_uicc_client_indication_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                         * ind_buf_ptr,
  int                            ind_buf_len,
  void                         * ind_cb_data
)
{
  uint32_t                    decoded_payload_len  =  0;
  void                      * decoded_payload_ptr  =  NULL;
  qmi_client_error_type       qmi_err_code         =  QMI_NO_ERR;

  if(ind_buf_ptr == NULL)
  {
    LOGE("ind_buf_ptr is NULL ! \n");
    return;
  }

  if(usb_uicc_global.qmi_uim_svc_client_ptr == NULL)
  {
    LOGE("qmi_uim_svc_client_ptr is NULL ! \n");
    return;
  }

  LOGI("QMI UIM Indication, msg_id : %d \n", msg_id);

  /* Find the length & decode the Indication */
  (void)qmi_idl_get_message_c_struct_len(uim_remote_get_service_object_v01(),
                                   QMI_IDL_INDICATION,
                                   msg_id,
                                   &decoded_payload_len);
  if(decoded_payload_len)
  {
    decoded_payload_ptr = (void*)malloc(decoded_payload_len);
    if (decoded_payload_ptr == NULL)
    {
      LOGE("Failed to allocate buffer for QMI UIM Indication %d \n", msg_id);
      return;
    }
  }
  else
  {
    LOGE("Failed to find decoded_payload_len: 0x%x \n", decoded_payload_len);
    return;
  }

  qmi_err_code = qmi_client_message_decode( usb_uicc_global.qmi_uim_svc_client_ptr,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf_ptr,
                                            ind_buf_len,
                                            decoded_payload_ptr,
                                            decoded_payload_len);
  if (qmi_err_code != QMI_NO_ERR)
  {
    LOGE("Indication decode failed for msg: %d with error %d \n", msg_id, qmi_err_code);
    free(decoded_payload_ptr);
    return;
  }

  /* Determine the indication ID and process appropriately */
  switch(msg_id)
  {
    case QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01:
      /*power up*/
      usb_uicc_power_up_cmd((uim_remote_card_power_up_ind_msg_v01*) decoded_payload_ptr);
      break;
    case QMI_UIM_REMOTE_CARD_POWER_DOWN_IND_V01:
      /*power down*/
      usb_uicc_power_down_cmd((uim_remote_card_power_down_ind_msg_v01*) decoded_payload_ptr);
      break;
    case QMI_UIM_REMOTE_APDU_IND_V01:
      /*send apdu*/
      usb_uicc_send_apdu_cmd((uim_remote_apdu_ind_msg_v01*) decoded_payload_ptr);
      break;
    case QMI_UIM_REMOTE_CARD_RESET_IND_V01:
      /*reset*/
      usb_uicc_reset_cmd((uim_remote_card_reset_ind_msg_v01*) decoded_payload_ptr);
      break;
    case QMI_UIM_REMOTE_CONNECT_IND_V01:
      LOGI("qmi connected.\n");
      break;
    case QMI_UIM_REMOTE_DISCONNECT_IND_V01:
      LOGI("qmi disconnected.\n");
      break;
    default:
      LOGI("Unknown QMI UIM Indication, msg_id : %d \n", msg_id);
  }

  /* Free the allocated buffer */
  if (decoded_payload_ptr)
  {
    free(decoded_payload_ptr);
    decoded_payload_ptr = NULL;
  }

} /* usb_uicc_client_indication_cb */

/*=========================================================================

  FUNCTION:  usb_uicc_error_cb

===========================================================================*/
/*!
    @brief
    process error notified by qmi(SSR)

    @return
    None
*/
/*=========================================================================*/

static void usb_uicc_error_cb
(
  qmi_client_type       user_handle,
  qmi_client_error_type error,
  void                  *err_cb_data
)
{
  LOGE("service error %d received, pCallbackData = %p", error, err_cb_data);
  LOGE("need to exit the daemon!");
  exit(1);
} /* usb_uicc_error_cb */

/*=========================================================================

  FUNCTION:  usb_uicc_init_qmi

===========================================================================*/
/*!
    @brief
    Finds out the QMI port to be used based on the modem type & initializes
    QMI_UIM service.

    @return
    TRUE if successful, else FALSE
*/
/*=========================================================================*/
boolean usb_uicc_init_qmi
(
  void
)
{
  qmi_client_error_type          qmi_err_code           = 0;
  int                            rc                     = 0;
  int                            num_retries            = 0;
  qmi_cci_os_signal_type         os_params;
  qmi_service_info               info;
  qmi_idl_service_object_type    qmi_client_service_obj;

  /* Init QMI_UIM service for USB_UICC */
  qmi_client_service_obj = uim_remote_get_service_object_v01();

  LOGI("Received service object \n");

  qmi_err_code = qmi_client_notifier_init(
                   qmi_client_service_obj,
                   &os_params,
                   &usb_uicc_global.qmi_uim_svc_client_ptr);

  if (qmi_err_code != QMI_NO_ERR)
  {
    LOGE("Notifier Init return code %d \n", qmi_err_code);
    return FALSE;
  }


  /* retry 30 iterations with 1000ms qmi init time out */
  do
  {
    /* wait for server to come up */
    QMI_CCI_OS_SIGNAL_WAIT(&os_params, USB_UICC_QMI_SVC_TIMEOUT_MS);

    if(os_params.timed_out)
    {
      ++ num_retries;
      LOGE("Timeout! No signal received. Retry num = %d\n", num_retries);
    }
    else
    {
      break;
    }

    QMI_CCI_OS_SIGNAL_CLEAR(&os_params);
  } while( num_retries < USB_UICC_QMI_INIT_MAX_RETRIES );

  if (num_retries >= USB_UICC_QMI_INIT_MAX_RETRIES)
  {
    qmi_client_release(usb_uicc_global.qmi_uim_svc_client_ptr);
    LOGE("Timeout! No signal received. Reach maximum retries!");
    return FALSE;
  }

  LOGI("qmi init num retries = %d", num_retries);

  qmi_err_code = qmi_client_get_service_instance(
                  qmi_client_service_obj,
                  QMI_CLIENT_INSTANCE_ANY,
                  &info);

  if(qmi_err_code != QMI_NO_ERR)
  {
    LOGE("Failed to find service. Error Code %d",qmi_err_code );
    qmi_client_release(usb_uicc_global.qmi_uim_svc_client_ptr);
    return FALSE;
  }

  LOGI("QMI Client Init \n");

  /* Initialize connection to QMI subsystem control port */
  qmi_err_code = qmi_client_init(&info,
                                 qmi_client_service_obj,
                                 (qmi_client_ind_cb)usb_uicc_client_indication_cb,
                                 NULL,
                                 &os_params,
                                 &usb_uicc_global.qmi_uim_svc_client_ptr);


  LOGI("QMI Client Init RC %d \n",qmi_err_code);
  if ((usb_uicc_global.qmi_uim_svc_client_ptr == NULL) ||
      (qmi_err_code > 0))
  {
    LOGE("Could not register with QMI UIM Service, qmi_uim_svc_client_ptr: %p, qmi_err_code: %d\n",
         usb_uicc_global.qmi_uim_svc_client_ptr, qmi_err_code);
    qmi_client_release(usb_uicc_global.qmi_uim_svc_client_ptr);
    usb_uicc_global.qmi_msg_lib_handle     = -1;
    usb_uicc_global.qmi_uim_svc_client_ptr = NULL;
    return FALSE;
  }

  /* register error callback */
  rc  = qmi_client_register_error_cb(usb_uicc_global.qmi_uim_svc_client_ptr,
                                     usb_uicc_error_cb, NULL);
  if(QMI_NO_ERR != rc)
  {
    qmi_client_release(usb_uicc_global.qmi_uim_svc_client_ptr);
    usb_uicc_global.qmi_msg_lib_handle     = -1;
    usb_uicc_global.qmi_uim_svc_client_ptr = NULL;
    LOGE("could not register QCCI error callback error:%d", rc);
    return FALSE;
  }
  else
  {
    LOGD("register QCCI error callback success.");
  }

  /* send connection available evt to modem */
  usb_uicc_create_and_send_remote_evt_req(UIM_REMOTE_CONNECTION_AVAILABLE_V01);
  return TRUE;
} /* usb_uicc_init_qmi */
#endif /* USB_UICC_QMI_UT */

#ifdef USB_UICC_QMI_UT
/*=========================================================================

  FUNCTION:  usb_uicc_ut_thread

===========================================================================*/
/*!
    @brief
    Perform to UT thread

    @return
    None.
*/
/*=========================================================================*/
static void usb_uicc_ut_thread
(
  void * in_param
)
{
  /* Retrieve global UIM struct pointer used in this thread */
  int                                    command_index           = 0;
  uim_remote_card_power_up_ind_msg_v01   card_power_up_ind_msg   = {0};
  uim_remote_card_power_down_ind_msg_v01 card_power_down_ind_msg = {0};
  uim_remote_card_reset_ind_msg_v01      card_reset_ind_msg      = {0};
  uim_remote_apdu_ind_msg_v01            apdu_ind_msg            = {0};

  /* Infinite loop blocked on read */
  while (1)
  {
    sem_wait(&usb_uicc_ut_sem);
    switch(command_index)
    {
      case 0:
        card_power_up_ind_msg.slot = 0;
        usb_uicc_power_up_cmd((uim_remote_card_power_up_ind_msg_v01*) &card_power_up_ind_msg);
        command_index = 3;
        break;
      case 1:
        card_power_down_ind_msg.slot = 0;
        usb_uicc_power_down_cmd((uim_remote_card_power_down_ind_msg_v01*) &card_power_down_ind_msg);
        command_index = -1;
        break;
      case 2:
        card_reset_ind_msg.slot = 0;
        usb_uicc_reset_cmd((uim_remote_card_reset_ind_msg_v01*) &card_reset_ind_msg);
        command_index = 1;
        break;
      case 3:
        apdu_ind_msg.apdu_id ++;
        if (apdu_ind_msg.apdu_id == 1)
        {
          apdu_ind_msg.command_apdu_len = 7;
          apdu_ind_msg.command_apdu[0] = 0x00;
          apdu_ind_msg.command_apdu[1] = 0xa4;
          apdu_ind_msg.command_apdu[2] = 0x00;
          apdu_ind_msg.command_apdu[3] = 0x04;
          apdu_ind_msg.command_apdu[4] = 0x02;
          apdu_ind_msg.command_apdu[5] = 0x2f;
          apdu_ind_msg.command_apdu[6] = 0xe2;
        }
        else if (apdu_ind_msg.apdu_id == 2)
        {
          apdu_ind_msg.command_apdu_len = 5;
          apdu_ind_msg.command_apdu[0] = 0x00;
          apdu_ind_msg.command_apdu[1] = 0xc0;
          apdu_ind_msg.command_apdu[2] = 0x00;
          apdu_ind_msg.command_apdu[3] = 0x00;
          apdu_ind_msg.command_apdu[4] = 0x19;
        }
        else
        {
          apdu_ind_msg.command_apdu_len = 5;
          apdu_ind_msg.command_apdu[0] = 0x00;
          apdu_ind_msg.command_apdu[1] = 0xb0;
          apdu_ind_msg.command_apdu[2] = 0x00;
          apdu_ind_msg.command_apdu[3] = 0x00;
          apdu_ind_msg.command_apdu[4] = 0x0a;

        }
        apdu_ind_msg.slot = 0;
        usb_uicc_send_apdu_cmd((uim_remote_apdu_ind_msg_v01*) &apdu_ind_msg);
        if (apdu_ind_msg.apdu_id > 10)
        {
          command_index = -1;
        }
        break;
      default:
        pthread_exit(0);
        break;
    }
  }
  LOGE("Exiting usb_uicc_ut_thread ... \n");
} /* usb_uicc_ut_thread */

/*=========================================================================

  FUNCTION:  usb_uicc_create_ut_thread

===========================================================================*/
/*!
    @brief
    Perform to UT thread

    @return
    None.
*/
/*=========================================================================*/
void usb_uicc_create_ut_thread
(
  void
)
{
  if (pthread_create(&usb_uicc_global.ut_thread_id,
                     NULL,
                     (void*)usb_uicc_ut_thread,
                     NULL) != 0)
  {
    usb_uicc_global.ut_thread_id = -1;
    LOGE("usb uicc ut thread cannot be created.\n");
  }
  else
  {
    LOGI("usb uicc ut thread has started.\n");
  }
}
#endif /* USB_UICC_QMI_UT */
