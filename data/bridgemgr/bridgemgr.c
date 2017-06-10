/******************************************************************************

                           B R I D G E M G R . C

******************************************************************************/

/******************************************************************************

  @file    bridgemgr.c
  @brief   Bridge Manager Main Implementation File

  DESCRIPTION
  Implementation file for BridgeMgr functions.

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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "bridgemgr.h"
#include "bridgemgr_usb_qmi.h"
#include "bridgemgr_qmi_proxy.h"
#include "bridgemgr_mdm_qmi.h"
#include "bridgemgr_port_switch.h"
#include "ds_util.h"
#include "ds_cmdq.h"


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

static int bridgemgr_reinit_init(bridgemgr_client_callbacks_type *client_cb);


/* Init function pointer type */
typedef int (*bridgemgr_init_func_type)(bridgemgr_client_callbacks_type *client_cb);

/* Callback functions that clients can register */
typedef struct
{
  bridgemgr_sys_type              sys;
  bridgemgr_init_func_type        init;
  bridgemgr_client_callbacks_type client_cb;
} bridgemgr_client_info_type;

/* Table of client info of different subsystems */
static bridgemgr_client_info_type bridgemgr_client_info_tbl[] =
{
  { BRIDGEMGR_SYS_PS_QMI_IND, bridgemgr_ps_qmi_ind_init, { BRIDGEMGR_FD_INVALID, NULL, NULL, 0 } },
  { BRIDGEMGR_SYS_PS_NETLINK, bridgemgr_ps_netlink_init, { BRIDGEMGR_FD_INVALID, NULL, NULL, 0 } },
  { BRIDGEMGR_SYS_MDM_QMI,    bridgemgr_mdm_qmi_init,    { BRIDGEMGR_FD_INVALID, NULL, NULL, 0 } },
  { BRIDGEMGR_SYS_QMI_PROXY,  bridgemgr_qmi_proxy_init,  { BRIDGEMGR_FD_INVALID, NULL, NULL, 0 } },
  { BRIDGEMGR_SYS_USB_QMI,    bridgemgr_usb_qmi_init,    { BRIDGEMGR_FD_INVALID, NULL, NULL, 0 } },

  /* Special system for re-initializing other modules */
  { BRIDGEMGR_SYS_REINIT,     bridgemgr_reinit_init,     { BRIDGEMGR_FD_INVALID, NULL, NULL, 0 } },

  /* This should be the last entry */
  { BRIDGEMGR_SYS_INVALID,    NULL,                      { BRIDGEMGR_FD_INVALID, NULL, NULL, 0 } }
};

/* QMI handle */
static int bridgemgr_qmi_hndl = QMI_INVALID_CLIENT_HANDLE;

/* Flag indicating bridgemgr initialization completion */
boolean bridgemgr_init_complete = FALSE;

#ifdef FEATURE_DATA_INTERNAL_LOG_TO_FILE
FILE *bm_fp = NULL;
#endif


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_get_client_info_tbl_entry
===========================================================================*/
/*!
@brief
  This function returns the bridgemgr_client_info_tbl[] entry corresponding to
  the given subsystem 'sys'

@param
  sys - subsystem being requested

@return
  Pointer to bridgemgr_client_info_tbl[] entry - sys is valid
  NULL - otherwise

*/
/*=========================================================================*/
static const bridgemgr_client_info_type *bridgemgr_get_client_info_tbl_entry
(
  bridgemgr_sys_type sys
)
{
  const bridgemgr_client_info_type *info = NULL;


  for (info = &bridgemgr_client_info_tbl[0]; info->sys != BRIDGEMGR_SYS_INVALID; ++info)
  {
    if (info->sys == sys)
    {
      break;
    }
  }

  return info;
}


