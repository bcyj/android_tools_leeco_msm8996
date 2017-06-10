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
03/27/14   xj      Update error processing
04/18/14   xj      Get target infor and disable bridge by default when init
07/04/14   xj      Add response timeout interface to usb ccid driver
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "usb_uicc_daemon.h"

#ifdef USB_UICC_UT
#include "usb_uicc_ut.h"
#endif /* USB_UICC_UT */

/*===========================================================================

                           GLOBALS

===========================================================================*/
#ifndef USB_UICC_UT
static const char                usb_uicc_device_handle[]         = "/dev/ccid_bridge";
#else
static const char                usb_uicc_device_handle[]         = "/dev/ut_ccid_bridge";
sem_t                            usb_uicc_ut_sem;
#endif /* USB_UICC_UT */

static const char                usb_uicc_bulk_timeout_handle []  = "/sys/module/ccid_bridge/parameters/bulk_msg_timeout";

/* Global mutex */
pthread_mutex_t                  usb_uicc_mutex                   = PTHREAD_MUTEX_INITIALIZER;
/* Global semaphore */
sem_t                            usb_uicc_sem;
/* Global usb uicc data */
usb_uicc_global_data_type        usb_uicc_global;

/* usb uicc power up states */
usb_uicc_state_enum_type USB_UICC_POWER_UP_STATES[] =
{
  USB_UICC_NOT_INIT_ST,
  USB_UICC_ATTACH_ST,
  USB_UICC_POWER_OFF_ST,
  USB_UICC_GET_SLOT_STATUS_ST,
  USB_UICC_POWER_ON_ST,
  USB_UICC_POLL_ST, /* poll if needed */
  USB_UICC_INIT_DONE_ST
};

/* usb uicc power down states */
usb_uicc_state_enum_type USB_UICC_POWER_DOWN_STATES[] =
{
  USB_UICC_POWER_OFF_ST,
  USB_UICC_CLOSE_ST,
  USB_UICC_DONE_ST
};

/* usb uicc apdu states */
usb_uicc_state_enum_type USB_UICC_APDU_STATES[] =
{
  USB_UICC_APDU_ST,
  USB_UICC_POLL_ST, /* poll if needed */
  USB_UICC_DONE_ST
};

static uint8                   * write_mem_buffer_ptr             = NULL; /* heap buffer used to write req to card */
static uint8                     read_static_buffer [STATIC_BUFFER_MAX_SIZE]; /* static buffer used to read rsp from card */
static uint8                     write_static_buffer[STATIC_BUFFER_MAX_SIZE]; /* static buffer used to write req to card */

/* usb uicc state name */
static const char *usb_uicc_state_name[] =
{
  "USB_UICC_NOT_INIT_ST",
  "USB_UICC_ATTACH_ST",
  "USB_UICC_POWER_OFF_ST",
  "USB_UICC_GET_SLOT_STATUS_ST",
  "USB_UICC_POWER_ON_ST",
  "USB_UICC_INIT_DONE_ST",
  "USB_UICC_APDU_ST",
  "USB_UICC_POLL_ST",
  "USB_UICC_CLOSE_ST",
  "USB_UICC_DONE_ST",
  "USB_UICC_MAX_ST"
};

/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/
static int usb_uicc_send_write
(
  int                           fd,
  const uint8                 * buffer_ptr,
  uint16                        buffer_len
);
static int usb_uicc_send_read
(
  int                           fd,
  uint8                       * buffer_ptr,
  uint16                        buffer_len
);
static int usb_uicc_req_msg_to_card
(
  const usb_uicc_req_msg_type *usb_uicc_req_msg_ptr,
  const uint8                 *payload_ptr
);

static boolean usb_uicc_is_atr_valid
(
  const uint8* atr_ptr,
  uint32       atr_len
);
static void usb_uicc_parse_rsp_hdr_msg_from_card
(
  uint8                 *usb_uicc_rsp_buffer,
  int                    usb_uicc_rsp_buffer_len,
  usb_uicc_rsp_msg_type *usb_uicc_rsp_msg_ptr
);
static void usb_uicc_enumeration_timer_cb
(
  sigval_t signal
);
static void usb_uicc_poll_timer_cb
(
  sigval_t signal
);
static int usb_uicc_start_timer
(
  usb_uicc_timer_info_type *timer_ptr,
  long                      sec,
  long                      usec
);
static int usb_uicc_delete_timer
(
  usb_uicc_timer_info_type *timer_ptr
);
static int usb_uicc_send_icc_power_on_req
(
  void
);
static int usb_uicc_send_get_slot_status_req
(
  void
);
static int usb_uicc_send_icc_power_off_req
(
  void
);
static int usb_uicc_send_xfr_block_req
(
  const uint8 *payload_ptr,
  uint32       payload_length
);
static int usb_uicc_get_slot_status_rsp
(
  void
);
static int usb_uicc_get_data_block_rsp
(
  void
);
static void usb_uicc_exit_thread_handler
(
  int signal
);
static void usb_uicc_open_thread
(
  void * in_param
);
static void usb_uicc_listen_event_thread
(
  void * in_param
);
static usb_uicc_return_enum_type usb_uicc_process_send_apdu_states
(
  void
);
static usb_uicc_return_enum_type usb_uicc_process_power_down_states
(
  void
);
static usb_uicc_return_enum_type usb_uicc_process_power_up_states
(
  void
);
static void usb_uicc_global_init
(
  void
);
static void usb_uicc_cleanup
(
  int signal
);

/*=========================================================================

  FUNCTION:  usb_uicc_send_write

===========================================================================*/

/*!
    @brief
    Performs the write command on the specified device node.

    @return
    int.
*/
/*=========================================================================*/
static int usb_uicc_send_write
(
  int                           fd,
  const uint8                 * buffer_ptr,
  uint16                        buffer_len
)
{
#ifndef USB_UICC_UT
  int           bytes_written = 0;
  int           ret           = 0;
  const uint8 * tx_buffer_ptr = NULL;
  uint16        tx_buffer_len = 0;

  tx_buffer_ptr = buffer_ptr;
  tx_buffer_len = buffer_len;
  if (fd < 0)
  {
    LOGE("Invalid fd, not sending write \n");
    return -1;
  }

  if (usb_uicc_global.usb_state != UIM_USB_STATE_CONNECTED)
  {
    LOGE("usb not connected, not sending write \n");
    return -1;
  }

  if ((tx_buffer_ptr == NULL) || (tx_buffer_len == 0))
  {
    LOGE("Invalid input: tx_buffer_ptr = 0x%x, tx_buffer_len: 0x%x \n",
         tx_buffer_ptr, tx_buffer_len);
    return -1;
  }

  /* Send the data */
  while (tx_buffer_len > 0)
  {
    bytes_written = write(fd, tx_buffer_ptr, tx_buffer_len);
    /* Check if there was en error */
    if (bytes_written <= 0)
    {
      ret = errno;
      LOGE("Error writing on fd: 0x%x, errno: 0x%x\n", fd, errno);
      break;
    }
    /* Check if the entire data was sent */
    if (bytes_written >= tx_buffer_len)
    {
      LOGI("Successfully written (0x%x) bytes on fd: 0x%x \n", bytes_written, fd);
      break;
    }
    /* Shift the data and continue sending */
    tx_buffer_ptr += bytes_written;
    tx_buffer_len -= bytes_written;
  }

  return ret;
#else
  LOGI("write buffer: ");
  usb_uicc_print_data(buffer_ptr, buffer_len);
  return usb_uicc_ut_send_write(fd, buffer_ptr, buffer_len);
#endif /* USB_UICC_UT */
} /* usb_uicc_send_write */

/*=========================================================================

  FUNCTION:  usb_uicc_send_read

===========================================================================*/

