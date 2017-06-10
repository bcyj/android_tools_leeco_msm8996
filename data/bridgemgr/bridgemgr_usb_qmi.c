/******************************************************************************

                    B R I D G E M G R _ U S B _ Q M I . C

******************************************************************************/

/******************************************************************************

  @file    bridgemgr_usb_qmi.c
  @brief   Bridge Manager USB QMI Functions Header File

  DESCRIPTION
  Header file for BridgeMgr USB QMI functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/25/11   sg         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>

#include "ds_string.h"
#include "bridgemgr_common.h"
#include "bridgemgr_usb_qmi.h"
#include "bridgemgr_mdm_qmi.h"


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define BRIDGEMGR_USB_QMI_USB_TTY_PORT        "/dev/rmnet_mux_ctrl"

/* Start offset of the service id in a QMUX message */
#define BRIDGEMGR_USB_QMI_QMUX_SRVC_ID_START  4
#define BRIDGEMGR_USB_QMI_QMUX_HDR_LEN        6
#define BRIDGEMGR_USB_QMI_QMUX_IF_ID          0x01

/* Defines for QMI CTL msg type */
#define BRIDGEMGR_USB_QMI_CTL_SDU_HDR_SIZE     2
#define BRIDGEMGR_USB_QMI_CTL_MSG_TYPE_SIZE    2
#define BRIDGEMGR_USB_QMI_CTL_MSG_LEN_SIZE     2
#define BRIDGEMGR_USB_QMI_CTL_MIN_SDU_LEN     (BRIDGEMGR_USB_QMI_CTL_SDU_HDR_SIZE  +\
                                               BRIDGEMGR_USB_QMI_CTL_MSG_TYPE_SIZE +\
                                               BRIDGEMGR_USB_QMI_CTL_MSG_LEN_SIZE)

/* Defines for a QMI CTL Get/Release client id message */
#define BRIDGEMGR_USB_QMI_CTL_GET_REL_CLNT_ID_TLV_ID         0x01

#define BRIDGEMGR_USB_QMI_CTL_GET_CLIENT_ID_MSG_LEN          1
#define BRIDGEMGR_USB_QMI_CTL_RELEASE_CLIENT_ID_MSG_LEN      2

#define BRIDGEMGR_USB_QMI_CTL_GET_REL_CLNT_ID_TYPE_LEN       1
#define BRIDGEMGR_USB_QMI_CTL_GET_REL_CLNT_ID_LENGTH_LEN     2
#define BRIDGEMGR_USB_QMI_CTL_GET_REL_CLNT_ID_SRVC_TYPE_LEN  1

#define BRIDGEMGR_USB_QMI_CTL_GET_REL_CLNT_ID_TLV_LEN          \
           (BRIDGEMGR_USB_QMI_CTL_GET_REL_CLNT_ID_TYPE_LEN   + \
            BRIDGEMGR_USB_QMI_CTL_GET_REL_CLNT_ID_LENGTH_LEN + \
            BRIDGEMGR_USB_QMI_CTL_GET_REL_CLNT_ID_SRVC_TYPE_LEN)


#define READ_16_BIT_VAL(buf,dest)                    \
do { const unsigned char *b_ptr = buf;               \
     unsigned char *d_ptr = (unsigned char *) &dest; \
     int unlikely_cntr;                              \
     dest = 0;                                       \
     for (unlikely_cntr=0; unlikely_cntr<2; unlikely_cntr++) {*d_ptr++ = *b_ptr++;} \
     buf = b_ptr;                                    \
   } while (0)

/* QMI CTL Service messages */
#define BRIDGEMGR_USB_QMI_CTL_SRVC_ID         0x0000

/* DTR related defines */
#define BRIDGEMGR_USB_QMI_DTR_UPDATE_DELAY    200000     /* In usec */
#define BRIDGEMGR_USB_QMI_GET_DTR_IOCTL       _IOR(0xFE, 0, int)

/* USB power-up state related defines */
#define BRIDGEMGR_USB_QMI_USB_STATE_FILE      "/sys/devices/platform/msm_hsusb/gadget/usb_state"
#define BRIDGEMGR_USB_QMI_USB_CONFIG_STR      "USB_STATE_CONFIGURED"
#define BRIDGEMGR_USB_QMI_USB_CONFIG_STR_LEN  (sizeof(BRIDGEMGR_USB_QMI_USB_CONFIG_STR)-1)


