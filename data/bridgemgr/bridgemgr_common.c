/******************************************************************************

                 B R I D G E M G R _ C O M M O N . C

******************************************************************************/

/******************************************************************************

  @file    bridgemgr_common.c
  @brief   Bridge Manager Common Functions Definitions

  DESCRIPTION
  Implementation of BridgeMgr common functions.

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

#include "bridgemgr.h"
#include "bridgemgr_common.h"


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define BRIDGEMGR_COMMON_NUM_BYTES_PER_LINE 8

typedef struct
{
  bridgemgr_sys_type  sys;
  const char          *sys_str;
} bridgemgr_common_sys_str_type;

static const bridgemgr_common_sys_str_type bridgemgr_common_sys_str_tbl[] =
{
  { BRIDGEMGR_SYS_USB_QMI,    "SYS_USB_QMI"    },
  { BRIDGEMGR_SYS_QMI_PROXY,  "SYS_QMI_PROXY"  },
  { BRIDGEMGR_SYS_MDM_QMI,    "SYS_MDM_QMI"    },
  { BRIDGEMGR_SYS_PS_NETLINK, "SYS_PS_NETLINK" },
  { BRIDGEMGR_SYS_PS_QMI_IND, "SYS_PS_QMI_IND" },
  { BRIDGEMGR_SYS_REINIT,     "SYS_REINIT"     },

  /* This should be the last entry */
  { BRIDGEMGR_SYS_MAX,        "SYS_INVALID"    }
};

/* Flag indicating whether or not to log QMUX QMI messages */
static boolean bridgemgr_debug_log_qmux_msg = TRUE;


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_common_get_hex_char
===========================================================================*/
/*!
@brief
  This function returns the hexadecimal character corresponding to the given
  integer value

@param
  data - integer value

@return
  hexadecimal character representation of the given integer data - if valid
  '*' - Otherwise

*/
/*=========================================================================*/
static char bridgemgr_common_to_hex_char
(
  unsigned int data
)
{
  static const char int_to_hex_tbl[] =
  {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };


  if (data > 0xF)
  {
    bridgemgr_log_err("bridgemgr_common_to_hex_char: invalid data\n");
    return '*';
  }

  return int_to_hex_tbl[data];
}


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_common_get_sys_str
===========================================================================*/
/*!
@brief
  This function returns the string representation of the given module enum

@param
  sys - Module enum

@return
  String corresponding to the given enum

*/
/*=========================================================================*/
const char *bridgemgr_common_get_sys_str
(
  bridgemgr_sys_type sys
)
{
  const bridgemgr_common_sys_str_type *sys_ptr = NULL;


  sys_ptr = &bridgemgr_common_sys_str_tbl[0];

  /* Find the entry corresponding entry in the table */
  for (; BRIDGEMGR_SYS_MAX != sys_ptr->sys; ++sys_ptr)
  {
    /* Break out if we have found the entry */
    if (sys_ptr->sys == sys)
    {
      break;
    }
  }

  return sys_ptr->sys_str;
}


/*===========================================================================
  FUNCTION  bridgemgr_common_get_reinit_str
===========================================================================*/
/*!
@brief
  This function returns the string representation of the given re-init type

@param
  reinit - reinit type 

@return
  String corresponding to the given enum

*/
/*=========================================================================*/
const char *bridgemgr_common_get_reinit_str
(
  bridgemgr_reinit_type reinit
)
{
  const char *reinit_str = NULL;

  switch (reinit)
  {
    case BRIDGEMGR_REINIT_TYPE_INIT:
      reinit_str = "INIT";
      break;

    case BRIDGEMGR_REINIT_TYPE_CLEANUP:
      reinit_str = "CLEANUP";
      break;

    case BRIDGEMGR_REINIT_TYPE_FULL:
      reinit_str = "FULL";
      break;

    default:
      reinit_str = "INVALID";
      break;
  }

  return reinit_str;
}