/*!
    @brief
    Performs the read command on the specified device node.

    @return
    error type.
*/
/*=========================================================================*/
static int usb_uicc_send_read
(
  int                           fd,
  uint8                       * buffer_ptr,
  uint16                        buffer_len
)
{
#ifndef USB_UICC_UT
  int           bytes_read    = 0;
  int           ret           = 0;
  uint8       * rx_buffer_ptr = NULL;
  uint16        rx_buffer_len = 0;

  rx_buffer_ptr = buffer_ptr;
  rx_buffer_len = buffer_len;
  if (fd < 0)
  {
    LOGE("Invalid fd, not sending write \n");
    return -1;
  }

  if (usb_uicc_global.usb_state != UIM_USB_STATE_CONNECTED)
  {
    LOGE("usb not connected, not sending write \n");
    return -1;
  }

  if ((rx_buffer_ptr == NULL) || (rx_buffer_len == 0))
  {
    LOGE("Invalid input: rx_buffer_ptr = 0x%x, tx_buffer_len: 0x%x \n",
          rx_buffer_ptr, rx_buffer_len);
    return -1;
  }

  LOGI("Calling read() with bytes 0x%x \n", rx_buffer_len);

  bytes_read = read(fd, rx_buffer_ptr, rx_buffer_len);
  /* Check if there was en error */
  if (bytes_read <= 0)
  {
    ret = errno;
    LOGE("Error reading on fd: 0x%x, errno: 0x%x\n", fd, errno);
  }

  if (bytes_read > 0)
  {
    LOGI("Successfully read (0x%x) bytes on fd: 0x%x \n", bytes_read, fd);
  }

  return ret;
#else
  int ret = 0;
  ret = usb_uicc_ut_send_read(fd, buffer_ptr, buffer_len);
  LOGI(" read buffer: ");
  usb_uicc_print_data(buffer_ptr, ret);
  return 0;
#endif /* USB_UICC_UT */
} /* usb_uicc_send_read */

/*=========================================================================

  FUNCTION:  usb_uicc_req_msg_to_card

===========================================================================*/

/*!
    @brief
    Performs to send request message to the card.

    @return
    0 success.
    else, error code.
*/
/*=========================================================================*/
static int usb_uicc_req_msg_to_card
(
  const usb_uicc_req_msg_type *usb_uicc_req_msg_ptr,
  const uint8                 *payload_ptr
)
{
  uint32 payload_len = 0;
  int    ret         = 0;

  assert (NULL != usb_uicc_req_msg_ptr);

  payload_len = usb_uicc_req_msg_ptr->req_msg_hdr.length;
  if ((NULL == payload_ptr) && (0 != payload_len))
  {
    LOGE("error when send msg to card!\n");
    return -1;
  }

  if (payload_len + USB_UICC_REQ_MSG_LEN <= STATIC_BUFFER_MAX_SIZE)
  {
    /* request data less than STATIC_BUFFER_MAX_SIZE, use write_static_buffer */
    write_static_buffer[0] = (uint8) usb_uicc_req_msg_ptr->req_msg_hdr.req_msg_type;
    write_static_buffer[1] = (uint8) usb_uicc_req_msg_ptr->req_msg_hdr.length;
    write_static_buffer[2] = (uint8)(usb_uicc_req_msg_ptr->req_msg_hdr.length >> 8 );
    write_static_buffer[3] = (uint8)(usb_uicc_req_msg_ptr->req_msg_hdr.length >> 16);
    write_static_buffer[4] = (uint8)(usb_uicc_req_msg_ptr->req_msg_hdr.length >> 24);
    write_static_buffer[5] =         usb_uicc_req_msg_ptr->req_msg_hdr.slot;
    write_static_buffer[6] =         usb_uicc_req_msg_ptr->req_msg_hdr.seq;
    write_static_buffer[7] =         usb_uicc_req_msg_ptr->rfu_req_data.rfu[0];
    write_static_buffer[8] =         usb_uicc_req_msg_ptr->rfu_req_data.rfu[1];
    write_static_buffer[9] =         usb_uicc_req_msg_ptr->rfu_req_data.rfu[2];

    if ( (NULL != payload_ptr) && (payload_len > 0) && (payload_len <= (STATIC_BUFFER_MAX_SIZE - USB_UICC_REQ_MSG_LEN)) )
    {
      memcpy (write_static_buffer + USB_UICC_REQ_MSG_LEN, payload_ptr, payload_len);
    }
    LOGI("write to usb uicc:");
    usb_uicc_print_data(write_static_buffer, payload_len + USB_UICC_REQ_MSG_LEN);
    ret = usb_uicc_send_write (usb_uicc_global.usb_uicc_fd, write_static_buffer, payload_len + USB_UICC_REQ_MSG_LEN);
  }
  else /* dynamic memory allocation */
  {
    if (NULL != write_mem_buffer_ptr)
    {
      free(write_mem_buffer_ptr);
      write_mem_buffer_ptr = NULL;
    }
    write_mem_buffer_ptr = (uint8*) malloc(payload_len + USB_UICC_REQ_MSG_LEN);
    if (NULL == write_mem_buffer_ptr)
    {
      LOGE("allocate memory error when send req to card!\n");
      return -1;
    }

    write_mem_buffer_ptr[0] = (uint8) usb_uicc_req_msg_ptr->req_msg_hdr.req_msg_type;
    write_mem_buffer_ptr[1] = (uint8) usb_uicc_req_msg_ptr->req_msg_hdr.length;
    write_mem_buffer_ptr[2] = (uint8)(usb_uicc_req_msg_ptr->req_msg_hdr.length >> 8 );
    write_mem_buffer_ptr[3] = (uint8)(usb_uicc_req_msg_ptr->req_msg_hdr.length >> 16);
    write_mem_buffer_ptr[4] = (uint8)(usb_uicc_req_msg_ptr->req_msg_hdr.length >> 24);
    write_mem_buffer_ptr[5] =         usb_uicc_req_msg_ptr->req_msg_hdr.slot;
    write_mem_buffer_ptr[6] =         usb_uicc_req_msg_ptr->req_msg_hdr.seq;
    write_mem_buffer_ptr[7] =         usb_uicc_req_msg_ptr->rfu_req_data.rfu[0];
    write_mem_buffer_ptr[8] =         usb_uicc_req_msg_ptr->rfu_req_data.rfu[1];
    write_mem_buffer_ptr[9] =         usb_uicc_req_msg_ptr->rfu_req_data.rfu[2];

    if ((NULL != payload_ptr) && (0 != payload_len))
    {
      memcpy (write_mem_buffer_ptr + USB_UICC_REQ_MSG_LEN, payload_ptr, payload_len);
    }
    LOGI("write to usb uicc:");
    usb_uicc_print_data(write_mem_buffer_ptr, payload_len + USB_UICC_REQ_MSG_LEN);
    ret = usb_uicc_send_write (usb_uicc_global.usb_uicc_fd, write_mem_buffer_ptr, payload_len + USB_UICC_REQ_MSG_LEN);
  }
  return ret;
} /* usb_uicc_req_msg_to_card */

/*=========================================================================

  FUNCTION:  usb_uicc_is_atr_valid

===========================================================================*/

/*!
    @brief
    Performs to check atr.

    @return
    boolean.
*/
/*=========================================================================*/
static boolean usb_uicc_is_atr_valid
(
  const uint8* atr_ptr,
  uint32       atr_len
)
{
  boolean is_valid   = FALSE;
  uint32  i          = 0;
  uint8   check_data = 0x00;

  if (NULL == atr_ptr || atr_len < 2 || atr_len > 32)
  {
    memset (usb_uicc_global.card_info.atr, 0x00, QMI_UIM_REMOTE_MAX_ATR_LEN_V01);
    usb_uicc_global.card_info.atr_len = 0;
    return FALSE;
  }

  /* XOR -- T0 to TCK, skip Ts */
  for (i = 1; i < atr_len; ++i)
  {
    check_data ^= atr_ptr[i];
  }

  if (0x00 == check_data)
  {
    memcpy (usb_uicc_global.card_info.atr, atr_ptr, atr_len);
    usb_uicc_global.card_info.atr_len = atr_len;
    return TRUE;
  }
  else
  {
    memset (usb_uicc_global.card_info.atr, 0x00, QMI_UIM_REMOTE_MAX_ATR_LEN_V01);
    usb_uicc_global.card_info.atr_len = 0;
    return FALSE;
  }
} /* usb_uicc_is_atr_valid */

/*=========================================================================

  FUNCTION:  usb_uicc_rsp_hdr_msg_from_card

===========================================================================*/