/*===========================================================================
  FUNCTION  bridgemgr_reinit_process_cb
===========================================================================*/
/*!
@brief
  This is the processing callback registered by BRIDGMGR_SYS_REINIT module

@param
  data - data to be processed
  size - size of the data

@return
  BRIDGEMGR_SUCCESS - If processing was successful
  BRIDGEMGR_FAILURE - Otherwise

@note 
  This function is called in the context of cmdq thread 
*/
/*=========================================================================*/
static int bridgemgr_reinit_process_cb
(
  const void *data,
  int size
)
{
  int ret = BRIDGEMGR_FAILURE;
  bridgemgr_client_info_type *info = NULL;
  bridgemgr_cmdq_reinit_cmd_type *reinit = (bridgemgr_cmdq_reinit_cmd_type *)data;


  if (NULL == reinit || size <= 0)
  {
    bridgemgr_log_err("bridgemgr_reinit_proccess_cb: bad params\n");
    goto bail;
  }

  info = (bridgemgr_client_info_type *)bridgemgr_get_client_info_tbl_entry(reinit->sys);

  if (NULL == info)
  {
    bridgemgr_log_err("bridgemgr_reinit_proccess_cb: invalid info_tbl entry sys=%s\n",
                      bridgemgr_common_get_sys_str(reinit->sys));
    goto bail;
  }

  bridgemgr_log_high("bridgemgr_reinit_proccess_cb: reiniting sys=%s, type=%s\n",
                     bridgemgr_common_get_sys_str(reinit->sys),
                     bridgemgr_common_get_reinit_str(reinit->type));

  /* First perform module cleanup, if necessary */
  if (reinit->type & BRIDGEMGR_REINIT_TYPE_CLEANUP)
  {
    if (NULL == info->client_cb.cleanup_func)
    {
      bridgemgr_log_err("bridgemgr_reinit_proccess_cb: invalid cleanup func sys=%s\n",
                        bridgemgr_common_get_sys_str(reinit->sys));
    }
    else
    {
      /* Call the cleanup function */
      info->client_cb.cleanup_func(info->sys, &info->client_cb);
      ret = BRIDGEMGR_SUCCESS;

      bridgemgr_log_high("bridgemgr_reinit_proccess_cb: cleanup ret=%s, sys=%s\n",
                         BRIDGEMGR_RET_TO_STR(ret),
                         bridgemgr_common_get_sys_str(reinit->sys));
    }
  }

  /* Initialize the module */
  if (reinit->type & BRIDGEMGR_REINIT_TYPE_INIT)
  {
    if (NULL == info->init)
    {
      bridgemgr_log_err("bridgemgr_reinit_proccess_cb: invalid init func sys=%s\n",
                        bridgemgr_common_get_sys_str(reinit->sys));
    }
    else
    {
      /* Call the init function */
      ret = info->init(&info->client_cb);

      bridgemgr_log_high("bridgemgr_reinit_proccess_cb: init ret=%s, sys=%s\n",
                         BRIDGEMGR_RET_TO_STR(ret),
                         bridgemgr_common_get_sys_str(reinit->sys));
    }
  }

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_reinit_init
===========================================================================*/
/*!
@brief
  This function is called to initialize the BRIDGEMGR_SYS_REINIT module

@param
  client_cb - Set of callbacks to register

@return
  BRIDGEMGR_SUCCESS - Module initialization was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
static int bridgemgr_reinit_init
(
  bridgemgr_client_callbacks_type *client_cb
)
{
  int ret = BRIDGEMGR_FAILURE;


  if (NULL == client_cb)
  {
    bridgemgr_log_err("bridgemgr_reinit_init: bad params\n");
    goto bail;
  }

  client_cb->fd           = BRIDGEMGR_FD_NONE;
  client_cb->process_func = bridgemgr_reinit_process_cb;
  client_cb->cleanup_func = NULL;

  ret = BRIDGEMGR_SUCCESS;

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_setup_logging
===========================================================================*/
/*!
@brief
  This function sets up the logging subsystem

@param
  None

@return
  None

*/
/*=========================================================================*/
static void bridgemgr_setup_logging(void)
{
#ifdef FEATURE_DATA_LOG_SYSLOG
  /* Initialize logging as per desired mode */
  bridgemgr_log_init(DS_DBG_LEVEL_DFLT, DS_LOG_MODE_DFLT);
#endif

#ifdef FEATURE_DATA_LOG_QXDM
  /* Initialize Diag services */
  if ( TRUE != Diag_LSM_Init(NULL) )
  {
    bridgemgr_log_err("failed on Diag_LSM_Init\n" );
  }
#endif

#ifdef FEATURE_DATA_INTERNAL_LOG_TO_FILE
  bm_fp = fopen("/data/bridgemgr.txt", "w+");
  if (NULL == bm_fp)
  {
    bridgemgr_log_err("failed on open /data/bridgemgr.txt\n" );
  }
#endif
}


/*===========================================================================
  FUNCTION  bridgemgr_cleanup
===========================================================================*/
/*!
@brief
  This function calls the registered cleanup function of each subsystem and
  deinitializes QXDM

@param
  None

@return
  None

@note

*/
/*=========================================================================*/
static void bridgemgr_cleanup(void)
{
  bridgemgr_client_info_type *info = NULL;


  if (QMI_INVALID_CLIENT_HANDLE != bridgemgr_qmi_hndl)
  {
    /* Release QMI handle */
    qmi_release(bridgemgr_qmi_hndl);
  }

  /* Cleanup the command queue */
  bridgemgr_cmdq_deinit();

  /* Call the cleanup function for each subsystem */
  for (info = &bridgemgr_client_info_tbl[0]; info->sys != BRIDGEMGR_SYS_INVALID; ++info)
  {
    if (NULL != info->client_cb.cleanup_func && BRIDGEMGR_FD_INVALID != info->client_cb.fd)
    {
      info->client_cb.cleanup_func(info->sys, &info->client_cb);
    }
  }

  /* Deinit diag */
#ifdef FEATURE_DATA_LOG_QXDM
  (void) Diag_LSM_DeInit();
#endif

#ifdef FEATURE_DATA_INTERNAL_LOG_TO_FILE
  if (NULL != bm_fp)
  {
    fclose(bm_fp);
  }
#endif
}


/*===========================================================================
  FUNCTION  bridgemgr_init
===========================================================================*/
/*!
@brief
  This is the main initialization function. It calls the init function of
  each subsystem and obtains the set of client callbacks and file descriptors
  to wait on

@param
  None

@return
  BRIDGEMGR_SUCCESS - All initialization was successful
  BRIDGEMGR_FAILURE - Otherwise
*/
/*=========================================================================*/
static int bridgemgr_init(void)
{
  int ret_val = BRIDGEMGR_SUCCESS;
  bridgemgr_client_info_type *info = NULL;


  /* Initialize QMI */
  if (QMI_INVALID_CLIENT_HANDLE == (bridgemgr_qmi_hndl = qmi_init(NULL, NULL)))
  {
    bridgemgr_log_err("bridgemgr_init: qmi_init failed\n");
    return BRIDGEMGR_FAILURE;
  }

  /* Initialize the command queue */
  bridgemgr_cmdq_init();

  /* Initialize each of the subsystems */
  for (info = &bridgemgr_client_info_tbl[0]; BRIDGEMGR_SYS_INVALID != info->sys; ++info)
  {
    bridgemgr_client_callbacks_type *client_cb = &info->client_cb;

    if (BRIDGEMGR_SUCCESS != info->init(client_cb))
    {
      bridgemgr_log_err("bridgemgr_init: init failed for sys=%s\n",
                        bridgemgr_common_get_sys_str(info->sys));
      ret_val = BRIDGEMGR_FAILURE;
    }
    else
    {
      bridgemgr_log_med("bridgemgr_init: init succeeded for sys=%s\n",
                        bridgemgr_common_get_sys_str(info->sys));
    }
  }

  bridgemgr_init_complete = TRUE;

  return ret_val;
}


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_cmd_dispatcher
===========================================================================*/
/*!
@brief
  This function calls the processing function of the subsystem module for
  which the command is destined

@param
  cmd_data - information about the command data

@return
  BRIDGEMGR_SUCCESS - if command data was successfully processed
  BRIDGEMGR_FAILURE - otherwise

@note
  This function gets called in the context of cmdq thread

*/
/*=========================================================================*/
int bridgemgr_cmd_dispatcher
(
  bridgemgr_cmdq_cmd_data_type *cmd_data
)
{
  int ret = BRIDGEMGR_FAILURE;
  const bridgemgr_client_info_type *info = NULL;
  bridgemgr_sys_type sys;
  void *data;


  if (NULL == cmd_data || !BRIDGEMGR_IS_VALID_SYS_TYPE(cmd_data->sys))
  {
    bridgemgr_log_err("bridgemgr_cmd_data_handler: bad params\n");
    goto bail;
  }

  sys = cmd_data->sys;
  info = bridgemgr_get_client_info_tbl_entry(sys); 

  if (NULL == info)
  {
    bridgemgr_log_err("bridgemgr_cmd_data_handler: client info not found sys=%s\n",
                      bridgemgr_common_get_sys_str(sys));
  }
  else if (NULL == info->client_cb.process_func)
  {
    bridgemgr_log_err("bridgemgr_cmd_data_handler: data processing func NULL sys=%s\n",
                      bridgemgr_common_get_sys_str(sys));
  }
  else
  {
    void *data;
    int size;


    if (BRIDGEMGR_SYS_PS_QMI_IND == sys)
    {
      data = &cmd_data->data.wds_ind;
      size = sizeof(qmi_wds_indication_data_type);
    }
    else if (BRIDGEMGR_SYS_REINIT == sys)
    {
      data = &cmd_data->data.reinit;
      size = sizeof(cmd_data->data.reinit);
    }
    else
    {
      data = cmd_data->data.qmux_nl.msg;
      size = cmd_data->data.qmux_nl.msg_len;
    }

    ret = info->client_cb.process_func(data, size);
  }

bail:
  return ret;
}

/*===========================================================================
  FUNCTION  bridgemgr_forward_data
===========================================================================*/
/*!
@brief
  This function is used to forward data to the given subsystem module

@param
  sys  - The subsystem module to send the data to
  data - Pointer to the data to be sent
  size - Size of the data

@return
  BRIDGEMGR_SUCCESS - if data was successfully sent
  BRIDGEMGR_FAILURE - otherwise

*/
/*=========================================================================*/
int bridgemgr_forward_data
(
  bridgemgr_sys_type sys,
  const void *data,
  int size
)
{
  int ret = BRIDGEMGR_FAILURE;
  const bridgemgr_client_info_type *info = NULL;


  if (!BRIDGEMGR_IS_VALID_SYS_TYPE(sys))
  {
    bridgemgr_log_err("bridgemgr_forward_data: bad params sys=%s\n",
                      bridgemgr_common_get_sys_str(sys));
    goto bail;
  }

  /* Find the entry corresponding to the given subsystem module */
  info = bridgemgr_get_client_info_tbl_entry(sys);

  if (NULL == info)
  {
    bridgemgr_log_err("bridgemgr_forward_data: client info not found sys=%s\n",
                      bridgemgr_common_get_sys_str(sys));
  }
  else if (!BRIDGEMGR_IS_VALID_FD(info->client_cb.fd))
  {
    bridgemgr_log_err("bridgemgr_forward_data: invalid fd=%d, sys=%s\n",
                      info->client_cb.fd,
                      bridgemgr_common_get_sys_str(sys));
  }
  else
  {
    if (size != write(info->client_cb.fd, data, size))
    {
      bridgemgr_log_err("bridgemgr_forward_data: writing data to fd failed for sys=%s\n",
                        bridgemgr_common_get_sys_str(sys));
    }
    else
    {
      ret = BRIDGEMGR_SUCCESS;
    }
  }

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  main
===========================================================================*/
/*!
@brief
  The main function of the bridgemgr daemon process

@param
  argc - Number of arguments
  argv - Argurments array

@return
  None

*/
/*=========================================================================*/
int main
(
  int argc,
  char *argv[]
)
{
  struct sigaction action;


  /* Setup QXDM logging */
  bridgemgr_setup_logging();

  /* Cleanup function to be called at exit */
  atexit(bridgemgr_cleanup);

  /* Register signal handlers with the kernel */
  memset(&action, 0, sizeof(action));
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = bridgemgr_common_signal_handler;

  if (sigaction(SIGUSR1, &action, NULL) < 0)
  {
    bridgemgr_log_err("main: SIGUSR1 sigaction failed\n");
  }

  if (sigaction(SIGTERM, &action, NULL) < 0)
  {
    bridgemgr_log_err("main: SIGTERM sigaction failed\n");
  }

  /* Perform initialization */
  if (BRIDGEMGR_SUCCESS != bridgemgr_init())
  {
    bridgemgr_log_err("main: some subsystems failed to initialize properly\n");
  }

  /* Wait indefinitely for cmdq thread */
  bridgemgr_cmdq_wait();

  exit(0);
}