/* Set of QMI CTL messages currently supported */
typedef enum
{
  BRIDGEMGR_USB_QMI_CTL_INVALID           = -1,
  BRIDGEMGR_USB_QMI_CTL_SET_INSTANCE_ID   = 0x0020,
  BRIDGEMGR_USB_QMI_CTL_GET_VERSION_INFO  = 0x0021,
  BRIDGEMGR_USB_QMI_CTL_GET_CLIENT_ID     = 0x0022,
  BRIDGEMGR_USB_QMI_CTL_RELEASE_CLIENT_ID = 0x0023,
  BRIDGEMGR_USB_QMI_CTL_SET_DATA_FORMAT   = 0x0026,
  BRIDGEMGR_USB_QMI_CTL_SYNC              = 0x0027
} bridgemgr_usb_qmi_ctl_msg_type;

typedef struct
{
  qmi_service_id_type  srvc_id;
  bridgemgr_sys_type   sys;
} bridgemgr_usb_qmi_forward_type;

/* Table for determining where to forward the QMUX message for a given service */
static const bridgemgr_usb_qmi_forward_type bridgemgr_usb_qmi_forward_tbl[] =
{
  /* Forward WDS, QoS messages to MDM */
  { QMI_WDS_SERVICE,    BRIDGEMGR_SYS_MDM_QMI   },
  { QMI_QOS_SERVICE,    BRIDGEMGR_SYS_MDM_QMI   },

  /* Forward DMS, NAS, WMS, VOICE, PBM, SAR to QMI Proxy */
  { QMI_DMS_SERVICE,    BRIDGEMGR_SYS_QMI_PROXY },
  { QMI_NAS_SERVICE,    BRIDGEMGR_SYS_QMI_PROXY },
  { QMI_WMS_SERVICE,    BRIDGEMGR_SYS_QMI_PROXY },
  { QMI_VOICE_SERVICE,  BRIDGEMGR_SYS_QMI_PROXY },
  { QMI_PBM_SERVICE,    BRIDGEMGR_SYS_QMI_PROXY },
  { QMI_RF_SAR_SERVICE, BRIDGEMGR_SYS_QMI_PROXY },

  /* This should be the last entry */
  { QMI_MAX_SERVICES,   BRIDGEMGR_SYS_INVALID   },
};

/* Function pointer type for processing a QMI CTL message */
typedef int (*bridgemgr_usb_qmi_ctl_msg_process_func)
(
  int type,
  int length,
  const unsigned char *msg,
  bridgemgr_sys_type *sys
);

static int bridgemgr_usb_qmi_generic_process_ctl_msg
(
  int                 msg_id,
  int                 msg_len,
  const unsigned char *msg,
  bridgemgr_sys_type  *sys
);

static int bridgemgr_usb_qmi_process_get_rel_client_id
(
  int                 msg_id,
  int                 msg_len,
  const unsigned char *msg,
  bridgemgr_sys_type  *sys
);

typedef struct
{
  bridgemgr_usb_qmi_ctl_msg_type          msg_id;
  bridgemgr_usb_qmi_ctl_msg_process_func  process_func;
} bridgemgr_usb_qmi_ctl_msg_process_type;

/* Table for determining processing furnction for different QMI CTL TLV IDs */
static const bridgemgr_usb_qmi_ctl_msg_process_type bridgemgr_usb_qmi_ctl_msg_process_tbl[] =
{
  { BRIDGEMGR_USB_QMI_CTL_SET_INSTANCE_ID,   bridgemgr_usb_qmi_generic_process_ctl_msg   },
  { BRIDGEMGR_USB_QMI_CTL_GET_VERSION_INFO,  bridgemgr_usb_qmi_generic_process_ctl_msg   },
  { BRIDGEMGR_USB_QMI_CTL_GET_CLIENT_ID,     bridgemgr_usb_qmi_process_get_rel_client_id },
  { BRIDGEMGR_USB_QMI_CTL_RELEASE_CLIENT_ID, bridgemgr_usb_qmi_process_get_rel_client_id },
  { BRIDGEMGR_USB_QMI_CTL_SET_DATA_FORMAT,   bridgemgr_usb_qmi_generic_process_ctl_msg   },
  { BRIDGEMGR_USB_QMI_CTL_SYNC,              bridgemgr_usb_qmi_generic_process_ctl_msg   },

  /* This should be the last entry */
  { BRIDGEMGR_USB_QMI_CTL_INVALID,           NULL                                        }
};

/* Enum for representing the current DTR state */
typedef enum
{
  BRIDGEMGR_USB_QMI_DTR_STATUS_INVALID = -1,
  BRIDGEMGR_USB_QMI_DTR_STATUS_LOW,
  BRIDGEMGR_USB_QMI_DTR_STATUS_HIGH
} bridgemgr_usb_qmi_dtr_status_type;

/* Info required for DTR handling */
typedef struct
{
  bridgemgr_usb_qmi_dtr_status_type  prev_dtr_status;
  int                                usb_fd;
  boolean                            dtr_thread_running;
  pthread_t                          dtr_thread_id;
} bridgemgr_usb_qmi_dtr_info_type;