/*!
    @brief
    Performs to get response message to the card.

    @return
    None
*/
/*=========================================================================*/
static void usb_uicc_parse_rsp_hdr_msg_from_card
(
  uint8                 *usb_uicc_rsp_buffer,
  int                    usb_uicc_rsp_buffer_len,
  usb_uicc_rsp_msg_type *usb_uicc_rsp_msg_ptr
)
{
  assert (NULL != usb_uicc_rsp_msg_ptr);
  assert (NULL != usb_uicc_rsp_buffer);
  assert (usb_uicc_rsp_buffer_len != USB_UICC_RSP_MSG_LEN);

  usb_uicc_rsp_msg_ptr->rsp_msg_hdr.rsp_msg_type =         usb_uicc_rsp_buffer[0];
  usb_uicc_rsp_msg_ptr->rsp_msg_hdr.length       = (uint32)usb_uicc_rsp_buffer[1];
  usb_uicc_rsp_msg_ptr->rsp_msg_hdr.length      += (uint32)usb_uicc_rsp_buffer[2] << 8;
  usb_uicc_rsp_msg_ptr->rsp_msg_hdr.length      += (uint32)usb_uicc_rsp_buffer[3] << 16;
  usb_uicc_rsp_msg_ptr->rsp_msg_hdr.length      += (uint32)usb_uicc_rsp_buffer[4] << 24;
  usb_uicc_rsp_msg_ptr->rsp_msg_hdr.slot         =         usb_uicc_rsp_buffer[5];
  usb_uicc_rsp_msg_ptr->rsp_msg_hdr.seq          =         usb_uicc_rsp_buffer[6];
  usb_uicc_rsp_msg_ptr->rsp_msg_hdr.status       =         usb_uicc_rsp_buffer[7];
  usb_uicc_rsp_msg_ptr->rsp_msg_hdr.err_code     =         usb_uicc_rsp_buffer[8];
  usb_uicc_rsp_msg_ptr->rfu                      =         usb_uicc_rsp_buffer[9];
} /* usb_uicc_rsp_msg_from_card */

/*=========================================================================

  FUNCTION:  usb_uicc_enumeration_timer_cb

===========================================================================*/
/*!
    @brief

    @return
    None.
*/
/*=========================================================================*/
static void usb_uicc_enumeration_timer_cb
(
  sigval_t   signal
)
{
  LOGI("usb enumeration timer out!\n");
  usb_uicc_reset_global_data(TRUE);
  usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
  usb_uicc_create_and_send_remote_evt_req(UIM_REMOTE_CARD_ERROR_V01);
} /* usb_uicc_enumeration_timer_cb */

/*=========================================================================

  FUNCTION:  usb_uicc_poll_timer_cb

===========================================================================*/
/*!
    @brief

    @return
    None.
*/
/*=========================================================================*/
static void usb_uicc_poll_timer_cb
(
  sigval_t   signal
)
{
  LOGI("poll timer out!\n");
  if (0 != usb_uicc_global.poll_timer.timer_id)
  {
    timer_delete(usb_uicc_global.poll_timer.timer_id);
  }
  usb_uicc_global.poll_timer.timer_id      = 0;
  usb_uicc_global.poll_timer.timer_started = FALSE;
  sem_post(&usb_uicc_sem);
} /* usb_uicc_poll_timer_cb */

/*=========================================================================

  FUNCTION:  usb_uicc_start_timer

===========================================================================*/
/*!
    @brief

    @return
    0      success
    others failed
*/
/*=========================================================================*/
static int usb_uicc_start_timer
(
  usb_uicc_timer_info_type *timer_ptr,
  long                      sec,
  long                      usec
)
{
  struct sigevent sigev;
  struct itimerspec itimers;

  assert (NULL != timer_ptr);

  if (timer_ptr->timer_started)
  {
    LOGE("timer has started.\n");
    return 0;
  }

  sigev.sigev_notify            = SIGEV_THREAD;
  sigev.sigev_notify_attributes = NULL;
  sigev.sigev_value.sival_ptr   = timer_ptr;
  sigev.sigev_notify_function   = timer_ptr->timer_cb;

  if (timer_create(CLOCK_REALTIME,&sigev, &timer_ptr->timer_id) == -1)
  {
    LOGE("timer create error.\n");
    return -1;
  }

  itimers.it_value.tv_sec      = sec;  /* sec */
  itimers.it_value.tv_nsec     = usec; /* usec */
  itimers.it_interval.tv_sec   = 0;    /* we want timer to expire only once. */
  itimers.it_interval.tv_nsec  = 0;

  if (timer_settime(timer_ptr->timer_id, 0, &itimers, NULL) == -1)
  {
    LOGE("timer set error.\n");
    timer_delete(timer_ptr->timer_id);
    timer_ptr->timer_id        = 0;
    timer_ptr->timer_started   = FALSE;
    return -1;
  }

  LOGI("timer started, timer id = 0x%x, %ld sec, %ld usec.\n", timer_ptr->timer_id, sec, usec);
  timer_ptr->timer_started     = TRUE;
  return 0;
} /* usb_uicc_start_timer */

/*=========================================================================

  FUNCTION:  usb_uicc_delete_timer

===========================================================================*/
/*!
    @brief

    @return
    0      success
    others failed
*/
/*=========================================================================*/
static int usb_uicc_delete_timer
(
  usb_uicc_timer_info_type *timer_ptr
)
{
  assert (NULL != timer_ptr);
  if (timer_ptr->timer_id != 0)
  {
    timer_delete(timer_ptr->timer_id);
  }
  timer_ptr->timer_id      = 0;
  timer_ptr->timer_started = FALSE;
  return 0;
} /* usb_uicc_delete_timer */

/*=========================================================================

  FUNCTION:  usb_uicc_send_icc_power_on_req

===========================================================================*/

/*!
    @brief
    To send power on request to usb uicc card.

    @return
    error type
*/
/*=========================================================================*/
static int usb_uicc_send_icc_power_on_req
(
  void
)
{
  int                    ret = 0;
  usb_uicc_req_msg_type  icc_power_on_req;

  memset((uint8*)&icc_power_on_req, 0x00, sizeof(usb_uicc_req_msg_type));
  /* populate icc_power_on_req buffer */
  icc_power_on_req.req_msg_hdr.req_msg_type = USB_UICC_ICC_POWER_ON_REQ;
  icc_power_on_req.req_msg_hdr.seq          = (++ usb_uicc_global.usb_uicc_msg_seq);
  icc_power_on_req.req_msg_hdr.slot         = 0; /* this slot field if from class description */
  icc_power_on_req.req_msg_hdr.length       = 0x00000000;

  ret = usb_uicc_req_msg_to_card(&icc_power_on_req, NULL);
  return ret;
} /* usb_uicc_send_icc_power_on_req */

/*=========================================================================

  FUNCTION:  usb_uicc_send_icc_power_on_req

===========================================================================*/

/*!
    @brief
    To send get slot status request to usb uicc card.

    @return
    error type
*/
/*=========================================================================*/
static int usb_uicc_send_get_slot_status_req
(
  void
)
{
  int                    ret = 0;
  usb_uicc_req_msg_type  get_slot_status_req;

  memset((uint8*)&get_slot_status_req, 0x00, sizeof(usb_uicc_req_msg_type));
  /* populate get_slot_status_req buffer */
  get_slot_status_req.req_msg_hdr.req_msg_type = USB_UICC_GET_SLOT_STATUS_REQ;
  get_slot_status_req.req_msg_hdr.seq          = (++ usb_uicc_global.usb_uicc_msg_seq);
  get_slot_status_req.req_msg_hdr.slot         = 0; /* this slot field if from class description */
  get_slot_status_req.req_msg_hdr.length       = 0x00000000;

  ret = usb_uicc_req_msg_to_card(&get_slot_status_req, NULL);
  return ret;
} /* usb_uicc_send_get_slot_status_req */

/*=========================================================================

  FUNCTION:  usb_uicc_send_icc_power_off_req

===========================================================================*/

