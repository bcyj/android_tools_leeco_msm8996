/******************************************************************************

                  B R I D G E M G R _ P O R T _ S W I T C H . C

******************************************************************************/

/******************************************************************************

  @file    bridgemgr_port_switch.c
  @brief   Bridge Manager Port Switch Functionality Implementation File

  DESCRIPTION
  Implementation file for BridgeMgr port switch functions.

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

#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>

#include "ds_string.h"
#include "bridgemgr.h"
#include "bridgemgr_usb_qmi.h"
#include "bridgemgr_port_switch.h"


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/* Macro for validating the call status indication */
#define BRIDGEMGR_PS_VALIDATE_CALL_STATUS(cs)              \
           ((QMI_WDS_DATA_CALL_ACTIVATED  == (cs)) ||      \
            (QMI_WDS_DATA_CALL_TERMINATED == (cs)))

/* Macro for validating the call type */
#define BRIDGEMGR_PS_VALIDATE_CALL_TYPE(ct)                \
           ((QMI_WDS_DATA_CALL_TYPE_EMBEDDED == (ct)) ||   \
            (QMI_WDS_DATA_CALL_TYPE_TETHERED == (ct)))

/* Macro for validating the tether type */
#define BRIDGEMGR_PS_VALIDATE_TETHER_TYPE(tt)              \
           ((QMI_WDS_TETHERED_CALL_TYPE_RMNET == (tt)) ||  \
            (QMI_WDS_TETHERED_CALL_TYPE_DUN   == (tt)))

/* Macro for validating the new transport */
#define BRIDGEMGR_PS_VALIDATE_TRANSPORT(nt)                \
          ((BRIDGEMGR_PS_SMD_TRANSPORT  == nt) ||          \
           (BRIDGEMGR_PS_SDIO_TRANSPORT == nt))


/* USB driver's configuration file which must be written with the appropriate
   value to perform a port switch */
#define BRIDGEMGR_PS_USB_SYSFS_CONFIG   "/sys/class/android_usb/f_rmnet_smd_sdio/transport"

/* Supported transports */
#define BRIDGEMGR_PS_INVALID_TRANSPORT  -1
#define BRIDGEMGR_PS_SMD_TRANSPORT       0
#define BRIDGEMGR_PS_SDIO_TRANSPORT      1

/* Values to write to the USB sysfs config for switching to the
   corresponding transport */
#define BRIDGEMGR_PS_USB_CFG_SMD        "0"
#define BRIDGEMGR_PS_USB_CFG_SDIO       "1"
#define BRIDGEMGR_PS_USB_CFG_SIZE        2

/* Strings to parse in the USB netlink message */
#define BRIDGEMGR_PS_USB_SUBSYSTEM                "SUBSYSTEM="
#define BRIDGEMGR_PS_USB_SUBSYSTEM_LEN            (sizeof(BRIDGEMGR_PS_USB_SUBSYSTEM)-1)

#define BRIDGEMGR_PS_USB_SWITCH_NAME              "SWITCH_NAME="
#define BRIDGEMGR_PS_USB_SWITCH_NAME_LEN          (sizeof(BRIDGEMGR_PS_USB_SWITCH_NAME)-1)

#define BRIDGEMGR_PS_USB_SWITCH_STATE             "SWITCH_STATE="
#define BRIDGEMGR_PS_USB_SWITCH_STATE_LEN         (sizeof(BRIDGEMGR_PS_USB_SWITCH_STATE)-1)

#define BRIDGEMGR_PS_USB_SUBSYSTEM_VAL            "switch"
#define BRIDGEMGR_PS_USB_SUBSYSTEM_VAL_LEN        (sizeof(BRIDGEMGR_PS_USB_SUBSYSTEM_VAL))

#define BRIDGEMGR_PS_USB_SWITCH_NAME_VAL          "MSM72K_UDC"
#define BRIDGEMGR_PS_USB_SWITCH_NAME_VAL_LEN      (sizeof(BRIDGEMGR_PS_USB_SWITCH_NAME_VAL))

#define BRIDGEMGR_PS_USB_SWITCH_STATE_ONLINE      "online"
#define BRIDGEMGR_PS_USB_SWITCH_STATE_ONLINE_LEN  (sizeof(BRIDGEMGR_PS_USB_SWITCH_STATE_ONLINE))

#define BRIDGEMGR_PS_USB_SWITCH_STATE_OFFLINE     "offline"
#define BRIDGEMGR_PS_USB_SWITCH_STATE_OFFLINE_LEN (sizeof(BRIDGEMGR_PS_USB_SWITCH_STATE_OFFLINE))

/* Group mask for receving netlink events from kernel */
#define BRIDGEMGR_PS_GRP_MASK_NL_EVTS   ((unsigned int)0xFFFFFFFF)

/* User data when registering with QMI WDS Service */
#define BRIDGEMGR_PS_USER_DATA          (0x0BADFEED)