static bridgemgr_usb_qmi_dtr_info_type bridgemgr_usb_qmi_dtr_info =
{
  BRIDGEMGR_USB_QMI_DTR_STATUS_INVALID,  /* prev_dtr_status */
  BRIDGEMGR_FD_INVALID,                  /* usb_fd */
  FALSE,                                 /* dtr_thread_running */
  0                                      /* dtr_thread_id */
};


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_get_dtr_string
===========================================================================*/
/*!
@brief
  This is the entry function for DTR monitoring thread

@param
  arg - Argument passed during creation

@return
  NULL

*/
/*=========================================================================*/
static const char *bridgemgr_usb_qmi_get_dtr_str
(
  bridgemgr_usb_qmi_dtr_status_type  dtr
)
{
  const char *ret = NULL;

  switch (dtr)
  {
    case BRIDGEMGR_USB_QMI_DTR_STATUS_HIGH:
      ret = "HIGH";
      break;

    case BRIDGEMGR_USB_QMI_DTR_STATUS_LOW:
      ret = "LOW";
      break;

    default:
      ret = "INVALID";
      break;
  }

  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_update_dtr
===========================================================================*/
/*!
@brief
  This function reads the DTR bits from USB and updates the modem if there's
  any change from the last time. Also, on DTR low informs SYS_QMI_PROXY to
  cleanup.

@param
  None

@return
  None

*/
/*=========================================================================*/
static void bridgemgr_usb_qmi_update_dtr(void)
{
  unsigned long usb_tio_bits = 0; /* USB terminal IO bits */
  int cmd;
  char *cmd_str = NULL;
  bridgemgr_usb_qmi_dtr_status_type dtr_status = BRIDGEMGR_USB_QMI_DTR_STATUS_INVALID;
  bridgemgr_usb_qmi_dtr_status_type prev_dtr_status;


  /* Obtain the current terminal status bits from USB */
  if (ioctl(bridgemgr_usb_qmi_dtr_info.usb_fd,
            BRIDGEMGR_USB_QMI_GET_DTR_IOCTL,
            &usb_tio_bits) < 0)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_update_dtr: USB TIOCMGET ioctl failed\n");
    goto bail;
  }

  /* Determine the current DTR status */
  dtr_status = (usb_tio_bits & TIOCM_DTR) ? BRIDGEMGR_USB_QMI_DTR_STATUS_HIGH
                                          : BRIDGEMGR_USB_QMI_DTR_STATUS_LOW;

  prev_dtr_status = bridgemgr_usb_qmi_dtr_info.prev_dtr_status;

  /* Avoid sending spurious DTR LOW update on the first run */
  if (BRIDGEMGR_USB_QMI_DTR_STATUS_INVALID == prev_dtr_status &&
      BRIDGEMGR_USB_QMI_DTR_STATUS_LOW     == dtr_status)
  {
    bridgemgr_log_med("bridgemgr_usb_qmi_update_dtr: Skipping spurious DTR update %s -> %s\n",
                      bridgemgr_usb_qmi_get_dtr_str(prev_dtr_status),
                      bridgemgr_usb_qmi_get_dtr_str(dtr_status));
  }
  /* If the DTR status has changed from last time */
  else if (dtr_status != prev_dtr_status)
  {
    unsigned long mdm_tio_bits = 0; /* MDM terminal IO bits */

    bridgemgr_log_med("bridgemgr_usb_qmi_update_dtr: DTR changed %s -> %s\n",
                      bridgemgr_usb_qmi_get_dtr_str(prev_dtr_status),
                      bridgemgr_usb_qmi_get_dtr_str(dtr_status));

    /* Read the current modem terminal IO bits */
    if (BRIDGEMGR_SUCCESS != bridgemgr_mdm_qmi_ioctl(TIOCMGET, &mdm_tio_bits))
    {
      bridgemgr_log_err("bridgemgr_usb_qmi_update_dtr: MDM ioctl cmd=%s failed\n",
                        BRIDGEMGR_TO_STR(TIOCMGET));
      goto bail;
    }

    /* If DTR went high enable the corresponding modem terminal IO bit, else clear it */
    if (BRIDGEMGR_USB_QMI_DTR_STATUS_HIGH == dtr_status)
    {
      mdm_tio_bits |= TIOCM_DTR;
    }
    else
    {
      bridgemgr_sys_type     sys         = BRIDGEMGR_SYS_QMI_PROXY;
      bridgemgr_reinit_type  reinit_type = BRIDGEMGR_REINIT_TYPE_FULL;


      mdm_tio_bits &= ~TIOCM_DTR;

      /* DTR went LOW, issue FULL re-init request for SYS_QMI_PROXY */
      if (BRIDGEMGR_SUCCESS != bridgemgr_common_issue_reinit_request(sys,
                                                                     reinit_type))
      {
        bridgemgr_log_err("bridgemgr_usb_qmi_update_dtr: renint type=%s failed for sys=%s\n",
                          bridgemgr_common_get_reinit_str(reinit_type),
                          bridgemgr_common_get_sys_str(sys));
      }
    }

    /* Update modem with the DTR change */
    if (BRIDGEMGR_SUCCESS != bridgemgr_mdm_qmi_ioctl(TIOCMSET, &mdm_tio_bits))
    {
      bridgemgr_log_err("bridgemgr_usb_qmi_update_dtr: MDM ioctl cmd=%s failed\n",
                        BRIDGEMGR_TO_STR(TIOCMSET));
      goto bail;
    }

    bridgemgr_log_med("bridgemgr_usb_qmi_update_dtr: MDM updated with DTR=%s\n",
                       bridgemgr_usb_qmi_get_dtr_str(dtr_status));
  }

  /* Save the last DTR status */
  bridgemgr_usb_qmi_dtr_info.prev_dtr_status = dtr_status;

bail:
  return;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_dtr_monitor
===========================================================================*/
/*!
@brief
  This is the entry function for DTR monitoring thread

@param
  arg - Argument passed during creation

@return
  NULL

*/
/*=========================================================================*/
static void *bridgemgr_usb_qmi_dtr_monitor
(
  void *arg
)
{
  struct sigaction action;


  (void)arg;

  memset(&action, 0, sizeof(action));
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = bridgemgr_common_signal_handler;

  if (sigaction(SIGUSR2, &action, NULL) < 0)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_dtr_monitor: sigaction failed\n");
  }

  /* Periodically read the DTR info from USB and update the modem if
     there's any change */
  for (;;)
  {
    bridgemgr_usb_qmi_update_dtr();
    usleep(BRIDGEMGR_USB_QMI_DTR_UPDATE_DELAY);
  }

  return NULL;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_start_dtr_monitor_thread
===========================================================================*/
/*!
@brief
  This function is called to start a thread when the USB cable is connected.
  This thread periodically reads the DTR bit from USB and updates the modem
  if there's any change

@param
  None

@return
  BRIDGEMGR_SUCCESS - Thread was successfully started
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
static int bridgemgr_usb_qmi_start_dtr_monitor_thread(void)
{
  int ret = BRIDGEMGR_FAILURE;
  int s;


  if (TRUE == bridgemgr_usb_qmi_dtr_info.dtr_thread_running)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_start_dtr_monitor_thread: "
                      "DTR monitor thread is already running\n");
    goto bail;
  }

  s = pthread_create(&bridgemgr_usb_qmi_dtr_info.dtr_thread_id,
                     NULL,
                     bridgemgr_usb_qmi_dtr_monitor,
                     NULL);

  if (0 != s)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_start_dtr_monitor_thread: "
                      "failed to start DTR monitor thread\n");
    goto bail;
  }

  bridgemgr_log_med("bridgemgr_usb_qmi_start_dtr_monitor_thread: "
                    "DTR monitor thread succesfully started\n");

  bridgemgr_usb_qmi_dtr_info.dtr_thread_running = TRUE;
  ret = BRIDGEMGR_SUCCESS;

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_dtr_cleanup
===========================================================================*/
/*!
@brief
  This function is called to stop the DTR processing thread on a USB cable
  disconnect

@param
  None

@return
  BRIDGEMGR_SUCCESS - Thread was successfully stopped
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
static int bridgemgr_usb_qmi_dtr_cleanup(void)
{
  int ret = BRIDGEMGR_FAILURE;
  int s;


  if (FALSE == bridgemgr_usb_qmi_dtr_info.dtr_thread_running)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_dtr_cleanup: "
                      "DTR monitor thread is NOT running\n");
    goto bail;
  }

  if (BRIDGEMGR_SUCCESS != bridgemgr_common_stop_thread(bridgemgr_usb_qmi_dtr_info.dtr_thread_id))
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_dtr_cleanup: DTR thread stop failure\n");
  }
  else
  {
    bridgemgr_log_med("bridgemgr_usb_qmi_dtr_cleanup: DTR thread stop success\n");

    /* Update the DTR info incase we stopped the DTR thread prematurely */
    bridgemgr_usb_qmi_update_dtr();
  }

  /* Reset to DTR info to invalid values */
  bridgemgr_usb_qmi_dtr_info.dtr_thread_running = FALSE;
  bridgemgr_usb_qmi_dtr_info.prev_dtr_status = BRIDGEMGR_USB_QMI_DTR_STATUS_INVALID;

  ret = BRIDGEMGR_SUCCESS;

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_validate_get_srvc_id
===========================================================================*/
/*!
@brief
  This function validates that the messaage is a QMUX message and extracts
  the service id from the message which is returned via srvc_id parameter

@param
  data - QMUX message
  size - size of the message
  srvc_id [out] - Service id extracted from the message

@return
  BRIDGEMGR_SUCCESS - If service id is successfully extracted
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
static int bridgemgr_usb_qmi_validate_get_srvc_id
(
  const unsigned char *qmsgh,
  int size,
  int *srvc_id
)
{
  int ret = BRIDGEMGR_FAILURE;


  if (NULL == qmsgh || size < BRIDGEMGR_USB_QMI_QMUX_HDR_LEN || NULL == srvc_id)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_validate_get_srvc_id: bad params\n");
    goto bail;
  }

  /* Make sure the i/f id is valid */
  if (*qmsgh != BRIDGEMGR_USB_QMI_QMUX_IF_ID)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_validate_get_srvc_id: invalid i/f in QMUX msg\n");
    goto bail;
  }

  /* Skip over the i/f, length and ctl flags fields in the QMUX mesg */
  qmsgh += BRIDGEMGR_USB_QMI_QMUX_SRVC_ID_START;
  *srvc_id = *qmsgh;

  ret = BRIDGEMGR_SUCCESS;

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_get_fwd_sys
===========================================================================*/
/*!
@brief
  This function determines the subsystem to which the message should be
  forwarded given the QMI message's service id

@param
  srvc_id   - QMI message service id
  sys [out] - The subsystem to which the message should be forwarded

@return
  BRIDGEMGR_SUCCESS - If a valid system has been found for the given srvc_id
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
static int bridgemgr_usb_qmi_get_fwd_sys
(
  qmi_service_id_type srvc_id,
  bridgemgr_sys_type  *sys
)
{
  const bridgemgr_usb_qmi_forward_type *fwd = NULL;
  int ret = BRIDGEMGR_FAILURE;


  if (NULL == sys)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_get_fwd_sys: bad params\n");
    goto bail;
  }

  for (fwd = &bridgemgr_usb_qmi_forward_tbl[0]; QMI_MAX_SERVICES != fwd->srvc_id; ++fwd)
  {
    if (fwd->srvc_id == srvc_id)
    {
      *sys = fwd->sys;
      ret  = BRIDGEMGR_SUCCESS;
      break;
    }
  }

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_generic_process_ctl_msg
===========================================================================*/
/*!
@brief
  Processes Get_Version_Info, Set_Data_Format, Sync TLV IDs of a QMI CTL
  msg and returns the corresponding sys which can handle this message

@param
  msg_id    - TLV ID of the message
  msg_len   - length of the message
  msg       - QMI message content
  sys [out] - corresponding system that can handle this QMI CTL msg

@return
  BRIDGEMGR_SUCCESS - If a valid sys is found
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
static int bridgemgr_usb_qmi_generic_process_ctl_msg
(
  int                 msg_id,
  int                 msg_len,
  const unsigned char *msg,
  bridgemgr_sys_type  *sys
)
{
  if (NULL == sys)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_generic_process_ctl_msg: bad params\n");
    return BRIDGEMGR_FAILURE;
  }

  (void)msg;
  (void)msg_len;

  switch (msg_id)
  {
    case BRIDGEMGR_USB_QMI_CTL_GET_VERSION_INFO:
      *sys = BRIDGEMGR_SYS_QMI_PROXY;
      break;

    case BRIDGEMGR_USB_QMI_CTL_SET_INSTANCE_ID:
    case BRIDGEMGR_USB_QMI_CTL_SET_DATA_FORMAT:
    case BRIDGEMGR_USB_QMI_CTL_SYNC:
      *sys = BRIDGEMGR_SYS_MDM_QMI;
      break;

    default:
      bridgemgr_log_err("bridgemgr_usb_qmi_generic_process_ctl_msg: unhandled msg_id=%d\n", msg_id);
      return BRIDGEMGR_FAILURE;
  }

  return BRIDGEMGR_SUCCESS;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_process_get_rel_client_id
===========================================================================*/
/*!
@brief
  Processes the Get, Release client ID TLV IDs of a QMI CTL msg and returns
  the sys corresponding to the srvc type in the msg

@param
  msg_id    - TLV ID of the message
  msg_len   - length of the message
  msg       - QMI message content
  sys [out] - system corresponding to the srvc type in the QMI CTL msg

@return
  BRIDGEMGR_SUCCESS - 
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
static int bridgemgr_usb_qmi_process_get_rel_client_id
(
  int                 msg_id,
  int                 msg_len,
  const unsigned char *msg,
  bridgemgr_sys_type  *sys
)
{
  int ret = BRIDGEMGR_FAILURE;
  unsigned short srvc_type_len;
  int srvc_type;


  if ((BRIDGEMGR_USB_QMI_CTL_GET_CLIENT_ID     != msg_id &&
       BRIDGEMGR_USB_QMI_CTL_RELEASE_CLIENT_ID != msg_id)     ||
      msg_len < BRIDGEMGR_USB_QMI_CTL_GET_REL_CLNT_ID_TLV_LEN ||
      NULL == msg                                             ||
      NULL == sys)

  {
    bridgemgr_log_err("bridgemgr_usb_qmi_process_get_rel_client_id: "
                      "bad params msg_id=%d, msg_len=%d\n",
                      msg_id, msg_len);
    goto bail;
  }

  /* Read the TLV which contains the service id */
  if (BRIDGEMGR_USB_QMI_CTL_GET_REL_CLNT_ID_TLV_ID != *msg)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_process_get_rel_client_id: invalid tlv=%d\n", *msg);
    goto bail;
  }

  /* Skip over the TLV ID */
  ++msg;

  /* Read the length value */
  READ_16_BIT_VAL(msg, srvc_type_len);

  /* Length should be 1 for Get_Client_Id and 2 for Release_Client_Id */
  if ((BRIDGEMGR_USB_QMI_CTL_GET_CLIENT_ID == msg_id && 
       srvc_type_len != BRIDGEMGR_USB_QMI_CTL_GET_CLIENT_ID_MSG_LEN) ||
      (BRIDGEMGR_USB_QMI_CTL_RELEASE_CLIENT_ID == msg_id &&
       srvc_type_len != BRIDGEMGR_USB_QMI_CTL_RELEASE_CLIENT_ID_MSG_LEN))
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_process_get_rel_client_id: invalid len=%d\n", srvc_type_len);
    goto bail;
  }

  srvc_type = *msg;

  /* Extract the service id from the message content */
  ret = bridgemgr_usb_qmi_get_fwd_sys(srvc_type, sys);

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_ctl_msg_get_fwd_sys
===========================================================================*/
/*!
@brief
  This function determines the subsystem to which the message should be
  forwarded to given a QMI message CTL message

@param
  qmi_ctl_sdu - QMI CTL message SDU
  qmi_sdu_len - Length of the QMI CTL msg
  sys [out]   - The subsystem to which the message should be forwarded

@return
  BRIDGEMGR_SUCCESS - If a valid system is found for the given QMI CTL msg
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
static int bridgemgr_usb_qmi_ctl_msg_get_fwd_sys
(
  const unsigned char *qmi_ctl_sdu,
  int                 qmi_sdu_len,
  bridgemgr_sys_type  *sys
)
{
  int ret = BRIDGEMGR_FAILURE;
  const bridgemgr_usb_qmi_ctl_msg_process_type *msg_proc = NULL;
  unsigned short qmi_ctl_msg_id;
  unsigned short qmi_ctl_msg_len;


  if (NULL == qmi_ctl_sdu                             ||
      qmi_sdu_len < BRIDGEMGR_USB_QMI_CTL_MIN_SDU_LEN ||
      NULL == sys)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_ctl_msg_get_fwd_sys: bad params\n");
    goto bail;
  }

  /* Skip the SDU header */
  qmi_ctl_sdu += BRIDGEMGR_USB_QMI_CTL_SDU_HDR_SIZE;

  READ_16_BIT_VAL(qmi_ctl_sdu, qmi_ctl_msg_id);
  READ_16_BIT_VAL(qmi_ctl_sdu, qmi_ctl_msg_len);

  msg_proc = &bridgemgr_usb_qmi_ctl_msg_process_tbl[0];
  for (; BRIDGEMGR_USB_QMI_CTL_INVALID != msg_proc->msg_id; ++msg_proc)
  {
    if (qmi_ctl_msg_id == msg_proc->msg_id)
    {
      ret = msg_proc->process_func(qmi_ctl_msg_id, qmi_ctl_msg_len, qmi_ctl_sdu, sys);
      break;
    }
  }

  if (BRIDGEMGR_SUCCESS != ret)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_ctl_msg_get_fwd_sys: unhandled CTL msg_id=0x%x\n",
                      qmi_ctl_msg_id);
  }

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_process_cb
===========================================================================*/
/*!
@brief
  This is the BRIDGMGR_SYS_MDM_QMI system registered processing callback
  function. This function forwards the QMI message received from USB to
  either the QMI Proxy Daemon or MDM depending on the message's service id

@param
  data - data to be processed
  size - size of the data

@return
  BRIDGEMGR_SUCCESS - If processing was successful
  BRIDGEMGR_FAILURE - Otherwise
 
@note 
  This function gets called in the context of cmdq thread 

*/
/*=========================================================================*/
static int bridgemgr_usb_qmi_process_cb
(
  const void *data,
  int size
)
{
  int                 ret     = BRIDGEMGR_FAILURE;
  bridgemgr_sys_type  sys     = BRIDGEMGR_SYS_INVALID;
  int                 srvc_id = QMI_MAX_SERVICES;
  const unsigned char *msg    = (unsigned char *)data;


  if (NULL == data || size <= 0)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_proccess_cb: bad params\n");
    goto bail;
  }

  /* We have received data from USB, determine where to forward it */
  /* Extract the QMI service id from the QMUX message */
  if (BRIDGEMGR_SUCCESS != bridgemgr_usb_qmi_validate_get_srvc_id(msg, size, &srvc_id))
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_proccess_cb: failed to validate data\n");
    goto bail;
  }

  /* For a QMI CTL msg, we need to extract the service id from the msg content */
  if (BRIDGEMGR_USB_QMI_CTL_SRVC_ID == srvc_id)
  {
    const unsigned char *qmi_sdu = msg + BRIDGEMGR_USB_QMI_QMUX_HDR_LEN;
    int qmi_sdu_size             = size - BRIDGEMGR_USB_QMI_QMUX_HDR_LEN;

    /* Determine which subsystem to forward the QMI CTL message */
    if (BRIDGEMGR_SUCCESS != bridgemgr_usb_qmi_ctl_msg_get_fwd_sys(qmi_sdu, qmi_sdu_size, &sys))
    {
      bridgemgr_log_err("bridgemgr_usb_qmi_proccess_cb: failed to find sys\n");
      goto bail;
    }
  }
  /* Determine which subsystem to forward the message for the service id */
  else if (BRIDGEMGR_SUCCESS != bridgemgr_usb_qmi_get_fwd_sys(srvc_id, &sys))
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_proccess_cb: no sys found for srvc=%d\n", srvc_id);
    goto bail;
  }


  bridgemgr_log_low("bridgemgr_usb_qmi_proccess_cb: found srvc_id=0x%x, sys=%s\n",
                    srvc_id, bridgemgr_common_get_sys_str(sys));

  /* Send the QMUX message to the corresponding subsystem */
  ret = bridgemgr_forward_data(sys, data, size);

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_cleanup_cb
===========================================================================*/
/*!
@brief
  This is the cleanup callback registered by BRIDGMGR_SYS_USB_QMI module 

@param
  cb - pointer to client registered callbacks

@return
  None

*/
/*=========================================================================*/
static void bridgemgr_usb_qmi_cleanup_cb
(
  bridgemgr_sys_type               sys,
  bridgemgr_client_callbacks_type  *cb
)
{
  if (BRIDGEMGR_SYS_USB_QMI != sys)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_cleanup_cb: invalid sys=%s\n",
                      bridgemgr_common_get_sys_str(sys));
    return;
  }

  /* Perform DTR cleanup */
  (void) bridgemgr_usb_qmi_dtr_cleanup();

  /* Perform common cleanup */
  bridgemgr_common_cleanup_cb(sys, cb);
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_read
===========================================================================*/
/*!
@brief
  This function is called by the read thread spawned during init

@param
  arg - argument passed during thread creation

@return
  NULL

*/
/*=========================================================================*/
static void *bridgemgr_usb_qmi_read
(
  void *arg
)
{
  int fd = (int)arg;
  struct sigaction action;


  memset(&action, 0, sizeof(action));
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = bridgemgr_common_signal_handler;

  /* Register to USR2 signal so as to exit when necessary */
  if (sigaction(SIGUSR2, &action, NULL) < 0)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_read: sigaction failed\n");
  }


  bridgemgr_log_low("bridgemgr_usb_qmi_read: reading from fd=%d\n", fd);

  /* Wait for initialization to complete */
  bridgemgr_common_wait_for_init(BRIDGEMGR_SYS_USB_QMI);

  for (;;)
  {
     /* Allocate a new cmd */
     bridgemgr_cmdq_cmd_type *cmd = bridgemgr_cmdq_alloc_cmd(BRIDGEMGR_SYS_USB_QMI);

    if (NULL == cmd)
    {
      bridgemgr_log_err("bridgemgr_usb_qmi_read: cmd alloc failed\n");
      break;
    }

    /* Read the QMUX message into the command buffer */
    if (BRIDGEMGR_SUCCESS == bridgemgr_common_read(BRIDGEMGR_SYS_USB_QMI,
                                                   fd,
                                                   cmd->cmd_data.data.qmux_nl.msg,
                                                   BRIDGEMGR_CMDQ_MAX_QMUX_NETLINK_MSG,
                                                   &cmd->cmd_data.data.qmux_nl.msg_len))
    {
      /* Print the received QMUX message */
      bridgemgr_common_print_qmux_msg(BRIDGEMGR_SYS_USB_QMI,
                                      cmd->cmd_data.data.qmux_nl.msg,
                                      cmd->cmd_data.data.qmux_nl.msg_len);

      /* Enqueue the command in the command queue */
      if (BRIDGEMGR_SUCCESS != bridgemgr_cmdq_enqueue_cmd(cmd))
      {
        bridgemgr_log_err("bridgemgr_usb_qmi_read: cmd enq failed\n");
        bridgemgr_cmdq_free_cmd(cmd);
      }
    }
    else
    {
      bridgemgr_cmdq_free_cmd(cmd);
      usleep(BRIDGEMGR_RETRY_DELAY);
    }
  }

  return NULL;
}