/*!
    @brief
    To send power off request to usb uicc card.

    @return
    error type
*/
/*=========================================================================*/
static int usb_uicc_send_icc_power_off_req
(
  void
)
{
  int                    ret = 0;
  usb_uicc_req_msg_type  icc_power_off_req;

  memset((uint8*)&icc_power_off_req, 0x00, sizeof(usb_uicc_req_msg_type));
  /* populate icc_power_off_req buffer */
  icc_power_off_req.req_msg_hdr.req_msg_type = USB_UICC_ICC_POWER_OFF_REQ;
  icc_power_off_req.req_msg_hdr.seq          = (++ usb_uicc_global.usb_uicc_msg_seq);
  icc_power_off_req.req_msg_hdr.slot         = 0; /* this slot field if from class description */
  icc_power_off_req.req_msg_hdr.length       = 0x00000000;

  ret = usb_uicc_req_msg_to_card(&icc_power_off_req, NULL);
  return ret;
} /* usb_uicc_send_icc_power_off_req */

/*=========================================================================

  FUNCTION:  usb_uicc_send_xfr_block_req

===========================================================================*/

/*!
    @brief
    To send xfr block request to usb uicc card.

    @return
    error type
*/
/*=========================================================================*/
static int usb_uicc_send_xfr_block_req
(
  const uint8 *payload_ptr,
  uint32       payload_length
)
{
  int                    ret = 0;
  usb_uicc_req_msg_type  xfr_block_req;

  if (NULL == payload_ptr || 0 == payload_length)
  {
    return -1;
  }

  memset((uint8*)&xfr_block_req, 0x00, sizeof(usb_uicc_req_msg_type));
  /* populate xfr_block_req buffer */
  xfr_block_req.req_msg_hdr.req_msg_type           = USB_UICC_XFR_BLOCK_REQ;
  xfr_block_req.req_msg_hdr.seq                    = (++ usb_uicc_global.usb_uicc_msg_seq);
  xfr_block_req.req_msg_hdr.slot                   = 0; /* this slot field if from class description */
  xfr_block_req.req_msg_hdr.length                 = payload_length;
  xfr_block_req.xfr_block_req_data.bwi             = 0x00;   /* extend to CCIDs Block Waiting timeout */
  xfr_block_req.xfr_block_req_data.level_parameter = 0x0000; /* need to be determined */

  ret = usb_uicc_req_msg_to_card(&xfr_block_req, payload_ptr);
  return ret;
} /* usb_uicc_send_xfr_block_req */

/*=========================================================================

  FUNCTION:  usb_uicc_get_slot_status_rsp

===========================================================================*/
/*!
    @brief

    @return
     error type
*/
/*=========================================================================*/
static int usb_uicc_get_slot_status_rsp
(
  void
)
{
  usb_uicc_rsp_msg_type  slot_status_rsp;
  int                    ret                 = 0;
  uint8                  icc_status          = 0x00;

  memset(read_static_buffer, 0x00, STATIC_BUFFER_MAX_SIZE);
  memset(&slot_status_rsp, 0x00, sizeof(usb_uicc_rsp_msg_type));

  LOGI("Reading slot status response \n");

  /* Read the response from USB Driver with the max response size */
  ret = usb_uicc_send_read(usb_uicc_global.usb_uicc_fd,
                           read_static_buffer,
                           STATIC_BUFFER_MAX_SIZE);

  if (0 != ret)
  {
    LOGE("read error when get slot status rsp.\n");
    return ret;
  }

  /* parse RDR_to_PC_SlotStatus rsp header */
  usb_uicc_parse_rsp_hdr_msg_from_card(read_static_buffer,
                                       USB_UICC_RSP_MSG_LEN,
                                       &slot_status_rsp);

  if (usb_uicc_global.usb_uicc_msg_seq != slot_status_rsp.rsp_msg_hdr.seq)
  {
    LOGE("seq in BULK OUT message 0x%x is not the same with BULK IN message 0x%x.\n",
         usb_uicc_global.usb_uicc_msg_seq, slot_status_rsp.rsp_msg_hdr.seq );
    return -1;
  }

  /* check status field of rsp*/
  icc_status =  slot_status_rsp.rsp_msg_hdr.status & USB_UICC_ICC_STATUS_MASK;
  usb_uicc_global.card_info.card_cmd_status = (slot_status_rsp.rsp_msg_hdr.status & USB_UICC_COMMAND_STATUS_MASK) >> 6;
  usb_uicc_global.card_info.card_error_code = slot_status_rsp.rsp_msg_hdr.err_code;
  switch (icc_status)
  {
    case 0x00:
      usb_uicc_global.card_info.card_state = USB_UICC_CARD_PRESENT_ACTIVE;
      break;
    case 0x01:
      usb_uicc_global.card_info.card_state = USB_UICC_CARD_PRESENT_INAVTIVE;
      break;
    case 0x02:
      usb_uicc_global.card_info.card_state = USB_UICC_CARD_ERROR;
      break;
    default:  /* RFU */
      usb_uicc_global.card_info.card_state = USB_UICC_CARD_ERROR;
  }

  return ret;
} /* usb_uicc_get_slot_status_rsp */

/*=========================================================================

  FUNCTION:  usb_uicc_get_data_block_rsp

===========================================================================*/
/*!
    @brief
     Performs to get data block response

    @return
     error type
*/
/*=========================================================================*/
static int usb_uicc_get_data_block_rsp
(
  void
)
{
  usb_uicc_rsp_msg_type  data_block_rsp;
  int                    ret                = 0;
  uint8                  icc_status         = 0x00;

  memset(read_static_buffer, 0x00, STATIC_BUFFER_MAX_SIZE);

  /* Read the response from USB Driver with the max response size */
  ret = usb_uicc_send_read(usb_uicc_global.usb_uicc_fd,
                           read_static_buffer,
                           STATIC_BUFFER_MAX_SIZE);

  if (0 != ret)
  {
    LOGE("read error when get data block rsp.\n");
    return ret;;
  }

  /* Parse RDR_to_PC_DataBlock header */
  usb_uicc_parse_rsp_hdr_msg_from_card(read_static_buffer,
                                       USB_UICC_RSP_MSG_LEN,
                                       &data_block_rsp);
  LOGI("read header from usb uicc:");
  usb_uicc_print_data(read_static_buffer, USB_UICC_RSP_MSG_LEN);
  if (usb_uicc_global.usb_uicc_msg_seq != data_block_rsp.rsp_msg_hdr.seq)
  {
    LOGE("seq in BULK OUT message 0x%x is not the same with BULK IN message 0x%x.\n",
         usb_uicc_global.usb_uicc_msg_seq, data_block_rsp.rsp_msg_hdr.seq );
    return -1;
  }

  /* check status field of rsp*/
  icc_status =  data_block_rsp.rsp_msg_hdr.status & USB_UICC_ICC_STATUS_MASK;
  usb_uicc_global.card_info.card_cmd_status = (data_block_rsp.rsp_msg_hdr.status & USB_UICC_COMMAND_STATUS_MASK) >> 6;

  switch(icc_status)
  {
    case 0x00:
      usb_uicc_global.card_info.card_state = USB_UICC_CARD_PRESENT_ACTIVE;
      break;
    case 0x01:
      usb_uicc_global.card_info.card_state = USB_UICC_CARD_PRESENT_INAVTIVE;
      break;
    case 0x02:
      usb_uicc_global.card_info.card_state = USB_UICC_CARD_ERROR;
      break;
    default:  /* RFU */
      usb_uicc_global.card_info.card_state = USB_UICC_CARD_ERROR;
  }

  usb_uicc_global.card_info.card_error_code = data_block_rsp.rsp_msg_hdr.err_code;

  /* get RDR_to_PC_DataBlock payload */
  if (usb_uicc_global.card_info.card_cmd_status == USB_UICC_CMD_SUCCESS &&
      data_block_rsp.rsp_msg_hdr.length > 0)
  {
    usb_uicc_global.usb_uicc_cmd.rsp_payload_ptr = read_static_buffer + USB_UICC_RSP_MSG_LEN;
    usb_uicc_global.usb_uicc_cmd.rsp_payload_len = data_block_rsp.rsp_msg_hdr.length;
  }
  else
  {
    usb_uicc_global.usb_uicc_cmd.rsp_payload_ptr = NULL;
    usb_uicc_global.usb_uicc_cmd.rsp_payload_len = 0;
  }
  if (usb_uicc_global.usb_uicc_cmd.rsp_payload_len > 0)
  {
    LOGI("read payload from usb uicc:");
    usb_uicc_print_data(read_static_buffer + USB_UICC_RSP_MSG_LEN,
                        usb_uicc_global.usb_uicc_cmd.rsp_payload_len);
  }
  return ret;
} /* usb_uicc_get_data_block_rsp */