/*===========================================================================
  FUNCTION  bridgemgr_common_read
===========================================================================*/
/*!
@brief
  This function reads data from the given fd into the buffer pointed by data.
  If the available data exceeds the buffer capacity, excess data is read and
  discarded and an error is returned

@param
  fd   - registered file descriptor
  data - buffer to read the data into
  size - allocated size of the data buffer
  fill_size [out] - size of the buffer actually filled by this function

@return
  BRIDGEMGR_SUCCESS - If data was successfully read
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_common_read
(
  bridgemgr_sys_type  sys,
  int                 fd,
  void                *data,
  int                 size,
  int                 *fill_size
)
{
  int ret   = BRIDGEMGR_FAILURE;
  int nread = 0;


  /* Validate the input */
  if (!BRIDGEMGR_IS_VALID_FD(fd) ||
      NULL == data               ||
      NULL == fill_size          ||
      size <= 0)
  {
    bridgemgr_log_err("bridgemgr_common_read: bad params sys=%s",
                      bridgemgr_common_get_sys_str(sys));
    goto bail;
  }

  /* Read data into buffer upto available capacity */
  if ((nread = read(fd, (unsigned char *)data, size)) <= 0)
  {
    bridgemgr_log_err("bridgemgr_common_read: read failed sys=%s\n",
                      bridgemgr_common_get_sys_str(sys));
    goto bail;
  }

  /* TODO: We shouldn't reach the buffer limit. But for some reason we did then
     flush out any remaining bytes */
  if (nread == size)
  {
    /* If we reached buffer limit, drain out any excess data and return error */
    bridgemgr_log_err("bridgemgr_common_read: read buffer limit reached sys=%s\n",
                      bridgemgr_common_get_sys_str(sys));
  }

  *fill_size = nread;
  ret = BRIDGEMGR_SUCCESS;

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_common_wait_for_init
===========================================================================*/
/*!
@brief
  This function delays the calling thread until the bridgemgr initialization
  is complete

@param
  sys - Module enum

@return
  none

*/
/*=========================================================================*/
void bridgemgr_common_wait_for_init
(
  bridgemgr_sys_type sys
)
{
  int i = 0;


  while (FALSE == bridgemgr_init_complete)
  {
    bridgemgr_log_low("bridgemgr_common_wait_for_init: sys=%s attempt=%d\n",
                      bridgemgr_common_get_sys_str(sys), ++i);

    usleep(BRIDGEMGR_RETRY_DELAY);
  }
}


/*===========================================================================
  FUNCTION  bridgemgr_common_print_qmux_msg
===========================================================================*/
/*!
@brief
  This function prints the hexadecimal representation of the given message

@param
  sys  - module enum
  data - integer data
  size - size of the data

@return
  none

*/
/*=========================================================================*/
void bridgemgr_common_print_qmux_msg
(
  bridgemgr_sys_type sys,
  const void *msg,
  int size
)
{
  int i, j;
  char buff[3*BRIDGEMGR_COMMON_NUM_BYTES_PER_LINE+1] = "";
  unsigned int upper_half;
  unsigned int lower_half;
  const unsigned char *data = (const unsigned char *)msg;


  if (TRUE == bridgemgr_debug_log_qmux_msg)
  {
    bridgemgr_log_low("bridgemgr_common_print_qmux_msg: sys=%s, size=%d\n",
                      bridgemgr_common_get_sys_str(sys),
                      size);

    for (i = 1, j = 0; i <= size; ++i, ++data)
    {
      upper_half = (*data) >> 4;
      lower_half = (*data) & 0x0F;
      buff[j++]  = bridgemgr_common_to_hex_char(upper_half);
      buff[j++]  = bridgemgr_common_to_hex_char(lower_half);
      buff[j++]  = ' ';

      if (i % BRIDGEMGR_COMMON_NUM_BYTES_PER_LINE == 0)
      {
        buff[j] = '\0';
        bridgemgr_log_low("%s\n", buff);
        j = 0;
      }
    }

    /* Print any remaining data */
    if (j > 0)
    {
      buff[j] = '\0';
      bridgemgr_log_low("%s\n", buff);
    }
  }
}


/*===========================================================================
  FUNCTION  bridgemgr_common_signal_handler
===========================================================================*/
/*!
@brief
  This is the common signal handler for all modules

@param
  sig - Signal being delivered

@return
  None

*/
/*=========================================================================*/
void bridgemgr_common_signal_handler
(
  int sig
)
{
  switch (sig)
  {
    case SIGUSR1:
      /* On USR1 signal, toggle the QMUX message logging flag */
      bridgemgr_debug_log_qmux_msg = (bridgemgr_debug_log_qmux_msg) ? FALSE : TRUE;
      bridgemgr_log_med("Signal Handler - Setting QMUX logging debug flag: %s\n",
                        (TRUE == bridgemgr_debug_log_qmux_msg) ? "TRUE" : "FALSE");
      break;

    case SIGUSR2:
      /* On USR2, calling thread exits */
      bridgemgr_log_med("bridgemgr_signal_handler: thread exiting\n");
      pthread_exit(NULL);
      break;

    case SIGTERM:
      /* On TERM signal, exit() so atexit cleanup functions gets called */
      exit(0);
      break;

    default:
      break;
  }
}