/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_is_usb_connected
===========================================================================*/
/*!
@brief
  This function is called to determine if USB cable is already connected
  so that DTR monitor thread can be spawned.

@param
  None

@return
  TRUE  - If USB cable is connected
  FALSE - Otherwise

*/
/*=========================================================================*/
static boolean bridgemgr_usb_qmi_is_usb_connected(void)
{
  boolean is_connected = FALSE;
  int fd = BRIDGEMGR_FD_INVALID;
  char usb_state_str[BRIDGEMGR_MAX_STR_LEN];
  int nbytes;

  fd = open(BRIDGEMGR_USB_QMI_USB_STATE_FILE, O_RDONLY);

  if (fd < 0)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_is_usb_connected: "
                      "failed to open USB state file\n");
    goto bail;
  }

  memset(usb_state_str, 0, sizeof(usb_state_str));

  if ((nbytes = read(fd, usb_state_str, BRIDGEMGR_MAX_STR_LEN)) < 0)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_is_usb_connected: "
                      "error reading from USB state file\n");
    goto bail;
  }

  /* Check if USB is connected */
  if(0 == std_strnicmp(usb_state_str,
                       BRIDGEMGR_USB_QMI_USB_CONFIG_STR,
                       BRIDGEMGR_USB_QMI_USB_CONFIG_STR_LEN))
  {
    is_connected = TRUE;
  }