/*=========================================================================

  FUNCTION:  usb_uicc_exit_thread_handler

===========================================================================*/
/*!
    @brief
     handler of exit thread

    @return
     None.
*/
/*=========================================================================*/
static void usb_uicc_exit_thread_handler
(
  int signal
)
{
  LOGI("exit thread due to signal %d\n", signal);
  pthread_exit(0);
} /* usb_uicc_exit_thread_handler */

/*=========================================================================

  FUNCTION:  usb_uicc_open_thread

===========================================================================*/
/*!
    @brief
     Performs to open ccid bridge

    @return
     None.
*/
/*=========================================================================*/
static void usb_uicc_open_thread
(
  void * in_param
)
{
  int  ret = 0;
  int  fd  = 1;
  char timeout_buffer[16] = "15000";
  struct sigaction actions;
  memset(&actions, 0x00, sizeof(actions));
  actions.sa_flags = 0;
  actions.sa_handler = usb_uicc_exit_thread_handler;
  sigaction(SIGUSR1, &actions, NULL);

  LOGI("open thread start, id = 0x%x\n", (unsigned int)(usb_uicc_global.open_thread_id));

  /*1. start enumeration_timer, time period = 3s */
  ret = usb_uicc_start_timer(&usb_uicc_global.enumeration_timer, 3, 0);

  /*2. write ccid bulk timeout value if necessary */
  fd = open(usb_uicc_bulk_timeout_handle, O_RDWR);
  if (fd != -1)
  {
    if (usb_uicc_global.usb_uicc_cmd.rsp_timeout_valid == TRUE)
    {
      snprintf( timeout_buffer, \
                sizeof(timeout_buffer), \
                "%d", \
                (int)usb_uicc_global.usb_uicc_cmd.rsp_timeout );
    }
    LOGI("bulk timeout = %s(ms)", timeout_buffer);
    if (write(fd, timeout_buffer, strlen(timeout_buffer) + 1) < 0)
    {
      LOGE("write bulk_time_handle error(errno = %d)", errno);
    }
    /* ccid driver does not use default timeout value, enumeration occurs only when write timeout successfully */
    else
    {
      /*3. open iccid device */
      usb_uicc_global.usb_uicc_fd = usb_uicc_open_ccid_device(usb_uicc_device_handle, O_RDWR);
    }
    close(fd);
  }
  else
  {
    LOGE("open bulk_timeout_handle error(errno = %d)", errno);
  }

  /*4. delete enumeration_timer */
  ret = usb_uicc_delete_timer(&usb_uicc_global.enumeration_timer);

  if (-1 != usb_uicc_global.usb_uicc_fd)
  {
    usb_uicc_global.usb_state = UIM_USB_STATE_CONNECTED;
  }
  else
  {
    usb_uicc_global.usb_state = UIM_USB_STATE_ERROR;
    LOGE("open usb ccid error = %s", strerror(errno));
  }

  usb_uicc_global.open_thread_id = -1;
  LOGI("Exiting usb_uicc_open_thread ... \n");
  sem_post(&usb_uicc_sem);
  pthread_exit(NULL);
} /* usb_uicc_open_thread */

/*=========================================================================

  FUNCTION:  usb_uicc_listen_event_thread

===========================================================================*/
/*!
    @brief
     Performs to listen MSG EVENT from the card

    @return
     None.
*/
/*=========================================================================*/
static void usb_uicc_listen_event_thread
(
  void * in_param
)
{
  /* Retrieve global UIM struct pointer used in this thread */
  uint8                    rx_buffer[4];
  int                      fd = -1;
  struct sigaction         actions;

  memset(&actions, 0x00, sizeof(actions));
  actions.sa_flags   = 0;
  actions.sa_handler = usb_uicc_exit_thread_handler;
  sigaction(SIGUSR1, &actions, NULL);
  memset(&rx_buffer, 0, 4);
  fd = *( (int*) (in_param) );

  /* Infinite loop blocked on read */
  LOGI("listen event thread start: id = 0x%x.\n", (unsigned int)(usb_uicc_global.listen_evt_thread_id));
  while (1)
  {
#ifndef USB_UICC_UT
    if (ioctl(fd, USB_CCID_GET_EVENT, rx_buffer) == -1)
    {
       LOGE("Could not send IOCTL, error = 0x%x ! \n", errno);
       break;
    }
#else
    if (usb_uicc_ut_send_ioctl(fd, USB_CCID_GET_EVENT, rx_buffer) == -1)
    {
       LOGE("Could not send IOCTL ! \n");
       break;
    }
#endif /* USB_UICC_UT */
    switch(rx_buffer[0])
    {
      case 0x50:/*RDR_to_PC_NotifySlotChange, used when resumed from suspend */
        /* process here */
        LOGI("notify slot change\n");
        break;

      case 0x51:/*RDR_to_PC_HardwareError*/
        /* process here */
        LOGI("hardware error, 0x%x, 0x%x, 0x%x\n", rx_buffer[1], rx_buffer[2], rx_buffer[3]);
        usb_uicc_global.is_hardware_error = TRUE;
        sem_post(&usb_uicc_sem); /* notify main thread to terminnate current process */
        break;
      default:
        LOGI("other events: 0x%x, 0x%x, 0x%x, 0x%x\n", rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3]);
    }
  }
  LOGE("Exiting usb_uicc_listen_event_thread ... \n");
} /* usb_uicc_listen_event_thread */

/*=========================================================================

  FUNCTION:  usb_uicc_process_send_apdu_states

===========================================================================*/