/*===========================================================================
  FUNCTION  bridgemgr_common_issue_reinit_request
===========================================================================*/
/*!
@brief
  This function is used to issue requests to SYS_REINIT module for
  re-initializing other modules

@param
  sys         - Module to re-initialize
  reinit_type - Type of reinit request

@return
  BRIDGEMGR_SUCCESS - If request was successfully issued
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_common_issue_reinit_request
(
  bridgemgr_sys_type     sys,
  bridgemgr_reinit_type  reinit_type
)
{
  int ret = BRIDGEMGR_FAILURE;

  /* Allocate a reinit module cmd */
  bridgemgr_cmdq_cmd_type *cmd = bridgemgr_cmdq_alloc_cmd(BRIDGEMGR_SYS_REINIT);


  if (NULL == cmd)
  {
    bridgemgr_log_err("bridgemgr_common_issue_reinit_cmd: cmd alloc failed\n");
    goto bail;
  }

  bridgemgr_log_med("bridgemgr_common_issue_reinit_cmd: issuing reinit request "
                    "sys=%s, type=%s\n",
                    bridgemgr_common_get_sys_str(sys),
                    bridgemgr_common_get_reinit_str(reinit_type));

  /* Update the reinit request */
  cmd->cmd_data.data.reinit.sys  = sys;
  cmd->cmd_data.data.reinit.type = reinit_type;

  /* Enqueue the command in the command queue */
  if (BRIDGEMGR_SUCCESS != bridgemgr_cmdq_enqueue_cmd(cmd))
  {
    bridgemgr_log_err("bridgemgr_common_issue_reinit_cmd: cmd enq failed\n");
    bridgemgr_cmdq_free_cmd(cmd);
    goto bail;
  }

  ret = BRIDGEMGR_SUCCESS;

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_common_cleanup_cb
===========================================================================*/
/*!
@brief
  This is the common cleanup callback registered by different modules

@param
  cb - pointer to client registered callbacks

@return
  None

*/
/*=========================================================================*/
void bridgemgr_common_cleanup_cb
(
  bridgemgr_sys_type               sys,
  bridgemgr_client_callbacks_type  *cb
)
{
  int s;


  if (NULL == cb)
  {
    bridgemgr_log_err("bridgemgr_common_cleanup_cb: bad param(s) sys=%s\n",
                      bridgemgr_common_get_sys_str(sys));
    return;
  }

  if (BRIDGEMGR_SUCCESS != bridgemgr_common_stop_thread(cb->read_thread_id))
  {
    bridgemgr_log_err("bridgemgr_common_cleanup_cb: sys=%s read thread stop failure\n",
                      bridgemgr_common_get_sys_str(sys));
  }
  else
  {
    bridgemgr_log_med("bridgemgr_common_cleanup_cb: sys=%s read thread stop success\n",
                      bridgemgr_common_get_sys_str(sys));
  }

  BRIDGEMGR_FD_CLOSE(cb->fd);
}


/*===========================================================================
  FUNCTION  bridgemgr_common_stop_thread
===========================================================================*/
/*!
@brief
  This function is used to stop a given thread

@param
  thread_id - ID of the thread to stop

@return
  BRIDGEMGR_SUCCESS - If thread was successfully stopped
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_common_stop_thread
(
  pthread_t  thread_id
)
{
  int ret = BRIDGEMGR_FAILURE;
  int s;


  /* Signal the read thread to exit */
  s = pthread_kill(thread_id, SIGUSR2);
  if (0 != s)
  {
    bridgemgr_log_err("bridgemgr_common_stop_thread: pthread_cancel failed\n");
  }
  else
  {
    s = pthread_join(thread_id, NULL);
    if (0 != s)
    {
      bridgemgr_log_err("bridgemgr_common_stop_thread: pthread_join failed\n");
    }
    else
    {
      ret = BRIDGEMGR_SUCCESS;
    }
  }

  return ret;
}