bail:
  BRIDGEMGR_FD_CLOSE(fd);

  return is_connected;
}


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_usb_qmi_init
===========================================================================*/
/*!
@brief
  This function is called to initialize the BRIDGEMGR_SYS_QMI_PROXY module

@param
  client_cb - Set of callbacks to register

@return
  BRIDGEMGR_SUCCESS - Module initialization was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_usb_qmi_init
(
  bridgemgr_client_callbacks_type *client_cb
)
{
  int ret = BRIDGEMGR_FAILURE;
  int fd = BRIDGEMGR_FD_INVALID;
  pthread_t thr;


  if (NULL == client_cb)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_init: bad params\n");
    goto bail;
  }

  /* If USB is not connected, initialize this module when it is connected */
  if (TRUE != bridgemgr_usb_qmi_is_usb_connected())
  {
    bridgemgr_log_med("bridgemgr_usb_qmi_init: USB not connected, skipping init\n");
    goto bail;
  }

  bridgemgr_log_med("bridgemgr_usb_qmi_init: USB connected starting init\n");

  if ((fd = open(BRIDGEMGR_USB_QMI_USB_TTY_PORT, O_RDWR)) < 0)
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_init: failed to open USB TTY port=%s\n",
                      BRIDGEMGR_USB_QMI_USB_TTY_PORT);
    goto bail;
  }

  if (pthread_create(&thr, NULL, &bridgemgr_usb_qmi_read, (void *)fd))
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_init: failed to create read thread\n");
    goto bail;
  }

  if (BRIDGEMGR_SUCCESS != bridgemgr_usb_qmi_start_dtr_monitor_thread())
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_init: failed to start DTR monitor thread\n");
  }

  /* Save the fd for use by ioctl function */
  bridgemgr_usb_qmi_dtr_info.usb_fd = fd;

  /* Update the client callback info */
  client_cb->fd             = fd;
  client_cb->process_func   = bridgemgr_usb_qmi_process_cb;
  client_cb->cleanup_func   = bridgemgr_usb_qmi_cleanup_cb;
  client_cb->read_thread_id = thr;

  ret = BRIDGEMGR_SUCCESS;

bail:
  if (BRIDGEMGR_SUCCESS != ret)
  {
    BRIDGEMGR_FD_CLOSE(fd);
  }

  return ret;
}