/*!
    @brief
    Performs to process send apdu state machine.

    @return
    usb_uicc_return_enum_type.
*/
/*=========================================================================*/
static usb_uicc_return_enum_type usb_uicc_process_send_apdu_states
(
  void
)
{
  int ret = -1;

  assert(NULL != usb_uicc_global.usb_uicc_state_ptr);
  LOGI ("curent state = %s.\n", usb_uicc_state_name[*usb_uicc_global.usb_uicc_state_ptr]);
  switch(*usb_uicc_global.usb_uicc_state_ptr)
  {
    case USB_UICC_APDU_ST:
      /* check card status */
      if (USB_UICC_CARD_PRESENT_ACTIVE != usb_uicc_global.card_info.card_state)
      {
        LOGE("card is not in active state when sending apdu.\n");
        return USB_UICC_ERR_FAIL;
      }

      /*1. write PC_to_RDR_XfrBlock_Request to usb icc card */
      ret = usb_uicc_send_xfr_block_req(usb_uicc_global.usb_uicc_cmd.data_ptr, usb_uicc_global.usb_uicc_cmd.data_len);
      /*2. read response from usb uicc card */
      if (ret == 0)
      {
        ret = usb_uicc_get_data_block_rsp();
      }

      if (ret != 0)
      {
        if (ret == ETIMEDOUT)
        {
          usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_COMMAND_TIMEOUT_V01;
        }
        else
        {
          usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01;
        }
        usb_uicc_global.card_info.card_state      = USB_UICC_CARD_ERROR;
        LOGE("Error reading in apdu st, err = %d.\n", ret);
        return USB_UICC_ERR_FAIL;
      }

      ++ usb_uicc_global.usb_uicc_state_ptr;
      /* do not need polling, skip poll state */
      if (usb_uicc_global.card_info.card_cmd_status == USB_UICC_CMD_SUCCESS)
      {
        ++ usb_uicc_global.usb_uicc_state_ptr;
      }
      /* need polling procedure */
      else if (usb_uicc_global.card_info.card_cmd_status == USB_UICC_CMD_TIME_EXTENSION)
      {
        usb_uicc_global.usb_uicc_cmd.poll_counter = 0;
        ret = usb_uicc_start_timer(&usb_uicc_global.poll_timer, 0, 10000);
        return USB_UICC_ERR_WAIT;
      }
      else
      {
        usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01;
        usb_uicc_global.card_info.card_state      = USB_UICC_CARD_ERROR;
        LOGE("command to usb uicc card error, cmd_status = 0x%x, cmd_err = 0x%x\n",
              usb_uicc_global.card_info.card_cmd_status,
              usb_uicc_global.card_info.card_error_code);
        return USB_UICC_ERR_FAIL;
      }

      break;
    case USB_UICC_POLL_ST:
      if (usb_uicc_global.usb_uicc_cmd.poll_counter >= USB_UICC_MAX_POLL)
      {
        read_static_buffer[USB_UICC_RSP_MSG_LEN]     = 0x6f; /* sw1 */
        read_static_buffer[USB_UICC_RSP_MSG_LEN+1]   = 0x00; /* sw2 */
        usb_uicc_global.usb_uicc_cmd.rsp_payload_len = 2;
        usb_uicc_global.usb_uicc_cmd.rsp_payload_ptr = read_static_buffer + USB_UICC_RSP_MSG_LEN;
        LOGE("poll reach max.\n");
        return USB_UICC_ERR_POLL_MAX;
      }

      ret = usb_uicc_get_data_block_rsp();
      if (ret != 0)
      {
        usb_uicc_global.card_info.card_state = USB_UICC_CARD_ERROR;
        if (ret == ETIMEDOUT)
        {
          usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_COMMAND_TIMEOUT_V01;
        }
        else
        {
          usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01;
        }
        LOGE("Error reading in power on poll st, err = %d.\n", ret);
        return USB_UICC_ERR_FAIL;
      }
      /* do not need polling, skip poll state */
      if (usb_uicc_global.card_info.card_cmd_status == USB_UICC_CMD_SUCCESS)
      {
        ++ usb_uicc_global.usb_uicc_state_ptr;
      }
      /* keep in polling state */
      else if (usb_uicc_global.card_info.card_cmd_status == USB_UICC_CMD_TIME_EXTENSION)
      {
        ret = usb_uicc_start_timer(&usb_uicc_global.poll_timer, 0, 10000);
        ++ usb_uicc_global.usb_uicc_cmd.poll_counter;
        return USB_UICC_ERR_WAIT;
      }
      else
      {
        usb_uicc_global.card_info.card_state      = USB_UICC_CARD_ERROR;
        usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01;
        LOGE("command to usb uicc card error, cmd_status = 0x%x, cmd_err = 0x%x\n",
              usb_uicc_global.card_info.card_cmd_status,
              usb_uicc_global.card_info.card_error_code);
        return USB_UICC_ERR_FAIL;
      }

      break;
    case USB_UICC_DONE_ST:

      break;
    default:
      LOGE("never hit here");
      return USB_UICC_ERR_FAIL;
  } /* end switch */

  return USB_UICC_ERR_NONE;
} /* usb_uicc_process_send_apdu_states */

/*=========================================================================

  FUNCTION:  usb_uicc_process_power_down_states

===========================================================================*/

/*!
    @brief
    Performs to process power down state machine.

    @return
    usb_uicc_return_enum_type.
*/
/*=========================================================================*/
static usb_uicc_return_enum_type usb_uicc_process_power_down_states
(
  void
)
{
  int ret = -1;

  assert(NULL != usb_uicc_global.usb_uicc_state_ptr);
  LOGI ("curent state = %s.\n", usb_uicc_state_name[*usb_uicc_global.usb_uicc_state_ptr]);
  switch(*usb_uicc_global.usb_uicc_state_ptr)
  {
    case USB_UICC_POWER_OFF_ST:
      /*1. write PC_to_RDR_IccPowerOff_Request to usb icc card */
      ret = usb_uicc_send_icc_power_off_req();

      /*2. read Response from usb icc card  & check response */
      if (ret == 0)
      {
        ret = usb_uicc_get_slot_status_rsp();
      }

      if (ret != 0)
      {
        if((usb_uicc_global.usb_uicc_cmd.power_down_mode_valid == TRUE) &&
           (usb_uicc_global.usb_uicc_cmd.power_down_mode == UIM_REMOTE_POWER_DOWN_TELECOM_INTERFACE_V01))
        {
          LOGI("Error reading in power off st with telecom, err = %d.\n", ret);
          usb_uicc_global.card_info.card_state      = USB_UICC_CARD_ERROR;
        }
        else
        {
          usb_uicc_detach_usb();
          usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN_V01;
          LOGE("Error reading in power off st, err = %d.\n", ret);
          return USB_UICC_ERR_FAIL;
        }
      }
      ++ usb_uicc_global.usb_uicc_state_ptr; /* move to next state */
      break;

    case USB_UICC_CLOSE_ST:
      if((usb_uicc_global.usb_uicc_cmd.power_down_mode_valid == TRUE) &&
         (usb_uicc_global.usb_uicc_cmd.power_down_mode == UIM_REMOTE_POWER_DOWN_TELECOM_INTERFACE_V01))
      {
        usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN_TELECOM_V01;
        usb_uicc_global.card_info.card_state      = USB_UICC_CARD_NOT_INIT;
        ++ usb_uicc_global.usb_uicc_state_ptr; /* move to next state */
      }
      else
      {
        usb_uicc_detach_usb();
        usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN_V01;
        ++ usb_uicc_global.usb_uicc_state_ptr; /* move to next state */
      }
      break;

    case USB_UICC_DONE_ST:
      break;

    default:
      LOGE("never hit here");
      return USB_UICC_ERR_FAIL;
  } /* end switch*/

  return USB_UICC_ERR_NONE;
} /* usb_uicc_process_power_down_states */

/*=========================================================================

  FUNCTION:  usb_uicc_process_power_up_states

===========================================================================*/