/* Structure containing port switching related state */
typedef struct bridgemgr_port_switch_info
{
  int      wds_clnt_hndl;  /* WDS client handle */
  boolean  enabled;        /* Flag indicating if the functionlity is enabled */
  boolean  pending;        /* Flag indicating if a port switch is pending */
  int      cur_transport;  /* Current transport */
  qmi_wds_pref_data_sys_type     cur_pref_sys;     /* Current preferred system */
  qmi_wds_data_call_status_type  cur_call_status;  /* Current call status */
} bridgemgr_port_switch_info;

/* Initialize the port switching state to default */
static bridgemgr_port_switch_info bridgemgr_port_switch =
{
  QMI_INVALID_CLIENT_HANDLE,         /* wds_clnt_hndl */
  FALSE,                             /* enabled */
  FALSE,                             /* pending */
  BRIDGEMGR_PS_INVALID_TRANSPORT,    /* cur_transport */
  QMI_WDS_DATA_SYS_UNKNOWN,          /* cur_pref_sys */
  QMI_WDS_DATA_CALL_TERMINATED,      /* cur_call_status */
};

typedef struct
{
  qmi_wds_pref_data_sys_type  pref_sys;
  int                         transport;
} bridgemgr_ps_transport_type;

/* Table containing the transport to use for a given preferred system */
static bridgemgr_ps_transport_type bridgemgr_port_switch_transport_tbl[] =
{
  { QMI_WDS_DATA_SYS_CDMA_1X, BRIDGEMGR_PS_SMD_TRANSPORT  },
  { QMI_WDS_DATA_SYS_GPRS,    BRIDGEMGR_PS_SMD_TRANSPORT  },
  { QMI_WDS_DATA_SYS_WCDMA,   BRIDGEMGR_PS_SMD_TRANSPORT  },
  { QMI_WDS_DATA_SYS_EVDO,    BRIDGEMGR_PS_SDIO_TRANSPORT },
  { QMI_WDS_DATA_SYS_LTE,     BRIDGEMGR_PS_SDIO_TRANSPORT },

  /* This should be the last entry */
  { QMI_WDS_DATA_SYS_UNKNOWN, BRIDGEMGR_PS_INVALID_TRANSPORT }
};


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_ps_netlink_read
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
static void *bridgemgr_ps_netlink_read
(
  void *arg
)
{
  int fd = (int)arg;
  int ret = BRIDGEMGR_FAILURE;
  bridgemgr_cmdq_cmd_type *cmd = NULL;
  int count;
  struct sockaddr_nl sa;
  struct iovec iov;
  struct msghdr msg = { (void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0 };


  bridgemgr_log_low("bridgemgr_ps_netlink_read: reading from fd=%d\n", fd);

  /* Wait for initialization to complete */
  bridgemgr_common_wait_for_init(BRIDGEMGR_SYS_PS_NETLINK);

  for (;;)
  {
     /* Allocate a new cmd */
     bridgemgr_cmdq_cmd_type *cmd = bridgemgr_cmdq_alloc_cmd(BRIDGEMGR_SYS_PS_NETLINK);

    if (NULL == cmd)
    {
      bridgemgr_log_err("bridgemgr_ps_netlink_read: cmd alloc failed\n");
      break;
    }

    memset(&sa, 0, sizeof(sa));

    iov.iov_base = cmd->cmd_data.data.qmux_nl.msg;
    iov.iov_len  = BRIDGEMGR_CMDQ_MAX_QMUX_NETLINK_MSG;

    /* Get the netlink message */
    count = recvmsg(fd, &msg, 0);

    if (-1 == count)
    {
      bridgemgr_log_err("bridgemgr_ps_netlink_read: recvmsg failed\n");

      bridgemgr_cmdq_free_cmd(cmd);
      usleep(BRIDGEMGR_RETRY_DELAY);
    }
    else
    {
      /* Update the actual message length */
      cmd->cmd_data.data.qmux_nl.msg_len = count;

      /* Enqueue the command in the command queue */
      if (BRIDGEMGR_SUCCESS != bridgemgr_cmdq_enqueue_cmd(cmd))
      {
        bridgemgr_log_err("bridgemgr_ps_netlink_read: cmd enq failed\n");
        bridgemgr_cmdq_free_cmd(cmd);
      }
    }
  }

  return NULL;
}


/*===========================================================================
  FUNCTION  bridgemgr_perform_port_switch
===========================================================================*/
/*!
@brief
  This function informs the USB driver to switch to the corresponding
  transport (via sysfs config)

@param
  new_transport - The new transport to switch to

@return
  none

*/
/*=========================================================================*/
static void bridgemgr_perform_port_switch
(
  int new_transport
)
{
  FILE *fp = NULL;
  char cur_usb_cfg[BRIDGEMGR_PS_USB_CFG_SIZE];
  const char *new_usb_cfg;

  /* Validate the new transport */
  if (!BRIDGEMGR_PS_VALIDATE_TRANSPORT(new_transport))
  {
    bridgemgr_log_err("bridgemgr_perform_port_switch: "
                      "invalid transport %d\n", new_transport);
    return;
  }

  /* Try to open the sysfs config file for reading and writing */
  if (NULL == (fp = fopen(BRIDGEMGR_PS_USB_SYSFS_CONFIG, "w+")))
  {
    bridgemgr_log_err("bridgemgr_perform_port_switch: "
                      "failed to open USB sysfs config\n");
    return;
  }

  bridgemgr_log_med("bridgemgr_perform_port_switch: "
                    "cur_transport=%d, new_transport=%d\n",
                    bridgemgr_port_switch.cur_transport, new_transport);

  if (BRIDGEMGR_PS_SMD_TRANSPORT == new_transport)
  {
    new_usb_cfg = BRIDGEMGR_PS_USB_CFG_SMD;
  }
  else if (BRIDGEMGR_PS_SDIO_TRANSPORT == new_transport)
  {
    new_usb_cfg = BRIDGEMGR_PS_USB_CFG_SDIO;
  }

  /* Make sure that the USB driver isn't already using the new transport */
  if (1 != fread(cur_usb_cfg, sizeof(cur_usb_cfg), 1, fp) ||
      0 != strncmp(cur_usb_cfg, new_usb_cfg, BRIDGEMGR_PS_USB_CFG_SIZE))
  {
    bridgemgr_log_med("bridgemgr_perform_port_switch: "
                      "writing transport=%s to USB config\n", new_usb_cfg);

    /* Go to the beginning of the file */
    rewind(fp);

    /* Inform the USB driver to use the new transport */
    if (1 != fwrite(new_usb_cfg, BRIDGEMGR_PS_USB_CFG_SIZE, 1, fp))
    {
      bridgemgr_log_err("bridgemgr_perform_port_switch: "
                        "write to USB config failed\n");
    }
  }

  fclose(fp);
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_netlink_process_cb
===========================================================================*/
/*!
@brief
  This is the processing callback registered by BRIDGMGR_SYS_PS_NETLINK module

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
static int bridgemgr_ps_netlink_process_cb
(
  const void *data,
  int  size
)
{
  int ret = BRIDGEMGR_FAILURE;
  const char *str = (const char *)data;
  const char *end = NULL;
  char subsystem[BRIDGEMGR_MAX_STR_LEN]    = "";
  char switch_name[BRIDGEMGR_MAX_STR_LEN]  = "";
  char switch_state[BRIDGEMGR_MAX_STR_LEN] = "";


  if (NULL == data || size <= 0)
  {
    bridgemgr_log_err("bridgemgr_ps_netlink_process_cb: bad params\n");
    goto bail;
  }

  end = str + size;

  /* Extract each substring and copy any relevant fields */
  while (str < end)
  {
    /* Copy the USB event data (if any) into corresponding buffers */
    if (0 == std_strnicmp(str, BRIDGEMGR_PS_USB_SUBSYSTEM, BRIDGEMGR_PS_USB_SUBSYSTEM_LEN))
    {
      strlcpy(subsystem, (str + BRIDGEMGR_PS_USB_SUBSYSTEM_LEN), BRIDGEMGR_MAX_STR_LEN);
    }
    else if (0 == std_strnicmp(str, BRIDGEMGR_PS_USB_SWITCH_NAME, BRIDGEMGR_PS_USB_SWITCH_NAME_LEN))
    {
      strlcpy(switch_name, (str + BRIDGEMGR_PS_USB_SWITCH_NAME_LEN), BRIDGEMGR_MAX_STR_LEN);
    }
    else if (0 == std_strnicmp(str, BRIDGEMGR_PS_USB_SWITCH_STATE, BRIDGEMGR_PS_USB_SWITCH_STATE_LEN))
    {
      strlcpy(switch_state, (str + BRIDGEMGR_PS_USB_SWITCH_STATE_LEN), BRIDGEMGR_MAX_STR_LEN);
    }

    /* Go to the start of next substring in the data */
    str += strlen(str) + 1;
  }

  /* Make sure this is the USB netlink event we are interested in */
  if (0 == std_strnicmp(subsystem,
                        BRIDGEMGR_PS_USB_SUBSYSTEM_VAL,
                        BRIDGEMGR_PS_USB_SUBSYSTEM_VAL_LEN) &&
      0 == std_strnicmp(switch_name,
                        BRIDGEMGR_PS_USB_SWITCH_NAME_VAL,
                        BRIDGEMGR_PS_USB_SWITCH_NAME_VAL_LEN))
  {
    bridgemgr_log_err("got switch_state=%s\n", switch_state);

    /* Update sysfs config if the switch_state is online and port switch
       has been already initialized */
    if (0 == std_strnicmp(switch_state,
                          BRIDGEMGR_PS_USB_SWITCH_STATE_ONLINE,
                          BRIDGEMGR_PS_USB_SWITCH_STATE_ONLINE_LEN))
    {
      if (BRIDGEMGR_PS_INVALID_TRANSPORT == bridgemgr_port_switch.cur_transport)
      {
        bridgemgr_log_err("usb is connected, current transport invalid=%d\n",
                          bridgemgr_port_switch.cur_transport);
      }
      else
      {
        bridgemgr_log_med("usb is connected, switching to current transport=%d\n",
                          bridgemgr_port_switch.cur_transport);

        bridgemgr_perform_port_switch(bridgemgr_port_switch.cur_transport);
      }

      /* USB cable is connected, initialize SYS_USB_QMI */
      if (BRIDGEMGR_SUCCESS != bridgemgr_common_issue_reinit_request(BRIDGEMGR_SYS_USB_QMI,
                                                                     BRIDGEMGR_REINIT_TYPE_INIT))
      {
        bridgemgr_log_med("bridgemgr_ps_netlink_process_cb: failed to init sys=%s\n",
                          bridgemgr_common_get_sys_str(BRIDGEMGR_SYS_USB_QMI));
        goto bail;
      }
    }
    else if (0 == std_strnicmp(switch_state,
                               BRIDGEMGR_PS_USB_SWITCH_STATE_OFFLINE,
                               BRIDGEMGR_PS_USB_SWITCH_STATE_OFFLINE_LEN))
    {
      /* USB cable is disconnected, cleanup SYS_USB_QMI */
      if (BRIDGEMGR_SUCCESS != bridgemgr_common_issue_reinit_request(BRIDGEMGR_SYS_USB_QMI,
                                                                     BRIDGEMGR_REINIT_TYPE_CLEANUP))
      {
        bridgemgr_log_med("bridgemgr_ps_netlink_process_cb: failed to cleanup sys=%s\n",
                          bridgemgr_common_get_sys_str(BRIDGEMGR_SYS_USB_QMI));
        goto bail;
      }
    }

    ret = BRIDGEMGR_SUCCESS;
  }

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_get_transport
===========================================================================*/
/*!
@brief
  This function returns the transport corresponding to the given
  preferred system

@param
  pref_sys - The new preferred system

@return
  New transport value

*/
/*=========================================================================*/
static int bridgemgr_ps_get_transport
(
  qmi_wds_pref_data_sys_type pref_sys
)
{
  bridgemgr_ps_transport_type *tt = &bridgemgr_port_switch_transport_tbl[0];


  for (; QMI_WDS_DATA_SYS_UNKNOWN != tt->pref_sys; ++tt)
  {
    if (tt->pref_sys == pref_sys)
    {
      break;
    }
  }

  bridgemgr_log_med("bridgemgr_ps_get_transport: returning transport=%d\n", tt->transport);
  return tt->transport;
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_pref_sys_ind
===========================================================================*/
/*!
@brief
  This function handles the QMI_WDS_EVENT_PREF_DATA_SYS_IND indication.
  Performs port switch if the call is currently disconnected. Otherwise,
  toggles the port switch pending flag.

@param
  pref_sys - The new preferred system

@return
  none

*/
/*=========================================================================*/
static void bridgemgr_ps_pref_sys_ind
(
  qmi_wds_pref_data_sys_type pref_sys
)
{
  int new_transport;


  if (FALSE == bridgemgr_port_switch.enabled)
  {
    bridgemgr_log_err("bridgemgr_ps_pref_sys_ind: port switch disabled\n");
    goto bail;
  }

  new_transport = bridgemgr_ps_get_transport(pref_sys);

  if (BRIDGEMGR_PS_INVALID_TRANSPORT == new_transport)
  {
    bridgemgr_log_err("bridgemgr_ps_pref_sys_ind: no valid transport found for "
                      "pref_sys=%d\n", pref_sys);
    goto bail;
  }

  /* If there is no change in the transport */
  if (bridgemgr_port_switch.cur_transport == new_transport)
  {
    bridgemgr_log_err("bridgemgr_ps_pref_sys_ind: "
                      "no change in transport=%d\n", new_transport);
    goto bail;
  }

  bridgemgr_log_low("bridgemgr_ps_pref_sys_ind: "
                    "cur_transport=%d, new_transport=%d\n",
                    bridgemgr_port_switch.cur_transport, new_transport);

  /* If the call is disconnected, perform the port switch right away.
     Otherwise, toggle the port switch pending flag and do any necessary
     switch when we get a disconnected indication later */
  if (QMI_WDS_DATA_CALL_TERMINATED == bridgemgr_port_switch.cur_call_status)
  {
    bridgemgr_perform_port_switch(new_transport);
  }
  else
  {
    /* Toggle the port switch pending flag */
    bridgemgr_log_low("bridgemgr_ps_pref_sys_ind: "
                      "toggling flag pending=%d\n", bridgemgr_port_switch.pending);

    bridgemgr_port_switch.pending = !bridgemgr_port_switch.pending;
  }

  bridgemgr_port_switch.cur_pref_sys  = pref_sys;
  bridgemgr_port_switch.cur_transport = new_transport;

bail:
  return;
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_call_status_ind
===========================================================================*/
/*!
@brief
  This function handles the QMI_WDS_EVENT_DATA_CALL_STATUS_CHG_IND
  indication. If this is a call disconnect indicaiton and a port switch is
  pending then it informs the USB driver of the transport change.

@param
  call_status - The new call status
  call_type   - Type of the call (embedded or tethered)
  tether_type - Type of a tether call - DUN or Rmnet

@return
  none

*/
/*=========================================================================*/
static void bridgemgr_ps_call_status_ind
(
  qmi_wds_data_call_status_type call_status,
  qmi_wds_data_call_type_type call_type,
  qmi_wds_tethered_call_type_type tether_type
)
{
  if (FALSE == bridgemgr_port_switch.enabled)
  {
    bridgemgr_log_err("bridgemgr_ps_call_status_ind: port switch disabled\n");
    goto bail;
  }

  if (!BRIDGEMGR_PS_VALIDATE_CALL_STATUS(call_status) ||
      !BRIDGEMGR_PS_VALIDATE_CALL_TYPE(call_type)     ||
      !BRIDGEMGR_PS_VALIDATE_TETHER_TYPE(tether_type))
  {
    bridgemgr_log_err("bridgemgr_ps_call_status_ind: "
                      "call_status=%d, call_type=%d, tether_type=%d\n",
                      call_status, call_type, tether_type);
    goto bail;
  }

  /* Make sure this indication is from sdio port on which we registered for
     the indications earlier and it is a rmnet tethered call indication */
  if ((QMI_WDS_DATA_CALL_TYPE_TETHERED == call_type)  &&
      (QMI_WDS_TETHERED_CALL_TYPE_RMNET == tether_type))
  {
    bridgemgr_log_low("bridgemgr_ps_call_status_ind: "
                      "handling call_status=%d, cur_call_status=%d\n",
                      call_status, bridgemgr_port_switch.cur_call_status);

    /* If there is no change in call status */
    if (bridgemgr_port_switch.cur_call_status == call_status)
    {
      bridgemgr_log_low("bridgemgr_ps_call_status_ind: "
                        "no change in call_status=%d\n", call_status);
      goto bail;
    }

    bridgemgr_log_low("bridgemgr_ps_call_status_ind: "
                   "call_status=%d\n", call_status);

    /* Switch the transport if the call was disconnected and
       a port switch is pending */
    if (QMI_WDS_DATA_CALL_TERMINATED == call_status &&
        TRUE == bridgemgr_port_switch.pending)
    {
      bridgemgr_perform_port_switch(bridgemgr_port_switch.cur_transport);
      bridgemgr_port_switch.pending = FALSE;
    }

    bridgemgr_port_switch.cur_call_status = call_status;
  }
  else
  {
    bridgemgr_log_err("bridgemgr_ps_call_status_ind: "
                      "received call_status=%d, call_type=%d, tether_type=%d\n",
                      call_status, call_type, tether_type);
  }

bail:
  return;
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_qmi_ind_process_cb
===========================================================================*/
/*!
@brief
  This is the processing callback registered by BRIDGMGR_SYS_PS_QMI_IND module

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
static int bridgemgr_ps_qmi_ind_process_cb
(
  const void *data,
  int size
)
{
  qmi_wds_indication_data_type *ind_data = NULL;
  unsigned long event_mask = 0;
  int ret = BRIDGEMGR_FAILURE;


  if (NULL == data || size <= 0)
  {
    bridgemgr_log_err("bridgemgr_ps_qmi_ind_proccess_cb: bad params\n");
    goto bail;
  }

  ind_data = (qmi_wds_indication_data_type *)data;

  event_mask = ind_data->event_report.event_mask;

  /* Handle preferred system change indication */
  if (0 != (event_mask & QMI_WDS_EVENT_PREF_DATA_SYS_IND))
  {
    /* Perform necessary port switch if the preferred system changed */
    bridgemgr_ps_pref_sys_ind(ind_data->event_report.pref_data_sys);
  }

  /* Handle data call status change indication. */
  if (0 != (event_mask & QMI_WDS_EVENT_DATA_CALL_STATUS_CHG_IND))
  {
    /* The data call type indication should have also been set */
    if(0 == (event_mask & QMI_WDS_EVENT_DATA_CALL_TYPE_IND))
    {
      bridgemgr_log_err("bridgemgr_wds_event_report_ind: got call status change ind "
                        "without call type ind");
      goto bail;
    }

    bridgemgr_ps_call_status_ind(
      ind_data->event_report.data_call_status_change.data_call_status,
      ind_data->event_report.data_call_type.data_call_type,
      ind_data->event_report.data_call_type.tethered_call_type
    );

    ret = BRIDGEMGR_SUCCESS;
  }

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_qmi_ind_cleanup_cb
===========================================================================*/
/*!
@brief
  This is the cleanup callback registered by BRIDGMGR_SYS_PS_QMI_IND module

@param
  cb - pointer to client registered callbacks

@return
  None

*/
/*=========================================================================*/
static void bridgemgr_ps_qmi_ind_cleanup_cb
(
  bridgemgr_sys_type               sys,
  bridgemgr_client_callbacks_type  *cb
)
{
  int qmi_err;


  if (BRIDGEMGR_SYS_PS_QMI_IND != sys || NULL == cb || BRIDGEMGR_FD_NONE != cb->fd)
  {
    bridgemgr_log_err("bridgemgr_ps_qmi_ind_cleanup_cb: bad param(s)\n");
    return;
  }

  bridgemgr_log_med("bridgemgr_ps_qmi_ind_cleanup_cb: cleaning wds_clnt_hndl=0x%x\n",
                    bridgemgr_port_switch.wds_clnt_hndl);

  if (QMI_INVALID_CLIENT_HANDLE != bridgemgr_port_switch.wds_clnt_hndl)
  {
    (void)qmi_wds_srvc_release_client(bridgemgr_port_switch.wds_clnt_hndl, &qmi_err);
  }

  /* Invalidate the port switching state */
  bridgemgr_port_switch.wds_clnt_hndl   = QMI_INVALID_CLIENT_HANDLE;
  bridgemgr_port_switch.enabled         = FALSE;
  bridgemgr_port_switch.pending         = FALSE;
  bridgemgr_port_switch.cur_transport   = BRIDGEMGR_PS_INVALID_TRANSPORT;
  bridgemgr_port_switch.cur_pref_sys    = QMI_WDS_DATA_SYS_UNKNOWN;
  bridgemgr_port_switch.cur_call_status = QMI_WDS_DATA_CALL_TERMINATED;
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_qmi_wds_ind_cb
===========================================================================*/
/*!
@brief
  This is the callback function registered for receiving QMI WDS indications.
  This function creates allocates a new command, updates it with the received
  indication data and enqueues it for processing by the cmdq thread

@param
  user_handle
  service_id
  user_data
  ind_id
  ind_data

@return
  None

*/
/*=========================================================================*/
static void bridgemgr_ps_qmi_wds_ind_cb
(
  int                           user_handle,
  qmi_service_id_type           service_id,
  void                          *user_data,
  qmi_wds_indication_id_type    ind_id,
  qmi_wds_indication_data_type  *ind_data
)
{
  bridgemgr_cmdq_cmd_type *cmd = NULL;


  /* Only handle event report indications which match our user data */
  if (QMI_WDS_SERVICE != service_id               ||
      QMI_WDS_SRVC_EVENT_REPORT_IND_MSG != ind_id ||
      BRIDGEMGR_PS_USER_DATA != (int)user_data    ||
      NULL == ind_data)
  {
    bridgemgr_log_err("bridgemgr_ps_wds_ind_cb: unrecognized ind recvd "
                      "service=%d, user_data=0x%x, ind_id=%d\n",
                      service_id, (int)user_data, ind_id);
    goto bail;
  }

  bridgemgr_log_high("bridgemgr_ps_wds_ind_cb: enqueuing received ind "
                     "service=%d, user_data=0x%x, ind_id=%d\n",
                     service_id, (int)user_data, ind_id);

  if (NULL == (cmd = bridgemgr_cmdq_alloc_cmd(BRIDGEMGR_SYS_PS_QMI_IND)))
  {
    bridgemgr_log_err("bridgemgr_ps_wds_ind_cb: cmd alloc failed\n");
    goto bail;
  }

  cmd->cmd_data.data.wds_ind = *ind_data;

  if (BRIDGEMGR_SUCCESS != bridgemgr_cmdq_enqueue_cmd(cmd))
  {
    bridgemgr_log_err("bridgemgr_ps_wds_ind_cb: cmd enq failed\n");
    bridgemgr_cmdq_free_cmd(cmd);
  }

bail:
  return;
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_is_config_present
===========================================================================*/
/*!
@brief
  Checks if USB config file required for port switch is present and writable

@param
  none

@return
  TRUE  - if the config file is accessible
  FALSE - otherwise

*/
/*=========================================================================*/
static boolean bridgemgr_ps_is_config_present(void)
{
  boolean present = FALSE;

  /* Check if the config file exists and is writable */
  if (0 == access(BRIDGEMGR_PS_USB_SYSFS_CONFIG, F_OK | W_OK))
  {
    present = TRUE;
  }

  return present;
}



/*===========================================================================
  FUNCTION  bridgemgr_ps_qmi_conn_init
===========================================================================*/
/*!
@brief
  This function initializes the QMI connection for SDIO port 0 and returns
  the QMI WDS client handle

@param
  wds_clnt_hndl [out] - QMI WDS client handle

@return
  BRIDGEMGR_SUCCESS - If initialization was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
static int bridgemgr_ps_qmi_conn_init
(
  int *wds_clnt_hndl
)
{
  int ret = BRIDGEMGR_FAILURE;
  int qmi_err;
  int qmi_ret;
  int clnt_hndl;
  int retry_cnt;


  if (NULL == wds_clnt_hndl)
  {
    bridgemgr_log_err("bridgemgr_ps_qmi_conn_init: bad params\n");
    goto bail;
  }

  /* Initialize the connection on first SDIO port */
  if (QMI_NO_ERR != (qmi_ret = qmi_connection_init(QMI_PORT_RMNET_SDIO_0, &qmi_err)))
  {
    bridgemgr_log_err("bridgemgr_ps_qmi_conn_init: failed to init "
                      "conn=%s, ret=%d, err=%d\n",
                      QMI_PORT_RMNET_SDIO_0, qmi_ret, qmi_err);
    goto bail;
  }

  for (retry_cnt = 1; retry_cnt <= BRIDGEMGR_MAX_RETRY_COUNT; ++retry_cnt)
  {
    if ((clnt_hndl = qmi_wds_srvc_init_client(QMI_PORT_RMNET_SDIO_0,
                                              bridgemgr_ps_qmi_wds_ind_cb,
                                              (void *)BRIDGEMGR_PS_USER_DATA,
                                              &qmi_err)) < 0)
    {
      bridgemgr_log_err("bridgemgr_ps_qmi_conn_init: qmi_wds_srvc_init_client failed "
                        "conn=%s, ret=%d, err=%d, attempt=%d\n",
                        QMI_PORT_RMNET_SDIO_0, clnt_hndl, qmi_err, retry_cnt);
      usleep(BRIDGEMGR_RETRY_DELAY);
    }
    else
    {
      *wds_clnt_hndl = clnt_hndl;
      ret = BRIDGEMGR_SUCCESS;
      break;
    }
  }

  if (retry_cnt > BRIDGEMGR_MAX_RETRY_COUNT)
  {
    bridgemgr_log_err("bridgemgr_ps_qmi_conn_init: qmi_wds_srvc_init_client failed "
                      "conn=%s, error=%d\n",
                      QMI_PORT_RMNET_SDIO_0, qmi_err);
  }

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_qmi_register_events
===========================================================================*/
/*!
@brief
  Register for QMI WDS indications for preferred system and call status change

@param
  wds_clnt_hndl - QMI WDS client handle

@return
  BRIDGEMGR_SUCCESS - If event registration was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
static int bridgemgr_ps_qmi_register_events
(
  int wds_clnt_hndl
)
{
  qmi_wds_event_report_params_type event_report_params;
  int qmi_err;
  int qmi_ret;
  int ret = BRIDGEMGR_FAILURE;
  int retry_cnt;


  /* Register for CALL_STATUS_CHG, CALL_TYPE, PREF_DATA_SYS indications */
  memset(&event_report_params, 0, sizeof(event_report_params));

  event_report_params.param_mask |= QMI_WDS_EVENT_DATA_CALL_STATUS_CHG_IND;
  event_report_params.param_mask |= QMI_WDS_EVENT_DATA_CALL_TYPE_IND;
  event_report_params.param_mask |= QMI_WDS_EVENT_PREF_DATA_SYS_IND;

  event_report_params.report_data_call_status_chg = TRUE;
  event_report_params.report_pref_data_sys        = TRUE;


  for (retry_cnt = 1; retry_cnt <= BRIDGEMGR_MAX_RETRY_COUNT; ++retry_cnt)
  {
    if (QMI_NO_ERR != (qmi_ret = qmi_wds_set_event_report(wds_clnt_hndl,
                                                          &event_report_params,
                                                          &qmi_err)))
    {
      bridgemgr_log_err("bridgemgr_ps_qmi_register_events: "
                        "unable to register for qmi indications ret=%d, err=%d, attempt=%d\n",
                        qmi_ret, qmi_err, retry_cnt);
    }
    else
    {
      ret = BRIDGEMGR_SUCCESS;
      break;
    }
  }

  if (retry_cnt > BRIDGEMGR_MAX_RETRY_COUNT)
  {
    bridgemgr_log_err("bridgemgr_ps_qmi_register_events: "
                      "unable to register for qmi indications err=%d\n", qmi_err);
  }

  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_set_default_transport
===========================================================================*/
/*!
@brief
  Inform the USB driver to switch to the default transport

@param
  None

@return
  none

@note
*/
/*=========================================================================*/
static void bridgemgr_ps_set_default_transport(void)
{
  int def_transport;

#if defined(FEATURE_PS_USE_DEFAULT_SMD)

  /* Use SMD as the default transport */
  bridgemgr_port_switch.cur_pref_sys    = QMI_WDS_DATA_SYS_WCDMA;
  bridgemgr_port_switch.cur_call_status = QMI_WDS_DATA_CALL_TERMINATED;

#elif defined(FEATURE_PS_USE_DEFAULT_SDIO)

  /* Use SDIO as the default transport */
  bridgemgr_port_switch.cur_pref_sys    = QMI_WDS_DATA_SYS_LTE;
  bridgemgr_port_switch.cur_call_status = QMI_WDS_DATA_CALL_TERMINATED;

#elif defined(FEATURE_PS_USE_MODEM_QUERY)

  /*
   * TODO: Use the new modem query API to initialize the current preferred
   *       system and switch USB driver to use the corresponding transport
  */
  bridgemgr_port_switch.cur_pref_sys    = QMI_WDS_DATA_SYS_LTE;
  bridgemgr_port_switch.cur_call_status = QMI_WDS_DATA_CALL_TERMINATED;

#else

  /* Set to invalid state */
  bridgemgr_port_switch.cur_pref_sys    = QMI_WDS_DATA_SYS_UNKNOWN;
  bridgemgr_port_switch.cur_call_status = QMI_WDS_DATA_CALL_TERMINATED;
  bridgemgr_port_switch.cur_transport   = BRIDGEMGR_PS_INVALID_TRANSPORT;

#endif

  /* Get the transport corresponding to the default preferred system */
  def_transport = bridgemgr_ps_get_transport(bridgemgr_port_switch.cur_pref_sys);

  /* Switch the USB driver to the default transport */
  bridgemgr_perform_port_switch(def_transport);
  bridgemgr_port_switch.cur_transport = def_transport;
}


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_ps_netlink_init
===========================================================================*/
/*!
@brief
  This function is called to initialize the BRIDGEMGR_SYS_PS_NETLINK module

@param
  client_cb - Set of callbacks to register

@return
  BRIDGEMGR_SUCCESS - Module initialization was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_ps_netlink_init
(
  bridgemgr_client_callbacks_type *client_cb
)
{
  struct sockaddr_nl loc_addr;
  int sock_fd = BRIDGEMGR_FD_INVALID;
  int ret = BRIDGEMGR_FAILURE;
  pthread_t thr;


  if (NULL == client_cb)
  {
    bridgemgr_log_err("bridgemgr_ps_netlink_init: invalid params\n");
    goto bail;
  }

  /* Create a netlink socket to receive kernel user events */
  if ((sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT)) < 0)
  {
    bridgemgr_log_err("bridgemgr_ps_netlink_init: socket() failed\n");
    goto bail;
  }
  
  /* Initialize the local address */
  memset(&loc_addr, 0, sizeof(struct sockaddr_nl));

  loc_addr.nl_family = AF_NETLINK;
  loc_addr.nl_pid    = getpid();  /* self pid */
  loc_addr.nl_groups = BRIDGEMGR_PS_GRP_MASK_NL_EVTS;

  /* Bind the socket to our local address */
  if (bind(sock_fd, (struct sockaddr*)&loc_addr, sizeof(loc_addr)) < 0)
  {
    bridgemgr_log_err("bridgemgr_ps_netlink_init: bind() failed\n");
    goto bail;
  }

  if (pthread_create(&thr, NULL, &bridgemgr_ps_netlink_read, (void *)sock_fd))
  {
    bridgemgr_log_err("bridgemgr_usb_qmi_init: failed to create read thread\n");
    goto bail;
  }

  /* Update the client callback info */
  client_cb->fd             = sock_fd;
  client_cb->process_func   = bridgemgr_ps_netlink_process_cb;
  client_cb->cleanup_func   = bridgemgr_common_cleanup_cb;
  client_cb->read_thread_id = thr;

  ret = BRIDGEMGR_SUCCESS;

bail:
  if (BRIDGEMGR_SUCCESS != ret)
  {
    BRIDGEMGR_FD_CLOSE(sock_fd);
  }

  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_ps_qmi_init
===========================================================================*/
/*!
@brief
  This function is called to initialize the BRIDGEMGR_SYS_PS_QMI_IND module

@param
  client_cb - Set of callbacks to register

@return
  BRIDGEMGR_SUCCESS - Module initialization was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_ps_qmi_ind_init
(
  bridgemgr_client_callbacks_type *client_cb
)
{
  int ret = BRIDGEMGR_FAILURE;
  int qmi_err;
  int wds_clnt_hndl = QMI_INVALID_CLIENT_HANDLE;


  if (NULL == client_cb)
  {
    bridgemgr_log_err("bridgemgr_ps_qmi_ind_init: bad params\n");
    goto bail;
  }

  if (FALSE == bridgemgr_ps_is_config_present())
  {
    bridgemgr_log_err("bridgemgr_ps_qmi_ind_init: port switch not enabled\n");
    goto bail;
  }

  if (BRIDGEMGR_SUCCESS != bridgemgr_ps_qmi_conn_init(&wds_clnt_hndl))
  {
    bridgemgr_log_err("bridgemgr_ps_qmi_ind_init: connection init failed\n");
    goto bail;
  }

  bridgemgr_log_med("bridgemgr_ps_qmi_ind_init: wds_clnt_handle=0x%x\n",
                    wds_clnt_hndl);

  if (BRIDGEMGR_SUCCESS != bridgemgr_ps_qmi_register_events(wds_clnt_hndl))
  {
    bridgemgr_log_err("bridgemgr_ps_qmi_ind_init: event registration failed\n");
    goto bail;
  }

  /* Enable port switch functionality */
  bridgemgr_port_switch.wds_clnt_hndl = wds_clnt_hndl;
  bridgemgr_port_switch.enabled       = TRUE;
  bridgemgr_port_switch.pending       = FALSE;

  /* Switch the USB driver to the default transport */
  bridgemgr_ps_set_default_transport();

  /* Update the client callback info */
  client_cb->fd           = BRIDGEMGR_FD_NONE;
  client_cb->process_func = bridgemgr_ps_qmi_ind_process_cb;
  client_cb->cleanup_func = bridgemgr_ps_qmi_ind_cleanup_cb;

  ret = BRIDGEMGR_SUCCESS;

bail:
  if (BRIDGEMGR_SUCCESS != ret && QMI_INVALID_CLIENT_HANDLE != wds_clnt_hndl)
  {
    (void)qmi_wds_srvc_release_client(wds_clnt_hndl, &qmi_err);
  }

  return ret;
}