/*!
    @brief
    Performs to process power up state machine.

    @return
    usb_uicc_return_enum_type.
*/
/*=========================================================================*/
static usb_uicc_return_enum_type usb_uicc_process_power_up_states
(
  void
)
{
  int ret = -1;

  assert (NULL != usb_uicc_global.usb_uicc_state_ptr);
  LOGI ("curent state = %s.\n", usb_uicc_state_name[*usb_uicc_global.usb_uicc_state_ptr]);
  LOGI ("usb state = %d\n", usb_uicc_global.usb_state);
  switch (*usb_uicc_global.usb_uicc_state_ptr)
  {
    case USB_UICC_NOT_INIT_ST:
      usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01;
      LOGI("usb uicc not init.\n");
      return USB_UICC_ERR_FAIL;
      break;

    case USB_UICC_ATTACH_ST:
      if (usb_uicc_global.usb_state == UIM_USB_STATE_UNKNOWN ||
          usb_uicc_global.usb_state == UIM_USB_STATE_DISCONNECTED)
      {
        if (pthread_create(&usb_uicc_global.open_thread_id,
                           NULL,
                           (void*)usb_uicc_open_thread,
                           NULL) != 0)
        {
          usb_uicc_global.card_info.card_state      = USB_UICC_CARD_NOT_INIT;
          usb_uicc_global.usb_state                 = UIM_USB_STATE_UNKNOWN;
          usb_uicc_global.usb_uicc_fd               = -1;
          usb_uicc_global.open_thread_id            = -1;
          usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
          LOGE("Could not start open thread ! \n");
          return USB_UICC_ERR_FAIL;
        }
        return USB_UICC_ERR_WAIT;
      }
      else if (usb_uicc_global.usb_state == UIM_USB_STATE_CONNECTED)/* ccid bridge has opened */
      {
        /* get standard configuration descriptor in ISO7816-12 section 7.1.2 */

        /* create listen event thread */
        if (pthread_create(&usb_uicc_global.listen_evt_thread_id,
                           NULL,
                          (void*)usb_uicc_listen_event_thread,
                          (void*)&usb_uicc_global.usb_uicc_fd) != 0)
        {
          usb_uicc_close_ccid_device(usb_uicc_global.usb_uicc_fd);
          usb_uicc_global.card_info.card_state      = USB_UICC_CARD_NOT_INIT;
          usb_uicc_global.usb_state                 = UIM_USB_STATE_DISCONNECTED;
          usb_uicc_global.usb_uicc_fd               = -1;
          usb_uicc_global.listen_evt_thread_id      = -1;
          usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
          LOGE("Could not start ioctl thread ! \n");
          return USB_UICC_ERR_FAIL;
        }
        ++ usb_uicc_global.usb_uicc_state_ptr; /* move to next state */
      }
      else
      {
        usb_uicc_global.usb_uicc_cmd.cmd_err_code   = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
        LOGE("Could not open usb ccid device!\n");
        usb_uicc_global.card_info.card_state        = USB_UICC_CARD_NOT_INIT;
        usb_uicc_global.usb_state                   = UIM_USB_STATE_DISCONNECTED;
        usb_uicc_global.usb_uicc_fd                 = -1;
        usb_uicc_detach_usb(); /* open error, shutdown usb bridge */
        return USB_UICC_ERR_FAIL;
      }
      break;

    case USB_UICC_POWER_OFF_ST:
      /*1. write PC_to_RDR_IccPowerOff_Request to usb icc card */
      ret = usb_uicc_send_icc_power_off_req();
      /*2. read Response from usb icc card  & check response */
      if (ret == 0)
      {
        ret = usb_uicc_get_slot_status_rsp();
      }
      if (ret != 0)
      {
        usb_uicc_detach_usb();
        usb_uicc_global.usb_uicc_cmd.cmd_err_code   = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
        LOGE("Error reading in power off st, err = %d.\n", ret);
        return USB_UICC_ERR_FAIL;
      }

      ++ usb_uicc_global.usb_uicc_state_ptr; /* move to next state */
      break;

    case USB_UICC_GET_SLOT_STATUS_ST:
      /*1. write PC_to_RDR_GetSlotStatus_Request to usb icc card */
      ret = usb_uicc_send_get_slot_status_req();
      /*2. read Response from usb icc card */
      if (ret == 0)
      {
        ret = usb_uicc_get_slot_status_rsp();
      }
      if (ret != 0)
      {
        usb_uicc_detach_usb();
        usb_uicc_global.usb_uicc_cmd.cmd_err_code   = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
        LOGE("Error reading in get slot status st, err = %d.\n", ret);
        return USB_UICC_ERR_FAIL;
      }

      ++ usb_uicc_global.usb_uicc_state_ptr; /* move to next state */
      break;

    case USB_UICC_POWER_ON_ST:
      /*1. write PC_to_RDR_IccPowerOn_Request to usb icc card */
      ret = usb_uicc_send_icc_power_on_req();

      /*2. read Response from usb icc card, ATR is included */
      if (ret == 0)
      {
        ret = usb_uicc_get_data_block_rsp();
      }
      if (ret != 0)
      {
        usb_uicc_detach_usb();
        usb_uicc_global.usb_uicc_cmd.cmd_err_code   = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
        LOGE("Error reading in power on st, err = %d.\n", ret);
        return USB_UICC_ERR_FAIL;
      }

      ++usb_uicc_global.usb_uicc_state_ptr; /* move to next state */
      if (usb_uicc_global.card_info.card_cmd_status == USB_UICC_CMD_SUCCESS) /* do not need polling, skip poll state */
      {
        /* ATR is received */
        if (FALSE == usb_uicc_is_atr_valid(usb_uicc_global.usb_uicc_cmd.rsp_payload_ptr,
                                           usb_uicc_global.usb_uicc_cmd.rsp_payload_len))
        {
          usb_uicc_detach_usb();
          usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
          LOGE("Invalid atr.\n");
          return USB_UICC_ERR_FAIL;
        }
        ++usb_uicc_global.usb_uicc_state_ptr;
      }
      /* need polling procedure */
      else if (usb_uicc_global.card_info.card_cmd_status == USB_UICC_CMD_TIME_EXTENSION)
      {
        /* start poll timer (10ms), move to poll state */
        usb_uicc_global.usb_uicc_cmd.poll_counter = 0;
        ret = usb_uicc_start_timer(&usb_uicc_global.poll_timer, 0, 10000);
        return USB_UICC_ERR_WAIT;
      }
      else
      {
        usb_uicc_detach_usb();
        usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01;
        LOGE("Command to usb uicc card error, cmd_status = 0x%x, cmd_err = 0x%x\n",
             usb_uicc_global.card_info.card_cmd_status,
             usb_uicc_global.card_info.card_error_code);
        return USB_UICC_ERR_FAIL;
      }
      break;

    case USB_UICC_POLL_ST:
      if (usb_uicc_global.usb_uicc_cmd.poll_counter >= USB_UICC_MAX_POLL)
      {
        usb_uicc_detach_usb();
        LOGE("ATR poll reach max.\n");
        return USB_UICC_ERR_FAIL;
      }

      ret = usb_uicc_get_data_block_rsp();
      if (ret != 0)
      {
        usb_uicc_detach_usb();
        usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
        LOGE("Error reading in power on st, err = %d.\n", ret);
        return USB_UICC_ERR_FAIL;
      }

      if (usb_uicc_global.card_info.card_cmd_status == USB_UICC_CMD_SUCCESS) /* do not need polling, skip poll state */
      {
        /* ATR is received */
        if (FALSE == usb_uicc_is_atr_valid(usb_uicc_global.usb_uicc_cmd.rsp_payload_ptr,
                                           usb_uicc_global.usb_uicc_cmd.rsp_payload_len))
        {
          usb_uicc_detach_usb();
          usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
          LOGE("Invalid atr.\n");
          return USB_UICC_ERR_FAIL;
        }
        ++ usb_uicc_global.usb_uicc_state_ptr;
      }
      else if (usb_uicc_global.card_info.card_cmd_status == USB_UICC_CMD_TIME_EXTENSION) /* keep in polling state */
      {
        ret = usb_uicc_start_timer(&usb_uicc_global.poll_timer, 0, 10000);
        ++ usb_uicc_global.usb_uicc_cmd.poll_counter;
        return USB_UICC_ERR_WAIT;
      }
      else
      {
        usb_uicc_detach_usb();
        usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01;
        LOGE("Command to usb uicc card error, cmd_status = 0x%x, cmd_err = 0x%x\n",
             usb_uicc_global.card_info.card_cmd_status,
             usb_uicc_global.card_info.card_error_code);
        return USB_UICC_ERR_FAIL;
      }
      break;

    case USB_UICC_INIT_DONE_ST:
      break;

    default:
      LOGE("never hit here.\n");
      return USB_UICC_ERR_FAIL;

  } /* end switch */

  return USB_UICC_ERR_NONE;
} /* usb_uicc_process_power_up_states */

/*=========================================================================

  FUNCTION:  usb_uicc_global_init

===========================================================================*/
/*!
    @brief
    Initializes the global variable.

    @return
    TRUE if successful, else FALSE
*/
/*=========================================================================*/
static void usb_uicc_global_init
(
  void
)
{
  char  value[PROPERTY_VALUE_MAX] = {'\0'};

  write_mem_buffer_ptr                       = NULL;
  memset(read_static_buffer,  0x00, sizeof (read_static_buffer));
  memset(write_static_buffer, 0x00, sizeof (write_static_buffer));

   /* Initialize the global structure */
  memset(&usb_uicc_global, 0x00, sizeof(usb_uicc_global_data_type));
  usb_uicc_global.listen_evt_thread_id               = -1;
  usb_uicc_global.open_thread_id                     = -1;
#ifndef USB_UICC_QMI_UT
  usb_uicc_global.qmi_msg_lib_handle                 = -1;
#endif /* USB_UICC_QMI_UT */
#ifdef USB_UICC_QMI_UT
  usb_uicc_global.ut_thread_id                       = -1;
#endif /* USB_UICC_QMI_UT */
  usb_uicc_global.usb_uicc_fd                        = -1;
  usb_uicc_global.is_hardware_error                  = FALSE;
  usb_uicc_global.card_info.card_state               = USB_UICC_CARD_NOT_INIT;
  usb_uicc_global.slot                               = UIM_REMOTE_SLOT_NOT_APPLICABLE_V01;
  usb_uicc_global.usb_uicc_cmd.command               = USB_UICC_MAX_CMD;
  usb_uicc_global.usb_uicc_cmd.power_down_mode_valid = FALSE;

  usb_uicc_global.enumeration_timer.timer_cb         = usb_uicc_enumeration_timer_cb;
  usb_uicc_global.poll_timer.timer_cb                = usb_uicc_poll_timer_cb;

  /* get target information
     8x10/8x12/8916: ME <-> SPI bus  <-> SPI BRIDGE <-> USB UICC
     8926:           ME <-> HSIC     <-> HSIC HUB   <-> USB UICC
     8974:           ME <-> USB2(HS) <-> USB UICC
     8936:           ME <-> USB2(FS) <-> USB UICC
  */
  if ( property_get("ro.board.platform", value, "0") &&
      (!strncmp(value, "msm8610", strlen("msm8610")) || !strncmp(value, "msm8916", strlen("msm8916"))) )
  {
    usb_uicc_global.bridge_type                      = USB_UICC_SPI_BRIDGE_SOLN_TYPE;
  }
  else if (property_get("ro.board.platform", value, "0") && (!strncmp(value, "msm8226", strlen("msm8226"))))
  {
    usb_uicc_global.bridge_type                      = USB_UICC_HSIC_HUB_SOLN_TYPE;
  }
  else if (property_get("ro.board.platform", value, "0") &&
          (!strncmp(value, "msm8974", strlen("msm8974")) || !strncmp(value, "msm8936", strlen("msm8936"))) )
  {
    usb_uicc_global.bridge_type                      = USB_UICC_DIRECT_CONNECT_SOLN_TYPE;
  }
  else
  {
    usb_uicc_global.bridge_type                      = USB_UICC_INVALID_SOLN_TYPE;
  }

  LOGI("target type = %d\n", usb_uicc_global.bridge_type);

  usb_uicc_close_ccid_device(usb_uicc_global.usb_uicc_fd);
} /* usb_uicc_global_init */

/*=========================================================================

  FUNCTION:  usb_uicc_cleanup

===========================================================================*/
/*!
    @brief
    Signal handler that performs the cleanup for the usb uicc Daemon. This is
    called for the common exit signals we registered with the kernel before.

    @return
    None.
*/
/*=========================================================================*/
static void usb_uicc_cleanup
(
  int   signal
)
{
  LOGI("usb_uicc_cleanup, signal received: 0x%x \n", signal);

  USB_UICC_ENTER_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");
#ifndef USB_UICC_QMI_UT
  LOGI("usb_uicc_cleanup, Issuing QMI deinit ... \n");
  usb_uicc_global.qmi_msg_lib_handle = -1;
  if (usb_uicc_global.qmi_uim_svc_client_ptr)
  {
    qmi_client_release(usb_uicc_global.qmi_uim_svc_client_ptr);
    usb_uicc_global.qmi_uim_svc_client_ptr = NULL;
  }
#endif /* USB_UICC_QMI_UT */
#ifdef USB_UICC_QMI_UT
  if (-1 != usb_uicc_global.ut_thread_id)
  {
    usb_uicc_cancel_thread(usb_uicc_global.ut_thread_id);
    usb_uicc_global.ut_thread_id = -1;
  }
#endif /* USB_UICC_QMI_UT */

  if (NULL != write_mem_buffer_ptr)
  {
    free(write_mem_buffer_ptr);
    write_mem_buffer_ptr = NULL;
  }
  usb_uicc_reset_global_data(TRUE);

  USB_UICC_LEAVE_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");

  exit(0);
} /* usb_uicc_cleanup */

/*=========================================================================

  FUNCTION:  main

===========================================================================*/
/*!
    @brief
    Main entry point for the usb uicc Daemon.

    @return
    None.
*/
/*=========================================================================*/
int main (int argc, char **argv)
{
  usb_uicc_return_enum_type ret;

  /* Init global variable */
  usb_uicc_global_init();

  sem_init(&usb_uicc_sem, 0, 0);

  LOGI("Starting USB UICC Task !!! \n");

#ifndef USB_UICC_QMI_UT
  /* Initialize QMI connection & QMI_UIM service */
  if (!usb_uicc_init_qmi())
  {
    LOGE("usb_uicc_init_qmi failed ! \n");
    goto cleanup_and_exit;
  }
#endif /* USB_UICC_UT */
  /* Register a signal handler for daemon exit signals
     Below are the process exit signals that are supported */
  signal(SIGINT,  usb_uicc_cleanup);
  signal(SIGQUIT, usb_uicc_cleanup);
  signal(SIGTERM, usb_uicc_cleanup);
  signal(SIGHUP,  usb_uicc_cleanup);

#ifdef USB_UICC_QMI_UT
  sem_init(&usb_uicc_ut_sem, 0, 0);
  usb_uicc_create_ut_thread();
  sem_post(&usb_uicc_ut_sem); /* start ut */
#endif /* USB_UICC_QMI_UT */

  LOGI("usb uicc daemon has started.\n");
  while(1)
  {
    sem_wait(&usb_uicc_sem);
    USB_UICC_ENTER_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");

    /* check hardware error first */
    if (TRUE == usb_uicc_global.is_hardware_error)
    {
      LOGE("hardware error happens!\n");
      usb_uicc_reset_global_data(TRUE);
      usb_uicc_global.usb_uicc_cmd.command      = USB_UICC_MAX_CMD;
      usb_uicc_global.usb_uicc_cmd.cmd_err_code = UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01;
      usb_uicc_create_and_send_remote_evt_req(UIM_REMOTE_CARD_ERROR_V01);
    }
    else
    {
      switch(usb_uicc_global.usb_uicc_cmd.command)
      {
        case USB_UICC_POWER_UP_CMD:
        case USB_UICC_RESET_CMD:
          ret = usb_uicc_process_power_up_states();
          /* process here
             if error or init complete, send information to modem
             else continue the state machine
          */
          if (ret == USB_UICC_ERR_FAIL)
          {
            usb_uicc_create_and_send_remote_evt_req(UIM_REMOTE_CARD_ERROR_V01);
          }
          else if (*usb_uicc_global.usb_uicc_state_ptr == USB_UICC_INIT_DONE_ST)
          {
            usb_uicc_create_and_send_remote_evt_req(UIM_REMOTE_CARD_RESET_V01);
          }
          else if (ret == USB_UICC_ERR_NONE)
          {
            sem_post(&usb_uicc_sem); /* continue the state machine */
          }
          break;
        case USB_UICC_POWER_DOWN_CMD:
          ret = usb_uicc_process_power_down_states();
          /* process here
             if error or init complete, send information to modem
             else continue the state machine
          */
          if((ret == USB_UICC_ERR_FAIL) || (*usb_uicc_global.usb_uicc_state_ptr == USB_UICC_DONE_ST))
          {
            usb_uicc_create_and_send_remote_evt_req(UIM_REMOTE_CARD_ERROR_V01);
          }
          else
          {
            sem_post(&usb_uicc_sem); /* continue the state machine */
          }
          break;
        case USB_UICC_SEND_APDU_CMD:
          ret = usb_uicc_process_send_apdu_states();
          /* process here
             if error or init complete, send information to modem
             else continue the state machine
          */
          if (ret == USB_UICC_ERR_FAIL)
          {
            usb_uicc_create_and_send_remote_apdu_req(QMI_RESULT_FAILURE_V01);
          }
          else if ((ret == USB_UICC_ERR_POLL_MAX) || (*usb_uicc_global.usb_uicc_state_ptr == USB_UICC_DONE_ST))
          {
            usb_uicc_create_and_send_remote_apdu_req(QMI_RESULT_SUCCESS_V01);
          }
          else if (ret == USB_UICC_ERR_NONE)
          {
            sem_post(&usb_uicc_sem); /* continue the state machine */
          }
          break;
        default:
          LOGE("usb uicc daemon is in idle\n");
      } /* end switch */
    }
    USB_UICC_LEAVE_CRITICAL_SECTION(&usb_uicc_mutex, "usb_uicc_mutex");
  }

cleanup_and_exit:
  /* clean up here */
  usb_uicc_cleanup(SIGQUIT);
  LOGI(" Exiting usb_uicc_daemon ...\n");
  return 0;
} /* main */
